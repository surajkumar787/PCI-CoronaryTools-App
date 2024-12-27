// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Thread.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace CommonPlatform
{
	struct ScopedSection
	{
		ScopedSection(CRITICAL_SECTION* section) : _section(section)
		{
			EnterCriticalSection(_section);
		}

		~ScopedSection()
		{
			if (_section != nullptr)
			{
				LeaveCriticalSection(_section);
			}
		}

		void unlock()
		{
			if (_section != nullptr)
			{
				LeaveCriticalSection(_section);
				_section = nullptr;
			}
		}

	private:
		CRITICAL_SECTION* _section;
	};

	Thread::Thread(Sense::IScheduler &scheduler, const std::string& name) :
		debugThreadName(name),
		section(std::make_unique<CRITICAL_SECTION>()),
		scheduler(scheduler),
		queued(CreateEvent(NULL, TRUE, FALSE, nullptr)),
		done(CreateEvent(NULL, TRUE, FALSE, nullptr)),
		exit(CreateEvent(NULL, TRUE, FALSE, nullptr)),
		thread(nullptr)
	{
		InitializeCriticalSection(section.get());
		thread = CreateThread(NULL, 0, threadProc, this, 0, 0);
	}

	Thread::~Thread()
	{
		scheduler.delAsync(*this);

		if (thread != nullptr)
		{
			SetEvent(exit);
			WaitForSingleObject(thread, INFINITE);
			CloseHandle(thread);
		}

		CloseHandle(queued);
		CloseHandle(done);
		CloseHandle(exit);

		DeleteCriticalSection(section.get());
	}

	bool Thread::isBusy() const
	{
		ScopedSection cs(section.get());
		return !requests.empty() || WaitForSingleObject(queued, 0) == WAIT_OBJECT_0;
	}

	void Thread::clear()
	{
		ScopedSection cs(section.get());
		requests.clear();
	}

	int Thread::queueSize() const
	{
		ScopedSection cs(section.get());
		return static_cast<int>(requests.size()) + (WaitForSingleObject(queued, 0) == WAIT_OBJECT_0 ? 1 : 0);
	}

	void Thread::flush() const
	{
		{
			ScopedSection cs(section.get());
			if (requests.empty() && (WaitForSingleObject(queued, 0) == WAIT_TIMEOUT))
			{
				return;
			}
			else
			{
				ResetEvent(done);
			}
		}
		WaitForSingleObject(done, INFINITE);
	}

	void Thread::asyncInvoke(const std::function<void()> &command) const
	{
		scheduler.addAsync(*this, command);
	}

	void Thread::executeUI(const std::function<void()> &function) const
	{
		asyncInvoke(function);
	}

	void Thread::executeAsync(const std::function<void()> &function)
	{
		ScopedSection cs(section.get());
		requests.push_back(function);
		SetEvent(queued);
	}

	void Thread::run()
	{
		HANDLE events[] = { exit, queued };

		while (WaitForMultipleObjects(2, events, FALSE, INFINITE) == WAIT_OBJECT_0 + 1)
		{
			ScopedSection cs(section.get());
			if (requests.empty())
			{
				ResetEvent(queued);
				SetEvent(done);
			}
			else
			{
				std::function<void()> function = requests.front();
				requests.pop_front();

				cs.unlock();

				if (function) function();
			}
		}
	}

	constexpr DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
	struct THREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	};
#pragma pack(pop)
	void SetThreadName(DWORD dwThreadID, const char* threadName)
	{
		THREADNAME_INFO info = { 0x1000, threadName, dwThreadID, 0 };
#pragma warning(push)
#pragma warning(disable: 6320 6322)
		__try
		{
			RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), reinterpret_cast<ULONG_PTR*>(&info));
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{}
#pragma warning(pop)
	}

	unsigned long __stdcall Thread::threadProc(void *param)
	{
		auto thread = reinterpret_cast<Thread*>(param);
		SetThreadName(GetCurrentThreadId(), thread->debugThreadName.c_str());

		thread->run();
		return 0;
	}

}
