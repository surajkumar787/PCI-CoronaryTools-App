// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "TestUiState.h"
#include "NoLicenseState.h"

namespace Pci { namespace Core
{

class TestNoLicenseState : public TestUiState
{
public:
	TestNoLicenseState() :
		TestUiState()
	{}

	void SetUp() override
	{
		state = std::unique_ptr<NoLicenseState>(new NoLicenseState(viewContext));
	}

	void enterState()
	{
		state->onEnter();
	}

	const std::wstring STATE_NAME = L"NoLicense";

	std::unique_ptr<NoLicenseState> state;
};

TEST_F(TestNoLicenseState, StateNameIsCorrect)
{
	EXPECT_EQ(state->getStateName(), STATE_NAME);
}

TEST_F(TestNoLicenseState, WhenEnteringTheState_ThenNoLicenseGuidanceViewerIsVisible)
{
	enterState();

	EXPECT_TRUE(viewContext.noLicenseView.visible);
}

TEST_F(TestNoLicenseState, GivenNoValidLicensesAtAll_WhenEnteringTheState_ThenGuidanceTextNoLicenseAvailableShown)
{
	licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
    licenseCollection.update(LicenseFeature::Coregistration, LicenseStatus::None);
	licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);
	licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);
	licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);

	std::vector<PciEpx> licensedEPXs;
	licensedEPXs.emplace_back(PciEpx::Type::Roadmap);
	licensedEPXs.emplace_back(PciEpx::SubType::Live);
	licensedEPXs.emplace_back(PciEpx::SubType::PostDeployment);
	licensedEPXs.emplace_back(PciEpx::SubType::Subtract);

	for (auto& epx : licensedEPXs)
	{
		viewContext.currentEpx = epx;
		enterState();

		auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

		ASSERT_NE(guidanceModel, nullptr);
		EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_NoLicense);
		EXPECT_EQ(guidanceModel->messageText, L"");
		EXPECT_EQ(guidanceModel->messageText, L"");
	}
}

TEST_F(TestNoLicenseState, GivenNoValidSBLLicense_WhenSBLEPXSelected_ThenGuidanceTextNoSBLLicenseShown)
{
    licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);
	viewContext.currentEpx = PciEpx(PciEpx::SubType::Live);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_NoLicenseStbLive);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, Pci::Core::Localization::Translation::IDS_ContactServiceProvider);
}

TEST_F(TestNoLicenseState, GivenNoValidSBLicense_WhenSBEPXSelected_ThenGuidanceTextNoSBLicenseShown)
{
    licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::None);
    licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);
    licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);
	viewContext.currentEpx = PciEpx(PciEpx::SubType::PostDeployment);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_NoLicenseStbPostDeployment);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, Pci::Core::Localization::Translation::IDS_ContactServiceProvider);
}

TEST_F(TestNoLicenseState, GivenNoValidSBSLicense_WhenSBSEPXSelected_ThenGuidanceTextNoSBSLicenseShown)
{
    licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);
	viewContext.currentEpx = PciEpx(PciEpx::SubType::Subtract);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_NoLicenseStbSubtract);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, Pci::Core::Localization::Translation::IDS_ContactServiceProvider);
}

TEST_F(TestNoLicenseState, GivenNoValidCrmLicense_WhenCoronaryEPXSelected_ThenGuidanceTextNoLicenseShown)
{
    licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
	viewContext.currentEpx = PciEpx(PciEpx::Type::Roadmap);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_NoLicense);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, L"");
}

TEST_F(TestNoLicenseState, GivenExpiredStbLiveLicense_WhenStbLiveEPXSelected_ThenGuidanceTextsStbLiveLicenseExpiredShown)
{
	licenseCollection.update(LicenseFeature::StentboostLive, LicenseStatus::Expired);
	viewContext.currentEpx = PciEpx(PciEpx::SubType::Live);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_StbLiveLicenseExpired);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, L"");
}

TEST_F(TestNoLicenseState, GivenExpiredStentBoostLicense_WhenStentBoostEPXSelected_ThenGuidanceTextStentBoostLicenseExpiredShown)
{
	licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::Expired);
	viewContext.currentEpx = PciEpx(PciEpx::SubType::PostDeployment);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_StbPostDeploymentLicenseExpired);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, L"");
}

TEST_F(TestNoLicenseState, GivenExpiredStbSubtractLicense_WhenStbSubtractEPXSelected_ThenGuidanceTextStbSubtractLicenseExpiredShown)
{
	licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::Expired);
	viewContext.currentEpx = PciEpx(PciEpx::SubType::Subtract);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_StbSubtractLicenseExpired);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, L"");
}

TEST_F(TestNoLicenseState, GivenExpiredCrmLicense_WheCrmEPXSelected_ThenGuidanceTextCrmLicenseExpiredShown)
{
	licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::Expired);
	viewContext.currentEpx = PciEpx(PciEpx::Type::Roadmap);

	enterState();

	auto guidanceModel = viewContext.noLicenseView.getGuidanceModel();

	ASSERT_NE(guidanceModel, nullptr);
	EXPECT_EQ(guidanceModel->messageHeaderText, Pci::Core::Localization::Translation::IDS_CrmLicenseExpired);
	EXPECT_EQ(guidanceModel->messageText, L"");
	EXPECT_EQ(guidanceModel->guidanceText, L"");
}

}}
