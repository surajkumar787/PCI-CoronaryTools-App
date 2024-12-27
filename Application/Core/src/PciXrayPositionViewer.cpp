// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciXrayPositionViewer.h"

#include "ViewType.h"
#include "XrayGeometry.h"
#include "AlluraColors.h"
#include "PciXrayPositionModel.h"

#include "Sense/Common/Font.h"

using namespace CommonPlatform::Xray;
namespace Pci { namespace Core
{

	static const double XrayPositionControlHeight = 154.0;
	static const double XrayPositionControlWidth = 48.0;
	static const double AzurionTextSize = 18.0;
	static const double TextSpacingFormImage = 20.0;
	static const double BottomMargin = 28.0;
	static const double LeftMargin = 10.0;

	PciXrayPositionViewer::PciXrayPositionViewer(Control& parent, ViewType type) :
		PciBaseViewer(parent, type),
		showXrayPosition(true, [this] { currentXrayPosition.visible = showXrayPosition; }),
		model(nullptr),
		currentXrayPosition(*this,type)
	{
		if (type == ViewType::Allura)
		{
			currentXrayPosition.color = UI::ColorAlluraLightGreyFont;
			currentXrayPosition.font = Sense::Font(L"Philips Healthcare Font", 28, false, false, false, false, Sense::Font::Antialias::None);
		}
		else
		{
			currentXrayPosition.size = Sense::Size(XrayPositionControlHeight, XrayPositionControlWidth);
			currentXrayPosition.color = Sense::ExperienceIdentity::Palette::Yellow010;
			Sense::Color outlineColor;
			outlineColor.a = 0.5;
			currentXrayPosition.outline = outlineColor;
			currentXrayPosition.font = Sense::Font(L"Philips Healthcare Font", AzurionTextSize, false, false, false, false, Sense::Font::Antialias::None);
		}
	}

	void PciXrayPositionViewer::setModel(PciXrayPositionModel& newModel)
	{
		model = &newModel;
		model->currentGeometry.setNotifier([this](const XrayGeometry& geo)
		{
			updateXrayGeometry(geo);
		});
	}

	void PciXrayPositionViewer::updateXrayGeometry(const XrayGeometry& geo)
	{
		currentXrayPosition.angulation = geo.angulation;
		currentXrayPosition.rotation = geo.rotation;
	}

	void PciXrayPositionViewer::onResize()
	{
		PciBaseViewer::onResize();

		if (viewType == ViewType::Allura)
		{
			currentXrayPosition.position = Sense::Point(1, TextSpacingFormImage);
		}
		else
		{
			currentXrayPosition.position = Sense::Point(LeftMargin, size->height - currentXrayPosition.size->height - BottomMargin);
		}
	}
}}