// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "CrmRecording.h"
#include "CrmOverlay.h"

using namespace Pci::Core;

class TestCrmRecording : public ::testing::Test
{
public:

	void SetUp() override
	{
	}

	const int MAX_NR_OF_FRAMES = 300;
	const int IMAGE_SIZE = 1024;

};

TEST_F(TestCrmRecording, When_Contructed_Then_Recording_Is_Empty)
{
	CrmRecording recording(MAX_NR_OF_FRAMES);

	EXPECT_EQ(0, recording.getImageCount());
	EXPECT_TRUE(recording.getAllImages().empty());
}

TEST_F(TestCrmRecording, When_Contructed_Recording_Then_RunIndex_Is_Invalid)
{
	const int INVALID_RUN_ID(-1);
	CrmRecording recording(MAX_NR_OF_FRAMES);

	EXPECT_EQ(INVALID_RUN_ID, recording.getRunIndex());
}

TEST_F(TestCrmRecording, When_setRunIndex_Called_Then_RunIndex_Is_Stored)
{
	const int NEW_RUNINDEX(5);
	CrmRecording recording(MAX_NR_OF_FRAMES);
	recording.setRunIndex(NEW_RUNINDEX);

	EXPECT_EQ(NEW_RUNINDEX, recording.getRunIndex());
}

TEST_F(TestCrmRecording, When_Image_Added_Then_Image_Is_Part_Of_AllImages)
{
	CrmRecording recording(MAX_NR_OF_FRAMES);

	auto overlay = std::make_shared<CrmOverlay>();
	auto xrayImage = std::make_shared<CommonPlatform::Xray::XrayImage>();

	recording.addImage(xrayImage, overlay);

	auto allImages = recording.getAllImages();

	EXPECT_EQ(1, recording.getImageCount());
	ASSERT_FALSE(allImages.empty());
	EXPECT_EQ(xrayImage, allImages.front().first);
	EXPECT_EQ(overlay, allImages.front().second);
}

TEST_F(TestCrmRecording, When_Image_Added_After_Max_Frames_Reached_Then_First_Frame_Is_Removed)
{
	const int MAX_FRAMES(1);

	CrmRecording recording(MAX_FRAMES);

	auto overlay1 = std::make_shared<CrmOverlay>();
	auto xrayImage1 = std::make_shared<CommonPlatform::Xray::XrayImage>();
	recording.addImage(xrayImage1, overlay1);

	auto overlay2 = std::make_shared<CrmOverlay>();
	auto xrayImage2 = std::make_shared<CommonPlatform::Xray::XrayImage>();
	recording.addImage(xrayImage2, overlay2);

	auto allImages = recording.getAllImages();

	EXPECT_EQ(MAX_FRAMES, recording.getImageCount());
	ASSERT_FALSE(allImages.empty());
	EXPECT_EQ(xrayImage2, allImages.front().first);
	EXPECT_EQ(overlay2, allImages.front().second);
}

TEST_F(TestCrmRecording, When_Recording_Cleared_Then_All_Frames_Are_Removed)
{
	const int NUMBER_OF_FRAMES(10);

	CrmRecording recording(MAX_NR_OF_FRAMES);

	for (int i = 0; i < NUMBER_OF_FRAMES; i++)
	{
		auto overlay = std::make_shared<CrmOverlay>();
		auto xrayImage = std::make_shared<CommonPlatform::Xray::XrayImage>();
		recording.addImage(xrayImage, overlay);
	}

	auto allImages = recording.getAllImages();

	EXPECT_EQ(NUMBER_OF_FRAMES, recording.getImageCount());
	EXPECT_FALSE(allImages.empty());

	recording.clear();
	allImages = recording.getAllImages();

	EXPECT_EQ(0, recording.getImageCount());
	EXPECT_TRUE(allImages.empty());
}
