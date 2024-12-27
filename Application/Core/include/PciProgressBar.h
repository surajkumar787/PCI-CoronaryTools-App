// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Sense/Controls/Control.h"
#include "Sense/Common/Property.h"
#include "Sense/Shapes/Drawing.h"

namespace Pci { namespace Core {

class PciProgressBar : public Sense::Control
{
	public:
		explicit PciProgressBar(Sense::Control &parent);

		Sense::Property<double> value;
		Sense::Property<double> min;
		Sense::Property<double> max;
		Sense::Property<Sense::Gradient> color;

	protected:
		virtual void render (Sense::IRenderer &renderer) const override;
		virtual void onResize() override;

	private:
		mutable Sense::Drawing drawing;
	};
}}

