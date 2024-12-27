// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"


class TestUiStateChangesGivenCrmApcGuideStateIsActive : public TestUiStateBase
{
public:
	TestUiStateChangesGivenCrmApcGuideStateIsActive(void){};

protected:
    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::Roadmap);
		switchRoadmapStatus(CrmAdministration::Status::MoveArc);
		controller.switchState(ViewState::CrmApcGuidanceState);
    }
};


#define GIVEN_CRM_APC(x,y,z) TEST_F(TestUiStateChangesGivenCrmApcGuideStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_CRM_APC( WhenEpxChangesToRoadmap_ApcGuidanceIsShown                      , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmApcGuidanceState			);
GIVEN_CRM_APC( WhenEpxChangesToNonPciEpx_ApcGuidanceIsShown                    , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::CrmApcGuidanceState			);
GIVEN_CRM_APC(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown			   , switchToEpx(PciEpx::SubType::Live)              , ViewState::StbLiveGuidanceState        );
GIVEN_CRM_APC(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown   , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState  );
GIVEN_CRM_APC(WhenEpxChangesToStbContrast_StbPostDeploymentGuidanceIsShown     , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState    );

GIVEN_CRM_APC( WhenStudyChanged_RoadmapGuidanceIsShown                         , studyChanged()							                   , ViewState::CrmCreationGuidanceState    );

GIVEN_CRM_APC( WhenRoadmapStatusChangedToRoadmapActive_RoadmapActiveIsShown    , switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CrmRoadmapActive			);
GIVEN_CRM_APC( WhenRoadmapStatusChangedToRoadmapMoveArc_ApcGuidanceIsShown     , switchRoadmapStatus(CrmAdministration::Status::MoveArc  ) , ViewState::CrmApcGuidanceState			);
GIVEN_CRM_APC( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShown , switchRoadmapStatus(CrmAdministration::Status::MoveTable) , ViewState::CrmTableGuidanceState		);
GIVEN_CRM_APC( WhenRoadmapStatusChangedToNoRoadmap_RoadmapGuidanceIsShown      , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmCreationGuidanceState    );

GIVEN_CRM_APC( WhenLastImageHold_ApcGuidanceIsShown                            , controller.lastImageHold()                                , ViewState::CrmApcGuidanceState				);
GIVEN_CRM_APC( WhenStentboostAvialable_ApcGuidanceIsShown                      , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmApcGuidanceState				);
GIVEN_CRM_APC( WhenAngioProcessingStarted_ProcessingRoadmapIsShown             , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState		);
GIVEN_CRM_APC( WhenAngioProcessingStopped_RoadmapOverlayIsShown                , angioProcessingStoppedRoadmapActive()                     , ViewState::CrmRoadmapActive				);
GIVEN_CRM_APC( WhenAngioProcessingStoppedWithErrors_ErrorGuidenceIsShown       , angioProcessingStopped(CrmAdministration::Status::NoRoadmap,
																										PciSuite::CrmErrors::Error::NoInjection)     , ViewState::CrmErrorGuidanceState		);
GIVEN_CRM_APC( WhenConnectionIsLost_ConnectionFailureIsShown                   , controller.connectionStatusChanged(false)                 , ViewState::NoConnection				);
GIVEN_CRM_APC( WhenConnectionIsRestored_ApcGuidanceIsShown                     , controller.connectionStatusChanged(true)                  , ViewState::CrmApcGuidanceState				);

