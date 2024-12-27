// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_MKXERRMESSAGES_H
#define		_MKXERRMESSAGES_H

// Array containing MkxLib fatal and non-fatal error codes with associated messages
static CErrError __MkxErrorArray[] =
{
	   //     Code               IsFatal           Message

	{MKX_WRN_INCORRECT_TIME,        0,   "MKX_WRN_INCORRECT_TIME: Incorrect time (none-incremental)" }, 
	{MKX_WRN_DARK_IMAGE,            0,   "MKX_WRN_DARK_IMAGE: Fully dark image (or Roi)" }, 
	{MKX_WRN_WRONG_EXTRACT_ROI,     0,   "MKX_WRN_WRONG_EXTRACT_ROI: Incorrect extract roi coordinates (parameter ignored)" }, 
	{MKX_WRN_WRONG_WIRE_STATUS,     0,   "MKX_WRN_WRONG_WIRE_STATUS: Incorrect wire status (" }, 
	{MKX_WRN_WRONG_INTERPOL,        0,   "MKX_WRN_WRONG_INTERPOL: Incorrect interpol (default chosen)" }, 
	{MKX_WRN_CLIPPED_ARG,           0,   "MKX_WRN_CLIPPED_ARG: An argument has been clipped to a reasonable value" }, 

	{MKX_ERR_IMADYN_RANGE,          1,   "MKX_ERR_IMADYN_RANGE:  Argument ImaDyn of MkxCreate out of range" }, 
	{MKX_ERR_IMAWIDTH_RANGE,        1,   "MKX_ERR_IMAWIDTH_RANGE: Argument ImaWidth of MkxCreate out of range" },
	{MKX_ERR_IMAHEIGHT_RANGE,       1,   "MKX_ERR_IMAHEIGHT_RANGE: Argument ImaHeight of MkxCreate out of range" }, 
	{MKX_ERR_CRITICAL_PARA,         1,   "MKX_ERR_CRITICAL_PARA: Critical parameters changed at t!=0" }, 
	{MKX_ERR_FUNC_ARG,              1,   "MKX_ERR_FUNC_ARG: An incorrect function argument passed" }, 
	{MKX_ERR_MEM_ALLOC,             1,   "MKX_ERR_MEM_ALLOC: Memory allocation failed" }, 
	{MKX_ERR_MEM_FUNC_ARG,          1,   "MKX_ERR_MEM_FUNC_ARG: Wrong arguments in memory management function" }, 
	{MKX_ERR_DLL_VERSION,           1,   "MKX_ERR_DLL_VERSION: Wrong dll version" }, 
	{MKX_ERR_INTERNAL,              1,   "MKX_ERR_INTERNAL: Internal (development) error" } 
};

#endif   //_MKXERRMESSAGES_H