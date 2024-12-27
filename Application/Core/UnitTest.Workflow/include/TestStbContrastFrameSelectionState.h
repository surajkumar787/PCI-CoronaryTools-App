// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbContrastFrameSelectionState.h"
#include "StbImageSelectTsmPage.h"
#include "MockIUiStateController.h"
#include "MockICopyToRefClient.h"
#include "MockIArchivingSettings.h"
#include "MockIPACSArchiver.h"
#include "PACSArchiver.h"
#include "MockICopyToRefClient.h"

namespace Pci {	namespace Core
{

	class TestStbContrastFrameSelectionState : public TestUiState, public testing::WithParamInterface<PciEpx>
	{
	public:
		TestStbContrastFrameSelectionState() :
			TestUiState()
		{}

		void SetUp() override
		{
            EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));

			state = std::unique_ptr<StbContrastFrameSelectionState>(new StbContrastFrameSelectionState(viewContext, uiStateController, copyToRefClient));
		}

		void enterState()
		{
			state->onEnter();
		}

		int getSelectedContrastImageIndex()
		{
			return state->m_selectedImage;
		}

		int getAbsoluteSelectedFrameNumber()
		{
			return state->m_selectedImage;
		}

		void setSelectedIndex(int index)
		{
			state->setSelection(index);
		}

		int getSelectedContrastFromStbRecording()
		{
			return state->m_stbRecording->getContrastImage().boost->imageNumber;
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
				stbAdmin.setImage(result.live, result.markers, result.registeredLive, result.boost);
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

        const std::wstring STATE_NAME = L"StbContrastFrameSelectionState";

		::testing::NiceMock<MockUiStateController> uiStateController;
		std::unique_ptr<StbContrastFrameSelectionState> state;
        testing::StrictMock<MockICopyToRefClient> copyToRefClient;
	};

	TEST_F(TestStbContrastFrameSelectionState, StateNameIsCorrect)
	{
		EXPECT_EQ(state->getStateName(), STATE_NAME);
	}

	TEST_F(TestStbContrastFrameSelectionState, WhenStatedEntered_CorrectGuidanceTextIsShown)
	{
		const int NUMBER_OF_FRAMES(20);

		createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

		EXPECT_TRUE(viewContext.stbContrastSelectViewer.visible);
		EXPECT_EQ(viewContext.stbContrastSelectViewer.guidanceTextLabel.text, Localization::Translation::IDS_UseTSMToSelectDifferentImage);
	}


	TEST_F(TestStbContrastFrameSelectionState, WhenStateEnteredAndAutoSelectedFrameSet_ThenSelectedFrameIsAutoSelectedOne)
	{
		const int NUMBER_OF_FRAMES(20);
		const int AUTO_SELECTED_INDEX(11);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		stbAdmin.setAutoSelectedContrastFrame(AUTO_SELECTED_INDEX);
		state->onEnter();

		EXPECT_TRUE(viewContext.stbContrastSelectViewer.visible);
		EXPECT_EQ(AUTO_SELECTED_INDEX, getAbsoluteSelectedFrameNumber());
	}

	TEST_F(TestStbContrastFrameSelectionState, WhenNavigateRightIsPressedAndNotAtLastImage_ThenNextImageIsSelected)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(15);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateRightButton);

		EXPECT_EQ(SELECTED_INDEX + 1, getSelectedContrastImageIndex());
	}

	TEST_F(TestStbContrastFrameSelectionState, WhenNavigateRightIsPressedAndAtLastImage_ThenSelectedImageDoesNotChange)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(19);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateRightButton);

		EXPECT_FALSE(getTsmPage().navigateRightButton->enabled);
		EXPECT_TRUE(getTsmPage().navigateLeftButton->enabled);
		EXPECT_EQ(SELECTED_INDEX, getSelectedContrastImageIndex());
	}

	TEST_F(TestStbContrastFrameSelectionState, WhenNavigateLeftIsPressedAndNotAtFirstImage_ThenPreviousImageIsSelected)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(15);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

		EXPECT_EQ(SELECTED_INDEX - 1, getSelectedContrastImageIndex());
	}

	TEST_F(TestStbContrastFrameSelectionState, WhenNavigateLeftIsPressedAndAtFirstImage_ThenSelectedImageDoesNotChange)
	{
		const int NUMBER_OF_FRAMES(20);
		const int SELECTED_INDEX(10);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
		state->onEnter();

		setSelectedIndex(SELECTED_INDEX);

		getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

		EXPECT_FALSE(getTsmPage().navigateLeftButton->enabled);
		EXPECT_TRUE(getTsmPage().navigateRightButton->enabled);
		EXPECT_EQ(SELECTED_INDEX, getSelectedContrastImageIndex());
	}

    TEST_F(TestStbContrastFrameSelectionState, WhenRunContainsInvalidContrastImagesAndNavigateRightIsPressedAndNotAtLastImage_ThenNextValidContrastImageIsSelected)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(12);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, false);
        state->onEnter();

        setSelectedIndex(SELECTED_INDEX);

        auto selectedImageNumber = getSelectedContrastImageIndex();

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateRightButton);

        EXPECT_EQ(selectedImageNumber + 2, getSelectedContrastImageIndex());
    }

    TEST_F(TestStbContrastFrameSelectionState, WhenRunContainsInvalidContrastImagesAndNavigateLeftIsPressedAndNotAtFirstImage_ThenPreviousValidContrastImageIsSelected)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(12);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, false);
        state->onEnter();

        setSelectedIndex(SELECTED_INDEX);

        auto selectedImageNumber = getSelectedContrastImageIndex();

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

        EXPECT_EQ(selectedImageNumber - 2, getSelectedContrastImageIndex());
    }


    TEST_F(TestStbContrastFrameSelectionState, WhenInStateAndApplyIsPressed_ThenUiStateSwitchesBackToReviewState)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(15);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

        setSelectedIndex(SELECTED_INDEX);

        EXPECT_CALL(uiStateController, switchState(ViewState::StbContrastReviewState)).Times(1);

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::ApplyButton);

        EXPECT_EQ(SELECTED_INDEX, getSelectedContrastFromStbRecording());
    }

    TEST_F(TestStbContrastFrameSelectionState, WhenInStateAndCancelIsPressed_ThenUiStateSwitchesBackToReviewState)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(10);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        state->onEnter();

        auto currentImageIndex = getSelectedContrastFromStbRecording();

        setSelectedIndex(SELECTED_INDEX);

        EXPECT_CALL(uiStateController, switchState(ViewState::StbContrastReviewState)).Times(1);

        getTsmPage().onButtonRelease(StbTsmPage::ContrastFrameSelectButton);

        EXPECT_EQ(currentImageIndex, getSelectedContrastFromStbRecording());
    }

    TEST_F(TestStbContrastFrameSelectionState, WhenNonDefaultImageIsSelectedAndResetToDefaultIsPressed_ThenDefaultImageIsSelected)
    {
        const int NUMBER_OF_FRAMES(20);
        const int SELECTED_INDEX(15);

        createStentBoostSubtractRunWithContrastImages(NUMBER_OF_FRAMES, true);
        stbAdmin.setAutoSelectedContrastFrame(SELECTED_INDEX);
        state->onEnter();

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::NavigateLeftButton);

        EXPECT_EQ(SELECTED_INDEX - 1, getSelectedContrastImageIndex());

        getTsmPage().onButtonRelease(StbImageSelectTsmPage::ResetDefaultButton);

        EXPECT_EQ(SELECTED_INDEX, getSelectedContrastImageIndex());
    }


    TEST_P(TestStbContrastFrameSelectionState, WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection)
    {
        viewContext.currentEpx = GetParam();
        ASSERT_NE(state->viewState, state->studyChanged());
        ASSERT_EQ(getGuidanceStateByEpxSelection(), state->studyChanged());
    }

    INSTANTIATE_TEST_SUITE_P(
        WhenStudyChanged_ThenReturnedStateIsGuidanceStateBasedOnEPXSelection,
        TestStbContrastFrameSelectionState,
        ::testing::Values(
            PciEpx(PciEpx::Type::Roadmap),
            PciEpx(PciEpx::Type::NonPci),
            PciEpx(PciEpx::SubType::Live),
            PciEpx(PciEpx::SubType::PostDeployment),
            PciEpx(PciEpx::SubType::Subtract)
        ));
}}