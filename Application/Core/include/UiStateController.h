// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "IUiStateController.h"
#include "UiState.h"
#include "PciViewContext.h"
#include "ViewState.h"

namespace Sense
{
	class Timer;
}

namespace CommonPlatform
{
	class Log;

	namespace TSM
	{
		class ITsmSource;
	}
}


namespace Pci { namespace Platform { namespace iControl {
	class ICopyToRefClient;
}}}

namespace iControlInterface = Pci::Platform::iControl;

namespace Pci { namespace Core
{
	namespace TSM = ::CommonPlatform::TSM;

	class IArchivingStatus;
	class IReplayControl;
	struct StbMarkers;
	struct StbImage;	
	class IUserPrefs;

	class UiStateController : public IUiStateController
	{
        friend class TestUiStateController;
	public:
		UiStateController(PciViewContext &viewContext,
						  Sense::IScheduler &scheduler,
						  TSM::ITsmSource& tsmSource,
						  IReplayControl& replayControl,
						  iControlInterface::ICopyToRefClient& copyToRefClient,
						  IUserPrefs& userPrefs,
						  IArchivingStatus* archivingStatus = nullptr);
		~UiStateController();

		void roadmapStatusChanged();
		void epxChanged();
		void angioProcessingStarted();
		void angioProcessingStopped();
		void overlayImageAvailable();
		void imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType);
        void endLive();
		void lastImageHold();
		void stentBoostImageAvailable(const std::shared_ptr<const XrayImage>& image,
									  const std::shared_ptr<const StbMarkers>& markers,
									  const std::shared_ptr<const StbImage>& boost);

		void boostImageAvailable(const std::shared_ptr<const StbImage>& boost, const std::shared_ptr<const StbImage>& registeredLive);
		void studyChanged();
        void geometryChanged();
        void geometryEnd();
		void apcStatusChanged();
		void coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState, const CoReg::CoRegistrationServerState& nextState);
		void coRegistrationError();
		void coRegistrationRunChanged(int runIndex);
		void coRegistrationConnectionChanged(bool connected);
		void connectionStatusChanged(bool connected);

        void switchState(ViewState nextState) override;
		
		void setStbRegionOfInterest(Sense::Rect roi) override;
		void startReplayRun(bool loop) override;
		void stopReplayRun() override;
		bool isReplayActive() override;
		
		UiState* getCurrentState() const;

		ViewState getSelectedView() const;
		std::wstring getSelectedViewName();

        std::function<void(ViewState)> eventViewStateChanged;
		std::function<void(Sense::Rect)> onStbRoiChanged;

		UiStateController(const UiStateController&) = delete;
		UiStateController& operator=(const UiStateController&) = delete;

	private:
		ViewState checkLicenseForRedirect(ViewState newState) const;

		PciViewContext&   m_pciViewContext;
		TSM::ITsmSource&  m_tsm;
		IArchivingStatus* m_archivingStatus;
		IReplayControl&   m_replayControl;
		Sense::Timer      m_autoSwitchTimer;

		const double      m_autoSwitchDelay;

		std::map<ViewState, std::unique_ptr<UiState>> m_states;
		
		UiState* m_currentUiState;
		UiState* m_autoReactivateState;

        CommonPlatform::Log& m_log; 

		void reactivatePreviousState();
		void LogStateShanged();
		bool isRequiredLicenseActive(LicenseFeature requiredLicense) const;
	};
}}

