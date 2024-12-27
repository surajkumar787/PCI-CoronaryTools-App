//Copyright (c)2014 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#ifndef		_CBDERRORMESSAGES_H
#define		_CBDERRORMESSAGES_H

// Array containing OscLib fatal and non-fatal error codes with associated messages

static CErrError CBDErrorArray[] =
{
	//         Code              IsFatal           Message
	{CBD_WRN_WRONG_ERRCODE,         0,     "CBD_WRN_WRONG_ERRCODE: This error code does not exit"},

	{CBD_ERR_MEM_ALLOC,             1,     "CBD_ERR_MEM_ALLOC: Memory allocation failed" },
    {CBD_ERR_SIZEEXCEEDSALLOCATION, 1,     "CBD_ERR_SIZEEXCEEDSALLOCATION: Image exceeds allocated memory" },
    {CBD_ERR_INTERNALERROR,         1,     "CBD_ERR_INTERNALERROR: Internal error" }

};

#endif   //_SWERRORMESSAGES_H
