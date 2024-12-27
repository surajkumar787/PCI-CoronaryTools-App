// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <gtest/gtest.h>

#include "Snapshot.h"


using namespace Pci::Core::PACS;

class TestSnapshot : public ::testing::Test
{
public:

	TestSnapshot()
	{
	}

	~TestSnapshot()
	{
	}

};


TEST_F(TestSnapshot, WhenCreateFromRawDataAndZeroHeight_ThenNoSnapshotCreated)
{
	const int HEIGHT(0);
	const int WIDTH(100);
	const Format IMAGE_FORMAT(Format::Rgb32);

	const std::vector<char> IMAGE_DATA(WIDTH * FormatDesc(IMAGE_FORMAT).pixelSize);
	
	EXPECT_EQ(nullptr, Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, IMAGE_DATA.data(), IMAGE_DATA.size()));
}

TEST_F(TestSnapshot, WhenCreateFromRawDataAndZeroWidth_ThenNoSnapshotCreated)
{
	const int HEIGHT(100);
	const int WIDTH(0);
	const Format IMAGE_FORMAT(Format::Rgb32);

	const std::vector<char> IMAGE_DATA(HEIGHT * FormatDesc(IMAGE_FORMAT).pixelSize);

	EXPECT_EQ(nullptr, Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, IMAGE_DATA.data(), IMAGE_DATA.size()));
}

TEST_F(TestSnapshot, WhenCreateFromRawDataAndZeroHeightAndWidth_ThenNoSnapshotCreated)
{
	const int HEIGHT(0);
	const int WIDTH(0);
	const Format IMAGE_FORMAT(Format::Rgb32);

	const std::vector<char> IMAGE_DATA(HEIGHT * FormatDesc(IMAGE_FORMAT).pixelSize);

	EXPECT_EQ(nullptr, Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, IMAGE_DATA.data(), IMAGE_DATA.size()));
}

TEST_F(TestSnapshot, WhenCreateFromRawDataAndImageDataTooBig_ThenNoSnapshotCreated)
{
	const int HEIGHT(100);
	const int WIDTH(100);
	const Format IMAGE_FORMAT(Format::Rgb32);

	const std::vector<char> IMAGE_DATA((HEIGHT * WIDTH * FormatDesc(IMAGE_FORMAT).pixelSize) + 1);

	EXPECT_EQ(nullptr, Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, IMAGE_DATA.data(), IMAGE_DATA.size()));
}

TEST_F(TestSnapshot, WhenCreateFromRawDataAndImageDataTooSmall_ThenNoSnapshotCreated)
{
	const int HEIGHT(100);
	const int WIDTH(100);
	const Format IMAGE_FORMAT(Format::Rgb32);

	const std::vector<char> IMAGE_DATA((HEIGHT * WIDTH * FormatDesc(IMAGE_FORMAT).pixelSize) - 1);

	EXPECT_EQ(nullptr, Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, IMAGE_DATA.data(), IMAGE_DATA.size()));
}

TEST_F(TestSnapshot, WhenCreateFromRawDataAndImageDataNullptr_ThenNoSnapshotCreated)
{
	const int HEIGHT(100);
	const int WIDTH(100);
	const Format IMAGE_FORMAT(Format::Rgb32);
	const int IMAGE_SIZE(HEIGHT * HEIGHT * FormatDesc(IMAGE_FORMAT).pixelSize);

	EXPECT_EQ(nullptr, Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, nullptr, IMAGE_SIZE));
}

TEST_F(TestSnapshot, WhenCreateFromRawDataAndAllDataValid_ThenSnapshotIsCreated)
{
	const int HEIGHT(100);
	const int WIDTH(200);
	const Format IMAGE_FORMAT(Format::Rgb32);

	const std::vector<char> IMAGE_DATA((HEIGHT * WIDTH * FormatDesc(IMAGE_FORMAT).pixelSize));

	auto mySnapshot = Snapshot::fromRawData(WIDTH, HEIGHT, IMAGE_FORMAT, IMAGE_DATA.data(), IMAGE_DATA.size());
	ASSERT_NE(nullptr, mySnapshot);
	EXPECT_EQ(HEIGHT, mySnapshot->getHeight());
	EXPECT_EQ(WIDTH, mySnapshot->getWidth());
	EXPECT_EQ(IMAGE_FORMAT, mySnapshot->getFormat());
	EXPECT_EQ(IMAGE_DATA, mySnapshot->getPixelData());
}
