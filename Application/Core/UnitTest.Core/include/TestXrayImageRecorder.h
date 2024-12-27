// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "XrayImageRecorder.h"
#include "XrayImageRecording.h"
#include "XrayImage.h"

using namespace Pci::Core;

class TestXRayImageRecorder : public ::testing::Test
{
public:

	void SetUp() override
	{
	}

	CommonPlatform::Xray::XrayImage createImage(int runIndex)
	{
		CommonPlatform::Xray::XrayImage image;
		image.runIndex = runIndex;
		return image;
	}

};

TEST_F(TestXRayImageRecorder, When_Contructed_Recording_Is_Null)
{
	XRayImageRecorder recorder;
	EXPECT_EQ(nullptr, recorder.getLastRecording());
}

TEST_F(TestXRayImageRecorder, When_Adding_Image_Recording_Is_Created)
{
	XRayImageRecorder recorder;
	auto image = createImage(0);

	recorder.addImage(image);
	auto recording = recorder.getLastRecording();

	ASSERT_NE(nullptr, recording);

	auto images = recording->getAllImages();
	image.type = CommonPlatform::Xray::XrayImage::Type::ExposureReplay;

	EXPECT_EQ(1, images.size());
	EXPECT_EQ(image, **images.begin());
}

TEST_F(TestXRayImageRecorder, When_Image_With_Other_RunIndex_Added_New_Recording_Is_Created)
{
	const int FIRST_RUN_INDEX(0);
	const int SECOND_RUN_INDEX(1);

	XRayImageRecorder recorder;

	recorder.addImage(createImage(FIRST_RUN_INDEX));
	auto firstRecording = recorder.getLastRecording();

	recorder.addImage(createImage(SECOND_RUN_INDEX));

	EXPECT_NE(recorder.getLastRecording(), firstRecording);
	EXPECT_NE(nullptr, recorder.getLastRecording());
}
