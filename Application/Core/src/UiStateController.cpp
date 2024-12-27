// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "UiStateController.h"
#include "CrmCreationGuidanceState.h"
#include "CrmProcessingAngioState.h"
#include "CrmErrorGuidanceState.h"
#include "CrmTableGuidanceState.h"
#include "CrmApcGuidanceState.h"
#include "CrmRoadmapActive.h"
#include "CrmOverlayState.h"
#include "NonPciEpxState.h"
#include "StbLiveGuidanceState.h"
#include "StbLiveReviewState.h"
#include "StbPostDeployGuidanceState.h"
#include "StbContrastGuidanceState.h"
#include "StbPostDeployRoiDefinitionState.h"
#include "StbContrastRoiDefinitionState.h"
#include "StbLiveProcessingState.h"
#include "StbPostDeployProcessingState.h"
#include "StbPostDeployReviewState.h"
#include "StbContrastProcessingState.h"
#include "StbContrastReviewState.h"
#include "StbContrastFrameSelectionState.h"
#include "StbBoostFrameSelectionState.h"
#include "CoRegistrationState.h"
#include "CoRegistrationGuidanceState.h"
#include "NoConnectionState.h"
#include "NoLicenseState.h"
#include <Sense/Common/Timer.h>
#include <assert.h>
#include "Log.h"
#include "UtilizationLogEvent.h"
#include <string>
#include <typeinfo>
#include "ITsmSource.h"
#include "PciTsmPage.h"
#include "IArchivingStatus.h"
#include "IUserPrefs.h"
#include "LicenseStatus.h"
#include "LicensesCollection.h"
#include "IReplayControl.h"

namespace Pci { namespace Core {
using namespace CommonPlatform; 
// this delay should be 2 seconds according to the design. It is set to 1 second because there is a 1 second delay between releasing the fluoro pedal
// and the application receiving an end of run event (triggering the timer).
const double SwitchDelayInSeconds = 1.0;

UiStateController::UiStateController(PciViewContext &viewContext, Sense::IScheduler &scheduler, TSM::ITsmSource& tsmSource, IReplayControl& replayControl, iControlInterface::ICopyToRefClient& copyToRefClient, IUserPrefs& userPrefs, IArchivingStatus* archivingStatus) :
	m_pciViewContext(viewContext),
	m_tsm(tsmSource),
	m_archivingStatus(archivingStatus),
	m_replayControl(replayControl),
	m_autoSwitchTimer(scheduler, [this]{ reactivatePreviousState(); }),
	m_currentUiState(nullptr),
	m_autoReactivateState(nullptr),
	m_autoSwitchDelay(SwitchDelayInSeconds),
	m_log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform))
{
	const IArchivingSettings& archivingSettings         = userPrefs.getArchivingSettings();

	m_states[ViewState::CrmCreationGuidanceState]		= std::unique_ptr<UiState>(new CrmCreationGuidanceState(viewContext));
	m_states[ViewState::CrmProcessingAngioState]		= std::unique_ptr<UiState>(new CrmProcessingAngioState(viewContext));
	m_states[ViewState::CrmErrorGuidanceState]			= std::unique_ptr<UiState>(new CrmErrorGuidanceState(viewContext, *this));
	m_states[ViewState::CrmTableGuidanceState]			= std::unique_ptr<UiState>(new CrmTableGuidanceState(viewContext));
	m_states[ViewState::CrmApcGuidanceState]			= std::unique_ptr<UiState>(new CrmApcGuidanceState(viewContext));
	m_states[ViewState::CrmRoadmapActive]				= std::unique_ptr<UiState>(new CrmRoadmapActive(viewContext));
	m_states[ViewState::CrmOverlayState]				= std::unique_ptr<UiState>(new CrmOverlayState(viewContext, *this));
	m_states[ViewState::NonPciEpxState]					= std::unique_ptr<UiState>(new NonPciEpxState(viewContext));
	m_states[ViewState::StbLiveGuidanceState]			= std::unique_ptr<UiState>(new StbLiveGuidanceState(viewContext, copyToRefClient, archivingSettings));
	m_states[ViewState::StbPostDeployGuidanceState]     = std::unique_ptr<UiState>(new StbPostDeployGuidanceState(viewContext, copyToRefClient, archivingSettings));
	m_states[ViewState::StbContrastGuidanceState]       = std::unique_ptr<UiState>(new StbContrastGuidanceState(viewContext, copyToRefClient, archivingSettings));
	m_states[ViewState::StbLiveProcessingState]			= std::unique_ptr<UiState>(new StbLiveProcessingState(viewContext, *this, copyToRefClient, archivingSettings));
	m_states[ViewState::StbLiveReviewState]				= std::unique_ptr<UiState>(new StbLiveReviewState(viewContext, copyToRefClient, archivingSettings));
	m_states[ViewState::StbPostDeployProcessingState]	= std::unique_ptr<UiState>(new StbPostDeployProcessingState(viewContext, *this, copyToRefClient, archivingSettings));
	m_states[ViewState::StbPostDeployReviewState]		= std::unique_ptr<UiState>(new StbPostDeployReviewState(viewContext, *this, copyToRefClient, archivingSettings));
	m_states[ViewState::StbPostDeployRoiDefinitionState] = std::unique_ptr<UiState>(new StbPostDeployRoiDefinitionState(viewContext, *this, copyToRefClient, archivingSettings));
	m_states[ViewState::StbContrastProcessingState]     = std::unique_ptr<UiState>(new StbContrastProcessingState(viewContext, *this, copyToRefClient, archivingSettings));
	m_states[ViewState::StbContrastReviewState]         = std::unique_ptr<UiState>(new StbContrastReviewState(viewContext, copyToRefClient, userPrefs, *this));
	m_states[ViewState::StbContrastRoiDefinitionState]  = std::unique_ptr<UiState>(new StbContrastRoiDefinitionState(viewContext, *this, copyToRefClient, archivingSettings));
	m_states[ViewState::StbContrastFrameSelectionState] = std::unique_ptr<UiState>(new StbContrastFrameSelectionState(viewContext, *this, copyToRefClient));
	m_states[ViewState::CoRegistrationState]			= std::unique_ptr<UiState>(new CoRegistrationState(viewContext));
	m_states[ViewState::CoRegistrationGuidanceState]	= std::unique_ptr<UiState>(new CoRegistrationGuidanceState(viewContext));
    m_states[ViewState::StbBoostFrameSelectionState]    = std::unique_ptr<UiState>(new StbBoostFrameSelectionState(viewContext, *this, copyToRefClient));
    m_states[ViewState::NoConnection]				    = std::unique_ptr<UiState>(new NoConnectionState(viewContext));
	m_states[ViewState::NoLicense]				        = std::unique_ptr<UiState>(new NoLicenseState(viewContext));

	//set the initial UI State
	m_currentUiState = m_states[ViewState::NoConnection].get();
	m_currentUiState->onEnter();
}


UiStateController::~UiStateController()
{
	m_tsm.setPage(nullptr);
}

void UiStateController::roadmapStatusChanged()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->roadmapStatusChanged());
	}
}

void UiStateController::apcStatusChanged()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->apcStatusChanged());
	}
}

void UiStateController::epxChanged()
{
	if(m_currentUiState != nullptr)
	{
		if (m_autoReactivateState != nullptr)
		{
			m_autoSwitchTimer.stop();
			m_autoReactivateState = nullptr;
		}
		switchState(m_currentUiState->epxChanged());
	}
	 
	std::wstring epx(L"Other");
	if(m_pciViewContext.currentEpx.getType() == PciEpx::Type::Roadmap)
	{
		 epx = L"CoronaryRoadmap";
	}
	else if(m_pciViewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
	{
		 epx = L"StentBoostLive";
	}

	UtilizationLogEvent::NewEPXSelected(m_log,epx);
}

void UiStateController::angioProcessingStarted()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->angioProcessingStarted());
	}
}

void UiStateController::angioProcessingStopped()
{
	if(m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->angioProcessingStopped());
	}
}

void UiStateController::overlayImageAvailable()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->overlayImageAvailable());
	}
}

void UiStateController::imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType)
{
	if (m_autoReactivateState != nullptr)
	{
		m_autoSwitchTimer.stop();
		if (imageType != CommonPlatform::Xray::XrayImage::Type::Fluoro)
		{
			m_autoReactivateState = nullptr;
		}
	}

	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->imageRunStarted(imageType));
	}
}

void UiStateController::endLive()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->endLive());
	}
}

void UiStateController::lastImageHold()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->lastImageHold());
	}

	// lastImageHold means endlive of fluoro. If autoReactivate was requested, the timer should be started here
	if (m_autoReactivateState != nullptr)
	{
		m_autoSwitchTimer.start(m_autoSwitchDelay);
	}
}

void UiStateController::stentBoostImageAvailable(const std::shared_ptr<const XrayImage>& image, const std::shared_ptr<const StbMarkers>& markers, const std::shared_ptr<const StbImage>& boost)
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->stentBoostImageAvailable(image, markers, boost));
	}
}

void UiStateController::boostImageAvailable(const std::shared_ptr<const StbImage>& boost, const std::shared_ptr<const StbImage>& registeredLive)
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->boostImageAvailable(boost, registeredLive));
	}
}

void UiStateController::studyChanged()
{
	if (m_currentUiState != nullptr)
	{
        if (m_autoReactivateState != nullptr)
        {
            m_autoSwitchTimer.stop();
            m_autoReactivateState = nullptr;
        }
		switchState(m_currentUiState->studyChanged());
	}
}

void UiStateController::switchState(ViewState nextState)
{
	nextState = checkLicenseForRedirect(nextState);

	if(nextState != m_currentUiState->viewState)
	{
		if(m_currentUiState->isAutoReactivated())
		{
			// only 1 auto reactivate can be active at any time
			assert(!m_autoReactivateState || m_autoReactivateState == m_currentUiState);
			m_autoReactivateState = m_currentUiState;
		}

		auto oldUiState = m_currentUiState;
		m_currentUiState = m_states[nextState].get();
		m_log.DeveloperInfo(L"UiStateController : Switching from %s , to %s", oldUiState->getStateName().c_str(), m_currentUiState->getStateName().c_str());

		LogStateShanged();
		
		oldUiState->onExit();
		oldUiState->setArchivingStatusSink(nullptr);
		if (m_archivingStatus != nullptr)
		{
			m_archivingStatus->clearStatus();
		}
		m_currentUiState->setArchivingStatusSink(m_archivingStatus);

		m_currentUiState->onEnter();
		m_tsm.setPage(m_currentUiState->getTsmPage());

		if (eventViewStateChanged) eventViewStateChanged(getSelectedView());
	}
	else if (m_currentUiState->isAutoReactivated())
	{
		m_currentUiState->resetAutoReactivate();
	}
}

void UiStateController::setStbRegionOfInterest(Sense::Rect roi)
{
	if (onStbRoiChanged)
	{
		onStbRoiChanged(roi);
	}
}

void UiStateController::startReplayRun(bool loop)
{
	m_replayControl.startXRayReplay(loop);
}

void UiStateController::stopReplayRun()
{
	m_replayControl.stopXRayReplay();
}

bool UiStateController::isReplayActive()
{
	return m_replayControl.isActive();
}

UiState* UiStateController::getCurrentState() const
{
	return m_currentUiState;
}

void UiStateController::LogStateShanged()
{
	std::wstring state;

	if( m_currentUiState->viewState == ViewState::NonPciEpxState)
	{
		state = L"Coronary Tools Guidanc";
	}else if( m_currentUiState->viewState ==  ViewState::CrmCreationGuidanceState||
			  m_currentUiState->viewState ==  ViewState::CrmErrorGuidanceState ||
			  m_currentUiState->viewState ==  ViewState::CrmApcGuidanceState||	 
			  m_currentUiState->viewState ==  ViewState::CrmTableGuidanceState)
	{
		state = L"Dynamic Coronary Roadmap Guidance";
	}else if ( m_currentUiState->viewState ==  ViewState::CrmOverlayState)
	{
		state = L"Dynamic Coronary Roadmap Overlay";
	}else if ( m_currentUiState->viewState ==  ViewState::CrmProcessingAngioState)
	{
		state = L"Dynamic Coronary Roadmap Creation";
	}else if ( m_currentUiState->viewState ==  ViewState::CoRegistrationGuidanceState)
	{
		state = L"FFR/iFR Roadmap Guidance";
	}else if ( m_currentUiState->viewState == ViewState::CoRegistrationState)
	{ 
		if( m_pciViewContext.coRegState.systemState == CoRegistration::CoRegistrationServerState::SystemState::Review)
		{
			state = L"FFR/iFR Roadmap Review";	
		}else if( m_pciViewContext.coRegState.systemState == CoRegistration::CoRegistrationServerState::SystemState::Pullback ||
				 m_pciViewContext.coRegState.systemState == CoRegistration::CoRegistrationServerState::SystemState::SpotMeasurement)
		{
			state = L"FFR/iFR Roadmap Overlay";
		}
	}else if ( m_currentUiState->viewState ==  ViewState::StbLiveGuidanceState)
	{
		state = L"Stentboost Live Guidance";
	}
	else if (m_currentUiState->viewState == ViewState::StbPostDeployGuidanceState)
	{
		state = L"Stentboost PostDeploymnet Guidance";
	}else if ( m_currentUiState->viewState ==  ViewState::StbLiveProcessingState)
	{
		state = L"StentBoost Live Active";
	}
	else if (m_currentUiState->viewState == ViewState::StbPostDeployProcessingState)
	{
		state = L"StentBoost Post Deployment Active";
	}
	else if (m_currentUiState->viewState == ViewState::StbContrastReviewState)
	{
		state = L"StentBoost Subtract fade in fade out review Active";
	}

	if(!state.empty())
	{
		UtilizationLogEvent::ApplicationStateChanged(m_log,state); 
	}}

void UiStateController::reactivatePreviousState()
{
	if (m_autoSwitchTimer.isActive())
	{
		m_autoSwitchTimer.stop();
	}

	if (m_autoReactivateState != nullptr)
	{
		switchState(m_autoReactivateState->viewState);
		m_autoReactivateState = nullptr;
	}
}

ViewState UiStateController::getSelectedView() const
{
	if (m_currentUiState != nullptr)
	{
		return m_currentUiState->viewState;
	}
	return ViewState::Unknown;
}

std::wstring UiStateController::getSelectedViewName()
{
	if (m_currentUiState != nullptr)
	{
		return m_currentUiState->getStateName();
	}

	return L"Unknown";
}

void UiStateController::geometryChanged()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->geometryChanged());
	}
}

void UiStateController::geometryEnd()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->geometryEnd());
	}
}

void UiStateController::connectionStatusChanged(bool connected)
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->connectionStatusChanged(connected));
	}
}

void UiStateController::coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState, const CoReg::CoRegistrationServerState& nextState)
{
	if(m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->coRegistrationStateChanged(prevState, nextState));
	}
}

void UiStateController::coRegistrationConnectionChanged(bool connected)
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->coRegistrationConnectionChanged(connected));
	}
}

void UiStateController::coRegistrationError()
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->coRegistrationError());
	}
}

void Pci::Core::UiStateController::coRegistrationRunChanged(int runIndex)
{
	if (m_currentUiState != nullptr)
	{
		switchState(m_currentUiState->coRegistrationRunChanged(runIndex));
	}
}

ViewState UiStateController::checkLicenseForRedirect(ViewState newState) const
{
	auto targetState = m_states.at(newState).get();
	auto requiredLicense = targetState->getLicenseFeature();

	bool hasAnyActiveLicense = m_pciViewContext.licenseCollection.isAnyActive({ LicenseFeature::StentboostLive, LicenseFeature::StbPostDeploy, LicenseFeature::StbSubtract, LicenseFeature::Roadmap });
	
	bool requiredLicenseActive = isRequiredLicenseActive(requiredLicense);


	ViewState redirectedState = newState;

	if ((requiredLicense == LicenseFeature::Roadmap || requiredLicense == LicenseFeature::Coregistration) &&
		(!m_pciViewContext.licenseCollection.hasAnyLicense({LicenseFeature::Roadmap})) &&
		m_pciViewContext.licenseCollection.hasAnyLicense({LicenseFeature::StentboostLive,LicenseFeature::StbPostDeploy, LicenseFeature::StbSubtract}))
	{
		if (m_pciViewContext.currentEpx.getType() == PciEpx::Type::StentBoost)
		{
			redirectedState = m_currentUiState->viewState;
		}
		else
		{
			redirectedState = ViewState::NonPciEpxState;
		}
	}
	else if (!requiredLicenseActive || !hasAnyActiveLicense)
	{
		redirectedState = ViewState::NoLicense;
	}

	if (redirectedState != newState)
	{
		m_log.DeveloperInfo(L"UiStateController: prevented a switch to %s, redirecting to %s instead [Stb live license: %d,Stb post deployment license: %d,Stb contrast license: %d, Crm license: %d, Coreg license: %d]",
		                    targetState->getStateName().c_str(), m_states.at(redirectedState)->getStateName().c_str(),
		                    m_pciViewContext.licenseCollection.isActive(LicenseFeature::StentboostLive),
							m_pciViewContext.licenseCollection.isActive(LicenseFeature::StbPostDeploy),
							m_pciViewContext.licenseCollection.isActive(LicenseFeature::StbSubtract),
							m_pciViewContext.licenseCollection.isActive(LicenseFeature::Roadmap),
							m_pciViewContext.licenseCollection.isActive(LicenseFeature::Coregistration));
	}
		
	return redirectedState;
}

bool UiStateController::isRequiredLicenseActive(LicenseFeature requiredLicense) const
{
	return ( m_pciViewContext.licenseCollection.isActive(requiredLicense) || (requiredLicense == LicenseFeature::None));
}

}}