// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ILogWriter.h"

namespace CommonPlatform {
	namespace Logging {

		class NullLogWriter : public ILogWriter
		{
		public:
			explicit NullLogWriter(const std::wstring &sourceName);
			~NullLogWriter();

			virtual void WriteLog(PCWSTR logType, PCWSTR eventType, PCWSTR severity, int eventId, PCWSTR eventName, PCWSTR description, PCWSTR additionalInfo, LoggerType originatorId, PCWSTR originatorName) override;
		};
	}
}

