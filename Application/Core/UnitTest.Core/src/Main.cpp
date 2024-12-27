// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "TestXrayImageRecorder.h"
#include "TestXRayImageRecording.h"
#include "TestStbRecording.h"
#include "TestLicensesCollection.h"
#include "TestCrmRecording.h"

#define _WINSOCKAPI_ 
#include <Windows.h>

int main(int argc, CHAR *argv[])
{
#ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();

    return result;
}