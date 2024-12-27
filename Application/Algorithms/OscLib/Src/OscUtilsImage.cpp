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
#include "BxGaussian.h"

//************************************************************************************
// Sub-sample the image of a factor "SSFactor" - by keeping the minimal value (to preserve GW)

int OscSubSampleImageMin(COscHdl OscHdl, short *ImIn, int IwIn, int IhIn, short *ImOut, int IwOut, int IhOut, int SSFactor)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

	if ( (IwOut != IwIn/SSFactor) || (IhOut != IhIn/SSFactor) ){
        char                 Reason[128];
        ExtractProc->FatalErrorOnPreviousImage = 1;
		sprintf_s(Reason, 128,"OscSubSampleImageMin: input and output image sizes (%dx%d and %dx%d) incoherent with subsample factor %d", IwIn, IhIn, IwOut, IhOut, SSFactor);
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscSubSampleImageMin", OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT, Reason);
		return OSC_ERR_INCOHERENT_SUBFUNCTION_ARGUMENT;
	}

	if(SSFactor == 1)	// No subsampling needed
		OSC_MEMCPY(ImOut, ImIn, IwOut*IhOut);
	else {
		for (int yy=0; yy<IhOut; yy++)
			for (int xx=0; xx<IwOut; xx++){	// Loop on the output image
				int xMin = xx*SSFactor, yMin = yy*SSFactor, dx, dy, pIn = yMin*IwIn+xMin;
				short minValIn = ImIn[pIn];

				// Min over the corresponding region of the input image
				pIn++;
				for (dx=1; dx<SSFactor; dx++, pIn++)
					if (minValIn > ImIn[pIn])
						minValIn = ImIn[pIn];

				for (dy=1; dy<SSFactor; dy++){
					pIn += IwIn-SSFactor;
					for (dx=0; dx<SSFactor; dx++, pIn++)
						if (minValIn > ImIn[pIn])
							minValIn = ImIn[pIn];

				}

				ImOut[yy*IwOut+xx] = minValIn;

				//ImOut[p] = ImIn[yMin*IwIn+xMin];
				//for (dx=1; dx<SSFactor; dx++)
				//	ImOut[p] = OSC_MIN(ImOut[p], ImIn[yMin*IwIn+xMin+dx]);
				//for (dy=1; dy<SSFactor; dy++)
				//	for (dx=0; dx<SSFactor; dx++)
				//		ImOut[p] = OSC_MIN(ImOut[p], ImIn[(yMin+dy)*IwIn+xMin+dx]);
			}
	}
	return 0;
}

//************************************************************************************
// Scales the image linearly between 0 and 1, after having thresholded it between percentileL and percentileH
// A tin Gaussian filtering is performed to avoid plateaus

int OscImDynamicsAdjust(COsc* This, float * Im, int Iw, int Ih, COscRoiCoord *ShutterPos
					, float percentileL, float percentileH, float * Buf, float * ImRescaled){

	int x, y, p, Err;
	float thresL, thresH, factor;
	float minIm, maxIm;
	int ErrorCode;

	// *** Adjust Im dynamic
	ErrorCode = OscPercentileComputation(This, Im, Iw, Ih, ShutterPos, percentileL, &thresL);
	if(ErrorCode)
	{
        int Fatal;
        ErrIsFatal(This->ErrHdl, ErrorCode, &Fatal); 
        if (Fatal){This->ExtractProc.FatalErrorOnPreviousImage = 1;}
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscImDynamicsAdjust", "OscPercentileComputation", 0, ErrorCode); 
		return ErrorCode;
	}

	ErrorCode = OscPercentileComputation(This, Im, Iw, Ih, ShutterPos, percentileH, &thresH);
	if(ErrorCode)
	{
        int Fatal;
        ErrIsFatal(This->ErrHdl, ErrorCode, &Fatal); 
        if (Fatal){This->ExtractProc.FatalErrorOnPreviousImage = 1;}
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscImDynamicsAdjust", "OscPercentileComputation", 0, ErrorCode); 
		return ErrorCode;
	}

	OSC_MEMCPY(ImRescaled, Im, Iw*Ih);
	for (p=0; p<Iw*Ih; p++)
		ImRescaled[p] = OSC_CLIP(thresL, ImRescaled[p], thresH);
	
	// *** Thin Gaussian filtering
	Err = BxIsoGaussFilterFloat2D(ImRescaled, Buf, 1.f, Iw, Ih);
    if (Err != 0){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscImDynamicsAdjust", OSC_ERR_MEM_ALLOC, "Allocation error in BxIsoGaussFilterFloat2D");
		return OSC_ERR_MEM_ALLOC;
	}

	// *** Scaling between 0 and 1
	minIm = Buf[0]; maxIm = Buf[0];
	for (p=0; p<Iw*Ih; p++){
		if (minIm > Buf[p])
			minIm = Buf[p];
		if (maxIm < Buf[p])
			maxIm = Buf[p];
	}

	memset(ImRescaled, 0, Iw*Ih*sizeof(float));
	factor = 1.f/(maxIm-minIm);
	for (y=ShutterPos->YMin; y<=ShutterPos->YMax; y++)
		for (x=ShutterPos->XMin; x<=ShutterPos->XMax; x++)
			ImRescaled[y*Iw+x] = factor * (Buf[y*Iw+x]-minIm);

	return 0;
}

//************************************************************************************
int OscPercentileComputation(COsc *This, float *Im, int Iw, int Ih, COscRoiCoord *RoiCoord, float percentileIn, float *thresholdOut)
{
	int   x, y, k, HistoLen = 1000, *Histo, Level;
	int   xMin, xMax, yMin, yMax;
	float minIm, maxIm, normFac;
	int nbPixKept;
	
	xMin = RoiCoord->XMin; xMax = RoiCoord->XMax; yMin = RoiCoord->YMin; yMax = RoiCoord->YMax;
	//nbPixKept = (int)OSC_RND(percentileIn * (xMax-xMin+1) * (yMax-yMin+1));
	nbPixKept = (int)(percentileIn * (xMax-xMin) * (yMax-yMin));	// To stay equivalent with proto

	// *** Image dynamics
	minIm = Im[yMin*Iw+xMin]; maxIm = Im[yMin*Iw+xMin];
	for(y=yMin; y<=yMax; y++)
		for(x=xMin; x<=xMax; x++){
			minIm = OSC_MIN(minIm, Im[y*Iw+x]);
			maxIm = OSC_MAX(maxIm, Im[y*Iw+x]);
		}

	if (minIm == maxIm){	
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscPercentileComputation", OSC_ERR_INTERNAL, "Homogeneous potential image");
		return OSC_ERR_INTERNAL;
	}

	// *** Fill histogram
	OSC_CALLOC(Histo, HistoLen+1);
	normFac = (float)HistoLen / (maxIm - minIm);

	for(y=yMin; y<=yMax; y++)
		for(x=xMin; x<=xMax; x++){
			Level = (int)(normFac * (Im[y*Iw+x]-minIm));
			Histo[Level]++;
		}

	// Cumulative histogram
	for(k = 1; k <= HistoLen; k++) Histo[k] += Histo[k-1];

	// *** Get threshold
	k = 0;
	while(Histo[HistoLen] - Histo[k] > nbPixKept) k++;
	k = OSC_CLIP(k, 0, HistoLen);

	*thresholdOut = k / normFac + minIm;

	// Ending
	free(Histo);

	return 0;
}

//************************************************************************************
int OscPercentileComputationMask(COsc *This, float *Im, unsigned char *Mask, int Iw, int Ih, COscRoiCoord *RoiCoord, float percentileIn, float *thresholdOut)
{
	int   x, y, k, HistoLen = 1000, *Histo, Level;
	int   xMin, xMax, yMin, yMax;
	float minIm, maxIm, normFac;
	int nbPixKept;
	
	xMin = RoiCoord->XMin; xMax = RoiCoord->XMax; yMin = RoiCoord->YMin; yMax = RoiCoord->YMax;
	//nbPixKept = (int)OSC_RND(percentileIn * (xMax-xMin+1) * (yMax-yMin+1));
	nbPixKept = (int)(percentileIn * (xMax-xMin) * (yMax-yMin));	// To stay equivalent with proto

	// *** Image dynamics
	minIm = Im[yMin*Iw+xMin]; maxIm = Im[yMin*Iw+xMin];
	for(y=yMin; y<=yMax; y++)
		for(x=xMin; x<=xMax; x++){
			minIm = OSC_MIN(minIm, Im[y*Iw+x]);
			maxIm = OSC_MAX(maxIm, Im[y*Iw+x]);
		}

	if (minIm == maxIm){	
        This->ExtractProc.FatalErrorOnPreviousImage = 1;
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscPercentileComputationMask", OSC_ERR_INTERNAL, "Homogeneous potential image");
		return OSC_ERR_INTERNAL;
	}

	// *** Fill histogram
	OSC_CALLOC(Histo, HistoLen+1);
	normFac = (float)HistoLen / (maxIm - minIm);

	for(y=yMin; y<=yMax; y++)
		for(x=xMin; x<=xMax; x++)
			if (Mask[y*Iw+x] == 1){
			    Level = (int)(normFac * (Im[y*Iw+x]-minIm));
			    Histo[Level]++;
		    }

	// Cumulative histogram
	for(k = 1; k <= HistoLen; k++) Histo[k] += Histo[k-1];

	// *** Get threshold
	k = 0;
	while(Histo[HistoLen] - Histo[k] > nbPixKept) k++;
	k = OSC_CLIP(k, 0, HistoLen);

	*thresholdOut = k / normFac + minIm;

	// Ending
	free(Histo);

	return 0;
}

//************************************************************************************
int OscPercentileComputationShort(COsc *This, short *Im, unsigned char *Mask, int Iw, int Ih, COscRoiCoord *RoiCoord, float percentileIn, float *thresholdOut)
{
	int   x, y, k, HistoLen = 1000, *Histo, Level;
	int   xMin, xMax, yMin, yMax;
	float minIm, maxIm, normFac;
	int nbPixKept;
	
	xMin = RoiCoord->XMin; xMax = RoiCoord->XMax; yMin = RoiCoord->YMin; yMax = RoiCoord->YMax;

	// *** Image dynamics
	minIm = (float)Im[yMin*Iw+xMin]; maxIm = (float)Im[yMin*Iw+xMin];
	for(y=yMin; y<=yMax; y++)
		for(x=xMin; x<=xMax; x++){
			minIm = OSC_MIN(minIm, (float)Im[y*Iw+x]);
			maxIm = OSC_MAX(maxIm, (float)Im[y*Iw+x]);
		}

	if (minIm == maxIm){
        This->ExtractProc.FatalErrorOnPreviousImage = 1;				
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscPercentileComputationShort", OSC_ERR_INTERNAL, "Homogeneous potential image");
		return OSC_ERR_INTERNAL;
	}

	// *** Fill histogram
	OSC_CALLOC(Histo, HistoLen+1);
	normFac = (float)HistoLen / (maxIm - minIm);

	nbPixKept = 0;
	for(y=yMin; y<=yMax; y++)
		for(x=xMin; x<=xMax; x++)
			if (Mask[y*Iw+x] == 1){
				Level = (int)(normFac * (Im[y*Iw+x]-minIm));
				Histo[Level]++;
				nbPixKept++;
			}
	nbPixKept = (int)(percentileIn *nbPixKept);


	// Cumulative histogram
	for(k = 1; k <= HistoLen; k++) Histo[k] += Histo[k-1];

	// *** Get threshold
	k = 0;
	while(Histo[HistoLen] - Histo[k] > nbPixKept) k++;
	k = OSC_CLIP(k, 0, HistoLen);

	*thresholdOut = k / normFac + minIm;

	// Ending
	free(Histo);

	return 0;
}


/*************************************************************************************/
void OscDilationRect(unsigned char * ImIn, int Iw, int Ih, COscRoiCoord *RoiCoord, int halfSizeX, int halfSizeY
					, unsigned char *Buf, unsigned char *ImOut){

	int x, y, k;

	// 1- On columns
	OSC_MEMCPY(Buf, ImIn, Iw*Ih);
	for (y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){

			for (k = OSC_MAX(-halfSizeX, RoiCoord->XMin-x); k<= OSC_MIN(halfSizeX, RoiCoord->XMax-x); k++)
				Buf[y*Iw+x] = OSC_MAX(Buf[y*Iw+x], ImIn[y*Iw+x+k]);
		}

	// 2- On rows
	OSC_MEMCPY(ImOut, Buf, Iw*Ih);
	for (y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
		for (x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){

			for (k = OSC_MAX(-halfSizeY, RoiCoord->YMin-y); k<= OSC_MIN(halfSizeY, RoiCoord->YMax-y); k++)
				ImOut[y*Iw+x] = OSC_MAX(ImOut[y*Iw+x], Buf[(y+k)*Iw+x]);
		}
}

/*************************************************************************************/
int OscMedian(COsc* This, float * stack1, float * stack2, int x0, int pos, int xEnd, float *resMed)
{
	int xMinus, xPlus, i;
	float pivot;

    if (x0<0 || pos < 0 || xEnd < 0 || xEnd < x0 || xEnd < pos || pos < x0){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "OscMedian", OSC_ERR_NEGATIVE_VECTOR_LENGTH, "Incoherent indices in OscMedian");
        return OSC_ERR_NEGATIVE_VECTOR_LENGTH;
    }

	xMinus = x0;
	xPlus = xEnd;
	pivot = stack1[x0];

	for (i=x0+1; i<=xEnd;i++)
		if (stack1[i]<pivot){
			stack2[xMinus] = stack1[i];
			xMinus ++;
		} else if (stack1[i]>pivot) {
			stack2[xPlus] = stack1[i];
			xPlus --;
		}
	
	xMinus--; xPlus++;

	if ( (xMinus < pos) && (xPlus > pos) ){
		*resMed = pivot;
        return 0;
    }else if (xMinus >= pos)
		return OscMedian(This, stack2,stack1,x0,pos,xMinus, resMed);
	else 
		return OscMedian(This, stack2,stack1,xPlus,pos,xEnd, resMed);
}
