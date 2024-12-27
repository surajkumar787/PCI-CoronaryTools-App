// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostPostDeployIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::PostDeployment);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbPostDeployProcessingState);
	}
};



#define GIVEN_STB_POST(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostPostDeployIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_STB_POST(WhenStudyChanged_StentboostGuidanceIsShown								, controller.studyChanged()								, ViewState::StbPostDeployGuidanceState);

GIVEN_STB_POST(WhenEpxChangesToRoadmap_StentboostPostdeploymentRemains, switchToEpx(PciEpx::Type::Roadmap), ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenEpxChangesToNonPciEpx_StentboostPostdeploymentRemains, switchToEpx(PciEpx::Type::NonPci), ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenEpxChangesToStentboostSubtract_StentboostPostdeploymentRemains, switchToEpx(PciEpx::SubType::Subtract), ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenEpxChangesToStentboostLive_StentboostPostdeploymentRemains, switchToEpx(PciEpx::SubType::Live), ViewState::StbPostDeployProcessingState);


GIVEN_STB_POST(WhenRoadmapStatusChangedToRoadmapActive_StentboostPostDeploymentIsShown	, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenRoadmapStatusChangedToNoRoadmap_StentboostPostDeploymentIsShown		, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbPostDeployProcessingState);

GIVEN_STB_POST(WhenExposureEndsAndNoImagesToReview_StentboostGuidanceIsShown			, sendExposureWithEventsFromAlgorithm()					, ViewState::StbPostDeployGuidanceState);
GIVEN_STB_POST(WhenFluroRunEnds_StentboostPostDeploymentIsShown							, sendFluoro()											, ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenStentboostAvailable_StentboostPostDeploymentIsShown					, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbPostDeployProcessingState);

GIVEN_STB_POST(WhenAngioProcessingStarted_StentboostPostDeploymentIsShown				, controller.angioProcessingStarted()					, ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenAngioProcessingStopped_StentboostPostDeploymentIsShown				, controller.angioProcessingStopped()					, ViewState::StbPostDeployProcessingState);

TEST_F(TestUiStateChangesGivenStentBoostPostDeployIsActive, WhenLastImageHoldAndEnoughImagesHaveBeenReceived_StentboostLiveIsShown)
{
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
		controller.stentBoostImageAvailable(img, stbMarkers, boost);
	}
	controller.endLive();
	controller.lastImageHold();

	ASSERT_EQ(ViewState::StbPostDeployReviewState, controller.getSelectedView());
}

GIVEN_STB_POST(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STB_POST(WhenConnectionIsRestored_StentboostPostDeploymentIsShown, controller.connectionStatusChanged(true), ViewState::StbPostDeployProcessingState);

GIVEN_STB_POST(WhenCoRegConnectionLost_StentboostProcessingIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbPostDeployProcessingState);
GIVEN_STB_POST(WhenCoRegConnectionIsRestored_StentboostProcessingIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbPostDeployProcessingState);