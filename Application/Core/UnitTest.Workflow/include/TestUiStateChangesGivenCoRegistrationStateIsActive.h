// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

using namespace CommonPlatform::CoRegistration;

class TestUiStateChangesGivenCoRegistrationStateIsActive : public TestUiStateBase
{
public:

	TestUiStateChangesGivenCoRegistrationStateIsActive() :
		recordedRunIndex(10)
	{
	}

	~TestUiStateChangesGivenCoRegistrationStateIsActive()
	{
	}

protected:

	int recordedRunIndex;

	virtual void SetUp() override
	{
		viewContext.forceHideCoregControl = false;
		viewContext.coRegRecordedRunIndex = recordedRunIndex;
		switchToEpx(PciEpx::Type::Roadmap);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		setCoRegConnected(true);
		controller.switchState(ViewState::CoRegistrationState);
	}

	void switchToFreeze()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Freeze, CoRegistrationServerState::FmState::IFRSpot);
	}

	void startExposureRun()
	{
		controller.imageRunStarted(XrayImage::Type::Exposure);
	}

	void startLiveRun()
	{
		controller.imageRunStarted(XrayImage::Type::Fluoro);
	}

	void stopLiveRun()
	{
		controller.lastImageHold();
	}

	void changeRunIndex()
	{
		controller.coRegistrationRunChanged(viewContext.coRegRecordedRunIndex + 1);
	}

	bool coregControlIsShown()
	{
		return !viewContext.forceHideCoregControl && viewContext.coRegistrationControl.visible;
	}

	bool coregControlIsHidden()
	{
		return !coregControlIsShown();
	}

	void switchToReviewStartExposureAndCheckCoregControlIsHidden()
	{
		switchToIfrReview();
		startExposureRun();
		checkCoregControlIsHidden();
	}

	void switchToReviewStartLiveAndCheckCoregControlIsHidden()
	{
		switchToIfrReview();
		switchRoadmapStatus(CrmAdministration::Status::Active);
		startLiveRun();
		checkCoregControlIsHidden();
	}

	void switchToLiveConnectWireAndCheckCoregControlIsShown()
	{
		switchToLive();
		connectWire();
		ASSERT_TRUE(coregControlIsShown());
	}

	void switchToPullbackReview()
	{
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Review, CoRegistrationServerState::FmState::IFRPullback);
	}

	void switchToRewiewAndTriggerStartFluroRun()
	{
		switchToIfrReview();
		startLiveRun();
	}

	void setCoRegStateSpotAndTriggerStartFluroRun()	
	{
		viewContext.coRegState.systemState = CoRegistrationServerState::SystemState::SpotMeasurement;
		startLiveRun();
	}

	void setCoregStatePullbackAndTriggerStartFluroRun()	
	{
		viewContext.coRegState.systemState = CoRegistrationServerState::SystemState::Pullback;
		startLiveRun();
	}

	void roadmapStatusNotActiveAndOverlayImageAvailable()
	{
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		startLiveRun();
	}

	void connectWire()
	{
		CoRegistrationServerState nextState;

		nextState.fmState				= viewContext.coRegState.fmState;
		nextState.systemState			= viewContext.coRegState.systemState;
		nextState.normalized			= true;
		nextState.pressurePlugConnected = true;
		nextState.wireConnected			= true;
		nextState.wireReady				= true;
		nextState.wireShorted			= false;
		nextState.pimConnected			= true;

		auto prevState = viewContext.coRegState;
		viewContext.coRegState = nextState;
		controller.coRegistrationStateChanged(prevState, nextState);
	}

	void connectWireAndCheckCoregControlIsShown()
	{
		switchToIfrReview();
		connectWire();
		ASSERT_TRUE(coregControlIsShown());
	}

	void startReviewDisconnectWireAndCheckReviewIsStopped()
	{
		std::shared_ptr<PullbackRecording> recording = std::make_shared<PullbackRecording>(900);
		recording->addImage(nullptr, nullptr);
		recording->addImage(nullptr, nullptr);
		viewContext.coRegistrationPlayer.setReview(recording, 1);

		viewContext.coRegistrationPlayer.startReview();
		ASSERT_TRUE(viewContext.coRegistrationPlayer.inReview());

		disconnectWire();
		ASSERT_FALSE(viewContext.coRegistrationPlayer.inReview());
	}
};

#define GIVEN_COREG(x,y,z) TEST_F(TestUiStateChangesGivenCoRegistrationStateIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_COREG( WhenEpxChangesToRoadmap_CrmGuidanceIsShown						, switchToEpx(PciEpx::Type::Roadmap)						, ViewState::CrmCreationGuidanceState	  );
GIVEN_COREG( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::NonPciEpxState		  );
GIVEN_COREG(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown			, switchToEpx(PciEpx::SubType::Live)				, ViewState::StbLiveGuidanceState);
GIVEN_COREG(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown	, switchToEpx(PciEpx::SubType::PostDeployment)	, ViewState::StbPostDeployGuidanceState);
GIVEN_COREG(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown          , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState);


GIVEN_COREG( WhenStudyChanged_CrmGuidanceIsShown	                        , studyChanged()						                    , ViewState::CrmCreationGuidanceState	  );

GIVEN_COREG( WhenRoadmapStatusChangedToRoadmapActive_CoRegIsShown			, switchRoadmapStatus(CrmAdministration::Status::Active   ) , ViewState::CoRegistrationState  );
GIVEN_COREG( WhenRoadmapStatusChangedToNoRoadmap_CoRegIsShown				, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::CoRegistrationState  );

GIVEN_COREG( WhenLastImageHold_CoRegGuidanceIsShown							, controller.lastImageHold()                                , ViewState::CoRegistrationGuidanceState  );
GIVEN_COREG( WhenStentboostAvailable_CoRegIsShown		                    , controller.stentBoostImageAvailable(liveImage, markers, boostImage)                     , ViewState::CoRegistrationState  );
GIVEN_COREG( WhenAngioProcessingStopped_CoRegIsShown		                , controller.angioProcessingStopped()                       , ViewState::CoRegistrationState  );

GIVEN_COREG( WhenAngioProcessingStarted_ProcessingRoadmapIsShown            , controller.angioProcessingStarted()                       , ViewState::CrmProcessingAngioState );
GIVEN_COREG( WhenOverlayImageAvailableAndRoadmapNotActive_CrmGuidanceShown	, roadmapStatusNotActiveAndOverlayImageAvailable()			, ViewState::CrmCreationGuidanceState );


GIVEN_COREG( WhenCoregInReviewAndExposureRunStarted_CoregControlIsHidden	, switchToReviewStartExposureAndCheckCoregControlIsHidden()	, ViewState::CrmProcessingAngioState);
GIVEN_COREG( WhenCoregInReviewAndCrmLiveRunStarted_CoregControlIsHidden		, switchToReviewStartLiveAndCheckCoregControlIsHidden()		, ViewState::CrmOverlayState);

GIVEN_COREG( WhenCoRegStatusChangedtoLive_CrmGuidanceIsShown				, switchToLive()											, ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusChangedtoLiveWireIsConnected_CrmGuidanceIsShown	, switchToLiveConnectWireAndCheckCoregControlIsShown()		, ViewState::CrmCreationGuidanceState );

GIVEN_COREG( WhenCoRegStatusInLiveAndWireBecomesDisconnected_CrmGuidanceIsShown   , switchToLive(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInReviewAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToIfrReview(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInIfrPullbackAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToIfrPullback(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInIfrSpotMeasurementAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToIfrSpotMeasurement(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInFfrSpotMeasurementAndWireBecomesDisconnected_CrmGuidanceIsShown , switchToFfrSpotMeasurement(); disconnectWireAndCheckCoregControlIsHidden(), ViewState::CrmCreationGuidanceState );

GIVEN_COREG( WhenCoRegStatusInLiveAndFMSystemDisconnected_CrmGuidanceIsShown   , switchToLive(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInReviewAndFMSystemDisconnected_CrmGuidanceIsShown , switchToIfrReview(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInIfrPullbackAndFMSystemDisconnected_CrmGuidanceIsShown , switchToIfrPullback(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInIfrSpotMeasurementAndFMSystemDisconnected_CrmGuidanceIsShown , switchToIfrSpotMeasurement(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoRegStatusInFfrSpotMeasurementAndFMSystemDisconnected_CrmGuidanceIsShown , switchToFfrSpotMeasurement(); setCoRegConnected(false), ViewState::CrmCreationGuidanceState );

GIVEN_COREG( WhenCoRegStatusBecomesValid_CoRegistrationStateRemainsActive	, connectWireAndCheckCoregControlIsShown()					, ViewState::CoRegistrationState );
GIVEN_COREG( WhenCoRegStatusBecomesInValid_CrmGuidanceShown					, startReviewDisconnectWireAndCheckReviewIsStopped()		, ViewState::CrmCreationGuidanceState );

GIVEN_COREG( whenCoregStatusSpotAndLiveRun_TheCoRegStateRemainsActive		, setCoRegStateSpotAndTriggerStartFluroRun()				, ViewState::CoRegistrationState );
GIVEN_COREG( whenCoregStatusReviewAndLiveRun_TheCreateNewRoadMapGuidanceIsShown, switchToRewiewAndTriggerStartFluroRun()				, ViewState::CrmCreationGuidanceState );
GIVEN_COREG( whenCoregStatusPullBackAndLiveRun_TheCoRegStateRemainsActive	, setCoregStatePullbackAndTriggerStartFluroRun()            , ViewState::CoRegistrationState );

GIVEN_COREG( WhenCoregStatusInPullbackReviewAndRunIndexIsChanged_CrmGuidanceIsShown, switchToPullbackReview(); changeRunIndex()               , ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoregStatusInIFRSpotReviewAndRunIndexIsChanged_CrmGuidanceIsShown, switchToIfrReview(); changeRunIndex()                     , ViewState::CrmCreationGuidanceState );
GIVEN_COREG( WhenCoregStatusInFFRSpotReviewAndRunIndexIsChanged_CrmGuidanceIsShown, switchToFfrReview(); changeRunIndex()                     , ViewState::CrmCreationGuidanceState );

TEST_F(TestUiStateChangesGivenCoRegistrationStateIsActive, WhenCoRegStatusInFfrSpotAndFluoroStoppedBeforeSwitchingToReview_TheNextRecordingWithoutFluoroEndsInCrmGuidance)
{
	switchToLive();
	switchToFfrSpotMeasurement();
	startLiveRun();
	viewContext.fluoroAcquiredWhileRecording = true; // simulate fluoro image input
	stopLiveRun();
	switchToFfrReview();

	switchToLive();
	switchToFfrSpotMeasurement();
	switchToFfrReview();

	ASSERT_EQ (ViewState::CrmCreationGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesGivenCoRegistrationStateIsActive, WhenCoRegStatusInFfrSpotAndFluoroStoppedAfterSwitchingToReview_TheNextRecordingWithoutFluoroEndsInCrmGuidance)
{
	switchToLive();
	switchToFfrSpotMeasurement();
	startLiveRun();
	viewContext.fluoroAcquiredWhileRecording = true; // simulate fluoro image input
	switchToFfrReview();
	stopLiveRun();

	switchToLive();
	switchToFfrSpotMeasurement();
	switchToFfrReview();

	ASSERT_EQ (ViewState::CrmCreationGuidanceState, controller.getSelectedView());
}


#define GIVEN_COREG_IN_REVIEW_SWITCH_EPX(Epx1, FMState, Epx2, testFun) \
	TEST_F(TestUiStateChangesGivenCoRegistrationStateIsActive, WhenEpxIs##Epx1##AndCoRegStatusIs##FMState##ReviewAndSwitchEpxTo##Epx2##_##testFun) \
	{ \
		switchToEpx(PciEpx::Type::##Epx1); \
		switchCoRegistrationState(CoRegistrationServerState::SystemState::Review, CoRegistrationServerState::FmState::FMState); \
		switchToEpx(PciEpx::Type::##Epx2); \
		ASSERT_TRUE(testFun()); \
	}

GIVEN_COREG_IN_REVIEW_SWITCH_EPX (Roadmap,    FFR,         NonPci,     coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (Roadmap,    IFRSpot,     StentBoost, coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (Roadmap,    IFRPullback, StentBoost, coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (NonPci,     IFRPullback, Roadmap,    coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (NonPci,     IFRSpot,     Roadmap,    coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (NonPci,     FFR,         StentBoost, coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (StentBoost, IFRPullback, NonPci,     coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (StentBoost, IFRSpot,     Roadmap,    coregControlIsHidden)
GIVEN_COREG_IN_REVIEW_SWITCH_EPX (StentBoost, FFR,         Roadmap,    coregControlIsHidden)

#define GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW(Epx1, FmRecording, Epx2, FmReview, testFun) \
	TEST_F(TestUiStateChangesGivenCoRegistrationStateIsActive, WhenEpxIs##Epx1##AndCoRegStatusIs##FmRecording##AndSwitchEpxTo##Epx2##AndSwitchTo##FmReview##_##testFun) \
	{ \
		switchToEpx(PciEpx::Type::##Epx1); \
		switchToLive(); \
		switchTo##FmRecording(); \
		startLiveRun(); \
		stopLiveRun(); \
		switchToEpx(PciEpx::Type::##Epx2); \
		switchTo##FmReview(); \
		ASSERT_TRUE(testFun()); \
	}

GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW (StentBoost, FfrSpotMeasurement, Roadmap, FfrReview, coregControlIsShown)
GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW (StentBoost, IfrSpotMeasurement, Roadmap, IfrReview, coregControlIsShown)
GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW (StentBoost, IfrPullback,        Roadmap, IfrReview, coregControlIsShown)
GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW (NonPci,     FfrSpotMeasurement, Roadmap, FfrReview, coregControlIsShown)
GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW (NonPci,     IfrSpotMeasurement, Roadmap, IfrReview, coregControlIsShown)
GIVEN_COREG_IN_MEASUREMENT_SWITCH_EPX_SWITCH_TO_REVIEW (NonPci,     IfrPullback,        Roadmap, IfrReview, coregControlIsShown)



TEST_F(TestUiStateChangesGivenCoRegistrationStateIsActive, WhenExposureRunStartedAndStbEpxIsActive_StbLiveIsShown)
{
	switchToEpx(PciEpx::Type::StentBoost);
	controller.switchState(ViewState::CoRegistrationState);

	controller.imageRunStarted(XrayImage::Type::Exposure);
	ASSERT_EQ(ViewState::StbLiveProcessingState, controller.getSelectedView());
}
