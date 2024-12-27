// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense/System/OutputOffscreen.h>
#include <Sense/Common/Timer.h>

namespace CommonPlatform
{
	class Log;
}

namespace Pci { namespace Core { namespace PACS
{
	using Sense::ICanvas;
	using Sense::OutputOffscreen;

	class Snapshot;

	constexpr size_t MaxNumberOfFramesMovie = 1000;

	class FrameGrabber
	{
	public:
        FrameGrabber(ICanvas& aCanvas, const Sense::Control& control);
		FrameGrabber(ICanvas& aCanvas, const Sense::Control& control1, const Sense::Control& control2);

		virtual ~FrameGrabber();

		FrameGrabber(const FrameGrabber&) = delete;
		FrameGrabber& operator=(const FrameGrabber&) = delete;

		std::shared_ptr<Snapshot> grabSingleFrame();

		void startMultiFrame(uint8_t framesPerSecond);
		void stopMultiFrame();
		std::vector<std::shared_ptr<Snapshot>> getFrames();
		
		void flushMultiFrame();

		std::function<void()> onMultiFrameStopped;

	private:
		void addFrameToMultiFrame();
		std::shared_ptr<Snapshot> readFrame();

		::CommonPlatform::Log& _log;
		OutputOffscreen _offscreen;

		std::vector<std::shared_ptr<Snapshot>> _recordedFrames;

		Sense::IScheduler& _scheduler;
		Sense::Timer _timer;
	};

}}}
