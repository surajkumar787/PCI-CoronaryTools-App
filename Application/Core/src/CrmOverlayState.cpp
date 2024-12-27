// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmOverlayState.h"
#include "PciViewContext.h"
#include "PciXrayViewer.h"
#include "CrmTsmPage.h"
#include "IArchivingStatus.h"
#include "UtilizationLogEvent.h"
#include "FrameGrabber.h"
#include "PACSArchiver.h"
#include "ViewStateHelper.h"
#include "IUiStateController.h"

#include "CrmMovieGrabber.h"

#include <assert.h>
using namespace Pci::Core::PACS;
using namespace CommonPlatform;
namespace Pci { namespace Core {

enum class RecordingStatus
{
	None,
	ExportRequested
};

CrmOverlayState::CrmOverlayState(PciViewContext &viewContext, IUiStateController& uiStateController) :
	UiState(viewContext, ViewState::CrmOverlayState, L"CrmOverlayState", LicenseFeature::Roadmap),
	m_log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap))
{
	auto* pciTsmPage = new CrmTsmPage();
	pciTsmPage->storeSnapshotButton->enabled = (pciViewContext.pacsPush != nullptr) && pciViewContext.snapshotsAvailable;
	pciTsmPage->storeMovieButton->enabled = (pciViewContext.pacsPush != nullptr) && pciViewContext.moviesAvailable;
	pciTsmPage->roadmapOptionsButton->enabled = false;

	pciTsmPage->eventButtonReleased = [this, &uiStateController](unsigned int button)
	{
		if (button == PciTsmPage::SnapshotButton)
		{
			if (!m_frameGrabber)
			{
				initFrameGrabber(pciViewContext.crmViewer.getCanvas(), pciViewContext.crmViewer);
			}

			if (!m_pacsArchiver)
			{
				initPACSArchiving(pciViewContext.crmViewer.getCanvas().getScheduler());
			}

			if (m_frameGrabber && m_pacsArchiver)
			{
				if (m_archivingStatus != nullptr)
				{
					m_archivingStatus->setSnapshotStatus(ArchivingStatus::Busy, CaptureArea::MainView);
				}
				
				auto frame = m_frameGrabber->grabSingleFrame();
				m_pacsArchiver->sendSnapshot(frame, pciViewContext.crmViewer.getImage()->seriesNumber,RunType::DCR);

				::UtilizationLogEvent::ArchiveSnapshotCrm(m_log);
			}
		}
		if (button == PciTsmPage::MovieButton)
		{
			if (m_movieRecordingStatus == RecordingStatus::None)
			{
				recordMovie();
			}

			getTsmPage().storeMovieButton->enabled = false;
		}

		if (button == CrmTsmPage::RoadmapSelectionButton)
		{
			uiStateController.switchState(ViewState::CrmRoadmapActive);
		}
	};

	tsmPage.reset(pciTsmPage);
}

CrmOverlayState::~CrmOverlayState()
{
}

PciXrayPositionModel& CrmOverlayState::getModel()
{
	return model;
}

CrmTsmPage& CrmOverlayState::getTsmPage() const
{
	return *static_cast<CrmTsmPage*>(tsmPage.get());
}

void CrmOverlayState::onEnter()
{
	// only reset the last image hold when we are about to be shown
	if (!pciViewContext.crmViewer.visible)
	{
		model.showLastImageHold = false;
		allowGeometryUpdates = true;
	}

	if (((pciViewContext.pacsPush != nullptr) && (m_archivingStatus != nullptr)) && 
		(!pciViewContext.crmViewer.visible && pciViewContext.snapshotsAvailable))
	{
		m_archivingStatus->setSnapshotStatus(ArchivingStatus::Ready, CaptureArea::MainView);
	}

	model.currentGeometry = pciViewContext.geometry;
	pciViewContext.crmViewer.setXrayModel(model);

	show(&pciViewContext.crmViewer);
}

void CrmOverlayState::onFrameGrabberFinished(PACS::GrabType grabType, bool finishedSuccessfully)
{
	if (m_archivingStatus == nullptr) return;

	if (grabType == PACS::GrabType::SingleFrame)
	{
		auto status = finishedSuccessfully ? ArchivingStatus::Success : ArchivingStatus::Failure;
		m_archivingStatus->setSnapshotStatus(status, CaptureArea::MainView);
	}
	else
	{
		assert(false);
	}
}

ViewState CrmOverlayState::roadmapStatusChanged()
{
	if(pciViewContext.crmAdminStatus == CrmAdministration::Status::Active)
	{
		return viewState;
	}

	return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
}

ViewState CrmOverlayState::angioProcessingStarted()
{
	return ViewState::CrmProcessingAngioState;
}

ViewState CrmOverlayState::overlayImageAvailable()
{
	UiState::overlayImageAvailable();

	model.showLastImageHold = false;
	allowGeometryUpdates = true;
	getTsmPage().storeMovieButton->enabled = false;
	getTsmPage().roadmapOptionsButton->enabled = false;
	getTsmPage().requestUpdate();
	return viewState;
}

ViewState CrmOverlayState::lastImageHold()
{
	model.showLastImageHold = true;
	allowGeometryUpdates = false;
	getTsmPage().storeMovieButton->enabled = (pciViewContext.pacsPush != nullptr) && pciViewContext.moviesAvailable;

    auto roadmaps = pciViewContext.crmAdmin.getRoadmapsForCurrentGeo();
	getTsmPage().roadmapOptionsButton->enabled = true;
	getTsmPage().requestUpdate();

	m_movieRecordingStatus = RecordingStatus::None;

	return viewState;
}

ViewState CrmOverlayState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
													  const CoReg::CoRegistrationServerState& nextState)
{
	auto newState =  UiState::coRegistrationStateChanged(prevState,nextState);

	if (!model.showLastImageHold && 
	   (prevState.systemState == CoReg::CoRegistrationServerState::SystemState::Live) &&
	   ((nextState.systemState == CoReg::CoRegistrationServerState::SystemState::Pullback) ||
		(nextState.systemState == CoReg::CoRegistrationServerState::SystemState::SpotMeasurement)))
	{
		return ViewState::CoRegistrationState;
	}

	return newState;
}

void CrmOverlayState::initMovieGrabber()
{
	if (m_movieGrabber) return;
	if (!pciViewContext.moviesAvailable) return;

	if (pciViewContext.pacsPush != nullptr && pciViewContext.msgBuilder != nullptr && pciViewContext.threadEncoding != nullptr)
	{
		m_movieGrabber.reset(new PACS::CrmMovieGrabber(pciViewContext.crmViewer.getCanvas().getScheduler(), *pciViewContext.threadEncoding, *pciViewContext.pacsPush, *pciViewContext.msgBuilder, pciViewContext.crmViewer.getViewType()));
	}

	if (m_movieGrabber == nullptr) return;

	m_movieGrabber->onGrabbingDone = [this](std::shared_ptr<PACS::ImageSequence> movie)
	{
		m_movie = std::move(movie);
		if (m_movie != nullptr)
		{
			sendMovie();
		}
		else
		{
			m_log.DeveloperError(L"CrmOverlayState::onGrabbingDone: movie grabbing failed, unable to export movie");
			if (m_archivingStatus)
			{
				m_archivingStatus->setMovieStatus(ArchivingStatus::Failure);
			}
		}
	};

	m_movieGrabber->onSent = [this](bool success, const std::shared_ptr<const PACS::ImageSequence>& movie)
	{
		onMovieGrabberFinished(success, movie);
	};
}


void CrmOverlayState::recordMovie()
{
	initMovieGrabber();
	if (m_movieGrabber == nullptr) return;

	if (m_archivingStatus != nullptr)
	{
		m_archivingStatus->setMovieStatus(ArchivingStatus::Busy);
	}

	m_movieRecordingStatus = RecordingStatus::ExportRequested;

	auto recording = pciViewContext.crmAdmin.getRecording();
	m_movieGrabber->grabMovie(recording, 15, pciViewContext.crmViewer.size);
}

void CrmOverlayState::sendMovie()
{
	if (m_movieGrabber == nullptr) return;

	if (m_archivingStatus != nullptr)
	{
		m_archivingStatus->setMovieStatus(ArchivingStatus::Busy);
	}

	m_movieGrabber->sendMovie(m_movie);
	CommonPlatform::UtilizationLogEvent::ArchiveMovieSbl(m_log);
}

void CrmOverlayState::onMovieGrabberFinished(bool finishedSuccessfully, const std::shared_ptr<const PACS::ImageSequence>& movie)
{
	if (m_archivingStatus != nullptr)
	{
		auto status = finishedSuccessfully ? ArchivingStatus::Success : ArchivingStatus::Failure;
		m_archivingStatus->setMovieStatus(status);
	}

	if (!finishedSuccessfully && m_movie == movie)
	{
		getTsmPage().storeMovieButton->enabled = true;
		getTsmPage().requestUpdate();
	}
}

}}