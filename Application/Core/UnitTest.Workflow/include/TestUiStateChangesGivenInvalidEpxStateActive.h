// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

class TestUiStateChangesGivenNonPciEpxStateActive : public TestUiStateBase
{
public:

    TestUiStateChangesGivenNonPciEpxStateActive()
    {
    }

    ~TestUiStateChangesGivenNonPciEpxStateActive()
    {
    }

protected:

    virtual void SetUp() override
    {
        switchToEpx(PciEpx::Type::NonPci);
        switchRoadmapStatus(CrmAdministration::Status::NoRoadmap);
        controller.switchState(ViewState::NonPciEpxState);
    }
};

#define GIVEN_NON_PCI(x,y,z) TEST_F(TestUiStateChangesGivenNonPciEpxStateActive, x) { y; ASSERT_EQ (z, controller.getSelectedView());}


GIVEN_NON_PCI( WhenEpxChangesToRoadmap_RoadmapGuidanceIsShown                  , switchToEpx(PciEpx::Type::Roadmap)								, ViewState::CrmCreationGuidanceState  );
GIVEN_NON_PCI( WhenEpxChangesToNonPciEpx_NonPciEpxStateIsShown                 , switchToEpx(PciEpx::Type::NonPci)								, ViewState::NonPciEpxState    );
GIVEN_NON_PCI( WhenEpxChangesToStentBoostLive_StbLiveGuidanceIsShown           , switchToEpx(PciEpx::SubType::Live)					, ViewState::StbLiveGuidanceState  );
GIVEN_NON_PCI(WhenEpxChangesToStbPostDeploy_StbPostDeploymentGuidanceIsShown   , switchToEpx(PciEpx::SubType::PostDeployment)         , ViewState::StbPostDeployGuidanceState);
GIVEN_NON_PCI(WhenEpxChangesToStbContrast_StbContrastGuidanceIsShown           , switchToEpx(PciEpx::SubType::Subtract)               , ViewState::StbContrastGuidanceState);

GIVEN_NON_PCI( WhenStudyChanged_NonPciEpxStateIsShown                          , controller.studyChanged()										, ViewState::NonPciEpxState    );

GIVEN_NON_PCI( WhenRoadmapStatusChangedToRoadmapActive_RoadmapGuidanceIsShown  , switchRoadmapStatus(CrmAdministration::Status::Active  )		, ViewState::CrmRoadmapActive  );
GIVEN_NON_PCI( WhenRoadmapStatusChangedToRoadmapMoveArc_ApcGuidanceIsShown     , switchRoadmapStatus(CrmAdministration::Status::MoveArc  )		, ViewState::CrmApcGuidanceState	);
GIVEN_NON_PCI( WhenRoadmapStatusChangedToRoadmapMoveTable_TableGuidanceIsShown , switchRoadmapStatus(CrmAdministration::Status::MoveTable)		, ViewState::CrmTableGuidanceState	);
GIVEN_NON_PCI( WhenRoadmapStatusChangedToNoRoadmap_NonPciEpxStateIsShown       , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap)		, ViewState::NonPciEpxState    );

GIVEN_NON_PCI( WhenLastImageHold_NonPciEpxStateIsShown                         , controller.lastImageHold()										, ViewState::NonPciEpxState    );
GIVEN_NON_PCI( WhenStentboostAvialable_NonPciEpxStateIsShown                   , controller.stentBoostImageAvailable(liveImage, markers, boostImage)	, ViewState::NonPciEpxState    );
GIVEN_NON_PCI( WhenOverlayAvailable_NonPciEpxStateIsShown                      , controller.overlayImageAvailable()								, ViewState::NonPciEpxState    );
GIVEN_NON_PCI( WhenAngioProcessingStarted_NonPciEpxStateIsShown                , controller.angioProcessingStarted()							, ViewState::NonPciEpxState    );
GIVEN_NON_PCI( WhenAngioProcessingStopped_NonPciEpxStateIsShown                , controller.angioProcessingStopped()						    , ViewState::NonPciEpxState    );
GIVEN_NON_PCI( WhenAngioProcessingStoppedWithErrors_NonPciEpxStateIsShown      , angioProcessingStopped(CrmAdministration::Status::NoRoadmap,
																									    PciSuite::CrmErrors::Error::NoInjection)			, ViewState::NonPciEpxState		);

GIVEN_NON_PCI( WhenConnectionIsLost_ConnectionFailureIsShown                   , controller.connectionStatusChanged(false)						, ViewState::NoConnection      );
GIVEN_NON_PCI( WhenConnectionIsRestored_RoadmapOverlayIsShown                  , controller.connectionStatusChanged(true)						, ViewState::NonPciEpxState    );