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

#include <MkxDev.h>


/*************************************************************************************/
// FillResults 
/*************************************************************************************/

void MkxFillResults(
  CMkxExtractResults *exr,
  CMkxCoupleFloat cpl,
  CMkxMarkerCoupleRefined MarkerCoupleRefined,
  int Status, int Locked,
  int twoRoisFlag, CMkxRoiCoord* roi, int SSFactor,
  int IwProc, int IhProc,
  CMkxSwStatus SwStatus,   int IwProc_SW, int IhProc_SW
  )
{
  exr->X1Refined=cpl.V1f.Xf;
  exr->Y1Refined=cpl.V1f.Yf;
  exr->X2Refined=cpl.V2f.Xf;
  exr->Y2Refined=cpl.V2f.Yf;
  exr->Status = (CMkxExtractionStatus) Status;
  exr->Locked = (CMkxLock) Locked;
 //rois
  exr->TwoRoisFlag = twoRoisFlag;
  exr->RoiXMin = roi[0].XMin*SSFactor; exr->RoiYMin = roi[0].YMin*SSFactor;
  exr->RoiXMax = roi[0].XMax*SSFactor; exr->RoiYMax = roi[0].YMax*SSFactor;
  exr->RoiXMin1 = roi[1].XMin*SSFactor; exr->RoiYMin1 = roi[1].YMin*SSFactor;
  exr->RoiXMax1 = roi[1].XMax*SSFactor; exr->RoiYMax1 = roi[1].YMax*SSFactor;
  exr->RoiXMin2 = roi[2].XMin*SSFactor; exr->RoiYMin2 = roi[2].YMin*SSFactor;
  exr->RoiXMax2 = roi[2].XMax*SSFactor; exr->RoiYMax2 = roi[2].YMax*SSFactor;
  //process image size
  exr->IwProc=IwProc; exr->IhProc= IhProc;
  exr->SwStatus = SwStatus;
  exr->IwProc_SW=IwProc_SW; exr->IhProc_SW= IhProc_SW;
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
