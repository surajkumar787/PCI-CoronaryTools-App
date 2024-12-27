// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "Sense/Controls/Control.h"

namespace Pci { namespace Core
{
	enum class ViewType;

	class PciBaseViewer : public Sense::Control
	{
	public:
		PciBaseViewer(Control& parent, ViewType type);

		ViewType getViewType() const { return viewType; }

	protected:
		ViewType viewType;
	};
}}