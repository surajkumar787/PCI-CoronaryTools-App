// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>

#include "CrmPictorialListControl.h"
#include "CrmThumbnail.h"
#include "CrmRoadmapSelectViewer.ui.h"

namespace Pci { namespace Core {

class CrmRoadmapSelectViewer : public UI::CrmRoadmapSelectViewer
{
public:
	CrmRoadmapSelectViewer(Control& parent, ViewType type);
	~CrmRoadmapSelectViewer();
	
	void setSelection(int index);
	void setRoadmaps(std::vector<std::shared_ptr<const CrmRoadmap>>& roadmap);

	void setGeometry(const CommonPlatform::Xray::XrayGeometry& geometry);
	void update();

private:
	std::vector<std::shared_ptr<const CrmRoadmap>> roadmaps;
};

}}