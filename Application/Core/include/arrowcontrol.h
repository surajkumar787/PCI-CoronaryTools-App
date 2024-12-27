// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense.h>
#include <Sense/Common/Property.h>
#include <ExperienceIdentity.h>

namespace Pci { namespace Core
{


class ArrowControl : public Sense::Control
{

public:
    explicit ArrowControl(Sense::Control &parent);
    virtual ~ArrowControl(void);

    virtual void render(Sense::IRenderer &renderer) const override;

private:
    void    draw(Sense::Space& space, Sense::Drawing &drawing) const;
    void    drawArrowhead(Sense::Rect &bounds, Sense::Drawing & drawing) const;
    void    drawArrowLine(Sense::Rect &bounds, Sense::Drawing & drawing) const;
    double  verticleCenter(Sense::Rect &bounds) const;
    void    drawPolyLine(std::vector<Sense::Point> &points, Sense::Drawing & drawing) const;

    Sense::Color	        m_color;
    bool                    m_showArrowHead;
    double                  m_arrowHeadWidth;
    double                  m_lineWidth;
    bool                    m_show;
    mutable Sense::Drawing  m_drawing;
};

}}