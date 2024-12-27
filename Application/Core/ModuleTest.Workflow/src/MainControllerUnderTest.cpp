// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MainControllerUnderTest.h"
#include "LicenseStatus.h"
#include "SystemType.h"

using namespace CommonPlatform::Xray;

MainControllerUnderTest::MainControllerUnderTest(MainViewer& viewer,
	                                             MainModel& model,
	                                             IXraySourceRto &rto,
	                                             IXrayGeometryController &apc,
	                                             IScheduler &scheduler,
	                                             CommonPlatform::TSM::ITsmSource& tsm,
	                                             PACS::IStorageCommitListener* storCommitListener,
	                                             PACS::IPACSPush* pacs,
	                                             PACS::IMessageBuilder* msgBuild,
												 Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
												 IUserPrefs& userPrefs) :
	MainController(viewer,
	               model,
	               rto,
	               apc,
	               L"",
	               L"",
	               scheduler,
	               tsm,
	               storCommitListener,
	               pacs,
	               msgBuild,
	               false,
	               false,
				   Pci::Core::SystemType::Allura,
				   copyToRefClient,
		           userPrefs)
{
}


MainControllerUnderTest::~MainControllerUnderTest()
{
}

Pci::Core::PciEpx& MainControllerUnderTest::getEpx()
{
    return MainController::getEpx();
}

Pci::Core::PciViewContext& MainControllerUnderTest::getViewCtx()
{
	return MainController::getPciViewContext();
}

Pci::Core::CrmAdministration& MainControllerUnderTest::getCrmAdmin()
{
	return MainController::getCrmAdministration();
}

Pci::Core::UiStateController& MainControllerUnderTest::getUiStateController()
{
	return MainController::getUiStateController();
}

void MainControllerUnderTest::waitForEncoder()
{
	MainController::waitForEncoder();
}