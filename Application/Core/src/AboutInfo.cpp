// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "AboutInfo.h"
#include "Build.hpp"
#include "Resources.h"
#include "stlstring.h"
#include "ProductInfoUtil.h"
#include <algorithm>
#include <iterator>
using namespace std;
using namespace CommonPlatform::Utilities;

namespace Pci { namespace Core
{
	static std::wstring getParsedBuilddate()
	{
		std::wstring buildDate;
		std::replace_copy_if(std::begin(BuildDate), std::end(BuildDate), std::back_inserter(buildDate), [] (wchar_t c) { return (c == L'/'); }, L'-');
		return buildDate;
	}

	namespace ProductInfo
	{
		int DCR_MAJOR, DCR_MINOR, DCR_LEVEL = 0;
		int SBL_MAJOR, SBL_MINOR, SBL_LEVEL = 0;
		bool success = getProductVersion(DcrSoftwareVersion, DCR_MAJOR, DCR_MINOR, DCR_LEVEL)
				&& getProductVersion(SblSoftwareVersion, SBL_MAJOR, SBL_MINOR, SBL_LEVEL);
	
		
		const AboutInfo AboutDcrCoreg =
		{
			L"Dynamic Coronary Roadmap",
			getProductVersionLabel(DCR_MAJOR, DCR_MINOR, DCR_LEVEL),

			getParsedBuilddate(),

			L"001016",
			getProductVersionLot(DCR_MAJOR, DCR_MINOR, DCR_LEVEL),
			std::wstring(L"(01)00884838101883(10)") + getProductVersionLot(DCR_MAJOR, DCR_MINOR, DCR_LEVEL),
			L"\nPhilips\n\nDynamic Coronary Roadmap R" + getProductVersionLabel(DCR_MAJOR, DCR_MINOR, DCR_LEVEL) + L"\n\n" +
			str_ext::stow(std::string(reinterpret_cast<const char*>(PciSuite::Resources::eula_dynamic_coronary_roadmap), sizeof(PciSuite::Resources::eula_dynamic_coronary_roadmap)))
		};

		const AboutInfo AboutSbl =
		{
			L"StentBoost Live",
			getProductVersionLabel(SBL_MAJOR, SBL_MINOR, SBL_LEVEL),

			getParsedBuilddate(),

			L"001017",
			getProductVersionLot(SBL_MAJOR, SBL_MINOR, SBL_LEVEL),
			std::wstring(L"(01)00884838101890(10)") + getProductVersionLot(SBL_MAJOR, SBL_MINOR, SBL_LEVEL),
			L"\nPhilips\n\nStentBoost Live R" + getProductVersionLabel(SBL_MAJOR, SBL_MINOR, SBL_LEVEL) + L"\n\n" +
			str_ext::stow(std::string(reinterpret_cast<const char*>(PciSuite::Resources::eula_stentboost_live), sizeof(PciSuite::Resources::eula_stentboost_live)))
		};
	}

}}