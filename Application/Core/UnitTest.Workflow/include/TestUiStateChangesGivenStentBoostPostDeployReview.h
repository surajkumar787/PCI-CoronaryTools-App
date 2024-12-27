// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostPostDeployReviewIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::PostDeployment);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);

		const int IMAGE_WIDTH = 720;
		const int IMAGE_HEIGHT = 720;

		for (int i = 0; i < 10; i++)
		{
			auto img = std::make_shared<CommonPlatform::Xray::XrayImage>();
			auto stbMarkers = std::make_shared<StbMarkers>();
			stbMarkers->valid = true;
			auto boost = std::make_shared<StbImage>();
			boost->resize(IMAGE_WIDTH, IMAGE_HEIGHT);
			img->imageIndex = i;
			stbAdmin.setImage(img, stbMarkers, boost);
		}

		controller.switchState(ViewState::StbPostDeployReviewState);
	}
};


#define GIVEN_STB_POST_DEPLOY_REVIEW(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostPostDeployReviewIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToRoadmap_ReviewRemainsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::StbPostDeployReviewState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToNonPciEpx_ReviewRemainsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::StbPostDeployReviewState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToStentBoost_ReviewRemainsShown, switchToEpx(PciEpx::SubType::Live), ViewState::StbPostDeployReviewState);

GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToRoadmapAndNewRunStarts_CrmProcessingIsShown, switchToEpx(PciEpx::Type::Roadmap); sendExposure(), ViewState::CrmProcessingAngioState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToSBPostAndNewRunStarts_SBLiveProcessingIsShown, switchToEpx(PciEpx::SubType::Live); sendExposure(), ViewState::StbLiveProcessingState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToSBSubtractAndNewRunStarts_SBSubtractProcessingIsShown, switchToEpx(PciEpx::SubType::Subtract); sendExposure(), ViewState::StbContrastProcessingState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenEpxChangesToNonPciEpxAndNewRunStarts_ReviewRemainsShown, switchToEpx(PciEpx::Type::NonPci); sendExposure(), ViewState::StbPostDeployReviewState);


GIVEN_STB_POST_DEPLOY_REVIEW(WhenStudyChanged_StentboostGuidanceIsShown, controller.studyChanged(), ViewState::StbPostDeployGuidanceState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenImageRunStarted_StentboostProcessingIsShown, controller.imageRunStarted(CommonPlatform::Xray::XrayImage::Type::Exposure), ViewState::StbPostDeployProcessingState);

GIVEN_STB_POST_DEPLOY_REVIEW(WhenRoadmapStatusChangedToRoadmapActive_StentboostReviewIsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbPostDeployReviewState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenRoadmapStatusChangedToNoRoadmap_StentboostReviewIsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbPostDeployReviewState);

GIVEN_STB_POST_DEPLOY_REVIEW(WhenFluroRunEnds_StentboostReviewIsShown, sendFluoro(), ViewState::StbPostDeployReviewState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenStentboostAvailable_StentboostReviewIsShown, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbPostDeployReviewState);

GIVEN_STB_POST_DEPLOY_REVIEW(WhenAngioProcessingStarted_StentboostReviewIsShown, controller.angioProcessingStarted(), ViewState::StbPostDeployReviewState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenAngioProcessingStopped_StentboostReviewIsShown, controller.angioProcessingStopped(), ViewState::StbPostDeployReviewState);

GIVEN_STB_POST_DEPLOY_REVIEW(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenConnectionIsRestored_StentboostReviewIsShown, controller.connectionStatusChanged(true), ViewState::StbPostDeployReviewState);

GIVEN_STB_POST_DEPLOY_REVIEW(WhenCoRegConnectionLost_StentboostReviewIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbPostDeployReviewState);
GIVEN_STB_POST_DEPLOY_REVIEW(WhenCoRegConnectionIsRestored_StentboostReviewIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbPostDeployReviewState);