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

namespace Pci { namespace Core {

struct StbImage;
class StbPictorialItem;

class StbPictorialListControl : public PictorialListControl
{
public: 
    StbPictorialListControl(Sense::Control& parent, ViewType viewType);
	~StbPictorialListControl();
	
	void createObjects(std::vector<std::shared_ptr<const StbImage>>& images);

private:
	std::shared_ptr<StbPictorialItem> createObject(std::shared_ptr<const StbImage> image, PictorialItem& item);

	std::vector<std::shared_ptr<StbPictorialItem>> contrastItems;
};

}}