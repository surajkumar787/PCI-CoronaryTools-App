// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <memory>

namespace Sense 
{
	class ITextureOutput;
}

namespace Pci { namespace Core { namespace PACS 
{
	//! The pixel format of the texture
	enum class Format
	{
		Rgb32,	//!< 32 bits-per-pixel RGB format.  Alpha channel is ignored
		Rgba32, //!< 32 bits-per-pixel RGBA format. Alpha channel is used for transparency
	};

	//! The FormatDesc can be used to retreive additional information about a pixel format
	struct FormatDesc
	{
	public:

		//! Constructs a FormatDesc for the given pixel format. 
		FormatDesc(Format format)
			:
			pixelSize(0),
			color(false),
			alpha(false)
		{
			switch (format)
			{
			case Format::Rgb32:		pixelSize = 4;	 color = true;		alpha = false;		break;
			case Format::Rgba32:	pixelSize = 4;	 color = true;		alpha = true;		break;
			}
		}

		int  pixelSize;		//!< The size of each pixel, in bytes
		bool color;			//!< Indicates weather the format contains color or only grayscale information
		bool alpha;			//!< Indicates weather the format contains an alpha channel
	};

	class Snapshot
	{
	public:
		static std::shared_ptr<Snapshot> fromSenseTexture(Sense::ITextureOutput& texture);
		static std::shared_ptr<Snapshot> fromRawData(int width, int height, Format pixelFormat, const char* data, size_t size);

		bool operator==(const Snapshot &other) const;

		~Snapshot();

		int getWidth() const;
		int getHeight() const;
		Format getFormat() const;

		const std::vector<char>& getPixelData() const;

	private:
		Snapshot(int height, int width, Format imageFormat);

		Format m_dataFormat;
		int m_width;
		int m_height;

		std::vector<char> m_pixelBuffer;
	};

}}}