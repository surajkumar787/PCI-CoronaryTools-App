// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmMovieGrabber.h"

#include "CrmRecording.h"
#include "IMessageBuilder.h"
#include "PACSPush.h"
#include "CrmViewer.h"
#include "PciXrayModel.h"
#include "FrameGrabber.h"
#include "Snapshot.h"

#include <ExperienceIdentity/Controls/Canvas.h>
#include <Sense/System/IScheduler.h>

namespace Pci { namespace Core { namespace PACS
{
	CrmMovieGrabber::CrmMovieGrabber(Sense::IScheduler& scheduler, CommonPlatform::Thread& worker, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, ViewType viewType) :
		_scheduler(scheduler),
		_worker(worker),
		_pacsPush(pacsPush),
		_msgBuilder(msgBuilder),
		_viewType(viewType)
	{
	}

	CrmMovieGrabber::~CrmMovieGrabber()
	{
		_worker.flush();
		_pacsPush.flush();
	}

	void CrmMovieGrabber::grabMovie(const std::shared_ptr<const CrmRecording>& recording, int framesPerSecond, Sense::Size viewSize)
	{
		if (!recording || recording->getImageCount() == 0)
		{
			asyncInvoke([this]
			{
				if (onGrabbingDone) onGrabbingDone(nullptr);
			});
			return;
		}


		_msgBuilder.setSeriesNumber(recording->getSeriesNumber());
		const auto& metaData = _msgBuilder.getMetaData();
		auto movie = _msgBuilder.StartMovie(metaData, framesPerSecond, static_cast<int>(viewSize.width), static_cast<int>(viewSize.height), RunType::DCR);
		if (!movie)
		{
			asyncInvoke([this]
			{
				if (onGrabbingDone) onGrabbingDone(nullptr);
			});
			return;
		}

		generateMovie(recording, viewSize, movie);
	}

	void CrmMovieGrabber::sendMovie(const std::shared_ptr<ImageSequence>& movie)
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

	void CrmMovieGrabber::generateMovie(const std::shared_ptr<const CrmRecording>& recording, Sense::Size viewSize, std::shared_ptr<ImageSequence> movie)
	{
		_worker.executeAsync([this, recording, viewSize, movie]
		{
			Sense::Scheduler scheduler;
			Sense::Gpu gpu(0);
			Sense::ExperienceIdentity::Canvas canvas(gpu, scheduler);
			Sense::OutputOffscreen offScreen(canvas);

			PciXrayModel model;
			CrmViewer crmViewer(canvas, _viewType);
			crmViewer.setXrayModel(model);
			// canvas & control size
			crmViewer.size = viewSize;
			canvas.size = crmViewer.size;
			offScreen.size = canvas.size;
			crmViewer.layout = Sense::Layout::Fill;

			crmViewer.showXrayPosition = true;
			model.showLastImageHold = true;

			gpu.show();
			auto& texture = offScreen.getTexture();

			bool result = true;

			auto& images = recording->getAllImages();
			auto startOffset = images.size() > MaxNumberOfFramesMovie ? images.size() - MaxNumberOfFramesMovie : 0;

			for (auto it = images.begin() + startOffset; it != images.end(); it++)
			{
				crmViewer.setImage(it->first);
				crmViewer.setOverlay(it->second);
				
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

	void CrmMovieGrabber::asyncInvoke(const std::function<void()> &command) const
	{
		_scheduler.addAsync(*this, command);
	}
}}}
