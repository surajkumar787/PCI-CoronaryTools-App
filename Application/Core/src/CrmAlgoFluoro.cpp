// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//

#include "CrmAlgoFluoro.h"
#include "SystemType.h"
#include <algorithm>

#undef min
#undef max

using namespace PciSuite;
namespace Pci { namespace Core {
	

static const short TransformationBorderValue = 8000;
static const float MaximumFrameRate = 30.0f;
static const float MaximumAngleDifferences = 4;


CrmAlgoFluoro::CrmAlgoFluoro(Simplicity::ThreadPool &pool, const CrmParameters &params, SystemType systemType):
	CrmAlgoFluoro( CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap), pool, params, systemType)
{}

CrmAlgoFluoro::CrmAlgoFluoro( CommonPlatform::Log &log,  Simplicity::ThreadPool &pool, const CrmParameters &params, SystemType systemType)
:
	buffer		                (),
	threadPool	                (pool),
	handle		                (new COscHdl()),
	count		                (0),
	error		                (CrmErrors::Error::OK),
	nodeLut		                (pool),
	basic						(pool),
	roadmap		                (nullptr),
	params		                (params),
	transformationBorderValue   (TransformationBorderValue),
	transform	                (pool),
	_systemType					(systemType),
	m_log                       (log)
{
	OscCreate( reinterpret_cast<COscHdl*>(handle), 14, 1024, 1024, 512, 512, MaximumFrameRate, MaximumAngleDifferences, CrmThreadPool::maxNumThreads);
}

CrmAlgoFluoro::~CrmAlgoFluoro()
{
	exit();
	if (handle != nullptr) OscDelete(*static_cast<COscHdl*>(handle));
	
	delete reinterpret_cast<COscHdl*>(handle);
}

void CrmAlgoFluoro::set(const std::shared_ptr<const CrmRoadmap> &Roadmap)
{
	if (roadmap == Roadmap) return;
	exit();
	
	if (!Roadmap || Roadmap->data.empty()) return;

	roadmap = Roadmap;
	buffer.resize( roadmap->width*roadmap->height);
	count	= 0;
	error   = CrmErrors::Error::OK;

	if (!CrmErrors::mapOscErrorToCrmError(OscLoadFromMemory(*static_cast<COscHdl*>(handle), (void*)roadmap->data.data()), error, m_log)) return;

	if (!CrmErrors::mapOscErrorToCrmError(OscSetTableApparentPositionMM	(*static_cast<COscHdl*>(handle), 0, 0, 0), error, m_log))	return;
	if (!CrmErrors::mapOscErrorToCrmError(OscSetTableApparentPositionPix	(*static_cast<COscHdl*>(handle), 0, 0), error, m_log))		return;
}

void CrmAlgoFluoro::exit()
{
	roadmap = nullptr;
	count   = 0;
}


static Matrix4x4 to4d(const Matrix &mat)
{
	Matrix4x4 result = Matrix4x4::identity();

	result.m[0][0] = mat.m[0][0];	result.m[0][1] = mat.m[0][1]; result.m[0][3] = mat.m[0][2];
	result.m[1][0] = mat.m[1][0];	result.m[1][1] = mat.m[1][1]; result.m[1][3] = mat.m[1][2];
	result.m[3][0] = mat.m[2][0];	result.m[3][1] = mat.m[2][1]; result.m[3][3] = mat.m[2][2];

	return result;
}

static Simplicity::NodeAffineTransform::Matrix to3d(const Matrix &mat)
{
	return Simplicity::NodeAffineTransform::Matrix( static_cast<float>(mat.m[0][0]), static_cast<float>(mat.m[0][1]), static_cast<float>(mat.m[0][2]),
													static_cast<float>(mat.m[1][0]), static_cast<float>(mat.m[1][1]), static_cast<float>(mat.m[1][2]));
}

Matrix CrmAlgoFluoro::getTransformationMatrix( const std::shared_ptr<const XrayImage> & src ) 
{
	//calculate 3D matrix to transform fluoro image to roadmap
	Matrix4x4 fluoro2angio =	to4d(	roadmap->frustumToPixels)		* 
										roadmap	->geometry.frustum.fromRoot * 
										roadmap	->geometry.patient.toRoot	* 
										src		->geometry.patient.fromRoot * 
										src		->geometry.frustum.toRoot	*  
										to4d(src->frustumToPixels).invert();

	//we have a 4D homogenous matrix with perspective; we need a 3D matrix homogenous without perspective. So we estimate the scale at iso and apply 
	//implicitly as scaling in the resulting matrix.

	//calculate axis system of fluoro image in fluoro coordinate space
	Vector3d fluoroOrigin = to4d(src->frustumToPixels) * src->geometry.frustum.fromRoot * src->geometry.iso.toRoot * Vector3d();
	Vector3d fluoroX      = fluoroOrigin + Vector3d(1,0,0);
	Vector3d fluoroY      = fluoroOrigin + Vector3d(0,1,0);

	//calculate axis system of fluoro image in roadmap coordinate space
	fluoroOrigin  = fluoro2angio * fluoroOrigin;
	fluoroX       = fluoro2angio * fluoroX - fluoroOrigin;     
	fluoroY       = fluoro2angio * fluoroY - fluoroOrigin;

	//calculate rotation matrix from x and y normals, containing the perspective scaling
	Matrix rotation = Matrix::identity();
	rotation.m[0][0] = fluoroX.x;	rotation.m[0][1] = fluoroY.x;  
	rotation.m[1][0] = fluoroX.y;	rotation.m[1][1] = fluoroY.y;  

	//calculate full matrix to transform fluoro image to roadmap
	return Matrix::translate(Point(fluoroOrigin.x, fluoroOrigin.y)) * rotation * Matrix::translate(-Point(src->width/2.0, src->height/2.0));
}

void CrmAlgoFluoro::correctShuttersRoi( const std::shared_ptr<const XrayImage> & src, Matrix &trans, COscRoiCoord &roi ) 
{
	// only correct the shutter roi if Larm rotation has not changed
	if ( abs(src->geometry.standLArmAngle - roadmap->geometry.standLArmAngle) < 0.1 ) 
	{
		//transform shutter coordinates to roadmap image space
		Point topLeft	= trans * src->shutters.topLeft();	
		Point topRight	= trans * src->shutters.topRight();
		Point botLeft	= trans * src->shutters.bottomLeft();
		Point botRight	= trans * src->shutters.bottomRight();

		roi.XMin = static_cast<int>(std::min( botLeft.x, topLeft .x));
		roi.YMin = static_cast<int>(std::min( topLeft.y, topRight.y));
		roi.XMax = static_cast<int>(std::max(topRight.x, botRight.x));
		roi.YMax = static_cast<int>(std::max(botRight.y, botLeft .y));

		// Algorithm requires a margin of 64x64 pixels between the shutter positions.
		int centerXOfShutters = (roi.XMin+roi.XMax)/2;
		int centerYOfShutters = (roi.YMin+roi.YMax)/2;
		roi.XMin = std::min(roi.XMin , centerXOfShutters-32);
		roi.XMax = std::max(roi.XMax , centerXOfShutters+32);
		roi.YMin = std::min(roi.YMin , centerYOfShutters-32);
		roi.YMax = std::max(roi.YMax , centerYOfShutters+32);

		// transformed shutter region must not have negative size
		roi.XMin = std::max(0,						                   std::min(roi.XMin, static_cast<int>(roadmap->width  / 2) - 1));
		roi.YMin = std::max(0,						                   std::min(roi.YMin, static_cast<int>(roadmap->height / 2) - 1));
		roi.XMax = std::max(static_cast<int>(roadmap->width  / 2) + 1, std::min(roi.XMax, roadmap->width  - 1));
		roi.YMax = std::max(static_cast<int>(roadmap->height / 2) + 1, std::min(roi.YMax, roadmap->height - 1));

	}
}

void CrmAlgoFluoro::applyLutsToLive( const std::shared_ptr<const XrayImage> & src, const std::shared_ptr<XrayImage> & live ) 
{
	live->resize(*src);

	live->runIndex			=src->runIndex;
	live->imageIndex		=src->imageIndex;
	live->imageNumber		=src->imageNumber;
	live->seriesNumber		=src->seriesNumber;
	live->type				=src->type;
	live->validRect			=src->validRect;
	live->shutters			=src->shutters;
	live->frustumToPixels	=src->frustumToPixels;
	live->mmPerPixel		=src->mmPerPixel;
	live->frameTime			=src->frameTime;
	live->geometry			=src->geometry;
	live->epx				=src->epx;
	
	// Apply perception lut to make images output DICOM compatible for allura systems
	// For SmartSuite systems the Perception lut is already applied on processed image. 
	// These fluoro image are assumed to be processed.
	if ( _systemType == CommonPlatform::Xray::SystemType::Allura )
	{
		nodeLut.process(*params.perceptionLut,*src,*live);
	}
	else
	{
		basic.copy(*src,*live);
	}
}


bool CrmAlgoFluoro::process(const std::shared_ptr<const XrayImage> &xrayImage, const std::shared_ptr<CrmOverlay> &dst, const std::shared_ptr<XrayImage> &live,bool skip)
{
	if (!xrayImage)							return false;
	if (!dst)								return false;
	if (!live)								return false;
	
	//apply luts to live always, to be able to show the live image only when there is no roadmap
	applyLutsToLive(xrayImage, live);

	if (handle == nullptr)      			return false;
	if (buffer.empty())			     		return false;
	if (error != CrmErrors::Error::OK)		return false;
	if (!roadmap)							return false;
	
	if (count == 0)
	{
        m_log.DeveloperInfo(L"Start processing of fluoro run. [SeriesNumber=%d]", xrayImage->seriesNumber);
		if (!CrmErrors::mapOscErrorToCrmError(OscSetFluoroSequenceParameters(*static_cast<COscHdl*>(handle), roadmap->width, roadmap->height, getFramesPerSecond(xrayImage), &threadPool), error, m_log)) return false;
	}

	Matrix trans = getTransformationMatrix(xrayImage);

	//apply transform to fluo image in src and return in srcInRoadmapSpace
	Simplicity::OmpImage<short> srcInRoadmapSpace; 
	srcInRoadmapSpace.resize(roadmap->width, roadmap->height);
	
	if (!skip) // For performance, only apply transform if fluo image is not skipped
	{
		transform.process(xrayImage->getMeta(), Simplicity::NodeAffineTransform::Interpol::Bilinear, to3d(trans), *xrayImage, srcInRoadmapSpace, transformationBorderValue);

		//because algorithm doesnt support stride, we have to copy the transformed data into a continuous buffer here...
		for (int y = 0; y < roadmap->height; y++)
		{
			memcpy(buffer.data() + y*roadmap->width, srcInRoadmapSpace.pointer + y*srcInRoadmapSpace.stride, srcInRoadmapSpace.width * sizeof(short));
		}
	}

	COscRoiCoord roi;
	roi.XMin = 0;
	roi.YMin = 0;
	roi.XMax = srcInRoadmapSpace.width  - 1;
	roi.YMax = srcInRoadmapSpace.height - 1;

	correctShuttersRoi(xrayImage, trans, roi);
	if (!CrmErrors::mapOscErrorToCrmError(OscSetShuttersRoi(*static_cast<COscHdl*>(handle), roi),error, m_log)) return false;
	
	COscFluoroPictureResults results;
	results.Advanced.Volatile.Ovr = nullptr;

	float mmPerPixel = getMmPerPixelInIsoCenter(xrayImage);

	if (!CrmErrors::mapOscErrorToCrmError(OscSetCurrentGeometryFluoro(*static_cast<COscHdl*>(handle), static_cast<float>(xrayImage->geometry.rotation), static_cast<float>(xrayImage->geometry.angulation), mmPerPixel), error, m_log))
	{
		return false;
	}
	
	if (!CrmErrors::mapOscErrorToCrmError(OscSetTableApparentPositionPix(*static_cast<COscHdl*>(handle), 0, 0), error, m_log))
	{
		return false;
	}

	if (!CrmErrors::mapOscErrorToCrmError(OscFluoroProcessPicture(*static_cast<COscHdl*>(handle), skip ? NULL : buffer.data(), count, skip ? OSC_FLUORO_SKIP_FRAME : OSC_FLUORO_FRAME, &results), error, m_log))
	{
		return false;
	}

	int width  = 0;
	int height = 0;
	int stride = 0;

	unsigned char *overlay = nullptr;
	if (!skip)
	{
		// Do not retrieve an overlay in case of a skipped Fluoro frame, for any value of [count]:
		// - If [count] == 0 the [handle->Pv->ExtractProc->SelectedPhase] field is uninitialized
		// - In case (count > 0) the [handle->Pv->ExtractProc->SelectedPhase] could be initialized (if there has been a non-skipped frame),
		//   but [results] is always uninitialized if [skip] == true -> [dst] cannot be computed
		// In both cases we keep [overlay] == NULL so [dst] is empty.
		// Note that the caller does not use the [dst] output in case [skip] == true
		overlay = OscFluoroGetOverlay(*static_cast<COscHdl*>(handle), &width, &height, &stride);
	}

	if ((overlay != nullptr) && (width > 0) && (height > 0) && (stride > 0))
	{
		dst->resize(width, height);
		dst->frustumToPixels = Matrix::scale(1.0 / static_cast<double>(stride)) * Matrix::translate(Point(results.dXSelectedAngio, results.dYSelectedAngio)) * trans * xrayImage->frustumToPixels;
		dst->shutters = roadmap->overlay->shutters;

		for (int y = 0; y < height; y++)
		{
			memcpy(dst->pointer + y*dst->stride, overlay + y*width, width);
		}
	}
	else
	{
		dst->resize(0, 0);
	}

	count++;
	return true;
}

bool CrmAlgoFluoro::end()
{
    m_log.DeveloperInfo(L"End processing of fluoro run. [Number of processed images=%d]", count);

	count = 0; 
	error = CrmErrors::Error::OK;
	return true;
}

float CrmAlgoFluoro::getFramesPerSecond(const std::shared_ptr<const XrayImage> & src) const
{
	if(src->frameTime > 0)
	{
		return static_cast<float>(1.0/src->frameTime);
	}
	else
	{
		return 0;
	}
}

float CrmAlgoFluoro::getMmPerPixelInIsoCenter(const std::shared_ptr<const XrayImage> & src) const
{
	if(src->geometry.sourceImageDistance > 0)
	{
		return static_cast<float>(src->mmPerPixel.x * src->geometry.sourceObjectDistance / src->geometry.sourceImageDistance);
	}
	else
	{
		return 0;
	}
}

}}