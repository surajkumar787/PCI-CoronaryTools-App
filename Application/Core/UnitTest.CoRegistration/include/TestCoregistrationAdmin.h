// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <memory>

#include <gtest/gtest.h>
#include <Sense/System/Scheduler.h>

#include "CoRegistrationAdministration.h"
#include "CoRegistrationServerState.h"
#include "PullbackRecording.h"
#include "PciReviewPlayer.h"
#include "PciEpx.h"
#include "CrmOverlay.h"


//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;

class TestCoregistrationAdmin : public ::testing::Test
{
public:

	TestCoregistrationAdmin() :
		m_reviewParams(-1, 0.0, 0, 600),
		m_player(m_scheduler, m_reviewParams),
		m_coRegAdmin(m_player)
	{
		m_coRegAdmin.eventReviewRecording = [this](const std::shared_ptr<PullbackRecording>& recording)
		{
			m_player.setReview(recording, 1.0 / 15.0);
		};
	}

	~TestCoregistrationAdmin()
	{
	}

	void SetUp() override
	{
		Test::SetUp();

		m_coRegAdmin.setCoRegReady(true);

		switchToIfrLive();
	}

	void setCoRegNotReady()
	{
		m_coRegAdmin.setCoRegReady(false);
		switchToIdle();
	}

	void switchToIdle()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::FFR;
		state.systemState = CoRegistrationServerState::SystemState::Idle;

		m_coRegAdmin.setCoRegState(state);
	}

	void switchToFfrLive()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::FFR;
		state.systemState = CoRegistrationServerState::SystemState::Live;

		m_coRegAdmin.setCoRegState(state);
	}

	void switchToIfrLive()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::IFRSpot;
		state.systemState = CoRegistrationServerState::SystemState::Live;

		m_coRegAdmin.setCoRegState(state);
	}

	void switchToIfrSpotRecording()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::IFRSpot;
		state.systemState = CoRegistrationServerState::SystemState::SpotMeasurement;

		m_coRegAdmin.setCoRegState(state);
	}

	void switchToPullback()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::IFRPullback;
		state.systemState = CoRegistrationServerState::SystemState::Pullback;

		m_coRegAdmin.setCoRegState(state);
	}

	void switchToReview()
	{
		CoRegistrationServerState state;
		state.fmState = CoRegistrationServerState::FmState::IFRPullback;
		state.systemState = CoRegistrationServerState::SystemState::Review;

		m_coRegAdmin.setCoRegState(state);
	}

	void sendImage()
	{
		std::shared_ptr<const CommonPlatform::Xray::XrayImage>  image = std::make_shared<CommonPlatform::Xray::XrayImage>();
		std::shared_ptr<const CrmOverlay> overlay = std::make_shared<CrmOverlay>();

		m_coRegAdmin.setImage(image, overlay);
	}

	void switchStudy()
	{
		CommonPlatform::Xray::XrayStudy study;
		m_coRegAdmin.setStudy(study);
	}

	void sendIfrPullbackCycle()
	{
		CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle cycle;
		m_coRegAdmin.setIfrPullbackCycle(cycle);
	}

	void sendIfrPullbackFilteredCycle()
	{
		CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle cycle;
		cycle.isFiltered = true;
		m_coRegAdmin.setIfrPullbackCycle(cycle);
	}

	void sendIfrSpotCycle()
	{
		CommonPlatform::CoRegistration::CoRegistrationIfrSpotCycle spotCycle;
		m_coRegAdmin.setIfrSpotCycle(spotCycle);
	}

	void sendPressureAndEcg()
	{
		CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg pressAndEcg;
		m_coRegAdmin.setPressureAndEcg(pressAndEcg);
	}

	void sendPdPaInfo()
	{
		CoRegistrationPdPaInfo pdPaInfo;
		m_coRegAdmin.setPdPaInfo(pdPaInfo);
	}

	void sendSpotMeasurement()
	{
		// Do nothing; FFR spot data is not routed to admin
	}

	bool recordingIsEmpty()
	{
		auto recording = m_coRegAdmin.getRecording();
		return !recording ||
			(recording->getImageCount() == 0 &&
				recording->getAllIfrPullbackCycles().size() == 0 &&
				recording->getAllIfrSpotCycles().size() == 0 &&
				recording->getAllPressureAndEcg().size() == 0);
	}

	CoRegistrationAdministration& getCoRegAdmin() { return m_coRegAdmin; };
	bool isPlayerInReview() {return m_player.inReview(); };

	private:
		Sense::Scheduler					m_scheduler;
		CoRegistrationAdministration		m_coRegAdmin;
		PciReviewParameters					m_reviewParams;
		PciReviewPlayer<PullbackRecording>	m_player;
};

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReady_WhenAPullbackIsStarted_ThenARecordingIsStarted)
{
	switchToPullback();
	ASSERT_TRUE(getCoRegAdmin().isRecording());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingStarted_WhenAPullbackIsStopped_ThenARecordingIsStopped)
{
	switchToPullback();
	ASSERT_TRUE(getCoRegAdmin().isRecording());

	switchToReview();
	ASSERT_FALSE(getCoRegAdmin().isRecording());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingStarted_WhenImageReceived_ThenTheImageIsAddedToTheRecording)
{
	switchToPullback();

	sendImage();

	std::shared_ptr<PullbackRecording> recording = getCoRegAdmin().getRecording();
	ASSERT_TRUE(recording->getImageCount() > 0);
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingStarted_WhenStudyChanged_ThenTheRecordingIsStoppedAndCleared)
{
	switchToPullback();
	sendImage();

	std::shared_ptr<PullbackRecording> recording = getCoRegAdmin().getRecording();
	ASSERT_TRUE(recording->getImageCount() > 0);

	switchStudy();
	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recording->getImageCount() == 0);
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingStarted_WhenAIfrNonFilteredCycleIsReceived_ThenTheRecordingIsNotUpdated)
{
	// The iFR pullback recording contains only _filtered_ measurements
	switchToPullback();
	sendIfrPullbackCycle();

	std::shared_ptr<PullbackRecording> recording = getCoRegAdmin().getRecording();
	ASSERT_TRUE(recording->getAllIfrPullbackCycles().size() == 0);
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingStarted_WhenAIfrFilteredCycleIsReceived_ThenTheRecordingIsUpdated)
{
	// The iFR pullback recording contains only _filtered_ measurements
	switchToPullback();
	sendIfrPullbackCycle();

	ASSERT_TRUE(getCoRegAdmin().getRecording()->getAllIfrPullbackCycles().size() == 0);

	sendIfrPullbackFilteredCycle();

	ASSERT_TRUE(getCoRegAdmin().getRecording()->getAllIfrPullbackCycles().size() > 0);
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingNotStarted_WhenPullbackDataIsReceived_ThenTheRecordingIsNotUpdated)
{
	sendIfrPullbackCycle();
	sendIfrSpotCycle();
	sendPressureAndEcg();
	sendImage();

	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyAndRecordingStarted_WhenSwitchedToReview_ThenReviewIsStarted)
{
	switchToPullback();
	sendImage();

	switchToReview();

	ASSERT_TRUE(isPlayerInReview());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationNotReady_WhenSwitchedToIfrRecording_ThenTheRecordingIsNotUpdated)
{
	setCoRegNotReady();

	switchToIfrSpotRecording();

	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationNotReady_WhenSwitchedToIfrPullback_ThenTheRecordingIsNotUpdated)
{
	setCoRegNotReady();

	switchToPullback();

	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrPullbackCycle();

	sendImage();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyInIFRLive_WhenPressureDataReceived_ThenTheRecordingIsNotUpdated)
{
	switchToIfrLive();
	sendPressureAndEcg();
	sendPdPaInfo();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyInFFRLive_WhenPressureDataReceived_ThenTheRecordingIsNotUpdated)
{
	switchToFfrLive();
	sendPressureAndEcg();
	sendPdPaInfo();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyInLive_WhenSwitchedToIfrRecordingAndDisconnected_ThenTheRecordingIsStoppedAndCleared)
{
	switchToIfrSpotRecording();

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	ASSERT_TRUE(getCoRegAdmin().isRecording());
	ASSERT_FALSE(recordingIsEmpty());

	setCoRegNotReady();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReconnected_WhenSwitchedToIfrRecording_ThenTheRecordingIsNotUpdated)
{
	setCoRegNotReady();
	getCoRegAdmin().setCoRegReady(true);

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	switchToIfrSpotRecording();

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationWasNotReadyAndBecameReadyInLiveMode_WhenSwitchedToIfrRecording_ThenTheRecordingIsNotUpdated)
{
	setCoRegNotReady();
	switchToIfrLive();
	getCoRegAdmin().setCoRegReady(true);

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	switchToIfrSpotRecording();

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	ASSERT_TRUE(getCoRegAdmin().isRecording());
	ASSERT_FALSE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationWasNotReadyAndBecameLiveWhenReady_WhenSwitchedToIfrRecording_ThenTheRecordingIsNotUpdated)
{
	setCoRegNotReady();
	getCoRegAdmin().setCoRegReady(true);
	switchToIfrLive();

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	switchToIfrSpotRecording();

	// Add iFR spot measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrSpotCycle();

	ASSERT_TRUE(getCoRegAdmin().isRecording());
	ASSERT_FALSE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReadyInLive_WhenSwitchedToIfrPullbackAndDisconnected_ThenTheRecordingIsStoppedAndCleared)
{
	switchToPullback();

	// Add iFR pullback measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrPullbackCycle();
	sendImage();

	ASSERT_TRUE(getCoRegAdmin().isRecording());
	ASSERT_FALSE(recordingIsEmpty());

	setCoRegNotReady();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReconnected_WhenSwitchedToIfrPullback_ThenTheRecordingIsNotUpdated)
{
	setCoRegNotReady();
	getCoRegAdmin().setCoRegReady(true);

	// Add iFR pullback measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrPullbackCycle();
	sendImage();

	switchToPullback();

	// Add iFR pullback measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrPullbackCycle();
	sendImage();

	ASSERT_FALSE(getCoRegAdmin().isRecording());
	ASSERT_TRUE(recordingIsEmpty());
}

TEST_F(TestCoregistrationAdmin, GivenCoregistrationReconnectedInLiveMode_WhenSwitchedToIfrPullback_ThenTheRecordingIsUpdated)
{
	setCoRegNotReady();
	getCoRegAdmin().setCoRegReady(true);
	switchToIfrLive();

	// Add iFR pullback measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrPullbackCycle();
	sendImage();

	switchToPullback();

	// Add iFR pullback measurement data
	sendPressureAndEcg();
	sendPdPaInfo();
	sendSpotMeasurement();
	sendIfrPullbackCycle();
	sendImage();

	ASSERT_TRUE(getCoRegAdmin().isRecording());
	ASSERT_FALSE(recordingIsEmpty());
}
}}
//TICS +OLC#009  magic numbers for a test are ok.