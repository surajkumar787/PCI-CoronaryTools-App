// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "UiState.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"
#include "PciXrayModel.h"
#include "PciGuidanceModel.h"

namespace CommonPlatform
{
	class Log;
}


namespace Pci {	
	
namespace Platform { namespace iControl {
	class ICopyToRefClient;
}}
	
namespace Core{

enum class ViewType;
enum class MovieRecordingStatus;
class StbTsmPage;
class IArchivingSettings;
class IUiStateController;

namespace PACS
{
	class MovieGrabber;
	struct ImageSequence;
}

class StbProcessingState : public UiState
{
public:
	explicit StbProcessingState(
		PciViewContext &viewContext, 
		ViewState state, 
		const std::wstring &name, 
		const LicenseFeature license, 
		IUiStateController& controller, 
		Pci::Platform::iControl::ICopyToRefClient& copyToRefClient, 
		const IArchivingSettings& archivingSettings);
	virtual ~StbProcessingState();

	void			  onEnter() override;
	virtual ViewState epxChanged() override;
	virtual ViewState imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType) override;
	virtual ViewState lastImageHold() override;
	virtual ViewState endLive() override;
	virtual ViewState stentBoostImageAvailable(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, const std::shared_ptr<const StbMarkers>& markers, const std::shared_ptr<const StbImage>& boost) override;
	virtual ViewState boostImageAvailable(const std::shared_ptr<const StbImage>& boost, const std::shared_ptr<const StbImage>& registeredLive) override;
	virtual ViewState overlayImageAvailable() override;
	virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,	const CoReg::CoRegistrationServerState& nextState) override;
	virtual ViewState coRegistrationConnectionChanged(bool connected) override;
	virtual void      onExit() override;

protected:
	virtual PciXrayPositionModel&	getModel() override;
	PciXrayModel&					getBoostModel();
	PciXrayModel&					getMarkerModel();
	void							resetView();
	bool							isExposerRunTooShort() const;
	void							setRunActive(bool active);

	StbTsmPage&						getTsmPage() const;

	virtual ViewState				getGuidanceState() const = 0;
	virtual ViewState				getReviewState() const = 0;
	virtual std::wstring            getCopyToRefTargetName() const = 0;

	CommonPlatform::Log&			m_log;
	bool							m_runActive;
	IUiStateController&             m_uiStateController;
	PciGuidanceModel				_guidanceModel;
	const IArchivingSettings&	    m_archivingSettings;
private:
	bool									   m_reviewPaused;

	ViewType								   m_viewType;
	bool									   m_archivingDone;
	PciXrayModel							   m_boostModel;
	PciXrayModel							   m_markerModel;
};

}}



