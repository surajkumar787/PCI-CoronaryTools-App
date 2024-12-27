// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbContrastProcessingState.h"
#include "MockIUiStateController.h"
#include "Translation.h"
#include "MockICopyToRefClient.h"
#include "ArchivingSettings.h"

namespace Pci {	namespace Core
{
	class TestStbContrastProcessingState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
		TestStbContrastProcessingState() :
			TestUiState()
		{}

		void SetUp() override
		{
            EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
			EXPECT_CALL(archivingSettings, getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract)).WillRepeatedly(testing::Return(L""));
			EXPECT_CALL(archivingSettings, getCopyToRefTargetNumber(testing::_, testing::_)).WillRepeatedly(testing::Return(1));

			state = std::unique_ptr<StbContrastProcessingState>(new StbContrastProcessingState(
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

		const std::wstring STATE_NAME = L"StbContrastProcessingState";

		::testing::NiceMock<MockUiStateController> uiStateController;
		std::unique_ptr<StbContrastProcessingState> state;
        testing::StrictMock<MockICopyToRefClient> copyToRefClient;
		testing::StrictMock<MockIArchivingSettings> archivingSettings;
	};

	TEST_F(TestStbContrastProcessingState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbContrastProcessingState, GivenStentBoostContrast_WhenEnteringStateTriggeredByNewRun_ThenStentBoostSubtractGuidanceIsShown)
	{
		enterState();

		EXPECT_TRUE(getGuidanceModel().showSBSGuidance);
		EXPECT_TRUE(viewContext.stbGuidance.visible);
	}

	TEST_F(TestStbContrastProcessingState, GivenStentBoostContrast_WhenEnteringStateTriggeredByRoiChange_ThenStentBoostProcessingGuidanceIsShown)
	{
		EXPECT_CALL(uiStateController, isReplayActive()).Times(2).WillRepeatedly(testing::Return(true));

		enterState();

		EXPECT_FALSE(getGuidanceModel().showSBSGuidance);
		const std::wstring& statusValue = getGuidanceModel().status;
		EXPECT_STREQ(Pci::Core::Localization::Translation::IDS_ProcessingStentBoost.c_str(), statusValue.c_str());
		EXPECT_TRUE(viewContext.stbGuidance.visible);
	}

	TEST_F(TestStbContrastProcessingState, GivenStentBoost_WhenLastImageHoldReceivedBeforeEndLive_ThenEventIsIgnored)
	{
		enterState();

		EXPECT_EQ(ViewState::StbContrastProcessingState, state->lastImageHold());
	}

	TEST_F(TestStbContrastProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsTooShort_ThenGuidanceStateSwitchRequested)
	{
		enterState();
		EXPECT_EQ(ViewState::StbContrastProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbContrastGuidanceState, state->lastImageHold());
	}

	TEST_F(TestStbContrastProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsLongEnough_ThenReviewStateSwitchRequested)
	{
		const int RUN_LENGTH_MINIMUM(6);
		const int RUN_INDEX(1);

		enterState();

		for (int i = 1; i <= RUN_LENGTH_MINIMUM; i++)
		{
			auto result = createStbResult(RUN_INDEX, i, 720, 720, true);
			stbAdmin.setImage(result.live, result.markers, result.boost);
		}

		EXPECT_EQ(ViewState::StbContrastProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbContrastReviewState, state->lastImageHold());
	}

	TEST_F(TestStbContrastProcessingState, GivenLastImageHoldReceivedAfterEndLive_WhenRunIsLongEnoughWithoutMarkers_ThenReviewStateSwitchRequested)
	{
		const int RUN_LENGTH_MINIMUM(6);
		const int RUN_INDEX(1);

		enterState();

		for (int i = 1; i <= RUN_LENGTH_MINIMUM; i++)
		{
			auto result = createStbResult(RUN_INDEX, i, 720, 720, false);
			stbAdmin.setImage(result.live, result.markers, result.boost);
		}

		EXPECT_EQ(ViewState::StbContrastProcessingState, state->endLive());
		EXPECT_EQ(ViewState::StbContrastReviewState, state->lastImageHold());
	}

    TEST_P(TestStbContrastProcessingState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
    {
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
    }

	INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbContrastProcessingState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap),
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));
}}