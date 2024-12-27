// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <Log.h>
#include <thread>
#include "XrayGeometryController.h"
#include "Sense/System/IScheduler.h"
#include "UtilizationLogEvent.h"

namespace CommonPlatform{ namespace Xray{
using namespace CommonPlatform;

const double XrayGeometryController::undefinedAngle = 999;

XrayGeometryController::XrayGeometryController(Sense::IScheduler& scheduler, const std::wstring &description)
:
    XrayGeometryController(scheduler, Logger(LoggerType::Platform), description)
{
}

XrayGeometryController::XrayGeometryController(Sense::IScheduler& scheduler, CommonPlatform::Log& logger, const std::wstring &description) :
	cwisApc(description.c_str()),
	programmingState(ApcCwisProgrammingStatus::Disabled),
	responseTimeOut(0.5 /*sec*/),
	log(logger),
	lastProgrammedRotation(undefinedAngle),
	lastProgrammedAngulation(undefinedAngle),
	lastProgrammedActivation(false),
	apcProgrammmingFinished(false),
	lastBlinkingStateAtMomentOfEnableAPC(false),
	conflictState(ApcExternalCwisActivity::NoActivity),
	responseTimer(scheduler, [this] { responseTimeOutElapsed(); }),
	initialCwisUpdateReceived(false),
	previousCwisApcModalityModelBlinking(false),
	previousCwisApcModalityModelEnabled(false),
	previousCwisApcModalityModelSelected(false)
{
	cwisApc.eventModelChanged = [this] { onCwisApc();			};
	cwisApc.eventConnectionChanged = [this](Cwis::CwisConnectionType status) { onCwisConnectionStatusChanged(status == Cwis::CwisConnectionType::Active); };
}

XrayGeometryController::~XrayGeometryController(void)
{
    responseTimer.stop();
}

void XrayGeometryController::connect(const std::wstring &cwisHost, int cwisPort)
{
    cwisApc.connect(cwisHost.c_str(), static_cast<unsigned short>(cwisPort));
    initialCwisUpdateReceived = false;
}

void XrayGeometryController::disconnect()
{
    cwisApc.disconnect();
}

bool XrayGeometryController::isConnected() const
{
    return (cwisApc.isConnected() == Cwis::CwisConnectionType::Active);
}

bool XrayGeometryController::enableApc(double angulation, double rotation, bool activate)
{
    if (!isConnected())
    {
        log.DeveloperInfo(L"XrayGeometryController: Cannot enable APC. Cwis APC not connected.");
        return false;
    }

    log.DeveloperInfo(  L"XrayGeometryController: Enable APC [Ang.=%.2f, Rot.=%.2f, Activate=%s, Current 3D-APC Enabled=%d, Blinking=%d]",
                        angulation, rotation, activate ? L"True" : L"False", cwisApc.modality.enabled ? 1 : 0, cwisApc.modality.blinking ? 1 : 0);

    lastProgrammedAngulation = angulation;
    lastProgrammedRotation   = rotation;
    lastProgrammedActivation = activate;
    lastBlinkingStateAtMomentOfEnableAPC = false;;

    apcProgrammmingFinished = false; // disable checks on conflicts during processing.
    responseTimer.stop();

    if ( activate )
    {
        if ( cwisApc.modality.blinking && cwisApc.modality.enabled)
        {
            log.DeveloperInfo(L"XrayGeometryController: 3D-APC already blinking and enabled. Angles programmed");
            apcProgrammmingFinished = true;
            sendApcAngles(angulation, rotation, /* activate= */ false);    
            setState(ApcCwisProgrammingStatus::Activated); 
        }
        else if ( cwisApc.modality.blinking && (!cwisApc.modality.enabled))
        {
            log.DeveloperInfo(L"XrayGeometryController: 3D-APC already blinking but not enabled. Angles programmed and wait for modality to set 'Enabled'");
            lastBlinkingStateAtMomentOfEnableAPC = true;
            sendApcAngles(angulation, rotation, /* activate= */ false);    
            setState(ApcCwisProgrammingStatus::WaitForEnable); 
            responseTimer.start(responseTimeOut);
        }
        else if ( cwisApc.modality.enabled && (!cwisApc.modality.blinking))
        {
            log.DeveloperInfo(L"XrayGeometryController: 3D-APC already enabled but not blinking. Angles programmed and activated. Wait for modality to set 'Blinking'.");
            sendApcAngles(angulation, rotation, /* activate= */ true);    
            setState(ApcCwisProgrammingStatus::WaitForBlinking);
            responseTimer.start(responseTimeOut);
        }
        else // Enabled=false and Blinking=false
        {
            log.DeveloperInfo(L"XrayGeometryController: 3D-APC angles programmed. Wait for modality to set 'Enabled' before activation.");
            sendApcAngles(angulation, rotation, /* activate= */false);    
            setState(ApcCwisProgrammingStatus::WaitForEnable);
            responseTimer.start(responseTimeOut);
        }
    }
    else
    {
        log.DeveloperInfo(L"XrayGeometryController: 3D-APC angles programmed. Wait for modality to set 'Enabled'.");
        sendApcAngles(angulation, rotation, /* activate= */ false);
        setState(ApcCwisProgrammingStatus::WaitForEnable);
        responseTimer.start(responseTimeOut);
    }

    return true;
}

void XrayGeometryController::sendApcAngles( double angulation, double rotation, bool activate ) 
{
    cwisApc.workstation.activate        = activate;
    cwisApc.workstation.valid           = true;
    cwisApc.workstation.angulation      = angulation;
    cwisApc.workstation.rotation        = rotation;
    cwisApc.workstation.validUnflipped	= false;
    cwisApc.workstation.lockTable		= false;
    cwisApc.workstation.unlockTable		= false;
    cwisApc.workstation.enableMaxSid	= false;
    cwisApc.workstation.disableMaxSid	= false;
    cwisApc.commit();
}

void XrayGeometryController::sendApcActivate()
{
    cwisApc.workstation.activate = true;   
    cwisApc.commit();
}

bool XrayGeometryController::disableApc()
{
    if (!isConnected())
    {
        log.DeveloperInfo(L"XrayGeometryController: Cannot disable APC. Cwis APC not connected.");
        return false;
    }

    log.DeveloperInfo(L"XrayGeometryController: Disable APC.");

    apcProgrammmingFinished  = false;

    // Then invalidates 3D-APC to disable 3D-APC (i.e. set enable 'false')
    cwisApc.workstation.activate		= false;
    cwisApc.workstation.valid			= false; 
    cwisApc.workstation.validUnflipped	= false;
    // Do not re-program the APC angles.
    cwisApc.workstation.lockTable		= false;
    cwisApc.workstation.unlockTable		= false;
    cwisApc.workstation.enableMaxSid	= false;
    cwisApc.workstation.disableMaxSid	= false;
    cwisApc.commit();

    setState(ApcCwisProgrammingStatus::Disabled);

    return true;
}

// Store current APC model to be able to detected changes in next APC model.
void XrayGeometryController::keepCwisApcModel(const Cwis::CwisApcModalityModel& model)
{
    previousCwisApcModalityModelBlinking = model.blinking;  
    previousCwisApcModalityModelEnabled  = model.enabled;
    previousCwisApcModalityModelSelected = model.selected;
}

bool XrayGeometryController::check3dApcPressedOnTSM(bool enabled, bool blinking ) const
{
    bool blinkingBecomesTrue = false;
    
    if ( apcProgrammmingFinished )
    {
        // note: when blinking is true, and TSM 3d-apc button is pressed, a glitch occurs (1110111) that triggers this the next statement to become true. That is correct as the 3D-APC button is pressed.
        blinkingBecomesTrue = enabled && !previousCwisApcModalityModelBlinking && blinking;
    }

    return blinkingBecomesTrue;
}

void XrayGeometryController::onCwisApc()
{
    log.DeveloperInfo(  L"XrayGeometryController: onCwisApc [Enabled=%d, Blinking=%d, Selected=%d]", 
                        cwisApc.modality.enabled ? 1 : 0, cwisApc.modality.blinking ? 1 : 0, cwisApc.modality.selected ? 1 : 0);

    if ( !respondToAlluraDuringProgramming(cwisApc.modality) )
    {
        if ( initialCwisUpdateReceived )
        {
            log.DeveloperInfo(  L"XrayGeometryController: onCwisApc [APC valid=%d, APC angulation=%.2f, APC rotation=%.2f]", 
                                cwisApc.workstation.valid ? 1 : 0, static_cast<double>(cwisApc.workstation.angulation), static_cast<double>(cwisApc.workstation.rotation));
            conflictDetection(cwisApc.modality);
        }
        else
        {
            log.DeveloperInfo(  L"XrayGeometryController: First onCwisApc, Skip conflict detection [APC valid=%d, APC angulation=%.2f, APC rotation=%.2f]", 
                                cwisApc.workstation.valid ? 1 : 0, static_cast<double>(cwisApc.workstation.angulation), static_cast<double>(cwisApc.workstation.rotation));
            initialCwisUpdateReceived = true;
        }

        if ( check3dApcPressedOnTSM(cwisApc.modality.enabled, cwisApc.modality.blinking) )
        {
            log.DeveloperInfo(  L"XrayGeometryController: 3D-APC button pressed on TSM. [Current state=%s, Last conflict state=%s]",
                                getProgrammingStateString(programmingState),
                                getExternalCwisActivityString(conflictState));
            notifyTsm3dApcButtonPressed();
        }
    }

    keepCwisApcModel(cwisApc.modality);
}

// When this functions returns true, the received CWIS model update was as expected and conflict detection can be skipped for this specific CWIS model update.
bool XrayGeometryController::respondToAlluraDuringProgramming(const Cwis::CwisApcModalityModel& model)
{
    bool updateHandled = false;

    if ( programmingState == ApcCwisProgrammingStatus::WaitForEnable )
    {
        if ((!previousCwisApcModalityModelEnabled) && model.enabled)
        {
            if ( lastProgrammedActivation )
            {
                if ( lastBlinkingStateAtMomentOfEnableAPC )
                {
                    apcProgrammmingFinished = true;
                    responseTimer.stop();
                    log.DeveloperInfo(L"XrayGeometryController: 'Enabled set' received from modality. 'Blinking set' already at start of programming. Programming APC is finished.");
                    setState(ApcCwisProgrammingStatus::Activated);
                }
                else
                {
                    log.DeveloperInfo(L"XrayGeometryController: 'Enabled set' received from modality. APC can be activated.");
                    sendApcActivate();
                    setState(ApcCwisProgrammingStatus::WaitForBlinking);
                    responseTimer.start(responseTimeOut);
                }
            }
            else
            {
                apcProgrammmingFinished = true;
                responseTimer.stop();
                log.DeveloperInfo(L"XrayGeometryController: 'Enabled set' received from modality. Programming APC is finished.");
                setState(ApcCwisProgrammingStatus::Enabled);
            }
            updateHandled = true;
        }
    }
    else if ( programmingState == ApcCwisProgrammingStatus::WaitForBlinking )
    {
        if ((!previousCwisApcModalityModelBlinking) && model.blinking)
        {
            apcProgrammmingFinished = true;
            responseTimer.stop();
            log.DeveloperInfo(L"XrayGeometryController: 'Blinking set' received from modality. Programming APC is finished.");
            setState(ApcCwisProgrammingStatus::Activated);
            updateHandled = true;
        }
    }
    else if ( programmingState == ApcCwisProgrammingStatus::Disabled )
    {
		
        if (isApcChangedToDisabled(model))
        {
            apcProgrammmingFinished = true;
            log.DeveloperInfo(L"XrayGeometryController: 'Enable reset' received from modality. Disabling APC is finished.");
            updateHandled = true;
        }
    }
    return updateHandled;
}

bool XrayGeometryController::isApcChangedToDisabled(const Cwis::CwisApcModalityModel& model)
{
	return initialCwisUpdateReceived && (!model.enabled) && (previousCwisApcModalityModelEnabled || previousCwisApcModalityModelBlinking);
}

void XrayGeometryController::conflictDetection(const Cwis::CwisApcModalityModel& model)
{
    if ( apcProgrammmingFinished )
    {
        if ( checkForApcStatusConflict (cwisApc.workstation.valid, model.enabled, model.blinking, model.selected))
        {
            log.DeveloperInfo(  L"XrayGeometryController: APC state conflict detected. Expected that 3D-APC is activated. [Modality values: Valid=%d, Enabled=%d, Blinking=%d, Selected=%d]",
                                cwisApc.workstation.valid ? 1 : 0, model.enabled ? 1 : 0, model.blinking ? 1 : 0, model.selected ? 1 : 0);

            notifyConflict(ApcExternalCwisActivity::StatusChangeDetected);
        }
    }

    // Always check angles
    if ( checkForApcAngleProgrammingConflict   ( cwisApc.workstation.valid, cwisApc.workstation.angulation, cwisApc.workstation.rotation, cwisApc.modality.enabled) )
    {
        log.DeveloperInfo(  L"XrayGeometryController: APC angles programming activity detected. [Expected values:Angulation=%.2f, Rotation=%.2f][Modality values:Angulation=%.2f, Rotation=%.2f, valid=%d, enabled=%d]",
                            lastProgrammedAngulation, lastProgrammedRotation,
                            static_cast<double>(cwisApc.workstation.angulation),
                            static_cast<double>(cwisApc.workstation.rotation),
                            cwisApc.workstation.valid ? 1 : 0, cwisApc.modality.enabled ? 1 : 0);

        notifyConflict(ApcExternalCwisActivity::AngleProgrammingDetected);
    }
}

void XrayGeometryController::onCwisConnectionStatusChanged(bool connected) const
{
	if (!connected && eventApcCwisProgrammingStatusChanged)
	{
		eventApcCwisProgrammingStatusChanged(ApcCwisProgrammingStatus::Disabled);
	}

	if (eventConnectionChanged)
	{
		eventConnectionChanged(connected);
	}
}

// Return true when rounded values (as displayed on GUI) are not the same
bool XrayGeometryController::areModalityApcAnglesChanged(double currentAngulation, double currentRotation) const
{
    int roundedLastProgrammedAngulation = static_cast<int>(lastProgrammedAngulation+0.5);
    int roundedLastProgrammedRotation   = static_cast<int>(lastProgrammedRotation+0.5);

    int roundedNewAngulation            = static_cast<int>(currentAngulation+0.5);
    int roundedNewRotation              = static_cast<int>(currentRotation+0.5);

    return ( roundedLastProgrammedAngulation !=  roundedNewAngulation ||
             roundedLastProgrammedRotation   !=  roundedNewRotation   );
}

bool XrayGeometryController::checkForApcAngleProgrammingConflict(bool valid, double angulation, double rotation, bool enabled) const
{
    if ( !isConnected() ) return false;
    if ( lastProgrammedAngulation >= undefinedAngle || lastProgrammedRotation >= undefinedAngle )  
    {
        log.DeveloperInfo(  L"XrayGeometryController: APC has not be programmed yet from our side. Expect that valid remains false.");
        // APC has not be programmed yet from our side. Assume that valid remains false, if not it was another client programming APC.
        return valid;
    }

    bool enabledBecomesFalse = previousCwisApcModalityModelEnabled && (!enabled);
    if ( enabledBecomesFalse && (!valid) )
    {
        log.DeveloperInfo(  L"XrayGeometryController: Both valid and enabled are false. we cannot recover by pressing the 3D-TSM button. This situation is regarded as an angle programming conflict.");
        // When !valid and !enabled the conflict state is more than a status conflict, because we cannot recover by pressing the 3D-TSM button, because it has been disabled by
        // the Geometry SW at Allura side. Therefore we can regard is as an angle conflict, so that PCI express will show 'manual' guidance.
        return true;
    }

    // only detect a conflict when angles are written on purpose by other app, that means that valid must be true
    bool conflict = valid && areModalityApcAnglesChanged(angulation,rotation);
    return conflict;
}

bool XrayGeometryController::checkForApcStatusConflict(bool valid, bool enabled, bool blinking, bool selected) const
{
    if ( !isConnected() ) return false;

    bool blinkingBecomesFalse    = previousCwisApcModalityModelBlinking && (!blinking);
    // We expect blinking (ready for recall) or selected (recalled, i.e. moving) to be set. Enabled and valid are set in normal situations.
    // Blinking could be stuck at 'true' when only enabled.
    return (!valid) || (!enabled) || (blinkingBecomesFalse && (!selected));
}

void XrayGeometryController::responseTimeOutElapsed()
{
	if (responseTimer.isActive())
	{
		responseTimer.stop();
	}

    if ( programmingState == ApcCwisProgrammingStatus::WaitForEnable )
    {
        // When a time out occurs, we can still continue as if we received 'enabled=true'.
        if ( lastProgrammedActivation )
        {
            log.DeveloperInfo(L"XrayGeometryController: Time out waiting for modality to set 'Enabled'. APC must be activated anyway.");
            sendApcActivate();
            setState(ApcCwisProgrammingStatus::WaitForBlinking);
            responseTimer.start(responseTimeOut);
        }
        else
        {
            log.DeveloperInfo(L"XrayGeometryController: Time out waiting for modality to set 'Enabled'.");
            setState(ApcCwisProgrammingStatus::Enabled);
            apcProgrammmingFinished = true;
			UtilizationLogEvent::APCFailedToStart(log);
        }
    }
    else if ( programmingState == ApcCwisProgrammingStatus::WaitForBlinking )
    {
        // When a time out occurs, we can continue as if we received only 'enabled=true' and not 'blinking=true'.
        log.DeveloperInfo(L"XrayGeometryController: Time out waiting for modality to set 'Blinking'.");
        setState(ApcCwisProgrammingStatus::Enabled);
        apcProgrammmingFinished = true;
		UtilizationLogEvent::APCFailedToStart(log);
    }
    else
    {
        log.DeveloperInfo(L"XrayGeometryController: Response time out in unexpected state. [State=%s]", getProgrammingStateString(programmingState));
    }
}

void XrayGeometryController::setState(ApcCwisProgrammingStatus newProgrammingState)
{
    if ( newProgrammingState != programmingState )
    {
        log.DeveloperInfo( L"XrayGeometryController: CWIS State change [Previous=%s, Next=%s]", getProgrammingStateString(programmingState), getProgrammingStateString(newProgrammingState));
        programmingState = newProgrammingState;
        if ( eventApcCwisProgrammingStatusChanged ) eventApcCwisProgrammingStatusChanged(programmingState);
    }
}

void XrayGeometryController::notifyConflict(ApcExternalCwisActivity newConflictState)
{
    conflictState = newConflictState;
    if ( eventApcExternalCwisActivityDetected ) eventApcExternalCwisActivityDetected(newConflictState);
}

void XrayGeometryController::notifyTsm3dApcButtonPressed()const
{
    if ( eventTsm3dApcButtonPressed) eventTsm3dApcButtonPressed();
}

const wchar_t* XrayGeometryController::getProgrammingStateString(ApcCwisProgrammingStatus internalState)
{
    switch(internalState)
    {
        case ApcCwisProgrammingStatus::Activated:               return L"Activated";
        case ApcCwisProgrammingStatus::Enabled:                 return L"Enabled";
        case ApcCwisProgrammingStatus::Disabled:                return L"Disabled";
        case ApcCwisProgrammingStatus::WaitForBlinking:         return L"WaitForBlinking";
        case ApcCwisProgrammingStatus::WaitForEnable:           return L"WaitForEnable";
        default:
            return L"Unknown state";
    }
}

const wchar_t* XrayGeometryController::getExternalCwisActivityString(ApcExternalCwisActivity conflict)
{
    switch(conflict)
    {
        case ApcExternalCwisActivity::NoActivity:                return L"NoActivity";
        case ApcExternalCwisActivity::AngleProgrammingDetected:  return L"AngleProgrammingDetected";
        case ApcExternalCwisActivity::StatusChangeDetected:      return L"StatusChangeDetected";
        default:
            return L"Unknown state";
    }
}
}}