// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciXrayViewer.h"

#include <Sense/Common/Font.h>
#include <Sense/Common/Point.h>
#include <ExperienceIdentity/Resource/Palette.h>
#include <assert.h>
#include "Translation.h"
#include "XrayImage.h"
#include "Resources.h"
#include "AlluraColors.h"
#include "ViewType.h"
#include "PciXrayModel.h"

namespace Pci { namespace Core
{
	using Sense::Font;
	using Sense::Rect;
	using Sense::Point;
	using Sense::HAlign;
	using Sense::ExperienceIdentity::Palette;

	PciXrayViewer::PciXrayViewer(Sense::Control& parent, ViewType type, bool includeShutters, Sense::VAlign imgAlign ) :
		PciXrayPositionViewer(parent, type),
		xray(nullptr),
		model(nullptr),
		includeShutters(includeShutters),
		imageAlignment(imgAlign),
		textureXray(gpu),
		lastImageHoldTexture(gpu)
	{
	}

	void PciXrayViewer::setImage(const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& image)
	{
		xray = image;
		textureXray.invalidate();

		if ((image != nullptr) && (model != nullptr) && image->isValid())
		{
			model->currentGeometry = image->geometry;
		}
		invalidate();
	}

	const std::shared_ptr<const CommonPlatform::Xray::XrayImage>& PciXrayViewer::getImage() const
	{
		return xray;
	}

	void PciXrayViewer::setXrayModel(PciXrayModel& newModel)
	{
		setModel(newModel);
		model = &newModel;
		model->showLastImageHold.setNotifier([this](bool) { invalidate(); });
	}

	void PciXrayViewer::render(Sense::IRenderer &renderer) const
	{
		if (xray != nullptr)
		{
			renderXray(renderer);
			renderOverlay(renderer);
			renderLastImageHold(renderer);
		}
		PciXrayPositionViewer::render(renderer);
	}

	void PciXrayViewer::renderXray(Sense::IRenderer& renderer) const
	{
		auto space = getClientSpace();
		auto matrix = pixelsToClient();
		const auto& rect = includeShutters ? xray->validRect : xray->shutters;
		textureXray.render(space, renderer, *xray, Sense::Color(4.0), rect, matrix);
	}

	void PciXrayViewer::renderLastImageHold(Sense::IRenderer& renderer) const
	{
		// todo position of icon imagehold

		if ((model != nullptr) && model->showLastImageHold)
		{
			if (!lastImageHoldTexture.isValid())
			{
				if( viewType == ViewType::Allura)
				{

					lastImageHoldTexture.set(PciSuite::Resources::indicator_lih.data());
				}
				else
				{
					lastImageHoldTexture.set(PciSuite::Resources::ImageIndicatorLih.data());
				}

			}
			assert(lastImageHoldTexture.isValid());
			auto width = lastImageHoldTexture.getWidth();
			auto height = lastImageHoldTexture.getHeight();
			int offSetFromTop = viewType == ViewType::Allura ? 40 : 106;
			int offseetFromRight = viewType == ViewType::Allura? width: width + 20;
			renderer.render(getClientSpace(), Rect(0, 0, width, height) + getClientSpace().bounds.topRight() + Point(-offseetFromRight, offSetFromTop),
							Sense::Color(1.0), Rect(0, 0, width, height), lastImageHoldTexture);
		}
	}

	Sense::Matrix PciXrayViewer::pixelsToClient() const
	{
		using Sense::Matrix;

		if (xray == nullptr)			return Matrix();
		if (xray->pointer == nullptr)	return Matrix();

		auto dst = getClientSpace().bounds;
		auto src = xray->validRect;
		if (src.empty()) src = dst;

		auto result = Matrix::identity();

		result = Matrix::translate(-src.center())												* result;
		result = Matrix::scale(std::min(dst.width()/src.width(), dst.height()/src.height()))	* result;
		result = Matrix::translate(dst.center())												* result;

		if ((imageAlignment == Sense::VAlign::Bottom) || (imageAlignment == Sense::VAlign::Top))
		{
			auto imageTopLeft = result * xray->validRect.topLeft();
			auto imageBottomLeft = result * xray->validRect.bottomLeft();
			auto imageHeight = imageBottomLeft.y - imageTopLeft.y;

			auto verticalOffset = (dst.height() / 2 - imageHeight / 2);
			if (imageAlignment == Sense::VAlign::Top)
			{
				verticalOffset = -verticalOffset;
			}

			result = Sense::Matrix::translate(Point(0.0, verticalOffset)) * result;
		}

		return result;
	}
}}