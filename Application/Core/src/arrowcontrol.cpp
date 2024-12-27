// Copyright 2018 Koninklijke Philips N.V.
#include "arrowcontrol.h"

namespace Pci { namespace Core
{

using namespace Sense;
using namespace Sense::ExperienceIdentity;    

const static double ArrowHeadWidthInactive = 10.0;
const static double ArrowHeadWidthActive   = 20.0;
const static double ArrowLineWidthInactive = 1.0;
const static double ArrowLineWidthActive   = 2.0;
const static Sense::Color& InProgressTextColor = Sense::ExperienceIdentity::Palette::Gray000;
const static Sense::Color& CompleteTextColor   = Sense::ExperienceIdentity::Palette::Gray160;
const static Sense::Color& NotStartedTextColor = Sense::ExperienceIdentity::Palette::Gray100;



ArrowControl::ArrowControl(Control &parent):
    Control(parent),
    m_color(Palette::Gray040),
    m_showArrowHead(true),
    m_arrowHeadWidth(ArrowHeadWidthActive),
    m_lineWidth(ArrowLineWidthActive),
    m_show(true),
    m_drawing(gpu)
{     
}

ArrowControl::~ArrowControl(void)
{
}

void ArrowControl::render(IRenderer &renderer) const
{
    Space space = getSpace();
    Sense::Drawing  drawing(gpu);
    draw(space, drawing);
    Control::render(renderer);
    drawing.render(space, renderer);    
}

void ArrowControl::draw(Sense::Space& space, Sense::Drawing &drawing) const
{
    if (isInvalidated() || (!drawing.isValid(space)))
    {
        if (m_show)
        {
            drawing.open(space);
            auto bounds = getClientSpace().bounds;
            drawArrowLine(bounds, drawing);
            if (m_showArrowHead)
            {
                drawArrowhead(bounds, drawing);
            }
            drawing.close();
        }
    }
}

void ArrowControl::drawArrowLine(Sense::Rect &bounds, Sense::Drawing & drawing) const
{
    double center = verticleCenter(bounds);
    double right  = bounds.right - 2 * m_lineWidth; // prevents arrowline going beyond arrowhead
    double left   = bounds.left;

    std::vector<Point> points;
    points.emplace_back(left,  center);
    points.emplace_back(right, center);
    drawPolyLine(points, drawing);
}

void ArrowControl::drawArrowhead(Sense::Rect &bounds, Sense::Drawing & drawing) const
{
    double center = verticleCenter(bounds);
    double top    = center + m_arrowHeadWidth;
    double bottom = center - m_arrowHeadWidth;
    double right  = bounds.right - m_lineWidth; // prevents tip of arrowhead going beyond bounds
    double left   = right - m_arrowHeadWidth;

    std::vector<Point> points;
    points.emplace_back(left,  top);
    points.emplace_back(right, center);
    points.emplace_back(left,  bottom);
    drawPolyLine(points, drawing);
}

double ArrowControl::verticleCenter(Sense::Rect &bounds) const
{
    return (bounds.top + bounds.bottom) / 2.0;
}

void ArrowControl::drawPolyLine(std::vector<Point> &points, Sense::Drawing & drawing) const
{
    int pointsCount = static_cast<int>(points.size());

    std::vector<Polyline::Band> lineBands;
    lineBands.emplace_back(m_lineWidth, Gradient(m_color));
    int bandsCount =  static_cast<int>(lineBands.size());

    Polyline::draw(drawing, points.data(), pointsCount, lineBands.data(), bandsCount, Polyline::Type::Sharp);
}
}}