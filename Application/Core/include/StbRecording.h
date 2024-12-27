// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense/Common/Rect.h>
#include <memory>
#include <deque>
#include "StbImage.h"


namespace Pci { namespace Core
{	
	struct ContrastScore
	{
		ContrastScore() { score = 0.0f; isValid = false; };
		float score;
		bool isValid;
	};

	struct StbResult
	{
		~StbResult() { live = nullptr; markers = nullptr; registeredLive = nullptr; boost = nullptr; };

		std::shared_ptr<const CommonPlatform::Xray::XrayImage>	live;
		std::shared_ptr<const StbMarkers>						markers;
		std::shared_ptr<const StbImage>							registeredLive;
		std::shared_ptr<const StbImage>							boost;	
		ContrastScore											contrastScore;
	};

	class StbRecording
	{
	public:
		explicit StbRecording(size_t loopMaxNrFrames);
		virtual ~StbRecording();

		void addImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,
		              const std::shared_ptr<const StbMarkers> &markers,
		              const std::shared_ptr<const StbImage>& registeredLive,
					 const std::shared_ptr<const StbImage>& boost);
		void setBoostImage(	const std::shared_ptr<const StbImage>& boost);
		void setRegisteredLiveImage(const std::shared_ptr<const StbImage>& registeredLive);

		int getSeriesNumber() const;

		int getImageCount() const;
		int getRunIndex() const;
		int getBoostedImageCount() const;
		bool isEmpty() const;

		const StbResult getContrastImage() const;
		const StbResult getLastBoostedImage() const;
		void setAutoDetectedFrame(int index);
        void setManualContrastFrame(int index);

		void setContrastScore(const std::vector<std::tuple<int, float>>& score);
		int getAutoDetectedContrastFrameIndex() const;

        void setBoostPhaseEnd(int index);

		const std::deque<StbResult> getAllImages() const;
        const std::deque<StbResult> getAllContrastImages() const;
        const std::deque<StbResult> getAllReferenceImages() const;
        Sense::Rect getMarkerRegion() const;
		Sense::Rect getImageBoundries() const;

		void setIncludeImagesWithoutMarkers(bool include) const;

		void restoreDefaultBoostedPhase();

        bool isAutoArchived() const;
        void setAutoArchived();

	private:
		int  getFirstLastIIndexWithMarkers();

		std::deque<StbResult> m_images;
		size_t                m_maxFrames;
		mutable bool          m_includeAllImagesWithoutMarkers;
		int					  m_currentContrastFrameIndex;
		int					  m_autoSelectedContrastFrameIndex;
		int					  m_autoSelectedLastBoostFrameIndex;
        bool                  m_isAutoArchived;
    };
}}