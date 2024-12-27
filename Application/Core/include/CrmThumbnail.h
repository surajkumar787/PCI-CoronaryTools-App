// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include <Sense.h>
#include <ExperienceIdentity.h>
#include "TextureSimplicity.h"
#include "XrayGeometry.h"
#include "CrmRoadmap.h"


namespace Pci { namespace Core {

class CrmThumbnail : public Sense::Control
{
public:

	explicit CrmThumbnail(Sense::Control &parent);
	virtual ~CrmThumbnail(void);

	void setRoadmap (const std::shared_ptr<const CrmRoadmap> &roadmap);
	void setGeometry(const XrayGeometry &geo);

	Property<Sense::Color> background;
	Property<bool>  active;
	Property<bool>  thumb;
	Property<Rect>  roi;

protected:

	virtual void render(Sense::IRenderer &renderer) const override;
	Point transform(const Sense::Point &pos, const Sense::Matrix &frustumToClient) const;

private:
	XrayGeometry									geometry;
	std::shared_ptr<const CrmRoadmap>				roadmap;
	
	mutable Sense::TextureSimplicity				vessel;
	mutable Sense::TextureInput						back;
	mutable Sense::ExperienceIdentity::Background	border;
}; 

}}

