
// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "AsyncLogWriter.h"
#include <memory>
#include <mutex>

namespace CommonPlatform { namespace Logging {

AsyncLogWriter::AsyncLogWriter(const std::wstring &sourceName):
	ILogWriter(sourceName),
	_messageQue(),
	_lockHandle(),
	_waitForMessageEvent( CreateEvent(NULL, true,false,NULL)),
	_stopEvent( CreateEvent(NULL,true,false,NULL))
{
	createMessageWorkerThread();
}

AsyncLogWriter::~AsyncLogWriter()
{
	BOOL succes = SetEvent(_stopEvent);

	succes &= WaitForSingleObject(_messageWorkerThread, static_cast<DWORD>(1000));
	succes &= CloseHandle( _messageWorkerThread );
	succes &= CloseHandle( _waitForMessageEvent );
	succes &= CloseHandle( _stopEvent );
}

void AsyncLogWriter::WriteLog(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName)
{
	std::wstring message = BuildMessageString(logType, eventType, severity, eventId, eventName, description, additionalInfo, originatorId, originatorName );
	LogMessage(message);
}


void AsyncLogWriter::LogMessage(const std::wstring &message)
{
	std::unique_lock<std::mutex> lock(_lockHandle);

	_messageQue.push_front(message);
	SetEvent( _waitForMessageEvent );
}

void AsyncLogWriter::createMessageWorkerThread()
{
	DWORD threadId;

	_messageWorkerThread = CreateThread( 
		NULL,          // default security attributes
		0,             // use default stack size  
		AsyncLogWriter::doWork,		   // thread function name
		this,          // argument to thread function 
		0,             // use default creation flags 
		&threadId);    // returns the thread identifier 
}

unsigned long __stdcall AsyncLogWriter::doWork(void *object)
{
	AsyncLogWriter *logClass = static_cast<AsyncLogWriter*>( object);
	return logClass->processMessageQueue();
}

unsigned long  AsyncLogWriter::processMessageQueue()
{
	std::wstring message;
	HANDLE waitHandles[]={ _stopEvent,_waitForMessageEvent};
	DWORD result = 0;

	do
	{
		if(getMessage(message))
		{
			LPCTSTR messageString = message.c_str();
			if (!ReportEvent(_hEventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &messageString, NULL))
			{
				DWORD error = GetLastError();
				wprintf(L"ReportEvent failed with 0x%x for event.\n", error);
			}
		}
		else
		{
			ResetEvent(_waitForMessageEvent);
		}

		result = WaitForMultipleObjects(_countof(waitHandles), waitHandles,false, INFINITE);
	}
	while(result == WAIT_OBJECT_0 + 1);

	return 0;
}

bool AsyncLogWriter::getMessage( std::wstring &messageString )
{
	std::unique_lock<std::mutex> lock(_lockHandle);

	if(_messageQue.empty())return false;

	messageString = std::move(_messageQue.front());
	_messageQue.pop_front();
	return true;
}

}}