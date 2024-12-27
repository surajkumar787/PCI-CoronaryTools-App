// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationIfrPullbackCycle.h"

using namespace CommonPlatform::CoRegistration;

CoRegistrationIfrPullbackCycle::CoRegistrationIfrPullbackCycle() :
	runIndex			  (0),
    timestamp         (0),
    ifrTrendValue     (0.0),
    ifrRawValue       (0.0),
    isFiltered        (false),
    isValidCycle      (false),
    waveFreeStartTime (0),
    waveFreeEndTime   (0)
{
}
