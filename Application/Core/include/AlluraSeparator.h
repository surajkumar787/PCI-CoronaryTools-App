// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense.h>

namespace Pci { namespace Core {

    class AlluraSeparator : public Sense::Control
    {
    public:
		enum class SeparatorStyle
		{
			Full,
			Top,
			Bottom
		};

        AlluraSeparator(Sense::Control &parent, SeparatorStyle separatorStyle);
        virtual ~AlluraSeparator();

    private:
		SeparatorStyle style;

        virtual void render(Sense::IRenderer &renderer) const override;
    };

}}