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

class SmartSuiteLicenseAgreementDialog : public Dialog
{
public:

	SmartSuiteLicenseAgreementDialog(Control &parent)
	:
		Dialog   (parent),
		eulaText (*this)
	{
		layoutUpdateBegin();

		size                     = Size(506, 404);
		layoutOuterMargin        = Margin(5, 5, 5, 5);
		layoutSpacing            = 0;
		text                     = L"License Agreement";

		eulaText.position        = Point(5, 5);
		eulaText.size            = Size(490, 350);
		eulaText.font            = Font(L"Philips Healthcare Font", 12, false, false, false, false, Font::Antialias::High);
		eulaText.layout          = Layout::Fill;
		eulaText.textType        = TextType::WordWrap;
		eulaText.horizontalAlign = HAlign::Center;

		layoutUpdateEnd();
	}

public:

	Label eulaText;

private:

	SmartSuiteLicenseAgreementDialog(const SmartSuiteLicenseAgreementDialog&);
	SmartSuiteLicenseAgreementDialog& operator=(const SmartSuiteLicenseAgreementDialog&);
};

}}