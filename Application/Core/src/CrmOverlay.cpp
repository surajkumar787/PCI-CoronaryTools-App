// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "CrmOverlay.h"

namespace Pci { namespace Core{


CrmOverlay::CrmOverlay()
:
	frustumToPixels(Matrix::identity())
{
}

struct CrmOverlayHeader
{
	//first is an fxd header, so we can open and view the roadmap files
	int		fxd_header;   
	int		fxd_width;
	int		fxd_height;
	int		fxd_count;
	int		fxd_type;
	int		fxd_bitdepth;
	float	fxd_maxValue;
	float	fxd_minValue;
	int		fxd_offset;
	int		fxd_reserved[23];

	//next is the actual header
	static const int Version = 2;

	int		version;
	int		width;
	int		height;
	Matrix	frustumToPixels;
	Rect	shutters;
};


static int Version = 0;

bool CrmOverlay::load(FILE *f)
{
	CrmOverlayHeader header;

	if (fread(&header, sizeof(header), 1, f) != 1)   return false;
	if (header.version != CrmOverlayHeader::Version) return false;
	if (header.width  < 0 || header.width  > 4096)   return false;
	if (header.height < 0 || header.height > 4096)   return false;
	
	frustumToPixels	= header.frustumToPixels;
	shutters		= header.shutters;
	
	resize(header.width, header.height);
	for (int y = 0; y < height; y++)
	{
		if (fread(pointer + y * stride, width * sizeof(unsigned char), 1, f) != 1)
		{
			return false;
		}
	}

	return true;
}
	
bool CrmOverlay::save(FILE *f) const
{
	CrmOverlayHeader header;
	memset(&header, 0, sizeof(header));

	header.fxd_header		= 'fdXd';
	header.fxd_width		= width;
	header.fxd_height		= height;
	header.fxd_count		= 1;
	header.fxd_type			= 2;
	header.fxd_bitdepth		= 0;
	header.fxd_offset		= sizeof(header)-128;

	header.version			= CrmOverlayHeader::Version;
	header.width			= width;
	header.height			= height;
	header.frustumToPixels	= frustumToPixels;
	header.shutters			= shutters;

	if (fwrite(&header, sizeof(header), 1, f) != 1)
	{
		return false;
	}

	for (int y = 0; y < height; y++)
	{
		if (fwrite(pointer + y * stride, width * sizeof(unsigned char), 1, f) != 1)
		{
			return false;
		}
	}

	return true;

}
}}

