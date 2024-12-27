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
#include "StbParameters.h"
#include "StbImage.h"
#include "StbMask.h"
#include "XraySourceFxd.h"
#include "XraySinkFxd.h"
#include "PathUtil.h"
#include "XraySourceFxd.h"

#include "UnitTestLog.h"

class StbImageVerification : public ::testing::Test
{
public:
	const float MaxZoomFactor = 2.0f;

	StbImageVerification() :
		stbParams		(L"stb.ini", L"settings.ini"),
		stbAlgoMarkers	(pool),
		stbAlgoTransform( MaxZoomFactor),
		stbAlgoBoost(pool),
		SampleRunBoostCrcFile(L"stb_sample_run.boost.crc"),
		SampleRunFile(L"stb_sample_run.fxd"),
		stbImagesFxd(SampleRunFile),
		stbMask()
	{}

	Pci::Core::StbParameters	stbParams;
	Simplicity::ThreadPool		pool;
	Pci::Core::StbAlgoMarkers	stbAlgoMarkers;
	Pci::Core::StbAlgoTransform stbAlgoTransform;
	Pci::Core::StbAlgoBoost		stbAlgoBoost;
	std::wstring				SampleRunBoostCrcFile;
	std::wstring				SampleRunFile;
	CommonPlatform::Xray::XraySourceFxd		stbImagesFxd;
	Pci::Core::StbMask			stbMask;

	std::shared_ptr<CommonPlatform::Xray::XrayImage> createDifferenceImage( const CommonPlatform::Xray::XrayImage& image1, const CommonPlatform::Xray::XrayImage& image2)
	{
		auto output = std::make_shared<CommonPlatform::Xray::XrayImage>();

		if ( image1.width == image2.width &&
		     image1.height == image2.height )
		{
			output->resize(image1.width, image1.height);

			for( int y = 0; y<image1.height; y++)
			{
				for (int x = 0; x<image1.width ; x++)
				{
					short* ptr1      =  image1.pointer + ( y*image1.stride) + x;
					short* ptr2      =  image2.pointer + ( y*image2.stride) + x;
					short* ptrOutput =  output->pointer + ( y*output->stride) + x;

					short dif  = static_cast<short>(abs(*ptr2 - *ptr1));
					*ptrOutput = dif;
				}
			}
		}
		return output;
	}

	struct HistoGram
	{
		int freq0;
		int freq1;
		int freq2;
		int freq3_20;
		int freq21_200;
		int freq201_2000;
		int freq2000_Max;
		short minValue;
		short maxValue;
		double meanValue;
	};

	struct HistoGram calculateHistogram(const CommonPlatform::Xray::XrayImage& image)
	{
		HistoGram data = {};

		data.minValue = SHRT_MAX;
		data.maxValue = 0;

		uint64_t sum = 0;
		uint64_t pixelcount = 0;

		for( int y = 0; y<image.height; y++)
		for (int x = 0; x<image.width;  x++)
		{
			short* ptr  = image.pointer + ( y*image.stride) + x;
			short pixel = *ptr;

			// Calculate frequencies in histogram
			if ( pixel == 0 )			data.freq0++;
			else if ( pixel == 1 )		data.freq1++;
			else if ( pixel == 2 )		data.freq2++;
			else if ( pixel <= 20 )		data.freq3_20++;
			else if ( pixel <= 200 )	data.freq21_200++;
			else if ( pixel <= 2000 )	data.freq201_2000++;
			else						data.freq2000_Max++;

			// Min/Max
			if ( pixel < data.minValue)
			{
				data.minValue = pixel;
			}
			if ( pixel > data.maxValue)
			{
				data.maxValue = pixel;
			}

			// Average
			sum += pixel;
			pixelcount++;
		}

		data.meanValue = (double) sum / pixelcount;

		return data;
	}

	void compareProcessedImagesWithReferenceImage(const std::wstring& inputFilename, const std::wstring& referenceFilename, double maxPixelValue, double maxMeanPixelValue)
	{
		CommonPlatform::Xray::XraySourceFxd stbInputImages(inputFilename);
		CommonPlatform::Xray::XraySourceFxd stbReferenceImages(referenceFilename);

		std::shared_ptr<CommonPlatform::Xray::XrayImage> resultingImage = createFinalBoostImage(stbInputImages);
		
		int lastImageInReferenceData	= stbReferenceImages.getImageCount()-1;
		auto referenceImage				= stbReferenceImages.getImage(lastImageInReferenceData);
		auto diffImage					= createDifferenceImage( *resultingImage, *referenceImage);

		// Save files
#ifndef _DEBUG
		{
			CommonPlatform::Xray::XraySinkFxd boostImageFile(L"boost.fxd");
			boostImageFile.addImage(resultingImage);

			CommonPlatform::Xray::XraySinkFxd diffImageFile(L"difference.fxd");
			diffImageFile.addImage(diffImage);
		}
#endif
		auto histogram = calculateHistogram( *diffImage );

		UNITTEST_LOG(L"Histogram of difference image:"			);
		UNITTEST_LOG(L"    0="		<< histogram.freq0			);
		UNITTEST_LOG(L"    1="		<< histogram.freq1			);
		UNITTEST_LOG(L"    2="		<< histogram.freq2			);
		UNITTEST_LOG(L"  <20="		<< histogram.freq3_20		);
		UNITTEST_LOG(L" <200="		<< histogram.freq21_200		);
		UNITTEST_LOG(L"<2000="		<< histogram.freq201_2000	);
		UNITTEST_LOG(L">2000="		<< histogram.freq2000_Max	);
		UNITTEST_LOG(L"Minimum:"	<< histogram.minValue		);
		UNITTEST_LOG(L"Maximum:"	<< histogram.maxValue		);
		UNITTEST_LOG(L"Mean Value:" << histogram.meanValue		);

		ASSERT_LE( histogram.meanValue, maxMeanPixelValue);
		ASSERT_LE( histogram.maxValue, maxPixelValue);
	}

	std::shared_ptr<CommonPlatform::Xray::XrayImage> createFinalBoostImage(CommonPlatform::Xray::XraySourceFxd& inputImages)
	{
		Pci::Core::StbAlgoTransform::Transform transform;
		auto markers = std::make_shared<Pci::Core::StbMarkers>();
		auto boost		= std::make_shared<Pci::Core::StbImage>();

		std::shared_ptr<CommonPlatform::Xray::XrayImage> outputImage;

		for (int i = 0; i < inputImages.getImageCount(); i++)
		{
			auto img = inputImages.getImage(i);

			// From metadata of reference experiment
			// detector pixel size   = 0.154
			// iso center pixel size = 0.112  (gives indication of SID during examination)
			// Valid rectangle and shutters :  80 pixels from image edge

			// Set metadata identical to circumstances in which reference image was generated
			img->mmPerPixel.x = 0.154;
			img->mmPerPixel.y = 0.154;
			img->geometry.sourceImageDistance = (img->mmPerPixel.x * img->geometry.sourceObjectDistance) / 0.112;
			img->validRect = Sense::Rect(80,80,944,944);
			img->shutters  = Sense::Rect(80,80,944,944);
			
			CommonPlatform::Xray::Point scale(0.0);
			if (img->mmPerPixel.x > 0 && img->mmPerPixel.y > 0)
			{
				scale = CommonPlatform::Xray::Point(1.0) / img->mmPerPixel;
			}

			img->frustumToPixels = CommonPlatform::Xray::Matrix::scale(CommonPlatform::Xray::Point(1.0, -1.0));
			img->frustumToPixels = CommonPlatform::Xray::Matrix::scale(scale) * img->frustumToPixels;
			img->frustumToPixels = CommonPlatform::Xray::Matrix::translate(CommonPlatform::Xray::Point(img->width, img->height) / 2.0) * img->frustumToPixels;

			stbAlgoMarkers.process(stbParams, img, markers, false, false);
			stbAlgoTransform.process(stbParams, img, markers, transform, false);
			stbAlgoBoost.boost(stbParams, transform, img, boost, false);
			stbMask.mask(boost->refMarkers, *boost, *boost);

			if (markers->valid )
			{
				UNITTEST_LOG(L"Image:" << i << " Markers at (" << markers->p1.x << "," << markers->p1.y << ")(" << markers->p2.x << "," << markers->p2.y << ")");
			}
			else
			{
				UNITTEST_LOG(L"Image:" << i << " No valid Markers found");
			}

			outputImage = boost;
		}

		return outputImage;
	}
};

#if !_BullseyeCoverage
#ifdef _DEBUG
TEST_F(StbImageVerification, GivenOutputDataOfVerifiedReleaseAsReference_WhenProcessingTheInputData_ThenOutputDataIsIdenticalToReferenceData)
{
	//Compare new created StentBoostLive images by SoftwareUnderTest with recorded processed output of previous release.
	compareProcessedImagesWithReferenceImage(	L"moving_contrast_HK293-16_run021_stb_angio.fxd", 
											    L"moving_contrast_HK293-16_run021_stb_angio.boost_DEBUG.fxd", 0, 0.0);
}
#else
TEST_F(StbImageVerification, GivenOutputDataOfVerifiedReleaseAsReference_WhenProcessingTheInputData_ThenOutputDataIsIdenticalToReferenceData)
{
	// Compare new created StentBoostLive images by SoftwareUnderTest with recorded processed output of previous release.
	compareProcessedImagesWithReferenceImage(L"moving_contrast_HK293-16_run021_stb_angio.fxd",
		L"SimStudy_2095_run007_stb_preview.fxd", 1, 0.00002);
}
#endif
#endif

TEST_F(StbImageVerification, GivenValidatedReferenceData_WhenProcessingTheInputData_ThenOutputDataIsComparableToReferenceData)
{
	// Compare new created StentBoostLive images by SoftwareUnderTest with recorded processed output of validated tuning software.
	// Differences caused by different floating point model between reference software (fast) and product (precise).
	// The maximum and mean value used as limits are the expected differences.
	compareProcessedImagesWithReferenceImage(	L"moving_contrast_HK293-16_run021_stb_angio.fxd", 
												L"moving_contrast_HK293-16_run021_stb_angio.boost.fxd", 896, 4.025);
}

#ifdef _DEBUG
TEST_F(StbImageVerification, DISABLED_GivenAValidRun_WhenProcessingTheRun_ThenTheCrcMatchesTheOutput)
#else
TEST_F(StbImageVerification, GivenAValidRun_WhenProcessingTheRun_ThenTheCrcMatchesTheOutput)
#endif
{
	Pci::Core::StbAlgoTransform::Transform transform;
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto boost = std::make_shared<Pci::Core::StbImage>();
	bool boosted = false;

	std::vector<uint32_t> boostCrc;

	std::ifstream boostCrcFile(CommonPlatform::Utilities::AbsolutePathFromExeDir(SampleRunBoostCrcFile), std::ios::binary);

	std::copy(std::istream_iterator<uint32_t>(boostCrcFile), std::istream_iterator<uint32_t>(), std::back_inserter(boostCrc));

	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		auto img = stbImagesFxd.getImage(i);
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
		ASSERT_TRUE(stbAlgoTransform.process(stbParams, img, markers, transform,  false));
		boosted |= transform.refMarkers.valid;
		ASSERT_EQ(boosted, stbAlgoBoost.boost(stbParams, transform, img, boost, false));

		ASSERT_EQ(boostCrc[i], Simplicity::OmpImage<short>::crc32(*boost));
	}
}

TEST_F(StbImageVerification, DISABLED_GenerateCrcForSampleRun)
{
	Pci::Core::StbAlgoTransform::Transform transform;
	auto markers = std::make_shared<Pci::Core::StbMarkers>();
	auto boost = std::make_shared<Pci::Core::StbImage>();

	std::vector<uint32_t> boostCrc;

	for (int i = 0; i < stbImagesFxd.getImageCount(); i++)
	{
		auto img = stbImagesFxd.getImage(i);
		ASSERT_TRUE(stbAlgoMarkers.process(stbParams, img, markers, false, false));
		ASSERT_TRUE(stbAlgoTransform.process(stbParams, img, markers, transform, false));
		ASSERT_TRUE(stbAlgoBoost.boost(stbParams, transform, img, boost, false));
		boostCrc.push_back(Simplicity::OmpImage<short>::crc32(*boost));
	}

	std::ofstream boostCrcFile(CommonPlatform::Utilities::AbsolutePathFromExeDir(SampleRunBoostCrcFile), std::ios::trunc);

	std::copy(boostCrc.cbegin(), boostCrc.cend(), std::ostream_iterator<uint32_t>(boostCrcFile, " "));
}