// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationIfrSpotCycle.h"

using namespace CommonPlatform::CoRegistration;

CoRegistrationIfrSpotCycle::CoRegistrationIfrSpotCycle() :
	runIndex			  (0),
    timestamp         (0),
    isValidCycle      (false),
    waveFreeStartTime (0),
    waveFreeEndTime   (0)
{

}
