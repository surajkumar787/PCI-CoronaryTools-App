// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

class TestUiStateChangesGivenCrmOverlayStateIsActive : public TestUiStateBase
{
public:

    TestUiStateChangesGivenCrmOverlayStateIsActive()
    {
    }


	void switchRegistrationState(CoRegistrationServerState::SystemState prev , CoRegistrationServerState::SystemState newState)
	{
		CoRegistrationServerState old;
		CoRegistrationServerState next;

		old.fmState = CoRegistrationServerState::FmState::Unknown;
		old.systemState = prev;

		next.fmState =  CoRegistrationServerState::FmState::Unknown;
		next.systemState = newState;

		controller.coRegistrationStateChanged(old,next);
	}

protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::Roadmap);
		switchRoadmapStatus(CrmAdministration::Status::Active);
        controller.switchState(ViewState::CrmOverlayState);
    }
};

#define GIVEN_CRM_OVERLAY(x,y,z) TEST_F(TestUiStateChangesGivenCrmOverlayStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_CRM_OVERLAY( WhenEpxChangesToRoadmap_RoadmapActiveIsShown                    , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmRoadmapActive );
GIVEN_CRM_OVERLAY( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                 , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::CrmRoadmapActive       );
GIVEN_CRM_OVERLAY(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown	           , switchToEpx(PciEpx::SubType::Live)              , ViewState::StbLiveGuidanceState);
GIVEN_CRM_OVERLAY(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown   , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState);
GIVEN_CRM_OVERLAY(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown           , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState);


GIVEN_CRM_OVERLAY( WhenStudyChanged_RoadmapGuidanceIsShown                         , studyChanged()					                           , ViewState::CrmCreationGuidanceState     );

GIVEN_CRM_OVERLAY( WhenRoadmapStatusChangedToRoadmapActive_RoadmapOverlayIsShown   , switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CrmOverlayState      );
GIVEN_CRM_OVERLAY( WhenRoadmapStatusChangedToRoadmapMoevArc_ApcGuidanceIsShown     , switchRoadmapStatus(CrmAdministration::Status::MoveArc  ) , ViewState::CrmApcGuidanceState			);
GIVEN_CRM_OVERLAY( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShown , switchRoadmapStatus(CrmAdministration::Status::MoveTable) , ViewState::CrmTableGuidanceState		);
GIVEN_CRM_OVERLAY( WhenRoadmapStatusChangedToNoRoadmap_RoadmapGuidanceIsShown      , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmCreationGuidanceState     );

GIVEN_CRM_OVERLAY( WhenLastImageHold_RoadmapOverlayIsShown                         , controller.lastImageHold()                                , ViewState::CrmOverlayState      );
GIVEN_CRM_OVERLAY( WhenStentboostAvialable_RoadmapOverlayIsShown                   , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmOverlayState      );
GIVEN_CRM_OVERLAY( WhenOverlayAvailable_RoadmapOverlayIsShown                      , controller.overlayImageAvailable()                        , ViewState::CrmOverlayState      );
GIVEN_CRM_OVERLAY( WhenAngioProcessingStarted_ProcessingRoadmapIsShown             , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState );
GIVEN_CRM_OVERLAY( WhenAngioProcessingStoppedWithErrors_ErrorGuidenceIsShown       , angioProcessingStopped(CrmAdministration::Status::NoRoadmap,
																											PciSuite::CrmErrors::Error::NoInjection)    , ViewState::CrmOverlayState		); // todo must we stay in overlay state ?

GIVEN_CRM_OVERLAY( WheniFrPullBackStarted_CoRegistrationStateIsShown               , switchRegistrationState( CoRegistrationServerState::SystemState::Live , 
																											  CoRegistrationServerState::SystemState::Pullback)   , ViewState::CoRegistrationState );


GIVEN_CRM_OVERLAY( WheniFrSpotStarted_CoRegistrationStateIsShown                   , switchRegistrationState( CoRegistrationServerState::SystemState::Live,
																											   CoRegistrationServerState::SystemState::SpotMeasurement)       , ViewState::CoRegistrationState );

GIVEN_CRM_OVERLAY( WhenAngioProcessingStopped_RoadmapOverlayIsShown                , angioProcessingStoppedRoadmapActive()                     , ViewState::CrmOverlayState      );
GIVEN_CRM_OVERLAY( WhenConnectionIsLost_ConnectionFailureIsShown                   , controller.connectionStatusChanged(false)                 , ViewState::NoConnection    );
GIVEN_CRM_OVERLAY( WhenConnectionIsRestored_RoadmapOverlayIsShown                  , controller.connectionStatusChanged(true)                  , ViewState::CrmOverlayState      );