// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "PciBaseViewer.h"

#include "ViewType.h"

namespace Pci { namespace Core
{
	PciBaseViewer::PciBaseViewer(Control& parent, ViewType type) :
		Sense::Control(parent),
		viewType(type)
	{
	}
}}