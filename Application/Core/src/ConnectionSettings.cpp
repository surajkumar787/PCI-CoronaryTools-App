// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ConnectionSettings.h"

#include "Settings.h"
#include "Log.h"

namespace Pci { namespace Core { namespace PACS 
{
	static const int DefaultMaxPduLength = 64234;
	static const int DefaultTimeoutAssocOpen = 30;
	static const int DefaultTimeoutReceiveRequest = 30;
	static const int DefaultTimeoutReceiveResponse = 30;

	Pci::Core::PACS::ConnectionSettings::ConnectionSettings(CommonPlatform::Settings& settings, const wchar_t* group) :
		MaxPduLength(settings.get(group, L"maxPduLength", DefaultMaxPduLength)),
		TimeoutAssocOpen(settings.get(group, L"timeoutAssocOpen", DefaultTimeoutAssocOpen)),
		TimeoutReceiveRequest(settings.get(group, L"timeoutReceiveRequest", DefaultTimeoutReceiveRequest)),
		TimeoutReceiveResponse(settings.get(group, L"timeoutReceiveResponse", DefaultTimeoutReceiveResponse))
	{
		CommonPlatform::Logger(CommonPlatform::LoggerType::Platform).DeveloperInfo(
			L"Loaded PACS Connection settings.\n"
			L"MaxPduLength = %d\n"
			L"Timeout Association open = %d\n"
			L"Timeout Receive request = %d\n"
			L"Timeout Receive response = %d\n",
			MaxPduLength, TimeoutAssocOpen, TimeoutReceiveRequest, TimeoutReceiveResponse);
	}

}}}