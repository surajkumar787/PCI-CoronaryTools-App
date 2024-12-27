// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <memory>

namespace Pci { namespace Core {

struct StbImage;
class PictorialItem;
class StbMarkersViewer;

class StbPictorialItem
{
public:
    StbPictorialItem(PictorialItem& pictorial, const std::wstring& number);
	virtual ~StbPictorialItem();

	void setImage(std::shared_ptr<const StbImage> image);

private:
	PictorialItem& pictorial;
	std::unique_ptr<StbMarkersViewer> stbViewer;
};

}}