// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "MovieGrabberStbSubtract.h"

#include "StbRecording.h"
#include "IMessageBuilder.h"
#include "PACSPush.h"
#include "StbContrastViewer.h"
#include "PciXrayModel.h"
#include "Snapshot.h"

#include <ExperienceIdentity/Controls/Canvas.h>
#include <Sense/System/IScheduler.h>

namespace Pci { namespace Core { namespace PACS
{
	MovieGrabberStbSubtract::MovieGrabberStbSubtract(Sense::IScheduler& scheduler, CommonPlatform::Thread& worker, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, ViewType viewType, const bool& invertBoost) :
		MovieGrabber(scheduler, worker, pacsPush, msgBuilder, viewType),
		invertBoostImage(invertBoost)
	{
	}

	MovieGrabberStbSubtract::~MovieGrabberStbSubtract()
	{
	}

	void MovieGrabberStbSubtract::generateMovie(const std::shared_ptr<const StbRecording>& boostedImages, Sense::Size viewSize, std::shared_ptr<ImageSequence> movie)
	{
		bool invertBoostImageCopy = invertBoostImage;

		_worker.executeAsync([this, boostedImages, viewSize, movie, invertBoostImageCopy]
		{
			Sense::Scheduler scheduler;
			Sense::Gpu gpu(0);
			Sense::ExperienceIdentity::Canvas canvas(gpu, scheduler);
			Sense::OutputOffscreen offScreen(canvas);

			PciXrayModel model;
			StbContrastViewer contrastViewer(canvas, scheduler, _viewType);
			contrastViewer.setXrayModel(model);
			// canvas & control size
			contrastViewer.size = viewSize;
			canvas.size = contrastViewer.size;
			offScreen.size = canvas.size;
			contrastViewer.layout = Sense::Layout::Fill;

			contrastViewer.showRunInfo = true;

			boostedImages->setIncludeImagesWithoutMarkers(true);

			contrastViewer.totalFrames = boostedImages->getImageCount();
			contrastViewer.boostedFrames = boostedImages->getBoostedImageCount();
			contrastViewer.showBoostedFrameCounter = true;

			model.showLastImageHold = true;

			gpu.show();
			auto& texture = offScreen.getTexture();

			bool result = true;

			auto boostedFrame = boostedImages->getLastBoostedImage().boost;
			auto contrastFrame = boostedImages->getContrastImage().registeredLive;

			if (invertBoostImageCopy)
			{
				boostedFrame = boostedFrame->invert();
			}

			contrastViewer.setImages(boostedFrame, contrastFrame);

			float numberOfFrames = (1.0f / BLEND_STEP) * 2;

			for (auto i = 0; i < numberOfFrames; i++)
			{
				contrastViewer.setFixedBlend(std::abs((i * 0.05f) - 1.0f));
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

	const float MovieGrabberStbSubtract::BLEND_STEP = 0.05f;
}}}
