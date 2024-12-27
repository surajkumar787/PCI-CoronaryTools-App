// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <memory>

#include "PictorialListControl.h"

#include "ViewType.h"

namespace CommonPlatform { namespace Xray {
	struct XrayGeometry;
}}

namespace Pci { namespace Core {

class CrmPictorialItem;
class CrmRoadmap;

class CrmPictorialListControl : public PictorialListControl
{
public: 
	CrmPictorialListControl(Sense::Control& parent, ViewType viewType);
	~CrmPictorialListControl();
	
	void createObjects(std::vector<std::shared_ptr<const CrmRoadmap>>& roadmaps);
	void updateGeo(const CommonPlatform::Xray::XrayGeometry& geo);

	void updateView() override;

private:
	std::shared_ptr<CrmPictorialItem> createObject(std::shared_ptr<const CrmRoadmap> roadmap, PictorialItem& item);

	std::vector<std::shared_ptr<CrmPictorialItem>> roadmapItems;
};

}}