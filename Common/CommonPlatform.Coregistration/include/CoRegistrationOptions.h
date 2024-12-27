// Copyright Koninklijke Philips Electronics N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform { namespace CoRegistration{

struct CoRegistrationOptions
{
public:
	CoRegistrationOptions();
	
	float maxPressure;
	float minPressure;
	float maxIfr;
	float minIfr;

	bool showIfrRawLine;
	bool showWaveFreePeriods;
};

}}

