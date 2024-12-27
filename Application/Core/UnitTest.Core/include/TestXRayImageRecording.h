// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "XrayImageRecording.h"
#include "XrayImage.h"

using namespace Pci::Core;

class TestXRayImageRecording : public ::testing::Test
{
public:

	void SetUp() override
	{
	}

	CommonPlatform::Xray::XrayImage createImage(int runIndex, int imageNumber)
	{
		CommonPlatform::Xray::XrayImage image;
		image.runIndex = runIndex;
		image.type = CommonPlatform::Xray::XrayImage::Type::ExposureReplay;
		image.imageNumber = imageNumber;
		return image;
	}

	std::vector<CommonPlatform::Xray::XrayImage> createImages(int numberOfImages, int runIndex)
	{
		std::vector<CommonPlatform::Xray::XrayImage> images;
		for (int i = 0; i < numberOfImages; i++)
		{
			images.push_back(createImage(runIndex, i));
		}
		
		return images;
	}

};

TEST_F(TestXRayImageRecording, When_Contructed_Images_Is_Empty)
{
	XRayImageRecording recording(0);
	
	EXPECT_EQ(0, recording.getImageCount());
	EXPECT_EQ(0, recording.getAllImages().size());
}

TEST_F(TestXRayImageRecording, When_Contructed_RunIndex_Is_Saved)
{
	const int RUN_INDEX(5);

	XRayImageRecording recording(RUN_INDEX);
	EXPECT_EQ(RUN_INDEX, recording.getRunIndex());
}

TEST_F(TestXRayImageRecording, On_AddImage_Image_Is_Added)
{
	const int RUN_INDEX(0);
	const int IMAGE_NUMBER(0);
	auto image = createImage(RUN_INDEX, IMAGE_NUMBER);
	
	XRayImageRecording recording(RUN_INDEX);
	recording.addImage(image);

	EXPECT_EQ(RUN_INDEX, recording.getRunIndex());
	EXPECT_EQ(1, recording.getImageCount());
	ASSERT_EQ(1, recording.getAllImages().size());
	EXPECT_EQ(image, **recording.getAllImages().begin());
}

TEST_F(TestXRayImageRecording, On_Multiple_AddImage_All_Images_Are_Added)
{
	const int RUN_INDEX(0);
	const int NR_OF_IMAGES(10);
	auto images = createImages(NR_OF_IMAGES, RUN_INDEX);

	XRayImageRecording recording(RUN_INDEX);
	
	for (auto image : images)
	{
		recording.addImage(image);
	}

	EXPECT_EQ(NR_OF_IMAGES, recording.getImageCount());
	ASSERT_EQ(NR_OF_IMAGES, recording.getAllImages().size());

	auto recordedImages = recording.getAllImages();
	for (int i = 0; i < NR_OF_IMAGES; i++)
	{
		ASSERT_NE(nullptr, recordedImages[i]);
		EXPECT_EQ(images[i], *recordedImages[i]);
	}
}


