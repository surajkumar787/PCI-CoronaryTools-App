// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"


class TestUiStateChangesGivenCrmTableGuideStateIsActive : public TestUiStateBase
{
public:
	TestUiStateChangesGivenCrmTableGuideStateIsActive(void){};

protected:
    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::Roadmap);
		switchRoadmapStatus(CrmAdministration::Status::MoveTable);
		controller.switchState(ViewState::CrmTableGuidanceState);
    }
};


#define GIVEN_CRM_TABLE(x,y,z) TEST_F(TestUiStateChangesGivenCrmTableGuideStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_CRM_TABLE( WhenEpxChangesToRoadmap_TableGuidanceIsShown                    , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmTableGuidanceState			);
GIVEN_CRM_TABLE( WhenEpxChangesToNonPciEpx_TableGuidanceIsShown                  , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::CrmTableGuidanceState			);
GIVEN_CRM_TABLE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown            , switchToEpx(PciEpx::SubType::Live)              , ViewState::StbLiveGuidanceState          );
GIVEN_CRM_TABLE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown   , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState    );
GIVEN_CRM_TABLE(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown           , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState      );

GIVEN_CRM_TABLE( WhenStudyChanged_RoadmapGuidanceIsShown                         , studyChanged()							                 , ViewState::CrmCreationGuidanceState		);

GIVEN_CRM_TABLE( WhenRoadmapStatusChangedToRoadmapActive_RoadmapActiveIsShown    , switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CrmRoadmapActive				);
GIVEN_CRM_TABLE( WhenRoadmapStatusChangedToRoadmapMoveArc_ApcGuidanceIsShown     , switchRoadmapStatus(CrmAdministration::Status::MoveArc  ) , ViewState::CrmApcGuidanceState			);
GIVEN_CRM_TABLE( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShown , switchRoadmapStatus(CrmAdministration::Status::MoveTable) , ViewState::CrmTableGuidanceState			);
GIVEN_CRM_TABLE( WhenRoadmapStatusChangedToNoRoadmap_RoadmapGuidanceIsShown      , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmCreationGuidanceState		);

GIVEN_CRM_TABLE( WhenLastImageHold_TableGuidanceIsShown                          , controller.lastImageHold()                                , ViewState::CrmTableGuidanceState			);
GIVEN_CRM_TABLE( WhenStentboostAvialable_TableGuidanceIsShown                    , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmTableGuidanceState			);
GIVEN_CRM_TABLE( WhenAngioProcessingStarted_ProcessingRoadmapIsShown             , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState		);
GIVEN_CRM_TABLE( WhenAngioProcessingStopped_RoadmapOverlayIsShown                , angioProcessingStoppedRoadmapActive()                     , ViewState::CrmRoadmapActive				);
GIVEN_CRM_TABLE( WhenAngioProcessingStoppedWithErrors_ErrorGuidenceIsShown       , angioProcessingStopped(CrmAdministration::Status::NoRoadmap,
																										PciSuite::CrmErrors::Error::NoInjection)       , ViewState::CrmErrorGuidanceState			);
GIVEN_CRM_TABLE( WhenConnectionIsLost_ConnectionFailureIsShown                   , controller.connectionStatusChanged(false)                 , ViewState::NoConnection					);
GIVEN_CRM_TABLE( WhenConnectionIsRestored_TableGuidanceIsShown                   , controller.connectionStatusChanged(true)                  , ViewState::CrmTableGuidanceState			);


