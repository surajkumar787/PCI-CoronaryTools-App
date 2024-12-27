// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>
#include <gtest/gtest-param-test.h>

#include "Sense/Common/Size.h"
#include "ViewTypeDetector.h"
#include "ViewType.h"

class TestViewTypeDetector : public ::testing::TestWithParam<std::tuple<std::wstring, Sense::Size, Pci::Core::ViewType>>
{
};

Sense::Size IncorrectSizes[] =
{
	Sense::Size(1, 1),
	Sense::Size(1280, 1025),
	Sense::Size(99999, 9999),
	Sense::Size(1281, 1024),
	Sense::Size(1920, 1079),
	Sense::Size(1919, 1080)
};

Sense::Size AlluraSize(1280, 1024);
Sense::Size SmartSuiteFlexVisionSize(1280, 990);
Sense::Size SmartSuite19InchSize(1280, 1024);
Sense::Size SmartSuiteFullHDSize(1920, 1080);

std::wstring IncorrectSystemTypes[] =
{
	L"Alura",
	L"Blurra",
	L"",
	L"Suite",
	L"iSuite",
	L"SmS"
};


TEST_P(TestViewTypeDetector, WhenASystemTypeAndScreenSizeIsPassed_ThenTheCorrectViewTypeIsReturned)
{
	auto res = Pci::Core::GetViewType(std::get<0>(GetParam()), std::get<1>(GetParam()));

	ASSERT_EQ(std::get<2>(GetParam()), res);
}

const char* viewTypeString(Pci::Core::ViewType viewType)
{
	if (viewType == Pci::Core::ViewType::Allura)               return "Allura";
	if (viewType == Pci::Core::ViewType::SmartSuiteFullHD)     return "SmartSuiteFullHD";
	if (viewType == Pci::Core::ViewType::SmartSuiteFlexVision) return "SmartSuiteFlexVision";
	if (viewType == Pci::Core::ViewType::SmartSuite19Inch)     return "SmartSuite19Inch";
	throw "Unknown ViewType";
}

static std::string printParams(const ::testing::TestParamInfo<std::tuple<std::wstring, Sense::Size, Pci::Core::ViewType>>& param)
{
	char buf[128];
	sprintf_s(buf, "%S_%.0f_%.0f_%s", std::get<0>(param.param).c_str(), std::get<1>(param.param).width, std::get<1>(param.param).height, viewTypeString(std::get<2>(param.param)));
	return buf;
}


INSTANTIATE_TEST_SUITE_P(GivenIncorrectSystemTypesAndUnknownSizes_DefaultAlluraSizeIsReturned, TestViewTypeDetector,
						::testing::Combine(::testing::ValuesIn(IncorrectSystemTypes),
						                   ::testing::ValuesIn(IncorrectSizes),
						                   ::testing::Values(Pci::Core::ViewType::Allura)),
						printParams);

INSTANTIATE_TEST_SUITE_P(GivenIncorrectSystemTypesAndAlluraSize_DefaultAlluraSizeIsReturned, TestViewTypeDetector,
						::testing::Combine(::testing::ValuesIn(IncorrectSystemTypes),
						                   ::testing::Values(AlluraSize),
						                   ::testing::Values(Pci::Core::ViewType::Allura)),
						printParams);

INSTANTIATE_TEST_SUITE_P(GivenCorrectSmartSuiteSystemAndUnknownSize_DefaultFlexvisionIsReturned, TestViewTypeDetector,
						::testing::Combine(::testing::Values(std::wstring(L"Azurion")),
						                   ::testing::ValuesIn(IncorrectSizes),
						                   ::testing::Values(Pci::Core::ViewType::SmartSuiteFlexVision)),
						printParams);

INSTANTIATE_TEST_SUITE_P(GivenCorrectSmartSuiteSystemAndFullHDSize_SmartSuiteFullHDIsReturned, TestViewTypeDetector,
						::testing::Values(std::make_tuple(std::wstring(L"Azurion"), SmartSuiteFullHDSize, Pci::Core::ViewType::SmartSuiteFullHD)), printParams);

INSTANTIATE_TEST_SUITE_P(GivenCorrectSmartSuiteSystemAndFlexvisionSize_SmartSuiteFlexvisionIsReturned, TestViewTypeDetector,
						::testing::Values(std::make_tuple(std::wstring(L"Azurion"), SmartSuiteFlexVisionSize, Pci::Core::ViewType::SmartSuiteFlexVision)), printParams);

INSTANTIATE_TEST_SUITE_P(GivenCorrectSmartSuiteSystemAnd19InchSize_SmartSuite19InchIsReturned, TestViewTypeDetector,
						::testing::Values(std::make_tuple(std::wstring(L"Azurion"), SmartSuite19InchSize, Pci::Core::ViewType::SmartSuite19Inch)), printParams);



INSTANTIATE_TEST_SUITE_P(GivenCorrectAlluraSystemAndUnknownSize_DefaultAlluraSizeIsReturned, TestViewTypeDetector,
						::testing::Combine(::testing::Values(std::wstring(L"Allura")),
						                   ::testing::ValuesIn(IncorrectSizes),
						                   ::testing::Values(Pci::Core::ViewType::Allura)));

INSTANTIATE_TEST_SUITE_P(GivenCorrectAlluraSystemAndCorrectSize_DefaultAlluraSizeIsReturned, TestViewTypeDetector,
						::testing::Values(std::make_tuple(std::wstring(L"Allura"), AlluraSize, Pci::Core::ViewType::Allura)));