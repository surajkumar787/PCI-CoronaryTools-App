#pragma once

#include <functional>

class IThreadPool
{
public:
	virtual ~IThreadPool() {}
	virtual void execute(const std::function<void(int, int)> &function) = 0;
};

