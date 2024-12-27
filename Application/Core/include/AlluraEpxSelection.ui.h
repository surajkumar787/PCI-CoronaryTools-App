// ********************* WARNING *****************************
// * This file was automatically generated by Sense Composer *
// *      Modifying this file manually is frowned upon       *
// ***********************************************************
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>
#include "AlluraColors.h"

namespace Pci { namespace Core { namespace UI
{

using namespace Sense;
using namespace Sense::ExperienceIdentity;

class AlluraEpxSelectionUi : public Control
{
public:

	AlluraEpxSelectionUi(Control &parent)
	:
		Control               (parent),
		container             (*this),
		panelMessage          (container),
		control               (panelMessage),
		picture               (control),
		labelAcquisition      (picture),
		control2              (panelMessage),
		labelMessage          (control2)
	{
		layoutUpdateBegin();

		size                             = Size(1000, 1000);

		container.size                   = Size(1000, 1000);
		container.layout                 = Layout::Fill;

		panelMessage.position            = Point(0, 696);
		panelMessage.size                = Size(1000, 304);
		panelMessage.layoutSpacing       = 0;
		panelMessage.layoutOrientation   = Orientation::Horizontal;
		panelMessage.border              = Margin(1, 0, 0, 0);
		panelMessage.edge                = Margin(0, 0, 0, 0);
		panelMessage.color               = Color(0.568627, 0.568627, 0.568627);

		control.size                     = Size(176, 304);
		control.layout                   = Layout::Fixed1d;

		picture.position                 = Point(24, 24);
		picture.size                     = Size(140, 140);

		labelAcquisition.position        = Point(0, 60);
		labelAcquisition.size            = Size(140, 40);
		labelAcquisition.font            = Font(L"Philips Healthcare Font", 18, true, false, false, false, Font::Antialias::High);
		labelAcquisition.text            = L"Acquisition";
		labelAcquisition.color           = UI::PanelDropShadow;
		labelAcquisition.horizontalAlign = HAlign::Center;
		labelAcquisition.verticalAlign   = VAlign::Middle;

		control2.position                = Point(176, 0);
		control2.size                    = Size(823, 304);
		control2.layout                  = Layout::Fill;
		control2.layoutOuterMargin       = Margin(24, 16, 8, 16);

		labelMessage.position            = Point(24, 16);
		labelMessage.size                = Size(791, 272);
		labelMessage.font                = Font(L"Philips Healthcare Font", 29, true, false, false, false, Font::Antialias::High);
		labelMessage.layout              = Layout::Fill;
		labelMessage.text                = L"Activate Dynamic Coronary Roadmap or StentBoost Live by selecting the appropriate acquisition protocol on the touchscreen";
		labelMessage.textType            = TextType::WordWrap;
		labelMessage.color               = Palette::Black;

		layoutUpdateEnd();
	}

public:

	Control    container;
	Panel      panelMessage;
	Control    control;
	PictureBox picture;
	Label      labelAcquisition;
	Control    control2;
	Label      labelMessage;

private:

	AlluraEpxSelectionUi(const AlluraEpxSelectionUi&);
	AlluraEpxSelectionUi& operator=(const AlluraEpxSelectionUi&);
};

}}}