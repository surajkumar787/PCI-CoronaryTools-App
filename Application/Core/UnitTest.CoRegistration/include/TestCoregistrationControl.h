// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "gtest/gtest.h"
#include "ExperienceIdentity/Controls/Canvas.h"
#include "Sense/System/Scheduler.h"
#include "CoRegistrationAdministration.h"
#include "CoRegistrationServerState.h"
#include "CoRegistrationControl.h"
#include "CoRegistrationSpotMeasurement.h"
#include <memory>

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;


class CoregistrationControlWrapper : public CoRegistrationControl
{
public:
	CoregistrationControlWrapper(Control& parent, ViewType type) :
		CoRegistrationControl(parent, type)
	{}

	CoregUiAdapter& getGui() { return gui; }
};

class TestCoregistrationControl : public ::testing::Test
{
public:

	TestCoregistrationControl() :
		m_gpu(0),
		m_canvas(m_gpu, m_scheduler),
		m_mouse(m_canvas),
		m_keyboard(m_canvas),
		m_coRegControl(m_canvas, ViewType::Allura),
		m_coregGui(m_coRegControl.getGui())
	{
	}

	~TestCoregistrationControl()
	{
	}

	void SetUp() override
	{
		Test::SetUp();

		Sense::Logging::eventAssert = [&](const wchar_t *t) { std::wcout << L"Sense assert: " << t << std::endl; FAIL(); };
		Sense::Logging::eventError = [&](const wchar_t *t) { std::wcout << L"Sense error: " << t << std::endl; FAIL(); };

		//start in live;
		m_coRegControl.setCoRegConnection(true);

		CoRegistrationServerState coRegState;
		coRegState.systemState = CoRegistrationServerState::SystemState::Live;
		coRegState.fmState = CoRegistrationServerState::FmState::Unknown;
		m_coRegControl.setCoRegState(coRegState);
	}

	void switchToIdle()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::FFR;
		state.systemState = CoRegistrationServerState::SystemState::Idle;

		m_coRegControl.setCoRegState(state);
	}

	void switchToPullback()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::IFRPullback;
		state.systemState = CoRegistrationServerState::SystemState::Pullback;

		m_coRegControl.setCoRegState(state);
	}

	void switchToIfrSpotRecording()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::IFRSpot;
		state.systemState = CoRegistrationServerState::SystemState::SpotMeasurement;

		m_coRegControl.setCoRegState(state);
	}

	void switchToFfrRecording()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::FFR;
		state.systemState = CoRegistrationServerState::SystemState::SpotMeasurement;

		m_coRegControl.setCoRegState(state);
	}

	void setPressureAndEcg()
	{
		CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg pressAndEcg;
		m_coRegControl.setPressureAndEcg(pressAndEcg);
	}

	void setPdPaInfo()
	{
		CommonPlatform::CoRegistration::CoRegistrationPdPaInfo pdpaInfo;
		m_coRegControl.setPdPaInfo(pdpaInfo);
	}

	void setSpotMeasurement()
	{
		CommonPlatform::CoRegistration::CoRegistrationSpotMeasurement spotData;
		m_coRegControl.setSpotMeasurement(spotData);
	}

	void setIfrSpotCycle()
	{
		// Do nothing; iFR spot data is not routed to control
	}

	void setIfrPullbackCycle()
	{
		CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle pullbackCycle;
		m_coRegControl.setIfrPullbackCycle(pullbackCycle);
	}

	CoregistrationControlWrapper& getCoRegControler() { return m_coRegControl; };
	CoregUiAdapter& getCoRegUi() { return m_coregGui; };

private:
	Sense::Scheduler					 m_scheduler;
	Sense::Gpu							 m_gpu;
	Sense::ExperienceIdentity::Canvas	 m_canvas;
	Sense::Mouse						 m_mouse;
	Sense::Keyboard						 m_keyboard;

	CoregistrationControlWrapper		 m_coRegControl;
	CoregUiAdapter&						 m_coregGui;
};

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenFmStateIsIfr_ThenTheFmLabelShowsIFR)
{
	CoRegistrationServerState coRegState;
	coRegState.fmState = CoRegistrationServerState::FmState::IFRPullback;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_EQ(getCoRegUi().fmTypeLabel->text, L"iFR");

	coRegState.fmState = CoRegistrationServerState::FmState::IFRSpot;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_EQ(getCoRegUi().fmTypeLabel->text, L"iFR");
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenFmStateIsFFR_ThenTheFmLabelShowsFFR)
{
	CoRegistrationServerState coRegState;
	coRegState.fmState = CoRegistrationServerState::FmState::FFR;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_EQ(getCoRegUi().fmTypeLabel->text, L"FFR");
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsPullback_ThenTheFmDisplayIsVisible)
{
	switchToPullback();

	ASSERT_TRUE( getCoRegControler().isFmDisplayVisible());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsSpot_ThenTheFmDisplayIsHidden)
{
	CoRegistrationServerState coRegState;
	coRegState.systemState = CoRegistrationServerState::SystemState::SpotMeasurement;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_FALSE( getCoRegControler().isFmDisplayVisible());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsSpotMeasurement_ThenTheFFRValuesAreHighLighted)
{
	CoRegistrationServerState coRegState;
	coRegState.systemState = CoRegistrationServerState::SystemState::SpotMeasurement;
	coRegState.fmState = CoRegistrationServerState::FmState::FFR;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_TRUE( getCoRegControler().isIfrValuesHighLighted());
}


TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsNotSpotMeasurement_ThenTheFFRValuesAreNotHighLighted)
{
	CoRegistrationServerState coRegState;
	coRegState.systemState = CoRegistrationServerState::SystemState::Review;
	coRegState.fmState = CoRegistrationServerState::FmState::IFRSpot;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_FALSE( getCoRegControler().isIfrValuesHighLighted());
}


TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsReviewAndFmStateIsIFRSpot_ThenTheSpotReviewPanelVisible)
{
	CoRegistrationServerState coRegState;
	coRegState.systemState = CoRegistrationServerState::SystemState::Review;
	coRegState.fmState = CoRegistrationServerState::FmState::IFRSpot;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_TRUE( getCoRegControler().isIfrSpotReviewPanelVisible());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsNotReviewAndFmStateIsIFRSpot_ThenTheSpotReviewPanelNotVisible)
{
	CoRegistrationServerState coRegState;
	coRegState.systemState = CoRegistrationServerState::SystemState::Pullback;
	coRegState.fmState = CoRegistrationServerState::FmState::IFRSpot;
	
	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_FALSE( getCoRegControler().isIfrSpotReviewPanelVisible());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsReview_FmStateIFRAndPullbackStarted_ThenTheIfrReviewPanelIsVisible)
{
	CoRegistrationServerState coRegState;

	coRegState.systemState = CoRegistrationServerState::SystemState::Pullback;
	 getCoRegControler().setCoRegState(coRegState);

	coRegState.systemState = CoRegistrationServerState::SystemState::Review;
	coRegState.fmState = CoRegistrationServerState::FmState::IFRPullback;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_TRUE( getCoRegControler().isIfrPullbackReviewPanelVisible());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnected_WhenSystemStateIsPullbackAndFmStateIFR_ThenTheIfrReviewPanelIsHidden)
{
	CoRegistrationServerState coRegState;
	coRegState.systemState = CoRegistrationServerState::SystemState::Pullback;
	coRegState.fmState = CoRegistrationServerState::FmState::IFRPullback;

	 getCoRegControler().setCoRegState(coRegState);

	ASSERT_FALSE( getCoRegControler().isIfrPullbackReviewPanelVisible());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnectedInLive_WhenDisconnected_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	// Live data is common for FFR/iFR modes
	setPressureAndEcg();
	setPdPaInfo();

	ASSERT_FALSE( getCoRegControler().allGraphsEmpty());
	ASSERT_FALSE( getCoRegControler().allValuesInvalid());

	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnectedInLive_WhenSwitchedToFfrRecordingAndDisconnected_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	switchToFfrRecording();

	// Add FFR spot measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();

	ASSERT_FALSE( getCoRegControler().allGraphsEmpty());
	ASSERT_FALSE( getCoRegControler().allValuesInvalid());

	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationReconnected_WhenSwitchedToFfrRecording_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);
	 getCoRegControler().setCoRegConnection(true);

	// Add FFR spot measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();

	switchToFfrRecording();

	// Add FFR spot measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnectedInLive_WhenSwitchedToIfrRecordingAndDisconnected_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	switchToIfrSpotRecording();

	// Add iFR measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();
	setIfrSpotCycle();

	ASSERT_FALSE( getCoRegControler().allGraphsEmpty());
	ASSERT_FALSE( getCoRegControler().allValuesInvalid());

	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationReconnected_WhenSwitchedToIfrRecording_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);
	 getCoRegControler().setCoRegConnection(true);

	// Add iFR measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();
	setIfrSpotCycle();

	switchToIfrSpotRecording();

	// Add iFR measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();
	setIfrSpotCycle();

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationConnectedInLive_WhenSwitchedToIfrPullbackAndDisconnected_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	switchToPullback();

	// Add iFR pullback measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();
	setIfrPullbackCycle();

	ASSERT_FALSE( getCoRegControler().allGraphsEmpty());
	ASSERT_FALSE( getCoRegControler().allValuesInvalid());

	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}

TEST_F(TestCoregistrationControl, GivenCoregistrationReconnected_WhenSwitchedToIfrPullback_ThenTheGraphsAreEmptyAndValuesInvalidated)
{
	switchToIdle();
	 getCoRegControler().setCoRegConnection(false);
	 getCoRegControler().setCoRegConnection(true);

	// Add iFR pullback measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();
	setIfrPullbackCycle();

	switchToPullback();

	// Add iFR pullback measurement data
	setPressureAndEcg();
	setPdPaInfo();
	setSpotMeasurement();
	setIfrPullbackCycle();

	ASSERT_TRUE( getCoRegControler().allGraphsEmpty());
	ASSERT_TRUE( getCoRegControler().allValuesInvalid());
}
}}
//TICS +OLC#009  magic numbers for a test are ok.
