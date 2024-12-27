// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <stdio.h>
#include "IXraySink.h"

namespace CommonPlatform {	namespace Xray {

class XraySinkFxd : public IXraySink
{
public:
	XraySinkFxd(const std::wstring &filename);
	~XraySinkFxd();

	void addImage(const std::shared_ptr<const XrayImage> &image);
	bool    saveTimingInfo;

	XraySinkFxd(const XraySinkFxd &) = delete;
	XraySinkFxd &operator=(const XraySinkFxd &) = delete;

private:
	FILE	*file;
	FILE	*meta;
	int		width;
	int		height;
	int		bitDepth;
	int		count;
	short   start[8];
	short   stop[8];

	std::shared_ptr<const XrayImage> first;
};

}}
