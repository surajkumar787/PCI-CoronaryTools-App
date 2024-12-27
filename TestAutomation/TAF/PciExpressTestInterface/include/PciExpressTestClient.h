// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#pragma once

#include "IPciExpressTestSscfInterface.h"

using namespace SscfTs1;

// This class implements an native client to the SSCF interface.
// Warning: not tested yet, our project uses the managed client in a TAF adapter.
class PciExpressTestClient
{
public:
    PciExpressTestClient(void);
    ~PciExpressTestClient(void);

    bool Connect(std::string IPAddressSystemUnderTest, uint16_t port);
    void Close  ();

    // API of IPciExpressTestSscfInterface
    bool             GetConnectionStateCwis(void);
    bool             GetConnectionStateDvlp(void);
    ApplicationState GetApplicationState   (void);
    std::string      GetLastUserMessage    (void);

protected:
    ClientNode* sscfClient;
    IPciExpressTestSscfInterface* serviceProxy;
    EventLoop eventloop;

};

