// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "CrmRoadmap.h"
#include "CrmErrors.h"
#include <Windows.h>
#include <Shlwapi.h>

namespace Pci { namespace Core{

CrmRoadmap::CrmRoadmap()
:
	width			(0),
	height			(0),
	runIndex		(0),
	seriesNumber	(0),
	selected		(false),
	locked			(false),
	mmPerPixel		(Point()),
	frustumToPixels	(Matrix::identity()),
 	geometry		(XrayGeometry()),
	overlay			(nullptr),
	data			(),
    quality			(0.0),
	warning			(PciSuite::CrmErrors::Error::OK)
{
}

CrmRoadmap::CrmRoadmap(unsigned runIndex, int seriesNumber, int width, int height, const Point &mmPerPixel, const XrayGeometry &geo, const Matrix &frustumToPixels, double quality)
:
	width			(width),
	height			(height),
	runIndex		(runIndex),
	seriesNumber	(seriesNumber),
	selected		(false),
	locked			(false),
	mmPerPixel		(mmPerPixel), 
    frustumToPixels	(frustumToPixels),
	geometry		(geo),
	overlay(nullptr),
	data(),
	quality         (quality),
	warning         (PciSuite::CrmErrors::Error::OK)
{
}
	
double CrmRoadmap::getAngleDistance(const XrayGeometry &geo) const
{
	double dx = geometry.angulation - geo.angulation;
	double dy = geometry.rotation   - geo.rotation;
	
	return sqrt((dx*dx)+(dy*dy));
}

double CrmRoadmap::getPanningDistance(const XrayGeometry &geo) const
{
	//calculate the center of the roadmap within the image space of the new geometry.
	Vector3d from = geo.image.fromRoot * geo.patient.toRoot * geometry.patient.fromRoot * geometry.iso.toRoot * Vector3d();
	Vector3d to   = geo.image.fromRoot * geo.patient.toRoot * geo     .patient.fromRoot *      geo.iso.toRoot * Vector3d();
	
	//ignore movements in the z-plane; we can compensate for that!
	Vector3d dist = to-from;
	dist.z = 0;

	return dist.length();
}


struct CrmRoadmapHeader
{
	static const int Version = 17;

	int					version;
	int					width;
	int					height;
	Matrix				frustumToPixels;
	XrayGeometry		geometry;
	int					runIndex;
	Point				mmPerPixel;
	int					size;
    double				quality;
	bool				selected;
	bool				locked;
	int					seriesNumber;
};


bool CrmRoadmap::load(FILE *f)
{
	CrmRoadmapHeader header;

	std::shared_ptr<CrmOverlay> crmOverlay = std::make_shared<CrmOverlay>();
	if (!crmOverlay->load(f)) return false;
	this->overlay = crmOverlay;

	if (fread(&header, sizeof(header), 1, f) != 1)   return false;
	if (header.version != CrmRoadmapHeader::Version) return false;
	if (header.width  < 0 || header.width  > 4096)   return false;
	if (header.height < 0 || header.height > 4096)   return false;

	width			= header.width;
	height			= header.height;
	frustumToPixels	= header.frustumToPixels;
	geometry		= header.geometry;
	runIndex		= header.runIndex;
	mmPerPixel		= header.mmPerPixel;
    quality			= header.quality;
	selected		= header.selected;
	locked			= header.locked;
	seriesNumber	= header.seriesNumber;

	data.resize(header.size);
	if (fread(data.data(), header.size, 1, f) != 1) return false;

	return true;
}
	
bool CrmRoadmap::save(FILE *f) const
{
	if (!overlay || !overlay->save(f)) return false;
	
	CrmRoadmapHeader header;

	header.version			= CrmRoadmapHeader::Version;
	header.width			= width;
	header.height			= height;
	header.frustumToPixels	= frustumToPixels;
	header.geometry			= geometry;
	header.runIndex			= runIndex;
	header.mmPerPixel		= mmPerPixel;
    header.quality			= quality;
	header.size				= static_cast<int>(data.size());
	header.selected			= selected;
	header.locked			= locked;
	header.seriesNumber		= seriesNumber;

	if (fwrite(&header, sizeof(header),  1, f) != 1) return false;
	if (fwrite(data.data(),	header.size, 1, f) != 1) return false;

	return true;
}
}}
