// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "LicensesCollection.h"
#include "LicenseStatus.h"
#include <assert.h>

namespace Pci {	namespace Core {


	LicenseCollection::LicenseCollection() :
		m_licenses()
	{

	}

	LicenseCollection::~LicenseCollection()
	{
	}

	void LicenseCollection::add(const LicenseFeature licenseFeature, const LicenseStatus &status)
	{
		assert(m_licenses.find(licenseFeature) == m_licenses.end());

		m_licenses[licenseFeature] = status;
	}

	bool LicenseCollection::hasAnyProductLicense() const
	{
		bool hasProductLicense(false);
		for (auto& license : m_licenses)
		{
			if (license.first != LicenseFeature::LanguagePack)
			{
				hasProductLicense |= (license.second != LicenseStatus::None);
			}
		}

		return hasProductLicense;
	}

	bool LicenseCollection::hasActiveLicense() const
	{
		for (auto license : m_licenses)
		{
			if (license.second == LicenseStatus::Active)
			{
				return true;
			}
		}
		return false;
	}

	bool LicenseCollection::isActive(const LicenseFeature licenseFeature) const
	{
		return get(licenseFeature) == LicenseStatus::Active;
	}

	bool LicenseCollection::isExpired(const LicenseFeature licenseFeature) const
	{
		return get(licenseFeature) == LicenseStatus::Expired;
	}

	LicenseStatus LicenseCollection::get(const LicenseFeature licenseFeature) const
	{
		auto license = m_licenses.find(licenseFeature);

		return (license != m_licenses.end()) ? license->second : LicenseStatus::None;
	}

	bool LicenseCollection::hasExpiredLicense() const
	{
		for (auto license : m_licenses)
		{
			if (license.second == LicenseStatus::Expired)
			{
				return true;
			}
		}
		return false;
	}

	bool LicenseCollection::isAnyActive(std::vector<LicenseFeature> features) const
	{
		for (auto feature : features)
		{
			if (isActive(feature))
			{
				return true;
			}
		}
		return false;
	}

	bool LicenseCollection::hasAnyLicense(std::vector<LicenseFeature> features) const
	{
		for (auto feature : features)
		{
			auto license = get(feature);
			if (license != LicenseStatus::None)
			{
				return true;
			}
		}
		return false;
	}

}
}