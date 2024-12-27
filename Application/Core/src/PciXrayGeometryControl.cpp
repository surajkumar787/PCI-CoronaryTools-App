// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciXrayGeometryControl.h"
#include "Translation.h"
#include "AlluraColors.h"
#include "ViewType.h"
#include "Sense.h"


namespace Pci { namespace Core
{
	using namespace Sense;
	using namespace Sense::ExperienceIdentity;

PciXrayGeometryControl::PciXrayGeometryControl(Control& parent, ViewType viewType) :
	UI::PciXrayGeometryControlUi(parent),
	color(UI::ColorAlluraLightGreyFont, [this]()
	      {
	          labelAngulationName.color = color;
	          labelAngulationValue.color = color;
	          labelRotationName.color = color;
	          labelRotationValue.color = color;
	      }),
	outline(Sense::Color(0.0), [this]()
		  {
			  labelAngulationName.outline = outline;
			  labelAngulationValue.outline = outline;
			  labelRotationName.outline = outline;
			  labelRotationValue.outline = outline;
		  }),
	rotation(0, [this]() { updateRotation(); }),
	angulation(0, [this]() { updateAngulation(); })
{
	labelAngulationName.color = color;
	labelAngulationValue.color = color;
	labelRotationName.color = color;
	labelRotationValue.color = color;

	if(viewType == ViewType::Allura)
	{
		labelRotationValue.horizontalAlign = HAlign::Right;
		labelAngulationValue.horizontalAlign = HAlign::Right;
	}
	else
	{
		labelRotationValue.horizontalAlign = HAlign::Left;
		labelAngulationValue.horizontalAlign = HAlign::Left;
	}

	labelAngulationName.text = L"";
	labelRotationName.text = L"";
}

void PciXrayGeometryControl::updateAngulation()
{
	auto roundedValue = static_cast<int>(abs(angulation) + 0.5);
	if (roundedValue == 0)
	{
		labelAngulationName.text = L"";
	}
	else if (angulation < 0)
	{
		labelAngulationName.text = Localization::Translation::IDS_AngulationCRAN;
	}
	else
	{
		labelAngulationName.text = Localization::Translation::IDS_AngulationCAUD;
	}

	wchar_t buf[32] = {};
	swprintf_s(buf, L"%d°", roundedValue);
	labelAngulationValue.text = buf;
}

void PciXrayGeometryControl::updateRotation()
{
	auto roundedValue = static_cast<int>(abs(rotation) + 0.5);
	if (roundedValue == 0)
	{
		labelRotationName.text = L"";
	}
	else if (rotation < 0)
	{
		labelRotationName.text = Localization::Translation::IDS_RotationRAO;
	}
	else
	{
		labelRotationName.text = Localization::Translation::IDS_RotationLAO;
	}

	wchar_t buf[32] = {};
	swprintf_s(buf, L"%d°", roundedValue);
	labelRotationValue.text = buf;
}

}}