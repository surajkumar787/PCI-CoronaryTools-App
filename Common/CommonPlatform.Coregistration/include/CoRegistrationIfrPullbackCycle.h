// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include <stdint.h>

namespace CommonPlatform { namespace CoRegistration{

struct CoRegistrationIfrPullbackCycle
{
	int	   runIndex;
    double timestamp;
    float  ifrTrendValue;
    float  ifrRawValue;
    bool   isFiltered;
    bool   isValidCycle;
    double waveFreeStartTime;
    double waveFreeEndTime;
	double runStartedTime;

    CoRegistrationIfrPullbackCycle();

};
}}

