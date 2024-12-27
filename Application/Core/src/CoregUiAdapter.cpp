// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CoregUiAdapter.h"
#include "AlluraCoregUi.ui.h"
#include "SmartSuiteFlexvisionCoregUi.ui.h"
#include "SmartSuiteFullHDCoregUi.ui.h"

namespace Pci { namespace Core
{
#pragma push_macro("GETCONTROL")
#pragma push_macro("GETCONTROLASTYPE")

	// 'type' is used from the enclosing scope (see CoregUiAdapter ctor)
#define GETCONTROL(ControlName) \
	type == ViewType::Allura ? getGui<AlluraCoregUi>()->ControlName : \
	type == ViewType::SmartSuiteFullHD ? getGui<SmartSuiteFullHDCoregUi>()->ControlName : \
	type == ViewType::SmartSuiteFlexVision ? getGui<SmartSuiteFlexvisionCoregUi>()->ControlName : \
	getGui<SmartSuiteFlexvisionCoregUi>()->ControlName

#define GETCONTROLASTYPE(ControlName, Type) \
	type == ViewType::Allura ? static_cast<Type&>(getGui<AlluraCoregUi>()->ControlName) : \
	type == ViewType::SmartSuiteFullHD ? static_cast<Type&>(getGui<SmartSuiteFullHDCoregUi>()->ControlName) : \
	type == ViewType::SmartSuiteFlexVision ? static_cast<Type&>(getGui<SmartSuiteFlexvisionCoregUi>()->ControlName) : \
	static_cast<Type&>(getGui<SmartSuiteFlexvisionCoregUi>()->ControlName)

	CoregUiAdapter::CoregUiAdapter(Sense::Control& parent, ViewType type) :
		gui(createGui(parent, type)),
		fmTypeLabel(GETCONTROL(labeliFRfFR)),
		fmTypeValue(GETCONTROL(labeliFRfFRValue)),
		fmTypePanel(GETCONTROLASTYPE(paneliFrFfrNames, Sense::ExperienceIdentity::Panel)),
		pdPaLabel(GETCONTROL(labelPdPa)),
		pdPaValue(GETCONTROL(labelPdPaValue)),
	    pdPaPanel(GETCONTROLASTYPE(panelPaPdNames, Sense::ExperienceIdentity::Panel)),
		pdLabel(GETCONTROL(labelPd)),
		pdValue(GETCONTROL(labelPdValue)),
		paLabel(GETCONTROL(labelPa)),
		paValue(GETCONTROL(labelPaValue)),

		ifrReviewFmTypeLabel(GETCONTROL(ifrReviewPanel.fmTypeLabel)),
		ifrReviewTrendValue(GETCONTROL(ifrReviewPanel.fmTrendValue)),
		ifrReviewTrendLabel(GETCONTROL(ifrReviewPanel.ifrTrendLabel)),
		ifrReviewDistalFmValue(GETCONTROL(ifrReviewPanel.distalFmValueLabel)),
		ifrReview(GETCONTROLASTYPE(ifrReviewPanel, Sense::ExperienceIdentity::Panel)),
		ifrProgressBar(GETCONTROLASTYPE(progress, PciProgressBar)),
		ifrProgressPanel(GETCONTROLASTYPE(progressPanel, Sense::ExperienceIdentity::Panel)),
		IfrValuePanel(GETCONTROL(panelValues)),

		ifrSpotReviewPanel(GETCONTROLASTYPE(ifrSpotReviewPanel, Sense::ExperienceIdentity::Panel)),
		ifrSpotReviewValue(GETCONTROL(labeliFrSpotValue)),
		ifrSpotReviewLabel( GETCONTROL(labeliFrSpot)),
		
		popupPannel(GETCONTROLASTYPE(panelPopup, Sense::ExperienceIdentity::Panel)),
		popupHeader(GETCONTROLASTYPE(popupPanelHeader, Sense::ExperienceIdentity::Panel)),
		popupHeaderText(GETCONTROL(popupPanelHeaderText)),
		
		graphPanel(GETCONTROLASTYPE(graphPanel, Sense::ExperienceIdentity::Panel)),
		topPannel(GETCONTROLASTYPE(topLayOutPannel,Sense::ExperienceIdentity::Panel))
	{
		gui->layout = Layout::Fill;
	}
#pragma pop_macro("GETCONTROLASTYPE")
#pragma pop_macro("GETCONTROL")

	template<typename GuiType>
	GuiType* CoregUiAdapter::getGui()
	{
		return static_cast<GuiType*>(gui.get());
	}

	Sense::Control* CoregUiAdapter::createGui(Sense::Control& parent, ViewType type)
	{
		if (type == ViewType::Allura)
		{
			return new AlluraCoregUi(parent);
		}
		else if (type == ViewType::SmartSuiteFullHD) 
		{
			auto hdUi = new SmartSuiteFullHDCoregUi(parent);
			hdUi->firstRowPanel.color = Palette::Transparent;
			hdUi->secontRowPanel.color = Palette::Transparent;
			return hdUi;
		}
		else if (type == ViewType::SmartSuiteFlexVision)
		{
			return new SmartSuiteFlexvisionCoregUi(parent);
		}
		else if (type == ViewType::SmartSuite19Inch)
		{
			return new SmartSuiteFlexvisionCoregUi(parent);
		}
		return nullptr;
	}

}}