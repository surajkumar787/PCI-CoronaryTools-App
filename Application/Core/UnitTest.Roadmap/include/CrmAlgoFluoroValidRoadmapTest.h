// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoFluoroValidRoadmapTest.h

#pragma once
#include <gtest/gtest.h>
#include "NullLogWriter.h"
#include "CrmAlgoFluoro.h"
#include "OscStub.h"

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;

class CrmAlgoFluoroValidRoadmapTest : public ::testing::Test
{
public:
	CrmAlgoFluoroValidRoadmapTest() :
		m_log					(CommonPlatform::Logger(CommonPlatform::LoggerType::NullLogger)),
		m_pool					(false,"CrmTest"),
		m_param					(new CrmParameters(L"crm.ini"))
	{
	}

	~CrmAlgoFluoroValidRoadmapTest()
	{
		m_xrayImage.reset();
		m_crmRoadmap.reset();
		m_algoFluoro.reset();
		m_param.reset();
		Simplicity::system().resetHeap();
	}

	void SetUp() override
	{
		m_algoFluoro	= std::make_unique<CrmAlgoFluoro>(m_log, m_pool, *m_param, SystemType::Allura);

		m_crmRoadmap = std::make_shared<CrmRoadmap>();		
		m_crmRoadmap->data.push_back('t');
		m_crmRoadmap->width  = 200;
		m_crmRoadmap->height = 210;
		m_crmRoadmap->geometry.sourceImageDistance  = 2;
		m_crmRoadmap->geometry.sourceObjectDistance = 1.5;

		m_xrayImage	= std::make_shared<XrayImage>();

		m_xrayGeometry.angulation = 10.5;
		m_xrayGeometry.rotation   = 10.6;

		m_xrayImage->geometry	= m_xrayGeometry;

		m_xrayImage->shutters.left   = 1;
		m_xrayImage->shutters.top    = 1;
		m_xrayImage->shutters.right  = m_crmRoadmap->width;
		m_xrayImage->shutters.bottom = m_crmRoadmap->height;

		m_xrayImage->frameTime = 0.0666666666666;
		m_xrayImage->mmPerPixel.x = 2;

		reset();

		Test::SetUp();	
	}

	void  setAlgoFluoro(std::shared_ptr<CrmRoadmap> roadmap) { m_algoFluoro->set(roadmap); };
	bool algoFluoroProcess(std::shared_ptr<XrayImage> source, std::shared_ptr<CrmOverlay> overlay, std::shared_ptr<XrayImage> live, bool skip) { return m_algoFluoro->process(source, overlay, live, skip); };

	std::shared_ptr<XrayImage>& getImage() { return m_xrayImage; };
	std::shared_ptr<CrmRoadmap>& getRoadmap() { return m_crmRoadmap; };

private:
	CommonPlatform::Log&			m_log;
	Simplicity::ThreadPool			m_pool;
	std::unique_ptr<CrmParameters>	m_param;
	std::unique_ptr<CrmAlgoFluoro>	m_algoFluoro;
	std::shared_ptr<CrmRoadmap>		m_crmRoadmap;
	std::shared_ptr<XrayImage>		m_xrayImage;
	XrayGeometry					m_xrayGeometry;

};

TEST_F(CrmAlgoFluoroValidRoadmapTest, GivenAValidRoadmapWasSet_WhenProcessIsCalled_AndNoOscErrorsOccur_ThenProcessSucceeds)
{
	setAlgoFluoro(getRoadmap());
	ASSERT_TRUE(algoFluoroProcess(getImage(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));
}

TEST_F(CrmAlgoFluoroValidRoadmapTest, GivenAValidRoadmapWasSet_WhenProcessIsCalled_ThenCorrectRoiIsPassedToOSc)
{
	setAlgoFluoro(getRoadmap());
	ASSERT_TRUE(algoFluoroProcess(getImage(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));

	COscRoiCoord expectedRoi;
	expectedRoi.XMin = int(getImage()->shutters.left);
	expectedRoi.YMin = int(getImage()->shutters.top);
	expectedRoi.XMax = int(getImage()->shutters.right  - 1);
	expectedRoi.YMax = int(getImage()->shutters.bottom - 1);

	ASSERT_EQ(expectedRoi.XMin , Osc_ShuttersRoi.XMin);
	ASSERT_EQ(expectedRoi.YMin , Osc_ShuttersRoi.YMin);
	ASSERT_EQ(expectedRoi.XMax , Osc_ShuttersRoi.XMax);
	ASSERT_EQ(expectedRoi.YMax , Osc_ShuttersRoi.YMax);
}

TEST_F(CrmAlgoFluoroValidRoadmapTest, GivenAValidRoadmapWasSet_WhenProcessIsCalled_ThenFluoroSeqParamteresAreSetAccordingToRoadmap)
{
	setAlgoFluoro(getRoadmap());

	ASSERT_TRUE(algoFluoroProcess(getImage(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));

	ASSERT_EQ(float(1.0 / getImage()->frameTime) , Osc_FluoroFrameRate);
	ASSERT_EQ(getRoadmap()->width					, Osc_FluoroWidth	 );
	ASSERT_EQ(getRoadmap()->height				, Osc_FluoroHeight	 );
}

TEST_F(CrmAlgoFluoroValidRoadmapTest, GivenAValidRoadmapWasSet_WhenProcessIsCalled_ThenCorrectMMPerPixelIsCalculated)
{
	setAlgoFluoro(getRoadmap());

	ASSERT_TRUE(algoFluoroProcess(getImage(), std::make_shared<CrmOverlay>(), std::make_shared<XrayImage>(), true));

	float expectedMMPerPixel = float(getImage()->mmPerPixel.x * (getImage()->geometry.sourceObjectDistance / getImage()->geometry.sourceImageDistance));
	float expectedRotation	 = float(getImage()->geometry.rotation);
	float expectedAngulation = float(getImage()->geometry.angulation);

	ASSERT_EQ(expectedMMPerPixel , Osc_MmPerPixel);
	ASSERT_EQ(expectedRotation   , Osc_Rotation);
	ASSERT_EQ(expectedAngulation , Osc_Angulation);
}
}}
//TICS +OLC#009  magic numbers for a test are ok.