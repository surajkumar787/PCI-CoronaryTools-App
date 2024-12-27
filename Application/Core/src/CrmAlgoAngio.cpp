// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: CrmAlgoAngio.cpp

#include "CrmAlgoAngio.h"
#include <algorithm>
#include <Osc.h>

#undef min
#undef max

using namespace PciSuite;
using namespace Pci::Core;

CrmAlgoAngio::CrmAlgoAngio(Simplicity::ThreadPool &pool, const CrmParameters &crmParams) :
	CrmAlgoAngio(CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap), pool, crmParams)
{
}

CrmAlgoAngio::CrmAlgoAngio(CommonPlatform::Log& log, Simplicity::ThreadPool &pool, const CrmParameters &crmParams)
:
	buffer			(nullptr),
	handle			(new COscHdl()),	
	threadPool		(pool),
	count			(0),
	overlayShutters	(),
	overlayMatrix	(),
	tableOrLArmMoved(false),
	cArmMoved		(false),
	error			(CrmErrors::Error::OK),
	roadmap			(),
	params			(crmParams),
	m_log			(log)
{
    int major=0,minor=0,patch=0;
    OscGetVersionNumbers(&major, &minor, &patch);
    m_log.DeveloperInfo(L"Roadmap processing library version=%d.%d.%d", major, minor, patch );

    OscCreate(static_cast<COscHdl*>(handle), 
                                                        14,
                                                        1024, 1024,
                                                        512, 512, 
                /* Maximum frame rate = */              30, 
                /* Maximum lenegth of angio [sec] ] */  4,
                                                        CrmThreadPool::maxNumThreads);
}

CrmAlgoAngio::~CrmAlgoAngio()
{
	exit();
	if (handle != nullptr) OscDelete(*static_cast<COscHdl*>(handle));
	
	delete reinterpret_cast<COscHdl*>(handle);
}

void CrmAlgoAngio::init(const XrayImage &src)
{
	exit();

    m_log.DeveloperInfo(L"Start processing of angio run. [RunIndex=%d, SeriesNumber=%d, ImageWidth=%d, ImageHeight=%d, mmPerPixel=(%f,%f)]",
                        src.runIndex, src.seriesNumber, src.width, src.height, src.mmPerPixel.x, src.mmPerPixel.y);

	roadmap		= CrmRoadmap(src.runIndex, src.seriesNumber, src.width, src.height, src.mmPerPixel, src.geometry, src.frustumToPixels, 0.0);
	buffer		= new short[roadmap.width*roadmap.height];
	count		= 0;
	tableOrLArmMoved = false;
	cArmMoved	= false;
	error		= CrmErrors::Error::OK;

	int	internalSSFactor = GetInternalSubsamplingFactor(src);		  

	float fps = src.frameTime > 0.0 ? static_cast<float>(1.0/src.frameTime) : 15.0f;

	if (!CrmErrors::mapOscErrorToCrmError(OscSetAngioSequenceParameters(*static_cast<COscHdl*>(handle), src.width, src.height, fps, internalSSFactor, &threadPool), error, m_log)) return;
	if (!CrmErrors::mapOscErrorToCrmError(OscSetTableApparentPositionMM(*static_cast<COscHdl*>(handle), 0, 0, 0), error, m_log)) return;

	overlayShutters = src.shutters;
	overlayMatrix   = src.frustumToPixels;
}

int CrmAlgoAngio::GetInternalSubsamplingFactor(const XrayImage &src) const
{
	 // use valid rectangle
	int						  internalSSFactor = 1;
	if (src.height > 500) internalSSFactor = 2;
	if (src.height > 800) internalSSFactor = 2;

	return internalSSFactor;
}

float CrmAlgoAngio::getMmPerPixelInIsoCenter(const std::shared_ptr<const XrayImage> & src)  const
{
	if(src->geometry.sourceImageDistance > 0)
	{
		return static_cast<float>(src->mmPerPixel.x * src->geometry.sourceObjectDistance / src->geometry.sourceImageDistance);
	}

	return 0;
}

void CrmAlgoAngio::exit()
{
	delete [] buffer;
	buffer = nullptr;
	count  = 0;
}
	

bool CrmAlgoAngio::process(const std::shared_ptr<const XrayImage> &src)
{
	if (!src) return false;

	if (count == 0) init(*src);

	// if table or C-arm moves, stop processing of incoming angio images, 
	// but finalize the end processing (of the sequence) because it may still be successful.
	if (roadmap.getPanningDistance(src->geometry) > 0.5)	tableOrLArmMoved = true;
	if (roadmap.getAngleDistance  (src->geometry) > 0.1)	cArmMoved = true;

	if (handle == nullptr)				return false;
	if (buffer == nullptr)				return false;
	if (src->width  != roadmap.width)	return false;
	if (src->height != roadmap.height)	return false;
	if (error != CrmErrors::Error::OK)	return false;
	if (tableOrLArmMoved)				return false;
    if (cArmMoved)						return false;

	//because algorithm doesn't support stride, we have to copy the data into a continuous buffer here...
	for (int y = 0; y < roadmap.height; y++)
	{
		memcpy(buffer + y*roadmap.width, src->pointer + y*src->stride, roadmap.width * sizeof(short));
	}

	COscRoiCoord roi;
	roi.XMin = static_cast<int>(src->shutters.left);
	roi.YMin = static_cast<int>(src->shutters.top);
	roi.XMax = static_cast<int>(src->shutters.right)  - 1;
	roi.YMax = static_cast<int>(src->shutters.bottom) - 1;

	//// Algorithm requires a margin of 64x64 pixels between the shutter positions
	int centerXOfShutters = (roi.XMin+roi.XMax)/2;
	int centerYOfShutters = (roi.YMin+roi.YMax)/2;
	roi.XMin = std::max(0,             std::min(roi.XMin , centerXOfShutters-32));
	roi.XMax = std::min(src->width-1,  std::max(roi.XMax , centerXOfShutters+32));
	roi.YMin = std::max(0,             std::min(roi.YMin , centerYOfShutters-32));
	roi.YMax = std::min(src->height-1, std::max(roi.YMax , centerYOfShutters+32));
	 
	if (!CrmErrors::mapOscErrorToCrmError(OscSetShuttersRoi(*static_cast<COscHdl*>(handle), roi), error, m_log)) return false;

	float mmPerPixelInIsoCenter = getMmPerPixelInIsoCenter(src);

	COscAngioPictureResults results;
	results.Advanced.Volatile.Ovr = nullptr;

	if (!CrmErrors::mapOscErrorToCrmError(OscSetCurrentGeometryAngio		(*static_cast<COscHdl*>(handle), static_cast<float>(src->geometry.rotation), static_cast<float>(src->geometry.angulation), mmPerPixelInIsoCenter), error, m_log))	return false;
	if (!CrmErrors::mapOscErrorToCrmError(OscSetTableApparentPositionPix	(*static_cast<COscHdl*>(handle), 0, 0), error, m_log))																					return false;
	if (!CrmErrors::mapOscErrorToCrmError(OscAngioProcessPicture			(*static_cast<COscHdl*>(handle), buffer, count, &results), error, m_log))																return false;

	count++;
	return true;
}

CrmErrors::Error CrmAlgoAngio::end(const std::shared_ptr<CrmRoadmap> &dst)
{
    m_log.DeveloperInfo(L"End processing of angio run. [Number of processed images=%d, tableOrLArmMoved=%s, CArmMoved=%s, Algorithm result after preprocessing images=%d]", 
                        count, tableOrLArmMoved ? L"Yes" : L"No", cArmMoved ? L"Yes" : L"No", error);

	if (count == 0)
	{
		return error == CrmErrors::Error::OK ? CrmErrors::Error::InjectionTooShort : error;
	}

	count = 0;
    COscAngioSequenceResults results = {};

	if (dst == nullptr)
	{
		return CrmErrors::Error::Unknown;
	}

	if (error != CrmErrors::Error::OK || !CrmErrors::mapOscErrorToCrmError(OscAngioProcessSequence(*static_cast<COscHdl*>(handle), &results), error, m_log))
	{
        m_log.DeveloperInfo(L"End processing of angio run. [Algorithm result after processing sequence=%d, RoadmapQuality=%f, CardiacCyle=%d-%d]", 
                            error, results.normalizedInjectionCoefficient, results.CardiacCycleStart, results.CardiacCycleEnd );

        if (tableOrLArmMoved && cArmMoved)
        {
            m_log.DeveloperInfo(L"End processing of angio run. [Override error=CArm and table moved]");
            return CrmErrors::Error::CarmAndTableMoved;
        }
		if (tableOrLArmMoved)
        {
            m_log.DeveloperInfo(L"End processing of angio run. [Override error=Table moved]");
            return CrmErrors::Error::PatientTableMoved;
        }
		if (cArmMoved)
        {
            m_log.DeveloperInfo(L"End processing of angio run. [Override error=CArm moved]");
            return CrmErrors::Error::CarmMoved;
        }

        m_log.DeveloperInfo(L"End processing of angio run. [Error=%d]", error);
		return error;
	}

	roadmap.warning = error;
    roadmap.quality = results.normalizedInjectionCoefficient;

    m_log.DeveloperInfo(L"End processing of angio run. [AlgorithmResultOfProcessingSequence=%d, RoadmapQuality=%f, CardiacCyle=%d-%d]", 
        error, results.normalizedInjectionCoefficient, results.CardiacCycleStart, results.CardiacCycleEnd );

	*dst = roadmap;
	dst->data.resize(OscComputeMemorySizeForSave(*static_cast<COscHdl*>(handle)));
	if (!CrmErrors::mapOscErrorToCrmError(OscSaveToMemory(*static_cast<COscHdl*>(handle), dst->data.data()), error, m_log))
	{
		return CrmErrors::Error::SaveFailure;
	}

	int index = (results.CardiacCycleStart + results.CardiacCycleEnd) / 2;
    
	int overlayWidth  = 0;
	int overlayHeight = 0;
	int overlayScale  = 0;

	unsigned char *ptr = OscAngioSeqGetOverlayAssociatedToAngioFrame(*static_cast<COscHdl*>(handle), index);
	OscAngioGetOvrSize(*static_cast<COscHdl*>(handle), &overlayWidth, &overlayHeight, &overlayScale);

	if ((ptr == nullptr) || overlayWidth == 0 || overlayHeight == 0 || overlayScale == 0)
	{
		return CrmErrors::Error::SaveFailure;
	}

	std::shared_ptr<CrmOverlay>	overlay = std::make_shared<CrmOverlay>();
	overlay->resize(overlayWidth, overlayHeight);
	overlay->frustumToPixels	= Matrix::scale(1.0/static_cast<double>(overlayScale)) * overlayMatrix;
	overlay->shutters			= overlayShutters / static_cast<double>(overlayScale);

	for (int y = 0; y < overlayHeight; y++)
	{
		memcpy(overlay->pointer + y*overlay->stride, ptr + y*overlayWidth, overlayWidth);
	}

	dst->overlay = overlay;

	return CrmErrors::Error::OK;
}