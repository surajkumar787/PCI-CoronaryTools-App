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

#ifndef		_CBDERRORCODES_H
#define		_CBDERRORCODES_H

#define CBD_ERRMG_MAJOR 1024

enum
{ 
	CBD_ERR_FIRST = 0, // To keep track of the library error code interval

	// Warnings or non-fatal errors -> subsequent calls to OscLib are possible

	CBD_WRN_WRONG_ERRCODE = 1, 

	// Fatal errors -> subsequent calls to OscLib should not be attempted

	CBD_ERR_MEM_ALLOC,
    CBD_ERR_SIZEEXCEEDSALLOCATION,
    CBD_ERR_INTERNALERROR,

	CBD_ERR_LAST // Just to keep track of the error code number (= CBD_ERR_LAST - CBD_ERR_FIRST - 1)
};

#endif   //_CBDERRORCODES_H
