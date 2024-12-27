// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "CrmPictorialItem.h"
#include "PictorialItem.h"
#include "CrmThumbnail.h"

namespace Pci { namespace Core {

CrmPictorialItem::CrmPictorialItem(PictorialItem& pictorial, const std::wstring& seriesNumber) :
	pictorial(pictorial),
	roadmap(),
	thumbnail(new CrmThumbnail(pictorial.getContentControl()))
{
	thumbnail->layout = Layout::Fill;
	pictorial.setLabel(seriesNumber);
}

CrmPictorialItem::~CrmPictorialItem()
{
}

void CrmPictorialItem::setRoadmap(std::shared_ptr<const CrmRoadmap> image)
{
	roadmap = image;

	thumbnail->setRoadmap(roadmap);
	thumbnail->roi = Rect(0, 0, pictorial.getContentControl().size->width, pictorial.getContentControl().size->height);
	thumbnail->thumb = true;
	thumbnail->active = true;
}

void CrmPictorialItem::setGeometry(const CommonPlatform::Xray::XrayGeometry& geometry)
{
	thumbnail->setGeometry(geometry);
}

void CrmPictorialItem::update()
{
	if (roadmap->locked) pictorial.lock();
	else			     pictorial.unlock();
}

}}