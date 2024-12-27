// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

using namespace CommonPlatform::CoRegistration;

class TestUiStateChangesGivenCrmCreationGuidanceStateIsActive : public TestUiStateBase
{
public:

    TestUiStateChangesGivenCrmCreationGuidanceStateIsActive()
    {
    }

protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::Roadmap);
        switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::CrmCreationGuidanceState);
    }

	void switchToFreeze()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Freeze, CoRegistrationServerState::FmState::IFRSpot);
	}

	void checkCoregControlIsHidden()
	{
		ASSERT_TRUE(viewContext.forceHideCoregControl);
	}

	void switchToFreezeAndCheckCoregControlIsHidden()
	{
		switchToFreeze();
		checkCoregControlIsHidden();
	}
};

#define GIVEN_CRM_GUIDANCE(x,y,z) TEST_F(TestUiStateChangesGivenCrmCreationGuidanceStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_CRM_GUIDANCE( WhenEpxChangesToRoadmap_RoadmapGuidanceIsShown                , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmCreationGuidanceState     );
GIVEN_CRM_GUIDANCE( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown               , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::NonPciEpxState               );
GIVEN_CRM_GUIDANCE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown          , switchToEpx(PciEpx::SubType::Live)              , ViewState::StbLiveGuidanceState         );
GIVEN_CRM_GUIDANCE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState   );
GIVEN_CRM_GUIDANCE(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown         , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState     );


GIVEN_CRM_GUIDANCE( WhenStudyChanged_RoadmapGuidanceIsShown                       , studyChanged()					                          , ViewState::CrmCreationGuidanceState     );

GIVEN_CRM_GUIDANCE( WhenRoadmapStatusChangedToRoadmapActive_RoadmapActiveIsShown  , switchRoadmapStatus(CrmAdministration::Status::Active   )   , ViewState::CrmRoadmapActive     );
GIVEN_CRM_GUIDANCE( WhenRoadmapStatusChangedToRoadmapMoveArc_ApcGuidanceIsShown   , switchRoadmapStatus(CrmAdministration::Status::MoveArc  ) , ViewState::CrmApcGuidanceState		 );
GIVEN_CRM_GUIDANCE( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShown, switchRoadmapStatus(CrmAdministration::Status::MoveTable) , ViewState::CrmTableGuidanceState		 );
GIVEN_CRM_GUIDANCE( WhenRoadmapStatusChangedToNoRoadmap_RoadmapGuidanceIsShown    , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmCreationGuidanceState     );

GIVEN_CRM_GUIDANCE( WhenLastImageHold_RoadmapGuidanceIsShown                      , controller.lastImageHold()                                , ViewState::CrmCreationGuidanceState     );
GIVEN_CRM_GUIDANCE( WhenStentboostAvialable_RoadmapGuidanceIsShown                , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmCreationGuidanceState     );
GIVEN_CRM_GUIDANCE( WhenAngioProcessingStopped_RoadmapActiveIsShown             , angioProcessingStoppedRoadmapActive()                       , ViewState::CrmRoadmapActive     );

GIVEN_CRM_GUIDANCE( WhenAngioProcessingStarted_ProcessingRoadmapIsShown           , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState );

TEST_F(TestUiStateChangesGivenCrmCreationGuidanceStateIsActive, WhenCoRegStatusChangedFromLivetoPullback_CoRegGuidanceIsShown)
{
	switchCoRegistrationState(CoRegistrationServerState::SystemState::Live, CoRegistrationServerState::FmState::FFR);
	switchCoRegistrationState(CoRegistrationServerState::SystemState::Pullback, CoRegistrationServerState::FmState::FFR);

	ASSERT_EQ(ViewState::CoRegistrationGuidanceState, controller.getSelectedView());
}


TEST_F(TestUiStateChangesGivenCrmCreationGuidanceStateIsActive, GivenCrmGuidance_WhenWhenRoadmapStatusActiveAndOverlayAvailable_RoadmapOverlayIsShown)
{
	viewContext.crmAdminStatus = CrmAdministration::Status::Active;
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::CrmOverlayState, controller.getSelectedView());
}

GIVEN_CRM_GUIDANCE( WhenCoregFrozen_CoregControlIsHidden						, switchToFreezeAndCheckCoregControlIsHidden()				, ViewState::CrmCreationGuidanceState);

//Exceptional
GIVEN_CRM_GUIDANCE( WhenRoadmapStatusChangedToNoRoadmap_AndStbEpxActive_CrmGuidanceIsShown          , switchToEpxAndRoadmapStatus(PciEpx::Type::StentBoost, CrmAdministration::Status::NoRoadmap ) , ViewState::CrmCreationGuidanceState );
GIVEN_CRM_GUIDANCE( WhenRoadmapStatusChangedToNoRoadmap_AndNonPciEpxActive_NonPciEpxGuidanceIsShown , switchToEpxAndRoadmapStatus(PciEpx::Type::NonPci    , CrmAdministration::Status::NoRoadmap ) , ViewState::NonPciEpxState   );
GIVEN_CRM_GUIDANCE( WhenConnectionIsLost_ConnectionFailureIsShown									, controller.connectionStatusChanged(false)                 , ViewState::NoConnection);
GIVEN_CRM_GUIDANCE( WhenConnectionIsRestored_RoadmapGuidanceIsShown									, controller.connectionStatusChanged(true)                  , ViewState::CrmCreationGuidanceState );

