// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TimeStampValidator.h"

namespace CommonPlatform { namespace CoRegistration{

TimeStampValidator::TimeStampValidator()
:	m_state(ValidationState::Deactivated)
{
}

bool TimeStampValidator::check(double timestamp)
{
	bool validTimeStamp = true;

	switch(m_state)
	{
		case ValidationState::Start:
			lastTimeStamp  = timestamp;
			m_state = ValidationState::Active;
			break;

		case ValidationState::Active:
			if (timestamp > lastTimeStamp)
			{
				lastTimeStamp = timestamp;
			}
			else
			{
				// timestamp is not increasing compared to previous timestamp
				validTimeStamp = false;
			}
			break;

		case ValidationState::Deactivated:
		default:
			break;
	}

	return validTimeStamp;
}

void TimeStampValidator::start()
{
	m_state = ValidationState::Start;
}

void TimeStampValidator::stop()
{
	m_state = ValidationState::Deactivated;
}

}}