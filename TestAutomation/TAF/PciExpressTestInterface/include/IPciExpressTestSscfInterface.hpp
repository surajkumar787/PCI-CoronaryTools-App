// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.
#include <sscfts1/interface.begin.h>
#ifdef SSCFTS1_BEGIN_INTERFACE


SSCFTS1_BEGIN_STRUCT(ApplicationState)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), ViewState)
	SSCFTS1_STRUCT_MEMBER(reftype(std::wstring), GuidanceState)
	SSCFTS1_STRUCT_MEMBER(bool, RoadmapActive)
SSCFTS1_END_STRUCT


SSCFTS1_BEGIN_INTERFACE(IPciExpressTestSscfInterface)

    SSCFTS1_BEGIN_METHODS(IPciExpressTestSscfInterface)
        SSCFTS1_INTERFACE_METHOD_0(bool,                      IPciExpressTestSscfInterface, GetConnectionStateCwis)
        SSCFTS1_INTERFACE_METHOD_0(bool,                      IPciExpressTestSscfInterface, GetConnectionStateDvlp)
        SSCFTS1_INTERFACE_METHOD_0(reftype(ApplicationState), IPciExpressTestSscfInterface, GetApplicationState)
        SSCFTS1_INTERFACE_METHOD_0(reftype(std::string),      IPciExpressTestSscfInterface, GetLastUserMessage)
    SSCFTS1_END_METHODS

    SSCFTS1_BEGIN_EVENTS(IPciExpressTestSscfInterface)
    SSCFTS1_END_EVENTS

SSCFTS1_END_INTERFACE

#endif
#include <sscfts1/interface.end.h>