// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TestUiStateBase.h"

class TestUiStateChangesGivenStbContrastGuidanceIsActive : public TestUiStateBase
{
public:

	TestUiStateChangesGivenStbContrastGuidanceIsActive()
	{
	}

	~TestUiStateChangesGivenStbContrastGuidanceIsActive()
	{
	}

protected:

	virtual void SetUp() override
	{
		switchToEpx(PciEpx::SubType::Subtract);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbContrastGuidanceState);
	}

};

class TestUiStateChangesGivenStbContrastGuidanceIsActiveWithNoRoadmapLicence : public TestUiStateBase
{
public:
	TestUiStateChangesGivenStbContrastGuidanceIsActiveWithNoRoadmapLicence() {};

	~TestUiStateChangesGivenStbContrastGuidanceIsActiveWithNoRoadmapLicence() {};

	virtual void SetUp() override
	{
		m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
		switchToEpx(PciEpx::SubType::Subtract);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::StbContrastGuidanceState);
	}
};

#define GIVEN_STBC_GUIDANCE(x,y,z) TEST_F(TestUiStateChangesGivenStbContrastGuidanceIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}
#define GIVEN_STBC_GUIDANCE_NOCRM_LICENCE(x,y,z) TEST_F(TestUiStateChangesGivenStbContrastGuidanceIsActiveWithNoRoadmapLicence, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_STBC_GUIDANCE(WhenEpxChangesToRoadmap_RoadmapGuidanceIsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::CrmCreationGuidanceState);
GIVEN_STBC_GUIDANCE(WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown, switchToEpx(PciEpx::Type::NonPci), ViewState::NonPciEpxState);
GIVEN_STBC_GUIDANCE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown, switchToEpx(PciEpx::SubType::Live), ViewState::StbLiveGuidanceState);
GIVEN_STBC_GUIDANCE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown, switchToEpx(PciEpx::SubType::PostDeployment), ViewState::StbPostDeployGuidanceState);
GIVEN_STBC_GUIDANCE(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown, switchToEpx(PciEpx::SubType::Subtract), ViewState::StbContrastGuidanceState);
GIVEN_STBC_GUIDANCE_NOCRM_LICENCE(WhenEpxChangesToRoadmap_ThenEpxSelectIsShown, switchToEpx(PciEpx::Type::Roadmap), ViewState::NonPciEpxState);



GIVEN_STBC_GUIDANCE(WhenStudyChanged_StentboostGuidanceIsShown, controller.studyChanged(), ViewState::StbContrastGuidanceState);

GIVEN_STBC_GUIDANCE(WhenRoadmapStatusChangedToRoadmapActive_StbGuidanceIsShown, switchRoadmapStatus(CrmAdministration::Status::Active), ViewState::StbContrastGuidanceState);
GIVEN_STBC_GUIDANCE(WhenRoadmapStatusChangedToNoRoadmap_StentboostGuidanceIsShown, switchRoadmapStatus(CrmAdministration::Status::NoRoadmap), ViewState::StbContrastGuidanceState);

GIVEN_STBC_GUIDANCE(WhenLastImageHold_StentboostGuidanceIsShown, controller.lastImageHold(), ViewState::StbContrastGuidanceState);
GIVEN_STBC_GUIDANCE(WhenImageRunStarted_StentboostContrastIsShown, controller.imageRunStarted(XrayImage::Type::Exposure), ViewState::StbContrastProcessingState);

GIVEN_STBC_GUIDANCE(WhenAngioProcessingStarted_StentboostGuidanceIsShown, controller.angioProcessingStarted(), ViewState::StbContrastGuidanceState);
GIVEN_STBC_GUIDANCE(WhenAngioProcessingStopped_StentboostGuidanceIsShown, controller.angioProcessingStopped(), ViewState::StbContrastGuidanceState);

TEST_F(TestUiStateChangesGivenStbContrastGuidanceIsActive, GivenStbGuidance_WhenWhenRoadmapStatusActiveAndOverlayAvailable_RoadmapOverlayIsShown)
{
	viewContext.crmAdminStatus = CrmAdministration::Status::Active;
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::CrmOverlayState, controller.getSelectedView());
}

GIVEN_STBC_GUIDANCE(WhenConnectionStatusChanged_ConnectionFailureIsShown, controller.connectionStatusChanged(false), ViewState::NoConnection);
GIVEN_STBC_GUIDANCE(WhenConnectionIsRestored_StentboostGuidanceIsShown, controller.connectionStatusChanged(true), ViewState::StbContrastGuidanceState);

GIVEN_STBC_GUIDANCE(WhenCoRegConnectionLost_StentboostGuidanceIsShown, controller.coRegistrationConnectionChanged(false), ViewState::StbContrastGuidanceState);
GIVEN_STBC_GUIDANCE(WhenCoRegConnectionIsRestored_StentboostGuidanceIsShown, controller.coRegistrationConnectionChanged(true), ViewState::StbContrastGuidanceState);