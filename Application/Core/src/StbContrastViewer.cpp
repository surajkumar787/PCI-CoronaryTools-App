// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastViewer.h"
#include "AlluraColors.h"
#include "StbImage.h"
#include "StbMask.h"
#include "ViewType.h"
#include "Translation.h"

namespace Pci {	namespace Core {

using Sense::ExperienceIdentity::Palette;

static const double UpdateTimeIntervalSec = 0.05;
static const double FadeInFadeOutIncrement = 0.025;
static const double MaximumOpacityRange = 1.2;
static const double MinimumOpacityRange = -0.2;

StbContrastViewer::StbContrastViewer(Control &parent, Sense::ITimerScheduler& scheduler, ViewType type) :
	StbViewer(parent, type),
	m_opacity(0.0),
	m_fadeinOutDirection(FadeInFadeOutIncrement),
	m_fadeInFadeOutTimer( scheduler, std::bind(&StbContrastViewer::onFadeInFadeOutTimer,this)),
	m_contrastImage(nullptr),	
	m_boostImage(nullptr),
	m_blendImage(std::make_shared<CommonPlatform::Xray::XrayImage>()),
	m_stbMask()
{
	showXrayPosition = true;
}


StbContrastViewer::~StbContrastViewer()
{
}


void StbContrastViewer::renderOverlay(Sense::IRenderer &renderer) const
{
	StbViewer::renderOverlay(renderer);

	Sense::Space  space = getClientSpace();
	Sense::Matrix matrix = pixelsToClient();

	if (m_boostImage && m_boostImage->isValid() && !m_contrastImage)
	{
		warningMain.text = Pci::Core::Localization::Translation::IDS_NoContrast;
		warningMain.image = Sense::ExperienceIdentity::Icons::Information;
		warningMain.render(space, renderer);
	}
	else if (!m_boostImage || !m_boostImage->isValid())
	{
		warningMain.text = Pci::Core::Localization::Translation::IDS_EnhancementFailed;
		warningMain.image = Sense::ExperienceIdentity::Icons::GeneralWarning;
		warningSub.text = Pci::Core::Localization::Translation::IDS_BalloonMarkersLost;

		warningMain.render(space, renderer);
		warningSub.render(space, renderer);
	}
}

void StbContrastViewer::updateBlendImage()
{
	if (m_contrastImage && m_boostImage)
	{
		double boostOpacity = std::max(0.0, std::min(m_opacity, 1.0));
		double contrastOpacity = 1 - boostOpacity;

		for (int y = 0; y < m_blendImage->height; y++)
		{
			int offsetContrast = y * m_contrastImage->stride;
			int offsetBoost = y * m_boostImage->stride;
			int offsetBlend = y * m_blendImage->stride;
			for (int x = 0; x < m_blendImage->width; x++)
			{
				m_blendImage->pointer[offsetBlend + x] = static_cast<short>(m_boostImage->pointer[offsetBoost + x] * boostOpacity + m_contrastImage->pointer[offsetContrast + x] * contrastOpacity);
			}
		}
	}
	else if (m_boostImage)
	{
		m_blendImage->copyFrom(*m_boostImage);
	}
}

void StbContrastViewer::onFadeInFadeOutTimer()
{
	m_opacity += m_fadeinOutDirection;
	if (m_opacity <= MinimumOpacityRange || m_opacity >= MaximumOpacityRange)
	{
		m_fadeinOutDirection *= -1.0;
	}

	updateBlendImage();
	setImage(m_blendImage);
}

void StbContrastViewer::setImages(const std::shared_ptr<const StbImage>& boost,
								  const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& contrast)
{
	if (!boost) return;

	m_blendImage.reset(new CommonPlatform::Xray::XrayImage(*boost));

	m_boostImage = std::make_unique<StbImage>(*boost);
	m_stbMask.mask(m_boostImage->refMarkers, *boost, *m_boostImage);
	m_contrastImage = nullptr;

	if (contrast)
	{
		m_contrastImage = std::make_unique<CommonPlatform::Xray::XrayImage>(*contrast);
		m_stbMask.mask(m_boostImage->refMarkers, *contrast, *m_contrastImage);	
	}
	
	updateBlendImage();
	setImage(m_blendImage);
}

void StbContrastViewer::setBoostImage(const std::shared_ptr<const StbImage>& boost)
{
	m_boostImage = std::make_unique<StbImage>(*boost);
	m_stbMask.mask(m_boostImage->refMarkers, *boost, *m_boostImage);
	updateBlendImage();
	setImage(m_blendImage);
}

void StbContrastViewer::clearImage()
{
	m_fadeInFadeOutTimer.stop();

	m_contrastImage.reset();
	m_boostImage.reset();

	m_opacity = 0;
	m_fadeinOutDirection = std::abs(m_fadeinOutDirection);
}

void StbContrastViewer::startAutoBlend()
{
	if (!m_fadeInFadeOutTimer.isActive())
	{
		m_fadeInFadeOutTimer.start(UpdateTimeIntervalSec);
	}
}

void StbContrastViewer::pauseAutoBlend()
{
	m_fadeInFadeOutTimer.stop();
}

double StbContrastViewer::getBlendRatio()
{
	return std::max(0.0, std::min(m_opacity, 1.0));
}

void StbContrastViewer::setFixedBlend(double ratio)
{
	if (ratio >= 0.0 && ratio <= 1.0)
	{
		m_fadeInFadeOutTimer.stop();
		m_opacity = ratio;
		
		updateBlendImage();
		setImage(m_blendImage);
	}
}

bool StbContrastViewer::hasContrastImage() const
{
	return (m_contrastImage != nullptr);
}

bool StbContrastViewer::isPaused() const
{
	return !m_fadeInFadeOutTimer.isActive();
}

}}