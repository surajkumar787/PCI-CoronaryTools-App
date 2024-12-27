// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "PciXrayViewer.h"


namespace Sense { class IRenderer; }

namespace Pci { namespace Core
{
	struct StbMarkers;
	struct StbImage;

	class StbMarkersViewer : public PciXrayViewer
	{
	public:
		StbMarkersViewer(Control &parent, ViewType type);

		void setMarkers(const std::shared_ptr<const StbMarkers> &markers);

		StbMarkersViewer(const StbMarkersViewer&) = delete;
		StbMarkersViewer& operator=(const StbMarkersViewer&) = delete;
	protected:

		void renderOverlay(Sense::IRenderer &renderer) const override;

		std::shared_ptr<const StbMarkers> markers;
	};

}}