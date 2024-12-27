// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#define NOMINMAX
#include <Sense.h>
#include <ExperienceIdentity.h>
#include <net.h>
#include "SccfLogDefs.h"
#include "XraySourceRto.h"
#include "XrayGeometryController.h"
#include "MainModel.h"
#include "MainViewer.h"
#include "MainController.h"
#include "PACSPush.h"
#include "stlstring.h"
#include "MessageBuilder.h"
#include "UtilizationLogEvent.h"
#include "DicomConstants.h"
#include "NodeSettings.h"
#include "ConnectionSettings.h"
#include "StorageCommitListener.h"
#include "library.h"
#include "exception.h"
#include "PathUtil.h"
#include "AppLauncherClient.h"
#include "TsmSource.h"
#include "UserMouseKeyboardMonitor.h"
#include "CoRegistrationSource.h"
#include "CoRegistrationSystemInfo.h"
#include "AuditTrailClient.h"
#include "Environment.h"
#include "Network.h"
#include "LicenseStatus.h"
#include "LicenseManager.h"
#include "LicensesCollection.h"
#include "ViewType.h"
#include "SystemType.h"
#include "ViewTypeDetector.h"
#include "PciGuidanceModel.h"
#include "PciGuidanceView.h"
#include "Translation.h"
#include "TranslationHelper.h"
#include "WindowKeyHandler.h"
#include "NullCopyToRefClient.h"
#include "CopyToRefClient.h"
#include "ArchivingSettings.h"
#include "UserPrefs.h"
#include "CharSetManagerSystemLanguage.h"
#include "NetworkIP.h"

#include <windows.h>
#include <dxgi.h>

//#define OVERRIDE_LICENSE_WITH_DEBUG_SETTINGS
//#define INCLUDE_MOVIE_AND_SNAPSHOT_TO_DISK

#ifdef INCLUDE_TEST_AND_DEMO_FUNCTIONALITY
// define this definition in propertysheet.
#pragma comment(lib,"PciExpressTestInterface.lib")
#include <sscfts1.h>
#include "PciExpressTestServer.h"
#include "MainControllerTestServer.h"
#endif

#include <Log.h>
#include <net/ipaddress.h>

namespace Pci {  namespace PciApp{

using namespace PciSuite;
using namespace Core;
using namespace Sense;
using namespace Sense::ExperienceIdentity;
using namespace CommonPlatform::CoRegistration;
using namespace AuditTrail;
using namespace CommonPlatform;
using namespace CommonPlatform::Xray;
namespace Merge = ::CommonPlatform::Dicom::Merge;
using CommonPlatform::AppLauncher::Applications;

static const std::wstring configFolder = L"..\\Configuration\\";
static const int MaxAuditTrailConnectRetryCount = 10;

bool setFullscreen(Sense::Screen& screen, bool fullscreen);
void startupNoConfig();
void connectToAuditTrailService(AuditTrailClient &auditTrailClient);
void LogMissingLicences(Log &log, const ILicenseCollection &licenses);
void LogFrrIfrConnected(const std::wstring &protocolVersion , const std::wstring &coRegAddress);
void setNoConfigErrorMessage(PciGuidanceModel &guidanceModel);
void subscribeToCoRegEvents(CoRegistrationSource &coRegSource, std::unique_ptr<MainController> &controller, std::wstring coRegAddress);
void overrideViewTypeForDebugging(CommonPlatform::Settings &ini, ViewType &viewType, SystemType &systemType);

void sscfTraceFunc(const char* fileName, int lineNumber, const char* message) noexcept
{
	TraceMessageEx(fileName, lineNumber) << "SSCF:" << message;
}

void CreateDicomStorage(
	IScheduler &scheduler, 		
	std::wstring configFolderFullPath,
	CommonPlatform::Settings &workstation_ini,
	PACS::StorageNodeSettings &storageNodeSettings,
	bool auditTrailActive,
	std::unique_ptr<PACS::IPACSPush> &pacsPush, 
	std::unique_ptr<PACS::MessageBuilder> &messageBuilder, 
	std::unique_ptr<PACS::IStorageCommitListener> &storCommit,
	std::unique_ptr<AuditTrailClient>& auditTrailClient)
{
	PACS::CommitNodeSettings	commitNodeSettings(workstation_ini);
	PACS::ConnectionSettings	connectionSettings(workstation_ini);

	bool storeDCM = false;
	std::wstring storeDCMFolder = L"";
#ifdef INCLUDE_MOVIE_AND_SNAPSHOT_TO_DISK
	// debug settings, causes the DCM file and/or the source snapshot file to be stored on disk
	storeDCM = ini.get(L"debug", L"storeDCM", false);
	sstoreDCMFolder = ini.get(L"debug", L"storeDCMFolder", L"");
#endif

	const auto NO_TIMEOUT(0);
	
	NodeAuditTrailInfo localNodeInfo;
	localNodeInfo.AETitle = storageNodeSettings.LocalAeTitle;
	localNodeInfo.HostName = CommonPlatform::Utilities::Network::GetHostName();
	localNodeInfo.IPAddress = str_ext::stow(CommonPlatform::Utilities::Network::ResolveNetwork("Hospital Network", NO_TIMEOUT).toString());

	NodeAuditTrailInfo remoteNodeInfo;
	remoteNodeInfo.AETitle = storageNodeSettings.RemoteAeTitle;
	remoteNodeInfo.HostName = storageNodeSettings.RemoteHost;

	auditTrailClient.reset(new AuditTrailClient(CommonPlatform::Environment::GetLoggedInUserName(), L"Coronary Tools", localNodeInfo, remoteNodeInfo));

	if (auditTrailActive)
	{
		connectToAuditTrailService(*auditTrailClient);
	}

	std::unique_ptr<CommonPlatform::Dicom::CharsetManagerSystemLanguage> charsetManager(new CommonPlatform::Dicom::CharsetManagerSystemLanguage(workstation_ini.get(L"General", L"UiLanguage", L"en")));

	messageBuilder.reset(new PACS::MessageBuilder(
		Merge::Service(Merge::sopClassUidToserviceName(CommonPlatform::Dicom::Constants::SopClass::SecondaryCaptureImageStorage),
			Merge::SyntaxList({ storageNodeSettings.SnapshotTransferSyntax }), Merge::Service::Role::SCU),
		Merge::Service(Merge::sopClassUidToserviceName(CommonPlatform::Dicom::Constants::SopClass::MultiFrameSecondaryCaptureTrueColor),
			Merge::SyntaxList({ storageNodeSettings.MovieTransferSyntax }), Merge::Service::Role::SCU),
		auditTrailClient.get(),
		std::move(charsetManager)
	));

	if (storageNodeSettings.SupportMovies || storageNodeSettings.SupportSnapshots)
	{
		pacsPush.reset(new PACS::PACSPush(scheduler, storageNodeSettings, localNodeInfo.IPAddress, commitNodeSettings, connectionSettings,
			*messageBuilder, *auditTrailClient, storeDCM, storeDCMFolder));
	}

	storCommit.reset(new PACS::StorageCommitListener(scheduler, commitNodeSettings, connectionSettings));
}

int CALLBACK WinMain()
{

	#ifdef _DEBUG
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif

	auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::Platform);

	log.DeveloperInfo(L"Starting PCI App");

	auto configFolderFullPath = Utilities::AbsolutePathFromExeDir(configFolder);
	auto fileAttributes = GetFileAttributes(configFolderFullPath.c_str());
	if (fileAttributes == INVALID_FILE_ATTRIBUTES)
	{
		// no config directory means we cannot do anything
		log.DeveloperError(L"The configuration directory does not exist. Unable to normally start the application");
		startupNoConfig();

		return 0;
	}
	else if ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		// the config directory is a file, this is a possible disk corruption
		log.DeveloperError(L"The configuration directory is corrupt. Unable to normally start the application");
		startupNoConfig();

		return 0;
	}
	
	LicenseManager licenseManager(str_ext::wtos(configFolderFullPath) + "license.dat");
	auto& licensesCollection = licenseManager.getLicenses();
	
	// Delete persistent data if necessary, to avoid getting into a loop when trying to restart
    // The file must be deleted before the CrmAdministration is started and attempts to read it.
	if (__argc > 1 && _stricmp(__argv[1], "reset") == 0)
	{
		CrmParameters crmParms(L"crm.ini");
		CrmRepository::deletePersistentData(log, crmParms.storeRoadmapFolder);
	}

	std::unique_ptr<Merge::Library> mergeLib;

	try
	{
		mergeLib.reset(new Merge::Library(str_ext::wtos(configFolderFullPath + L"Merge.ini")));
	}
	catch (Merge::Exception& e)
	{
		log.DeveloperError(L"Mergecom init failure: %S\nSnapshots and movies will not be available", e.what());
	}

	if (GetFileAttributes((configFolderFullPath + L"settings.ini").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		log.DeveloperWarning(L"settings.ini does not exist, default settings will be used");
	}

    // Application settings
    CommonPlatform::Settings ini(configFolderFullPath + L"settings.ini");

	int          adapter		= ini.get(L"general", L"gpu",		0);
	int          monitor		= ini.get(L"general", L"monitor",	0);
	bool         fullscreen		= ini.get(L"general", L"fullscreen",true);
	std::wstring key			= ini.get(L"general", L"license",	L"");
	std::wstring minidump		= ini.get(L"general", L"minidump",	L"");
	std::wstring grabber		= ini.get(L"general", L"grabber",	L"");
    std::wstring screenshotDir  = ini.get(L"general", L"screenshot",L"");

	const int MAX_TRACE_DIR_SIZE(1024 * 1024 * 100);
	Trace::TraceWriterFile traceWriter("D:\\Tracing\\SSCF\\", Trace::TraceWriterFile::defaultMaxFileSize, MAX_TRACE_DIR_SIZE);		
	SscfTs1::system().registerTraceMsgHandler(&sscfTraceFunc);

#ifdef OVERRIDE_LICENSE_WITH_DEBUG_SETTINGS
	// when there is no license file, use the debug settings
	if (GetFileAttributesA(licenseFilePath.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		std::wstring crmLic		= ini.get(L"debug", L"crmLicense", L"Active");
		std::wstring stbLic		= ini.get(L"debug", L"stbLicense", L"Active");
		std::wstring coregLic	= ini.get(L"debug", L"coregLicense", L"Active");

		crmLicense   = crmLic   == L"Active" ? LicenseStatus::Active : crmLic   == L"Expired" ? LicenseStatus::Expired : LicenseStatus::None;
		stbLiveLicense   = stbLic   == L"Active" ? LicenseStatus::Active : stbLic   == L"Expired" ? LicenseStatus::Expired : LicenseStatus::None;
		coregLicense = coregLic == L"Active" ? LicenseStatus::Active : coregLic == L"Expired" ? LicenseStatus::Expired : LicenseStatus::None;
		coregLicense = (crmLicense == LicenseStatus::Active ? coregLicense : Pci::Core::LicenseStatus::None);
	}
#endif
	
	if (GetFileAttributes((configFolderFullPath + L"Workstation.ini").c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		log.DeveloperWarning(L"Workstation.ini does not exist, default settings will be used");
	}

	LogMissingLicences(log, licensesCollection);

	
	// Workstation connection settings
    CommonPlatform::Settings workstation_ini(configFolderFullPath + L"Workstation.ini");

	std::unique_ptr<Platform::iControl::ICopyToRefClient> copyToRefClient(new Platform::iControl::NullCopyToRefClient);
	std::unique_ptr<Pci::Core::IUserPrefs> userPrefs(new Pci::Core::UserPrefs(workstation_ini));


	std::wstring systemTypeSetting  = workstation_ini.get(L"General", L"SystemType", L"Allura");
	std::wstring language = workstation_ini.get(L"General", L"UiLanguage", L"en");

	auto		 cwisHost		    = workstation_ini.get(L"Xray", L"ip",			                L"127.0.0.1");
	int          cwisPort		    = workstation_ini.get(L"Xray", L"CwisPort",				        6910);
																							         
	auto		 dvlpAdapter      = workstation_ini.get(L"Xray", L"DvlpAdapter",			        L"127.0.0.1");
    std::wstring dvlpMulticast	    = workstation_ini.get(L"Xray", L"DvlpMulticastGroup",	        L"224.0.0.99");
	int          dvlpPort		    = workstation_ini.get(L"Xray", L"DvlpPort",				        29121);
																							         
	auto		 coRegAddress		= workstation_ini.get(L"CoReg", L"IP",					        L"192.68.32.242");
    int          coRegPort			= workstation_ini.get(L"CoReg", L"Port",				        1919);
																							         
	bool         auditTrailActive   = workstation_ini.get(L"AuditTrail", L"Enabled",		        false);
	
	PACS::StorageNodeSettings	storageNodeSettings(workstation_ini);

    if (!minidump.empty())
	{
		system().setMinidump(minidump.c_str());
	}

    std::wstringstream configLog;
    configLog << L"Configuration read from ini files: " << std::endl;
    configLog << L"GPU adapter:"                    << adapter							<< std::endl;
    configLog << L"Monitor:"                        << monitor							<< std::endl;
    configLog << L"Fullscreen:"                     << fullscreen						<< std::endl;
    configLog << L"Cwis host:"                      << cwisHost							<< std::endl;
    configLog << L"Cwis port:"                      << cwisPort							<< std::endl;
    configLog << L"Dvlp adapter:"                   << dvlpAdapter						<< std::endl;
    configLog << L"Dvlp multicast:"                 << dvlpMulticast					<< std::endl;
    configLog << L"Dvlp port:"                      << dvlpPort							<< std::endl;
    configLog << L"3D-APC automatic acvitation:"    << userPrefs->getApcAutoActivation()<< std::endl;
    configLog << L"Screenshot folder:"              << screenshotDir					<< std::endl;
    log.DeveloperInfo( configLog.str().c_str() );

	Localization::Translation::setLanguage(Localization::getLanguageId(language, licensesCollection.isActive(LicenseFeature::LanguagePack)));

	Sense::Logging::eventError = [&log](const wchar_t *message)
	{
		log.DeveloperError(L"Sense error: %s", message);
		OutputDebugString(message);
		OutputDebugString(L"\n");
	};

	Scheduler			    scheduler;
    Gpu					    gpu		(adapter);
    Canvas                  canvas	(gpu, scheduler);

    CommonPlatform::TSM::TsmSource                       tsm(scheduler, L"PciApp TSM");

	WindowKeyHandler keyHandler(fullscreen);
	
	OutputScreen             screen  (canvas, monitor);
	UserMouseKeyboardMonitor user   (canvas, !fullscreen);

	auto systemType = GetSystemType(systemTypeSetting);
	auto viewType = GetViewType(systemTypeSetting, screen.getSize());

	overrideViewTypeForDebugging(ini, viewType, systemType);

	NetworkIP netIP(scheduler);
	if (systemType == SystemType::SmartSuite)
	{
		copyToRefClient.reset(new Platform::iControl::CopyToRefClient());
		std::string hospitalNetwork = "Hospital Network";
		netIP.eventIPObtained = [&copyToRefClient](const std::string& ip) { copyToRefClient->connect(ip); };
		netIP.getIPAdreess(hospitalNetwork);
	}

	std::unique_ptr<PACS::MessageBuilder>			messageBuilder;
	std::unique_ptr<PACS::IPACSPush>				pacsPush;
	std::unique_ptr<PACS::IStorageCommitListener>	storCommit;
	std::unique_ptr<AuditTrailClient>				auditTrailClient;

	std::unique_ptr<MainModel>      mainModel(new MainModel(licensesCollection, systemType));
	std::unique_ptr<MainViewer>     mainView(new MainViewer(canvas, *mainModel, viewType));
    std::unique_ptr<MainController> controller;

    XraySourceRto		    rto		(scheduler, systemType);
    XrayGeometryController  xrayGeometryController (scheduler);
	CoRegistrationSource    coRegSource(scheduler);

	if (mergeLib != nullptr)
	{
		CreateDicomStorage(scheduler, configFolderFullPath, workstation_ini, storageNodeSettings, auditTrailActive, pacsPush, messageBuilder, storCommit, auditTrailClient);
	}


#ifdef INCLUDE_TEST_AND_DEMO_FUNCTIONALITY
	// define this definition in propertysheet.
	// Let SSCF use the windows event loop. SSCF will use the windows message queue and messages will be handles from there
    // by the Sense scheduler.
    SscfTs1::WindowsEventLoop eventLoop;

    std::wstring tafIp          = workstation_ini.get(L"Taf", L"Ip",   L"127.0.0.1");
    uint16_t tafPort  = (uint16_t)workstation_ini.get(L"Taf", L"Port", 62003);
	bool forceStateControlUtil  = ini.get(L"debug", L"showStateControlUtility", false);

	SscfTs1::ServerNode server(eventLoop, tafPort, str_ext::wtos(tafIp));
    PciExpressTestServer testServer;
    server.registerSingleton<IPciExpressTestSscfInterface>(&testServer);

    if (server.listen())
    {
        log.DeveloperInfo(L"TAF Test Server is running.");
    }
    else
    {
        log.DeveloperError(L"TAF Test Server could not be started.");
    }

	auto argvEnd = __argv + __argc;
	bool enableStateControlUtil = forceStateControlUtil || (std::find(__argv, argvEnd, std::string("demo")) != argvEnd);

	// Why not let testServer be a member of PciViewerTestServer?
	controller.reset( new MainControllerTestServer(*mainView,
	                                               *mainModel,
	                                               rto,
	                                               xrayGeometryController,
	                                               grabber,
	                                               screenshotDir,
	                                               scheduler,
	                                               gpu,
	                                               tsm,
	                                               storCommit.get(),
	                                               pacsPush.get(),
	                                               storageNodeSettings.SupportSnapshots,
	                                               storageNodeSettings.SupportMovies,
	                                               testServer,
	                                               enableStateControlUtil,
	                                               apcAutoActivation,
	                                               crmLicense,
	                                               stbLiveLicense,
	                                               coregLicense));

#else

	controller.reset(new MainController(*mainView, *mainModel, rto, xrayGeometryController, grabber,
	                                    screenshotDir,scheduler, tsm, storCommit.get(), pacsPush.get(), messageBuilder.get(),
	                                    storageNodeSettings.SupportSnapshots, storageNodeSettings.SupportMovies,
	                                    systemType, *copyToRefClient, *userPrefs));

#endif

	canvas.size  = mainView->size;

	screen.text  = L"Coronary Tools";
	screen.image = Icons::HeartSegmentation;

	mainView->visible = true;

	rto.eventImage			    = [&controller]	(const std::shared_ptr<XrayImage> &image, bool skip)	{ controller->setImage(image, skip);	};
	rto.eventGeometry		    = [&controller]	(const XrayGeometry &geometry)							{ controller->setGeometry(geometry);	};
	rto.eventEpx			    = [&controller]	(const XrayEpx      &epx)								{ controller->setEpx(epx);				};
	rto.eventStudy			    = [&controller]	(const XrayStudy    &study)								{ controller->setStudy(study);			};
	rto.eventImageBegin		    = [&controller]	(XrayImage::Type type)									{ controller->beginRun(type);			};
	rto.eventImageEnd		    = [&controller]	()														{ controller->endRun();					};
	rto.eventGeometryEnd	    = [&controller]	()														{ controller->endGeometry();			};
	rto.eventConnectionChanged  = [&controller,&log,&cwisHost, &dvlpAdapter]	(bool status, IXraySourceRto::CwisStatus cwis, IXraySourceRto::DvlpStatus dvlp) 
                                                                                                        { controller->connectionChanged(status, cwis, dvlp, cwisHost, dvlpAdapter); };
	rto.eventCwisError			= [&log] (const wchar_t* modelName, const wchar_t* errorDescr)          { log.DeveloperError(L"CWIS error [Model=%s, Error=%s]", modelName, errorDescr); } ;

	tsm.eventConnectionChanged  = [&controller]	(Cwis::CwisConnectionType connection)                   { controller->tsmConnectionChanged(connection); };
	tsm.eventFunctionAvailabilityChanged = [&controller](bool available)							    { controller->setTsmFunctionsAvailable(available); };

	subscribeToCoRegEvents(coRegSource, controller, coRegAddress);


	xrayGeometryController.eventConnectionChanged               = [&controller] (bool connected)                                              { controller->setApcConnectionChanged(connected);           };
	xrayGeometryController.eventApcExternalCwisActivityDetected = [&controller] (XrayGeometryController::ApcExternalCwisActivity apcConflict) { controller->setExternalApcCwisActivity(apcConflict);   };
	xrayGeometryController.eventTsm3dApcButtonPressed           = [&controller] ()                                                            { controller->setTsm3dApcButtonPressed();                };

	if (licensesCollection.isActive(LicenseFeature::Coregistration)) 
	{
		coRegSource.connect(coRegAddress, coRegPort);
	}
	
	tsm                     .connect(cwisHost.c_str(), static_cast<uint16_t>(cwisPort));
	rto                     .connect(dvlpAdapter, dvlpMulticast, dvlpPort, cwisHost, cwisPort);
	xrayGeometryController  .connect(cwisHost,cwisPort);

	gpu.show();
	if (!setFullscreen(screen, fullscreen))
	{
		CommonPlatform::Logger(CommonPlatform::LoggerType::Platform).DeveloperWarning(L"setFullscreen failed, app will run in non-exclusive mode");
	}
	auto ret = scheduler.run();

    return ret;
}

bool setFullscreen(Sense::Screen& screen, bool fullscreen)
{
	HWND foregroundHwnd = GetForegroundWindow();
	DWORD currThreadId = GetCurrentThreadId();
	DWORD windowThreadId = GetWindowThreadProcessId(foregroundHwnd, NULL);

	if (currThreadId != windowThreadId)
	{
		AttachThreadInput(currThreadId, windowThreadId, TRUE);
	}

	SetForegroundWindow(screen.getHandle());

	if (currThreadId != windowThreadId)
	{
		AttachThreadInput(currThreadId, windowThreadId, FALSE);
	}

	screen.setFullscreen(fullscreen);
	BOOL curFullScreen = FALSE;
	if (SUCCEEDED(screen.getSwapChain()->GetFullscreenState(&curFullScreen, nullptr)))
	{
		return fullscreen == (curFullScreen == TRUE);
	}
	return false;
}

void LogFrrIfrConnected(const std::wstring &protocolVersion , const std::wstring &coRegAddress )
{
	auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::CoronaryRoadmap);
	UtilizationLogEvent::ConnectedToFfrIfrSystem(log, L"FFR/iFR Network",coRegAddress,protocolVersion);
}

void startupNoConfig()
{
	// start a minimal application with a locked guidance screen
	// no inputs or outputs are set since they cannot be configured
	Scheduler			scheduler;
	Gpu					gpu		(0);
	Canvas				canvas	(gpu, scheduler);
	OutputScreen        screen  (canvas, 0);
	UserMouseKeyboard	user	(canvas);

	MainAlluraView   pci(canvas);
	PciGuidanceView  crmGuidance(pci, ViewType::Allura);
	PciGuidanceModel guidanceModel;
	crmGuidance.setGuidanceModel(guidanceModel);	
	crmGuidance.setBounds(pci.main.getBounds());

    pci.systemMenu.eventClicked = []()
	{
		AppLauncher::AppLauncherClient client(AppLauncher::Applications::PciApp, false);
		if (client.WaitForConnection(1000))
		{
			client.Start(AppLauncher::Applications::FieldService);
		}
		else
		{
			auto& log = CommonPlatform::Logger(CommonPlatform::LoggerType::CoronaryRoadmap);
			log.DeveloperWarning(L"Failed to start FieldService: could not connect to LoaderService");
		}
		Scheduler::exit();
	};

	canvas.size  = pci.size;
	screen.text  = L"Coronary Tools";
	screen.image = Icons::HeartSegmentation;

	setNoConfigErrorMessage(guidanceModel);
	
	pci.visible = true;

	gpu.show();
	scheduler.run();
}

void subscribeToCoRegEvents(CoRegistrationSource &coRegSource, std::unique_ptr<MainController> &controller, std::wstring coRegAddress)
{
	coRegSource.eventIfrPullbackCycle = [&controller](const CoRegistrationIfrPullbackCycle &coRegCycle) { controller->setIfrPullbackCycle(coRegCycle);  };
	coRegSource.eventIfrSpotCycle = [&controller](const CoRegistrationIfrSpotCycle &coRegCycle) { controller->setIfrSpotCycle(coRegCycle);		 };
	coRegSource.eventIfrSpotResult = [&controller](const float ifrSpotResult) { controller->setIfrSpotResult(ifrSpotResult);	 };
	coRegSource.eventPressureAndEcgData = [&controller](const CoRegistrationPressureAndEcg &pressAndEcg) { controller->setPressureAndEcg(pressAndEcg);   };
	coRegSource.eventStateData = [&controller](const CoRegistrationServerState &coRegState) { controller->setCoRegState(coRegState);		 };
	coRegSource.eventPdPaInfo = [&controller](const CoRegistrationPdPaInfo &pdPaInfo) { controller->setPdPaInfo(pdPaInfo);			 };
	coRegSource.eventSpotMeasurement = [&controller](const CoRegistrationSpotMeasurement &spot) { controller->setSpotMeasurement(spot);		 };
	coRegSource.eventError = [&controller](const CoRegistrationError error) { controller->setCoRegError(error);			 };
	coRegSource.eventOptions = [&controller](const CoRegistrationOptions &options) { controller->setCoRegOptions(options);		 };
	coRegSource.eventRunSelected = [&controller](int runIndex) { controller->setSelectedCoRegRun(runIndex);		 };
	coRegSource.eventConnectionChanged = [&controller](const CoRegistrationSource::ConnectionStatus status) { controller->setCoRegConnection(status);	     };
	coRegSource.eventSysInfo = [coRegAddress](const CoRegistrationSystemInfo &coRegSystemInfo) { LogFrrIfrConnected(coRegSystemInfo.protocolVersion, coRegAddress); };
}



void setNoConfigErrorMessage(PciGuidanceModel &guidanceModel)
{
	guidanceModel.showMessagePanel = true;
	guidanceModel.showMessagePicturePanel = false;
	guidanceModel.showPositionPanel = false;
	guidanceModel.showProcessingPanel = false;
	guidanceModel.messageHeaderColor= Palette::White;
	guidanceModel.guidanceText = Localization::Translation::IDS_UnknownRoadmapFailure;
	guidanceModel.messageHeaderIcon = &Icons::GeneralWarning;
}


void connectToAuditTrailService(AuditTrailClient &auditTrailClient)
{
	int retryCount = 0;
	while (!auditTrailClient.WaitForConnection(1000) && retryCount++ < MaxAuditTrailConnectRetryCount)
	{};
}


void LogMissingLicences(Log &log, const ILicenseCollection &licenses )
{	
	if ( !licenses.hasActiveLicense() || licenses.hasExpiredLicense())
	{
		std::vector<UtilizationLogEvent::LicensesData>licensesMessage;
		
		if( !licenses.isActive(LicenseFeature::Roadmap))
		{		
			UtilizationLogEvent::LicensesData crm;
			crm.license = L"DynamicCoronaryRoadmap";
			crm.status = licenses.isExpired(LicenseFeature::Roadmap) ? L"expired": L"missing";
			licensesMessage.push_back(crm);
		}

		if (!licenses.isActive(LicenseFeature::StentboostLive))
		{
			UtilizationLogEvent::LicensesData stb;	
			stb.license = L"StentBoostLive";
			stb.status = licenses.isExpired(LicenseFeature::StentboostLive) ? L"expired" : L"missing";
			licensesMessage.push_back(stb);
		}

		if (!licenses.isActive(LicenseFeature::StbPostDeploy))
		{
			UtilizationLogEvent::LicensesData stb;
			stb.license = L"StentBoost (PostDeploy)";
			stb.status = licenses.isExpired(LicenseFeature::StbPostDeploy) ? L"expired" : L"missing";
			licensesMessage.push_back(stb);
		}

		if (!licenses.isActive(LicenseFeature::StbSubtract))
		{
			UtilizationLogEvent::LicensesData stb;
			stb.license = L"StentBoostSubtract (contrast) ";
			stb.status = licenses.isExpired(LicenseFeature::StbSubtract) ? L"expired" : L"missing";
			licensesMessage.push_back(stb);
		}

		UtilizationLogEvent::MissingLicense(log, licensesMessage);
	}
}

void overrideViewTypeForDebugging(CommonPlatform::Settings &ini, ViewType &viewType, SystemType &systemType)
{
	std::wstring viewTypeSetting = ini.get(L"debug", L"ViewType", L"");

	if (!viewTypeSetting.empty())
	{
		if (_wcsicmp(viewTypeSetting.c_str(), L"Allura") == 0)                    viewType = ViewType::Allura;
		else if (_wcsicmp(viewTypeSetting.c_str(), L"SmartSuiteFullHD") == 0)     viewType = ViewType::SmartSuiteFullHD;
		else if (_wcsicmp(viewTypeSetting.c_str(), L"SmartSuiteFlexVision") == 0) viewType = ViewType::SmartSuiteFlexVision;
		else if (_wcsicmp(viewTypeSetting.c_str(), L"SmartSuite19Inch") == 0) viewType = ViewType::SmartSuite19Inch;

		systemType = viewType == ViewType::Allura ? SystemType::Allura : SystemType::SmartSuite;
	}
}

}}

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	return Pci::PciApp::WinMain();
}