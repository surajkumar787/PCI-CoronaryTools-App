// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.


#include "PciGuidanceView.h"
#include "PciGuidanceModel.h"
#include "AlluraColors.h"
#include "ViewType.h"
#include "Translation.h"

#include "AlluraGuidance.ui.h"
#include "SmartSuiteGuidance.ui.h"

namespace Pci { namespace Core {

using namespace Sense;
using namespace Sense::ExperienceIdentity;
static const double RoadMapImageSize = 280.0;


PciGuidanceView::PciGuidanceView(Control &parent, ViewType type):
	PciXrayPositionViewer(parent, type),
	_model(nullptr)
{
	if (type == ViewType::Allura)
	{
		auto alluraView = std::unique_ptr<UI::AlluraGuidanceUi>(new UI::AlluraGuidanceUi(*this));
		alluraView->topStandard.labelHeader.color       = UI::ColorAlluraLightGreyFont;
		alluraView->topStandard.positionTarget.color    = UI::ColorAlluraLightGreyFont;
		alluraView->topStandard.imageRoadmap.roi        = Rect(0, 0, RoadMapImageSize, RoadMapImageSize);
		alluraView->topStandard.imageRoadmap.background = Palette::Black;
		alluraView->topStandard.imageRoadmap.thumb      = false;
		alluraView->topStandard.visible = true;
		alluraView->topSBS.visible = false;

		alluraView->topSBS.stentboostProgress.timerName.text = L"StentBoost";
		alluraView->topSBS.contrastProgress.timerName.text = Localization::Translation::IDS_InjectContrast;

		view = std::move(alluraView);
	}
	else
	{
		auto smsView = std::unique_ptr<UI::SmartSuiteGuidanceUi>(new UI::SmartSuiteGuidanceUi(*this));
		smsView->topStandard.imageRoadmap.roi        = Rect(0, 0, RoadMapImageSize, RoadMapImageSize);
		smsView->topStandard.imageRoadmap.background = Palette::Black;
		smsView->topStandard.imageRoadmap.thumb      = false;
		smsView->topStandard.visible = true;
		smsView->topSBS.visible = false;

		smsView->topSBS.stentboostProgress.timerName.text = L"StentBoost";
		smsView->topSBS.contrastProgress.timerName.text = Localization::Translation::IDS_InjectContrast;

		view = std::move(smsView);
		view->layout = Sense::Layout::Fill;
	}
}

template<typename View>
void setGuidanceModelForView(View* view, PciGuidanceModel* model);

void PciGuidanceView::setGuidanceModel(PciGuidanceModel &guidanceModel)
{
	_model = &guidanceModel;

	if (viewType == ViewType::Allura) setGuidanceModelForView(static_cast<UI::AlluraGuidanceUi*>(view.get()), &guidanceModel);
	else							  setGuidanceModelForView(static_cast<UI::SmartSuiteGuidanceUi*>(view.get()), &guidanceModel);

	setModel(guidanceModel);
}

const PciGuidanceModel* PciGuidanceView::getGuidanceModel() const
{
	return _model;
}

void PciGuidanceView::updateXrayGeometry(const CommonPlatform::Xray::XrayGeometry& geo)
{
	PciXrayPositionViewer::updateXrayGeometry(geo);

	if (viewType == ViewType::Allura) static_cast<UI::AlluraGuidanceUi*>(view.get())->topStandard.imageRoadmap.setGeometry(geo);
	else							  static_cast<UI::SmartSuiteGuidanceUi*>(view.get())->topStandard.imageRoadmap.setGeometry(geo);
}

template<typename View>
void setPicture(View* view, const Sense::Image *image)
{
	if ((image == nullptr) || image->empty())
	{
		view->panelImage.visible = false;
		return;
	}

	view->picture.setImage((*image).data());
	view->panelImage.visible = true;
};

template<typename View>
void setStatusImage(View* view, const Sense::Image *image)
{
	if ((image == nullptr) || image->empty())
	{
		view->topStandard.imageHeader.visible = false;
		return;
	}

	view->topStandard.imageHeader.image = *image;
	view->topStandard.imageHeader.visible = true;
};

template<typename View>
void setIcon(View* view, const Sense::Image *icon)
{
	if (icon != nullptr)
	{
		view->labelMessage.image = *icon;
	}
	else
	{
		view->labelMessage.image = Sense::Image();
	}
}

template<typename View>
void setRoadmap(View* view, PciXrayPositionModel* model, const std::shared_ptr<const CrmRoadmap> &roadmap)
{
	if (!roadmap)
	{
		view->topStandard.imageRoadmap.visible = false;
		return;
	}

	view->topStandard.imageRoadmap.setRoadmap(roadmap);
	view->topStandard.imageRoadmap.setGeometry(model->currentGeometry);
	view->topStandard.imageRoadmap.visible = true;
};

template<typename View>
void setGuidanceModelForView(View* view, PciGuidanceModel* model)
{
	model->showMessagePanel.setNotifier(       [view](bool value)                { view->panelMessage.visible = value; });
	model->messageHeaderText.setNotifier(      [view](const std::wstring &value) { view->labelMessage.text = value; });
	model->messageHeaderColor.setNotifier(     [view](const Sense::Color &color) { view->labelMessage.color = color; });
	model->messageHeaderIcon.setNotifier(      [view](const Sense::Image *icon)  { setIcon(view, icon); });
	model->messageText.setNotifier(            [view](const std::wstring &value) { view->labelMessageSubtext.text = value; });
	model->messageColor.setNotifier(           [view](const Sense::Color &color) { view->labelMessageSubtext.color = color; });
	model->guidanceText.setNotifier(           [view](const std::wstring &value) { view->labelGuidance.text = value; });

	model->showMessagePicturePanel.setNotifier([view](bool value)                { view->panelImage.visible = value; });
	model->messagePicture.setNotifier(         [view](const Sense::Image *icon)  { setPicture(view, icon); });
	model->messagePictureText.setNotifier(     [view](const std::wstring &value) { view->labelIcon.text = value; view->labelIcon.visible = !value.empty(); });

	model->showProcessingPanel.setNotifier(    [view](bool show)                         { view->topStandard.processing.visible = show; });
	model->status.setNotifier(          [view](const std::wstring &value)         { view->topStandard.labelHeader.text = value; view->topStandard.labelHeader.visible = !value.empty(); });
	model->statusIcon.setNotifier(             [view](const Sense::Image *image)         { setStatusImage(view, image); });
	model->roadMapImage.setNotifier(           [view, model](const std::shared_ptr<const CrmRoadmap> &roadmap) { setRoadmap(view, model, roadmap); });
	model->roadmapActive.setNotifier(          [view](bool active)                       { view->topStandard.imageRoadmap.active = active; });
	model->showPositionPanel.setNotifier(      [view](bool show)                         { view->topStandard.positionTarget.visible = show; });
	model->currentRoadmapAngulation.setNotifier([view](double angulation)                 { view->topStandard.positionTarget.angulation = angulation; });
	model->currentRoadmapRotation.setNotifier(  [view](double rotation)                   { view->topStandard.positionTarget.rotation = rotation; });

	model->showSBSGuidance.setNotifier(        [view](bool sbsViewVisible) { view->topSBS.visible = sbsViewVisible; view->topStandard.visible = !sbsViewVisible; });
	model->stentBoostPhaseDuration.setNotifier([view](double duration) { view->topSBS.stentboostProgress.setDuration(duration); });
	model->contrastPhaseActive.setNotifier([view](bool contrastPhaseActive) { contrastPhaseActive ? view->topSBS.contrastProgress.setCountDownToInProgress() : view->topSBS.contrastProgress.setCountDownToNotStarted();  });
	model->stentBoostPhaseActive.setNotifier([view](bool boostPhaseActive) { boostPhaseActive ? view->topSBS.stentboostProgress.setCountDownToInProgress() : view->topSBS.stentboostProgress.setCountDownToNotStarted();  });
	model->stentBoostTimeRemaining.setNotifier([view](double timeRemaining) { view->topSBS.stentboostProgress.onTimeChanged(timeRemaining); if (timeRemaining == 0) view->topSBS.stentboostProgress.setCountDownToComplete(); });
}

}}