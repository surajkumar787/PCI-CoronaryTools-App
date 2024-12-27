// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmApcController.h"
#include "CrmRoadmap.h"

#include <Log.h>

using namespace Pci::Core;
using namespace CommonPlatform::Xray;
using namespace CommonPlatform;



CrmApcController::CrmApcController( IXrayGeometryController &xrayGeometryController,
									bool automatic3DApcActivationConfiguration,
									const std::wstring& studyId,
									bool lastAutomatic3DApcActivationState)
	:CrmApcController( CommonPlatform::Logger(::CommonPlatform::LoggerType::CoronaryRoadmap),
					   xrayGeometryController,
					   automatic3DApcActivationConfiguration,
					   studyId,
					   lastAutomatic3DApcActivationState)
{}

CrmApcController::CrmApcController( Log& log,
									IXrayGeometryController &xrayGeometryController, 
                                    bool automatic3DApcActivationConfiguration, 
                                    const std::wstring& studyId, 
                                    bool lastAutomatic3DApcActivationState)
:   
    status                                  (lastAutomatic3DApcActivationState ? ApcModeStatus::AutomaticActivation : ApcModeStatus::TargetSetWithoutActivaton),
    apcAutomaticActivationConfiguration     (automatic3DApcActivationConfiguration),
    geometryController                      (xrayGeometryController),
	m_log                                     (log),
    isConnected                             (false)
{
	m_log.DeveloperInfo(L"CrmApcController initial state: automaticApc %u, lastActivationState %u", automatic3DApcActivationConfiguration, lastAutomatic3DApcActivationState);

    study.studyId = studyId;

    if ( !automatic3DApcActivationConfiguration && lastAutomatic3DApcActivationState)
    {
        m_log.DeveloperInfo(L"Disable automatic activated. Persistent setting updated with configuration setting.");
        setAutomaticActivation(false);
    }
}

CrmApcController::~CrmApcController(void)
{ 
}

void CrmApcController::setApcExternalActivity(IXrayGeometryController::ApcExternalCwisActivity conflictEvent)
{
    switch( conflictEvent )
    {
        case IXrayGeometryController::ApcExternalCwisActivity::AngleProgrammingDetected:
            setApcAllowed(false);
            m_log.DeveloperInfo(L"CrmApcController: APC angle programming conflict. Disable all 3D-APC programming.");
            break;

        case IXrayGeometryController::ApcExternalCwisActivity::StatusChangeDetected:

            if (( status != ApcModeStatus::DisallowBecauseOfProgrammingConflict) && getAutomaticActivation() )
            {
                setAutomaticActivation(false);
                m_log.DeveloperInfo(L"CrmApcController: APC Status conflict. Disable automatic 3D-APC activation.");
            }
            break;

        case IXrayGeometryController::ApcExternalCwisActivity::NoActivity:
            m_log.DeveloperInfo(L"CrmApcController: Event 'No Conflict' received.");
            break;

        default:
            m_log.DeveloperInfo(L"CrmApcController: Unknown Apc conflict event received.");
            break;
    }    
}

void CrmApcController::setTsm3dApcButtonPressed ()
{
    if ( status == ApcModeStatus::TargetSetWithoutActivaton )
    {
        setAutomaticActivation(true); 

        m_log.DeveloperInfo(L"CrmApcController: 3D-APC Activated by TSM button. [State=%s, AutoActivate=%s, ApcAllowed=%s]",
            getStateName(status),
            getAutomaticActivation() ? L"True" : L"False",
            getApcAllowed() ? L"True" : L"False");
    }
}

bool CrmApcController::enableApc( double angulation, double rotation)
{
    if (!getApcAllowed()) 
    {
        m_log.DeveloperInfo(L"CrmApcController: Cannot enable APC. APC communication blocked.");
        return false;
    }

    return geometryController.enableApc(angulation, rotation, getAutomaticActivation());
}

bool CrmApcController::disableApc()
{
    if (!getApcAllowed()) 
    {
        m_log.DeveloperInfo(L"CrmApcController: Cannot disable APC. APC communication blocked.");
        return false;
    }

    return geometryController.disableApc();
}

void CrmApcController::setStudy(const XrayStudy &newStudy)
{
    if (study.studyId != newStudy.studyId) // because only studyID is persistent.....
    {
        // Allow APC communication again (and reset to configure 3D-APC activation mode)
        setApcAllowed(true);

        m_log.DeveloperInfo(  L"CrmApcController: Study changed. Reset automatic 3D-APC activation to configure mode. [Auto 3D-APC activation mode=%s]",
                            (apcAutomaticActivationConfiguration ? L"Enabled" : L"Disabled"));

        study = newStudy;
    }
}

void CrmApcController::setAutomaticActivation(bool enable)
{
    m_log.DeveloperInfo(L"CrmApcController: Set automatic 3D-APC Activation. [Enabled=%s]",enable ? L"True" : L"False");

    if ( enable )
    {
        updateStatus(ApcModeStatus::AutomaticActivation);
    }
    else
    {
        updateStatus(ApcModeStatus::TargetSetWithoutActivaton);
    }
}

bool CrmApcController::getAutomaticActivation() const 
{
    if ( !isConnected ) return false;
    return  ( status == CrmApcController::ApcModeStatus::AutomaticActivation );
}

const wchar_t* CrmApcController::getStateName( ApcModeStatus status) 
{
    switch(status)
    {
        case ApcModeStatus::AutomaticActivation:                    return L"AutomaticActivation";
        case ApcModeStatus::TargetSetWithoutActivaton:              return L"TargetSetWithoutActivaton";
        case ApcModeStatus::DisallowBecauseOfProgrammingConflict:   return L"DisallowBecauseOfProgrammingConflict";
        default:                                                    return L"Unknown state";
    }
}

void CrmApcController::updateStatus(ApcModeStatus newStatus)
{
    if ( status != newStatus )
    {
        m_log.DeveloperInfo(  L"CrmApcController: Status change. [Prev=%s, Next=%s]", getStateName(status), getStateName(newStatus));
        status = newStatus;

        if ( onApcModeStatusChanged ) onApcModeStatusChanged(newStatus);
    }
}

CrmApcController::ApcModeStatus CrmApcController::getStatus() const
{
    if ( !isConnected ) return ApcModeStatus::DisallowBecauseOfProgrammingConflict;
    return status;
}

void CrmApcController::setApcAllowed( bool allowed)
{
    m_log.DeveloperInfo(  L"CrmApcController: Set APC communication allowed = %s", allowed ? L"True" : L"False");

    if ( !allowed )
    {
        updateStatus(CrmApcController::ApcModeStatus::DisallowBecauseOfProgrammingConflict);
    }
    else
    {
        setAutomaticActivation( apcAutomaticActivationConfiguration);
    }
}

bool CrmApcController::getApcAllowed() const
{
    if ( !isConnected ) return false;
    return (status != CrmApcController::ApcModeStatus::DisallowBecauseOfProgrammingConflict);
}

bool CrmApcController::setApc( const std::shared_ptr<const CrmRoadmap> & roadmap )
{
    if ( roadmap )
    {
        return enableApc(roadmap->geometry.angulation, roadmap->geometry.rotation);
    }
    else
    {
        return disableApc();
    }
}

void CrmApcController::setCwisApcConnected( bool connected )
{
    isConnected = connected;
    m_log.DeveloperInfo(  L"CrmApcController: setCwisApcConnected = %s", isConnected ? L"Connected" : L"Disconnected");
	if ( connected && onApcModeStatusChanged ) onApcModeStatusChanged(getStatus());
}


bool CrmApcController::getAutomaticActivationMode() const 
{
    return apcAutomaticActivationConfiguration; 
}
