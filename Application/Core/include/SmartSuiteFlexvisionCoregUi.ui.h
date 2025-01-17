/*----------------------------------------------------
 |   This file was generated by Sense Composer 2.0   |
 |  Do not modify the Sense Composer Generated Code  |
 ----------------------------------------------------*/
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>
#include "SmartSuiteFlexvisionIfrReviewPanel.ui.h"

namespace Pci { namespace Core
{
using namespace Sense;
using namespace Sense::ExperienceIdentity;

class SmartSuiteFlexvisionCoregUi : public Control
{
public:

	SmartSuiteFlexvisionCoregUi(Control &parent)
	:
		Control(parent)
		#pragma region Sense Composer (do not modify)
	,	panelPopup       (*this)
	,	popupPanelHeader ((Control&)panelPopup)
	,	popupPanelHeaderText(popupPanelHeader)
	,	topLayOutPannel  ((Control&)panelPopup)
	,	ifrReviewPanel   (topLayOutPannel)
	,	panelValues      (topLayOutPannel)
	,	paneliFrFfrNames (panelValues)
	,	labeliFRfFR      (paneliFrFfrNames)
	,	labeliFRfFRValue (paneliFrFfrNames)
	,	progressPanel    ((Control&)paneliFrFfrNames)
	,	progress         (progressPanel)
	,	panelPaPdNames   (panelValues)
	,	labelPdPa        (panelPaPdNames)
	,	labelPdPaValue   (panelPaPdNames)
	,	panelPaNames     ((Control&)panelValues)
	,	labelPa          (panelPaNames)
	,	labelPaValue     (panelPaNames)
	,	panelPdNames     ((Control&)panelValues)
	,	labelPd          (panelPdNames)
	,	labelPdValue     (panelPdNames)
	,	ifrSpotReviewPanel((Control&)topLayOutPannel)
	,	labeliFrSpot     (ifrSpotReviewPanel)
	,	labeliFrSpotValue(ifrSpotReviewPanel)
	,	graphLayoutPanel (panelPopup)
	,	graphPanel       (graphLayoutPanel)
	,	bottumLayoutSpace(*this)
		#pragma endregion
	{
		#pragma region Sense Composer (do not modify)
		layoutUpdateBegin();
		size                             = Size(296, 936);
		layoutSpacing                    = 0;
		panelPopup.name                  = L"panelPopup";
		panelPopup.size                  = Size(296, 670);
		panelPopup.layout                = Layout::Fill;
		panelPopup.layoutSpacing         = 0;
		panelPopup.radius                = Corners(5, 5, 5, 5);
		panelPopup.border                = Margin(0, 0, 0, 0);
		panelPopup.edge                  = Margin(1, 1, 1, 1);
		popupPanelHeader.name            = L"popupPanelHeader";
		popupPanelHeader.position        = Point(8, 2);
		popupPanelHeader.size            = Size(284, 24);
		popupPanelHeader.layoutUseSpacing = false;
		popupPanelHeader.layout          = Layout::Fixed1d;
		popupPanelHeader.layoutOuterMargin = Margin(8, 2, 2, 2);
		popupPanelHeader.layoutSpacing   = 0;
		popupPanelHeader.radius          = Corners(5, 5, 0, 0);
		popupPanelHeader.border          = Margin(0, 0, 0, 0);
		popupPanelHeader.edge            = Margin(0, 0, 0, 0);
		popupPanelHeaderText.name        = L"popupPanelHeaderText";
		popupPanelHeaderText.size        = Size(284, 24);
		popupPanelHeaderText.font        = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);
		popupPanelHeaderText.layout      = Layout::Fill;
		popupPanelHeaderText.text        = L"Header";
		popupPanelHeaderText.color       = Palette::Yellow050;
		popupPanelHeaderText.verticalAlign = VAlign::Middle;
		topLayOutPannel.name             = L"topLayOutPannel";
		topLayOutPannel.position         = Point(2, 30);
		topLayOutPannel.size             = Size(290, 206);
		topLayOutPannel.layout           = Layout::Fixed1d;
		topLayOutPannel.layoutOuterMargin = Margin(2, 2, 2, 8);
		topLayOutPannel.layoutSpacing    = 0;
		topLayOutPannel.border           = Margin(0, 0, 0, 0);
		topLayOutPannel.edge             = Margin(0, 0, 0, 0);
		//ifrReviewPanel.composerClass   = Control;
		ifrReviewPanel.name              = L"ifrReviewPanel";
		ifrReviewPanel.position          = Point(6, 6);
		ifrReviewPanel.size              = Size(284, 57);
		ifrReviewPanel.layout            = Layout::Fill;
		ifrReviewPanel.layoutOuterMargin = Margin(6, 6, 0, 6);
		ifrReviewPanel.layoutSpacing     = 0;
		panelValues.name                 = L"panelValues";
		panelValues.position             = Point(6, 75);
		panelValues.size                 = Size(278, 42);
		panelValues.layout               = Layout::Fill;
		panelValues.layoutOuterMargin    = Margin(6, 6, 6, 20);
		panelValues.layoutSpacing        = 0;
		paneliFrFfrNames.name            = L"paneliFrFfrNames";
		paneliFrFfrNames.size            = Size(278, 11);
		paneliFrFfrNames.layout          = Layout::Fill;
		paneliFrFfrNames.layoutSpacing   = 0;
		paneliFrFfrNames.layoutOrientation = Orientation::Horizontal;
		paneliFrFfrNames.border          = Margin(0, 0, 0, 0);
		paneliFrFfrNames.edge            = Margin(0, 0, 0, 0);
		labeliFRfFR.name                 = L"labeliFRfFR";
		labeliFRfFR.size                 = Size(93, 11);
		labeliFRfFR.font                 = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labeliFRfFR.layout               = Layout::Fill;
		labeliFRfFR.text                 = L"iFR";
		labeliFRfFR.color                = Palette::Gray070;
		labeliFRfFR.verticalAlign        = VAlign::Middle;
		labeliFRfFRValue.name            = L"labeliFRfFRValue";
		labeliFRfFRValue.position        = Point(93, 0);
		labeliFRfFRValue.size            = Size(92, 11);
		labeliFRfFRValue.font            = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labeliFRfFRValue.layout          = Layout::Fill;
		labeliFRfFRValue.text            = L"0.85";
		labeliFRfFRValue.color           = Palette::Gray070;
		labeliFRfFRValue.horizontalAlign = HAlign::Right;
		labeliFRfFRValue.verticalAlign   = VAlign::Middle;
		progressPanel.name               = L"progressPanel";
		progressPanel.position           = Point(185, 0);
		progressPanel.size               = Size(93, 11);
		progressPanel.layout             = Layout::Fill;
		progressPanel.layoutSpacing      = 0;
		progressPanel.border             = Margin(4, 18, 4, 18);
		progressPanel.edge               = Margin(0, 0, 0, 0);
		//progress.composerClass         = Control;
		progress.name                    = L"progress";
		progress.size                    = Size(85, -25);
		progress.layout                  = Layout::Fill;
		panelPaPdNames.name              = L"panelPaPdNames";
		panelPaPdNames.position          = Point(0, 11);
		panelPaPdNames.size              = Size(278, 10);
		panelPaPdNames.layout            = Layout::Fill;
		panelPaPdNames.layoutSpacing     = 0;
		panelPaPdNames.layoutOrientation = Orientation::Horizontal;
		panelPaPdNames.border            = Margin(0, 0, 0, 0);
		panelPaPdNames.edge              = Margin(0, 0, 0, 0);
		labelPdPa.name                   = L"labelPdPa";
		labelPdPa.size                   = Size(139, 10);
		labelPdPa.font                   = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labelPdPa.layout                 = Layout::Fill;
		labelPdPa.text                   = L"Pd/Pa";
		labelPdPa.color                  = Palette::Gray070;
		labelPdPa.verticalAlign          = VAlign::Middle;
		labelPdPaValue.name              = L"labelPdPaValue";
		labelPdPaValue.position          = Point(139, 0);
		labelPdPaValue.size              = Size(139, 10);
		labelPdPaValue.font              = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labelPdPaValue.layout            = Layout::Fill;
		labelPdPaValue.text              = L"0.85";
		labelPdPaValue.color             = Palette::Gray070;
		labelPdPaValue.horizontalAlign   = HAlign::Right;
		labelPdPaValue.verticalAlign     = VAlign::Middle;
		panelPaNames.name                = L"panelPaNames";
		panelPaNames.position            = Point(0, 21);
		panelPaNames.size                = Size(278, 11);
		panelPaNames.layout              = Layout::Fill;
		panelPaNames.layoutSpacing       = 0;
		panelPaNames.layoutOrientation   = Orientation::Horizontal;
		labelPa.name                     = L"labelPa";
		labelPa.size                     = Size(139, 11);
		labelPa.font                     = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labelPa.layout                   = Layout::Fill;
		labelPa.text                     = L"Pa";
		labelPa.color                    = Palette::Red050;
		labelPa.verticalAlign            = VAlign::Middle;
		labelPaValue.name                = L"labelPaValue";
		labelPaValue.position            = Point(139, 0);
		labelPaValue.size                = Size(139, 11);
		labelPaValue.font                = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labelPaValue.layout              = Layout::Fill;
		labelPaValue.text                = L"74";
		labelPaValue.color               = Palette::Red050;
		labelPaValue.horizontalAlign     = HAlign::Right;
		labelPaValue.verticalAlign       = VAlign::Middle;
		panelPdNames.name                = L"panelPdNames";
		panelPdNames.position            = Point(0, 32);
		panelPdNames.size                = Size(278, 10);
		panelPdNames.layout              = Layout::Fill;
		panelPdNames.layoutSpacing       = 0;
		panelPdNames.layoutOrientation   = Orientation::Horizontal;
		labelPd.name                     = L"labelPd";
		labelPd.size                     = Size(139, 10);
		labelPd.font                     = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labelPd.layout                   = Layout::Fill;
		labelPd.text                     = L"Pd";
		labelPd.verticalAlign            = VAlign::Middle;
		labelPdValue.name                = L"labelPdValue";
		labelPdValue.position            = Point(139, 0);
		labelPdValue.size                = Size(139, 10);
		labelPdValue.font                = Font(L"Philips Healthcare Font", 34, false, false, false, false, Font::Antialias::High);
		labelPdValue.layout              = Layout::Fill;
		labelPdValue.text                = L"69";
		labelPdValue.horizontalAlign     = HAlign::Right;
		labelPdValue.verticalAlign       = VAlign::Middle;
		ifrSpotReviewPanel.name          = L"ifrSpotReviewPanel";
		ifrSpotReviewPanel.position      = Point(0, 137);
		ifrSpotReviewPanel.size          = Size(290, 69);
		ifrSpotReviewPanel.layout        = Layout::Fill;
		ifrSpotReviewPanel.border        = Margin(0, 0, 0, 0);
		ifrSpotReviewPanel.edge          = Margin(0, 0, 0, 0);
		labeliFrSpot.name                = L"labeliFrSpot";
		labeliFrSpot.size                = Size(290, 31);
		labeliFrSpot.font                = Font(L"Philips Healthcare Font", 50, false, false, false, false, Font::Antialias::High);
		labeliFrSpot.layout              = Layout::Fill;
		labeliFrSpot.text                = L"iFR";
		labeliFrSpot.color               = Palette::Gray070;
		labeliFrSpot.horizontalAlign     = HAlign::Center;
		labeliFrSpot.verticalAlign       = VAlign::Middle;
		labeliFrSpotValue.name           = L"labeliFrSpotValue";
		labeliFrSpotValue.position       = Point(0, 39);
		labeliFrSpotValue.size           = Size(290, 30);
		labeliFrSpotValue.font           = Font(L"Philips Healthcare Font", 60, false, false, false, false, Font::Antialias::High);
		labeliFrSpotValue.layout         = Layout::Fill;
		labeliFrSpotValue.text           = L"---";
		labeliFrSpotValue.color          = Palette::Gray070;
		labeliFrSpotValue.horizontalAlign = HAlign::Center;
		labeliFrSpotValue.verticalAlign  = VAlign::Middle;
		graphLayoutPanel.name            = L"graphLayoutPanel";
		graphLayoutPanel.position        = Point(2, 252);
		graphLayoutPanel.size            = Size(292, 416);
		graphLayoutPanel.layout          = Layout::Fill;
		graphLayoutPanel.layoutOuterMargin = Margin(2, 8, 0, 0);
		graphLayoutPanel.layoutSpacing   = 0;
		graphPanel.name                  = L"graphPanel";
		graphPanel.size                  = Size(292, 416);
		graphPanel.layout                = Layout::Fill;
		graphPanel.border                = Margin(0, 0, 0, 0);
		graphPanel.edge                  = Margin(0, 0, 0, 0);
		bottumLayoutSpace.name           = L"bottumLayoutSpace";
		bottumLayoutSpace.position       = Point(0, 670);
		bottumLayoutSpace.size           = Size(296, 266);
		bottumLayoutSpace.layout         = Layout::Fixed1d;
		layoutUpdateEnd();
		#pragma endregion
	}

	#pragma region Sense Composer (do not modify)
	public:    Panel             panelPopup;
	public:    Panel             popupPanelHeader;
	public:    Label             popupPanelHeaderText;
	public:    Panel             topLayOutPannel;
	public:    SmartSuiteFlexvisionIfrReviewPanel ifrReviewPanel;
	public:    Control           panelValues;
	public:    Panel             paneliFrFfrNames;
	public:    Label             labeliFRfFR;
	public:    Label             labeliFRfFRValue;
	public:    Panel             progressPanel;
	public:    PciProgressBar    progress;
	public:    Panel             panelPaPdNames;
	public:    Label             labelPdPa;
	public:    Label             labelPdPaValue;
	public:    Control           panelPaNames;
	public:    Label             labelPa;
	public:    Label             labelPaValue;
	public:    Control           panelPdNames;
	public:    Label             labelPd;
	public:    Label             labelPdValue;
	public:    Panel             ifrSpotReviewPanel;
	public:    Label             labeliFrSpot;
	public:    Label             labeliFrSpotValue;
	public:    Spacer            graphLayoutPanel;
	public:    Panel             graphPanel;
	public:    Spacer            bottumLayoutSpace;
	#pragma endregion

private:

	SmartSuiteFlexvisionCoregUi(const SmartSuiteFlexvisionCoregUi&);
	SmartSuiteFlexvisionCoregUi& operator=(const SmartSuiteFlexvisionCoregUi&);
};

}}
