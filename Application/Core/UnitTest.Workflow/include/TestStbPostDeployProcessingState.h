// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbPostDeployProcessingState.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"

namespace Pci { namespace Core
{

	class TestStbPostDeployProcessingState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
		TestStbPostDeployProcessingState() :
			TestUiState()
		{}

		void SetUp() override
		{
            EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
			EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostPostDeploy)).WillRepeatedly(testing::Return(L""));
			EXPECT_CALL(archivingSettings, getCopyToRefTargetNumber(testing::_, testing::_)).WillRepeatedly(testing::Return(1));

			state = std::unique_ptr<StbPostDeployProcessingState>(new StbPostDeployProcessingState(
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

		const std::wstring STATE_NAME = L"StbPostDeployProcessingState";

		::testing::NiceMock<MockUiStateController> uiStateController;
		std::unique_ptr<StbPostDeployProcessingState> state;
        testing::StrictMock<MockICopyToRefClient> copyToRefClient;
		testing::StrictMock<MockIArchivingSettings> archivingSettings;
	};

	TEST_F(TestStbPostDeployProcessingState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbPostDeployProcessingState, GivenStentBoost_WhenEnteringStateTriggeredByNewRun_ThenStentBoostStbViewerIsShown)
	{
		enterState();

		EXPECT_FALSE(viewContext.stbGuidance.visible);
		EXPECT_TRUE(viewContext.stbViewer.visible);
	}

	TEST_F(TestStbPostDeployProcessingState, GivenStentBoost_WhenEnteringStateTriggeredByRoiChange_ThenStentBoostProcessingGuidanceIsShown)
	{
		EXPECT_CALL(uiStateController, isReplayActive()).Times(1).WillRepeatedly(testing::Return(true));

		enterState();

		const std::wstring& statusValue = getGuidanceModel().status;
		EXPECT_STREQ(Pci::Core::Localization::Translation::IDS_ProcessingStentBoost.c_str(), statusValue.c_str());
		EXPECT_TRUE(viewContext.stbGuidance.visible);
		EXPECT_FALSE(viewContext.stbViewer.visible);
	}

	TEST_F(TestStbPostDeployProcessingState, GivenStentBoost_WhenLastImageHoldReceivedBeforeEndLive_ThenEventIsIgnored)
	{
		enterState();

		EXPECT_EQ(ViewState::StbPostDeployProcessingState, state->lastImageHold());
	}	

	TEST_F(TestStbPostDeployProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsTooShort_ThenGuidanceStateSwitchRequested)
	{
		enterState();
		EXPECT_EQ(ViewState::StbPostDeployProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbPostDeployGuidanceState, state->lastImageHold());
	}

	TEST_F(TestStbPostDeployProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsLongEnough_ThenReviewStateSwitchRequested)
	{
		const int RUN_LENGTH_MINIMUM(6);
		const int RUN_INDEX(1);

		enterState();

		for (int i = 1; i <= RUN_LENGTH_MINIMUM; i++)
		{
			auto result = createStbResult(RUN_INDEX, i, 720, 720, true);
			stbAdmin.setImage(result.live, result.markers, result.boost);
		}

		EXPECT_EQ(ViewState::StbPostDeployProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbPostDeployReviewState, state->lastImageHold());
	}

	TEST_F(TestStbPostDeployProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsLongEnoughWithoutMarkers_ThenReviewStateSwitchRequested)
	{
		const int RUN_LENGTH_MINIMUM(6);
		const int RUN_INDEX(1);

		enterState();

		for (int i = 1; i <= RUN_LENGTH_MINIMUM; i++)
		{
			auto result = createStbResult(RUN_INDEX, i, 720, 720, false);
			stbAdmin.setImage(result.live, result.markers, result.boost);
		}

		EXPECT_EQ(ViewState::StbPostDeployProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbPostDeployReviewState, state->lastImageHold());
	}

    TEST_P(TestStbPostDeployProcessingState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
    {
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
    }

    INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbPostDeployProcessingState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap),
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));
}}