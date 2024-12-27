// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>

typedef const wchar_t* PCWSTR;

namespace CommonPlatform
{
	enum class LoggerType
	{
		SetMonitorResolution,
		SystemShutdownListener,
		Platform,
		CoronaryRoadmap,
		StentboostLive,
		NullLogger
	};
	
	namespace Logging{
	class ILogWriter;
	}

	class Log
	{
	public:
		Log(Logging::ILogWriter& logWriter, LoggerType originatorId, PCWSTR originatorName);
		
		~Log(void);

		void LogEvent(PCWSTR  description);

		void ServiceAppError(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );
		void ServiceSwAppError(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );
		void ServiceAppWarning(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );

		void UtilizationStartAction(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );
		void UtilizationStopAction(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );
		void UtilizationUserMessage(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );
		void UtilizationUserAction(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );
		void UtilizationAppEvent(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo = NULL );

		void DeveloperError(PCWSTR format, ...);
		void DeveloperWarning(PCWSTR format, ...);
		void DeveloperInfo(PCWSTR format, ...);
		int  GetOriginatorId();


	private:
		Logging::ILogWriter&		_logWriter;
		LoggerType					_originatorId;
		wchar_t						_originatorName[255];

		Log(const Log&);
		void operator= (const Log&);
	};

	Log& Logger(LoggerType type);
}