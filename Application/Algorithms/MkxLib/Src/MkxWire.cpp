// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <MkxDev.h>
#include <BxWire.h>

/***************************************************************************/
//  MkxWireInit                                                     */
/***************************************************************************/
void MkxWireInit(void* WireHdl, BxWireParameters *WireParams, 
  int DynOri, int SSFactor, CPrmPara *ExtractParam)
{
	BxWireDefaultParameters(WireParams, DynOri) ; 

	if((ExtractParam->Val[MKX_WIREFLAG_I].Int == 2) && SSFactor > 1)
		WireParams->OutputResolution = (float)1.0/SSFactor;
	else
		WireParams->OutputResolution = (float)1;

	if(WireHdl)		BxWireDelete(WireHdl);
	WireHdl = 0;

	BxWireCreate(&WireHdl, WireParams) ;
}

/***************************************************************************/
// MkxGetWire                                                     */
/***************************************************************************/
int MkxGetWire(void* WireHdl, short *InIma, int IwOri, int IhOri, short *ImSub, CMkxExtractResults *exr, 
  int SSFactor, int IwProc, int IhProc, 
  CPrmPara *ExtractParam)
{
	BxCurve Curve;
	float X1=-1.f, Y1=-1.f, X2=-1.f, Y2=-1.f;

	if((ExtractParam->Val[MKX_WIREFLAG_I].Int == 2)&& SSFactor > 1)
	{
		X1 = (float)exr->X1Refined / SSFactor;
		Y1 = (float)exr->Y1Refined / SSFactor;
		X2 = (float)exr->X2Refined / SSFactor;
		Y2 = (float)exr->Y2Refined / SSFactor;
	}else
	{
		X1 = (float)exr->X1Refined ;
		Y1 = (float)exr->Y1Refined ;
		X2 = (float)exr->X2Refined ;
		Y2 = (float)exr->Y2Refined ;
	}
	if((ExtractParam->Val[MKX_WIREFLAG_I].Int == 2)&& SSFactor > 1)
	{
		if(BxWireDoExtract(WireHdl, &Curve, ImSub, IwProc, IhProc, 
			X1, Y1, X2, Y2 ))return 1;
		BxCurveUpsampleCoordinates(&Curve, SSFactor) ; 
	}
	else
	{
		if(BxWireDoExtract(WireHdl, &Curve, InIma, IwOri, IhOri, 
			X1, Y1, X2, Y2 ))return 1;
	}
	BxCurveCopy(&Curve, &(exr->Wire)) ;

	exr->WireStatus = MKX_CST_WIRE_AVAILABLE ;

	return 0;
}

