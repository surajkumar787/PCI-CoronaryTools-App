/*----------------------------------------------------
 |   This file was generated by Sense Composer 2.0   |
 |  Do not modify the Sense Composer Generated Code  |
 ----------------------------------------------------*/
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>

#include "countdowncontrol.h"
#include "arrowcontrol.h"

using namespace Sense;
using namespace Sense::ExperienceIdentity;

namespace Pci { namespace Core { namespace UI
{

class AlluraTopGuidanceStbContrast : public Control
{
public:

	AlluraTopGuidanceStbContrast(Control &parent)
	:
		Control(parent)
		#pragma region Sense Composer (do not modify)
	,	control          ((Control&)*this)
	,	spacer           (control)
	,	spacer2          (control)
	,	stentboostProgress(spacer2)
	,	ArrowControl     (spacer2)
	,	contrastProgress (spacer2)
	,	spacer3          (control)
		#pragma endregion
	{
		#pragma region Sense Composer (do not modify)
		layoutUpdateBegin();
		size                             = Size(1000, 696);
		control.size                     = Size(1000, 696);
		control.layout                   = Layout::Fill;
		spacer.size                      = Size(1000, 190);
		spacer.layout                    = Layout::Fixed1d;
		spacer2.position                 = Point(173, 198);
		spacer2.size                     = Size(655, 304);
		spacer2.layout                   = Layout::Fixed2d;
		spacer2.layoutSpacing            = 0;
		spacer2.layoutOrientation        = Orientation::Horizontal;
		//stentboostProgress.composerClass = Control;
		stentboostProgress.name          = L"stentboostProgress";
		stentboostProgress.size          = Size(296, 304);
		stentboostProgress.layout        = Layout::Fill;
		stentboostProgress.layoutOrientation = Orientation::Horizontal;
		//ArrowControl.composerClass     = Control;
		ArrowControl.name                = L"ArrowControl";
		ArrowControl.position            = Point(296, 0);
		ArrowControl.size                = Size(64, 224);
		ArrowControl.layout              = Layout::Fixed1d;
		ArrowControl.layoutOuterMargin   = Margin(0, 0, 0, 80);
		//contrastProgress.composerClass = Control;
		contrastProgress.name            = L"contrastProgress";
		contrastProgress.position        = Point(360, 0);
		contrastProgress.size            = Size(295, 304);
		contrastProgress.layout          = Layout::Fill;
		contrastProgress.layoutOrientation = Orientation::Horizontal;
		spacer3.position                 = Point(0, 510);
		spacer3.size                     = Size(1000, 186);
		spacer3.layout                   = Layout::Fill;
		layoutUpdateEnd();
		#pragma endregion
	}

	#pragma region Sense Composer (do not modify)
	private:   Control           control;
	private:   Spacer            spacer;
	private:   Spacer            spacer2;
	public:    CountDownControl  stentboostProgress;
	public:    ArrowControl      ArrowControl;
	public:    CountDownControl  contrastProgress;
	private:   Spacer            spacer3;
	#pragma endregion

private:

	AlluraTopGuidanceStbContrast(const AlluraTopGuidanceStbContrast&);
	AlluraTopGuidanceStbContrast& operator=(const AlluraTopGuidanceStbContrast&);
};
}}}

