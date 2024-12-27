// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Sense.h"
#include "ExperienceIdentity.h"

namespace Pci { namespace Core
{
    class ButtonTransparent : public Sense::ExperienceIdentity::Button
    {
    public:
        explicit ButtonTransparent(Sense::Control &parent);
        virtual ~ButtonTransparent(void);

        void setFontColor(Sense::Color normalColor, Sense::Color hoverColor);
    };

}}