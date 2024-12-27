// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ViewTypeDetector.h"
#include "ViewType.h"
#include "SystemType.h"
#include "Sense/Common/Size.h"

namespace Pci { namespace Core
{
	using namespace CommonPlatform::Xray;
	static const std::wstring SystemTypeAllura = L"Allura";
	static const std::wstring SystemTypeSmartSuite = L"Azurion";
	static const std::wstring SystemTypeAzurion1 = L"Azurion_1x";

	SystemType GetSystemType(const std::wstring& systemType)
	{
		// default Allura when nothing matches
		auto sysType = SystemType::Allura;
		if (SystemTypeAllura.compare(systemType) == 0)
		{
			sysType = SystemType::Allura;
		}
		else if (SystemTypeSmartSuite.compare(systemType) == 0)
		{
			sysType = SystemType::SmartSuite;
		}
		else if (SystemTypeAzurion1.compare(systemType) == 0)
		{
			sysType = SystemType::SmartSuite_1x;
		}

		return sysType;
	}

	static const Sense::Size AlluraSize(1280, 1024);
	static const Sense::Size SmSFullHDSize(1920, 1080);
	static const Sense::Size SmSFlexVisionSize(1280, 990);
	static const Sense::Size SmS19InchSize(1280, 1024);

	ViewType GetViewType(const std::wstring& systemType, const Sense::Size& screenSize)
	{
		auto sysType = GetSystemType(systemType);

		if (sysType == SystemType::Allura)
		{
			// screenSize does not matter when system is Allura because it only supports 1 size
			return ViewType::Allura;
		}

		if (screenSize == SmSFullHDSize)     return ViewType::SmartSuiteFullHD;
		if (screenSize == SmSFlexVisionSize) return ViewType::SmartSuiteFlexVision;
		if (screenSize == SmS19InchSize)     return ViewType::SmartSuite19Inch;

		// unknown screen sizes for SmartSuite default to flex vision
		return ViewType::SmartSuiteFlexVision;
	}

}}