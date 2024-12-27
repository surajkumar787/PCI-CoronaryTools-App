// Copyright Koninklijke Philips N.V. 2019
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Bitmap.h"

#include <windows.h>

namespace CommonPlatform { namespace Utilities {

	bool Bitmap::fromRgb32ToBmp24(const std::vector<char>& rgb, std::vector<uint8_t>& bmp, int width, int height, int stride)
	{
		const int PIXEL_SIZE_RGB32(4);
		const int PIXEL_SIZE_BMP24(3);

		if (stride <= 0) stride = width;

		if ((width > 0) && (height > 0) && (rgb.size() == (width * height * PIXEL_SIZE_RGB32)))
		{
			int width_in_bytes_24 = ((width * 24 + 31) / 32) * 4;
			DWORD size = width_in_bytes_24 * height;

			bmp.resize(
				sizeof(BITMAPFILEHEADER) +
				sizeof(BITMAPINFOHEADER) +
				(sizeof(RGBQUAD) * 256) +
				size);

			BITMAPFILEHEADER * const header = reinterpret_cast<BITMAPFILEHEADER *>(&(bmp[0]));
			memset(header, 0, sizeof(*header));
			header->bfType = 0x4D42;
			header->bfOffBits = DWORD(bmp.size() - size);
			header->bfSize = DWORD(bmp.size());

			BITMAPINFOHEADER * const info = reinterpret_cast<BITMAPINFOHEADER *>(header + 1);
			memset(info, 0, sizeof(*info));
			info->biSize = sizeof(*info);
			info->biWidth = width;
			info->biHeight = height;
			info->biPlanes = 1;
			info->biBitCount = 24;
			info->biCompression = BI_RGB;
			info->biSizeImage = 0;
			info->biClrUsed = 0;
			info->biClrImportant = 0;

			uint8_t* dstPtr = (bmp.data() + header->bfOffBits);

			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					int i = (height - h - 1) * (width * PIXEL_SIZE_RGB32) + (w * PIXEL_SIZE_RGB32);
					int j = h * width_in_bytes_24 + w * PIXEL_SIZE_BMP24;
					dstPtr[j + 0] = rgb[i + 0];
					dstPtr[j + 1] = rgb[i + 1];
					dstPtr[j + 2] = rgb[i + 2];
				}
			}

			return true;
		}

		return false;
	}
}}