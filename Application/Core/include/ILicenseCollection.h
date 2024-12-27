// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <map>
#include <vector>
#include "LicenseStatus.h"

namespace Pci
{
	namespace Core
	{

		class ILicenseCollection
		{
		public:
			virtual ~ILicenseCollection() {};

			virtual bool hasAnyProductLicense() const = 0;
			virtual bool hasActiveLicense() const = 0;
			virtual bool isActive(const LicenseFeature) const = 0;
			virtual bool isExpired(const LicenseFeature) const = 0;

			virtual bool hasExpiredLicense() const = 0;
			virtual bool isAnyActive(std::vector<LicenseFeature> features) const = 0;
			virtual bool hasAnyLicense(std::vector<LicenseFeature> features) const = 0;
		};
	}
}

