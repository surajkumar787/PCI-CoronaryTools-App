// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "messagebaseimpl.h"

#include "exception.h"
#include "item.h"

#include <mergecom.h>
#include <iomanip>
#include <mc3media.h>
#include <array>
#include <sstream>
#include <algorithm>

#undef min
#undef max

namespace CommonPlatform { namespace Dicom { namespace Merge
{
	bool mcCheckGet(MC_STATUS status, unsigned long tagOrElm)
	{
		switch(status)
		{
		case MC_NORMAL_COMPLETION:
		case MC_EMPTY_VALUE:
			//Ok
			return true;
		case MC_NO_MORE_VALUES:
		case MC_INVALID_TAG:
		case MC_NULL_VALUE:
		case MC_NULL_POINTER_PARM:
			return false;
		default:
			throw Exception(status, std::string("Cannot get tagOrElm ") + to_hstring(tagOrElm));
		}
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, std::string & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);

		unsigned long valueLength(0);
		status = pBlock
			? MC_Get_pValue_Length(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, valNr, &valueLength)
			: MC_Get_Value_Length (msg, tagOrElm, valNr, &valueLength);
		switch(status)
		{
		case MC_NORMAL_COMPLETION:
			//Ok
			break;
		case MC_EMPTY_VALUE:
		case MC_NULL_VALUE:
			value.clear();
			return true;
			break;
		default:
			return false;
			break;
		}

		MC_VR vr(UNKNOWN_VR);
		int numValues(0);
		status = pBlock
			? MC_Get_pAttribute_Info(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &vr, &numValues)
			: MC_Get_Attribute_Info (msg, tagOrElm, &vr, &numValues);

		char * bufferPtr(nullptr);
		int bufferSize(0);
		std::array<char, 128> smallStackBuffer;
		bool smallStackBufferUsed(false);
		bool toTagFormat = false;
		switch (vr)
		{
		default:
			value.resize(valueLength+1);
			bufferPtr = &value[0];
			bufferSize = static_cast<int>(value.size());
			break;
		case UL:
		case SL:        
		case US:
		case SS:
		case FL:
		case FD:
		case SQ:
			smallStackBufferUsed = true;
			bufferPtr = &smallStackBuffer[0];
			bufferSize = int(smallStackBuffer.size());
			break;
		case AT:
			toTagFormat = true;
			bufferSize = 0;
			break;
		}

		if (!toTagFormat)
		{
			if(pBlock != nullptr)
			{
				status = valNr > 1
					? MC_Get_Next_pValue_To_String(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, bufferSize, bufferPtr)
					: MC_Get_pValue_To_String     (msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, bufferSize, bufferPtr);
			} else {
				status = valNr > 1
					? MC_Get_Next_Value_To_String(msg, tagOrElm, bufferSize, bufferPtr)
					: MC_Get_Value_To_String     (msg, tagOrElm, bufferSize, bufferPtr);
			}
		}
		else
		{
			uint32_t resVal;
			auto res = getImpl(msg, pBlock, tagOrElm, resVal, valNr);
			if (res)
			{
				std::ostringstream ss;
				ss << '(' << std::hex << std::setw(4) << std::setfill('0') << (resVal >> 16) << ',' << std::setw(4) << (resVal & 0xFFFF) << ')';
				value = ss.str();
				return true;
			}
			else
			{
				return false;
			}
		}

		switch(status)
		{
		case MC_NORMAL_COMPLETION:
			if(smallStackBufferUsed)
			{
				smallStackBuffer[smallStackBuffer.size()-1] = 0; //Ensure proper termination
				value = &smallStackBuffer[0];
			} else {
				value.resize(std::strlen(value.c_str()));
				value.shrink_to_fit();
			}
			return true;
		case MC_EMPTY_VALUE:
		case MC_NULL_VALUE:
			value.clear();
			return true;
		case MC_INCOMPATIBLE_VR:
			{
				//read into a buffer
				if(valNr != 1) return false;
				int valueSizeInBytes;
				pBlock
					? mcCheck(MC_Get_pValue_To_Buffer(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value.size(), &value[0], &valueSizeInBytes))
					: mcCheck(MC_Get_Value_To_Buffer (msg, tagOrElm, value.size(), &value[0], &valueSizeInBytes));
				return true;
			}
		case MC_NO_MORE_VALUES:
		case MC_INVALID_TAG:
		case MC_BUFFER_TOO_SMALL:
			return false;
		default:
			{
				std::string e("Cannot get tagOrElm ");
				e += pBlock ? to_hstring(*pBlock, tagOrElm & 0xff) : to_hstring(tagOrElm);
				throw Exception(status, e);
			}
		}
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, std::wstring & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);

		MC_VR valueRepresentation;
		status = MC_Get_Attribute_Info(msg, tagOrElm, &valueRepresentation, nullptr);

		if (mcCheckGet(status, tagOrElm) &&
			(valueRepresentation == MC_VR::LO ||
				valueRepresentation == MC_VR::LT ||
				valueRepresentation == MC_VR::PN ||
				valueRepresentation == MC_VR::SH ||
				valueRepresentation == MC_VR::ST ||
				valueRepresentation == MC_VR::UT ||
				valueRepresentation == MC_VR::UC))
		{
			value.reserve(1000);
			int size(0);
			if ((pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_UnicodeString(msg, tagOrElm, (int)value.capacity(), &size, (MC_UChar*)&value[0]);
			else if ((pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_UnicodeString(msg, tagOrElm, (int)value.capacity(), &size, (MC_UChar*)&value[0]);
			else return false;
		}
		else
		{
			std::string tmp;
			if (!getImpl(msg, pBlock, tagOrElm, tmp, valNr)) return false;
			value = fromUtf8(tmp);
		}

		return mcCheckGet(status, tagOrElm);
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int8_t & value, int valNr)
	{
		short tmp(0);
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if(      (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_ShortInt         (msg, tagOrElm, &tmp);
		else if( (pBlock == nullptr) && valNr >  1) status = MC_Get_Next_Value_To_ShortInt    (msg, tagOrElm, &tmp);
		else if( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_ShortInt        (msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else if( (pBlock != nullptr) && valNr >  1) status = MC_Get_Next_pValue_To_ShortInt   (msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else return false;
		auto result = mcCheckGet(status, tagOrElm);                

		if (tmp > std::numeric_limits<int8_t>::max() || tmp < std::numeric_limits<int8_t>::min())
		{
			throw Exception(MC_SYSTEM_ERROR, "Over- or underflow while reading int8_t for tag: " + to_hstring(tagOrElm));
		}

		value = static_cast<int8_t>(tmp);
		return result;
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint8_t & value, int valNr)
	{
		unsigned short tmp(0);
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if(		  (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_UShortInt(msg, tagOrElm, &tmp);
		else if ( (pBlock == nullptr) && valNr >  1) status = MC_Get_Next_Value_To_UShortInt(msg, tagOrElm, &tmp);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_UShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else if ( (pBlock != nullptr) && valNr >  1) status = MC_Get_Next_pValue_To_UShortInt   (msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else return false;
		auto result = mcCheckGet(status, tagOrElm);

		if (tmp > std::numeric_limits<uint8_t>::max())
		{
			throw Exception(MC_SYSTEM_ERROR, "Overflow while reading uint8_t for tag: " + to_hstring(tagOrElm));
		}
		value = uint8_t(tmp);

		return result;
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int16_t & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if(		  (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_ShortInt(msg, tagOrElm, &value);
		else if ( (pBlock == nullptr) && valNr >  1) status = MC_Get_Next_Value_To_ShortInt(msg, tagOrElm, &value);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_ShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else if ( (pBlock != nullptr) && valNr >  1) status = MC_Get_Next_pValue_To_ShortInt   (msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else return false;
		return mcCheckGet(status, tagOrElm);
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint16_t & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (	  (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_UShortInt(msg, tagOrElm, &value);
		else if ( (pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_UShortInt(msg, tagOrElm, &value);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_UShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else if ( (pBlock != nullptr) && valNr > 1) status = MC_Get_Next_pValue_To_UShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else return false;
		return mcCheckGet(status, tagOrElm);
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int32_t & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if(       (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_Int(msg, tagOrElm, &value);
		else if ( (pBlock == nullptr) && valNr >  1) status = MC_Get_Next_Value_To_Int(msg, tagOrElm, &value);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_Int(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else if ( (pBlock != nullptr) && valNr >  1) status = MC_Get_Next_pValue_To_Int   (msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else return false;
		return mcCheckGet(status, tagOrElm);
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint32_t & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (      (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_UInt(msg, tagOrElm, &value);
		else if ( (pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_UInt(msg, tagOrElm, &value);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_UInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else if ( (pBlock != nullptr) && valNr > 1) status = MC_Get_Next_pValue_To_UInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else return false;
		return mcCheckGet(status, tagOrElm);
	}


	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int64_t & value, int valNr)
	{
		long tmp;
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (      (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_LongInt(msg, tagOrElm, &tmp);
		else if ( (pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_LongInt(msg, tagOrElm, &tmp);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_LongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else if ( (pBlock != nullptr) && valNr > 1) status = MC_Get_Next_pValue_To_LongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else return false;
		auto result = mcCheckGet(status, tagOrElm);
		value = tmp;
		return result;
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint64_t & value, int valNr)
	{
		unsigned long tmp;
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (      (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_ULongInt(msg, tagOrElm, &tmp);
		else if ( (pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_ULongInt(msg, tagOrElm, &tmp);
		else if ( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_ULongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else if ( (pBlock != nullptr) && valNr > 1) status = MC_Get_Next_pValue_To_ULongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &tmp);
		else return false;
		auto result = mcCheckGet(status, tagOrElm);
		value = tmp;
		return result;
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, float & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (     (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_Float(msg, tagOrElm, &value);
		else if( (pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_Float(msg, tagOrElm, &value);
		else if( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_Float(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else if( (pBlock != nullptr) && valNr > 1) status = MC_Get_Next_pValue_To_Float(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else return false;
		return mcCheckGet(status, tagOrElm);
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, double & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (     (pBlock == nullptr) && valNr == 1) status = MC_Get_Value_To_Double(msg, tagOrElm, &value);
		else if( (pBlock == nullptr) && valNr > 1) status = MC_Get_Next_Value_To_Double(msg, tagOrElm, &value);
		else if( (pBlock != nullptr) && valNr == 1) status = MC_Get_pValue_To_Double(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else if( (pBlock != nullptr) && valNr > 1) status = MC_Get_Next_pValue_To_Double(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &value);
		else return false;
		return mcCheckGet(status, tagOrElm);
	}

	bool getImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, bool & value, int valNr)
	{
		uint8_t tmp;
		auto result = getImpl(msg, pBlock, tagOrElm, tmp, valNr);
		value = tmp != 0;
		return result;
	}



	bool mcCheckSet(MC_STATUS status, unsigned long tagOrElm)
	{
		switch(status)
		{
		case MC_NORMAL_COMPLETION:
			//Ok
			return true;
		case MC_INVALID_CHARS_IN_VALUE:
		case MC_INVALID_VALUE_FOR_VR:
			return false;
		default:
			throw Exception(status, std::string("Cannot set tagOrElm ") + to_hstring(tagOrElm));
		}
	}

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, const std::string  & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);
		if (     (pBlock == nullptr) && valNr == 1) status = MC_Set_Value_From_String(msg, tagOrElm, value.c_str());
		else if( (pBlock == nullptr) && valNr > 1) status = MC_Set_Next_Value_From_String(msg, tagOrElm, value.c_str());
		else if( (pBlock != nullptr) && valNr == 1) status = MC_Set_pValue_From_String(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value.c_str());
		else if( (pBlock != nullptr) && valNr > 1) status = MC_Set_Next_pValue_From_String(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value.c_str());
		else return false;
		return mcCheckSet(status, tagOrElm);
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, const std::wstring & value, int valNr)
	{
		MC_STATUS status(MC_NORMAL_COMPLETION);

		MC_VR valueRepresentation;
		status = MC_Get_Attribute_Info(msg, tagOrElm, &valueRepresentation, nullptr);

		if (mcCheckSet(status, tagOrElm) &&
			(valueRepresentation == MC_VR::LO ||
			valueRepresentation == MC_VR::LT ||
			valueRepresentation == MC_VR::PN ||
			valueRepresentation == MC_VR::SH ||
			valueRepresentation == MC_VR::ST ||
			valueRepresentation == MC_VR::UT ||
			valueRepresentation == MC_VR::UC))
		{
			if ((pBlock == nullptr) && valNr == 1) status = MC_Set_Value_From_UnicodeString(msg, tagOrElm, (int)value.length(), (const MC_UChar*)value.c_str());
			else if ((pBlock == nullptr) && valNr > 1) status = MC_Set_Next_Value_From_UnicodeString(msg, tagOrElm, (int)value.length(), (const MC_UChar*)value.c_str());
			else return false;
		}
		else
		{
			return setImpl(msg, pBlock, tagOrElm, toUtf8(value), valNr);
		}

		return mcCheckSet(status, tagOrElm);
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int8_t value, int valNr)
	{
		return setImpl(msg, pBlock, tagOrElm, static_cast<int16_t>(value), valNr);
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint8_t value, int valNr)
	{
		return setImpl(msg, pBlock, tagOrElm, static_cast<uint16_t>(value), valNr);
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int16_t value, int valNr)
	{
		if (     (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_ShortInt(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_ShortInt(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_ShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_ShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint16_t value, int valNr)
	{
		if ( (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_UShortInt(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_UShortInt(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_UShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_UShortInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int32_t value, int valNr)
	{
		if ( (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_Int(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_Int(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_Int(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_Int(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint32_t value, int valNr)
	{
		if (      (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_UInt(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_UInt(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_UInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_UInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, int64_t value, int valNr)
	{
		if(value > std::numeric_limits<long>::max()) throw Exception(MC_SYSTEM_ERROR, "Overflow");
		if(value < std::numeric_limits<long>::min()) throw Exception(MC_SYSTEM_ERROR, "Underflow");
		if (      (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_LongInt(msg, tagOrElm, static_cast<long>(value)), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_LongInt(msg, tagOrElm, static_cast<long>(value)), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_LongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, static_cast<long>(value)), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_LongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, static_cast<long>(value)), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, uint64_t value, int valNr)
	{
		if(value > std::numeric_limits<unsigned long>::max()) throw Exception(MC_SYSTEM_ERROR, "Overflow");
		if (     (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_ULongInt(msg, tagOrElm, static_cast<unsigned long>(value)), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_ULongInt(msg, tagOrElm, static_cast<unsigned long>(value)), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_ULongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, static_cast<unsigned long>(value)), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_ULongInt(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, static_cast<unsigned long>(value)), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, float value, int valNr)
	{
		if (      (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_Float(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_Float(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_Float(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_Float(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, double value, int valNr)
	{
		if (     (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_Double(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_Double(msg, tagOrElm, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_Double(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_Double(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value), tagOrElm);
		return false;
	};

	bool setImpl(int msg, const PrivateBlock * pBlock, unsigned long tagOrElm, Item value, int valNr)
	{
		if (      (pBlock == nullptr) && valNr == 1) return mcCheckSet(MC_Set_Value_From_Int(msg, tagOrElm, value.id()), tagOrElm);
		else if( (pBlock == nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_Value_From_Int(msg, tagOrElm, value.id()), tagOrElm);
		else if( (pBlock != nullptr) && valNr == 1) return mcCheckSet(MC_Set_pValue_From_Int(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value.id()), tagOrElm);
		else if( (pBlock != nullptr) && valNr > 1) return mcCheckSet(MC_Set_Next_pValue_From_Int(msg, pBlock->code(), pBlock->group(), tagOrElm & 0xff, value.id()), tagOrElm);
		return false;
	};

}}}