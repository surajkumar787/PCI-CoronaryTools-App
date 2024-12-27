// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciEpxSelectionView.h"
#include "PciGuidanceModel.h"
#include "AlluraEpxSelection.ui.h"
#include "SmartSuiteEpxSelection.ui.h"
#include "ViewType.h"

namespace Pci { namespace Core {

PciEpxSelectionView::PciEpxSelectionView(Sense::Control &parent, ViewType type):
	PciXrayPositionViewer(parent, type),
	_model(nullptr)
{
	if (type == ViewType::Allura)
	{
		view.reset(new UI::AlluraEpxSelectionUi(*this));
	}
	else
	{
		view.reset(new UI::SmartSuiteEpxSelectionUi(*this));
		view->layout = Sense::Layout::Fill;
	}
}

template<typename View>
void setPicture(View* view, const Sense::Image *image)
{
	if (!image || image->empty())
	{
		view->picture.visible = false;
		return;
	}
	auto lock = (*image).data();
	view->picture.setImage(lock);
	view->picture.visible = true;
};

template<typename View>
void setGuidanceModelForView(View* view, PciGuidanceModel* model)
{
	model->messagePicture.setNotifier( [view](const Sense::Image *icon)  { setPicture(view, icon); });
	model->messagePictureText.setNotifier( [view](const std::wstring &value) { view->labelAcquisition.text = value; view->labelAcquisition.visible = !value.empty(); });
	model->messageHeaderText.setNotifier( [view](const std::wstring &value) { view->labelMessage.text = value; });
}

void PciEpxSelectionView::setGuidanceModel(PciGuidanceModel &guidanceModel)
{
	setModel(guidanceModel);
	_model = &guidanceModel;
	if (viewType == ViewType::Allura)	setGuidanceModelForView(static_cast<UI::AlluraEpxSelectionUi*>(view.get()), &guidanceModel);
	else								setGuidanceModelForView(static_cast<UI::SmartSuiteEpxSelectionUi*>(view.get()), &guidanceModel);
}

}}