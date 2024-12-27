// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense/System/IScheduler.h>
#include <string>
#include <atomic>
#include <vector>

#include "IStorageCommitListener.h"
#include "Thread.h"
#include "application.h"
#include "secureassociation.h"
#include "ConnectionSettings.h"
#include "Log.h"

namespace CommonPlatform
{
	class Log;
}

namespace Pci { namespace Core { namespace PACS
{
	namespace Merge = ::CommonPlatform::Dicom::Merge;

	struct CommitNodeSettings;

	class StorageCommitListener : public IStorageCommitListener
	{
	public:
		StorageCommitListener(Sense::IScheduler &scheduler, const CommitNodeSettings& settings, const ConnectionSettings& connectionSettings);
		~StorageCommitListener();

	private:
		void HandleMessage(Merge::Association& assoc, Merge::Message& msg);
		void HandleEchoMessage(Merge::Association& assoc, Merge::Message& msg);
		void HandleEventReportMessage(Merge::Association& assoc, Merge::Message& msg);

		::CommonPlatform::Log& m_log;
		ConnectionSettings m_connectionSettings;
		void* m_exit;
		::CommonPlatform::Thread m_thread;
	};

}}}