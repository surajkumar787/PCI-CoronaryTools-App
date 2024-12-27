// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdarg.h>
#include <float.h> 

#include <ErrLib.h>
#include <PrmLib.h>
#include <PrmLibDev.h>
#include <PrmErrMessages.h>
#include <PrmLibVersion.h>

/*************************************************************************************/
// Library instantiation  (and valid Error-handle must be provided)
/*************************************************************************************/
int PrmCreate(CPrmHdl* PtPrmHdl, CErrHdl ErrHdl, int NbPara, CPrmSetCtxFuncType Func)
{
	CPrm    *This;
	int     NbErrors;
	char    Reason[128];
	char    FuncName[] = "PrmCreate";
	int     Err, Dummy;

	This = (CPrm*)malloc(sizeof(CPrm));
	This->ErrorHdl   = ErrHdl;
	PtPrmHdl->Pv     = (void*)This;
	PtPrmHdl->ErrHdl = ErrHdl;
	This->PrmHdl.ErrHdl = ErrHdl;
	This->PrmHdl.Pv     = (void*)This;
	This->WithinCreate = 1;
	This->MaxNbCharInVersionInfo = (int)(strlen(PRM_VERSION_HEADER) + strlen(PRM_VERSION_INFO) + strlen(PRM_CONFIG) + strlen(PRM_PLATFORM) + 16);

	NbErrors = PRM_ERR_LAST - PRM_ERR_FIRST - 1;

	// Adding PrmLib-errors to ErrLib instance
	Err = ErrIsFatal(ErrHdl, PRM_ERR_FIRST+1, &Dummy); // Just to check that PrmLib-errors have not been added yet (other param set)
	if (Err==ERR_WRN_WRONG_ERRCODE)
	{
		Err = ErrAdd(ErrHdl, __PrmErrorArray, NbErrors);
		if(Err)
		{
			ErrAppendErrorReportFromCaller(This->ErrorHdl, FuncName, "ErrAdd", 0, Err);
			return Err;
		}
	}

	// Setting library version material
	Err = PrmSetVersion(This);
	
	// Argument checking
	if(NbPara>PRM_CST_MAXPARANB)
	{
		Err = PRM_ERR_OVERFLOW;
		sprintf_s(Reason, 128, "NbPara = (%d) : Overflow (>%d)", NbPara, PRM_CST_MAXPARANB);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	// Preparing context
	This->Context.Count = NbPara;
	PrmClearCtx(*PtPrmHdl);

	// Setting context with external function
	Err = Func(*PtPrmHdl);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(ErrHdl, FuncName, "Function-Passed-To-PrmCreate", 0, Err);
		return Err;
	}

	// Checking context
	Err = PrmCheckParaCtx(*PtPrmHdl);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(ErrHdl, FuncName, "PrmCheckParaCtx", 0, Err);
		return Err;
	}

	// Preparing default parameters (from param context)
	PrmSetParaToDefault(*PtPrmHdl);

	This->WithinCreate = 0;
	return(0);
} 

/*************************************************************************************/
// Library deleting 
/*************************************************************************************/
int PrmDelete(CPrmHdl PrmHdl)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;
	free(This->VersionInfo);
	free(This->VersionInfoCopy);
	free(This);
	return 0;
}

/*************************************************************************************/
// Set version material from version include
/*************************************************************************************/
int PrmSetVersion(CPrm *This)
{
	int  NbChar;
	char *Format;

	NbChar = This->MaxNbCharInVersionInfo;
	This->VersionInfo     = (char*)malloc(NbChar*sizeof(char));
	This->VersionInfoCopy = (char*)malloc(NbChar*sizeof(char));
	Format                = (char*)malloc(NbChar*sizeof(char));

	sprintf_s(Format, NbChar, "%s%s", PRM_VERSION_HEADER, PRM_VERSION_INFO);
	sprintf_s(This->VersionInfo, NbChar, Format, PRM_CONFIG, PRM_PLATFORM, PRM_VERSION_MAJOR, PRM_VERSION_MINOR, PRM_VERSION_PATCH);

	free(Format);
	return 0;
}

/*************************************************************************************/
// Returns a pointer on version info 
/*************************************************************************************/
int PrmGetVersionInfo(CPrmHdl ErrHdl, char** VersionInfo)
{
	CPrm* This = (CPrm*)ErrHdl.Pv;
	strcpy_s(This->VersionInfoCopy, This->MaxNbCharInVersionInfo, This->VersionInfo);
	*VersionInfo = This->VersionInfoCopy;
	return 0;
}

/*************************************************************************************/
// Returns version numbers 
/*************************************************************************************/
int PrmGetVersionNumbers(int* Major, int* Minor, int* Patch)
{
	*Major = PRM_VERSION_MAJOR; *Minor = PRM_VERSION_MINOR; *Patch = PRM_VERSION_PATCH;
	return 0;
}

/*************************************************************************************/
// Clears/Initializes the parameter context array, this will be used to check if all 
// contexts were defined when setting parameter context
/*************************************************************************************/
int PrmClearCtx(CPrmHdl PrmHdl)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;
	// Set parameter context to zero
	memset(This->Context.Ctx, 0, sizeof(This->Context.Ctx));
	return 0;
}

/************************************************************/
/* Check if the field and index for a para are within range */
/************************************************************/
int PrmCheckParaCtxIdentity(CPrm *This, int Idx, int Field)
{
	char Reason[128];
	int  Err, Count;

	if(Field < PRM_CST_AKA || Field > PRM_CST_DEF)
	{
		Err = PRM_ERR_FIELD_RANGE;
		sprintf_s(Reason, 128, "Field = (%d) : Out-of-Range [%d, %d]", Field, PRM_CST_AKA, PRM_CST_DEF);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaCtxIdentity", Err, Reason);
		return Err;
	}

	Count = This->Context.Count;

	if(Idx < 0 || Idx >= Count)
	{
		Err = PRM_ERR_INDEX_RANGE;
		sprintf_s(Reason, 128, "Index = (%d) : Out-of-Range [0,%d]", Idx, Count-1);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaCtxIdentity", Err, Reason);
		return Err;
	}

	return 0;
}

/*******************************************************/
/* Set a string field of internal param context struct */
/*******************************************************/
int PrmSetCtxStr(CPrmHdl PrmHdl, int Idx, int Field, char *Str)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;
	CPrmGenCtx *PtCtx;
	int        Err;
	char       Reason[128];
	char       FuncName[] = "PrmSetCtxStr";

	if(This->WithinCreate==0)
	{
		Err = PRM_WRN_NOT_IN_CREATE;
		sprintf_s(Reason, 128, "Function call (for ParaIdx = %d) not permitted outside PrmCreate context", Idx);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	Err = PrmCheckParaCtxIdentity(This, Idx, Field);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrorHdl, FuncName, "PrmCheckParaCtxIdentity", 0, Err);
		return Err;
	}

	PtCtx = &(This->Context.Ctx[Idx]);
	PtCtx->Len = 1;

	if     (Field==PRM_CST_AKA) strcpy_s(PtCtx->Aka, PRM_CST_PARA_AKA_LENGTH, Str);
	else if(Field==PRM_CST_TXT) strcpy_s(PtCtx->Txt, PRM_CST_PARA_TXT_LENGTH, Str);
	else
	{
		Err = PRM_ERR_FIELD_RANGE;
		sprintf_s(Reason, 128, "Field = (%d) : inconsistent value", Field);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	return 0;
}

/*****************************************************/
/* Set an int field of internal param context struct */
/*****************************************************/
int PrmSetCtxInt(CPrmHdl PrmHdl, int Idx, int Field, int Val)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;	
	CPrmGenCtx *PtCtx;
	int        Err;
	char       Reason[128];
	char       FuncName[] = "PrmSetCtxInt";

	if(This->WithinCreate==0)
	{
		Err = PRM_WRN_NOT_IN_CREATE;
		sprintf_s(Reason, 128, "Function call (for ParaIdx = %d) not permitted outside PrmCreate context", Idx);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	Err = PrmCheckParaCtxIdentity(This, Idx, Field);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrorHdl, FuncName, "PrmCheckParaCtxIdentity", 0, Err);
		return Err;
	}

	PtCtx = &(This->Context.Ctx[Idx]);
	PtCtx->Len = 1;

	if     (Field==PRM_CST_TAG) PtCtx->Tag = Val;
	else if(Field==PRM_CST_TYP) PtCtx->Typ = Val;
	else if(Field==PRM_CST_MIN) PtCtx->Min.Int = Val;
	else if(Field==PRM_CST_MAX) PtCtx->Max.Int = Val;
	else if(Field==PRM_CST_ACC) PtCtx->Acc.Int = Val;
	else if(Field==PRM_CST_DEF) PtCtx->Def.Int = Val;
	else
	{
		Err = PRM_ERR_FIELD_RANGE;
		sprintf_s(Reason, 128, "Field = (%d) : inconsistent value", Field);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	return 0;
}

/******************************************************/
/* Set a float field of internal param context struct */
/******************************************************/
int PrmSetCtxFlt(CPrmHdl PrmHdl, int Idx, int Field, float Val)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;	
	CPrmGenCtx *PtCtx;
	int        Err;
	char       Reason[128];
	char       FuncName[] = "PrmSetCtxFlt";

	if(This->WithinCreate==0)
	{
		Err = PRM_WRN_NOT_IN_CREATE;
		sprintf_s(Reason, 128, "Function call (for ParaIdx = %d) not permitted outside PrmCreate context", Idx);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}
	
	Err = PrmCheckParaCtxIdentity(This, Idx, Field);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrorHdl, FuncName, "PrmCheckParaCtxIdentity", 0, Err);
		return Err;
	}

	PtCtx = &(This->Context.Ctx[Idx]);
	PtCtx->Len = 1;

	if(Field==PRM_CST_MIN) PtCtx->Min.Float = Val;
	else if(Field==PRM_CST_MAX) PtCtx->Max.Float = Val;
	else if(Field==PRM_CST_ACC) PtCtx->Acc.Float = Val;
	else if(Field==PRM_CST_DEF) PtCtx->Def.Float = Val;
	else
	{
		Err = PRM_ERR_FIELD_RANGE;
		sprintf_s(Reason, 128, "Field = (%d) : inconsistent value", Field);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	return 0;
}

/****************************************************************/
/* Set an integer vector field of internal param context struct */
/****************************************************************/
int PrmSetCtxIVc(CPrmHdl PrmHdl, int Idx, int Field, int NbVal, ...)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;	
	CPrmGenCtx *PtCtx;
	int        Err,k;
	int        Val;
	va_list    VarList;
	char       Reason[128];
	char       FuncName[] = "PrmSetCtxIVx";

	if(This->WithinCreate==0)
	{
		Err = PRM_WRN_NOT_IN_CREATE;
		sprintf_s(Reason, 128, "Function call (for ParaIdx = %d) not permitted outside PrmCreate context", Idx);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	Err = PrmCheckParaCtxIdentity(This, Idx, Field);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrorHdl, FuncName, "PrmCheckParaCtxIdentity", 0, Err);
		return Err;
	}

	PtCtx = &(This->Context.Ctx[Idx]);

	va_start(VarList, NbVal);

	PtCtx->Len = NbVal;

	for(k=0; k<NbVal; k++)
	{
		Val = (int)va_arg(VarList, int);
		if(Field==PRM_CST_MIN)      PtCtx->Min.IVec[k] = Val;
		else if(Field==PRM_CST_MAX) PtCtx->Max.IVec[k] = Val;
		else if(Field==PRM_CST_ACC) PtCtx->Acc.IVec[k] = Val;
		else if(Field==PRM_CST_DEF) PtCtx->Def.IVec[k] = Val;
		else
		{
			Err = PRM_ERR_FIELD_RANGE;
			sprintf_s(Reason, 128, "Field = (%d) : inconsistent value", Field);
			ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
			return Err;
		}
	}

	va_end(VarList);

	return 0;
}

/*************************************************************/
/* Set a float vector field of internal param context struct */
/*************************************************************/
int PrmSetCtxFVc(CPrmHdl PrmHdl, int Idx, int Field, int NbVal, ...)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;
	CPrmGenCtx *PtCtx;
	int        Err,k;
	float      Val;
	va_list    VarList;
	char       Reason[128];
	char       FuncName[] = "PrmSetCtxFVx";

	if(This->WithinCreate==0)
	{
		Err = PRM_WRN_NOT_IN_CREATE;
		sprintf_s(Reason, 128, "Function call (for ParaIdx = %d) not permitted outside PrmCreate context", Idx);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	Err = PrmCheckParaCtxIdentity(This, Idx, Field);
	if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrorHdl, FuncName, "PrmCheckParaCtxIdentity", 0, Err);
		return Err;
	}

	PtCtx = &(This->Context.Ctx[Idx]);

	va_start(VarList, NbVal);

	PtCtx->Len = NbVal;

	for(k=0; k<NbVal; k++)
	{
		Val = (float)va_arg(VarList, double);
		if(Field==PRM_CST_MIN)      PtCtx->Min.FVec[k] = Val;
		else if(Field==PRM_CST_MAX) PtCtx->Max.FVec[k] = Val;
		else if(Field==PRM_CST_ACC) PtCtx->Acc.FVec[k] = Val;
		else if(Field==PRM_CST_DEF) PtCtx->Def.FVec[k] = Val;
		else
		{
			Err = PRM_ERR_FIELD_RANGE;
			sprintf_s(Reason, 128, "Field = (%d) : inconsistent value", Field);
			ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
			return Err;
		}
	}

	va_end(VarList);

	return 0;
}

/****************************************************************************************************************/
/* Check if any current parameter bearing a certain tag has changed as compared to a reference para struct      */
/* If at least one such parameter has changed, the index of the first one is returned, otherwise -1 is returned */ 
/****************************************************************************************************************/
int PrmHasAnyTaggedParaChanged(CPrmHdl PrmHdl, CPrmPara *RefPara, int Tag, int* ParaIndexOrMinusOne)
{
	CPrm* This = (CPrm*)PrmHdl.Pv;
	CPrmGenType Ref, Cur;
	int  Nb, ADiff, Err, Type, k, j, NbParaScanned;
	char Reason[128];
	char FuncName[] = "PrmHasAnyTaggedParaChanged";
	
	NbParaScanned = (RefPara->Count <= This->Context.Count) ? RefPara->Count : This->Context.Count;

	for(k = 0; k<NbParaScanned; k++)
	{
		if(This->Context.Ctx[k].Tag != Tag) 
			continue;
		
		Type = This->Context.Ctx[k].Typ;
		Nb   = This->Context.Ctx[k].Len;
		Ref  = RefPara->Val[k];
		Cur  = This->Para.Val[k];
		
		if(Type == PRM_CST_INT)
		{
			if(Ref.Int != Cur.Int) break;
		}
		else if(Type == PRM_CST_FLOAT)
		{
			if(PRM_ABS(Ref.Float - Cur.Float)>FLT_EPSILON) break;
		}
		else if(Type == PRM_CST_IVEC)
		{
			for(ADiff=0, j=0; j<Nb; j++)
				ADiff += (Ref.IVec[j] != Cur.IVec[j]);
			if(ADiff) break;
		}
		else if(Type==PRM_CST_FVEC)
		{
			for(ADiff=0, j=0; j<Nb; j++)
				ADiff += (PRM_ABS(Ref.FVec[j]-Cur.FVec[j])>FLT_EPSILON);
			if(ADiff) break;
		}
	}

	*ParaIndexOrMinusOne = ((k!=NbParaScanned) ? k : -1); // Found diff if break occured (k not incremented to its limit)
		
	if(RefPara->Count != This->Context.Count)
	{
		Err = PRM_WRN_INCORRECT_PARANB;
		printf(Reason, "RefPara number = %d, different from current parameter number = %d", RefPara->Count, This->Context.Count);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, FuncName, Err, Reason);
		return Err;
	}

	return 0;
}

/**************************************/
/* Check an elementary generic contex */
/**************************************/
int PrmCheckGenCtx(CPrm *This, CPrmGenCtx *Ctx)
{
	char Reason[128];
	int  n, VecLen;

	VecLen = Ctx->Len;

	// Checks if all parameters have default contexts defined in OscSetParamCtx.h
	if (Ctx->Aka==0 || Ctx->Aka[0]=='\0')
	{
		sprintf_s(Reason, 128, "Internal Error, uninitialized/missing context in PrmSetParamCtx.h");
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_INTERNAL, Reason);
		return PRM_ERR_INTERNAL;
	}

	if (strlen(Ctx->Aka) > PRM_CST_PARA_AKA_LENGTH) 
	{
		sprintf_s(Reason, 128, "Overflow on Aka length (%d > %d)", strlen(Ctx->Aka), PRM_CST_PARA_AKA_LENGTH);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_AKA_RANGE, Reason);
		return PRM_ERR_AKA_RANGE;
	}

	if (strlen(Ctx->Txt) > PRM_CST_PARA_TXT_LENGTH)
	{
		sprintf_s(Reason, 128, "Overflow on Txt length (%d > %d)", strlen(Ctx->Txt), PRM_CST_PARA_TXT_LENGTH);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_TXT_RANGE, Reason);
		return PRM_ERR_TXT_RANGE;
	}

	if(Ctx->Typ<PRM_CST_INT || Ctx->Typ>PRM_CST_FVEC)
	{
		sprintf_s(Reason, 128, "Typ=%d out of [%d, %d]", Ctx->Typ, PRM_CST_INT, PRM_CST_FVEC);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_TYP_RANGE, Reason);
		return PRM_ERR_TYP_RANGE;
	}

	if(Ctx->Typ==PRM_CST_INT && Ctx->Min.Int>Ctx->Max.Int)
	{
		sprintf_s(Reason, 128, "Min=%d > Max=%d", Ctx->Min.Int, Ctx->Max.Int);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_MINMAX_RANGE, Reason);
		return PRM_ERR_MINMAX_RANGE;
	}

	if(Ctx->Typ==PRM_CST_FLOAT && Ctx->Min.Float>Ctx->Max.Float)
	{
		sprintf_s(Reason, 128, "Min=%f > Max=%f", Ctx->Min.Float, Ctx->Max.Float);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_MINMAX_RANGE, Reason);
		return PRM_ERR_MINMAX_RANGE;
	}

	for(n=0; n<VecLen; n++)
	{
		if(Ctx->Typ==PRM_CST_FVEC && Ctx->Min.FVec[n]>Ctx->Max.FVec[n])
		{
			sprintf_s(Reason, 128, "VecIdx:%d  Min=%f > Max=%f", n, Ctx->Min.FVec[n], Ctx->Max.FVec[n]);
			ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_MINMAX_RANGE, Reason);
			return PRM_ERR_MINMAX_RANGE;
		}
		if(Ctx->Typ==PRM_CST_IVEC && Ctx->Min.IVec[n]>Ctx->Max.IVec[n])
		{
			sprintf_s(Reason, 128, "VecIdx:%d  Min=%d > Max=%d", n, Ctx->Min.IVec[n], Ctx->Max.IVec[n]);
			ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_MINMAX_RANGE, Reason);
			return PRM_ERR_MINMAX_RANGE;
		}
	}

	if(Ctx->Typ==PRM_CST_INT && (Ctx->Def.Int<Ctx->Min.Int || Ctx->Def.Int>Ctx->Max.Int))
	{
		sprintf_s(Reason, 128, "Def=%d not in [%d, %d]", Ctx->Def.Int, Ctx->Min.Int, Ctx->Max.Int);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_DEF_RANGE, Reason);
		return PRM_ERR_DEF_RANGE;
	}

	if(Ctx->Typ==PRM_CST_FLOAT && (Ctx->Def.Float<Ctx->Min.Float || Ctx->Def.Float>Ctx->Max.Float))
	{
		sprintf_s(Reason, 128, "Def=%f not in [%f, %f]", Ctx->Def.Float, Ctx->Min.Float, Ctx->Max.Float);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_DEF_RANGE, Reason);
		return PRM_ERR_DEF_RANGE;
	}

	for(n=0; n<VecLen; n++)
	{
		if(Ctx->Typ==PRM_CST_FVEC && (Ctx->Def.FVec[n]<Ctx->Min.FVec[n] || Ctx->Def.FVec[n]>Ctx->Max.FVec[n]))
		{
			sprintf_s(Reason, 128, "VecIdx:%d  Def=%f not in [%f, %f]", n, Ctx->Def.FVec[n], Ctx->Min.FVec[n], Ctx->Max.FVec[n]);
			ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_DEF_RANGE, Reason);
			return PRM_ERR_DEF_RANGE;
		}
		if(Ctx->Typ==PRM_CST_IVEC && (Ctx->Def.IVec[n]<Ctx->Min.IVec[n] || Ctx->Def.IVec[n]>Ctx->Max.IVec[n]))
		{
			sprintf_s(Reason, 128, "VecIdx:%d  Def=%d not in [%d, %d]", n, Ctx->Def.IVec[n], Ctx->Min.IVec[n], Ctx->Max.IVec[n]);
			ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_DEF_RANGE, Reason);
			return PRM_ERR_DEF_RANGE;
		}
	}

	if(Ctx->Typ==PRM_CST_INT && (Ctx->Acc.Int > (Ctx->Max.Int - Ctx->Min.Int)))
	{
		sprintf_s(Reason, 128, "Acc=%d > Max-Min=%d", Ctx->Acc.Int, (Ctx->Max.Int - Ctx->Min.Int));
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_ACC_RANGE, Reason);
		return PRM_ERR_ACC_RANGE;
	}

	if(Ctx->Typ==PRM_CST_FLOAT && (Ctx->Acc.Float > (Ctx->Max.Float - Ctx->Min.Float)))
	{
		sprintf_s(Reason, 128, "Acc=%f > Max-Min=%f", Ctx->Acc.Float, (Ctx->Max.Float - Ctx->Min.Float));
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_ACC_RANGE, Reason);
		return PRM_ERR_ACC_RANGE;
	}

	for(n=0; n<VecLen; n++)
	{
		if(Ctx->Typ==PRM_CST_FVEC && (Ctx->Acc.FVec[n] > (Ctx->Max.FVec[n] - Ctx->Min.FVec[n])))
		{
			sprintf_s(Reason, 128, "VecIdx:%d  Acc=%f > Max-Min=%f", n, Ctx->Acc.FVec[n], (Ctx->Max.FVec[n] - Ctx->Min.FVec[n]));
			ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_ACC_RANGE, Reason);
			return PRM_ERR_ACC_RANGE;
		}	
		if(Ctx->Typ==PRM_CST_IVEC && (Ctx->Acc.IVec[n] > (Ctx->Max.IVec[n] - Ctx->Min.IVec[n])))
		{
			sprintf_s(Reason, 128, "VecIdx:%d  Acc=%d > Max-Min=%d", n, Ctx->Acc.IVec[n], (Ctx->Max.IVec[n] - Ctx->Min.IVec[n]));
			ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckGenCtx", PRM_ERR_ACC_RANGE, Reason);
			return PRM_ERR_ACC_RANGE;
		}
	}
	return 0;
}

/*************************************/
/* Check the param context structure */
/*************************************/
int PrmCheckParaCtx(CPrmHdl PrmHdl)
{
	CPrm *This = (CPrm*)PrmHdl.Pv;
	int Idx, Err;
	char Reason[128];
	CPrmParaCtx *Ctx = &(This->Context);

	for(Idx=0; Idx<Ctx->Count; Idx++)
	{
		Err = PrmCheckGenCtx(This, &(Ctx->Ctx[Idx]));
		if(Err)
		{
			sprintf_s(Reason, 128, "Index=%d, Aka=%s", Idx, Ctx->Ctx[Idx].Aka);
			ErrAppendErrorReportFromCaller(This->ErrorHdl, "PrmCheckParaCtx", "PrmCheckGenCtx", Reason, Err);
			return Err;
		}
	}

	return 0;
}

/**************************/
/*  Get parameter context */
/**************************/
int PrmGetParaContext(CPrmHdl PrmHdl, CPrmParaCtx *ParaCtx)
{	
	CPrm *This = (CPrm*)PrmHdl.Pv;

	*ParaCtx = This->Context; // Deep copy

	return 0;
}

/**************************************************/
/* Get the default parameters values from context */
/**************************************************/
int PrmGetDefaultPara(CPrmHdl PrmHdl, CPrmPara *Para)
{	
	CPrm         *This = (CPrm*)PrmHdl.Pv;
	CPrmParaCtx  *ParaCtx;
	int          Idx;

	ParaCtx = &(This->Context);
	Para->Count = ParaCtx->Count;

	for(Idx=0; Idx<ParaCtx->Count; Idx++)
		Para->Val[Idx] = ParaCtx->Ctx[Idx].Def;

	return 0;
}

/*************************************/
/* Get the current parameters values */
/*************************************/
int PrmGetPara(CPrmHdl PrmHdl, CPrmPara *Para)
{	
	CPrm *This = (CPrm*)PrmHdl.Pv;
	*Para = This->Para;
	return 0;
}

/***********************************/
/* Set parameter to default values */
/***********************************/
int PrmSetParaToDefault(CPrmHdl PrmHdl)
{	
	CPrm *This = (CPrm*)PrmHdl.Pv;
	PrmGetDefaultPara(PrmHdl, &(This->Para));
	return 0;
}

/*************************************************************************************/
/* Print parameter values + context (just a check param VS context with Verbose on)  */
/*************************************************************************************/
int PrmPrintParaWithContext(CPrmHdl PrmHdl)
{
	CPrm        *This = (CPrm*)PrmHdl.Pv;
	CPrmGenCtx  *GenCtx;
	CPrmPara    Para;
	int         k,n,VecLen;

	PrmGetPara(PrmHdl, &Para);

	for(k=0; k<This->Context.Count; k++)
	{
		GenCtx = &(This->Context.Ctx[k]);
		printf("\nPara : %s\n", GenCtx->Aka);
		printf("Txt  : %s\n", GenCtx->Txt);
		printf("Tag  : %d\n", GenCtx->Tag);
		printf("Typ  : %d\n", GenCtx->Typ);

		if(GenCtx->Typ==PRM_CST_INT)
		{
			printf("Min  : %d\n", GenCtx->Min.Int);
			printf("Max  : %d\n", GenCtx->Max.Int);
			printf("Acc  : %d\n", GenCtx->Acc.Int);
			printf("Def  : %d\n", GenCtx->Def.Int);
			// Param val
			printf("\nVal  : %d\n", Para.Val[k].Int);
		}
		else if (GenCtx->Typ==PRM_CST_FLOAT)
		{
			printf("Min  : %f\n", GenCtx->Min.Float);
			printf("Max  : %f\n", GenCtx->Max.Float);
			printf("Acc  : %f\n", GenCtx->Acc.Float);
			printf("Def  : %f\n", GenCtx->Def.Float);
			// Param val
			printf("\nVal  : %f\n", Para.Val[k].Float);

		}
		else if (GenCtx->Typ==PRM_CST_FVEC)
		{
			VecLen = GenCtx->Len;
			printf("Min  :");   for(n=0; n<VecLen; n++) printf(" %f ", GenCtx->Min.FVec[n]);
			printf("\nMax  :"); for(n=0; n<VecLen; n++) printf(" %f ", GenCtx->Max.FVec[n]);
			printf("\nAcc  :"); for(n=0; n<VecLen; n++) printf(" %f ", GenCtx->Acc.FVec[n]);
			printf("\nDef  :"); for(n=0; n<VecLen; n++) printf(" %f ", GenCtx->Def.FVec[n]);
			printf("\n");
			// Param val
			printf("\nVal  :"); for(n=0; n<VecLen; n++) printf(" %f ", Para.Val[k].FVec[n]);
			printf("\n");
		}
		else if (GenCtx->Typ==PRM_CST_IVEC)
		{
			VecLen = GenCtx->Len;
			printf("Min  :");   for(n=0; n<VecLen; n++) printf(" %d ", GenCtx->Min.IVec[n]);
			printf("\nMax  :"); for(n=0; n<VecLen; n++) printf(" %d ", GenCtx->Max.IVec[n]);
			printf("\nAcc  :"); for(n=0; n<VecLen; n++) printf(" %d ", GenCtx->Acc.IVec[n]);
			printf("\nDef  :"); for(n=0; n<VecLen; n++) printf(" %d ", GenCtx->Def.IVec[n]);
			printf("\n");
			// Param val
			printf("\nVal  :"); for(n=0; n<VecLen; n++) printf(" %d ", Para.Val[k].IVec[n]);
			printf("\n");
		}
	}

	return 0;
}

/*************************************************************************************/
// Print parameter name + value  
/*************************************************************************************/
int PrmPrintParaCurVal(CPrmHdl PrmHdl)
{
	CPrm        *This = (CPrm*)PrmHdl.Pv;
	CPrmGenCtx  *GenCtx;
	CPrmPara    Para;
	int         k, n, VecLen, MaxAkaLength, CurAkaLength;
	char        Format[32];
	char        FuncName[] = "PrmPrintParaCurVal";

	PrmGetPara(PrmHdl, &Para);

	// Getting max-aka string length for nice aligned printing
	for(MaxAkaLength=0, k=0; k<This->Context.Count; k++)
	{
		CurAkaLength = (int)strlen(This->Context.Ctx[k].Aka);
		MaxAkaLength = ((CurAkaLength > MaxAkaLength) ? CurAkaLength : MaxAkaLength);
	}
	sprintf_s(Format, 32, "Para: %%-%ds : ", MaxAkaLength);
	
	printf("\n%s Start\n\n", FuncName);
	for(k=0; k<This->Context.Count; k++)
	{
		GenCtx = &(This->Context.Ctx[k]);
		printf(Format, GenCtx->Aka);

		if(GenCtx->Typ==PRM_CST_INT)
		{
			// Param val
			printf(" %d\n", Para.Val[k].Int);
		}
		else if (GenCtx->Typ==PRM_CST_FLOAT)
		{
			// Param val
			printf(" %f\n", Para.Val[k].Float);

		}
		else if (GenCtx->Typ==PRM_CST_FVEC)
		{
			VecLen = GenCtx->Len;
			// Param val
			printf(" "); for(n=0; n<VecLen; n++) printf("%f ", Para.Val[k].FVec[n]);
			printf("\n");
		}
		else if (GenCtx->Typ==PRM_CST_IVEC)
		{
			VecLen = GenCtx->Len;
			// Param val
			printf(" "); for(n=0; n<VecLen; n++) printf("%d ", Para.Val[k].IVec[n]);
			printf("\n");
		}
	}
	printf("\n%s End\n\n", FuncName);

	return 0;
}

/*************************************************************************************/
// Check param values versus context (checking if values are within ranges) 
/*************************************************************************************/
int PrmCheckParaVsCtx(CPrm *This, CPrmPara *Para)
{
	int k, n, VecLen;
	char Message[512], Append[512];
	CPrmParaCtx *ParaCtx;
	CPrmGenCtx  *GenCtx;

	ParaCtx = &(This->Context);

	if(Para->Count != ParaCtx->Count)
	{
		sprintf_s(Message, 512, "%d para in Context != %d para\n", ParaCtx->Count, Para->Count);
		ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaVsCtx", PRM_WRN_INCORRECT_PARANB, Message);
		return PRM_WRN_INCORRECT_PARANB;
	}

	for(k=0; k<Para->Count; k++)
	{
		GenCtx = &(ParaCtx->Ctx[k]);
		if(GenCtx->Typ== PRM_CST_INT)
		{
			if(Para->Val[k].Int < GenCtx->Min.Int || Para->Val[k].Int > GenCtx->Max.Int)
			{
				sprintf_s(Message, 512, "%s : %s\n", GenCtx->Aka, GenCtx->Txt);
				sprintf_s(Append, 512, "Min = %d  Max = %d  Accuracy = %d  Default = %d  ", GenCtx->Min.Int, GenCtx->Max.Int, GenCtx->Acc.Int, GenCtx->Def.Int);
				strcat_s(Message, 512, Append);
				sprintf_s(Append, 512, "Val = %d\n", Para->Val[k].Int);
				strcat_s(Message, 512, Append);
				strcat_s(Message, 512, "Value not in [Min, Max]\n");
				ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaVsCtx", PRM_WRN_INT_RANGE, Message);
				return PRM_WRN_INT_RANGE;
			}
		}
		else if(GenCtx->Typ== PRM_CST_FLOAT)
		{
			if(Para->Val[k].Float < GenCtx->Min.Float || Para->Val[k].Float > GenCtx->Max.Float)
			{
				sprintf_s(Message, 512, "%s : %s\n", GenCtx->Aka, GenCtx->Txt);
				sprintf_s(Append, 512, "Min = %f  Max = %f  Accuracy = %f  Default = %f  ", 
					GenCtx->Min.Float, GenCtx->Max.Float, GenCtx->Acc.Float, GenCtx->Def.Float);
				strcat_s(Message, 512, Append);
				sprintf_s(Append, 512, "Val = %f\n", Para->Val[k].Float);
				strcat_s(Message, 512, Append);
				strcat_s(Message, 512, "Value not in [Min, Max]\n");
				ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaVsCtx", PRM_WRN_FLOAT_RANGE, Message);
				return PRM_WRN_FLOAT_RANGE;
			}
		}
		else if(GenCtx->Typ== PRM_CST_FVEC)
		{
			VecLen = GenCtx->Len;
			for(n=0; n<VecLen; n++) 
			{
				if(Para->Val[k].FVec[n] < GenCtx->Min.FVec[n] || Para->Val[k].FVec[n] > GenCtx->Max.FVec[n])
				{
					sprintf_s(Message, 512, "%s[%d]: %s\n", GenCtx->Aka, n, GenCtx->Txt);
					sprintf_s(Append, 512, "Min = %f  Max = %f  Accuracy = %f  Default = %f  ", 
						GenCtx->Min.FVec[n], GenCtx->Max.FVec[n], GenCtx->Acc.FVec[n], GenCtx->Def.FVec[n]);
					strcat_s(Message, 512, Append);
					sprintf_s(Append, 512, "Val = %f\n", Para->Val[k].FVec[n]);
					strcat_s(Message, 512, Append);
					strcat_s(Message, 512, "Value not in [Min, Max]\n");
					ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaVsCtx", PRM_WRN_FVEC_RANGE, Message);
					return PRM_WRN_FVEC_RANGE;
				}
			}
		}
		else if(GenCtx->Typ== PRM_CST_IVEC)
		{
			VecLen = GenCtx->Len;
			for(n=0; n<VecLen; n++) 
			{
				if(Para->Val[k].IVec[n] < GenCtx->Min.IVec[n] || Para->Val[k].IVec[n] > GenCtx->Max.IVec[n])
				{
					sprintf_s(Message, 512, "%s[%d]: %s\n", GenCtx->Aka, n, GenCtx->Txt);
					sprintf_s(Append, 512, "Min = %d  Max = %d  Accuracy = %d  Default = %d  ", 
						GenCtx->Min.IVec[n], GenCtx->Max.IVec[n], GenCtx->Acc.IVec[n], GenCtx->Def.IVec[n]);
					strcat_s(Message, 512, Append);
					sprintf_s(Append, 512, "Val = %d\n", Para->Val[k].IVec[n]);
					strcat_s(Message, 512, Append);
					strcat_s(Message, 512, "Value not in [Min, Max]\n");
					ErrStartErrorReportFromLeaf(This->ErrorHdl, "PrmCheckParaVsCtx", PRM_WRN_IVEC_RANGE, Message);
					return PRM_WRN_FLOAT_RANGE;
				}
			}
		}
	}

	return 0;
}

/*************************************************************************************/
// Set parameter from provided struct 
/*************************************************************************************/
int PrmSetPara(CPrmHdl PrmHdl, CPrmPara *Para)
{
	CPrm  *This = (CPrm*)PrmHdl.Pv;
	int   Err;

	// Checking parameter ranges
	Err = PrmCheckParaVsCtx(This, Para);
	if(Err) 
	{	
		ErrAppendErrorReportFromCaller(This->ErrorHdl, "PrmSetPara", "PrmCheckParaVsCtx", 0, Err); 
		return Err;
	}

	// Copy from provided struct to library para
	This->Para = *Para; // Deep Copy

	return 0;
}











