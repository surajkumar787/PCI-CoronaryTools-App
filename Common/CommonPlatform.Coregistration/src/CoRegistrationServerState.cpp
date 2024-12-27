// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationServerState.h"

using namespace CommonPlatform::CoRegistration;

CoRegistrationServerState::CoRegistrationServerState() :
    timestamp               (0),
    fmState                 (FmState::Unknown),
    systemState             (SystemState::Idle),				
    normalized              (false),
    ecgConnected            (false),
    demoModeActive          (false),
    pimConnected            (false),
    pressurePlugConnected   (false),
    wireConnected           (false),
    wireShorted		        (false),
	wireReady				(false)
{
}

bool CoRegistrationServerState::operator==(const CoRegistrationServerState &serverState) const
{
	if(serverState.systemState			  == systemState			  && 
		serverState.fmState				  == fmState				  &&
		serverState.normalized			  == normalized			      &&
		serverState.ecgConnected          == ecgConnected             &&
		serverState.demoModeActive        == demoModeActive           &&
		serverState.pimConnected          == pimConnected             &&
		serverState.pressurePlugConnected == pressurePlugConnected    &&
		serverState.wireConnected         == wireConnected            &&
		serverState.wireShorted		      == wireShorted			  &&
		serverState.wireReady			  == wireReady)
		
		return true;

	else
		return false;
}

bool CoRegistrationServerState::operator!=(const CoRegistrationServerState &serverState) const
{
	return !(*this == serverState);
}
