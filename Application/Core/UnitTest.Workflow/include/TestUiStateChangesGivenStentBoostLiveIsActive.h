// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostLiveIsActive : public TestUiStateBase
{
	protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::SubType::Live);
        switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbLiveProcessingState);
	}
};


#define GIVEN_STB_LIVE(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostLiveIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_STB_LIVE( WhenEpxChangesToRoadmap_StentBoostLiveProcessingRemains       , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE( WhenEpxChangesToNonPciEpx_StentBoostLiveProcessingRemains     , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE( WhenEpxChangesToStentBoost_StentBoostLiveProcessingRemains    , switchToEpx(PciEpx::SubType::PostDeployment)              , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE( WhenEpxChangesToStentBoostSubtract_StentBoostLiveProcessingRemains, switchToEpx(PciEpx::SubType::Subtract)                , ViewState::StbLiveProcessingState);

GIVEN_STB_LIVE( WhenStudyChanged_StentboostGuidanceIsShown                    , controller.studyChanged()                                 , ViewState::StbLiveGuidanceState);

GIVEN_STB_LIVE( WhenRoadmapStatusChangedToRoadmapActive_StentboostLiveIsShown , switchRoadmapStatus(CrmAdministration::Status::Active  )  , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE( WhenRoadmapStatusChangedToNoRoadmap_StentboostLiveIsShown     , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::StbLiveProcessingState);

GIVEN_STB_LIVE( WhenExposureEndsAndNoImagesToReview_StentboostGuidanceIsShown , sendExposureWithEventsFromAlgorithm()                     , ViewState::StbLiveGuidanceState);
GIVEN_STB_LIVE( WhenFluroRunEnds_StentboostLiveIsShown                        , sendFluoro()                                              , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE( WhenStentboostAvialable_StentboostLiveIsShown                 , controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbLiveProcessingState);

GIVEN_STB_LIVE( WhenAngioProcessingStarted_StentboostLiveIsShown              , controller.angioProcessingStarted()                       , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE( WhenAngioProcessingStopped_StentboostLiveIsShown              , controller.angioProcessingStopped()                       , ViewState::StbLiveProcessingState);

TEST_F(TestUiStateChangesGivenStentBoostLiveIsActive, WhenLastImageHoldAndEnoughImagesHaveBeenReceived_StentboostLiveIsShown)
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

	ASSERT_EQ(ViewState::StbLiveReviewState, controller.getSelectedView());
}

GIVEN_STB_LIVE( WhenConnectionStatusChanged_ConnectionFailureIsShown          , controller.connectionStatusChanged(false)                 , ViewState::NoConnection  );
GIVEN_STB_LIVE( WhenConnectionIsRestored_StentboostLiveIsShown                , controller.connectionStatusChanged(true)                  , ViewState::StbLiveProcessingState  );

GIVEN_STB_LIVE(WhenCoRegConnectionLost_StentboostProcessingIsShown            , controller.coRegistrationConnectionChanged(false)         , ViewState::StbLiveProcessingState);
GIVEN_STB_LIVE(WhenCoRegConnectionIsRestored_StentboostProcessingIsShown      , controller.coRegistrationConnectionChanged(true)          , ViewState::StbLiveProcessingState);