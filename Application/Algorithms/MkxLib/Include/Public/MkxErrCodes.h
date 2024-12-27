// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_MKXERRCODES_H
#define		_MKXERRCODES_H


enum
{ 
	MKX_ERR_FIRST = 0, // To keep track of the library error code interval

	MKX_WRN_INCORRECT_TIME,
	MKX_WRN_DARK_IMAGE,
	MKX_WRN_WRONG_EXTRACT_ROI,
	MKX_WRN_WRONG_WIRE_STATUS,
	MKX_WRN_WRONG_INTERPOL,
	MKX_WRN_CLIPPED_ARG,

	MKX_ERR_IMADYN_RANGE, 
	MKX_ERR_IMAWIDTH_RANGE, 
	MKX_ERR_IMAHEIGHT_RANGE,
	MKX_ERR_CRITICAL_PARA,
	MKX_ERR_FUNC_ARG,
	MKX_ERR_MEM_ALLOC,
	MKX_ERR_MEM_FUNC_ARG,
	MKX_ERR_DLL_VERSION,

	MKX_ERR_INTERNAL,
	MKX_ERR_LAST // Just to keep track of the error code number (= MKX_ERR_LAST - MKX_ERR_FIRST - 1)

};

#endif   //_MKXERRCODES_H