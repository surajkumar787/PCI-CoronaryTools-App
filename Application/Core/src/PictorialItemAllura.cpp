// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PictorialItemAllura.h"

namespace Pci { namespace Core {

PictorialItemAllura::PictorialItemAllura(Sense::Control& parent) : PictorialItem(parent)
{
	pictorialLabel.font = Sense::Font(L"Arial", 18, true, false, false, false, Font::Antialias::High);
}

PictorialItemAllura::~PictorialItemAllura()
{
}

const Sense::Color PictorialItemAllura::getColorSelectedGradientFrom() const
{
	return Sense::ExperienceIdentity::Palette::White;;
}

const Sense::Color PictorialItemAllura::getColorSelectedGradientTo() const
{
	return Sense::ExperienceIdentity::Palette::White;
}

const Sense::Color PictorialItemAllura::getColorSelected() const
{
	return Sense::ExperienceIdentity::Palette::White;
}

const Sense::Color PictorialItemAllura::getColorNotSelected() const
{
	return Sense::Color::fromHex(0x4E4D4B);
}

}}