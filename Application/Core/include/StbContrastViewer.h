// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbViewer.h"
#include "StbMarkers.h"
#include "StbMask.h"


namespace Pci {	namespace Core{
	struct StbImage;

class StbContrastViewer : public StbViewer
{
public:
	StbContrastViewer(Control &parent, Sense::ITimerScheduler& scheduler, ViewType type);
	virtual ~StbContrastViewer();

	void setImages(const std::shared_ptr<const StbImage>& boost,
				   const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& contrast);

	void setBoostImage(const std::shared_ptr<const StbImage>& boost);
	void clearImage();

	void startAutoBlend();
	void pauseAutoBlend();
	double getBlendRatio();
	void setFixedBlend(double ratio);

	bool hasContrastImage() const;

	bool isPaused() const;

	StbContrastViewer(const StbContrastViewer&) = delete;
	StbContrastViewer& operator=(const StbContrastViewer&) = delete;

private:
	void renderOverlay(Sense::IRenderer &renderer) const override;

	void updateBlendImage();
	void onFadeInFadeOutTimer();

	double			m_opacity;
	double			m_fadeinOutDirection;
	Sense::Timer	m_fadeInFadeOutTimer;
	std::unique_ptr< CommonPlatform::Xray::XrayImage>		  m_contrastImage;
	std::unique_ptr< StbImage>								  m_boostImage;
	std::shared_ptr<CommonPlatform::Xray::XrayImage>		  m_blendImage;
	StbMask													  m_stbMask;
};

}}