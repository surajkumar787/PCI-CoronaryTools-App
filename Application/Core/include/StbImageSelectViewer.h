// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>

#include "StbPictorialListControl.h"
#include "StbMarkersViewer.h"
#include "StbImageSelectViewer.ui.h"

#include "StbImage.h"
#include "StbMask.h"

namespace Pci { namespace Core {

class StbImageSelectViewer : public UI::StbImageSelectViewer
{
public:
    StbImageSelectViewer(Control& parent, ViewType type, bool showRange);
	~StbImageSelectViewer();
	
	void setSelection(int index);
	void setFrames(std::vector<std::shared_ptr<const StbImage>>& images);
private:
	std::vector<std::shared_ptr<const StbImage>> contrastImages;
	StbMask mask;
};

}}