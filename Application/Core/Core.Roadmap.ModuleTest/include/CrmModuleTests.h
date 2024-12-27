// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmModuleTests.h
#pragma once
#include "XraySourceFxd.h"
#include "XrayImage.h"
#include "XraySinkFxd.h"
#include "CrmAlgoAngio.h"
#include "CrmAlgoFluoro.h"
#include "CrmErrors.h"
#include "SystemType.h"
#include "PathUtil.h"
#include <gtest/gtest.h>
#include <stdio.h>
#include <Windows.h>
#include "IpSimplicity/Simplicity/Base.h"
#include <fstream>
#include <string>

using namespace Pci::Core;

class CrmModuleTests : public testing::Test
{
public:
	std::shared_ptr<CrmRoadmap> roadMap;
	XraySourceFxd sourceFxd;
	Simplicity::ThreadPool pool;
	CrmParameters crmParams;
	CrmAlgoAngio algoAngio;
	int imageCount;

	const short black;
	const short white;

	std::function<void(const std::shared_ptr<XrayImage> &)>  eventImage;

	CrmModuleTests() :
		sourceFxd(L"crm_expo_2.fxd"),
		crmParams(L"crm.ini", L"settings.ini"),
		algoAngio(pool, crmParams),
		imageCount(0),
		black(0x0000),
		white(0x3FFF)
	{
	}

	~CrmModuleTests()
	{
	}

	void SetUp() override
	{
		roadMap = std::make_shared<CrmRoadmap>();
	}

	void TearDown() override
	{
		imageCount = 0;
	}

	void createRoadmap()
	{
		eventImage = [&](const std::shared_ptr<XrayImage> & image) { ASSERT_TRUE(algoAngio.process(image)); };
		sendImages();

		CrmErrors::Error error = algoAngio.end(roadMap);

		ASSERT_TRUE(error == CrmErrors::Error::OK);
	}

	void sendImages(int imagesToSend = -1, XraySourceFxd * source = nullptr) 
	{
		if(!source)
			source = &sourceFxd;

		if(imagesToSend == -1)
			imagesToSend = source->getImageCount();

		imageCount = imagesToSend;

		for(int i = 0; i < imageCount; i++)
		{
			std::shared_ptr<XrayImage> image = source->getImage(i);
			if(eventImage)
				eventImage(image);
		}
	}

	std::wstring getFxdOutputFileName() 
	{
		const ::testing::TestInfo* const test_info =
			::testing::UnitTest::GetInstance()->current_test_info();

		std::string testName = test_info->name();
		std::wstring fxdFileName(testName.begin(), testName.end());

		return CommonPlatform::Utilities::AbsolutePathFromExeDir(fxdFileName + L".fxd");
	}

	void readVectorFromFile(std::wstring fileName, char* buffer, size_t size)
	{
		std::ifstream fin(CommonPlatform::Utilities::AbsolutePathFromExeDir(fileName), std::ios::in | std::ios::binary);
		ASSERT_TRUE(fin.is_open());
		fin.read(buffer, size);
		fin.close();
	}

    void writeVectorToFile(std::wstring fileName, char* buffer, size_t totalSizeInBytes)
    {
        std::ofstream fout(CommonPlatform::Utilities::AbsolutePathFromExeDir(fileName), std::ios::out | std::ios::binary);
		ASSERT_TRUE(fout.is_open());
        fout.write(buffer, totalSizeInBytes);
        fout.close();
    }

	void storePerformanceMeasurement(std::string name, double timeInMs)
	{
		bool saved = false;
		std::string filename = CommonPlatform::Utilities::AbsolutePathFromExeDir(name + ".csv");
		try
		{
			std::ofstream out(filename);
			out << "Measurement,TimeInMs" << std::endl;
			out << name << "," << timeInMs << std::endl;
			out.close();
			saved = true;
		}
		catch (...)
		{
			std::cout << "CrmModuleTest::storePerformanceMeasurement : coudln't save performance measurement";
		}
	}

	double getTimeStamp()
	{
		LARGE_INTEGER perfCount;
		LARGE_INTEGER perfFreq;
		QueryPerformanceCounter(&perfCount);
		QueryPerformanceFrequency(&perfFreq);

		return (double)(perfCount.QuadPart / ((perfFreq.QuadPart) / 1000));
	}


};

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenProcessed_ProcessSucceeds)
{
	createRoadmap();
}

TEST_F(CrmModuleTests, GivenATooShortAngioRun_WhenProcessed_ProcessFailesWithTooShortAngioError)
{
	eventImage = [&](const std::shared_ptr<XrayImage> & image) { ASSERT_TRUE(algoAngio.process(image)); };
	sendImages(20);

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_TRUE(error == CrmErrors::Error::AngioTooShort);
}

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenTableMovedAfterAngio_ProcessSucceeds)
{
	double originalTablePos = 0;

	eventImage = [&](const std::shared_ptr<XrayImage> & image) {
		if(image->imageIndex == 0) originalTablePos = image->geometry.tableLongitudinalOffset;
		if(image->imageIndex > (imageCount - 5)) 
		{
			image->geometry.tableLongitudinalOffset = (originalTablePos + 0.6);
			image->geometry.updateSpaces();
			ASSERT_FALSE(algoAngio.process(image));
		}
		else
		{
			ASSERT_TRUE(algoAngio.process(image));
		}
	};

	sendImages();

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_TRUE(error == CrmErrors::Error::OK);
}

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenTableMovedDuringAngio_ProcessFailsWithTableMoved)
{
	double originalTablePos = 0;

	eventImage = [&](const std::shared_ptr<XrayImage> & image) {
		if(image->imageIndex == 0) originalTablePos = image->geometry.tableLongitudinalOffset;

		if(image->imageIndex > 10 && image->imageIndex < (imageCount - 20)) 
		{
			image->geometry.tableLongitudinalOffset = (originalTablePos + 10);
			image->geometry.updateSpaces();
			ASSERT_FALSE(algoAngio.process(image));
		}
		else if( image->imageIndex <= 10)
		{
			ASSERT_TRUE(algoAngio.process(image));
		}
		else
		{
			ASSERT_FALSE(algoAngio.process(image));
		}

	};

	sendImages();

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_TRUE(error == CrmErrors::Error::PatientTableMoved);
}

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenInvalidGeometryDuringAngio_ProcessFailsWithUnknownError)
{
	eventImage = [&](const std::shared_ptr<XrayImage> & image) {
		image->geometry.angulation = -900;
		image->shutters.left = -10000;
		image->geometry.rotation = 10000;
		ASSERT_TRUE(algoAngio.process(image));
	};

	sendImages();

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_TRUE(error == CrmErrors::Error::OK);
}

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenFullWhiteImages_ProcessFailsWithUnknownError)
{
	XraySinkFxd fxdSink(getFxdOutputFileName());
	eventImage = [&](const std::shared_ptr<XrayImage> & image) {
		short onepx = white;

		for(int i = 0; i < image->height * image->stride; i++)
		{
			memcpy(image->pointer + i, &onepx, sizeof(short));
		}

		ASSERT_FALSE(algoAngio.process(image));
		fxdSink.addImage(image);
	};

	sendImages();

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_TRUE(error == CrmErrors::Error::Unknown);
}

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenFullBlackImages_ProcessFailsWithUnknownError)
{
	XraySinkFxd fxdSink(getFxdOutputFileName());
	eventImage = [&](const std::shared_ptr<XrayImage> & image) {
		short onepx = black;

		for(int i = 0; i < image->height * image->stride; i++)
		{
			memcpy(image->pointer + i, &onepx, sizeof(short));
		}

		ASSERT_FALSE(algoAngio.process(image));
		fxdSink.addImage(image);
	};

	sendImages();

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_EQ(CrmErrors::Error::Unknown, error);
}

TEST_F(CrmModuleTests, GivenAValidAngioRun_WhenImagesContainOnlyNoise_ProcessSucceedsWithWarning)
{
	XraySinkFxd fxdSink(getFxdOutputFileName());
	eventImage = [&](const std::shared_ptr<XrayImage> & image) {

		for(int i = 0; i < image->height * image->stride; i++)
		{
			short onepx = rand() % white;
			memcpy(image->pointer + i, &onepx, sizeof(short));
		}

		ASSERT_TRUE(algoAngio.process(image));
		fxdSink.addImage(image);
	};

	sendImages();

	CrmErrors::Error error = algoAngio.end(roadMap);

	ASSERT_EQ(CrmErrors::Error::OK, error);
	ASSERT_NE(CrmErrors::Error::OK, roadMap->warning);
}

#ifdef _DEBUG
TEST_F(CrmModuleTests, DISABLED_GivenAValidRoadmapIsSet_WhenAValidFluoroRunIsProcessed_ThenTheCorrectOverlaysAreSelected)
#else
TEST_F(CrmModuleTests, GivenAValidRoadmapIsSet_WhenAValidFluoroRunIsProcessed_ThenTheCorrectOverlaysAreSelected)
#endif
{
	double timeStampStart = getTimeStamp();
	createRoadmap();
	double timeStampStop = getTimeStamp();

	storePerformanceMeasurement("CreateRoadmap_crm_expo_2.fxd", timeStampStop - timeStampStart);

	XraySourceFxd sourceFxdFluoro(CommonPlatform::Utilities::AbsolutePathFromExeDir(L"crm_fluo_2.fxd"));

	int fxdImageCount = sourceFxdFluoro.getImageCount();
	CrmAlgoFluoro algoFluoro(pool, crmParams, SystemType::Allura);

	algoFluoro.set(roadMap);

//#define GENERATE_NEW_GROUD_THRUTH_DATA_FILES 

#ifdef GENERATE_NEW_GROUD_THRUTH_DATA_FILES
    std::vector<uint32_t> crcVector;
    std::vector<uint32_t> crcLiveVector;
    std::vector<Matrix>   frustumToPixelVector;

    for(int i = 0; i < fxdImageCount; i++)
    {
        std::shared_ptr<CrmOverlay> crmOverlay	= std::make_shared<CrmOverlay>();
        std::shared_ptr<XrayImage>  live		= std::make_shared<XrayImage>();
        std::shared_ptr<XrayImage> image = sourceFxdFluoro.getImage(i);

        algoFluoro.process(image, crmOverlay, live, false);

        frustumToPixelVector.push_back( crmOverlay->frustumToPixels);
        crcVector           .push_back( Simplicity::OmpImage<unsigned char>::crc32(*crmOverlay));
        crcLiveVector       .push_back( Simplicity::OmpImage<short>::crc32(*live));
    }

    writeVectorToFile(L"new_crm_fluo_2_roadmap_CRC.dat",      (char*)&crcVector[0],               crcVector.size() * sizeof(uint32_t));
    writeVectorToFile(L"new_crm_fluo_2_liveImages_CRC.dat",   (char*)&crcLiveVector[0],           crcLiveVector.size() * sizeof(uint32_t));
    writeVectorToFile(L"new_crm_fluo_2_frustumToPixels.dat", (char*)&frustumToPixelVector[0],    frustumToPixelVector.size() * sizeof(Matrix));

#else

    std::vector<uint32_t> crcVector(fxdImageCount);
    std::vector<uint32_t> crcLiveVector(fxdImageCount);
    std::vector<Matrix>   frustumToPixelVector(fxdImageCount);

    readVectorFromFile(L"crm_fluo_2_roadmap_CRC.dat", (char *)crcVector.data(), crcVector.size() * sizeof(uint32_t));
    readVectorFromFile(L"crm_fluo_2_liveImages_CRC.dat", (char *)crcLiveVector.data(), crcLiveVector.size() * sizeof(uint32_t));
    readVectorFromFile(L"crm_fluo_2_frustumToPixels.dat", (char *)frustumToPixelVector.data(), frustumToPixelVector.size() * sizeof(Matrix));


	timeStampStart = getTimeStamp();
	for(int i = 0; i < fxdImageCount; i++)
	{
		std::shared_ptr<CrmOverlay> crmOverlay	= std::make_shared<CrmOverlay>();
		std::shared_ptr<XrayImage>  live		= std::make_shared<XrayImage>();
		std::shared_ptr<XrayImage> image = sourceFxdFluoro.getImage(i);

		ASSERT_TRUE (algoFluoro.process(image, crmOverlay, live, false));
        ASSERT_EQ   (frustumToPixelVector[i], crmOverlay->frustumToPixels);

        auto crc = Simplicity::OmpImage<unsigned char>::crc32(*crmOverlay);
		if ( i < 10 ) std::cout << "CRC frame " << i << " = " << crc << std::endl;
        ASSERT_EQ   (crcVector[i], crc );

        crc = Simplicity::OmpImage<short>::crc32(*live);
        ASSERT_EQ	(crc, crcLiveVector[i]);
        ASSERT_TRUE (live->runIndex				== image->runIndex				);
        ASSERT_TRUE (live->imageIndex			== image->imageIndex			);
        ASSERT_TRUE (live->imageNumber			== image->imageNumber			);
        ASSERT_TRUE (live->seriesNumber			== image->seriesNumber			);
        ASSERT_TRUE (live->type					== image->type					);
        ASSERT_TRUE (live->validRect			== image->validRect				);
        ASSERT_TRUE (live->shutters				== image->shutters				);
        ASSERT_TRUE (live->frustumToPixels		== image->frustumToPixels		);
        ASSERT_TRUE (live->mmPerPixel			== image->mmPerPixel			);
        ASSERT_TRUE (live->frameTime			== image->frameTime				);
        ASSERT_TRUE	(live->geometry.angulation	== image->geometry.angulation	);
        ASSERT_TRUE	(live->epx.activityType     == image->epx.activityType		);
	}
	timeStampStop = getTimeStamp();

	storePerformanceMeasurement("ProcessFluoroImages_crm_fluo_2.fxd", timeStampStop - timeStampStart);
#endif

}

