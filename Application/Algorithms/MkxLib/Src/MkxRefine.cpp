// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************



/*************************************************************************************/
// MkxRefine.c  PL  2006-03-22
/*************************************************************************************/

#include <MkxDev.h> 

/*************************************************************************************/
// MkxRefineMarkersAndGetBlobSize: get refined markers from image and marker coordinates in integer
/*************************************************************************************/
int	 MkxRefineMarkersAndGetBlobSize(
  CMkxHdl MkxHdl, 
  short* InIma, int x1, int y1, int x2, int y2, 
  float* x1Refined, float* y1Refined, float* x2Refined, float* y2Refined, 
  CMkxAdvancedResults *advancedResults
  )
{
  CMkx* This = (CMkx*)(MkxHdl).Pv;
  CPrmPara *ExtractParam = &(This->ActualPara);
  CMkxExtractProc  *ep = &(This->ExtractProc);

  PrmGetPara(This->PrmHdl, ExtractParam);

  MkxSetExtractProcFromCriticalParam(&ep->SwStatus,
    &ep->SSFactor, &ep->IwProc, &ep->IhProc,
    &ep->SSFactor_SW, &ep->IwProc_SW, &ep->IhProc_SW, &ep->UserRoiCoord_SW,
    ep->IwOri, ep->IhOri, ExtractParam, ep->SeqPara);

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

  CMkxCoupleInt MarkerCoupleInt;
  MarkerCoupleInt.V1.X = x1;
  MarkerCoupleInt.V1.Y = y1;
  MarkerCoupleInt.V2.X = x2;
  MarkerCoupleInt.V2.Y = y2;

  CMkxMarkerCoupleRefined MarkerCoupleRefined;

  MkxRefining(
    &MarkerCoupleRefined,
    MarkerCoupleInt,
    (const short*)InIma, ep->IwOri, ep->IhOri,
    ep->ProcessPara.BlobEnhRingRadiusInPix, ep->SSFactor
    );

  *x1Refined=MarkerCoupleRefined.CplFine.V1f.Xf;
  *y1Refined=MarkerCoupleRefined.CplFine.V1f.Yf;
  *x2Refined=MarkerCoupleRefined.CplFine.V2f.Xf;
  *y2Refined= MarkerCoupleRefined.CplFine.V2f.Yf;

  float Cx, Cy, Br, An;
  MkxUpdateCbaFromXyOne(*x1Refined, *y1Refined, *x2Refined, *y2Refined, &Cx, &Cy, &Br, &An);


  advancedResults->An = An;
  advancedResults->Br = Br;
  advancedResults->Cx = Cx;
  advancedResults->Cy = Cy;
  advancedResults->Dir1 = ep->MarkerCoupleRefined.Advanced1.Dir;
  advancedResults->Dir1Confidence = ep->MarkerCoupleRefined.Advanced1.DirConfidence;
  advancedResults->Dir2 = ep->MarkerCoupleRefined.Advanced2.Dir;
  advancedResults->Dir2Confidence = ep->MarkerCoupleRefined.Advanced2.DirConfidence;
  advancedResults->Intens1 = -1;
  advancedResults->Intens2 = -1;
  advancedResults->Length1 = ep->MarkerCoupleRefined.Advanced1.a;
  advancedResults->Length2 = ep->MarkerCoupleRefined.Advanced2.a;
  advancedResults->LevelIn1 = ep->MarkerCoupleRefined.Advanced1.level_in;
  advancedResults->LevelIn2 = ep->MarkerCoupleRefined.Advanced2.level_in;
  advancedResults->LevelOut1 = ep->MarkerCoupleRefined.Advanced1.level_out;
  advancedResults->LevelOut2 = ep->MarkerCoupleRefined.Advanced2.level_out;
  advancedResults->RefiningConf1 = ep->MarkerCoupleRefined.Advanced1.err;
  advancedResults->RefiningConf2 = ep->MarkerCoupleRefined.Advanced2.err;
  advancedResults->Thickness1 = ep->MarkerCoupleRefined.Advanced1.b;
  advancedResults->Thickness2 = ep->MarkerCoupleRefined.Advanced2.b;

  return 0;
}

///************************************************************************************/
//// End of file
///*************************************************************************************/
