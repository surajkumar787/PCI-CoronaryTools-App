// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Applications.h"

#include <sscfts1/interface.begin.h>
#ifdef SSCFTS1_BEGIN_INTERFACE

	SSCFTS1_BEGIN_INTERFACE(IAppExitNotifier)

		SSCFTS1_BEGIN_EVENTS(IAppExitNotifier)
            SSCFTS1_INTERFACE_EVENT_2(IAppExitNotifier, EventAppExit, Applications /*target*/, int /* exitCode */)
		SSCFTS1_END_EVENTS

	SSCFTS1_END_INTERFACE

#endif
#include <sscfts1/interface.end.h>
