// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "StbImage.h"
#include "StbMarkers.h"

class TestUiStateChangesGivenStentBoostContrastRoiDefinitionIsActive : public TestUiStateBase
{
protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::Subtract);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbContrastRoiDefinitionState);
	}
};



#define GIVEN_STBC_ROI(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostContrastRoiDefinitionIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_STBC_ROI(WhenEpxChangesToRoadmap_StentBoostRoiDefinitionRemainsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::StbContrastRoiDefinitionState);
GIVEN_STBC_ROI(WhenEpxChangesToNonPciEpx_StentBoostRoiDefinitionRemainsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::StbContrastRoiDefinitionState);

GIVEN_STBC_ROI(WhenStudyChanged_StentboostContrastGuidanceIsShown, controller.studyChanged(), ViewState::StbContrastGuidanceState);

GIVEN_STBC_ROI(WhenRoadmapStatusChangedToRoadmapActive_StentboostRoiDefinitionRemainsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbContrastRoiDefinitionState);
GIVEN_STBC_ROI(WhenRoadmapStatusChangedToNoRoadmap_StentboostRoiDefinitionRemainsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbContrastRoiDefinitionState);

GIVEN_STBC_ROI(WhenFluroRunEnds_StentboostRoiDefinitionIsShown, sendFluoro(), ViewState::StbContrastRoiDefinitionState);
GIVEN_STBC_ROI(WhenStentboostAvailable_StentboostRoiDefinitionIsShown, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::StbContrastRoiDefinitionState);

GIVEN_STBC_ROI(WhenAngioProcessingStarted_StentBoostRoiDefinitionIsShown, controller.angioProcessingStarted(), ViewState::StbContrastRoiDefinitionState);
GIVEN_STBC_ROI(WhenAngioProcessingStopped_StentBoostRoiDefinitionIsShown, controller.angioProcessingStopped(), ViewState::StbContrastRoiDefinitionState);

TEST_F(TestUiStateChangesGivenStentBoostContrastRoiDefinitionIsActive, GivenStbLive_WhenRoadmapStatusActiveAndOverlayAvailable_RoadmapOverlayIsShown)
{
	viewContext.crmAdminStatus = CrmAdministration::Status::Active;
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::CrmOverlayState, controller.getSelectedView());
}

GIVEN_STBC_ROI(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STBC_ROI(WhenConnectionIsRestored_StentboostRoiDefinitionIsShown, controller.connectionStatusChanged(true), ViewState::StbContrastRoiDefinitionState);

GIVEN_STBC_ROI(WhenCoRegConnectionLost_StentboostRoiDefinitionIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbContrastRoiDefinitionState);
GIVEN_STBC_ROI(WhenCoRegConnectionIsRestored_StentboostRoiDefinitionIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbContrastRoiDefinitionState);