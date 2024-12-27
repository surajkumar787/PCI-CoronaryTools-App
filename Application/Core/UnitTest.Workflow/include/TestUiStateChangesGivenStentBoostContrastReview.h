// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostContrastReviewIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::Subtract);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);

		for (int i = 0; i < 10; i++)
		{
			auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
			auto stbMarkers = std::make_shared<StbMarkers>();
			stbMarkers->valid = true;
			auto boost = std::make_shared<StbImage>();
			img->imageIndex = i;
			stbAdmin.setImage(img, stbMarkers, boost);
		}

		controller.switchState(ViewState::StbContrastReviewState);
	}
};


#define GIVEN_STB_CONTRAST_REVIEW(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostContrastReviewIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToRoadmap_ReviewRemainsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::StbContrastReviewState);
GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToNonPciEpx_ReviewRemainsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::StbContrastReviewState);
GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToStentBoost_ReviewRemainsShown, switchToEpx(PciEpx::SubType::Live), ViewState::StbContrastReviewState);

GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToRoadmapAndNewRunStarts_CrmProcessingIsShown, switchToEpx(PciEpx::Type::Roadmap); sendExposure(), ViewState::CrmProcessingAngioState);
GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToSBPostAndNewRunStarts_SBPostProcessingIsShown, switchToEpx(PciEpx::SubType::PostDeployment); sendExposure(), ViewState::StbPostDeployProcessingState);
GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToSBLiveAndNewRunStarts_SBLiveProcessingIsShown, switchToEpx(PciEpx::SubType::Live); sendExposure() , ViewState::StbLiveProcessingState);
GIVEN_STB_CONTRAST_REVIEW(WhenEpxChangesToNonPciEpxAndNewRunStarts_ReviewRemainsShown, switchToEpx(PciEpx::Type::NonPci); sendExposure(), ViewState::StbContrastReviewState);

GIVEN_STB_CONTRAST_REVIEW(WhenStudyChanged_StentboostGuidanceIsShown, controller.studyChanged(), ViewState::StbContrastGuidanceState);
GIVEN_STB_CONTRAST_REVIEW(WhenImageRunStarted_StentboostProcessingIsShown, controller.imageRunStarted(CommonPlatform::Xray::XrayImage::Type::Exposure), ViewState::StbContrastProcessingState);

GIVEN_STB_CONTRAST_REVIEW(WhenRoadmapStatusChangedToRoadmapActive_StentboostReviewIsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbContrastReviewState);
GIVEN_STB_CONTRAST_REVIEW(WhenRoadmapStatusChangedToNoRoadmap_StentboostReviewIsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbContrastReviewState);

GIVEN_STB_CONTRAST_REVIEW(WhenFluroRunEnds_StentboostReviewIsShown, sendFluoro(), ViewState::StbContrastReviewState);
GIVEN_STB_CONTRAST_REVIEW(WhenStentboostAvailable_StentboostReviewIsShown, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbContrastReviewState);

GIVEN_STB_CONTRAST_REVIEW(WhenAngioProcessingStarted_StentboostReviewIsShown, controller.angioProcessingStarted(), ViewState::StbContrastReviewState);
GIVEN_STB_CONTRAST_REVIEW(WhenAngioProcessingStopped_StentboostReviewIsShown, controller.angioProcessingStopped(), ViewState::StbContrastReviewState);

GIVEN_STB_CONTRAST_REVIEW(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STB_CONTRAST_REVIEW(WhenConnectionIsRestored_StentboostReviewIsShown, controller.connectionStatusChanged(true), ViewState::StbContrastReviewState);

GIVEN_STB_CONTRAST_REVIEW(WhenCoRegConnectionLost_StentboostReviewIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbContrastReviewState);
GIVEN_STB_CONTRAST_REVIEW(WhenCoRegConnectionIsRestored_StentboostReviewIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbContrastReviewState);
