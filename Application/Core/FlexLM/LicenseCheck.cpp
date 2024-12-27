// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "LicenseCheck.h"

#include "LicenseStatus.h"
#include "lmclient.h"
#include "lm_attr.h"
#include <stdlib.h>
#include <string>

namespace Pci { namespace FlexLM {

static VENDORCODE m_vendorCode;
static LM_HANDLE_PTR m_job;

using Pci::Core::LicenseFeature;
using Pci::Core::LicenseStatus;

static const std::string roadmapFeature = "DynamicCoronaryRoadmap";
static const std::string  stentBoostLiveFeature = "StentBoostLive";
static const std::string  stbPostDeployFeature = "StentBoostPostDeployment";
static const std::string  stentBoostSubtractFeature = "StentBoostSubtract";
static const std::string  coregFeature = "FFRiFRroadmap";
static const std::string  LanguagePackFeature = "LanguagePack";

static const std::string  roadmapCE = "Label_DynCorRoadmapCE";
static const std::string  roadmapUKCA = "Label_DynCorRoadmapUKCA";
static const std::string  stentBoostLiveCE = "Label_StentBoostLiveCE";
static const std::string  stentBoostLiveUKCA = "Label_StentBoostLiveUKCA";

void  FeatureToString(LicenseFeature feature, char buffer[], rsize_t size)
{

	if (feature == LicenseFeature::Roadmap)
	{
		strcpy_s(buffer, size, roadmapFeature.c_str());
	}
	else if (feature == LicenseFeature::StentboostLive)
	{
		strcpy_s(buffer, size, stentBoostLiveFeature.c_str());
	}
	else if (feature == LicenseFeature::StbPostDeploy)
	{
		strcpy_s(buffer, size, stbPostDeployFeature.c_str());
	}
	else if (feature == LicenseFeature::StbSubtract)
	{
		strcpy_s(buffer, size, stentBoostSubtractFeature.c_str());
	}
	else if (feature == LicenseFeature::Coregistration)
	{
		strcpy_s(buffer, size, coregFeature.c_str());
	}
	else if (feature == LicenseFeature::LanguagePack)
	{
		strcpy_s(buffer, size, LanguagePackFeature.c_str());
	}
	else if (feature == LicenseFeature::roadmapCE)
	{
		strcpy_s(buffer, size, roadmapCE.c_str());
	}
	else if (feature == LicenseFeature::roadmapUKCA)
	{
		strcpy_s(buffer, size, roadmapUKCA.c_str());
	}
	else if (feature == LicenseFeature::stentBoostLiveCE)
	{
		strcpy_s(buffer, size, stentBoostLiveCE.c_str());
	}
	else if (feature == LicenseFeature::stentBoostLiveUKCA)
	{
		strcpy_s(buffer, size, stentBoostLiveUKCA.c_str());
	}
	else
	{
		strcpy_s(buffer, size, "");
	}
}

void InitializeLicenseCheck(const char * licenseFile)
{
	lc_new_job(nullptr, lc_new_job_arg2, &m_vendorCode, &m_job);

	lc_set_attr(m_job, LM_A_PERROR_MSGBOX, reinterpret_cast<LM_A_VAL_TYPE>(false));
	lc_set_attr(m_job, LM_A_APP_DISABLE_CACHE_READ, reinterpret_cast<LM_A_VAL_TYPE>(1));
	lc_set_attr(m_job, LM_A_PROMPT_FOR_FILE, reinterpret_cast<LM_A_VAL_TYPE>(false));
	lc_set_attr(m_job, LM_A_LICENSE_DEFAULT, const_cast<LM_A_VAL_TYPE>(licenseFile));
}

LicenseStatus CheckLicenseStatus(LicenseFeature feature)
{
	char featureStr[64];
	FeatureToString(feature, featureStr, _countof(featureStr));
	auto res = lc_checkout(m_job, featureStr, "", 1, 0, &m_vendorCode, LM_DUP_NONE);
	if (res == 0)
	{
		// lc_checkout can return 0 when the license is expired, this needs to be double checked
		auto conf = lc_auth_data(m_job, featureStr);
		auto days = lc_expire_days(m_job, conf);
		if (days < 0) return LicenseStatus::Expired;

		return LicenseStatus::Active;
	}
	else if (res == LM_LONGGONE) // license is expired
	{
		return LicenseStatus::Expired;
	}
	return LicenseStatus::None;
}

void UninitializeLicenseCheck()
{
	lc_free_job(m_job);
}

}}