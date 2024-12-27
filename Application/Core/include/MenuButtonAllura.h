// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "MenuButton.h"

namespace Pci { namespace Core
{

class MenuButtonAllura : public MenuButton
{
public:
	MenuButtonAllura(Sense::Control &parent);
	virtual ~MenuButtonAllura(void);
};

}}
