// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "TimedPoint.h"
#include <functional>

namespace Pci { namespace Core {

	class LineSubsampler
	{
	public:
		explicit LineSubsampler(const std::function<void(const TimedPoint& point)> &addPoint);
		virtual ~LineSubsampler();

		void start();
		void stop();
		void addPoint(const TimedPoint &point);
		void reset();

		bool isStarted() const { return started; }

	private:
		TimedPoint lowestPoint;
		TimedPoint highestPoint;
		bool subsampleAvailable;

		bool started;

		std::function<void(const TimedPoint& point)> addPointCallback; 
	};

}}

