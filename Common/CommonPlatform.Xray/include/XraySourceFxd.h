// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <stdio.h>
#include "IXraySource.h"

namespace CommonPlatform {	namespace Xray {

class XraySourceFxd : public IXraySource
{
public:
	XraySourceFxd(const std::wstring &filename);
	~XraySourceFxd();

	int getImageCount() const override;
	std::shared_ptr<XrayImage> getImage(int index) override;
	

	XraySourceFxd(const XraySourceFxd &) = delete ;
	XraySourceFxd &operator=(const XraySourceFxd &) = delete;

private:

	void readExtended();
	void readText(const wchar_t *file);
	void readMeta(const wchar_t *file);
	
	FILE			*hfile;
	FILE			*hmeta;
	__int64			offset;
	bool			valid;

	int				width;
	int				height;
	int				bitdepth;
	int				metaStride;
	int				count;
	XrayImage::Type	type;
    XrayEpx         epx;
	Rect			shutters;
	Rect			validRect;
	double			frameTime;
	Point			mmPerPixel;

	std::vector<XrayGeometry> geometry;

};

}}