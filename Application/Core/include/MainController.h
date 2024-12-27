// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Sense.h"
#include <ExperienceIdentity.h>

#include "CrmAdministration.h"
#include "CrmParameters.h"
#include "CrmRepository.h"
#include "CrmApcController.h"

#include "StbParameters.h"
#include "StbAdministration.h"

#include "PciThreadBackground.h"
#include "PciThreadLive.h"
#include "PciThreadStentBoost.h"
#include "PciThreadGrabber.h"
#include "PciReviewPlayer.h"
#include "PciReviewParameters.h"

#include "IXraySourceRto.h"
#include "IXrayGeometryController.h"
#include "PciViewContext.h"

#include "CoRegistrationAdministration.h"
#include "CoRegistrationSource.h"
#include "PullbackRecording.h"

#include "UiStateController.h"
#include "IArchivingStatus.h"
#include "AppLauncherClient.h"
#include "Thread.h"

#include "XRayImageRecorder.h"
#include "XRayImageRecording.h"

#include "IReplayControl.h"
#include "iacquisitionnotification.h"

namespace Cwis { enum class CwisConnectionType; }
namespace Pci { namespace Platform { namespace iControl 
{
	class ICopyToRefClient;
}}}

namespace Pci { namespace Core
{

namespace PACS
{
	class IStorageCommitListener;
	class IPACSPush;
	class IMessageBuilder;
}

using CommonPlatform::Xray::IXraySourceRto;
using CommonPlatform::Xray::IXrayGeometryController;

namespace Coreg = ::CommonPlatform::CoRegistration;
namespace iControlInterface = ::Pci::Platform::iControl;

class MainModel;
class MainViewer;

class IArchivingSettings;
class IUserPrefs;

class MainController : private IArchivingStatus, public IAsync, public IReplayControl, public IAcquisition::IAcquisitionNotification
{
public:
	MainController(MainViewer& viewer,
	               MainModel& model,
	               IXraySourceRto &rto,
	               IXrayGeometryController &apc,
	               const std::wstring &grabber,
	               const std::wstring &screenshotFolder,
	               IScheduler &scheduler,
	               TSM::ITsmSource& tsm,
	               PACS::IStorageCommitListener* storCommitListener,
	               PACS::IPACSPush* pacsPush,
	               PACS::IMessageBuilder* msgBuilder,
	               bool supportSnapshots,
	               bool supportMovies,
	               SystemType systemType,
				   iControlInterface::ICopyToRefClient& copyToRefClient,
				   IUserPrefs& userPrefs);

	virtual ~MainController();

	void setImage   (const std::shared_ptr<CommonPlatform::Xray::XrayImage>& image, bool skip);
	void setGeometry(const CommonPlatform::Xray::XrayGeometry &geometry);
	void setEpx     (const CommonPlatform::Xray::XrayEpx      &epx);
	void setStudy	(const CommonPlatform::Xray::XrayStudy    &study);

    void setIfrPullbackCycle(const Coreg::CoRegistrationIfrPullbackCycle &cycle);
	void setIfrSpotCycle	(const Coreg::CoRegistrationIfrSpotCycle &cycle);
	void setIfrSpotResult	(float ifrSpotResult);
    void setCoRegState      (const Coreg::CoRegistrationServerState &coRegState);
	void setPressureAndEcg  (const Coreg::CoRegistrationPressureAndEcg &pressAndEcg);
	void setPdPaInfo		(const Coreg::CoRegistrationPdPaInfo &pdPaInfo);
	void setSpotMeasurement	(const Coreg::CoRegistrationSpotMeasurement &spot);
	void setCoRegError		(Coreg::CoRegistrationError error);
	void setCoRegOptions	(const Coreg::CoRegistrationOptions &options);
	void setSelectedCoRegRun(int runIndex);


	void endGeometry();
	void beginRun  (CommonPlatform::Xray::XrayImage::Type type);
	void endRun    ();

    virtual void connectionChanged(bool status, IXraySourceRto::CwisStatus cwisStatus, IXraySourceRto::DvlpStatus dvlpStatus, const std::wstring &cwisHost, const  std::wstring &dvlpAdapter);
	void setCoRegConnection(Coreg::CoRegistrationSource::ConnectionStatus connectionStatus);

	void tsmConnectionChanged(Cwis::CwisConnectionType connection);

    void setApcConnectionChanged    (bool connected);
    void setExternalApcCwisActivity (IXrayGeometryController::ApcExternalCwisActivity activity);
    void setTsm3dApcButtonPressed   ();

	void startXRayReplay(bool loop) override;
	void stopXRayReplay() override;
	bool isActive() override;

	void setTsmFunctionsAvailable(bool available);

protected:
    void close();
	void asyncInvoke(const std::function<void()> &command) const override;

	void    onFieldService();
	void    onGenerateProblemReport();
	void    onShowAboutBoxRoadmap();
	void    onShowAboutBoxStentBoost();
	void    startupXrayConnectionTimeout();
	void    saveSceenshot();

	virtual void setSnapshotStatus(ArchivingStatus status, CaptureArea area) override;
	virtual void setMovieStatus(ArchivingStatus status) override;
	virtual void clearStatus() override;

	void    onSnapshotStatusTimer();
	void    onMovieStatusTimer();
	void    createScreenshotFile(const std::wstring& filename);
	void    onStbProcessingTimeout();
	void    onExportBusyUpdate();
	void    logXrayConnectionStatus(ViewXrayConnectionStatus viewXrayConnectionStatus,
	                                IXraySourceRto::CwisStatus cwisStatus,
	                                IXraySourceRto::DvlpStatus dvlpStatus,
	                                const std::wstring &cwisHost,
	                                const std::wstring &dvlpAdapter);



	void    handleSystemStateChanged(Coreg::CoRegistrationServerState::SystemState prevState,
		                             Coreg::CoRegistrationServerState::SystemState newState);
	void    updateCoRegStatusText();
	CrmAdministration& getCrmAdministration();
	PciViewContext& getPciViewContext();
	virtual PciEpx&		getEpx();
	virtual UiStateController& getUiStateController();
	virtual void waitForEncoder();

private:
	::CommonPlatform::Log& m_log;

	SystemType systemType;
	MainViewer& view;
	MainModel&  model;

    IXraySourceRto&   rto;
	CrmParameters     crmParams;
    CrmRepository     crmRepository;

	CrmAdministration crmAdministration;

	CoRegistrationAdministration        coRegAdmin;
	PciReviewParameters			        coregReviewParams;
	PciReviewParameters			        xrayRecordingPlayerParams;
	PciReviewPlayer<PullbackRecording>  coRegPlayer;
	PciReviewPlayer<XRayImageRecording> xrayRecordingPlayer;
	
	CrmApcController    crmApcController;

	StbParameters		stbParamsLive;
	StbParameters		stbParamsPost;
	PciReviewParameters	stbReviewParams;
	StbAdministration	stbAdmin;

	IScheduler&			scheduler;
	PciThreadBackground	threadBackground;
	PciThreadLive		threadLive;
	PciThreadStentBoost threadStentBoost;
	PciThreadGrabber	threadGrabber;

	std::shared_ptr<XRayImageRecorder> xrayRecorder;

	CommonPlatform::Thread threadEncoding;

	PACS::IStorageCommitListener* m_storageCommitListener;
	PACS::IMessageBuilder* msgBuilder;
	PACS::IPACSPush* pacsPush;

	CommonPlatform::Xray::XrayImage::Type     acquisitionType;

	PciViewContext	    viewContext;

	Sense::Timer        xrayConnectionTimer;

	PciEpx				epx;
	UiStateController   uiStateController;

	std::unique_ptr<CommonPlatform::AppLauncher::AppLauncherClient> appLauncherClient;
	iControlInterface::ICopyToRefClient& copyToRefClient;

	Sense::Timer snapshotStatusTimer;
	Sense::Timer movieStatusTimer;
	std::wstring screenshotFolder;

	Sense::Timer stbProcessingTimeoutTimer;
	long lastFrame;
	long lastStbProcessedFrame;
	bool unreliableCoRegConnection;
	Sense::Timer exportBusyTimer;

	bool acquistionInProgress;
	std::unique_ptr<SscfTs1::EventLoop> serverLoop;
	std::unique_ptr<SscfTs1::ServerNode> serverNode;
	bool IsInProgress() override;

	bool dvlpBecomesDisconnected(IXraySourceRto::DvlpStatus dvlpStatus, bool dvlpConnected) const;
	bool cwisBecomesDisconnected(IXraySourceRto::CwisStatus cwisStatus, bool cwisConnected) const;
	bool dvlpBecomesConnected(IXraySourceRto::DvlpStatus dvlpStatus, bool dvlpConnected) const;
	bool cwisBecomesConnected(IXraySourceRto::CwisStatus cwisStatus, bool cwisConnected) const;

    void updateStudyForDynamicCoronaryRoadmap(const XrayStudy& study);
    void updateStudyForStentBoostLive();
};

}}