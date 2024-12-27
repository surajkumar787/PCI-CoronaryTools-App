// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform { namespace Xray { class IXrayGeometryController; }}

#include "XrayGeometry.h"
#include "XrayStudy.h"
#include "IXrayGeometryController.h" 
#include <memory>

namespace CommonPlatform { class Log; }

namespace Pci { namespace Core {

class CrmRoadmap;

class CrmApcController
{
public:
    CrmApcController(CommonPlatform::Xray::IXrayGeometryController &xrayGeometryController,bool automatic3DApcActivationConfiguration, const std::wstring& studyId, bool lastAutomatic3DApcActivationState);
	CrmApcController(CommonPlatform::Log &log, CommonPlatform::Xray::IXrayGeometryController &xrayGeometryController, bool automatic3DApcActivationConfiguration, const std::wstring& studyId, bool lastAutomatic3DApcActivationState);

	virtual ~CrmApcController();


    /// APC programming status communicated to client(s)
    enum class ApcModeStatus 
    {
        AutomaticActivation,                    // When APC is enabled, the 3D-APC function will be automatically activated. (i.e. Ready for Recall)
        TargetSetWithoutActivaton,              // When APC is enabled, the 3D-APC function will not be automatically activated. Only that target angles will be set.
        DisallowBecauseOfProgrammingConflict,   // When APC is enabled, no communication with the modality's 3D-APC function is allowed to avoid escalation of programming conflict.
    };

    // Enable or disable APC for a given target roadmap.
    // Pass 'nullptr' to disable APC. 
    // Pass valid roadmap object with rotation and angulation angles set to enable APC.
    // Returns true when APC command are send, return false when APC communication is blocked.
    bool        setApc                  ( const std::shared_ptr<const CrmRoadmap> & roadmap );

    // Set to false to prevent automatic 3D-APC activation during enabling/programming of angles.
    // The idea behind automatic 3D-APC activation to program angles and make the TSO's Accept APC button blinking in one programming step.
    void        setAutomaticActivation  (bool enabled);
    bool        getAutomaticActivation  () const ;

    // Call this function when the studyID on the modality has changes.
    void        setStudy                (const CommonPlatform::Xray::XrayStudy& newStudy);

    // Pass external activity from IXrayGeometryController
    void        setApcExternalActivity  (CommonPlatform::Xray::IXrayGeometryController::ApcExternalCwisActivity conflictEvent);

    // Pass external activity from IXrayGeometryController
    void        setTsm3dApcButtonPressed ();

    // Return event indicating changes in APC programming status or APC conflict detection status
    std::function<void(ApcModeStatus)> onApcModeStatusChanged;

    // Return the current status
    ApcModeStatus getStatus() const;

    // Get/Set the APC states related to blocking CWIS APC communication.
    void        setApcAllowed       (bool allowed);
    bool        getApcAllowed       () const;

    // Return true when CWIS APC connection is active
    void        setCwisApcConnected (bool connected );

    // Returns true when APC mode includes automatic activation of 3D-APC function
    bool        getAutomaticActivationMode() const;

    static const wchar_t* CrmApcController::getStateName( ApcModeStatus status);

	CrmApcController(const CrmApcController&) = delete;
	CrmApcController& operator=(const CrmApcController&) = delete;
protected:
    bool    enableApc      (double angulation, double rotation );
    bool    disableApc     ();
    void    updateStatus   (ApcModeStatus status);

private:
    ApcModeStatus           status;
    bool                    apcAutomaticActivationConfiguration;
    CommonPlatform::Xray::IXrayGeometryController& geometryController;
    ::CommonPlatform::Log&  m_log;
	CommonPlatform::Xray::XrayStudy     study;
    bool                    isConnected;
};

}}