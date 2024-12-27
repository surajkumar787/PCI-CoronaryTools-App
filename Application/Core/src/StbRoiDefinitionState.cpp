// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbRoiDefinitionState.h"
#include "PciViewContext.h"
#include "StbRoiViewer.h"
#include "StbMarkersViewer.h"
#include "PciTsmPage.h"
#include "FrameGrabber.h"
#include "MovieGrabber.h"
#include "PciReviewParameters.h"
#include "StbAdministration.h"
#include "UtilizationLogEvent.h"
#include "ViewStateHelper.h"
#include "ViewType.h"
#include "Log.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "StbRoiTsmPage.h"
#include "IUiStateController.h"
#include "ICopyToRefClient.h"
#include "Translation.h"
#include "stlstring.h"

#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#include <assert.h>

using namespace CommonPlatform;

namespace Pci {	namespace Core {

	const int StbRoiDefinitionState::MINIMUM_ROI_SIZE = 64;

	StbRoiDefinitionState::StbRoiDefinitionState(PciViewContext &viewContext,
		ViewState state,
		const std::wstring &name,
		IUiStateController& controller,
		ViewState returnState,
		ViewState cancelState,
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
		const IArchivingSettings& archivingSettings)
			: UiState(viewContext, state, name, LicenseFeature::None),
			m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive)),
			m_viewType(pciViewContext.stbRoiViewer.getViewType()),
			m_boostModel(),
			m_markerModel(),
			m_uiController(controller),
			m_zoomSpeed(1.0),
			m_regionOfInterest(),
			m_returnState(returnState),
            m_cancelState(cancelState),
		    m_archivingSettings(archivingSettings)
	{
        auto stbTsmPage = new StbRoiTsmPage();
        stbTsmPage->copyToRefButton->visible = copyToRefClient.isAvailable();
        stbTsmPage->defineRoiButton->visible = true;
        tsmPage.reset(stbTsmPage);

		tsmPage->eventButtonPressed = [this](unsigned int button, bool repeated)
		{
			const double MAX_ZOOMSPEED(12.0);
			const double ZOOM_STEP(1.0);
			m_log.DeveloperInfo(L"StbRoiDefinitionState: TSM button %d pressed", button);
			
			if (repeated) m_zoomSpeed += ZOOM_STEP;
			else m_zoomSpeed = 1.0;
			m_zoomSpeed = std::min(m_zoomSpeed, MAX_ZOOMSPEED);

			auto callbackIt = m_tsmButtonCallbacks.find(button);
			if (callbackIt != m_tsmButtonCallbacks.end())
			{
				callbackIt->second();
			}
		};

		tsmPage->eventButtonReleased = [this](unsigned int button)
		{
			m_log.DeveloperInfo(L"StbRoiDefinitionState: TSM button %d released", button);

			if (button == StbTsmPage::DefineROIButton)
			{
				cancel();
			}
			else if (button == StbRoiTsmPage::ApplyButton)
			{
				apply();
			}
			else
			{
				updateTSMControls();
			}
		};

		m_tsmButtonCallbacks[StbRoiTsmPage::PanDownButton] = std::bind(&StbRoiDefinitionState::onPanDown, this);
		m_tsmButtonCallbacks[StbRoiTsmPage::PanUpButton] = std::bind(&StbRoiDefinitionState::onPanUp, this);
		m_tsmButtonCallbacks[StbRoiTsmPage::PanLeftButton] = std::bind(&StbRoiDefinitionState::onPanLeft, this);
		m_tsmButtonCallbacks[StbRoiTsmPage::PanRightButton] = std::bind(&StbRoiDefinitionState::onPanRight, this);
		m_tsmButtonCallbacks[StbRoiTsmPage::SizeUpButton] = std::bind(&StbRoiDefinitionState::onIncrease, this);
		m_tsmButtonCallbacks[StbRoiTsmPage::SizeDownButton] = std::bind(&StbRoiDefinitionState::onDecrease, this);
    }

	StbRoiDefinitionState::~StbRoiDefinitionState()
	{
	}

	void StbRoiDefinitionState::onEnter()
	{
		m_boostModel.showLastImageHold = true;

		pciViewContext.stbRoiViewer.setXrayModel(m_boostModel);
		pciViewContext.stbAngioViewer.setXrayModel(m_markerModel);

		show(&pciViewContext.stbRoiViewer);

		auto images = pciViewContext.stbAdministration.prepareReview(true);
		if (images != nullptr)
		{
			pciViewContext.stbRoiViewer.totalFrames = images->getImageCount();
			pciViewContext.stbRoiViewer.showRunInfo = true;

			m_imageBoundries = images->getImageBoundries();
			auto markerRegion = images->getMarkerRegion();

			markerRegion = setRoiAsSquare(markerRegion);
			if ((markerRegion.width() < MINIMUM_ROI_SIZE) || (markerRegion.height() < MINIMUM_ROI_SIZE))
			{
				markerRegion = createDefaultROI();
			}

			updateRoi(markerRegion);
            updateTSMControls();
		}

		m_uiController.startReplayRun(true);

		auto refTargetName = getCopyToRefTargetName();
		auto refTargetNr = m_archivingSettings.getCopyToRefTargetNumber(m_log, refTargetName);
		auto targetNumber = refTargetNr == -1 ? L"" : std::to_wstring(refTargetNr);
		getTsmPage().copyToRefButton->centerText = str_ext::replace(Localization::Translation::IDS_CopyToReference, L"{0}", targetNumber);

		autoReactivate = false;
	}

	PciXrayPositionModel& StbRoiDefinitionState::getModel()
	{
		return m_boostModel;
	}

	ViewState StbRoiDefinitionState::imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType)
	{
		if (imageType == CommonPlatform::Xray::XrayImage::Type::Exposure)
		{
			m_uiController.stopReplayRun();
		}

		return UiState::imageRunStarted(imageType);
	}

	ViewState StbRoiDefinitionState::epxChanged()
	{
		return viewState;
	}

	ViewState StbRoiDefinitionState::stentBoostImageAvailable(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, const std::shared_ptr<const StbMarkers>& markers, const std::shared_ptr<const StbImage>& /*boost*/)
	{
		pciViewContext.stbRoiViewer.currentFrame = image->imageNumber;
		pciViewContext.stbRoiViewer.setImage(image, markers);

		return viewState;
	}

	ViewState StbRoiDefinitionState::overlayImageAvailable()
	{
		autoReactivate = true;

		if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
		{
			return ViewState::CrmOverlayState;
		}

		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}

	ViewState StbRoiDefinitionState::coRegistrationConnectionChanged(bool connected)
	{
		UiState::coRegistrationConnectionChanged(connected);
		return viewState;
	}

	void StbRoiDefinitionState::updateRoi(const Sense::Rect& markerRoi)
	{
		m_regionOfInterest = markerRoi;

		pciViewContext.stbRoiViewer.regionOfInterest = m_regionOfInterest;
		m_uiController.setStbRegionOfInterest(m_regionOfInterest);
	}

	void StbRoiDefinitionState::updateTSMControls()
	{
		auto& tsm = getTsmPage();

		tsm.panDownButton->enabled = m_regionOfInterest.bottom < m_imageBoundries.bottom;
		tsm.panUpButton->enabled = m_regionOfInterest.top > m_imageBoundries.top;
		tsm.panLeftButton->enabled = m_regionOfInterest.left > m_imageBoundries.left;
		tsm.panRightButton->enabled = m_regionOfInterest.right < m_imageBoundries.right;
		tsm.sizeUpButton->enabled = m_regionOfInterest.width() < m_imageBoundries.width() && m_regionOfInterest.height() < m_imageBoundries.height();
		tsm.sizeDownButton->enabled = m_regionOfInterest.width() > MINIMUM_ROI_SIZE && m_regionOfInterest.height() > MINIMUM_ROI_SIZE;
		
		tsm.requestUpdate();
	}

	Sense::Rect StbRoiDefinitionState::moveROIWithinBoundries(const Sense::Rect& roi) const
	{
		Sense::Rect validRoi(roi);

		if (roi.left < m_imageBoundries.left)
		{
			validRoi.left = m_imageBoundries.left;
			validRoi.right += (m_imageBoundries.left - roi.left);
		}
		else if (roi.right > m_imageBoundries.right)
		{
			validRoi.right = m_imageBoundries.right;
			validRoi.left -= (roi.right - m_imageBoundries.right);
		}
		
		if (roi.top < m_imageBoundries.top)
		{
			validRoi.top = m_imageBoundries.top;
			validRoi.bottom += (m_imageBoundries.top - roi.top);
		}
		else if (roi.bottom > m_imageBoundries.bottom)
		{
			validRoi.bottom = m_imageBoundries.bottom;
			validRoi.top -= (roi.bottom - m_imageBoundries.bottom);
		}

		return validRoi;
	}

	Sense::Rect StbRoiDefinitionState::setRoiAsSquare(Sense::Rect markerRoi)
	{
		if (markerRoi.width() > markerRoi.height())
		{
			markerRoi.bottom = markerRoi.top + markerRoi.width();
		}
		else
		{
			markerRoi.right = markerRoi.left + markerRoi.height();
		}

		return markerRoi;
	}

	Sense::Rect StbRoiDefinitionState::createDefaultROI() const
	{
		const Sense::Point center(m_imageBoundries.width() / 2.0, m_imageBoundries.height() / 2.0);
		const Sense::Size DEFAULT_REGION_SIZE(std::min(m_imageBoundries.width(), m_imageBoundries.height()) * 0.4);

		return Sense::Rect(center - (DEFAULT_REGION_SIZE / 2.0), center + (DEFAULT_REGION_SIZE / 2.0));
	}

	void StbRoiDefinitionState::onPanUp()
	{
		auto moveStep = std::min(m_zoomSpeed, (m_regionOfInterest.top - m_imageBoundries.top));

		auto newROI(m_regionOfInterest);
		newROI.top -= moveStep;
		newROI.bottom -= moveStep;
		updateRoi(newROI);
	}


	void StbRoiDefinitionState::onPanDown()
	{
		auto moveStep = std::min(m_zoomSpeed, (m_imageBoundries.bottom - m_regionOfInterest.bottom));

		auto newROI(m_regionOfInterest);
		newROI.top += moveStep;
		newROI.bottom += moveStep;
		updateRoi(newROI);
	}

	void StbRoiDefinitionState::onPanLeft()
	{
		auto moveStep = std::min(m_zoomSpeed, (m_regionOfInterest.left - m_imageBoundries.left));

		auto newROI(m_regionOfInterest);
		newROI.left -= moveStep;
		newROI.right -= moveStep;
		updateRoi(newROI);
	}

	void StbRoiDefinitionState::onPanRight()
	{
		auto moveStep = std::min(m_zoomSpeed, (m_imageBoundries.right - m_regionOfInterest.right));

		auto newROI(m_regionOfInterest);
		newROI.left += moveStep;
		newROI.right += moveStep;
		updateRoi(newROI);
	}

	void StbRoiDefinitionState::onIncrease()
	{
		auto moveStep = std::min(m_zoomSpeed, m_imageBoundries.height() - m_regionOfInterest.height());
		moveStep = std::min(moveStep, m_imageBoundries.width() - m_regionOfInterest.width());
		moveStep /= 2.0;

		auto newROI(m_regionOfInterest);
		newROI.left -= moveStep;
		newROI.right += moveStep;
		newROI.top -= moveStep;
		newROI.bottom += moveStep;
		
		newROI = moveROIWithinBoundries(newROI);	
		
		updateRoi(newROI);
	}

	void StbRoiDefinitionState::onDecrease()
	{
		auto moveStep = std::min(m_zoomSpeed, m_regionOfInterest.width() - MINIMUM_ROI_SIZE);
		moveStep /= 2.0;

		auto newROI(m_regionOfInterest);
		newROI.left += moveStep;
		newROI.right -= moveStep;
		newROI.top += moveStep;
		newROI.bottom -= moveStep;
		updateRoi(newROI);
	}

    void StbRoiDefinitionState::apply()
    {
		m_uiController.setStbRegionOfInterest(m_regionOfInterest);

        m_uiController.stopReplayRun();
        m_uiController.startReplayRun(false);
		m_uiController.switchState(m_returnState);
    }

    
    StbRoiTsmPage& StbRoiDefinitionState::getTsmPage() const
    {
        return *static_cast<StbRoiTsmPage*>(tsmPage.get());
    }

    void StbRoiDefinitionState::cancel()
    {
        if (m_uiController.isReplayActive())
        {
            m_uiController.stopReplayRun();
        }

        m_uiController.switchState(m_cancelState);
    }
}}

