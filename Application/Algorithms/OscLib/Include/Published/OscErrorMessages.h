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

#ifndef		_OSCERRORMESSAGES_H
#define		_OSCERRORMESSAGES_H

// Array containing OscLib fatal and non-fatal error codes with associated messages

static CErrError OscErrorArray[] =
{
	//         Code              IsFatal           Message
	{OSC_INFO_CARM_MOVED_INSEQ,					0,     "OSC_INFO_CARM_MOVED_INSEQ: C-arm position has changed since last informed" },
	{OSC_INFO_PIXSIZE_CHANGED,					0,     "OSC_INFO_PIXSIZE_CHANGED: pixSize at isocenter has been changed" },
	{OSC_INFO_CARM_MOVED_ANGIOFLUORO,			0,     "OSC_INFO_CARM_MOVED_ANGIOFLUORO: C-arm change between angio and fluoro" },
	{OSC_INFO_SHUTTERINFO_CHANGE,				0,     "OSC_INFO_SHUTTERINFO_CHANGE: Shutter value has changed" },

    {OSC_WNG_TOO_LONG_ANGIO,					0,     "OSC_WNG_TOO_LONG_ANGIO: The angio is too long. Extra frames are not processed." },
	{OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE,		0,     "OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE: inhomogeneous extracted cycle" }, 
	{OSC_WRN_NONINJECTED_ANGIO,					0,     "OSC_WRN_NONINJECTED_ANGIO: non-injected angio" }, 
	{OSC_WRN_BADLY_INJECTED_ANGIO,				0,     "OSC_WRN_BADLY_INJECTED_ANGIO: badly injected angio" }, 
	{OSC_WRN_IMSIZE_INCOMP_WITH_PARAMS,			0,     "OSC_WRN_IMSIZE_INCOMP_WITH_PARAMS: image size not compatible with pyramid and SS parameters. Need to pad input image." }, 
	
	{OSC_ERR_DLL_VERSION,						1,     "OSC_ERR_DLL_VERSION: A dll version is not compatible with library" },
	{OSC_ERR_IMADYN_RANGE,						1,     "OSC_ERR_IMADYN_RANGE:  Argument ImaDyn of OscCreate out of range" }, 
	{OSC_ERR_INCORRECT_TIME,					1,     "OSC_ERR_INCORRECT_TIME: incorrect time (none-incremental)" },
	{OSC_ERR_SYSTEM_INFO_MISSING,				1,     "OSC_ERR_SYSTEM_INFO_MISSING: mandatory system information missing (SOD, SID or detector pixel size)" },
	{OSC_ERR_INCORRECT_SHUTTERINFO,				1,     "OSC_ERR_INCORRECT_SHUTTERINFO: incorrect shutter information" },
	{OSC_ERR_MEM_ALLOC,							1,     "OSC_ERR_MEM_ALLOC: Memory allocation failed" },
	{OSC_ERR_CRITICAL_PARA,						1,     "OSC_ERR_CRITICAL_PARA: Critical parameter has been changed during the computation" },
	{OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT,	1,     "OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT: Incoherent arguments passed to a subfunction" },
    {OSC_ERR_PRMLIB,							1,     "OSC_ERR_PRMLIB: Error in the parameter library PrmLib" },
	{OSC_ERR_DIALIB,							1,     "OSC_ERR_DIALIB: Error in the diaphragm library DiaLib" },
	{OSC_ERR_CBDLIB,							1,     "OSC_ERR_CBDLIB: Error in the catheter body library CBD" },
	{OSC_ERR_SWLIB,								1,     "OSC_ERR_SWLIB: Error in the sternal wire library SW" },
	{OSC_ERR_INTERNAL,							1,     "OSC_ERR_INTERNAL: Internal library error" },
	{OSC_ERR_LABELING,							1,     "OSC_ERR_LABELING: Error in the labeling process" },
	{OSC_ERR_ORIGIN,							1,     "OSC_ERR_ORIGIN: Error before the ErrHdl has been initialized" },
	{OSC_ERR_SHORT_ANGIOSEQ,					1,     "OSC_ERR_SHORT_ANGIOSEQ: Angio sequence is too short" },
	{OSC_ERR_NO_CARDIAC_CYCLE,					1,     "OSC_ERR_NO_CARDIAC_CYCLE: Could not estimate any heart cycle" },
	{OSC_ERR_ANGIO_UNPROCESSED,					1,     "OSC_ERR_ANGIO_UNPROCESSED: No matching angio has been correctly processed yet" },
	{OSC_ERR_EMPTY_POINTER,						1,     "OSC_ERR_EMPTY_POINTER: Mandatory pointer is NULL" },
	{OSC_ERR_IMSIZEEXCEEDSALLOCATION,			1,     "OSC_ERR_IMSIZEEXCEEDSALLOCATION: (To process) image size exceeds maximum image size (set during handle creation)" },
	{OSC_ERR_PREVIOUS_FATAL_ERROR,		        1,     "OSC_ERR_ON_PREVIOUS_IMAGE: Fatal error has been reported previously" },
    {OSC_ERR_LOAD_MEMORY,                       1,     "OSC_ERR_LOAD_MEMORY: Error loading from memory"},
    {OSC_ERR_NEGATIVE_VECTOR_LENGTH,            1,     "OSC_ERR_NEGATIVE_VECTOR_LENGTH: Tentative to work with vector of negative length"}

	
};

#endif   //_OSCERRORMESSAGES_H
