// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdlib.h> 
#include <math.h> 

#include <Tr1.h> 

// new version with faster correlation
// does not change detection quality, but faster

//====================================================================
// Calculates the difference between 2 positive angles (folding up)
float Tr1AngleErr(float Theta1, float Theta2)
{
	float AngleDiff;
	AngleDiff = TR1_ABS(Theta1 - Theta2);
	if( AngleDiff > 90 ) AngleDiff = 180 - AngleDiff;
	AngleDiff *= (float)(100.0/90.0);  
	return AngleDiff;
}

//====================================================================
// Calculate elementary error on breadth
float Tr1BreadthErr(float Breadth1, float Breadth2)
{
	float Error, Distance;
	Distance = TR1_ABS(Breadth1 - Breadth2);
	Error    = (Distance/TR1_MAX(Breadth1,Breadth2))*100;
	return Error;
}

//====================================================================
// Calculate elementary error on centroid
float Tr1CentroidErr(float X1, float Y1, float X2, float Y2, int Iw, int Ih)
{
	float Error, Distance;
	Distance = TR1_EUCLI_DIST(X1, Y1, X2, Y2);
	Error    = (Distance/(float)sqrt((double)(Iw*Iw + Ih*Ih))) * 100; 
	return Error;
}

//====================================================================
// Calculate elementary error on centroid
float Tr1CentroidErrNew(float X1, float Y1, float X2, float Y2)
{
	float Error;
	Error = TR1_EUCLI_DIST(X1, Y1, X2, Y2);
	return Error;
}

//====================================================================
// Non-linearity function for elementary error weighting
float Tr1NonLinearity(float X, float Low, float High, float SlopeFac, float FHigh)
{
	float Fx;

	if(X<Low)
	{
		Fx = (float)0;
	}
	else if(X>High) 
	{
		Fx = ((X-High)*(SlopeFac*FHigh))/(High-Low) + FHigh;
	}
	else
	{
		Fx = ((X-Low)*FHigh)/(High-Low);
	}

	return Fx;
}

//====================================================================
// Prediction error calculation between TrackPlane[t-1] and AlarmPlane[t] (stored in TrackPlane[t-1])
void Tr1CalcPredErr(CTr1Tracking* This, CTr1AlarmPlane *AlPlane, CTr1TrackPlane *TrPlaneTm1)
{
	CTr1Alarm   *Alarm;
	CTr1Track   *TrTm1;
	CTr1Pred    *Pred;
	CTr1PredErr *AvrPredErr, *CurPredErr;
	int        kAl, kTr;
	float      AngleErr, BreadthErr, CentroidErr,  Composite;
	float      AngleErrFuz, BreadthErrFuz, CentroidErrFuz, CompositeFuz;
	float      RawCompositeNorm;

	float TrAlpha       = (float)This->TrAlpha;
	float FHigh         = (float)20;
	float SlopeFrac     = (float)5;
	float Compo2Max     = (float)100;
	float HighAngle     = This->Param.HighAngleDist;
	float LowAngle      = This->Param.LowAngleDist;
	float HighBreadth   = This->Param.HighBreadthDist;
	float LowBreadth    = This->Param.LowBreadthDist;
	float HighCentroid  = This->Param.HighCentroidDist;
	float LowCentroid   = This->Param.LowCentroidDist;
	float CoefAngle     = This->Param.CoefAngle;
	float CoefBreadth   = This->Param.CoefBreadth;
	float CoefCentroid  = This->Param.CoefCentroid;

	// arrays to cache the moments of alarms and predictions
	float *alarm_moments_cache, *pred_moments_cache;

	// size of cache for one predictor, *2 because 2 moments (sum+sq)
    int    pred_moments_size=-1;
	alarm_moments_cache = (float*)calloc(2*2*AlPlane->NbAlarm, sizeof(alarm_moments_cache[0]));         // 2*2* because two markers per alarm times 2 moments (sum+sq)
	pred_moments_cache  = (float*)calloc(2*pred_moments_size*TrPlaneTm1->NbTrack, sizeof(pred_moments_cache[0])); // 2* because two markers per alarm 

		RawCompositeNorm  = (float)(1.0/(100.0 * (CoefAngle + CoefBreadth + CoefCentroid)));

	// For each alarm at time t
	for(kAl=0; kAl<AlPlane->NbAlarm; kAl++)
	{
		Alarm = &(AlPlane->Alarms[kAl]);

		// For each track at time t-1
		for(kTr=0; kTr<TrPlaneTm1->NbTrack; kTr++)
		{
			TrTm1        = &(TrPlaneTm1->Tracks[kTr]);
			Pred         = &(TrTm1->Pred);
			CurPredErr   = &(TrTm1->CurPredErrors[kAl]);
			AvrPredErr   = &(TrTm1->AvrPredErrors[kAl]);

			AngleErr     = Tr1AngleErr(Pred->Angle, Alarm->Angle);
			BreadthErr   = Tr1BreadthErr(Pred->Breadth, Alarm->Breadth);
			CentroidErr  = Tr1CentroidErrNew(Pred->Cx, Pred->Cy, Alarm->Cx, Alarm->Cy);
				Composite    = (CoefAngle * AngleErr + CoefBreadth * BreadthErr + CoefCentroid * CentroidErr)*RawCompositeNorm;

			AngleErrFuz    = Tr1NonLinearity(AngleErr,    LowAngle,    HighAngle,    SlopeFrac, FHigh);
			BreadthErrFuz  = Tr1NonLinearity(BreadthErr,  LowBreadth,  HighBreadth,  SlopeFrac, FHigh);
			CentroidErrFuz = Tr1NonLinearity(CentroidErr, LowCentroid, HighCentroid, SlopeFrac, FHigh);
				CompositeFuz   = CoefAngle * AngleErrFuz + CoefBreadth * BreadthErrFuz + CoefCentroid * CentroidErrFuz;
			CompositeFuz   = TR1_MIN(CompositeFuz, Compo2Max);
			CompositeFuz   = CompositeFuz/Compo2Max;

			CurPredErr->Raw.Angle     = AngleErr;
			CurPredErr->Raw.Breadth   = BreadthErr;
			CurPredErr->Raw.Centroid  = CentroidErr;
			CurPredErr->Raw.Composite = Composite;

			CurPredErr->Fuz.Angle     = AngleErrFuz;
			CurPredErr->Fuz.Breadth   = BreadthErrFuz;
			CurPredErr->Fuz.Centroid  = CentroidErrFuz;
			CurPredErr->Fuz.Composite = CompositeFuz;

			AvrPredErr->Raw.Angle     = TrTm1->AvrPredErr.Raw.Angle     * TrAlpha + AngleErr     * (1-TrAlpha);
			AvrPredErr->Raw.Breadth   = TrTm1->AvrPredErr.Raw.Breadth   * TrAlpha + BreadthErr   * (1-TrAlpha);
			AvrPredErr->Raw.Centroid  = TrTm1->AvrPredErr.Raw.Centroid  * TrAlpha + CentroidErr  * (1-TrAlpha);
			AvrPredErr->Raw.Composite = TrTm1->AvrPredErr.Raw.Composite * TrAlpha + Composite    * (1-TrAlpha);

			AvrPredErr->Fuz.Angle     = TrTm1->AvrPredErr.Fuz.Angle     * TrAlpha + AngleErrFuz    * (1-TrAlpha);
			AvrPredErr->Fuz.Breadth   = TrTm1->AvrPredErr.Fuz.Breadth   * TrAlpha + BreadthErrFuz  * (1-TrAlpha);
			AvrPredErr->Fuz.Centroid  = TrTm1->AvrPredErr.Fuz.Centroid  * TrAlpha + CentroidErrFuz * (1-TrAlpha);
			AvrPredErr->Fuz.Composite = TrTm1->AvrPredErr.Fuz.Composite * TrAlpha + CompositeFuz   * (1-TrAlpha);

		}
	}
	free(alarm_moments_cache);
	free(pred_moments_cache);

}

/***************************************************************************/
//         END OF FILE                                                     */
/***************************************************************************/
