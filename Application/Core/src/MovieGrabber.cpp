// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MovieGrabber.h"

#include "StbRecording.h"
#include "StbImage.h"
#include "StbMarkers.h"
#include "IMessageBuilder.h"
#include "PACSPush.h"
#include "StbBoostViewer.h"
#include "PciXrayModel.h"
#include "FrameGrabber.h"
#include "Snapshot.h"

#include <ExperienceIdentity/Controls/Canvas.h>
#include <Sense/System/IScheduler.h>

namespace Pci { namespace Core { namespace PACS
{
	MovieGrabber::MovieGrabber(Sense::IScheduler& scheduler, CommonPlatform::Thread& worker, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, ViewType viewType) :
		_scheduler(scheduler),
		_worker(worker),
		_pacsPush(pacsPush),
		_msgBuilder(msgBuilder),
		_viewType(viewType)
	{
	}

	MovieGrabber::~MovieGrabber()
	{
		_worker.flush();
		_pacsPush.flush();
	}

	void MovieGrabber::grabMovie(const std::shared_ptr<const StbRecording>& boostedImages, int framesPerSecond, Sense::Size viewSize)
	{
		if (!boostedImages || boostedImages->getImageCount() == 0)
		{
			asyncInvoke([this]
			{
				if (onGrabbingDone) onGrabbingDone(nullptr);
			});
			return;
		}

		_msgBuilder.setSeriesNumber(boostedImages->getSeriesNumber());

		const auto& metaData = _msgBuilder.getMetaData();
		auto movie = _msgBuilder.StartMovie(metaData, framesPerSecond, static_cast<int>(viewSize.width), static_cast<int>(viewSize.height),RunType::SBL);
		if (!movie)
		{
			asyncInvoke([this]
			{
				if (onGrabbingDone) onGrabbingDone(nullptr);
			});
			return;
		}

		generateMovie(boostedImages, viewSize, movie);
	}

	void MovieGrabber::sendMovie(const std::shared_ptr<ImageSequence>& movie)
	{
		if (!movie) return;
		if (!movie->m_ended) return;

		_pacsPush.sendMovie(movie->m_metaData, *movie->m_message, [this, movie](bool success)
		{
			asyncInvoke([this, success, movie]
			{
				if (onSent) onSent(success, movie);
			});
		});
	}

	void MovieGrabber::generateMovie(const std::shared_ptr<const StbRecording>& boostedImages, Sense::Size viewSize, std::shared_ptr<ImageSequence> movie)
	{
		_worker.executeAsync([this, boostedImages, viewSize, movie]
		{
			Sense::Scheduler scheduler;
			Sense::Gpu gpu(0);
			Sense::ExperienceIdentity::Canvas canvas(gpu, scheduler);
			Sense::OutputOffscreen offScreen(canvas);

			PciXrayModel model;
			StbBoostViewer boostViewer(canvas, _viewType);
			boostViewer.setXrayModel(model);
			// canvas & control size
			boostViewer.size = viewSize;
			canvas.size = boostViewer.size;
			offScreen.size = canvas.size;
			boostViewer.layout = Sense::Layout::Fill;

			boostViewer.showRunInfo = true;
			model.showLastImageHold = true;

			gpu.show();
			auto& texture = offScreen.getTexture();

			bool result = true;

			auto& images = boostedImages->getAllImages();
			auto startOffset = images.size() > MaxNumberOfFramesMovie ? images.size() - MaxNumberOfFramesMovie : 0;
			boostViewer.totalFrames = static_cast<int>(images.size() - startOffset);
			int imgIndex = 0;
			for (auto it = images.begin() + startOffset; it != images.end(); it++)
			{
				boostViewer.currentFrame = imgIndex++;
				boostViewer.setImage(it->boost);
				offScreen.render();

				auto snapshot = Snapshot::fromSenseTexture(texture);

				result = _msgBuilder.AddFrame(*movie, *snapshot);
				if (!result)
					break;
			}

			if (result) result = _msgBuilder.EndMovie(*movie);

			asyncInvoke([this, movie = (result ? std::move(movie) : nullptr)]
			{
				if (onGrabbingDone) onGrabbingDone(movie);
			});
		});
	}

	void MovieGrabber::asyncInvoke(const std::function<void()> &command) const
	{
		_scheduler.addAsync(*this, command);
	}
}}}
