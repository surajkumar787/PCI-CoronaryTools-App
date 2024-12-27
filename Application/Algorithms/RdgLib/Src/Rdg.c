// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <stdio.h> 
#include <string.h> 
#include <math.h> 
#include <windows.h> 
#include <RdgDev.h>
#include <RdgVersion.h>



/*************************************************************************************/
// Create a ridge extraction process : valid for a given image format
/*************************************************************************************/
int RdgCreate(CRdgHdl* PtRdgHdl, CRdgExtractParam *ExtractParam, int ImaDyn, int ImaWidth, int ImaHeight, int SSFactor)
{
    CRdg*   This;
    CRdgHdl RdgHdl;
    int     IwProc, IhProc, Err = 0;

    // Allocating an Rdg skeleton
    This = (CRdg*)malloc(sizeof(CRdg));
    RdgHdl = *PtRdgHdl = (CRdgHdl) This;

    // Argument checking
    if(ImaDyn<RDG_CST_MIN_IMADYN || ImaDyn>RDG_CST_MAX_IMADYN)
    {
        return RDG_ERR_IMADYN_RANGE;
    }

    if(ImaWidth<RDG_CST_MIN_IMAWIDTH)
    {
        return RDG_ERR_IMAWIDTH_RANGE;
    }

    if(ImaHeight<RDG_CST_MIN_IMAHEIGHT)
    {
        return RDG_ERR_IMAHEIGHT_RANGE;
    }

    // Field initialisation
    This->ExtractProc.ExtractTime = -1;
    This->ExtractProc.NewParameters = FALSE;

    This->ExtractProc.IwOri    = ImaWidth;
    This->ExtractProc.IhOri    = ImaHeight;
    This->ExtractProc.DynOri   = ImaDyn;
    This->ExtractProc.SSFactor = SSFactor;

    This->VersionInfo = (char*)malloc((strlen(RDG_VERSION_INFO)+strlen(RDG_CONFIG)) * sizeof(char));
    sprintf(This->VersionInfo, RDG_VERSION_INFO, RDG_CONFIG);

    // Field setting
    This->ExtractProc.ImSub = NULL;
    This->ExtractProc.ImPel = NULL;
    This->ExtractProc.ImDil = NULL;
    This->ExtractProc.ImClo = NULL;
    This->ExtractProc.ImRwo = NULL;
    This->ExtractProc.ImNoE = NULL;
    This->ExtractProc.ImNoi = NULL;
    This->ExtractProc.ImGau = NULL;
    This->ExtractProc.ImRdg = NULL;
    This->ExtractProc.ImTipRdg = NULL;
    This->ExtractProc.ImDir = NULL;
    This->ExtractProc.ImSca = NULL;
    This->ExtractProc.ImBkg    = NULL;
    This->ExtractProc.ImFrg    = NULL;
    This->ExtractProc.ImTipBkg = NULL;
    This->ExtractProc.ImTipFrg = NULL;
    This->ExtractProc.ImSee       = NULL;
    This->ExtractProc.ImThr       = NULL;
    This->ExtractProc.ImTipSee    = NULL;
    This->ExtractProc.ImTipThr    = NULL;
    This->ExtractProc.ImLab       = NULL;
    This->ExtractProc.ImTipLab    = NULL;
    This->ExtractProc.ImLabTm1    = NULL;
    This->ExtractProc.ImMask      = NULL;
    This->ExtractProc.ImTipMask   = NULL;
    This->ExtractProc.CentroidTm1 = NULL;
    This->ExtractProc.PreObjectList.NbObject = 0;
    This->ExtractProc.ImGMMask = NULL;
    This->ExtractProc.ImPastThr = NULL;
    This->ExtractProc.PastGlobalMotion = NULL;

    This->ExtractProc.ImCircBuffer    = NULL;
    This->ExtractProc.ImTipCircBuffer = NULL;
    This->ExtractProc.CircBufferSize = 0;

    IwProc = This->ExtractProc.IwProc = This->ExtractProc.IwOri / This->ExtractProc.SSFactor;
    IhProc = This->ExtractProc.IhProc = This->ExtractProc.IhOri / This->ExtractProc.SSFactor;

    // allocation of images
    Err |= RdgImAlloc(0, &This->ExtractProc.ImSub,    sizeof(This->ExtractProc.ImSub[0][0]),    IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImPel,    sizeof(This->ExtractProc.ImPel[0][0]),    IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImDil,    sizeof(This->ExtractProc.ImDil[0][0]),    IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImClo,    sizeof(This->ExtractProc.ImClo[0][0]),    IwProc, IhProc);
#if RDG_USE_SSE_GAUSSIAN
    Err |= RdgImAlignedAlloc(0, &This->ExtractProc.ImGau, sizeof(This->ExtractProc.ImGau[0][0]), IwProc, IhProc, 16);
#else
    Err |= RdgImAlloc(0, &This->ExtractProc.ImGau,    sizeof(This->ExtractProc.ImGau[0][0]),    IwProc, IhProc);
#endif
    Err |= RdgImAlloc(0, &This->ExtractProc.ImRwo,     sizeof(This->ExtractProc.ImRwo[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImNoE,     sizeof(This->ExtractProc.ImNoE[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImNoi,     sizeof(This->ExtractProc.ImNoi[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImRdg,     sizeof(This->ExtractProc.ImRdg[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImTipRdg,  sizeof(This->ExtractProc.ImTipRdg[0][0]),  IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImDir,     sizeof(This->ExtractProc.ImDir[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImSca,     sizeof(This->ExtractProc.ImSca[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImFrg,     sizeof(This->ExtractProc.ImFrg[0][0]),     IwProc, IhProc);
    if (ExtractParam->BackgroundEstimationFlag)
    {
       Err |= RdgImAlloc(0, &This->ExtractProc.ImBkg,     sizeof(This->ExtractProc.ImBkg[0][0]),     IwProc, IhProc);
       Err |= RdgImAlloc(0, &This->ExtractProc.ImTipBkg,  sizeof(This->ExtractProc.ImTipBkg[0][0]),  IwProc, IhProc);
       Err |= RdgImAlloc(0, &This->ExtractProc.ImTipFrg,  sizeof(This->ExtractProc.ImTipFrg[0][0]),  IwProc, IhProc);
    }
    Err |= RdgImAlloc(0, &This->ExtractProc.ImSee,     sizeof(This->ExtractProc.ImSee[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImThr,     sizeof(This->ExtractProc.ImThr[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImTipSee,  sizeof(This->ExtractProc.ImTipSee[0][0]),  IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImTipThr,  sizeof(This->ExtractProc.ImTipThr[0][0]),  IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImLab,     sizeof(This->ExtractProc.ImLab[0][0]),     IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImTipLab,  sizeof(This->ExtractProc.ImTipLab[0][0]),  IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImLabTm1,  sizeof(This->ExtractProc.ImLabTm1[0][0]),  IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImMask,    sizeof(This->ExtractProc.ImMask[0][0]),    IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImTipMask, sizeof(This->ExtractProc.ImTipMask[0][0]), IwProc, IhProc);
    Err |= RdgImAlloc(0, &This->ExtractProc.ImGMMask,  sizeof(This->ExtractProc.ImGMMask[0][0]),  IwProc, IhProc);

    {
       int k;

       if (This->ExtractProc.ImPastThr == NULL)
          Err |= RdgImAlloc((void **) &This->ExtractProc.ImPastThr, 0, sizeof(unsigned char **), RDG_CST_GM_PAST_N_FRAMES, 1);

       for (k=0; k<RDG_CST_GM_PAST_N_FRAMES; k++)
          Err |= RdgImAlloc(0, &This->ExtractProc.ImPastThr[k], sizeof(char), IwProc, IhProc);
    }

    if (This->ExtractProc.PastGlobalMotion == NULL)
       Err |= RdgImAlloc(&This->ExtractProc.PastGlobalMotion, 0, sizeof(CRdgVi), RDG_CST_GM_PAST_N_FRAMES, 1);

    // allocate pictures for circular buffer to hold one second of data
    if (ExtractParam->BackgroundEstimationFlag)
    {
       int k;

       // allocate circular buffer to allocate one second of data
       This->ExtractProc.CircBufferSize = (int) RDG_MAX(1, ExtractParam->FramesPerSecond);

       Err |= RdgImAlloc((void **) &This->ExtractProc.ImCircBuffer, 0, sizeof(float **), This->ExtractProc.CircBufferSize, 1);

       for (k=0; k<This->ExtractProc.CircBufferSize; k++)
          Err |= RdgImAlloc(0, &This->ExtractProc.ImCircBuffer[k], sizeof(float), IwProc, IhProc);

       Err |= RdgImAlloc((void **) &This->ExtractProc.ImTipCircBuffer, 0, sizeof(float **), This->ExtractProc.CircBufferSize, 1);

       for (k=0; k<This->ExtractProc.CircBufferSize; k++)
          Err |= RdgImAlloc(0, &This->ExtractProc.ImTipCircBuffer[k], sizeof(float), IwProc, IhProc);
    }

    // make sure internal parameters are set to zero,
    // before passing external ones
    memset(&This->ExtractProc.CachedParam, 0, sizeof(This->ExtractProc.CachedParam)); 
    // set external parameters
    RdgSetParam(This, ExtractParam);

    return Err;
} 

/*************************************************************************************/
// Return current RdgLib version number
/*************************************************************************************/
int RdgGetVersionInfo(CRdgHdl RdgHdl, char** VersionInfo)
{
	CRdg* This = (CRdg*)RdgHdl;
	*VersionInfo = This->VersionInfo;

	return 0;
}


/*************************************************************************************/
// Delete a stent extrat-boost process : free everything
/*************************************************************************************/
int RdgDelete(CRdgHdl RdgHdl)
{
	CRdg* This = (CRdg*)RdgHdl;
	
	free(This->VersionInfo);

	if(This->ExtractProc.ImSub != NULL)       RdgImFree(0, This->ExtractProc.ImSub);
	if(This->ExtractProc.ImPel != NULL)       RdgImFree(0, This->ExtractProc.ImPel);
	if(This->ExtractProc.ImDil != NULL)       RdgImFree(0, This->ExtractProc.ImDil);
	if(This->ExtractProc.ImClo != NULL)       RdgImFree(0, This->ExtractProc.ImClo);
	if(This->ExtractProc.ImRwo != NULL)       RdgImFree(0, This->ExtractProc.ImRwo);
	if(This->ExtractProc.ImNoE != NULL)       RdgImFree(0, This->ExtractProc.ImNoE);
	if(This->ExtractProc.ImNoi != NULL)       RdgImFree(0, This->ExtractProc.ImNoi);
#if RDG_USE_SSE_GAUSSIAN
	if(This->ExtractProc.ImGau != NULL)       RdgImAlignedFree(0, This->ExtractProc.ImGau);
#else
	if(This->ExtractProc.ImGau != NULL)       RdgImFree(0, This->ExtractProc.ImGau);
#endif
	if(This->ExtractProc.ImRdg != NULL)       RdgImFree(0, This->ExtractProc.ImRdg);
	if(This->ExtractProc.ImTipRdg != NULL)    RdgImFree(0, This->ExtractProc.ImTipRdg);
	if(This->ExtractProc.ImDir != NULL)       RdgImFree(0, This->ExtractProc.ImDir);
	if(This->ExtractProc.ImSca != NULL)       RdgImFree(0, This->ExtractProc.ImSca);
	if(This->ExtractProc.ImBkg != NULL)       RdgImFree(0, This->ExtractProc.ImBkg);
	if(This->ExtractProc.ImFrg != NULL)       RdgImFree(0, This->ExtractProc.ImFrg);
	if(This->ExtractProc.ImTipBkg != NULL)    RdgImFree(0, This->ExtractProc.ImTipBkg);
	if(This->ExtractProc.ImTipFrg != NULL)    RdgImFree(0, This->ExtractProc.ImTipFrg);
	if(This->ExtractProc.ImSee != NULL)       RdgImFree(0, This->ExtractProc.ImSee);
	if(This->ExtractProc.ImThr != NULL)       RdgImFree(0, This->ExtractProc.ImThr);
	if(This->ExtractProc.ImTipSee != NULL)    RdgImFree(0, This->ExtractProc.ImTipSee);
	if(This->ExtractProc.ImTipThr != NULL)    RdgImFree(0, This->ExtractProc.ImTipThr);
	if(This->ExtractProc.ImLab != NULL)       RdgImFree(0, This->ExtractProc.ImLab);
	if(This->ExtractProc.ImLabTm1 != NULL)    RdgImFree(0, This->ExtractProc.ImLabTm1);
	if(This->ExtractProc.ImTipLab != NULL)    RdgImFree(0, This->ExtractProc.ImTipLab);
	if(This->ExtractProc.ImMask != NULL)      RdgImFree(0, This->ExtractProc.ImMask);
	if(This->ExtractProc.ImTipMask != NULL)   RdgImFree(0, This->ExtractProc.ImTipMask);
	if(This->ExtractProc.CentroidTm1 != NULL) RdgImFree(This->ExtractProc.CentroidTm1, 0);
   if(This->ExtractProc.ImGMMask != NULL)    RdgImFree(0, This->ExtractProc.ImGMMask);

   if(This->ExtractProc.ImPastThr != NULL)
   {
      int k;

      for (k=0; k<RDG_CST_GM_PAST_N_FRAMES; k++)
         if(This->ExtractProc.ImPastThr[k] != NULL) RdgImFree(0, This->ExtractProc.ImPastThr[k]);
      
      RdgImFree(This->ExtractProc.ImPastThr, 0);
   }
   if(This->ExtractProc.PastGlobalMotion != NULL) RdgImFree(This->ExtractProc.PastGlobalMotion, 0);

   // free circular buffer
   if(This->ExtractProc.ImCircBuffer != NULL)
   {
      int k;

      for (k=0; k<This->ExtractProc.CircBufferSize; k++)
         if(This->ExtractProc.ImCircBuffer[k] != NULL) RdgImFree(0, This->ExtractProc.ImCircBuffer[k]);

      RdgImFree(This->ExtractProc.ImCircBuffer, 0);
   }
   if(This->ExtractProc.ImTipCircBuffer != NULL)
   {
      int k;

      for (k=0; k<This->ExtractProc.CircBufferSize; k++)
         if(This->ExtractProc.ImTipCircBuffer[k] != NULL) RdgImFree(0, This->ExtractProc.ImTipCircBuffer[k]);

      RdgImFree(This->ExtractProc.ImTipCircBuffer, 0);
   }

	RdgClearObjectList(&(This->ExtractProc.PreObjectList));

	free(This);

	return 0;
} 
