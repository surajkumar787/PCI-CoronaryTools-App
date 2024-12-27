// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "Sense/Common/Color.h"
#include "Sense/Graphics/TextureInput.h"
#include "Sense/Common/Vertex.h"
#include <vector>

namespace Sense{
	struct Point;
	class IRenderer;
	class Drawing;
	class Space;
	class TextureInput;
}

namespace Pci { namespace Core {

class Line
{
public:
	enum class LineType
	{
		Solid,
		Dashed
	};

	explicit Line(double lineWidth = 1.0);

	
	void drawLine(Sense::IRenderer &renderer, Sense::Space &space, const Sense::TextureInput& dotTexture) const;
	void drawPolyLine(Sense::Drawing &drawing) const;

	void addPoint(const Sense::Point &point);

	void setLineType(LineType type);
	void setColor(const Sense::Color &color);

	int getLength() const;
	int getPosition() const;
	double getLineWidth()const {return m_lineWidth;};
	Sense::Point getLastPoint() const;

	void wrap();
	void reset();
	void clear();

private:
	std::vector<Sense::Point> m_points;
	mutable std::vector<Sense::Vertex> m_vertices;
	mutable bool isValid;
	mutable double m_pixelSize;

	int m_pos;
	int m_length;
	double m_lineWidth;

	LineType m_type;
	Sense::Color m_color;

	int getNextPosition() const;

	void drawSolidLine(Sense::Drawing &drawing) const;
};

}}
