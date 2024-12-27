// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbImageSelectTsmPage.h"
#include "StbBoostFrameSelectionState.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"
#include "PciViewContext.h"
#include "PciEpx.h"

namespace Pci {	namespace Core
{

	class TestStbBoostFrameSelectionState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
        TestStbBoostFrameSelectionState() :
			TestUiState()
		{}

		void SetUp() override
		{
            EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));

			state = std::unique_ptr<StbBoostFrameSelectionState>(new StbBoostFrameSelectionState(viewContext, uiStateController, copyToRefClient));
		}

		void enterState()
		{
			state->onEnter();
		}

		int getSelectedBoostImageIndex()
		{
			return state->m_selectedImage;
		}

		void setSelectedIndex(int index)
		{
			state->setSelection(index);
		}

		StbImageSelectTsmPage& getTsmPage()
		{
			return state->getTsmPage();
		}

		void createValidStentBoostPostDeployRun(int numberOfFrames)
		{
			int runIndex = 1;
			for (int i = 0; i < numberOfFrames; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, true);
				stbAdmin.setImage(result.live, result.markers, result.boost);
			}
		}

		void createStentBoostSubtractRunWithContrastImages(int numberOfFrames, bool validContrastImages)
		{
			int runIndex = 1;
			for (int i = 0; i < numberOfFrames / 2; i++)
			{
				auto result = createStbResult(runIndex, i, 720, 720, true);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}

			for (int i = 0; i < numberOfFrames / 2; i++)
			{
				int imageIndex = i + (numberOfFrames / 2);
				bool valid = ((imageIndex % 2) == 0) || validContrastImages;
				auto result = createStbResult(runIndex, imageIndex, 720, 720, valid, true);
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
			}
		}

        const std::wstring STATE_NAME = L"StbBoostFrameSelectionState";

		::testing::NiceMock<MockUiStateController> uiStateController;
		std::unique_ptr<StbBoostFrameSelectionState> state;
        testing::StrictMock<MockICopyToRefClient> copyToRefClient;
	};

	TEST_F(TestStbBoostFrameSelectionState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbBoostFrameSelectionState, WhenStatedEntered_CorrectGuidanceTextIsShown)
	{
		const int NUMBER_OF_FRAMES(20);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

		EXPECT_TRUE(viewContext.stbBoostSelectViewer.visible);
		EXPECT_EQ(viewContext.stbBoostSelectViewer.guidanceTextLabel.text, Localization::Translation::IDS_UseTSMToSelectLastImageBoostedPhase);
	}
	
	TEST_F(TestStbBoostFrameSelectionState, WhenStateEntered_ThenSelectedFrameIsTheLastBoostedFrame)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(9);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

        EXPECT_TRUE(viewContext.stbBoostSelectViewer.visible);
        EXPECT_EQ(SELECTED_INDEX, getSelectedBoostImageIndex());
    }

	TEST_F(TestStbBoostFrameSelectionState, WhenNavigateRightIsPressedAndNotAtLastImage_ThenNextImageIsSelected)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(15);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateRightButton);

		EXPECT_EQ(SELECTED_INDEX + 1, getSelectedBoostImageIndex());
	}

	TEST_F(TestStbBoostFrameSelectionState, WhenNavigateRightIsPressedAndAtLastImage_ThenSelectedImageDoesNotChange)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(19);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateRightButton);

		EXPECT_FALSE(getTsmPage().navigateRightButton->enabled);
		EXPECT_TRUE(getTsmPage().navigateLeftButton->enabled);
		EXPECT_EQ(SELECTED_INDEX, getSelectedBoostImageIndex());
	}

	TEST_F(TestStbBoostFrameSelectionState, WhenNavigateLeftIsPressedAndNotAtFirstImage_ThenPreviousImageIsSelected)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(15);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

		EXPECT_EQ(SELECTED_INDEX - 1, getSelectedBoostImageIndex());
	}

	TEST_F(TestStbBoostFrameSelectionState, WhenNavigateLeftIsPressedAndAtFirstImage_ThenSelectedImageDoesNotChange)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(0);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

		EXPECT_FALSE(getTsmPage().navigateLeftButton->enabled);
		EXPECT_TRUE(getTsmPage().navigateRightButton->enabled);
		EXPECT_EQ(SELECTED_INDEX, getSelectedBoostImageIndex());
	}


    TEST_F(TestStbBoostFrameSelectionState, WhenInStateAndApplyIsPressed_ThenChangeIsAppliedAndUiStateSwitchesBackToReviewState)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(15);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

        setSelectedIndex(SELECTED_INDEX);

        EXPECT_CALL(uiStateController, switchState(ViewState::StbContrastReviewState)).Times(1);

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::ApplyButton);

		auto recording = viewContext.stbAdministration.prepareReview();
		EXPECT_EQ(SELECTED_INDEX, recording->getLastBoostedImage().boost->imageNumber);
    }

    TEST_F(TestStbBoostFrameSelectionState, WhenInStateAndCancelIsPressed_ThenUiStateSwitchesBackToReviewState)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(15);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

        setSelectedIndex(SELECTED_INDEX);

        EXPECT_CALL(uiStateController, switchState(ViewState::StbContrastReviewState)).Times(1);

        getTsmPage().onButtonRelease(StbTsmPage::BoostFrameSelectButton);
    }

    TEST_F(TestStbBoostFrameSelectionState, WhenNonDefaultImageIsSelectedAndResetToDefaultIsPressed_ThenDefaultImageIsSelected)
    {
        const int NUMBER_OF_FRAMES(20);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

        auto getCurrentSelectedBoostImageIndex = getSelectedBoostImageIndex();

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

        EXPECT_EQ(getCurrentSelectedBoostImageIndex - 1, getSelectedBoostImageIndex());

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::ResetDefaultButton);

        EXPECT_EQ(getCurrentSelectedBoostImageIndex, getSelectedBoostImageIndex());
    }

    TEST_P(TestStbBoostFrameSelectionState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
	{
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
	}

	INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbBoostFrameSelectionState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap), 
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));

}}