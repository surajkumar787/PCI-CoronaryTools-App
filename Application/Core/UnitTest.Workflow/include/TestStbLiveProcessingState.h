// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbLiveProcessingState.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"

namespace Pci { namespace Core
{

	class TestStbLiveProcessingState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
		TestStbLiveProcessingState() :
			TestUiState()
		{}

		void SetUp() override
		{
            EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
			EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostLive)).WillRepeatedly(testing::Return(L""));
			EXPECT_CALL(archivingSettings, getCopyToRefTargetNumber(testing::_, testing::_)).WillRepeatedly(testing::Return(1));

			state = std::unique_ptr<StbLiveProcessingState>(new StbLiveProcessingState(
				viewContext, 
				uiStateController, 
				copyToRefClient,
				archivingSettings));
		}

		void enterState()
		{
			state->onEnter();
		}

		PciGuidanceModel& getGuidanceModel()
		{
			return state->_guidanceModel;
		}

		const std::wstring STATE_NAME = L"StbLiveProcessingState";

		::testing::NiceMock<MockUiStateController> uiStateController;
		std::unique_ptr<StbLiveProcessingState> state;
        testing::StrictMock<MockICopyToRefClient> copyToRefClient;
		testing::StrictMock<MockIArchivingSettings> archivingSettings;
	};

	TEST_F(TestStbLiveProcessingState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbLiveProcessingState, GivenStentBoost_WhenEnteringStateTriggeredByNewRun_ThenStentBoostStbViewerIsShown)
	{
		enterState();

		EXPECT_FALSE(viewContext.stbGuidance.visible);
		EXPECT_TRUE(viewContext.stbViewer.visible);
	}

	TEST_F(TestStbLiveProcessingState, GivenStentBoost_WhenLastImageHoldReceivedBeforeEndLive_ThenEventIsIgnored)
	{
		enterState();

		EXPECT_EQ(ViewState::StbLiveProcessingState, state->lastImageHold());
	}	

	TEST_F(TestStbLiveProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsTooShort_ThenGuidanceStateSwitchRequested)
	{
		enterState();
		EXPECT_EQ(ViewState::StbLiveProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbLiveGuidanceState, state->lastImageHold());
	}

	TEST_F(TestStbLiveProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsLongEnough_ThenReviewStateSwitchRequested)
	{
		const int RUN_LENGTH_MINIMUM(6);
		const int RUN_INDEX(1);

		enterState();

		for (int i = 1; i <= RUN_LENGTH_MINIMUM; i++)
		{
			auto result = createStbResult(RUN_INDEX, i, 720, 720, true);
			stbAdmin.setImage(result.live, result.markers, result.boost);
		}

		EXPECT_EQ(ViewState::StbLiveProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbLiveReviewState, state->lastImageHold());
	}

	TEST_F(TestStbLiveProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsLongEnoughWithoutMarkers_ThenReviewStateSwitchRequested)
	{
		const int RUN_LENGTH_MINIMUM(6);
		const int RUN_INDEX(1);

		enterState();

		for (int i = 1; i <= RUN_LENGTH_MINIMUM; i++)
		{
			auto result = createStbResult(RUN_INDEX, i, 720, 720, false);
			stbAdmin.setImage(result.live, result.markers, result.boost);
		}

		EXPECT_EQ(ViewState::StbLiveProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbLiveReviewState, state->lastImageHold());
	}

    TEST_P(TestStbLiveProcessingState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
    {
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
    }

    INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbLiveProcessingState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap),
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));
}}