// Copyright Koninklijke Philips N.V. 2019
// 
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <queue>
#include <condition_variable>
#include <mutex>
#include <chrono>

namespace CommonPlatform { namespace Utilities {

template<typename T>
class ConcurrentQueue
{
public:
	T pop()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		while (m_queue.empty())
		{
			m_queueCv.wait(lock);
		}
		auto val = m_queue.front();
		m_queue.pop();
		return val;
	}

	bool pop(T& item, std::chrono::milliseconds timeout)
	{
		bool popped(false);
		auto timeoutPoint = std::chrono::steady_clock::now() + timeout;

		std::unique_lock<std::mutex> lock(m_queueMutex);
		if (m_queueCv.wait_until(lock, timeoutPoint, [&]() { return !m_queue.empty(); }))
		{
			item = m_queue.front();
			m_queue.pop();

			popped = true;
		}
		return popped;
	}

	void push(const T& item)
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_queue.push(item);
		lock.unlock();
		m_queueCv.notify_one();
	}

	bool empty() const
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		return m_queue.empty();
	}

	size_t size() const
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		return m_queue.size();
	}

	void clear()
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		while (!m_queue.empty())
		{
			m_queue.pop();
		}
	}

	ConcurrentQueue() = default;
	ConcurrentQueue(const ConcurrentQueue&) = delete;
	ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;

private:
	std::condition_variable m_queueCv;
	mutable std::mutex m_queueMutex;
	std::queue<T> m_queue;
};

}
}