// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbLiveReviewState.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"
#include "MockIArchivingSettings.h"

using namespace Pci::Core::PACS;
namespace Pci { namespace Core
{

	class TestStbLiveReviewState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
		TestStbLiveReviewState() :
			TestUiState(),
			lastUsedSeriesNumber(0)
		{}

		void SetUp() override
		{
			EXPECT_CALL(copyToRefClient, subscribeConnectionEvents(STATE_NAME, testing::_)).Times(1);
			EXPECT_CALL(copyToRefClient, unsubscribeConnectionEvents(STATE_NAME)).Times(1);

			state = std::unique_ptr<StbLiveReviewState>(new StbLiveReviewState(viewContext, copyToRefClient, archivingSettings));
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

		const StbTsmPage& getTsmPage()
		{
			return state->getTsmPage();
		}

		const std::shared_ptr<const StbRecording> getRecording()
		{
			return state->m_stbRecording;
		}

		PACS::MockIPACSArchiver& getMockArchiver() const
		{
			return *static_cast<PACS::MockIPACSArchiver*>(state->m_pacsArchiver.get());
		}

		void createValidStentBoostLiveRun(int numberOfFrames)
		{
			int runIndex = ++lastUsedSeriesNumber;
			for (int i = 0; i < numberOfFrames; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, true);
				stbAdmin.setImage(result.live, result.markers, result.boost);
			}
		}

		void createInvalidStentBoostLiveRun(int numberOfFrames)
		{
			int runIndex = ++lastUsedSeriesNumber;
			for (int i = 0; i < numberOfFrames; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, false);
				stbAdmin.setImage(result.live, result.markers, result.boost);
			}
		}

		const std::wstring STATE_NAME = L"StbLiveReviewState";

		testing::StrictMock<MockICopyToRefClient> copyToRefClient;
		testing::NiceMock<MockIArchivingSettings> archivingSettings;
		std::unique_ptr<StbLiveReviewState> state;

		int lastUsedSeriesNumber;
	};

	TEST_F(TestStbLiveReviewState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbLiveReviewState, GivenLastImageHold_WhenCopyToRefEnabledForRef2_ThenSnapshotIsSentWithCopyToRefToRef2)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref2");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbLiveReviewState, GivenLastImageHold_WhenCopyToRefEnabledForRef1_ThenSnapshotIsSentWithCopyToRefToRef1)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbLiveReviewState, GivenLastImageHold_WhenCopyToRefDisabled_ThenNoSnapshotIsSent)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));
		
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

    TEST_F(TestStbLiveReviewState, GivenLastImageHold_WhenCopyToRefEnabledForRef2_ThenTSMButtonShowsReference2)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);
        const std::wstring REF_VIEWER(L"Ref2");
        const std::wstring TSM_TEXT(L"Copy to\nReference 2");

        EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

        EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));
		EXPECT_CALL(archivingSettings, getCopyToRefTargetNumber(testing::_, testing::_)).Times(1).WillOnce(testing::Return(2));

        createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
        enterState();

        EXPECT_EQ(TSM_TEXT, getTsmPage().copyToRefButton->centerText);
    }

    TEST_F(TestStbLiveReviewState, GivenLastImageHold_WhenNoCopyToRefClientAvailable_ThenCopyToRefButtonIsNotVisible)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(false));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

        createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
        enterState();

        EXPECT_FALSE(getTsmPage().copyToRefButton->visible);
    }

    TEST_F(TestStbLiveReviewState, GivenLastImageHold_WhenCopyToRefClientAvailable_ThenCopyToRefButtonIsVisible)
    {
        const int NUMBER_OF_FRAMES_IN_RUN(10);

        EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

        createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
        enterState();

        EXPECT_TRUE(getTsmPage().copyToRefButton->visible);
    }

	TEST_F(TestStbLiveReviewState, GivenInState_WhenStentBoostAvailableEventTriggered_ThenImagesAreNotAddedToAdministration)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		auto stbResult = createStbResult(1, 0, 100, 100, true);

		auto initialRecording = getRecording();
		auto initialVisibleBoostImage = viewContext.stbViewer.getImage();

		EXPECT_EQ(ViewState::StbLiveReviewState, state->stentBoostImageAvailable(stbResult.live, stbResult.markers, stbResult.boost));
		EXPECT_EQ(ViewState::StbLiveReviewState, state->boostImageAvailable(stbResult.boost, stbResult.registeredLive));

		auto newRecording = stbAdmin.prepareReview();

		EXPECT_EQ(initialRecording, newRecording);
		EXPECT_EQ(initialRecording->getImageCount(), newRecording->getImageCount());

		EXPECT_EQ(viewContext.stbViewer.getImage(), initialVisibleBoostImage);
	}

	TEST_F(TestStbLiveReviewState, GivenArchiveSnapshotTriggerdAndNewRunWithoutMarkers_WhenArchivingFailed_ThenSnapshotButtonIsRemainDisabled)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(2).WillRepeatedly(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		createInvalidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		failSnapshotArchive();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbLiveReviewState, GivenRecordingAlreadyAutoArchived_WhenEnteringState_ThenNotAutoArchivedAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref2");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(3).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, ::testing::_, RunType::SBL)).Times(1);
		EXPECT_CALL(messageBuilder, StartMovie(::testing::_, ::testing::_, ::testing::_, ::testing::_, RunType::SBL)).Times(1);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(2).WillRepeatedly(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(2).WillRepeatedly(testing::Return(true));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		
		enterState();
		
		// Enter state for the second time; this simulates coming back from for instance DCR overlay
		enterState();
	}

	TEST_F(TestStbLiveReviewState, GivenRecordingNotAutoArchivedAndAutoArchivedEnabled_WhenEnteringState_ThenAutoArchivingPerformed)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref2");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(2).WillRepeatedly(testing::Return(REF_VIEWER));

		EXPECT_CALL(copyToRefClient, sendSnapshot(testing::_, testing::TypedEq<const std::wstring&>(REF_VIEWER), testing::_)).Times(1);
		EXPECT_CALL(getMockArchiver(), sendSnapshot(::testing::_, ::testing::_,RunType::SBL)).Times(1);
		EXPECT_CALL(messageBuilder, StartMovie(::testing::_, ::testing::_, ::testing::_, ::testing::_, RunType::SBL)).Times(1);

		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(true));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();
	}

	TEST_F(TestStbLiveReviewState, GivenArchiveSnapshotTriggerd_WhenArchivingSuccessful_ThenSnapshotButtonIsDisabledUntilSuccessReported)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		successSnapshotArchive();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbLiveReviewState, GivenArchiveSnapshotTriggerd_WhenArchivingFailed_ThenSnapshotButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);

		archiveSnapshot();

		EXPECT_FALSE(getTsmPage().storeSnapshotButton->enabled);

		failSnapshotArchive();

		EXPECT_TRUE(getTsmPage().storeSnapshotButton->enabled);
	}

	TEST_F(TestStbLiveReviewState, GivenArchiveMovieTriggerd_WhenArchivingSuccessful_ThenMovieButtonIsDisabledUntilSuccessReported)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		succesMovieArchive();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
	}

	TEST_F(TestStbLiveReviewState, GivenArchiveMovieTriggerd_WhenArchivingFailed_ThenMovieButtonIsEnabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(1).WillOnce(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		failMovieArchive();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);
	}

	TEST_F(TestStbLiveReviewState, GivenArchiveMovieFailed_WhenArchiveMovieButtonPressedAgain_ThenMovieButtonIsDisabledAgain)
	{
		const int NUMBER_OF_FRAMES_IN_RUN(10);
		const std::wstring REF_VIEWER(L"Ref1");

		EXPECT_CALL(messageBuilder, getMetaData()).Times(2).WillRepeatedly(testing::Return(PACS::MetaData(XrayStudy(), XrayGeometry(), 0, 0)));

		EXPECT_CALL(archivingSettings, isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(false));
		EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
		EXPECT_CALL(copyToRefClient, isConnected()).Times(1).WillOnce(testing::Return(false));

		EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).Times(1).WillOnce(testing::Return(REF_VIEWER));

		createValidStentBoostLiveRun(NUMBER_OF_FRAMES_IN_RUN);
		enterState();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);

		failMovieArchive();

		EXPECT_TRUE(getTsmPage().storeMovieButton->enabled);

		archiveMovie();

		EXPECT_FALSE(getTsmPage().storeMovieButton->enabled);
	}


    TEST_P(TestStbLiveReviewState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
    {
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
    }

    INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbLiveReviewState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap),
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));
}}