// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Log.h"

#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <strsafe.h>
#include "AsyncLogWriter.h"
#include "SynchronousLogWriter.h"
#include "NullLogWriter.h"


namespace CommonPlatform{

static const int DEVELOPER_EVENT_ID           = 0;
static const WCHAR DEVELOPER_EVENT_NAME[]     = L"Developer";

static const WCHAR LOG_TYPE_SERVICE[]         = L"Service";
static const WCHAR LOG_TYPE_STATISTICAL[]     = L"Statistical";
static const WCHAR LOG_TYPE_STANDARD[]        = L"Standard";

static const WCHAR LOG_EVENT_TYPE_NONE[]      = L"None";
static const WCHAR LOG_EVENT_TYPE_EXCEPTION[] = L"Exception";

static const WCHAR LOG_SEVERITY_ERROR[]       = L"Error";
static const WCHAR LOG_SEVERITY_WARNING[]     = L"Warning";
static const WCHAR LOG_SEVERITY_INFO[]        = L"Info";


Log::Log(Logging::ILogWriter& logWriter, LoggerType originatorId, PCWSTR originatorName) :
	_logWriter(logWriter),
	_originatorId(originatorId)
{
	wcscpy_s(_originatorName, originatorName);
}

Log::~Log()
{
}

void Log::ServiceAppError(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	_logWriter.WriteLog(LOG_TYPE_SERVICE, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_ERROR, eventId, eventName, description, additionalInfo, _originatorId , _originatorName);
}

void Log::ServiceSwAppError(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	_logWriter.WriteLog(LOG_TYPE_SERVICE, LOG_EVENT_TYPE_EXCEPTION, LOG_SEVERITY_ERROR, eventId, eventName, description, additionalInfo,_originatorId,_originatorName);
}

void Log::ServiceAppWarning(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	_logWriter.WriteLog(LOG_TYPE_SERVICE, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_WARNING, eventId, eventName, description, additionalInfo, _originatorId , _originatorName);
}

void Log::UtilizationStartAction(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	std::wstring desc(L"Start: ");
	desc += description;

	_logWriter.WriteLog(LOG_TYPE_STATISTICAL, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_INFO, eventId, eventName, desc.c_str(), additionalInfo, _originatorId , _originatorName);
}

void Log::UtilizationStopAction(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	std::wstring desc(L"Stop: ");
	desc += description;

	_logWriter.WriteLog(LOG_TYPE_STATISTICAL, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_INFO, eventId, eventName, desc.c_str(), additionalInfo, _originatorId , _originatorName);
}


void Log::UtilizationUserAction(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	// to do check if this is correct
	std::wstring desc(L"UserAction: ");
	desc += description;

	_logWriter.WriteLog(LOG_TYPE_STATISTICAL, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_INFO, eventId, eventName, desc.c_str(), additionalInfo, _originatorId , _originatorName);
}


void Log::UtilizationUserMessage(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	std::wstring desc(L"User message: ");
	desc += description;

	_logWriter.WriteLog(LOG_TYPE_STATISTICAL, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_INFO, eventId, eventName, desc.c_str(), additionalInfo, _originatorId , _originatorName);
}

void Log::UtilizationAppEvent(int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo)
{
	_logWriter.WriteLog(LOG_TYPE_STATISTICAL, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_INFO, eventId, eventName, description, additionalInfo, _originatorId , _originatorName);
}

void Log::DeveloperError(PCWSTR format, ...)
{
	wchar_t buffer[2048];
	va_list args;
	va_start(args, format);
	StringCbVPrintf(buffer, sizeof(buffer), format, args);
	_logWriter.WriteLog(LOG_TYPE_STANDARD, LOG_EVENT_TYPE_EXCEPTION, LOG_SEVERITY_ERROR, DEVELOPER_EVENT_ID, DEVELOPER_EVENT_NAME, buffer, nullptr, _originatorId , _originatorName);
	va_end (args);
}

void Log::DeveloperWarning(PCWSTR format, ...)
{
	wchar_t buffer[2048];
	va_list args;
	va_start(args, format);
	StringCbVPrintf(buffer, sizeof(buffer), format, args);
	_logWriter.WriteLog(LOG_TYPE_STANDARD, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_WARNING, DEVELOPER_EVENT_ID, DEVELOPER_EVENT_NAME, buffer, nullptr, _originatorId , _originatorName);
	va_end (args);
}

void Log::DeveloperInfo(PCWSTR format, ...)
{
	wchar_t buffer[2048];
	va_list args;
	va_start(args, format);
	StringCchVPrintfW(buffer, _countof(buffer), format, args);
	_logWriter.WriteLog(LOG_TYPE_STANDARD, LOG_EVENT_TYPE_NONE, LOG_SEVERITY_INFO, DEVELOPER_EVENT_ID, DEVELOPER_EVENT_NAME, buffer, nullptr, _originatorId , _originatorName);
	va_end (args);
}

int  Log::GetOriginatorId()
{
	return _logWriter.GetOriginatorId(_originatorId);
}


Log& CommonPlatform::Logger(LoggerType type)
{
	static Logging::AsyncLogWriter asyncLogWriter(L"PciApp");
	static Logging::SynchronousLogWriter customResLogWriter(L"SetCustomResolution");
	static Logging::SynchronousLogWriter shutdownLogWriter(L"SystemShutdownListener");
	static Logging::NullLogWriter nullLoggerWriter(L"NullLogWriter");
	static Log Platformlogger(asyncLogWriter, LoggerType::Platform, L"Coronary Tools");
	static Log Crmlogger(asyncLogWriter, LoggerType::CoronaryRoadmap, L"Dynamic Coronary Roadmap");
	static Log Sbllogger(asyncLogWriter, LoggerType::StentboostLive, L"StentBoost Live");
	static Log CustomResolution(customResLogWriter, LoggerType::SetMonitorResolution , L"Custom Resolution Creator");
	static Log SystemShutdownListener(shutdownLogWriter, LoggerType::SystemShutdownListener, L"System Shutdown listener");
	static Log NullLogger(nullLoggerWriter, LoggerType::Platform, L" NullLogger");

	switch (type)
	{
		case LoggerType::Platform:
			return Platformlogger;
		case LoggerType::CoronaryRoadmap:
			return Crmlogger ;
		case LoggerType::StentboostLive:
			return Sbllogger;
		case LoggerType::SetMonitorResolution:
			return CustomResolution;
		case LoggerType::SystemShutdownListener:
			return SystemShutdownListener;
		case LoggerType::NullLogger:
			return NullLogger;
	}

	return Platformlogger;
}
}