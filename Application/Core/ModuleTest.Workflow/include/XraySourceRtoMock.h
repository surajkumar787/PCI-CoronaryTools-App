// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#pragma once

#include "IXraySourceRto.h"

class XraySourceRtoMock : public CommonPlatform::Xray::IXraySourceRto
{
public:
     // IXraySourceRto interface members
    virtual void        connect(const std::wstring &dvlpAdapter, const std::wstring &dvlpMulticast, int dvlpPort, const std::wstring &cwisHost, int cwisPort) override;
	virtual void        disconnect()   override;
    virtual bool        isConnected()  override;
    virtual DvlpStatus  getDvlpConnectionStatus() override;
    virtual CwisStatus  getCwisConnectionStatus() override;

    // Test API
    void setNewPatient      (const std::wstring& patientName);
    void setNewEPX          (const std::wstring& activityType);

};

