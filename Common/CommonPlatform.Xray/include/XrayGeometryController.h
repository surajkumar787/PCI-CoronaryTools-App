// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "IXrayGeometryController.h"
#include "CwisApc.h"
#include <Sense/Common/Timer.h>


namespace Sense { class IScheduler; }
namespace CommonPlatform { class Log; }

namespace CommonPlatform { namespace Xray {


// This class implements communication with the modality to control the automatic positioning (APC) function.
class XrayGeometryController : public IXrayGeometryController
{
public:
    explicit XrayGeometryController(Sense::IScheduler& scheduler, const std::wstring &description = L"PciExpressXrayGeometryController");
	XrayGeometryController(Sense::IScheduler& scheduler, CommonPlatform::Log& logger, const std::wstring &description = L"PciExpressXrayGeometryController");
    ~XrayGeometryController(void);

    // IXrayGeometryControllerInterface: Program APC angles on the modality.
    virtual bool enableApc      (double angulation, double rotation, bool activate)      override;

    // IXrayGeometryControllerInterface: Disabled previously programmed APC angles on the modality.
    virtual bool disableApc     ()                          override;

    // Connect to modality. Required string containing IPV address where CWIS server is running and Cwis port.
    void connect                (const std::wstring &cwisHost, int cwisPort);

    // Disconnect from modality.
    void disconnect             ();

    // Return true when there is an active connection with modality.
    bool isConnected            () const ;

    // Raises an event when the connection status changes.
    std::function<void(bool)>   eventConnectionChanged;

    // Support function to log human readable names of enumerations
    static const wchar_t* getProgrammingStateString(ApcCwisProgrammingStatus state);

    // Support function to log human readable names of enumerations
    static const wchar_t* getExternalCwisActivityString(ApcExternalCwisActivity conflict);

protected:

    Cwis::CwisApcWorkstation	cwisApc;
    ApcCwisProgrammingStatus    programmingState;
    const double                responseTimeOut;

    void setState               (ApcCwisProgrammingStatus newprogrammingState);
    void notifyConflict         (ApcExternalCwisActivity newConflictState);
    void notifyTsm3dApcButtonPressed() const;
    void sendApcAngles          (double angulation, double rotation, bool activate );
    void sendApcActivate        ();

private:
    CommonPlatform::Log&		log; 
    double                      lastProgrammedRotation;
    double                      lastProgrammedAngulation;
    bool                        lastProgrammedActivation; // last enableAPC programming was with activation on or off.
    bool                        apcProgrammmingFinished; // True when angles were programmed by XrayGeometryController. Do not confuse with enabled state of 3D-APC on Modality.
    bool                        lastBlinkingStateAtMomentOfEnableAPC;
	ApcExternalCwisActivity     conflictState;
	Sense::Timer                responseTimer;
    bool                        initialCwisUpdateReceived;

	bool                        previousCwisApcModalityModelBlinking;
	bool                        previousCwisApcModalityModelEnabled;
	bool                        previousCwisApcModalityModelSelected;

    void onCwisApc();
    void onCwisConnectionStatusChanged(bool connected)const;
    bool checkForApcAngleProgrammingConflict(bool valid, double angulation, double rotation, bool enabled) const;
    bool check3dApcPressedOnTSM(bool enabled, bool blinking) const;
    bool checkForApcStatusConflict(bool valid, bool enabled, bool blinking, bool selected) const;
    bool areModalityApcAnglesChanged(double currentAngulation, double currentRotation) const;
    void responseTimeOutElapsed();    
    bool respondToAlluraDuringProgramming(const Cwis::CwisApcModalityModel& model);
    void conflictDetection(const Cwis::CwisApcModalityModel& model);
    void keepCwisApcModel(const Cwis::CwisApcModalityModel& model);
	bool isApcChangedToDisabled(const Cwis::CwisApcModalityModel& model);
    static const double undefinedAngle;
};

}}
 