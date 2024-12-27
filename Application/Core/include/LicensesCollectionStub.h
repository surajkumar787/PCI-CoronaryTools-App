// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <map>
#include <vector>
#include "LicenseStatus.h"
#include "LicensesCollection.h"


namespace Pci
{
	namespace Core
	{
		class LicenseCollectionStub : public LicenseCollection
		{
		public:
			LicenseCollectionStub() 
			{
				add(LicenseFeature::Roadmap, LicenseStatus::Active);
				add(LicenseFeature::Coregistration, LicenseStatus::Active);
				add(LicenseFeature::StentboostLive, LicenseStatus::Active);
				add(LicenseFeature::StbPostDeploy, LicenseStatus::Active);
				add(LicenseFeature::StbSubtract, LicenseStatus::Active);


			};
			virtual ~LicenseCollectionStub() {};

			bool update(LicenseFeature feature, LicenseStatus status)
			{
				auto item = m_licenses.find(feature);
				if (item != m_licenses.end())
				{
					m_licenses[feature] = status;
					return true;
				}

				return false;
			}


		};
	}
}

