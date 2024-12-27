// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <fstream>
#include "TestCoregistrationAdmin.h"
#include "TestCoregistrationRecording.h"
#include "TestCoregistrationControl.h"

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