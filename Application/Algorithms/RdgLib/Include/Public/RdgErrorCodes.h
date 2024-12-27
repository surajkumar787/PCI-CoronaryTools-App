// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_RDGERRORCODES_H
#define		_RDGERRORCODES_H

#define RDG_ERRMG_MAJOR 1024

// Warnings or non-fatal errors -> subsequent calls to RdgLib are possible
enum
{ 
	RDG_WRN_WRONG_ERRCODE = 1, 
	RDG_WRN_INCORRECT_TIME
};

// Fatal errors -> subsequent calls to RdgLib should not be attempted
enum
{ 
	RDG_ERR_IMADYN_RANGE = RDG_ERRMG_MAJOR, 
	RDG_ERR_IMAWIDTH_RANGE, 
	RDG_ERR_IMAHEIGHT_RANGE,
   RDG_ERR_WRONG_PROFILE,
	RDG_ERR_MEM_ALLOC,
	RDG_ERR_MEM_FUNC_ARG
};

#endif   //_RDGERRORCODES_H