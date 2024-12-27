// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbProcessingState.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbTsmPage.h"
#include "FrameGrabber.h"
#include "MovieGrabber.h"
#include "PACSArchiver.h"
#include "PciReviewParameters.h"
#include "StbAdministration.h"
#include "UtilizationLogEvent.h"
#include "IArchivingStatus.h"
#include "ViewStateHelper.h"
#include "ViewType.h"
#include "Log.h"
#include "StbMarkers.h"
#include "StbImage.h"
#include "ICopyToRefClient.h"
#include "IArchivingSettings.h"
#include "Snapshot.h"
#include "Bitmap.h"
#include "IUiStateController.h"
#include "PciGuidanceView.h"
#include "Translation.h"
#include "stlstring.h"

#include <assert.h>

using namespace CommonPlatform;

namespace Pci {	namespace Core{

static const int MinimumRunLength = 6; // combination of init time of marker detection (4) and maxLostMarkerCount of StbAlgoBoost (6)
static const int MovieFrameRate = 15;

StbProcessingState::StbProcessingState(
	PciViewContext &viewContext,
	ViewState state,
	const std::wstring &name,
	const LicenseFeature license,
	IUiStateController& controller,
	Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
	const IArchivingSettings& archivingSettings)
		: UiState(viewContext, state, name, license),
		m_log(CommonPlatform::Logger(CommonPlatform::LoggerType::StentboostLive)),
		m_runActive(false),
		m_reviewPaused(false),
		m_viewType(pciViewContext.stbViewer.getViewType()),
		m_archivingDone(false),
		m_boostModel(),
		m_markerModel(),
		m_uiStateController(controller),
		_guidanceModel(),
		m_archivingSettings(archivingSettings)
{
    auto stbTsmPage = new StbTsmPage();
    stbTsmPage->copyToRefButton->visible = copyToRefClient.isAvailable();
	tsmPage.reset(stbTsmPage);
}

StbProcessingState::~StbProcessingState()
{
}

void StbProcessingState::onEnter()
{
	m_boostModel.currentGeometry = pciViewContext.geometry;

	pciViewContext.stbViewer.setXrayModel(m_boostModel);
	pciViewContext.stbAngioViewer.setXrayModel(m_markerModel);

	resetView();

	m_runActive = true;

	if (!m_uiStateController.isReplayActive())
	{
		show(&pciViewContext.stbViewer);
	}
	else
	{
		_guidanceModel.status = Localization::Translation::IDS_ProcessingStentBoost;
		_guidanceModel.showProcessingPanel = true;
		pciViewContext.stbGuidance.setGuidanceModel(_guidanceModel);
		show(&pciViewContext.stbGuidance);
	}

	auto refTargetName = getCopyToRefTargetName();
	auto refTargetNr = m_archivingSettings.getCopyToRefTargetNumber(m_log, refTargetName);
	auto targetNumber = refTargetNr == -1 ? L"" : std::to_wstring(refTargetNr);
	getTsmPage().copyToRefButton->centerText = str_ext::replace(Localization::Translation::IDS_CopyToReference, L"{0}", targetNumber);	
}

ViewState StbProcessingState::epxChanged()
{
	return viewState;
}

ViewState StbProcessingState::stentBoostImageAvailable(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, 
														const std::shared_ptr<const StbMarkers>& markers,
														const std::shared_ptr<const StbImage>& boost)
{
	if (boost && (boost->isValid() || boost->showWarning)) 
	{
		pciViewContext.stbViewer.setImage(boost);
	}

	pciViewContext.stbAngioViewer.setImage(image);
	pciViewContext.stbAngioViewer.setMarkers(markers);
	
	pciViewContext.stbAdministration.setImage(image, markers, boost);

	m_markerModel.showLastImageHold = false;
	return viewState;
}

ViewState StbProcessingState::boostImageAvailable(const std::shared_ptr<const StbImage>& boost, const std::shared_ptr<const StbImage>& registeredLive)
{
	pciViewContext.stbAdministration.setBoostImage(boost, registeredLive);

	pciViewContext.stbViewer.setImage(boost);
	m_markerModel.showLastImageHold = false;
	return viewState;
}

ViewState StbProcessingState::imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType)
{
	UiState::imageRunStarted(imageType);

	if (imageType != CommonPlatform::Xray::XrayImage::Type::Fluoro)
	{
		m_runActive = true;
	}
	return viewState;
}

bool StbProcessingState::isExposerRunTooShort() const
{
	return (pciViewContext.stbAdministration.isEmpty() ||
		   (pciViewContext.stbAdministration.lastImageIndex() < MinimumRunLength));
}

void StbProcessingState::setRunActive(bool active)
{
	m_runActive = active;
}

StbTsmPage& StbProcessingState::getTsmPage() const
{
	return *static_cast<StbTsmPage*>(tsmPage.get());
}

ViewState StbProcessingState::overlayImageAvailable()
{
	return viewState;
}

ViewState StbProcessingState::lastImageHold()
{
	m_log.DeveloperInfo(L"StbProcessingState::lastImageHold");

	if (!m_runActive)
	{
		if (isExposerRunTooShort())
		{
			return getGuidanceState();
		}

		return getReviewState();
	}

	return viewState;
}

ViewState StbProcessingState::endLive()
{
	m_log.DeveloperInfo(L"StbProcessingState::endlive");
	setRunActive(false);

	return viewState;
}


ViewState StbProcessingState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState, const CoReg::CoRegistrationServerState& nextState)
{
	UiState::coRegistrationStateChanged(prevState, nextState);
	return viewState;
}

ViewState StbProcessingState::coRegistrationConnectionChanged(bool connected)
{
	UiState::coRegistrationConnectionChanged(connected);
	return viewState;
}

void StbProcessingState::resetView()
{
	pciViewContext.stbViewer.totalFrames = 0;
	pciViewContext.stbViewer.currentFrame = 0;
	pciViewContext.stbViewer.showRunInfo = false;
	pciViewContext.stbViewer.setImage(std::make_shared<const StbImage>());
    pciViewContext.stbAngioViewer.setImage(std::make_shared<const XrayImage>());

	m_boostModel.showLastImageHold = false;
	m_markerModel.showLastImageHold = false;
	allowGeometryUpdates = true;
}

void StbProcessingState::onExit()
{
}

PciXrayPositionModel& StbProcessingState::getModel()
{
	return m_boostModel;
}

PciXrayModel & StbProcessingState::getBoostModel()
{
	return m_boostModel;
}

PciXrayModel & StbProcessingState::getMarkerModel()
{
	return m_markerModel;
}

}}

