//Copyright (c)2014 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#include <DiaLibDev.h>

// ***************************************************************************
// *** Tracking from the previous time instant

int DiaParabolicContrastTracking(CDiaHdl DiaHdl, short * Im, int Iw, int Ih, CDiaRoiCoord RoiCoord	// Input image, dimension and ROI
								  , int delta, float stepMaxPix, int nbIterations		// Delta to compute contrast, stepMaxPix=first update amplitude in pixels, nb gradient descent iterations
								  , CDiaDiaphragmParab coeffIn, CDiaDiaphragmParab *coeffOut){

	CDia			*This = (CDia*)DiaHdl.Pv;
	CDiaProc		*DiaProc = &(This->DiaProc);
	int it;

	for (it = 0; it<nbIterations; it++){
		// Iterations with decreasing shift steps
		DiaParabolicContrastTrackingStep(Im, DiaProc->BufS1S, Iw, Ih, RoiCoord, delta
										, (0.1f*stepMaxPix + 0.9f * (nbIterations-1-it) *stepMaxPix / (nbIterations-1))		// Shift step
										, coeffIn, coeffOut);
		coeffIn.Param[0] = coeffOut->Param[0];
		coeffIn.Param[1] = coeffOut->Param[1];
		coeffIn.Param[2] = coeffOut->Param[2];
	}

	return 0;
}


void DiaParabolicContrastTrackingStep(short * Im, short * Mask, int Iw, int Ih, CDiaRoiCoord RoiCoord, int delta, float lambda
									   , CDiaDiaphragmParab coeffIn, CDiaDiaphragmParab *coeffOut){

	int x,y, xMed = Iw/2;
	double gradA, gradB, gradC, coeffHomA, coeffHomB, shiftTypical;

	// Mask computation
	memset(Mask, 0, Iw*Ih*sizeof(short));
	for (x=RoiCoord.XMin; x<RoiCoord.XMax; x++){
		y = (int)(coeffIn.Param[0]*(x-Iw/2)*(x-Iw/2) + coeffIn.Param[1]*(x-Iw/2) + coeffIn.Param[2]+0.5f);
		Mask[DIA_CLIP(RoiCoord.YMin,y,RoiCoord.YMax-1)*Iw+x] += 1; Mask[DIA_CLIP(RoiCoord.YMin,y-1,RoiCoord.YMax-1)*Iw+x] += 1;
		Mask[DIA_CLIP(RoiCoord.YMin,y+delta,RoiCoord.YMax-1)*Iw+x] += -1; Mask[DIA_CLIP(RoiCoord.YMin,y-delta,RoiCoord.YMax-1)*Iw+x] += -1;
	}

	// Gradient computation (over the parabola parameters, to determine their update)
	coeffHomA = 1/(double)(xMed*xMed); coeffHomB = 1/(double)(xMed);
	gradA = 0; gradB = 0; gradC = 0;
	for (y=RoiCoord.YMin; y<RoiCoord.YMax; y++)
		for (x=RoiCoord.XMin; x<RoiCoord.XMax; x++)
			if (Mask[y*Iw+x] != 0)
			{
				gradA += coeffHomA*(x-Iw/2)*(x-Iw/2)*Mask[y*Iw+x]*Im[y*Iw+x];
				gradB += coeffHomB*(x-Iw/2)*Mask[y*Iw+x]*Im[y*Iw+x];
				gradC += Mask[y*Iw+x]*Im[y*Iw+x];
			}

	// For normalization (of lambda in update), step size at both side of the image, and in the middle
	shiftTypical = DIA_MAX( DIA_MAX( DIA_ABS(coeffHomA*gradA*(RoiCoord.XMin-Iw/2)*(RoiCoord.XMin-Iw/2) + coeffHomB*gradB*(RoiCoord.XMin-Iw/2) + gradC)
									, DIA_ABS(coeffHomA*gradA*(xMed-Iw/2)*(xMed-Iw/2) + coeffHomB*gradB*(xMed-Iw/2) + gradC) )
						, DIA_ABS(coeffHomA*gradA*(RoiCoord.XMax-Iw/2)*(RoiCoord.XMax-Iw/2) + coeffHomB*gradB*(RoiCoord.XMax-Iw/2) + gradC) );

	if (shiftTypical == 0){
		coeffOut->Param[0] = coeffIn.Param[0];
		coeffOut->Param[1] = coeffIn.Param[1];
		coeffOut->Param[2] = coeffIn.Param[2];
	} else {
		// normalized update
		coeffOut->Param[0] = coeffIn.Param[0] + (float)(lambda * coeffHomA* gradA / shiftTypical);
		coeffOut->Param[1] = coeffIn.Param[1] + (float)(lambda * coeffHomB* gradB / shiftTypical);
		coeffOut->Param[2] = coeffIn.Param[2] + (float)(lambda * gradC / shiftTypical);
	}
}
