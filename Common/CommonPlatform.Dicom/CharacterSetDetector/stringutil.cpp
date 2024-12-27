// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "stringutil.h"
#include "charsetconstants.h"
#include <Shlwapi.h>
#include <algorithm>
#include <memory>
#include <set>

namespace CommonPlatform {	namespace Dicom {

	bool StringUtil::isValidHighSurrogate(unsigned int high)
	{
		return (high & SurrogateMask) == HighSurrogateStartVal;
	}

	bool StringUtil::isValidLowSurrogate(unsigned int low)
	{
		return (low & SurrogateMask) == LowSurrogateStartVal;
	}

	unsigned int StringUtil::getValueFromSurrogatePairs(const std::wstring& c)
	{
		unsigned int decVal(0);
		unsigned int high = 0, low = 0;
		for (auto eachChar : c)
		{
			if (StringUtil::isValidHighSurrogate(eachChar))
			{
				high = eachChar;
				continue;
			}
			else if (StringUtil::isValidLowSurrogate(eachChar))
			{
				low = eachChar;
				decVal = ((high - HighSurrogateStartVal) * TenBits) + ((low - LowSurrogateStartVal) + LowestSurrogateChar);
			}
		}
		return decVal;
	}

	unsigned int StringUtil::getDecimalValue(const std::wstring& character)
	{
		unsigned int decimalValue = 0;
		int length = static_cast<int>(character.length());
		if (length == 1)
		{
			decimalValue = character[0];
		}
		else
		{
			decimalValue = getValueFromSurrogatePairs(character);
		}
		return decimalValue;
	}

	std::wstring StringUtil::stringToWString(const std::string& multiByteStr_i, const int codePageIdentifier)
	{
		std::wstring unicodeStr = L"";
		int currentSize = static_cast<int>(multiByteStr_i.size());
		if (currentSize != 0)
		{
			int wideStringSize = MultiByteToWideChar(codePageIdentifier, 0, multiByteStr_i.c_str(), -1, NULL, 0);
			if (0 < wideStringSize)
			{
				std::vector<wchar_t> buffer(wideStringSize + 1);
				wideStringSize = MultiByteToWideChar(codePageIdentifier, 0, multiByteStr_i.c_str(), -1, buffer.data(), wideStringSize);
				if (0 < wideStringSize)
				{
					buffer[wideStringSize] = '\0';
					unicodeStr = buffer.data();
				}
			}
		}
		return unicodeStr;
	}

	std::string StringUtil::wstringToString(const std::wstring& wideStr, const int codePageIdentifier)
	{
		std::string multiByteStr;

		int currentSize = static_cast<int>(wideStr.size());
		if (currentSize != 0)
		{
			int resultStringSize = WideCharToMultiByte(codePageIdentifier, 0, wideStr.c_str(), currentSize, NULL, 0, NULL, NULL);
			if (resultStringSize > 0)
			{
				std::vector<char> buffer(resultStringSize + 1);
				resultStringSize = WideCharToMultiByte(codePageIdentifier, 0, wideStr.c_str(), currentSize, buffer.data(), resultStringSize, NULL, NULL);
				if (resultStringSize > 0)
				{
					buffer[resultStringSize] = '\0';
					multiByteStr = buffer.data();
				}
			}
		}
		return multiByteStr;
	}	

	bool StringUtil::contains(const std::vector<std::string>& vec, const std::string& str)
	{
		bool retVal = false;
		if (std::find(vec.begin(), vec.end(), str) != vec.end())
		{
			retVal = true;
		}
		return retVal;
	}

	std::vector<std::string> StringUtil::removeDuplicatesWithOrder(const std::vector<std::string>& charsets)
	{
		std::set<std::string> uniqueSet;
		auto characterSetsWithoutDuplicates = charsets;

		auto duplicateStart = std::remove_if(characterSetsWithoutDuplicates.begin(), characterSetsWithoutDuplicates.end(), [&uniqueSet](const std::string& value)
		{
			if (uniqueSet.find(value) != std::end(uniqueSet))
				return true;

			uniqueSet.insert(value);
			return false;
		});

		characterSetsWithoutDuplicates.erase(duplicateStart, characterSetsWithoutDuplicates.end());

		return characterSetsWithoutDuplicates;
	}

}}