// Copyright Koninklijke Philips N.V. 2020
//
// All rights are reserved. Reproduction or transmission in whole or in part, in
// any form or by any means, electronic, mechanical or otherwise, is prohibited
// without the prior written consent of the copyright owner.

#include <sscfts1/interface.begin.h>

#ifdef SSCFTS1_BEGIN_INTERFACE

SSCFTS1_BEGIN_INTERFACE_VERSION(IAcquisitionNotification, "1")
	SSCFTS1_BEGIN_METHODS(IAcquisitionNotification)
		SSCFTS1_INTERFACE_METHOD_0(bool, IAcquisitionNotification, IsInProgress)
	SSCFTS1_END_METHODS

	SSCFTS1_BEGIN_EVENTS(IAcquisitionNotification)
		SSCFTS1_INTERFACE_EVENT_0(IAcquisitionNotification, StatusChanged)
	SSCFTS1_END_EVENTS
SSCFTS1_END_INTERFACE

#endif
#include <sscfts1/interface.end.h>
