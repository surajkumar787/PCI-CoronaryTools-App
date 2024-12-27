// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef _ERRLIB_H
#define _ERRLIB_H

#define ERR_VERSION_MAJOR    1
#define ERR_VERSION_MINOR    3
#define ERR_VERSION_PATCH    0

#define ERR_MESSAGE_MAXLEN 256

enum // Errors specific to ErrLib !
{ 
	ERR_LIB_FIRST = 1000000,   // Just to keep track of the library code interval
	ERR_WRN_WRONG_ERRCODE,     // This error code does not exist
	ERR_WRN_REPORT_OVERFLOW,   // Error report overflowed
	ERR_ERR_CODE_CLASH,        // An error code clash occured
	ERR_ERR_INTERNAL,          // Internal error
	ERR_LIB_LAST               // Number of codes for error lib : ERR_LIB_LAST-ERR_LIB_FIRST-1
};

// Library handle
typedef struct 
{ 
	void* Pv;  // Access to private material
} CErrHdl; 

typedef struct
{
	int  Code;                         // Error code (an int from enum)
	int  IsFatal;                      // 0 = warning, 1 = fatal
	char Message[ERR_MESSAGE_MAXLEN];  // Error message (non-contextual report)

}CErrError; 

//======================================================================================
#ifdef __cplusplus
extern "C" {
#endif

// Can be called before ErrCreate for early version checking (no library handle needed)
int ErrGetVersionNumbers(int* Major, int* Minor, int* Patch); 

int ErrCreate(CErrHdl* ErrHdl);
int ErrDelete(CErrHdl ErrHdl);
int ErrAdd(CErrHdl ErrHdl, CErrError ErrorArrayToAdd[], int ErrorNbToAdd);

int ErrGetVersionInfo(CErrHdl ErrHdl, char **VersionInfo); 

int ErrPrintErrorMessages(CErrHdl ErrHdl);
int ErrGetErrorMessage(CErrHdl ErrHdl, int ErrorCode, char **ErrorMessage);
int ErrIsFatal(CErrHdl ErrHdl, int ErrorCode, int *PtIsFatal);
int ErrGetLastErrorReport(CErrHdl ErrHdl, char **ErrorReport);

int ErrStartErrorReportFromLeaf(CErrHdl ErrHdl, char* LeafFunc, int ErrorCode, char *Reason);
int ErrAppendErrorReportFromCaller(CErrHdl ErrHdl, char* Caller, char* Callee, char* ArgContext, int CalleeCode);

int ErrPrintErrorReport(CErrHdl);
int ErrPrintErrorReportAndFreeze(CErrHdl);

// Remark on functions returning a pointer on an array of char. These are:
// ErrGetVersionInfo(CErrHdl ErrHdl, char **VersionInfo); 
// ErrGetLastErrorReport(CErrHdl ErrHdl, char **ErrorReport);
// ErrGetErrorMessage(CErrHdl ErrHdl, int ErrorCode, char **ErrorMessage);
// 
// For all those function, a pointer to a char* must be passed WITHOUT allocation
// Example: 
// 
// char* VersionInfo;
// ErrGetVersionInfo(ErrHdl, &VersionInfo);
// 
// Do NOT attempt to free the returned pointer (allocation & free done by library)

#ifdef __cplusplus
};
#endif

#endif   // _ERRLIB_H