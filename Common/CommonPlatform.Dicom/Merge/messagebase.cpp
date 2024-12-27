// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "messagebase.h"

#include "messagebaseimpl.h"
#include "exception.h"
#include "item.h"

#include <vector>
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
	MessageBase::MessageBase()
		: m_handle()
		, m_id    ()
	{
	}
	
	MessageBase::~MessageBase()
	{
	}

	int MessageBase::id() const
	{
		return m_id;
	}

	std::vector<MessageBase::Attribute> MessageBase::attributes() const
	{
		std::vector<Attribute> aList;
		Attribute a;
		for(auto status = MC_Get_First_Attribute(m_id, &a.tag, &a.vr, &a.numValues)
			; status == MC_NORMAL_COMPLETION
			; status = MC_Get_Next_Attribute(m_id, &a.tag, &a.vr, &a.numValues
			))
		{
			aList.push_back(a);
		}
		return aList;
	}

	PrivateBlock MessageBase::addPrivateBlock(const std::string & privateCode, uint16_t group)
	{
		PrivateBlock block(privateCode, group);
		std::string code(block.code());
		auto status(MC_Add_Private_Block(m_id, code.c_str(), block.group()));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   : return block;
		default                     : throw Exception(status);
		}
	}

	bool MessageBase::addAttribute(const PrivateBlock & block, uint8_t element, MC_VR vr)
	{
		std::string code(block.code());
		auto status(MC_Add_Private_Attribute(m_id, code.c_str(), block.group(), element, vr));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   :
		case MC_TAG_ALREADY_EXISTS  : return true;
		default                     : throw Exception(status);
		}
	}

	bool MessageBase::addAttribute(unsigned long tag)
	{
		auto status(MC_Add_Standard_Attribute(m_id, tag));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   :
		case MC_TAG_ALREADY_EXISTS  : return true;
		default                     : throw Exception(status);
		}
	}

	bool MessageBase::addAttribute(unsigned long tag, MC_VR vr)
	{
		auto status(MC_Add_Nonstandard_Attribute(m_id, tag, vr));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   :
		case MC_TAG_ALREADY_EXISTS  : return true;
		default                     : throw Exception(status);
		}
	}

	bool MessageBase::deleteAttribute(unsigned long tag)
	{
		auto status(MC_Delete_Attribute (m_id, tag));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_INVALID_TAG         : return false;
		default                     : throw Exception(status);
		}
	}

	bool MessageBase::deletePrivateAttribute(const PrivateBlock & block, uint8_t elementByte)
	{
		std::string code(block.code());
		auto status(MC_Delete_Private_Attribute(m_id, code.c_str(), block.group(), elementByte));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_NOT_FOUND           : return false;
		default                     : throw Exception(status);
		}
	}

	bool MessageBase::deletePrivateBlock(const PrivateBlock & block)
	{
		std::string code(block.code());
		auto status(MC_Delete_Private_Block(m_id, code.c_str(), block.group()));
		switch(status)
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_NOT_FOUND           : return false;
		default                     : throw Exception(status);
		}
	}


	bool MessageBase::get(unsigned long tag                          ,  std::string   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          , std::wstring   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,       int8_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,      uint8_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,      int16_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,     uint16_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,      int32_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,     uint32_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,      int64_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,     uint64_t   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,        float   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,       double   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::get(unsigned long tag                          ,         bool   & value) const { return getImpl(m_id, nullptr, tag, value, 1); }

	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,  std::string   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::wstring   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,       int8_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,      uint8_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,      int16_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,     uint16_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,      int32_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,     uint32_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,      int64_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,     uint64_t   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,        float   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,       double   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element,         bool   & value) const { return getImpl(m_id, &pBlock, element, value, 1); }

	bool MessageBase::get(unsigned long tag, std::vector<  std::string > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector< std::wstring > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<       int8_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<      uint8_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<      int16_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<     uint16_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<      int32_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<     uint32_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<      int64_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<     uint64_t > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<        float > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<       double > & value) const { return internalGetVector(nullptr, tag, value); }
	bool MessageBase::get(unsigned long tag, std::vector<         Item > & value) const 
	{   
		value.clear();
		std::vector<int> items(0);
		get(tag, items);
		if (items.size() == 0)
		{
			return false;
		}

		//Remove items with id 0 (invalid)
		auto b = std::remove_if(begin(items), end(items), [](int i){ return i == 0; });
		items.erase(b, end(items));

		value.reserve(items.size());
		for(const auto item : items)
			value.emplace_back(item, false);
		
		return true;
	}


	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<  std::string > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector< std::wstring > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<       int8_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<      uint8_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<      int16_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<     uint16_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<      int32_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<     uint32_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<      int64_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<     uint64_t > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<        float > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<       double > & value) const { return internalGetVector(&pBlock, element, value); }
	bool MessageBase::get(const PrivateBlock & pBlock, uint8_t element, std::vector<         Item > & value) const 
	{   
		value.clear();
		std::vector<int> items(0);

		get(pBlock, element, items);
		if (items.size() == 0)
		{
			return false;
		}

		value.reserve(items.size());
		for(const auto item : items)
			value.emplace_back(item, false);

		return true;
	}

	template <typename T>
	bool MessageBase::internalGetVector(const PrivateBlock * pBlock, unsigned long tagOrElm, std::vector<T> & value) const
	{
		MC_VR           vr(UNKNOWN_VR);
		int             numValues(0);
		int             valueCount(0);
		auto aiStatus(MC_NORMAL_COMPLETION);
		auto vcStatus(MC_NORMAL_COMPLETION);
		//numValues: number of values stored in the attribute; 1 if the attribute value is NULL
		//valueCount: number of values stored in the attribute
		//valueCount: if vr == OB/OW: valueCount will be not more than 1
		//vcStatus == MC_NULL_VALUE if the value is NULL
		//vcStatus == MC_EMPTY_VALUE the value is not assigned
		if(pBlock != nullptr)
		{
			aiStatus = MC_Get_pAttribute_Info(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &vr, &numValues);
			vcStatus = MC_Get_pValue_Count   (m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &valueCount);
		} else {
			aiStatus = MC_Get_Attribute_Info(m_id, tagOrElm, &vr, &numValues);
			vcStatus = MC_Get_Value_Count   (m_id, tagOrElm, &valueCount);
		}

		if (aiStatus == MC_INVALID_MESSAGE_ID)
		{
			return false;
		}

		if(vcStatus == MC_EMPTY_VALUE)
		{
			value.clear();
			return false;
		}

		if(vcStatus == MC_NULL_VALUE)
		{
			value.clear();
			return true;
		}

		if(vr == OB || vr == OW || vr == OF || aiStatus == MC_INVALID_TAG) //MC_INVALID_TAG: perhaps we skipped reading this value, and it's provided by an overloaded method
		{
			if(aiStatus != MC_INVALID_TAG && valueCount != 1) throw Exception(MC_SYSTEM_ERROR, "Value count must be 1 for OB/OW/OF VRs");

			const bool isPod(std::is_pod<T>::value);
			if(isPod)
			{
				//Blindly read the buffer;
				unsigned long dataSize(0);
				if(!getBlobSize(pBlock, tagOrElm, dataSize)) return false;
				value.resize(dataSize / sizeof(T));
				return getBlob(pBlock, tagOrElm, reinterpret_cast<uint8_t*>(value.data()), uint32_t(value.size()*sizeof(T)));
			} else {
				value.resize(1);
				return getImpl(m_id, pBlock, tagOrElm, value[0], 1);
			}
		} else {
			if(vr == SQ)
			{
				unsigned long valueLength(0); //number if items in the sequence
				auto vlStatus = pBlock
					? MC_Get_pValue_Length(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, 1, &valueLength)
					: MC_Get_Value_Length(m_id, tagOrElm, 1, &valueLength);
				if (vlStatus == MC_INVALID_VALUE_NUMBER)
				{
					return false;
				}
				value.resize(valueLength);
			} else {
				value.resize(valueCount);
			}

			bool success(true);
			for (int i = 0; i < static_cast<int>(value.size()); ++i)
			{
				success = getImpl(m_id, pBlock, tagOrElm, value[i], i + 1) && success;
			}
			
			return success;
		}
	}


	bool MessageBase::set(unsigned long tag, const std::wstring & value) const{ return setImpl(m_id, nullptr, tag, value, 1); } 
	bool MessageBase::set(unsigned long tag, const std::string  & value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,               int8_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,              uint8_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,              int16_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,             uint16_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,              int32_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,             uint32_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,              int64_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,             uint64_t value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,                float value) const{ return setImpl(m_id, nullptr, tag, value, 1); }
	bool MessageBase::set(unsigned long tag,               double value) const{ return setImpl(m_id, nullptr, tag, value, 1); }

	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::wstring & value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::string  & value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,               int8_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,              uint8_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,              int16_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,             uint16_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,              int32_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,             uint32_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,              int64_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,             uint64_t value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,                float value) const{ return setImpl(m_id, &block, element, value, 1); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element,               double value) const{ return setImpl(m_id, &block, element, value, 1); }

	bool MessageBase::set(unsigned long tag, const std::vector<  std::string > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector< std::wstring > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<       int8_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<      uint8_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<      int16_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<     uint16_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<      int32_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<     uint32_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<      int64_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<     uint64_t > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<        float > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<       double > & value) { return internalSetVector(nullptr, tag, value); }
	bool MessageBase::set(unsigned long tag, const std::vector<         Item > & value) { return internalSetVector(nullptr, tag, value); }

	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<  std::string > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector< std::wstring > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<       int8_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<      uint8_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<      int16_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<     uint16_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<      int32_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<     uint32_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<      int64_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<     uint64_t > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<        float > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<       double > & value) { return internalSetVector(&block, element, value); }
	bool MessageBase::set(const PrivateBlock & block, uint8_t element, const std::vector<         Item > & value) { return internalSetVector(&block, element, value); }

	bool MessageBase::setVR(const PrivateBlock & block, uint8_t element, MC_VR vr) const
	{
		auto result = MC_Set_pValue_Representation(m_id, block.code(), block.group(), element, vr);
		return (result == MC_NORMAL_COMPLETION || result == MC_VR_ALREADY_VALID);
	}

	bool MessageBase::setEmpty(unsigned long tag)
	{
		switch(MC_Set_Value_To_Empty(m_id, tag))
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_INVALID_TAG         : return false;
		default                     : throw Exception(MC_INVALID_MESSAGE_ID, std::string("Cannot set tag to empty ") + to_hstring(tag));
		}
	}

	bool MessageBase::setEmpty(const PrivateBlock & block, uint8_t element)
	{
		switch(MC_Set_pValue_To_Empty(m_id, block.code(), block.group(), element))
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_INVALID_TAG         : return false;
		default                     : throw Exception(MC_INVALID_MESSAGE_ID, std::string("Cannot set private tag to empty in block: ") + block.code() + " group: " + std::to_string(block.group()));
		}
	}

	bool MessageBase::setNull(unsigned long tag)
	{
		switch(MC_Set_Value_To_NULL(m_id, tag))
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_INVALID_TAG         : return false;
		default                     : throw Exception(MC_INVALID_MESSAGE_ID, std::string("Cannot set private tag to null"));
		}
	}

	bool MessageBase::setNull(const PrivateBlock & block, uint8_t element)
	{
		switch(MC_Set_pValue_To_NULL(m_id, block.code(), block.group(), element))
		{
		case MC_NORMAL_COMPLETION   : return true;
		case MC_INVALID_TAG         : return false;
		default                     : throw Exception(MC_INVALID_MESSAGE_ID, std::string("Cannot set private tag to null in block: ") + block.code() + " group: " + std::to_string(block.group()));
		}

	}

	template <typename T>
	bool MessageBase::internalSetVector(const PrivateBlock * pBlock, unsigned long tagOrElm, const std::vector<T> & value)
	{
		if (value.size() == 0)
		{
			if (pBlock != nullptr)
			{
				return MC_Set_pValue_To_Empty(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff) == MC_NORMAL_COMPLETION;
			}
			else
			{
				return MC_Set_Value_To_Empty(m_id, tagOrElm) == MC_NORMAL_COMPLETION;
			}
		}

		MC_VR       vr(UNKNOWN_VR);
		int         numValues(0);
		MC_STATUS   aiStatus(MC_NORMAL_COMPLETION);
		if (pBlock != nullptr)
		{
			aiStatus = MC_Get_pAttribute_Info(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, &vr, &numValues);
		}
		else
		{
			aiStatus = MC_Get_Attribute_Info(m_id, tagOrElm, &vr, &numValues);
		}

		if (aiStatus == MC_INVALID_MESSAGE_ID || aiStatus == MC_INVALID_TAG)
		{
			return false;
		}

		if(vr == OB || vr == OW || vr == OF)
		{
			const bool isPod(std::is_pod<T>::value);
			if(isPod)
			{
				//Blindly write the buffer;
				unsigned long valueLength(long(sizeof(T) * value.size()));
				if(vr == OW && (valueLength % 2) != 0) throw Exception(MC_INVALID_VALUE_FOR_VR, "Cannot store uneven bytes into a 16 bit word!");
				if(vr == OF && (valueLength % 4) != 0) throw Exception(MC_INVALID_VALUE_FOR_VR, "Cannot store uneven bytes into a 32 bit word!");
				void * data(const_cast<void *>(static_cast<const void *>(&value[0])));
				auto setBufStatus(MC_NORMAL_COMPLETION);
				if (pBlock != nullptr)
				{
					setBufStatus = MC_Set_pValue_From_Buffer(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, data, valueLength);  //Fixme: check return type
				}
				else 
				{
					setBufStatus = MC_Set_Value_From_Buffer(m_id, tagOrElm, data, valueLength); //Fixme: check return type
				}
				return setBufStatus == MC_NORMAL_COMPLETION;
			} else {
				if(value.size() == 1) return false;

				return setImpl(m_id, pBlock, tagOrElm, value[0], 1);
			}
		} else {
			if(vr == SQ && typeid(T) != typeid(Item)) throw Exception(MC_SYSTEM_ERROR, "Only vector<Item> can be used to fill sequences");

			bool success(true);
			for (int i = 0; i < static_cast<int>(value.size()); ++i)
			{
				success = setImpl(m_id, pBlock, tagOrElm, value[i], i + 1) && success;
			}

			return success;
		}
	}    
  
	void MessageBase::setTransferSyntax(TRANSFER_SYNTAX ts)
	{
		mcCheck(MC_Set_Message_Transfer_Syntax(m_id, ts), "Error setting transfer syntax");
		char tsString[32] = {};
		MC_Get_Transfer_Syntax_From_Enum(ts, tsString, _countof(tsString));
	}

	TRANSFER_SYNTAX MessageBase::getTransferSyntax() const
	{
		TRANSFER_SYNTAX ts;
		mcCheck(MC_Get_Message_Transfer_Syntax(m_id, &ts));
		return ts;
	}
	
	bool MessageBase::hasUncompressedPixelData() const
	{
		switch (getTransferSyntax())
		{
		case EXPLICIT_BIG_ENDIAN:
		case EXPLICIT_LITTLE_ENDIAN:
		case IMPLICIT_BIG_ENDIAN:
		case IMPLICIT_LITTLE_ENDIAN:
			return true;
		default:
			return false;
		}
	}

	bool MessageBase::hasAttribute(unsigned long tag) const
	{
		auto status = MC_Get_Attribute_Info(m_id, tag, nullptr, nullptr);
		return status == MC_NORMAL_COMPLETION;
	}

	std::vector<std::vector<uint8_t>> MessageBase::getPixelData(uint32_t frameIndex, uint32_t bytesPerImage) const
	{        
		unsigned long size;
		if (getBlobSize(nullptr, MC_ATT_PIXEL_DATA, size) && size != 0xFFFFFFFF)
		{
			std::vector<std::vector<uint8_t>> result;
			// Non-encapsulated data            
			std::vector<uint8_t> buffer(size); // Copies the bulk of all frames. Suboptimal, but ok for our design. All files are streamed to File which has an efficient overload. This code will only execute for thumbnails which are small
			if (getBlob(nullptr, MC_ATT_PIXEL_DATA, buffer.data(), static_cast<unsigned long>(buffer.size())))
			{
				auto offset = frameIndex * bytesPerImage;
				if (size >= offset + bytesPerImage)
				{
					result.emplace_back(std::move(buffer));
				}
			}
			return result;
		}
		else
		{
			struct DataReader
			{
				MC_STATUS add(int /*msgid*/, unsigned long /*tag*/, int datasize, void* buffer, int /*first*/, int /*last*/)
				{
					data.emplace_back(datasize);
					auto& latestData = data[data.size() - 1];
					memcpy_s(latestData.data(), latestData.size(), buffer, datasize);
					return MC_NORMAL_COMPLETION;
				}
				std::vector<std::vector<uint8_t>> data;
			};
			// Encapsulated data
			uint32_t currentIndex = 0;
			DataReader reader;
			auto status = MC_Get_Encapsulated_Value_To_Function(m_id, MC_ATT_PIXEL_DATA, &reader, [](int msgid, unsigned long tag, void* info, int datasize, void* buffer, int first, int last)
			{
				DataReader& reader = *reinterpret_cast<DataReader*>(info);
				auto result = reader.add(msgid, tag, datasize, buffer, first, last);
				return result;
			});

			while (status == MC_END_OF_FRAME)
			{
				if (currentIndex == frameIndex)
				{
					return std::move(reader.data);
				}
				else
				{
					reader.data.clear();
					status = MC_Get_Next_Encapsulated_Value_To_Function(m_id, MC_ATT_PIXEL_DATA, &reader, [](int msgid, unsigned long tag, void* info, int datasize, void* buffer, int first, int last)
					{
						DataReader& reader = *reinterpret_cast<DataReader*>(info);
						auto result = reader.add(msgid, tag, datasize, buffer, first, last);
						return result;
					});
					currentIndex++;
				}
			}
			if (status == MC_NORMAL_COMPLETION)
			{
				if (currentIndex == frameIndex)
				{
					return std::move(reader.data);
				}
			}
			return std::vector<std::vector<uint8_t>>();
		}
	}

	bool MessageBase::getBlobSize(const PrivateBlock * pBlock, unsigned long tagOrElm, unsigned long& dataSize) const
	{
		auto status = pBlock
			? MC_Get_pValue_Length(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, 1, &dataSize)
			: MC_Get_Value_Length(m_id, tagOrElm, 1, &dataSize);
		
		return (status == MC_NORMAL_COMPLETION);
	}

	bool MessageBase::getBlob(const PrivateBlock * pBlock, unsigned long tagOrElm, void* buffer, unsigned long bufferSize) const
	{
		int valSize;
		auto status = pBlock
			? MC_Get_pValue_To_Buffer(m_id, pBlock->code(), pBlock->group(), tagOrElm & 0xff, bufferSize, buffer, &valSize)
			: MC_Get_Value_To_Buffer(m_id, tagOrElm, bufferSize, buffer, &valSize);
		switch(status)
		{
		case MC_NORMAL_COMPLETION:
		case MC_EMPTY_VALUE:
		case MC_NULL_VALUE:
			return bufferSize == static_cast<uint32_t>(valSize);
		default:
			{
				std::string e("Error while calling MC_Get_Value_To_Buffer for ");
				e += pBlock ? to_hstring(*pBlock, tagOrElm & 0xff) : to_hstring(tagOrElm);
				throw Exception(status, e);
			}
		}
	}

	void print(std::ostream & os, const MessageBase & msg, int indent)
	{
		os << "\n" << std::setw(indent * 2) << std::setfill(' ') << "{\n";
		int keywordBufferSize(4096);
		std::vector<char> keywordBuffer(keywordBufferSize);
		for(const auto & a : msg.attributes())
		{
			std::string valueString;
			try
			{
				keywordBuffer[0] = 0; //Reset buffer contents
				MC_Get_Tag_Keyword(a.tag, &keywordBuffer[0], keywordBufferSize);
				switch(a.vr)
				{
				case MC_VR::OB:
				case MC_VR::OW:
				case MC_VR::OF:
					//case MC_VR::UNKNOWN_VR:
					{
						std::vector<uint8_t> bulk;
						msg.get(a.tag, bulk);
						std::stringstream ss;
						for (size_t i = 0; i < 30 && i < bulk.size(); i++)
						{
							ss << std::setfill('0') << std::setw(2) << std::hex << int(bulk[i]);
						}
						valueString = ss.str();
						if(bulk.size() > 30) valueString += "... (" + std::to_string(bulk.size()) + ")";
						break;
					}
				case MC_VR::SQ:
					{
						std::vector<Item> v;
						msg.get(a.tag, v);
						for(const auto & m : v) print (os, m, indent+1);
						break;
					}
				default:
					{
						std::vector<std::string> values;
						auto getResult = msg.get(a.tag, values);
						auto numVals = values.size();

						if(!getResult)
						{
							valueString = "[error reading value]";
						} else if(numVals == 0) {
							valueString = "(empty)";
						} else if(numVals == 1) {
							valueString = values[0];
						} else {
							std::stringstream ss;
							ss << "[";
							for(size_t i = 0; i < numVals; i++)
							{
								if(i > 0) ss << ", ";
								ss << values[i];
							}
							ss << "] (" << numVals << ")";
							valueString = ss.str();
						}
					}
					break;
				}
			}
			catch(Merge::Exception & ex)
			{
				valueString = std::string("[error: ") + ex.what() + "]";
			}
			os << std::setw(indent * 2 + 13) << std::setfill(' ') << to_hstring(a.tag) << ": " << std::setfill(' ') << std::setw(50) << &keywordBuffer[0] << " -> " << valueString << "\n";
		}
		os << std::setw(indent * 2) << std::setfill(' ') << "}\n";
	}

	std::ostream & operator<<(std::ostream & os, const MessageBase & msg)
	{
		os << "Message " << msg.id();
		print(os, msg, 0);
		return os;
	}


	std::string to_hstring(unsigned long tag)
	{
		std::stringstream stream;
		stream << "(" 
			<< std::setfill ('0') << std::setw(8)
			<< std::hex << tag
			<< ")";
		auto res = stream.str();
		res.insert(5, ",");
		return res;
	}

	std::string to_hstring(const PrivateBlock & block, uint8_t element)
	{
		return to_hstring((block.group() << 16) + element) + " in block " + block.code();
	}

}}}