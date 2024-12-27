#pragma once
#include <IpSimplicity.h>
#include <gtest/gtest.h>
#include "CrmAlgoAngio.h"
#include "OscStub.h"

using namespace PciSuite;

class CrmAlgoAngioTest : public ::testing::Test
{
public:

	const double moveTableJustOverMagin;
	const double moveCarmJustOverMargin;
	const double dontMove;

	Simplicity::ThreadPool pool;
	CrmParameters param;
	CrmAlgoAngio* algoAngio;

	std::shared_ptr<CrmRoadmap> crmRoadmap;
	std::shared_ptr<XrayImage> xrayImage;

	XrayGeometry xrayGeometry;

	CrmAlgoAngioTest() :
		pool					(false,"CrmTest"),
		param					(L"crm.ini"),
		moveTableJustOverMagin	(0.6),
		moveCarmJustOverMargin	(0.2),
		dontMove				(0.0)
	{
	}

	void SetUp() override
	{
		algoAngio	= new CrmAlgoAngio(pool, param);
		crmRoadmap	= std::make_shared<CrmRoadmap>();
		xrayImage	= std::make_shared<XrayImage>();
		xrayImage->geometry = xrayGeometry;

		OscAngioProcessSequence_ReturnValue = 0;

		Test::SetUp();	
	}

	void TearDown() override
	{
		delete algoAngio;
		Test::TearDown();
	}

	const std::shared_ptr<XrayImage> moveTable(const std::shared_ptr<XrayImage> xrayImage, double offset) 
	{
		std::shared_ptr<XrayImage> xrayImageMoved = std::make_shared<XrayImage>();
		XrayGeometry geometryMoved;

		geometryMoved.tableLongitudinalOffset = xrayImage->geometry.tableLongitudinalOffset + offset;
		geometryMoved.updateSpaces();

		xrayImageMoved->geometry = geometryMoved;
		return xrayImageMoved;
	}

	const std::shared_ptr<XrayImage> moveCarm(const std::shared_ptr<XrayImage> xrayImage, double offset) 
	{
		std::shared_ptr<XrayImage> xrayImageMoved = std::make_shared<XrayImage>();
		XrayGeometry geometryMoved;

		geometryMoved.angulation = xrayImage->geometry.angulation + offset;
		geometryMoved.updateSpaces();

		xrayImageMoved->geometry = geometryMoved;
		return xrayImageMoved;
	}
};

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenTableHasMovedDuringAngio_AndAlgoReturnsError_ThenPatientTableMovedError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_FALSE(algoAngio->process(moveTable(xrayImage, moveTableJustOverMagin)));

	OscAngioProcessSequence_ReturnValue = OSC_ERR_LAST;
	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::PatientTableMoved);

}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenTableHasMovedDuringAngio_AndAlgoSucceeds_ThenProcessSucceeds)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_FALSE(algoAngio->process(moveTable(xrayImage, moveTableJustOverMagin)));

	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::OK);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenTableDoesntMove_ThenProcessSucceeds)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveTable(xrayImage, dontMove)));

	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::OK);

}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenCarcIsMoved_ThenCarmMovedError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_FALSE(algoAngio->process(moveCarm(xrayImage, moveCarmJustOverMargin)));

	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::CarmMoved);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenCarcDoesnt_ThenProcessSucceeds)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveCarm(xrayImage, dontMove)));

	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::OK);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenAngioToShort_ThenAngioTooShortError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveCarm(xrayImage, dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_ERR_SHORT_ANGIOSEQ;
	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::AngioTooShort);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenNoHeartCycleDetected_ThenNoHeartCycleError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveCarm(xrayImage, dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_ERR_NO_CARDIAC_CYCLE;
	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::NoHeartCycle);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenInjectionTooShort_ThenInjectionTooShortError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveCarm(xrayImage, dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE;
	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::InjectionTooShort);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenNoInjection_ThenNoInjectionError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveCarm(xrayImage, dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_WRN_NONINJECTED_ANGIO;
	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::NoInjection);
}

TEST_F(CrmAlgoAngioTest, GivenTwoImagesToProcess_WhenInjectionNotWellDefined_ThenInjectionNotWellDefinedError)
{
	ASSERT_TRUE(algoAngio->process(xrayImage));

	ASSERT_TRUE(algoAngio->process(moveCarm(xrayImage, dontMove)));

	OscAngioProcessSequence_ReturnValue = OSC_WRN_BADLY_INJECTED_ANGIO;
	ASSERT_TRUE(algoAngio->end(crmRoadmap) == CrmRoadmap::Error::InjectionNotWellDefined);
}

