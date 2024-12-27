// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once
#include "Sense/Controls/Control.h" 
#include "Sense/Shapes/Drawing.h"
#include "Sense/Common/Types.h"
#include "Line.h"

#include <locale.h>

namespace Pci { namespace Core {

class GraphDisplay : public Sense::Control
{
public:
	GraphDisplay(Sense::Control &parent, double maxY, double minY, double scaleY, double maxX, double minX, double scaleX, _locale_t &locale, bool showHorizontalScale = false);
	virtual ~GraphDisplay();

	void   setCursor(double timestamp);
	double getCursorPosition() const;

	double startOffset;

	double getMaxX() const;


protected:
	mutable double maxY;
	mutable double minY;
	mutable double scaleY;

	mutable double maxX;
	mutable double minX;
	mutable double scaleX;

	mutable double horizontalSampleSize;
	mutable double verticalSampleSize;


	virtual void render (Sense::IRenderer &renderer) const override;
	virtual void onResize() override;
	void		 drawInvalidCycleCrosses(Sense::IRenderer &renderer, std::vector<Sense::Point> &positions, bool elevate) const;
	void         transform(Sense::Point &point) const;
	Sense::Point transformDoubleToPoint(double timestamp, double value) const;

	mutable Sense::Rect bounds;

	double cursorPosition;
	mutable Line cursor;

protected:

	_locale_t userLocale;

private:
	mutable std::vector<Line> grid;
    mutable Sense::Drawing drawing;

	mutable bool showHorizontalScale;

	void            generateGrid(Sense::IRenderer &renderer) const;

	double			CalculateMultiplicationFactor() const;
	std::wstring    doubleToString(double value) const;
	std::wstring    roundedDoubleToString(double value) const;

};
}}

