// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <algorithm>
#include <Sense/Common/Rect.h>

#include "PictorialListControl.h"
#include "PictorialItemAllura.h"
#include "PictorialItemAzurion.h"

namespace Pci { namespace Core {

PictorialListControl::PictorialListControl(Sense::Control& parent, ViewType viewType) :
	Sense::Control(parent),
	viewType(viewType),
	scrollArea(*this),
    m_showRange(false)
{
	scrollArea.layout = Layout::Fixed2d;
	scrollArea.layoutOrientation = Orientation::Horizontal;
	scrollArea.showVerticalBar = ScrollArea::ShowState::Hide;
	scrollArea.showHorizontalBar = ScrollArea::ShowState::Auto;

	scrollArea.layoutSpacing = 16;
	layoutUseSpacing = false;
}

PictorialListControl::~PictorialListControl()
{
	pictorialItems.clear();
}

void PictorialListControl::showRangeSelection()
{
    m_showRange = true;
}

void PictorialListControl::hideRangeSelection()
{
    m_showRange = false;
}

void PictorialListControl::selectItem(int index)
{
	if (pictorialItems.empty())
	{
		return;
	}

	auto pictorialWidth = pictorialItems.back()->size->width;
	auto scrollWidth = ((index + 1) * (pictorialWidth + scrollArea.layoutSpacing)) - ((pictorialWidth + scrollArea.layoutSpacing) / 2.0) - (size->width / 2.0);
	scrollArea.setTranslation(Sense::Point(scrollWidth, 0.0), 0.15);

	updateView();

	for (int i = 0; i < pictorialItems.size(); i++)
	{
        if (i < index && m_showRange)
        {
            pictorialItems[i]->used();
        }
        else if (i == index)
        {
            pictorialItems[i]->select();
        }
        else
        {
            pictorialItems[i]->deselect();
        }
	}
}

PictorialItem& PictorialListControl::createItem()
{
	std::shared_ptr<PictorialItem> item;
	if (viewType == ViewType::Allura)
	{
		item = std::make_shared<PictorialItemAllura>(scrollArea);
	}
	else
	{
		item = std::make_shared<PictorialItemAzurion>(scrollArea);

	}
	pictorialItems.push_back(item);
	return *item;
}

void PictorialListControl::updateScrollArea()
{
	auto pictorialWidth = !pictorialItems.empty() ? pictorialItems.back()->size->width : 0;

	scrollArea.size = Sense::Size(std::min(size->width, (pictorialItems.size() * (pictorialWidth + scrollArea.layoutSpacing))), size->height);
	scrollArea.area = Sense::Size((pictorialItems.size() * (pictorialWidth + scrollArea.layoutSpacing)), size->height);
}

}}