// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "StbViewer.h"
#include "StbMask.h"

namespace Pci { namespace Core
{
	struct StbImage;
	struct StbMarkers;

	class StbBoostViewer : public StbViewer
	{
	public:
		StbBoostViewer(Control &parent, ViewType type);

		void setImage(const std::shared_ptr<const StbImage> &boost);

		Sense::Property<bool> showUpdateDelayed;

		StbBoostViewer(const StbBoostViewer&) = delete;
		StbBoostViewer& operator=(const StbBoostViewer&) = delete;

	protected:
		void renderOverlay(Sense::IRenderer &renderer) const override;

	private:
		bool _hasMarkers;
		mutable uint32_t fadeCounter;

		StbMask m_stbMask;
	};

}}