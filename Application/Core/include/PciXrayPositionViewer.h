// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "PciBaseViewer.h"

#include "PciXrayGeometryControl.h"

namespace CommonPlatform{ namespace Xray
{
	struct XrayGeometry;
}}

namespace Pci { namespace Core
{
	class PciXrayPositionModel;

	class PciXrayPositionViewer : public PciBaseViewer
	{
	public:
		PciXrayPositionViewer(Control& parent, ViewType type);

		void setModel(PciXrayPositionModel& model);
		Sense::Property<bool> showXrayPosition;
	protected:
		virtual void updateXrayGeometry(const CommonPlatform::Xray::XrayGeometry& geo);
		virtual void onResize() override;

	private:
		PciXrayPositionModel* model;
		PciXrayGeometryControl currentXrayPosition;

	};
}}