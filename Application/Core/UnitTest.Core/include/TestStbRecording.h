// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "StbRecording.h"
#include "StbImage.h"
#include "StbMarkers.h"

using namespace Pci::Core;

class TestStbRecording : public ::testing::Test
{
public:

	void SetUp() override
	{
	}

	StbResult createStbResult(int runIndex, int imageNumber, int width, int height, bool hasContrast = false)
	{
		auto image = std::make_shared<CommonPlatform::Xray::XrayImage>();
		image->runIndex = runIndex;
		image->type = CommonPlatform::Xray::XrayImage::Type::Exposure;
		image->imageNumber = imageNumber;
		image->width = width;
		image->height = height;
		image->validRect = Sense::Rect(Sense::Point(0, 0), Sense::Point(width, height));

		auto boostImage = std::make_shared<StbImage>();
		boostImage->resize(width, height);
		boostImage->phase = hasContrast ? StbImagePhase::contrastPhase : StbImagePhase::boostPhase;
		boostImage->imageNumber = imageNumber;
		auto transformImage = std::make_shared<StbImage>();
		transformImage->resize(width, height);
		transformImage->runIndex = runIndex;
		transformImage->imageNumber = imageNumber;
			
		auto markers = std::make_shared<Pci::Core::StbMarkers>();
		markers->valid = true;
		
		return StbResult{image, markers,transformImage,boostImage};
	}

	StbResult createStbResult(int runIndex, int imageNumber, bool hasContrast = false)
	{
		return createStbResult(runIndex, imageNumber, IMAGE_SIZE, IMAGE_SIZE, hasContrast);
	}

	StbResult createStbResultWithoutMarkers(int runIndex, int imageNumber, int width, int height, bool hasContrast = false)
	{
		auto result = createStbResult(runIndex, imageNumber, width, height, hasContrast);
		result.markers = nullptr;

		return result;
	}

	StbResult createStbResultWithoutMarkers(int runIndex, int imageNumber, bool hasContrast = false)
	{
		return createStbResultWithoutMarkers(runIndex, imageNumber, IMAGE_SIZE, IMAGE_SIZE, hasContrast);
	}

	StbResult createStbResultWithMarkers(int runIndex, int imageNumber, std::shared_ptr<Pci::Core::StbMarkers> markers)
	{
		auto result = createStbResult(runIndex, imageNumber);
		result.markers = markers;

		return result;
	}

	std::shared_ptr<Pci::Core::StbMarkers> createStbMarkers(Sense::Point marker1, Sense::Point marker2)
	{
		auto markers = std::make_shared<Pci::Core::StbMarkers>();
		markers->valid = true;
		markers->p1.x = static_cast<float>(marker1.x);
		markers->p1.y = static_cast<float>(marker1.y);
		markers->p2.x = static_cast<float>(marker2.x);
		markers->p2.y = static_cast<float>(marker2.y);

		return markers;
	}

	const int MAX_NR_OF_FRAMES = 100;
	const int IMAGE_SIZE = 1024;

};

TEST_F(TestStbRecording, When_Contructed_Recording_Is_Empty)
{
	StbRecording recording(MAX_NR_OF_FRAMES);
	
	EXPECT_EQ(0, recording.getImageCount());
	EXPECT_TRUE(recording.getAllImages().empty());
}

TEST_F(TestStbRecording, When_Image_And_Valid_Markers_Added_Then_Image_Is_Part_Of_AllImages)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResult = createStbResult(RUN_INDEX, IMAGE_NUMBER);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	auto allImages = recording.getAllImages();

	EXPECT_EQ(1, recording.getImageCount());
	ASSERT_FALSE(allImages.empty());
	EXPECT_EQ(stbResult.live, allImages[0].live);
	EXPECT_EQ(stbResult.boost, allImages[0].boost);
	EXPECT_EQ(stbResult.markers, allImages[0].markers);
}

TEST_F(TestStbRecording, When_Image_Added_Without_Markers_Then_Image_Is_Not_Part_Of_AllImages)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResult = createStbResultWithoutMarkers(RUN_INDEX, IMAGE_NUMBER);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	auto allImages = recording.getAllImages();

	EXPECT_EQ(0, recording.getImageCount());
	EXPECT_TRUE(allImages.empty());
}

TEST_F(TestStbRecording, When_Image_Added_Without_Markers_And_Include_All_Flag_Set_Then_Image_Is_Part_Of_AllImages)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResult = createStbResultWithoutMarkers(RUN_INDEX, IMAGE_NUMBER);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);
	recording.setIncludeImagesWithoutMarkers(true);

	auto allImages = recording.getAllImages();

	EXPECT_EQ(1, recording.getImageCount());
	ASSERT_FALSE(allImages.empty());
	EXPECT_EQ(stbResult.live, allImages[0].live);
	EXPECT_EQ(stbResult.boost, allImages[0].boost);
	EXPECT_EQ(stbResult.markers, allImages[0].markers);
}

TEST_F(TestStbRecording, When_Image_Added_Then_RunIndex_Matches_The_Image)
{
	const int RUN_INDEX(10);
	const int IMAGE_NUMBER(0);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResult = createStbResult(RUN_INDEX, IMAGE_NUMBER);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	EXPECT_EQ(RUN_INDEX, recording.getRunIndex());
}

TEST_F(TestStbRecording, When_No_Images_Added_Then_RunIndex_Equals_MinusOne)
{
	const int RUN_INDEX(-1);

	StbRecording recording(MAX_NR_OF_FRAMES);

	EXPECT_EQ(RUN_INDEX, recording.getRunIndex());
}

TEST_F(TestStbRecording, When_More_Than_Max_Frames_Added_Then_Old_Frames_Are_Removed)
{
	const int RUN_INDEX(10);	
	const int FRAMES_TO_ADD = MAX_NR_OF_FRAMES + 10;

	StbRecording recording(MAX_NR_OF_FRAMES);

	for (int i = 0; i < FRAMES_TO_ADD; i++)
	{
		auto stbResult = createStbResult(RUN_INDEX, i + 1);
		recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);
	}

	EXPECT_EQ(MAX_NR_OF_FRAMES, recording.getImageCount());
	EXPECT_EQ(MAX_NR_OF_FRAMES, recording.getAllImages().size());

	EXPECT_EQ(FRAMES_TO_ADD, recording.getAllImages().back().live->imageNumber);
}

TEST_F(TestStbRecording, When_Single_MarkerImage_Available_Then_Marker_Region_Equals_Markers)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	const Sense::Point marker1(10, 10);
	const Sense::Point marker2(20, 20);

	auto markers = createStbMarkers(marker1, marker2);
	auto stbResult = createStbResultWithMarkers(RUN_INDEX, IMAGE_NUMBER, markers);

	StbRecording recording(MAX_NR_OF_FRAMES);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	auto markerRegion = recording.getMarkerRegion();
	EXPECT_EQ(marker2, markerRegion.bottomRight());
	EXPECT_EQ(marker1, markerRegion.topLeft());
}

TEST_F(TestStbRecording, When_Multiple_MarkerImages_Available_Then_Marker_Region_Equals_Extremes)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	const Sense::Point FIRST_MARKER_1(10, 10);
	const Sense::Point FIRST_MARKER_2(20, 20);

	const Sense::Point SECOND_MARKER_1(5, 10);
	const Sense::Point SECOND_MARKER_2(10, 10);

	auto markers1 = createStbMarkers(FIRST_MARKER_1, FIRST_MARKER_2);
	auto stbResult1 = createStbResultWithMarkers(RUN_INDEX, IMAGE_NUMBER, markers1);
	auto markers2 = createStbMarkers(SECOND_MARKER_1, SECOND_MARKER_2);
	auto stbResult2 = createStbResultWithMarkers(RUN_INDEX, IMAGE_NUMBER, markers2);

	StbRecording recording(MAX_NR_OF_FRAMES);
	recording.addImage(stbResult1.live, stbResult1.markers, stbResult1.registeredLive, stbResult1.boost);
	recording.addImage(stbResult2.live, stbResult2.markers, stbResult2.registeredLive, stbResult2.boost);

	auto markerRegion = recording.getMarkerRegion();
	EXPECT_EQ(FIRST_MARKER_2, markerRegion.bottomRight());
	EXPECT_EQ(SECOND_MARKER_1, markerRegion.topLeft());
}


TEST_F(TestStbRecording, When_No_MarkerImages_Available_In_Square_Image_Then_Marker_Region_Equals_Empty_Rectangle)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	auto stbResult = createStbResultWithoutMarkers(RUN_INDEX, IMAGE_NUMBER);

	StbRecording recording(MAX_NR_OF_FRAMES);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	auto markerRegion = recording.getMarkerRegion();
	EXPECT_EQ(Sense::Point(0,0), markerRegion.center());
	EXPECT_EQ(0, markerRegion.width());
	EXPECT_EQ(0, markerRegion.height());
}

TEST_F(TestStbRecording, When_Images_In_Recording_Then_getBoundries_Returns_ValidRect_Of_First_Image)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);
	const int IMAGE_WIDTH(1000);
	const int IMAGE_HEIGHT(800);

	auto stbResult = createStbResultWithoutMarkers(RUN_INDEX, IMAGE_NUMBER, IMAGE_WIDTH, IMAGE_HEIGHT);

	StbRecording recording(MAX_NR_OF_FRAMES);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	auto boundries = recording.getImageBoundries();
	EXPECT_EQ(IMAGE_WIDTH, boundries.width());
	EXPECT_EQ(IMAGE_HEIGHT, boundries.height());
	EXPECT_EQ(stbResult.live->validRect, boundries);
}

TEST_F(TestStbRecording, When_No_Images_In_Recording_Then_getBoundries_Return_Empty_Rectangle)
{
	StbRecording recording(MAX_NR_OF_FRAMES);
	
	EXPECT_EQ(Sense::Rect(), recording.getImageBoundries());
}

TEST_F(TestStbRecording, When_No_Images_In_Recording_Then_getLastBoostImage_Returns_Empty_StbResult)
{
	StbRecording recording(MAX_NR_OF_FRAMES);
	const StbResult EMPTY_RESULT;

	auto lastBoostedImage = recording.getLastBoostedImage();

	EXPECT_EQ(EMPTY_RESULT.boost, lastBoostedImage.boost);
	EXPECT_EQ(EMPTY_RESULT.live, lastBoostedImage.live);
	EXPECT_EQ(EMPTY_RESULT.markers, lastBoostedImage.markers);
}

TEST_F(TestStbRecording, When_MultipleBoostedAndContrastImages_In_Recording_Then_getLastBoostedImage_Returns_Last_StbResult_Without_Contrast)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER_BASE(0);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResultNoContrast1 = createStbResult(RUN_INDEX, IMAGE_NUMBER_BASE + 1, false);
	auto stbResultNoContrast2 = createStbResult(RUN_INDEX, IMAGE_NUMBER_BASE + 2, false);
	auto stbResultContrast1 = createStbResult(RUN_INDEX, IMAGE_NUMBER_BASE + 3, true);
	auto stbResultContrast2 = createStbResult(RUN_INDEX, IMAGE_NUMBER_BASE + 4, true);

	recording.addImage(stbResultNoContrast1.live, stbResultNoContrast1.markers, stbResultNoContrast1.registeredLive, stbResultNoContrast1.boost);
	recording.addImage(stbResultNoContrast2.live, stbResultNoContrast2.markers, stbResultNoContrast2.registeredLive, stbResultNoContrast2.boost);
	recording.addImage(stbResultContrast1.live, stbResultContrast1.markers, stbResultContrast1.registeredLive, stbResultContrast1.boost);
	recording.addImage(stbResultContrast2.live, stbResultContrast2.markers, stbResultContrast2.registeredLive, stbResultContrast2.boost);


	auto lastBoostedImage = recording.getLastBoostedImage();

	EXPECT_EQ(stbResultNoContrast2.boost, lastBoostedImage.boost);
	EXPECT_EQ(stbResultNoContrast2.live, lastBoostedImage.live);
	EXPECT_EQ(stbResultNoContrast2.markers, lastBoostedImage.markers);
}

TEST_F(TestStbRecording, When_No_Images_In_Recording_Then_getContrastImage_Returns_Empty_StbResult)
{
	StbRecording recording(MAX_NR_OF_FRAMES);
	const StbResult EMPTY_RESULT;

	auto contrastImage = recording.getContrastImage();

	EXPECT_EQ(EMPTY_RESULT.boost, contrastImage.boost);
	EXPECT_EQ(EMPTY_RESULT.live, contrastImage.live);
	EXPECT_EQ(EMPTY_RESULT.markers, contrastImage.markers);
}

TEST_F(TestStbRecording, When_No_ContrastImages_In_Recording_Then_getContrastImage_Returns_Empty_StbResult)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER(0);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResult = createStbResult(RUN_INDEX, IMAGE_NUMBER, false);
	recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);

	const StbResult EMPTY_RESULT;
	auto contrastImage = recording.getContrastImage();

	EXPECT_EQ(EMPTY_RESULT.boost, contrastImage.boost);
	EXPECT_EQ(EMPTY_RESULT.live, contrastImage.live);
	EXPECT_EQ(EMPTY_RESULT.markers, contrastImage.markers);
}

TEST_F(TestStbRecording, When_MultipleContrastImages_In_Recording_Then_getContrastImage_Returns_Last_StbResult_With_Contrast)
{
	const int RUN_INDEX(1);
	const int IMAGE_NUMBER_1(0);
	const int IMAGE_NUMBER_2(1);

	StbRecording recording(MAX_NR_OF_FRAMES);
	auto stbResult1 = createStbResult(RUN_INDEX, IMAGE_NUMBER_1, true);
	auto stbResult2 = createStbResult(RUN_INDEX, IMAGE_NUMBER_2, true);
	recording.addImage(stbResult1.live, stbResult1.markers, stbResult1.registeredLive, stbResult1.boost);
	recording.addImage(stbResult2.live, stbResult2.markers, stbResult2.registeredLive, stbResult2.boost);

	auto contrastImage = recording.getContrastImage();

	EXPECT_EQ(stbResult2.boost, contrastImage.boost);
	EXPECT_EQ(stbResult2.live, contrastImage.live);
	EXPECT_EQ(stbResult2.markers, contrastImage.markers);
}

TEST_F(TestStbRecording, GivenStentBoostRunPressentAndLastImageHasNoMarkers_WhenGetLastBoostedImage_ThenTheLastBoostedImageWithMarkersIsReterned)
{
	const int RUN_INDEX(1);

	StbRecording recording(MAX_NR_OF_FRAMES);
	for (int i = 0; i < 4; i++)
	{
		auto stbResult = createStbResult(RUN_INDEX, i);
		recording.addImage(stbResult.live, stbResult.markers, stbResult.registeredLive, stbResult.boost);
	}

	auto stbResult4 = createStbResultWithoutMarkers(RUN_INDEX, 4);
	stbResult4.markers = std::make_shared<StbMarkers>();

	recording.addImage(stbResult4.live, stbResult4.markers, stbResult4.registeredLive, stbResult4.boost);


	auto boostedImage = recording.getLastBoostedImage();

	EXPECT_EQ(boostedImage.boost->imageNumber, 3);
}

