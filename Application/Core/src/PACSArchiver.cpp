// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PACSArchiver.h"

#include <assert.h>

#include "Log.h"
#include "Thread.h"
#include "FrameGrabber.h"
#include "IMessageBuilder.h"
#include "IPACSPush.h"
#include "Snapshot.h"


namespace Pci { namespace Core { namespace PACS {

	PACSArchiver::PACSArchiver(Sense::IScheduler& scheduler, IPACSPush& pacsPush, IMessageBuilder& msgBuilder, CommonPlatform::Thread& worker) :
		m_log(::CommonPlatform::Logger(::CommonPlatform::LoggerType::Platform)),
		m_scheduler(scheduler),
		m_pacsPush(pacsPush),
		m_msgBuilder(msgBuilder),
		m_worker(worker)
	{
	}

	PACSArchiver::~PACSArchiver()
	{
	}

	void PACSArchiver::sendSnapshot(std::shared_ptr<Snapshot> frame, int seriesNumber, RunType runType)
	{
		if (frame == nullptr)
		{
			return;
		}

		m_log.DeveloperInfo(L"PACSArchiver: send snapshot");

		m_msgBuilder.setSeriesNumber(seriesNumber); 
      	m_worker.executeAsync([this, queuedMetaData = m_msgBuilder.getMetaData(), queuedFrame = std::move(frame),runType]
		{
			auto msg = m_msgBuilder.CreateSnapshotMessage(queuedMetaData, *queuedFrame, runType);
			if (msg)
			{
				m_pacsPush.sendSnapshot(queuedMetaData, *msg, [this, msg](bool success)
				{
					asyncInvoke([this, success] { if (onSent) onSent(GrabType::SingleFrame, success); });
				});
			}
			else
			{
				asyncInvoke([this] { if (onSent) onSent(GrabType::SingleFrame, false); });
			}
		});
	}

	void PACSArchiver::sendMovie(std::vector<std::shared_ptr<Snapshot>> frames, int framesPerSecond, int seriesNumber, RunType runType)
	{
		m_log.DeveloperInfo(L"PACSArchiver: send movie");

		if (frames.empty() == true)
		{
			// ERROR
			return;
		}

		auto& firstFrame = frames[0];

		assert(firstFrame->getFormat() == Format::Rgba32);
		assert(firstFrame->getWidth() != 0);
		assert(firstFrame->getHeight() != 0);

		m_msgBuilder.setSeriesNumber(seriesNumber);
		auto sequence = m_msgBuilder.StartMovie(m_msgBuilder.getMetaData(), framesPerSecond, firstFrame->getWidth(), firstFrame->getHeight(),runType);

        if (!sequence)
        {
            asyncInvoke([this] { if (onSent) onSent(GrabType::MultiFrame, false); });
            return;
        }

		for (auto& frame : frames)
		{
			m_msgBuilder.AddFrame(*sequence, *frame);
		}

		m_msgBuilder.EndMovie(*sequence);

		m_worker.executeAsync([this, queuedMovie = sequence]
		{
			m_pacsPush.sendMovie(queuedMovie->m_metaData, *queuedMovie->m_message, [this, queuedMovie](bool success)
			{
				asyncInvoke([this, success] { if (onSent) onSent(GrabType::MultiFrame, success); });
			});
		});
	}

	void PACSArchiver::asyncInvoke(const std::function<void()> &command) const
	{
		m_scheduler.addAsync(*this, command);
	}
}}}