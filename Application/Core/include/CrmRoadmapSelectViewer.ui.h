/*----------------------------------------------------
 |   This file was generated by Sense Composer 2.0   |
 |  Do not modify the Sense Composer Generated Code  |
 ----------------------------------------------------*/
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>

#include "ViewType.h"
#include "Translation.h"

using namespace Sense;
using namespace Sense::ExperienceIdentity;

using  namespace Pci::Core;

namespace Pci { namespace Core { namespace UI
{

class CrmRoadmapSelectViewer : public Control
{
public:

	CrmRoadmapSelectViewer(Control& parent, ViewType type)
		:
		Control (parent)
		#pragma region Sense Composer (do not modify)
	,	spacer           (*this)
	,	selectedRoadmapLabel(spacer)
	,	crmThumbnail     (spacer)
	,	spacer4          ((Control&)spacer)
	,	roadmapReadyLabel(spacer4)
	,	separator        (spacer)
	,	spacer2          (*this)
	,	panel            (spacer2)
	,	label            (panel)
	,	guidanceTextLabel(panel)
	,	spacer3          ((Control&)spacer2)
	,	pictorialListControl(spacer2, type)
		#pragma endregion
	{
		#pragma region Sense Composer (do not modify)
		layoutUpdateBegin();
		name                             = L"CrmRoadmapSelectViewer";
		size                             = Size(990, 992);
		layoutSpacing                    = 0;
		spacer.size                      = Size(990, 585);
		spacer.layout                    = Layout::Fixed1d;
		selectedRoadmapLabel.name        = L"selectedRoadmapLabel";
		selectedRoadmapLabel.position    = Point(0, 32);
		selectedRoadmapLabel.size        = Size(990, 40);
		selectedRoadmapLabel.font        = Font(L"Philips Healthcare Font", 30, true, false, false, false, Font::Antialias::High);
		selectedRoadmapLabel.layout      = Layout::Fixed2d;
		selectedRoadmapLabel.layoutOuterMargin = Margin(0, 32, 0, 0);
		selectedRoadmapLabel.text        = L"Roadmap X";
		selectedRoadmapLabel.color       = Palette::Gray070;
		selectedRoadmapLabel.horizontalAlign = HAlign::Center;
		selectedRoadmapLabel.verticalAlign = VAlign::Middle;
		//crmThumbnail.composerClass     = Control;
		crmThumbnail.name                = L"crmThumbnail";
		crmThumbnail.position            = Point(320, 92);
		crmThumbnail.size                = Size(350, 350);
		crmThumbnail.layoutUseSpacing    = false;
		crmThumbnail.layout              = Layout::Fixed2d;
		crmThumbnail.layoutOuterMargin   = Margin(0, 20, 0, 14);
		spacer4.position                 = Point(0, 456);
		spacer4.size                     = Size(990, 111);
		spacer4.layout                   = Layout::Fill;
		roadmapReadyLabel.name           = L"roadmapReadyLabel";
		roadmapReadyLabel.size           = Size(990, 111);
		roadmapReadyLabel.font           = Font(L"Philips Healthcare Font", 30, false, false, false, false, Font::Antialias::High);
		roadmapReadyLabel.layoutHideInvisible = false;
		roadmapReadyLabel.layout         = Layout::Fill;
		roadmapReadyLabel.text           = ::Localization::Translation::IDS_RoadmapReady;    //L"Roadmap ready";
		roadmapReadyLabel.textType       = TextType::WordWrap;
		roadmapReadyLabel.color          = Palette::Gray070;
		roadmapReadyLabel.outline        = Palette::Black;
		roadmapReadyLabel.horizontalAlign = HAlign::Center;
		separator.position               = Point(10, 575);
		separator.size                   = Size(970, 10);
		separator.layout                 = Layout::Fixed1d;
		separator.layoutOuterMargin      = Margin(10, 0, 10, 0);
		separator.color2                 = Color::fromRgb(61, 61, 61);
		spacer2.position                 = Point(0, 585);
		spacer2.size                     = Size(990, 407);
		spacer2.layout                   = Layout::Fill;
		panel.position                   = Point(0, 16);
		panel.size                       = Size(990, 120);
		panel.layoutUseSpacing           = false;
		panel.layout                     = Layout::Fixed1d;
		panel.layoutOuterMargin          = Margin(0, 16, 0, 0);
		panel.layoutSpacing              = 12;
		panel.layoutOrientation          = Orientation::Horizontal;
		panel.border                     = Margin(0, 0, 0, 0);
		panel.edge                       = Margin(0, 0, 0, 0);
		panel.color                      = Palette::Transparent * 0.00;
		label.position                   = Point(14, 0);
		label.size                       = Size(32, 120);
		label.layout                     = Layout::Fixed1d;
		label.layoutOuterMargin          = Margin(14, 0, 0, 0);
		label.text                       = L"";
		label.color                      = Palette::Transparent * 0.00;
		label.image                      = Icons::Information;
		guidanceTextLabel.name           = L"guidanceTextLabel";
		guidanceTextLabel.position       = Point(58, 0);
		guidanceTextLabel.size           = Size(922, 120);
		guidanceTextLabel.font           = Font(L"Philips Healthcare Font", 30, true, false, false, false, Font::Antialias::High);
		guidanceTextLabel.layout         = Layout::Fill;
		guidanceTextLabel.layoutOuterMargin = Margin(0, 0, 10, 0);
		guidanceTextLabel.text           = ::Localization::Translation::IDS_RoadmapsCanBeLockerThroughTSM;    //L"Roadmaps can be locked via the touch screen module";
		guidanceTextLabel.textType       = TextType::WordWrap;
		guidanceTextLabel.color          = Palette::Gray070;
		spacer3.position                 = Point(0, 136);
		spacer3.size                     = Size(990, 42);
		spacer3.layoutUseSpacing         = false;
		spacer3.layout                   = Layout::Fill;
		//pictorialListControl.composerClass = Control;
		pictorialListControl.name        = L"pictorialListControl";
		pictorialListControl.position    = Point(10, 178);
		pictorialListControl.size        = Size(970, 229);
		pictorialListControl.layout      = Layout::Fixed1d;
		pictorialListControl.layoutOuterMargin = Margin(10, 0, 10, 0);
		layoutUpdateEnd();
		#pragma endregion
	}

	#pragma region Sense Composer (do not modify)
	private:   Spacer            spacer;
	public:    Label             selectedRoadmapLabel;
	public:    CrmThumbnail      crmThumbnail;
	private:   Spacer            spacer4;
	public:    Label             roadmapReadyLabel;
	private:   Separator         separator;
	private:   Spacer            spacer2;
	private:   Panel             panel;
	private:   Label             label;
	public:    Label             guidanceTextLabel;
	private:   Spacer            spacer3;
	public:    CrmPictorialListControl pictorialListControl;
	#pragma endregion

private:

	CrmRoadmapSelectViewer(const CrmRoadmapSelectViewer&);
	CrmRoadmapSelectViewer& operator=(const CrmRoadmapSelectViewer&);
};
}}}



