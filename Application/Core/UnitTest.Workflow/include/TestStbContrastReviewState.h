// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbContrastReviewState.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"
#include "MockIArchivingSettings.h"
#include "MockIPACSArchiver.h"
#include "PACSArchiver.h"

namespace Pci { namespace Core
{

	class TestStbContrastReviewState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
		TestStbContrastReviewState() :
			TestUiState(),
			archivingSettings(userPrefs.getArchivingSettings()),
			lastUsedSeriesNumber(0)
		{}

		void SetUp() override
		{
			EXPECT_CALL(copyToRefClient, subscribeConnectionEvents(STATE_NAME, testing::_)).Times(1);
			EXPECT_CALL(copyToRefClient, unsubscribeConnectionEvents(STATE_NAME)).Times(1);
			EXPECT_CALL(archivingSettings, getCopyToRefTargetNumber(testing::_, testing::_)).WillRepeatedly(testing::Return(1));

			state = std::unique_ptr<StbContrastReviewState>(new StbContrastReviewState(viewContext, copyToRefClient, userPrefs, uiStateController));
			state->setArchivingStatusSink(this);
			state->initPACSArchiving(std::unique_ptr<PACS::IPACSArchiver>(new ::testing::NiceMock<PACS::MockIPACSArchiver>()));
		}

		void enterState()
		{
			state->onEnter();
		}

		void archiveSnapshot()
		{
			state->getTsmPage().onButtonRelease(state->getTsmPage().SnapshotButton);
		}

		void archiveMovie()
		{
			state->getTsmPage().onButtonRelease(state->getTsmPage().MovieButton);
		}

		void copyToRef()
		{
			state->getTsmPage().onButtonRelease(state->getTsmPage().CopyToRefButton);
		}

        void pauseResumeFading()
        {
            state->getTsmPage().onButtonRelease(state->getTsmPage().StopStartButton);
        }

        void activateContrastFrameSelection()
        {
            state->getTsmPage().onButtonRelease(state->getTsmPage().ContrastFrameSelectButton);
        }

        void activateBoostFrameSelection()
        {
            state->getTsmPage().onButtonRelease(state->getTsmPage().BoostFrameSelectButton);
        }
        
		void failSnapshotArchive()
		{
			state->onFrameGrabberFinished(Pci::Core::PACS::GrabType::SingleFrame, false);
		}

		void successSnapshotArchive()
		{
			state->onFrameGrabberFinished(Pci::Core::PACS::GrabType::SingleFrame, true);
		}

		void failMovieArchive()
		{
			state->onMovieGrabberFinished(false, nullptr);
		}

		void succesMovieArchive()
		{
			state->onMovieGrabberFinished(true, nullptr);
		}

		bool isBoostedImageInverted() const
		{
			return state->m_boostInverted;
		}

		bool isFadingPaused() const
		{
			return state->m_fadeInOutPaused;
		}

		PACS::MockIPACSArchiver& getMockArchiver() const
		{
			return *static_cast<PACS::MockIPACSArchiver*>(state->m_pacsArchiver.get());
		}

		const StbTsmPage& getTsmPage()
		{
			return state->getTsmPage();
		}

		const std::shared_ptr<const StbRecording> getRecording()
		{
			return state->m_stbRecording;
		}

		int createValidStentBoostPostDeployRun(int numberOfFrames)
		{
			int runIndex = ++lastUsedSeriesNumber;
			for (int i = 0; i < numberOfFrames; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, true);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}

			return runIndex;
		}

		int createValidStentBoostSubtractRun(int numberOfFrames)
		{
			int runIndex = ++lastUsedSeriesNumber;
			for (int i = 0; i < numberOfFrames / 2; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, true);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}

			for (int i = 0; i < numberOfFrames / 2; i++)
			{
				auto result = createStbResult(runIndex, i + (numberOfFrames / 2), 720, 720, true, true);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}

			return runIndex;
		}

		void createInvalidStentBoostPostDeployRun(int numberOfFrames)
		{
			int runIndex = ++lastUsedSeriesNumber;
			for (int i = 0; i < numberOfFrames; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, false);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}
		}

		void createInvalidStentBoostSubtractRun(int numberOfFrames)
		{
			int runIndex = ++lastUsedSeriesNumber;
			for (int i = 0; i < numberOfFrames / 2; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, false);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}

			for (int i = 0; i < numberOfFrames / 2; i++)
			{
				auto result = createStbResult(runIndex, i + (numberOfFrames / 2), 720, 720, true, true);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}
		}

		const std::wstring STATE_NAME = L"StbContrastReviewState";

		::testing::NiceMock<MockUiStateController> uiStateController;
		testing::StrictMock<MockICopyToRefClient> copyToRefClient;
		const MockIArchivingSettings &archivingSettings ;
		std::unique_ptr<StbContrastReviewState> state;

		int lastUsedSeriesNumber;
	};

	TEST_F(TestStbContrastReviewState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbContrastReviewState, GivenLastImageHold_WhenCopyToRefEnabledForRef2_ThenSnapshotIsSentWithCopyToRefToRef2)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref2");

		EXPECT_CALL(archivingSettings, isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(archivingSettings, isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbContrastReviewState, GivenLastImageHold_WhenCopyToRefEnabledForRef1_ThenSnapshotIsSentWithCopyToRefToRef1)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));
		
		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbContrastReviewState, GivenLastImageHold_WhenCopyToRefDisabled_ThenNoSnapshotIsSent)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(testing::_)).Times(0);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotTriggerd_WhenArchivingSuccessful_ThenSnapshotButtonIsDisabledUntilSuccessReported)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createInvalidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		successSnapshotArchive();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotTriggerd_WhenArchivingFailed_ThenSnapshotButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createInvalidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		failSnapshotArchive();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveMovieTriggerd_WhenArchivingSuccessful_ThenMovieButtonIsDisabledUntilSuccessReported)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createInvalidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		succesMovieArchive();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveMovieTriggerd_WhenArchivingFailed_ThenMovieButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createInvalidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		failMovieArchive();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotTriggerdAndNewRunWithoutMarkers_WhenArchivingFailed_ThenSnapshotButtonIsRemainDisabled)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(2).WillRepeatedly(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		createInvalidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		createInvalidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		failSnapshotArchive();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotSucceded_WhenInvertingStentColors_ThenSnapshotButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);
		
		getTsmPage().eventButtonReleased(StbTsmPage::InvertBoostButton);

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveMovieSucceded_WhenInvertingStentColors_ThenMovieButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));
		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		getTsmPage().eventButtonReleased(StbTsmPage::InvertBoostButton);

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotSucceded_WhenPauseFading_ThenSnapshotButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		getTsmPage().eventButtonReleased(StbTsmPage::StopStartButton);

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotSucceded_WhenToBoostedPressed_ThenSnapshotButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		getTsmPage().eventButtonReleased(StbTsmPage::ToBoostButton);

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveSnapshotSucceded_WhenToContrastPressed_ThenSnapshotButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		getTsmPage().eventButtonReleased(StbTsmPage::ToContrastButton);

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenNoContrastDetected_ThenMovieButtonIsDisabled)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
	}
	

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenContrastDetected_ThenMovieButtonIsEnabled)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenContrastDetected_ThenTwoSnapshotsAreSentToPACS)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		const auto SERIES_NUMBER = createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, SERIES_NUMBER, RunType::SBL)).Times(2);

		enterState();
		archiveSnapshot();
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenArchivingWhileFading_ThenViewerResumesFadingAfterArchiving)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		const auto SERIES_NUMBER = createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, SERIES_NUMBER, RunType::SBL)).Times(2);

		enterState();

		EXPECT_FALSE(viewContext.stbContrastViewer.isPaused());
		EXPECT_FALSE(isFadingPaused());

		archiveSnapshot();

		EXPECT_FALSE(viewContext.stbContrastViewer.isPaused());
		EXPECT_FALSE(isFadingPaused());
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenArchivingWhilePaused_ThenViewerRemainsPausedAfterArchiving)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const double PAUSED_BLEND_RATIO(0.33);

		const auto SERIES_NUMBER = createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, SERIES_NUMBER, RunType::SBL)).Times(1);

		enterState();
		pauseResumeFading();

		viewContext.stbContrastViewer.setFixedBlend(PAUSED_BLEND_RATIO);

		EXPECT_TRUE(viewContext.stbContrastViewer.isPaused());
		EXPECT_TRUE(isFadingPaused());

		archiveSnapshot();

		EXPECT_TRUE(viewContext.stbContrastViewer.isPaused());
		EXPECT_TRUE(isFadingPaused());
		EXPECT_EQ(PAUSED_BLEND_RATIO, viewContext.stbContrastViewer.getBlendRatio());
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenNoContrastDetected_ThenOneSnapshotsIsSentToPACS)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		const int SERIES_NUMBER = createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, SERIES_NUMBER, RunType::SBL)).Times(1);

		enterState();
		archiveSnapshot();
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRunWithNoBoostedImages_WhenEnteringTheState_ThenBoostedImagesIsZero)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		createInvalidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		enterState();

		EXPECT_EQ(0, viewContext.stbContrastViewer.boostedFrames);
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenNoContrastDetected_ThenContrastRelatedTSMButtonsAreDisabled)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		enterState();

		EXPECT_FALSE(getTsmPage().toBoostButton->enabled);
		EXPECT_FALSE(getTsmPage().toContrastButton->enabled);
		EXPECT_FALSE(getTsmPage().stopStartButton->enabled);
		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
		EXPECT_FALSE(getTsmPage().contrastFrameSelectButton->enabled);
		EXPECT_TRUE(getTsmPage().boostFrameSelectButton->enabled);
		EXPECT_TRUE(getTsmPage().invertBoostButton->enabled);
		EXPECT_TRUE(getTsmPage().defineRoiButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenMoreThanOneContrastDetected_ThenContrastRelatedTSMButtonsAreEnabled)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

        createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

        enterState();

		EXPECT_TRUE(getTsmPage().toBoostButton->enabled);
		EXPECT_TRUE(getTsmPage().toContrastButton->enabled);
		EXPECT_TRUE(getTsmPage().stopStartButton->enabled);
		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
		EXPECT_TRUE(getTsmPage().contrastFrameSelectButton->enabled);
		EXPECT_TRUE(getTsmPage().boostFrameSelectButton->enabled);
	}

    TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRunWithMoreThanOneContrastDetected_WhenContrastFrameSelectionButtonIsPressed_ThenContrastFrameSelectionStateIsActive)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);

        createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

        EXPECT_CALL(uiStateController, switchState(ViewState::StbContrastFrameSelectionState)).Times(1);

        enterState();

        activateContrastFrameSelection();
    }

    TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRunWithMoreThanOneContrastDetected_WhenBoostFrameSelectionButtonIsPressed_ThenBoostFrameSelectionStateIsActive)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);

        createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

        EXPECT_CALL(uiStateController, switchState(ViewState::StbBoostFrameSelectionState)).Times(1);

        enterState();

        activateBoostFrameSelection();
    }

    TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenNoCopyToRefClientAvailable_ThenCopyToRefButtonIsNotVisible)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);

        createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

        enterState();

        EXPECT_FALSE(getTsmPage().copyToRefButton->visible);
    }

    TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenCopyToRefClientAvailable_ThenCopyToRefButtonIsVisible)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);

        createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

        enterState();

        EXPECT_TRUE(getTsmPage().copyToRefButton->visible);
    }

    TEST_F(TestStbContrastReviewState, GivenLastImageHold_WhenCopyToRefEnabledForRef1_ThenTSMButtonShowsReference1)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");
		const std::wstring TSM_TEXT(L"Copy to\nReference 1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_EQ(TSM_TEXT, getTsmPage().copyToRefButton->centerText);
	}

	TEST_F(TestStbContrastReviewState, GivenLastImageHold_WhenCopyToRefClickedWhileFading_ThenFadingResumesAfterCopyToRef)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));
		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_FALSE(viewContext.stbContrastViewer.isPaused());
		EXPECT_FALSE(isFadingPaused());

		copyToRef();

		EXPECT_FALSE(viewContext.stbContrastViewer.isPaused());
		EXPECT_FALSE(isFadingPaused());
	}

	TEST_F(TestStbContrastReviewState, GivenLastImageHold_WhenCopyToRefClickedWhilePaused_ThenFadingRemainsPausedAfterCopyToRef)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");
		const double PAUSED_BLEND_RATIO(0.33);

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));
		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);

		createValidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
		pauseResumeFading();

		viewContext.stbContrastViewer.setFixedBlend(PAUSED_BLEND_RATIO);

		EXPECT_TRUE(viewContext.stbContrastViewer.isPaused());
		EXPECT_TRUE(isFadingPaused());

		copyToRef();

		EXPECT_TRUE(viewContext.stbContrastViewer.isPaused());
		EXPECT_TRUE(isFadingPaused());
		EXPECT_EQ(PAUSED_BLEND_RATIO, viewContext.stbContrastViewer.getBlendRatio());
	}

	TEST_F(TestStbContrastReviewState, GivenBoostImage_WhenPressInvert_ThenImageIsInverted)
	{
		const std::wstring REF_VIEWER(L"Ref1");

		std::shared_ptr<StbImage> boostImage = std::make_shared<StbImage>();
		std::shared_ptr<StbMarkers> markers = std::make_shared<StbMarkers>();

		boostImage->resize(200, 200);
		boostImage->imageIndex = 1;
		boostImage->phase = StbImagePhase::boostPhase;

		markers->p1.x = markers->p1.y = 80;
		markers->p2.x = markers->p2.y = 120;
		markers->valid = true;
		boostImage->refMarkers = *markers;

		short value = 0;
		for (int y = 0; y < boostImage->height; y++)
		{
			value++;
			for (int x = 0; x < boostImage->stride; x++)
			{
				boostImage->pointer[x + y *boostImage->stride] = value;

			}
		}
		stbAdmin.setImage(boostImage, markers, boostImage);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		enterState();

		ASSERT_FALSE(isBoostedImageInverted());

		getTsmPage().eventButtonReleased(StbTsmPage::InvertBoostButton);
		stbContrastViewer.setFixedBlend(1.0f);

		auto &invetedImage = stbContrastViewer.getImage();

		short maxval = 0;
		for (int i = 0; i < boostImage->height* boostImage->stride; i++)
		{
			maxval = std::max(maxval, boostImage->pointer[i]);
		}

		// check between the markers 5 pixels around 
		for (int y = static_cast<int>(markers->p1.y); y < static_cast<int>(markers->p2.y); y++)
		{
			for (int i = 0; i < 5; i++)
			{
				int yp = y - i;
				int xp = y + i;

				EXPECT_EQ(boostImage->pointer[xp + boostImage->stride *yp], maxval - invetedImage->pointer[xp + invetedImage->stride * yp]);
			}
		}
	}

	TEST_F(TestStbContrastReviewState, GivenInState_WhenStentBoostAvailableEventTriggered_ThenImagesAreNotAddedToAdministration)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
		pauseResumeFading();

		auto stbResult = createStbResult(1, 0, 100, 100, true);

		auto initialRecording = getRecording();
		auto initialVisibleBoostImage = viewContext.stbContrastViewer.getImage();

		EXPECT_EQ(ViewState::StbContrastReviewState, state->stentBoostImageAvailable(stbResult.live, stbResult.markers, stbResult.boost));
		EXPECT_EQ(ViewState::StbContrastReviewState, state->boostImageAvailable(stbResult.boost, stbResult.registeredLive));

		auto newRecording = stbAdmin.prepareReview();

		EXPECT_EQ(initialRecording, newRecording);
		EXPECT_EQ(initialRecording->getImageCount(), newRecording->getImageCount());
		
		EXPECT_EQ(viewContext.stbContrastViewer.getImage(), initialVisibleBoostImage);
	}

	TEST_F(TestStbContrastReviewState, GivenStentBoostSubtractRun_WhenNoMarkersFoundInRun_ThenOnlyAdjustROIButtonIsEnabledOnTSM)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		createInvalidStentBoostPostDeployRun(NUMBER_OF_FRAMES_IN_RUN);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));

		enterState();

		EXPECT_TRUE(getTsmPage().defineRoiButton->enabled);
		EXPECT_FALSE(getTsmPage().toBoostButton->enabled);
		EXPECT_FALSE(getTsmPage().toContrastButton->enabled);
		EXPECT_FALSE(getTsmPage().stopStartButton->enabled);
		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);
		EXPECT_FALSE(getTsmPage().invertBoostButton->enabled);
		EXPECT_FALSE(getTsmPage().contrastFrameSelectButton->enabled);
		EXPECT_FALSE(getTsmPage().boostFrameSelectButton->enabled);
	}

	TEST_F(TestStbContrastReviewState, GivenRecordingAlreadyAutoArchived_WhenEnteringState_ThenNotAutoArchivedAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref2");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(3).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, ::testing::_, RunType::SBL)).Times(2);
		EXPECT_CALL(messageBuilder, StartMovie(::testing::_, ::testing::_, ::testing::_, ::testing::_, RunType::SBL)).Times(1);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(2).WillRepeatedly(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(2).WillRepeatedly(testing::Return(true));

		createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		// Enter state for the second time; this simulates coming back from for instance DCR overlay
		enterState();
	}

	TEST_F(TestStbContrastReviewState, GivenRecordingNotAutoArchivedAndAutoArchivedEnabled_WhenEnteringState_ThenAutoArchivingPerformed)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref2");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, ::testing::_, RunType::SBL)).Times(2);
		EXPECT_CALL(messageBuilder, StartMovie(::testing::_, ::testing::_, ::testing::_, ::testing::_, RunType::SBL)).Times(1);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbContrastReviewState, GivenArchiveMovieFailed_WhenArchiveMovieButtonPressedAgain_ThenMovieButtonIsDisabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(2).WillRepeatedly(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostSubtractRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		failMovieArchive();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
	}

    TEST_P(TestStbContrastReviewState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
    {
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
    }

    INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbContrastReviewState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap),
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));
}}