// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostLiveReviewIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::Live);
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

		controller.switchState(ViewState::StbLiveReviewState);
	}
};


#define GIVEN_STB_LIVE_REVIEW(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostLiveReviewIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToRoadmap_ReviewRemainsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::StbLiveReviewState);
GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToNonPciEpx_ReviewRemainsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::StbLiveReviewState);
GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToStentBoost_ReviewRemainsShown, switchToEpx(PciEpx::SubType::Live), ViewState::StbLiveReviewState);

GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToRoadmapAndNewRunStarts_CrmProcessingIsShown, switchToEpx(PciEpx::Type::Roadmap); sendExposure(), ViewState::CrmProcessingAngioState);
GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToSBPostAndNewRunStarts_SBPostProcessingIsShown, switchToEpx(PciEpx::SubType::PostDeployment); sendExposure(), ViewState::StbPostDeployProcessingState);
GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToSBSubtractAndNewRunStarts_SBSubtractProcessingIsShown, switchToEpx(PciEpx::SubType::Subtract); sendExposure(), ViewState::StbContrastProcessingState);
GIVEN_STB_LIVE_REVIEW(WhenEpxChangesToNonPciEpxAndNewRunStarts_ReviewRemainsShown, switchToEpx(PciEpx::Type::NonPci); sendExposure(), ViewState::StbLiveReviewState);

GIVEN_STB_LIVE_REVIEW(WhenStudyChanged_StentboostGuidanceIsShown, controller.studyChanged(), ViewState::StbLiveGuidanceState);
GIVEN_STB_LIVE_REVIEW(WhenImageRunStarted_StentboostProcessingIsShown, sendExposure(), ViewState::StbLiveProcessingState);

GIVEN_STB_LIVE_REVIEW(WhenRoadmapStatusChangedToRoadmapActive_StentboostLiveIsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbLiveReviewState);
GIVEN_STB_LIVE_REVIEW(WhenRoadmapStatusChangedToNoRoadmap_StentboostLiveIsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbLiveReviewState);

GIVEN_STB_LIVE_REVIEW(WhenFluroRunEnds_StentboostLiveIsShown, sendFluoro(), ViewState::StbLiveReviewState);
GIVEN_STB_LIVE_REVIEW(WhenStentBoostavailable_reviewStateIsShown, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbLiveReviewState);

GIVEN_STB_LIVE_REVIEW(WhenAngioProcessingStarted_StentboostLiveIsShown, controller.angioProcessingStarted(), ViewState::StbLiveReviewState);
GIVEN_STB_LIVE_REVIEW(WhenAngioProcessingStopped_StentboostLiveIsShown, controller.angioProcessingStopped(), ViewState::StbLiveReviewState);

GIVEN_STB_LIVE_REVIEW(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STB_LIVE_REVIEW(WhenConnectionIsRestored_StentboostLiveIsShown, controller.connectionStatusChanged(true), ViewState::StbLiveReviewState);

GIVEN_STB_LIVE_REVIEW(WhenCoRegConnectionLost_StentboostReviewIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbLiveReviewState);
GIVEN_STB_LIVE_REVIEW(WhenCoRegConnectionIsRestored_StentboostReviewIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbLiveReviewState);