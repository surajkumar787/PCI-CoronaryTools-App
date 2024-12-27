// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <functional>
#include <map>

#include "StbReviewState.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"
#include "PciXrayModel.h"
#include "StbRecording.h"

namespace CommonPlatform
{
	class Log;
}

namespace Pci {	namespace Core {

enum class ViewType;
class IUiStateController;
class StbTsmPage;
class IUserPrefs;

class StbContrastReviewState : public StbReviewState
{
	friend class TestStbContrastReviewState;
public:
	StbContrastReviewState(PciViewContext &viewContext, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, IUserPrefs& userPrefs, IUiStateController& uiController);
	~StbContrastReviewState();

	void onEnter() override;
	void onExit() override;

protected:
	StbTsmPage&	getTsmPage() const;

private:
	virtual void initFrameGrabber() override;
	virtual std::unique_ptr<PACS::MovieGrabber> createMovieGrabber() override;
	virtual void prepareReview() override;
	virtual void sendSnapshotToRefViewer() override;
	virtual void sendSnapshot() override;
	virtual void recordMovie() override;

	void pauseFading();
	void resumeFading();

	void invertBoostImage();

	virtual bool isCopyToRefEnabled() const override;
	virtual std::wstring getCopyToRefTargetName() const override;

	virtual bool isAutoPACSArchivingMovieEnabled() const override;
	virtual bool isAutoPACSArchivingSnapshotEnabled() const override;

	void updateBoostFrame();
	void toBoostFrame();
	void toContrastFrame();

	CommonPlatform::Log&					m_log;
	ViewType								m_viewType;

	bool									m_switch_away;
	bool									m_archivingDone;
	std::map<int, std::function<void()>>	m_tsmButtonCallbacks;
	IUiStateController&						m_uiController;
	bool									m_fadeInOutPaused;
	bool									m_defaultBoostInverted;	
	bool									m_boostInverted;
	
	static const double						SNAPSHOT_BLEND_RATIO;
};

}}