// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Line.h"
#include "Sense/Common/Point.h"
#include "Sense.h"
#include "Sense/Common/Color.h"

#include <algorithm>

namespace Pci { namespace Core{
using namespace Sense;

Line::Line(double lineWidth) :
	m_points(),
	m_vertices(),
	isValid(false),
	m_pixelSize(0.0),
	m_pos(0),
	m_length(0),
	m_lineWidth(lineWidth),
	m_type(LineType::Solid),
	m_color(Sense::Color(1,1,1))
{
}

void Line::addPoint(const Sense::Point &point)
{
	if(point.x >= 0)
	{
		if (static_cast<int>(m_points.size())<= m_pos)
		{
			m_points.push_back(point);
		}
		else
		{
			m_points[m_pos] = point;
		}

		m_pos++;

		if (m_length < m_pos)
		{
			m_length = m_pos;
		}

		isValid = false;
	}
}

int Line::getNextPosition() const
{
	int next = m_pos;
	while((next > 0) && ((m_points[next-1].x - m_points[m_pos-1].x) < 1) && (next < m_length))
	{
		next++;
	}

	return next;
}

Sense::Point Line::getLastPoint() const
{
	return m_points[m_pos-1];
}

int Line::getPosition() const
{
	return m_pos;
}

int Line::getLength() const
{
	return m_length;
}

void Line::wrap()
{
	m_length = m_pos;
	m_pos = 0;
}

void Line::reset()
{
	m_pos    = 0;
	m_length = 0;
}

void Line::setLineType(LineType lineType)
{
	m_type = lineType;
	isValid = false;
}

void Line::clear()
{
	reset();
	m_points.clear();
	isValid = false;
}

void Line::setColor(const Sense::Color &newColor)
{
	m_color = newColor;
	isValid = false;
}



void Line::drawPolyLine(Sense::Drawing &drawing) const
{
	drawSolidLine(drawing);
}

static const int OneStep3Lines = 12;
static const int OneStepOneLines = 4;
static const int LineOne = 0;
static const int LineTwo = 1;
static const int LineThree = 2;

void Line::drawLine(Sense::IRenderer &renderer, Sense::Space &space, const Sense::TextureInput& dotTexture) const
{
	if (m_points.empty()) return;

	double ps = space.getScale();
	if (ps > 0.0) ps = 1.0 / ps;

	//if the pixel size has changed, we need to update the anti-aliasing, so re-calculate the line...
	if (std::abs( ps - m_pixelSize)< FLT_EPSILON)
	{
		m_pixelSize = ps;
		isValid = false;
	}

	if (!isValid)
	{
		isValid = true;

		//calculate the total number of vertices we're gonna need
		m_vertices.resize(m_points.size() * static_cast<size_t>(OneStep3Lines));

		for (int i = 0; i < ( static_cast<int>(m_points.size()) - 1); i++)
		{
			//we actually create three parallel lines; the main line, plus a line on each side
			//of the main line that fade to transparency. This creates nice-looking anti-aliased lines
			Vertex *v1 = m_vertices.data() + (i * OneStep3Lines) + (OneStepOneLines * LineOne);
			Vertex *v2 = m_vertices.data() + (i * OneStep3Lines) + (OneStepOneLines * LineTwo);
			Vertex *v3 = m_vertices.data() + (i * OneStep3Lines) + (OneStepOneLines * LineThree);

			double dist = 0.0;

			//calculate the width of the line, plus the number of dots to show
			double w = std::max(0.0, (m_lineWidth / 2.0) - (m_pixelSize / 2.0));
			double d = m_type == Line::LineType::Solid ? 0.0 : 0.5;

			Point a = m_points[i];
			Point b = m_points[i+1];

			//calculate the direction and perpendicular vector
			Point dir = (b - a).normalize();
			Point ortho = Point(dir.y, -dir.x);

			//if we don't want dots, make sure we're sampling in the middle of the solid pixel in the texture
			if (std::abs(d) <= FLT_EPSILON)
			{
				dist = (floor(dist / 2.0) * 2.0) + 0.5;
			}

			//create the vertices for the beginning of the segment
			*v1 = Vertex(a + ortho * -(w + m_pixelSize), Color(), Point(dist, 0.5));	 v1++;
			*v1 = Vertex(a + ortho * -w, m_color, Point(dist, 0.5));					 v1++;
			*v2 = Vertex(a + ortho * -w, m_color, Point(dist, 0.5));					 v2++;
			*v2 = Vertex(a + ortho *  w, m_color, Point(dist, 0.5));					 v2++;
			*v3 = Vertex(a + ortho *  w, m_color, Point(dist, 0.5));					 v3++;
			*v3 = Vertex(a + ortho *  (w + m_pixelSize), Color(), Point(dist, 0.5));	 v3++;

			//if we want dots, calculate the end position. Because the texture is rendered in repeat-mode, this will give a continuous dotted line effect
			if (std::abs(d)  > FLT_EPSILON)
			{
				dist += (b - a).length() * d;
			}

			//create the vertices for the end of the segment
			*v1 = Vertex(b + ortho * -(w + m_pixelSize), Color(), Point(dist, 0.5));	 v1++;
			*v1 = Vertex(b + ortho * -w, m_color, Point(dist, 0.5));					 v1++;
			*v2 = Vertex(b + ortho * -w, m_color, Point(dist, 0.5));					 v2++;
			*v2 = Vertex(b + ortho *  w, m_color, Point(dist, 0.5));					 v2++;
			*v3 = Vertex(b + ortho *  w, m_color, Point(dist, 0.5));					 v3++;
			*v3 = Vertex(b + ortho *  (w + m_pixelSize), Color(), Point(dist, 0.5));	 v3++;
		}
	}

	//finally, render the vertices to the screen
	for (int i = 0; i < (static_cast<int>(m_points.size()) - 1); i++)
	{
		renderer.render(space, m_vertices.data() + (i * OneStep3Lines) + (OneStepOneLines * LineOne), OneStepOneLines, dotTexture, IRenderer::TextureFlags::Repeat);
		renderer.render(space, m_vertices.data() + (i * OneStep3Lines) + (OneStepOneLines * LineTwo), OneStepOneLines, dotTexture, IRenderer::TextureFlags::Repeat);
		renderer.render(space, m_vertices.data() + (i * OneStep3Lines) + (OneStepOneLines * LineThree), OneStepOneLines, dotTexture, IRenderer::TextureFlags::Repeat);
	}
}

static  const double LineWidth = 0.75;
void Line::drawSolidLine(Drawing &drawing) const
{
	if(getLength() > 0)
	{
		Polyline::draw(drawing, m_points.data(), getPosition(), LineWidth, m_color, Polyline::Type::Segmented);

		int next = getNextPosition();
		if (next < getLength())
		{
			Polyline::draw(drawing, &m_points[next], getLength() - next, LineWidth, m_color, Polyline::Type::Segmented);
		}
	}
}
}}