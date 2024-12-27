// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include <algorithm>
#include <iterator>

#include "CharSetManagerSystemLanguage.h"
#include "DicomConstants.h"

namespace CommonPlatform {	namespace Dicom 
{
	CharsetManagerSystemLanguage::CharsetManagerSystemLanguage(const std::wstring& systemLanguage)
	{
		std::string displayLanguageUpper("");
		std::transform(systemLanguage.begin(), systemLanguage.end(), std::back_inserter(displayLanguageUpper), [](wchar_t c) {return static_cast<char>(::towupper(c)); });

		std::vector<std::string> charSetList;
		
		// Always first try default and latin character set
		charSetList.push_back(Constants::ISO_IR_6);
		charSetList.push_back(Constants::ISO_IR_100);

		if (displayLanguageUpper.find("JA") != std::string::npos)
		{
			charSetList.push_back(Constants::ISO_IR_13);
			charSetList.push_back(Constants::ISO_IR_87);
			charSetList.push_back(Constants::ISO_IR_159);
		}
		else if ((displayLanguageUpper.find("ZH-HANS") != std::string::npos) || (displayLanguageUpper.find("ZH-HANT") != std::string::npos))
		{
			charSetList.push_back(Constants::GB18030);
		}

		// As last option, use UTF8 encoding
		charSetList.push_back(Constants::ISO_IR_192);

		setPossibleCharacterset(charSetList);
	}
}}
