// Copyright Koninklijke Philips N.V. 2020
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
	class StbImageSelectTsmPage;

	class StbBoostFrameSelectionState : public UiState
	{
	public:
		friend class TestStbBoostFrameSelectionState;

		explicit StbBoostFrameSelectionState(PciViewContext &viewContext, IUiStateController& controller, Pci::Platform::iControl::ICopyToRefClient& copyToRefClient);
		~StbBoostFrameSelectionState();

		void onEnter() override;

		ViewState overlayImageAvailable() override;
		ViewState epxChanged() override;

	private:
        StbImageSelectTsmPage& getTsmPage() const;

        void enableBoostImageSelectionButton();

		virtual PciXrayPositionModel&	getModel() override;

		void onNavigateLeftDown();
		void onNavigateRightDown();
		void onResetDefaultDown();
        void apply();
        void cancel();

		void setSelection(int index);

		CommonPlatform::Log&					m_log;
		PciXrayModel							m_boostModel;
		PciXrayModel							m_markerModel;

		std::vector<std::shared_ptr<const StbImage>>			m_referenceFrames;

		int										m_selectedImage;
		IUiStateController&						m_uiController;
		std::map<int, std::function<void()>>	m_tsmButtonCallbacks;
	};
}}



