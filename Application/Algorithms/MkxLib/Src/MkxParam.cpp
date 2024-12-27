// **********************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. * 
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        * 
// * in whole or in part is prohibited without the prior written        * 
// * consent of the copyright owner.                                    * 
// *                                                                         *
// **********************************************************************

#include <stdio.h> 
#include <stdlib.h> 

#include <MkxDev.h>

int   MkxFreeProcessImages(CMkxExtractProc *ep)
{
  MKX_FREE(ep->ImBlobMin1);
  MKX_FREE(ep->ImSub1);
  MKX_FREE(ep->ImEnh1);
  MKX_FREE(ep->ImPel1);
  MKX_FREE(ep->ImThr1);
  MKX_FREE(ep->ImLab1);

  MKX_FREE(ep->ImSubSw);
  MKX_FREE(ep->ImPelSw);
  MKX_FREE(ep->ImSwMskSub);
  MKX_FREE(ep->ImSwMskProc);

  return 0;
}

int   MkxAllocProcessImages(CMkxExtractProc *ep, int Iw, int Ih)
{
  ep->ImBlobMin1 = (float *)calloc(Iw* Ih, sizeof(float));
  ep->ImSub1 = (short *)calloc(Iw* Ih, sizeof(short));
  ep->ImEnh1 = (float *)calloc(Iw* Ih, sizeof(float));
  ep->ImPel1 = (CMkxVs *)calloc(Iw* Ih, sizeof(CMkxVs));
  ep->ImLab1 = (int *)calloc(Iw* Ih, sizeof(int));
  ep->ImThr1 = (unsigned char  *)calloc(Iw* Ih, sizeof(unsigned char));

  return 0;
}
int   MkxAllocProcessImages_SW(CMkxExtractProc *ep, int Iw, int Ih)
{
  ep->ImSubSw = (short *)calloc(Iw* Ih, sizeof(short));
  ep->ImPelSw = (CMkxVs *)calloc(Iw* Ih, sizeof(CMkxVs));
  ep->ImSwMskSub = (unsigned char *)calloc(Iw* Ih, sizeof(char));

  return 0;
}


/*************************************************************************************/
// Set MarginRoiCoord (the largest useable ROI)
// Check coherency with margins
// Margin for taking into account circular excursion
// Margin1 = extractPara->Val[MKX_BLOBENHRINGRADIUS_F].Float + MKX_BORDER_FOR_GAUSS + 1;
// Margin for taking into account correlation area
// Margin2 = extractPara->Val[MKX_TRCORRELMSKR_I].Int + extractPara->Val[MKX_TRCORRELEXCUR_I].Int;
// Take the maximum of the 2 margins
// Margin = MKX_MAX(Margin1, Margin2); 
/*************************************************************************************/
static int  SetExtractMarginRoi(
  float radiusInPix,int IwProc, int IhProc,
  CMkxRoiCoord *MarginRoiCoord,  int *pt_Margin
  )
{
  int Err = 0;
  int Radius, Margin;

  Radius = (int)(1 + radiusInPix);
  Margin = MKX_GET_X_MARGIN(Radius);

  MarginRoiCoord->XMin = Margin;
  MarginRoiCoord->XMax = IwProc - 1 - Margin;
  MarginRoiCoord->YMin = Margin;
  MarginRoiCoord->YMax = IhProc - 1 - Margin;
  *pt_Margin = Margin;

  if (MarginRoiCoord->XMin > MarginRoiCoord->XMax || MarginRoiCoord->YMin > MarginRoiCoord->YMax)
    Err = MKX_ERR_INTERNAL;

  return Err;
}


static int	SetExtractUserRoi(
  CMkxSeqPara seqPara, int IwProc, int IhProc,  CMkxRoiCoord MarginRoiCoord, 
  CMkxRoiCoord *UserRoiCoord,
  CPrmPara *extractPara
  )
{
  int Warning = 0, Rc;
  CMkxRoiCoord clipRoiCoord;

  //set coordinates of Default Roi for extract process from user parameters
  if (extractPara->Val[MKX_DEFROIFLAG_I].Int == MKX_CST_EXTRACT_ROI_NODEF)
  {
    clipRoiCoord = MarginRoiCoord;
  }
  else
  {
    clipRoiCoord.XMin = (int)(seqPara.DefRoiFacXMin * (IwProc-1));
    clipRoiCoord.XMax = (int)(seqPara.DefRoiFacXMax * (IwProc-1));
    clipRoiCoord.YMin = (int)(seqPara.DefRoiFacYMin * (IhProc-1));
    clipRoiCoord.YMax = (int)(seqPara.DefRoiFacYMax * (IhProc-1));
  }

  // check Roi validity and correct
  if (clipRoiCoord.XMin > clipRoiCoord.XMax || clipRoiCoord.YMin > clipRoiCoord.YMax)
  {
    Warning = 1;           // If ROI incoherent,
    *UserRoiCoord = MarginRoiCoord; // use the maximal ROI
  }
  else
    MkxClipRoi(UserRoiCoord, MarginRoiCoord, clipRoiCoord ); // If ROI coherent, possibly clip it but don't return a warning if clipped.

  Rc = ((Warning) ? MKX_WRN_WRONG_EXTRACT_ROI : 0);
  return Rc;
}

/*************************************************************************************/
// Set Roi coordinates from user parameters and apply adjustments if necessary
/*************************************************************************************/
static int	SetExtractInitRoi(
  CMkxSeqPara seqPara, int IwProc, int IhProc,  CMkxRoiCoord MarginRoiCoord, 
  CMkxRoiCoord *UserInitRoiCoord,
  CPrmPara *extractPara
  )
{
  int Warning = 0, Rc;
  CMkxRoiCoord clipRoiCoord;

  //set coordinates of Default Roi for extract process from user parameters
  if (extractPara->Val[MKX_INITROIFLAG_I].Int == MKX_CST_EXTRACT_ROI_NODEF)
  {
    clipRoiCoord = MarginRoiCoord;
  }
  else
  {
    clipRoiCoord.XMin = (int)(seqPara.InitRoiFacXMin * (IwProc-1));
    clipRoiCoord.XMax = (int)(seqPara.InitRoiFacXMax * (IwProc-1));
    clipRoiCoord.YMin = (int)(seqPara.InitRoiFacYMin * (IhProc-1));
    clipRoiCoord.YMax = (int)(seqPara.InitRoiFacYMax * (IhProc-1));
  }
  // check Roi validity and correct
  if (clipRoiCoord.XMin > clipRoiCoord.XMax || clipRoiCoord.YMin > clipRoiCoord.YMax)
  {
    Warning = 1;           // If ROI incoherent,
    *UserInitRoiCoord = MarginRoiCoord; // use the maximal ROI
  }
  else
    MkxClipRoi(UserInitRoiCoord, MarginRoiCoord, clipRoiCoord); // If ROI coherent, possibly clip it but don't return a warning if clipped.

  Rc = ((Warning) ? MKX_WRN_WRONG_EXTRACT_ROI : 0);
  return Rc;
}



/*************************************************************************************/
// Set Roi coordinates from user parameters and apply adjustments if necessary
/*************************************************************************************/
static void SetExtractRoi(
  CMkxRoiCoord *MarginRoiCoord, int *Margin, CMkxRoiCoord *UserInitRoiCoord, CMkxRoiCoord *UserRoiCoord, 
  CMkxSeqPara SeqPara,
  int IwProc, int IhProc, float BlobEnhRingRadiusInPix,
  CPrmPara *extractPara
  )
{
  SetExtractMarginRoi(
    BlobEnhRingRadiusInPix, IwProc, IhProc,
    MarginRoiCoord, Margin
    );
  SetExtractInitRoi(
    SeqPara, IwProc, IhProc, *MarginRoiCoord,
    UserInitRoiCoord, 
    extractPara
    );
  SetExtractUserRoi(
    SeqPara, IwProc, IhProc, *MarginRoiCoord,
    UserRoiCoord,
    extractPara
    );
}

/*************************************************************************************/
// Set Roi coordinates from user parameters and apply adjustments if necessary
/*************************************************************************************/
void  MkxDistToler(
  float *AdaptiveDistTolerMin, float *AdaptiveDistTolerMax, 
  int dist,
  int DistToler 
  )
{
  int Offset = 10;
  *AdaptiveDistTolerMin = MKX_MAX(((float)dist * ((float)1 - (float)DistToler*(float)0.01) - Offset), (float)0);
  *AdaptiveDistTolerMax = ((float)dist * ((float)1 + (float)DistToler*(float)0.01) + Offset);
}

void PrmParaToProcessPara(
  CMkxProcessPara* pp, 
  CMkxSeqPara seqPara, int SSFactor,
  CPrmPara *extractPara
  )
{
  pp->BlobEnhRingRadiusInPix = extractPara->Val[MKX_BLOBENHRINGRADIUS_F].Float / (float)(seqPara.MmPerPixel * SSFactor);
  pp->BlobEnhRingSigInPix = extractPara->Val[MKX_BLOBENHRINGSIG_F].Float*extractPara->Val[MKX_BLOBENHRINGRADIUS_F].Float / (float)(seqPara.MmPerPixel*SSFactor);
  pp->TrAngleHighInDeg = extractPara->Val[MKX_TRANGLEHIGH_F].Float / ((float)seqPara.Fps*0.9f);//0.9f = 90/100 conversion deg to grad
  pp->TrAngleSkipInDeg = extractPara->Val[MKX_TRANGLESKIP_F].Float / ((float)seqPara.Fps*0.9f);
  pp->TrBreadthSkipPercent = extractPara->Val[MKX_TRBREADTHSKIP_F].Float / (float)seqPara.Fps;//PL_110407 bug fixed
  pp->TrBreadthHighPercent = extractPara->Val[MKX_TRBREADTHHIGH_F].Float / (float)seqPara.Fps;
  pp->TrCentroidHighInPix = extractPara->Val[MKX_TRCENTROIDHIGH_F].Float / (float)(seqPara.Fps*seqPara.MmPerPixel*SSFactor);//PL_110321
  pp->TrCentroidSkipInPix = extractPara->Val[MKX_TRCENTROIDSKIP_F].Float / (float)(seqPara.Fps*seqPara.MmPerPixel*SSFactor);
  pp->HistoryCentroidXLowInPix = extractPara->Val[MKX_HistoryCentroidXLow_I].Int / (float)(seqPara.MmPerPixel*SSFactor);
  pp->HistoryCentroidXHighInPix = extractPara->Val[MKX_HistoryCentroidXHigh_I].Int / (float)(seqPara.MmPerPixel*SSFactor);
  pp->HistoryCentroidYLowInPix = extractPara->Val[MKX_HistoryCentroidYLow_I].Int / (float)(seqPara.MmPerPixel*SSFactor);
  pp->HistoryCentroidYHighInPix = extractPara->Val[MKX_HistoryCentroidYHigh_I].Int / (float)(seqPara.MmPerPixel*SSFactor);
}

void PrintProcessPara(CMkxProcessPara *pp, CPrmPara *extractPara)
{
  printf("\n");
  printf("BlobEnhRingRadiusInPix    :%8.2f    :%8.2f \n", pp->BlobEnhRingRadiusInPix, extractPara->Val[MKX_BLOBENHRINGRADIUS_F].Float);
  printf("BlobEnhRingSigInPix       :%8.2f    :%8.2f \n", pp->BlobEnhRingSigInPix, extractPara->Val[MKX_BLOBENHRINGSIG_F].Float);
  printf("TrAngleHighInDeg          :%8.2f    :%8.2f \n", pp->TrAngleHighInDeg, extractPara->Val[MKX_TRANGLEHIGH_F].Float);
  printf("TrAngleSkipInDeg          :%8.2f    :%8.2f \n", pp->TrAngleSkipInDeg, extractPara->Val[MKX_TRANGLESKIP_F].Float);
  printf("TrBreadthHighPercent      :%8.2f    :%8.2f \n", pp->TrBreadthHighPercent, extractPara->Val[MKX_TRBREADTHHIGH_F].Float);
  printf("TrBreadthSkipPercent      :%8.2f    :%8.2f \n", pp->TrBreadthSkipPercent, extractPara->Val[MKX_TRBREADTHSKIP_F].Float);
  printf("TrCentroidHighInPix       :%8.2f    :%8.2f \n", pp->TrCentroidHighInPix, extractPara->Val[MKX_TRCENTROIDHIGH_F].Float);
  printf("TrCentroidSkipInPix       :%8.2f    :%8.2f \n", pp->TrCentroidSkipInPix, extractPara->Val[MKX_TRCENTROIDSKIP_F].Float);
  printf("HistoryCentroidXLowInPix  :%8.2f    :%8d \n", pp->HistoryCentroidXLowInPix, extractPara->Val[MKX_HistoryCentroidXLow_I].Int);
  printf("HistoryCentroidXHighInPix :%8.2f    :%8d \n", pp->HistoryCentroidXHighInPix, extractPara->Val[MKX_HistoryCentroidXHigh_I].Int);
  printf("HistoryCentroidYLowInPix  :%8.2f    :%8d \n", pp->HistoryCentroidYLowInPix, extractPara->Val[MKX_HistoryCentroidYLow_I].Int);
  printf("HistoryCentroidYHighInPix :%8.2f    :%8d \n", pp->HistoryCentroidYHighInPix, extractPara->Val[MKX_HistoryCentroidYHigh_I].Int);
  printf("\n");
}

int   MkxSetExtractProcFromOnTheFlyParam(
  CMkxProcessPara *pp, float *MarkersDistMm, int *CplSelectDistToler, int *SubMarkersDist, int *EstimatedMaxNbPixPerBlob, 
   int *WireCplMaxNbCouple, float *WireCplPruningFactor,
  CMkxRoiCoord *MarginRoiCoord, int *Margin, CMkxRoiCoord *UserInitRoiCoord, CMkxRoiCoord *UserRoiCoord,
  CTr1Hdl TrackingHdl,
  int IwProc, int IhProc,
  int SSFactor, int ExtractTime, int Locked, 
  CMkxSeqPara SeqPara,
  CPrmPara *extractPara
  )
{
  PrmParaToProcessPara(
    pp,
    SeqPara, SSFactor,
    extractPara
    );
  *MarkersDistMm = 0.5f*(SeqPara.MarkersDistMaxMm + SeqPara.MarkersDistMinMm);
  *CplSelectDistToler = MKX_RND(100 * (SeqPara.MarkersDistMaxMm - SeqPara.MarkersDistMinMm) / (SeqPara.MarkersDistMaxMm + SeqPara.MarkersDistMinMm));

  float Coef = 0.8f; //NbPix = Coef * Radius**2
  float radius = pp->BlobEnhRingRadiusInPix;

  *SubMarkersDist = (int)((*MarkersDistMm) / SeqPara.MmPerPixel);
  if (SSFactor > 1) *SubMarkersDist /= SSFactor;

  SetExtractRoi(
    MarginRoiCoord, Margin, UserInitRoiCoord, UserRoiCoord,
    SeqPara,
    IwProc, IhProc, pp->BlobEnhRingRadiusInPix,
    extractPara
    );

  //build the table for ring excursion
  pp->BlobEnhNbDir = extractPara->Val[MKX_BLOBENHRINGNBDIR_I].Int;
  double StepAngle = (double)(2.* MKX_PI / (double)pp->BlobEnhNbDir);
  for (int k = 0; k < pp->BlobEnhNbDir; k++)
  {
    double Angle = (double)k * StepAngle;
    pp->BlobEnhTabRing[k].X = MKX_RND(radius * cos(Angle));
    pp->BlobEnhTabRing[k].Y = MKX_RND(radius * sin(Angle));
  }

  *EstimatedMaxNbPixPerBlob = (int)(Coef * radius * radius);

  // Update non-critical tracker parameters
  MkxUpdateTrackingNonCriticalParam(TrackingHdl, pp, extractPara);

    // init wire couple maximum number of couples and pruning factor
  *WireCplMaxNbCouple = extractPara->Val[MKX_CPLSELECTMAXNBWIRES_I].Int;
  *WireCplPruningFactor = MKX_WIRECPLPRUNINGFACTOR_F;

  return 0;
}
void  MkxInitAdaptiveParams(int* twoRoisFlag, CMkxRoiCoord *roi,
  int* AdaptiveWorkNbLblBlobsKept, int* AdaptiveWorkNbPixelsKept, int* AdaptiveWorkMarkersDist, float* AdaptiveDistTolerMin, float* AdaptiveDistTolerMax,
  CMkxRoiCoord UserInitRoiCoord, CMkxRoiCoord UserRoiCoord, int EstimatedMaxNbPixPerBlob,
  int NotYetLockedFlag, int SubMarkersDist, int CplSelectDistToler, CPrmPara *extractPara)
{
  // init useful roi
  if (extractPara->Val[MKX_INITROIFLAG_I].Int == MKX_CST_EXTRACT_ROI_DEF && NotYetLockedFlag)
    roi[0] = UserInitRoiCoord;
  else
    roi[0] = UserRoiCoord;

  //init TwoRoisFlag
  *twoRoisFlag = 0;
  //init UsefulRoiCoord[1] and UsefulRoiCoord[2]
  roi[1] = roi[2] = roi[0];

  // init NbLabelsKept
  *AdaptiveWorkNbLblBlobsKept = extractPara->Val[MKX_CPLSELECTNBLABELSKEPT_I].Int;
  *AdaptiveWorkNbPixelsKept = *AdaptiveWorkNbLblBlobsKept * EstimatedMaxNbPixPerBlob;

  //init Markers dist and dist toler
  *AdaptiveWorkMarkersDist = SubMarkersDist;
  MkxDistToler(
    AdaptiveDistTolerMin, AdaptiveDistTolerMax,
    *AdaptiveWorkMarkersDist,
    CplSelectDistToler
    );
}


/*************************************************************************************/
// Set some extract process fields according to critical parameters
/*************************************************************************************/
int   MkxSetExtractProcFromCriticalParam(CMkxSwStatus *SwStatus,
  int* SSFactor, int *IwProc, int *IhProc,
  int* SSFactor_SW, int *IwProc_SW, int *IhProc_SW, CMkxRoiCoord *UserRoiCoord_SW,
  int IwOri, int IhOri, CPrmPara *extractPara, CMkxSeqPara seqPara)
{
  if (extractPara->Val[MKX_BLOBENHSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_NONE)		*SSFactor = 1;
  else if (extractPara->Val[MKX_BLOBENHSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_2)		*SSFactor = 2;
  else if (extractPara->Val[MKX_BLOBENHSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_4)		*SSFactor = 4;
  else if (extractPara->Val[MKX_BLOBENHSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_AUTO)// automatic 
  {
    // Getting intermediate image size
    if (IhOri > 512 || IwOri > 512)*SSFactor = 2;
    else                                                    *SSFactor = 1;
  }
  *IwProc = IwOri / (*SSFactor);
  *IhProc = IhOri / (*SSFactor);

  *SwStatus = extractPara->Val[MKX_CPLSELECTSWLibFLAG_I].Int > 0 ? MKX_CST_EXTRACT_RES_STERNALWIRE_INIT_PHASE : MKX_CST_EXTRACT_RES_STERNALWIRE_NOTAVAILABLE;//PL_160308

    if (extractPara->Val[MKX_SWSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_NONE)		*SSFactor_SW = 1;
    else if (extractPara->Val[MKX_SWSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_2)		*SSFactor_SW = 2;
    else if (extractPara->Val[MKX_SWSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_4)		*SSFactor_SW = 4;
    else if (extractPara->Val[MKX_SWSUBSAMPLINGMODE_I].Int == MKX_CST_SUBSAMPLING_AUTO)// automatic 
    {
      // Getting intermediate image size
      if (IhOri > 512 || IwOri > 512)*SSFactor_SW = 2;
      else                           *SSFactor_SW = 1;
    }
    *IwProc_SW = IwOri / (*SSFactor_SW);
    *IhProc_SW = IhOri / (*SSFactor_SW);

    //set coordinates of Default Roi for SWLib from user parameters
    int xMin, yMin, xMax, yMax;
    if (extractPara->Val[MKX_DEFROIFLAG_I].Int == MKX_CST_EXTRACT_ROI_NODEF)
    {
    //xMin = 0;
    //yMin = 0;
    //xMax = *IwProc_SW - 1;
    //yMax = *IhProc_SW - 1;
    xMin = 0;
    yMin = 0;
    xMax = *IwProc_SW - 1;
    yMax = *IhProc_SW - 1;
    }
    else
    {
		xMin = (int)(seqPara.DefRoiFacXMin * (*IwProc_SW-1));
		yMin = (int)(seqPara.DefRoiFacYMin * (*IhProc_SW-1));
		xMax = (int)(seqPara.DefRoiFacXMax * (*IwProc_SW-1));
		yMax = (int)(seqPara.DefRoiFacYMax * (*IhProc_SW-1));
    }
    // check Roi validity and correct
    if (xMin > xMax || yMin > yMax)
    {
    MkxRoiCoord(UserRoiCoord_SW, 0, 0, *IwProc_SW - 1, *IhProc_SW - 1); // use the maximal ROI
    }
    else
    {
      MkxRoiCoord(UserRoiCoord_SW, xMin, yMin, xMax, yMax);
  }
  return 0;
}



CMkxSeqPara DefaultSeqPara(int MaxImaDim)
{
	CMkxSeqPara p;

	p.MmPerPixel =  MKX_CST_MmPerPixel_Def(MaxImaDim);
	p.Fps =  MKX_CST_Fps_Def ;
	p.MarkersDistMinMm =  MKX_CST_MarkersDistMin_Def ;
	p.MarkersDistMaxMm =  MKX_CST_MarkersDistMax_Def ;
	p.DefRoiFacXMin =  MKX_CST_DefRoiXMin_Def ;
	p.DefRoiFacXMax =  MKX_CST_DefRoiXMax_Def ;
	p.DefRoiFacYMin =  MKX_CST_DefRoiYMin_Def ;
	p.DefRoiFacYMax =  MKX_CST_DefRoiYMax_Def ;
	p.InitRoiFacXMin =  MKX_CST_InitRoiXMin_Def ;
	p.InitRoiFacXMax =  MKX_CST_InitRoiXMax_Def ;
	p.InitRoiFacYMin =  MKX_CST_InitRoiYMin_Def ;
	p.InitRoiFacYMax =  MKX_CST_InitRoiYMax_Def ;

	return p;
}



/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
