// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Translation.h"
#include "StbImageSelectViewer.h"
#include "StbPictorialItem.h"

#include "AlluraColors.h"

namespace Pci { namespace Core
{

StbImageSelectViewer::StbImageSelectViewer(Control& parent, ViewType type, bool showRange) : UI::StbImageSelectViewer(parent, type),
	contrastImages(),
	mask()
{
    if (showRange)
    {
        pictorialListControl.showRangeSelection();
    }

	if (type == ViewType::Allura)
	{
		guidanceTextLabel.color = UI::ColorAlluraLightGreyFont;
		guidanceTextLabel.font = Sense::Font(L"Arial", 34, true, false, false, false, Font::Antialias::High);

		selectedImageLabel.color = UI::ColorAlluraLightGreyFont;
		guidanceTextLabel.font = Sense::Font(L"Arial", 34, true, false, false, false, Font::Antialias::High);
	}
}

StbImageSelectViewer::~StbImageSelectViewer()
{
}

void StbImageSelectViewer::setSelection(int index)
{
	auto image = std::find_if(contrastImages.begin(), contrastImages.end(), [index](const std::shared_ptr<const StbImage> &image) {return image->imageNumber == index; });
	if (image != contrastImages.end())
	{
		pictorialListControl.selectItem( static_cast<int>(std::distance(contrastImages.begin(), image)));
		imageViewer.setImage(*image);
		selectedImageLabel.text = Localization::Translation::IDS_Image + L" " + std::to_wstring((*image)->imageNumber);
	}
}

void StbImageSelectViewer::setFrames(std::vector<std::shared_ptr<const StbImage>>& images)
{
	contrastImages.clear();
	for (const auto& image : images)
	{
		auto maskedImage = std::make_shared<StbImage>(*image);
		mask.mask(image->refMarkers, *image, *maskedImage);

		contrastImages.push_back(maskedImage);
	}

	pictorialListControl.createObjects(contrastImages);
}

}}