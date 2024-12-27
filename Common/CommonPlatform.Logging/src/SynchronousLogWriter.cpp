
// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ILogWriter.h"
#include "SynchronousLogWriter.h"

namespace CommonPlatform { namespace Logging {

SynchronousLogWriter::SynchronousLogWriter(const std::wstring &sourceName):
	ILogWriter(sourceName)
{
}

SynchronousLogWriter::~SynchronousLogWriter(void)
{
}


void SynchronousLogWriter::WriteLog(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName)
{
	std::wstring message = BuildMessageString(logType, eventType, severity, eventId, eventName, description, additionalInfo, originatorId, originatorName );
	LPCTSTR messageString = message.c_str();
	ReportEvent( _hEventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, &messageString, NULL);
}

}}