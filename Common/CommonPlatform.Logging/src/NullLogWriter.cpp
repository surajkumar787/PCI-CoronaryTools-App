// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "ILogWriter.h"
#include "NullLogWriter.h"

namespace CommonPlatform {
	namespace Logging {

NullLogWriter::NullLogWriter(const std::wstring &sourceName):
	ILogWriter(sourceName)
{
}


NullLogWriter::~NullLogWriter()
{
}

void NullLogWriter::WriteLog(PCWSTR, PCWSTR, PCWSTR, int, PCWSTR, PCWSTR, PCWSTR, LoggerType, PCWSTR)
{
}

}}