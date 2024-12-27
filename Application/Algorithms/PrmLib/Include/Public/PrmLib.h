// ***************************************************************************
// * Copyright (c) 2002-2014 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


//
// Library client-level structures
//
#ifndef _PRMUSRSTRUCT_H
#define _PRMUSRSTRUCT_H

#include <ErrLib.h>

// Version numbers
#define PRM_VERSION_MAJOR    1
#define PRM_VERSION_MINOR    4
#define PRM_VERSION_PATCH    0

// Maximum vector length in Generic para 
#define PRM_CST_PARA_VEC_LENGTH  16

// Number of characters for parameter AKA (Also.Known.As) and TEXT (info)
#define PRM_CST_PARA_AKA_LENGTH  64
#define PRM_CST_PARA_TXT_LENGTH  256

#define PRM_CST_MAXPARANB 1024

// Error codes for this library
enum 
{ 
	PRM_ERR_FIRST = 2000000,
	PRM_WRN_INT_RANGE, 
	PRM_WRN_FLOAT_RANGE,
	PRM_WRN_IVEC_RANGE,
	PRM_WRN_FVEC_RANGE,
	PRM_WRN_NOT_IN_CREATE,
	PRM_WRN_INCORRECT_PARANB,
	PRM_ERR_INDEX_RANGE,
	PRM_ERR_FIELD_RANGE,
	PRM_ERR_TYP_RANGE,
	PRM_ERR_AKA_RANGE,
	PRM_ERR_TXT_RANGE,
	PRM_ERR_MINMAX_RANGE,
	PRM_ERR_DEF_RANGE,
	PRM_ERR_ACC_RANGE,
	PRM_ERR_OVERFLOW,
	PRM_ERR_MISSING_CTX,
	PRM_ERR_INTERNAL,
	PRM_ERR_LAST  // Just to keep track of the number (= PRM_ERR_LAST - PRM_ERR_FIRST - 1)
};

// Library handle
typedef struct 
{ 
	CErrHdl ErrHdl; // Error handle associated to library
	void*   Pv;     // Access to private material
} CPrmHdl; 

typedef enum {
	PRM_CST_AKA,  // enum min
	PRM_CST_TXT,
	PRM_CST_TAG,
	PRM_CST_TYP,
	PRM_CST_MIN,
	PRM_CST_MAX,
	PRM_CST_ACC,
	PRM_CST_DEF   // enum max
}CPrmCtxFields;

typedef enum  
{
	PRM_CST_INT,     
	PRM_CST_FLOAT,    
	PRM_CST_IVEC,    
	PRM_CST_FVEC    
} 
CPrmParamType;

typedef union 
{
	int   Int;
	float Float;
	int   IVec[PRM_CST_PARA_VEC_LENGTH];
	float FVec[PRM_CST_PARA_VEC_LENGTH];
}
CPrmGenType;

typedef struct 
{
	char Aka[PRM_CST_PARA_AKA_LENGTH];  // Parameter name (Also-Known-As)
	char Txt[PRM_CST_PARA_TXT_LENGTH];  // Related information
	int              Tag;               // Parameter tag (to group parameters, when needed)
	int              Len;               // Actual number of elements when vector (1 otherwise)
	CPrmParamType    Typ;               // Parameter type (int or float) 
	CPrmGenType      Min;               // Minimum value
	CPrmGenType      Max;               // Maximum value
	CPrmGenType      Acc;               // Recommanded minimum accuracy
	CPrmGenType      Def;               // Default value
}
CPrmGenCtx;

typedef struct
{
	CPrmGenType Val[PRM_CST_MAXPARANB];
	int         Count;
}
CPrmPara;

typedef struct
{
	CPrmGenCtx Ctx[PRM_CST_MAXPARANB];
	int        Count;
}
CPrmParaCtx;

typedef int (*CPrmSetCtxFuncType)(CPrmHdl P);

//======================================================================================
#ifdef __cplusplus
extern "C" {
#endif

// Can be called before PrmCreate for early version checking (no library handle needed)
int PrmGetVersionNumbers(int* Major, int* Minor, int* Patch); 

int PrmCreate(CPrmHdl* PtPrmHdl, CErrHdl ErrHdl, int NbPara, CPrmSetCtxFuncType Func);
int PrmDelete(CPrmHdl PrmHdl);

int PrmGetVersionInfo(CPrmHdl PrmHdl, char** VersionInfo);

int PrmGetDefaultPara(CPrmHdl PrmHdl, CPrmPara *Para);
int PrmGetParaContext(CPrmHdl PrmHdl, CPrmParaCtx *ParaCtx);
int PrmGetPara(CPrmHdl PrmHdl, CPrmPara *Para);

int PrmSetParaToDefault(CPrmHdl PrmHdl);
int PrmSetPara(CPrmHdl PrmHdl, CPrmPara *Para);

// Check if any current parameter bearing a certain tag has changed as compared to a reference para struct
// If at least one such parameter has changed, the index of the first one is returned in ParaIndexOrMinusOne, 
// otherwise *ParaIndexOrMinusOne == -1
int PrmHasAnyTaggedParaChanged(CPrmHdl PrmHdl, CPrmPara *RefPara, int Tag, int* ParaIndexOrMinusOne);

int PrmPrintParaWithContext(CPrmHdl PrmHdl);
int PrmPrintParaCurVal(CPrmHdl PrmHdl);

// Remark on PrmGetVersionInfo
// For argument VersionInfo, a pointer to a char* must be passed WITHOUT allocation
// 
// char* VersionInfo;
// PrmGetVersionInfo(ErrHdl, &VersionInfo);
// 
// Do NOT attempt to free the returned pointer (allocation & free done by library)

// Functions to be used to fill context in Func-pointer passed to PrmCreate
// Those functions are neutral when called elsewhere (built-in library protection)
int PrmSetCtxStr(CPrmHdl PrmHdl, int Idx, int Field, char *Str);
int PrmSetCtxInt(CPrmHdl PrmHdl, int Idx, int Field, int Val);
int PrmSetCtxFlt(CPrmHdl PrmHdl, int Idx, int Field, float Val);
int PrmSetCtxIVc(CPrmHdl PrmHdl, int Idx, int Field, int NbVal, ...);
int PrmSetCtxFVc(CPrmHdl PrmHdl, int Idx, int Field, int NbVal, ...);

#ifdef __cplusplus
};
#endif

#endif   //_PRMUSRSTRUCT_H