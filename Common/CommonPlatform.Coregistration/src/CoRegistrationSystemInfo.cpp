// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoRegistrationSystemInfo.h"

namespace CommonPlatform { namespace CoRegistration
{

CoRegistrationSystemInfo::CoRegistrationSystemInfo() :
	swVersion(),
	protocolVersion(),
	serialNumber(),
	license(false)
{
}

bool operator==(const CoRegistrationSystemInfo& left, const CoRegistrationSystemInfo& right)
{
	return left.swVersion       == right.swVersion &&
	       left.protocolVersion == right.protocolVersion &&
	       left.serialNumber    == right.serialNumber &&
	       left.license         == right.license;
}

bool operator!=(const CoRegistrationSystemInfo& left, const CoRegistrationSystemInfo& right)
{
	return !(left == right);
}

}}