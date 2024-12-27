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

#ifndef		_DIAERRORMESSAGESS_H
#define		_DIAERRORMESSAGES_H

// Array containing OscLib fatal and non-fatal error codes with associated messages

static CErrError DiaErrorArray[] =
{
	//         Code              IsFatal           Message
	{DIA_ERR_ORIGIN,							1,     "DIA_ERR_ORIGIN: Error origin"},
	{DIA_ERR_MEM_ALLOC,							1,     "DIA_ERR_MEM_ALLOC: Error image range"},
	{DIA_ERR_ARGUMENTS,							1,     "DIA_ERR_ARGUMENTS: Error image range"},
	{DIA_ERR_IMSIZEABOVEALLOCATED,			    1,     "DIA_ERR_IMSIZEABOVEALLOCATED: Allocated image size smaller than the targeted image size"}
};

#endif   //_DIAERRORMESSAGES_H