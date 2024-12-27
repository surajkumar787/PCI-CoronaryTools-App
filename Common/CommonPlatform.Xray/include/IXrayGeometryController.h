// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <functional>

namespace CommonPlatform { namespace Xray {

class IXrayGeometryController
{
public:
	virtual ~IXrayGeometryController() {}

    virtual bool enableApc      (double angulation, double rotation, bool activate) = 0;
    virtual bool disableApc     ()                                                  = 0;

    // State of APC programming via CWIS
    enum class ApcCwisProgrammingStatus
    {
        Disabled,           // APC is not programmed by PCI client
        WaitForEnable,      // APC angles are programmed in sequence to activate 3D-APC. Waiting until modality enables 3D-APC
        WaitForBlinking,    // APC active command is set in sequence to activate 3D-APC. Waiting until modality activates 3D-APC (blinking)
        Enabled,            // APC angles are programmed. For manual 3D-APC mode. End state for user. No automatic activation.
        Activated           // APC is activated/blinking.
    };

    // State of external APC activity detected via CWIS model
    enum class ApcExternalCwisActivity 
    {
        NoActivity,                 // remove???
        AngleProgrammingDetected,   // Other APC Cwis Client programs valid 3D-APC angles.
        StatusChangeDetected,       // Modality (e.g. APC sequence) or other APC CWIS changes the status of 3D-APC.
    };

    // Raises an event when the APC programming status on the CWIS interface changes
    std::function<void(ApcCwisProgrammingStatus)>   eventApcCwisProgrammingStatusChanged;

    // Raises an event when an conflict is detected on the on the CWIS interface changes
    std::function<void(ApcExternalCwisActivity)>	eventApcExternalCwisActivityDetected;

    // Raises an event when the 3D-APC button is pressed on the TSM
    std::function<void()>	                        eventTsm3dApcButtonPressed;

};

}}