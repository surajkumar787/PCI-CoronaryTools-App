// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "FrameGrabber.h"

#include <algorithm>
#include <assert.h>

#include "Log.h"
#include "Snapshot.h"

namespace Pci { namespace Core { namespace PACS
{
	static Sense::Rect getTargetRect(ICanvas& aCanvas, const Sense::Control& control)
	{
		auto rootSpace = aCanvas.getRoot().getSpace();
		auto childSpace = control.getSpace();

		return rootSpace.mapFrom(childSpace, control.getBounds());
	}

	static Sense::Rect getTargetRect(ICanvas& aCanvas, const Sense::Control& control1, const Sense::Control& control2)
	{
		auto rootSpace = aCanvas.getRoot().getSpace();
		auto childSpace1 = control1.getSpace();
		auto childSpace2 = control2.getSpace();

		auto childRect1 = rootSpace.mapFrom(childSpace1, control1.getBounds());
		auto childRect2 = rootSpace.mapFrom(childSpace2, control2.getBounds());

		return Sense::Rect(std::min(childRect1.left, childRect2.left),
		                   std::min(childRect1.top, childRect2.top),
		                   std::max(childRect1.right, childRect2.right),
		                   std::max(childRect1.bottom, childRect2.bottom));
	}

    FrameGrabber::FrameGrabber(ICanvas& aCanvas, const Sense::Control& control) :
		_log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
		_offscreen(aCanvas),
		_scheduler(aCanvas.getScheduler()),
		_timer(_scheduler, [this] { addFrameToMultiFrame(); })
    {
		auto targetRect = getTargetRect(aCanvas, control);
        if (!targetRect.empty())
        {
            _offscreen.position  = targetRect.position();
            _offscreen.size      = targetRect.size();
        }
    }

	FrameGrabber::FrameGrabber(ICanvas& aCanvas, const Sense::Control& control1, const Sense::Control& control2) :
		_log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
		_offscreen(aCanvas),
		_scheduler(aCanvas.getScheduler()),
		_timer(_scheduler, [this] { addFrameToMultiFrame(); })
	{
		auto targetRect = getTargetRect(aCanvas, control1, control2);
		if (!targetRect.empty())
		{
			_offscreen.position  = targetRect.position();
			_offscreen.size      = targetRect.size();
		}
	}

	FrameGrabber::~FrameGrabber()
	{
		flushMultiFrame();
	}

	void FrameGrabber::flushMultiFrame()
	{
		if (_recordedFrames.empty() == false)
		{
			stopMultiFrame();
			if (onMultiFrameStopped)
			{
				onMultiFrameStopped();
			}
		}
		_recordedFrames.clear();
	}

	std::shared_ptr<Snapshot> FrameGrabber::grabSingleFrame()
	{
		return readFrame();
	}

	void FrameGrabber::startMultiFrame(uint8_t framesPerSecond)
	{
		_recordedFrames.clear();
		_timer.start(1.0 / static_cast<float>(framesPerSecond));
	}

	void FrameGrabber::stopMultiFrame()
	{
		_timer.stop();
	}

	std::vector<std::shared_ptr<Snapshot>> FrameGrabber::getFrames()
	{
		return _recordedFrames;
	}

	void FrameGrabber::addFrameToMultiFrame()
	{
		auto frame = readFrame();

		if (!frame) return;

		assert (frame->getHeight() >= 0);
		assert (frame->getWidth() >= 0);

		if (_recordedFrames.empty() == false)
		{
			auto firstFrame = *_recordedFrames.begin();
			if (firstFrame->getHeight() != frame->getHeight() ||
				firstFrame->getWidth() != frame->getWidth())
			{
				return;
			}
		}
				
		_recordedFrames.push_back(frame);

		if (_recordedFrames.size() > MaxNumberOfFramesMovie)
		{
			// Maximum number of frames reached
			if (onMultiFrameStopped)
			{
				onMultiFrameStopped();
			}
		}
	}

	std::shared_ptr<Snapshot> FrameGrabber::readFrame()
	{
		_offscreen.render();
		auto& texture = _offscreen.getTexture();

		auto snapshot = Snapshot::fromSenseTexture(texture);

		if (!snapshot)
		{
			_log.DeveloperError(L"FrameGrabber: unable to read frame");
			return nullptr;
		}
		
		return snapshot;
	}
}}}
