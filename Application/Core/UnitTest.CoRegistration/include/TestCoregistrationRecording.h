// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "gtest/gtest.h"
#include "Sense/System/Scheduler.h"
#include "PullbackRecording.h"
#include "PciReviewPlayer.h"
#include <memory>

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace Pci::Core;
using namespace CommonPlatform::CoRegistration;


class TestCoregistrationRecording : public ::testing::Test
{
public:
	const int	 m_maxImages;
	const int	 m_justOverMaxImages;
	const double m_imageFrameTime;

	TestCoregistrationRecording() :
		m_reviewParams(-1, 0.0, 0, 600),
		m_imageNumber(1),
		m_maxImages(10),
		m_imageFrameTime(1/15),
		m_justOverMaxImages(m_maxImages + 1),
		m_timeoutTimer(m_scheduler, [&](){ timeoutTimerTick(); })
	{
	}

	~TestCoregistrationRecording()
	{
	}

	void SetUp() override
	{
		Test::SetUp();
		m_imageNumber = 1;
		m_recording = std::make_shared<PullbackRecording>(m_maxImages);
		m_player.reset(new PciReviewPlayer<PullbackRecording>(m_scheduler, m_reviewParams));
		m_player->setReview(m_recording, m_imageFrameTime);
	}

	void timeoutTimerTick()
	{
		m_scheduler.exit();
	}

	void sendImage()
	{
		auto image	 = std::make_shared<CommonPlatform::Xray::XrayImage>();
		auto overlay = std::make_shared<CrmOverlay>();

		image->frameTime = m_imageFrameTime;
		image->imageNumber = m_imageNumber++;
		m_recording->addImage(image, overlay);
	}

	void sendIfrPullbackCycle()
	{
		CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle cycle;
		m_recording->addiFrPullbackCycle(cycle);
	}

	void sendIfrSpotCycle()
	{
		CommonPlatform::CoRegistration::CoRegistrationIfrSpotCycle spotCycle;
		m_recording->addiFrSpotCycle(spotCycle);
	}

	void sendPressureAndEcg()
	{
		CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg pressAndEcg;
		m_recording->addPressureAndEcg(pressAndEcg);
	}

	void setRunIndex(int runIndex)
	{
		m_recording->setRunIndex(runIndex);
	}

	float calculatedExpectedIfrValue(float lowerIfrValue, double lowerTimeStamp, float upperIfrValue, double upperTimeStamp, double timeStamp)
	{
		double ifrDiff  = upperIfrValue - lowerIfrValue;
		double timeDiff = upperTimeStamp - lowerTimeStamp;
		double calculatedTimestamp = timeStamp - lowerTimeStamp;

		return static_cast<float>(((ifrDiff / timeDiff) * calculatedTimestamp) + lowerIfrValue);
	}

	std::shared_ptr<PullbackRecording> getRecording() { return m_recording; };
	void schedulerRun() { m_scheduler.run(); };
	void schedulerExit() { m_scheduler.exit(); };
	void setEventReviewImagePlayer(std::function<void(int, const PciReviewPlayer<PullbackRecording>::SingleFrame&, bool)> callback) { m_player->eventReviewImage = callback; };
	void startReview() { m_player->startReview(); };
	void previousFrame() { m_player->previousFrame(); };
	void nextFrame() { m_player->nextFrame(); };
	void stopReview() { m_player->stopReview(); };
	void startTimeOutTimer(int time) { m_timeoutTimer.start(time); };
	void stopTimeOutTimer() { m_timeoutTimer.stop(); };

private:
	Sense::Scheduler									m_scheduler;
	Sense::Timer										m_timeoutTimer;
	PciReviewParameters									m_reviewParams;
	std::unique_ptr<PciReviewPlayer<PullbackRecording>> m_player;
	std::shared_ptr<PullbackRecording>					m_recording;
	int													m_imageNumber;
};

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenARunIndexIsChanged_ThenTheRecordedRunIndexIsUpdated)
{
	int runIndex = 111;
	setRunIndex(runIndex);
	ASSERT_EQ(runIndex, getRecording()->getRunIndex());
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenARunIndexIsChangedTwice_ThenTheRecordedRunIndexIsUpdatedWithTheLatestId)
{
	int firstRunIndex = 111;
	setRunIndex(firstRunIndex);
	ASSERT_EQ(firstRunIndex, getRecording()->getRunIndex());

	int secondRunIndex = 112;
	setRunIndex(secondRunIndex);
	ASSERT_EQ(secondRunIndex, getRecording()->getRunIndex());
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenAIfrSpotCycleIsSent_ThenTheCycleIsRecorded)
{
	sendIfrSpotCycle();
	ASSERT_TRUE(getRecording()->getAllIfrSpotCycles().size() > 0);
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenAIfrPullbackCycleIsSent_ThenTheCycleIsRecorded)
{
	sendIfrPullbackCycle();
	ASSERT_TRUE(getRecording()->getAllIfrPullbackCycles().size() > 0);
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenPressureAndEcgIsSent_ThenThisDataIsRecorded)
{
	sendPressureAndEcg();
	ASSERT_TRUE(getRecording()->getAllPressureAndEcg().size() > 0);
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenAnImageIsSent_ThenThisImageIsRecorded)
{
	sendImage();
	ASSERT_TRUE(getRecording()->getImageCount() > 0);
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenARunIsRecorded_ThenNoMoreThanMaxImagesAreRecorded)
{

	for(int i = 0; i <= m_justOverMaxImages + 1; i++)
	{
		sendImage();
	}

	ASSERT_TRUE(getRecording()->getImageCount() == m_maxImages);
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenAReviewIsStarted_ThenAllImagesAreReplayedUsingImageFrametime)
{
	for(int i = 0; i < m_justOverMaxImages; i++)
	{
		sendImage();
	}

	int replayCount = 0;

	setEventReviewImagePlayer(
		[&] (int /*frameIndex*/, const PciReviewPlayer<PullbackRecording>::SingleFrame& /*frame*/, bool /*lastFrame*/)
	{
		replayCount++;
		if(replayCount == m_maxImages)
			schedulerExit();
	});

	startReview();

	startTimeOutTimer(5);
	schedulerRun();
	stopTimeOutTimer();

	stopReview();

	ASSERT_TRUE(replayCount == m_maxImages);
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenAReviewIsStoppedAndNextframePressed_ThenNextFrameIsSent)
{
	for(int i = 0; i < m_maxImages; i++)
	{
		sendImage();
	}

	int imageNumber     = 1;
	setEventReviewImagePlayer(
		[&] (int /*frameIndex*/, const PciReviewPlayer<PullbackRecording>::SingleFrame& frame, bool /*lastFrame*/)
	{
		ASSERT_EQ(frame.first->imageNumber, imageNumber);
		imageNumber++;
	});

	for(int i = 0; i < m_maxImages; i++)
	{
		nextFrame();
	}
}

TEST_F(TestCoregistrationRecording, GivenRecordingActive_WhenAReviewIsStoppedAndPreviousframePressed_ThenPreviousFrameIsSent)
{
	for(int i = 0; i < m_maxImages; i++)
	{
		sendImage();
	}

	int imageNumber = 10;

	setEventReviewImagePlayer(
		[&] (int /*frameIndex*/, const PciReviewPlayer<PullbackRecording>::SingleFrame& frame, bool /*lastFrame*/)
	{
		ASSERT_EQ(frame.first->imageNumber, imageNumber);
		imageNumber--;
	});

	for(int i = 0; i < m_maxImages; i++)
	{
		previousFrame();
	}
}

TEST_F(TestCoregistrationRecording, GivenPullbackRecorded_WhenIfrTrendValueAtASpecificTimeIsRequested_ThenTheCorrectIfrValueIsCalculated)
{
	CoRegistrationIfrPullbackCycle pullbackCycle1;
	pullbackCycle1.timestamp = 0.10;
	pullbackCycle1.ifrTrendValue = 0.3f;

	CoRegistrationIfrPullbackCycle pullbackCycle2;
	pullbackCycle2.timestamp = 0.20;
	pullbackCycle2.ifrTrendValue = 0.5f;

	CoRegistrationIfrPullbackCycle pullbackCycle3;
	pullbackCycle3.timestamp = 0.30;
	pullbackCycle3.ifrTrendValue = 0.7f;

	getRecording()->addiFrPullbackCycle(pullbackCycle1);
	getRecording()->addiFrPullbackCycle(pullbackCycle2);
	getRecording()->addiFrPullbackCycle(pullbackCycle3);


	double timeStampToVerify1 = 0.12;
	double timeStampToVerify2 = 0.15;
	double timeStampToVerify3 = 0.17;

	double timeStampToVerify4 = 0.23;
	double timeStampToVerify5 = 0.25;
	double timeStampToVerify6 = 0.29;

	float expectedIfrValue1 = calculatedExpectedIfrValue(pullbackCycle1.ifrTrendValue, pullbackCycle1.timestamp, pullbackCycle2.ifrTrendValue, pullbackCycle2.timestamp, timeStampToVerify1);
	float expectedIfrValue2 = calculatedExpectedIfrValue(pullbackCycle1.ifrTrendValue, pullbackCycle1.timestamp, pullbackCycle2.ifrTrendValue, pullbackCycle2.timestamp, timeStampToVerify2);
	float expectedIfrValue3 = calculatedExpectedIfrValue(pullbackCycle1.ifrTrendValue, pullbackCycle1.timestamp, pullbackCycle2.ifrTrendValue, pullbackCycle2.timestamp, timeStampToVerify3);

	float expectedIfrValue4 = calculatedExpectedIfrValue(pullbackCycle2.ifrTrendValue, pullbackCycle2.timestamp, pullbackCycle3.ifrTrendValue, pullbackCycle3.timestamp, timeStampToVerify4);
	float expectedIfrValue5 = calculatedExpectedIfrValue(pullbackCycle2.ifrTrendValue, pullbackCycle2.timestamp, pullbackCycle3.ifrTrendValue, pullbackCycle3.timestamp, timeStampToVerify5);
	float expectedIfrValue6 = calculatedExpectedIfrValue(pullbackCycle2.ifrTrendValue, pullbackCycle2.timestamp, pullbackCycle3.ifrTrendValue, pullbackCycle3.timestamp, timeStampToVerify6);

	float ifrValue1; float ifrValue2; float ifrValue3;
	float ifrValue4; float ifrValue5; float ifrValue6;

	ASSERT_TRUE(getRecording()->getIfrTrendValueAt(timeStampToVerify1, ifrValue1));
	ASSERT_TRUE(getRecording()->getIfrTrendValueAt(timeStampToVerify2, ifrValue2));
	ASSERT_TRUE(getRecording()->getIfrTrendValueAt(timeStampToVerify3, ifrValue3));
	ASSERT_TRUE(getRecording()->getIfrTrendValueAt(timeStampToVerify4, ifrValue4));
	ASSERT_TRUE(getRecording()->getIfrTrendValueAt(timeStampToVerify5, ifrValue5));
	ASSERT_TRUE(getRecording()->getIfrTrendValueAt(timeStampToVerify6, ifrValue6));

	ASSERT_EQ(expectedIfrValue1, ifrValue1);
	ASSERT_EQ(expectedIfrValue2, ifrValue2);
	ASSERT_EQ(expectedIfrValue3, ifrValue3);
	ASSERT_EQ(expectedIfrValue4, ifrValue4);
	ASSERT_EQ(expectedIfrValue5, ifrValue5);
	ASSERT_EQ(expectedIfrValue6, ifrValue6);
}

TEST_F(TestCoregistrationRecording, GivenPullbackRecorded_WhenDistalIfrValueIsRequested_ThenTheCorrectIfrValueIsReturned)
{
	CoRegistrationIfrPullbackCycle pullbackCycle1;
	pullbackCycle1.timestamp = 0.10;
	pullbackCycle1.ifrTrendValue = 0.3f;
	pullbackCycle1.isValidCycle = true;

	CoRegistrationIfrPullbackCycle pullbackCycle2;
	pullbackCycle2.timestamp = 0.20;
	pullbackCycle2.ifrTrendValue = 0.5f;
	pullbackCycle2.isValidCycle = true;

	CoRegistrationIfrPullbackCycle pullbackCycle3;
	pullbackCycle3.timestamp = 0.30;
	pullbackCycle3.ifrTrendValue = 0.7f;
	pullbackCycle3.isValidCycle = true;

	getRecording()->addiFrPullbackCycle(pullbackCycle1);
	getRecording()->addiFrPullbackCycle(pullbackCycle2);
	getRecording()->addiFrPullbackCycle(pullbackCycle3);

	float distalIFR;
	bool valid = getRecording()->getDistalFm(distalIFR);
	ASSERT_TRUE(valid);

	ASSERT_EQ(pullbackCycle1.ifrTrendValue, distalIFR);
}

TEST_F(TestCoregistrationRecording, GivenPullbackRecorded_WhenCleared_ThenRecordingIsEmpty)
{
	sendIfrPullbackCycle();
	sendIfrSpotCycle();
	sendPressureAndEcg();
	sendImage();

	ASSERT_TRUE(getRecording()->getAllIfrPullbackCycles().size() > 0);
	ASSERT_TRUE(getRecording()->getAllIfrSpotCycles().size()	 > 0);
	ASSERT_TRUE(getRecording()->getAllPressureAndEcg().size()	 > 0);
	ASSERT_TRUE(getRecording()->getImageCount()					 > 0);

	getRecording()->clear();

	ASSERT_TRUE(getRecording()->getAllIfrPullbackCycles().size() == 0);
	ASSERT_TRUE(getRecording()->getAllIfrSpotCycles().size()	 == 0);
	ASSERT_TRUE(getRecording()->getAllPressureAndEcg().size()	 == 0);
	ASSERT_TRUE(getRecording()->getImageCount()					 == 0);
}

TEST_F(TestCoregistrationRecording, GivenInvalidIfrPullbackCycleRecorded_WhenDistalFmIsRequested_ThenTheValidFlagIsFalse)
{
	CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle invalidCycle;
	invalidCycle.timestamp = 0.10;
	invalidCycle.ifrTrendValue = 0;
	invalidCycle.isValidCycle = false;
	getRecording()->addiFrPullbackCycle(invalidCycle);

	CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle validCycle;
	validCycle.timestamp = 0.20;
	validCycle.ifrTrendValue = 0.5f;
	validCycle.isValidCycle = true;
	getRecording()->addiFrPullbackCycle(validCycle);

	float distalIFR;
	bool valid = getRecording()->getDistalFm(distalIFR);

	ASSERT_FALSE(valid);
}
}}
//TICS +OLC#009  magic numbers for a test are ok.