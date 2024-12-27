// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PictorialItem.h"

namespace Pci { namespace Core {

PictorialItem::PictorialItem(Sense::Control& parent) : UI::PictorialItem(parent),
	locked(false),
	selected(false)
{
	layout = Layout::Fixed1d;
}

PictorialItem::~PictorialItem()
{
}

Sense::Control& PictorialItem::getContentControl()
{
	return imageBox;
}

void PictorialItem::setLabel(const std::wstring& label)
{
	pictorialLabel.text = label;
}

void PictorialItem::used()
{
    panel.border = Margin(2,2,2,2);
    panel.colorBorder = Gradient(getColorSelected(), getColorSelected(), getBounds().topLeft(), getBounds().bottomLeft());
    pictorialLabel.color = getColorSelected();

	update();
}

void PictorialItem::select()
{
    panel.border = Margin(4, 4, 4, 4);
    panel.colorBorder = Gradient(getColorSelectedGradientFrom(), getColorSelectedGradientTo(), panel.getBounds().topLeft(), panel.getBounds().bottomLeft());
	pictorialLabel.color = getColorSelected();
	selected = true;
	
	update();
}

void PictorialItem::deselect()
{
    panel.border = Margin(2, 2, 2, 2);
    panel.colorBorder = getColorNotSelected();
	pictorialLabel.color = getColorNotSelected();
	selected = false;

	update();
}

void PictorialItem::lock()
{
	locked = true;
	update();
}

void PictorialItem::unlock()
{
	locked = false;
	update();
}

void PictorialItem::update()
{
	lockIcon.visible = selected && locked;
}


}}