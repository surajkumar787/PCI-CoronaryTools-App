// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "NodeRunningAvg.h"

namespace Pci {	namespace StentBoostAlgo {

using namespace Simplicity;

NodeRunningAvg::NodeRunningAvg(ThreadPool &pool)
:
	_threadPool	(pool),
	_reset		(true),
	_index		(0)
{
}

NodeRunningAvg::~NodeRunningAvg()
{
}

void NodeRunningAvg::initialize(int width, int height)
{
	for(auto &i:_buffer) i.resize(width, height);
	_sum.resize(width, height);
	_reset = false;
	_index = 0;
}

void NodeRunningAvg::process(const Image<short> &input, Image<short> &output)
{
	if (_buffer.empty()) return;

	if (_reset || input.width != _sum.width || input.height != _sum.height) initialize(input.width, input.height);

	OmpImage<short> &stack = _buffer[_index % _buffer.size()];
	
	bool add = _index >= 1;
	bool sub = _index >= _buffer.size();
	
	_index++;
	int count = std::min(_index, int(_buffer.size()));

	_threadPool.execute([&](Thread &thread)
	{
		_threadPool.profileBegin("NodeRunningAvg::process");
		
		Strip strip = thread.getStrip(output);
	
		for(int y = strip.top; y < strip.bottom; y++)
		for(int x = 0; x < output.width; x++)
		{			
			if (add)	_sum.pointer[y*_sum.stride + x] += input.pointer[y*input.stride+x];	
			else		_sum.pointer[y*_sum.stride + x]  = input.pointer[y*input.stride+x];	
			if (sub)	_sum.pointer[y*_sum.stride + x] -= stack.pointer[y*stack.stride+x];
			
			stack .pointer[y*stack .stride + x] = input.pointer[y*input.stride+x];
			output.pointer[y*output.stride + x] = short(_sum.pointer[y*_sum.stride+x]/count);
		}
		
		_threadPool.profileEnd();
	});
}

bool  NodeRunningAvg::current(Image<short> &output)
{
	int count = std::min(_index, int(_buffer.size()));
	
	_threadPool.execute([&](Thread &thread)
	{
		_threadPool.profileBegin("NodeRunningAvg::process");
		
		Strip strip = thread.getStrip(output);
		
		for(int y = strip.top; y < strip.bottom; y++)
		for(int x = 0; x < output.width; x++)
			output.pointer[y*output.stride+x] = count == 0 ? 8192 : short(_sum.pointer[y*_sum.stride+x]/count);
		
		_threadPool.profileEnd();
	});

	return true;
}

void NodeRunningAvg::reset(int size)
{
	_reset = true;
	_index = 0;
	_buffer.resize(size);
}

}}