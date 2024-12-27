// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "Thread.h"
#include "CrmAlgoAngio.h"
#include "CrmRepository.h"

namespace Pci { namespace Core {

class PciThreadBackground
{
public:
	PciThreadBackground(IScheduler &scheduler, const CrmParameters &params, CrmRepository &repository);
	~PciThreadBackground();
	
	bool isBusy() const;

	void processRoadmap(const std::shared_ptr<const XrayImage> &image);
	void endLive();
	void cancel();

	std::function<void()>											onBusyChanged;
	std::function<void(const std::shared_ptr<const CrmRoadmap> &)>	onRoadmapCreated;
	std::function<void(CrmErrors::Error error)>						onRoadmapError;
	std::function<void()>											onRoadmapCanceled;

protected:
	
	Simplicity::ThreadPool	pool;
	CrmAlgoAngio			algorithm;
	::CommonPlatform::Thread	thread;
	int						queue;
	bool					active;
	bool					canceled;

	const CrmParameters &params;
    CrmRepository &repository;

	PciThreadBackground(const PciThreadBackground&);
	PciThreadBackground& operator=(const PciThreadBackground&);
};

}}