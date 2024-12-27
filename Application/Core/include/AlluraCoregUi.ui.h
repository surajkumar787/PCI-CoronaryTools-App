// ********************* WARNING *****************************
// * This file was automatically generated by Sense Composer *
// *      Modifying this file manually is frowned upon       *
// ***********************************************************
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>
#include "AlluraSeparator.h"
#include "PciBaseViewer.h"
#include "AlluraIfrReviewPanel.ui.h"

namespace Pci { namespace Core
{

using namespace Sense;
using namespace Sense::ExperienceIdentity;

class AlluraCoregUi : public Control
{
public:

	AlluraCoregUi(Control &parent)
	:
		Control              (parent),
		panelPopup           (*this),
		popupPanelHeader     ((Control&)panelPopup),
		popupPanelHeaderText (popupPanelHeader),
		topLayOutPannel      (*this),
		ifrReviewPanel       (topLayOutPannel),
		panelValues          (topLayOutPannel),
		paneliFrFfrNames     (panelValues),
		labeliFRfFR          (paneliFrFfrNames),
		labeliFRfFRValue     (paneliFrFfrNames),
		progressPanel        ((Control&)paneliFrFfrNames),
		progress             (progressPanel),
		panelPaPdNames       ((Control&)panelValues),
		labelPdPa            (panelPaPdNames),
		labelPdPaValue       (panelPaPdNames),
		control3             ((Control&)panelValues),
		labelPa              (control3),
		labelPaValue         (control3),
		control4             ((Control&)panelValues),
		labelPd              (control4),
		labelPdValue         (control4),
		ifrSpotReviewPanel   ((Control&)topLayOutPannel),
		labeliFrSpot         (ifrSpotReviewPanel),
		labeliFrSpotValue    (ifrSpotReviewPanel),
		graphLayoutPanel     (*this),
		graphPanel           (graphLayoutPanel),
		bottomSeparator      (*this, AlluraSeparator::SeparatorStyle::Bottom),
		bottomLayoutSpace    (*this)
	{
		layoutUpdateBegin();

		size                               = Size(256, 1000);

		panelPopup.position                = Point(-10, -10);
		panelPopup.size                    = Size(1, 1);

		popupPanelHeader.size              = Size(1, 1);

		popupPanelHeaderText.size          = Size(128, 24);

		topLayOutPannel.name               = L"topLayOutPannel";
		topLayOutPannel.size               = Size(256, 226);
		topLayOutPannel.layout             = Layout::Fixed1d;
		topLayOutPannel.layoutOuterMargin  = Margin(2, 2, 2, 0);
		topLayOutPannel.layoutSpacing      = 0;
		topLayOutPannel.border             = Margin(0, 0, 0, 0);
		topLayOutPannel.edge               = Margin(0, 0, 0, 0);

		ifrReviewPanel.name                = L"ifrReviewPanel";
		ifrReviewPanel.position            = Point(2, 2);
		ifrReviewPanel.size                = Size(252, 74);
		ifrReviewPanel.layout              = Layout::Fill;
		ifrReviewPanel.layoutOuterMargin   = Margin(6, 6, 0, 6);
		ifrReviewPanel.layoutSpacing       = 0;

		panelValues.name                   = L"vakuePanel";
		panelValues.position               = Point(2, 76);
		panelValues.size                   = Size(252, 74);
		panelValues.layout                 = Layout::Fill;
		panelValues.layoutOuterMargin      = Margin(8, 20, 8, 10);

		paneliFrFfrNames.position          = Point(8, 20);
		paneliFrFfrNames.size              = Size(236, 5);
		paneliFrFfrNames.layout            = Layout::Fill;
		paneliFrFfrNames.layoutSpacing     = 0;
		paneliFrFfrNames.layoutOrientation = Orientation::Horizontal;
		paneliFrFfrNames.border            = Margin(0, 0, 0, 0);
		paneliFrFfrNames.edge              = Margin(0, 0, 0, 0);

		labeliFRfFR.name                   = L"fmTypeLabel";
		labeliFRfFR.size                   = Size(78.6667, 5);
		labeliFRfFR.font                   = Font(L"Philips Healthcare Font", 35, false, false, false, false, Font::Antialias::High);
		labeliFRfFR.layout                 = Layout::Fill;
		labeliFRfFR.text                   = L"iFR";
		labeliFRfFR.verticalAlign          = VAlign::Middle;

		labeliFRfFRValue.name              = L"fmValue";
		labeliFRfFRValue.position          = Point(78.6667, 0);
		labeliFRfFRValue.size              = Size(78.6667, 5);
		labeliFRfFRValue.font              = Font(L"Philips Healthcare Font", 40, false, false, false, false, Font::Antialias::High);
		labeliFRfFRValue.layout            = Layout::Fill;
		labeliFRfFRValue.text              = L"0.85";
		labeliFRfFRValue.horizontalAlign   = HAlign::Right;
		labeliFRfFRValue.verticalAlign     = VAlign::Middle;

		progressPanel.position             = Point(157.333, 0);
		progressPanel.size                 = Size(78.6667, 5);
		progressPanel.layout               = Layout::Fill;
		progressPanel.layoutSpacing        = 0;
		progressPanel.border               = Margin(4, 4, 4, 4);
		progressPanel.edge                 = Margin(0, 0, 0, 0);
		progressPanel.colorBorder          = Color(1, 1, 0);
		progressPanel.color                = Color(1, 1, 0);

		progress.size                      = Size(70.6667, -3);
		progress.layout                    = Layout::Fill;

		panelPaPdNames.position            = Point(8, 33);
		panelPaPdNames.size                = Size(236, 5);
		panelPaPdNames.layout              = Layout::Fill;
		panelPaPdNames.layoutSpacing       = 0;
		panelPaPdNames.layoutOrientation   = Orientation::Horizontal;
		panelPaPdNames.border              = Margin(0, 0, 0, 0);
		panelPaPdNames.edge                = Margin(0, 0, 0, 0);


		labelPdPa.name                     = L"pdPaLabel";
		labelPdPa.size                     = Size(118, 5);
		labelPdPa.font                     = Font(L"Philips Healthcare Font", 35, false, false, false, false, Font::Antialias::High);
		labelPdPa.layout                   = Layout::Fill;
		labelPdPa.text                     = L"Pd/Pa";
		labelPdPa.verticalAlign            = VAlign::Middle;

		labelPdPaValue.name                = L"labelPdPaValue";
		labelPdPaValue.position            = Point(118, 0);
		labelPdPaValue.size                = Size(118, 5);
		labelPdPaValue.font                = Font(L"Philips Healthcare Font", 40, false, false, false, false, Font::Antialias::High);
		labelPdPaValue.layout              = Layout::Fill;
		labelPdPaValue.text                = L"0.85";
		labelPdPaValue.horizontalAlign     = HAlign::Right;
		labelPdPaValue.verticalAlign       = VAlign::Middle;

		control3.position                  = Point(8, 46);
		control3.size                      = Size(236, 5);
		control3.layout                    = Layout::Fill;
		control3.layoutSpacing             = 0;
		control3.layoutOrientation         = Orientation::Horizontal;

		labelPa.name                       = L"paLabel";
		labelPa.size                       = Size(118, 5);
		labelPa.font                       = Font(L"Philips Healthcare Font", 35, false, false, false, false, Font::Antialias::High);
		labelPa.layout                     = Layout::Fill;
		labelPa.text                       = L"Pa";
		labelPa.verticalAlign              = VAlign::Middle;

		labelPaValue.name                  = L"labelPaValue";
		labelPaValue.position              = Point(118, 0);
		labelPaValue.size                  = Size(118, 5);
		labelPaValue.font                  = Font(L"Philips Healthcare Font", 40, false, false, false, false, Font::Antialias::High);
		labelPaValue.layout                = Layout::Fill;
		labelPaValue.text                  = L"74";
		labelPaValue.color                 = Palette::Gray040;
		labelPaValue.horizontalAlign       = HAlign::Right;
		labelPaValue.verticalAlign         = VAlign::Middle;

		control4.position                  = Point(8, 59);
		control4.size                      = Size(236, 5);
		control4.layout                    = Layout::Fill;
		control4.layoutSpacing             = 0;
		control4.layoutOrientation         = Orientation::Horizontal;

		labelPd.name                       = L"pdLabel";
		labelPd.size                       = Size(118, 5);
		labelPd.font                       = Font(L"Philips Healthcare Font", 35, false, false, false, false, Font::Antialias::High);
		labelPd.layout                     = Layout::Fill;
		labelPd.text                       = L"Pd";
		labelPd.verticalAlign              = VAlign::Middle;

		labelPdValue.name                  = L"labelPdValue";
		labelPdValue.position              = Point(118, 0);
		labelPdValue.size                  = Size(118, 5);
		labelPdValue.font                  = Font(L"Philips Healthcare Font", 40, false, false, false, false, Font::Antialias::High);
		labelPdValue.layout                = Layout::Fill;
		labelPdValue.text                  = L"69";
		labelPdValue.horizontalAlign       = HAlign::Right;
		labelPdValue.verticalAlign         = VAlign::Middle;

		ifrSpotReviewPanel.position        = Point(2, 150);
		ifrSpotReviewPanel.size            = Size(252, 74);
		ifrSpotReviewPanel.layout          = Layout::Fill;
		ifrSpotReviewPanel.border          = Margin(0, 0, 0, 0);
		ifrSpotReviewPanel.edge            = Margin(0, 0, 0, 0);
		ifrSpotReviewPanel.color           = Color(1, 1, 0);

		labeliFrSpot.size                  = Size(252, 33);
		labeliFrSpot.font                  = Font(L"Philips Healthcare Font", 50, false, false, false, false, Font::Antialias::High);
		labeliFrSpot.layout                = Layout::Fill;
		labeliFrSpot.text                  = L"iFR";
		labeliFrSpot.color                 = Palette::Black;
		labeliFrSpot.horizontalAlign       = HAlign::Center;
		labeliFrSpot.verticalAlign         = VAlign::Middle;

		labeliFrSpotValue.position         = Point(0, 41);
		labeliFrSpotValue.size             = Size(252, 33);
		labeliFrSpotValue.font             = Font(L"Philips Healthcare Font", 60, false, false, false, false, Font::Antialias::High);
		labeliFrSpotValue.layout           = Layout::Fill;
		labeliFrSpotValue.text             = L"---";
		labeliFrSpotValue.color            = Palette::Black;
		labeliFrSpotValue.horizontalAlign  = HAlign::Center;
		labeliFrSpotValue.verticalAlign    = VAlign::Middle;

		graphLayoutPanel.name              = L"graphLayoutPanel";
		graphLayoutPanel.position          = Point(0, 224);
		graphLayoutPanel.size              = Size(256, 440);
		graphLayoutPanel.layoutSpacing     = 0;

		graphPanel.name                    = L"graphPanel";
		graphPanel.size                    = Size(256, 440);
		graphPanel.layout                  = Layout::Fill;
		graphPanel.border                  = Margin(0, 0, 0, 0);
		graphPanel.edge                    = Margin(0, 0, 0, 0);
		graphPanel.color                   = Color(0, 0, 0);

		bottomSeparator.position           = Point(0, 660);
		bottomSeparator.size               = Size(256, 7);

		bottomLayoutSpace.size             = Size(256, 64);

		layoutUpdateEnd();
	}

public:

	Panel                panelPopup;
	Panel                popupPanelHeader;
	Label                popupPanelHeaderText;
	Panel				 topLayOutPannel;
	AlluraIfrReviewPanel ifrReviewPanel;
	Control              panelValues;
	Panel                paneliFrFfrNames;
	Label                labeliFRfFR;
	Label                labeliFRfFRValue;
	Panel                progressPanel;
	PciProgressBar       progress;
	Panel				 panelPaPdNames;
	Label                labelPdPa;
	Label                labelPdPaValue;
	Control              control3;
	Label                labelPa;
	Label                labelPaValue;
	Control              control4;
	Label                labelPd;
	Label                labelPdValue;
	Panel                ifrSpotReviewPanel;
	Label                labeliFrSpot;
	Label                labeliFrSpotValue;
	Spacer               graphLayoutPanel;
	Panel                graphPanel;
	AlluraSeparator      bottomSeparator;
	Spacer               bottomLayoutSpace;

private:

	AlluraCoregUi(const AlluraCoregUi&);
	AlluraCoregUi& operator=(const AlluraCoregUi&);
};

}}