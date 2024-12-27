// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "log.h"
#include <map>
#include <string>
#include "windows.h"

namespace CommonPlatform { namespace Logging {

	class  ILogWriter
	{
	public:
		explicit ILogWriter(const std::wstring &sourceName);
		virtual ~ILogWriter() = 0;

		virtual void WriteLog(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName) = 0;
		int  GetOriginatorId(LoggerType logType);

	protected:
		HANDLE		_hEventLog;
		wchar_t		_computerName[255];	
		std::map<LoggerType, int> _originatorId;
		std::wstring BuildMessageString(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, 
		PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName);

	};
}}

