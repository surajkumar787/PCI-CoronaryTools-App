// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>

namespace Pci { namespace Core{


class PciThumbnail : public Sense::Control
{
public:
	PciThumbnail(Sense::Control &parent);
	~PciThumbnail();

	Sense::Property<Sense::Image>   image;
	
	Sense::Property<Sense::Image>	blink;
	Sense::Property<double>			speed;
	
	Sense::Property<Sense::Color>	border;
	Sense::Property<double>			tickness;
	Sense::Property<double>			radius;

	PciThumbnail(const PciThumbnail&) = delete;
	PciThumbnail &operator=(const PciThumbnail&) = delete;

protected:

	void onVisibleChanged() override;
	void render(Sense::IRenderer &renderer) const override;
	
	Sense::Timer				blinkTimer;
	mutable Sense::TextureInput	imageTexture;
	mutable bool				imageValid;
	
	mutable Sense::TextureInput	blinkTexture;
	mutable bool				blinkValid;
	

};


}}