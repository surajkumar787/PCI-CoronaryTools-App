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

#include <OscDev.h>
#include <OscVersion.h>
#include <OscErrorMessages.h>

/*************************************************************************************/
// Create an Oscar process : valid for a given image format
/*************************************************************************************/
int OscCreate(COscHdl* PtOscHdl, int ImaDyn, int ImaWidthAlloc, int ImaHeightAlloc, int ImaWidthSubAlloc, int ImaHeightSubAlloc
              , float maxFrameRate, float maxAngioDuration, int maxNbProc)
{
	COsc			*This;
	int   Err = 0, ErrorCodeOut = 0;
	int   Major, Minor, Patch;
	char  Reason[128];
	char  FuncName[] ="OscCreate";

    

    // Allocating an Osc skeleton
	OSC_MALLOC(This, 1);
    if (This == NULL){
        printf("OscCreate: could not allocate This\n");
        return -1;
    }
	PtOscHdl->Pv = This;

	// Setting OscLib version material
	OscSetVersion(This);

	// Checking ErrLib version before library instantiation
	Err = ErrGetVersionNumbers(&Major, &Minor, &Patch);
	if (Err){
		printf("Error in ErrGetVersionNumbers call (function OscCreate)\n");
		return OSC_ERR_ORIGIN;
	}
	if(Major != ERR_VERSION_MAJOR || Minor != ERR_VERSION_MINOR)
	{
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128, "ErrLib V.%d.%d.* not compatible with expected V.%d.%d.*", 
				Major, Minor, ERR_VERSION_MAJOR, ERR_VERSION_MINOR);
		ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, OSC_ERR_DLL_VERSION, Reason);
		return OSC_ERR_DLL_VERSION;
	}

	// Init error management material
	Err = ErrCreate(&(This->ErrHdl));
	if (Err){
		printf("Error in ErrCreate call (function OscCreate)\n");
		return OSC_ERR_ORIGIN;
	}
	Err = ErrAdd(This->ErrHdl, OscErrorArray, OSC_ERR_LAST-OSC_ERR_FIRST-1);
	if (Err){
		ErrPrintErrorReport(This->ErrHdl);
		printf("Error in ErrAdd call (function OscCreate)\n");
		return OSC_ERR_ORIGIN;
	}
	PtOscHdl->ErrHdl = This->ErrHdl;

	// Checking PrmLib version before instantiation
	Err = PrmGetVersionNumbers(&Major, &Minor, &Patch);
	if (Err){
		ErrPrintErrorReport(This->ErrHdl);
		printf("Error in PrmGetVersionNumbers call (function OscCreate)\n");
		return OSC_ERR_ORIGIN;
	}
	if(Major != PRM_VERSION_MAJOR || Minor != PRM_VERSION_MINOR)
	{
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128, "PrmLib V.%d.%d.* not compatible with expected V.%d.%d.*", 
				Major, Minor, PRM_VERSION_MAJOR, PRM_VERSION_MINOR);
		ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, OSC_ERR_DLL_VERSION, Reason);
		return OSC_ERR_DLL_VERSION;
	}

	// Instantiate parameter library
	Err = PrmCreate(&(This->PrmHdl), This->ErrHdl, OSC_X_NBPARA, OscSetParaCtx);
	if (Err){
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(This->PrmHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "PrmCreate", OSC_ERR_PRMLIB, "Error in PrmHdl");
		return OSC_ERR_PRMLIB;
	}
	PtOscHdl->PrmHdl = This->PrmHdl;

	// Get default parameters
	Err = PrmGetDefaultPara(This->PrmHdl, &This->ExtractParam);
	if (Err){
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(This->PrmHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "PrmGetDefaultPara", OSC_ERR_PRMLIB, "Error in PrmHdl");
		return OSC_ERR_PRMLIB;
	}

	// Argument checking
	if(ImaDyn<OSC_CST_MIN_IMADYN || ImaDyn>OSC_CST_MAX_IMADYN)
	{
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128, "ImaDyn argument = %d : out of range [%d %d]", ImaDyn, OSC_CST_MIN_IMADYN, OSC_CST_MAX_IMADYN);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscCreate", OSC_ERR_IMADYN_RANGE, Reason);
		return OSC_ERR_IMADYN_RANGE;
	}


	// Data dimension init
	OscDefaultInternalParameters(PtOscHdl);
    OscReInitializeInternalParameters(PtOscHdl);

	// Read parameters
	Err = OscSetExtractProcFromExternalParam(This, 0);
	if(Err){
		int Fatal; 
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscCreate", "OscSetExtractProcFromExternalParam", 0, Err); 
		ErrIsFatal(This->ErrHdl, Err, &Fatal); 
		if (Fatal){This->ExtractProc.FatalErrorOnPreviousImage = 1; return Err; } else {ErrorCodeOut = Err;} 
	}

	// Memory allocations at t=0
	Err = OscMemoryDesallocation(*PtOscHdl);
	if(Err){
		int Fatal; 
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscCreate", "OscMemoryDesallocation", 0, Err); 
		ErrIsFatal(This->ErrHdl, Err, &Fatal); 
		if (Fatal){This->ExtractProc.FatalErrorOnPreviousImage = 1; return Err; } else {ErrorCodeOut = Err;} 
	}

    {
	    Err = OscMemoryAllocationGivenSize(*PtOscHdl, ImaWidthAlloc, ImaHeightAlloc, ImaWidthSubAlloc, ImaHeightSubAlloc
                                            , maxFrameRate, maxAngioDuration, maxNbProc, OSC_NB_PYR);
	    if(Err){
		    int Fatal; 
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "OscCreate", "OscMemoryAllocation", 0, Err); 
		    ErrIsFatal(This->ErrHdl, Err, &Fatal); 
		    if (Fatal){This->ExtractProc.FatalErrorOnPreviousImage = 1; return Err; } else {ErrorCodeOut = Err;} 
	    }
    }

	return ErrorCodeOut;
} 

/*************************************************************************************/
// Set variables to default values
/*************************************************************************************/
void OscDefaultInternalParameters(COscHdl* PtOscHdl){
    
	COsc* This							= (COsc*)PtOscHdl->Pv;
	COscExtractProc     *ExtractProc	= &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &This->ExtractSequence;
	int					k;

    ExtractProc->FatalErrorOnPreviousImage = 0;
    ExtractSequence->injectionCoefficientThreshold = 0.5f;

    ExtractProc->IwOriRawAngio = -1;
    ExtractProc->IhOriRawAngio = -1;
    ExtractProc->IwOriRawFluoro = -1;
    ExtractProc->IhOriRawFluoro = -1;
	ExtractProc->IwOriAngio = -1; 
	ExtractProc->IhOriAngio = -1; 
	ExtractProc->IwOriFluoro = -1; 
	ExtractProc->IhOriFluoro = -1; 
	ExtractProc->IwProcAngio = -1; 
	ExtractProc->IhProcAngio = -1;
	ExtractProc->IwProcFluoro = -1; 
	ExtractProc->IhProcFluoro = -1; 
    ExtractProc->SSFactor = -1;
    ExtractProc->frameRateAngio = -1;
    ExtractProc->frameRateFluoro = -1;
    ExtractProc->CircBufferSize = -1;
    ExtractProc->NbProc = -1;
    ExtractProc->SWHdlCreatedFlag = 0;
    
	ExtractProc->CardiacCycleMaxFrames = 0;

	ExtractProc->isoCenterPixSize = -1;
	ExtractProc->CArmAnglesAngioGiven = 0; ExtractProc->CArmAnglesFluoroGiven = 0;
	ExtractProc->CArmAnglesAngioGivenRef = 0; ExtractProc->CArmAnglesFluoroGivenRef = 0;
    
	ExtractProc->ShuttersRoiPosGiven = 0;
	ExtractProc->ShuttersRoiFullScale.XMin = -1; ExtractProc->ShuttersRoiFullScale.XMax = -1;
	ExtractProc->ShuttersRoiFullScale.YMin = -1; ExtractProc->ShuttersRoiFullScale.YMax = -1;
	
	ExtractProc->DiaHdlCreatedFlag	= 0;

	ExtractProc->CBDLibCreatedFlag = 0;
    
	ExtractProc->TableInformationGiven = 0;
	ExtractProc->tableInfoFromtheStart = 0;

	// Pointers init
	ExtractProc->ImOriWorking		= NULL;
	ExtractProc->ImSub				= NULL;
	ExtractProc->ImRdg				= NULL;
	ExtractProc->ImDir				= NULL;
	ExtractProc->ImCathPotential	= NULL;
	ExtractProc->ImBkg				= NULL;
	ExtractProc->ImFrg				= NULL;
	ExtractProc->ImSee				= NULL; 
	ExtractProc->ImThr				= NULL;
	ExtractProc->ImMask				= NULL;
	ExtractProc->ImLab				= NULL;
	ExtractProc->BufsF				= NULL;
	ExtractProc->BufsS				= NULL;
	ExtractProc->BufsI				= NULL;
	ExtractProc->BufsUC				= NULL;
	ExtractProc->ImCircBuffer		= NULL;
    ExtractProc->MaskSWStored		= NULL;

    ExtractSequence->ExtractPictures = NULL;
    ExtractSequence->MatchingScores = NULL;
    ExtractSequence->dXAngioToAngio = NULL;
    ExtractSequence->dYAngioToAngio = NULL;

    ExtractProc->ViterbiStateScore = NULL;
    ExtractProc->FluoroToAngioScore = NULL;
    for (k=0; k<OSC_VITERBI_FLUORO_DEPTH; k++){ ExtractProc->FluoroToAngioDX[k] = NULL; ExtractProc->FluoroToAngioDY[k] = NULL;}

	if (ExtractProc->ImCircBuffer != NULL){
        int maxCircBufferSize = (int)ceil(OSC_INTEGRATION_TIME_CIRC_BKG * ExtractProc->maxFrameRateAlloc);
		for (k=0; k<maxCircBufferSize; k++)
			if (ExtractProc->ImCircBuffer[k] != NULL){
				free(ExtractProc->ImCircBuffer[k]); ExtractProc->ImCircBuffer[k] = NULL;
			}
		free(ExtractProc->ImCircBuffer); ExtractProc->ImCircBuffer = NULL;
	}

	ExtractSequence->PyrW = NULL; ExtractSequence->PyrH = NULL;
    ExtractProc->PyrWFluoro = NULL; ExtractProc->PyrHFluoro = NULL;

	ExtractProc->BuffOverlayMask = NULL;
	ExtractProc->BuffPyrDir1D = NULL;
	ExtractProc->ImFluoroDirPyr1D = NULL;

	ExtractProc->BufVectF1 = NULL;
	ExtractProc->BufVectF2 = NULL;
	ExtractProc->BufStack1 = NULL;
	ExtractProc->BufStack2 = NULL;
	ExtractProc->BufVectUC = NULL;
	ExtractProc->BufVectI1 = NULL;
	ExtractProc->BufVectI2 = NULL;
    for (k=0; k<NB_EXTRACTED_CATHINJROI_MAX; k++){ ExtractProc->cumRdgRegions[k] = NULL; ExtractProc->cumRdgRegionsFiltered[k] = NULL;}

    ExtractSequence->ExtractPictures = NULL;
    ExtractSequence->MatchingScores = NULL;
    ExtractSequence->dXAngioToAngio = NULL;
    ExtractSequence->dYAngioToAngio = NULL;
    ExtractProc->ViterbiStateScore = NULL;

    //ExtractSequence->LabNbElts = NULL;
    ExtractProc->nbThresholdsForMedianComputation = 0;
    ExtractProc->storedThresholdsBuf1 = NULL;
    ExtractProc->storedThresholdsBuf2 = NULL;

    ExtractSequence->maxImageNbCycle = 0;
    ExtractSequence->storeCycleShort = NULL;

#ifdef OSC_CLEAN_DEBUG
    ExtractSequence->nbSelectedCathInjROI = 0;
    ExtractSequence->ImLabOverSeq = NULL; ExtractSequence->ImLabOverSeqAfterRdgThres = NULL;
    ExtractSequence->ImLabLargestRegions = NULL; ExtractSequence->ImLabAfterClustering = NULL;
#endif 
}

void OscReInitializeInternalParameters(COscHdl* PtOscHdl){

	COsc* This							= (COsc*)PtOscHdl->Pv;
	COscExtractProc     *ExtractProc	= &(This->ExtractProc);
	
	ExtractProc->t = -1; 
	ExtractProc->angioProcessed = 0;
	ExtractProc->detectedCatheter = 0;
    ExtractProc->FatalErrorOnPreviousImage = 0;
}

/*************************************************************************************/
// Set version info material
/*************************************************************************************/
int OscSetVersion(COsc *This)
{
	int  NbChar;
	char *Format;

	NbChar = (int)(strlen(OSC_VERSION_HEADER) + strlen(OSC_VERSION_INFO) + strlen(OSC_CONFIG) + 16);
	OSC_MALLOC(This->VersionInfo    , NbChar);
	OSC_MALLOC(This->VersionInfoCopy, NbChar);
	OSC_MALLOC(Format               , NbChar);
    if (Format == NULL || This->VersionInfo == NULL || This->VersionInfoCopy == NULL){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSetVersion", OSC_ERR_MEM_ALLOC, "Allocation impossible");
		return OSC_ERR_MEM_ALLOC;
    }

	sprintf_s(Format, NbChar, "%s%s", OSC_VERSION_HEADER, OSC_VERSION_INFO);
	sprintf_s(This->VersionInfo, NbChar, Format, OSC_CONFIG, OSC_VERSION_MAJOR, OSC_VERSION_MINOR, OSC_VERSION_PATCH);

	free(Format);
	return 0;
}

/*************************************************************************************/
// Return current OscLib version numbers
/*************************************************************************************/
int OscGetVersionNumbers(int* Major, int* Minor, int* Patch)
{
	*Major = OSC_VERSION_MAJOR; *Minor = OSC_VERSION_MINOR; *Patch = OSC_VERSION_PATCH;
	return 0;
}

/*************************************************************************************/
// Delete an Oscar process : free everything
/*************************************************************************************/
int OscGetVersionInfo(COscHdl OscHdl, char** VersionInfo){
	COsc* This = (COsc*)OscHdl.Pv;
    int NbChar = (int)(strlen(OSC_VERSION_HEADER) + strlen(OSC_VERSION_INFO) + strlen(OSC_CONFIG) + 16);

	strcpy_s(This->VersionInfoCopy,NbChar, This->VersionInfo);
	*VersionInfo = This->VersionInfoCopy;
	return 0;
}

/*************************************************************************************/
// Set PrmLib Parameters Contexts 
/*************************************************************************************/
int OscSetParaCtx(CPrmHdl P)
{
   int  E = 0;
   char FuncName[] ="OscSetParaCtx";

#include "OscSetParamCtx.h"

   if(E) 
      ErrAppendErrorReportFromCaller(P.ErrHdl, FuncName, "PrmSetCtx???", 0, E);
   return E;
}

/*************************************************************************************/
// Delete an Oscar process : free everything
/*************************************************************************************/
int OscDelete(COscHdl OscHdl)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	int					Err;

	// Free image memory
	OscMemoryDesallocation(OscHdl);

	// Free version info
	free(This->VersionInfo);
	free(This->VersionInfoCopy);

	Err = PrmDelete(This->PrmHdl);
	if (Err){
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrPrintErrorReport(This->PrmHdl.ErrHdl);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "PrmGetDefaultPara", OSC_ERR_PRMLIB, "Error in PrmHdl");
		return OSC_ERR_PRMLIB;
	}
	Err |= ErrDelete(This->ErrHdl);
	if (Err){
		printf("Error in ErrDelete call (function OscDelete)\n");
		return OSC_ERR_ORIGIN;
	}

	free(This);

	return 0;
} 