// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "UiState.h"
#include "PciXrayModel.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"

namespace CommonPlatform
{
	class Log;
}

namespace Pci {	
	
namespace Platform { namespace iControl {
	class ICopyToRefClient;
}}
	
namespace Core {

enum class ViewType;
enum class RecordingStatus;
class StbTsmPage;
class IArchivingSettings;
class StbRecording;

namespace PACS
{
	class MovieGrabber;
	struct ImageSequence;
}

class StbReviewState : public UiState
{
public: 
	StbReviewState(
		PciViewContext &viewContext, 
		ViewState state, 
		const std::wstring &name, 
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, 
		const IArchivingSettings& archivingSettings, 
		const LicenseFeature license);
	virtual ~StbReviewState();

	void onEnter() override;
	void onExit() override;

	virtual ViewState overlayImageAvailable() override;
	virtual ViewState epxChanged() override;
	virtual ViewState imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType) override;

	virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState, const CoReg::CoRegistrationServerState& nextState) override;
	virtual ViewState coRegistrationConnectionChanged(bool connected) override;

	virtual void onFrameGrabberFinished(PACS::GrabType grabType, bool finishedSuccessfully) override;
	void onMovieGrabberFinished(bool finishedSuccessfully, const std::shared_ptr<const PACS::ImageSequence>& movie);

protected:
	virtual PciXrayPositionModel& getModel() override;

	virtual void initFrameGrabber() = 0;
	virtual std::unique_ptr<PACS::MovieGrabber> createMovieGrabber();

	virtual bool isCopyToRefEnabled() const = 0;
	virtual std::wstring getCopyToRefTargetName() const = 0;

	virtual bool isAutoPACSArchivingMovieEnabled() const = 0;
	virtual bool isAutoPACSArchivingSnapshotEnabled() const = 0;

	virtual void prepareReview();
	virtual void sendSnapshot();
	virtual void sendSnapshotToRefViewer();
	virtual void recordMovie();

	virtual  ViewState getProcessingState() const;

	StbTsmPage&	getTsmPage() const;

	bool hasBoostedImages() const;

	CommonPlatform::Log& m_log;
	const IArchivingSettings& m_archivingSettings;

	std::shared_ptr<StbRecording> m_stbRecording;
	PciXrayModel m_boostModel;
	PciXrayModel m_markerModel;

	PciReviewPlayer<StbRecording> m_player;
private:
	void archiveRun();
	void initMovieGrabber();
	void sendMovie();

	Pci::Platform::iControl::ICopyToRefClient& m_copyToRefClient;
	std::unique_ptr<PACS::MovieGrabber>		   m_movieGrabber;
	std::shared_ptr<PACS::ImageSequence>	   m_movie;
};

}}