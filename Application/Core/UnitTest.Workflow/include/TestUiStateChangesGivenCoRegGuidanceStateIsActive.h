// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

using namespace CommonPlatform::CoRegistration;

class TestUiStateChangesGivenCoRegGuidanceStateIsActive : public TestUiStateBase
{
public:

	TestUiStateChangesGivenCoRegGuidanceStateIsActive()
	{
	}

	~TestUiStateChangesGivenCoRegGuidanceStateIsActive()
	{
	}

protected:

	virtual void SetUp() override
	{
		viewContext.forceHideCoregControl = false;
		switchToEpx(PciEpx::Type::Roadmap);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		setCoRegConnected(true);
		controller.switchState(ViewState::CoRegistrationGuidanceState);
	}

	void captureWireAndSwitchToReview()
	{
		viewContext.fluoroAcquiredWhileRecording = true;
		switchToIfrReview();
	}
};

#define GIVEN_COREG_GUIDANCE(x,y,z) TEST_F(TestUiStateChangesGivenCoRegGuidanceStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_COREG_GUIDANCE( WhenEpxChangesToRoadmap_CrmGuidanceIsShown                    , switchToEpx(PciEpx::Type::Roadmap)						, ViewState::CrmCreationGuidanceState    );
GIVEN_COREG_GUIDANCE( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown               , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::NonPciEpxState       );
GIVEN_COREG_GUIDANCE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown			, switchToEpx(PciEpx::SubType::Live)			    , ViewState::StbLiveGuidanceState);
GIVEN_COREG_GUIDANCE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown , switchToEpx(PciEpx::SubType::PostDeployment)    , ViewState::StbPostDeployGuidanceState);
GIVEN_COREG_GUIDANCE(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown         , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState);

GIVEN_COREG_GUIDANCE( WhenStudyChanged_CrmGuidanceIsShown                           , studyChanged()											, ViewState::CrmCreationGuidanceState    );

GIVEN_COREG_GUIDANCE( WhenRoadmapStatusChangedToRoadmapActive_CoRegGuidanceIsShown  , switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CoRegistrationGuidanceState     );
GIVEN_COREG_GUIDANCE( WhenRoadmapStatusChangedToNoRoadmap_CoRegGuidanceIsShown      , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CoRegistrationGuidanceState     );

GIVEN_COREG_GUIDANCE( WhenLastImageHold_CoRegGuidanceIsShown                        , controller.lastImageHold()                                , ViewState::CoRegistrationGuidanceState     );
GIVEN_COREG_GUIDANCE( WhenStentboostAvialable_CoRegGuidanceIsShown                  , controller.stentBoostImageAvailable(liveImage, markers, boostImage)                     , ViewState::CoRegistrationGuidanceState     );
GIVEN_COREG_GUIDANCE( WhenAngioProcessingStopped_CoRegGuidanceIsShown               , controller.angioProcessingStopped()                       , ViewState::CoRegistrationGuidanceState     );

GIVEN_COREG_GUIDANCE( WhenAngioProcessingStarted_ProcessingRoadmapIsShown           , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState );
GIVEN_COREG_GUIDANCE( WhenOverlayImageAvailable_CoRegIsShown						, controller.overlayImageAvailable()						, ViewState::CoRegistrationState );

GIVEN_COREG_GUIDANCE( WhenWireCapturedAndCoregStateSwitchesToReview_CoregIsShown	, captureWireAndSwitchToReview()							, ViewState::CoRegistrationState );
GIVEN_COREG_GUIDANCE( WhenWireNotCapturedAndCoregStateToReview_CrmGuidanceIsShown	, switchToIfrReview()										, ViewState::CrmCreationGuidanceState	 );

GIVEN_COREG_GUIDANCE( WhenCoRegStatusChangedtoLive_CrmGuidanceIsShown				, switchCoRegistrationState(CoRegistrationServerState::SystemState::Live, 
	CoRegistrationServerState::FmState::FFR), ViewState::CrmCreationGuidanceState );

GIVEN_COREG_GUIDANCE( WhenCoRegStatusInLiveAndWireBecomesDisconnected_CrmGuidanceIsShown   , switchToLive(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInReviewAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToIfrReview(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInIfrPullbackAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToIfrPullback(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInIfrSpotMeasurementAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToIfrSpotMeasurement(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInFfrSpotMeasurementAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToFfrSpotMeasurement(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );

GIVEN_COREG_GUIDANCE( WhenCoRegStatusInLiveAndFMSystemDisconnected_CrmGuidanceIsShown   , switchToLive(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInReviewAndFMSystemDisconnected_CrmGuidanceIsShown , switchToIfrReview(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInIfrPullbackAndFMSystemDisconnected_CrmGuidanceIsShown , switchToIfrPullback(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInIfrSpotMeasurementAndFMSystemDisconnected_CrmGuidanceIsShown , switchToIfrSpotMeasurement(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG_GUIDANCE( WhenCoRegStatusInFfrSpotMeasurementAndFMSystemDisconnected_CrmGuidanceIsShown , switchToFfrSpotMeasurement(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
