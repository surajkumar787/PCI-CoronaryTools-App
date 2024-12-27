// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "ILogWriter.h"
#include <string>
#include <deque> 
#include <memory>
#include <mutex>


namespace CommonPlatform { namespace Logging {


class AsyncLogWriter :public ILogWriter
{
public:
	explicit AsyncLogWriter(const std::wstring &sourceName);
	~AsyncLogWriter();

	virtual void WriteLog(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName) override;

private:
	HANDLE _waitForMessageEvent;
	HANDLE _stopEvent;
	HANDLE _messageWorkerThread;
	std::mutex _lockHandle;		
	std::deque<std::wstring> _messageQue;

	void LogMessage(const std::wstring &message);
	bool getMessage( std::wstring &messageString);
	void createMessageWorkerThread();	
	unsigned long  processMessageQueue();

	static unsigned long __stdcall doWork(void *object);
};
}}
