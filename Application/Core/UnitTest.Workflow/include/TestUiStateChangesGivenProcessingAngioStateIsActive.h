// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

class TestUiStateChangesGivenProcessingAngioStateIsActive : public TestUiStateBase
{
public:

    TestUiStateChangesGivenProcessingAngioStateIsActive()
    {
    }

    ~TestUiStateChangesGivenProcessingAngioStateIsActive()
    {
    }

protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::Roadmap);
        switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
        controller.switchState(ViewState::CrmProcessingAngioState);
    }


};

#define GIVEN_PROCESSING_ANGIO(x,y,z) TEST_F(TestUiStateChangesGivenProcessingAngioStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_PROCESSING_ANGIO( WhenEpxChangesToRoadmap_RoadmapGuidanceIsShown                      , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmCreationGuidanceState     );
GIVEN_PROCESSING_ANGIO( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                     , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::NonPciEpxState       );
GIVEN_PROCESSING_ANGIO(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown				, switchToEpx(PciEpx::SubType::Live)				, ViewState::StbLiveGuidanceState);
GIVEN_PROCESSING_ANGIO(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown		, switchToEpx(PciEpx::SubType::PostDeployment)	, ViewState::StbPostDeployGuidanceState);
GIVEN_PROCESSING_ANGIO(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown               , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState);


GIVEN_PROCESSING_ANGIO( WhenStudyChanged_RoadmapGuidanceIsShown                             , studyChanged()					                        , ViewState::CrmCreationGuidanceState     );

GIVEN_PROCESSING_ANGIO( WhenRoadmapStatusChangedToRoadmapActive_ProcessingAngioStateIsShown , switchRoadmapStatus(CrmAdministration::Status::Active  )  , ViewState::CrmProcessingAngioState );
GIVEN_PROCESSING_ANGIO( WhenRoadmapStatusChangedToRoadmapMoveArc_ProcessingeIsShown			, switchRoadmapStatus(CrmAdministration::Status::MoveArc  )	, ViewState::CrmProcessingAngioState		 );
GIVEN_PROCESSING_ANGIO( WhenRoadmapStatusChangedToRoadmapMoveTable_ProcessingIsShown		, switchRoadmapStatus(CrmAdministration::Status::MoveTable)	, ViewState::CrmProcessingAngioState		 );
GIVEN_PROCESSING_ANGIO( WhenRoadmapStatusChangedToNoRoadmap_ProcessingAngioStateIsShown     , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CrmProcessingAngioState );

GIVEN_PROCESSING_ANGIO( WhenLastImageHold_ProcessingAngioStateIsShown                       , controller.lastImageHold()                                , ViewState::CrmProcessingAngioState );
GIVEN_PROCESSING_ANGIO( WhenStentboostAvialable_ProcessingAngioStateIsShown                 , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::CrmProcessingAngioState );
GIVEN_PROCESSING_ANGIO( WhenOverlayAvailable_ProcessingAngioStateIsShown                    , controller.overlayImageAvailable()                        , ViewState::CrmProcessingAngioState );
GIVEN_PROCESSING_ANGIO( WhenAngioProcessingStarted_ProcessingAngioStateIsShown              , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState );
GIVEN_PROCESSING_ANGIO( WhenAngioProcessingStopped_RoadmapGuidanceIsShown                   , angioProcessingStoppedRoadmapActive()                     , ViewState::CrmRoadmapActive     );
GIVEN_PROCESSING_ANGIO( WhenConnectionIsLost_ConnectionFailureIsShown                       , controller.connectionStatusChanged(false)                 , ViewState::NoConnection         );
GIVEN_PROCESSING_ANGIO( WhenConnectionIsRestored_ProcessingAngio                            , controller.connectionStatusChanged(true)                  , ViewState::CrmProcessingAngioState );
GIVEN_PROCESSING_ANGIO( WhenProcessingStoppedWithError_errorStateIsShown                    , angioProcessingStopped(CrmAdministration::Status::NoRoadmap,
																													 PciSuite::CrmErrors::Error::AngioTooShort)	, ViewState::CrmErrorGuidanceState );  

TEST_F(TestUiStateChangesGivenProcessingAngioStateIsActive, WhenAngioProcessingStoppedAndCoRegActive_CoRegGuidanceIsShown)
{
	switchCoRegistrationState(CoRegistrationServerState::SystemState::Pullback, CoRegistrationServerState::FmState::IFRPullback);
	controller.angioProcessingStopped();

	ASSERT_EQ(controller.getSelectedView(), ViewState::CoRegistrationGuidanceState);
}





