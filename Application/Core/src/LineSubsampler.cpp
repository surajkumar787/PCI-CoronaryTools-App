// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "LineSubsampler.h"

using namespace Pci::Core;

LineSubsampler::LineSubsampler(const std::function<void(const TimedPoint& point)> &addPoint) :
	lowestPoint(),
	highestPoint(),
	subsampleAvailable(false),
	started(false),
	addPointCallback(addPoint)
{
}


LineSubsampler::~LineSubsampler(void)
{
}

void LineSubsampler::start()
{
	started = true;
}

void LineSubsampler::stop()
{
	started = false;
}

void LineSubsampler::addPoint(const TimedPoint &timedPoint)
{
	if(started)
	{
		if(!subsampleAvailable || (lowestPoint.point.y > timedPoint.point.y))
		{
			lowestPoint = timedPoint;
		}

		if(!subsampleAvailable || (highestPoint.point.y < timedPoint.point.y))
		{
			highestPoint = timedPoint;
		}

		subsampleAvailable = true;
	}
	else
	{
		if(subsampleAvailable)
		{
			if(lowestPoint.point.x < highestPoint.point.x)
			{
				addPointCallback(lowestPoint);
				addPointCallback(highestPoint);
			}
			else
			{
				addPointCallback(highestPoint);
				addPointCallback(lowestPoint);
			}			
		}

		addPointCallback(timedPoint);

		subsampleAvailable = false;
	}
}

void Pci::Core::LineSubsampler::reset()
{
	subsampleAvailable = false;
	started = false;
}
