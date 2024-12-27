// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoroErrorHandlingTest.h

#pragma once
#include <gtest/gtest.h>
#include "CrmAlgoFluoro.h"
#include "SystemType.h"
#include "OscStub.h"


//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;

class CrmAlgoFluoroErrorHandlingTest : public ::testing::Test
{
public:
	CrmAlgoFluoroErrorHandlingTest() :
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_pool(false, "CrmTest"),
		m_param(new CrmParameters(L"crm.ini"))
	{
	}

	void SetUp() override
	{
		m_algoFluoro = std::make_unique<CrmAlgoFluoro>(m_log, m_pool, *m_param, SystemType::Allura);

		m_crmRoadmap = std::make_shared<CrmRoadmap>();
		m_crmRoadmap->data.push_back('t');
		m_crmRoadmap->height = 10;
		m_crmRoadmap->width = 10;

		m_xrayImage = std::make_shared<XrayImage>();

		reset();

		Test::SetUp();
	}

	~CrmAlgoFluoroErrorHandlingTest()
	{
		m_xrayImage.reset();
		m_crmRoadmap.reset();
		m_algoFluoro.reset();
		m_param.reset();
		Simplicity::system().resetHeap();
	}

	std::shared_ptr<XrayImage>& getImage() { return m_xrayImage; };
	std::shared_ptr<CrmRoadmap> getRoadmap() { return m_crmRoadmap; };

	void  setAlgoFluoro(std::shared_ptr<CrmRoadmap> roadmap) { m_algoFluoro->set(roadmap); };
	bool algoFluoroProcess(std::shared_ptr<XrayImage> source, std::shared_ptr<CrmOverlay> overlay, std::shared_ptr<XrayImage> live, bool skip) {return m_algoFluoro->process(source, overlay, live, skip); };

private:
	CommonPlatform::Log&		    m_log;
	Simplicity::ThreadPool			m_pool;
	std::unique_ptr<CrmParameters>  m_param;
	std::unique_ptr<CrmAlgoFluoro>  m_algoFluoro;
	std::shared_ptr<CrmRoadmap>		m_crmRoadmap;
	std::shared_ptr<XrayImage>		m_xrayImage;
	XrayGeometry					m_xrayGeometry;
};

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmap_WhenSetIsCalled_ThenTableApparentPositionMM_AndTableApparentPositionPix_AreReset)
{
	Osc_TablePosXMM = -1;
	Osc_TablePosYMM = -1;
	Osc_TablePosZMM = -1;

	Osc_TablePosXPix = -1;
	Osc_TablePosYPix = -1;

	setAlgoFluoro(getRoadmap());

	ASSERT_TRUE(Osc_TablePosXMM  == 0);
	ASSERT_TRUE(Osc_TablePosYMM  == 0);
	ASSERT_TRUE(Osc_TablePosZMM  == 0);
	ASSERT_TRUE(Osc_TablePosXPix == 0);
	ASSERT_TRUE(Osc_TablePosYPix == 0);
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenNoValidRoadmapWasSet_WhenProcessIsCalled_ThenError)
{
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_WhenProcessIsCalledWithEmptySourceImage_ThenError)
{
	setAlgoFluoro(getRoadmap() );
	ASSERT_FALSE(algoFluoroProcess(nullptr,std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(),true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_WhenProcessIsCalledWithEmptyCrmOverlay_ThenError)
{
	setAlgoFluoro(getRoadmap() );
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), nullptr , std::make_shared<XrayImage>(),true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_WhenProcessIsCalledWithEmptyLiveImage_ThenError)
{
	setAlgoFluoro(getRoadmap() );
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), nullptr, true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_AndProcessIsCalled_WhenSetFluoroSequenceParametersFails_ThenError)
{
	setAlgoFluoro(getRoadmap() );

	OscSetFluoroSequenceParameters_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_AndProcessIsCalled_WhenOscSetShuttersRoiFails_ThenError)
{
	setAlgoFluoro(getRoadmap() );

	OscSetShuttersRoi_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_AndProcessIsCalled_WhenSetCurrentGeometryFluoroFails_ThenError)
{
	setAlgoFluoro(getRoadmap() );

	OscSetCurrentGeometryFluoro_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_AndProcessIsCalled_WhenSetTableApparentPositionPixFails_ThenError)
{
	setAlgoFluoro(getRoadmap() );

	OscSetTableApparentPositionPix_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}

TEST_F(CrmAlgoFluoroErrorHandlingTest, GivenAValidRoadmapWasSet_AndProcessIsCalled_WhenFluoroProcessPictureFails_ThenError)
{
	setAlgoFluoro(getRoadmap() );

	OscFluoroProcessPicture_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(algoFluoroProcess(std::make_shared<XrayImage>(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}
}}

//TICS +OLC#009  magic numbers for a test are ok.