// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include "PciExpressTestServer.h"
#include <iostream>

PciExpressTestServer::PciExpressTestServer(void) :
	lastUserMessage("Last user message: Not set yet")
{
}


PciExpressTestServer::~PciExpressTestServer(void)
{
}

bool PciExpressTestServer::GetConnectionStateCwis(void)
{
    std::cout << "GetConnectionStateCwis" << std::endl;
    return isCwisConnected;
}

bool PciExpressTestServer::GetConnectionStateDvlp(void)
{
    std::cout << "GetConnectionStateDvlp" << std::endl;
    return isDvlpConnected;
}

ApplicationState PciExpressTestServer::GetApplicationState(void)
{
    std::wcout << L"GetApplicationState: " << applicationState.ViewState << L", " << applicationState.GuidanceState << std::endl;
    return applicationState;
}

std::string PciExpressTestServer::GetLastUserMessage(void)
{
    std::cout << "GetLastUserMessage: " << lastUserMessage << std::endl;
    return lastUserMessage;
}

void PciExpressTestServer::setLastUserMessage(const std::string& message)
{
    std::cout << "SetLastUserMessage: " << message << std::endl;
    lastUserMessage = message;
}

void PciExpressTestServer::setApplicationState(const ApplicationState& appState)
{
    std::cout << "setApplicationState" << std::endl;
    applicationState = appState;
}

void PciExpressTestServer::setCwisConnectionState(bool connected)
{
    isCwisConnected = connected;
}

void PciExpressTestServer::setDvlpConnectionState(bool connected)
{
    isDvlpConnected = connected;
}

void PciExpressTestServer::setRoadmapActive(bool active)
{
	std::cout << "setRoadmapAvailable" << std::endl;
	applicationState.RoadmapActive = active;
}

void PciExpressTestServer::setGuidanceState(const std::wstring& guidanceState)
{
	std::cout << "setGuidanceState" << std::endl;
	applicationState.GuidanceState = guidanceState;
}

void PciExpressTestServer::setViewState(const std::wstring& viewState)
{
	std::cout << "setViewState" << std::endl;
	applicationState.ViewState = viewState;
}
