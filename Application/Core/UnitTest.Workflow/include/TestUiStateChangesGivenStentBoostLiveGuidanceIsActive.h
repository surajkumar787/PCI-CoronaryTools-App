// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"
#include "LicensesCollection.h"

class TestUiStateChangesGivenStentBoostLiveGuidanceIsActive : public TestUiStateBase
{
public:

    TestUiStateChangesGivenStentBoostLiveGuidanceIsActive()
    {
    }

    ~TestUiStateChangesGivenStentBoostLiveGuidanceIsActive()
    {
    }

protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::SubType::Live);
        switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
        controller.switchState(ViewState::StbLiveGuidanceState);
    }
};

class TestUiStateChangesGivenStentBoostGuidanceIsActiveWithNoRoadmapLicence  : public TestUiStateBase
{
public:
	TestUiStateChangesGivenStentBoostGuidanceIsActiveWithNoRoadmapLicence() {};
	~TestUiStateChangesGivenStentBoostGuidanceIsActiveWithNoRoadmapLicence(){};

	virtual void SetUp() override
	{
		m_licenseCollection.update(LicenseFeature::Roadmap, LicenseStatus::None);
		switchToEpx(PciEpx::SubType::Live);
        switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
        controller.switchState(ViewState::StbLiveGuidanceState);
	}
};

#define GIVEN_STB_GUIDANCE(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostLiveGuidanceIsActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}
#define GIVEN_STB_GUIDANCE_NOCRM_LICENCE(x,y,z) TEST_F(TestUiStateChangesGivenStentBoostGuidanceIsActiveWithNoRoadmapLicence, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

GIVEN_STB_GUIDANCE( WhenEpxChangesToRoadmap_RoadmapGuidanceIsShown                  , switchToEpx(PciEpx::Type::Roadmap)                        , ViewState::CrmCreationGuidanceState );
GIVEN_STB_GUIDANCE( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                 , switchToEpx(PciEpx::Type::NonPci)                         , ViewState::NonPciEpxState   );
GIVEN_STB_GUIDANCE(WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown			, switchToEpx(PciEpx::SubType::Live)				, ViewState::StbLiveGuidanceState);
GIVEN_STB_GUIDANCE(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown	, switchToEpx(PciEpx::SubType::PostDeployment)	, ViewState::StbPostDeployGuidanceState);
GIVEN_STB_GUIDANCE(WhenEpxChangesToStbContrast_StbCOntrastGuidanceIsShown           , switchToEpx(PciEpx::SubType::Subtract)          , ViewState::StbContrastGuidanceState);

GIVEN_STB_GUIDANCE_NOCRM_LICENCE( WhenEpxChangesToRoadmap_NoCRoadmapLicens_ThenEpxSelectIsShown   , switchToEpx(PciEpx::Type::Roadmap)			, ViewState::NonPciEpxState   );


GIVEN_STB_GUIDANCE( WhenStudyChanged_StentboostGuidanceIsShown                      , controller.studyChanged()                                 , ViewState::StbLiveGuidanceState);

GIVEN_STB_GUIDANCE( WhenRoadmapStatusChangedToRoadmapActive_StbGuidanceIsShown      , switchRoadmapStatus(CrmAdministration::Status::Active  )  , ViewState::StbLiveGuidanceState);
GIVEN_STB_GUIDANCE( WhenRoadmapStatusChangedToNoRoadmap_StentboostGuidanceIsShown   , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap) , ViewState::StbLiveGuidanceState);

GIVEN_STB_GUIDANCE( WhenLastImageHold_StentboostGuidanceIsShown                     , controller.lastImageHold()                                , ViewState::StbLiveGuidanceState);
GIVEN_STB_GUIDANCE( WhenImageRunStarted_StentboostLiveIsShown                        , controller.imageRunStarted(XrayImage::Type::Exposure)    , ViewState::StbLiveProcessingState);

GIVEN_STB_GUIDANCE( WhenAngioProcessingStarted_StentboostGuidanceIsShown            , controller.angioProcessingStarted()                       , ViewState::StbLiveGuidanceState);
GIVEN_STB_GUIDANCE( WhenAngioProcessingStopped_StentboostGuidanceIsShown            , controller.angioProcessingStopped()                       , ViewState::StbLiveGuidanceState);

TEST_F(TestUiStateChangesGivenStentBoostLiveGuidanceIsActive, GivenStbGuidance_WhenWhenRoadmapStatusActiveAndOverlayAvailable_RoadmapOverlayIsShown)
{
	viewContext.crmAdminStatus = CrmAdministration::Status::Active;
	controller.overlayImageAvailable();

	ASSERT_EQ(ViewState::CrmOverlayState, controller.getSelectedView());
}

GIVEN_STB_GUIDANCE( WhenConnectionStatusChanged_ConnectionFailureIsShown            , controller.connectionStatusChanged(false)                 , ViewState::NoConnection      );
GIVEN_STB_GUIDANCE( WhenConnectionIsRestored_StentboostGuidanceIsShown              , controller.connectionStatusChanged(true)                  , ViewState::StbLiveGuidanceState);

GIVEN_STB_GUIDANCE(WhenCoRegConnectionLost_StentboostGuidanceIsShown                , controller.coRegistrationConnectionChanged(false)         , ViewState::StbLiveGuidanceState);
GIVEN_STB_GUIDANCE(WhenCoRegConnectionIsRestored_StentboostGuidanceIsShown          , controller.coRegistrationConnectionChanged(true)          , ViewState::StbLiveGuidanceState);