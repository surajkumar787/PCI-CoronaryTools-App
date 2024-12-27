// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <algorithm>

#include <Sense/Common/Rect.h>

#include "CrmPictorialListControl.h"
#include "CrmPictorialItem.h"
#include "PictorialItem.h"
#include "CrmRoadmap.h"


namespace Pci { namespace Core {

CrmPictorialListControl::CrmPictorialListControl(Sense::Control& parent, ViewType viewType) :
	PictorialListControl(parent, viewType)
{
}

CrmPictorialListControl::~CrmPictorialListControl()
{
}

void CrmPictorialListControl::createObjects(std::vector<std::shared_ptr<const CrmRoadmap>>& roadmaps)
{
	roadmapItems.clear();
	pictorialItems.clear();

	for (int i = 0; i < roadmaps.size(); i++)
	{
		auto& item = createItem();
		auto contrastItem = createObject(roadmaps[i], item);
		roadmapItems.push_back(contrastItem);
	}

	updateScrollArea();
}

void CrmPictorialListControl::updateGeo(const CommonPlatform::Xray::XrayGeometry& geo)
{
	for (auto& roadmapItem : roadmapItems)
	{
		roadmapItem->setGeometry(geo);
	}
}

void CrmPictorialListControl::updateView()
{
	for (auto& roadmapItem : roadmapItems)
	{
		roadmapItem->update();
	}
}

std::shared_ptr<CrmPictorialItem> CrmPictorialListControl::createObject(std::shared_ptr<const CrmRoadmap> roadmap, PictorialItem& item)
{
	auto seriesNumber = std::to_wstring(roadmap->seriesNumber);

	std::shared_ptr<CrmPictorialItem> roadmapItem;

	roadmapItem = std::make_shared<CrmPictorialItem>(item, seriesNumber);
	roadmapItem->setRoadmap(roadmap);
	
	return roadmapItem;
}

}}