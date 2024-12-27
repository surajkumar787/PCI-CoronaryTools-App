// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense/System/IScheduler.h>

#include <deque>
#include <functional>
#include <string>
#include <memory>

// avoid needing to include Windows.h
typedef void * HANDLE;
struct _RTL_CRITICAL_SECTION; typedef _RTL_CRITICAL_SECTION RTL_CRITICAL_SECTION; typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;

namespace CommonPlatform
{

class Thread : public Sense::IAsync
{
public:
	explicit Thread(Sense::IScheduler &scheduler, const std::string& name = "Thread");
	Thread(const Thread&) = delete;
	Thread& operator=(const Thread&) = delete;

	virtual ~Thread();
	
    // Execute function on the thread.
	void executeAsync(const std::function<void()> &function);
    // Execute function on the client thread (via the scheduler).
	void executeUI   (const std::function<void()> &function) const;

	bool isBusy() const;
	void clear();
	int queueSize() const;

	void flush() const;

protected:
	void run();

private:
	std::string debugThreadName;
	std::deque<std::function<void()>> requests;

	mutable std::unique_ptr<CRITICAL_SECTION> section;
	HANDLE queued;
	HANDLE done;
	HANDLE exit;
	HANDLE thread;

	Sense::IScheduler &scheduler;

protected:
	void asyncInvoke(const std::function<void()> &command) const override;
	static unsigned long __stdcall threadProc(void *param);

};


}