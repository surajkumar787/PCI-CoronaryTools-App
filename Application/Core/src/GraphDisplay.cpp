// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "GraphDisplay.h"
#include "Sense/Shapes/ImageText.h"
#include "ExperienceIdentity/Resource/Palette.h"

using namespace Sense;
using namespace Sense::ExperienceIdentity;

namespace Pci { namespace Core {

static const int StrBufferSize = 512;	
static const int VerticalAxesOffset = 30;
static const int HorizontalAxesOffset = 30;
static const int NormalFontSize = 15;
static const int smallFontSize = 12;
static const int LargeCrossSize = 22;
static const int normalCrossSize =18;
static const double DistanceOfCrossAboveLine = 20.0;
static const double SmallGraphSize = 200.0;
static const int NrOfLabelSmallGraph = 5;
static const int NrOfLabelsLargeGraph = 7;

GraphDisplay::GraphDisplay(Control &parent, double maxYValue, double minYValue, double scaleYValue, double maxXValue, double minXValue, double scaleXValue, _locale_t &locale,  bool ShowHorizontalScale) :
	 Control(parent),
	 startOffset(0.0),
	 maxY(maxYValue),
	 minY(minYValue),
	 scaleY(scaleYValue),
     maxX(maxXValue),
     minX(minXValue),
	 scaleX(scaleXValue),
	 drawing(gpu),
	 bounds(0.0),
	 horizontalSampleSize(0.0),
	 verticalSampleSize(0.0),
	 cursorPosition(0.0),
	 userLocale(locale),
	 showHorizontalScale(ShowHorizontalScale)
{
	cursor.setColor(Color(1.0,1.0,1.0));
}


GraphDisplay::~GraphDisplay()
{
}

void GraphDisplay::render (Sense::IRenderer &renderer) const
{
    Space space = getSpace();

   generateGrid(renderer);

	if(isInvalidated())
    {
        drawing.open(space /*,L"grid"*/);

        for(const Line &segment : grid)
        {
			segment.drawPolyLine(drawing);
        }

        drawing.close();
    }

   drawing.render(space, renderer);
}


void GraphDisplay::generateGrid(Sense::IRenderer &renderer) const
{
    grid.clear();

	Space space = getSpace();

    double hSampleSize =  bounds.width()  / ((maxX - minX) / scaleX);
    double vSampleSize   =  bounds.height() / ((maxY - minY) / scaleY);

	Line horizontal;
	horizontal.setColor(Palette::Gray090);
	horizontal.addPoint(Point(bounds.left, bounds.bottom));
	horizontal.addPoint(Point(bounds.right, bounds.bottom));
    grid.push_back(horizontal);

	Line vertical;
	vertical.setColor(Palette::Gray090);
	vertical.addPoint(Point(bounds.left, bounds.top));
	vertical.addPoint(Point(bounds.left, bounds.bottom));
    grid.push_back(vertical);

	ImageText text(gpu);

	if(showHorizontalScale)
	{
		double label = 0.0;
		for(double i = bounds.left; i <= bounds.right; i += hSampleSize)
		{
			text.text = roundedDoubleToString(label);
			text.font = Font(L"Philips Healthcare Font", NormalFontSize, false, false, false, false, Font::Antialias::High);

			Size textSize			= text.calcTextSize(space, HorizontalAxesOffset - 2, 0);
			text.bounds             = Rect(i - (textSize.width/2), bounds.bottom, i + (textSize.width/2), bounds.bottom  + textSize.height);
			text.color              = Palette::White;
			text.horizontalAlign    = HAlign::Center;

			text.render(space, renderer);

			label += scaleX;
		}
	}

    double verticalLabel = maxY;

	double i = bounds.top;

	double multiplier = CalculateMultiplicationFactor();

    while(verticalLabel >= minY)
    {

        text.text = doubleToString(verticalLabel);
        text.font = Font(L"Philips Healthcare Font", NormalFontSize, false, false, false, false, Font::Antialias::High);

        Size textSize = text.calcTextSize(space, VerticalAxesOffset - 2, HorizontalAxesOffset - 2);
		textSize.width = static_cast<double>( VerticalAxesOffset -2) ;

        text.bounds             = Rect(0.0, i - (textSize.height/2), textSize.width, i + (textSize.height/2));
        text.color              = Palette::White;
		text.horizontalAlign    = HAlign::Right;
        text.verticalAlign      = VAlign::Middle;

        text.render(space, renderer);

		verticalLabel -= scaleY * multiplier;
		i += vSampleSize * multiplier;
    }

}

void GraphDisplay::transform(Sense::Point &point) const
{
	if(horizontalSampleSize == 0.0)
	{
		horizontalSampleSize = (bounds.width()-1) / (maxX - minX);
	}

	double newX = (point.x - startOffset) * horizontalSampleSize;
    double range = maxY - minY;

	if(verticalSampleSize == 0.0)
	{
		verticalSampleSize   =  bounds.height() / range;
	}

    double inverted = maxY - point.y;
    double newY = inverted * verticalSampleSize;

    point.x = newX;
    point.y = newY;

	point += Point(VerticalAxesOffset, 10); //move points to the inside of the grid
}

Sense::Point GraphDisplay::transformDoubleToPoint(double timestamp, double value) const
{
	Point p(timestamp, value);
	transform(p);

	return p;
}

std::wstring GraphDisplay::doubleToString(double value) const
{
	wchar_t str[StrBufferSize];
	if(value < 10.0 && value > 0.0)
	{
		_swprintf_s_l(str, StrBufferSize, L"%.2f", userLocale, value);
	}
	else
	{
		return roundedDoubleToString(value);
	}

    return str;
}

std::wstring Pci::Core::GraphDisplay::roundedDoubleToString(double value) const
{
	wchar_t str[StrBufferSize];
	_swprintf_s_l(str, StrBufferSize, L"%.0f", userLocale, value);

	return str;
}

void GraphDisplay::setCursor(double timestamp)
{
	cursorPosition = timestamp;

	cursor.clear();

	Point pa(timestamp, maxY);
    transform(pa);

	Point pb(timestamp, minY);
    transform(pb);

	cursor.addPoint(pa);
	cursor.addPoint(pb);

	invalidate();
}

void GraphDisplay::onResize() 
{
	verticalSampleSize   = 0.0;
	horizontalSampleSize = 0.0;
	bounds = getSpace().bounds - Margin(VerticalAxesOffset,10,10,HorizontalAxesOffset);
}

double GraphDisplay::getMaxX() const
{
	return maxX;
}

void Pci::Core::GraphDisplay::drawInvalidCycleCrosses(Sense::IRenderer &renderer, std::vector<Point> &positions, bool elevate) const
{
	Space space = getSpace();

	for(const Point &point : positions)
	{
		ImageText text(gpu);

		double ypos = elevate ? point.y-DistanceOfCrossAboveLine : point.y;
		int    crossSize = elevate ? LargeCrossSize : normalCrossSize;

		text.text				= L"X";
		text.font				= Font(L"Philips Healthcare Font", smallFontSize, true, false, false, false, Font::Antialias::High);
		Size textSize			= text.calcTextSize(space, crossSize, 0); //temp fix niladri
		text.bounds             = Rect(point.x - (textSize.width/2), ypos - (textSize.height/2), point.x + (textSize.width/2), ypos + (textSize.height/2));
		text.color              = Color(1.0,0.5,0.0);
		text.horizontalAlign    = HAlign::Center;
		text.verticalAlign      = VAlign::Middle;
		if (elevate) text.outline = Color(0.0,0.0,0.0);

		text.render(space, renderer);
	}
}

double GraphDisplay::getCursorPosition() const
{
	return cursorPosition;
}

double GraphDisplay::CalculateMultiplicationFactor() const
{
	int numberOfValues = bounds.height() < SmallGraphSize ? NrOfLabelSmallGraph : NrOfLabelsLargeGraph;

	double multiplier = (maxY - minY)/( numberOfValues * scaleY);
	multiplier = multiplier > 1.0 ? std::floor(multiplier): multiplier > 0.75 ? 1.0 :  0.5;
	double expectedNrLabels	 = std::floor( (maxY - minY)/( multiplier * scaleY));
	if( expectedNrLabels > numberOfValues)
	{
		multiplier*=2.0;
	}
	return multiplier;
}

}}