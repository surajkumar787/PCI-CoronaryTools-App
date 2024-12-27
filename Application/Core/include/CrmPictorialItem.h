// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include <memory>

namespace CommonPlatform { namespace Xray {
	struct XrayGeometry;
}}

namespace Pci { namespace Core {

class PictorialItem;
class CrmThumbnail;
class CrmRoadmap;

class CrmPictorialItem
{
public:
	CrmPictorialItem(PictorialItem& pictorial, const std::wstring& seriesNumber);
	virtual ~CrmPictorialItem();

	void setRoadmap(std::shared_ptr<const CrmRoadmap> roadmap);
	void setGeometry(const CommonPlatform::Xray::XrayGeometry& geometry);
	void update();

private:
	PictorialItem& pictorial;
	std::shared_ptr<const CrmRoadmap> roadmap;
	std::unique_ptr<CrmThumbnail> thumbnail;
};

}}