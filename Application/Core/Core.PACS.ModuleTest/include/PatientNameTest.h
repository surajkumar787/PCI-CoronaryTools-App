// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "MessageBuilderTests.h"

class PatientNameTest : public MessageBuilderTest,
						public ::testing::WithParamInterface<std::wstring>
{
};

TEST_P(PatientNameTest, CheckPatientNameIsSupported)
{
	const int imgSize = 5;

	Pci::Core::PACS::MetaData data(GenerateStudy(), GenerateGeometry(), seriesNumber, instanceNumber);
	data.study.patientName = GetParam();

	auto msg = messageBuilder->CreateSnapshotMessage(data, *GenerateImage(imgSize, imgSize),RunType::UNKNOWN);

	EXPECT_TRUE(msg);

	ValidateMessage(*msg);
}

std::wstring names[] =
{
	// japanese
	L"询꾆 끿",
	L"文雄",
	L"弘 樹",

	// chinese
	L"爱 蔼",
	L"金锦 津",
	L"煌 凰",
	L"静 精 晶 京"

	// korean
	L"민준",
	L"준서",
	L"하은"
};

INSTANTIATE_TEST_SUITE_P(Names, PatientNameTest, ::testing::ValuesIn(names));