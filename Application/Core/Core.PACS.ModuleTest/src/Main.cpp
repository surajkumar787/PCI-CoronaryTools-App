// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include <Windows.h>
#include <gtest/gtest.h>

#pragma warning (disable:4702)

#include "PatientNameTest.h"
#include "TransferSyntaxTests.h"
#include "MessageBuilderTests.h"
#include "TestSnapshot.h"
#include "TestPACSArchiver.h"
#include "MessageBuilderCharacterSetsTest.h"

int main(int argc, CHAR *argv[])
{
	std::unique_ptr<CommonPlatform::Dicom::Merge::Library> mergecomLib(new CommonPlatform::Dicom::Merge::Library("Merge.ini"));

#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}