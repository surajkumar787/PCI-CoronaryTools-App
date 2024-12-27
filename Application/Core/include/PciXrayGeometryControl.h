// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "PciXrayGeometryControl.ui.h"


namespace Pci { namespace Core
{
enum class ViewType;

class PciXrayGeometryControl : public UI::PciXrayGeometryControlUi
{
public:
	PciXrayGeometryControl(Sense::Control& parent, ViewType viewType);

	Sense::Property<Sense::Color> color;
	Sense::Property<Sense::Color> outline;

	Sense::Property<double> rotation;
	Sense::Property<double> angulation;

private:
	void updateRotation();
	void updateAngulation();
};

}}