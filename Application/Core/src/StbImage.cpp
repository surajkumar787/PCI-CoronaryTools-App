// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "StbImage.h"

namespace Pci{ namespace Core{

StbImage::StbImage()
:
	refMarkers(),
	showWarning(false),
	phase(StbImagePhase::boostPhase)
{
}

StbImage::StbImage(const StbImage &source):
	CommonPlatform::Xray::XrayImage(source)
{
	refMarkers = source.refMarkers;
	showWarning = source.showWarning;
	phase = source.phase;
}

std::shared_ptr<StbImage> StbImage::invert() const
{
	auto inverted = std::make_shared<StbImage>(*this);
	int size = height * stride;
	short max = 0;

	for (int i = 0; i < size; i++)
	{
		short value = pointer[i];
		max = std::max(max, value);
	}

	for (int i = 0; i < size; i++)
	{
		short value = pointer[i];
		value = max - value;
		inverted->pointer[i] = value;
	}

	return inverted;
}

}}