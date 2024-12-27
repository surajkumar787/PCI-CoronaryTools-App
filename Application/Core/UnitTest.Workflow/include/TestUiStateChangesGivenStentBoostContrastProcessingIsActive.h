// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostContrastProcessingIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::Subtract);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbContrastProcessingState);
	}
};

#define GIVEN_STBC_PROCESSING(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostContrastProcessingIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_STBC_PROCESSING(WhenEpxChangesToRoadmap_StentboostContrastProcessingRemains,	switchToEpx(PciEpx::Type::Roadmap)					, ViewState::StbContrastProcessingState);
GIVEN_STBC_PROCESSING(WhenEpxChangesToNonPciEpx_StentboostContrastProcessingRemains, switchToEpx(PciEpx::Type::NonPci)						, ViewState::StbContrastProcessingState);
GIVEN_STBC_PROCESSING(WhenEpxChangesToStentboostPostDeployment_StentboostContrastProcessingRemains, switchToEpx(PciEpx::SubType::PostDeployment), ViewState::StbContrastProcessingState);
GIVEN_STBC_PROCESSING(WhenEpxChangesToStentboostLive_StentboostContrastProcessingRemains, switchToEpx(PciEpx::SubType::Live), ViewState::StbContrastProcessingState);


GIVEN_STBC_PROCESSING(WhenStudyChanged_StentboostGuidanceIsShown								, controller.studyChanged()								, ViewState::StbContrastGuidanceState);

GIVEN_STBC_PROCESSING(WhenRoadmapStatusChangedToRoadmapActive_StentboostContrastProcessingIsShown	, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbContrastProcessingState);
GIVEN_STBC_PROCESSING(WhenRoadmapStatusChangedToNoRoadmap_StentboostContrastProcessingIsShown	, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbContrastProcessingState);

GIVEN_STBC_PROCESSING(WhenExposureEndsAndNoImagesToReview_StentboostContrastGuidanceIsShown	    , sendExposureWithEventsFromAlgorithm()					, ViewState::StbContrastGuidanceState);
GIVEN_STBC_PROCESSING(WhenStentboostAvailable_StentboostContrastProcessingIsShown				, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbContrastProcessingState);

GIVEN_STBC_PROCESSING(WhenAngioProcessingStarted_StentboostContrastProcessingIsShown			, controller.angioProcessingStarted()					, ViewState::StbContrastProcessingState);
GIVEN_STBC_PROCESSING(WhenAngioProcessingStopped_StentboostContrastProcessingIsShown			, controller.angioProcessingStopped()					, ViewState::StbContrastProcessingState);

TEST_F(TestUiStateChangesGivenStentBoostContrastProcessingIsActive, WhenLastImageHoldAndEnoughImagesHaveBeenReceived_StentboostContrastReviewIsShown)
{
	for (int i = 0; i < 10; i++)
	{
		auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
		auto stbMarkers = std::make_shared<StbMarkers>();
		stbMarkers->valid = true;
		auto boost = std::make_shared<StbImage>();
		img->imageIndex = i;
		controller.stentBoostImageAvailable(img, stbMarkers, boost);
	}
	controller.endLive();
	controller.lastImageHold();

	ASSERT_EQ(ViewState::StbContrastReviewState, controller.getSelectedView());
}

GIVEN_STBC_PROCESSING(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STBC_PROCESSING(WhenConnectionIsRestored_StentboostPostDeploymentIsShown, controller.connectionStatusChanged(true), ViewState::StbContrastProcessingState);

GIVEN_STBC_PROCESSING(WhenCoRegConnectionLost_StentboostProcessingIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbContrastProcessingState);
GIVEN_STBC_PROCESSING(WhenCoRegConnectionIsRestored_StentboostProcessingIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbContrastProcessingState);