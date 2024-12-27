// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoAngioValidAngioTest.h

#pragma once
#include <gtest/gtest.h>
#include "CrmAlgoAngio.h"
#include "OscStub.h"


//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;

class CrmAlgoAngioValidAngioTest : public ::testing::Test
{
public:

	CrmAlgoAngioValidAngioTest() :
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_pool					(false,"CrmTest"),
		m_param					(new CrmParameters(L"crm.ini")),
		m_xrayGeometry()
	{
	}

	~CrmAlgoAngioValidAngioTest()
	{
		Simplicity::system().resetHeap();
	}

	void SetUp() override
	{
		m_crmRoadmap = std::make_shared<CrmRoadmap>();

		m_algoAngio	= std::make_unique<CrmAlgoAngio>(m_log, m_pool, *m_param);
		m_xrayImage	= std::make_shared<XrayImage>();
		m_xrayImage->resize(200,210);

		m_xrayGeometry.angulation = 10.5;
		m_xrayGeometry.rotation   = 10.6;
		m_xrayGeometry.sourceImageDistance  = 2;
		m_xrayGeometry.sourceObjectDistance = 1.5;

		m_xrayImage->geometry		= m_xrayGeometry;
		m_xrayImage->mmPerPixel.x = 2;

		m_xrayImage->shutters.left   = 1;
		m_xrayImage->shutters.top    = 1;
		m_xrayImage->shutters.right  = m_xrayImage->width;
		m_xrayImage->shutters.bottom = m_xrayImage->height;

		reset();

		Test::SetUp();	
	}

	void TearDown() override
	{
		m_xrayImage.reset();
		m_algoAngio.reset();
		m_crmRoadmap.reset();
		m_param.reset();
	}

	bool crmProcess(std::shared_ptr<XrayImage> image) { return m_algoAngio->process(image); };
	CrmErrors::Error crmEnd(const std::shared_ptr<CrmRoadmap>& roadmap) { return m_algoAngio->end(roadmap); };
	std::shared_ptr<XrayImage>& getImage() { return m_xrayImage; };
	std::shared_ptr<CrmRoadmap> getRoadmap() { return m_crmRoadmap; };

private:
	CommonPlatform::Log				&m_log;
	Simplicity::ThreadPool			 m_pool;
	std::unique_ptr<CrmParameters>	 m_param;
	std::unique_ptr<CrmAlgoAngio>    m_algoAngio;
	std::shared_ptr<CrmRoadmap>      m_crmRoadmap;
	std::shared_ptr<XrayImage>       m_xrayImage;
	XrayGeometry					 m_xrayGeometry;
};

TEST_F(CrmAlgoAngioValidAngioTest, GivenAValidImage_WhenProcessed_ThenCorrectRoiIsPassedToOSc)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_EQ(getImage()->shutters.left			, Osc_ShuttersRoi.XMin);
	ASSERT_EQ(getImage()->shutters.top			, Osc_ShuttersRoi.YMin);
	ASSERT_EQ((getImage()->shutters.right  - 1)	, Osc_ShuttersRoi.XMax);
	ASSERT_EQ((getImage()->shutters.bottom - 1)	, Osc_ShuttersRoi.YMax);
}

TEST_F(CrmAlgoAngioValidAngioTest, GivenAValidImage_WhenProcessed_ThenCorrectMMPerPixelIsCalculated)
{
	ASSERT_TRUE(crmProcess(getImage()));

	float expectedMMPerPixel = static_cast<float>(getImage()->mmPerPixel.x * (getImage()->geometry.sourceObjectDistance / getImage()->geometry.sourceImageDistance));

	ASSERT_EQ(expectedMMPerPixel					, Osc_MmPerPixel);
	ASSERT_EQ(static_cast<float>(getImage()->geometry.rotation)	, Osc_Rotation	);
	ASSERT_EQ(static_cast<float>(getImage()->geometry.angulation) , Osc_Angulation);
}

TEST_F(CrmAlgoAngioValidAngioTest, GivenAValidImage_WhenRunEnded_ThenFrameAtMiddleOfCardiacCycleIsUsedForThumbnail)
{
	Osc_HeartCycleStart = 2;
	Osc_HeartCycleEnd   = 4;

	int expectedFrameIndexRequested = (Osc_HeartCycleStart + Osc_HeartCycleEnd) / 2;
	
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_EQ(CrmErrors::Error::OK , crmEnd(getRoadmap()));
	ASSERT_EQ(expectedFrameIndexRequested , OscAngioSeqGetOverlayAssociatedToAngioFrame_indexRequested);
}

}}

//TICS +OLC#009  magic numbers for a test are ok.