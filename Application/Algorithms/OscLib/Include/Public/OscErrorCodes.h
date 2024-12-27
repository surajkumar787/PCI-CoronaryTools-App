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

#ifndef		_OSCERRORCODES_H
#define		_OSCERRORCODES_H

#define OSC_ERRMG_MAJOR 1024

/** Error codes returned by OscLib */
enum
{ 
	OSC_ERR_FIRST = 0,                              /** To keep track of the library error code interval */

	/** Information messages. Can be ignored. */

	OSC_INFO_CARM_MOVED_INSEQ = 1,                  /** Information: C-arm position has changed since last informed. */
	OSC_INFO_PIXSIZE_CHANGED,                       /** Information: PixSize at isocenter has been changed */
	OSC_INFO_CARM_MOVED_ANGIOFLUORO,                /** Information: C-arm change between angio and fluoro */
	OSC_INFO_SHUTTERINFO_CHANGE,                    /** Information: Shutter value has changed */

    /** Warnings. Non-fatal, can be ignored. Subsequent OscLib computations possible. */

    OSC_WNG_TOO_LONG_ANGIO,                         /** Warning: The angio is too long. Extra frames are not processed. */
	OSC_WRN_INHOMOGENEOUS_EXTRACTED_CYCLE,          /** Warning: Inhomogeneous extracted cycle. Unpleasant repeated frashes are expected.*/
	OSC_WRN_NONINJECTED_ANGIO,                      /** Warning: Non-injected angio. The injection indicator is so low that we think no injection was performed in the sequence.*/
	OSC_WRN_BADLY_INJECTED_ANGIO,                   /** Warning: Badly injected angio. The injection indicator is so low that we think that only a faint injection was performed in the sequence.*/
	OSC_WRN_IMSIZE_INCOMP_WITH_PARAMS,              /** Warning: Image size not compatible with pyramid and SS parameters. Need to pad input image. This takes a little extra time.*/

	/** Errors. Fatal, no subsequent OscLib computations possible. */

	OSC_ERR_DLL_VERSION,                            /** Error: A dll version is not compatible with library. */
	OSC_ERR_IMADYN_RANGE,                           /** Error: Argument ImaDyn of OscCreate out of range. */
	OSC_ERR_INCORRECT_TIME,                         /** Error: Incorrect time (none-incremental). */
	OSC_ERR_SYSTEM_INFO_MISSING,                    /** Error: Mandatory system information missing (SOD, SID or detector pixel size). */
	OSC_ERR_INCORRECT_SHUTTERINFO,                  /** Error: Incorrect shutter information. */
	OSC_ERR_MEM_ALLOC,                              /** Error: Memory allocation failed. */
	OSC_ERR_CRITICAL_PARA,                          /** Error: Critical parameter has been changed during the computation. */
	OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT,        /** Error: Incoherent arguments passed to a subfunction. */
	OSC_ERR_PRMLIB,                                 /** Error: Error in the parameter library PrmLib. */
	OSC_ERR_DIALIB,                                 /** Error: Error in the diaphragm library DiaLib. */
	OSC_ERR_CBDLIB,                                 /** Error: Error in the catheter body library CBD. */
	OSC_ERR_SWLIB,                                  /** Error: Error in the sternal wire library SW. */
	OSC_ERR_INTERNAL,                               /** Error: Internal library error. It often refers to a division by 0 (homogeneous input image?)*/
	OSC_ERR_LABELING,                               /** Error: Error in the labeling process. So many labels that they cannot be stored.*/
	OSC_ERR_ORIGIN,                                 /** Error: Error before the ErrHdl has been initialized. */
	OSC_ERR_SHORT_ANGIOSEQ,                         /** Error: Angio sequence is too short. */
	OSC_ERR_NO_CARDIAC_CYCLE,                       /** Error: Could not estimate any heart cycle. The injection was probably not long enough.*/
	OSC_ERR_ANGIO_UNPROCESSED,                      /** Error: No matching angio has been correctly processed yet. */
	OSC_ERR_EMPTY_POINTER,                          /** Error: Mandatory pointer is NULL. */
	OSC_ERR_IMSIZEEXCEEDSALLOCATION,                /** Error: (To process) image size exceeds maximum image size (set during handle creation). */
	OSC_ERR_PREVIOUS_FATAL_ERROR,                   /** Error: Fatal error has been reported previously. */
    OSC_ERR_LOAD_MEMORY,                            /** Error: Error loading from memory. */
    OSC_ERR_NEGATIVE_VECTOR_LENGTH,                 /** Error: Vector with negative length. */

	OSC_ERR_LAST // Just to keep track of the error code number (= OSC_ERR_LAST - OSC_ERR_FIRST - 1)
};

#endif   //_OSCERRORCODES_H
