// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once
#include "UiState.h"
#include "PciXrayModel.h"

namespace Pci { namespace Core {

namespace PACS {
	class CrmMovieGrabber;
	struct ImageSequence;
}

class CrmTsmPage;
class IUiStateController;
enum class RecordingStatus;

class CrmOverlayState : public UiState
{
	friend class TestCrmOverlayState;

public:
    explicit CrmOverlayState(PciViewContext &viewContext, IUiStateController& uiStateController);
	virtual ~CrmOverlayState();

    virtual void onEnter() override;
	virtual void onFrameGrabberFinished(PACS::GrabType grabType, bool finishedSuccessfully) override;

    virtual ViewState roadmapStatusChanged() override;
    virtual ViewState angioProcessingStarted() override;
	virtual ViewState overlayImageAvailable() override;
	virtual ViewState lastImageHold() override;		
	virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
												    const CoReg::CoRegistrationServerState& nextState) override;

protected:
	virtual PciXrayPositionModel& getModel() override;

	PciXrayModel model;
private:
	CrmTsmPage& getTsmPage() const;

	void initMovieGrabber();
	void recordMovie();
	void sendMovie();
	void onMovieGrabberFinished(bool finishedSuccessfully, const std::shared_ptr<const PACS::ImageSequence>& movie);

	std::unique_ptr<PACS::CrmMovieGrabber> m_movieGrabber;
	std::shared_ptr<PACS::ImageSequence>   m_movie;

	RecordingStatus							   m_movieRecordingStatus;

	CommonPlatform::Log& m_log;
};
}}

