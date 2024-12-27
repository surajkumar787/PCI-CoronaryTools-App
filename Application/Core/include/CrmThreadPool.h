// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "IThreadPool.h"
#include <IpSimplicity/Simplicity/ThreadPool.h>
#include <algorithm>
#undef min

namespace Pci{ namespace Core {

class CrmThreadPool : public IThreadPool
{
public:
	CrmThreadPool(Simplicity::ThreadPool &pool) : m_pool(pool) {}

	CrmThreadPool(const CrmThreadPool&) = delete;
	CrmThreadPool& operator=(const CrmThreadPool&) = delete;

	void execute(const std::function<void(int, int)> &function) override
	{
		m_pool.execute([&](Simplicity::Thread &thread)
		{
			if (thread.getIndex() < maxNumThreads) function(thread.getIndex(), std::min(maxNumThreads, thread.getCount()));
		});
	}

	static const int maxNumThreads = 3;
private:
	Simplicity::ThreadPool &m_pool;

};

}}