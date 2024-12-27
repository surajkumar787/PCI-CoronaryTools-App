// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "StbPictorialItem.h"
#include "PictorialItem.h"
#include "StbImage.h"

namespace Pci { namespace Core {

StbPictorialItem::StbPictorialItem(PictorialItem& pictorial, const std::wstring& number) :
	pictorial(pictorial),
	stbViewer(new StbMarkersViewer(pictorial.getContentControl(), ViewType::Allura))
{
	stbViewer->layout = Layout::Fill;
	pictorial.setLabel(number);
}

    StbPictorialItem::~StbPictorialItem()
{
}

void StbPictorialItem::setImage(std::shared_ptr<const StbImage> image)
{
	stbViewer->setImage(image);
}

}}