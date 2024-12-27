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

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci{ namespace Test{
using namespace CommonPlatform::Xray;

// This class test the initialisation of the CrmApcController 
class CrmApcControllerInitialisationTest : public ::testing::Test
{
public:
    const static int defaultTimeOut = 250;

	CrmApcControllerInitialisationTest() 
    :   m_log(CommonPlatform::Logger(CommonPlatform::LoggerType:: NullLogger)),
		m_geometryController          (),
        m_apcController               (m_log, m_geometryController, true, L"StudyID", true),
        m_apcControllerStatus         (CrmApcController::ApcModeStatus::AutomaticActivation),
        m_eventApcControllerStatus    (CreateEvent(nullptr, TRUE, FALSE, L"RoadmapZoneStatus"))
    {
	}

    ~CrmApcControllerInitialisationTest()
	{
	    CloseHandle(m_eventApcControllerStatus); m_eventApcControllerStatus = nullptr;
		Simplicity::system().resetHeap();
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

        // Note: start is disconnected state
    }

    std::shared_ptr<const CrmRoadmap> createRoadmap(double angulation, double rotation)
    {
        XrayGeometry geometry;
        geometry.angulation = angulation;
        geometry.rotation   = rotation;
        auto roadmap = std::make_shared<CrmRoadmap>(5, 5,1024,1024, 0.1, geometry, Sense::Matrix(), 0.0);
       return roadmap;
    }

	bool getApcAllowed() {return m_apcController.getApcAllowed();	};
	bool getAutomaticActivation() { return m_apcController.getAutomaticActivation(); };
	XrayGeometryControllerMock& getGeometryController() { return m_geometryController; };

protected:
	CommonPlatform::Log				&m_log;

private:
	XrayGeometryControllerMock      m_geometryController;
	CrmApcController                m_apcController;
	CrmApcController::ApcModeStatus m_apcControllerStatus;
	HANDLE                          m_eventApcControllerStatus;

};

// todo: test connection status and influence on allowed

TEST_F(CrmApcControllerInitialisationTest, When_ApcController_Is_Initialized_Then_ApcCommunication_Is_Not_Allowed)
{
    ASSERT_FALSE ( getApcAllowed() );
}

TEST_F(CrmApcControllerInitialisationTest, Given_Configuration_Is_AutomaticActivation_And_LastMode_Is_AutomaticActivation_When_Connected_Then_AutomaticActivation_Is_Enabled)
{
    CrmApcController apcController(m_log, getGeometryController(), true, L"StudyID1", true);
    apcController.setCwisApcConnected(true);
    ASSERT_TRUE  ( apcController.getApcAllowed()                                                            );   
    ASSERT_TRUE  ( apcController.getAutomaticActivation()                                                   );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::AutomaticActivation, apcController.getStatus()          );
}

TEST_F(CrmApcControllerInitialisationTest, Given_Configuration_Is_AutomaticActivation_And_LastMode_IsNot_AutomaticActivation_When_Connected_Then_AutomaticActivation_Is_Disabled)
{
    CrmApcController apcController(m_log, getGeometryController(), true, L"StudyID2", false);
    apcController.setCwisApcConnected(true);
    ASSERT_TRUE  ( apcController.getApcAllowed()                                                            );   
    ASSERT_FALSE ( apcController.getAutomaticActivation()                                                   );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, apcController.getStatus()    );
}

TEST_F(CrmApcControllerInitialisationTest, Given_Configuration_Is_Not_AutomaticActivation_And_LastMode_AutomaticActivation_When_Connected_Then_AutomaticActivation_Is_Disabled)
{
    CrmApcController apcController(m_log, getGeometryController(), false, L"StudyID3", true);
    apcController.setCwisApcConnected(true);
    ASSERT_TRUE  ( apcController.getApcAllowed()                                                            );   
    ASSERT_FALSE ( apcController.getAutomaticActivation()                                                   );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, apcController.getStatus()    );
}

TEST_F(CrmApcControllerInitialisationTest, Given_Configuration_Is_Not_AutomaticActivation_And_LastMode_IsNot_AutomaticActivation_When_Connected_Then_AutomaticActivation_Is_Disabled)
{
    CrmApcController apcController(m_log, getGeometryController(), false, L"StudyID4", false);
    apcController.setCwisApcConnected(true);
    ASSERT_TRUE  ( apcController.getApcAllowed()                                                            );   
    ASSERT_FALSE ( apcController.getAutomaticActivation()                                                   );
    ASSERT_EQ    ( CrmApcController::ApcModeStatus::TargetSetWithoutActivaton, apcController.getStatus()    );
}

TEST_F(CrmApcControllerInitialisationTest, When_ApcController_Is_Disconnected_Then_ApcCommunication_Is_Not_Allowed)
{
    CrmApcController apcController(m_log, getGeometryController(), true, L"StudyID1", true);
    apcController.setCwisApcConnected(true);
    ASSERT_TRUE  ( apcController.getApcAllowed()                                                            );   

    apcController.setCwisApcConnected(false);
    ASSERT_FALSE ( apcController.getApcAllowed()                                                            );   
}
}}
//TICS +OLC#009  magic numbers for a test are ok.