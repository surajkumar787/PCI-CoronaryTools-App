// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XraySinkFxd.h"
#include "PathUtil.h"
#include <Shlwapi.h>
#include <stlstring.h>
#include "FxdSourceHeader.h"

namespace CommonPlatform {	namespace Xray {

static const int FxdImageBitDepth = 14;
#pragma pack(1)
struct FxdSinkMeta : FxdSourceMeta
{ 
	char			reserved[4096];
};

#pragma pack()

XraySinkFxd::XraySinkFxd(const std::wstring &Filename)
:
	saveTimingInfo( true),
	file	(nullptr),
	meta	(nullptr),
	width	(0),
	height	(0),
	bitDepth (FxdImageBitDepth),
	count	(0),
	start	(),
	stop	(),
	first	(nullptr)
{
    GetLocalTime(reinterpret_cast<SYSTEMTIME*>(start));
    GetLocalTime(reinterpret_cast<SYSTEMTIME*>(stop));

	wchar_t path[MAX_PATH] = {};
	wcscpy_s(path, CommonPlatform::Utilities::AbsolutePathFromExeDir(Filename).c_str());

	if (_wfopen_s(&file, path, L"wb") != 0) return;

	FxdSourceHeader   header;
	FxdSourceHeaderEx ex;

	memset(&header, 0, sizeof(header));
	memset(&ex,     0, sizeof(ex));

	fwrite(&header, sizeof(header), 1, file);
	fwrite(&ex,     sizeof(ex),     1, file);

	PathRenameExtension(path, L".meta");
	if (_wfopen_s(&meta, path, L"wb") != 0) return;

	int version = 0;
	int stride  = sizeof(FxdSinkMeta);
	
	fwrite(&version, sizeof(version), 1, meta);
	fwrite(&stride,  sizeof(stride),  1, meta);
}


static const float MaxiumPixelValue = 16383.5f;
static const double CmToMm = 10.0;
static const double MeterToMm = 1000.0;
static const double AngelScaling = 100.0;

XraySinkFxd::~XraySinkFxd()
{
	if ((file != nullptr) && (meta != nullptr) && (first != nullptr))
	{
		fseek(file, 0, SEEK_SET);

		FxdSourceHeader header;
		memset(&header, 0, sizeof(header));

		header.id       = 'fdXd';
		header.width    = first->width;
		header.height   = first->height;
		header.bitdepth = first->bitdepth;
		header.count    = count;
		header.type     = 4;
		header.maxValue = MaxiumPixelValue;
		header.minValue = -0.5f;
		header.offset   = sizeof(FxdSourceHeader) + sizeof(FxdSourceHeaderEx) - 128;
	
		fwrite(&header, sizeof(header), 1, file);

		FxdSourceHeaderEx ex;
		memset(&ex, 0, sizeof(ex));
		ex.metaId = 'fxdx';
		
		if ( saveTimingInfo )
		{
			*reinterpret_cast<SYSTEMTIME*>(ex.start) = *reinterpret_cast<SYSTEMTIME*>(start);
			*reinterpret_cast<SYSTEMTIME*>(ex.stop)  = *reinterpret_cast<SYSTEMTIME*>(stop);
		}
		else
		{
			memset( ex.start, 0, sizeof(ex.start));
			memset( ex.stop,  0, sizeof(ex.stop));
		}

		switch(first->type)
		{
			case XrayImage::Type::Fluoro:		strcpy_s(ex.type,     "Fluoro");	 break;
			case XrayImage::Type::Exposure:		strcpy_s(ex.type,     "Cine");		 break;
			case XrayImage::Type::SingleShot:	strcpy_s(ex.type,     "SingleShot"); break;
			case XrayImage::Type::None:			strcpy_s(ex.type,     "None");       break;
		}																			 
																				 
		double mmPerPixelIso = first->mmPerPixel.x * first->geometry.sourceObjectDistance / first->geometry.sourceImageDistance;

        sprintf_s(ex.procName,                  "%S",                       first->epx.activityType.c_str());
		sprintf_s(ex.subImages,					"1");
		sprintf_s(ex.length,					"%d",						count);
		sprintf_s(ex.framespeed,				"%d",						static_cast<int>(1.0/first->frameTime + 0.5));
		sprintf_s(ex.detectorFormat,			"%dx%d",					static_cast<int>(first->geometry.detectorFieldSizeX), static_cast<int>(first->geometry.detectorFieldSizeY));
		sprintf_s(ex.angulation,				"%f",						first->geometry.angulation);
		sprintf_s(ex.rotation,					"%f",						first->geometry.rotation);
		sprintf_s(ex.frontalBeamLongitudinal,	"%f",						first->geometry.beamLongitudinalOffset	* CmToMm);
		sprintf_s(ex.detectorFrontal,			"%f",						first->geometry.sourceImageDistance		* CmToMm);
		sprintf_s(ex.frontalRotateDetector,		"%f",						first->geometry.detectorRotationAngle	* AngelScaling);
		sprintf_s(ex.SID,						"%f",						first->geometry.sourceImageDistance		/ MeterToMm);
		sprintf_s(ex.frontalZRotation,			"%f",						first->geometry.standLArmAngle			* AngelScaling);
		sprintf_s(ex.propellor,					"%f",						first->geometry.standPropellerAngle		* AngelScaling);
		sprintf_s(ex.cArm,						"%f",						first->geometry.standRollAngle			* AngelScaling);
		sprintf_s(ex.tableHeight,				"%f",						first->geometry.tableHeight				* CmToMm);
		sprintf_s(ex.tag,						"detector: %.3f iso: %.3f", first->mmPerPixel.x, mmPerPixelIso);

		ex.validLeft	 = static_cast<short>(first->validRect.left);
		ex.validTop		 = static_cast<short>(first->validRect.top);
		ex.validRight	 = static_cast<short>(first->validRect.right);
		ex.validBottom	 = static_cast<short>(first->validRect.bottom);
		
		ex.shutterLeft	 = static_cast<short>(first->shutters.left);
		ex.shutterTop	 = static_cast<short>(first->shutters.top);
		ex.shutterRight	 = static_cast<short>(first->shutters.right);
		ex.shutterBottom = static_cast<short>(first->shutters.bottom);

		fwrite(&ex, sizeof(ex), 1, file);
		
		fflush(file);
		fflush(meta);
	}

	if (file != nullptr) fclose(file);
	if (meta != nullptr) fclose(meta);
}


void XraySinkFxd::addImage(const std::shared_ptr<const XrayImage> &image)
{
	if ((file == nullptr) || (meta == nullptr) || (image == nullptr)) return;

	if (count == 0)
	{
		width	= image->width;
		height	= image->height;
	
	    GetLocalTime( reinterpret_cast<SYSTEMTIME*>(start));
		first = image;
	}

    if (image->width  != width)  return;
    if (image->height != height) return;
		
	GetLocalTime(reinterpret_cast<SYSTEMTIME*>(stop));

	for (int y = 0; y < height; y++)
	{
		fwrite(image->pointer + y*image->stride, width * sizeof(short), 1, file);
	}

	FxdSinkMeta data;
	memset(&data, 0, sizeof(data));

	data.runIndex		 = image->runIndex;
	data.imageIndex		 = image->imageIndex;
	data.type			 = image->type;
	data.validRect		 = image->validRect;
	data.shutters		 = image->shutters;
	data.frustumToPixels = image->frustumToPixels;
	data.mmPerPixel		 = image->mmPerPixel;
	data.frameTime		 = image->frameTime;
	data.geometry		 = image->geometry;
//	data.epx			 = image->epx;
	
	fwrite(&data, sizeof(data), 1, meta);

	count++;
}

}}