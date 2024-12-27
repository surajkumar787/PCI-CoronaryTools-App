// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <vector>
#include <memory>

#include <Sense.h>
#include <ExperienceIdentity.h>

#include "ViewType.h"

namespace Pci { namespace Core {

class PictorialItem;

class PictorialListControl : public Sense::Control
{
public: 
	PictorialListControl(Sense::Control& parent, ViewType viewType);
	~PictorialListControl();
	
	void selectItem(int index);
	PictorialItem& createItem();

    void showRangeSelection();
    void hideRangeSelection();

protected:
	virtual void updateView() {};

	void updateScrollArea();

	int selectedIndex;
	ViewType viewType;

	std::vector<std::shared_ptr<PictorialItem>> pictorialItems;
	Sense::ExperienceIdentity::ScrollArea scrollArea;

private:
    bool m_showRange;

};

}}