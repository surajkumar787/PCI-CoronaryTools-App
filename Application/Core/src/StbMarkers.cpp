// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbMarkers.h"

namespace Pci{ namespace Core{


StbMarkers::StbMarkers()
:
	StentMarkers()
{
}

StbMarkers::StbMarkers(const StentMarkers &markers) :
	StentMarkers(markers)
{
}

struct StbMarkersData
{
	static const int Version = 1;

	int   id;
	int   valid;
	float confidence;
	float p1x;
	float p1y;
	float p2x;
	float p2y;
};


bool StbMarkers::load(FILE *f)
{
	StbMarkersData data;

	if (fread(&data, sizeof(data), 1, f) != 1)		return false;
	if (data.id != StbMarkersData::Version)			return false;

	valid		= data.valid == 1;
	confidence	= data.confidence;
	p1.x		= data.p1x;
	p1.y		= data.p1y;
	p2.x		= data.p2x;
	p2.y		= data.p2y;
	
	return true;
}
	
bool StbMarkers::save(FILE *f) const
{
	StbMarkersData data;

	data.id			= StbMarkersData::Version;
	data.valid		= valid ? 1 : 0;
	data.confidence = confidence; 
	data.p1x        = p1.x;
	data.p1y        = p1.y;
	data.p2x        = p2.x;
	data.p2y        = p2.y;
	
	if (fwrite(&data, sizeof(data), 1, f) != 1) return false;
	return true;
}
}}