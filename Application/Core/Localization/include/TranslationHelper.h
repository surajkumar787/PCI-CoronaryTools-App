// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "Translation.h"

namespace Pci {	namespace Core { namespace Localization {

Translation::Language getLanguageId(std::wstring language, bool languagePackEnabled)
{
	Translation::Language result = Translation::Language::English;
	std::map<std::wstring,Translation::Language> map;
	map[L"da"] = Translation::Language::Danish;
	map[L"nl"] = Translation::Language::Dutch;
	map[L"en"] = Translation::Language::English;
	map[L"fr"] = Translation::Language::French;
	map[L"de"] = Translation::Language::German;
	map[L"it"] = Translation::Language::Italian;
	map[L"no"] = Translation::Language::Norwegian;
	map[L"es"] = Translation::Language::Spanish;
	map[L"sv"] = Translation::Language::Swedish;
	map[L"ja"] = languagePackEnabled ? Translation::Language::Japanese: Translation::Language::English;
	map[L"zh-Hans"] = languagePackEnabled ? Translation::Language::ChineseSimplified: Translation::Language::English;
	map[L"zh-Hant"] = languagePackEnabled ? Translation::Language::ChineseTraditional: Translation::Language::English;

	auto item = map.find(language);
	if (item != map.end())
	{
		result = item->second;
	}

	return result;
}

} } }
