// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>
#include "StbFrangi.h"
#include "XraySinkFxd.h"
#include "StbContrastDetect.h"
#include "XraySourceFxd.h"
#include "XrayImage.h"
#include "TestUtils.h"


namespace Pci{  namespace Test{

using namespace StentBoostAlgo;
using namespace CommonPlatform::Xray;

static const int contrastFrangiDownSample = 4;	//for efficiency, the input is downsampled before frangi is applied. this is the scale factor
static const float contrastFrangiStrength = 8.0f;	//the strength of the frangi filter
static const float contrastFrangiBlobbiness = 2.5f;	//the blobbiness of the frangi filter
static const float contrastFrangiScale = 2.0f;	//the filter scale of the frangi filter

// This class tests the implementation of the class PciEpx 
class TestStbFrange : public ::testing::Test
{
public:

	void SetUp() override
	{
		XraySourceFxd xrayImage(L"stb_subtract_run15.fxd");

		std::shared_ptr<const CommonPlatform::Xray::XrayImage> image = xrayImage.getImage(40);
		m_widthDownSampled = image->width / contrastFrangiDownSample;
		m_heightDownSample = image->height / contrastFrangiDownSample;
			
		m_image.resize(m_widthDownSampled, m_heightDownSample);
		std::memset(m_image.pointer, 0, m_image.height*m_image.width * sizeof(float));
		StbContrastDetect::imageDownScale(getMaxGrayValue(), *image, m_image);
		m_frangiImage = std::make_shared<StbFrangiImage<float>>();
		m_frangiImage->resize(m_widthDownSampled, m_heightDownSample);
	}

	void TearDown() override
	{
		Simplicity::system().resetHeap();
	}

	std::shared_ptr<StbFrangiImage<float>>& getFrangiImage() { return m_frangiImage; };
	StbFrangiImage<float>& getImage() { return m_image; };
	float getMaxGrayValue() { return static_cast<float>((1 << 15) - 1); };

private:
	int m_widthDownSampled;
	int m_heightDownSample;
	StbFrangiImage<float>   m_image;
	std::shared_ptr<StbFrangiImage<float>>	m_frangiImage;

};



TEST_F(TestStbFrange, GivenImageWithSizeNull_ThenInintializationisNotSuccesfull)
{
	StbFrangi frangi;
	ASSERT_FALSE(frangi.init(0, 0, 1.0f));
}

TEST_F(TestStbFrange, GivenImageWithScallNull_ThenInintializationisNotSuccesfull)
{

	StbFrangi frangi;
	ASSERT_FALSE(frangi.init(512, 512, 0.0f));
}

TEST_F(TestStbFrange, GivenImageWithScallToLarge_ThenInintializationisNotSuccesfull)
{

	StbFrangi frangi;
	ASSERT_FALSE(frangi.init(512, 512, 1000.0f));
}


TEST_F(TestStbFrange, GivenImageWithHeightNull_ThenInintializationisNotSuccesfull)
{

	StbFrangi frangi;
	ASSERT_FALSE(frangi.init(0, 512, 2.0f));
}


TEST_F(TestStbFrange, GivenImageWithWidthNull_ThenInintializationisNotSuccesfull)
{

	StbFrangi frangi;
	ASSERT_FALSE(frangi.init(512, 0, 2.0f));
}


TEST_F(TestStbFrange, GivenAValidImage_ThenFrangiImageIsCreated)
{

	StbFrangi frangi;

	auto& image = getImage();
	auto& output = getFrangiImage();
	frangi.init(image.width, image.height, contrastFrangiScale);
	frangi.filter(image, output, contrastFrangiBlobbiness, contrastFrangiStrength);

	saveNewReferenceResult(output, "frangiRawFloatImage.raw");
	
	StbFrangiImage<float> expected;
	expected.resize(image.width, image.height);
	loadExpectedImage(expected, "frangiRawFloatImage.raw");

	auto result = Simplicity::OmpImage<float>::crc32(*output);
	auto expectedCrc = Simplicity::OmpImage<float>::crc32(expected);
	ASSERT_EQ( result,expectedCrc);

}
}}
