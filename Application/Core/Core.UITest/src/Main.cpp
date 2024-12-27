// Copyright Koninklijke Philips N.V. 2018
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TestArchivingMessages.h"
#include "gtest/gtest.h"

void parseArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{
#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    ::testing::InitGoogleTest(&argc, argv);

	parseArgs(argc, argv);

	int result = RUN_ALL_TESTS();
    return result;
}

void parseArgs(int argc, char *argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "showUI") == 0)
			TestArchivingMessages::showUI = true;
		else if (strcmp(argv[i], "manual") == 0)
			TestArchivingMessages::manual = true;
		else if (strcmp(argv[i], "review") == 0)
			TestArchivingMessages::review = true;
	}
}
