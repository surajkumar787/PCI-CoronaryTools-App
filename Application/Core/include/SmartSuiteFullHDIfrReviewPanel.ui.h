// ********************* WARNING *****************************
// * This file was automatically generated by Sense Composer *
// *      Modifying this file manually is frowned upon       *
// ***********************************************************
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>

namespace Pci { namespace Core
{

using namespace Sense;
using namespace Sense::ExperienceIdentity;

class SmartSuiteFullHDIfrReviewPanel : public Panel
{
public:

	SmartSuiteFullHDIfrReviewPanel(Control &parent)
	:
		Panel              (parent),
		distalFmValueLabel (*this),
		ifrTrendLabel      (*this),
		fmTrendValue       (*this),
		fmTypeLabel        (*this)
	{
		layoutUpdateBegin();

		name                             = L"fmReviewPanel";
		size                             = Size(386, 86);
		border                           = Margin(0, 0, 0, 0);
		edge                             = Margin(0, 0, 0, 0);
		colorEdge                        = Color(0, 0, 0);
		color                            = Color();

		distalFmValueLabel.name          = L"distalFmValueLabel";
		distalFmValueLabel.position      = Point(248, 0);
		distalFmValueLabel.size          = Size(136, 40);
		distalFmValueLabel.font          = Font(L"Philips Healthcare Font", 35, true, false, false, false, Font::Antialias::Low);
		distalFmValueLabel.text          = L"---";
		distalFmValueLabel.color         = Palette::Gray070;
		distalFmValueLabel.verticalAlign = VAlign::Middle;

		ifrTrendLabel.name               = L"ifrTrendLabel";
		ifrTrendLabel.position           = Point(0, 48);
		ifrTrendLabel.size               = Size(232, 40);
		ifrTrendLabel.font               = Font(L"Philips Healthcare Font", 28, false, false, false, false, Font::Antialias::High);
		ifrTrendLabel.text               = L"iFR";
		ifrTrendLabel.color              = Palette::Gray070;
		ifrTrendLabel.horizontalAlign    = HAlign::Right;

		fmTrendValue.name                = L"fmTrendValue";
		fmTrendValue.position            = Point(248, 48);
		fmTrendValue.size                = Size(136, 40);
		fmTrendValue.font                = Font(L"Philips Healthcare Font", 28, false, false, false, false, Font::Antialias::High);
		fmTrendValue.text                = L"---";
		fmTrendValue.color               = Palette::Gray070;

		fmTypeLabel.name                 = L"fmTypeLabel";
		fmTypeLabel.size                 = Size(232, 40);
		fmTypeLabel.font                 = Font(L"Philips Healthcare Font", 34, true, false, false, false, Font::Antialias::Low);
		fmTypeLabel.text                 = L"iFR Distal";
		fmTypeLabel.color                = Palette::Gray070;
		fmTypeLabel.horizontalAlign      = HAlign::Right;

		layoutUpdateEnd();
	}

public:

	Label     distalFmValueLabel;
	Label     ifrTrendLabel;
	Label     fmTrendValue;
	Label     fmTypeLabel;

private:

	SmartSuiteFullHDIfrReviewPanel(const SmartSuiteFullHDIfrReviewPanel&);
	SmartSuiteFullHDIfrReviewPanel& operator=(const SmartSuiteFullHDIfrReviewPanel&);
};

}}