// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "TestUiStateBase.h"

class TestUiStateChangesGivenNoConnectionState : public TestUiStateBase
{
public:

    TestUiStateChangesGivenNoConnectionState()
    {
    }

    ~TestUiStateChangesGivenNoConnectionState()
    {
    }

protected:

    virtual void SetUp() override
    {
        controller.switchState(ViewState::NoConnection);
    }
};

#define GIVEN_NO_CONNECTION(x,y,z) TEST_F(TestUiStateChangesGivenNoConnectionState, x) { y; ASSERT_EQ (z, controller.getSelectedView());}

// Note: when connection failed because of DVLP error, CWIS messages can still be send

GIVEN_NO_CONNECTION( WhenEpxChangesToRoadmap_NoConnectionIsShown                   , switchToEpx(PciEpx::Type::Roadmap)                           , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenEpxChangesToNonPciEpx_NoConnectionIsShown                 , switchToEpx(PciEpx::Type::NonPci)                            , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenEpxChangesToStentBoost_NoConnectionIsShown                , switchToEpx(PciEpx::Type::StentBoost)                        , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenStudyChanged_NoConnectionIsShown                          , controller.studyChanged()                                    , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenRoadmapStatusChangedToRoadmapActive_NoConnectionIsShown   , switchRoadmapStatus(CrmAdministration::Status::Active  )     , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenRoadmapStatusChangedToRoadmapMoveArc_NoConnectionIsShown  , switchRoadmapStatus(CrmAdministration::Status::MoveArc  )    , ViewState::NoConnection		 );
GIVEN_NO_CONNECTION( WhenRoadmapStatusChangedToRoadmapMoveTable_NoConnectionIsShown, switchRoadmapStatus(CrmAdministration::Status::MoveTable)    , ViewState::NoConnection		 );
GIVEN_NO_CONNECTION( WhenRoadmapStatusChangedToNoRoadmap_NoConnectionIsShown       , switchRoadmapStatus(CrmAdministration::Status::NoRoadmap)    , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenLastImageHold_NoConnectionIsShown                         , controller.lastImageHold()                                   , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenStentboostAvialable_NoConnectionIsShown                   , controller.stentBoostImageAvailable(liveImage, markers, boostImage)                        , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenOverlayAvailable_NoConnectionIsShown                      , controller.overlayImageAvailable()                           , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenAngioProcessingStarted_NoConnectionIsShown                , controller.angioProcessingStarted()                          , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenAngioProcessingStopped_NoConnectionIsShown                , controller.angioProcessingStopped()                          , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenGeometryMoves_NoConnectionIsShown                         , controller.geometryChanged()                                 , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenGeometryMoveEnds_NoConnectionIsShown                      , controller.geometryEnd()                                     , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenCoRegStatusChanged_NoConnectionIsShown					   , switchCoRegistrationState(CoRegistrationServerState::SystemState::Live, \
																						CoRegistrationServerState::FmState::IFRSpot)              , ViewState::NoConnection		 );
GIVEN_NO_CONNECTION( WhenConnectionIsLost_NoConnectionIsShown                      , controller.connectionStatusChanged(false)                    , ViewState::NoConnection      );
GIVEN_NO_CONNECTION( WhenConnectionIsRestored_NonPciEpxStateIsShown                , controller.connectionStatusChanged(true)                     , ViewState::NonPciEpxState    );