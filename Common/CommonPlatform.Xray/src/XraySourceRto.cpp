// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XraySourceRto.h"
#include "Log.h"
#include <Windows.h>
#include <ipsimplicity.h>
#include "SystemType.h"
#include "stlstring.h"

namespace CommonPlatform {	namespace Xray {
using namespace Simplicity;

#undef min
#undef max


static const double MeterToMm = 1000.0;

XraySourceRto::XraySourceRto(IScheduler &scheduler, SystemType systemType, const std::wstring &description) :
	XraySourceRto(scheduler, systemType, CommonPlatform::Logger(CommonPlatform::LoggerType::Platform), description)
{}

XraySourceRto::XraySourceRto(IScheduler &scheduler, SystemType systemType, CommonPlatform::Log& logger, const std::wstring &description) :
	areCwisAndDvlpConnected(false),
	cwisConnectionStatus(CwisStatus::NotConnectedSinceNoCwisServer),
	dvlpConnectionStatus(DvlpStatus::NotConnected),
	scheduler(scheduler),
	thread(nullptr),
	section(new CRITICAL_SECTION()),
	cwisEpx(description.c_str()),
	cwisGeometry(description.c_str()),
	cwisRun(description.c_str()),
	cwisBiopsy(description.c_str()),
	cwisExam(description.c_str()),
	dvlp(),
	dvlpPort(0),
	dvlpTimer(scheduler, [this] { if (dvlpConnect()) dvlpTimer.stop(); }),
	type(XrayImage::Type::None),
	geoTimer(scheduler, [this] { geoTimer.stop(); if (eventGeometryEnd) eventGeometryEnd(); }),
	imageIndex(0),
	lastReceivedRunIndex(0),
	runActive(false),
	DvlpImagePollingInterval(1000), // ms
	DvlpImageReceiveTimeOut(1000), // ms
	GeometryMovementTimeOut(0.5),   // sec
	m_log(logger),
	m_systemType(systemType)
{
	InitializeCriticalSection(section.get());

	cwisEpx.eventModelChanged = [this] { onCwisEpx();			};
	cwisGeometry.eventModelChanged = [this] { onCwisGeometry();	};
	cwisRun.eventModelChanged = [this] { onCwisRun();    	    };
	cwisBiopsy.eventModelChanged = [this] { onCwisBiopsy();	    };
	cwisExam.eventModelChanged = [this] { onCwisExam();		};

	cwisEpx.eventConnectionChanged = [this](Cwis::CwisConnectionType /* status */) { onCwisConnectionStatusChanged(); };
	cwisGeometry.eventConnectionChanged = [this](Cwis::CwisConnectionType /* status */) { onCwisConnectionStatusChanged(); };
	cwisRun.eventConnectionChanged = [this](Cwis::CwisConnectionType /* status */) { onCwisConnectionStatusChanged(); };
	cwisBiopsy.eventConnectionChanged = [this](Cwis::CwisConnectionType /* status */) { onCwisConnectionStatusChanged(); };
	cwisExam.eventConnectionChanged = [this](Cwis::CwisConnectionType /* status */) { onCwisConnectionStatusChanged(); };

	cwisEpx.eventError = [this](Cwis::CwisErrorType error) { onCwisError(L"CwisEpx", error); };
	cwisGeometry.eventError = [this](Cwis::CwisErrorType error) { onCwisError(L"CwisGeometry", error); };
	cwisRun.eventError = [this](Cwis::CwisErrorType error) { onCwisError(L"CwisRun", error); };
	cwisBiopsy.eventError = [this](Cwis::CwisErrorType error) { onCwisError(L"CwisBiopsy", error); };
	cwisExam.eventError = [this](Cwis::CwisErrorType error) { onCwisError(L"CwisExam", error); };
}


XraySourceRto::~XraySourceRto()
{
	scheduler.delAsync(*this);
	disconnect();

	DeleteCriticalSection(section.get());
	section = nullptr;
}

static const int DvlpTimingInterval = 5;

void XraySourceRto::connect(const std::wstring &DvlpAdapter, const std::wstring &DvlpMulticast, int DvlpPort, const std::wstring &cwisHost, int cwisPort)
{
	disconnect();

	cwisEpx		.connect(cwisHost.c_str(), static_cast<unsigned short>(cwisPort));
	cwisGeometry.connect(cwisHost.c_str(), static_cast<unsigned short>(cwisPort));
	cwisRun		.connect(cwisHost.c_str(), static_cast<unsigned short>(cwisPort));
	cwisBiopsy	.connect(cwisHost.c_str(), static_cast<unsigned short>(cwisPort));
	cwisExam	.connect(cwisHost.c_str(), static_cast<unsigned short>(cwisPort));
	
	dvlpPort      = DvlpPort;
	dvlpAdapter   = DvlpAdapter;
	dvlpMulticast = DvlpMulticast;

	if (!dvlpConnect()) dvlpTimer.start(DvlpTimingInterval);
}

void XraySourceRto::disconnect()
{
	dvlpTimer.stop();

	cwisEpx		.disconnect();
	cwisGeometry.disconnect();
	cwisRun		.disconnect();
	cwisBiopsy	.disconnect();
	cwisExam	.disconnect();

	if (thread != nullptr)
	{
		dvlp.close();
		WaitForSingleObject(thread, INFINITE);
		thread = nullptr;

        dvlpConnectionStatus = getDvlpConnectionStatus();
        updateConnectionStatus();
	}

	images    .clear();
	incomplete.clear();
	
	imageIndex = 0;
	lastReceivedRunIndex   = 0;
	runActive  = false;
}

bool XraySourceRto::dvlpConnect()
{
	char adapter  [1024];	sprintf_s(adapter,   "%S", dvlpAdapter  .c_str());
	char multicast[1024];	sprintf_s(multicast, "%S", dvlpMulticast.c_str());

	if (dvlpAdapter.empty())
	{
		if (!dvlp.open(0, static_cast<unsigned short>(dvlpPort), 0))
		{
			return false;
		}
	}
	else if (dvlpMulticast.empty())
	{
		if (!dvlp.open(dvlp.resolve(adapter), static_cast<unsigned short>(dvlpPort), 0))
		{
			return false;
		}
	}
	else
	{
		if (!dvlp.open(dvlp.resolve(multicast), dvlp.resolve(adapter), static_cast<unsigned short>(dvlpPort), 0))
		{
			return false;
		}
	}

    dvlpConnectionStatus = getDvlpConnectionStatus();
    updateConnectionStatus();

    // todo add one-time jumbo frames check

	thread = CreateThread(nullptr, 0, &threadProc, this, 0, nullptr);
	return true;
}

void XraySourceRto::dvlpLoop()
{
	ThreadPool pool(false, "dvlp");
	NodeBasic  basicProcessor(pool);

	for (;;)
	{
		Dvlp::Metadata	meta;

		Dvlp::Receiver::Status status = dvlp.receiveHeader(meta, DvlpImagePollingInterval);
		if (status == Dvlp::Receiver::Status_Closed) break; // socket is closed by disconnect, application is closing.

		std::shared_ptr<XrayImage> image;
		
		if (status == Dvlp::Receiver::Status_ImageReady)
		{
            image = receiveXrayImage();
            if (meta.getXrayTaste() == 4) image = nullptr;	//ignore review images

			if (image)
            {
                image->runIndex = meta.getRunNumber();

                // Shift Most significant bit-aligned images from DVLP to Least significant bit-aligned images for processing
                // In other words: ImageStored remains 14 bit, bit high bit is changed from 15 to 13.
                basicProcessor.shift(*image, *image, 2);
            }
		}

        // Always put an image in the queue, even if it is empty.
        // With empty images the synchronization mechanism can detect that no images have been received by the dvlp thread.
        EnterCriticalSection(section.get());
		images.push_back(image);
		if (images.size() == 1) asyncInvoke([this] { onDvlp(); });
        if (!image)
        {
            DvlpStatus connectionStatus = getDvlpConnectionStatus();
            if ( dvlpConnectionStatus != connectionStatus )
            {
                dvlpConnectionStatus = connectionStatus;
                asyncInvoke([this] { updateConnectionStatus(); });
            }
        }
		LeaveCriticalSection(section.get());
	}

        dvlpConnectionStatus = DvlpStatus::NotConnected;
        asyncInvoke([this] { updateConnectionStatus(); });

}

std::shared_ptr<XrayImage> XraySourceRto::receiveXrayImage()
{
    Dvlp::Image		src;

	std::shared_ptr<XrayImage> image = std::make_shared<XrayImage>();
	image->resize(dvlp.getWidth(), dvlp.getHeight()); // Allocate buffer to receive buffer

    // Read image number of the next image (Only valid after receiveHeader() and before receive()
    // The image number from dvlp is reset every run.
    image->imageNumber = dvlp.getImageNumber();

	src.width   = image->width;
	src.height  = image->height;
	src.stride  = image->stride;
	src.pointer = reinterpret_cast<unsigned short*>(image->pointer);
		
	if (dvlp.receive(src, DvlpImageReceiveTimeOut) != 0)	image = nullptr;	//ignore if not received

    return image;
}

void XraySourceRto::onCwisGeometry()
{
    // Attributes are updated if both corresponding model and attribute valid flag are true
    // Otherwise, keep previous attribute values or worst-case the default of the XrayGeometry c'tor

    if ( cwisGeometry.modality.modelValid ) 
    {
        readCwisGeometryModel();
        updateXrayGeometry();

        //start timer to trigger end-of-geometry when no movement for a predefined time...
	    geoTimer.start(GeometryMovementTimeOut);
    }    
}

void XraySourceRto::onCwisRun()
{
    if ( cwisRun.modality.modelValid )
    {
        readCwisRunModel();   
        updateXrayGeometry();
    }
}

void XraySourceRto::onCwisBiopsy()
{
    readCwisBiopsyModel();
    updateXrayGeometry();
}

void XraySourceRto::updateXrayGeometry()
{
    geometry.updateSpaces();

    if (eventGeometry) eventGeometry(geometry);
}

bool XraySourceRto::requiredCwisModelsAreValid() const
{
    return ( cwisGeometry.modality.modelValid || cwisRun.modality.modelValid );
}

void XraySourceRto::readCwisGeometryModel()
{
    if (cwisGeometry.modality.frontalPatientToXraySourceRotationAngleValid       ) geometry.rotation                       = cwisGeometry.modality.frontalPatientToXraySourceRotationAngle;
    if (cwisGeometry.modality.frontalPatientToXraySourceAngulationAngleValid     ) geometry.angulation                     = cwisGeometry.modality.frontalPatientToXraySourceAngulationAngle;
    if (cwisGeometry.modality.beamIsoCenterToRoomLocationHeightValid             ) geometry.beamIsoCenterHeight            = cwisGeometry.modality.beamIsoCenterToRoomLocationHeight;
    if (cwisGeometry.modality.frontalXraySourceToDetectorSourceImageDistanceValid) geometry.sourceImageDistance            = cwisGeometry.modality.frontalXraySourceToDetectorSourceImageDistance;
    if (cwisGeometry.modality.frontalPatientToXraySourceSourceObjectDistanceValid) geometry.sourceObjectDistance           = cwisGeometry.modality.frontalPatientToXraySourceSourceObjectDistance;
    if (cwisGeometry.modality.patientToPatientSupportNoseOrientationValid        ) geometry.patientNoseOrientation         = XrayGeometry::NoseOrientation(int(cwisGeometry.modality.patientToPatientSupportNoseOrientation));
    if (cwisGeometry.modality.patientToPatientSupportLegsOrientationValid        ) geometry.patientLegsToRoomOrientation   = XrayGeometry::LegsOrientation(int(cwisGeometry.modality.patientToPatientSupportLegsOrientation));
    if (cwisGeometry.modality.frontalPatientToDetectorNosetowardsDetectorValid   ) geometry.noseTowardsDetector            = cwisGeometry.modality.frontalPatientToDetectorNosetowardsDetector;

	readCwisTableGeometryModel();
	readCwisFrontalStandGeometryModel();
    geometry.updatePatientLegsToTableOrientation();
}

void XraySourceRto::readCwisTableGeometryModel()
{
	if (cwisGeometry.modality.patientSupportToRoomLocationCradleAngleValid		) geometry.tableCradleAngle				= cwisGeometry.modality.patientSupportToRoomLocationCradleAngle;
	if (cwisGeometry.modality.patientSupportToRoomLocationHeightValid			) geometry.tableHeight					= cwisGeometry.modality.patientSupportToRoomLocationHeight;
	if (cwisGeometry.modality.patientSupportToRoomLocationLateralOffsetValid	) geometry.tableLateralOffset			= cwisGeometry.modality.patientSupportToRoomLocationLateralOffset;
	if (cwisGeometry.modality.patientSupportToRoomLocationLongitudinalOffsetValid) geometry.tableLongitudinalOffset		= cwisGeometry.modality.patientSupportToRoomLocationLongitudinalOffset;
	if (cwisGeometry.modality.patientSupportToRoomLocationPivotAngleValid		) geometry.tablePivotAngle				= cwisGeometry.modality.patientSupportToRoomLocationPivotAngle;
	if (cwisGeometry.modality.patientSupportToRoomLocationSwivelAngleValid		) geometry.tableSwivelAngle				= cwisGeometry.modality.patientSupportToRoomLocationSwivelAngle;
	if (cwisGeometry.modality.patientSupportToRoomLocationTiltAngleValid		) geometry.tableTiltAngle				= cwisGeometry.modality.patientSupportToRoomLocationTiltAngle;
}

void XraySourceRto::readCwisFrontalStandGeometryModel()
{
	if (cwisGeometry.modality.frontalStandBeamTransversalValid) geometry.beamLateralOffset = cwisGeometry.modality.frontalStandBeamTransversal;
	if (cwisGeometry.modality.frontalStandBeamLongitudinalValid) geometry.beamLongitudinalOffset = cwisGeometry.modality.frontalStandBeamLongitudinal;

	if (cwisGeometry.modality.frontalStandZRotationValid) geometry.standLArmAngle = cwisGeometry.modality.frontalStandZRotation;
	if (cwisGeometry.modality.frontalStandPropellerRotationValid) geometry.standPropellerAngle = cwisGeometry.modality.frontalStandPropellerRotation;
	if (cwisGeometry.modality.frontalStandCarcRotationValid) geometry.standRollAngle = cwisGeometry.modality.frontalStandCarcRotation;


	// in the Azurion 1.x the frontalStentDetectorRotation is incorrect. Therefore we must use the sourceto detector spine angle.
	// this value will be obsolete so we may only use it for the Azurion 1.x. in all othercases we must use standDetectorRotation.
	if ( m_systemType == SystemType::SmartSuite_1x)
	{		
		if (cwisGeometry.modality.frontalXraySourceToDetectorSpinAngleValid)
		{
			// in the Azurion 1.x the frontalStentDetectorRotation is incorrect. Therefore we must use the sourceto detector spine angle.
			geometry.detectorRotationAngle = -cwisGeometry.modality.frontalXraySourceToDetectorSpinAngle;
		}
	}
	else
	{
		if (cwisGeometry.modality.frontalStandDetectorRotationValid)	
		{
			geometry.detectorRotationAngle = cwisGeometry.modality.frontalStandDetectorRotation;
		}
	}

}


void XraySourceRto::readCwisRunModel()
{
    // Note: without X-ray, the modality sends out Cwis with frontalTechnicalReceptorFieldSizeValid=false
    geometry.detectorFieldSizeX		        = cwisRun     .modality.frontalTechnicalReceptorFieldSizeX * MeterToMm;
    geometry.detectorFieldSizeY		        = cwisRun     .modality.frontalTechnicalReceptorFieldSizeY * MeterToMm;
}

void XraySourceRto::readCwisBiopsyModel()
{
    geometry.detectorHorizontalFlip             = cwisBiopsy  .modality.frontalHflipped;
    geometry.detectorImageRotated               = cwisBiopsy  .modality.frontalZrotated;
    geometry.detectorVerticalFlip               = cwisBiopsy  .modality.frontalVflipped;
}

void XraySourceRto::onCwisEpx()
{
    if ( cwisEpx.modality.modelValid && cwisEpx.modality.activityTypeValid )
    {
        epx.activityType = cwisEpx.modality.activityType;
	    if (eventEpx) eventEpx(epx);
    }
}

void XraySourceRto::onCwisExam()
{
	std::wstring sid;

	//TODO: CHECK IF THIS IS VALID, SHOULDN'T THE INSTANCEUID BE USED; JUST ADD ALL THE INFO TO THE STRUCT AND CHECK LATER ON
	if (cwisExam.modality.studyIdValid					&& !cwisExam.modality.studyId					->empty()) sid = cwisExam.modality.studyId;						else
	if (cwisExam.modality.performedProcedureStepIdValid && !cwisExam.modality.performedProcedureStepId	->empty()) sid = cwisExam.modality.performedProcedureStepId;	else
	if (cwisExam.modality.studyInstanceUidValid			&& !cwisExam.modality.studyInstanceUid			->empty()) sid = cwisExam.modality.studyInstanceUid; 
	
	if (!cwisExam.modality.patientIdValid	||	cwisExam.modality.patientId	-> empty())	return;	
	if (sid.empty())								                                    return;
	
	XrayStudy study;
		
	study.studyId					= sid;
	study.studyInstanceUid			= cwisExam.modality.studyInstanceUid;
	study.performedProcedureStepId  = cwisExam.modality.performedProcedureStepId;
	study.patientId					= cwisExam.modality.patientId;
	study.patientName				= cwisExam.modality.patientName;
	study.patientBirthdate			= cwisExam.modality.patientBirthDate;
	study.patientGender				= cwisExam.modality.patientSex;
    study.referringPhysician		= cwisExam.modality.referringPhysicianName;
    study.studyDate					= str_ext::wtos(cwisExam.modality.studyDate);
    study.studyTime					= str_ext::wtos(cwisExam.modality.studyTime);
    study.accessionNumber			= cwisExam.modality.accessionNumber;

	if (cwisExam.modality.requestedProcedureIdValid && !cwisExam.modality.requestedProcedureId->empty())
	{
		study.requestedProcedureId = cwisExam.modality.requestedProcedureId;
	}
	if (cwisExam.modality.institutionNameValid  && !cwisExam.modality.institutionName->empty())
	{
		study.institutionName = cwisExam.modality.institutionName;
	}
	if (cwisExam.modality.otherPatientIdsValid && !cwisExam.modality.otherPatientIds->empty())
	{
		study.otherPatientIds = cwisExam.modality.otherPatientIds;
	}

	if (eventStudy) eventStudy(study); 
}

static const wchar_t* getCwisConnectionString(Cwis::CwisConnectionType connection)
{
	if (connection == Cwis::CwisConnectionType::Active)       return L"Active";
	if (connection == Cwis::CwisConnectionType::Connected)    return L"Connected";
	if (connection == Cwis::CwisConnectionType::Disconnected) return L"Disconnected";

	return L"";
}

void XraySourceRto::onCwisConnectionStatusChanged()
{
    CwisStatus status = getCwisConnectionStatus();

	m_log.DeveloperInfo(L"XraySourceRto::onCwisConnectionChanged [status=%d, cwisGeometry=%s, cwisRun=%s, cwisBiopsy=%s, cwisExam=%s, cwisEpx=%s]",
			status,
			getCwisConnectionString(cwisGeometry.isConnected()),
			getCwisConnectionString(cwisRun.isConnected()),
			getCwisConnectionString(cwisBiopsy.isConnected()),
			getCwisConnectionString(cwisExam.isConnected()),
			getCwisConnectionString(cwisEpx.isConnected()));

    if ( cwisConnectionStatus != status )
    {
        cwisConnectionStatus = status;
        updateConnectionStatus();
    }
}

void XraySourceRto::HandleEndOfRun()
{
	m_log.DeveloperInfo(L"Run ended");

	// The dvlp thread notified us that no images were received, or the cwis indicates a new run; signal end-of-run if needed.
	if (eventImageEnd) eventImageEnd();
			
	imageIndex = 0;
	lastReceivedRunIndex   = cwisRun.modality.seriesNumber;
	type	   = XrayImage::Type::None;
	runActive  = false;
}

void XraySourceRto::HandleAbortedRun()
{
	m_log.DeveloperInfo(L"Run aborted");

	// The dvlp thread notified us that no images were received, or the cwis indicates a new run; signal end-of-run if needed.
	if (eventImageEnd) eventImageEnd();
			
	imageIndex = 0;
	lastReceivedRunIndex   = -1; // invalidate last aborted run
	type	   = XrayImage::Type::None;
	runActive  = false; // is set to TRUE in handing of image that aborted the run
}
bool XraySourceRto::isRunAborted(std::shared_ptr<XrayImage> &image)
{
    return (image && image->imageNumber == 0 && runActive);
}

void XraySourceRto::SyncIncompleteImages()
{
	for (auto &i:incomplete) if (synchronizeImageWithMetaData(i))
	{ 
		i->imageIndex = imageIndex; 
		if (eventImage) eventImage(i, true);
		imageIndex++;
	} 
    incomplete.clear();
}

// This function is executed on the client thread
void XraySourceRto::onDvlp()
{
    while ( !isImageQueueEmpty() )
	{
        std::shared_ptr<XrayImage> image = getImageFromQueue();

        if (image)
		{
            if ( isRunAborted(image) ) HandleAbortedRun();

    		if ( !synchronizeImageWithMetaData(image) )
	    	{
			    // Image cannot be matched with CWIS data yet; store temporarily; we might be able to match in the near future...
			    incomplete.push_back(image);
			    if (incomplete.size() > 15) incomplete.pop_front();
		    }
            else
            {
				if ((!runActive) && (eventImageBegin!= nullptr))
				{
					if (lastReceivedRunIndex == image->runIndex)
					{
						m_log.DeveloperError(L"Second run with same runIndex as previous run. Might be caused by RTO interruption. Run index: %d", lastReceivedRunIndex);
					}

					asyncInvoke([&, image]() { eventImageBegin(image->type); });
				}

    		    // The current image matches the CWIS data. 
                // Check if there are images (older then current image) of this run that we should process first...
                SyncIncompleteImages();

                // Is the queue is not empty yet, it is an indication that the client is not handling event slower than the incoming rate.
                bool skip = !isImageQueueEmpty();
		
    		    image->imageIndex = imageIndex;
	    	    imageIndex++;

		        runActive = true;
		        if (eventImage) eventImage(image, skip);
            }
        }
        else
        {
            if ( runActive ) HandleEndOfRun();
        }
	}
}

bool XraySourceRto::synchronizeImageWithMetaData(std::shared_ptr<XrayImage> &image)
{	
    if ( !canSyncImage(image) ) return false;
    return fillMetaData(image);
}

bool XraySourceRto::canSyncImage(std::shared_ptr<XrayImage> &image) const
{
	if(!image)															return false;
	if(cwisRun.modality.seriesNumber != int(image->runIndex))			return false;
	if(cwisEpx	   .isConnected() != Cwis::CwisConnectionType::Active)	return false;
	if(cwisGeometry.isConnected() != Cwis::CwisConnectionType::Active)	return false;
	if(cwisRun	   .isConnected() != Cwis::CwisConnectionType::Active)	return false;
	if(cwisBiopsy  .isConnected() != Cwis::CwisConnectionType::Active)	return false;
	if(cwisExam	   .isConnected() != Cwis::CwisConnectionType::Active)	return false;
    return true;
}

bool XraySourceRto::fillMetaData(std::shared_ptr<XrayImage> &image)
{
	if (type == XrayImage::Type::None)
	{
		if (cwisRun.modality.fluoroActive)		type = XrayImage::Type::Fluoro;
		if (cwisRun.modality.exposureActive)	type = XrayImage::Type::Exposure;
	}
	
	if (type == XrayImage::Type::None) return false;

	image->type			= type;
	image->epx			= epx;
	image->geometry     = geometry;
	image->validRect	= Rect(0, 0, image->width, image->height);
	image->shutters		= Rect(0, 0, image->width, image->height);
	image->mmPerPixel	= Point(cwisRun.modality.frontalImagerPixelSpacingX, cwisRun.modality.frontalImagerPixelSpacingY);
	image->frameTime    = type == XrayImage::Type::Fluoro ? cwisEpx.modality.fluoroFrameTime / MeterToMm : cwisEpx.modality.exposureFrameTime / MeterToMm;
    image->seriesNumber = cwisRun.modality.seriesNumber;
	image->bitdepth = cwisRun.modality.frontalImageInfoBitsStored;

    if (cwisRun.modality.frontalValidRectangleValid)
	{
		image->validRect.left   = std::max(0, std::min(image->width,  static_cast<int>(cwisRun.modality.frontalValidRectangleTopLeftX) - 1));
		image->validRect.top    = std::max(0, std::min(image->height, static_cast<int>(cwisRun.modality.frontalValidRectangleTopLeftY) - 1));
		image->validRect.right  = std::max(0, std::min(image->width,  static_cast<int>(cwisRun.modality.frontalValidRectangleBottomRightX)));
		image->validRect.bottom = std::max(0, std::min(image->height, static_cast<int>(cwisRun.modality.frontalValidRectangleBottomRightY)));
	}
	
	if (cwisRun.modality.frontalShutterValid && cwisRun.modality.frontalImageInfoValid)
	{
        // todo check cwis models for shutter coordinates relative to matrix size
		image->shutters.left   = std::max(0, std::min(image->width,  static_cast<int>(cwisRun.modality.frontalShutterLeft)));
		image->shutters.top    = std::max(0, std::min(image->height, static_cast<int>(cwisRun.modality.frontalShutterTop)));
		image->shutters.right  = std::max(0, std::min(image->width,  static_cast<int>(cwisRun.modality.frontalImageInfoColumns) - static_cast<int>(cwisRun.modality.frontalShutterRight)));
		image->shutters.bottom = std::max(0, std::min(image->height, static_cast<int>(cwisRun.modality.frontalImageInfoRows)    - static_cast<int>(cwisRun.modality.frontalShutterBottom)));
	}
	
    // For patient orientation (HeadFirst,NoseUp) the positive Y-axis of the image corresponds to the negative Y-axis of the system's coordinate space.
	image->frustumToPixels = Matrix::scale(Point(1.0, -1.0));
	image->frustumToPixels = Matrix::scale(image->mmPerPixel.x > 0 && image->mmPerPixel.y > 0 ? Point(1.0) / image->mmPerPixel : Point(0.0)) * image->frustumToPixels;
	image->frustumToPixels = Matrix::translate(Point(image->width, image->height) / 2.0) * image->frustumToPixels;
	
	image->shutters = image->shutters & image->validRect;
	return true;
}


unsigned long XraySourceRto::threadProc(void *param)
{
	static_cast<XraySourceRto*>(param)->dvlpLoop();
	return 0;
}

void XraySourceRto::asyncInvoke(const std::function<void()> &command) const
{
	scheduler.addAsync(*this, command);
}

IXraySourceRto::CwisStatus XraySourceRto::getCwisConnectionStatus()
{
    auto status = CwisStatus::NotConnectedSinceNoCwisServer;

    if      ( allCwisClientsActive() )       status = CwisStatus::Connected;
    else if ( atLeastOneClientActive())      status = CwisStatus::PartiallyConnected;
    else if ( atLeastOneClientConnected())   status = CwisStatus::NotConnectedSinceNoActiveConnections;

    return status;
}

bool XraySourceRto::allCwisClientsActive() const
{
    return (    cwisGeometry .isConnected() == Cwis::CwisConnectionType::Active &&
                cwisRun      .isConnected() == Cwis::CwisConnectionType::Active &&
                cwisBiopsy   .isConnected() == Cwis::CwisConnectionType::Active &&
                cwisExam     .isConnected() == Cwis::CwisConnectionType::Active &&
                cwisEpx      .isConnected() == Cwis::CwisConnectionType::Active );
}

bool XraySourceRto::atLeastOneClientActive() const
{
    return (    cwisGeometry .isConnected() == Cwis::CwisConnectionType::Active ||
                cwisRun      .isConnected() == Cwis::CwisConnectionType::Active ||
                cwisBiopsy   .isConnected() == Cwis::CwisConnectionType::Active ||
                cwisExam     .isConnected() == Cwis::CwisConnectionType::Active ||
                cwisEpx      .isConnected() == Cwis::CwisConnectionType::Active );
}

bool XraySourceRto::atLeastOneClientConnected() const
{
    return  (   cwisGeometry .isConnected() == Cwis::CwisConnectionType::Connected ||
                cwisRun      .isConnected() == Cwis::CwisConnectionType::Connected ||
                cwisBiopsy   .isConnected() == Cwis::CwisConnectionType::Connected ||
                cwisExam     .isConnected() == Cwis::CwisConnectionType::Connected ||
                cwisEpx      .isConnected() == Cwis::CwisConnectionType::Connected );
}

// Return true, when all Cwis models are connected and Dvlp is connected.
bool XraySourceRto::isConnected()
{
    return areCwisAndDvlpConnected;
}

IXraySourceRto::DvlpStatus XraySourceRto::getDvlpConnectionStatus()
{
    DvlpStatus status = DvlpStatus::NotConnectedSinceNoAdapterDefined; 

    switch ( dvlp.checkConnection() )
    {
        case DvlpReceiverMonitor::DvlpConnectionStatus::Connected:                              status = DvlpStatus::Connected;                             break;
        case DvlpReceiverMonitor::DvlpConnectionStatus::NotConnectedSinceNoCable:               status = DvlpStatus::NotConnectedSinceNoCable;              break;
        case DvlpReceiverMonitor::DvlpConnectionStatus::NotConnectedSinceOpeningSocketFailed:   status = DvlpStatus::NotConnectedSinceOpeningSocketFailed;  break;
        case DvlpReceiverMonitor::DvlpConnectionStatus::NotConnectedSinceNoAdapterDefined:      status = DvlpStatus::NotConnectedSinceNoAdapterDefined;     break;
    }

    return status;
}

void XraySourceRto::updateConnectionStatus()
{
    bool combinedConnected = cwisConnectionStatus == CwisStatus::Connected &&
                             dvlpConnectionStatus == DvlpStatus::Connected;

    // Check if we need to send an ImageEnd event due to a change to disconnected
    if ( areCwisAndDvlpConnected != combinedConnected )
    {
        areCwisAndDvlpConnected = combinedConnected;

        if (runActive && (!areCwisAndDvlpConnected) && (eventImageEnd != nullptr)) 
        {
            eventImageEnd();
        }
    }

    if (eventConnectionChanged) eventConnectionChanged( areCwisAndDvlpConnected, 
                                                        cwisConnectionStatus, 
                                                        dvlpConnectionStatus);
}

bool XraySourceRto::isImageQueueEmpty()const
{
	EnterCriticalSection(section.get());
	bool empty = images.empty();
    LeaveCriticalSection(section.get());
	return empty;
}

std::shared_ptr<XrayImage> XraySourceRto::getImageFromQueue()
{
    EnterCriticalSection(section.get());
    std::shared_ptr<XrayImage> image = images.front();
	images.pop_front();
    LeaveCriticalSection(section.get());
    return image;
}

static const wchar_t* getCwisErrorDescription(Cwis::CwisErrorType error)
{
	switch (error)
	{
		case Cwis::CwisErrorType::IncompatibleProtocol:   return L"IncompatibleProtocol";
		case Cwis::CwisErrorType::IncompatibleEncoding:   return L"IncompatibleEncoding";
		case Cwis::CwisErrorType::InvalidData:            return L"InvalidData";
		case Cwis::CwisErrorType::None:                   return L"None";
		default:                                          return L"Unknown error";
	}
}

void XraySourceRto::onCwisError(const wchar_t* modelName, Cwis::CwisErrorType error) const
{
	if (eventCwisError)
	{
		eventCwisError(modelName, getCwisErrorDescription(error));
	}
}

}}
