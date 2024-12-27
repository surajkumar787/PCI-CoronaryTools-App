// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <deque>
#include <memory>
#include <functional>

#include "Sense/Common/Timer.h"
#include "PciReviewParameters.h"

namespace Pci { namespace Core
{
	template<typename T>
	class PciReviewPlayer
	{
	public:
		typedef typename std::remove_reference<decltype(std::declval<T>().getAllImages().front())>::type SingleFrame;

		PciReviewPlayer(Sense::ITimerScheduler& scheduler, const PciReviewParameters& reviewParams) :
			m_reviewParams(reviewParams),
			m_frameTime(1.0),
			m_currentFrame(-1),
			m_currentLoop(0),
			m_reviewTimer(scheduler, [this] { reviewTimerTick(); })
		{
		}

		void setReview(const std::shared_ptr<T>& reviewImages, double frameTime)
		{
			m_reviewImages = reviewImages;
			m_currentFrame = -1;
			m_currentLoop = 0;
			m_frameTime = frameTime;
		}

		void resetReview()
		{
			m_currentFrame = -1;
			m_currentLoop = 0;
		}

        void clearReview()
		{
            m_reviewImages.reset();
		}

		void startReview()
		{
			if (m_reviewParams.NrOfLoops == 0)
			{
				reviewImage(getFrameCount() - 1);
			}
			else if (m_reviewImages && m_reviewImages->getImageCount() != 0)
			{
				if (m_currentFrame == (getFrameCount() - 1))
				{
					m_reviewTimer.start(m_reviewParams.LoopDelay);
				}
				else
				{
					m_reviewTimer.start(m_frameTime);
				}
			}
		}

		void stopReview()
		{
			m_reviewTimer.stop();
		}

		void previousFrame()
		{
			m_currentFrame--;
			if (m_currentFrame < 0)
			{
				m_currentFrame = getFrameCount() - 1;
				if (m_currentLoop > 0) m_currentLoop--;
			}

			reviewImage(m_currentFrame);
		}

		bool nextFrame()
		{
			m_currentFrame++;

			if (m_currentFrame >= getFrameCount())
			{
				m_currentFrame = 0;
				m_currentLoop++;
				if (m_reviewParams.NrOfLoops != -1 && m_currentLoop >= m_reviewParams.NrOfLoops)
					return false;
			}

			reviewImage(m_currentFrame);
			return true;
		}

		bool isFirstLoop() const
		{
			return m_currentLoop == 0;
		}

		bool inReview() const
		{
			return m_reviewTimer.isActive();
		}

		int getFrameCount() const
		{
			if (!m_reviewImages) return 0;
			return m_reviewImages->getImageCount();
		}

		std::function<void(int /*frameIndex*/, const SingleFrame& /*reviewImage*/, bool /*lastFrameReached*/)> eventReviewImage;

		PciReviewPlayer(const PciReviewPlayer&) = delete;
		void operator= (const PciReviewPlayer&) = delete;

	private:
		bool isLastFrame() const
		{
			return isLastLoop() && m_currentFrame == (getFrameCount() - 1);
		}

		bool isLastLoop() const
		{
			return m_reviewParams.NrOfLoops != -1 && m_currentLoop == (m_reviewParams.NrOfLoops - 1);
		}

		void reviewImage(int frame) const
		{
			if (eventReviewImage && m_reviewImages && 
				frame < m_reviewImages->getAllImages().size())
			{
				eventReviewImage(frame, m_reviewImages->getAllImages()[frame], isLastFrame());
			}
		}

		void reviewTimerTick()
		{
			if (!m_reviewImages || m_reviewImages->getAllImages().empty())
			{
				stopReview();
				return;
			}

			if (!nextFrame())
				stopReview();
			else if (m_currentFrame == (getFrameCount() - 1))
				m_reviewTimer.start(m_reviewParams.LoopDelay);
			else
				m_reviewTimer.start(m_frameTime);
		}

		const PciReviewParameters& m_reviewParams;
		double                     m_frameTime;
		int                        m_currentFrame;
		int                        m_currentLoop;
		Sense::Timer               m_reviewTimer;
		std::shared_ptr<T>         m_reviewImages;
	};
}}