// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

class TestUiStateChangesGivenCrmRoadmapActive : public TestUiStateBase
{
public:
	TestUiStateChangesGivenCrmRoadmapActive(void){};

protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::Roadmap);
        switchRoadmapStatus(CrmAdministration::Status::Active);
		controller.switchState(ViewState::CrmRoadmapActive);
    }
};

#define GIVEN_CRM_ROADMAPACTIVE(x,y,z) TEST_F(TestUiStateChangesGivenCrmRoadmapActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_CRM_ROADMAPACTIVE( WhenEpxChangesToRoadmap_RoadmapActiveIsShown                    , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmRoadmapActive           );
GIVEN_CRM_ROADMAPACTIVE( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                 , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::CrmRoadmapActive           );
GIVEN_CRM_ROADMAPACTIVE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown            , switchToEpx(PciEpx::SubType::Live)              , ViewState::StbLiveGuidanceState       );
GIVEN_CRM_ROADMAPACTIVE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown   , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState );
GIVEN_CRM_ROADMAPACTIVE(WhenEpxChangesToStbContrast_StbPostDeploymentGuidanceIsShown     , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState   );

GIVEN_CRM_ROADMAPACTIVE( WhenStudyChanged_RoadmapGuidanceIsShown                         , studyChanged()					                         , ViewState::CrmCreationGuidanceState    );

GIVEN_CRM_ROADMAPACTIVE( WhenRoadmapStatusChangedToRoadmapActive_RoadmapActiveIsShown    , switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CrmRoadmapActive			);
GIVEN_CRM_ROADMAPACTIVE( WhenRoadmapStatusChangedToRoadmapMoveArc_ApcGuidanceIsShown     , switchRoadmapStatus(CrmAdministration::Status::MoveArc  ) , ViewState::CrmApcGuidanceState		);
GIVEN_CRM_ROADMAPACTIVE( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShown , switchRoadmapStatus(CrmAdministration::Status::MoveTable) , ViewState::CrmTableGuidanceState		);
GIVEN_CRM_ROADMAPACTIVE( WhenRoadmapStatusChangedToNoRoadmap_RoadmapGuidanceIsShown      , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmCreationGuidanceState  );

GIVEN_CRM_ROADMAPACTIVE( WhenLastImageHold_RoadmapActiveIsShown                          , controller.lastImageHold()                                , ViewState::CrmRoadmapActive      );
GIVEN_CRM_ROADMAPACTIVE( WhenStentboostAvialable_RoadmapActiveIsShown                    , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmRoadmapActive      );
GIVEN_CRM_ROADMAPACTIVE( WhenOverlayAvailable_RoadmapOverlayIsShown                      , controller.overlayImageAvailable()                        , ViewState::CrmOverlayState      );
GIVEN_CRM_ROADMAPACTIVE( WhenAngioProcessingStarted_ProcessingRoadmapIsShown             , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState );
GIVEN_CRM_ROADMAPACTIVE( WhenAngioProcessingStoppedWithErrors_ErrorGuidenceIsShown       , angioProcessingStopped(CrmAdministration::Status::NoRoadmap,
																											      PciSuite::CrmErrors::Error::NoInjection)    , ViewState::CrmErrorGuidanceState		);

GIVEN_CRM_ROADMAPACTIVE( WhenAngioProcessingStopped_RoadmapActiveIsShown                 , angioProcessingStoppedRoadmapActive()                     , ViewState::CrmRoadmapActive      );
GIVEN_CRM_ROADMAPACTIVE( WhenConnectionIsLost_ConnectionFailureIsShown                   , controller.connectionStatusChanged(false)                 , ViewState::NoConnection    );
GIVEN_CRM_ROADMAPACTIVE( WhenConnectionIsRestored_RoadmapActiveIsShown                   , controller.connectionStatusChanged(true)                  , ViewState::CrmRoadmapActive      );