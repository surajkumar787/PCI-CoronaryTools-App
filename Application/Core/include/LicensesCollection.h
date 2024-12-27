// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <map>
#include <vector>
#include "ILicenseCollection.h"


namespace Pci{	namespace Core{


	class LicenseCollection : public ILicenseCollection
	{
	public:
		LicenseCollection();
		virtual ~LicenseCollection();

		void add(const LicenseFeature, const LicenseStatus &);
		virtual bool hasAnyProductLicense() const override;
		virtual bool hasActiveLicense() const override;
		virtual bool isActive(const LicenseFeature) const override;
		virtual bool isExpired(const LicenseFeature) const override;

		LicenseStatus get(const LicenseFeature) const;
		virtual bool hasExpiredLicense() const override;
		virtual bool isAnyActive(std::vector<LicenseFeature> features) const override;
		virtual bool hasAnyLicense(std::vector<LicenseFeature> features) const override;

	protected:
		std::map<LicenseFeature, LicenseStatus> m_licenses;
	};
}}

