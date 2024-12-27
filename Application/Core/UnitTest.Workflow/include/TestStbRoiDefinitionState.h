// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "StbRoiDefinitionState.h"
#include "MockIUiStateController.h"
#include "StbRoiTsmPage.h"
#include "MockICopyToRefClient.h"

namespace Pci { namespace Core
{
	class TestStbRoiDefinitionState : public TestUiState, public testing::WithParamInterface<std::tuple<ViewState, std::wstring, ViewState> >
	{
	public:
		TestStbRoiDefinitionState() : TestUiState()
		{
		}

		class StbRoiDefinitionStateTest : public StbRoiDefinitionState
		{
		public:
			StbRoiDefinitionStateTest(PciViewContext &viewContext,
				ViewState state,
				const std::wstring &name,
				IUiStateController& controller,
				ViewState returnState,
				Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
				const IArchivingSettings& archivingSettings)
					: StbRoiDefinitionState(viewContext, 
						state, 
						name, 
						controller, 
						returnState,
						ViewState::StbContrastRoiDefinitionState,
						copyToRefClient, 
						archivingSettings) 
			{
			}

			~StbRoiDefinitionStateTest() {};

		private:
			virtual std::wstring getCopyToRefTargetName() const override { return L""; };
		};


		void SetUp() override
		{
            EXPECT_CALL(copyToRefClient, isAvailable()).Times(1).WillOnce(testing::Return(true));
			EXPECT_CALL(archivingSettings, getCopyToRefTargetNumber(testing::_, testing::_)).WillRepeatedly(testing::Return(1));

            state = std::unique_ptr<StbRoiDefinitionStateTest>(new StbRoiDefinitionStateTest(
				viewContext, 
				std::get<0>(GetParam()), 
				std::get<1>(GetParam()), 
				uiStateController, 
				std::get<2>(GetParam()), 
				copyToRefClient,
				archivingSettings));

			setBoundries(Sense::Rect(0.0, 0.0, 1024.0, 1024.0));
			setRegionOfInterest(Sense::Rect(state->m_imageBoundries.center(), Sense::Size(100, 100)));
		}

		Sense::Rect getRegionOfInterest()
		{
			return state->m_regionOfInterest;
		}

		bool isLastImageHoldShown()
		{
			return state->m_boostModel.showLastImageHold;
		}

		void setRegionOfInterest(const Sense::Rect& roi)
		{
			state->m_regionOfInterest = roi;
			state->updateTSMControls();
		}

		void setBoundries(const Sense::Rect& boundries)
		{
			stbAdmin.newRun();
			state->m_imageBoundries = boundries;
			addImageToStentBoostAdministration(boundries);
		}

		void addImageToStentBoostAdministration(Sense::Rect imageSize)
		{
			auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
			img->validRect = imageSize;
			
			auto markers = createStbMarkers(Sense::Point(0,0), Sense::Point(0,0));
			markers->valid = false;

			auto boost = std::make_shared<StbImage>();

			stbAdmin.setImage(img, markers, boost);
		}


		void addImageToStentBoostAdministration(Sense::Point marker1, Sense::Point marker2)
		{
			auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
			auto markers = createStbMarkers(marker1, marker2);
			markers->valid = true;

			auto boost = std::make_shared<StbImage>();

			stbAdmin.setImage(img, markers, boost);
		}

		std::shared_ptr<CommonPlatform::Xray::XrayImage> createXRayImage(int imageNumber)
		{
			auto image = std::make_shared<CommonPlatform::Xray::XrayImage>();
			image->runIndex = 1;
			image->imageNumber = imageNumber;
			return image;
		}

		std::shared_ptr<StbMarkers> createStbMarkers(Sense::Point marker1, Sense::Point marker2)
		{
			auto markers = std::make_shared<StbMarkers>();
			markers->valid = true;
			markers->p1.x = static_cast<float>(marker1.x);
			markers->p1.y = static_cast<float>(marker1.y);
			markers->p2.x = static_cast<float>(marker2.x);
			markers->p2.y = static_cast<float>(marker2.y);

			return markers;
		}

        StbRoiTsmPage& getTsmPage()
        {
            return state->getTsmPage();
        }

        const std::wstring STATE_NAME = L"StbRoiDefinitionState";

        ::testing::NiceMock<MockUiStateController> uiStateController;
        std::unique_ptr<StbRoiDefinitionStateTest> state;
        testing::StrictMock<MockICopyToRefClient> copyToRefClient;
		testing::StrictMock<MockIArchivingSettings> archivingSettings;
	};

	TEST_P(TestStbRoiDefinitionState, WhenStateEntered_ThenOnlyRoiViewerIsVisible)
	{
		state->onEnter();

		EXPECT_TRUE(viewContext.stbRoiViewer.visible);
		EXPECT_FALSE(viewContext.stbViewer.visible);
		EXPECT_FALSE(viewContext.stbGuidance.visible);
	}

	TEST_P(TestStbRoiDefinitionState, WhenStateEnteredAndNoMarkers_ThenRoiIsInitializedWithStbRecordingMarkerRegion)
	{
		const double IMAGE_BOUNDRY_WIDTH(100);
		const double IMAGE_BOUNDRY_HEIGHT(100);
		const double DEFAULT_ROI_SIZE(IMAGE_BOUNDRY_WIDTH * 0.4);
		const Sense::Point IMAGE_CENTER(IMAGE_BOUNDRY_WIDTH / 2.0, IMAGE_BOUNDRY_WIDTH / 2);

		setBoundries(Sense::Rect(0, 0, IMAGE_BOUNDRY_WIDTH, IMAGE_BOUNDRY_HEIGHT));

		state->onEnter();

		auto stateROI = getRegionOfInterest();

		EXPECT_EQ(IMAGE_CENTER, stateROI.center());
		EXPECT_NEAR(DEFAULT_ROI_SIZE, stateROI.width(), 0.1);
		EXPECT_NEAR(DEFAULT_ROI_SIZE, stateROI.height(), 0.1);
	}

	TEST_P(TestStbRoiDefinitionState, WhenStateEnteredAndMarkers_ThenSquareRoiIsCreatedBasedOnStbRecording)
	{
		const Sense::Point MARKER1(10.0, 10.0);
		const Sense::Point MARKER2(15.0, 80.0);
		addImageToStentBoostAdministration(MARKER1, MARKER2);

		double regionSize = std::max((MARKER2.x - MARKER1.x), (MARKER2.y - MARKER1.y));

		state->onEnter();

		auto stbRecordingMarkerRegion = viewContext.stbAdministration.prepareReview()->getMarkerRegion();
		auto stateROI = getRegionOfInterest();

		EXPECT_EQ(stbRecordingMarkerRegion.topLeft(), stateROI.topLeft());
		EXPECT_EQ(stateROI.height(), stateROI.width());
		EXPECT_EQ(regionSize, stateROI.width());
	}

	TEST_P(TestStbRoiDefinitionState, WhenStateEntered_ThenRegionOfInterestIsSet)
	{
		::testing::InSequence sequence;

		EXPECT_CALL(uiStateController, setStbRegionOfInterest(::testing::_)).Times(1);
		EXPECT_CALL(uiStateController, startReplayRun(true)).Times(1);
		state->onEnter();
	}

	TEST_P(TestStbRoiDefinitionState, WhenStateEntered_ThenReplayIsStarted)
	{
		::testing::InSequence sequence;

		EXPECT_CALL(uiStateController, startReplayRun(true)).Times(1);
		EXPECT_CALL(uiStateController, stopReplayRun()).Times(0);
		state->onEnter();
	}

	TEST_P(TestStbRoiDefinitionState, WhenStateActive_ThenLastImageHoldIsShown)
	{
		state->onEnter();
		EXPECT_TRUE(isLastImageHoldShown());
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMIncreaseButtonPressed_ThenRoiIncreasesBy1Pixels)
	{
		const int SINGLE_INCREMENT(1);

		auto initialRoi = getRegionOfInterest();
        getTsmPage().onButtonPress(StbRoiTsmPage::SizeUpButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_INCREMENT, newRoi.width() - initialRoi.width());
		EXPECT_EQ(SINGLE_INCREMENT, newRoi.height() - initialRoi.height());
		EXPECT_EQ(newRoi.center(), initialRoi.center());
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMIncreaseButtonRepeated_ThenIncrementSpeedIncreasesBy1)
	{
		const int SINGLE_INCREMENT(1);
		const int REPEATED_INCREMENT(SINGLE_INCREMENT+1);

		auto initialRoi = getRegionOfInterest();
        getTsmPage().onButtonPress(StbRoiTsmPage::SizeUpButton, true);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(REPEATED_INCREMENT, newRoi.width() - initialRoi.width());
		EXPECT_EQ(REPEATED_INCREMENT, newRoi.height() - initialRoi.height());
		EXPECT_EQ(newRoi.center(), initialRoi.center());
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtLeftBoundry_WhenTSMIncreaseButtonPressed_ThenRoiIncreasesTopBottomAndRight)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(0, 25), Sense::Point(25, 50));

		setBoundries(Sense::Rect(0, 0, 100, 100));
		setRegionOfInterest(initialRoi);

		const double SINGLE_INCREMENT_SINGLE_DIRECTION(0.5);

        getTsmPage().onButtonPress(StbRoiTsmPage::SizeUpButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.width() - initialRoi.width());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.height() - initialRoi.height());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, initialRoi.top - newRoi.top);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.right - initialRoi.right);
		EXPECT_EQ(initialRoi.left, newRoi.left);

		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtRightBoundry_WhenTSMIncreaseButtonPressed_ThenRoiIncreasesTopBottomAndLeft)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(75, 25), Sense::Point(100, 50));

		setBoundries(Sense::Rect(0, 0, 100, 100));
		setRegionOfInterest(initialRoi);

		const double SINGLE_INCREMENT_SINGLE_DIRECTION(0.5);

        getTsmPage().onButtonPress(StbRoiTsmPage::SizeUpButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.width() - initialRoi.width());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.height() - initialRoi.height());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, initialRoi.top - newRoi.top);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, initialRoi.left - newRoi.left);
		EXPECT_EQ(initialRoi.right, newRoi.right);

		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtTopBoundry_WhenTSMIncreaseButtonPressed_ThenRoiIncreasesBottomLeftAndRight)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(25, 0), Sense::Point(50, 25));

		setBoundries(Sense::Rect(0, 0, 100, 100));
		setRegionOfInterest(initialRoi);

		const double SINGLE_INCREMENT_SINGLE_DIRECTION(0.5);

        getTsmPage().onButtonPress(StbRoiTsmPage::SizeUpButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.width() - initialRoi.width());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.height() - initialRoi.height());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, newRoi.right - initialRoi.right);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, initialRoi.left - newRoi.left);
		EXPECT_EQ(initialRoi.top, newRoi.top);

		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtBottomBoundry_WhenTSMIncreaseButtonPressed_ThenRoiIncreasesTopLeftAndRight)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(25, 75), Sense::Point(50, 100));

		setBoundries(Sense::Rect(0, 0, 100, 100));
		setRegionOfInterest(initialRoi);

		const double SINGLE_INCREMENT_SINGLE_DIRECTION(0.5);

        getTsmPage().onButtonPress(StbRoiTsmPage::SizeUpButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.width() - initialRoi.width());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, newRoi.height() - initialRoi.height());
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION * 2, initialRoi.top - newRoi.top);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, newRoi.right - initialRoi.right);
		EXPECT_EQ(SINGLE_INCREMENT_SINGLE_DIRECTION, initialRoi.left - newRoi.left);
		EXPECT_EQ(initialRoi.bottom, newRoi.bottom);

		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtBottomAndTopBoundry_ThenIncreaseAndUpAndDownButtonAreDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(50, 0), Sense::Point(150, 100));

		setBoundries(Sense::Rect(0, 0, 200, 100));
		setRegionOfInterest(initialRoi);

		EXPECT_FALSE(getTsmPage().panDownButton->enabled);
		EXPECT_FALSE(getTsmPage().panUpButton->enabled);
		EXPECT_TRUE(getTsmPage().panLeftButton->enabled);
		EXPECT_TRUE(getTsmPage().panRightButton->enabled);
		EXPECT_FALSE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtLeftAndRightBoundry_ThenIncreaseAndLeftAndRightButtonAreDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(0, 50), Sense::Point(100, 150));

		setBoundries(Sense::Rect(0, 0, 100, 200));
		setRegionOfInterest(initialRoi);

		EXPECT_TRUE(getTsmPage().panDownButton->enabled);
		EXPECT_TRUE(getTsmPage().panUpButton->enabled);
		EXPECT_FALSE(getTsmPage().panLeftButton->enabled);
		EXPECT_FALSE(getTsmPage().panRightButton->enabled);
		EXPECT_FALSE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtBoundry_ThenSizeUpAndMoveButtonsAreDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(0, 0), Sense::Point(100, 100));

		setBoundries(Sense::Rect(0, 0, 100, 100));
		setRegionOfInterest(initialRoi);

		EXPECT_FALSE(getTsmPage().panDownButton->enabled);
		EXPECT_FALSE(getTsmPage().panUpButton->enabled);
		EXPECT_FALSE(getTsmPage().panLeftButton->enabled);
		EXPECT_FALSE(getTsmPage().panRightButton->enabled);
		EXPECT_FALSE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMDecreaseButtonPressed_ThenRoiDecreasesBy1Pixel)
	{
		const int SINGLE_DECREMENT(-1);

		auto initialRoi = getRegionOfInterest();
        getTsmPage().onButtonPress(StbRoiTsmPage::SizeDownButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_DECREMENT, newRoi.width() - initialRoi.width());
		EXPECT_EQ(SINGLE_DECREMENT, newRoi.height() - initialRoi.height());
		EXPECT_EQ(newRoi.center(), initialRoi.center());
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMIncreaseRoiButtonRepeated_ThenDecrementSpeedIncreasesBy1)
	{
		const int SINGLE_DECREMENT(-1);
		const int REPEATED_DECREMENT(SINGLE_DECREMENT - 1);

		auto initialRoi = getRegionOfInterest();
        getTsmPage().onButtonPress(StbRoiTsmPage::SizeDownButton, true);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(REPEATED_DECREMENT, newRoi.width() - initialRoi.width());
		EXPECT_EQ(REPEATED_DECREMENT, newRoi.height() - initialRoi.height());
		EXPECT_EQ(newRoi.center(), initialRoi.center());
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, GivenROIAtMinimumSize_ThenDecreaseButtonIsDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(0, 0), Sense::Point(64, 64));

		setRegionOfInterest(initialRoi);
		EXPECT_TRUE(getTsmPage().sizeUpButton->enabled);
		EXPECT_FALSE(getTsmPage().sizeDownButton->enabled);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveUpButtonPressed_ThenRoiMoves1PixelUp)
	{
		const double SINGLE_MOVE(-1.0);

		auto initialRoi = getRegionOfInterest();
        getTsmPage().onButtonPress(StbRoiTsmPage::PanUpButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_MOVE, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_MOVE, newRoi.top - initialRoi.top);
		EXPECT_EQ(newRoi.left, initialRoi.left);
		EXPECT_EQ(newRoi.right, initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveDownButtonPressed_RoiMoves1PixelDown)
	{
		const double SINGLE_MOVE(1.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanDownButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_MOVE, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_MOVE, newRoi.top - initialRoi.top);
		EXPECT_EQ(newRoi.left, initialRoi.left);
		EXPECT_EQ(newRoi.right, initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveLeftButtonPressed_ThenRoiMoves1PixelLeft)
	{
		const double SINGLE_MOVE(-1.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanLeftButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(newRoi.bottom, initialRoi.bottom);
		EXPECT_EQ(newRoi.top, initialRoi.top);
		EXPECT_EQ(SINGLE_MOVE, newRoi.left - initialRoi.left);
		EXPECT_EQ(SINGLE_MOVE, newRoi.right - initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveRightButtonPressed_ThenRoiMoves1PixelRight)
	{
		const double SINGLE_MOVE(1.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanRightButton, false);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(newRoi.bottom, initialRoi.bottom);
		EXPECT_EQ(newRoi.top, initialRoi.top);
		EXPECT_EQ(SINGLE_MOVE, newRoi.left - initialRoi.left);
		EXPECT_EQ(SINGLE_MOVE, newRoi.right - initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveUpButtonRepeated_ThenRoiMoves2PixelUp)
	{
		const double SINGLE_MOVE(-2.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanUpButton, true);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_MOVE, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_MOVE, newRoi.top - initialRoi.top);
		EXPECT_EQ(newRoi.left, initialRoi.left);
		EXPECT_EQ(newRoi.right, initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveDownButtonRepeated_ThenRoiMoves2PixelDown)
	{
		const double SINGLE_MOVE(2.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanDownButton, true);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(SINGLE_MOVE, newRoi.bottom - initialRoi.bottom);
		EXPECT_EQ(SINGLE_MOVE, newRoi.top - initialRoi.top);
		EXPECT_EQ(newRoi.left, initialRoi.left);
		EXPECT_EQ(newRoi.right, initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveLeftButtonRepeated_ThenRoiMoves2PixelLeft)
	{
		const double SINGLE_MOVE(-2.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanLeftButton, true);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(newRoi.bottom, initialRoi.bottom);
		EXPECT_EQ(newRoi.top, initialRoi.top);
		EXPECT_EQ(SINGLE_MOVE, newRoi.left - initialRoi.left);
		EXPECT_EQ(SINGLE_MOVE, newRoi.right - initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenTSMMoveRightButtonRepeated_ThenRoiMoves2PixelRight)
	{
		const double SINGLE_MOVE(2.0);

		auto initialRoi = getRegionOfInterest();
		getTsmPage().onButtonPress(StbRoiTsmPage::PanRightButton, true);
		auto newRoi = getRegionOfInterest();

		EXPECT_EQ(newRoi.bottom, initialRoi.bottom);
		EXPECT_EQ(newRoi.top, initialRoi.top);
		EXPECT_EQ(SINGLE_MOVE, newRoi.left - initialRoi.left);
		EXPECT_EQ(SINGLE_MOVE, newRoi.right - initialRoi.right);
		EXPECT_EQ(viewContext.stbRoiViewer.regionOfInterest, newRoi);
	}

	TEST_P(TestStbRoiDefinitionState, WhenAtRightBoundry_ThenPanRightButtonIsDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(900,50), Sense::Point(1000,150));

		setBoundries(Sense::Rect(0, 0, 1000, 1000));
		setRegionOfInterest(initialRoi);

		EXPECT_TRUE(getTsmPage().panDownButton->enabled);
		EXPECT_TRUE(getTsmPage().panUpButton->enabled);
		EXPECT_TRUE(getTsmPage().panLeftButton->enabled);
		EXPECT_FALSE(getTsmPage().panRightButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}

	TEST_P(TestStbRoiDefinitionState, WhenAtLeftBoundry_ThenPanLeftButtonIsDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(0, 50), Sense::Point(100, 150));

		setBoundries(Sense::Rect(0, 0, 1000, 1000));
		setRegionOfInterest(initialRoi);

		EXPECT_TRUE(getTsmPage().panDownButton->enabled);
		EXPECT_TRUE(getTsmPage().panUpButton->enabled);
		EXPECT_FALSE(getTsmPage().panLeftButton->enabled);
		EXPECT_TRUE(getTsmPage().panRightButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}

	TEST_P(TestStbRoiDefinitionState, AtTopBoundry_ThenPanUpButtonIsDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(25, 0), Sense::Point(125, 100));

		setBoundries(Sense::Rect(0, 0, 1000, 1000));
		setRegionOfInterest(initialRoi);

		EXPECT_TRUE(getTsmPage().panDownButton->enabled);
		EXPECT_FALSE(getTsmPage().panUpButton->enabled);
		EXPECT_TRUE(getTsmPage().panLeftButton->enabled);
		EXPECT_TRUE(getTsmPage().panRightButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}


	TEST_P(TestStbRoiDefinitionState, AtBottomBoundry_ThenPanDownButtonIsDisabled)
	{
		const auto initialRoi = Sense::Rect(Sense::Point(50, 900), Sense::Point(150, 1000));

		setBoundries(Sense::Rect(0, 0, 1000, 1000));
		setRegionOfInterest(initialRoi);
		
		EXPECT_FALSE(getTsmPage().panDownButton->enabled);
		EXPECT_TRUE(getTsmPage().panUpButton->enabled);
		EXPECT_TRUE(getTsmPage().panLeftButton->enabled);
		EXPECT_TRUE(getTsmPage().panRightButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeUpButton->enabled);
		EXPECT_TRUE(getTsmPage().sizeDownButton->enabled);
	}

    TEST_P(TestStbRoiDefinitionState, WhenApplyButtonPressed_ThenReprocessRunWithROI)
    {
        auto selectedRoi = getRegionOfInterest();

        ::testing::InSequence sequence;

        EXPECT_CALL(uiStateController, setStbRegionOfInterest(selectedRoi)).Times(1);
        EXPECT_CALL(uiStateController, stopReplayRun()).Times(1);
        EXPECT_CALL(uiStateController, startReplayRun(false)).Times(1);
		EXPECT_CALL(uiStateController, switchState(std::get<2>(GetParam()))).Times(1);

		getTsmPage().onButtonRelease(StbRoiTsmPage::ApplyButton);
    }

    TEST_P(TestStbRoiDefinitionState, WhenCancelButtonPressed_ThenreviewStateIsSelected)
    {
        EXPECT_CALL(uiStateController, switchState(ViewState::StbContrastRoiDefinitionState)).Times(1);
        EXPECT_CALL(uiStateController, isReplayActive()).Times(1).WillOnce(testing::Return(true));
        EXPECT_CALL(uiStateController, stopReplayRun()).Times(1);
        
        getTsmPage().onButtonRelease(StbRoiTsmPage::DefineROIButton);
    }

    TEST_P(TestStbRoiDefinitionState, WhenStentBoostAvailable_ThenImageIsShownInViewer)
	{
		const int IMAGE_NUMBER(25);

		state->onEnter();

		auto image = createXRayImage(IMAGE_NUMBER);
		auto markers = std::make_shared<StbMarkers>();
		auto boost = std::make_shared<StbImage>();

		state->stentBoostImageAvailable(image, markers, boost);

		EXPECT_EQ(image, viewContext.stbRoiViewer.getImage());
		EXPECT_EQ(image->imageNumber, viewContext.stbRoiViewer.currentFrame);
	}

	INSTANTIATE_TEST_SUITE_P(StbPostDeployRoiDefinition,
		TestStbRoiDefinitionState,
		::testing::Values(std::tuple<ViewState, std::wstring, ViewState>(ViewState::StbPostDeployRoiDefinitionState, L"StbPostDeployRoiDefinitionState", ViewState::StbPostDeployProcessingState)));
	
	INSTANTIATE_TEST_SUITE_P(StbContrastRoiDefinition,
		TestStbRoiDefinitionState,
		::testing::Values(std::tuple<ViewState, std::wstring, ViewState>(ViewState::StbContrastRoiDefinitionState, L"StbContrastRoiDefinitionState", ViewState::StbContrastProcessingState)));
}}