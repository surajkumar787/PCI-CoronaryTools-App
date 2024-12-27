/*----------------------------------------------------
 |   This file was generated by Sense Composer 2.0   |
 |  Do not modify the Sense Composer Generated Code  |
 ----------------------------------------------------*/
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>
#include "ExperienceIdentity/Shapes/Background.h"
#include "ButtonTransparent.h"
#include "Resources.h"

namespace Pci { namespace Core
{
using namespace Sense;
using namespace Sense::ExperienceIdentity;

class SmartSuiteAboutBoxDialog : public Panel
{
public:

	SmartSuiteAboutBoxDialog(Control &parent)
	:
		Panel(parent)
		#pragma region Sense Composer (do not modify)
	,	wordMarkPanel    ((Control&)*this)
	,	imageBox         (wordMarkPanel)
	,	close            (wordMarkPanel)
	,	panelBadge       ((Control&)*this)
	,	productName      (panelBadge)
	,	spacer           (*this)
	,	releaseLabel     (*this)
	,	spacer2          (*this)
	,	refLotUdiPanel   ((Control&)*this)
	,	panel6           ((Control&)refLotUdiPanel)
	,	panel7           ((Control&)panel6)
	,	label4           (panel7)
	,	refNumberLabel   (panel6)
	,	panel            ((Control&)refLotUdiPanel)
	,	panel2           ((Control&)panel)
	,	label            (panel2)
	,	lotNumberLabel   (panel)
	,	panel3           ((Control&)refLotUdiPanel)
	,	panel4           ((Control&)panel3)
	,	label2           (panel4)
	,	udiNumberLabel   (panel3)
	,	spacer3          (*this)
	,	logoSectionPanel ((Control&)*this)
	,	panel8           ((Control&)logoSectionPanel)
	,	label5           (panel8)
	,	ceLabel          (logoSectionPanel)
	,	spacer4          (*this)
	,	panel9           ((Control&)*this)
	,	manufacturingDate(panel9)
	,	address          (panel9)
	,	spacer5          (*this)
	,	copyrightClause  (*this)
	,	reproduceClause  (*this)
	,	eulaStatement    (*this)
	,	spacer9          (*this)
	,	spacer6          (*this)
	,	spacer7          ((Control&)spacer6)
	,	panel10          (spacer6)
	,	spacer8          (panel10)
	,	label3           (panel10)
	,	ifuIndicatorLabel(panel10)
	,	warningLabel     (panel10)
		#pragma endregion
	{
		#pragma region Sense Composer (do not modify)
		layoutUpdateBegin();
		//composerReference              = C:\Users\320024878\Pictures\AboutBoxPCI.png;
		//composerImageFolder            = .\Resources;
		name                             = L"PciAboutBoxDialog";
		size                             = Size(774, 750);
		layoutSpacing                    = 0;
		radius                           = Corners(6, 6, 6, 6);
		edge                             = Margin(2, 2, 2, 2);
		shadow                           = Margin(0, 0, 15, 15);
		colorEdge                        = Color::fromRgb(92, 90, 84);
		color                            = Color::fromRgb(0, 0, 0);
		wordMarkPanel.name               = L"wordMarkPanel";
		wordMarkPanel.size               = Size(770, 180);
		wordMarkPanel.layout             = Layout::Fixed1d;
		wordMarkPanel.colorBorder        = Palette::Transparent * 0.00;
		wordMarkPanel.colorEdge          = Palette::Transparent * 0.00;
		wordMarkPanel.color              = Palette::Transparent * 0.00;
		imageBox.name                    = L"wordMark";
		imageBox.position                = Point(50, 50);
		imageBox.size                    = Size(670, 80);
		imageBox.layoutUseSpacing        = false;
		imageBox.layoutHideInvisible     = false;
		imageBox.layout                  = Layout::Fill;
		imageBox.layoutOuterMargin       = Margin(50, 50, 50, 50);
		//close.composerClass            = Control;
		close.name                       = L"close";
		close.position                   = Point(733, 2);
		close.size                       = Size(32, 32);
		panelBadge.name                  = L"panelBadge";
		panelBadge.position              = Point(249, 180);
		panelBadge.size                  = Size(272, 49);
		panelBadge.layout                = Layout::Fixed2d;
		panelBadge.radius                = Corners(0, 0, 15, 15);
		panelBadge.colorBorder           = Palette::Transparent * 0.00;
		panelBadge.colorEdge             = Palette::Transparent * 0.00;
		panelBadge.color                 = Palette::Transparent * 0.00;
		productName.name                 = L"productName";
		productName.size                 = Size(272, 49);
		productName.font                 = Font(L"Gill Sans Alt One WGL", 28, true, false, false, false, Font::Antialias::High);
		productName.layout               = Layout::Fill;
		productName.text                 = L"<Product name>";
		productName.color                = Palette::White;
		productName.horizontalAlign      = HAlign::Center;
		productName.verticalAlign        = VAlign::Middle;
		spacer.position                  = Point(0, 229);
		spacer.size                      = Size(770, 8);
		spacer.layout                    = Layout::Fixed1d;
		releaseLabel.name                = L"releaseLabel";
		releaseLabel.position            = Point(0, 237);
		releaseLabel.size                = Size(770, 30);
		releaseLabel.font                = Font(L"Philips Healthcare Font", 18, false, false, false, false, Font::Antialias::High);
		releaseLabel.layout              = Layout::Fixed1d;
		releaseLabel.text                = L"< Release number >";
		releaseLabel.horizontalAlign     = HAlign::Center;
		releaseLabel.verticalAlign       = VAlign::Middle;
		spacer2.position                 = Point(0, 267);
		spacer2.size                     = Size(770, 26);
		spacer2.layout                   = Layout::Fixed1d;
		refLotUdiPanel.name              = L"refLotUdiPanel";
		refLotUdiPanel.position          = Point(237, 293);
		refLotUdiPanel.size              = Size(297, 104);
		refLotUdiPanel.layoutUseSpacing  = false;
		refLotUdiPanel.layout            = Layout::Fixed2d;
		refLotUdiPanel.colorEdge         = Palette::Transparent * 0.00;
		refLotUdiPanel.color             = Palette::Transparent * 0.00;
		panel6.size                      = Size(297, 35);
		panel6.layoutUseSpacing          = false;
		panel6.layout                    = Layout::Fill;
		panel6.layoutOrientation         = Orientation::Horizontal;
		panel6.colorBorder               = Palette::Transparent * 0.00;
		panel6.colorEdge                 = Palette::Transparent * 0.00;
		panel6.color                     = Palette::Transparent * 0.00;
		panel7.size                      = Size(40, 35);
		panel7.layout                    = Layout::Fixed1d;
		panel7.colorBorder               = Palette::Transparent * 0.00;
		panel7.colorEdge                 = Palette::Transparent * 0.00;
		panel7.color                     = Palette::Transparent * 0.00;
		label4.name                      = L"label4";
		label4.size                      = Size(40, 35);
		label4.font                      = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		label4.layout                    = Layout::Fill;
		label4.text                      = L"REF";
		label4.horizontalAlign           = HAlign::Center;
		label4.verticalAlign             = VAlign::Middle;
		refNumberLabel.name              = L"refNumberLabel";
		refNumberLabel.position          = Point(63, 0);
		refNumberLabel.size              = Size(234, 35);
		refNumberLabel.font              = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		refNumberLabel.layout            = Layout::Fill;
		refNumberLabel.layoutOuterMargin = Margin(15, 0, 0, 0);
		refNumberLabel.text              = L"<Ref number>";
		refNumberLabel.verticalAlign     = VAlign::Middle;
		panel.position                   = Point(0, 35);
		panel.size                       = Size(297, 34);
		panel.layout                     = Layout::Fill;
		panel.layoutOrientation          = Orientation::Horizontal;
		panel.colorBorder                = Palette::Transparent * 0.00;
		panel.colorEdge                  = Palette::Transparent * 0.00;
		panel.color                      = Palette::Transparent * 0.00;
		panel2.size                      = Size(40, 34);
		panel2.layout                    = Layout::Fixed1d;
		panel2.colorBorder               = Palette::Transparent * 0.00;
		panel2.colorEdge                 = Palette::Transparent * 0.00;
		panel2.color                     = Palette::Transparent * 0.00;
		label.name                       = L"label";
		label.size                       = Size(40, 34);
		label.font                       = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		label.layout                     = Layout::Fill;
		label.text                       = L"LOT";
		label.horizontalAlign            = HAlign::Center;
		label.verticalAlign              = VAlign::Middle;
		lotNumberLabel.name              = L"lotNumberLabel";
		lotNumberLabel.position          = Point(63, 0);
		lotNumberLabel.size              = Size(234, 34);
		lotNumberLabel.font              = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		lotNumberLabel.layout            = Layout::Fill;
		lotNumberLabel.layoutOuterMargin = Margin(15, 0, 0, 0);
		lotNumberLabel.text              = L"<Lot number>";
		lotNumberLabel.verticalAlign     = VAlign::Middle;
		panel3.position                  = Point(0, 69);
		panel3.size                      = Size(297, 35);
		panel3.layoutUseSpacing          = false;
		panel3.layout                    = Layout::Fill;
		panel3.layoutOrientation         = Orientation::Horizontal;
		panel3.colorBorder               = Palette::Transparent * 0.00;
		panel3.colorEdge                 = Palette::Transparent * 0.00;
		panel3.color                     = Palette::Transparent * 0.00;
		panel4.size                      = Size(40, 35);
		panel4.layout                    = Layout::Fixed1d;
		panel4.colorBorder               = Palette::Transparent * 0.00;
		panel4.colorEdge                 = Palette::Transparent * 0.00;
		panel4.color                     = Palette::Transparent * 0.00;
		label2.name                      = L"label2";
		label2.size                      = Size(40, 35);
		label2.font                      = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		label2.layout                    = Layout::Fill;
		label2.text                      = L"UDI";
		label2.horizontalAlign           = HAlign::Center;
		label2.verticalAlign             = VAlign::Middle;
		udiNumberLabel.name              = L"udiNumberLabel";
		udiNumberLabel.position          = Point(63, 0);
		udiNumberLabel.size              = Size(234, 35);
		udiNumberLabel.font              = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		udiNumberLabel.layout            = Layout::Fill;
		udiNumberLabel.layoutOuterMargin = Margin(15, 0, 0, 0);
		udiNumberLabel.text              = L"<UDI number>";
		udiNumberLabel.verticalAlign     = VAlign::Middle;
		spacer3.position                 = Point(0, 397);
		spacer3.size                     = Size(770, 24);
		spacer3.layout                   = Layout::Fixed1d;
		logoSectionPanel.name            = L"logoSectionPanel";
		logoSectionPanel.position        = Point(282, 421);
		logoSectionPanel.size            = Size(207, 40);
		logoSectionPanel.layout          = Layout::Fixed2d;
		logoSectionPanel.layoutOrientation = Orientation::Horizontal;
		logoSectionPanel.border			 = Margin(0, 0, 0, 0);
		logoSectionPanel.edge			 = Margin(0, 0, 0, 0);
		logoSectionPanel.colorBorder     = Palette::Transparent * 0.00;
		logoSectionPanel.colorEdge       = Palette::Transparent * 0.00;
		logoSectionPanel.color           = Palette::Transparent * 0.00;
		panel8.name                      = L"mdLogoPanel";
		panel8.size                      = Size(64, 40);
		panel8.layoutUseSpacing          = false;
		panel8.layout                    = Layout::Fixed1d;
		panel8.edge						 = Margin(0, 0, 0, 0);
		panel8.colorBorder               = Palette::Transparent * 0.00;
		panel8.colorEdge                 = Palette::Transparent * 0.00;
		panel8.color                     = Palette::Transparent * 0.00;
		label5.name                      = L"mdLogoText";
		label5.size                      = Size(64, 40);
		label5.font                      = Font(L"Philips Healthcare Font", 28, true, false, false, false, Font::Antialias::High);
		label5.layout                    = Layout::Fill;
		label5.text                      = L"MD";
		label5.horizontalAlign           = HAlign::Center;
		label5.verticalAlign             = VAlign::Middle;
		ceLabel.name                     = L"ceLabel";
		ceLabel.position                 = Point(79, 0);
		ceLabel.size                     = Size(128, 40);
		ceLabel.font                     = Font(L"Philips Healthcare Font", 16, false, false, false, false, Font::Antialias::High);
		ceLabel.layout                   = Layout::Fill;
		ceLabel.layoutOuterMargin        = Margin(15, 0, 0, 0);
		ceLabel.verticalAlign            = VAlign::Middle;
		spacer4.position                 = Point(0, 461);
		spacer4.size                     = Size(770, 16);
		spacer4.layout                   = Layout::Fixed1d;
		panel9.position                  = Point(137, 477);
		panel9.size                      = Size(497, 64);
		panel9.layout                    = Layout::Fixed2d;
		panel9.layoutSpacing             = 15;
		panel9.layoutOrientation         = Orientation::Horizontal;
		panel9.colorBorder               = Palette::Transparent * 0.00;
		panel9.colorEdge                 = Palette::Transparent * 0.00;
		panel9.color                     = Palette::Transparent * 0.00;
		manufacturingDate.name           = L"manufacturingDate";
		manufacturingDate.size           = Size(128, 64);
		manufacturingDate.font           = Font(L"Philips Healthcare Font", 14, false, false, false, false, Font::Antialias::High);
		manufacturingDate.layout         = Layout::Fixed1d;
		manufacturingDate.text           = L"<Manufacturing date>";
		manufacturingDate.horizontalAlign = HAlign::Right;
		manufacturingDate.verticalAlign  = VAlign::Middle;
		manufacturingDate.imageSpacing   = 0;
		manufacturingDate.imagePosition  = ImagePosition::Top;
		address.name                     = L"address";
		address.position                 = Point(143, 0);
		address.size                     = Size(354, 64);
		address.font                     = Font(L"Philips Healthcare Font", 14, false, false, false, false, Font::Antialias::High);
		address.layout                   = Layout::Fill;
		address.text                     = L"Philips Medical Systems Nederland B.V.\nVeenpluis 6 5684 PC Best THE NETHERLANDS";
		address.textType                 = TextType::MultiLine;
		address.verticalAlign            = VAlign::Middle;
		spacer5.position                 = Point(0, 541);
		spacer5.size                     = Size(770, 8);
		spacer5.layout                   = Layout::Fixed1d;
		copyrightClause.name             = L"copyrightClause";
		copyrightClause.position         = Point(0, 549);
		copyrightClause.size             = Size(770, 16);
		copyrightClause.font             = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		copyrightClause.layout           = Layout::Fixed1d;
		copyrightClause.text             = L"\u00a9 Koninklijke Philips N.V., 2016 - 2024. All rights reserved.";
		copyrightClause.horizontalAlign  = HAlign::Center;
		reproduceClause.name             = L"reproduceClause";
		reproduceClause.position         = Point(21, 565);
		reproduceClause.size             = Size(728, 87);
		reproduceClause.font             = Font(L"Philips Healthcare Font", 12, false, false, false, false, Font::Antialias::High);
		reproduceClause.layout           = Layout::Fixed2d;
		reproduceClause.text             = L"Reproduction or transmission in whole or in part, in any form or by any means, electronic, mechanical or otherwise, is prohibited without the prior written consent of the copyright owner. Copyrights and all other proprietary rights in any software and related documentation (\"Software\") made available to you rest exclusively with Philips or its licensors. No title or ownership in the Software is conferred to you. Use of the Software is subject to the end user license conditions as are available on request. To the maximum extent permitted by law, you shall not decompile and/or reverse engineer the software or any part thereof.";
		reproduceClause.textType         = TextType::WordWrap;
		reproduceClause.horizontalAlign  = HAlign::Center;
		//eulaStatement.composerClass    = Control;
		eulaStatement.name               = L"eulaStatement";
		eulaStatement.position           = Point(0, 662);
		eulaStatement.size               = Size(770, 20);
		eulaStatement.font               = Font(L"Philips Healthcare Font", 12, false, false, true, false, Font::Antialias::High);
		eulaStatement.layoutOuterMargin  = Margin(0, 10, 0, 0);
		spacer9.position                 = Point(0, 682);
		spacer9.size                     = Size(770, 16);
		spacer9.layout                   = Layout::Fill;
		spacer6.position                 = Point(0, 698);
		spacer6.size                     = Size(770, 48);
		spacer6.layout                   = Layout::Fixed1d;
		spacer6.layoutOrientation        = Orientation::Horizontal;
		spacer7.size                     = Size(256, 48);
		spacer7.layout                   = Layout::Fixed1d;
		panel10.position                 = Point(256, 0);
		panel10.size                     = Size(514, 48);
		panel10.layoutUseSpacing         = false;
		panel10.layout                   = Layout::Fill;
		panel10.layoutOrientation        = Orientation::Horizontal;
		panel10.colorBorder              = Palette::Transparent * 0.00;
		panel10.colorEdge                = Palette::Transparent * 0.00;
		panel10.color                    = Palette::Transparent * 0.00;
		spacer8.size                     = Size(184, 48);
		spacer8.layout                   = Layout::Fixed1d;
		label3.name                      = L"prescriptionOnlyLabel";
		label3.position                  = Point(192, 0);
		label3.size                      = Size(64, 48);
		label3.font                      = Font(L"Philips Healthcare Font", 12, true, false, false, false, Font::Antialias::High);
		label3.layout                    = Layout::Fixed1d;
		label3.text                      = L"Rx only";
		label3.horizontalAlign           = HAlign::Center;
		label3.verticalAlign             = VAlign::Middle;
		ifuIndicatorLabel.name           = L"ifuIndicatorLabel";
		ifuIndicatorLabel.position       = Point(264, 0);
		ifuIndicatorLabel.size           = Size(176, 48);
		ifuIndicatorLabel.layout         = Layout::Fixed1d;
		ifuIndicatorLabel.text           = L"www.philips.com/IFU";
		ifuIndicatorLabel.horizontalAlign = HAlign::Center;
		ifuIndicatorLabel.verticalAlign  = VAlign::Middle;
		ifuIndicatorLabel.imageSize      = Size(30, 30);
		warningLabel.name                = L"warningLabel";
		warningLabel.position            = Point(448, 0);
		warningLabel.size                = Size(56, 48);
		warningLabel.layout              = Layout::Fill;
		warningLabel.layoutOuterMargin   = Margin(0, 0, 10, 0);
		warningLabel.text                = L"";
		warningLabel.horizontalAlign     = HAlign::Right;
		warningLabel.verticalAlign       = VAlign::Middle;
		warningLabel.imageSize           = Size(30, 30);
		warningLabel.imageSpacing        = 0;
		warningLabel.imagePosition       = ImagePosition::Right;
		layoutUpdateEnd();
		#pragma endregion
	}

	#pragma region Sense Composer (do not modify)
	public:    Panel             wordMarkPanel;
	public:    ImageBox          imageBox;
	public:    CloseButton       close;
	public:    Panel             panelBadge;
	public:    Label             productName;
	private:   Spacer            spacer;
	public:    Label             releaseLabel;
	private:   Spacer            spacer2;
	public:    Panel             refLotUdiPanel;
	private:   Panel             panel6;
	private:   Panel             panel7;
	public:    Label             label4;
	public:    Label             refNumberLabel;
	private:   Panel             panel;
	private:   Panel             panel2;
	public:    Label             label;
	public:    Label             lotNumberLabel;
	private:   Panel             panel3;
	private:   Panel             panel4;
	public:    Label             label2;
	public:    Label             udiNumberLabel;
	private:   Spacer            spacer3;
	public:    Panel             logoSectionPanel;
	public:    Panel             panel8;
	public:    Label             label5;
	public:    Label             ceLabel;
	private:   Spacer            spacer4;
	private:   Panel             panel9;
	public:    Label             manufacturingDate;
	public:    Label             address;
	private:   Spacer            spacer5;
	public:    Label             copyrightClause;
	public:    Label             reproduceClause;
	public:    ButtonTransparent eulaStatement;
	private:   Spacer            spacer9;
	private:   Spacer            spacer6;
	private:   Spacer            spacer7;
	private:   Panel             panel10;
	private:   Spacer            spacer8;
	private:   Label             label3;
	public:    Label             ifuIndicatorLabel;
	public:    Label             warningLabel;
	#pragma endregion

private:

	SmartSuiteAboutBoxDialog(const SmartSuiteAboutBoxDialog&);
	SmartSuiteAboutBoxDialog& operator=(const SmartSuiteAboutBoxDialog&);
};

}}