// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbMask.h"


namespace Pci {	namespace Core 	{

const  Simplicity::NodeAffineTransform::Vector NodeAffinedInner(0.3f, 0.1f);
const Simplicity::NodeAffineTransform::Vector NodeAffinedOuter(0.6f, 0.3f);

StbMask::StbMask():
	m_pool(),
	m_mask(),
	m_markers()
{
}


StbMask::~StbMask()
{
}


void StbMask::mask(const StbMarkers& markers, const Simplicity::OmpImage<short>& input, Simplicity::OmpImage<short>& output)
{
	if ( markers.valid)
	{
		if (!isMaskValues(markers)|| !sizeEquals(input))
		{
			createMask(markers, input, NodeAffinedInner, NodeAffinedOuter);
		}

		applyMask(input, output);
	}
	else
	{
		for (int i = 0; i < output.height * output.stride; i++)
		{
			output.pointer[i] = input.pointer[i];
		}
	}
}


void StbMask::createMask(const StbMarkers& markers, const Simplicity::OmpImage<short> &input, const Simplicity::NodeAffineTransform::Vector& inner, const Simplicity::NodeAffineTransform::Vector& outer)
{
	m_mask = std::make_unique< Simplicity::OmpImage<float>>();
	m_mask->resize(input.width, input.height);

	auto ep = (markers.p2 - markers.p1);
	float stentLength = ep.length();

	ep = (1.0f / stentLength) * ep;

	auto cm = 0.5f * (markers.p1 + markers.p2);

	float lmaxP = outer.y * static_cast<float>(input.width);
	float lminP = inner.y * static_cast<float>(input.width);
	float lmaxT = outer.x * static_cast<float>(input.width);
	float lminT = inner.x * static_cast<float>(input.width);

	float invBandWidthP = 1.0f / (lmaxP - lminP);
	float invBamdWidthT = 1.0f / (lmaxT - lminT);


	m_pool.execute([&](Simplicity::Thread& thread)
	{
		m_pool.profileBegin("StbAlgorithm::mask");
		Simplicity::Strip strip = thread.getStrip(*m_mask);
		for (int y = strip.top; y < strip.bottom; y++)
		{
			int offset = y * m_mask->stride;
			for (int x = 0; x < m_mask->width; x++)
			{
				auto diffp = Simplicity::NodeAffineTransform::Vector(static_cast<float>(x), static_cast<float>(y)) - markers.p1;

				// distance perpendicular to the stent
				float dot = diffp.x * ep.x + diffp.y * ep.y;
				float disty = (dot * ep - diffp).length();

				float alpha;

				if (disty > lmaxP)		alpha = 0.0f;
				else if (disty > lminP)	alpha = (lmaxP - disty) * invBandWidthP;
				else 					alpha = 1.0f;

				// distance along the stent

				float distx = (dot * ep + markers.p1 - cm).length();

				if (distx > lmaxT)		alpha = 0.0f;
				else if (distx > lminT)	alpha *= (lmaxT - distx) *invBamdWidthT;
				else					alpha *= 1.0f;

				m_mask->pointer[offset + x] = 1.0f - (1.0f - alpha) * (1.0f - alpha);
			}

		}
		m_pool.profileEnd();
	});
	
	m_markers = markers;
}

bool StbMask::isMaskValues(const StbMarkers &markers) const
{
	return ((m_markers.p1.x == markers.p1.x) &&
			(m_markers.p1.y == markers.p1.y) &&
			(m_markers.p2.x == markers.p2.x) &&
			(m_markers.p2.y == markers.p2.y) &&
			m_mask);
}

bool StbMask::sizeEquals(const Simplicity::OmpImage<short>&  input)
{
    return m_mask && m_mask->width == input.width && m_mask->height == input.width;
}

void StbMask::applyMask(const Simplicity::OmpImage<short>& input, Simplicity::OmpImage<short>& output)
{
	for (int y = 0; y < output.height; y++)
	{
		int offsetOut = y * output.stride;
		int offsetIn = y * input.stride;
		int offsetMask = y * m_mask->stride;
		for (int x = 0; x < output.width; x++)
		{
			float alpha = m_mask->pointer[offsetMask + x];
			output.pointer[offsetOut + x] = short(alpha * float(input.pointer[offsetIn + x]) + 0.5f);
		}
	}
}

}}