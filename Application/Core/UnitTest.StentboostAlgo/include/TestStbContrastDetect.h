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


namespace Pci {	namespace Test {

using namespace StentBoostAlgo;
using namespace CommonPlatform::Xray;

// This class tests the implementation of the class PciEpx 
class TestStbContrastDetect : public ::testing::Test
{
public:

	void SetUp() override
	{
	}

private:

};


TEST_F(TestStbContrastDetect, GivenImageWithSizeNull_ThenInintializationisNotSuccesfull)
{

	StbContrastDetect constrast;
	constrast.init(512, 512, 14);

	XraySourceFxd xrayImage(L"stb_subtract_run15.fxd");

	std::shared_ptr<const CommonPlatform::Xray::XrayImage> image = xrayImage.getImage(40);
	StbContrastDesc contrastImage;
	constrast.preProcess(*image, contrastImage, 40);

	ASSERT_EQ(contrastImage.intensityFiltered, 0.001f);
}

}}
