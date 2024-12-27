// Copyright Koninklijke Philips N.V. 2019
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>

#include "LicensesCollection.h"
#include "LicenseStatus.h"

using namespace Pci::Core;

namespace Test {

	class TestLicensesCollection : public ::testing::Test
	{
	public:
		TestLicensesCollection():
			m_collection()
		{

		}

		void SetUp() override
		{
			m_collection = std::make_unique<Pci::Core::LicenseCollection>();
		}

		protected:
		std::unique_ptr<LicenseCollection> m_collection;

	};

	TEST_F(TestLicensesCollection, WhenOneActiveLicense_thenHasAciveLicenseReturnsTrue)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::Active);
		m_collection->add(Pci::Core::LicenseFeature::StbPostDeploy, Pci::Core::LicenseStatus::None);

		ASSERT_TRUE(m_collection->hasActiveLicense());
	}

	TEST_F(TestLicensesCollection, WhenNoActiveLicense_thenHasAciveLicenseReturnsFalse)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::Expired);
		m_collection->add(Pci::Core::LicenseFeature::StbPostDeploy, Pci::Core::LicenseStatus::None);

		ASSERT_FALSE(m_collection->hasActiveLicense());
	}

	TEST_F(TestLicensesCollection, WhenActiveLicense_thenIsAciveReturnsTrue)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::Active);

		ASSERT_TRUE(m_collection->isActive(Pci::Core::LicenseFeature::StbSubtract));
	}


	TEST_F(TestLicensesCollection, WhenNoActiveLicense_thenIsAciveReturnsFalse)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::None);

		ASSERT_FALSE(m_collection->isActive(Pci::Core::LicenseFeature::StbSubtract));
	}

	TEST_F(TestLicensesCollection, WhenExpiredLicense_thenIsExpiredReturnsTrue)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::Expired);

		ASSERT_TRUE(m_collection->isExpired(Pci::Core::LicenseFeature::StbSubtract));
	}


	TEST_F(TestLicensesCollection, WhenActiveLicense_thenIsExpiredReturnsFalse)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::Active);

		ASSERT_FALSE(m_collection->isExpired(Pci::Core::LicenseFeature::StbSubtract));
	}

#ifdef _DEBUG
	TEST_F(TestLicensesCollection, WhenAddingTwiceTheSameLicense_thenAnAssertIsReased)
	{
		m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::Active);
		ASSERT_DEATH(m_collection->add(Pci::Core::LicenseFeature::StbSubtract, Pci::Core::LicenseStatus::None),"");
	}
#endif
	TEST_F(TestLicensesCollection, WhenNoLicenseAdded_thenGetreturnsNone)
	{
		ASSERT_TRUE(m_collection->get(Pci::Core::LicenseFeature::LanguagePack) == Pci::Core::LicenseStatus::None ) ;
	}

}