// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "XrayGeometry.h"
#include "PropertyNotifier.h"

namespace Pci { namespace Core
{
	class PciXrayPositionModel
	{
	public:
		virtual ~PciXrayPositionModel(){};
		PropertyNotifier<CommonPlatform::Xray::XrayGeometry> currentGeometry;
	};
}}