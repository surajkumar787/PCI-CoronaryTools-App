// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbReviewState.h"

namespace Pci { namespace Core { 


class StbLiveReviewState : public StbReviewState
{
public:
	friend class TestStbLiveReviewState;
	
	StbLiveReviewState(PciViewContext &viewContext, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,const IArchivingSettings& archivingSettings);
	virtual ~StbLiveReviewState();

protected:
	virtual void onEnter() override;
	virtual void onExit() override;

private:
	virtual void initFrameGrabber() override;

	virtual bool isCopyToRefEnabled() const override;
	virtual std::wstring getCopyToRefTargetName() const override;

	virtual bool isAutoPACSArchivingMovieEnabled() const override;
	virtual bool isAutoPACSArchivingSnapshotEnabled() const override;
};

}}
