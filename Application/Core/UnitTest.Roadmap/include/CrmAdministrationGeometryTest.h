// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <gtest/gtest.h>

#include <CrmAdministration.h>
#include <CrmRepository.h>

#include "TestHelper.h"
#include "UnitTestLog.h"
#include "log.h"

//TICS -OLC#009  magic numbers for a geo test are oke.
namespace Pci{ namespace Test{
using namespace Pci::Core;

// This class implements tests the behavior of the CrmAdministration class when XrayGeometry changes class.
class CrmAdministrationGeometryTest : public ::testing::Test
{
public:
 
    enum class AdminRoadmapZoneStatus
    {
        NoResponse,
        WithinRoadmapRange,
        OutOfRoadmapRange,
        AtRoadmapPosition
    };

	const static int NoRunSelected = -1;
	const static int defaultTimeOut = 1000;
	const double NoAngleSet = 9999.0;

	HANDLE                      eventAdminStatusChanged;

    CrmAdministrationGeometryTest()
    :   log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		crmParameters               (new CrmParameters(L"crm.ini")),
        crmRepository               (new CrmRepository(log, L"")),
        crmAdmin                    (new CrmAdministration(log, *crmParameters, *crmRepository)),
        lastChangedRoadmapStatus    (CrmAdministration::Status::NoRoadmap),
        validRoadmapAvailable       (false),
        lastChangedRoadmapRunNumber (NoRunSelected),
        runNumberCount              (0),
        lastChangedApcRunNumber     (NoRunSelected),
        eventAdminStatusChanged     (CreateEvent(nullptr, TRUE, FALSE, L"RoadmapStatusChanged")),
        roadmapForOverlayAngulation (NoAngleSet),
        roadmapForOverlayRotation   (NoAngleSet)
    {
        crmParameters->storeRoadmapFolder = L"";
    }

    ~CrmAdministrationGeometryTest()
    {
        CloseHandle(eventAdminStatusChanged); eventAdminStatusChanged = nullptr;

		// All Memory must be disposed here so the memory leak detector in the test will not report false positive results. 
		crmAdmin.reset();
		crmRepository.reset();
		CrmRepository::deletePersistentData(log, crmParameters->storeRoadmapFolder);
		crmParameters.reset();
		Simplicity::system().resetHeap();
	}

    void SetUp() override
    {
      ResetEvent(eventAdminStatusChanged);

      crmAdmin->onRoadmapStatusChanged = [this] 
        { 
            CrmAdministration::Status status = crmAdmin->getStatus();
            const auto &roadmap              = crmAdmin->getRoadmap();

            UNITTEST_LOG( L"onRoadmapStatusChanged [Status=" << (int)status << 
                          L", Roadmap=" << (roadmap ? roadmap->runIndex : -1));

            if ( roadmap )
            {
                lastChangedRoadmapRunNumber = roadmap->runIndex;
                lastChangedRoadmapStatus    = status; 
                validRoadmapAvailable       = true;
                roadmapForOverlayAngulation = roadmap->geometry.angulation;
                roadmapForOverlayRotation   = roadmap->geometry.rotation;
            }
            else
            {
				lastChangedRoadmapRunNumber = NoRunSelected;
                lastChangedRoadmapStatus    = CrmAdministration::Status::NoRoadmap;
                validRoadmapAvailable       = false;
                roadmapForOverlayAngulation = NoAngleSet;
                roadmapForOverlayRotation   = NoAngleSet;
            }

            if ( status ==  CrmAdministration::Status::MoveArc && roadmap )
            {
                lastChangedApcRunNumber = roadmap->runIndex;
            }
            else
            {
                lastChangedApcRunNumber = NoRunSelected;
            }

            SetEvent(eventAdminStatusChanged);
        };
    }

	CommonPlatform::Xray::XrayGeometry createXrayGeometry(double angulation, double rotation)
    {
		CommonPlatform::Xray::XrayGeometry geometry;
        geometry.angulation = angulation;
        geometry.rotation   = rotation;
        geometry.updateSpaces();
        return geometry;
    }

	CommonPlatform::Xray::XrayGeometry createXrayGeometry(double angulation, double rotation, double tableX, double tableY)
    {
		CommonPlatform::Xray::XrayGeometry geometry;
        geometry.angulation              = angulation;
        geometry.rotation                = rotation;
        geometry.tableLateralOffset      = tableX;
        geometry.tableLongitudinalOffset = tableY;
        geometry.updateSpaces();
        return geometry;
    }

    int AddRoadmap(CommonPlatform::Xray::XrayGeometry xrayGeometry)
    {
        const double pixelSize = 0.1;
		crmAdmin->addRoadmap(std::make_shared<CrmRoadmap>(++runNumberCount, runNumberCount,1024,1024, pixelSize, xrayGeometry, Sense::Matrix(), 0.0));
        return runNumberCount;
    }

	CommonPlatform::Xray::XrayStudy createStudy(const std::wstring& id, const std::wstring& name) const
    {
		CommonPlatform::Xray::XrayStudy study;
        study.studyId       = id;
        study.patientName   = name;
        return study;
    }

	void setCrmGeometry(const XrayGeometry& geometry) { crmAdmin->setGeometry(geometry);};
	void endCrmGeometry() { crmAdmin->endGeometry(); };
	bool isCrmAdminEmpty()const { return crmAdmin->isEmpty(); };
	void setCrmAdminStudy(XrayStudy& study) { crmAdmin->setStudy(study); }
	int getLastChangedRoadmapRunNumber() const{ return lastChangedRoadmapRunNumber; } ;
	int getLastChangedApcRunNumber()const{ return lastChangedApcRunNumber; };
	CrmAdministration::Status getlastChangedRoadmapStatus() const{	return lastChangedRoadmapStatus;};
	double getRoadmapForOverlayAngulation() const{ return roadmapForOverlayAngulation;};
	double getRoadmapForOverlayRotation() const{	return roadmapForOverlayRotation;};

	void selectRoadmap(int runIndex)
	{
		auto roadmaps = crmAdmin->getRoadmapsForCurrentGeo();

		for (auto& roadmap : roadmaps)
		{
			if (roadmap->runIndex == runIndex)
			{
				crmAdmin->selectRoadmap(roadmap);
				break;
			}
		}
	}

	void lockRoadmap(int runIndex)
	{
		auto roadmaps = crmAdmin->getRoadmapsForCurrentGeo();

		for (auto& roadmap : roadmaps)
		{
			if (roadmap->runIndex == runIndex)
			{
				crmAdmin->lockRoadmap(roadmap);
				break;
			}
		}
	}

	void unlockRoadmap(int runIndex)
	{
		auto roadmaps = crmAdmin->getRoadmapsForCurrentGeo();

		for (auto& roadmap : roadmaps)
		{
			if (roadmap->runIndex == runIndex)
			{
				crmAdmin->unlockRoadmap(roadmap);
				break;
			}
		}
	}

	int getNumberOfRoadmapsForCurrentPosition()
	{
		return static_cast<int>(crmAdmin->getRoadmapsForCurrentGeo().size());
	}

	private:
		CommonPlatform::Log						&log;
	    std::unique_ptr<CrmParameters>			crmParameters;
		std::unique_ptr<CrmRepository>			crmRepository;
		std::unique_ptr<CrmAdministration>		crmAdmin;
		CrmAdministration::Status				lastChangedRoadmapStatus;
		bool									validRoadmapAvailable;
		int										lastChangedRoadmapRunNumber;
		int										runNumberCount;
		int										lastChangedApcRunNumber;
		double									roadmapForOverlayAngulation;
		double									roadmapForOverlayRotation;
};

TEST_F(CrmAdministrationGeometryTest, When_Geometry_Moved_Towards_AngioPosition_Then_Corresponding_RoadmapForOverlay_Is_Selected)
{
    auto xrayGeometry1 = createXrayGeometry(10,20);
    auto xrayGeometry2 = createXrayGeometry(10,30);

    int runNumber1 = AddRoadmap(xrayGeometry1);
    int runNumber2 = AddRoadmap(xrayGeometry2);
	
	ASSERT_FALSE(isCrmAdminEmpty());
    
    setCrmGeometry(xrayGeometry1);
    ASSERT_EQ( runNumber1, getLastChangedRoadmapRunNumber());
    ASSERT_EQ( CrmAdministration::Status::Active, getlastChangedRoadmapStatus() );

    setCrmGeometry(xrayGeometry2);
    ASSERT_EQ( runNumber2, getLastChangedRoadmapRunNumber());
    ASSERT_EQ( CrmAdministration::Status::Active, getlastChangedRoadmapStatus() );

    setCrmGeometry(xrayGeometry1);
    endCrmGeometry();
    ASSERT_EQ( runNumber1,                  getLastChangedRoadmapRunNumber() );
    ASSERT_EQ( xrayGeometry1.angulation,    getRoadmapForOverlayAngulation() );
    ASSERT_EQ( xrayGeometry1.rotation,      getRoadmapForOverlayRotation()   );
    ASSERT_EQ( CrmAdministration::Status::Active, getlastChangedRoadmapStatus() );

    setCrmGeometry(xrayGeometry2);
    endCrmGeometry();
    ASSERT_EQ( runNumber2,                  getLastChangedRoadmapRunNumber() );
    ASSERT_EQ( xrayGeometry2.angulation,    getRoadmapForOverlayAngulation() );
    ASSERT_EQ( xrayGeometry2.rotation,      getRoadmapForOverlayRotation()   );
    ASSERT_EQ( CrmAdministration::Status::Active, getlastChangedRoadmapStatus() );
}

TEST_F(CrmAdministrationGeometryTest, When_Geometry_Is_Close_To_AngioPosition_Then_Corresponding_RoadmapForApc_Is_Selected)
{
    auto angioPositionA = createXrayGeometry(10,20);
    auto livePositionA  = createXrayGeometry(12,20); // In APC range of angioPositionA
    auto angioPositionB = createXrayGeometry(10,30);
    auto livePositionB  = createXrayGeometry(10,28); // In APC range of angioPositionB
    
    int runNumberA = AddRoadmap(angioPositionA);
    int runNumberB = AddRoadmap(angioPositionB);

    ASSERT_FALSE ( isCrmAdminEmpty()                       );
    ASSERT_EQ    ( NoRunSelected,   getLastChangedApcRunNumber() );
    ASSERT_EQ    ( NoAngleSet,      getRoadmapForOverlayRotation()   );
    ASSERT_EQ    ( NoAngleSet,      getRoadmapForOverlayAngulation() );
    ASSERT_EQ    ( CrmAdministration::Status::NoRoadmap, getlastChangedRoadmapStatus() );

    // Move towards angio position A
    ResetEvent(eventAdminStatusChanged);
    setCrmGeometry(livePositionA);
    endCrmGeometry();

    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( runNumberA,                   getLastChangedApcRunNumber() );
    ASSERT_EQ   ( angioPositionA.rotation,      getRoadmapForOverlayRotation()   );
    ASSERT_EQ   ( angioPositionA.angulation,    getRoadmapForOverlayAngulation() );
    ASSERT_EQ    ( CrmAdministration::Status::MoveArc, getlastChangedRoadmapStatus() );

    // Move towards angio position B
    ResetEvent(eventAdminStatusChanged);
    setCrmGeometry(livePositionB);
    endCrmGeometry();

    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( runNumberB,                   getLastChangedApcRunNumber() );
    ASSERT_EQ   ( angioPositionB.rotation,      getRoadmapForOverlayRotation()   );
    ASSERT_EQ   ( angioPositionB.angulation,    getRoadmapForOverlayAngulation() );
    ASSERT_EQ    ( CrmAdministration::Status::MoveArc, getlastChangedRoadmapStatus() );
}

TEST_F(CrmAdministrationGeometryTest, When_Geometry_Moved_Away_From_AngioPosition_Then_RoadmapForApc_Is_Not_Selected)
{
    auto angioPositionA = createXrayGeometry(10,20);
    auto livePositionA  = createXrayGeometry(11,18); // In APC range (>2 deg) of angio position A
    auto livePositionB  = createXrayGeometry(10,32); // more than 10 degrees away from angio position A

    int runNumberA = AddRoadmap(angioPositionA);

    ASSERT_FALSE ( isCrmAdminEmpty() );
    ASSERT_EQ    ( NoRunSelected,           getLastChangedApcRunNumber() );
    ASSERT_EQ    ( CrmAdministration::Status::NoRoadmap, getlastChangedRoadmapStatus() );

    // Move towards angio position A
    ResetEvent(eventAdminStatusChanged);
    setCrmGeometry(livePositionA);
    endCrmGeometry();

    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( runNumberA,                           getLastChangedApcRunNumber()     );
    ASSERT_EQ   ( angioPositionA.rotation,              getRoadmapForOverlayRotation()   );
    ASSERT_EQ   ( angioPositionA.angulation,            getRoadmapForOverlayAngulation() );
    ASSERT_EQ   ( CrmAdministration::Status::MoveArc,   getlastChangedRoadmapStatus()    );

    // Move towards angio position B
    ResetEvent(eventAdminStatusChanged);
    setCrmGeometry(livePositionB);
    endCrmGeometry();

    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( NoRunSelected,                        getLastChangedApcRunNumber()     );
    ASSERT_EQ   ( NoAngleSet,                           getRoadmapForOverlayRotation()   );
    ASSERT_EQ   ( NoAngleSet,                           getRoadmapForOverlayAngulation() );
    ASSERT_EQ   ( CrmAdministration::Status::NoRoadmap, getlastChangedRoadmapStatus()    );
}

TEST_F(CrmAdministrationGeometryTest, When_Geometry_Reaches_AngioPosition_Then_RoadmapForApc_Is_Not_Selected)
{
    auto angioPositionA  = createXrayGeometry( 10.0, 20.0 );
    auto livePositionA   = createXrayGeometry( 12.0, 20.0); // In APC range of angioPositionA
    auto farAwayPosition = createXrayGeometry(-20.0,  0.0); // Out of  APC range of angioPositionA

    setCrmGeometry(farAwayPosition); // this prevents and admin status change
    ResetEvent(eventAdminStatusChanged);
    int runNumberA = AddRoadmap(angioPositionA);

    ASSERT_FALSE ( isCrmAdminEmpty()                                               );
    ASSERT_FALSE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ    ( CrmAdministration::Status::NoRoadmap, getlastChangedRoadmapStatus()   );

    // Move close towards roadmap position A
    ResetEvent(eventAdminStatusChanged);
    setCrmGeometry(livePositionA);
    endCrmGeometry();

    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( runNumberA,                           getLastChangedApcRunNumber()     );
    ASSERT_EQ   ( angioPositionA.rotation,              getRoadmapForOverlayRotation()   );
    ASSERT_EQ   ( angioPositionA.angulation,            getRoadmapForOverlayAngulation() );
    ASSERT_EQ   ( CrmAdministration::Status::MoveArc,   getlastChangedRoadmapStatus()    );

    // Move towards exact roadmap position A, APC can be disabled.
    ResetEvent(eventAdminStatusChanged);
    setCrmGeometry(angioPositionA);
    endCrmGeometry();

    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( runNumberA,                           getLastChangedRoadmapRunNumber() );
    ASSERT_EQ   ( angioPositionA.angulation,            getRoadmapForOverlayAngulation() );
    ASSERT_EQ   ( angioPositionA.rotation,              getRoadmapForOverlayRotation()   );
    ASSERT_EQ   ( NoRunSelected,                        getLastChangedApcRunNumber()     );
    ASSERT_EQ   ( CrmAdministration::Status::Active,    getlastChangedRoadmapStatus()    );
}

TEST_F(CrmAdministrationGeometryTest, When_Geometry_Is_Moving_Then_Correct_RoadmapForOverlay_Is_Selected)
{
    auto angioPositionA = createXrayGeometry( -10.0, 20.0);
    auto livePositionA1 = createXrayGeometry( -11.0, 20.0); // In Roadmap range of angioPositionA
    auto livePositionA2 = createXrayGeometry( -14.0, 20.0); // In APC range of angioPositionA
    auto angioPositionB = createXrayGeometry( -10.0, 30.0 );
    auto livePositionB  = createXrayGeometry( -10.0, 27.0); // In APC range of angioPositionB

    ResetEvent(eventAdminStatusChanged);

    int runNumberA = AddRoadmap(angioPositionA);
    int runNumberB = AddRoadmap(angioPositionB);

    ASSERT_FALSE ( isCrmAdminEmpty()                                                    );
    ASSERT_FALSE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ    ( NoRunSelected,                        getLastChangedRoadmapRunNumber() );
    ASSERT_EQ    ( NoRunSelected,                        getLastChangedApcRunNumber()     );
    
    ResetEvent(eventAdminStatusChanged);
    // Move towards roadmap position A, close enough for overlay
    setCrmGeometry(livePositionA1);
    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( CrmAdministration::Status::Active,    getlastChangedRoadmapStatus()    );
    ASSERT_EQ   ( runNumberA,                           getLastChangedRoadmapRunNumber() );
    ASSERT_EQ   ( NoRunSelected,                        getLastChangedApcRunNumber()     );

    ResetEvent(eventAdminStatusChanged);
    // Move towards roadmap position A, too far for roadmapping but close enough for APC
    setCrmGeometry(livePositionA2);
    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( CrmAdministration::Status::MoveArc,    getlastChangedRoadmapStatus()    );
    ASSERT_EQ   ( runNumberA,                            getLastChangedRoadmapRunNumber() ); // run A is still the best matching roadmap.
    ASSERT_EQ   ( runNumberA,                            getLastChangedApcRunNumber()     );

    ResetEvent(eventAdminStatusChanged);
    // Move towards roadmap position B
    setCrmGeometry(livePositionB);
    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( CrmAdministration::Status::MoveArc,   getlastChangedRoadmapStatus()    );
    ASSERT_EQ   ( runNumberB,                           getLastChangedRoadmapRunNumber() );
    ASSERT_EQ   ( runNumberB,                           getLastChangedApcRunNumber()     );

    ResetEvent(eventAdminStatusChanged);
    // Move towards roadmap position A and stop, exact position is not in AC range.
    setCrmGeometry(livePositionA1);
    endCrmGeometry();
    ASSERT_TRUE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ   ( CrmAdministration::Status::Active,    getlastChangedRoadmapStatus()    );
    ASSERT_EQ   ( runNumberA,                           getLastChangedRoadmapRunNumber() );
    ASSERT_EQ   ( NoRunSelected,                        getLastChangedApcRunNumber()     );
}

TEST_F(CrmAdministrationGeometryTest, When_CArm_Is_In_Range_Of_Multiple_Roadmaps_Then_Most_Recent_Roadmap_Is_Selected)
{
    //       |<----------A range-------------->|
    //                           |<----------B range-------------->|
    // ------|---------A-------------A1---B1-----------B-----------|-----

    // Note: Range = +/- 2 deg

    auto angioPositionA = createXrayGeometry( 0.0, 20.0);
    auto livePositionA1 = createXrayGeometry( 1.4, 20.0); // In APC range of angioPositionA
    auto angioPositionB = createXrayGeometry( 3.0, 20.0);
    auto livePositionB1 = createXrayGeometry( 1.6, 20.0); // In APC range of angioPositionB

	CommonPlatform::Xray::XrayStudy study1; study1.studyId = L"1"; study1.patientName = L"Dummy1";
	CommonPlatform::Xray::XrayStudy study2; study2.studyId = L"2"; study2.patientName = L"Dummy2";

    setCrmAdminStudy(study1);

    int runNumberA = AddRoadmap(angioPositionA);
    int runNumberB = AddRoadmap(angioPositionB);
    ASSERT_NE( runNumberA, runNumberB);

    // Move towards roadmap position B1 in range of both A and B, but B is most recent run
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    setCrmAdminStudy(study2); // new study, same positions
    runNumberA = AddRoadmap(angioPositionA);
    runNumberB = AddRoadmap(angioPositionB);

    // Move towards roadmap position A1, in range of both A and B, but B is most recent run
    setCrmGeometry(livePositionA1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    setCrmAdminStudy(study1); // new study, same positions in different order
    runNumberB = AddRoadmap(angioPositionB);
    runNumberA = AddRoadmap(angioPositionA);

    // Move towards roadmap position B1 in range of both A and B, but A is most recent run
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberA, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, When_CArm_Is_In_Range_Of_Multiple_Roadmaps_Then_Selected_Roadmap_Remains_The_Same_Until_CArm_Moves_OutOfRange_Of_Current_Roadmap)
{
    // comment expected results, e.g. A1 can have different results, coming from A or from B.

    //       |<----------A range-------------->|
    //                           |<----------B range-------------->|
    // ------|---------A------A2-----A1---B1------B2---B-----------|-----
    //
    // Note: Range = +/- 2 deg

    auto angioPositionA = createXrayGeometry( 0.0, 20.0);
    auto livePositionA1 = createXrayGeometry( 1.4, 20.0); // In APC range of angioPositionA
    auto livePositionA2 = createXrayGeometry( 0.9, 20.0); // In APC range of angioPositionA
    auto angioPositionB = createXrayGeometry( 3.0, 20.0);
    auto livePositionB1 = createXrayGeometry( 1.6, 20.0); // In APC range of angioPositionB
    auto livePositionB2 = createXrayGeometry( 2.1, 20.0); // In APC range of angioPositionB

    int runNumberA = AddRoadmap(angioPositionA);
    int runNumberB = AddRoadmap(angioPositionB);
    ASSERT_NE( runNumberA, runNumberB);

    // Move towards roadmap position B2, in range B, out of range A
    setCrmGeometry(livePositionB2);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position B1, in range B, in of range A
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position A1, in range B, in range A
    setCrmGeometry(livePositionA1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position A2, out of range B, in range A
    setCrmGeometry(livePositionA2);
    endCrmGeometry();
    ASSERT_EQ( runNumberA, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position A1, in range B, but still in range A
    setCrmGeometry(livePositionA1);
    endCrmGeometry();
    ASSERT_EQ( runNumberA, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position B1, in range B, but still in range A
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberA, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position B2, in range B, out of range A
    setCrmGeometry(livePositionB2);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Table_Is_In_Range_Of_Multiple_Roadmaps_When_No_Selection_Made_By_User_Then_Most_Recent_Roadmap_Is_Selected)
{
    //       |<----------A range-------------->|
    //                           |<----------B range-------------->|
    // ------|---------A-------------A1---B1-----------B-----------|-----
    //
    // Note: Range = +/- 30 mm
    //       We need different C-arm positions larger than minimum distance from each other

    //                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
    auto angioPositionA = createXrayGeometry( 0.0, 0,           0, 600 );
    auto livePositionA1 = createXrayGeometry( 1.5, 0,           0, 620 ); // In APC range of angio Position A
    auto angioPositionB = createXrayGeometry( 3.0, 0,           0, 640 );
    auto livePositionB1 = createXrayGeometry( 1.5, 0,           0, 625 ); // In APC range of angio Position B

    auto study1 = createStudy(L"1", L"Dummy1");
    auto study2 = createStudy(L"2", L"Dummy2");

    setCrmAdminStudy(study1);

    int runNumberA = AddRoadmap(angioPositionA);
    int runNumberB = AddRoadmap(angioPositionB);
    ASSERT_NE( runNumberA, runNumberB);

    // Move towards roadmap position A1, in range of both A and B, but B is most recent
    setCrmGeometry(livePositionA1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    setCrmAdminStudy(study2); // new study, same positions
    runNumberA = AddRoadmap(angioPositionA);
    runNumberB = AddRoadmap(angioPositionB);
    ASSERT_NE( runNumberA, runNumberB);

    // Move towards roadmap position B1 in range of both A and B, but B is most recent
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    setCrmAdminStudy(study1); // new study, same positions but different order
    runNumberB = AddRoadmap(angioPositionB);
    runNumberA = AddRoadmap(angioPositionA);
    ASSERT_NE( runNumberA, runNumberB);

    // Move towards roadmap position B1 in range of both A and B, but A is most recent
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberA, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Table_Is_In_Range_Of_Multiple_Roadmaps_When_Selection_Made_By_User_Then_User_Selected_Roadmap_Is_Used)
{
	//       |<----------A range-------------->|
	//                           |<----------B range-------------->|
	// ------|---------A-------------A1---B1-----------B-----------|-----
	//
	// Note: Range = +/- 30 mm
	//       We need different C-arm positions larger than minimum distance from each other

	//                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
	auto angioPositionA = createXrayGeometry(0.0, 0, 0, 600);
	auto livePositionA1 = createXrayGeometry(1.5, 0, 0, 620); // In APC range of angio Position A
	auto angioPositionB = createXrayGeometry(3.0, 0, 0, 640);
	auto livePositionB1 = createXrayGeometry(1.5, 0, 0, 625); // In APC range of angio Position B

	auto study1 = createStudy(L"1", L"Dummy1");
	auto study2 = createStudy(L"2", L"Dummy2");

	setCrmAdminStudy(study1);

	int runNumberA = AddRoadmap(angioPositionA);
	int runNumberB = AddRoadmap(angioPositionB);
	ASSERT_NE(runNumberA, runNumberB);

	// Move towards roadmap position A1, in range of both A and B, but B is most recent
	setCrmGeometry(livePositionA1);
	endCrmGeometry();
	ASSERT_EQ(runNumberB, getLastChangedRoadmapRunNumber());

	setCrmAdminStudy(study2); // new study, same positions
	runNumberA = AddRoadmap(angioPositionA);
	runNumberB = AddRoadmap(angioPositionB);
	ASSERT_NE(runNumberA, runNumberB);

	selectRoadmap(runNumberA);

	// Move towards roadmap position B1 in range of both A and B, but B is most recent
	setCrmGeometry(livePositionB1);
	endCrmGeometry();
	ASSERT_EQ(runNumberA, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Table_Is_In_Range_Of_Multiple_Roadmaps_When_Locked_By_User_Then_User_Locked_Roadmap_Is_Used)
{
	//       |<----------A range-------------->|
	//                           |<----------B range-------------->|
	// ------|---------A-------------A1---B1-----------B-----------|-----
	//
	// Note: Range = +/- 30 mm
	//       We need different C-arm positions larger than minimum distance from each other

	//                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
	auto angioPositionA = createXrayGeometry(0.0, 0, 0, 600);
	auto livePositionA1 = createXrayGeometry(1.5, 0, 0, 620); // In APC range of angio Position A
	auto angioPositionB = createXrayGeometry(3.0, 0, 0, 640);
	auto livePositionB1 = createXrayGeometry(1.5, 0, 0, 625); // In APC range of angio Position B

	auto study1 = createStudy(L"1", L"Dummy1");
	auto study2 = createStudy(L"2", L"Dummy2");

	setCrmAdminStudy(study1);

	int runNumberA = AddRoadmap(angioPositionA);
	int runNumberB = AddRoadmap(angioPositionB);
	ASSERT_NE(runNumberA, runNumberB);

	// Move towards roadmap position A1, in range of both A and B, but B is most recent
	setCrmGeometry(livePositionA1);
	endCrmGeometry();
	ASSERT_EQ(runNumberB, getLastChangedRoadmapRunNumber());

	setCrmAdminStudy(study2); // new study, same positions
	runNumberA = AddRoadmap(angioPositionA);
	runNumberB = AddRoadmap(angioPositionB);
	ASSERT_NE(runNumberA, runNumberB);

	lockRoadmap(runNumberA);

	// Move towards roadmap position B1 in range of both A and B, but B is most recent
	setCrmGeometry(livePositionB1);
	endCrmGeometry();
	ASSERT_EQ(runNumberA, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Table_Is_In_Range_Of_Multiple_Roadmaps_When_Multiple_Locked_Roadmap_Then_Last_Locked_Roadmap_Is_Used)
{
	//       |<----------A range-------------->|
	//                           |<----------B range-------------->|
	// ------|---------A-----A1-----------B1-----------B-----------|-----
	//
	// Note: Range = +/- 30 mm
	//       We need different C-arm positions larger than minimum distance from each other

	//                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
	auto angioPositionA = createXrayGeometry(0.0, 0, 0, 600);
	auto livePositionA1 = createXrayGeometry(0.9, 0, 0, 620); // In APC range of angio Position A
	auto angioPositionB = createXrayGeometry(3.0, 0, 0, 640);
	auto livePositionB1 = createXrayGeometry(1.5, 0, 0, 625); // In APC range of angio Position B

	auto study1 = createStudy(L"1", L"Dummy1");

	setCrmAdminStudy(study1);

	setCrmGeometry(angioPositionA);
	endCrmGeometry();

	int runNumberA1 = AddRoadmap(angioPositionA);
	int runNumberA2 = AddRoadmap(angioPositionA);

	lockRoadmap(runNumberA1);

	setCrmGeometry(angioPositionB);
	endCrmGeometry();

	int runNumberB1 = AddRoadmap(angioPositionB);
	int runNumberB2 = AddRoadmap(angioPositionB);

	lockRoadmap(runNumberB2);

	ASSERT_NE(runNumberA1, runNumberA2);
	ASSERT_NE(runNumberA2, runNumberB1);
	ASSERT_NE(runNumberB1, runNumberB2);

	setCrmGeometry(livePositionB1);
	endCrmGeometry();
	ASSERT_EQ(runNumberB2, getLastChangedRoadmapRunNumber());

	setCrmGeometry(livePositionA1);
	endCrmGeometry();
	ASSERT_EQ(runNumberA1, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Table_Is_In_Range_Of_Multiple_Roadmaps_When_One_Locked_Roadmap_And_One_Selected_Then_Locked_Roadmap_Is_Used)
{
	//       |<----------A range-------------->|
	//                           |<----------B range-------------->|
	// ------|---------A-----A1-----------B1-----------B-----------|-----
	//
	// Note: Range = +/- 30 mm
	//       We need different C-arm positions larger than minimum distance from each other

	//                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
	auto angioPositionA = createXrayGeometry(0.0, 0, 0, 600);
	auto livePositionA1 = createXrayGeometry(0.9, 0, 0, 620); // In APC range of angio Position A
	auto angioPositionB = createXrayGeometry(3.0, 0, 0, 640);
	auto livePositionB1 = createXrayGeometry(1.5, 0, 0, 625); // In APC range of angio Position B

	auto study1 = createStudy(L"1", L"Dummy1");

	setCrmAdminStudy(study1);

	setCrmGeometry(angioPositionA);
	endCrmGeometry();

	int runNumberA1 = AddRoadmap(angioPositionA);
	int runNumberA2 = AddRoadmap(angioPositionA);

	selectRoadmap(runNumberA1);

	setCrmGeometry(angioPositionB);
	endCrmGeometry();

	int runNumberB1 = AddRoadmap(angioPositionB);
	int runNumberB2 = AddRoadmap(angioPositionB);

	lockRoadmap(runNumberB2);

	ASSERT_NE(runNumberA1, runNumberA2);
	ASSERT_NE(runNumberA2, runNumberB1);
	ASSERT_NE(runNumberB1, runNumberB2);

	setCrmGeometry(livePositionB1);
	endCrmGeometry();
	ASSERT_EQ(runNumberB2, getLastChangedRoadmapRunNumber());

	setCrmGeometry(livePositionA1);
	endCrmGeometry();
	ASSERT_EQ(runNumberA1, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Roadmap_Selection_Made_By_User_When_New_Roadmap_Created_Then_New_Roadmap_Is_Used)
{
	auto angioPosition = createXrayGeometry(0.0, 0, 0, 600);

	auto study = createStudy(L"1", L"Dummy1");

	setCrmAdminStudy(study);

	int runNumberA = AddRoadmap(angioPosition);
	int runNumberB = AddRoadmap(angioPosition);
	ASSERT_NE(runNumberA, runNumberB);

	// Move towards roadmap position A1, in range of both A and B, but B is most recent
	setCrmGeometry(angioPosition);
	endCrmGeometry();
	ASSERT_EQ(runNumberB, getLastChangedRoadmapRunNumber());

	selectRoadmap(runNumberA);

	ASSERT_EQ(runNumberA, getLastChangedRoadmapRunNumber());

	int runNumberC = AddRoadmap(angioPosition);

	ASSERT_EQ(runNumberC, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Roadmap_Locked_By_User_When_New_Roadmap_Created_Then_Locked_Roadmap_Is_Used)
{
	auto angioPosition = createXrayGeometry(0.0, 0, 0, 600);

	auto study = createStudy(L"1", L"Dummy1");

	setCrmAdminStudy(study);

	int runNumberA = AddRoadmap(angioPosition);
	int runNumberB = AddRoadmap(angioPosition);
	ASSERT_NE(runNumberA, runNumberB);

	// Move towards roadmap position A1, in range of both A and B, but B is most recent
	setCrmGeometry(angioPosition);
	endCrmGeometry();
	ASSERT_EQ(runNumberB, getLastChangedRoadmapRunNumber());

	lockRoadmap(runNumberA);

	ASSERT_EQ(runNumberA, getLastChangedRoadmapRunNumber());

	int runNumberC = AddRoadmap(angioPosition);

	ASSERT_NE(runNumberA, runNumberC);
	ASSERT_EQ(runNumberA, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, When_Multiple_Roadmaps_Created_For_Same_Geo_Then_All_Of_Them_Are_Stored)
{

	//                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
	auto angioPosition = createXrayGeometry(0.0, 0, 0, 600);

	auto study1 = createStudy(L"1", L"Dummy1");

	setCrmAdminStudy(study1);

	int runNumberA = AddRoadmap(angioPosition);
	int runNumberB = AddRoadmap(angioPosition);
	ASSERT_NE(runNumberA, runNumberB);

	setCrmGeometry(angioPosition);
	endCrmGeometry();
	ASSERT_EQ(2, getNumberOfRoadmapsForCurrentPosition());
}

TEST_F(CrmAdministrationGeometryTest, When_Table_Moves_Along_Ranges_Of_Multiple_Roadmaps_Then_Selected_Roadmap_Remains_The_Most_Recent_Roadmap_Is_Selected)
{
    //       |<----------A range-------------->|
    //                           |<----------B range-------------->|
    // ------|---------A------A1-----A2---B1------B2---B-----------|-----
    //
    // Note: Range = +/- 200 mm
    //       We need different C-arm positions larger than minimum distance from each other

    //                                        C-Arm (Ang,Rot)   Table pos (Lat,Long)
    auto angioPositionA = createXrayGeometry( 0.0, 0,           0, 600 ); 
    auto livePositionA1 = createXrayGeometry( 1.5, 0,           0, 620 ); 
    auto livePositionA2 = createXrayGeometry( 1.5, 0,           0, 750 ); 
    auto angioPositionB = createXrayGeometry( 3.0, 0,           0, 900 ); 
    auto livePositionB1 = createXrayGeometry( 1.5, 0,           0, 780 ); 
    auto livePositionB2 = createXrayGeometry( 1.5, 0,           0, 800 ); 

    int runNumberA = AddRoadmap(angioPositionA);
    int runNumberB = AddRoadmap(angioPositionB);
    ASSERT_NE( runNumberA, runNumberB);

    // Move towards roadmap position B2, in range B, out of range A
    setCrmGeometry(livePositionB2);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position B1, in range B, in of range A
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position A2, in range B, in range A, but B is most recent
    setCrmGeometry(livePositionA2);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position A1, out of range B, in range A
    setCrmGeometry(livePositionA1);
    endCrmGeometry();
    ASSERT_EQ( runNumberA, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position A2, in range B, in range A, but B is most recent
    setCrmGeometry(livePositionA2);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position B1, in range B, out of range A
    setCrmGeometry(livePositionB1);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());

    // Move towards roadmap position B2, in range B, out of range A
    setCrmGeometry(livePositionB2);
    endCrmGeometry();
    ASSERT_EQ( runNumberB, getLastChangedRoadmapRunNumber());
}

TEST_F(CrmAdministrationGeometryTest, Given_Geometry_Has_Been_Move_Between_EndOfRun_And_AddRoadmap_When_RoadmapIsAdded_Then_No_Status_Update_Is_Received)
{
    // Happens when AddRoadmap() is called (when processing is finished) and the geometry has moved very quickly to another position
    auto angioPositionA   = createXrayGeometry( 10.0, 20.0);
    auto farAwayPositionA = createXrayGeometry(-20.0,  0.0); // Out of  APC range of angioPositionA

    setCrmGeometry(farAwayPositionA);
    ResetEvent(eventAdminStatusChanged);
    AddRoadmap(angioPositionA);

    ASSERT_FALSE ( isCrmAdminEmpty()                                               );
    ASSERT_FALSE ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ    ( CrmAdministration::Status::NoRoadmap, getlastChangedRoadmapStatus()   );
}

TEST_F(CrmAdministrationGeometryTest, Given_Geometry_Has_Been_Move_Between_EndOfRun_And_AddRoadmap_When_RoadmapIsAdded_Then_Status_Becomes_MoveCArc)
{
    auto angioPositionA  = createXrayGeometry ( 10.0, 20.0);
    auto nearPositionA    = createXrayGeometry( 15.0, 20.0); // In  APC range of angioPositionA

    setCrmGeometry(nearPositionA);
    ResetEvent(eventAdminStatusChanged);
    AddRoadmap(angioPositionA);

    ASSERT_FALSE ( isCrmAdminEmpty()                                               );
    ASSERT_TRUE  ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ    ( CrmAdministration::Status::MoveArc, getlastChangedRoadmapStatus()   );
}

TEST_F(CrmAdministrationGeometryTest, Given_Geometry_Has_Not_Moved_Between_EndOfRun_And_AddRoadmap_When_RoadmapIsAdded_Then_Status_Becomes_Active)
{
    auto angioPositionA  = createXrayGeometry( 10.0, 20.0);

    setCrmGeometry(angioPositionA); // Same position as angio of added roadmap
    ResetEvent(eventAdminStatusChanged);
    AddRoadmap(angioPositionA);

    ASSERT_FALSE ( isCrmAdminEmpty()                                               );
    ASSERT_TRUE  ( TestHelper::waitForEventWithTimeOut(eventAdminStatusChanged, defaultTimeOut));
    ASSERT_EQ    ( CrmAdministration::Status::Active, getlastChangedRoadmapStatus()   );
}


}}
//TICS +OLC#009  magic numbers