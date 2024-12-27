// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "PictorialItem.h"

namespace Pci { namespace Core {

class PictorialItemAzurion : public PictorialItem
{
public:
	PictorialItemAzurion(Sense::Control& parent);
	~PictorialItemAzurion();

private:
	virtual const Sense::Color getColorSelectedGradientFrom() const override;
	virtual const Sense::Color getColorSelectedGradientTo() const override;
	virtual const Sense::Color getColorSelected() const override;
	virtual const Sense::Color getColorNotSelected() const override;
};

}}