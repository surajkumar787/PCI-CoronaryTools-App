// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationPressureAndEcg.h"

using namespace CommonPlatform::CoRegistration;

CoRegistrationPressureAndEcg::CoRegistrationPressureAndEcg() :
    timestamp    (0),
	origTimestamp(0),
    ecgValue     (0),
    pdValue      (0),
    paValue      (0)
{
}
