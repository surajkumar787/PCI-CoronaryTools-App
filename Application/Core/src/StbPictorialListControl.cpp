// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <algorithm>

#include <Sense/Common/Rect.h>

#include "StbPictorialListControl.h"
#include "StbPictorialItem.h"
#include "StbImage.h"

namespace Pci { namespace Core {

    StbPictorialListControl::StbPictorialListControl(Sense::Control& parent, ViewType viewType) :
	PictorialListControl(parent, viewType)
{
}

    StbPictorialListControl::~StbPictorialListControl()
{
}

void StbPictorialListControl::createObjects(std::vector<std::shared_ptr<const StbImage>>& images)
{
	contrastItems.clear();
	pictorialItems.clear();

	for (int i = 0; i < images.size(); i++)
	{
		auto& item = createItem();
		auto contrastItem = createObject(images[i], item);
		contrastItems.push_back(contrastItem);
	}

	updateScrollArea();
}

std::shared_ptr<StbPictorialItem> StbPictorialListControl::createObject(std::shared_ptr<const StbImage> image, PictorialItem& item)
{
	auto imageNumber = std::to_wstring(image->imageNumber);

	std::shared_ptr<StbPictorialItem> contrastItem;

	contrastItem = std::make_shared<StbPictorialItem>(item, imageNumber);
	contrastItem->setImage(image);
	
	return contrastItem;
}

}}