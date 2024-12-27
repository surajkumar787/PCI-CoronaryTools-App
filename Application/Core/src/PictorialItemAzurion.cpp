// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PictorialItemAzurion.h"

namespace Pci { namespace Core {

PictorialItemAzurion::PictorialItemAzurion(Sense::Control& parent) : PictorialItem(parent)
{
}

PictorialItemAzurion::~PictorialItemAzurion()
{
}

const Sense::Color PictorialItemAzurion::getColorSelectedGradientFrom() const
{
	return Sense::ExperienceIdentity::Palette::Yellow040;;
}

const Sense::Color PictorialItemAzurion::getColorSelectedGradientTo() const
{
	return Sense::ExperienceIdentity::Palette::Yellow070;
}

const Sense::Color PictorialItemAzurion::getColorSelected() const
{
	return Sense::ExperienceIdentity::Palette::Yellow060;
}

const Sense::Color PictorialItemAzurion::getColorNotSelected() const
{
	return Sense::ExperienceIdentity::Palette::Gray140;
}

}}