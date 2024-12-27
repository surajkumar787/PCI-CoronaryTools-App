// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "PictorialItem.ui.h"

namespace Pci { namespace Core {

class PictorialItem : public UI::PictorialItem
{
public:
	PictorialItem(Sense::Control& parent);
	virtual ~PictorialItem();

	Sense::Control& getContentControl();
	void setLabel(const std::wstring& label);
    
    void used();
	void select();
	void deselect();
	void lock();
	void unlock();

private:
	void update();

	virtual const Sense::Color getColorSelectedGradientFrom() const = 0;
	virtual const Sense::Color getColorSelectedGradientTo() const = 0;
	virtual const Sense::Color getColorSelected() const = 0;
	virtual const Sense::Color getColorNotSelected() const = 0;

	bool locked;
	bool selected;
};

}}