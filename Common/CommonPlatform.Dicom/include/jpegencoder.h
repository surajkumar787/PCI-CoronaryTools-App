// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <cstdint>
#include <vector>

namespace CommonPlatform { namespace Dicom
{

	std::vector<char> encodeJpeg(const char* pixels,
	                             int         width,
								 int         height,
								 int         bitsAllocated,
								 int         highBit,
								 int         samplesPerPixel,
	                             bool        lossless);
}}