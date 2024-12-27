// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Snapshot.h"

#include <map>
#include <Sense/Graphics/ITexture.h>

namespace Pci { namespace Core { namespace PACS
{

	std::shared_ptr<Snapshot> Snapshot::fromSenseTexture(Sense::ITextureOutput& texture)
	{
		const std::map<Sense::ITexture::Format, Format> formatConversionTable =
		{
			{ Sense::ITexture::Format::Rgb32, Format::Rgb32 },
			{ Sense::ITexture::Format::Rgba32, Format::Rgba32 }
		};

		auto pixelFormat = formatConversionTable.find(texture.getFormat());
		if (pixelFormat == formatConversionTable.end())
		{
			return std::shared_ptr<Snapshot>();
		}

		auto snapShot = std::shared_ptr<Snapshot>(new Snapshot(texture.getHeight(), texture.getWidth(), pixelFormat->second));
		auto* data = snapShot->m_pixelBuffer.data();
		Sense::ITexture::Lock lock(texture.getFormat(), data, snapShot->getWidth(), snapShot->getHeight());

		texture.get(lock);

		return snapShot;
	}

	std::shared_ptr<Snapshot> Snapshot::fromRawData(int width, int height, Format pixelFormat, const char* data, size_t size)
	{
		std::shared_ptr<Snapshot> snapshot(nullptr);

		if ((width != 0) && (height != 0) && (data != nullptr))
		{
			snapshot = std::shared_ptr<Snapshot>(new Snapshot(height, width, pixelFormat));

			if (snapshot->getPixelData().size() == size)
			{
				memcpy(snapshot->m_pixelBuffer.data(), data, size);
			}
			else
			{
				snapshot.reset();
			}
		}

		return snapshot;
	}

	Snapshot::Snapshot(int height, int width, Format imageFormat) :
		m_height(height),
		m_width(width),
		m_dataFormat(imageFormat)
	{
		m_pixelBuffer.resize(height * width * FormatDesc(m_dataFormat).pixelSize);
	}

	Snapshot::~Snapshot()
	{
	}

	bool Snapshot::operator==(const Snapshot &other) const
	{
		bool equal(true);

		equal &= (other.getWidth() == getWidth());
		equal &= (other.getHeight() == getHeight());
		equal &= (other.getFormat() == getFormat());
		equal &= (other.getPixelData() == getPixelData());

		return equal;
	}

	int Snapshot::getWidth() const
	{
		return m_width;
	}

	int Snapshot::getHeight() const 
	{
		return m_height;
	}

	Format Snapshot::getFormat() const
	{
		return m_dataFormat;
	}

	const std::vector<char>& Snapshot::getPixelData() const
	{
		return m_pixelBuffer;
	}

}}}