// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <complex>
#include "XraySourceFxd.h"
#include "stlstring.h"
#include "PathUtil.h"
#include <Shlwapi.h>
#include "FxdSourceHeader.h"

namespace CommonPlatform {	namespace Xray {

static const int FxdImageBitDepth = 14;
static const double CmToMm = 10.0;
static const double AngelScaling = 100.0;
static const double DefaultFrameTime = 15.0;
static const double DefaultPixelSize = 0.184;

static double atod_s(const char *value);
static int    atoi_s(const char *value);

static void parse(const char *target, const char *label, const char *value, std::string &result)	{ if (target != nullptr && strcmp(target, label) != 0) return;	result = value;									}
static void parse(const char *target, const char *label, const char *value, bool		&result)	{ if (target != nullptr && strcmp(target, label) != 0) return;	result = (strstr(value, "True") != nullptr);	}
static void parse(const char *target, const char *label, const char *value, int			&result)	{ if (target != nullptr && strcmp(target, label) != 0) return;	result = atoi_s(value);							}
static void parse(const char *target, const char *label, const char *value, double		&result)    { if (target != nullptr && strcmp(target, label) != 0) return;	result = atod_s(value);							}

static char *trim(char *str)
{
	char *head = str;
	char *tail = str+strlen(str)-1;
	
	while ((*head == ' ' || *head == '\n' || *head == '=' || *head == '\t') && head < tail)
	{
		head++;
	}

	while ((*tail == ' ' || *tail == '\n' || *tail == '=' || *tail == '\t') && tail > head)
	{
		tail--;
	}

	tail[1] = 0;
	return head;
}


static double atod_s(const char *value)
{
	//we perform string-to-float manually, to be indifferent to comma or periods
	const char *ptr = value + strspn(value, " \t\n");
	
	int hi   = 0;
	int lo   = 0;
	int dec  = 1;
	int sign = 1;

	if (*ptr == '-') { sign = -1; ptr++; }

	while(ptr && *ptr >= '0' && *ptr <= '9') { hi = hi * 10 + (*ptr - '0');	ptr++;									} 
	if (*ptr == '.' || *ptr == ',') ptr++;
	while(ptr && *ptr >= '0' && *ptr <= '9' && dec < 1000000000) { lo = lo * 10 + (*ptr - '0'); ptr++; dec *= 10;	}

	double x = sign * (hi + double(lo) / double(dec));
	return x;
}

static int atoi_s(const char *value)
{
	int result = 0;
	int sign   = 1;

	const char *ptr = value + strspn(value, " \t\n");
	if (*ptr == '-')	{ sign = -1; ptr++; }
	while(ptr && *ptr >= '0' && *ptr <= '9') { result = result * 10 + (*ptr - '0');	ptr++;	} 

	return sign * result;
}


XraySourceFxd::XraySourceFxd(const std::wstring &Filename)
:
	hfile		(nullptr),
	hmeta		(nullptr),
	offset		(0),
	valid		(false),
	width		(0),
	height		(0),
	bitdepth    (FxdImageBitDepth),
	count		(0),
	metaStride	(0),
	type		(XrayImage::Type::Fluoro),
    epx         (),
	frameTime	(0.0),
	mmPerPixel	(0.0)
{
	wchar_t path[MAX_PATH] = {};
	wcscpy_s(path, CommonPlatform::Utilities::AbsolutePathFromExeDir(Filename).c_str());

	if (_wfopen_s(&hfile, path, L"rb" ) != 0) return;
	
	FxdSourceHeader header;
	if (!fread(&header, sizeof(header), 1, hfile))	return;
	if (header.id != 'fdXd' && header.id != 'FDXD')	return;

	width		= header.width;
	height		= header.height;
	count		= header.count;
	offset		= header.offset;

	if (header.bitdepth <= 0xFF)
	{
		bitdepth = header.bitdepth;
	}
	else
	{
		bitdepth = (int)(std::log(header.bitdepth) / std::log(2));
	}
	
	 
	frameTime	= 1.0/ DefaultFrameTime;
	mmPerPixel  = DefaultPixelSize;
	shutters	= Rect(0,0,header.width, header.height);
	validRect	= Rect(0,0,header.width, header.height);
	
	if (wcsstr(path, L"fluoro")   != nullptr || wcsstr(path, L"Fluoro")   != nullptr) type = XrayImage::Type::Fluoro;
	if (wcsstr(path, L"expo")     != nullptr || wcsstr(path, L"Expo")     != nullptr) type = XrayImage::Type::Exposure;
	if (wcsstr(path, L"angio")    != nullptr || wcsstr(path, L"Angio")    != nullptr) type = XrayImage::Type::Exposure;
	if (wcsstr(path, L"exposure") != nullptr || wcsstr(path, L"Exposure") != nullptr) type = XrayImage::Type::Exposure;

	for (int i = 0 ; i < count; i++)
	{
		XrayGeometry geo;

		geo.detectorFieldSizeX = validRect.width()  * mmPerPixel.x;
		geo.detectorFieldSizeY = validRect.height() * mmPerPixel.y;

		geometry.push_back(geo);
	}

	//try to get extended metadata...
	readExtended();
	readText(path);
	readMeta(path);

	valid = (count > 0);
}

void XraySourceFxd::readExtended()
{
	//try to read FXD extended header
	FxdSourceHeaderEx meta;
	if (fread(&meta, sizeof(meta), 1, hfile) == 0) return;
	if (meta.metaId != 'fxdx') return;
		
	//get shutters and validrect
	if (meta.shutterLeft != 0 || meta.shutterTop != 0 || meta.shutterRight != 0 || meta.shutterBottom != 0)	shutters  = Rect(meta.shutterLeft, meta.shutterTop, meta.shutterRight, meta.shutterBottom);
	if (meta.validLeft   != 0 || meta.validTop   != 0 || meta.validRight   != 0 || meta.validBottom   != 0)	validRect = Rect(meta  .validLeft, meta  .validTop, meta  .validRight, meta  .validBottom);

	int w = 0;
	int h = 0;

	//get pixel size
	if (sscanf_s(meta.detectorFormat, "%dx%d", &w,&h) == 2 || sscanf_s(meta.detectorFormat, "%d x %d", &w,&h) == 2)
	{
		if (w != 0 && validRect.right  > validRect.left) mmPerPixel.x = w/ static_cast<double>(validRect.right  - validRect.left);
		if (h != 0 && validRect.bottom > validRect.top)  mmPerPixel.y = h/ static_cast<double>(validRect.bottom - validRect.top);
	}

	//get image time
	int fps = atoi(meta.framespeed);
	if (fps != 0) frameTime = 1.0/ static_cast<double>(fps);

    // check acquisition type
	if (strstr(meta.type, "Fluoro")     != nullptr)	type = XrayImage::Type::Fluoro;
	if (strstr(meta.type, "Cine")       != nullptr)	type = XrayImage::Type::Exposure;
	if (strstr(meta.type, "SingleShot") != nullptr)	type = XrayImage::Type::SingleShot;

    // get activity type
    epx.activityType = str_ext::stow(meta.procName);

    //BELOW CODE WAS ADDED IN PROTO, added for future purposes
    /*wchar_t str[512];
    swprintf_s(str, L"%S", meta.appName);  epx.application = str;
    swprintf_s(str, L"%S", meta.procName); epx.procedure   = str;*/

	for (int i = 0 ; i < count; i++)
	{
		if (w != 0) geometry[i].detectorFieldSizeX	= static_cast<double>(w);
		if (h != 0) geometry[i].detectorFieldSizeY	= static_cast<double>(h);
		geometry[i].rotation						= atod_s(meta.rotation);
		geometry[i].angulation						= atod_s(meta.angulation);
		geometry[i].beamLongitudinalOffset			= atod_s(meta.frontalBeamLongitudinal)	/ CmToMm;
		geometry[i].sourceImageDistance				= atod_s(meta.detectorFrontal)			/ CmToMm;
		geometry[i].detectorRotationAngle			= atod_s(meta.frontalRotateDetector)	/ AngelScaling;
		geometry[i].standLArmAngle					= atod_s(meta.frontalZRotation)			/ AngelScaling;
		geometry[i].standPropellerAngle				= atod_s(meta.propellor)				/ AngelScaling;
		geometry[i].standRollAngle					= atod_s(meta.cArm)						/ AngelScaling;
		geometry[i].tableHeight						= atod_s(meta.tableHeight)				/ CmToMm;
			
		/*
		//the following parameters exist in RADAR but are not filled in by excelerate yet...
		geometry[i].tableLateralOffset		= atof_s(meta.tablePosLateral)			/ CmToMm;
		geometry[i].tableLongitudinalOffset	= atof_s(meta.tablePosLongitudinal)		/ CmToMm;
		geometry[i].beamLateralOffset		= atof_s(meta.frontalBeamTransversal)	/ CmToMm;*/

		geometry[i].updateSpaces();
	}
}


void XraySourceFxd::readMeta(const wchar_t *file)
{
	wchar_t path[MAX_PATH];
	wcscpy_s(path, file);
	PathRenameExtension(path, L".meta");

	if (_wfopen_s(&hmeta, path, L"rb") != 0) return;

	int version = 0;
	
	int stride = 0;

	if (fread(&version, sizeof(version), 1, hmeta) != 1) return;
	if (fread(&stride,  sizeof(stride),  1, hmeta) != 1) return;
	
	if (version != 0)					return;
	if (stride < sizeof(FxdSourceMeta)) return;

	metaStride = stride;
}

bool isNotZero(double value)
{
	return (std::abs(value) > std::numeric_limits<float>::epsilon());
}

void XraySourceFxd::readText(const wchar_t *file)
{
	wchar_t path[MAX_PATH];
	wcscpy_s(path, file);
	PathRenameExtension(path, L".txt");

	FILE *f = nullptr;

	//try to read metadata from extra text file
	if (_wfopen_s(&f, path, L"r") != 0) return;

	static const int maxLineSize = 4096;
	static char data[maxLineSize];

	std::string	patientID								  = "";
	int			runID									  = 0;
	std::string	EPXprocedure							  = "";
	bool		exposureactive							  = false;
	bool		fluoactive								  = false;
	double		imagetime								  = 0.0;
	int			correctedshuttervalues					  = 0;
	int			seriesnumber							  = 0;
	int			correspondingRoadmapnumber				  = 0;
	bool		readyforroadmap							  = false;
	int			frontalTechnicalReceptorFieldSizeX		  = 0;
	int			frontalImageInfoBitsStored				  = 0;
	int			frontalImageHighBit						  = 0;
	int			frontalImageRows						  = 0;
	int			frontalImageColumns						  = 0;
	double		frontalImagerPixelSpacingX				  = 0.0;
	double		frontalImagerPixelSpacingY				  = 0.0;
	int			frontalImagerValidRectangleBottomRightX	  = 0;
	int			frontalImagerValidRectangleBottomRightY	  = 0;
	int			frontalImagerValidRectangleTopLeftX		  = 0;
	int			frontalImagerValidRectangleTopLeftY		  = 0;
	int			frontalImagerShutterRectangleBottomRightX = 0;
	int			frontalImagerShutterRectangleBottomRightY = 0;
	int			frontalImagerShutterRectangleTopLeftX	  = 0;
	int			frontalImagerShutterRectangleTopLeftY	  = 0;
	int			activeImageRows							  = 0;
	int			activeImageColumns						  = 0;
	std::string	frontalImagerFOV						  = "";
	int			frontalImagerSOD						  = 0;
	int			beamIsoCenterHeight						  = 0;

	while (fgets(data, maxLineSize, f) != NULL)
	{
		if (strstr(data, "END_HEADER") != 0) break;
		if (data[0] == '%' || data[0] == 0) continue;

		char *label = data;
		char *value = strchr(data, '=');
		
		if (value == nullptr) continue;
		value[0] = 0; value++;

		//remove spaces
		value = trim(value);
		label = trim(label);

		parse("Patient ID",										label, value, patientID);
		parse("Run ID",											label, value, runID);
		parse("EPX procedure",									label, value, EPXprocedure);
		parse("Exposure active",								label, value, exposureactive);
		parse("Fluo active",									label, value, fluoactive);
		parse("Image time",										label, value, imagetime);
		parse("Corrected shutter values",						label, value, correctedshuttervalues);
		parse("Series number",									label, value, seriesnumber);
		parse("Corresponding Roadmap number",					label, value, correspondingRoadmapnumber);
		parse("Ready for roadmap (no overlay mismatch)",		label, value, readyforroadmap);
		parse("Frontal Technical Receptor FieldSize X",			label, value, frontalTechnicalReceptorFieldSizeX);
		parse("Frontal Image Info Bits Stored",					label, value, frontalImageInfoBitsStored);
		parse("Frontal Image High Bit",							label, value, frontalImageHighBit);
		parse("Frontal Image Rows",								label, value, frontalImageRows);
		parse("Frontal Image Columns",							label, value, frontalImageColumns);
		parse("Frontal Imager Pixel SpacingX",					label, value, frontalImagerPixelSpacingX);
		parse("Frontal Imager Pixel SpacingY",					label, value, frontalImagerPixelSpacingY);
		parse("Frontal Imager ValidRectangleBottomRightX",		label, value, frontalImagerValidRectangleBottomRightX);
		parse("Frontal Imager ValidRectangleBottomRightY",		label, value, frontalImagerValidRectangleBottomRightY);
		parse("Frontal Imager ValidRectangleTopLeftX",			label, value, frontalImagerValidRectangleTopLeftX);
		parse("Frontal Imager ValidRectangleTopLeftY",			label, value, frontalImagerValidRectangleTopLeftY);
		parse("Frontal Imager ShutterRectangleBottomRightX",	label, value, frontalImagerShutterRectangleBottomRightX);
		parse("Frontal Imager ShutterRectangleBottomRightY",	label, value, frontalImagerShutterRectangleBottomRightY);
		parse("Frontal Imager ShutterRectangleTopLeftX",		label, value, frontalImagerShutterRectangleTopLeftX);
		parse("Frontal Imager ShutterRectangleTopLeftY",		label, value, frontalImagerShutterRectangleTopLeftY);
		parse("Active Image Rows",								label, value, activeImageRows);
		parse("Active Image Columns",							label, value, activeImageColumns);
		parse("Frontal Imager FOV",								label, value, frontalImagerFOV);
		parse("Frontal Imager SOD",								label, value, frontalImagerSOD);
		parse("Beam IsoCenter Height",							label, value, beamIsoCenterHeight);
	}

	//get shutters and validrect
	if (frontalImagerValidRectangleTopLeftX   != 0 || frontalImagerValidRectangleTopLeftY   != 0 || frontalImagerValidRectangleBottomRightX    != 0 || frontalImagerValidRectangleBottomRightY   != 0)
	if (frontalImagerShutterRectangleTopLeftX != 0 || frontalImagerShutterRectangleTopLeftY != 0 || frontalImagerShutterRectangleBottomRightX  != 0 || frontalImagerShutterRectangleBottomRightY != 0) 
	{
		shutters  = Rect(frontalImagerShutterRectangleTopLeftX, frontalImagerShutterRectangleTopLeftY, frontalImagerShutterRectangleBottomRightX,  frontalImagerShutterRectangleBottomRightY);
		validRect = Rect(frontalImagerValidRectangleTopLeftX,   frontalImagerValidRectangleTopLeftY,   frontalImagerValidRectangleBottomRightX,    frontalImagerValidRectangleBottomRightY);

		shutters  -= Point(validRect.left, validRect.top);
		validRect -= Point(validRect.left, validRect.top);
	}

	if ( isNotZero(frontalImagerPixelSpacingX)) mmPerPixel.x = frontalImagerPixelSpacingX;
	if ( isNotZero(frontalImagerPixelSpacingY)) mmPerPixel.y = frontalImagerPixelSpacingY;
	if (frontalImageInfoBitsStored != 0)		bitdepth = frontalImageInfoBitsStored;

	if (fluoactive)		type = XrayImage::Type::Fluoro;
	if (exposureactive) type = XrayImage::Type::Exposure;

	for (int i = 0 ; i < count; i++)
	{
		if (fgets(data, maxLineSize, f) == NULL) break;
		if (data[0] == '%' || data[0] == 'T' || data[0] == 0) { i--; continue; };

		char *context = nullptr;
		char *token   = nullptr;
		
		std::string	time			= "";	token = strtok_s(data,    " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, time);
		int			image			= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, image);
		double		rotation		= 0.0f;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, rotation);
		double		angulation		= 0.0f;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, angulation);
		double		tableX			= 0.0f;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, tableX);
		double		tableY			= 0.0f;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, tableY);
		double		tableZ			= 0.0f;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, tableZ);
		int			SID				= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, SID);
		int			FOV				= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, FOV);
		int			shutterLeft		= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, shutterLeft);
		int			shutterTop		= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, shutterTop);
		int			shutterRight	= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, shutterRight);
		int			shutterBot		= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, shutterBot);
		int			oscarX			= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, oscarX);
		int			oscarY			= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, oscarY);
		int			oscarCorIndex	= 0;	token = strtok_s(nullptr, " ,\t", &context);	if (token == nullptr) continue; parse(nullptr, nullptr, token, oscarCorIndex);

		if (isNotZero(angulation)					)	geometry[i].angulation	 			= angulation;
		if (isNotZero(rotation	)					)	geometry[i].rotation		 		= rotation;
		if (SID									!= 0)	geometry[i].sourceImageDistance		= static_cast<double>(SID);
		if (frontalImagerSOD					!= 0)	geometry[i].sourceObjectDistance	= static_cast<double>(frontalImagerSOD);
		if (isNotZero(tableX)						)	geometry[i].tableLateralOffset		= tableX;
		if (isNotZero(tableY)						)	geometry[i].tableLongitudinalOffset	= tableY;
		if (isNotZero(tableZ)						)	geometry[i].tableHeight				= tableZ;
		if (frontalTechnicalReceptorFieldSizeX	!= 0)	geometry[i].detectorFieldSizeX		= static_cast<double>(frontalTechnicalReceptorFieldSizeX);
		if (frontalTechnicalReceptorFieldSizeX	!= 0)	geometry[i].detectorFieldSizeY		= static_cast<double>(frontalTechnicalReceptorFieldSizeX);

		if (shutterLeft != 0 || shutterTop != 0 || shutterRight != 0 || shutterBot != 0)  shutters = validRect - Margin(shutterLeft, shutterTop, shutterRight, shutterBot);
		geometry[i].updateSpaces();
	}

	fclose(f);
}


XraySourceFxd::~XraySourceFxd()
{
	if (hfile != nullptr) fclose(hfile);
	if (hmeta != nullptr) fclose(hmeta);
}

int XraySourceFxd::getImageCount() const
{
	return valid ? count : 0;
}

std::shared_ptr<XrayImage> XraySourceFxd::getImage(int index)
{
	if((hfile == nullptr)|| (!valid)|| (index < 0)||(index >= count) ) return nullptr;
	
	_fseeki64(hfile, __int64(offset) + __int64(128) + __int64(index) * width * height * sizeof(short),  SEEK_SET);

	std::shared_ptr<XrayImage> image = std::make_shared<XrayImage>();

	image->resize(width, height);
	for (int y = 0; y < height; y++)
	{
		if (fread(image->pointer + y*image->stride, width * sizeof(short), 1, hfile) == 0)
		{
			return nullptr;
		}
	}

	image->bitdepth			= bitdepth;
    image->frameTime	    = frameTime;
    image->validRect	    = validRect;
    image->shutters		    = shutters & validRect;
    image->mmPerPixel       = mmPerPixel;
    image->type			    = type;
    image->geometry		    = geometry[index];
    image->imageIndex       = index;
    image->runIndex         = 0;
    image->epx.activityType = epx.activityType;

	image->frustumToPixels = Matrix::scale(Point(1.0, -1.0));
	image->frustumToPixels = Matrix::scale(image->mmPerPixel.x > 0 && image->mmPerPixel.y > 0 ? Point(1.0) / image->mmPerPixel : Point(0.0)) * image->frustumToPixels;
	image->frustumToPixels = Matrix::translate(Point(image->width, image->height) / 2.0) * image->frustumToPixels;

	FxdSourceMeta data;
	
	// If extra metadata is available then read it.
	if (hmeta != nullptr && metaStride > 0)
	{
		if (fseek(hmeta, long(8 + index * metaStride), SEEK_SET) == 0)
		{
			if (fread(&data, sizeof(data), 1, hmeta) == 1)
			{
				//image->runIndex		= data.runIndex;	//we ignore the run index
				image->imageIndex = data.imageIndex;
				image->type = data.type;
				image->validRect = data.validRect;
				image->shutters = data.shutters;
				image->frustumToPixels = data.frustumToPixels;
				image->mmPerPixel = data.mmPerPixel;
				image->frameTime = data.frameTime;
				image->geometry = data.geometry;
				image->geometry.patientNoseOrientation = XrayGeometry::NoseOrientation::NoseUp;
				image->geometry.updateSpaces();
				// use from header, not from meta data file	image->epx				= data.epx;
			}
		}
	}
	return image;
}
}}

