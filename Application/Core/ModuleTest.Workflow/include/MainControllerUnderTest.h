// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "MainController.h"

using namespace Pci::Core;

class MainControllerUnderTest : public ::Pci::Core::MainController
{
public:
	MainControllerUnderTest(Pci::Core::MainViewer& viewer,
	                        Pci::Core::MainModel& model,
	                        CommonPlatform::Xray::IXraySourceRto &rto,
	                        CommonPlatform::Xray::IXrayGeometryController &apc, 
	                        IScheduler &scheduler, 
	                        CommonPlatform::TSM::ITsmSource& tsm, 
	                        PACS::IStorageCommitListener* storCommitListener, 
	                        PACS::IPACSPush* pacs,
	                        PACS::IMessageBuilder* msgBuild,
							Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		                    IUserPrefs& userPrefs);

	virtual ~MainControllerUnderTest();

	// Test functions to access protected data of MainController class
	virtual Pci::Core::PciEpx&         getEpx() override;
	Pci::Core::PciViewContext& getViewCtx();
	Pci::Core::CrmAdministration& getCrmAdmin();
	virtual Pci::Core::UiStateController& getUiStateController() override;

	virtual void waitForEncoder() override;
};
