// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
//
// Filename: main.cpp

#include "CrmModuleTests.h"
#include <Windows.h>
#include <gtest/gtest.h>

int main(int argc, CHAR *argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}