// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "FxdSink.h"

namespace Pci {	namespace StentBoostAlgo {


FxdSink::FxdSink(const std::wstring &filename)
:
	valid	(false),
	count	(0),
	width	(0),
	height	(0),
	file	(nullptr)
{
	if(filename.empty()) return;
	if (_wfopen_s(&file, filename.c_str(), L"wb") != 0) return;
	if (!writeHeader())									return;
	
	valid = true;
}

FxdSink::~FxdSink()
{
	writeHeader();
	if (file != nullptr) fclose(file);
}


void FxdSink::add(Simplicity::Image<short> &image)
{
	if (file == nullptr) return;

	if (width  != 0 && image.width  != width)  return;
	if (height != 0 && image.height != height) return;

	width    = image.width;
	height   = image.height;

	for (int y = 0; y < height; y++)
	{
		if (2 != fwrite(image.pointer + y*image.stride, width, 2, file))	return;
	}

	count++;
}



bool FxdSink::writeHeader()
{
	if (file != nullptr) return false;

	struct
	{
		unsigned id;
		unsigned width;
		unsigned height;
		unsigned count;
		unsigned type;
		unsigned bitdepth;
		float	 maxValue;
		float	 minValue;
		unsigned offset;
		unsigned reserved[23];
	} 
	fxd;

	fxd.id       = 'FDXD';
	fxd.width    = static_cast<unsigned>( width);
	fxd.height   = height;
	fxd.count    = count;
	fxd.type     = 4;
	fxd.bitdepth = 14;
	fxd.maxValue = 16383;
	fxd.minValue = 0;
	fxd.offset   = 0;

	if ( 0 != fseek(file, 0, SEEK_SET) ) return false;
	if ( 1 != fwrite(&fxd, sizeof(fxd), 1, file) ) return false;
	return true;
}
}}