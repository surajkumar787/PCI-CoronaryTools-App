// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "IPciExpressTestSscfInterface.h"

// This class implements an SSCF interface
class PciExpressTestServer : public IPciExpressTestSscfInterface
{
public:
    PciExpressTestServer(void);
    ~PciExpressTestServer(void);

    void setCwisConnectionState(bool connected);
    void setDvlpConnectionState(bool connected);
    void setLastUserMessage    (const std::string& message);
    void setApplicationState   (const ApplicationState& appState);
	void setViewState          (const std::wstring& viewState);
	void setGuidanceState      (const std::wstring& guidanceState);
	void setRoadmapActive      (bool active);

public:
    // API of IPciExpressTestSscfInterface
    virtual bool             GetConnectionStateCwis() override;
    virtual bool             GetConnectionStateDvlp() override;
    virtual ApplicationState GetApplicationState   () override;
    virtual std::string      GetLastUserMessage    () override;

    // critical section to protect members

protected:
    // members
    bool isCwisConnected;
    bool isDvlpConnected;
    ApplicationState applicationState;
    std::string lastUserMessage;     // todo use std::wstring

};

