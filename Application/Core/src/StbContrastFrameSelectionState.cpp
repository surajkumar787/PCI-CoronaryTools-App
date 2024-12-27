// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastFrameSelectionState.h"
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

StbContrastFrameSelectionState::StbContrastFrameSelectionState(PciViewContext &viewContext, IUiStateController& uiController, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient) :
	UiState(viewContext, ViewState::StbContrastFrameSelectionState, L"StbContrastFrameSelectionState", LicenseFeature::StbSubtract),
	m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive)),
	m_uiController(uiController)
{
    auto stbTsmPage = new StbImageSelectTsmPage();
    stbTsmPage->copyToRefButton->visible = copyToRefClient.isAvailable();
    tsmPage.reset(stbTsmPage);

	tsmPage->eventButtonReleased = [this](unsigned int button)
	{
		m_log.DeveloperInfo(L"StbContrastFrameSelectionState: TSM button %d released", button);

		auto callbackIt = m_tsmButtonCallbacks.find(button);
		if (callbackIt != m_tsmButtonCallbacks.end())
		{
			callbackIt->second();
		}
	};

	m_tsmButtonCallbacks[StbImageSelectTsmPage::NavigateLeftButton] = std::bind(&StbContrastFrameSelectionState::onNavigateLeftDown, this);
	m_tsmButtonCallbacks[StbImageSelectTsmPage::NavigateRightButton] = std::bind(&StbContrastFrameSelectionState::onNavigateRightDown, this);
	m_tsmButtonCallbacks[StbImageSelectTsmPage::ResetDefaultButton] = std::bind(&StbContrastFrameSelectionState::onResetDefaultDown, this);
    m_tsmButtonCallbacks[StbImageSelectTsmPage::ApplyButton] = std::bind(&StbContrastFrameSelectionState::apply, this);
    m_tsmButtonCallbacks[StbTsmPage::ContrastFrameSelectButton] = std::bind(&StbContrastFrameSelectionState::cancel, this);

    enableContrastImageSelectionButton();
}

StbContrastFrameSelectionState::~StbContrastFrameSelectionState()
{
}

void StbContrastFrameSelectionState::enableContrastImageSelectionButton()
{
    getTsmPage().contrastFrameSelectButton->visible = true;
    getTsmPage().contrastFrameSelectButton->enabled = true;
    getTsmPage().contrastFrameSelectButton->selected = true;

    getTsmPage().boostFrameSelectButton->visible = true;
    getTsmPage().defineRoiButton->visible = true;
    getTsmPage().invertBoostButton->visible = true;

}

void StbContrastFrameSelectionState::onEnter()
{
	pciViewContext.stbBoostSelectViewer.guidanceTextLabel.text = Localization::Translation::IDS_UseTSMToSelectDifferentImage;

	m_contrastFrames.clear();

	m_stbRecording = pciViewContext.stbAdministration.prepareReview(false);
	if (m_stbRecording != nullptr)
	{
		auto contrastImages = m_stbRecording->getAllContrastImages();
			
		for (const auto& contrastImage : contrastImages)
		{
            if (contrastImage.registeredLive->isValid())
            {
                m_contrastFrames.push_back(contrastImage.registeredLive);
            }
		}

		if (!m_contrastFrames.empty())
		{

			pciViewContext.stbContrastSelectViewer.setFrames(m_contrastFrames);
			setSelection(m_stbRecording->getContrastImage().boost->imageNumber);

			show(&pciViewContext.stbContrastSelectViewer);
		}
	}

	autoReactivate = false;
}

StbImageSelectTsmPage& StbContrastFrameSelectionState::getTsmPage() const
{
	return *static_cast<StbImageSelectTsmPage*>(tsmPage.get());
}

PciXrayPositionModel& StbContrastFrameSelectionState::getModel()
{
	return m_boostModel;
}

ViewState StbContrastFrameSelectionState::epxChanged()
{
	return viewState;
}

ViewState StbContrastFrameSelectionState::overlayImageAvailable()
{
	autoReactivate = true;

	if (pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		return ViewState::CrmOverlayState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
}

void StbContrastFrameSelectionState::onNavigateLeftDown()
{
	auto currentSelected = m_selectedImage;	
    auto selection = std::find_if(m_contrastFrames.begin(), m_contrastFrames.end(), [currentSelected](std::shared_ptr<const StbImage> &image) {return image->imageNumber == currentSelected; });

    if (selection != m_contrastFrames.begin())
    {
        selection--;
        setSelection((*selection)->imageNumber);
    }
}

void StbContrastFrameSelectionState::onNavigateRightDown()
{
	auto currentSelected = m_selectedImage;
	auto selection = std::find_if(m_contrastFrames.begin(), m_contrastFrames.end(), [currentSelected](std::shared_ptr<const StbImage> &image) {return image->imageNumber == currentSelected; });

    selection++;

	if ((selection != m_contrastFrames.end()))
	{
		setSelection((*selection)->imageNumber);
	}
}

void StbContrastFrameSelectionState::onResetDefaultDown()
{
	setSelection(m_stbRecording->getAutoDetectedContrastFrameIndex());
}

void StbContrastFrameSelectionState::apply() 
{
    m_stbRecording->setManualContrastFrame(m_selectedImage);
    m_uiController.switchState(ViewState::StbContrastReviewState);
}

void StbContrastFrameSelectionState::cancel()
{
    m_uiController.switchState(ViewState::StbContrastReviewState);
}

void StbContrastFrameSelectionState::setSelection(int index)
{
	auto selected = std::find_if(m_contrastFrames.begin(), m_contrastFrames.end(), [index](std::shared_ptr<const StbImage> &image) {return image->imageNumber == index; });

	if (selected == m_contrastFrames.end()) return;

	m_selectedImage = (*selected)->imageNumber;
	pciViewContext.stbContrastSelectViewer.setSelection(m_selectedImage);

	getTsmPage().navigateRightButton->enabled = (m_selectedImage < (m_contrastFrames.back()->imageNumber));
	getTsmPage().navigateLeftButton->enabled = (m_selectedImage > (m_contrastFrames.front()->imageNumber));
}

}}

