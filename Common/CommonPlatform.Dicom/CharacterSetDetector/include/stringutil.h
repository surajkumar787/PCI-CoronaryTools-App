// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <vector>
#include <string>


namespace CommonPlatform {	namespace Dicom{

	class StringUtil
	{
	public:
		static bool isValidHighSurrogate(unsigned int high);
		static bool isValidLowSurrogate(unsigned int low);

		static unsigned int getDecimalValue(const std::wstring& character);
		static std::string wstringToString(const std::wstring& wideStr, const int codePageIdentifier);
		static std::wstring stringToWString(const std::string& multiByteStr_i, const int codePageIdentifier);		
		static bool contains(const std::vector<std::string>& vec, const std::string& str);	
		static std::vector<std::string> removeDuplicatesWithOrder(const std::vector<std::string>& charsets);

	private:	
		static unsigned int getValueFromSurrogatePairs(const std::wstring& c);
	};
}}
