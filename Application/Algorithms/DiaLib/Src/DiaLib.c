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

#include <DiaLibDev.h> 
#include <DiaErrorMessages.h> 

int DiaCreate(CDiaHdl *PtrDiaHdl, int IwOri, int IhOri, int SSFactor){
    return DiaCreateGivenWorkingImSize(PtrDiaHdl, IwOri, IhOri, IwOri / SSFactor, IhOri / SSFactor);
}
	
int DiaCreateGivenWorkingImSize(CDiaHdl *PtrDiaHdl, int IwOri, int IhOri, int IwProc1, int IhProc1){
	CDia			*This;
	char			Reason[128];
	int				Err;

	// Handle skeletton
	This = (CDia*)malloc(sizeof(CDia));
    if (This == NULL){
        printf("DiaCreateGivenWorkingImSize:could not allocate This");
        return -1;
    }

	PtrDiaHdl->Pv	= This;

	// Init error management material
	Err = ErrCreate(&(PtrDiaHdl->ErrHdl));
	if (Err){
		printf("Error in ErrCreate call (function OscCreate)\n");
		return DIA_ERR_ORIGIN;
	}
	Err = ErrAdd(PtrDiaHdl->ErrHdl, DiaErrorArray, DIA_ERR_LAST-DIA_ERR_FIRST-1);
	if (Err){
		ErrPrintErrorReport(PtrDiaHdl->ErrHdl);
		printf("Error in ErrAdd call (function DiaCreate)\n");
		return DIA_ERR_ORIGIN;
	}
	
	// Internal material
	This->DiaProc.IwOriAlloc = IwOri;
	This->DiaProc.IwSS1Alloc = IwProc1;
	This->DiaProc.IhSS1Alloc = IhProc1;
	This->DiaProc.IwSS1 = IwProc1;
	This->DiaProc.IhSS1 = IhProc1;
	This->DiaProc.IwSS2 = This->DiaProc.IwSS1 /EXTRA_SSFACTOR_HOUGH;
	This->DiaProc.IhSS2 = This->DiaProc.IhSS1 /EXTRA_SSFACTOR_HOUGH;

	This->DiaProc.hist  = (long*) malloc(NB_BINS_PERCENTILE * sizeof(long));
	if (This->DiaProc.hist == NULL){
		sprintf_s(Reason, 128, "Could not allocate hist.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}

	This->DiaProc.ImSubSS2 = (short*)malloc(This->DiaProc.IwSS2*This->DiaProc.IhSS2*sizeof(short));
	if (This->DiaProc.ImSubSS2 == NULL){
		sprintf_s(Reason, 128, "Could not allocate ImSubSS2.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}
	This->DiaProc.gradImSS2 = (float*)malloc(This->DiaProc.IwSS2*This->DiaProc.IhSS2*sizeof(float));
	if (This->DiaProc.gradImSS2 == NULL){
		sprintf_s(Reason, 128, "Could not allocate gradImSS2.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}
	
	This->DiaProc.BufSS2F = (float*)malloc(This->DiaProc.IwSS2*This->DiaProc.IhSS2*sizeof(float));
	if (This->DiaProc.BufSS2F == NULL){
		sprintf_s(Reason, 128, "Could not allocate BufSS2F.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}
	
	This->DiaProc.BufS1S = (short*)malloc(This->DiaProc.IwSS1*This->DiaProc.IhSS1*sizeof(short));
	if (This->DiaProc.BufS1S == NULL){
		sprintf_s(Reason, 128, "Could not allocate BufS1S.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}

	This->DiaProc.AccumulationMatrix = (float*)malloc(NB_CELLS_PER_DIM_HOUGHA *NB_CELLS_PER_DIM_HOUGH *NB_CELLS_PER_DIM_HOUGH *sizeof(float));
	if (This->DiaProc.AccumulationMatrix == NULL){
		sprintf_s(Reason, 128, "Could not allocate AccumulationMatrix.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}
	This->DiaProc.AccumulationMatrix1D = (float*)malloc(NB_CELLS_PER_DIM_HOUGH *sizeof(float));
	if (This->DiaProc.AccumulationMatrix1D == NULL){
		sprintf_s(Reason, 128, "Could not allocate AccumulationMatrix1D.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}
	
	This->DiaProc.cont  = (float*) malloc(IwOri * sizeof(float));

	if (This->DiaProc.cont == NULL){
		sprintf_s(Reason, 128, "Could not allocate cont.");
		ErrStartErrorReportFromLeaf(PtrDiaHdl->ErrHdl, "DiaCreate", DIA_ERR_MEM_ALLOC, Reason);
		return DIA_ERR_MEM_ALLOC;
	}

	return 0;
}

int DiaDelete(CDiaHdl DiaHdl){

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int Err;

	if (DiaProc->hist != NULL){free(DiaProc->hist); DiaProc->hist = NULL;}
	if (DiaProc->gradImSS2 != NULL){free(DiaProc->gradImSS2); DiaProc->gradImSS2 = NULL;}
	if (DiaProc->ImSubSS2 != NULL){free(DiaProc->ImSubSS2); DiaProc->ImSubSS2 = NULL;}
	if (DiaProc->BufSS2F != NULL){free(DiaProc->BufSS2F); DiaProc->BufSS2F = NULL;}
	if (DiaProc->BufS1S != NULL){free(DiaProc->BufS1S); DiaProc->BufS1S = NULL;}
	if (DiaProc->AccumulationMatrix != NULL){free(DiaProc->AccumulationMatrix); DiaProc->AccumulationMatrix = NULL;}
	if (DiaProc->AccumulationMatrix1D != NULL){free(DiaProc->AccumulationMatrix1D); DiaProc->AccumulationMatrix1D = NULL;}
	if (DiaProc->cont != NULL){free(DiaProc->cont); DiaProc->cont = NULL;}

	Err = ErrDelete(DiaHdl.ErrHdl);
	if (Err)
	{
		ErrAppendErrorReportFromCaller(DiaHdl.ErrHdl, "DiaDelete", "ErrDelete", 0, Err);
	}

	free(This);

	return 0;
}

/*************************************************************************************/
// Return current DiaLib version numbers
/*************************************************************************************/
int DiaGetVersionNumbers(int* Major, int* Minor, int* Patch)
{
	*Major = DIA_VERSION_MAJOR; *Minor = DIA_VERSION_MINOR; *Patch = DIA_VERSION_PATCH;
	return 0;
}