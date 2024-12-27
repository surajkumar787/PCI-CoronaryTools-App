// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


// Developer-level first-layer structures.
#ifndef		_ERRMESSAGES_H
#define		_ERRMESSAGES_H

static CErrError __ErrErrorArray[] =
{
	//     Error Code         IsFatal            Message
	{ERR_WRN_WRONG_ERRCODE,     0,    "ERR_WRN_WRONG_ERRCODE: This error code does not exist"},
	{ERR_WRN_REPORT_OVERFLOW,   0,    "ERR_WRN_REPORT_OVERFLOW: Error report overflowed"},
	{ERR_ERR_CODE_CLASH,        0,    "ERR_ERR_CODE_CLASH: An error code clash occured"},
	{ERR_ERR_INTERNAL,          1,    "ERR_ERR_INTERNAL: Internal error"},
};

#endif   //_ERRMESSAGES_H