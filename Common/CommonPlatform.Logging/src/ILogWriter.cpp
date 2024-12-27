
// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ILogWriter.h"
#include <sstream>
#include <Windows.h>
#include <psapi.h>

namespace CommonPlatform { namespace Logging {

static const std::wstring DateTimeFormat = L"yyyyMMdd-HHmmss.ffffff";
static const std::wstring Delimiter = L"\n";
static const std::wstring FieldSplitter = L"=";
static const std::wstring NewlineToken = L"[CRLF]"; // Newlines in field are replaced by this token in the eventlog

static const int PlatformOrginatorId		  = 200309;
static const int CoronaryRoadmapOrginatorId   = 200310;
static const int StentboostLiveOrginatorId    = 200311;

ILogWriter::ILogWriter(const std::wstring &sourceName):
	_hEventLog(RegisterEventSource(NULL, sourceName.c_str())),
	_originatorId()
{
	DWORD numChars = _countof( _computerName ) - 1;
	GetComputerNameW(_computerName,  &numChars);

	if (NULL == _hEventLog)
	{
		_hEventLog = RegisterEventSource(NULL, L"Application");
	}

	_originatorId[LoggerType::Platform]				= PlatformOrginatorId;
	_originatorId[LoggerType::CoronaryRoadmap]		= CoronaryRoadmapOrginatorId;
	_originatorId[LoggerType::StentboostLive]		= StentboostLiveOrginatorId;
	_originatorId[LoggerType::SetMonitorResolution]	= PlatformOrginatorId;
}


ILogWriter::~ILogWriter(void)
{
	if (_hEventLog != NULL)
	{
		DeregisterEventSource(_hEventLog);
	}
}

int  ILogWriter::GetOriginatorId(LoggerType logType)
{
	return _originatorId[logType];
}

static bool GetSystemTimeTimeStampWithMicroSeconds(LPSYSTEMTIME pSystemTime, int* pMicroseconds);

static void StringReplaceAll(std::wstring& str, const std::wstring& from, const std::wstring& to) 
{
	if(from.empty())
		return;

	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) 
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

std::wstring ILogWriter::BuildMessageString
	(
	PCWSTR logType,
	PCWSTR eventType, 
	PCWSTR severity, 
	int eventId, 
	PCWSTR eventName, 
	PCWSTR description, 
	PCWSTR additionalInfo,
	LoggerType originatorId,
	PCWSTR originatorName
	)
{
	SYSTEMTIME localTime;
	int        microseconds = 0;
	GetSystemTimeTimeStampWithMicroSeconds(&localTime, &microseconds);

	std::wstringstream wss;

	wss << L"ModuleID" << FieldSplitter << _originatorId[originatorId] << Delimiter;

	if (originatorName != NULL)
	{
		wss << L"HumanReadableModuleID" << FieldSplitter << originatorName << Delimiter;
	}

	wss << L"EventID" << FieldSplitter << eventId << Delimiter;

	if (eventName != NULL) 
	{
		std::wstring packedEventName(eventName);
		StringReplaceAll(packedEventName, L"\n", NewlineToken);
		StringReplaceAll(packedEventName, L"\r", L"");

		wss << L"HumanReadableEventID" << FieldSplitter << packedEventName << Delimiter;
	}

	WCHAR dateTimeString[23]; //"yyyyMMdd-HHmmss.ffffff";
	wsprintf(dateTimeString, L"%04d%02d%02d-%02d%02d%02d.%06d", localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour, localTime.wMinute, localTime.wSecond, microseconds);
	wss << L"DateTime" << FieldSplitter << dateTimeString << Delimiter;

	if (logType != NULL) 
	{
		wss << L"LogType" << FieldSplitter << logType << Delimiter;
	}

	if (eventType != NULL) 
	{
		wss << L"EventType" << FieldSplitter << eventType << Delimiter;
	}

	if (severity != NULL) 
	{
		wss << L"Severity" << FieldSplitter << severity << Delimiter;
	}

	if (description != NULL)
	{
		std::wstring packedDescription = std::wstring(description);
		StringReplaceAll(packedDescription, L"\n", NewlineToken);
		StringReplaceAll(packedDescription, L"\r", L"");

		wss << L"Description" << FieldSplitter << packedDescription << Delimiter;
	}

	wss << L"ThreadID" << FieldSplitter << GetCurrentThreadId() << Delimiter;

	wss << L"ProcessID" << FieldSplitter << GetCurrentProcessId() << Delimiter;

	if (additionalInfo != NULL) 
	{
		std::wstring packedAdditionalInfo(additionalInfo);
		StringReplaceAll(packedAdditionalInfo, L"\n", NewlineToken);
		StringReplaceAll(packedAdditionalInfo, L"\r", L"");

		wss << L"AdditionalInfo" << FieldSplitter << packedAdditionalInfo << Delimiter;
	}

	if (_computerName != NULL)
	{
		wss << L"MachineName" << FieldSplitter << _computerName << Delimiter;
	}

	HANDLE process = GetCurrentProcess();
	if (process != NULL)
	{
		WCHAR processName[1024];
		if (GetModuleBaseName(process, NULL, (LPWSTR)processName, 1024))
		{
			wss << L"ProcessName" << FieldSplitter << processName << Delimiter;
		}

		CloseHandle(process);
	}

	return wss.str();
}



bool GetSystemTimeTimeStampWithMicroSeconds(LPSYSTEMTIME pSystemTime, int* pMicroseconds)
{
	const unsigned int TicksPerMicroSecond         = 10;
	const unsigned int HundredNanoSecondsPerSecond = 10000000; // 1*10^7

	// Get the actual time stamp (UTC relative to windows epoch)
	FILETIME fileTime;
	GetSystemTimeAsFileTime(&fileTime);

	// Convert to local time
	FILETIME localFileTime;
	FileTimeToLocalFileTime(&fileTime, &localFileTime);

	// Convert to absolute system time
	SYSTEMTIME systemTime;
	if (FileTimeToSystemTime(&localFileTime, &systemTime) == FALSE) return false;

	// Get the number of ticks (of 100ns) by combining two 32bit values into 64bit
	ULARGE_INTEGER timestampInTicks;
	timestampInTicks.HighPart = localFileTime.dwHighDateTime;
	timestampInTicks.LowPart  = localFileTime.dwLowDateTime;

	*pSystemTime    = systemTime;

	// Calculate the fraction of seconds (in microseconds). 
	// Because epoch has been defined at a date/time with 0 microseconds, there is no difference between relative and absolute value of microseconds.
	*pMicroseconds  = (timestampInTicks.QuadPart % HundredNanoSecondsPerSecond) / TicksPerMicroSecond;

	return true;
}

}}