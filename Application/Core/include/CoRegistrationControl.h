// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "PciBaseViewer.h"
#include "FmTrendDisplay.h"
#include "PressureDisplay.h"
#include "PullbackRecording.h"
#include "CoRegistrationServerState.h"
#include "CoregUiAdapter.h"

#include <locale.h>

namespace Sense { 
	class Control; 
	class Timer;
}

namespace CommonPlatform { namespace CoRegistration {
    struct CoRegistrationPressureAndEcg;
    struct CoRegistrationIfrPullbackCycle;
    struct CoRegistrationPdPaInfo;
	struct CoRegistrationSpotMeasurement;
	struct CoRegistrationIfrSpotCycle;
	struct CoRegistrationOptions;
	enum class CoRegistrationError;
}}


namespace Pci { namespace Core {

    class CoRegistrationControl : public PciBaseViewer
    {

    public:
        CoRegistrationControl(Sense::Control &parent, ViewType type);
        virtual ~CoRegistrationControl();

		void setIfrSpotResult	(float iFrCycle);

		void setPressureAndEcg(const ::CommonPlatform::CoRegistration::CoRegistrationPressureAndEcg   &pressAndEcg);
        void setIfrPullbackCycle(const ::CommonPlatform::CoRegistration::CoRegistrationIfrPullbackCycle &iFrCycle);
        void setPdPaInfo        (const ::CommonPlatform::CoRegistration::CoRegistrationPdPaInfo         &pdPaInfo);
        void setCoRegState      (const ::CommonPlatform::CoRegistration::CoRegistrationServerState		&coRegState);

		void setSpotMeasurement	(const ::CommonPlatform::CoRegistration::CoRegistrationSpotMeasurement	&coRegSpot);
		void setCoRegOptions	(const ::CommonPlatform::CoRegistration::CoRegistrationOptions			&options);
		void setCoRegError		(	   ::CommonPlatform::CoRegistration::CoRegistrationError			 error);

		void setCoRegConnection (bool connected);

		void setReview			(double timestamp);
		void setRecording		(const std::shared_ptr<PullbackRecording> &recording);

		void setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image);
		void endLive();

		std::function<void(const std::shared_ptr<const CommonPlatform::Xray::XrayImage> &image)> eventReviewImage;

		void showFmDisplay();
		void hideFmDisplay();

		void showTotalPdPaDisplay();
		void hideTotalPdPaDisplay();

		void showIfrReviewPanel();
		void hideIfrReviewPanel();

		void showValuePanel(const ::CommonPlatform::CoRegistration::CoRegistrationServerState& coRegState);
		void hideValuePanel();

		void showIfrSpotReviewPanel();
		void hideIfrSpotReviewPanel();

		void showPopupPanel(::CommonPlatform::CoRegistration::CoRegistrationServerState::FmState state);
		void hidePopupPanel();

		bool isFmDisplayVisible() const { return fmDisplay.visible; }
		bool isIfrSpotReviewPanelVisible() const { return gui.ifrSpotReviewPanel->visible; }
		bool isIfrValuesHighLighted() const;
		bool isIfrPullbackReviewPanelVisible() const { return gui.ifrReview->visible; }

		void clearGraphs();
		bool allGraphsEmpty() const;
		bool allValuesInvalid() const;

		void resetFmDisplayScale();

		CoRegistrationControl(const CoRegistrationControl&) = delete;
		CoRegistrationControl& operator=(const CoRegistrationControl&) = delete;

	protected:
		CoregUiAdapter gui;
    private:

		_locale_t userLocale;

        FmTrendDisplay  fmDisplay;
		PressureDisplay pdPaDisplay;
		PressureDisplay totalPdPaDisplay;

		std::shared_ptr<PullbackRecording> recording;

		::CommonPlatform::CoRegistration::CoRegistrationPdPaInfo currentPdPaInfo;
		::CommonPlatform::CoRegistration::CoRegistrationServerState currentState;

		bool m_coregConnected;
		::CommonPlatform::CoRegistration::CoRegistrationError m_lastError;
		bool m_enableValueUpdates;

		Sense::Timer progressTimer;
		void onProgressTimerTick();

		void setGraphTop(Control& graph);
		void setGraphBottom(Control& graph);
		void setGraphFull(Control& graph);

		std::wstring ifrFloatToString (float ifrValue);
		std::wstring pdPaDoubleToString(double pdPaValue);

		void setFfrRecording (const std::shared_ptr<PullbackRecording> &recording, double cursorPosition);
		void invalidateValues();
		void enableValueUpdates(bool enable);
		void clearGraphsAndValues();
		bool notConnecedOrUpdatesDisabled() const;
	};
}}

