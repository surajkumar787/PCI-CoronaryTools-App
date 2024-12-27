// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "ViewState.h"
#include "XrayImage.h"
#include "LicenseStatus.h"
#include <memory>
#include "ApplicationType.h"

namespace CommonPlatform
{
	class Log;
	namespace CoRegistration { struct CoRegistrationServerState; }
	namespace TSM { class TsmPage; }
}

namespace Sense { class Control; }

namespace CoReg = ::CommonPlatform::CoRegistration;


namespace Pci { namespace Core
{
	namespace PACS { class FrameGrabber; enum class GrabType; class IPACSArchiver; }
	class IArchivingStatus;
    struct PciViewContext;
    class PciTsmPage;
	struct StbImage;
	class PciXrayPositionViewer;
	class PciXrayPositionModel;
	struct StbMarkers;

    class UiState
    {
    public:
        UiState(PciViewContext &viewContext, ViewState state, const std::wstring &name, LicenseFeature feature);
        virtual ~UiState(void);

		virtual void onEnter() = 0;
		virtual void onExit() {};

        virtual ViewState epxChanged();
        virtual ViewState roadmapStatusChanged();
        virtual ViewState angioProcessingStarted();
        virtual ViewState angioProcessingStopped();
        virtual ViewState overlayImageAvailable();
        virtual ViewState imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType);
        virtual ViewState endLive();
        virtual ViewState lastImageHold();
        virtual ViewState stentBoostImageAvailable(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, const std::shared_ptr<const StbMarkers>& markers, const std::shared_ptr<const StbImage>& boost);
		virtual ViewState boostImageAvailable(const std::shared_ptr<const StbImage>& boost, const std::shared_ptr<const StbImage>& registeredLive);
		virtual ViewState studyChanged();
		virtual ViewState geometryChanged();
        virtual ViewState geometryEnd();
		virtual ViewState apcStatusChanged();
		virtual ViewState coRegistrationStateChanged(const CoReg::CoRegistrationServerState& prevState,
			                                         const CoReg::CoRegistrationServerState& nextState);
		virtual ViewState coRegistrationError();
		virtual ViewState connectionStatusChanged(bool connected);
		virtual ViewState coRegistrationRunChanged(int runIndex);
		virtual ViewState coRegistrationConnectionChanged(bool connected);
		virtual void      onFrameGrabberFinished(PACS::GrabType, bool /*finishedSuccessfully*/) {}

        bool isAutoReactivated() const;
		void resetAutoReactivate();
        std::wstring getStateName() const;
		CommonPlatform::TSM::TsmPage* getTsmPage() const { return tsmPage.get(); }
		LicenseFeature getLicenseFeature() const { return m_feature; }

		void setArchivingStatusSink(IArchivingStatus* archivingStatus);

        const ViewState viewState;

		UiState(const UiState&) = delete;
		UiState& operator=(const UiState&) = delete;

    protected:
		virtual PciXrayPositionModel& getModel() = 0;

		void show(Sense::Control *control);
		void showCoRegControl(const CoReg::CoRegistrationServerState& coRegState);
		bool isPressureWireConnectedAndReady(const CoReg::CoRegistrationServerState& coRegState) const;
		
		virtual void initFrameGrabber(Sense::ICanvas& canvas, const Sense::Control& control1, const Sense::Control& control2);
		virtual void initFrameGrabber(Sense::ICanvas& canvas, const Sense::Control& control);

		void initPACSArchiving(Sense::IScheduler& scheduler);
		void initPACSArchiving(std::unique_ptr<PACS::IPACSArchiver> pacsArchiver);

        const std::wstring  stateName;
        PciViewContext      &pciViewContext;
		std::unique_ptr<PACS::FrameGrabber> m_frameGrabber;
		std::unique_ptr<PACS::IPACSArchiver> m_pacsArchiver;
		IArchivingStatus* m_archivingStatus;
		bool allowGeometryUpdates;

        bool autoReactivate;
        std::unique_ptr<CommonPlatform::TSM::TsmPage> tsmPage;

	private:
		ViewState getGuidanceStateByEpxSelection() const;

		LicenseFeature m_feature;
    };

}}


