// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "MovieGrabber.h"

namespace Sense
{
	struct Size;
}

namespace CommonPlatform { class Thread; }

namespace Pci { namespace Core
{
	namespace PACS
	{
		class MovieGrabberStbSubtract : public MovieGrabber
		{
		public:
			explicit MovieGrabberStbSubtract(Sense::IScheduler& scheduler, CommonPlatform::Thread& worker, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, ViewType viewType, const bool& invertBoostImage);
			virtual ~MovieGrabberStbSubtract();

		private:
			virtual void generateMovie(const std::shared_ptr<const StbRecording>& boostedImages, Sense::Size viewSize, std::shared_ptr<ImageSequence> movie) override;

			static const float BLEND_STEP;

			const bool& invertBoostImage;
		};
	}
}}
