/*----------------------------------------------------
 |   This file was generated by Sense Composer 2.0   |
 |  Do not modify the Sense Composer Generated Code  |
 ----------------------------------------------------*/
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>
#include "MenuButtonSmartSuite.h"
#include "PatientBar.h"

namespace Pci { namespace Core
{
using namespace Sense;
using namespace Sense::ExperienceIdentity;

class MainSmartSuiteFullHDView : public Control
{
public:

	MainSmartSuiteFullHDView(Control &parent)
	:
		Control(parent)
		#pragma region Sense Composer (do not modify)
	,	side             ((Control&)*this)
	,	separator        (side)
	,	stbMarkerImage   ((Control&)side)
	,	bottomBar        (*this)
	,	labelMessageArea1(bottomBar)
	,	labelCoregInfo   (bottomBar)
	,	separator2       (bottomBar)
	,	labelMessageArea2(bottomBar)
	,	buffer           (bottomBar)
	,	fmDisconnect     (buffer)
	,	icontrolDisconnectLabel(buffer)
	,	main             ((Control&)*this)
	,	topBar           (*this)
	,	systemMenu       (topBar)
	,	helpMenu         (topBar)
	,	label            (topBar)
	,	tab              (topBar)
	,	tabCoronaryTools (tab)
	,	patientBar       (topBar, PatientBarColorScheme::SmS)
	,	separator3       (*this)
	,	separator4       (*this)
		#pragma endregion
	{
		#pragma region Sense Composer (do not modify)
		layoutUpdateBegin();
		size                             = Size(1920, 1080);
		font                             = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);
		side.name                        = L"side";
		side.position                    = Point(968, 30);
		side.size                        = Size(952, 962);
		separator.name                   = L"separator";
		separator.position               = Point(0, 4);
		separator.size                   = Size(2, 954);
		separator.color1                 = Palette::Gray150;
		separator.color2                 = Palette::Transparent;
		separator.orientation            = Orientation::Vertical;
		stbMarkerImage.name              = L"stbMarkerImage";
		stbMarkerImage.position          = Point(4, 4);
		stbMarkerImage.size              = Size(360, 360);
		bottomBar.name                   = L"bottomBar";
		bottomBar.position               = Point(0, 992);
		bottomBar.size                   = Size(1920, 88);
		bottomBar.border                 = Margin(0, 0, 0, 0);
		bottomBar.color                  = Color::fromRgb(0, 0, 0);
		labelMessageArea1.name           = L"labelMessageArea1";
		labelMessageArea1.position       = Point(135, 8);
		labelMessageArea1.size           = Size(1745, 24);
		labelMessageArea1.font           = Font(L"Philips Healthcare Font", 18, false, false, false, false, Font::Antialias::High);
		labelMessageArea1.text           = L"Use touchscreen XperModule to archive a snapshot or movie";
		labelMessageArea1.textType       = TextType::WordWrap;
		labelMessageArea1.color          = Palette::Gray070;
		labelCoregInfo.name              = L"labelCoregInfo";
		labelCoregInfo.position          = Point(135, 60);
		labelCoregInfo.size              = Size(1745, 24);
		labelCoregInfo.font              = Font(L"Philips Healthcare Font", 18, false, false, false, false, Font::Antialias::High);
		labelCoregInfo.text              = L"Unreliable FFR/iFR network connection detected";
		labelCoregInfo.color             = Palette::Gray070;
		separator2.name                  = L"separator2";
		separator2.position              = Point(125, 2);
		separator2.size                  = Size(2, 84);
		separator2.color1                = Palette::Gray150;
		separator2.color2                = Palette::Transparent;
		separator2.orientation           = Orientation::Vertical;
		labelMessageArea2.name           = L"labelMessageArea2";
		labelMessageArea2.position       = Point(135, 34);
		labelMessageArea2.size           = Size(1745, 24);
		labelMessageArea2.font           = Font(L"Philips Healthcare Font", 18, false, false, false, false, Font::Antialias::High);
		labelMessageArea2.text           = L"Use touchscreen XperModule to archive a snapshot or movie";
		labelMessageArea2.textType       = TextType::WordWrap;
		labelMessageArea2.color          = Palette::Gray070;
		buffer.position                  = Point(4, 8);
		buffer.size                      = Size(116, 72);
		buffer.layoutOrientation         = Orientation::Horizontal;
		fmDisconnect.name                = L"fmDisconnect";
		fmDisconnect.size                = Size(54, 72);
		fmDisconnect.layout              = Layout::Fill;
		fmDisconnect.text                = L"";
		fmDisconnect.horizontalAlign     = HAlign::Center;
		fmDisconnect.verticalAlign       = VAlign::Middle;
		fmDisconnect.image               = Icons::RemoteConnectivityOff;
		fmDisconnect.imageSize           = Size(48, 48);
		icontrolDisconnectLabel.name     = L"icontrolDisconnectLabel";
		icontrolDisconnectLabel.position = Point(62, 0);
		icontrolDisconnectLabel.size     = Size(54, 72);
		icontrolDisconnectLabel.layout   = Layout::Fill;
		icontrolDisconnectLabel.text     = L"";
		icontrolDisconnectLabel.horizontalAlign = HAlign::Center;
		icontrolDisconnectLabel.verticalAlign = VAlign::Middle;
		icontrolDisconnectLabel.image    = Icons::ConnectionWarning;
		icontrolDisconnectLabel.imageSize = Size(48, 48);
		main.name                        = L"main";
		main.position                    = Point(0, 32);
		main.size                        = Size(960, 960);
		topBar.name                      = L"topBar";
		topBar.size                      = Size(1920, 32);
		topBar.border                    = Margin(0, 0, 0, 0);
		topBar.edge                      = Margin(0, 0, 0, 0);
		topBar.color                     = Color::fromRgb(0, 0, 0);
		//systemMenu.composerClass       = Control;
		systemMenu.name                  = L"systemMenu";
		systemMenu.position              = Point(1656, 0);
		systemMenu.size                  = Size(80, 31);
		systemMenu.font                  = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);
		//helpMenu.composerClass         = Control;
		helpMenu.name                    = L"helpMenu";
		helpMenu.position                = Point(1736, 0);
		helpMenu.size                    = Size(80, 30);
		helpMenu.font                    = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);
		label.name                       = L"label";
		label.position                   = Point(1816, 0);
		label.size                       = Size(104, 32);
		label.text                       = L"";
		label.horizontalAlign            = HAlign::Center;
		label.verticalAlign              = VAlign::Middle;
		label.image                      = Icons::Wordmark;
		label.imageSize                  = Size(96, 32);
		tab.name                         = L"tab";
		tab.position                     = Point(8, 0);
		tab.size                         = Size(256, 30);
		tab.font                         = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);
		tab.headerWidth                  = 230;
		tab.headerHeight                 = 30;
		tab.headerImageSize              = Size(22, 22);
		tabCoronaryTools.name            = L"tabCoronaryTools";
		tabCoronaryTools.text            = L"Coronary Tools";
		tabCoronaryTools.image           = Icons::HeartSegmentation;
		//patientBar.composerClass       = Control;
		patientBar.name                  = L"PatientBar";
		patientBar.position              = Point(618, 0);
		patientBar.size                  = Size(684, 30);
		patientBar.font                  = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);
		separator3.name                  = L"separator3";
		separator3.position              = Point(0, 29);
		separator3.size                  = Size(1920, 2);
		separator3.color1                = Palette::Gray150;
		separator3.color2                = Palette::Transparent;
		separator4.name                  = L"separator4";
		separator4.position              = Point(0, 992);
		separator4.size                  = Size(1920, 2);
		separator4.color1                = Palette::Gray150;
		separator4.color2                = Palette::Transparent;
		layoutUpdateEnd();
		#pragma endregion
	}

	#pragma region Sense Composer (do not modify)
	public:    Control              side;
	public:    Separator            separator;
	public:    Control              stbMarkerImage;
	public:    Panel                bottomBar;
	public:    Label             labelMessageArea1;
	public:    Label                labelCoregInfo;
	public:    Separator            separator2;
	public:    Label             labelMessageArea2;
	private:   Buffer            buffer;
	public:    Label                fmDisconnect;
	public:    Label             icontrolDisconnectLabel;
	public:    Control              main;
	public:    Panel                topBar;
	public:    MenuButtonSmartSuite systemMenu;
	public:    MenuButtonSmartSuite helpMenu;
	public:    Label                label;
	public:    TabButtonBar         tab;
	public:    TabButton            tabCoronaryTools;
	public:    PatientBar           patientBar;
	public:    Separator            separator3;
	public:    Separator            separator4;
	#pragma endregion

private:

	MainSmartSuiteFullHDView(const MainSmartSuiteFullHDView&);
	MainSmartSuiteFullHDView& operator=(const MainSmartSuiteFullHDView&);
};

}}