// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "TestUiStateBase.h"
#include "LicenseStatus.h"


class TestUiStateChangesWithLicenseLicenseScreen : public TestUiStateBase
{
protected:
	virtual void SetUp() override
	{
		m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);
		m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);
		m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);
		switchToEpx(PciEpx::Type::NonPci);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
		controller.switchState(ViewState::NoLicense);
	}
};


TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenRoadmapLicenseActiveButNoStentboostLicense_WhenEpxChangesToRoadmap_RoadmapCreationGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::CrmCreationGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenRoadmapLicenseActiveButStentboostLicenseExpired_WhenEpxChangesToRoadmap_RoadmapCreationGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Expired);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::CrmCreationGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostLicenseActiveButNoRoadmapLicense_WhenEpxChangesToStentboostLive_StentboostLiveIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::StbLiveGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostLicenseActiveButRoadmapLicenseExpired_WhenEpxChangesToStentboostLive_StentboostLiveIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::StbLiveGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStbPostdeploymentLicensePostActiveButNoRoadmapLicense_WhenEpxChangesToStentboostPostDeploy_StentboostPostDeployIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::PostDeployment);

	ASSERT_EQ(ViewState::StbPostDeployGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStbPostdeploymentLicenseActiveButRoadmapLicenseExpired_WhenEpxChangesToStentboostPostDeploy_StentboostPostDeployIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::PostDeployment);

	ASSERT_EQ(ViewState::StbPostDeployGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostContrastLicenseActiveButNoRoadmapLicense_WhenEpxChangesToStentboostContrast_StentboostContrastIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Subtract);

	ASSERT_EQ(ViewState::StbContrastGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostConrtastLicenseActiveButRoadmapLicenseExpired_WhenEpxChangesToSTbContrast_StentboostContrastIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Subtract);

	ASSERT_EQ(ViewState::StbContrastGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenRoadmapLicenseActiveButNoStentboostLicense_WhenStudyChanged_NonPciEpxIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	controller.studyChanged();

	ASSERT_EQ(ViewState::NonPciEpxState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenRoadmapLicenseActiveButStentboostLicenseExpired_WhenStudyChanged_NonPciEpxIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Expired);

	controller.studyChanged();

	ASSERT_EQ(ViewState::NonPciEpxState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostLicenseActiveButNoRoadmapLicense_WhenStudyChanged_NonPciEpxIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	controller.studyChanged();

	ASSERT_EQ(ViewState::NonPciEpxState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostLicenseActiveButRoadmapLicenseExpired_WhenStudyChanged_NonPciEpxIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	controller.studyChanged();

	ASSERT_EQ(ViewState::NonPciEpxState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenNoLicenseAtAll_WhenEpxChangesToRoadmap_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenNoLicenseAtAll_WhenEpxChangesToStentboost_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenNoLicenseAtAll_WhenEpxChangesToNonPci_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::Type::NonPci);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostLicenseActiveAndRoadMapLicenseDisabled_WhenMakingAnFluro_StbLiveIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Live);
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::StbLiveGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseLicenseScreen, GivenStentboostPostDeployLicenseActiveAndRoadMapLicenseDisabled_WhenMakingAnFluro_StbPostDeploymnetIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::PostDeployment);
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::StbPostDeployGuidanceState, controller.getSelectedView());
}
