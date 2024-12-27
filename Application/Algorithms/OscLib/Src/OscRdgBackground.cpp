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

#include <OscDev.h>

void OscRdgCircularBackgroundSubtraction(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	float				** ImCircBuffer = ExtractProc->ImCircBuffer;
	COscRoiCoord		*RoiCoord = &ExtractProc->ShuttersRoi;

	int					t = ExtractProc->t - ExtractProc->NonInitialSkippedImages; 
	int					CircBufferSize = ExtractProc->CircBufferSize, Iw = ExtractProc->IwProcFluoro, Ih = ExtractProc->IhProcFluoro;
	int					xMin1, yMin1, xMin2, xMax2, yMin2, yMax2, width, height, x, y, k;//, p1, p2;
	float				*ImBkg = ExtractProc->ImBkg;


	if (ExtractProc->t == 0){
		ExtractProc->initialDeltaTabCircBackX = ExtractProc->deltaTabXSub;
		ExtractProc->initialDeltaTabCircBackY = ExtractProc->deltaTabYSub;
	}

	// Store warped background
	yMin1 = OSC_MAX(RoiCoord->YMin, RoiCoord->YMin+ExtractProc->deltaTabYSub-ExtractProc->initialDeltaTabCircBackY);
	//yMax1 = OSC_MIN(RoiCoord->YMax, RoiCoord->YMax+ExtractProc->deltaTabYSub-ExtractProc->initialDeltaTabCircBackY);
	xMin1 = OSC_MAX(RoiCoord->XMin, RoiCoord->XMin+ExtractProc->deltaTabXSub-ExtractProc->initialDeltaTabCircBackX);
	//xMax1 = OSC_MIN(RoiCoord->XMax, RoiCoord->XMax+ExtractProc->deltaTabXSub-ExtractProc->initialDeltaTabCircBackX);

	yMin2 = OSC_MAX(RoiCoord->YMin-ExtractProc->deltaTabYSub+ExtractProc->initialDeltaTabCircBackY, RoiCoord->YMin);
	yMax2 = OSC_MIN(RoiCoord->YMax-ExtractProc->deltaTabYSub+ExtractProc->initialDeltaTabCircBackY, RoiCoord->YMax);
	xMin2 = OSC_MAX(RoiCoord->XMin-ExtractProc->deltaTabXSub+ExtractProc->initialDeltaTabCircBackX, RoiCoord->XMin);
	xMax2 = OSC_MIN(RoiCoord->XMax-ExtractProc->deltaTabXSub+ExtractProc->initialDeltaTabCircBackX, RoiCoord->XMax);
	
	height = yMax2-yMin2; width = xMax2-xMin2;

	OSC_MEMCPY(ImCircBuffer[t%CircBufferSize], ExtractProc->ImRdg, Iw*Ih);

	for (y=0; y<height; y++)
		for (x=0; x<width; x++)
			ImCircBuffer[t%CircBufferSize][(yMin1+y)*Iw + xMin1+x] = ExtractProc->ImRdg[(yMin2+y)*Iw + xMin2+x];


	// Compute temporal minimum
	OSC_MEMCPY(ImBkg, ExtractProc->ImRdg, Iw*Ih);
	for (k=0; k<OSC_MIN(t, CircBufferSize); k++)
	{
		float * v1 = &ImCircBuffer[k][yMin1*Iw+xMin1];
		float * v2 = &ImBkg[yMin2*Iw+xMin2];

		if (k==t%CircBufferSize)
			continue;

		for (y=0; y<height; y++, v1 += Iw-width, v2 += Iw-width)
			for (x=0; x<width; x++, v1++,v2++){

				if (*v2>*v1)
				{
					*v2 = *v1;
				}
			}
	}

	// Subtraction
	if (ExtractProc->t == 0)
		OSC_MEMCPY(ExtractProc->ImFrg, ExtractProc->ImRdg, Iw*Ih);
	else {
		OSC_MEMZERO(ExtractProc->ImFrg, Iw*Ih);
		for (y=yMin2; y<yMax2; y++)
			for (x=xMin2; x<xMax2; x++)
				ExtractProc->ImFrg[y*Iw+x] = ExtractProc->ImRdg[y*Iw+x] - ExtractProc->ImBkg[y*Iw+x];
	}
}


void OscRdgInitialWarpedBackgroundSubtraction(COscHdl OscHdl)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	int					Iw = ExtractProc->IwProcAngio,Ih = ExtractProc->IhProcAngio;


	// * Background construction
	if (ExtractProc->t < OSC_BKGANGIO_DURATION_BEFORE_MOTION * ExtractProc->frameRateAngio)
		OscTemporalMinimum(ExtractProc->ImRdg, ExtractProc->ImBkg, Iw, Ih, ExtractProc->t
							, &ExtractProc->ShuttersRoiSafe, ExtractProc->deltaTabXSub, ExtractProc->deltaTabYSub);

	// * Background subtraction
	if (ExtractProc->t == 0)
		OSC_MEMCPY(ExtractProc->ImFrg, ExtractProc->ImRdg, Iw*Ih);
	else {
		if (ExtractProc->t <= OSC_FREEZE_THRESHOLDS_AT){
			int xMin, xMax, yMin, yMax;
			xMin = OSC_MAX(ExtractProc->ShuttersRoiSafe.XMin-ExtractProc->deltaTabXSub, ExtractProc->ShuttersRoiSafe.XMin);
			xMax = OSC_MIN(ExtractProc->ShuttersRoiSafe.XMax-ExtractProc->deltaTabXSub, ExtractProc->ShuttersRoiSafe.XMax);
			yMin = OSC_MAX(ExtractProc->ShuttersRoiSafe.YMin-ExtractProc->deltaTabYSub, ExtractProc->ShuttersRoiSafe.YMin);
			yMax = OSC_MIN(ExtractProc->ShuttersRoiSafe.YMax-ExtractProc->deltaTabYSub, ExtractProc->ShuttersRoiSafe.YMax);

			// Compensation of the table motion only
			for (int y=yMin; y<=yMax; y++)
				for (int x=xMin; x<=xMax; x++)
					ExtractProc->BufsF[3][y*Iw+x]
						= ExtractProc->ImBkg[(y+ExtractProc->deltaTabYSub)*Iw+x+ExtractProc->deltaTabXSub];
		} else 
			// Table motion and local block matching warping (to compensate for some breathing for instance)
			OscBlockMatchingCompensation(ExtractProc->ImRdg, ExtractProc->ImBkg, Iw, Ih, &ExtractProc->ShuttersRoiSafe
										, OSC_BKGANGIO_NB_BLOCKS/ ExtractProc->SSFactor, OSC_BKGANGIO_NB_BLOCKS/ ExtractProc->SSFactor, OSC_BKGANGIO_BM_AMPLITUDE/ ExtractProc->SSFactor
										, ExtractProc->deltaTabXSub, ExtractProc->deltaTabYSub, ExtractProc->BufsF[3]);

		// Subtraction
		memset(ExtractProc->ImFrg, 0, Iw*Ih*sizeof(float));
		for (int y=ExtractProc->ShuttersRoiSafe.YMin; y<=ExtractProc->ShuttersRoiSafe.YMax; y++)
			for (int x=ExtractProc->ShuttersRoiSafe.XMin; x<=ExtractProc->ShuttersRoiSafe.XMax; x++)
				ExtractProc->ImFrg[y*Iw+x] = OSC_MAX(0,ExtractProc->ImRdg[y*Iw+x] - ExtractProc->BufsF[3][y*Iw+x]);
	}

}

void OscTemporalMinimum(float *Im, float *ImMin, int Iw, int Ih, int t, COscRoiCoord *RoiCoord, int TabX, int TabY){

	if (t==0)
		OSC_MEMCPY(ImMin, Im, Iw*Ih);
	else {
		int xMin, xMax, yMin, yMax, x, y;

		xMin = OSC_MAX(RoiCoord->XMin, RoiCoord->XMin+TabX);
		xMax = OSC_MIN(RoiCoord->XMax, RoiCoord->XMax+TabX);
		yMin = OSC_MAX(RoiCoord->YMin, RoiCoord->YMin+TabY);
		yMax = OSC_MIN(RoiCoord->YMax, RoiCoord->YMax+TabY);

		for (y = yMin; y<=yMax; y++)
			for (x = xMin; x<=xMax; x++)
				ImMin[y*Iw+x] = OSC_MIN(ImMin[y*Iw+x], Im[(y-TabY)*Iw+x-TabX]);
	}

}