/*----------------------------------------------------
 |   This file was generated by Sense Composer 2.0   |
 |  Do not modify the Sense Composer Generated Code  |
 ----------------------------------------------------*/
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>

#include "CrmThumbnail.h"
#include "PciThumbnail.h"

using namespace Sense;
using namespace Sense::ExperienceIdentity;

namespace Pci { namespace Core { namespace UI
{

class AlluraTopGuidanceStandard : public Control
{
public:

	AlluraTopGuidanceStandard(Control &parent)
	:
		Control(parent)
		#pragma region Sense Composer (do not modify)
	,	processing       (*this)
	,	imageRoadmap     (*this)
	,	labelHeader      (*this)
	,	positionTarget   (*this, ViewType::Allura)
	,	imageHeader      (*this)
		#pragma endregion
	{
		#pragma region Sense Composer (do not modify)
		layoutUpdateBegin();
		size                             = Size(1000, 696);
		processing.name                  = L"processing";
		processing.position              = Point(360, 212);
		processing.size                  = Size(280, 280);
		processing.determinate           = false;
		//imageRoadmap.composerClass     = Control;
		imageRoadmap.name                = L"imageRoadmap";
		imageRoadmap.position            = Point(360, 212);
		imageRoadmap.size                = Size(280, 280);
		labelHeader.name                 = L"labelHeader";
		labelHeader.position             = Point(0, 500);
		labelHeader.size                 = Size(1000, 120);
		labelHeader.font                 = Font(L"Philips Healthcare Font", 29, true, false, false, false, Font::Antialias::High);
		labelHeader.text                 = L"Roadmap ready";
		labelHeader.textType             = TextType::WordWrap;
		labelHeader.horizontalAlign      = HAlign::Center;
		//positionTarget.composerClass   = Control;
		positionTarget.name              = L"positionTarget";
		positionTarget.position          = Point(424, 540);
		positionTarget.size              = Size(152, 64);
		positionTarget.font              = Font(L"Philips Healthcare Font", 24, false, false, false, false, Font::Antialias::High);
		//imageHeader.composerClass      = Control;
		imageHeader.name                 = L"imageHeader";
		imageHeader.position             = Point(360, 212);
		imageHeader.size                 = Size(280, 280);
		imageHeader.layoutUseSpacing     = false;
		layoutUpdateEnd();
		#pragma endregion
	}

	#pragma region Sense Composer (do not modify)
	public:    ProgressCircle    processing;
	public:    CrmThumbnail      imageRoadmap;
	public:    Label             labelHeader;
	public:    PciXrayGeometryControl positionTarget;
	public:    PciThumbnail      imageHeader;
	#pragma endregion

private:

	AlluraTopGuidanceStandard(const AlluraTopGuidanceStandard&);
	AlluraTopGuidanceStandard& operator=(const AlluraTopGuidanceStandard&);
};

}}}

