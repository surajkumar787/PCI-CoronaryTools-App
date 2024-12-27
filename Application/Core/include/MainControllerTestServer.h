// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "MainController.h"
#include "StateControlUtility.h"

#include <PciExpressTestServer.h>
#include <memory>

namespace Pci { namespace Core {

enum class GuidanceState;

class MainControllerTestServer : public MainController
{
public:
	MainControllerTestServer(MainViewer& viewer,
	                         MainModel& model,
	                         CommonPlatform::Xray::IXraySourceRto &rto,
	                         CommonPlatform::Xray::IXrayGeometryController &apc,
	                         const std::wstring &grabber,
	                         const std::wstring &screenshotFolder,
	                         IScheduler &scheduler,
	                         Gpu &gpu,
	                         TSM::ITsmSource& tsm,
	                         PACS::IStorageCommitListener* storCommitListener,
	                         PACS::IPACSPush* pacs,
	                         PACS::IMessageBuilder* msgBuilder,
	                         bool supportSnapshots,
	                         bool supportMovies,
	                         PciExpressTestServer& test,
	                         bool showStateControlUtility,
							 SystemType systemType,
							 iControlInterface::ICopyToRefClient& copyToRefClient,
							 IUserPrefs& userPrefs);

	virtual ~MainControllerTestServer() {}

	MainControllerTestServer& operator=(const MainControllerTestServer&) = delete;  // Disallow copying
	MainControllerTestServer(const MainControllerTestServer&) = delete;

protected:
	virtual void connectionChanged(bool status, CommonPlatform::Xray::IXraySourceRto::CwisStatus cwisStatus, CommonPlatform::Xray::IXraySourceRto::DvlpStatus dvlpStatus, const std::wstring &cwisHost , const  std::wstring &dvlpAdapter) override;

private:
	PciExpressTestServer& testServer;
	std::unique_ptr<StateControlUtility> m_stateControlUtility;

	friend class StateControlUtility;
};

}}