// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "MainControllerTestServer.h"


using namespace ::Pci::Core;
using namespace CommonPlatform::Xray;

MainControllerTestServer::MainControllerTestServer(MainViewer& viewer,
                                                   MainModel& model,
                                                   IXraySourceRto &rto,
                                                   IXrayGeometryController &apc,
                                                   const std::wstring &grabber,
                                                   const std::wstring &screenshotFolder, 
                                                   IScheduler &scheduler,
                                                   Gpu &gpu,
                                                   TSM::ITsmSource& tsm,
                                                   PACS::IStorageCommitListener* storCommitListener,
                                                   PACS::IPACSPush* pacsPush,
                                                   PACS::IMessageBuilder* msgBuilder,
                                                   bool supportSnapshots,
                                                   bool supportMovies,
                                                   PciExpressTestServer& test,
                                                   bool showStateControlUtility,
												   SystemType systemType,
	                                               iControlInterface::ICopyToRefClient& copyToRefClient,
												   IUserPrefs& userPrefs) :
	MainController(viewer, model, rto, apc, grabber, screenshotFolder, scheduler, tsm, storCommitListener, pacsPush, msgBuilder, supportSnapshots, supportMovies,
					 systemType, copyToRefClient, userPrefs),
    testServer(test)
{
    testServer.setCwisConnectionState(false);
    testServer.setDvlpConnectionState(false);
    testServer.setLastUserMessage("Ik ben gestart.");
    testServer.setApplicationState(ApplicationState());

    getUiStateController().eventViewStateChanged = [&](ViewState /*viewState*/)
	{
		testServer.setGuidanceState(getUiStateController().getSelectedViewName());
		testServer.setViewState(getUiStateController().getSelectedViewName());
		testServer.setRoadmapActive(getCrmAdministration().getStatus() == CrmAdministration::Status::Active);
	};

#ifdef INCLUDE_TEST_AND_DEMO_FUNCTIONALITY
// define this definition in propertysheet.
	view.crmGuidance.eventGuidanceStateChanged = [this](GuidanceState crmGuidanceState, CrmAdministration::Status crmAdminStatus)
	{
		testServer.setGuidanceState(view.crmGuidance.getGuidanceStateName());
		testServer.setRoadmapActive(crmAdminStatus == CrmAdministration::Status::Active);
		if (m_stateControlUtility) m_stateControlUtility->onGuidanceStateChanged(crmGuidanceState, crmAdminStatus);
	};
#endif

	if (showStateControlUtility)
	{
		m_stateControlUtility.reset(new StateControlUtility(*this, gpu, scheduler));
	}
}

void MainControllerTestServer::connectionChanged(bool status, IXraySourceRto::CwisStatus cwisStatus, IXraySourceRto::DvlpStatus dvlpStatus, const std::wstring &cwisHost , const  std::wstring &dvlpAdapter)
{
	MainController::connectionChanged(status,cwisStatus,dvlpStatus, cwisHost, dvlpAdapter);

	testServer.setCwisConnectionState( cwisStatus == IXraySourceRto::CwisStatus::Connected);
	testServer.setDvlpConnectionState( dvlpStatus == IXraySourceRto::DvlpStatus::Connected);
}