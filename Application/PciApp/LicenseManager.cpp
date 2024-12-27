// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "LicenseManager.h"
#include "LicensesCollection.h"
#include "..\FlexLM\LicenseCheck.h"

using Pci::Core::LicenseStatus;
using Pci::Core::LicenseFeature;

namespace Pci {	namespace PciApp {

	LicenseManager::LicenseManager(const std::string& licenseFile)
	{
		FlexLM::InitializeLicenseCheck(licenseFile.c_str());

		m_licenses.add(LicenseFeature::Roadmap, GetLicenseStatus(LicenseFeature::Roadmap));
		m_licenses.add(LicenseFeature::StentboostLive, GetLicenseStatus(LicenseFeature::StentboostLive));
		m_licenses.add(LicenseFeature::StbSubtract, GetLicenseStatus(LicenseFeature::StbSubtract));
		m_licenses.add(LicenseFeature::LanguagePack, GetLicenseStatus(LicenseFeature::LanguagePack));

		m_licenses.add(LicenseFeature::roadmapCE, GetLicenseStatus(LicenseFeature::roadmapCE));
		m_licenses.add(LicenseFeature::roadmapUKCA, GetLicenseStatus(LicenseFeature::roadmapUKCA));
		m_licenses.add(LicenseFeature::stentBoostLiveCE, GetLicenseStatus(LicenseFeature::stentBoostLiveCE));
		m_licenses.add(LicenseFeature::stentBoostLiveUKCA, GetLicenseStatus(LicenseFeature::stentBoostLiveUKCA));

		if (m_licenses.isActive(LicenseFeature::Roadmap))
		{
			m_licenses.add(LicenseFeature::Coregistration, GetLicenseStatus(LicenseFeature::Coregistration));
		}

		if (m_licenses.isActive(LicenseFeature::StentboostLive) || m_licenses.isActive(LicenseFeature::StbSubtract))
		{
			m_licenses.add(LicenseFeature::StbPostDeploy, LicenseStatus::Active);
		}
		else
		{
			m_licenses.add(LicenseFeature::StbPostDeploy, GetLicenseStatus(LicenseFeature::StbPostDeploy));
		}

		FlexLM::UninitializeLicenseCheck();
	}

	const Core::ILicenseCollection& LicenseManager::getLicenses() const
	{
		return m_licenses;
	}

	LicenseManager::~LicenseManager()
	{
	}

	LicenseStatus LicenseManager::GetLicenseStatus(LicenseFeature feature) const
	{
		return FlexLM::CheckLicenseStatus(feature);
	}

}}
