// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <stdint.h>

namespace CommonPlatform { namespace CoRegistration{

struct CoRegistrationServerState{
    enum class FmState{
        Unknown,
        FFR,
        IFRSpot,
		IFRPullback
    };

    enum class SystemState{
        Idle,
        Live,
        SpotMeasurement,
        Pullback,
        Review,
        Normalization,
        Freeze,
        Zero
    };

    double          timestamp;
    FmState         fmState;
    SystemState     systemState;
    bool            normalized;
    bool            ecgConnected;
    bool            demoModeActive;
    bool            pimConnected;
    bool            pressurePlugConnected;
    bool            wireConnected;
    bool            wireShorted;
    bool            wireReady;

    CoRegistrationServerState();

	bool operator==(const CoRegistrationServerState &serverState) const;
	bool operator!=(const CoRegistrationServerState &serverState) const;
};
}}