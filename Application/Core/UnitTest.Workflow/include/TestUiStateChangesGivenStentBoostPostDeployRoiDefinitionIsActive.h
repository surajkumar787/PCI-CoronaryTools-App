// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostPostDeployRoiDefinitionIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::PostDeployment);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbPostDeployRoiDefinitionState);
	}
};



#define GIVEN_STB_ROI(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostPostDeployRoiDefinitionIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_STB_ROI(WhenEpxChangesToRoadmap_StentBoostRoiDefinitionRemainsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::StbPostDeployRoiDefinitionState);
GIVEN_STB_ROI(WhenEpxChangesToNonPciEpx_StentBoostRoiDefinitionRemainsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::StbPostDeployRoiDefinitionState);

GIVEN_STB_ROI(WhenStudyChanged_StentboostGuidanceIsShown, controller.studyChanged(), ViewState::StbPostDeployGuidanceState);

GIVEN_STB_ROI(WhenRoadmapStatusChangedToRoadmapActive_StentboostRoiDefinitionrRemainsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbPostDeployRoiDefinitionState);
GIVEN_STB_ROI(WhenRoadmapStatusChangedToNoRoadmap_StentboostRoiDefinitionrRemainsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbPostDeployRoiDefinitionState);

GIVEN_STB_ROI(WhenFluroRunEnds_StentboostRoiDefinitionIsShown, sendFluoro(), ViewState::StbPostDeployRoiDefinitionState);
GIVEN_STB_ROI(WhenStentboostAvailable_StentboostRoiDefinitionIsShown, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbPostDeployRoiDefinitionState);

GIVEN_STB_ROI(WhenAngioProcessingStarted_StentBoostRoiDefinitionIsShown, controller.angioProcessingStarted(), ViewState::StbPostDeployRoiDefinitionState);
GIVEN_STB_ROI(WhenAngioProcessingStopped_StentBoostRoiDefinitionIsShown, controller.angioProcessingStopped(), ViewState::StbPostDeployRoiDefinitionState);

TEST_F(TestUiStateChangesGivenStentBoostPostDeployRoiDefinitionIsActive, GivenStbLive_WhenRoadmapStatusActiveAndOverlayAvailable_RoadmapOverlayIsShown)
{
	viewContext.crmAdminStatus = CrmAdministration::Status::Active;
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::CrmOverlayState, controller.getSelectedView());
}

GIVEN_STB_ROI(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STB_ROI(WhenConnectionIsRestored_StentboostRoiDefinitionIsShown, controller.connectionStatusChanged(true), ViewState::StbPostDeployRoiDefinitionState);

GIVEN_STB_ROI(WhenCoRegConnectionLost_StentboostRoiDefinitionIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbPostDeployRoiDefinitionState);
GIVEN_STB_ROI(WhenCoRegConnectionIsRestored_StentboostRoiDefinitionIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbPostDeployRoiDefinitionState);