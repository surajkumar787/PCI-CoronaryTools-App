// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "UiState.h"
#include "XrayImage.h"
#include "PciXrayModel.h"

namespace Pci { namespace Core {
	class CrmTsmPage;

class CoRegistrationState : public UiState
{
public:
    explicit CoRegistrationState(PciViewContext& viewContext);
	virtual ~CoRegistrationState() {}

	void stopReview();
	void startReview();

    virtual void onEnter() override;

	void updateArchivingStatus();

    virtual ViewState roadmapStatusChanged() override;
    virtual ViewState angioProcessingStarted() override;
	virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
		                                         const CoReg::CoRegistrationServerState& nextState) override;
	virtual ViewState epxChanged() override;
	virtual ViewState lastImageHold() override;
	virtual ViewState imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType) override;
	virtual ViewState coRegistrationRunChanged(int runIndex) override;
	virtual ViewState coRegistrationConnectionChanged(bool connected) override;
	virtual ViewState studyChanged() override;
	virtual ViewState overlayImageAvailable() override;
	virtual void      onFrameGrabberFinished(PACS::GrabType grabType, bool finishedSuccessfully) override;
	virtual	void      onExit() override;

protected:
	virtual PciXrayPositionModel& getModel() override;
	PciXrayModel model;

private:
	void startRecordingMovie();
	void stopRecordingMovie();

	void enableTsmControls();
	void enablePullbackControlsOnTsm();
	void enableArchiveControlsOnTsm();

	inline CrmTsmPage& getTsmPage() const;

	bool movieRecordingBusy;
	::CommonPlatform::Log& log;
};
}}

