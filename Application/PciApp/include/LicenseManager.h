// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <string>
#include "LicensesCollection.h"

namespace Pci
{
	namespace PciApp 
	{
		class LicenseManager
		{
		public:
			explicit LicenseManager(const std::string& licenseFile);
			virtual ~LicenseManager();

			const Core::ILicenseCollection& getLicenses() const;

			LicenseManager(const LicenseManager&) = delete;
			void operator=(const LicenseManager&) = delete;

		private:
			Core::LicenseCollection m_licenses;

			Core::LicenseStatus GetLicenseStatus(Core::LicenseFeature feature) const;
		};
	}
}