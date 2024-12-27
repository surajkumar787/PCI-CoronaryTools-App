// Copyright Koninklijke Philips N.V. 2017
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

namespace CommonPlatform { namespace Logging {

class SynchronousLogWriter : public ILogWriter
{
public:
	explicit SynchronousLogWriter(const std::wstring &sourceName);
	~SynchronousLogWriter();

	virtual void WriteLog(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName) override;
};
}}
