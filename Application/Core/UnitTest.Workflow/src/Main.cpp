// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TestViewTypeDetector.h"
#include "TestPciEpx.h"
#include "TestUiStateChangesGivenInvalidEpxStateActive.h"
#include "TestUiStateChangesGivenCrmCreationGuidanceStateIsActive.h"
#include "TestUiStateChangesGivenCrmRoadmapActive.h"
#include "TestUiStateChangesGivenCrmOverlayStateIsActive.h"
#include "TestUiStateChangesGivenCrmErrorStateActive.h"
#include "TestUiStateChangesGivenCrmApcGuideStateIsActive.h"
#include "TestUiStateChangesGivenCrmTableGuideStateIsActive.h"
#include "TestUiStateChangesGivenProcessingAngioStateIsActive.h"
#include "TestUiStateChangesGivenStentBoostLiveGuidanceIsActive.h"
#include "TestUiStateChangesGivenStentBoostPostDeployGuidanceIsActive.h"
#include "TestUiStateChangesGivenStentBoostContrastGuidanceIsActive.h"
#include "TestUiStateChangesGivenStentBoostLiveIsActive.h"
#include "TestUiStateChangesGivenStentBoostPostDeploymentIsActive.h"
#include "TestUiStateChangesGivenStentBoostPostDeployRoiDefinitionIsActive.h"
#include "TestUiStateChangesGivenStentBoostContrastRoiDefinitionIsActive.h"
#include "TestUiStateChangesGivenNoConnectionState.h"
#include "TestUiStateChangesGivenCoRegGuidanceStateIsActive.h"
#include "TestUiStateChangesGivenCoRegistrationStateIsActive.h"
#include "TestUiStateChangesWithLicenseEpxScreen.h"
#include "TestUiStateChangesWithLicenseLicenseScreen.h"
#include "TestUiStateChangesGivenStentBoostContrastProcessingIsActive.h"
#include "TestUiStateChangesGivenStentBoostLiveReviewIsActive.h"
#include "TestUiStateChangesGivenStentBoostPostDeployReview.h"
#include "TestUiStateChangesGivenStentBoostContrastReview.h"

#include "TestCrmOverlayState.h"
#include "TestCrmRoadmapActiveState.h"
#include "TestStbRoiDefinitionState.h"
#include "TestStbLiveReviewState.h"
#include "TestStbPostDeployReviewState.h"
#include "TestStbContrastReviewState.h"
#include "TestStbContrastFrameSelectionState.h"
#include "TestStbBoostFrameSelectionState.h"
#include "TestStbContrastProcessingState.h"
#include "TestStbPostDeployProcessingState.h"
#include "TestStbLiveProcessingState.h"
#include "TestUiStateChangesWithLicense.h"
#include "TestNoLicenseState.h"
#include "TestUiStateController.h"

#define _WINSOCKAPI_ 
#include <Windows.h>

int main(int argc, CHAR *argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}