// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
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

#include <MkxDev.h>
#include <MkxVersion.h>
#include <MkxErrMessages.h>
#include <IThreadPool.h>

/*************************************************************************************/
// Create a stent-boost process : valid for a given image format
/*************************************************************************************/
int MkxCreate(CMkxHdl* PtMkxHdl, int ImaDyn, int ImaWidth, int ImaHeight,  IThreadPool* pool)
{
  CMkx* This;
  int Major, Minor, Patch, Err = 0, i;
  char Reason[128];
  char FuncName[] = "MkxCreate";

  // Allocating an Mkx skeleton
  This = (CMkx*)malloc(sizeof(CMkx));
  PtMkxHdl->Pv = (void *)This;

  CMkxExtractProc *ep = &This->ExtractProc;

  // Init error management material
  ErrCreate(&This->ErrHdl);
  This->ErrHdlDeleted = 0;           // will be set to 1 if deleted
  PtMkxHdl->ErrHdl = This->ErrHdl;
  Err = ErrAdd(This->ErrHdl, __MkxErrorArray, MKX_ERR_LAST - MKX_ERR_FIRST - 1);
  if (Err)
  {
    ErrAppendErrorReportFromCaller(This->ErrHdl, FuncName, "ErrAdd", 0, Err);
    return Err;
  }
  // Checking ErrLib version (here done after instantiation to be able to use error mechanism)
  ErrGetVersionNumbers(&Major, &Minor, &Patch);
  if (Major != ERR_VERSION_MAJOR || Minor != ERR_VERSION_MINOR)
  {
    sprintf(Reason, "ErrLib.dll V.%d.%d.* not compatible with library include version V.%d.%d.*",
      Major, Minor, ERR_VERSION_MAJOR, ERR_VERSION_MINOR);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, MKX_ERR_DLL_VERSION, Reason);
    return MKX_ERR_DLL_VERSION;
  }

  // Argument checking
  if (ImaDyn<MKX_CST_MIN_IMADYN || ImaDyn>MKX_CST_MAX_IMADYN)
  {
    sprintf(Reason, "ImaDyn argument = %d : out of range [%d %d]", ImaDyn, MKX_CST_MIN_IMADYN, MKX_CST_MAX_IMADYN);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, MKX_ERR_IMADYN_RANGE, Reason);
    return MKX_ERR_IMADYN_RANGE;
  }
  if (ImaWidth < MKX_CST_MIN_IMAWIDTH)
  {
    sprintf(Reason, "ImaWidth argument = %d : too small", ImaWidth);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, MKX_ERR_IMAWIDTH_RANGE, Reason);
    return MKX_ERR_IMAWIDTH_RANGE;
  }
  if (ImaHeight < MKX_CST_MIN_IMAHEIGHT)
  {
    sprintf(Reason, "ImaHeight argument = %d : too small", ImaHeight);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, MKX_ERR_IMAHEIGHT_RANGE, Reason);
    return MKX_ERR_IMAHEIGHT_RANGE;
  }

  // Checking PrmLib version (before instantiation)
  PrmGetVersionNumbers(&Major, &Minor, &Patch);
  if (Major != PRM_VERSION_MAJOR || Minor != PRM_VERSION_MINOR)
  {
    sprintf(Reason, "PrmLib V.%d.%d.* not compatible with expected V.%d.%d.*",
      Major, Minor, PRM_VERSION_MAJOR, PRM_VERSION_MINOR);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, MKX_ERR_DLL_VERSION, Reason);
    return MKX_ERR_DLL_VERSION;
  }
  // Instantiate parameter library
  Err = PrmCreate(&(This->PrmHdl), This->ErrHdl, MKX_NBPARA, MkxSetParaCtx);
  if (Err)
  {
    ErrAppendErrorReportFromCaller(This->ErrHdl, FuncName, "PrmCreate", 0, Err);
    return Err;
  }
  PtMkxHdl->PrmHdl = This->PrmHdl;

  // Setting Mkx version material
  MkxSetVersion(This);

  // Field initialisation
  ep->ExtractTime = -1;
  ep->WireTime = -1;

  ep->IwOri = ImaWidth;
  ep->IhOri = ImaHeight;
  ep->DynOri = ImaDyn;

  // Setting clues to unavailable
  ep->Clue.Nature = MKX_CST_CLUE_NOTAVAILABLE;

  ep->KBestLblBlobs = NULL;
  ep->RawCouples = NULL;
  ep->TrCouples = NULL;
  ep->TrackingHdl = 0; // no tracker launched yet
  ep->TrIn.Couples = NULL;
  ep->TrOut.Couples = NULL;

  ep->TrIn.Clue.Nature = TR1_CLUE_NOTAVAILABLE;//PL_071205

  ep->TrOut.Locked = TR1_RES_UNLOCKED;

  ep->SWHdl.Pv = 0;
  ep->WireHdl = 0;
  ep->ExtraMeasuresWireHdl = 0; // Handler for wire used for extra measures


  // Create a BxWire instance for wire-based couple selection
  // This used to be done in MkxMarkerCoupleWireRescore()
  {
    BxWireDefaultParameters(&ep->ExtraMeasuresWireParams, ImaDyn); //tobechecked

    // added by SV
    // ExtraMeasuresWireParams->ContrastDeadZone = 4; // same as blob radius
    ep->ExtraMeasuresWireParams.ExpectedContrast = 10;
    ep->ExtraMeasuresWireParams.OutputResolution = 1.0;
    ep->ExtraMeasuresWireParams.TrackingMode = None;
    ep->ExtraMeasuresWireParams.InternalFieldOfView = 0.2f;
    //ep->ExtraMeasuresWireParams.InternalNSearch = 32;

    // parameters for faster fitting
    ep->ExtraMeasuresWireParams.InitSearch = 0;
    //ep->ExtraMeasuresWireParams.InternalSampling = 24; // 16 ; // 12 ; // 24 ;
    //ep->ExtraMeasuresWireParams.InternalNSearch = 24; // 16 ; // 16 ; // 24 ;

    // parameters for extra faster fitting at the expense of fitting quality
    ep->ExtraMeasuresWireParams.InitSearch = 0;
    ep->ExtraMeasuresWireParams.InternalSampling = 16; // 16 ; // 12 ; // 24 ;
    ep->ExtraMeasuresWireParams.InternalNSearch = 16; // 16 ; // 16 ; // 24 ;

    BxWireCreate((void**)&ep->ExtraMeasuresWireHdl, (BxWireParameters*)&ep->ExtraMeasuresWireParams);
  }

  ep->Clue.X1f = -1;
  ep->Clue.Y1f = -1;
  ep->Clue.X2f = -1;
  ep->Clue.Y2f = -1;

  ep->ImBlobMin1 = NULL;

  ep->ImSub1 = NULL;
  ep->ImPel1 = NULL;
  ep->ImEnh1 = NULL;
  ep->ImThr1 = NULL;
  ep->ImLab1 = NULL;

  ep->ImSubSw = NULL;
  ep->ImPelSw = NULL;
  ep->ImSwMskSub = NULL;
  ep->ImSwMskProc = NULL;
  ep->SwPels = NULL;
  ep->NbSwPels = 0;


  MkxRoiCoordInit(&ep->UsefulRoiCoord[1]);
  MkxRoiCoordInit(&ep->UsefulRoiCoord[2]);

  PrmGetParaContext(This->PrmHdl, &This->ParaContext);

  //allocate tabRing with the max number of directions
  ep->ProcessPara.BlobEnhTabRing = (CMkxVi*)malloc(This->ParaContext.Ctx[MKX_BLOBENHRINGNBDIR_I].Max.Int * sizeof(CMkxVi));

  //allocate History planes with the max depth
  ep->History.StackSize = This->ParaContext.Ctx[MKX_HISTORYDEPTH_I].Max.Int;
  ep->History.Plane = (CMkxHistoryPlane *)calloc(ep->History.StackSize, sizeof(CMkxHistoryPlane));

  ep->NbRawCouples = 0; //PL_071205

  ep->MaxNbCouples = This->ParaContext.Ctx[MKX_CPLSELECTNBLABELSKEPT_I].Max.Int * (This->ParaContext.Ctx[MKX_CPLSELECTNBLABELSKEPT_I].Max.Int - 1) / 2;
  ep->KBestLblBlobs = (CMkxBlob      *)malloc(This->ParaContext.Ctx[MKX_CPLSELECTNBLABELSKEPT_I].Max.Int * sizeof(CMkxBlob));
  for (i = 0; i < This->ParaContext.Ctx[MKX_CPLSELECTNBLABELSKEPT_I].Max.Int; i++)
  {
    MkxBlobInit(&ep->KBestLblBlobs[i]);//PL_071205
  }
  ep->RawCouples = (CMkxBlobCouple*)malloc(ep->MaxNbCouples  * sizeof(CMkxBlobCouple));
  for (i = 0; i < ep->MaxNbCouples; i++)
  {
    MkxBlobCoupleInit(&ep->RawCouples[i]);//PL_071205
  }
  ep->TrCouples = (CMkxBlobCouple*)malloc(ep->MaxNbCouples  * sizeof(CMkxBlobCouple));
  for (i = 0; i < ep->MaxNbCouples; i++)
  {
    MkxBlobCoupleInit(&ep->TrCouples[i]);//PL_071205
  }

  // Set TrIn 
  ep->TrIn.Couples = (CTr1Couple*)malloc((ep->MaxNbCouples + 1)*sizeof(CTr1Couple));
  //MaxNbCouples instead of NbRawCouples because it is a fixed value, +1 for compatibility with 0 case

  // Set TrOut 
  ep->TrOut.Couples = (CTr1Couple*)malloc((ep->MaxNbCouples + 1)*sizeof(CTr1Couple));

  // Setting clue to unavailable
  ep->Clue.Nature = MKX_CST_CLUE_NOTAVAILABLE;

  //
  ep->SeqPara = DefaultSeqPara((int)MKX_MAX(ImaWidth, ImaHeight));

  //timing removed!
  ep->Timing.IsAvailable=false;

  // Instantiate SWLib library
  int nbProc;
  pool->execute([&nbProc](int threadIndex, int threadCount){ nbProc = threadCount; });

  // Checking SWLib version (before instantiation)
  Err = SWCreate(&(ep->SWHdl), ep->IwOri, ep->IhOri, nbProc);
  if (Err)
  {
    ErrAppendErrorReportFromCaller(This->ErrHdl, FuncName, "SWCreate", 0, Err);
    return Err;
  }
  SWSetThreadpool(&(ep->SWHdl), pool);

  return(0);
}



/*************************************************************************************/
// Return current MkxLib version number
/*************************************************************************************/
void MkxGetVersionNumbers(int* Major, int* Minor, int* Patch)
{
  *Major = MKX_VERSION_MAJOR; *Minor = MKX_VERSION_MINOR; *Patch = MKX_VERSION_PATCH;
}

//-------------------------------------------------------------------------------------
// Set version info material
int MkxSetVersion(CMkx *This)
{
  int  NbChar;
  char *Format;

  NbChar = (int)(strlen(MKX_VERSION_HEADER) + strlen(MKX_VERSION_INFO) + strlen(MKX_CONFIG) + 16);
  This->VersionInfo = (char*)malloc(NbChar*sizeof(char));
  This->VersionInfoCopy = (char*)malloc(NbChar*sizeof(char));
  Format = (char*)malloc(NbChar*sizeof(char));

  sprintf(Format, "%s%s", MKX_VERSION_HEADER, MKX_VERSION_INFO);
  sprintf(This->VersionInfo, Format, MKX_CONFIG, MKX_PLATFORM, MKX_VERSION_MAJOR, MKX_VERSION_MINOR, MKX_VERSION_PATCH);

  free(Format);
  return 0;
}

//-------------------------------------------------------------------------------------
// Return current MkxLib version info
void MkxGetVersionInfo(CMkxHdl MkxHdl, char** VersionInfo)
{
  CMkx* This = (CMkx*)MkxHdl.Pv;
  strcpy(This->VersionInfoCopy, This->VersionInfo);
  *VersionInfo = This->VersionInfoCopy;
}

/*************************************************************************************/
// Delete a stent extrat-boost process : free everything
/*************************************************************************************/
void MkxDelete(CMkxHdl MkxHdl)
{
  CMkx* This = (CMkx*)(MkxHdl.Pv);
  CMkxExtractProc *ep = &This->ExtractProc;

  if (ep->ExtraMeasuresWireHdl) BxWireDelete(ep->ExtraMeasuresWireHdl);
  if (ep->WireHdl)              BxWireDelete(ep->WireHdl);
  if (ep->TrackingHdl)          Tr1Delete((CTr1Tracking*)ep->TrackingHdl);
  if (ep->SWHdl.Pv)             SWDelete(&ep->SWHdl);

  MkxFreeProcessImages(ep);

  MKX_FREE(ep->ProcessPara.BlobEnhTabRing);
  MKX_FREE(ep->KBestLblBlobs);
  MKX_FREE(ep->RawCouples);
  MKX_FREE(ep->TrCouples);
  MKX_FREE(ep->TrIn.Couples);
  MKX_FREE(ep->TrOut.Couples);
  MKX_FREE(ep->History.Plane);
  MKX_FREE(ep->SwPels);

  PrmDelete(This->PrmHdl);

  if (!This->ErrHdlDeleted) ErrDelete(This->ErrHdl);

  MKX_FREE(This->VersionInfo);
  MKX_FREE(This->VersionInfoCopy);


  free(This);
}

/*************************************************************************************/
// Setting the clues provided by the client application
/*************************************************************************************/
void MkxSetClue(CMkxHdl MkxHdl, CMkxClue *Clue)
{
  CMkx* This = (CMkx*)(MkxHdl.Pv);

  This->ExtractProc.Clue = *Clue; // reset after each extraction
}

void MkxSetNewErrHdl(CMkxHdl MkxHdl, CErrHdl NewErrHdl)
{
  CMkx* This = (CMkx*)(MkxHdl.Pv);

  // Destroying current error object
  ErrDelete(MkxHdl.ErrHdl);

  // Installing new error object
  This->ErrHdl = NewErrHdl;
  MkxHdl.ErrHdl = NewErrHdl;
  This->ErrHdlDeleted = 1;

  // Adding errors without checking (since no clash recorder at Mkx creation time)
  ErrAdd(This->ErrHdl, __MkxErrorArray, MKX_ERR_LAST - MKX_ERR_FIRST - 1);
}

int	MkxSetMmPerPixel(CMkxHdl hdl, float mmPerPixel)
{
  int   Err = 0;
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;

  if (mmPerPixel <= 0)
  {
	char  Reason[128];
	char  FuncName[] = "MkxSetMmPerPixel";
	ep->SeqPara.MmPerPixel = MKX_CST_MmPerPixel_Def(MKX_MAX(ep->IwOri, ep->IhOri));
	Err = MKX_WRN_CLIPPED_ARG;
	sprintf(Reason, "Wrong mmPerPixel (%f), set to default (%f)", mmPerPixel, ep->SeqPara.MmPerPixel);
	ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, Err, Reason);
  }
  else
  {
    ep->SeqPara.MmPerPixel = mmPerPixel;
  }
  return Err;
}

int	MkxSetFps(CMkxHdl hdl, float fps)
{

  int   Err = 0;
  char  Reason[128];
  char  FuncName[] = "MkxSetFps";
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;

  if (fps < MKX_CST_MIN_FPS)
  {
    Err = MKX_WRN_CLIPPED_ARG;
    sprintf(Reason, "Frame rate too low (%f), has been clipped to (%f)", fps, (double)MKX_CST_MIN_FPS);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, Err, Reason);
    ep->SeqPara.Fps = MKX_CST_MIN_FPS;
  }
  else if (fps > MKX_CST_MAX_FPS)
  {
    Err = MKX_WRN_CLIPPED_ARG;
    sprintf(Reason, "Frame rate too high (%f), has been clipped to (%f)", fps, (double)MKX_CST_MAX_FPS);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, Err, Reason);
    ep->SeqPara.Fps = MKX_CST_MAX_FPS;
  }
  else
  {
    ep->SeqPara.Fps = fps;
  }
  return Err;
}

int MkxSetDefRoi(CMkxHdl hdl, float xMinFac, float xMaxFac, float yMinFac, float yMaxFac)
{
  int   Err = 0;
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;
  CMkxRoiCoordFac roi;

  Err = MkxSetRoiCoord(This->ErrHdl, xMinFac, xMaxFac, yMinFac, yMaxFac, &roi); //just to check the input values
  if (Err)
  { 
    char  FuncName[] = "MkxSetDefRoi";
    ErrAppendErrorReportFromCaller(This->ErrHdl, FuncName, "MkxSetRoiCoord", 0, Err);
    return Err;
  }
  ep->SeqPara.DefRoiFacXMin = roi.XMinFac;
  ep->SeqPara.DefRoiFacXMax = roi.XMaxFac;
  ep->SeqPara.DefRoiFacYMin = roi.YMinFac;
  ep->SeqPara.DefRoiFacYMax = roi.YMaxFac;

  return 0;
}

int MkxSetInitRoi(CMkxHdl hdl, float xMinFac, float xMaxFac, float yMinFac, float yMaxFac)
{
  int   Err = 0;
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;
  CMkxRoiCoordFac roi;

  Err = MkxSetRoiCoord(This->ErrHdl, xMinFac, xMaxFac, yMinFac, yMaxFac, &roi); //just to check the input values
  if (Err)
  {
	char  FuncName[] = "MkxSetInitRoi";
    ErrAppendErrorReportFromCaller(This->ErrHdl, FuncName, "MkxSetRoiCoord", 0, Err);
    return Err;
  }
  ep->SeqPara.InitRoiFacXMin = roi.XMinFac;
  ep->SeqPara.InitRoiFacXMax = roi.XMaxFac;
  ep->SeqPara.InitRoiFacYMin = roi.YMinFac;
  ep->SeqPara.InitRoiFacYMax = roi.YMaxFac;

  return 0;
}

int	MkxSetMarkersDistMinMaxMm(CMkxHdl hdl, float markersDistMinMm, float markersDistMaxMm)
{
  int   Err = 0;
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;

  if (markersDistMinMm <= 0 || markersDistMaxMm < markersDistMinMm)
  {
	char  Reason[128];
	char  FuncName[] = "MkxSetMarkersDistMinMaxMm";
    Err = MKX_WRN_CLIPPED_ARG;
    sprintf(Reason, "Wrong (markersDistMinMm,markersDistMaxMm) (%f,%f)), set to default (%f,%f)", markersDistMinMm, markersDistMaxMm, (double)MKX_CST_MarkersDistMin_Def, (double)MKX_CST_MarkersDistMax_Def);
    ErrStartErrorReportFromLeaf(This->ErrHdl, FuncName, Err, Reason);
    ep->SeqPara.MarkersDistMinMm = MKX_CST_MarkersDistMin_Def;
    ep->SeqPara.MarkersDistMaxMm = MKX_CST_MarkersDistMax_Def;
  }
  else
  {
    ep->SeqPara.MarkersDistMinMm = markersDistMinMm;
    ep->SeqPara.MarkersDistMaxMm = markersDistMaxMm;
  }
  return Err;
}

// Set the seq params
int MkxSetSeqPara(CMkxHdl hdl, CMkxSeqPara seqPara)
{
  int Err = 0;
  Err = MkxSetMmPerPixel(hdl, seqPara.MmPerPixel);
  Err |= MkxSetFps(hdl, seqPara.Fps);
  Err |= MkxSetMarkersDistMinMaxMm(hdl, seqPara.MarkersDistMinMm, seqPara.MarkersDistMaxMm);
  Err |= MkxSetMarkersDistMinMaxMm(hdl, seqPara.MarkersDistMinMm, seqPara.MarkersDistMaxMm);
  Err |= MkxSetDefRoi(hdl, seqPara.DefRoiFacXMin, seqPara.DefRoiFacXMax, seqPara.DefRoiFacYMin, seqPara.DefRoiFacYMax);
  Err |= MkxSetInitRoi(hdl, seqPara.InitRoiFacXMin, seqPara.InitRoiFacXMax, seqPara.InitRoiFacYMin, seqPara.InitRoiFacYMax);

  return Err;
}

// Get the default seq params
int MkxGetDefaultSeqPara(CMkxHdl hdl, CMkxSeqPara *defSeqPara)
{
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;

  *defSeqPara = DefaultSeqPara((int)MKX_MAX(ep->IwOri, ep->IhOri));

  return 0;
}
// Get the current seq params
int MkxGetCurrentSeqPara(CMkxHdl hdl, CMkxSeqPara *curSeqPara)
{
  CMkx   *This = (CMkx*)hdl.Pv;
  CMkxExtractProc *ep = &This->ExtractProc;

  *curSeqPara = ep->SeqPara;

  return 0;
}

// Get the default SkipFromConfidence params
int MkxGetDefaultSkipFromConfidencePara(CMkxSkipFromConfidencePara *defSkipFromConfPara)
{
  defSkipFromConfPara->Mode = MKX_CST_SKIPFROMCONFIDENCE_NOSKIPPING;
  defSkipFromConfPara->NbBest = 16;
  defSkipFromConfPara->PercentBest = 50.0f;
  defSkipFromConfPara->Threshold = 0.0f;
  defSkipFromConfPara->MinNbImages = 10;

  return 0;
}

/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
