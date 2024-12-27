// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationError.h"

namespace CommonPlatform { namespace CoRegistration
{

	std::wstring ToString(CoRegistrationError error)
	{
		switch (error)
		{
		case CommonPlatform::CoRegistration::CoRegistrationError::OK:                        return L"OK";
		case CommonPlatform::CoRegistration::CoRegistrationError::Unknown:                   return L"Unknown";
		case CommonPlatform::CoRegistration::CoRegistrationError::NormalizationError:        return L"NormalizationError";
		case CommonPlatform::CoRegistration::CoRegistrationError::FFRError:                  return L"FFRError";
		case CommonPlatform::CoRegistration::CoRegistrationError::IFRError:                  return L"IFRError";
		case CommonPlatform::CoRegistration::CoRegistrationError::CommandError:              return L"CommandError";
		case CommonPlatform::CoRegistration::CoRegistrationError::LatencyTooHigh:            return L"LatencyTooHigh";
		case CommonPlatform::CoRegistration::CoRegistrationError::UnreliableDataTransfer:    return L"UnreliableDataTransfer";
		default:                                                                             return L"";
		}
	}

}}