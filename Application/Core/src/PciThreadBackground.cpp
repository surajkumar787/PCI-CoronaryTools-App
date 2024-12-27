// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "PciThreadBackground.h"
#include "CrmRepository.h"

using namespace ::Pci::Core;

PciThreadBackground::PciThreadBackground(IScheduler &scheduler, const CrmParameters &params, CrmRepository &repository)
:
	pool		(false, "background"),
	algorithm	(pool, params),
	thread		(scheduler),
	queue		(0),
	active		(false),
	canceled	(false),
	params		(params),
    repository  (repository)
{
}

PciThreadBackground::~PciThreadBackground()
{
}

bool PciThreadBackground::isBusy() const
{
	return queue > 0;
}


void PciThreadBackground::processRoadmap(const std::shared_ptr<const XrayImage> &image)
{
	if (!active)
	{
		queue++;
		active = true;
		if (queue == 1 && onBusyChanged) onBusyChanged();
	}

	thread.executeAsync([this, image]
	{
		algorithm.process(image);
	});
}

void PciThreadBackground::cancel()
{
	canceled  = true;
}

void PciThreadBackground::endLive()
{
	if (!active) return;
	active = false;
	canceled = false;

	thread.executeAsync([this]
	{
		std::shared_ptr<CrmRoadmap> roadmap = std::make_shared<CrmRoadmap>();
		CrmErrors::Error error = algorithm.end(roadmap);

		if (error == CrmErrors::Error::OK && !canceled)
		{
            // Also save resulting data on background thread.
            error = repository.saveRoadmapToFile(roadmap);
		}

		thread.executeUI([this, roadmap, error]
		{ 
			if(canceled) 
			{
				if (onRoadmapCanceled)   onRoadmapCanceled();
			} 
			else if (error == CrmErrors::Error::OK)		
			{
				if (onRoadmapCreated)	onRoadmapCreated(roadmap); 
			}
			else
			{
				if (onRoadmapError)	onRoadmapError(error);	
			}

			queue--;
			if (queue == 0 && onBusyChanged) onBusyChanged();
		});
	});
}

