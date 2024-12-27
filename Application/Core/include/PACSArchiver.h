// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <memory>
#include <vector>
#include <Sense/System/IScheduler.h>

#include "IPACSArchiver.h"

namespace CommonPlatform
{
	class Log;
	class Thread;
}

namespace Pci { namespace Core { namespace PACS
{
	class IMessageBuilder;
	class IPACSPush;
	class Snapshot;

	enum class GrabType
	{
		SingleFrame,
		MultiFrame
	};

	class PACSArchiver : public IPACSArchiver, public Sense::IAsync
	{
	public:
		PACSArchiver(Sense::IScheduler& scheduler, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, ::CommonPlatform::Thread& worker);
		~PACSArchiver();

		virtual void sendSnapshot(std::shared_ptr<Snapshot> frame, int seriesNumber, RunType runType) override;
		virtual void sendMovie(std::vector<std::shared_ptr<Snapshot>> frame, int framesPerSecond, int seriesNumber,RunType runType) override;
		
		std::function<void(GrabType grabType, bool finishedSuccessfully)> onSent;

	private:
		virtual void asyncInvoke(const std::function<void()>& command) const override;

		::CommonPlatform::Log& m_log;

		Sense::IScheduler& m_scheduler;
		IPACSPush& m_pacsPush;
		IMessageBuilder& m_msgBuilder;
		::CommonPlatform::Thread& m_worker;
	};


}}}