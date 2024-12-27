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
#include <string.h> 

#include <MkxDev.h>
#include <BxWire.h>
#include <BxCurve.h>

//#include "vld.h"

static void MkxInitAdvancedResults(CMkxAdvancedResults* adv);
static int MkxGetSwPels(int IwProc, int IhProc, const unsigned char *ImSwMsk, int SSFactor, const CMkxVs* imPel, int* NbSwPels, CMkxVi* SwPels);

int MkxExtract(CMkxHdl MkxHdl, int t, short* InIma1, CMkxExtractResults *ExtractResults)
{
  CMkx                        *This = (CMkx*)(MkxHdl.Pv);
  CPrmPara                   *ExtractParam = &(This->ActualPara);
  CMkxExtractProc	            *ep = &(This->ExtractProc);
  int                         ErrCode = 0, DarkImageFlag = 0;
  char                        Reason[128];
  BxWireParameters            WireParams;

  //***********************************************************************************************************
  // Preparation ***********************************************************************************************
  //***********************************************************************************************************
  MkxExtractResultsInit(ExtractResults);

  // Time management
  if (t != ep->ExtractTime + 1 && t != 0)
  {
    sprintf(Reason, "MkxExtract called at non-incremental times %d and %d : Extract aborted", ep->ExtractTime, t);
    ErrStartErrorReportFromLeaf(This->ErrHdl, "MkxExtract", MKX_WRN_INCORRECT_TIME, Reason);
    return MKX_WRN_INCORRECT_TIME;
  }
  ep->ExtractTime = t;

  // set the actual extraction parameters
  if (t > 0) //check if tentative of changing critical param at t!=0
  {
    int IndexOfCritical;
    PrmHasAnyTaggedParaChanged(This->PrmHdl, ExtractParam, MKX_CST_PARA_CRITICAL, &IndexOfCritical);
    if (IndexOfCritical >= 0)
    {
      sprintf(Reason, "Critical Extract-parameters changed at t=%d != 0", t);
      ErrStartErrorReportFromLeaf(This->ErrHdl, "MkxExtract", MKX_ERR_CRITICAL_PARA, Reason);
      return MKX_ERR_CRITICAL_PARA;
    }
  }
  PrmGetPara(This->PrmHdl, ExtractParam);
  if (t == 0)
  {
    MkxSetExtractProcFromCriticalParam(&ep->SwStatus,
      &ep->SSFactor, &ep->IwProc, &ep->IhProc,
      &ep->SSFactor_SW, &ep->IwProc_SW, &ep->IhProc_SW, &ep->UserRoiCoord_SW,
      ep->IwOri, ep->IhOri, ExtractParam, ep->SeqPara);

    MkxFreeProcessImages(ep);
    MkxAllocProcessImages(ep, ep->IwProc, ep->IhProc);
    MkxAllocProcessImages_SW(ep, ep->IwProc_SW, ep->IhProc_SW);

    MKX_FREE(ep->SwPels);
    ep->SwPels = (CMkxVi*)malloc(ep->IwOri* ep->IhOri*sizeof(CMkxVi));

    MKX_FREE(ep->ImSwMskSub);
    ep->ImSwMskSub = (unsigned char *)calloc(ep->IwProc_SW* ep->IhProc_SW, sizeof(unsigned char));
    MKX_FREE(ep->ImSwMskProc);
    ep->ImSwMskProc = (unsigned char *)calloc(ep->IwProc* ep->IhProc, sizeof(unsigned char));
  }
  MkxSetExtractProcFromOnTheFlyParam(
    &ep->ProcessPara, &ep->MarkersDistMm, &ep->CplSelectDistToler, &ep->SubMarkersDist, &ep->EstimatedMaxNbPixPerBlob, 
    &ep->WireCplMaxNbCouple, &ep->WireCplPruningFactor,
    &ep->MarginRoiCoord, &ep->Margin, &ep->UserInitRoiCoord, &ep->UserRoiCoord,
    ep->TrackingHdl,
    ep->IwProc, ep->IhProc,
    ep->SSFactor, ep->ExtractTime, ep->TrOut.Locked, 
    ep->SeqPara,
    ExtractParam
    );
  // Reinit if t=0
  if (t == 0)
  {
    ep->TrOut.Locked = TR1_RES_UNLOCKED;
    ep->NotYetLockedFlag = 1;
    //ep->StitchRemovalStoppedFlag = 0; //PL_070417

    //reset history 
    MkxResetHistory(&ep->History);
    MkxRoiCoord(&ep->HistoryRoi, 0, 0, ep->IwProc - 1, ep->IhProc - 1);

    // Restarting a tracking process
    if (ExtractParam->Val[MKX_TRACKONFLAG_I].Int)
    {
      CTr1Param TrParam;
      if (ep->TrackingHdl) Tr1Delete((CTr1Tracking*)ep->TrackingHdl);
      MkxSetTrackingParam(&TrParam, ExtractParam, &This->ParaContext, ep->IwProc, ep->IhProc, &ep->ProcessPara);
      ep->TrackingHdl = Tr1Create(&TrParam);
    }
  }

  //set NotYetLockedFlag when locked
  if (ep->TrOut.Locked == TR1_RES_LOCKED)        ep->NotYetLockedFlag = 0;

  //desactivate locking if tracking is disactivated
  if (!ExtractParam->Val[MKX_TRACKONFLAG_I].Int)    ep->TrOut.Locked = TR1_RES_UNLOCKED;

  //reinit adaptive params
  MkxInitAdaptiveParams(
    &ep->TwoRoisFlag, ep->UsefulRoiCoord,
    &ep->AdaptiveWorkNbLblBlobsKept, &ep->AdaptiveWorkNbPixelsKept, &ep->AdaptiveWorkMarkersDist, &ep->AdaptiveDistTolerMin, &ep->AdaptiveDistTolerMax,
    ep->UserInitRoiCoord, ep->UserRoiCoord, ep->EstimatedMaxNbPixPerBlob,
    ep->NotYetLockedFlag, ep->SubMarkersDist, ep->CplSelectDistToler, ExtractParam
    );

  //possibly adapt adaptive params
  if (ep->TrOut.Locked == TR1_RES_LOCKED)
  {
    int ret = MkxAdaptWhenLocked(
      &ep->AdaptiveWorkMarkersDist, &ep->AdaptiveWorkNbPixelsKept, &ep->AdaptiveWorkNbLblBlobsKept, ep->UsefulRoiCoord, &ep->AdaptiveDistTolerMin, &ep->AdaptiveDistTolerMax,
      ep->EstimatedMaxNbPixPerBlob, ep->IwProc, ep->IhProc, ep->TrOut,
      ep->CplSelectDistToler,
      ExtractParam
      );
    if (ret) ep->TrOut.Locked = 0; //delock immediately
    else  ep->TwoRoisFlag = (ExtractParam->Val[MKX_ADAPTIVEROIFLAG_I].Int == MKX_CST_ADAPT_2ROIS) ? 1 : 0;
  }

  //possibly clip rois from history
  if (ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYANDUPDATE || ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYONLY)
  {
    int ret = MkxHistoryRoi(
      &ep->HistoryRoi,
      (const CMkxHistory*)&ep->History,
      ep->IwProc, ep->IhProc,
      ExtractParam
      );
    if (ret == 0)
    {
      MkxClipRoi(&ep->UsefulRoiCoord[0], ep->HistoryRoi, ep->UsefulRoiCoord[0]);
      MkxClipRoi(&ep->UsefulRoiCoord[1], ep->HistoryRoi, ep->UsefulRoiCoord[1]);
      MkxClipRoi(&ep->UsefulRoiCoord[2], ep->HistoryRoi, ep->UsefulRoiCoord[2]);
    }
  }

  // apply a margin for the WorkRoiCoord
  SetExtractWorkRoi(ep->Margin, ep->UsefulRoiCoord[0], &(ep->WorkRoiCoord));


  //***********************************************************************************************************
  // Process ***********************************************************************************************
  //***********************************************************************************************************

  if (ep->SSFactor > 1)
    MkxSubSampleImage(InIma1, ep->IwOri, ep->ImSub1, ep->ImPel1, ep->IwProc, ep->IhProc, ep->SSFactor, ep->WorkRoiCoord, 0);
  else
    memcpy(ep->ImSub1, (const short*)InIma1, ep->IwOri* ep->IhOri*sizeof(short));

  if (ExtractParam->Val[MKX_CPLSELECTSWLibFLAG_I].Int > 0)
  {
  if (ep->SSFactor_SW > 1)
      //for SwLib, larger roi (user defined roi) subsampled image
    MkxSubSampleImage(InIma1, ep->IwOri, ep->ImSubSw, ep->ImPelSw, ep->IwProc_SW, ep->IhProc_SW, ep->SSFactor_SW, ep->UserRoiCoord_SW, 0);
  else
    memcpy(ep->ImSubSw, (const short*)InIma1, ep->IwOri* ep->IhOri*sizeof(short));
  }

  //Sternal Wire mask computing 
  if (ExtractParam->Val[MKX_CPLSELECTSWLibFLAG_I].Int == 1)
  {
    CSWRoiCoord RoiCoord;
    RoiCoord.XMax = ep->UserRoiCoord_SW.XMax;
    RoiCoord.XMin = ep->UserRoiCoord_SW.XMin;
    RoiCoord.YMax = ep->UserRoiCoord_SW.YMax;
    RoiCoord.YMin = ep->UserRoiCoord_SW.YMin;
    float pixSize = ep->SeqPara.MmPerPixel*ep->SSFactor_SW;

    if (t <= MKX_CST_SWDETECTIONTIME)
    {
      ep->SwStatus = MKX_CST_EXTRACT_RES_STERNALWIRE_INIT_PHASE;
      SWExtract(&(ep->SWHdl), (const short*)ep->ImSubSw, ep->IwProc_SW, ep->IhProc_SW, &RoiCoord, t, pixSize, true, &ep->SWRes);
      if (t == MKX_CST_SWDETECTIONTIME)
        ep->SwStatus = ep->SWRes.nbPixMask > 0 ? MKX_CST_EXTRACT_RES_STERNALWIRE_DETECTED : MKX_CST_EXTRACT_RES_STERNALWIRE_NOT_DETECTED;
    }
    else
    {
      if (ep->SwStatus == MKX_CST_EXTRACT_RES_STERNALWIRE_DETECTED)
        SWTracking(&(ep->SWHdl), (const short*)ep->ImSubSw, ep->IwProc_SW, ep->IhProc_SW, &RoiCoord, t, pixSize, &ep->SWRes);
    }
    if (ep->SwStatus == MKX_CST_EXTRACT_RES_STERNALWIRE_INIT_PHASE || ep->SwStatus == MKX_CST_EXTRACT_RES_STERNALWIRE_DETECTED)
    {
      memcpy(ep->ImSwMskSub, ep->SWRes.SWMask, ep->IwProc_SW* ep->IhProc_SW*sizeof(char));
      ////get sw pels at full resolution
      //MkxGetSwPels(ep->IwProc_SW, ep->IhProc_SW, (const unsigned char *)ep->ImSwMskSub, ep->SSFactor_SW, (const CMkxVs*)ep->ImPelSw, &ep->NbSwPels, ep->SwPels);
      //get sw pels at current resolution
      MkxGetSwPels(ep->IwProc_SW, ep->IhProc_SW, (const unsigned char *)ep->ImSwMskSub, 1, (const CMkxVs*)ep->ImPelSw, &ep->NbSwPels, ep->SwPels);
	}
  }

  //Blob enhancement 
  //input: ImSub1, ImBlobMin1 (if dkpx)
  //output: ImEnh1, ImBlobMin1 (if dkpx)
  MkxBlobEnhanceRing(
    (const short*)ep->ImSub1, ep->IwProc, ep->IhProc,
    ep->ImEnh1,
    ep->ImBlobMin1,
    &ep->ImEnhMin1, &ep->ImEnhMax1, &ep->ImEnhMin2, &ep->ImEnhMax2,
    ep->WorkRoiCoord, ep->UsefulRoiCoord, ep->TwoRoisFlag,
    t,
    ep->ProcessPara.BlobEnhNbDir,
    ep->ProcessPara.BlobEnhTabRing,
    ep->ProcessPara.BlobEnhRingSigInPix,
    ExtractParam->Val[MKX_BLOBENHINTENSITYFACTOR_F].Float,
    ExtractParam->Val[MKX_PARA_DarkPixelsPenalize_I].Int,
    (float)MKX_CST_DarkPixelsPenalizeIntegrationFactor,
    (int)MKX_CST_DarkPixelsPenalizeDelay
    );

  //apply sternal wire removal
  if (ep->SwStatus == MKX_CST_EXTRACT_RES_STERNALWIRE_INIT_PHASE || ep->SwStatus == MKX_CST_EXTRACT_RES_STERNALWIRE_DETECTED)
  {
    //Sw mask rescaling
    if(ep->SSFactor==ep->SSFactor_SW)
    {
      memcpy(ep->ImSwMskProc, ep->ImSwMskSub, ep->IwProc* ep->IhProc*sizeof(char));
    }
    else if(ep->SSFactor > ep->SSFactor_SW)
    {
      //get mask at process resolution from swpels at sw resoltion
      float fac=ep->SSFactor_SW/(float)ep->SSFactor;
      memset(ep->ImSwMskProc,0,ep->IwProc* ep->IhProc*sizeof(char));
      for(int k=0;k<ep->NbSwPels;k++)
      {
        int x=(int)MKX_RND(ep->SwPels[k].X*fac);
        int y=(int)MKX_RND(ep->SwPels[k].Y*fac);
        ep->ImSwMskProc[x+y*ep->IwProc]=1;
      }
    }
	else //ep->SSFactor_SW > ep->SSFactor)
	{
		//get mask at process resolution from swpels at sw resoltion
		int N = ep->SSFactor_SW / ep->SSFactor;
		int N1 = (N - 1) / 2;
		memset(ep->ImSwMskProc, 0, ep->IwProc* ep->IhProc*sizeof(char));
		for (int k = 0; k < ep->NbSwPels; k++)
		{
			int x0 = ep->SwPels[k].X *N;
			int y0 = ep->SwPels[k].Y *N;
			for (int i = 0; i < N; i++)
				for (int j = 0; j < N; j++)
				{
					int x = MKX_CLIP(x0 + j - N1, 0, ep->IwProc - 1);
					int y = MKX_CLIP(y0 + i - N1, 0, ep->IhProc - 1);
					ep->ImSwMskProc[x + y*ep->IwProc] = 1;
				}
		}
    }
    //get sw pels at full resolution (just for info)
    MkxGetSwPels(ep->IwProc, ep->IhProc, (const unsigned char *)ep->ImSwMskProc, ep->SSFactor, (const CMkxVs*)ep->ImPel1, &ep->NbSwPels, ep->SwPels);

    //apply mask
    DarkImageFlag |= MkxApplyRigidMotionMask(
      ep->ImEnh1, ep->IwProc, ep->IhProc,
      ep->UsefulRoiCoord[0],
      ep->ImSwMskProc, ep->IwProc, ep->IhProc
      );
  }

  //pixel selection 
  //input: ImEnh1
  //output: ImThr1
  DarkImageFlag |= MkxPixSelect(
    (const float*)ep->ImEnh1,
    ep->ImThr1,
    ep->IwProc, ep->IhProc,
    ep->ImEnhMax1, ep->ImEnhMax2,
    ep->TwoRoisFlag, ep->UsefulRoiCoord,
    ep->AdaptiveWorkNbPixelsKept
    );

  if (DarkImageFlag)
  {
    ep->NbRawCouples = 0;
    ep->NbLblBlobs = 0;
  }

  //labelling
  //input: ImThr1
  //output: ImLab1, NbLblBlobs
  MkxLabelling(
    (const UC*)ep->ImThr1,
    ep->ImLab1,
    &(ep->NbLblBlobs),
    ep->IwProc, ep->IhProc,
    ep->UsefulRoiCoord[0]
    );

  //blob selection
  //input: ImLab1,ImEnh1
  //output: KBestLblBlobs, NbBestLblBlobs
  MkxBlobSelect(
    ep->NbLblBlobs,
    (const int*)ep->ImLab1, (const float*)ep->ImEnh1,
    ep->IwProc, ep->IhProc,
    ep->UsefulRoiCoord[0],
    &(ep->NbBestLblBlobs),
    ep->KBestLblBlobs,
    ep->AdaptiveWorkNbLblBlobsKept
    );

  //couples selection and measure merit w.r.t blob enhancer
  //input : KBestLblBlobs, NbBestLblBlobs
  //output: RawCouples, NbRawCouples
  MkxCouplesSelect(
    ep->NbBestLblBlobs,
    (const CMkxBlob*)ep->KBestLblBlobs,
    &ep->NbRawCouples,
    ep->RawCouples,
    ep->TwoRoisFlag, ep->UsefulRoiCoord[1], ep->UsefulRoiCoord[2],
    ep->AdaptiveDistTolerMin, ep->AdaptiveDistTolerMax,
    ExtractParam
    );

  // modify CplMeritOverall w.r.t wire (compute IntensityWrtWire field)
  if (ExtractParam->Val[MKX_CPLSELECTWIREFLAG_I].Int)
  {
    // score marker couples according to contrast on fitted curve (put into CplMeritWrtWire, CplMeritWrtWire_markers_correlation and CplMeritWrtWire_wire_correlation)
    MkxMarkerCoupleWireRescore(ep->RawCouples, ep->NbRawCouples, ep->ImSub1, ep->IwProc, ep->IhProc, InIma1, ep->IwOri, ep->IhOri,
      ep->ExtraMeasuresWireHdl, ep->WireCplMaxNbCouple, ep->WireCplPruningFactor);
  }

  // compute IntensityWrtHistory field in RawCouples
  if (ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYANDUPDATE || ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYONLY)
  {
    MkxCouplesSetIntensityWrtHistory(
      ep->NbRawCouples,
      ep->RawCouples,
      &ep->History,
      ExtractParam->Val[MKX_HISTORYFACTOR_F].Float, ExtractParam->Val[MKX_HISTORYFLAG_I].Int
      );
  }

  //set final couple intensity field and sort couples w.r.t it
  MkxCombineIntensities(ep->NbRawCouples, ep->RawCouples, ExtractParam);
  MkxBlobCoupleOrderByIntensity(ep->RawCouples, ep->NbRawCouples);

  // Tracking process
  if (ExtractParam->Val[MKX_TRACKONFLAG_I].Int)
  {
    ep->NbTrInCouples = MKX_MIN(ep->NbRawCouples, ExtractParam->Val[MKX_TRMAXNBCOUPLE_I].Int);	// Limit the number of couples
    MkxSetTrackingInput(
      &ep->TrIn,
      ep->NbTrInCouples, (const CMkxBlobCouple*)ep->RawCouples, ep->Clue
      );
    Tr1Iterate(
      ep->TrackingHdl,
      t,
      (const CTr1Input*)&ep->TrIn,
      &ep->TrOut, &ep->Tr1AdvancedOutput
      );
    MkxGetTrackingOutput(
      (const CTr1Output*)&ep->TrOut,
      &ep->NbTrOutCouples, ep->TrCouples,
      &ep->Status, &ep->Locked
      );
  }

  //get marker couple
  if (ep->Status == MKX_CST_EXTRACT_RES_OK)
  {
    ep->MarkerCoupleInt.V1 = ep->TrCouples[0].Blob1.V;
    ep->MarkerCoupleInt.V2 = ep->TrCouples[0].Blob2.V;
  }
  else
  {
    ep->MarkerCoupleInt.V1 = ep->RawCouples[0].Blob1.V;
    ep->MarkerCoupleInt.V2 = ep->RawCouples[0].Blob2.V;
  }

  // Polarizing markers (swapping if necessary)
  if (t == 0) ep->PreviousCoupleFlag = 0;
  if (ep->Status == MKX_CST_EXTRACT_RES_OK && ep->PreviousCoupleFlag)
  {
    MkxPolarize(&ep->MarkerCoupleInt, ep->MarkerCoupleInt, ep->PreviousCouple);
  }

  // Upsampling  markers (if subsampling used for extraction)
  if (ep->SSFactor > 1)
  {
    if (ep->MarkerCoupleInt.V1.X >= 0 && ep->MarkerCoupleInt.V1.Y >= 0)MkxUpSampleCoordinates(ep->ImPel1, ep->IwProc, &ep->MarkerCoupleInt.V1.X, &ep->MarkerCoupleInt.V1.Y);
    if (ep->MarkerCoupleInt.V2.X >= 0 && ep->MarkerCoupleInt.V2.Y >= 0)MkxUpSampleCoordinates(ep->ImPel1, ep->IwProc, &ep->MarkerCoupleInt.V2.X, &ep->MarkerCoupleInt.V2.Y);
  }

  ep->MarkerCoupleRefined.Advanced1.n = 0;
  ep->MarkerCoupleRefined.Advanced2.n = 0;
  // Refining
  if (ep->Status == MKX_CST_EXTRACT_RES_OK)
  {
    MkxRefining(
      &ep->MarkerCoupleRefined,
      ep->MarkerCoupleInt,
      (const short*)InIma1, ep->IwOri, ep->IhOri,
      ep->ProcessPara.BlobEnhRingRadiusInPix, ep->SSFactor
      );
  }

  MkxMarkerCoupleFloat(
    &ep->MarkerCoupleFloat,
    ep->MarkerCoupleInt, ep->MarkerCoupleRefined,
    MKX_MARKERMODE_I
    );

  MkxExtractResultsInit(ExtractResults);

  // Get extracted markers
  MkxFillResults(
    ExtractResults,
    ep->MarkerCoupleFloat,
    ep->MarkerCoupleRefined,
    ep->Status, ep->Locked,
    ep->TwoRoisFlag, ep->UsefulRoiCoord, ep->SSFactor,
    ep->IwProc, ep->IhProc,
    ep->SwStatus, ep->IwProc_SW, ep->IhProc_SW
    );

  // Resetting client's provided clue (extract copy)
  ep->Clue.Nature = MKX_CST_CLUE_NOTAVAILABLE;

  //***********************************************************************************************************
  // Ending ***********************************************************************************************
  //***********************************************************************************************************

  // guide wire extraction
  ExtractResults->WireStatus = MKX_CST_WIRE_NOT_AVAILABLE;
  if (ExtractParam->Val[MKX_WIREFLAG_I].Int)
  {
    if (t == 0)
    {
      MkxWireInit(ep->WireHdl, &WireParams, ep->DynOri, ep->SSFactor, ExtractParam);
    }
    if (ep->TrOut.CTr1OutputStatus == MKX_CST_EXTRACT_RES_OK)
      MkxGetWire(ep->WireHdl, InIma1, ep->IwOri, ep->IhOri, ep->ImSub1, ExtractResults,
      ep->SSFactor, ep->IwProc, ep->IhProc, ExtractParam);
  }

  //update Buf with new results for polarizing next extracted markers
  if (ExtractResults->Status == MKX_CST_EXTRACT_RES_OK)  //PL_060522
  {
    ep->PreviousCouple.V1f.Xf = ExtractResults->X1Refined;
    ep->PreviousCouple.V1f.Yf = ExtractResults->Y1Refined;
    ep->PreviousCouple.V2f.Xf = ExtractResults->X2Refined;
    ep->PreviousCouple.V2f.Yf = ExtractResults->Y2Refined;
    ep->PreviousCoupleFlag = 1;
  }

  //update history from current results
  MkxUpdateCbaFromXyOne(ExtractResults->X1Refined, ExtractResults->Y1Refined, ExtractResults->X2Refined, ExtractResults->Y2Refined, &ExtractResults->Advanced.Cx, &ExtractResults->Advanced.Cy, &ExtractResults->Advanced.Br, &ExtractResults->Advanced.An);
  if (ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_UPDATEONLY || ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYANDUPDATE) //PL_080716
  {
    MkxUpdateHistory(&ep->History, t, ExtractResults, &ExtractResults->Advanced, ep->SSFactor, ExtractParam, &ep->ProcessPara);
    //increment index in history stack
    ep->History.CurrIndexInStack++;
    ep->History.CurrIndexInStack %= ep->History.StackSize;
  }

  ExtractResults->_Volatile = (void*)This;

  return(ErrCode);
}


void MkxMarkerCoupleFloat(CMkxCoupleFloat* cpl, CMkxCoupleInt mkcpl, CMkxMarkerCoupleRefined mkcplRefined, int Method)
{
  if (Method == MKX_CST_MARKER_INT || mkcplRefined.Advanced1.n < 1)
  {
    cpl->V1f.Xf = (float)mkcpl.V1.X;
    cpl->V1f.Yf = (float)mkcpl.V1.Y;
  }
  else
  {
    cpl->V1f = mkcplRefined.CplFine.V1f;
  }
  if (Method == MKX_CST_MARKER_INT || mkcplRefined.Advanced2.n < 1)
  {
    cpl->V2f.Xf = (float)mkcpl.V2.X;
    cpl->V2f.Yf = (float)mkcpl.V2.Y;
  }
  else
  {
    cpl->V2f = mkcplRefined.CplFine.V2f;
  }
}

void MkxExtractResultsInit(CMkxExtractResults* er)
{
  MkxInitAdvancedResults(&er->Advanced);
  er->X1Refined = -1.f;
  er->Y1Refined = -1.f;
  er->X2Refined = -1.f;
  er->Y2Refined = -1.f;
  er->Status = MKX_CST_EXTRACT_RES_SKIP;
  er->Locked = MKX_CST_EXTRACT_RES_UNLOCKED;
  er->RoiXMin = -1;
  er->RoiYMin = -1;
  er->RoiXMax = -1;
  er->RoiYMax = -1;
  er->RoiXMin1 = -1;
  er->RoiYMin1 = -1;
  er->RoiXMax1 = -1;
  er->RoiYMax1 = -1;
  er->RoiXMin2 = -1;
  er->RoiYMin2 = -1;
  er->RoiXMax2 = -1;
  er->RoiYMax2 = -1;
  er->TwoRoisFlag = -1;
  er->WireStatus = MKX_CST_WIRE_NOT_AVAILABLE;
  //er->StitchesStatus = MKX_CST_EXTRACT_RES_STITCHES_NOT_DETECTED;
  er->Confidence = -1.f;
  er->SwStatus = MKX_CST_EXTRACT_RES_STERNALWIRE_NOTAVAILABLE;
}

void MkxInitAdvancedResults(CMkxAdvancedResults* adv)
{
  adv->Dir1 = -1.f;
  adv->Dir2 = -1.f;
  adv->Dir1Confidence = -1.f;
  adv->Dir2Confidence = -1.f;
  adv->Length1 = -1.f;
  adv->Thickness1 = -1.f;
  adv->Length2 = -1.f;
  adv->Thickness2 = -1.f;
  adv->Intens1 = -1.f;
  adv->Intens2 = -1.f;
  adv->LevelIn1 = -1.f;
  adv->LevelOut1 = -1.f;
  adv->LevelIn2 = -1.f;
  adv->LevelOut2 = -1.f;
  adv->RefiningConf1 = -1.f;
  adv->RefiningConf2 = -1.f;
  adv->Radius1 = -1;
  adv->Radius2 = -1;
  adv->Cx = adv->Cy = adv->Br = adv->An = -1.f;
}


//get SwPels (for debug purpose)
static int MkxGetSwPels(int IwProc, int IhProc, const unsigned char *ImSwMsk, int SSFactor, const CMkxVs* imPel, int* NbSwPels, CMkxVi* SwPels)
{
  int q = 0;
  for (int i = 0, k = 0; i < IhProc; i++)
  {
    for (int j = 0; j<IwProc; j++, k++)
    {
      if (ImSwMsk[k]>0)
      {
        SwPels[q].X = j;
        SwPels[q].Y = i;
        q++;
      }
    }
  }
  *NbSwPels = q;

  if (SSFactor > 1)
  {
    for (int k = 0; k < *NbSwPels; k++)
      MkxUpSampleCoordinates(imPel, IwProc, &SwPels[k].X, &SwPels[k].Y);
  }

  return 0;
}

/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
