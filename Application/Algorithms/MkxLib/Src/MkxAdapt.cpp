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
#include <math.h>
#include <MkxDev.h>

int MkxHistoryRoiFromPredict(
  CMkxRoiCoord *historyRoi,
  const CMkxHistory *history,
  int iwProc, int ihProc
  )
{
  CMkxPredict *predict;
  int curIndex = history->CurrIndexInStack - 1;
  curIndex += history->StackSize;
  curIndex %= history->StackSize;
  predict = &history->Plane[curIndex].HistoryPredict;

  if (predict->PredictValid == 1
    && predict->FeatPredict_available[MKX_CPL_Cx] == 1
    && predict->FeatPredict_available[MKX_CPL_Cy] == 1
    && predict->FeatPredict_available[MKX_CPL_Br] == 1
    )
  {
    float br = (predict->FeatPredict[MKX_CPL_Br].PredictVal + predict->FeatPredict[MKX_CPL_Br].FuzzyHigh); // the component related to breadth

    //radius=0;//PL_110722
    float an = predict->FeatPredict[MKX_CPL_An].PredictVal;
    float halfbrx = 0.5f * br * MKX_ABS(cos(an*MKX_PI / 180.0f));
    float halfbry = 0.5f * br * MKX_ABS(sin(an*MKX_PI / 180.0f));

    float cxMin = predict->FeatPredict[MKX_CPL_Cx].PredictVal - predict->FeatPredict[MKX_CPL_Cx].FuzzyHigh * 1.5f;
    float cxMax = predict->FeatPredict[MKX_CPL_Cx].PredictVal + predict->FeatPredict[MKX_CPL_Cx].FuzzyHigh * 1.5f;
    float cyMin = predict->FeatPredict[MKX_CPL_Cy].PredictVal - predict->FeatPredict[MKX_CPL_Cy].FuzzyHigh * 1.5f;
    float cyMax = predict->FeatPredict[MKX_CPL_Cy].PredictVal + predict->FeatPredict[MKX_CPL_Cy].FuzzyHigh * 1.5f;

    MkxRoiCoord(historyRoi, (int)MKX_MAX(0, cxMin - halfbrx), (int)MKX_MAX(0, cyMin - halfbry), (int)MKX_MIN(iwProc - 1, cxMax + halfbrx), (int)MKX_MIN(ihProc - 1, cyMax + halfbry));
  }
  else//no history roi
  {
    MkxRoiCoordInit(historyRoi);
    return 1;
  }
  return 0;
}

int MkxHistoryRoiFromStat(
  CMkxRoiCoord *historyRoi,
  const CMkxHistory *history,
  int iwProc, int ihProc,
  float historyFactor
  )
{
#define COEF_STDV 2.0f
#define MARGIN 20.0f
  int xMargin = (int)MARGIN, yMargin = (int)MARGIN; //PL_110725 reduced after changing 4stdv to be coherent with 3stdv for fuzzyHigh
  int curIndex = history->CurrIndexInStack - 1;
  curIndex += history->StackSize;
  curIndex %= history->StackSize;
  CMkxHistoryStat *historyStat = &history->Plane[curIndex].HistoryStatShallow;
  float coefFuzzy = 1 + 2 * (1 - historyFactor); //PL_080725

  float coefStdv = coefFuzzy * COEF_STDV;

  if (historyStat->FeatStat[MKX_CPL_Cx].N > 0)
  {
    float br = (historyStat->FeatStat[MKX_CPL_Br].Mean + historyStat->FeatStat[MKX_CPL_Br].Stdv *coefStdv); // the component related to breadth
    float an = historyStat->FeatStat[MKX_CPL_An].Mean;
    float halfbrx = 0.5f * br * MKX_ABS(cos(an*MKX_PI / 180.0f));
    float halfbry = 0.5f * br * MKX_ABS(sin(an*MKX_PI / 180.0f));

    float cxMean = historyStat->FeatStat[MKX_CPL_Cx].Mean;
    float cxStdv = historyStat->FeatStat[MKX_CPL_Cx].Stdv;
    float xMin = cxMean - halfbrx - coefStdv * cxStdv - xMargin;
    float xMax = cxMean + halfbrx + coefStdv * cxStdv + xMargin;

    float cyMean = historyStat->FeatStat[MKX_CPL_Cy].Mean;
    float cyStdv = historyStat->FeatStat[MKX_CPL_Cy].Stdv;
    float yMin = cyMean - halfbry - coefStdv * cyStdv - yMargin;
    float yMax = cyMean + halfbry + coefStdv * cyStdv + yMargin;

    MkxRoiCoord(historyRoi, (int)MKX_MAX(0, xMin), (int)MKX_MAX(0, yMin), (int)MKX_MIN(iwProc - 1, xMax), (int)MKX_MIN(ihProc - 1, yMax));
  }
  else//no history roi
  {
    MkxRoiCoordInit(historyRoi);
    return 1;
  }
  return 0;
}

int MkxHistoryRoi(
  CMkxRoiCoord *historyRoi,
  const CMkxHistory *history,
  int iwProc, int ihProc,
  CPrmPara   *ExtractParam
  )
{
  int ret = 0;
  if (ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYANDUPDATE)
  {
    ret = MkxHistoryRoiFromPredict(
      historyRoi,
      history,
      iwProc, ihProc
      );
  }
  else if (ExtractParam->Val[MKX_HISTORYFLAG_I].Int == MKX_CST_HISTORY_APPLYONLY)
  {
    ret = MkxHistoryRoiFromStat(
      historyRoi,
      history,
      iwProc, ihProc,
      ExtractParam->Val[MKX_HISTORYFACTOR_F].Float
      );
  }
  else
  {
    ret = 1;//no history roi
  }
  return ret;
}

static int  MkxSubRoi(int x, int y, CMkxRoiCoord *in_roi, CMkxRoiCoord *out_roi)
{
  int dx0, dx1, dy0, dy1;

  dx0 = x - in_roi->XMin;
  dx1 = in_roi->XMax - x;
  dy0 = y - in_roi->YMin;
  dy1 = in_roi->YMax - y;

  if (dx0 <= 0 || dx1 <= 0 || dy0 <= 0 || dy1 <= 0 ||
    dx0 >= in_roi->Width || dx1 >= in_roi->Width ||
    dy0 >= in_roi->Height || dy1 >= in_roi->Height)
    return 1;

  if (dx0 <= dx1 && dy0 <= dy1)
  {
    out_roi->XMin = in_roi->XMin;
    out_roi->YMin = in_roi->YMin;
    out_roi->XMax = in_roi->XMin + 2 * dx0;
    out_roi->YMax = in_roi->YMin + 2 * dy0;
  }
  else if (dx0 > dx1 && dy0 <= dy1)
  {
    out_roi->XMin = in_roi->XMax - 2 * dx1;
    out_roi->YMin = in_roi->YMin;
    out_roi->XMax = in_roi->XMax;
    out_roi->YMax = in_roi->YMin + 2 * dy0;
  }
  else if (dx0 > dx1 && dy0 > dy1)
  {
    out_roi->XMin = in_roi->XMax - 2 * dx1;
    out_roi->YMin = in_roi->YMax - 2 * dy1;
    out_roi->XMax = in_roi->XMax;
    out_roi->YMax = in_roi->YMax;
  }
  else if (dx0 <= dx1 && dy0 > dy1)
  {
    out_roi->XMin = in_roi->XMin;
    out_roi->YMin = in_roi->YMax - 2 * dy1;
    out_roi->XMax = in_roi->XMin + 2 * dx0;
    out_roi->YMax = in_roi->YMax;
  }
  return 0;
}

//
/*************************************************************************************/
// Build a ROI that will act as a default ROI
// Change Work and Useful ROI according to detected markers
// HalfW = 0.5 * DX + RotationFac * DY + DilationFac * DX +  TranslationOffset
// 0.5 * DX: minimal size (to cope with static markers) 
// RotationFac * DY + DilationFac * DX: variable part predictive of potential motion 
//                                      proportional to marker distance
// TranslationOffset: security part (with adaptivity factor) from 0 to 0.25 image size
/*************************************************************************************/
static int  MkxAdaptRoi(
  CMkxRoiCoord *UsefulRoiCoord,
  int TwoRoisFlag,
  CTr1Output TrOut, float AdaptFac, int Iw, int Ih
  )
{
  int HalfW, HalfH, X0, Y0, DX, DY;
  static int X1, Y1, X2, Y2;
  float TranslationOffset;
  double RotationFac = 0.2, DilationFac = 0.1;//set on 17-02-05
  CMkxRoiCoord LockRoi;
  float FactorForSkippedCase = 1.5;

  TranslationOffset = ((float)1 - AdaptFac) * (float)sqrt((float)(Iw*Iw + Ih*Ih)) / 5;

  if (TrOut.CTr1OutputStatus == MKX_CST_EXTRACT_RES_SKIP)    //PL-060621
  {
    RotationFac *= FactorForSkippedCase;
    DilationFac *= FactorForSkippedCase;
    TranslationOffset *= FactorForSkippedCase;
  }

  // modify ROI according to ExtractResults
  if (TrOut.CTr1OutputStatus == MKX_CST_EXTRACT_RES_OK)
  {
    X1 = TrOut.X1; Y1 = TrOut.Y1;
    X2 = TrOut.X2; Y2 = TrOut.Y2;
  }
  X0 = (X1 + X2) / 2;
  Y0 = (Y1 + Y2) / 2;
  DX = MKX_ABS(X2 - X1);
  DY = MKX_ABS(Y2 - Y1);
  HalfW = (int)(0.5 * DX + RotationFac * DY + DilationFac * DX + TranslationOffset);
  HalfH = (int)(0.5 * DY + RotationFac * DX + DilationFac * DY + TranslationOffset);

  LockRoi.XMin = X0 - HalfW;
  LockRoi.YMin = Y0 - HalfH;
  LockRoi.XMax = X0 + HalfW;
  LockRoi.YMax = Y0 + HalfH;
  LockRoi.Width = 2 * HalfW;
  LockRoi.Height = 2 * HalfH;

  if (TwoRoisFlag) //compute UsefulRoiCoord[1] and UsefulRoiCoord[2]
  {
    if (MkxSubRoi(X1, Y1, &LockRoi, &UsefulRoiCoord[1]))
      return 1;
    if (MkxSubRoi(X2, Y2, &LockRoi, &UsefulRoiCoord[2]))
      return 1;
    MkxClipRoi(&UsefulRoiCoord[1], UsefulRoiCoord[0], UsefulRoiCoord[1]);
    MkxClipRoi(&UsefulRoiCoord[2], UsefulRoiCoord[0], UsefulRoiCoord[2]);
    MkxClipRoi(&UsefulRoiCoord[0], LockRoi, UsefulRoiCoord[0]); //update UsefulRoiCoord for AdaptiveNbLabelsKept
  }
  else //copy UsefulRoiCoord in UsefulRoiCoord[1] and UsefulRoiCoord[2]
  {
    MkxClipRoi(&UsefulRoiCoord[0], LockRoi, UsefulRoiCoord[0]);
    MkxClipRoi(&UsefulRoiCoord[1], UsefulRoiCoord[0], UsefulRoiCoord[0]);
    MkxClipRoi(&UsefulRoiCoord[2], UsefulRoiCoord[0], UsefulRoiCoord[0]);
  }
  return 0;
}



void MkxAdaptNbLabelsKept(int* AdaptiveWorkNbPixelsKept, int* AdaptiveWorkNbLblBlobsKept,
  int EstimatedMaxNbPixPerBlob, CMkxRoiCoord roi, int IwProc, int IhProc, CPrmPara *ExtractParam)
{
  int NbLabelsKept_Offset = 7; // PL_110408
  int Width, Height;
  float ScaleFactor, ScaleFactorCoef = 2.f; //to moderate the ScaleFactor impact

  Width = roi.XMax - roi.XMin;
  Height = roi.YMax - roi.YMin;

  ScaleFactor = (float)(Width * Height) / (float)(IwProc * IhProc);
  ScaleFactor = MKX_MIN(1, ScaleFactorCoef * ScaleFactor);
  *AdaptiveWorkNbLblBlobsKept = MKX_MAX(NbLabelsKept_Offset, (int)(ScaleFactor * ExtractParam->Val[MKX_CPLSELECTNBLABELSKEPT_I].Int));
  *AdaptiveWorkNbPixelsKept = (*AdaptiveWorkNbLblBlobsKept) * EstimatedMaxNbPixPerBlob;
}

void MkxAdaptMarkersDist(int* AdaptiveWorkMarkersDist, CTr1Output TrOut)
{
  // modify AdaptiveWorkMarkersDist according to ExtractResults
  if (TrOut.CTr1OutputStatus == MKX_CST_EXTRACT_RES_OK)
  {
	int X1, Y1, X2, Y2;
    X1 = TrOut.X1; Y1 = TrOut.Y1;
    X2 = TrOut.X2; Y2 = TrOut.Y2;
    *AdaptiveWorkMarkersDist = MKX_RND(MKX_EUCLI_DIST(X1, Y1, X2, Y2));
  }
}


int  MkxAdaptWhenLocked(
  int* AdaptiveWorkMarkersDist, int* AdaptiveWorkNbPixelsKept, int* AdaptiveWorkNbLblBlobsKept, CMkxRoiCoord* UsefulRoiCoord, float* AdaptiveDistTolerMin, float* AdaptiveDistTolerMax,
  int EstimatedMaxNbPixPerBlob, int IwProc, int IhProc, CTr1Output TrOut,
  int CplSelectDistToler,
  CPrmPara *ExtractParam
  )
{
  int Tolerance_WhenLocked = MKX_CST_DIST_TOLERANCE_WHEN_LOCKED; // set on 17-02-05 // set tolerance to 20%

  // Adapt ROI and NBLABELSKEPT
  if (ExtractParam->Val[MKX_ADAPTIVEROIFLAG_I].Int == MKX_CST_ADAPT
    || ExtractParam->Val[MKX_ADAPTIVEROIFLAG_I].Int == MKX_CST_ADAPT_2ROIS
    )
  {
	int ret;
    int TwoRoisFlag = (ExtractParam->Val[MKX_ADAPTIVEROIFLAG_I].Int == MKX_CST_ADAPT_2ROIS) ? 1 : 0;
    ret = MkxAdaptRoi(
      UsefulRoiCoord,
      TwoRoisFlag,
      TrOut, ExtractParam->Val[MKX_ADAPTIVEROIFACTOR_F].Float, IwProc, IhProc
      );


    if (ret) return 1; //wrong roi

    if (ExtractParam->Val[MKX_ADAPTIVENBLABELSKEPTFLAG_I].Int == MKX_CST_ADAPT)
      MkxAdaptNbLabelsKept(AdaptiveWorkNbPixelsKept, AdaptiveWorkNbLblBlobsKept, EstimatedMaxNbPixPerBlob, UsefulRoiCoord[0], IwProc, IhProc, ExtractParam);
  }
  // Adapt Distance and Tolerance
  if (ExtractParam->Val[MKX_ADAPTIVEMARKERSDISTFLAG_I].Int == MKX_CST_ADAPT)
  {
    MkxAdaptMarkersDist(AdaptiveWorkMarkersDist, TrOut);

    if (TrOut.CTr1OutputStatus == MKX_CST_EXTRACT_RES_OK)
    {
      int Toler = MKX_MIN(Tolerance_WhenLocked, CplSelectDistToler);
      MkxDistToler(
        AdaptiveDistTolerMin, AdaptiveDistTolerMax,
        *AdaptiveWorkMarkersDist,
        Toler
        );
    }
  }
  return 0;
}

/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
