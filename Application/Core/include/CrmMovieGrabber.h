// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <functional>
#include <Sense/System/IScheduler.h>

namespace Sense
{
	struct Size;
}

namespace CommonPlatform { class Thread; }

namespace Pci { namespace Core
{
	class CrmRecording;
	enum class ViewType;

	namespace PACS
	{
		enum class GrabType;
		struct ImageSequence;
		class IPACSPush;
		class IMessageBuilder;

		class CrmMovieGrabber :  private Sense::IAsync
		{
		public:
			explicit CrmMovieGrabber(Sense::IScheduler& scheduler, CommonPlatform::Thread& worker, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, ViewType viewType);
			virtual ~CrmMovieGrabber();

			void grabMovie(const std::shared_ptr<const CrmRecording>& recording, int framesPerSecond, Sense::Size viewSize);
			void sendMovie(const std::shared_ptr<ImageSequence>& movie);

			std::function<void(std::shared_ptr<ImageSequence> movie)> onGrabbingDone;
			std::function<void(bool finishedSuccessfully, std::shared_ptr<const ImageSequence> movie)> onSent;

		protected:
			virtual void generateMovie(const std::shared_ptr<const CrmRecording>& recording, Sense::Size viewSize, std::shared_ptr<ImageSequence> movie);
			virtual void asyncInvoke(const std::function<void()>& command) const override;

			Sense::IScheduler& _scheduler;
			CommonPlatform::Thread& _worker;
			IPACSPush& _pacsPush;
			IMessageBuilder& _msgBuilder;
			ViewType _viewType;
		};
	}
}}