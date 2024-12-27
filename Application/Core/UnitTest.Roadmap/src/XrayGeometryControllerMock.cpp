// Copyright Koninklijke Philips N.V. 2015
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "XrayGeometryControllerMock.h"
#include "UnitTestLog.h"

//TICS -OLC#009  magic numbers for a test are ok.
namespace Pci {	namespace Test {
using namespace CommonPlatform::Xray;


XrayGeometryControllerMock::XrayGeometryControllerMock()
: apcEnabled        (false),
  apcAngulationAngle(-999.0),
  apcRotationAngle  (-999.0),
  apcActivated      (false)
{
}

XrayGeometryControllerMock::~XrayGeometryControllerMock()
{
}

bool XrayGeometryControllerMock::enableApc( double angulation, double rotation, bool activate)
{
    UNITTEST_LOG( "enableApc [Angulation=" << angulation << ",Rotation=" << rotation << ", Activate=" << activate << "]");
    apcEnabled = true;
    apcAngulationAngle  = angulation;
    apcRotationAngle    = rotation;
    apcActivated        = activate;
    return true;
}

bool XrayGeometryControllerMock::disableApc()
{
    UNITTEST_LOG( "disableApc");
    apcEnabled = false;
    return true;
}
}}
//TICS +OLC#009  magic numbers for a test are ok.