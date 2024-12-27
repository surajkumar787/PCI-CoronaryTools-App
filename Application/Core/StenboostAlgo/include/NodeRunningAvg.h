// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <IpSimplicity.h>

namespace Pci {	namespace StentBoostAlgo {

	class NodeRunningAvg
	{
	public:
		
		NodeRunningAvg(Simplicity::ThreadPool &threadPool);
		virtual ~NodeRunningAvg();

		void reset(int queueSize);
		void process(const Simplicity::Image<short> &input, Simplicity::Image<short> &output);
		bool current(Simplicity::Image<short> &output);

		NodeRunningAvg(const NodeRunningAvg&) = delete;
		NodeRunningAvg& operator=(const NodeRunningAvg&) = delete;

	protected:
		void initialize(int width, int height);

		std::vector<Simplicity::OmpImage<short>> _buffer;
		Simplicity::OmpImage<int>				 _sum;
		int										 _index;
		bool									 _reset;
		Simplicity::ThreadPool&					 _threadPool;
	
	};

}}
