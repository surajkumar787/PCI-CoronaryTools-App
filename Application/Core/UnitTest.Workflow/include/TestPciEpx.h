// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include <gtest/gtest.h>
#include <PciEpx.h>
#include <XrayEpx.h>

using namespace Pci::Core;

// This class tests the implementation of the class PciEpx 
class TestPciEpx : public ::testing::Test
{
public:

	CommonPlatform::Xray::XrayEpx incomingRoadmapXrayEpx;
	CommonPlatform::Xray::XrayEpx incomingStentBoostXrayEpx;
	CommonPlatform::Xray::XrayEpx incomingUnknownXrayEpx;
	CommonPlatform::Xray::XrayEpx incomingXrayEpxWithoutActivityType;

    void SetUp() override
	{
        incomingRoadmapXrayEpx.activityType             = L"LCA";
        incomingStentBoostXrayEpx.activityType          = L"StbPCISuite";
        incomingUnknownXrayEpx.activityType             = L"3dra";
        incomingXrayEpxWithoutActivityType.activityType = L"";
	}

    PciEpx::Type getExpTypeFromActivityType(wchar_t* activityType)
    {
		CommonPlatform::Xray::XrayEpx xrayEpx;
        xrayEpx.activityType = activityType;
		PciEpx pciEpx(xrayEpx);
        return pciEpx.getType();
    }
};

TEST_F(TestPciEpx, When_Contructor_Is_Called_Then_DefaultType_Is_Selected)
{
    PciEpx xrayEpx;
    ASSERT_EQ(  xrayEpx.getType(), PciEpx::Type::NonPci);
}

TEST_F(TestPciEpx, When_EpxType_Changes_Then_Changed_Must_Be_True)
{
	PciEpx pciEpx( PciEpx::Type::Roadmap);

    ASSERT_TRUE( pciEpx.changed(incomingStentBoostXrayEpx) );
    ASSERT_TRUE( pciEpx.changed(incomingUnknownXrayEpx) );
    ASSERT_TRUE( pciEpx.changed(incomingXrayEpxWithoutActivityType) );

	PciEpx nonePciEpx( PciEpx::Type::NonPci);
    ASSERT_TRUE(nonePciEpx.changed(incomingRoadmapXrayEpx) );
    ASSERT_TRUE(nonePciEpx.changed(incomingStentBoostXrayEpx) );
}

TEST_F(TestPciEpx, When_EpxType_Remains_The_Same_Then_Changed_Must_Be_False)
{
    PciEpx pciEpxCrm( PciEpx::Type::Roadmap);
    ASSERT_FALSE(pciEpxCrm.changed(incomingRoadmapXrayEpx) );

	PciEpx nonePciEpx( PciEpx::Type::NonPci);
    ASSERT_FALSE(nonePciEpx.changed(incomingUnknownXrayEpx) );
    ASSERT_FALSE(nonePciEpx.changed(incomingXrayEpxWithoutActivityType) );

	PciEpx pciEpxStentBoost(PciEpx::Type::StentBoost);
    ASSERT_FALSE(pciEpxStentBoost.changed(incomingStentBoostXrayEpx) );
}

TEST_F(TestPciEpx, When_Incoming_ActivityType_Is_Empty_Then_Epx_Type_Is_NonPci)
{
    PciEpx pciEpx(incomingXrayEpxWithoutActivityType);
    ASSERT_EQ(  pciEpx.getType(), PciEpx::Type::NonPci);
}

TEST_F(TestPciEpx, When_Incoming_ActivityType_Is_For_Other_Application_Then_Epx_Type_Is_NonPci)
{
    PciEpx pciEpx(incomingUnknownXrayEpx);
    ASSERT_EQ(  pciEpx.getType(), PciEpx::Type::NonPci);
}

TEST_F(TestPciEpx, When_Incoming_ActivityType_Is_For_CoronaryRoadmap_Then_Epx_Type_Is_Roadmap)
{
    PciEpx pciEpx(incomingRoadmapXrayEpx);
    ASSERT_EQ(  pciEpx.getType(), PciEpx::Type::Roadmap);
}

TEST_F(TestPciEpx, When_ActivityType_Has_More_Tags_Including_CoronaryRoadmap_Then_Epx_Type_Is_Roadmap)
{
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,RCA,OtherOption"   ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,rca,OtherOption"   ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,rca"               ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,RCA"               ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,RCA,"              ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest, RCA"              ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,RCA ,"             ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest, RCA"              ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest, RCA "             ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L"3draTest,RCA , "            ) );
    ASSERT_EQ ( PciEpx::Type::Roadmap, getExpTypeFromActivityType(L" 3draTest,RCA,"             ) );
}

TEST_F(TestPciEpx, When_ActivityType_Does_Not_Contain_PCI_Tokens_Then_Epx_Type_Is_NonPci)
{
    ASSERT_EQ ( PciEpx::Type::NonPci, getExpTypeFromActivityType(L"3draLCA"                    ) );
    ASSERT_EQ ( PciEpx::Type::NonPci, getExpTypeFromActivityType(L"lca2"                       ) );
    ASSERT_EQ ( PciEpx::Type::NonPci, getExpTypeFromActivityType(L"3draAnypos,3draStbPCISuite" ) );
}

TEST_F(TestPciEpx, When_Incoming_ActivityType_Is_For_StentBoostLive_Then_Epx_Type_Is_StentBoost)
{
    PciEpx pciEpx(incomingStentBoostXrayEpx);
    ASSERT_EQ(  pciEpx.getType(), PciEpx::Type::StentBoost);
}

TEST_F(TestPciEpx, When_ActivityType_Has_More_Tags_Including_StentBoostLive_Then_Epx_Type_Is_StentBoost)
{
    ASSERT_EQ ( PciEpx::Type::StentBoost, getExpTypeFromActivityType(L"3draStentBoost,stbpcisuite,3draside") );
    ASSERT_EQ ( PciEpx::Type::StentBoost, getExpTypeFromActivityType(L"3draStentBoost,StbPCISuite,3draside") );
    ASSERT_EQ ( PciEpx::Type::StentBoost, getExpTypeFromActivityType(L"3draStentBoost,STBPCISUITE,3draside") );
    ASSERT_EQ ( PciEpx::Type::StentBoost, getExpTypeFromActivityType(L"3draAnyPos,StbPCISuite"             ) );
}