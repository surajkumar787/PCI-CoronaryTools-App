// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"


class TestUiStateChangesGivenCrmErrorStateActive : public TestUiStateBase
{
public:
	TestUiStateChangesGivenCrmErrorStateActive(void){};

	static const PciSuite::CrmErrors::Error CurrentError = PciSuite::CrmErrors::Error::AngioTooShort;
protected:

    virtual void SetUp() override
    {
		switchToEpx(PciEpx::Type::Roadmap);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::CrmErrorGuidanceState);
		viewContext.roadmapErrorState = CurrentError; 
	}
};

#define GIVEN_CRM_ERROR(x,y,z,e) TEST_F(TestUiStateChangesGivenCrmErrorStateActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());ASSERT_EQ(e,viewContext.roadmapErrorState);}


GIVEN_CRM_ERROR( WhenEpxChangesToRoadmap_ErrorGuidanceIsShown                    , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmErrorGuidanceState      , CurrentError );
GIVEN_CRM_ERROR( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                 , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::NonPciEpxState		     , CurrentError);
GIVEN_CRM_ERROR( WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown		     , switchToEpx(PciEpx::SubType::Live)              , ViewState::StbLiveGuidanceState       , CurrentError);
GIVEN_CRM_ERROR( WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown  , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState , CurrentError);
GIVEN_CRM_ERROR(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown           , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState   , CurrentError);

GIVEN_CRM_ERROR( WhenStudyChanged_RoadmapGuidanceIsShown                         , studyChanged()				                             , ViewState::CrmCreationGuidanceState, PciSuite::CrmErrors::Error::OK);

GIVEN_CRM_ERROR( WhenRoadmapStatusChangedToRoadmapActive_RoadmapActiveIsShown   , switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CrmRoadmapActive 		  , PciSuite::CrmErrors::Error::OK);
GIVEN_CRM_ERROR( WhenRoadmapStatusChangedToRoadmapMoveArc_ApcGuidanceIsShow     , switchRoadmapStatus(CrmAdministration::Status::MoveArc  ) , ViewState::CrmApcGuidanceState	  , PciSuite::CrmErrors::Error::OK);
GIVEN_CRM_ERROR( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShow , switchRoadmapStatus(CrmAdministration::Status::MoveTable) , ViewState::CrmTableGuidanceState    , PciSuite::CrmErrors::Error::OK);
GIVEN_CRM_ERROR( WhenRoadmapStatusChangedToNoRoadmap_ErrorGuidanceIsShown       , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmCreationGuidanceState , PciSuite::CrmErrors::Error::OK);

GIVEN_CRM_ERROR( WhenLastImageHold_ErrorGuidanceIsShown                         , controller.lastImageHold()                                , ViewState::CrmErrorGuidanceState	  , CurrentError);
GIVEN_CRM_ERROR( WhenStentboostAvialable_ErrorGuidanceIsShown                   , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmErrorGuidanceState    , CurrentError);
GIVEN_CRM_ERROR( WhenAngioProcessingStarted_ProcessingRoadmapIsShown            , angioProcessingStartedClearError()			             , ViewState::CrmProcessingAngioState , PciSuite::CrmErrors::Error::OK);
GIVEN_CRM_ERROR( WhenAngioProcessingStopped_RoadmapOverlayIsShown               , angioProcessingStoppedRoadmapActive()                     , ViewState::CrmRoadmapActive  		  , PciSuite::CrmErrors::Error::OK);
GIVEN_CRM_ERROR( WhenConnectionIsLost_ConnectionFailureIsShown                  , controller.connectionStatusChanged(false)                 , ViewState::NoConnection   		  , CurrentError);
GIVEN_CRM_ERROR( WhenConnectionIsRestored_ErrorGuidanceyIsShown                 , controller.connectionStatusChanged(true)                  , ViewState::CrmErrorGuidanceState    , CurrentError);
