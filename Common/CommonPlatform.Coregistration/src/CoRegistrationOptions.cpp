// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationOptions.h"

using namespace CommonPlatform::CoRegistration;

CoRegistrationOptions::CoRegistrationOptions() :
	maxPressure			(0),
	minPressure			(0),
	maxIfr				(0),
	minIfr				(0),
	showIfrRawLine		(false),
	showWaveFreePeriods	(false)
{
}
