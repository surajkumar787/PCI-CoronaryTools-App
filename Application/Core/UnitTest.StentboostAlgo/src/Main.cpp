// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <fstream>

#include "TestStbFrangi.h"

#include <gtest/gtest.h>
#include "MemoryLeakDetector.h"
#include <Windows.h>



int main(int argc, CHAR *argv[])
{
	Simplicity::system();


#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	::testing::UnitTest::GetInstance()->listeners().Append(new MemoryLeakDetector());
#endif



    ::testing::InitGoogleTest(&argc, argv);
	
	return RUN_ALL_TESTS();
}
