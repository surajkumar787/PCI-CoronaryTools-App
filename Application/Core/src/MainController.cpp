// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <thread>
#include <chrono>

#include "MainController.h"
#include "MainModel.h"
#include "AppLauncherClient.h"
#include "PciEpx.h"
#include "CwisProtocol.h"
#include "Build.hpp"
#include "LicensesCollection.h"

#include "IPACSPush.h"
#include "MessageBuilder.h"
#include "IStorageCommitListener.h"

#include "Resources.h"
#include "Translation.h"

#include "UtilizationLogEvent.h"
#include "SystemType.h"
#include "AboutInfo.h"
#include "MainViewer.h"
#include "IArchivingStatus.h"
#include "XRayImageRecording.h"

#include "ICopyToRefClient.h"
#include "IUserPrefs.h"

#include <Shlwapi.h>

using namespace Pci::Core;
using namespace CommonPlatform;
using namespace CommonPlatform::CoRegistration;
using namespace CommonPlatform::Xray;

namespace Pci { namespace Core
{

static const double XrayConnectionTimeout    = 4 * 60.0; /* secs. Timeout large enough to not display failure when both Allura and PCI have a cold restart */
static const double ArchivingBusyTimeout     = 120.0;
static const double ArchivingCompleteTimeout = 10.0;


wchar_t* getCwisStatusString(IXraySourceRto::CwisStatus cwisStatus);
wchar_t* getDvlpStatusString(IXraySourceRto::DvlpStatus dvlpStatus);


MainController::MainController(MainViewer& viewer,
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
							   IUserPrefs& userPrefs) :
	serverLoop(std::make_unique<SscfTs1::EventLoop>()),
	serverNode(std::make_unique<SscfTs1::ServerNode>(*serverLoop, "127.0.0.1", "urn:philips-igt-pci:acquisitionservice:1")),
	systemType               (systemType),
	view                     (viewer),
	model                    (model),
	m_log                    (::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
	rto                      (rto),
	crmParams                (L"crm.ini", L"../Configuration/settings.ini"),
	crmRepository            (crmParams.storeRoadmapFolder),
	crmAdministration        (crmParams, crmRepository),
	crmApcController         (apc, userPrefs.getApcAutoActivation(), crmAdministration.getStudyID(), crmAdministration.getPersistent3dApcAutomaticActivation()),
	stbParamsLive            (L"stb.ini", L"../Configuration/settings.ini"),
	stbParamsPost            (L"stb_post.ini", L"../Configuration/settings.ini"),
	stbReviewParams          (stbParamsLive.getReviewParameters()),
	stbAdmin                 (stbReviewParams),
	scheduler                (scheduler),
	coregReviewParams        (-1, 0.0, 0, 600),
	coRegPlayer              (scheduler, coregReviewParams),
	xrayRecordingPlayerParams(1, 0.0, 0, 0),
	xrayRecordingPlayer      (scheduler, xrayRecordingPlayerParams),
	coRegAdmin               (coRegPlayer),
	threadBackground         (scheduler, crmParams, crmRepository),
	threadLive               (scheduler, crmParams, stbParamsLive, systemType, coRegAdmin.getLatencyCompensation()),
	threadStentBoost		     (scheduler,stbParamsPost,systemType),
	threadGrabber            (scheduler, grabber, 999, true),
	xrayRecorder             (std::make_shared<XRayImageRecorder>()),
	threadEncoding           (scheduler, "Encoder"),
	m_storageCommitListener  (storCommitListener),
	msgBuilder               (msgBuilder),
	pacsPush                 (pacsPush),
	viewContext              (view.crmViewer, view.stbViewer, view.stbAngio, view.stbRoiViewer, view.stbContrastViewer, view.stbContrastFrameSelectionView, view.stbBoostFrameSelectionView, view.coRegViewer, view.crmGuidance, view.crmRoadmapSelectViewer, view.stbGuidance, view.coRegGuidance, view.coRegControl,
	                          coRegPlayer, stbReviewParams, stbAdmin, view.pciEpxView, view.noConnection, view.noLicenseView, epx, model.getLicenseCollection(),
	                          pacsPush, msgBuilder, &threadEncoding, crmAdministration, supportSnapshots, supportMovies, model.getSystemType()),
	acquisitionType          (XrayImage::Type::None),
	uiStateController        (viewContext, scheduler, tsm, *this, copyToRefClient, userPrefs, this),
	copyToRefClient			 (copyToRefClient),
	xrayConnectionTimer      (scheduler, [this] { startupXrayConnectionTimeout(); }),
	snapshotStatusTimer      (scheduler, [this] { onSnapshotStatusTimer(); }),
	movieStatusTimer         (scheduler, [this] { onMovieStatusTimer(); }),
	screenshotFolder         (screenshotFolder),
	stbProcessingTimeoutTimer(scheduler, [this] { onStbProcessingTimeout(); }),
	lastFrame                (0),
	lastStbProcessedFrame    (0),
	unreliableCoRegConnection(false),
	exportBusyTimer(scheduler, [this] { onExportBusyUpdate(); })
{
	m_log.DeveloperInfo(L"PCI viewer, build %s, created on %s", BuildRevision, BuildDate);

	serverNode->registerSingleton<IAcquisitionNotification>(this);
	serverNode->listen();

	view.crmViewer.blendFactor   = crmParams.blendFactor;
	view.coRegViewer.blendFactor = crmParams.blendFactor;

	view.eventClickService =         [this] { onFieldService(); };
	view.eventClickProblemReport =   [this] { onGenerateProblemReport(); };
	view.eventClickAboutRoadmap =    [this] { onShowAboutBoxRoadmap(); };
	view.eventClickAboutStentBoost = [this] { onShowAboutBoxStentBoost(); };

	view.stbViewer.showUpdateDelayed = false;

	if (m_storageCommitListener != nullptr)
	{
		m_storageCommitListener->onStoreFailure = [this](const PACS::StorageReport& report)
		{
			std::wstringstream strm;
			strm << L"Received a storage failure status for " << report.TransactionUid << L" With items:" << std::endl;
			for (const auto& item : report.Items)
			{
				strm << L"Sop Class: " << item.SopClass << L" Sop Instance: " << item.SopInstance << std::endl;
			}
			m_log.DeveloperWarning(strm.str().c_str());
		};
	}

	if (m_storageCommitListener != nullptr)
	{
		m_storageCommitListener->onStoreSuccess = [this](const PACS::StorageReport& report)
		{
			std::wstringstream strm;
			strm << L"Received a storage success status for " << report.TransactionUid << L" With items:" << std::endl;
			for (const auto& item : report.Items)
			{
				strm << L"Sop Class: " << item.SopClass << L" Sop Instance: " << item.SopInstance << std::endl;
			}
			m_log.DeveloperInfo(strm.str().c_str());
		};
	}

    crmApcController.onApcModeStatusChanged = [this]( CrmApcController::ApcModeStatus apcStatus)
    {
		viewContext.crmApcStatus = apcStatus;

		// make APC status persistent
		crmAdministration.setPersistent3dApcAutomaticActivation( crmApcController.getAutomaticActivation() );
		uiStateController.apcStatusChanged();
    };

	threadLive.onRoadmap = [this](const std::shared_ptr<const XrayImage> &image, const std::shared_ptr<const CrmOverlay> &overlay)
	{
		viewContext.roadmapErrorState = CrmErrors::Error::OK;
		viewContext.fluoroAcquiredWhileRecording = true;

		if (viewContext.licenseCollection.isActive(LicenseFeature::Coregistration))
		{
			view.coRegControl.setImage(image);
			view.coRegViewer.setImage(image);
			view.coRegViewer.setOverlay(overlay);
			coRegAdmin.setImage(image, overlay);
		}

		if (crmAdministration.getStatus() == CrmAdministration::Status::Active)
		{
			crmAdministration.setImage(image, overlay);
			
			view.crmViewer.setImage(image);
			view.crmViewer.setOverlay(overlay);
		}
		uiStateController.overlayImageAvailable();
	};

	threadLive.onStentBoost = [this](const std::shared_ptr<const XrayImage>	&image, const std::shared_ptr<const StbMarkers> &markers, const std::shared_ptr<const StbImage> &boost)
	{
		if ( stbProcessingTimeoutTimer.isActive() )
		{
			view.stbViewer.showUpdateDelayed = false;
		}

		stbProcessingTimeoutTimer.start(stbParamsLive.processingTimeout);
		lastStbProcessedFrame = lastFrame;

		if (boost->isValid())
		{
			threadGrabber.addBoost(boost);
		}

		uiStateController.stentBoostImageAvailable(image, markers, boost);
	};

	threadStentBoost.onLiveStent = [this](const std::shared_ptr<const XrayImage>& image, const std::shared_ptr<const StentMarkers>& markers)
	{
		if (stbProcessingTimeoutTimer.isActive())
		{
			view.stbViewer.showUpdateDelayed = false;
		}

		stbProcessingTimeoutTimer.start(stbParamsLive.processingTimeout);
		lastStbProcessedFrame = lastFrame;

		auto stbMarker = std::make_shared<const StbMarkers>(*markers);
		auto boost = std::make_shared<StbImage>();
		boost->imageNumber = image->imageNumber;

		uiStateController.stentBoostImageAvailable(image, stbMarker, boost);
	};

	threadStentBoost.onStentBoost = [this](const std::shared_ptr<const StbImage>& boost, const std::shared_ptr<const StbImage>& registeredLive)
	{
		if (boost->isValid())
		{
			threadGrabber.addBoost(boost);
		}
		uiStateController.boostImageAvailable(boost, registeredLive);
	};

	threadStentBoost.onContrastScoreAvailable = [this](const std::vector<std::tuple<int,float>> score)
	{
		stbAdmin.setContrastScore(score);
	};

	threadLive.onEnd = [this]()
	{
		lastFrame = 0;
		lastStbProcessedFrame = 0;

		if (!xrayRecordingPlayer.inReview())
		{
			uiStateController.lastImageHold();
		}
	};

	threadStentBoost.onEnd = [this]()
	{
		lastFrame = 0;
		lastStbProcessedFrame = 0;

		if (!xrayRecordingPlayer.inReview())
		{
			uiStateController.lastImageHold();
		}
	};
	
	threadBackground.onBusyChanged = [this]()
	{
		if(threadBackground.isBusy()) 
		{
			uiStateController.angioProcessingStarted();
		}
		else
		{
			uiStateController.angioProcessingStopped();
		}
	};

	threadBackground.onRoadmapCreated = [this](const std::shared_ptr<const CrmRoadmap> &roadmap)
	{
		// Test is new roadmap has equal or better quality that current.
		viewContext.roadmapErrorState = crmAdministration.testRoadmap(roadmap);

		if (viewContext.roadmapErrorState == CrmErrors::Error::OK)
		{
			crmAdministration.addRoadmap(roadmap);
			threadGrabber.addRoadmap(roadmap);
		}
	};

	threadBackground.onRoadmapError = [this](CrmErrors::Error error)
	{
		viewContext.roadmapErrorState = error;
	};

	crmAdministration.onRoadmapStatusChanged = [this]
	{ 
		viewContext.roadmapErrorState = CrmErrors::Error::OK;

		CrmAdministration::Status status = crmAdministration.getStatus();
		const auto &roadmap              = crmAdministration.getRoadmap();

		m_log.DeveloperInfo(L"PciViewer: onRoadmapStatusChanged [AdminStatus=%d, RunIndex=%d, RoadmapActive=%s, MoveArc=%s]",
			static_cast<int>(status),
			roadmap ? roadmap->runIndex : -1,
			status == CrmAdministration::Status::Active ? L"True" : L"False", 
			status == CrmAdministration::Status::MoveArc ? L"True" : L"False");

		threadLive.setRoadmap(status == CrmAdministration::Status::Active ? roadmap : nullptr);

		viewContext.crmAdminStatus = status;
		viewContext.crmAdminIsEmpty = crmAdministration.isEmpty();

		CrmAdministration::Status adminStatus = crmAdministration.getStatus();

		// Update APC with information of roadmap administration after a new run was created.
		// But do not program APC while the geometry is moving and the target roadmap is changed. 
		// If geometry is not moving then it is a new created roadmap.
		if ( !viewContext.geometryIsMoving )
		{
			crmApcController.setApc(adminStatus == CrmAdministration::Status::MoveArc ? roadmap : nullptr); 
		}

		viewContext.crmApcStatus              = crmApcController.getStatus();
		viewContext.crmApcAutomaticActivation = crmApcController.getAutomaticActivationMode();

		viewContext.currentRoadmap = roadmap;

		uiStateController.roadmapStatusChanged();
	};

	coRegPlayer.eventReviewImage = [&](int /*frameIndex*/, const PciReviewPlayer<PullbackRecording>::SingleFrame& frame, bool /*last*/)
	{ 
		//image could be null to indicate a run was ended
		if(frame.first)
		{
			view.coRegViewer.setImage(frame.first);
			view.coRegViewer.setOverlay(frame.second);
			view.coRegControl.setReview(frame.first->timestamp);
		}
	};

	xrayRecordingPlayer.eventReviewImage = [&](int frameIndex, const PciReviewPlayer<XRayImageRecording>::SingleFrame& frame, bool last)
	{
		if (frameIndex == 0)
		{
			beginRun(XrayImage::Type::ExposureReplay);
		}

		setImage(frame, false);

		if (last)
		{
			endRun();
		}
	};

	coRegAdmin.eventReviewRecording = [this] (const std::shared_ptr<PullbackRecording> &recording)
	{
		viewContext.coRegRecordedRunIndex = recording->getRunIndex();

		if (recording->getMeasurementError() == Coreg::CoRegistrationError::UnreliableDataTransfer || 
			recording->getMeasurementError() == Coreg::CoRegistrationError::LatencyTooHigh ||
			unreliableCoRegConnection )
		{
			view.coRegControl.setRecording(nullptr);
		}
		else
		{
			view.coRegControl.setRecording(recording);
			if (!recording->getAllImages().empty() && recording->getAllImages().front().first)
				coRegPlayer.setReview(recording, recording->getAllImages().front().first->frameTime);
		}
	};

	uiStateController.onStbRoiChanged = [this] (Sense::Rect roi)
	{
		stbParamsPost.regionOfInterest = roi;
	};

	copyToRefClient.subscribeConnectionEvents(L"connectionIcon", [&](bool connected) { this->asyncInvoke([&, connected]() {model.setIControlConnected(connected); }); });
	model.setIControlConnected(copyToRefClient.isConnected());

	threadEncoding.executeAsync([] { SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL); });

	crmAdministration.onRoadmapStatusChanged();

	xrayConnectionTimer.start(XrayConnectionTimeout);
}

MainController::~MainController()
{
	copyToRefClient.unsubscribeConnectionEvents(L"connectionIcon");
}

void MainController::connectionChanged(bool status, IXraySourceRto::CwisStatus cwisStatus, IXraySourceRto::DvlpStatus dvlpStatus, const std::wstring &cwisHost , const  std::wstring &dvlpAdapter)
{
	if (status)
	{
		viewContext.viewXrayConnectionStatus = ConnectedSinceStartup;
		xrayConnectionTimer.stop();
	}
	else
	{
		m_log.DeveloperInfo(L"PciViewer::connectionChanged [Status=False, CwisStatus=%s, DvlpStatus=%s]",
		                    getCwisStatusString(cwisStatus), getDvlpStatusString(dvlpStatus));
	}

	logXrayConnectionStatus(viewContext.viewXrayConnectionStatus, cwisStatus, dvlpStatus, cwisHost, dvlpAdapter);

	viewContext.xrayModalityConnected = status;
	viewContext.crmApcStatus              = crmApcController.getStatus();
	viewContext.crmApcAutomaticActivation = crmApcController.getAutomaticActivationMode();
	uiStateController.connectionStatusChanged(status);
}

void MainController::endRun()
{
	if (viewContext.exposureActive)
	{
		threadStentBoost.endLive();
	}
	viewContext.exposureActive = false;

	threadGrabber.endLive();
	threadLive.endLive();
	threadBackground .endLive();
	crmAdministration.endLive();
	coRegAdmin       .endLive(acquisitionType);
	view.coRegControl.endLive();

	uiStateController.endLive();

	acquistionInProgress = false;
	StatusChanged();
}

void MainController::setGeometry(const XrayGeometry &Geometry)
{
	crmAdministration.setGeometry(Geometry);

	if (msgBuilder != nullptr)
	{
		msgBuilder->setGeometry(Geometry);
	}

	viewContext.geometryIsMoving = viewContext.geometry.changedMoving(Geometry);
	viewContext.geometry = Geometry;
	uiStateController.geometryChanged();
}

void MainController::endGeometry()
{
	                      crmAdministration.endGeometry();
	auto adminStatus    = crmAdministration.getStatus();
	const auto &roadmap = crmAdministration.getRoadmap();

	crmApcController  .setApc( adminStatus == CrmAdministration::Status::MoveArc ? roadmap : nullptr ); 
	
	viewContext.geometryIsMoving = false;
	uiStateController.geometryEnd();
}

void MainController::setEpx(const XrayEpx &Epx)
{
	PciEpx newEpx(Epx);

	if (epx.changed(newEpx))
	{
		viewContext.roadmapErrorState = CrmErrors::Error::OK;
	}

	epx = newEpx;

	viewContext.currentEpx = epx;
	uiStateController.epxChanged();
	updateCoRegStatusText();
}

void MainController::setStudy(const XrayStudy &study)
{
	if (model.getStudy().changed(study))
	{
        m_log.DeveloperInfo(L"MainController::setStudy Study info has changed. Cancel processing and clear CoReg control.");

        viewContext.fluoroAcquiredWhileRecording = false;

        updateStudyForDynamicCoronaryRoadmap(study);
        updateStudyForStentBoostLive();
        uiStateController.studyChanged();

	    threadGrabber.setStudy(study);
        xrayRecordingPlayer.clearReview();

        if (msgBuilder != nullptr)
        {
            msgBuilder->setSeriesNumber(-1);
        }

        viewContext.currentPatient = study.patientId;
	}

    if (msgBuilder != nullptr)
    {
        msgBuilder->setStudy(study);
    }

	model.setStudy(study);
}

void MainController::close()
{
	viewContext.viewXrayConnectionStatus = ClosingConnectionAtShutdown;

	AppLauncher::AppLauncherClient client(AppLauncher::Applications::PciApp, false);

	if (client.WaitForConnection(1000))
	{
		client.Start(AppLauncher::Applications::FieldService);
	}
	else
	{
		m_log.DeveloperWarning(L"Failed to start FieldService: could not connect to LoaderService");
	}

	Scheduler::exit();
}

void MainController::tsmConnectionChanged(Cwis::CwisConnectionType connection)
{
	if (connection == Cwis::CwisConnectionType::Disconnected)
	{
		m_log.DeveloperInfo(L"PciViewer: disconnected from TSM");
	}
	else
	{
		m_log.DeveloperInfo(L"PciViewer: connection with TSM established");
	}
}

void MainController::setIfrPullbackCycle( const CoRegistrationIfrPullbackCycle &cycle )
{
	view.coRegControl .setIfrPullbackCycle(cycle);
	coRegAdmin        .setIfrPullbackCycle(cycle);
}

void MainController::setIfrSpotCycle( const CoRegistrationIfrSpotCycle &cycle )
{
	coRegAdmin	.setIfrSpotCycle(cycle);
}

void MainController::setCoRegState(const CoRegistrationServerState &coRegState)
{
	if (coRegState.systemState != viewContext.coRegState.systemState)
	{
		handleSystemStateChanged(viewContext.coRegState.systemState, coRegState.systemState);
	}

	if (coRegState != viewContext.coRegState)
	{
		view.coRegControl.setCoRegState(coRegState);
		coRegAdmin	.setCoRegState(coRegState);
		auto prevCoRegState = viewContext.coRegState;
		viewContext.coRegState = coRegState;
		uiStateController.coRegistrationStateChanged(prevCoRegState, coRegState);
	}
}

void MainController::setCoRegError(CoRegistrationError error)
{
	m_log.DeveloperWarning(L"MainController: received CoRegistration error [Error=%s]", ToString(error).c_str());

	viewContext.coRegError = error;

	if ( error == CoRegistrationError::LatencyTooHigh ||
		 error == CoRegistrationError::UnreliableDataTransfer )
	{
		unreliableCoRegConnection = true;
	}

	view.coRegControl	.setCoRegError(error);
	coRegAdmin			.setCoRegError(error);

	updateCoRegStatusText();
}

void MainController::setSelectedCoRegRun(int runIndex)
{
	coRegAdmin.setSelectedCoRegRun(runIndex);
	uiStateController.coRegistrationRunChanged(runIndex);
}

void MainController::setPressureAndEcg(const CoRegistrationPressureAndEcg &pressAndEcg)
{
	view.coRegControl.setPressureAndEcg(pressAndEcg);
	coRegAdmin	.setPressureAndEcg(pressAndEcg);
}

void MainController::handleSystemStateChanged(CoRegistrationServerState::SystemState prevState, CoRegistrationServerState::SystemState nextState)
{
	if (nextState != CoRegistrationServerState::SystemState::Review && prevState == CoRegistrationServerState::SystemState::Review)
	{
		// clear error message when user is going to prepare for a new recording
		viewContext.coRegError = CoRegistrationError::OK;
		view.coRegControl.setCoRegError(CoRegistrationError::OK);
		unreliableCoRegConnection = false;
		updateCoRegStatusText();
	}

	if (((nextState == CoRegistrationServerState::SystemState::Pullback) ||
		(nextState == CoRegistrationServerState::SystemState::SpotMeasurement)) &&
		(prevState == CoRegistrationServerState::SystemState::Live))
	{
		// New recording started
		viewContext.fluoroAcquiredWhileRecording = false;
	}
}

void MainController::setPdPaInfo(const CoRegistrationPdPaInfo &pdPaInfo)
{
	view.coRegControl.setPdPaInfo(pdPaInfo);
	coRegAdmin	.setPdPaInfo(pdPaInfo);
}

void MainController::setSpotMeasurement(const CoRegistrationSpotMeasurement &spot)
{
	view.coRegControl.setSpotMeasurement(spot);
}

void MainController::setIfrSpotResult(float ifrSpotResult)
{
	view.coRegControl.setIfrSpotResult(ifrSpotResult);
}

void MainController::setCoRegOptions(const CoRegistrationOptions &options)
{
	view.coRegControl.setCoRegOptions(options);
}

void MainController::setApcConnectionChanged(bool connected)
{
    crmApcController.setCwisApcConnected(connected);

	if (viewContext.crmAdminStatus == CrmAdministration::Status::MoveArc)
	{
		crmAdministration.onRoadmapStatusChanged();
	}
}

void MainController::setExternalApcCwisActivity(IXrayGeometryController::ApcExternalCwisActivity activity)
{
    crmApcController.setApcExternalActivity(activity);
}

void MainController::setTsm3dApcButtonPressed()
{
    crmApcController.setTsm3dApcButtonPressed();
}

void MainController::startXRayReplay(bool loop)
{
	if (loop)
	{
		xrayRecordingPlayerParams.NrOfLoops = -1;
	}
	else
	{
		xrayRecordingPlayerParams.NrOfLoops = 1;
	}

	if (!xrayRecordingPlayer.inReview())
	{
		auto lastRecording = xrayRecorder->getLastRecording();
		xrayRecordingPlayer.setReview(lastRecording, lastRecording->getAllImages().front()->frameTime);
		xrayRecordingPlayer.startReview();
	}
}

void MainController::stopXRayReplay()
{
	if (xrayRecordingPlayer.inReview())
	{
		xrayRecordingPlayer.stopReview();
		xrayRecordingPlayer.resetReview();
		endRun();
	}
}

bool MainController::isActive()
{
	return xrayRecordingPlayer.inReview();
}

void MainController::setTsmFunctionsAvailable(bool available)
{
	asyncInvoke([&, available]() {	model.setTsmFunctionsAvailable(available); });
}

void MainController::setCoRegConnection(CoRegistrationSource::ConnectionStatus connectionStatus)
{
	bool synchronised  = connectionStatus == CoRegistrationSource::ConnectionStatus::ConnectedWithTimeSync;
	bool connected     = connectionStatus == CoRegistrationSource::ConnectionStatus::ConnectedNoTimeSync || synchronised;

	m_log.DeveloperInfo(	L"PciViewer::setCoRegConnection [Connected=%s, TimeSynchronized=%s]", connected ? L"True" : L"False", synchronised ? L"True" : L"False");

	if (!connected)
	{
		// Set CoReg state to the initial value
		setCoRegState(CoRegistrationServerState());
	}

	if (synchronised)
	{
		viewContext.coRegError = CoRegistrationError::OK;
		view.coRegControl.setCoRegError(CoRegistrationError::OK);
		unreliableCoRegConnection = false;
	}

	model				.setCoregConnected(connected);
	uiStateController	.coRegistrationConnectionChanged(connected);
	coRegAdmin			.setCoRegReady(synchronised);
	view.coRegControl	.setCoRegConnection(connected);

	updateCoRegStatusText();
}

void MainController::createScreenshotFile(const std::wstring& filename)
{
	OutputOffscreen offscreen(view.getCanvas());
	offscreen.render();

	auto& texture = offscreen.getTexture();
	ITexture::Lock lock;
	ITexture::FormatDesc fm(texture.getFormat());
	std::vector<char> pixelBuffer;

	pixelBuffer.resize(texture.getWidth() * texture.getHeight() * fm.pixelSize);
	lock = ITexture::Lock(texture.getFormat(), pixelBuffer.data(), texture.getWidth(), texture.getHeight());
	texture.get(lock);

	try
	{
		Sense::Image::save(lock, filename);
		m_log.DeveloperInfo(L"Screenshot of application is stored [File=%s]", filename.c_str());
	}
	catch(const std::exception& ex)
	{
		m_log.DeveloperError(L"Failed to store screenshot [File=%s, Exception=%s].", filename.c_str(), ex.what());
	}
}

void MainController::saveSceenshot()
{
	if (!PathIsDirectory(screenshotFolder.c_str()) &&
		!CreateDirectory(screenshotFolder.c_str(), nullptr) )
	{
		m_log.DeveloperError(L"Cannot create folder to store snapshot file.");
	}
	else
	{
		wchar_t tempFilePath[MAX_PATH] = {};
		PathAppend(tempFilePath, screenshotFolder.c_str());
		PathAppend(tempFilePath, L"PciAppScreenshot.png");

		if (!DeleteFile(tempFilePath))
		{
			auto err = GetLastError();
			m_log.DeveloperError(L"Failed to delete previous screenshot [File=%s, Exception=%u].", tempFilePath, err);
		}

		createScreenshotFile( tempFilePath );
	}
}

void MainController::onStbProcessingTimeout()
{
	stbProcessingTimeoutTimer.stop();

	if (lastFrame == lastStbProcessedFrame)
	{
		// end of run
		view.stbViewer.showUpdateDelayed = false;
	}
	else
	{
		if (!view.stbViewer.showUpdateDelayed)
		{
			m_log.DeveloperWarning(L"PciViewer: Stentboost image processing took longer than %f seconds", stbParamsLive.processingTimeout);
		}

		view.stbViewer.showUpdateDelayed = true;
	}
}

void MainController::setImage(const std::shared_ptr<XrayImage> &image, bool skip)
{
	if (image == nullptr) return;

	PciEpx pciEpx(image->epx);
	bool imageForCrmLive = image->type == XrayImage::Type::Fluoro;
	bool imageForCrmAngio = image->isFromExposureRun() && pciEpx.getType() == PciEpx::Type::Roadmap && viewContext.licenseCollection.isActive(LicenseFeature::Roadmap);
	bool imageForStbLive = image->isFromExposureRun() && pciEpx.getSubType() == PciEpx::SubType::Live && viewContext.licenseCollection.isActive(LicenseFeature::StentboostLive);
	bool imageForStbPostDeploy = image->isFromExposureRun() && pciEpx.getSubType() == PciEpx::SubType::PostDeployment && viewContext.licenseCollection.isActive(LicenseFeature::StbPostDeploy);
	bool imageForStbContrast = image->isFromExposureRun() && pciEpx.getSubType() == PciEpx::SubType::Subtract && viewContext.licenseCollection.isActive(LicenseFeature::StbSubtract);


	if (!imageForCrmLive && !imageForCrmAngio && !imageForStbLive && !imageForStbPostDeploy && !imageForStbContrast)
	{
		return;
	}

	view.smartSuiteAboutBox.hide();
	view.problemReportDialog.hide();

	if (!image->isFromRecording())
	{
		// Override runIndex with persistent value.
		image->runIndex = crmAdministration.getPersistentRunIndex();

		threadGrabber.addXray(image);

		if (image->isFromExposureRun() == true)
		{
			xrayRecorder->addImage(*image);
		}
	}

	if (imageForCrmLive)
	{
		threadLive.processRoadmap(image, skip);
	}
	else if (imageForCrmAngio)
	{
		threadBackground.processRoadmap(image);
	}
	else if ( imageForStbLive )
	{
		_InterlockedIncrement(&lastFrame);

		threadLive.processStentBoost(image, skip);
	}
	else if (imageForStbPostDeploy || imageForStbContrast)
	{
		threadStentBoost.processStentBoost(image);
	}
}

void MainController::beginRun(XrayImage::Type type)
{
	if ((type != XrayImage::Type::ExposureReplay) && xrayRecordingPlayer.inReview())
	{
		xrayRecordingPlayer.stopReview();
		endRun();
	}

	if (type == XrayImage::Type::Exposure && epx.getType() == PciEpx::Type::StentBoost)
	{
		stbParamsPost.regionOfInterest = Sense::Rect();
		
		threadLive.setEpxSubType(epx.getSubType());
		threadStentBoost.setEpxSubType(epx.getSubType());
	}

	if ((type == XrayImage::Type::Exposure) || (type == XrayImage::Type::ExposureReplay))
	{
		viewContext.exposureActive = true;
	}

	threadLive.init();
	threadStentBoost.init();
	uiStateController.imageRunStarted(type);
    acquisitionType = type;

	acquistionInProgress = true;
	StatusChanged();
}

void MainController::onExportBusyUpdate()
{
	auto items = threadEncoding.queueSize() + pacsPush->queueSize();
	view.exportBusyDialog.labelRemaining.text = Localization::Translation::IDS_ExportRemaining + L" " + std::to_wstring(items);

	if (items == 0)
	{
		exportBusyTimer.stop();
		close();
	}
}

void MainController::onFieldService()
{
	m_log.DeveloperInfo(L"User clicked on Field Service");

	if (( pacsPush == nullptr ) || (!threadEncoding.isBusy() && (!pacsPush->isBusy())))
	{
		close();
	}
	else
	{
		m_log.DeveloperInfo(L"ExportBusy: Field Service requested while DICOM export is in progress");

		view.exportBusyDialog.close.eventClicked = [this]
		{
			view.exportBusyDialog.hide();
			exportBusyTimer.stop();
		};

		view.exportBusyDialog.buttonAbort.eventClicked = [this]
		{
			m_log.DeveloperWarning(L"ExportBusy: Active DICOM transfers canceled by user");
			// clear work queues and abort currently running tasks
			threadEncoding.clear();
			threadEncoding.flush();
			pacsPush->abort();
			exportBusyTimer.stop();
			close();
		};

		view.exportBusyDialog.modal = true;
		onExportBusyUpdate();
		exportBusyTimer.start(0.25);
		view.exportBusyDialog.show();
	}
}

void MainController::asyncInvoke(const std::function<void()> &command) const
{
	scheduler.addAsync(*this, command);
}

void MainController::onGenerateProblemReport()
{
	m_log.DeveloperInfo(L"User clicked on Generate problem report.");

	model.setSystemMenuEnabled(false);

	saveSceenshot();

	if (!appLauncherClient)
	{
		appLauncherClient.reset(new AppLauncher::AppLauncherClient(AppLauncher::Applications::PciApp, true));

		appLauncherClient->EventAppExited = [this] (AppLauncher::Applications app, int /* exitCode */)
		{
			if (app == AppLauncher::Applications::ProblemReportHidden)
			{
				scheduler.addAsync(*this, [this]
				{
					m_log.DeveloperInfo(L"Problem report generated. Showing message to user.");
					view.problemReportDialog.show();
					model.setSystemMenuEnabled(true);
				});
			}
		};
	}

	if (appLauncherClient->WaitForConnection(1000))
	{
		appLauncherClient->Start(AppLauncher::Applications::ProblemReportHidden);
	}
	else
	{
		m_log.DeveloperWarning(L"Failed to generate problem report: could not connect to LoaderService");
	}
}

void MainController::onShowAboutBoxRoadmap()
{
	m_log.DeveloperInfo(L"User clicked on About box of Dynamic Coronary Roadmap.");

	view.smartSuiteAboutBox.setContents(ProductInfo::AboutDcrCoreg);
	view.smartSuiteAboutBox.bringToFront();
	view.smartSuiteAboutBox.show();
}

void MainController::onShowAboutBoxStentBoost()
{
	m_log.DeveloperInfo(L"User clicked on About box of StentBoost Live.");

	view.smartSuiteAboutBox.setContents(ProductInfo::AboutSbl);
	view.smartSuiteAboutBox.bringToFront();
	view.smartSuiteAboutBox.show();
}

void MainController::startupXrayConnectionTimeout()
{
	if (xrayConnectionTimer.isActive()) 
	{
		xrayConnectionTimer.stop();
	}

	m_log.DeveloperInfo(L"Timeout to connected after startup elapsed. Notify GUI that connection has not been established.");
	viewContext.viewXrayConnectionStatus = TimeOut;
	connectionChanged(rto.isConnected(), rto.getCwisConnectionStatus(), rto.getDvlpConnectionStatus(), L"", L"");
}

void MainController::onSnapshotStatusTimer()
{
	setSnapshotStatus(ArchivingStatus::None, CaptureArea::MainView);
}

void MainController::onMovieStatusTimer()
{
	setMovieStatus(ArchivingStatus::None);
}

static void updateStatusTimer(ArchivingStatus status, Sense::Timer& timer)
{
	switch (status)
	{
	case ArchivingStatus::None:
	case ArchivingStatus::Ready:
		timer.stop();
		break;
	case ArchivingStatus::Busy:
		timer.start(ArchivingBusyTimeout);
		break;
	case ArchivingStatus::Success:
	case ArchivingStatus::Failure:
		timer.start(ArchivingCompleteTimeout);
		break;
	}
}

void MainController::setSnapshotStatus(ArchivingStatus status, CaptureArea area)
{
	model.setSnapshotStatus(status, area);
	updateStatusTimer(status, snapshotStatusTimer);
}

void MainController::setMovieStatus(ArchivingStatus status)
{
	model.setMovieStatus(status);
	updateStatusTimer(status, movieStatusTimer);
}

void MainController::clearStatus()
{
	model.setSnapshotStatus(ArchivingStatus::None, CaptureArea::MainView);
	model.setMovieStatus(ArchivingStatus::None);
}

void MainController::logXrayConnectionStatus(ViewXrayConnectionStatus viewXrayConnectionStatus, IXraySourceRto::CwisStatus cwisStatus, IXraySourceRto::DvlpStatus dvlpStatus,
                                             const std::wstring &cwisHost, const std::wstring &dvlpAdapter)
{
	static bool cwisConnected = false;
	static bool dvlpConnected = false;

	switch ( viewXrayConnectionStatus )
	{
	case NeverConnected:
	case ConnectedSinceStartup:
		// Log all connection changes
		if (cwisBecomesConnected(cwisStatus, cwisConnected))
		{
			::CommonPlatform::UtilizationLogEvent::ConnectedHospitalNetworkToXray(m_log, L"Hospital network", cwisHost);
		}
		
		if (dvlpBecomesConnected(dvlpStatus, dvlpConnected))
		{
			::CommonPlatform::UtilizationLogEvent::ConnectedRealTimeLinkToXray(m_log, L"RealTimeLink", dvlpAdapter);
		}

		if (cwisBecomesDisconnected(cwisStatus, cwisConnected))
		{
			::CommonPlatform::UtilizationLogEvent::ConnectionLostToHospitalNetworkXray(m_log);
		}
		
		if (dvlpBecomesDisconnected(dvlpStatus, dvlpConnected))
		{
			::CommonPlatform::UtilizationLogEvent::ConnectionLostToRealTimeLinkXray(m_log);
		}
		break;

	case TimeOut:
		// Log that links are not connected
		::CommonPlatform::UtilizationLogEvent::ConnectionLostToHospitalNetworkXray  (m_log);
		::CommonPlatform::UtilizationLogEvent::ConnectionLostToRealTimeLinkXray     (m_log);
		break;

	case ClosingConnectionAtShutdown:
		// do not log any connection changes
		break;
	}

	cwisConnected = cwisStatus == IXraySourceRto::CwisStatus::Connected;
	dvlpConnected = dvlpStatus == IXraySourceRto::DvlpStatus::Connected;
}

void MainController::updateCoRegStatusText()
{
	if ((epx.getType() == PciEpx::Type::Roadmap) && (viewContext.licenseCollection.isExpired(LicenseFeature::Coregistration)))
	{
		model.setCoregStatusText(Localization::Translation::IDS_CoregLicenseExpired);
	}
	else if ( model.getCoregConnected() && 
		      ( (viewContext.coRegError == CoRegistrationError::UnreliableDataTransfer) || 
		        (viewContext.coRegError == CoRegistrationError::LatencyTooHigh) || 
			    unreliableCoRegConnection ))
	{
		model.setCoregStatusText(Localization::Translation::IDS_UnreliableIFRConnection + L". " + Localization::Translation::IDS_ContactServiceProvider);
	}
	else 
	{
		model.setCoregStatusText(L"");
	}
}

CrmAdministration& MainController::getCrmAdministration()
{
	return crmAdministration;
}

PciViewContext& MainController::getPciViewContext()
{
	return viewContext;
}

PciEpx& MainController::getEpx()
{
	return epx;
}

UiStateController& MainController::getUiStateController()
{
	return uiStateController;
}

void MainController::waitForEncoder()
{
	threadEncoding.flush();
}

bool MainController::IsInProgress()
{
	return acquistionInProgress;
}

wchar_t* getCwisStatusString(IXraySourceRto::CwisStatus cwisStatus)
{
	switch(cwisStatus)
	{
	case IXraySourceRto::CwisStatus::Connected                              : return L"Connected";
	case IXraySourceRto::CwisStatus::NotConnectedSinceNoActiveConnections   : return L"Not Connected Since No Active Connections";
	case IXraySourceRto::CwisStatus::NotConnectedSinceNoCwisServer          : return L"Not Connected Since No CWIS Server";
	case IXraySourceRto::CwisStatus::PartiallyConnected                     : return L"PartiallyConnected";
	default                                                                 : return L"Unknown CWIS connection status";
	}
}

wchar_t* getDvlpStatusString(IXraySourceRto::DvlpStatus dvlpStatus) 
{
	switch(dvlpStatus)
	{
	case IXraySourceRto::DvlpStatus::Connected                              : return L"Connected";
	case IXraySourceRto::DvlpStatus::NotConnected                           : return L"Not Connected";
	case IXraySourceRto::DvlpStatus::NotConnectedSinceNoAdapterDefined      : return L"Not Connected Since No Adapter Defined";
	case IXraySourceRto::DvlpStatus::NotConnectedSinceNoCable               : return L"Not Connected Since No Cable";
	case IXraySourceRto::DvlpStatus::NotConnectedSinceOpeningSocketFailed   : return L"Not Connected Since Opening Socket Failed";
	default                                                                 : return L"Unknown DVLP connection status";
	}
}

bool MainController::cwisBecomesConnected(IXraySourceRto::CwisStatus cwisStatus, bool cwisConnected) const
{
	return  (cwisStatus == IXraySourceRto::CwisStatus::Connected) && !cwisConnected;
}

bool MainController::dvlpBecomesConnected(IXraySourceRto::DvlpStatus dvlpStatus, bool dvlpConnected) const
{
	return (dvlpStatus == IXraySourceRto::DvlpStatus::Connected) && !dvlpConnected;
}

bool MainController::cwisBecomesDisconnected(IXraySourceRto::CwisStatus cwisStatus, bool cwisConnected) const
{
	return  (cwisStatus != IXraySourceRto::CwisStatus::Connected) && cwisConnected;
}

bool MainController::dvlpBecomesDisconnected(IXraySourceRto::DvlpStatus dvlpStatus, bool dvlpConnected) const
{
	return (dvlpStatus != IXraySourceRto::DvlpStatus::Connected) && dvlpConnected;
}

void MainController::updateStudyForDynamicCoronaryRoadmap(const XrayStudy& study)
{
    crmAdministration.setStudy(study);
    crmApcController.setStudy(study);
    coRegAdmin.setStudy(study);
    viewContext.roadmapErrorState = CrmErrors::Error::OK;
    threadBackground.cancel();
    view.coRegControl.clearGraphs();
}

void MainController::updateStudyForStentBoostLive()
{
    stbAdmin.studyChanged();
}


}}