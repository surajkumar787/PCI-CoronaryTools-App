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

#ifndef		_DIAERRORCODES_H
#define		_DIAERRORCODES_H

#define DIA_ERRMG_MAJOR 1024

// Warnings or non-fatal errors -> subsequent calls to DIALib are possible
enum
{ 
	DIA_WRN_WRONG_ERRCODE = 1, 
	DIA_WRN_INCORRECT_TIME
};

// Fatal errors -> subsequent calls to DIALib should not be attempted
enum
{ 
	DIA_ERR_FIRST = 0, // To keep track of the library error code interval
    DIA_ERR_ORIGIN,
	DIA_ERR_MEM_ALLOC,
	DIA_ERR_ARGUMENTS,
	DIA_ERR_IMSIZEABOVEALLOCATED,
	DIA_ERR_LAST // Just to keep track of the error code number (= OSC_ERR_LAST - OSC_ERR_FIRST - 1)
};

#endif   //_RDGERRORCODES_H