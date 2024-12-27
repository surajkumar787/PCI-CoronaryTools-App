// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbReviewState.h"

namespace Pci { namespace Core { 

class IUiStateController;


class StbPostDeployReviewState : public StbReviewState
{
public:
	friend class TestStbPostDeployReviewState;

	StbPostDeployReviewState(PciViewContext &viewContext, IUiStateController& stateController, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, const IArchivingSettings& archivingSettings);
	virtual ~StbPostDeployReviewState();

protected:
	virtual void onEnter() override;
	virtual void onExit() override;

	virtual void prepareReview() override;

private:
	virtual void initFrameGrabber() override;
	virtual void sendSnapshot() override;

	virtual bool isCopyToRefEnabled() const override;
	virtual std::wstring getCopyToRefTargetName() const override;

	virtual bool isAutoPACSArchivingMovieEnabled() const override;
	virtual bool isAutoPACSArchivingSnapshotEnabled() const override;
	
	IUiStateController& m_uiStateController;
};

}}
