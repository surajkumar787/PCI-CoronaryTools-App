// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>


namespace Pci { namespace Core
{
enum class ViewType;

class MenuButton : public Sense::ExperienceIdentity::ButtonFlat
{
public:
	MenuButton(Sense::Control &parent);
	virtual ~MenuButton(void);

	void clearMenuItems();
    void addMenuItem( const std::wstring& itemText, const std::function<void()> &eventClicked);
    void collapse();
    void enable();
    void disable();

private:
	
	Sense::ExperienceIdentity::ContextMenu menu;
	Sense::ExperienceIdentity::ContextMenu::Item root;
};

}}
