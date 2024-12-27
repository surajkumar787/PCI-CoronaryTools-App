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

#include <MkxDev.h>

/*************************************************************************************/
// Input interface function with tracking process : provide inputs to tracker
/*************************************************************************************/
void MkxSetTrackingInput(
  CTr1Input *TrIn,                                                      //output
  int NbTrInCouples, const CMkxBlobCouple*  RawCouples, CMkxClue Clue //inputs
  )
{
  //CTr1Tracking *This = (CTr1Tracking*)TrackingHdl;

  // Set field NbCouple
  TrIn->NbCouple = NbTrInCouples;

  // Set field Couples X1, Y1, X2, Y2, Strength
  for (int i = 0; i < TrIn->NbCouple; i++)
  {
    TrIn->Couples[i].X1 = RawCouples[i].Blob1.V.X;
    TrIn->Couples[i].Y1 = RawCouples[i].Blob1.V.Y;
    TrIn->Couples[i].X2 = RawCouples[i].Blob2.V.X;
    TrIn->Couples[i].Y2 = RawCouples[i].Blob2.V.Y;
    TrIn->Couples[i].Strength = RawCouples[i].CplMerit.CplMeritOverall;
  }

  // Set field Clue
  // New 15-06-04
  if (Clue.Nature == MKX_CST_CLUE_NOTAVAILABLE){
    TrIn->Clue.Nature = TR1_CLUE_NOTAVAILABLE;
    return;
  }

  // Set field Clue X1, Y1, X2, Y2, Nature

  if (Clue.Nature == MKX_CST_CLUE_DEBUG)        TrIn->Clue.Nature = TR1_CLUE_DEBUG;
  if (Clue.Nature == MKX_CST_CLUE_STRONG)       TrIn->Clue.Nature = TR1_CLUE_STRONG;
  TrIn->Clue.X1 = (int)Clue.X1f;
  TrIn->Clue.Y1 = (int)Clue.Y1f;
  TrIn->Clue.X2 = (int)Clue.X2f;
  TrIn->Clue.Y2 = (int)Clue.Y2f;

}
/***************************************************************************/
// CouplePrint :                                                     */
// Printing the content of the label-couple array                          */
/***************************************************************************/
void CouplePrint(CMkxBlobCouple *Couples, int NbCouples)
{
  int j;

  printf("CouplePrint START ****************\n");
  for (j = 0; j < NbCouples; j++)
  {
	  CMkxBlobCouple * Pt = &(Couples[j]);
    printf("Cpl %3d : Markers:(%3d %3d) (%3d %3d) Merit=%.3f Rank=%3d MatchingRef=%d\n",
      j, Pt->Blob1.V.X, Pt->Blob1.V.Y, Pt->Blob2.V.X, Pt->Blob2.V.Y, Pt->CplMerit.CplMeritOverall, Pt->Rank, Pt->MatchingRef);
  }
  printf("CouplePrint END ******************\n");
}

/*************************************************************************************/
// Output interface function with tracking process : get results from tracker
/*************************************************************************************/
void MkxGetTrackingOutput(const CTr1Output *TrOut, int* NbTrOutCouples,  CMkxBlobCouple* TrCouples, int* Status , int* Locked )
{
  // re-initializing marker couples to zero 
  for (int i = 0; i < TrOut->NbCouple; i++)
  {
    MkxBlobCoupleInit(&TrCouples[i]);
  }

  for (int i = 0; i < TrOut->NbCouple; i++)
  {
    TrCouples[i].Blob1.V.X = TrOut->Couples[i].X1;
    TrCouples[i].Blob1.V.Y = TrOut->Couples[i].Y1;
    TrCouples[i].Blob2.V.X = TrOut->Couples[i].X2;
    TrCouples[i].Blob2.V.Y = TrOut->Couples[i].Y2;
    TrCouples[i].CplMerit.CplMeritOverall = TrOut->Couples[i].Strength;
    TrCouples[i].Rank = i + 1;
  }
  *NbTrOutCouples = TrOut->NbCouple;
  *Status = TrOut->CTr1OutputStatus== TR1_RES_OK ? MKX_CST_EXTRACT_RES_OK : MKX_CST_EXTRACT_RES_SKIP;
  *Locked = TrOut->Locked == TR1_RES_LOCKED ? MKX_CST_EXTRACT_RES_LOCKED : MKX_CST_EXTRACT_RES_UNLOCKED;
}

/*************************************************************************************/
// MkxUpdateTrackingNonCriticalParam
/*************************************************************************************/
void MkxUpdateTrackingNonCriticalParam(CTr1Hdl TrackerHdl, CMkxProcessPara *pp, CPrmPara* ExtractParam)
{
  CTr1Param* ParamPt;
  if (TrackerHdl == 0) return; // When the tracker is not started

  Tr1GetParamPt(TrackerHdl, &ParamPt);

  ParamPt->MaxNbAlarm = ExtractParam->Val[MKX_TRMAXNBCOUPLE_I].Int;
  ParamPt->InitTime = ExtractParam->Val[MKX_TRINITTIME_I].Int;
  ParamPt->LockCount = ExtractParam->Val[MKX_TRLOCKCOUNT_I].Int;
  ParamPt->VirtualPenalize = ExtractParam->Val[MKX_TRVIRTUALPENALIZE_I].Int;
  ParamPt->LockMaxConSkip = ExtractParam->Val[MKX_TRLOCKMAXCONSKIP_I].Int;

  ParamPt->TrAlpha = ExtractParam->Val[MKX_TRALPHA_F].Float;

  ParamPt->SkipAngleDist = pp->TrAngleSkipInDeg;
  ParamPt->HighAngleDist = pp->TrAngleHighInDeg;

  ParamPt->SkipBreadthDist = pp->TrBreadthSkipPercent;
  ParamPt->HighBreadthDist = pp->TrBreadthHighPercent;

  ParamPt->SkipCentroidDist = pp->TrCentroidSkipInPix;
  ParamPt->HighCentroidDist = pp->TrCentroidHighInPix;

  ParamPt->StrengthImpact = ExtractParam->Val[MKX_TRSTRENGTHIMPACT_F].Float;
}


/*************************************************************************************/
// SetTrackingParam : Load TrParam structure
/*************************************************************************************/
void MkxSetTrackingParam(CTr1Param* TrParam, CPrmPara *ExtractParam, CPrmParaCtx* ParaContext, int IwProc, int IhProc, CMkxProcessPara *pp)
{
  // Setting tracker parameters in local structure
  TrParam->MaxNbAlarmEver = ParaContext->Ctx[MKX_TRMAXNBCOUPLE_I].Max.Int;

  TrParam->MaxNbAlarm = ExtractParam->Val[MKX_TRMAXNBCOUPLE_I].Int;
  TrParam->InitTime = ExtractParam->Val[MKX_TRINITTIME_I].Int;
  TrParam->LockCount = ExtractParam->Val[MKX_TRLOCKCOUNT_I].Int;
  TrParam->VirtualPenalize = ExtractParam->Val[MKX_TRVIRTUALPENALIZE_I].Int;
  TrParam->LockMaxConSkip = ExtractParam->Val[MKX_TRLOCKMAXCONSKIP_I].Int;
  TrParam->Iw = IwProc;
  TrParam->Ih = IhProc;
  TrParam->VirtualWhileInit = 0;   // Whether or not virtual tracks are allowed during the tracker init phase
  TrParam->WarningOnFlag = 0;   // No warning from tracker (ex: in case of too many or too few alarms)  
  TrParam->StackDepth = 100; // StackDepth >= 2

  TrParam->TrAlpha = ExtractParam->Val[MKX_TRALPHA_F].Float;

  TrParam->LowAngleDist = (float)0;
  TrParam->SkipAngleDist = pp->TrAngleSkipInDeg;
  TrParam->HighAngleDist = pp->TrAngleHighInDeg;

  TrParam->LowBreadthDist = (float)0;
  TrParam->SkipBreadthDist = pp->TrBreadthSkipPercent;
  TrParam->HighBreadthDist = pp->TrBreadthHighPercent;

  TrParam->LowCentroidDist = (float)0;
  TrParam->SkipCentroidDist = pp->TrCentroidSkipInPix;
  TrParam->HighCentroidDist = pp->TrCentroidHighInPix;

  TrParam->CoefAngle = (float)1;
  TrParam->CoefBreadth = (float)1;
  TrParam->CoefCentroid = (float)0.5;

  TrParam->StrengthImpact = ExtractParam->Val[MKX_TRSTRENGTHIMPACT_F].Float;
}

/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
