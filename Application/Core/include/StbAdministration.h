// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <memory>
#include <functional>
#include <vector>

namespace CommonPlatform{ namespace Xray
{
	struct XrayImage;
}}


namespace Pci { namespace Core
{
	struct PciReviewParameters;
	class StbRecording;
	struct StbMarkers;
	struct StbImage;

	class StbAdministration
	{
	public:
		explicit StbAdministration(const PciReviewParameters &reviewParams);

		void setImage(const ::std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,
					  const ::std::shared_ptr<const StbMarkers> &markers,
			 	      const ::std::shared_ptr<const StbImage> &registeredLive,
					  const ::std::shared_ptr<const StbImage>& boost);
		void setImage(const ::std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image,
					  const ::std::shared_ptr<const StbMarkers> &markers,
					  const ::std::shared_ptr<const StbImage> &boost);
		void setBoostImage(const ::std::shared_ptr<const StbImage>& boost,
						   const ::std::shared_ptr<const StbImage>& registeredLive);
		std::shared_ptr<StbRecording> prepareReview();
		std::shared_ptr<StbRecording> prepareReview(bool includeFramesWithoutMarkers);
		void studyChanged();

		bool isEmpty() const;
		void newRun();

		int lastImageIndex() const;
		int numberOfValidImages() const;

		int getBoostImageIndex();
		void setBoostPhaseEnd(int index);
		void restoreDefaultBoostedPhase();


		void setAutoSelectedContrastFrame(int index);
		int getBestContrastImage();
		void setContrastScore(const std::vector<std::tuple<int, float>>& score);

		StbAdministration(const StbAdministration&) = delete;
		void operator = (const StbAdministration&) = delete;

	private:
		void clear();

		const PciReviewParameters&      m_reviewParams;
		std::shared_ptr<StbRecording>   m_stbImages;
		bool                            m_reviewReady;
		int                             m_lastImageIndex;
	};
}}


