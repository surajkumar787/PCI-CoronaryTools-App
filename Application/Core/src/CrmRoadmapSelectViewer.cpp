// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Translation.h"
#include "CrmRoadmapSelectViewer.h"
#include "StbPictorialItem.h"

#include "AlluraColors.h"

namespace Pci { namespace Core
{

CrmRoadmapSelectViewer::CrmRoadmapSelectViewer(Control& parent, ViewType type) : UI::CrmRoadmapSelectViewer(parent, type)
{
	if (type == ViewType::Allura)
	{
		guidanceTextLabel.color = UI::ColorAlluraLightGreyFont;
		guidanceTextLabel.font = Sense::Font(L"Arial", 34, true, false, false, false, Font::Antialias::High);

		selectedRoadmapLabel.color = UI::ColorAlluraLightGreyFont;
		guidanceTextLabel.font = Sense::Font(L"Arial", 34, true, false, false, false, Font::Antialias::High);
	}

	crmThumbnail.roi = Rect(0, 0, crmThumbnail.size->width, crmThumbnail.size->height);
}

CrmRoadmapSelectViewer::~CrmRoadmapSelectViewer()
{
}

void CrmRoadmapSelectViewer::setSelection(int index)
{
	pictorialListControl.selectItem(index);
	crmThumbnail.setRoadmap(roadmaps[index]);
	crmThumbnail.active = true;
	selectedRoadmapLabel.text = L"Roadmap " + std::to_wstring(roadmaps[index]->seriesNumber);

	update();
}

void CrmRoadmapSelectViewer::setRoadmaps(std::vector<std::shared_ptr<const CrmRoadmap>>& _roadmaps)
{
	roadmaps.clear();
	for (const auto& roadmap : _roadmaps)
	{
		roadmaps.push_back(roadmap);
	}

	pictorialListControl.createObjects(roadmaps);
}

void CrmRoadmapSelectViewer::setGeometry(const CommonPlatform::Xray::XrayGeometry& geometry)
{
	crmThumbnail.setGeometry(geometry);
	pictorialListControl.updateGeo(geometry);
}

void CrmRoadmapSelectViewer::update()
{
	pictorialListControl.updateView();
}


}}