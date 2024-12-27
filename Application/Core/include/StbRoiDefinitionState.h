// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <functional>
#include <map>

#include "UiState.h"
#include "PciReviewPlayer.h"
#include "StbRecording.h"
#include "PciXrayModel.h"
#include "IArchivingSettings.h"

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
	class IUiStateController;
    class StbRoiTsmPage;

	class StbRoiDefinitionState : public UiState
	{
	public:
		friend class TestStbRoiDefinitionState;

		explicit StbRoiDefinitionState(PciViewContext &viewContext, 
									   ViewState state,
									   const std::wstring &name, 
			                           IUiStateController& controller, 
									   ViewState returnState,
                                       ViewState cancelState,
                                       Pci::Platform::iControl::ICopyToRefClient& copyToRefClient,
									   const IArchivingSettings& archivingSettings);
		virtual ~StbRoiDefinitionState();

		void onEnter() override;

		ViewState overlayImageAvailable() override;
		ViewState imageRunStarted(CommonPlatform::Xray::XrayImage::Type imageType) override;
		ViewState epxChanged() override;
		ViewState stentBoostImageAvailable(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image, 
			const std::shared_ptr<const StbMarkers>& markers,
			const std::shared_ptr<const StbImage>& boost) override;

		ViewState coRegistrationConnectionChanged(bool connected) override;

        StbRoiTsmPage& getTsmPage() const;

	protected:
		virtual std::wstring getCopyToRefTargetName() const = 0;

		const IArchivingSettings& m_archivingSettings;

	private:
		virtual PciXrayPositionModel&	getModel() override;

		void updateRoi(const Sense::Rect& markerRoi);
		void updateTSMControls();
		Sense::Rect moveROIWithinBoundries(const Sense::Rect& rect) const;
		Sense::Rect setRoiAsSquare(Sense::Rect markerRoi);
		Sense::Rect createDefaultROI() const;

		void onPanUp();
		void onPanDown();
		void onPanLeft();
		void onPanRight();

		void onIncrease();
		void onDecrease();

		void apply();
        void cancel();

		CommonPlatform::Log&					m_log;
		ViewType								m_viewType;
		PciXrayModel							m_boostModel;
		PciXrayModel							m_markerModel;

		IUiStateController&						m_uiController;

		double									m_zoomSpeed;
		Sense::Rect								m_regionOfInterest;
		Sense::Rect								m_imageBoundries;

		std::map<int, std::function<void()>>	m_tsmButtonCallbacks;

		ViewState								m_returnState;
        ViewState                               m_cancelState;

		static const int						MINIMUM_ROI_SIZE;
	};
}}



