// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbBoostFrameSelectionState.h"
#include "PciViewContext.h"
#include "StbRoiViewer.h"
#include "StbMarkersViewer.h"
#include "PciTsmPage.h"
#include "FrameGrabber.h"
#include "MovieGrabber.h"
#include "PciReviewParameters.h"
#include "StbAdministration.h"
#include "UtilizationLogEvent.h"
#include "IArchivingStatus.h"
#include "ViewStateHelper.h"
#include "ViewType.h"
#include "Log.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "StbImageSelectTsmPage.h"
#include "StbImageSelectViewer.h"
#include "IUiStateController.h"
#include "ICopyToRefClient.h"

#include <assert.h>

using namespace CommonPlatform;

namespace Pci {	namespace Core {

StbBoostFrameSelectionState::StbBoostFrameSelectionState(PciViewContext &viewContext, IUiStateController& uiController, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient) :
	UiState(viewContext, ViewState::StbBoostFrameSelectionState, L"StbBoostFrameSelectionState", LicenseFeature::StbSubtract),
	m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive)),
	m_uiController(uiController)
{
    auto stbTsmPage = new StbImageSelectTsmPage();
    stbTsmPage->copyToRefButton->visible = copyToRefClient.isAvailable();
	tsmPage.reset(stbTsmPage);

	tsmPage->eventButtonReleased = [this](unsigned int button)
	{
		m_log.DeveloperInfo(L"StbBoostFrameSelectionState: TSM button %d released", button);

		auto callbackIt = m_tsmButtonCallbacks.find(button);
		if (callbackIt != m_tsmButtonCallbacks.end())
		{
			callbackIt->second();
		}
	};

	m_tsmButtonCallbacks[StbImageSelectTsmPage::NavigateLeftButton] = std::bind(&StbBoostFrameSelectionState::onNavigateLeftDown, this);
	m_tsmButtonCallbacks[StbImageSelectTsmPage::NavigateRightButton] = std::bind(&StbBoostFrameSelectionState::onNavigateRightDown, this);
	m_tsmButtonCallbacks[StbImageSelectTsmPage::ResetDefaultButton] = std::bind(&StbBoostFrameSelectionState::onResetDefaultDown, this);
    m_tsmButtonCallbacks[StbImageSelectTsmPage::ApplyButton] = std::bind(&StbBoostFrameSelectionState::apply, this);
    m_tsmButtonCallbacks[StbTsmPage::BoostFrameSelectButton] = std::bind(&StbBoostFrameSelectionState::cancel, this);

    enableBoostImageSelectionButton();
}

void StbBoostFrameSelectionState::enableBoostImageSelectionButton()
{
    getTsmPage().boostFrameSelectButton->visible = true;
    getTsmPage().boostFrameSelectButton->enabled = true;
    getTsmPage().boostFrameSelectButton->selected = true;

    getTsmPage().contrastFrameSelectButton->visible = true;
    getTsmPage().defineRoiButton->visible = true;
    getTsmPage().invertBoostButton->visible = true;
}

StbBoostFrameSelectionState::~StbBoostFrameSelectionState()
{
}

void StbBoostFrameSelectionState::onEnter()
{
    m_referenceFrames.clear();

	pciViewContext.stbBoostSelectViewer.guidanceTextLabel.text = Localization::Translation::IDS_UseTSMToSelectLastImageBoostedPhase;

	auto stbRecording = pciViewContext.stbAdministration.prepareReview(false);
	if (stbRecording != nullptr)
	{
		auto referenceImages = stbRecording->getAllReferenceImages();
			
		for (const auto& referenceImage : referenceImages)
		{
            if (referenceImage.registeredLive->isValid())
            {
                m_referenceFrames.push_back(referenceImage.registeredLive);
            }
		}

		if (!m_referenceFrames.empty())
		{
			pciViewContext.stbBoostSelectViewer.setFrames(m_referenceFrames);
			setSelection(stbRecording->getLastBoostedImage().boost->imageNumber);

			show(&pciViewContext.stbBoostSelectViewer);
		}
	}

	autoReactivate = false;   
}

StbImageSelectTsmPage& StbBoostFrameSelectionState::getTsmPage() const
{
	return *static_cast<StbImageSelectTsmPage*>(tsmPage.get());
}

PciXrayPositionModel& StbBoostFrameSelectionState::getModel()
{
	return m_boostModel;
}

ViewState StbBoostFrameSelectionState::epxChanged()
{
	return viewState;
}

ViewState StbBoostFrameSelectionState::overlayImageAvailable()
{
	autoReactivate = true;

	if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		return ViewState::CrmOverlayState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}

void StbBoostFrameSelectionState::onNavigateLeftDown()
{
	auto currentSelected = m_selectedImage;	
    auto selection = std::find_if(m_referenceFrames.begin(), m_referenceFrames.end(), [currentSelected](std::shared_ptr<const StbImage> &image) {return image->imageNumber == currentSelected; });

    if (selection != m_referenceFrames.begin())
    {
        selection--;
        setSelection((*selection)->imageNumber);
    }
}

void StbBoostFrameSelectionState::onNavigateRightDown()
{
	auto currentSelected = m_selectedImage;
	auto selection = std::find_if(m_referenceFrames.begin(), m_referenceFrames.end(), [currentSelected](std::shared_ptr<const StbImage> &image) {return image->imageNumber == currentSelected; });

    selection++;

	if ((selection != m_referenceFrames.end()))
	{
		setSelection((*selection)->imageNumber);
	}
}

void StbBoostFrameSelectionState::onResetDefaultDown()
{
	pciViewContext.stbAdministration.restoreDefaultBoostedPhase();
	setSelection(pciViewContext.stbAdministration.getBoostImageIndex());
}

void StbBoostFrameSelectionState::apply()
{
	pciViewContext.stbAdministration.setBoostPhaseEnd(m_selectedImage);
    m_uiController.switchState(ViewState::StbContrastReviewState);
}

void StbBoostFrameSelectionState::cancel()
{
    m_uiController.switchState(ViewState::StbContrastReviewState);
}

void StbBoostFrameSelectionState::setSelection(int index)
{
	auto selected = std::find_if(m_referenceFrames.begin(), m_referenceFrames.end(), [index](std::shared_ptr<const StbImage> &image) {return image->imageNumber == index; });

	if (selected == m_referenceFrames.end()) return;

	m_selectedImage = (*selected)->imageNumber;
	pciViewContext.stbBoostSelectViewer.setSelection(m_selectedImage);

	getTsmPage().navigateRightButton->enabled = (m_selectedImage < (m_referenceFrames.back()->imageNumber - 1));
	getTsmPage().navigateLeftButton->enabled = (m_selectedImage > (m_referenceFrames.front()->imageNumber));
}

}}

