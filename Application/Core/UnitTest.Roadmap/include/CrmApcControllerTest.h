// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <gtest/gtest.h>

#include "UnitTestLog.h"
#include "TestHelper.h"

#include "XrayGeometryControllerMock.h"
#include "CrmApcController.h"

namespace Pci{ namespace Test{
using namespace CommonPlatform::Xray;

// This class test the implementation of the CrmApcController and it's interaction with the XrayGeometryController
class CrmApcControllerTest : public ::testing::Test
{
public:
    const static int defaultTimeOut = 250;

	CrmApcControllerTest() 
    :   m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_geometryController          (),
        m_apcController               (m_log, m_geometryController, true, L"StudyID", true),
        m_apcControllerStatus         (CrmApcController::ApcModeStatus::AutomaticActivation),
        m_eventApcControllerStatus    (CreateEvent(nullptr, TRUE, FALSE, L"RoadmapZoneStatus"))
    {
	}

    ~CrmApcControllerTest()
	{
	    CloseHandle(m_eventApcControllerStatus); m_eventApcControllerStatus = nullptr;
	}

    void SetUp() override
    {
        Test::SetUp();

        m_apcController.onApcModeStatusChanged = [this] (CrmApcController::ApcModeStatus status)
        {
            UNITTEST_LOG("ApcController status change. Status=" << (int)status );
            m_apcControllerStatus = status;
            SetEvent(m_eventApcControllerStatus);
        };

        m_apcController.setCwisApcConnected(true);
    }

	void TearDown() override
	{
		Simplicity::system().resetHeap();
	}
	

    std::shared_ptr<const CrmRoadmap> createRoadmap(double angulation, double rotation)
    {
        XrayGeometry geometry;
        geometry.angulation = angulation;
        geometry.rotation   = rotation;
        auto roadmap = std::make_shared<CrmRoadmap>(5,5,1024,1024, 0.1, geometry, Sense::Matrix(), 0.0);
       return roadmap;
    }

	XrayGeometryControllerMock& getGeometryController() { return m_geometryController; };
	bool setApc(std::shared_ptr<const CrmRoadmap> roadmap) { return m_apcController.setApc(roadmap); };
	CrmApcController::ApcModeStatus getStatus() const { return m_apcController.getStatus(); }
	void setAutomaticActivation(bool enabled) {	m_apcController.setAutomaticActivation( enabled );};
	bool getAutomaticActivation() const { return m_apcController.getAutomaticActivation(); };
	bool getApcAllowed() const { return m_apcController.getApcAllowed(); };
	void setApcAllowed( bool allowed ) { return m_apcController.setApcAllowed(allowed); };
	void setApcExternalActivity(IXrayGeometryController::ApcExternalCwisActivity conflictingEvent) { m_apcController.setApcExternalActivity(conflictingEvent); };
	void setApcControllerStatus(CrmApcController::ApcModeStatus status) { m_apcControllerStatus = status; };
	CrmApcController::ApcModeStatus getApcControllerStatus()const { return m_apcControllerStatus; };
	HANDLE getEventApcControllerStatus() { return m_eventApcControllerStatus; };


private:
	CommonPlatform::Log&			m_log;
	XrayGeometryControllerMock      m_geometryController;
	CrmApcController                m_apcController;
	CrmApcController::ApcModeStatus m_apcControllerStatus;
	HANDLE                          m_eventApcControllerStatus;

};

// todo: test with changing studyid
// todo: test with 3D-APC tsm button pressed
// todo: test connection status and influence on allowed

TEST_F(CrmApcControllerTest, Given_AutomaticActivation_Is_Enabled_When_Apc_Is_Disabled_Then_ApcControllerStatus_Does_Not_Change)
{
    const double rot = 1.0;
    const double ang = 0.0;
    auto roadmap = createRoadmap(ang,rot);

    setAutomaticActivation(true);

    ASSERT_TRUE  ( setApc(roadmap) );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::AutomaticActivation, getStatus()  );

    ASSERT_TRUE  ( setApc(nullptr) ); // Disable APC
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::AutomaticActivation, getStatus()  );
    ASSERT_EQ    ( rot, getGeometryController().apcRotationAngle         );
    ASSERT_EQ    ( ang, getGeometryController().apcAngulationAngle       );
}

TEST_F(CrmApcControllerTest, Given_AutomaticActivation_Is_Disabled_When_Apc_Is_Disabled_Then_ApcControllerStatus_Does_Not_Change)
{
    const double ang = 20.0;
    const double rot = 10.0;
    auto roadmap = createRoadmap(ang,rot);

    setAutomaticActivation(false);
    ASSERT_EQ   ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getStatus()  );

    setApc(roadmap);
    ASSERT_EQ   ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getStatus()  );
}

TEST_F(CrmApcControllerTest, Given_AutomaticActivation_Is_Enabled_When_Apc_Is_Enabled_Then_ApcControllerStatus_Does_Not_Change)
{
    const double ang = 20.0;
    const double rot = 10.0;
    auto roadmap = createRoadmap(ang,rot);

    setAutomaticActivation(true);
    ASSERT_EQ   ( CrmApcController::ApcModeStatus::AutomaticActivation, getStatus()  );

    setApc(roadmap);
    ASSERT_EQ   ( CrmApcController::ApcModeStatus::AutomaticActivation, getStatus()  );
}

TEST_F(CrmApcControllerTest, Given_AutomaticActivation_Is_Disabled_When_Apc_Is_Enabled_Then_ApcControllerStatus_Does_Not_Change)
{
    const double ang = 20.0;
    const double rot = 10.0;
    auto roadmap = createRoadmap(ang,rot);

    setAutomaticActivation(false);
    ASSERT_EQ   ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getStatus()  );

    setApc(roadmap);
    ASSERT_EQ   ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getStatus()  );
}

TEST_F(CrmApcControllerTest, When_APC_Is_Enabled_Then_GeometryController_Receives_Angles)
{
    const double ang = 20.0;
    const double rot = 10.0;
    auto roadmap = createRoadmap(ang,rot);

    setApc(roadmap);

    ASSERT_EQ    ( rot, getGeometryController().apcRotationAngle         );
    ASSERT_EQ    ( ang, getGeometryController().apcAngulationAngle       );
}

TEST_F(CrmApcControllerTest, When_ProgrammingConflict_Is_Detected_Then_Status_Is_Updated_And_Event_Is_Raised)
{
    setApcControllerStatus( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton); // something else than CrmAppController::CrmAppController::ApcStatus::Disabled
    setApcExternalActivity( IXrayGeometryController::ApcExternalCwisActivity::AngleProgrammingDetected);
    ASSERT_TRUE  ( TestHelper::waitForEventWithTimeOut(getEventApcControllerStatus(), defaultTimeOut )              );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::DisallowBecauseOfProgrammingConflict, getApcControllerStatus()   );
}

TEST_F(CrmApcControllerTest, When_ProgrammingConflict_Is_Detected_Then_APC_Communication_Gets_Blocked)
{
    ASSERT_TRUE ( getApcAllowed() );
    setApcExternalActivity( IXrayGeometryController::ApcExternalCwisActivity::AngleProgrammingDetected);
    ASSERT_FALSE ( getApcAllowed() );
}

TEST_F(CrmApcControllerTest, When_StatusConflict_Is_Detected_Then_Status_Is_Updated_And_Event_Is_Raised)
{
    setApcControllerStatus( CrmApcController::ApcModeStatus::AutomaticActivation);
    setApcExternalActivity( IXrayGeometryController::ApcExternalCwisActivity::StatusChangeDetected);
    ASSERT_TRUE  ( TestHelper::waitForEventWithTimeOut(getEventApcControllerStatus(), defaultTimeOut ) );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, getApcControllerStatus());
}

TEST_F(CrmApcControllerTest, When_StatusConflict_Is_Detected_Then_AutoMaticActivation_Is_TurnedOff)
{
    ASSERT_TRUE ( getAutomaticActivation() );
    setApcExternalActivity( IXrayGeometryController::ApcExternalCwisActivity::StatusChangeDetected);
    ASSERT_FALSE ( getAutomaticActivation() );
}

TEST_F(CrmApcControllerTest, When_ApcCommunication_Is_Not_Allowed_Then_APC_Cannot_Be_Enabled)
{
    const double angDefault = getGeometryController().apcAngulationAngle;
    const double rotDefault = getGeometryController().apcRotationAngle;

    const double rot = 31.0;
    const double ang = 1.0;
    auto roadmap = createRoadmap(ang,rot);

    ASSERT_NE    ( angDefault, rot  ); // just to check the assumption that the initial angles of the mock geometryController are different
    ASSERT_NE    ( rotDefault, ang  );

    setAutomaticActivation(false);
    setApcAllowed         (false);

    ASSERT_FALSE ( setApc(roadmap) ); // return false because apc is not allowed
    ASSERT_EQ    ( angDefault, getGeometryController().apcAngulationAngle  );
    ASSERT_EQ    ( rotDefault, getGeometryController().apcRotationAngle    );
}

TEST_F(CrmApcControllerTest, When_ApcCommunication_Is_Not_Allowed_Then_APC_Cannot_Be_Activated)
{
    const double angDefault = getGeometryController().apcAngulationAngle;
    const double rotDefault = getGeometryController().apcRotationAngle;

    const double rot = 31.0;
    const double ang = 1.0;
    auto roadmap = createRoadmap(ang,rot);

    ASSERT_NE    ( angDefault, rot  ); // just to check the assumption that the initial angles of the mock geometryController are different
    ASSERT_NE    ( rotDefault, ang  );

    setApcAllowed         (false);

    ASSERT_FALSE ( setApc(roadmap) );
    ASSERT_EQ    ( angDefault, getGeometryController().apcAngulationAngle  );
    ASSERT_EQ    ( rotDefault, getGeometryController().apcRotationAngle    );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::DisallowBecauseOfProgrammingConflict, getApcControllerStatus());
}

TEST_F(CrmApcControllerTest, When_ApcCommunication_Is_Not_Allowed_Then_APC_Cannot_Be_Disabled)
{
    const double angDefault = getGeometryController().apcAngulationAngle;
    const double rotDefault = getGeometryController().apcRotationAngle;

    setApcAllowed         (false);

    ASSERT_FALSE ( setApc(nullptr) );
    ASSERT_EQ    ( angDefault, getGeometryController().apcAngulationAngle  );
    ASSERT_EQ    ( rotDefault, getGeometryController().apcRotationAngle    );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::DisallowBecauseOfProgrammingConflict, getApcControllerStatus());
}

}}