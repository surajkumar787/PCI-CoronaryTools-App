// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "TestUiStateBase.h"

class TestUiStateChangesGivenStbPostDeployGuidanceIsActive : public TestUiStateBase
{
public:

	TestUiStateChangesGivenStbPostDeployGuidanceIsActive()
	{
	}

	~TestUiStateChangesGivenStbPostDeployGuidanceIsActive()
	{
	}

protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::PostDeployment);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbPostDeployGuidanceState);
	}

};

class TestUiStateChangesGivenStbPostDeployGuidanceIsActiveWithNoRoadmapLicence : public TestUiStateBase
{
public:
	TestUiStateChangesGivenStbPostDeployGuidanceIsActiveWithNoRoadmapLicence() {};

	~TestUiStateChangesGivenStbPostDeployGuidanceIsActiveWithNoRoadmapLicence() {};

	virtual void SetUp() override
	{
		m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
		switchToEpx(PciEpx::SubType::PostDeployment);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbPostDeployGuidanceState);
	}
};

#define GIVEN_STB_PDGUIDANCE(x,y,z) TEST_F(TestUiStateChangesGivenStbPostDeployGuidanceIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}
#define GIVEN_STB_PDGUIDANCE_NOCRM_LICENCE(x,y,z) TEST_F(TestUiStateChangesGivenStbPostDeployGuidanceIsActiveWithNoRoadmapLicence, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_STB_PDGUIDANCE(WhenEpxChangesToRoadmap_RoadmapGuidanceIsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::CrmCreationGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::NonPciEpxState);
GIVEN_STB_PDGUIDANCE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown, switchToEpx(PciEpx::SubType::Live), ViewState::StbLiveGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown, switchToEpx(PciEpx::SubType::PostDeployment), ViewState::StbPostDeployGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown, switchToEpx(PciEpx::SubType::Subtract), ViewState::StbContrastGuidanceState);
GIVEN_STB_PDGUIDANCE_NOCRM_LICENCE(WhenEpxChangesToRoadmap_NoCRoadmapLicens_ThenEpxSelectIsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::NonPciEpxState);



GIVEN_STB_PDGUIDANCE(WhenStudyChanged_StentboostGuidanceIsShown, controller.studyChanged(), ViewState::StbPostDeployGuidanceState);

GIVEN_STB_PDGUIDANCE(WhenRoadmapStatusChangedToRoadmapActive_StbGuidanceIsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbPostDeployGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenRoadmapStatusChangedToNoRoadmap_StentboostGuidanceIsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbPostDeployGuidanceState);

GIVEN_STB_PDGUIDANCE(WhenLastImageHold_StentboostGuidanceIsShown, controller.lastImageHold(), ViewState::StbPostDeployGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenImageRunStarted_StentboostPostDeploymentIsShown, controller.imageRunStarted(XrayImage::Type::Exposure), ViewState::StbPostDeployProcessingState);

GIVEN_STB_PDGUIDANCE(WhenAngioProcessingStarted_StentboostGuidanceIsShown, controller.angioProcessingStarted(), ViewState::StbPostDeployGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenAngioProcessingStopped_StentboostGuidanceIsShown, controller.angioProcessingStopped(), ViewState::StbPostDeployGuidanceState);

TEST_F(TestUiStateChangesGivenStbPostDeployGuidanceIsActive, GivenStbGuidance_WhenWhenRoadmapStatusActiveAndOverlayAvailable_RoadmapOverlayIsShown)
{
	viewContext.crmAdminStatus = CrmAdministration::Status::Active;
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::CrmOverlayState, controller.getSelectedView());
}

GIVEN_STB_PDGUIDANCE(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STB_PDGUIDANCE(WhenConnectionIsRestored_StentboostGuidanceIsShown, controller.connectionStatusChanged(true), ViewState::StbPostDeployGuidanceState);

GIVEN_STB_PDGUIDANCE(WhenCoRegConnectionLost_StentboostGuidanceIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbPostDeployGuidanceState);
GIVEN_STB_PDGUIDANCE(WhenCoRegConnectionIsRestored_StentboostGuidanceIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbPostDeployGuidanceState);