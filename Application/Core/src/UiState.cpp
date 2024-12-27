// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "UiState.h"
#include "PciViewContext.h"
#include "StbBoostViewer.h"
#include "StbMarkersViewer.h"
#include "StbRoiViewer.h"
#include "StbContrastViewer.h"
#include "StbImageSelectViewer.h"
#include "LicenseStatus.h"
#include "FrameGrabber.h"
#include "PACSArchiver.h"
#include "IArchivingStatus.h"
#include "ViewStateHelper.h"
#include "PciEpxSelectionView.h"
#include "PciXrayViewer.h"
#include "pciGuidanceView.h"
#include "PciReviewPlayer.h"
#include "PciTsmPage.h"
#include "CoRegistrationServerState.h"
#include "PciXrayPositionModel.h"
#include "CrmRoadmapSelectViewer.h"

using namespace Pci::Core;

UiState::UiState(PciViewContext &viewContext, ViewState state, const std::wstring &name, LicenseFeature feature) :
	viewState(state),
    stateName(name),
	pciViewContext(viewContext),
	m_frameGrabber(nullptr),
	m_pacsArchiver(nullptr),
	m_archivingStatus(nullptr),
	allowGeometryUpdates(true),
	autoReactivate(false),
	tsmPage(nullptr),
	m_feature(feature)
{
}


UiState::~UiState(void)
{
}

ViewState UiState::epxChanged()
{
	return getGuidanceStateByEpxSelection();
}

ViewState UiState::getGuidanceStateByEpxSelection() const
{
	// Clear iFR/FFR graphs if CoReg is in review mode
	if (pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Review)
	{
		pciViewContext.forceHideCoregControl = true;
	}

	if (pciViewContext.currentEpx.getType() == PciEpx::Type::Roadmap)
    {
        if (pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Pullback || 
            pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::SpotMeasurement)
        {
            // IFR measurement already started when EPX changes to Roadmap. Goto Co-reg state.
            return ViewState::CoRegistrationGuidanceState;
        }
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
    }
	else if (pciViewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
	{
		if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Live)
		{
			return ViewState::StbLiveGuidanceState;
		}
		else if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Subtract)
		{
			return ViewState::StbContrastGuidanceState;
		}
		return ViewState::StbPostDeployGuidanceState;
	}
	else // Non-PCI EPX
	{
		if (pciViewContext.crmAdminStatus != CrmAdministration::Status::NoRoadmap)
		{
			// If a Roadmap has been created, allow fluoro overlay for each EPX.
			return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
		}
		else
		{
			return ViewState::NonPciEpxState;
		}
	}
}

ViewState UiState::roadmapStatusChanged()
{
	return viewState;
}

ViewState UiState::angioProcessingStarted()
{
    return viewState;
}

ViewState UiState::angioProcessingStopped()
{
	return viewState;
}

ViewState UiState::overlayImageAvailable()
{
    return viewState;
}

ViewState UiState::lastImageHold()
{
	return viewState;
}

ViewState UiState::stentBoostImageAvailable(const std::shared_ptr<const XrayImage>& /*image*/, 
											const std::shared_ptr<const StbMarkers>& /*markers*/,
											const std::shared_ptr<const StbImage>& /*boost*/)
{
    return viewState;
}

ViewState  UiState::boostImageAvailable(const std::shared_ptr<const StbImage>& /*boost*/, const std::shared_ptr<const StbImage>& /*registeredLive*/)
{
	return viewState;
}

ViewState UiState::studyChanged()
{
	if (pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Pullback ||
		pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::SpotMeasurement ||
		pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Review)
	{
		pciViewContext.forceHideCoregControl = true;
        showCoRegControl(pciViewContext.coRegState);
	}

	if (pciViewContext.currentEpx.getType() == PciEpx::Type::Roadmap &&
		(pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Pullback ||
		 pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::SpotMeasurement))
	{
		// study changed during pullback/spot. Reset the view
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus, pciViewContext.roadmapErrorState);
	}
    return getGuidanceStateByEpxSelection();
}

bool UiState::isAutoReactivated() const
{
	return autoReactivate;
}

void UiState::resetAutoReactivate()
{
	autoReactivate = false;
}

std::wstring UiState::getStateName() const
{
    return stateName;
}

void UiState::show(Sense::Control *control)
{
    pciViewContext.crmGuidance		     .visible = (&pciViewContext.crmGuidance            == control);
    pciViewContext.crmViewer		     .visible = (&pciViewContext.crmViewer              == control);
	pciViewContext.crmRoadmapSelectViewer.visible = (&pciViewContext.crmRoadmapSelectViewer == control);
    pciViewContext.stbViewer		     .visible = (&pciViewContext.stbViewer              == control);
    pciViewContext.stbGuidance		     .visible = (&pciViewContext.stbGuidance            == control);
    pciViewContext.pciEpxSelection	     .visible = (&pciViewContext.pciEpxSelection        == control);
    pciViewContext.noConnection          .visible = (&pciViewContext.noConnection           == control);
	pciViewContext.noLicenseView         .visible = (&pciViewContext.noLicenseView          == control);
	pciViewContext.coRegViewer			 .visible = (&pciViewContext.coRegViewer			== control);
	pciViewContext.coRegistrationGuidance.visible = (&pciViewContext.coRegistrationGuidance == control);
	pciViewContext.stbRoiViewer			 .visible = (&pciViewContext.stbRoiViewer			== control);
	pciViewContext.stbContrastViewer     .visible = (&pciViewContext.stbContrastViewer      == control);
	pciViewContext.stbContrastSelectViewer.visible = (&pciViewContext.stbContrastSelectViewer == control);
    pciViewContext.stbBoostSelectViewer.visible = (&pciViewContext.stbBoostSelectViewer == control);

    pciViewContext.stbAngioViewer        .visible = pciViewContext.stbViewer.visible || pciViewContext.stbContrastViewer.visible;

	showCoRegControl(pciViewContext.coRegState);
}

void UiState::showCoRegControl(const CoReg::CoRegistrationServerState& coRegState)
{
	if( isPressureWireConnectedAndReady(coRegState) &&
		!pciViewContext.stbViewer.visible &&
		!pciViewContext.stbGuidance.visible &&
		!pciViewContext.noLicenseView.visible &&
		!pciViewContext.forceHideCoregControl &&
		(pciViewContext.currentEpx.getType() != PciEpx::Type::StentBoost))
	{
		pciViewContext.coRegistrationControl.visible = true;
	}
	else
	{
		pciViewContext.coRegistrationControl.visible = false;
	}
}

bool UiState::isPressureWireConnectedAndReady(const CoReg::CoRegistrationServerState& coRegState) const
{
	return (coRegState.normalized && 
			coRegState.wireConnected && 
			coRegState.pressurePlugConnected && 
			coRegState.pimConnected);
}

void UiState::initFrameGrabber(Sense::ICanvas& canvas, const Sense::Control& control1, const Sense::Control& control2)
{
	m_frameGrabber.reset(new PACS::FrameGrabber(canvas, control1, control2));
}

void UiState::initFrameGrabber(Sense::ICanvas& canvas, const Sense::Control& control)
{
	m_frameGrabber.reset(new PACS::FrameGrabber(canvas, control));

}

void UiState::initPACSArchiving(Sense::IScheduler& scheduler)
{
	if ((pciViewContext.pacsPush == nullptr) ||
		(pciViewContext.msgBuilder == nullptr) ||
		(pciViewContext.threadEncoding == nullptr))
	{
		return;
	}

	auto archiver = std::unique_ptr<PACS::PACSArchiver>(new PACS::PACSArchiver(scheduler, *pciViewContext.pacsPush, *pciViewContext.msgBuilder, *pciViewContext.threadEncoding));
	archiver->onSent = [this](PACS::GrabType grabType, bool finishedSuccessfully)
	{
		onFrameGrabberFinished(grabType, finishedSuccessfully);
	};

	m_pacsArchiver = std::move(archiver);
}

void UiState::initPACSArchiving(std::unique_ptr<PACS::IPACSArchiver> pacsArchiver)
{
	m_pacsArchiver = std::move(pacsArchiver);
}

void UiState::setArchivingStatusSink(IArchivingStatus* archivingStatus)
{
	m_archivingStatus = archivingStatus;
}

ViewState UiState::geometryChanged()
{
	if (allowGeometryUpdates)
	{
		getModel().currentGeometry = pciViewContext.geometry;
	}

	return viewState;
}

ViewState UiState::geometryEnd()
{
    return viewState;
}

ViewState UiState::apcStatusChanged()
{
	return viewState;
}

ViewState UiState::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
											  const CoReg::CoRegistrationServerState& nextState)
{
	if (nextState.systemState == CoReg::CoRegistrationServerState::SystemState::Live)
	{
		pciViewContext.forceHideCoregControl = false;
	}

	if ((nextState.systemState == CoReg::CoRegistrationServerState::SystemState::Freeze) || 
		(nextState.systemState == CoReg::CoRegistrationServerState::SystemState::Idle)  ) 
	{
		pciViewContext.forceHideCoregControl = true;
	}

	showCoRegControl(nextState);

	if ((prevState.systemState == CoReg::CoRegistrationServerState::SystemState::Live) &&
		((nextState.systemState == CoReg::CoRegistrationServerState::SystemState::Pullback) ||
		(nextState.systemState == CoReg::CoRegistrationServerState::SystemState::SpotMeasurement)))
	{
		return ViewState::CoRegistrationGuidanceState;
	}

	return viewState;
}

ViewState Pci::Core::UiState::coRegistrationError()
{
	return viewState;
}

ViewState Pci::Core::UiState::connectionStatusChanged(bool connected)
{
    if ( !connected )
    {
        return ViewState::NoConnection;
    }
    return viewState;
}

ViewState UiState::coRegistrationRunChanged(int /*runIndex*/)
{
	if((pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Review) && !pciViewContext.forceHideCoregControl)
	{
		// Hide the CoReg UI when any run is reviewed
		pciViewContext.forceHideCoregControl = true;
	}

	showCoRegControl(pciViewContext.coRegState);

	return viewState;
}

ViewState Pci::Core::UiState::coRegistrationConnectionChanged(bool connected)
{
	if (connected &&
		(pciViewContext.coRegState.systemState != CoReg::CoRegistrationServerState::SystemState::Idle) &&
		(pciViewContext.coRegState.systemState != CoReg::CoRegistrationServerState::SystemState::Freeze))
	{
		pciViewContext.forceHideCoregControl = false;
	}
	else if(!connected)
	{
		pciViewContext.forceHideCoregControl = true;
		return ViewStateHelper::DetermineCrmState(pciViewContext.crmAdminStatus,pciViewContext.roadmapErrorState);
	}

	return viewState;
}

ViewState UiState::imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType)
{
	if(pciViewContext.coRegState.systemState == CoReg::CoRegistrationServerState::SystemState::Review)
		pciViewContext.forceHideCoregControl = true;

	showCoRegControl(pciViewContext.coRegState);

	if (imageType == CommonPlatform::Xray::XrayImage::Type::Exposure)
	{
		if (pciViewContext.currentEpx.getType() == PciEpx::Type::Roadmap)
		{
			return ViewState::CrmProcessingAngioState;
		}

		if (pciViewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
		{
			if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Live)
			{
				return ViewState::StbLiveProcessingState;
			}
			else  if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::PostDeployment)
			{
				return ViewState::StbPostDeployProcessingState;
			}
			else  if (pciViewContext.currentEpx.getSubType() == PciEpx::SubType::Subtract)
			{
				return ViewState::StbContrastProcessingState;
			}
		}
	}

	return viewState;
}

ViewState UiState::endLive()
{
    return viewState;
}
