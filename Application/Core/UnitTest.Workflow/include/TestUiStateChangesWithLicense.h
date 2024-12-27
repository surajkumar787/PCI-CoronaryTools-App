// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "TestUiStateBase.h"
#include "LicenseStatus.h"

class TestUiStateChangesWithLicense : public TestUiStateBase
{
protected:
	virtual void SetUp() override
	{
		m_licenseCollection.update(LicenseFeature::StbPostDeploy, LicenseStatus::None);
		m_licenseCollection.update(LicenseFeature::StbSubtract, LicenseStatus::None);
		switchToEpx(PciEpx::Type::NonPci);
		switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
	}

	void setLicenseStatus(LicenseStatus crmLicense, LicenseStatus stbLicense)
	{
		m_licenseCollection.update(LicenseFeature::Roadmap, crmLicense);
		m_licenseCollection.update(LicenseFeature::StentboostLive, stbLicense);
	}
};

class TestUiStateChangesWithLicenseCoregScreen : public TestUiStateChangesWithLicense
{
protected:
	virtual void SetUp() override
	{
		TestUiStateChangesWithLicense::SetUp();
		switchToEpx(PciEpx::SubType::Live);
		controller.switchState(ViewState::CoRegistrationState);
	}
};

class TestUiStateChangesWithLicenseStbLiveScreen : public TestUiStateChangesWithLicense
{
protected:
	virtual void SetUp() override
	{
		TestUiStateChangesWithLicense::SetUp();
		switchToEpx(PciEpx::SubType::Live);
		controller.switchState(ViewState::StbLiveProcessingState);
	}
};

class TestUiStateChangesWithLicenseStbGuidanceScreen : public TestUiStateChangesWithLicense
{
protected:
	virtual void SetUp() override
	{
		TestUiStateChangesWithLicense::SetUp();
		switchToEpx(PciEpx::SubType::Live);
		controller.switchState(ViewState::StbLiveProcessingState);
	}
};

#define BASE_FIXTURE(fixture, testName, crmLicense, stbLicense, action, expectedViewState) \
	TEST_F(fixture, testName) \
{ \
	setLicenseStatus(crmLicense, stbLicense); \
	action; \
	ASSERT_EQ (expectedViewState, controller.getSelectedView()); \
}

#define AT_STARTSCREEN(name, crmLicense, stbLicense, action, expectedViewState) \
	BASE_FIXTURE(TestUiStateChangesWithLicense, name, crmLicense, stbLicense, action, expectedViewState)



#define AT_COREGSCREEN(name, crmLicense, stbLicense, action, expectedViewState) \
	BASE_FIXTURE(TestUiStateChangesWithLicenseCoregScreen, name, crmLicense, stbLicense, action, expectedViewState)

#define  AT_STBLIVESCREEN(name, crmLicense, stbLicense, action, expectedViewState) \
	BASE_FIXTURE(TestUiStateChangesWithLicenseStbLiveScreen, name, crmLicense, stbLicense, action, expectedViewState)

#define  AT_STBGUIDANCESCREEN(name, crmLicense, stbLicense, action, expectedViewState) \
	BASE_FIXTURE(TestUiStateChangesWithLicenseStbGuidanceScreen, name, crmLicense, stbLicense, action, expectedViewState)

AT_STARTSCREEN(GivenNoLicenseIsAvailable_WhenPciIsConnected_NoLicenseIsShown,
			   LicenseStatus::None, LicenseStatus::None, controller.connectionStatusChanged(true), ViewState::NoLicense);
AT_STARTSCREEN(GivenLicensesAreExpired_WhenPciIsConnected_NoLicenseIsShown,
			   LicenseStatus::Expired, LicenseStatus::Expired, controller.connectionStatusChanged(true), ViewState::NoLicense);
AT_STARTSCREEN(GivenRoadmapLicenseActiveButNoStenboostLicense_WhenPciIsConnected_NonPciEpxIsShown,
			   LicenseStatus::Active, LicenseStatus::None, controller.connectionStatusChanged(true), ViewState::NonPciEpxState);
AT_STARTSCREEN(GivenRoadmapLicenseActiveButStenboostLicenseExpired_WhenPciIsConnected_NonPciEpxIsShown,
			   LicenseStatus::Active, LicenseStatus::Expired, controller.connectionStatusChanged(true), ViewState::NonPciEpxState);
AT_STARTSCREEN(GivenStentBoostLicenseActiveButNoRoadmapLicense_WhenPciIsConnected_NonPciEpxIsShown,
			   LicenseStatus::None, LicenseStatus::Active, controller.connectionStatusChanged(true), ViewState::NonPciEpxState);
AT_STARTSCREEN(GivenStentboostLicenseActiveButRoadmapLicenseExpired_WhenPciIsConnected_NonPciEpxIsShown,
			   LicenseStatus::Expired, LicenseStatus::Active, controller.connectionStatusChanged(true), ViewState::NonPciEpxState);


// It should only be possible to leave the license screen through an epx or study change
// (assuming you have an active license for the corresponding product)


// below are actions that should not allow you to leave the license screen (no matter the license status).

class GivenTheNoLicenseScreen :
	public TestUiStateChangesWithLicense,
	public ::testing::WithParamInterface<std::tuple<LicenseStatus, LicenseStatus>>
{
public:
	virtual void SetUp() override
	{
		TestUiStateChangesWithLicense::SetUp();
		controller.switchState(ViewState::NoLicense);
	}
};

#define LICENSE_PARAM_TEST(testName, action, expectedView) \
TEST_P(GivenTheNoLicenseScreen, testName) \
{ \
	setLicenseStatus(std::get<0>(GetParam()), std::get<1>(GetParam())); \
	action; \
	ASSERT_EQ (expectedView, controller.getSelectedView()); \
}

LICENSE_PARAM_TEST(WhenRoadmapStatusChanged_NoLicenseIsShown, controller.roadmapStatusChanged(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenGeometryChanged_NoLicenseIsShown, controller.geometryChanged(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenGeometryChangeEnded_NoLicenseIsShown, controller.geometryEnd(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenStenboostImageAvailable_NoLicenseIsShown, controller.stentBoostImageAvailable(liveImage, markers, boostImage), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenOverlayImageAvailable_NoLicenseIsShown, controller.overlayImageAvailable(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenAngioProcessingStarted_NoLicenseIsShown, controller.angioProcessingStarted(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenAngioProcessingStopped_NoLicenseIsShown, controller.angioProcessingStopped(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenLastImageHoldReceived_NoLicenseIsShown, controller.lastImageHold(), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenConnectionLost_NoLicenseIsShown, controller.connectionStatusChanged(false), ViewState::NoLicense);
LICENSE_PARAM_TEST(WhenReconnected_NoLicenseIsShown, controller.connectionStatusChanged(true), ViewState::NoLicense);

// test all combinations of license statuses for crm and stb
INSTANTIATE_TEST_SUITE_P(LicenseVariations, GivenTheNoLicenseScreen,
						::testing::Combine(::testing::Values(LicenseStatus::None, LicenseStatus::Expired, LicenseStatus::Active),
										   ::testing::Values(LicenseStatus::None, LicenseStatus::Expired, LicenseStatus::Active)),
	[](const testing::TestParamInfo<std::tuple<LicenseStatus, LicenseStatus>>& info)
	{
		return std::string("CrmLicense") + (std::get<0>(info.param) == LicenseStatus::Active ? "Active" : std::get<0>(info.param) == LicenseStatus::Expired ? "Expired" : "Inactive") +
		       std::string("_StbLicense") + (std::get<1>(info.param) == LicenseStatus::Active ? "Active" : std::get<1>(info.param) == LicenseStatus::Expired ? "Expired" : "Inactive");
	});