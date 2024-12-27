// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoAngioErrorHandlingTest.h
#pragma once
#include <gtest/gtest.h>
#include "CrmAlgoAngio.h"
#include "OscStub.h"

using namespace Pci::Core;
namespace Pci {	namespace Test {

//TICS -OLC#009  magic numbers for a test are ok.
class CrmAlgoAngioErrorHandlingTest : public ::testing::Test
{
public:

	const double moveTableJustOverMagin;
	const double moveCarmJustOverMargin;
	const double dontMove;

	CrmAlgoAngioErrorHandlingTest() :
		moveTableJustOverMagin	(0.6),
		moveCarmJustOverMargin	(0.2),
		dontMove				(0.0),
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_pool					(false,"CrmTest"),
		m_param					(new CrmParameters(L"crm.ini")),// make sure to use same parameters as application
		m_algoAngio             (nullptr),
		m_crmRoadmap            (nullptr),
		m_xrayImage             (nullptr),
		m_xrayGeometry          ()
	{
	}

	~CrmAlgoAngioErrorHandlingTest()
	{
		// All Memory must be disposed here so the memory leak detector in the test will not report false positive results. 
		m_xrayImage.reset();
		m_crmRoadmap.reset();
		m_algoAngio.reset();	
		m_param.reset();
		Simplicity::system().resetHeap();
	}

	void SetUp() override
	{
		m_algoAngio	 = std::make_unique<CrmAlgoAngio>(m_log, m_pool, *m_param);
		m_crmRoadmap = std::make_shared<CrmRoadmap>();
		m_xrayImage	 = std::make_shared<XrayImage>();
		m_xrayImage->geometry = m_xrayGeometry;

		reset();

		Test::SetUp();	
	}
	
	const std::shared_ptr<XrayImage> moveTable(const std::shared_ptr<XrayImage> xrayImage, double offset) 
	{
		std::shared_ptr<XrayImage> xrayImageMoved = std::make_shared<XrayImage>();
		XrayGeometry geometryMoved(xrayImage->geometry);

		geometryMoved.tableLongitudinalOffset = xrayImage->geometry.tableLongitudinalOffset + offset;
		geometryMoved.updateSpaces();

		xrayImageMoved->geometry = geometryMoved;
		return xrayImageMoved;
	}

	const std::shared_ptr<XrayImage> moveCarm(const std::shared_ptr<XrayImage> xrayImage, double offset) 
	{
		std::shared_ptr<XrayImage> xrayImageMoved = std::make_shared<XrayImage>();
		XrayGeometry geometryMoved(xrayImage->geometry);

		geometryMoved.angulation = xrayImage->geometry.angulation + offset;
		geometryMoved.updateSpaces();

		xrayImageMoved->geometry = geometryMoved;
		return xrayImageMoved;
	}

	bool crmProcess(std::shared_ptr<XrayImage> image) { return m_algoAngio->process(image);};
	CrmErrors::Error crmEnd(const std::shared_ptr<CrmRoadmap>& roadmap) { return m_algoAngio->end(roadmap); };
	std::shared_ptr<XrayImage> getImage() { return m_xrayImage; };
	std::shared_ptr<CrmRoadmap> getRoadmap() { return m_crmRoadmap; };


private:
	CommonPlatform::Log				&m_log;
	Simplicity::ThreadPool			m_pool;
	std::unique_ptr<CrmParameters>	m_param;
	std::unique_ptr<CrmAlgoAngio>	m_algoAngio;
	std::shared_ptr<CrmRoadmap>		m_crmRoadmap;
	std::shared_ptr<XrayImage>		m_xrayImage;
	XrayGeometry					m_xrayGeometry;
};


TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenTableHasMovedDuringAngio_AndAlgoReturnsError_ThenPatientTableMovedError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_FALSE(crmProcess(moveTable(getImage(), moveTableJustOverMagin)));

	OscAngioProcessSequence_ReturnValue = OSC_ERR_LAST;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::PatientTableMoved);

}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenTableHasMovedDuringAngio_AndAlgoSucceeds_ThenProcessSucceeds)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_FALSE(crmProcess(moveTable(getImage(), moveTableJustOverMagin)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::OK);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenTableDoesntMove_ThenProcessSucceeds)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveTable(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::OK);

}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenCarcIsMoved_AndAlgoReturnsError_ThenCarmMovedError)
{
    ASSERT_TRUE(crmProcess(getImage()));

    ASSERT_FALSE(crmProcess(moveCarm(getImage(), moveCarmJustOverMargin)));

    OscAngioProcessSequence_ReturnValue = OSC_ERR_NO_CARDIAC_CYCLE;
	ASSERT_EQ(CrmErrors::Error::CarmMoved, crmEnd(getRoadmap()));
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenCarcIsMoved_AndAlgoSucceeds_ThenProcessSucceeds)
{
    ASSERT_TRUE(crmProcess(getImage()));

    ASSERT_FALSE(crmProcess(moveCarm(getImage(), moveCarmJustOverMargin)));

    ASSERT_EQ(CrmErrors::Error::OK, crmEnd(getRoadmap()));
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenCarcDoesntMove_ThenProcessSucceeds)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::OK);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenCarcAndTableIsMoved_AndAlgoReturnsError_ThenCarmAndTableMovedError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_FALSE(crmProcess(moveCarm(moveTable(getImage(), moveTableJustOverMagin), moveCarmJustOverMargin)));

    OscAngioProcessSequence_ReturnValue = OSC_ERR_NO_CARDIAC_CYCLE;
	ASSERT_EQ(CrmErrors::Error::CarmAndTableMoved, crmEnd(getRoadmap()));
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenCarcAndTableIsMoved_AndAlgoSucceeds_ThenProcessSucceeds)
{
    ASSERT_TRUE(crmProcess(getImage()));

    ASSERT_FALSE(crmProcess(moveCarm(moveTable(getImage(), moveTableJustOverMagin), moveCarmJustOverMargin)));

    ASSERT_EQ(CrmErrors::Error::OK, crmEnd(getRoadmap()));
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenAngioToShort_ThenAngioTooShortError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_ERR_SHORT_ANGIOSEQ;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::AngioTooShort);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenNoHeartCycleDetected_ThenNoHeartCycleError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_ERR_NO_CARDIAC_CYCLE;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::NoHeartCycle);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenInjectionTooShort_ThenInjectionTooShortError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::InjectionTooShort);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenNoInjection_ThenNoInjectionError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_WRN_NONINJECTED_ANGIO;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::OK);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenInjectionNotWellDefined_ThenInjectionNotWellDefinedError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_WRN_BADLY_INJECTED_ANGIO;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::OK);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenSettingAngioParamsFails_ThenUnknownError)
{
	OscSetAngioSequenceParameters_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(crmProcess(getImage()));
	ASSERT_FALSE(crmProcess(moveCarm(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::Unknown);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenSettingTableApparentPositionMMFails_ThenUnknownError)
{
	OscSetTableApparentPositionMM_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(crmProcess(getImage()));
	ASSERT_FALSE(crmProcess(moveCarm(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::Unknown);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenOscSetShuttersRoiFails_ThenUnknownError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	OscSetShuttersRoi_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(crmProcess(moveCarm(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::Unknown);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenOscSetCurrentGeometryAngioFails_ThenUnknownError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	OscSetCurrentGeometryAngio_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(crmProcess(moveCarm(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::Unknown);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenOscSetTableApparentPositionPixFails_ThenUnknownError)
{
	ASSERT_TRUE(crmProcess(getImage()));

	OscSetTableApparentPositionPix_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_FALSE(crmProcess(moveCarm(getImage(), dontMove)));

	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::Unknown);
}

TEST_F(CrmAlgoAngioErrorHandlingTest, GivenTwoImagesToProcess_WhenOscSaveToMemoryFails_ThenSaveFailureError)
{
	ASSERT_TRUE(crmProcess(getImage()));
	ASSERT_TRUE(crmProcess(moveCarm(getImage(), dontMove)));

	OscSaveToMemory_ReturnValue = OSC_ERR_INTERNAL;
	ASSERT_TRUE(crmEnd(getRoadmap()) == CrmErrors::Error::SaveFailure);
}

//TICS +OLC#009  magic numbers for a test are ok.

}}