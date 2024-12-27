/*-----------------------------------------------------------------------------
 | Copyright: Copyright(c) 2015, Philips Medical Systems Nederland B.V.       |
 | Author:    I.W.R.A. Limpens                                                |
 -----------------------------------------------------------------------------*/

#include "patientbar.h"

#include "Resources.h"
#include "Translation.h"

namespace Pci { namespace Core
{

const double PatientBarCollapsedSize = 36.0;
const Size PatientBarSize = Size(684.0, PatientBarCollapsedSize);
const Size PatientBarExpanderSize = Size(684.0, 525);

const double iconSizePatientInAcquisition = 28.0;
const double iconSizeGeneral = 24.0;

const double marginBetweenLabelValue = 8;
const double marginMinimumBetweenFields = 12;


PatientBarHeader::PatientBarHeader(Control &parent, PatientBarColorScheme scheme) :
	Control(parent),
	patientData			(PatientInfo(), [this] { onPatientDataChanged(); }),
	patientName		    (*this),
	patientIdLabel	    (gpu),
	patientId		    (*this),
	patientDobLabel	    (gpu),
	patientDob		    (*this)
{
	patientName.image = (scheme == PatientBarColorScheme::SmS ? PciSuite::Resources::PatientAcquisitionIxr : Icons::PatientAcquisition);
	
	Font f = Font(L"Philips Healthcare Font", 15, false, false, false, false, Font::Antialias::High);

	patientName.imageSize			= iconSizePatientInAcquisition;	
	patientName.textType			= TextType::SingleLineEllipsis;
	patientName.enabled				= isEnabled();
	patientName.horizontalAlign		= HAlign::Left;
	patientName.verticalAlign		= VAlign::Middle;
	patientName.imagePosition		= ImagePosition::Left;
	patientName.nonclient			= true;

	patientDob.horizontalAlign		= HAlign::Left;
	patientDob.verticalAlign		= VAlign::Middle;
	patientDob.nonclient			= true;

	patientDobLabel.font			= f;
	patientDobLabel.text			= Localization::Translation::IDS_PatientBarDob;
	patientDobLabel.horizontalAlign	= HAlign::Center;
	patientDobLabel.verticalAlign	= VAlign::Middle;

	patientId.horizontalAlign		= HAlign::Left;
	patientId.verticalAlign			= VAlign::Middle;
	patientId.textType				= TextType::SingleLineEllipsis;
	patientId.nonclient				= true;

	patientIdLabel.font				= f;
	patientIdLabel.text				= Localization::Translation::IDS_PatientBarId;
	patientIdLabel.horizontalAlign	= HAlign::Center;
	patientIdLabel.verticalAlign	= VAlign::Middle;

	size = PatientBarSize;

	applyColorScheme(scheme);
}

void PatientBarHeader::render( IRenderer &renderer ) const 
{
	Space space = getSpace();
	auto bounds = space.bounds;

	Control::render(renderer);

	patientIdLabel	.render(space, renderer);
	patientDobLabel	.render(space, renderer);
}

void PatientBarHeader::onResize()
{
	auto bounds = getSpace().bounds;

	auto xpos = bounds.right - marginMinimumBetweenFields; // most right
	xpos -= iconSizeGeneral;

	arrangePatientInfo();
}

void PatientBarHeader::arrangePatientInfo() const
{
	auto space = getSpace();
	auto bounds = space.bounds;
	bounds -= Margin(marginBetweenLabelValue, 0.0, marginBetweenLabelValue, 0.0);
	Text text(gpu);
	text.font = getFont();

	const double margin = 10.0;
	text.text				 = patientName.text;
	auto patientNameWidth	 = text.measureSize(space).width + patientName.imageSize->width + patientName.imageSpacing + margin;
	text.text				 = patientId.text;
	auto patientIdWidth		 = text.measureSize(space).width;
	text.text				 = patientIdLabel.text; 
	auto patientIdLabelWidth = text.measureSize(space).width;
	text.text				 = patientDob.text;
	auto patientDobWidth	 = text.measureSize(space).width + margin;
	text.text				 = patientDobLabel.text;
	auto patientDobLabelWidth= text.measureSize(space).width;
	

	auto totalNameWidth = patientNameWidth;
	auto totalIdWidth = marginMinimumBetweenFields + patientIdLabelWidth + marginBetweenLabelValue + patientIdWidth + marginMinimumBetweenFields;
	
	auto totalDobWidth = patientDobLabelWidth + marginBetweenLabelValue + patientDobWidth;

	// most right position
	auto rpos = bounds.right - marginMinimumBetweenFields;

	// most left position
	auto lpos = bounds.left; // most left

	auto totalWidth = rpos - lpos;
	auto availableCenter = totalWidth - totalNameWidth - totalDobWidth;
	if (availableCenter < totalIdWidth)
	{	
		// Id does not fit, truncate as much as possible
		const double minimumIdValueWidth = 100;
		auto minimumIdWidth = marginMinimumBetweenFields + patientIdLabelWidth + marginBetweenLabelValue + minimumIdValueWidth + marginMinimumBetweenFields;
		totalIdWidth = availableCenter < minimumIdWidth ? minimumIdWidth : availableCenter;
	}
	auto availableLeft = totalWidth - totalIdWidth - totalDobWidth;
	if (availableLeft < totalNameWidth)
	{
		// Name does not fit, truncate 
		totalNameWidth = availableLeft;
	}
	// center id between name and dob
	auto cpos = totalNameWidth + (totalWidth - totalNameWidth - totalDobWidth - totalIdWidth) / 2;

	// DOB (align right)
	rpos -= patientDobWidth;
	patientDob.setBounds(Rect(rpos, bounds.top, rpos + patientDobWidth, bounds.bottom));
	rpos -= marginBetweenLabelValue;
	rpos -= patientDobLabelWidth;
	patientDobLabel.bounds = Rect(rpos, bounds.top, rpos + patientDobLabelWidth, bounds.bottom);
	rpos -= marginMinimumBetweenFields;

	// PatientName (align left)
	patientName.setBounds(Rect(lpos, bounds.top, cpos, bounds.bottom));

	// ID (centered between patientname and dob)
	cpos += marginMinimumBetweenFields;
	patientIdLabel.bounds = Rect(cpos, bounds.top, cpos + patientIdLabelWidth, bounds.bottom);
	cpos += patientIdLabelWidth;
	cpos += marginBetweenLabelValue;
	patientId.setBounds(Rect(cpos, bounds.top, rpos, bounds.bottom));
}

void PatientBarHeader::onPatientDataChanged()
{
	patientName.text = patientData->name;
	patientId.text = patientData->id;
	patientDob.text = patientData->birthdate;

	arrangePatientInfo();
	invalidate();
}

void PatientBarHeader::applyColorScheme(PatientBarColorScheme scheme)
{
	// ColorScheme yellow
	const Color PatientBarLabelBright = Palette::Gray130;
	const Color PatientBarTextBright = Palette::Black;

	// ColorScheme Dark
	const Color PatientBarLabelDark = Palette::Gray060;
	const Color PatientBarTextDark = Palette::Yellow060;

	switch (scheme)
	{
	case PatientBarColorScheme::Bright:
		patientName.color				= PatientBarTextBright;
		patientDob.color				= PatientBarTextBright;
		patientDobLabel.color			= PatientBarLabelBright;
		patientId.color					= PatientBarTextBright;
		patientIdLabel.color			= PatientBarLabelBright;
		break;
	case PatientBarColorScheme::Dark:
		patientName.color				= PatientBarTextDark;
		patientDob.color				= PatientBarTextDark;
		patientDobLabel.color			= PatientBarLabelDark;
		patientId.color					= PatientBarTextDark;
		patientIdLabel.color			= PatientBarLabelDark;
		break;
	case PatientBarColorScheme::SmS:
		patientName.color				= PatientBarTextBright;
		patientDob.color				= PatientBarTextBright;
		patientDobLabel.color			= PatientBarLabelBright;
		patientId.color					= PatientBarTextBright;
		patientIdLabel.color			= PatientBarLabelBright;
		break;
	}
}


PatientBar::PatientBar( Control &parent , PatientBarColorScheme scheme ) :
	PatientBarHeader(parent,scheme),
	radius				(Corners(0.0, 0.0, 4.0, 4.0), [this] { invalidate(); }),
	border				(Margin(1.0),				  [this] { invalidate(); invalidateSpace(); }),
	edge				(Margin(0,1,0,0),		      [this] { invalidate(); invalidateSpace(); }),
	shadow				(Margin(),					  [this] { invalidate(); invalidateSpace(); }),
	background(gpu),
	dimmedBackground(gpu)
{
}

void PatientBar::render( IRenderer &renderer ) const 
{
	Space space = getSpace();
	auto bounds = space.bounds;

	if (isInvalidated())
	{
		background.bounds		= space.bounds;
		background.radius		= radius;
		background.border		= border;
		background.edge			= edge;

		background.color		= Color::fromRgb(184,167,124);
		background.colorBorder	= Palette::Black;
		background.colorEdge	= Palette::Yellow180;

		background.shadow       = shadow;
		background.colorShadow  = Palette::Black * 0.5;

		dimmedBackground.bounds	= space.bounds;
		dimmedBackground.radius	= Corners(0.0, 0.0, 4.0, 4.0);
		dimmedBackground.color	= Palette::Black * 0.4;
	}

	background.render(space, renderer);

	PatientBarHeader::render(renderer);

	if (!isEnabled()) dimmedBackground.render(space, renderer);
}

}}

