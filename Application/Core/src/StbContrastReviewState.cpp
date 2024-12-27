// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbContrastReviewState.h"

#include "PciViewContext.h"
#include "StbContrastViewer.h"
#include "StbMarkersViewer.h"
#include "StbTsmPage.h"
#include "FrameGrabber.h"
#include "MovieGrabberStbSubtract.h"
#include "PciReviewParameters.h"
#include "StbAdministration.h"
#include "UtilizationLogEvent.h"
#include "IArchivingStatus.h"
#include "ViewStateHelper.h"
#include "ViewType.h"
#include "Log.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "StbTsmPage.h"
#include "IUiStateController.h"
#include "IArchivingSettings.h"
#include "IUserPrefs.h"
#include "StbAlgoBoost.h"
#include "TsmButtonDefinitions.h"
#include "Translation.h"

using namespace CommonPlatform;

namespace Pci {	namespace Core{


StbContrastReviewState::StbContrastReviewState(PciViewContext &viewContext, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, IUserPrefs& userPrefs, IUiStateController& uiController) :
	StbReviewState(viewContext, ViewState::StbContrastReviewState, L"StbContrastReviewState", copyToRefClient, userPrefs.getArchivingSettings(), LicenseFeature::StbSubtract),
	m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive)),
	m_viewType(pciViewContext.stbContrastViewer.getViewType()),
	m_switch_away(false),
	m_archivingDone(false),
	m_tsmButtonCallbacks(),
	m_uiController(uiController),
	m_fadeInOutPaused(false),
	m_defaultBoostInverted(userPrefs.getDefaultShowInvertedStent()),
	m_boostInverted(m_defaultBoostInverted)
{

	m_player.eventReviewImage = [this](int /*index*/, const StbResult& frame, bool /*lastFrameReached*/)
	{
		pciViewContext.stbAngioViewer.setImage(frame.live);
		pciViewContext.stbAngioViewer.setMarkers(frame.markers);
	};


	getTsmPage().toContrastButton->enabled = false;
	getTsmPage().toBoostButton->enabled = false;
	getTsmPage().stopStartButton->enabled = false;

	getTsmPage().defineRoiButton->visible = true;
	getTsmPage().toBoostButton->visible = true;
	getTsmPage().toContrastButton->visible = true;
    getTsmPage().invertBoostButton->visible = true;

	getTsmPage().defineRoiButton->enabled = false;

    getTsmPage().contrastFrameSelectButton->visible = true;
    getTsmPage().contrastFrameSelectButton->enabled = false;

    getTsmPage().boostFrameSelectButton->visible = true;
    getTsmPage().boostFrameSelectButton->enabled = false;

	getTsmPage().stopStartButton->visible = true;

	auto baseEventHandler = tsmPage->eventButtonReleased;
	tsmPage->eventButtonReleased = [this, baseEventHandler](unsigned int button)
	{
		if (baseEventHandler) baseEventHandler(button);

		if (button == StbTsmPage::DefineROIButton)
		{
			m_uiController.switchState(ViewState::StbContrastRoiDefinitionState);
		}

        if (button == StbTsmPage::ContrastFrameSelectButton)
        {
            m_uiController.switchState(ViewState::StbContrastFrameSelectionState);
        }

        if (button == StbTsmPage::BoostFrameSelectButton)
        {
            m_uiController.switchState(ViewState::StbBoostFrameSelectionState);
        }

        if (button == StbTsmPage::StopStartButton)
		{
			if (m_fadeInOutPaused)
			{
				resumeFading();
			}
			else
			{
				pauseFading();
			}
		}

		if (button == StbTsmPage::ToBoostButton)
		{
			pauseFading();
			toBoostFrame();
		}

		if (button == StbTsmPage::ToContrastButton)
		{
			pauseFading();
			toContrastFrame();
		}

		if (button == StbTsmPage::InvertBoostButton)
		{
			invertBoostImage();
			getTsmPage().storeSnapshotButton->enabled = pciViewContext.snapshotsAvailable;
			getTsmPage().storeMovieButton->enabled = pciViewContext.moviesAvailable;
		}
	};
}

StbContrastReviewState::~StbContrastReviewState()
{
}

void StbContrastReviewState::onEnter()
{
	show(&pciViewContext.stbContrastViewer);

	StbReviewState::onEnter();

	m_boostModel.showLastImageHold = true;
	m_fadeInOutPaused = false;
	m_boostInverted = m_defaultBoostInverted;

	getTsmPage().defineRoiButton->enabled = true;
	getTsmPage().defineRoiButton->visible = true;
	getTsmPage().invertBoostButton->visible = true;
	getTsmPage().invertBoostButton->selected = false;

	if (hasBoostedImages())
	{
		m_player.setReview(m_stbRecording, m_stbRecording->getAllImages().front().live->frameTime);

		auto hasContrastImage = pciViewContext.stbContrastViewer.hasContrastImage();

		getTsmPage().storeMovieButton->enabled &= hasContrastImage;
		getTsmPage().contrastFrameSelectButton->enabled = hasContrastImage;
		getTsmPage().toContrastButton->enabled = hasContrastImage;
		getTsmPage().toBoostButton->enabled = hasContrastImage;
		getTsmPage().stopStartButton->enabled = hasContrastImage;
		getTsmPage().boostFrameSelectButton->enabled = true;
		getTsmPage().invertBoostButton->enabled = true;

		pciViewContext.stbContrastViewer.showRunInfo = true;

		resumeFading();

		m_player.startReview();
	}
	else
	{
		getTsmPage().invertBoostButton->enabled = false;
		getTsmPage().storeMovieButton->enabled = false;
		getTsmPage().contrastFrameSelectButton->enabled = false;
		getTsmPage().boostFrameSelectButton->enabled = false;
		getTsmPage().toContrastButton->enabled = false;
		getTsmPage().toBoostButton->enabled = false;
		getTsmPage().stopStartButton->enabled = false;
	}

	getTsmPage().requestUpdate();
}

void StbContrastReviewState::onExit()
{
	m_player.stopReview();

	pciViewContext.stbContrastViewer.clearImage();
}

void StbContrastReviewState::prepareReview()
{
	auto boostedFrame = m_stbRecording->getLastBoostedImage();
	auto contrastFrame = m_stbRecording->getContrastImage();

	auto& contrastViewer = pciViewContext.stbContrastViewer;

	contrastViewer.setXrayModel(m_boostModel);

	contrastViewer.setImages(boostedFrame.boost, contrastFrame.registeredLive);
	updateBoostFrame();

	contrastViewer.showRunInfo = true;

	m_stbRecording->setIncludeImagesWithoutMarkers(true);

	contrastViewer.totalFrames = m_stbRecording->getImageCount();
	contrastViewer.boostedFrames = m_stbRecording->getBoostedImageCount();
	contrastViewer.showBoostedFrameCounter = true;

	m_boostModel.showLastImageHold = true;
	allowGeometryUpdates = false;

	m_stbRecording->setIncludeImagesWithoutMarkers(false);

	m_log.DeveloperInfo(L"StbContrastReviewState::prepareReview");
}

void StbContrastReviewState::sendSnapshotToRefViewer()
{
	const auto currentBlend = pciViewContext.stbContrastViewer.getBlendRatio();

	if (!m_fadeInOutPaused)
	{
		pciViewContext.stbContrastViewer.setFixedBlend(SNAPSHOT_BLEND_RATIO);
	}
	
	pciViewContext.stbContrastViewer.showRunInfo = false;
	StbReviewState::sendSnapshotToRefViewer();
	pciViewContext.stbContrastViewer.showRunInfo = true;

	if (!m_fadeInOutPaused)
	{
		pciViewContext.stbContrastViewer.setFixedBlend(currentBlend);
		pciViewContext.stbContrastViewer.startAutoBlend();
	}
}

void StbContrastReviewState::sendSnapshot()
{
	auto& contrastViewer = pciViewContext.stbContrastViewer;

	const auto currentBlend = contrastViewer.getBlendRatio();

	if (!m_fadeInOutPaused)
	{
		contrastViewer.setFixedBlend(1.0f);
		contrastViewer.showRunInfo = true;

		StbReviewState::sendSnapshot();

		if (contrastViewer.hasContrastImage())
		{
			pciViewContext.stbContrastViewer.setFixedBlend(SNAPSHOT_BLEND_RATIO);
			StbReviewState::sendSnapshot();

			contrastViewer.setFixedBlend(currentBlend);
			contrastViewer.startAutoBlend();
		}
	}
	else
	{
		StbReviewState::sendSnapshot();
	}
}

void StbContrastReviewState::recordMovie()
{
	if (pciViewContext.stbContrastViewer.hasContrastImage())
	{
		StbReviewState::recordMovie();
	}
}

void StbContrastReviewState::initFrameGrabber()
{
	UiState::initFrameGrabber(pciViewContext.stbContrastViewer.getCanvas(), pciViewContext.stbContrastViewer);
}

std::unique_ptr<PACS::MovieGrabber> StbContrastReviewState::createMovieGrabber()
{
	std::unique_ptr<PACS::MovieGrabber> moviegrabber(nullptr);

	if (pciViewContext.pacsPush != nullptr && pciViewContext.msgBuilder != nullptr && pciViewContext.threadEncoding != nullptr)
	{
		moviegrabber = std::make_unique<PACS::MovieGrabberStbSubtract>(pciViewContext.stbContrastViewer.getCanvas().getScheduler(), *pciViewContext.threadEncoding, *pciViewContext.pacsPush, *pciViewContext.msgBuilder, pciViewContext.stbContrastViewer.getViewType(), m_boostInverted);
	}

	return std::move(moviegrabber);
}

StbTsmPage& StbContrastReviewState::getTsmPage() const
{
	return *static_cast<StbTsmPage*>(tsmPage.get());
}

void StbContrastReviewState::pauseFading()
{
	getTsmPage().stopStartButton->icon = TSM::ButtonIcon::MoviePlay;
	getTsmPage().storeSnapshotButton->enabled = pciViewContext.snapshotsAvailable;
	getTsmPage().requestUpdate();
	pciViewContext.stbContrastViewer.pauseAutoBlend();

	m_fadeInOutPaused = true;
}

void StbContrastReviewState::resumeFading()
{
	getTsmPage().stopStartButton->icon = TSM::ButtonIcon::MovieStop;
	getTsmPage().requestUpdate();

	pciViewContext.stbContrastViewer.startAutoBlend();

	m_fadeInOutPaused = false;
}

void StbContrastReviewState::invertBoostImage()
{
	m_boostInverted = !m_boostInverted;

	updateBoostFrame();

	getTsmPage().requestUpdate();

}

void StbContrastReviewState::updateBoostFrame()
{
	std::shared_ptr<const StbImage> boost;
	auto images = m_stbRecording->getLastBoostedImage();

	if (m_boostInverted)
	{
		if (images.boost)
		{
			boost = images.boost->invert();
		}
	}
	else
	{
		boost = images.boost;
	}
	if (boost) pciViewContext.stbContrastViewer.setBoostImage(boost);
}


bool StbContrastReviewState::isCopyToRefEnabled() const
{
	return m_archivingSettings.isAutoCopyToRefEnabled(IArchivingSettings::Feature::StentBoostSubtract);
}

std::wstring StbContrastReviewState::getCopyToRefTargetName() const
{
	return m_archivingSettings.getCopyToRefViewer(IArchivingSettings::Feature::StentBoostSubtract);
}

bool StbContrastReviewState::isAutoPACSArchivingMovieEnabled() const
{
	return m_archivingSettings.isAutoPACSArchiveMovieEnabled(IArchivingSettings::Feature::StentBoostSubtract);
}

bool StbContrastReviewState::isAutoPACSArchivingSnapshotEnabled() const
{
	return m_archivingSettings.isAutoPACSArchiveSnapshotEnabled(IArchivingSettings::Feature::StentBoostSubtract);
}

void StbContrastReviewState::toBoostFrame()
{
	pciViewContext.stbContrastViewer.setFixedBlend(1.0);
}

void StbContrastReviewState::toContrastFrame()
{
	pciViewContext.stbContrastViewer.setFixedBlend(0.0);
}

const double StbContrastReviewState::SNAPSHOT_BLEND_RATIO = 0.4;

}}