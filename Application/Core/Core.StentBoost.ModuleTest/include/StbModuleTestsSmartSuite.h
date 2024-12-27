// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>
#include <IpSimplicity.h>
#include <Sense/Common/Rect.h>
#include <fstream>

#include "XrayImage.h"
#include "StbAlgoMarkers.h"
#include "StbAlgoBoost.h"
#include "StbAlgoLive.h"
#include "StbParameters.h"
#include "StbImage.h"
#include "XraySourceFxd.h"
#include "XraySinkFxd.h"
#include "SystemType.h"
#include <algorithm>
class StbTestsSmartSuite : public ::testing::Test
{
	const float MaxZoomFactor = 2.0f;
public:
	StbTestsSmartSuite() :
		stbParams(L"stb.ini", L"settings.ini"),
		stbAlgoMarkers(pool),
		stbAlgoTransform(MaxZoomFactor),
		stbAlgoBoost(pool),
		stbAlgoLive(pool, CommonPlatform::Xray::SystemType::SmartSuite),
		stbImagesFxd(SampleRunFile),
		NoiseRunFile(L"stb_noise_run.fxd"),
		SampleRunFile(L"stb_sample_run.fxd"),
		SampleRunLiveCrcFile(L"stb_sample_run.live_smartsuite.crc")
	{}

	static const int TestImageCount = 20;
	static const short Black = 0x0000;
	static const short White = 0x3FFF;

	const std::wstring NoiseRunFile;
	const std::wstring SampleRunFile;
	const std::wstring SampleRunLiveCrcFile;

	std::shared_ptr<CommonPlatform::Xray::XrayImage> getColouredImage(short colour, int width = 256, int height = 256)
	{
		auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
		img->resize(width, height);
		auto pixelCount = img->stride * img->height;
		std::fill(img->pointer, img->pointer + pixelCount, colour);
		return img;
	}

	Pci::Core::StbParameters stbParams;
	Simplicity::ThreadPool pool;
	Pci::Core::StbAlgoMarkers stbAlgoMarkers;
	Pci::Core::StbAlgoTransform stbAlgoTransform;
	Pci::Core::StbAlgoBoost stbAlgoBoost;
	Pci::Core::StbAlgoLive stbAlgoLive;

	CommonPlatform::Xray::XraySourceFxd stbImagesFxd;
};

TEST_F(StbTestsSmartSuite, GivenABlackImageRun_WhenExtractingMarkers_ThenReturnsNoValidMarkers)
{
	auto blackImg = getColouredImage(Black);
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < TestImageCount; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, blackImg, markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_FALSE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenAWhiteImageRun_WhenExtractingMarkers_ThenReturnsNoValidMarkers)
{
	auto whiteImg = getColouredImage(White);
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < TestImageCount; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, whiteImg, markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_FALSE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenANoiseImageRun_WhenExtractingMarkers_ThenReturnsNoValidMarkers)
{
	CommonPlatform::Xray::XraySourceFxd noiseRun(NoiseRunFile);
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < noiseRun.getImageCount(); i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, noiseRun.getImage(i), markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_FALSE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenAValidRun_WhenExtractingMarkers_ThenReturnsValidMarkers)
{
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, stbImagesFxd.getImage(i), markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_TRUE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenAValidRunWithInvalidShutters_WhenExtractingMarkers_ThenReturnsValidMarkers)
{
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		auto img = stbImagesFxd.getImage(i);
		auto partialWidth = img->width * 0.8;
		auto partialHeight = img->height * 0.8;
		img->shutters = Sense::Rect(partialWidth, partialHeight, partialWidth, partialHeight);
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_TRUE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenAValidRunWithAnInvalidRect_WhenExtractingMarkers_ThenReturnsValidMarkers)
{
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		auto img = stbImagesFxd.getImage(i);
		auto partialWidth = img->width * 0.8;
		auto partialHeight = img->height * 0.8;
		img->validRect = Sense::Rect(partialWidth, partialHeight, partialWidth, partialHeight);
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_TRUE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenAValidRunEndingWithBlackImages_WhenExtractingMarkers_ThenReturnsNoValidMarkers)
{
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, stbImagesFxd.getImage(i), markers, false, false));
	}
	ASSERT_TRUE(markers->valid);
	auto blackImg = getColouredImage(Black);
	for (int i = 0; i < TestImageCount; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, blackImg, markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_FALSE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenBlackImagesEndingWithAValidRun_WhenExtractingMarkers_ThenReturnsValidMarkers)
{
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto blackImg = getColouredImage(Black);
	for (int i = 0; i < TestImageCount; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, blackImg, markers, false, false));
	}
	ASSERT_FALSE(markers->valid);
	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, stbImagesFxd.getImage(i), markers, false, false));
	}
	
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_TRUE(markers->valid);
}

TEST_F(StbTestsSmartSuite, GivenAValidRunWithBlackImagesInbetween_WhenExtractingMarkers_ThenReturnsValidMarkers)
{
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	int totalImages = stbImagesFxd.getImageCount();
	int halfImages = totalImages / 2;
	auto blackImg = getColouredImage(Black);
	for (int i = 0; i < halfImages; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, stbImagesFxd.getImage(i), markers, false, false));
	}
	ASSERT_TRUE(markers->valid);
	for (int i = 0; i < TestImageCount; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, blackImg, markers, false, false));
	}
	ASSERT_FALSE(markers->valid);
	for (int i = halfImages; i < totalImages; i++)
	{
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, stbImagesFxd.getImage(i), markers, false, false));
	}
	ASSERT_TRUE(stbAlgoMarkers.end());
	ASSERT_TRUE(markers->valid);
}

#ifdef _DEBUG
TEST_F(StbTestsSmartSuite, DISABLED_GivenAValidRun_WhenProcessingTheRunWithSmartSuiteLUT_ThenTheCrcMatchesTheOutput)
#else
TEST_F(StbTestsSmartSuite, GivenAValidRun_WhenProcessingTheRunWithSmartSuiteLUT_ThenTheCrcMatchesTheOutput)
#endif
{
	Pci::Core::StbAlgoTransform::Transform transform;
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto boost = std::make_shared<Pci::Core::StbImage>();
	auto live = std::make_shared<Pci::Core::StbImage>();
	bool boosted = false;
	std::vector<uint32_t> liveCrc;

	std::ifstream liveCrcFile( CommonPlatform::Utilities::AbsolutePathFromExeDir(SampleRunLiveCrcFile), std::ios::binary);

	std::copy(std::istream_iterator<uint32_t>(liveCrcFile), std::istream_iterator<uint32_t>(), std::back_inserter(liveCrc));

	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		auto img = stbImagesFxd.getImage(i);
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
		ASSERT_TRUE(stbAlgoTransform.process(stbParams, img, markers, transform, false));
		boosted |= transform.refMarkers.valid;
		ASSERT_EQ(boosted, stbAlgoBoost.boost(stbParams, transform, img, boost, false));
		ASSERT_TRUE(stbAlgoLive.process(stbParams, img, live, false));

		ASSERT_EQ(liveCrc[i], Simplicity::OmpImage<short>::crc32(*live));
	}
}

/////////////////////////// Generator functions below, use only to update testdata /////////////////////////////////////////////

TEST_F(StbTestsSmartSuite, DISABLED_GenerateNoiseRun)
{
	CommonPlatform::Xray::XraySinkFxd sink(NoiseRunFile);
	srand(_time32(nullptr));
	for (int i = 0; i < TestImageCount; i++)
	{
	auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
		img->resize(512, 512);
		auto pixelCount = img->stride * img->height;
	
		std::generate(static_cast<short*>(img->pointer), static_cast<short*> (img->pointer + pixelCount), []() { return static_cast<short> (rand() % White); });

		sink.addImage(img);
	}
}

TEST_F(StbTestsSmartSuite, DISABLED_GenerateCrcForSampleRun)
{
	Pci::Core::StbAlgoTransform::Transform transform;
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto boost = std::make_shared<Pci::Core::StbImage>();
	auto live = std::make_shared<Pci::Core::StbImage>();

	std::vector<uint32_t> liveCrc;

	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		auto img = stbImagesFxd.getImage(i);
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
		ASSERT_TRUE(stbAlgoTransform.process(stbParams, img, markers, transform, false));
		ASSERT_TRUE(stbAlgoBoost.boost(stbParams, transform, img, boost, false));
		ASSERT_TRUE(stbAlgoLive.process(stbParams, img, live, false));
		liveCrc.push_back(Simplicity::OmpImage<short>::crc32(*live));
	}

	std::ofstream liveCrcFile( CommonPlatform::Utilities::AbsolutePathFromExeDir(SampleRunLiveCrcFile), std::ios::trunc);

	std::copy(liveCrc.cbegin(), liveCrc.cend(), std::ostream_iterator<uint32_t>(liveCrcFile, " "));
}