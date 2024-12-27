// ***************************************************************************
// * Copyright (c) 2002-2015 Philips Medical Research Paris, Philips France. *
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
// Polarize markers w.r.t previous ones
/*************************************************************************************/
static int MkxGetPolarity(
  float prevX1,  float prevY1,  float prevX2,  float prevY2, 
 int curX1, int curY1,  int curX2,  int curY2
 )
{
  float ps = (prevX2 - prevX1) * (curX2 - curX1) + (prevY2 - prevY1) * (curY2 - curY1);
  int polarity = ps >= 0 ? MKX_CST_DIRECT : MKX_CST_REVERSE;
  return polarity;
}


/*************************************************************************************/
// swap markers
/*************************************************************************************/
static void MkxSwapMarkers(int* x1, int* y1, int* x2, int* y2)
{
	int XA, YA, XB, YB;

  XA = *x1;
  YA = *y1;
  XB = *x2;
  YB = *y2;

  *x1 = XB;
  *y1 = YB;
  *x2 = XA;
  *y2 = YA;
}


/*************************************************************************************/
// Polarize markers w.r.t previous ones
/*************************************************************************************/

void MkxPolarize(CMkxCoupleInt* polarized, CMkxCoupleInt cur, CMkxCoupleFloat prev)
{
  //polarized is the previous markercouple
  int  polarity = MkxGetPolarity(
    prev.V1f.Xf, prev.V1f.Yf, prev.V2f.Xf, prev.V2f.Yf,
    cur.V1.X, cur.V1.Y, cur.V2.X, cur.V2.Y
    );
  //init the new polarized 
  *polarized=cur;
  //then possibly polarize it
  if (polarity == MKX_CST_REVERSE)
    MkxSwapMarkers(&polarized->V1.X, &polarized->V1.Y, &polarized->V2.X, &polarized->V2.Y);
}


/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
