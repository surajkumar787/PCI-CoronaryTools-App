// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <gtest/gtest.h>
#include "XraySourceRtoMock.h"

void XraySourceRtoMock::connect(const std::wstring & /* dvlpAdapter */, const std::wstring & /* dvlpMulticast */, int /* dvlpPort */, const std::wstring & /* cwisHost */, int /* cwisPort */)
{
}

void XraySourceRtoMock::disconnect()
{
}

bool XraySourceRtoMock::isConnected()
{
    return true;
}

void XraySourceRtoMock::setNewPatient(const std::wstring& patientName)
{
	CommonPlatform::Xray::XrayStudy study;
    study.patientName = patientName;

    if (eventStudy) eventStudy(study);
}

void XraySourceRtoMock::setNewEPX(const std::wstring& activityType)
{
	CommonPlatform::Xray::XrayEpx epx;
    epx.activityType = activityType;

    if (eventEpx) eventEpx(epx);
}

CommonPlatform::Xray::IXraySourceRto::CwisStatus XraySourceRtoMock::getCwisConnectionStatus()
{
    return CwisStatus::Connected;
}

CommonPlatform::Xray::IXraySourceRto::DvlpStatus XraySourceRtoMock::getDvlpConnectionStatus()
{
    return DvlpStatus::Connected;
}