// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform { namespace CoRegistration{

// This class implemented the timestamp validation of one type of timestamp used in the CoRegistrationSource class.
class TimeStampValidator
{
public:
	TimeStampValidator();

	bool check(double timeStamp);

	void start();
	void stop();

	enum class ValidationState
	{
		Deactivated,
		Start,
		Active
	};

private:
	double lastTimeStamp;
	ValidationState m_state;
};

}}