// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <fstream>

#include "CrmAlgoFluoroErrorHandlingTest.h"
#include "CrmAlgoFluoroValidRoadmapTest.h"
#include "CrmAlgoAngioErrorHandlingTest.h"
#include "CrmAlgoAngioValidAngioTest.h"
#include "CrmAlgoFluoroTransformationsTest.h"
#include "CrmAdministrationRoadmapTest.h"
#include "CrmAdministrationPersistentFileTest.h"
#include "CrmAdministrationGeometryTest.h"
#include "CrmAdministrationWithoutPersistentFile.h"
#include "CrmRepositoryTest.h"
#include "CrmRepositoryTestManyRuns.h"
#include "CrmRepositoryTestHighRunNumbers.h"
#include "CrmApcControllerInitialisationTest.h"
#include "CrmApcControllerTest.h"

#include <Windows.h>
#include <gtest/gtest.h>
#include "MemoryLeakDetector.h"

using namespace Pci::Test;
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