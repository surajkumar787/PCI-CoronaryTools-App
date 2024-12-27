// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "TestUiStateBase.h"
#include "LicenseStatus.h"


class TestUiStateChangesWithLicenseEpxScreen : public TestUiStateBase
{
protected:
	virtual void SetUp() override
	{
		m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);
		m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);
		switchToEpx(PciEpx::Type::NonPci);
		controller.switchState(ViewState::NonPciEpxState);
	}
};

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoRoadmapLicenseAvailable_WhenEpxChangesToRoadmap_NonPciEpxIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::NonPciEpxState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenRoadmapLicenseExpired_WhenEpxChangesToRoadmap_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoStentboostLicenseAvailable_WhenEpxChangesToRoadmap_RoadmapCreationGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::CrmCreationGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenStentboostLicenseExpired_WhenEpxChangesToRoadmap_RoadmapCreationGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Expired);

	switchToEpx(PciEpx::Type::Roadmap);

	ASSERT_EQ(ViewState::CrmCreationGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoStentboostLicenseAvailable_WhenEpxChangesToStentboost_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenStentboostLicenseExpired_WhenEpxChangesToStentboost_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Expired);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoRoadmapLicenseAvailable_WhenEpxChangesToStentboostLive_StentboostLiveGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::StbLiveGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenRoadmapLicenseExpired_WhenEpxChangesToStentboostLive_StentboostLiveGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::Live);

	ASSERT_EQ(ViewState::StbLiveGuidanceState, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoStentboostpostDeployLicenseAvailable_WhenEpxChangesToStentboostPostDeploy_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);

	switchToEpx(PciEpx::SubType::PostDeployment);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenStentboostPostDeployLicenseExpired_WhenEpxChangesToStentboostPostDeploy_NoLicenseIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Expired);

	switchToEpx(PciEpx::SubType::PostDeployment);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoRoadmapLicenseAvailable_WhenEpxChangesToStbPostDeploy_StentboostPostdeployGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::PostDeployment);

	ASSERT_EQ(ViewState::StbPostDeployGuidanceState, controller.getSelectedView());
}


TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenRoadmapLicenseExpired_WhenEpxChangesToStentboostPostDeploy_StentboostPostdeployGuidanceIsShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Active);

	switchToEpx(PciEpx::SubType::PostDeployment);

	ASSERT_EQ(ViewState::StbPostDeployGuidanceState, controller.getSelectedView());
}


TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoStentboostLicenseAvailable_WhenMakingAnExposure_NoLicenseShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);

	switchToEpx(PciEpx::SubType::Live);
	controller.imageRunStarted(XrayImage::Type::Exposure);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}


TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenStentboostLicenseExpired_WhenMakingAnExposure_NoLicensesShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Expired);

	switchToEpx(PciEpx::SubType::Live);
	controller.imageRunStarted(XrayImage::Type::Exposure);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoStbPostDeploymnetLicenseAvailable_WhenMakingAnExposure_NoLicenseShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);

	switchToEpx(PciEpx::SubType::PostDeployment);
	controller.imageRunStarted(XrayImage::Type::Exposure);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}


TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenStbPostDeploymneteExpired_WhenMakingAnExposure_NoLicensesShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Expired);

	switchToEpx(PciEpx::SubType::PostDeployment);
	controller.imageRunStarted(XrayImage::Type::Exposure);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenNoStbContrastLicenseAvailable_WhenMakingAnExposure_NoLicenseShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);

	switchToEpx(PciEpx::SubType::Subtract);
	controller.imageRunStarted(XrayImage::Type::Exposure);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}

TEST_F(TestUiStateChangesWithLicenseEpxScreen, GivenStbContrastExpired_WhenMakingAnExposure_NoLicensesShown)
{
	m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Active);
	m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::Expired);

	switchToEpx(PciEpx::SubType::Subtract);
	controller.imageRunStarted(XrayImage::Type::Exposure);

	ASSERT_EQ(ViewState::NoLicense, controller.getSelectedView());
}
