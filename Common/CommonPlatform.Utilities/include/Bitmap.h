// Copyright Koninklijke Philips N.V. 2019
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>

namespace CommonPlatform { namespace Utilities {

	class Bitmap
	{
	public:
		static bool fromRgb32ToBmp24(const std::vector<char>& rgb, std::vector<uint8_t>& bmp, int width, int height, int stride);

	private:
		Bitmap() = delete;
	};

}}