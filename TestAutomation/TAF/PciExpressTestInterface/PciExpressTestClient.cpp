// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "PciExpressTestClient.h"

PciExpressTestClient::PciExpressTestClient(void)
:   sscfClient(nullptr),
    serviceProxy(nullptr),
    eventloop()
{
}


PciExpressTestClient::~PciExpressTestClient(void)
{
    Close();
}

/// <summary>
/// Connect to the system under test
/// </summary>
/// <param name="IPAddressSystemUnderTest">Provide a string containing the IP address of the system under tests. a string containing the name and version of the TAF adapter.</param>
/// <returns>KeywordResult</returns>
bool PciExpressTestClient::Connect(std::string IPAddressSystemUnderTest, uint16_t port)
{
    bool status = false;

    if ( port == 0 )
    {
        sscfClient = new ClientNode(eventloop, IPAddressSystemUnderTest, "urn:Philips-PciExpress:service:TestService:1");
    }
    else
    {
		std::stringstream hostname;
		hostname << IPAddressSystemUnderTest << ":" << port;
        sscfClient = new ClientNode(eventloop, IPAddressSystemUnderTest, hostname.str());
    }

    if (sscfClient->connect())
    {
        if (sscfClient->providesType<IPciExpressTestSscfInterface>())
        {
            status = true;
        }
    }

    return status;
}

void PciExpressTestClient::Close()
{
    if (sscfClient != nullptr)
    {
        sscfClient->disconnect();
        delete sscfClient;
        sscfClient = nullptr;
    }
}

bool PciExpressTestClient::GetConnectionStateCwis(void)
{
    if ( serviceProxy != nullptr ) return serviceProxy->GetConnectionStateCwis();
    return false;
}

bool PciExpressTestClient::GetConnectionStateDvlp(void)
{
    if ( serviceProxy != nullptr ) return serviceProxy->GetConnectionStateDvlp();
    return false;
}

ApplicationState PciExpressTestClient::GetApplicationState(void)
{
    if ( serviceProxy != nullptr ) return serviceProxy->GetApplicationState();
    return ApplicationState();
}

std::string PciExpressTestClient::GetLastUserMessage(void)
{
    if ( serviceProxy != nullptr ) return serviceProxy->GetLastUserMessage();    
    return "NotConnected";
}
