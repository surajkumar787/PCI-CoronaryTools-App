// Copyright Koninklijke Philips Electronics N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

namespace CommonPlatform { namespace CoRegistration{

	struct CoRegistrationSystemInfo
	{
		CoRegistrationSystemInfo();

		std::wstring	swVersion;
		std::wstring	protocolVersion;
		std::wstring	serialNumber;
		bool			license;
	};

	bool operator==(const CoRegistrationSystemInfo& left, const CoRegistrationSystemInfo& right);
	bool operator!=(const CoRegistrationSystemInfo& left, const CoRegistrationSystemInfo& right);

}}

