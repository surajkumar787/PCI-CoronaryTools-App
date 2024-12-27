// Copyright Koninklijke Philips N.V. 2016
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include "Applications.h"

#include <sscfts1/interface.begin.h>
#ifdef SSCFTS1_BEGIN_INTERFACE

	SSCFTS1_BEGIN_INTERFACE(IAppLauncher)

		SSCFTS1_BEGIN_METHODS(IAppLauncher)
			SSCFTS1_INTERFACE_METHOD_2(void, IAppLauncher, Start, in(Applications) /*target*/, in(Applications) /*requestor*/)
			SSCFTS1_INTERFACE_METHOD_2(void, IAppLauncher, Restart, in(Applications) /*target*/, in(Applications) /*requestor*/)

			SSCFTS1_INTERFACE_METHOD_1(void, IAppLauncher, Stop, in(Applications) /*target*/)
			SSCFTS1_INTERFACE_METHOD_1(void, IAppLauncher, WarmRestart, in(Applications) /*target*/)

			SSCFTS1_INTERFACE_METHOD_1(void, IAppLauncher, Reboot, in(Applications) /*requestor*/)
			SSCFTS1_INTERFACE_METHOD_1(void, IAppLauncher, Shutdown, in(Applications) /*requestor*/)
		SSCFTS1_END_METHODS

		SSCFTS1_BEGIN_EVENTS(IAppLauncher)
		SSCFTS1_END_EVENTS

	SSCFTS1_END_INTERFACE

#endif
#include <sscfts1/interface.end.h>
