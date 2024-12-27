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

#include <math.h>
#include <stdio.h>
#include <OscDev.h>
#include <BxGaussian.h>
#include <mutex> 

//#define OSC_TIMING
//#define OSC_TIMING_DETAIL

#ifdef OSC_MXLAB_SHOW_SUBRES
#include "MxLab.h"
#endif

/*************************************************************************************/
// Angio Sequence Extraction Process
/*************************************************************************************/
int OscAngioProcessSequence(COscHdl OscHdl, COscAngioSequenceResults* ProcessResults)
{
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	int					Err, ErrorCodeOut = 0, k, p;
    float               theta;
	
#ifdef OSC_TIMING
	double t0 = omp_get_wtime(), t1;
#endif
#ifdef OSC_TIMING_DETAIL
	double tAfterRegistration, tAfterCycleSelec, tAfterCleaning, tAfterOverlaysAndPyr, tAfterMotionCycle, tAfterInjQualityEst, tAfterCathInjROIDetection;
#endif

    ProcessResults->meanInjectedSurfaceMM2 = 0.f; ProcessResults->normalizedInjectionCoefficient = 0.f;
    
    if (ExtractProc->FatalErrorOnPreviousImage){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscAngioProcessSequence", OSC_ERR_PREVIOUS_FATAL_ERROR, NULL);
		return OSC_ERR_PREVIOUS_FATAL_ERROR;
    }


	// * Register each angio with each other
	Err = OscRegisterAngioImagesOnEachOther(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscRegisterAngioImagesOnEachOther", "OscAngioProcessSequence");
	
#ifdef OSC_TIMING_DETAIL
	tAfterRegistration = omp_get_wtime();
#endif
    

	// * Cycle length estimation
	Err = OscHeartCycleSelection(OscHdl, &ExtractSequence->CardiacCycleStart, &ExtractSequence->CardiacCycleEnd);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscHeartCycleSelection", "OscAngioProcessSequence");
    	
#ifdef OSC_TIMING_DETAIL
	tAfterCycleSelec = omp_get_wtime();
#endif

    // * Consolidated angio cleaning
    Err = OscRdgCleanVasculatureSequenceConsolidation(OscHdl, ExtractSequence->CardiacCycleStart, ExtractSequence->CardiacCycleEnd);
    OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscRdgCleanVasculatureSequenceConsolidation", "OscAngioProcessSequence");
    	
#ifdef OSC_TIMING_DETAIL
	tAfterCleaning = omp_get_wtime();
#endif

	// * Overlay computation and pyramid computation for selected cycle
	for (k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
		COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

		// Overlay
		ExtractPicture->ImOverlayMask = ExtractProc->BuffOverlayMask[k - ExtractSequence->CardiacCycleStart];

		Err = OscComputeOverlay(OscHdl, k);
		OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscComputeOverlay", "OscAngioProcessSequence");

		// Scalar product pyramid construction
		ExtractPicture->ImPyrDir1D = ExtractProc->BuffPyrDir1D[k - ExtractSequence->CardiacCycleStart];

		for(p=0; p<ExtractProc->IwProcAngio * ExtractProc->IhProcAngio; p++)
			if (ExtractPicture->ImLabS[p] > 1) {	
                theta = (float)(OSC_PI * (ExtractPicture->ImDirUC[p]-128.f) / 254.f);

                ExtractPicture->ImPyrDir1D[0][p][0] = (short)OSC_CLIP_SHORT(32768 * cosf(theta));
                ExtractPicture->ImPyrDir1D[0][p][1] = (short)OSC_CLIP_SHORT(32768 * sinf(theta));
			} else {
				ExtractPicture->ImPyrDir1D[0][p][0] = 0; ExtractPicture->ImPyrDir1D[0][p][1] = 0;
			}

		OscMrGaussianDecompPyrShort2(ExtractPicture->ImPyrDir1D, ExtractSequence->PyrNbBands, ExtractSequence->PyrW, ExtractSequence->PyrH);
	}
#ifdef OSC_TIMING_DETAIL
	tAfterOverlaysAndPyr = omp_get_wtime();
#endif

	// * Full resolution cycle motion estimation
	Err =  OscFullResolutionCycleMotionEstimation(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscFullResolutionCycleMotionEstimation", "OscAngioProcessSequence");
	
#ifdef OSC_TIMING_DETAIL
	tAfterMotionCycle = omp_get_wtime();
#endif


	// * Injection quality analysis
	Err = OscInjectionQualityEstimation(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscInjectionQualityEstimation", "OscAngioProcessSequence");
#ifdef OSC_TIMING_DETAIL
	tAfterInjQualityEst = omp_get_wtime();
#endif

    // * Cath inj ROI detection
    //OscCatheterInjectionCatheterROIDetection(OscHdl);
    ExtractSequence->nbSelectedCathInjROI = 0;  // Not used, so not computed
#ifdef OSC_TIMING_DETAIL
	tAfterCathInjROIDetection = omp_get_wtime();
#endif

    //{
    //    MxOpen(); MxSetVisible(1);
    //    for (k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
    //        unsigned char*ImTmp = (unsigned char*)malloc(ExtractProc->IwProcAngio * ExtractProc->IhProcAngio*sizeof(unsigned char));

    //        COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);
    //        for (int p=0; p<ExtractProc->IwProcAngio * ExtractProc->IhProcAngio; p++)
    //            if (ExtractPicture->ImLabS[p] > 1)
    //                ImTmp[p] = 1;
    //            else
    //                ImTmp[p] = 0;

    //        MxPutMatrixUC(ImTmp,ExtractProc->IwProcAngio,ExtractProc->IhProcAngio,"ImMask");

    //        if (k == ExtractSequence->CardiacCycleStart){
    //            //MxPutShort((short)ExtractProc->IwProcAngio,"Iw"); 
    //            //MxPutShort((short)ExtractProc->IhProcAngio,"Ih");
    //            //MxPutShort((short)(ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart),"In");
    //            
    //           MxCommand("fidOut = fopen('D://Temp//CycleNotCompletelyCleaned.tmp','wb');");
    //           //MxCommand("fprintf(fidOut,'%d %d %d %d\n',int32(Iw), int32(Ih), int32(In), 2);");

    //        }
    //        MxCommand("fwrite(fidOut, uint16(ImMask),'uint16');");

    //        
    //        if (k == ExtractSequence->CardiacCycleEnd){
    //            MxCommand("fclose(fidOut);");
    //        }
    //    }
    //}
	
	// * Transfer results
	OscFillAngioSequenceResults(ProcessResults, ExtractProc, ExtractSequence);

	ExtractProc->angioProcessed = 1;
	if (ExtractProc->detectedDiaphragm){
		// Diaphragm parabola in original table referential
		float a, b, c;
		a = ExtractProc->DiaphragmParams[0]; 
		b = ExtractProc->DiaphragmParams[1] + 2*ExtractProc->DiaphragmParams[0]*ExtractProc->deltaTabXSub; 
		c = ExtractProc->DiaphragmParams[2] + ExtractProc->DiaphragmParams[0]*ExtractProc->deltaTabXSub*ExtractProc->deltaTabXSub 
								+ ExtractProc->DiaphragmParams[1]*ExtractProc->deltaTabXSub - ExtractProc->deltaTabYSub;
		ExtractProc->DiaphragmParams[0] = a; ExtractProc->DiaphragmParams[1] = b; ExtractProc->DiaphragmParams[2] = c;
	}

	Err = OscStoreRefAngioStatus(OscHdl);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscStoreAngioStatus", "OscAngioProcessSequence");

	ExtractProc->t = -1;

#ifdef OSC_MXLAB_SHOW_SUBRES
{
    int nbCols = (int)ceil((double)sqrt((ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart+1) / 2.0));
    int nbLines = (int)ceil((ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart+1)/(double)nbCols);

    MxOpen(); MxSetVisible(1); MxCommand("figure(2);"); MxPutDouble((double)nbCols, "nbCols"); MxPutDouble((double)nbLines, "nbLines");
    for (k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
        MxPutDouble((double)(k-ExtractSequence->CardiacCycleStart+1), "k"); 
        MxPutMatrixUC(ExtractProc->BuffOverlayMask[k - ExtractSequence->CardiacCycleStart], ExtractProc->IwOri, ExtractProc->IhOri, "Ovr");
        MxCommand("xWidth = 1/nbLines; yHeight = 1/nbCols; yIndice = floor((k-1)/nbLines); xIndice = double((k-1)-yIndice*nbLines);");
        MxCommand("subplot('Position',[xIndice*xWidth,1-yHeight-yIndice*yHeight,xWidth,yHeight]);");
        MxCommand("imagesc(Ovr'); colormap gray; axis image; axis off;");
    }

     MxCommand("figure(3);"); MxPutDouble((double)ExtractSequence->PyrNbBands, "pyrNbLevels");
     for (k=0; k<= ExtractSequence->PyrNbBands; k++){
         int x,y, l = (ExtractSequence->CardiacCycleEnd+ExtractSequence->CardiacCycleStart)/2;
         short *ImX = (short*)malloc( ExtractSequence->PyrW[k]*ExtractSequence->PyrH[k]*sizeof(short));
         short *ImY = (short*)malloc( ExtractSequence->PyrW[k]*ExtractSequence->PyrH[k]*sizeof(short));
         COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[l]);
         for (y=0; y<ExtractSequence->PyrH[k]; y++)
             for (x=0; x<ExtractSequence->PyrW[k]; x++){
                 ImX[y*ExtractSequence->PyrW[k]+x] = ExtractPicture->ImPyrDir1D[k][y*ExtractSequence->PyrW[k]+x][0];
                 ImY[y*ExtractSequence->PyrW[k]+x] = ExtractPicture->ImPyrDir1D[k][y*ExtractSequence->PyrW[k]+x][1];
             }
        MxPutMatrixShort(ImX, ExtractSequence->PyrW[k], ExtractSequence->PyrH[k], "ImX");
        MxPutMatrixShort(ImY, ExtractSequence->PyrW[k], ExtractSequence->PyrH[k], "ImY");
        MxPutDouble((double)(k+1), "k");
        MxCommand("subplot(2, pyrNbLevels+1, k); imagesc(ImX'); colormap gray; axis image; axis off;");
        MxCommand("subplot(2, pyrNbLevels+1, pyrNbLevels+1+k); imagesc(ImY'); colormap gray; axis image; axis off;");
        free(ImX); free(ImY);
     }
}
#endif

#ifdef OSC_TIMING
	t1 = omp_get_wtime();
	printf("Time to process angio sequence: %.1f ms\n", 1000.0 * (t1-t0));
#endif
#ifdef OSC_TIMING_DETAIL
	printf("     (Cross registr %.1f Cycle selection %.1f Cleaning %.1f Overlays and pyramdis %.1f)\n", 1000.0 * (tAfterRegistration-t0), 1000.0 * (tAfterCycleSelec-tAfterRegistration)
		, 1000*(tAfterCleaning-tAfterCycleSelec), 1000.0 * (tAfterOverlaysAndPyr-tAfterCleaning));
	printf("     (Motion cycle reestimation %.1f Injection evaluation %.1f Cath inj ROI detection %.1f)\n"
		, 1000.0 * (tAfterMotionCycle-tAfterOverlaysAndPyr), 1000.0 * (tAfterInjQualityEst-tAfterMotionCycle), 1000.0 * (tAfterCathInjROIDetection-tAfterInjQualityEst));
#endif

	return ErrorCodeOut;
}

/*************************************************************************************/
int OscRegisterAngioImagesOnEachOther(COscHdl OscHdl){
    
    COsc                *This = (COsc*)OscHdl.Pv;
    COscExtractProc     *ExtractProc = &(This->ExtractProc);
    COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
    int                 k, p;
    std::mutex          mutex;

    // *** Loop on the angios
	for (k=0; k<ExtractSequence->nExtractPictures; k++){
		COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

		// Compute multi-resolution image for present frame
        OSC_MEMZERO(ExtractPicture->ImPyr1D[0], ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
		for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
            if (ExtractPicture->ImLabS[p] > 1)
                ExtractPicture->ImPyr1D[0][p] = 128;
		OscMrGaussianDecompPyr(ExtractPicture->ImPyr1D, ExtractSequence->PyrNbBands, ExtractSequence->PyrW, ExtractSequence->PyrH);

        
	    ExtractProc->pool->execute([&mutex,&OscHdl,&k](int threadIndex, int threadCount)
	    {	
            COsc                *This = (COsc*)OscHdl.Pv;
	        COscExtractProc     *ExtractProc     = &(This->ExtractProc);
	        COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	        int                  l;
	        int					CardiacCycleMaxFrames = (int)ceil(ExtractProc->frameRateAngio*60.f / OSC_MIN_CARDIACFREQ);
		    COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

            int                 firstIndice = OSC_MAX(0, k-CardiacCycleMaxFrames) 
                                            + (int)OSC_RND((float)threadIndex*(k+1-OSC_MAX(0, k-CardiacCycleMaxFrames))/threadCount);
            int                 lastIndice = OSC_MAX(0, k-CardiacCycleMaxFrames) 
                                            + (int)OSC_RND((float)(threadIndex+1)*(k+1-OSC_MAX(0, k-CardiacCycleMaxFrames))/threadCount);

		    for (l=firstIndice; l<lastIndice; l++){
			    float score;
			    int dX, dY;
			    int dXTab = ExtractSequence->ExtractPictures[l].deltaTabXSub - ExtractPicture->deltaTabXSub;
			    int dYTab = ExtractSequence->ExtractPictures[l].deltaTabYSub - ExtractPicture->deltaTabYSub;

			    score    =  
				    OscMultiResGlobalTranslationEstimationProduct(ExtractPicture->ImPyr1D, ExtractSequence->ExtractPictures[l].ImPyr1D
										    , ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, &ExtractProc->ShuttersRoi, dXTab, dYTab
										    , OSC_ANGIO_MAXRV, OSC_ANGIO_MAXRV, ExtractSequence->PyrNbBands, &dX, &dY );

		        std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
		
		        lock.lock();
			    {
				    ExtractSequence->MatchingScores[k][l]  = score;
				    ExtractSequence->dXAngioToAngio[k][l] = dX;
				    ExtractSequence->dYAngioToAngio[k][l] = dY;
				    ExtractSequence->MatchingScores[l][k]  = score;
				    ExtractSequence->dXAngioToAngio[l][k] = -dX;
				    ExtractSequence->dYAngioToAngio[l][k] = -dY;
			    }
		        lock.unlock();
		    }
        });
    }

	return 0;
}

/*************************************************************************************/
int OscComputeOverlay(COscHdl OscHdl, int t){

	
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

	short        		*ImLab = ExtractSequence->ExtractPictures[t].ImLabS;
	unsigned char   	*ImFrg = ExtractSequence->ExtractPictures[t].ImFrgUC;

	unsigned char		*ImOvr = ExtractSequence->ExtractPictures[t].ImOverlayMask;
    short               *ImS = ExtractProc->BufsS[0];
    unsigned char		*MaskHere= ExtractProc->BufsUC[0];

	int					IwProc = ExtractProc->IwProcAngio, IhProc = ExtractProc->IhProcAngio;
	int					p, x, y, Error;
	float				threshold, factor;
	COscRoiCoord		RoiCoord;
    
	// Converting to shorts
	OSC_MEMZERO(ImS, IwProc*IhProc);
	for (y=0; y<IhProc; y++)
		for (x=0; x<IwProc; x++)
			if (ImLab[y*IwProc+x] > 1)
				ImS[y*IwProc+x] = (short)(ImFrg[y*IwProc+x]);

	// Gaussian filtering
	Error = BxIsoGaussFilterShort2D(ImS, ImS, 1.f/2.82f, IwProc, IhProc);
	if(Error)
	{
        ExtractProc->FatalErrorOnPreviousImage = 1;
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscComputeOverlay", "BxIsoGaussFilterShort2D", 0, Error); 
		return Error;
	}

	// Normalization and convert to unsigned char
	OSC_MEMZERO(MaskHere, IwProc*IhProc);
	for (p=0; p<IwProc*IhProc; p++)
		if (ImS[p] > 0)
			MaskHere[p] = 1;
    
	RoiCoord.XMin = 0; RoiCoord.XMax = IwProc-1; RoiCoord.YMin = 0; RoiCoord.YMax = IhProc-1;

	OscPercentileComputationShort(This, ImS, MaskHere, IwProc, IhProc, &RoiCoord, 1-0.95f, &threshold);

	if (threshold > 0)
		factor = 255.0f/ threshold;
	else
		factor = 1.f;
	
	OSC_MEMZERO(ImOvr, IwProc*IhProc);
	for (p=0; p<IwProc*IhProc; p++)
		if (ImS[p])
			ImOvr[p] = (unsigned char) OSC_CLIP(factor * ImS[p], 0, 255);

	return 0;
}

/*************************************************************************************/
// Computing global motion field during cycle. 
// Full resolution estimation, and linear itnerpolation in time during cycle.
int OscFullResolutionCycleMotionEstimation(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

	int					firstFrameCycle, lastFrameCycle, cycleDX, cycleDY, k, l, dXTab, dYTab;
	int					searchRangeFullRes = (int)pow(2.f, ExtractSequence->PyrNbBands) * OSC_ANGIO_MAXRV;

	if (ExtractSequence->CardiacCycleEnd < ExtractSequence->nExtractPictures){
		firstFrameCycle = ExtractSequence->CardiacCycleStart; lastFrameCycle = ExtractSequence->CardiacCycleEnd+1;
	} else {
		firstFrameCycle = OSC_MAX(0,ExtractSequence->CardiacCycleStart-1); lastFrameCycle = ExtractSequence->CardiacCycleEnd;
	}

	// Full resolution motion estimation between begin and end of the selected cycle
	dXTab = ExtractSequence->ExtractPictures[lastFrameCycle].deltaTabXSub - ExtractSequence->ExtractPictures[firstFrameCycle].deltaTabXSub;
	dYTab = ExtractSequence->ExtractPictures[lastFrameCycle].deltaTabYSub - ExtractSequence->ExtractPictures[firstFrameCycle].deltaTabYSub;
	OscMultiResGlobalTranslationEstimationProduct(ExtractSequence->ExtractPictures[firstFrameCycle].ImPyr1D, ExtractSequence->ExtractPictures[lastFrameCycle].ImPyr1D
										, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, &ExtractProc->ShuttersRoi, dXTab, dYTab
										, searchRangeFullRes, searchRangeFullRes, 0, &cycleDX, &cycleDY );

	// Linear interpolation of the estimated motion field
	for (k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++)
		for (l=ExtractSequence->CardiacCycleStart; l<=k; l++){
			ExtractSequence->dXAngioToAngio[k][l] = cycleDX * (k-l) / (ExtractSequence->CardiacCycleEnd - ExtractSequence->CardiacCycleStart +1); 
			ExtractSequence->dYAngioToAngio[k][l] = cycleDY * (k-l) / (ExtractSequence->CardiacCycleEnd - ExtractSequence->CardiacCycleStart +1); 
			ExtractSequence->dXAngioToAngio[l][k] = - ExtractSequence->dXAngioToAngio[k][l];
			ExtractSequence->dYAngioToAngio[l][k] = - ExtractSequence->dYAngioToAngio[k][l];
		}

	return 0;
}

/*************************************************************************************/
float OscComputeInjectionIndicator(COscHdl OscHdl, int CardiacCycleStart, int CardiacCycleEnd){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

    float meanNbInjPix = 0.f;
    
	for (int k=CardiacCycleStart; k<= CardiacCycleEnd; k++){

		COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

		// Compute multi-resolution image for present frame
		for (int p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
			if (ExtractPicture->ImLabS[p] > 1)
               meanNbInjPix++;
	}

    return ( (meanNbInjPix * ExtractProc->isoCenterPixSize * ExtractProc->SSFactor * ExtractProc->isoCenterPixSize * ExtractProc->SSFactor)
         / (float)(CardiacCycleEnd-CardiacCycleStart+1) );
}


int OscInjectionQualityEstimation(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

	int ErrorCode =0;

 //   // *** Computing stats
 //   ExtractSequence->activeSurfacePix = (float)(ExtractProc->ShuttersRoi.XMax - ExtractProc->ShuttersRoi.XMin +1)
 //                                       * (ExtractProc->ShuttersRoi.YMax - ExtractProc->ShuttersRoi.YMin +1);

	//meanNbInjPix = 0;
	//for (k=ExtractSequence->CardiacCycleStart; k<= ExtractSequence->CardiacCycleEnd; k++){

 //       int i = k-ExtractSequence->CardiacCycleStart;
	//	COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

	//	// Compute multi-resolution image for present frame
 //       ExtractSequence->nbInjPixInSelectedCycle[i] = 0;
	//	for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
	//		if (ExtractPicture->ImLabS[p] > 1)
 //               ExtractSequence->nbInjPixInSelectedCycle[i] ++;

	//	meanNbInjPix += ExtractSequence->nbInjPixInSelectedCycle[i];
	//}

 //   ExtractSequence->meanInjectedSurfacePix = meanNbInjPix / (float)(ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart+1);
 //   ExtractSequence->meanInjectedSurfaceMM2 = ExtractSequence->meanInjectedSurfacePix * ExtractProc->isoCenterPixSize * ExtractProc->isoCenterPixSize;
 //   
	//if (ExtractSequence->meanInjectedSurfacePix < OSC_MEANRDGCYCLE_THRES_NOINJECTION * ExtractSequence->activeSurfacePix){
	//	ErrStartErrorReportFromLeaf(This->ErrHdl, "OscInjectionQualityEstimation", OSC_WRN_NONINJECTED_ANGIO, "Non injected angio");
	//	ErrorCode = OSC_WRN_NONINJECTED_ANGIO;
	//} else {
	//	if (ExtractSequence->meanInjectedSurfacePix < OSC_MEANRDGCYCLE_THRES_UNSUFFIENTINJECTION * ExtractSequence->activeSurfacePix){
	//		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscInjectionQualityEstimation", OSC_WRN_BADLY_INJECTED_ANGIO, "Barely injected angio");
	//		ErrorCode = OSC_WRN_BADLY_INJECTED_ANGIO;
	//	}
	//}

    // *** Real discussion
    ExtractSequence->meanInjectedSurfaceMM2 = OscComputeInjectionIndicator(OscHdl, ExtractSequence->CardiacCycleStart, ExtractSequence->CardiacCycleEnd);
    ExtractSequence->normalizedInjectionCoefficient = 0.5f * ExtractSequence->meanInjectedSurfaceMM2/OSC_MEANRDGCYCLE_THRES_UNSUFFIENTINJECTION;

	if (ExtractSequence->normalizedInjectionCoefficient < ExtractSequence->injectionCoefficientThreshold * OSC_MEANRDGCYCLE_THRES_NOINJECTION 
                                                    / OSC_MEANRDGCYCLE_THRES_UNSUFFIENTINJECTION){
		ErrStartErrorReportFromLeaf(This->ErrHdl, "OscInjectionQualityEstimation", OSC_WRN_NONINJECTED_ANGIO, "Non injected angio");
		ErrorCode = OSC_WRN_NONINJECTED_ANGIO;
	} else {
		if (ExtractSequence->normalizedInjectionCoefficient < ExtractSequence->injectionCoefficientThreshold){
			ErrStartErrorReportFromLeaf(This->ErrHdl, "OscInjectionQualityEstimation", OSC_WRN_BADLY_INJECTED_ANGIO, "Barely injected angio");
			ErrorCode = OSC_WRN_BADLY_INJECTED_ANGIO;
		}
	}

	return ErrorCode;
}

/*************************************************************************************/
void OscFillAngioSequenceResults(COscAngioSequenceResults *SequenceResults, COscExtractProc *ExtractProc, COscExtractSequence *ExtractSequence)
{
    int i;

	SequenceResults->CardiacCycleStart	    = ExtractSequence->CardiacCycleStart;
	SequenceResults->CardiacCycleEnd	    = ExtractSequence->CardiacCycleEnd;

    SequenceResults->Advanced.nbDetectedCathInjROI   = ExtractSequence->nbSelectedCathInjROI;
    memset(SequenceResults->detectedContactWithBorder, 0, 4*sizeof(unsigned char));
    for (i=0; i<ExtractSequence->nbSelectedCathInjROI; i++){
        SequenceResults->Advanced.DetectedCathInjROI[i].XMin = ExtractSequence->SelectedCathInjROI[i].XMin * ExtractProc->SSFactor;
        SequenceResults->Advanced.DetectedCathInjROI[i].XMax = ExtractSequence->SelectedCathInjROI[i].XMax * ExtractProc->SSFactor;
        SequenceResults->Advanced.DetectedCathInjROI[i].YMin = ExtractSequence->SelectedCathInjROI[i].YMin * ExtractProc->SSFactor;
        SequenceResults->Advanced.DetectedCathInjROI[i].YMax = ExtractSequence->SelectedCathInjROI[i].YMax * ExtractProc->SSFactor;
        if (ExtractSequence->SelectedCathInjROI[i].XMin < ExtractProc->ShuttersRoiSafe.XMin + OSC_CATHINJ_MARGINTOINCLUDELAB)
            SequenceResults->detectedContactWithBorder[0] = 1;
        if (ExtractSequence->SelectedCathInjROI[i].XMax > ExtractProc->ShuttersRoiSafe.XMax - OSC_CATHINJ_MARGINTOINCLUDELAB)
            SequenceResults->detectedContactWithBorder[2] = 1;
        if (ExtractSequence->SelectedCathInjROI[i].YMin < ExtractProc->ShuttersRoiSafe.YMin + OSC_CATHINJ_MARGINTOINCLUDELAB)
            SequenceResults->detectedContactWithBorder[1] = 1;
        if (ExtractSequence->SelectedCathInjROI[i].YMax > ExtractProc->ShuttersRoiSafe.YMax - OSC_CATHINJ_MARGINTOINCLUDELAB)
            SequenceResults->detectedContactWithBorder[3] = 1;
        
    }
    SequenceResults->detectionDubious = 0;
    if (SequenceResults->Advanced.nbDetectedCathInjROI != 1)
        SequenceResults->detectionDubious = 1;
    SequenceResults->Advanced.tInjBeginEstimated = ExtractProc->tInjBeginEstimated;

    SequenceResults->meanInjectedSurfaceMM2 = ExtractSequence->meanInjectedSurfaceMM2;
    SequenceResults->normalizedInjectionCoefficient = ExtractSequence->normalizedInjectionCoefficient;
    
    SequenceResults->Advanced.Volatile.AngiosMatchingScores = ExtractSequence->MatchingScores;
    SequenceResults->Advanced.Volatile.AngiosMatchingdX = ExtractSequence->dXAngioToAngio;
    SequenceResults->Advanced.Volatile.AngiosMatchingdY = ExtractSequence->dYAngioToAngio;

    
#ifdef OSC_CLEAN_DEBUG
    SequenceResults->Advanced.Volatile.ImLabOverSeg                     = ExtractSequence->ImLabOverSeq;
    SequenceResults->Advanced.Volatile.ImLabOverSeqAfterRdgThres        = ExtractSequence->ImLabOverSeqAfterRdgThres;
    SequenceResults->Advanced.Volatile.ImLabLargestRegions              = ExtractSequence->ImLabLargestRegions;
    SequenceResults->Advanced.Volatile.ImLabAfterClustering             = ExtractSequence->ImLabAfterClustering;
#else
    SequenceResults->Advanced.Volatile.ImLabOverSeg                     = NULL;
    SequenceResults->Advanced.Volatile.ImLabOverSeqAfterRdgThres        = NULL;
    SequenceResults->Advanced.Volatile.ImLabLargestRegions              = NULL;
    SequenceResults->Advanced.Volatile.ImLabAfterClustering             = NULL;
#endif

	return;
}

//int OscConsolidateSelectedHeartCycleTemporally(COscHdl OscHdl){
//
//	COsc                *This = (COsc*)OscHdl.Pv;
//	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
//	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
//
//  //  // Relabel lab
//  //  for (int k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
//		//COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);
//  //      short maxLab = 0;
//  //      for (int p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
//  //          maxLab = OSC_MAX(maxLab, ExtractPicture->ImLabS[p]);
//
//  //      int *nbEltsHere = (int*)malloc((maxLab+1)*sizeof(int));
//  //      memset(nbEltsHere, 0, (maxLab+1)*sizeof(int));
//  //      for (int p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
//  //          nbEltsHere[ExtractPicture->ImLabS[p]] ++;
//  //      
//  //      int *LabCorrespondance = (int*)malloc((maxLab+1)*sizeof(int));
//  //      int currentLab = 0;
//  //      for (int lab = 0; lab<=maxLab; lab++)
//  //          if (nbEltsHere[lab] > 0){
//  //              LabCorrespondance[lab] = currentLab; 
//  //              currentLab++;
//  //          } else {
//  //              LabCorrespondance[lab] = 0;
//  //          }
//
//  //      for (int p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
//  //          ExtractPicture->ImLabS[p] = LabCorrespondance[ExtractPicture->ImLabS[p]];
//
//  //      free(nbEltsHere); free(LabCorrespondance);
//  //  }
//
//
//
//    // * Compute stats coverage
//    int nbEltsLab[OSC_MAXLABELSNB+1], nbEltsCoveredLab[OSC_MAXLABELSNB+1];
//
//    for (int k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
//        memset(ExtractSequence->LabNbElts[k-ExtractSequence->CardiacCycleStart], 0, (OSC_MAXLABELSNB+1)*sizeof(float));
//    }
//
//    unsigned char **DilatedMasks = (unsigned char **)malloc( (ExtractSequence->CardiacCycleEnd-ExtractSequence->CardiacCycleStart+1)*sizeof(unsigned char*));
//    unsigned char *Buf = (unsigned char*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(unsigned char));
//    int sizeHalfDilPix = (int)OSC_RND(0.5*OSC_CONSOLIDATE_SIZE_DILATATION / (ExtractProc->isoCenterPixSize * ExtractProc->SSFactor));
//    
//    for (int k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
//		COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);
//
//        DilatedMasks[k-ExtractSequence->CardiacCycleStart] = (unsigned char*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(unsigned char));
//        memset(Buf, 0, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(unsigned char));
//        for (int dX = -sizeHalfDilPix; dX <= sizeHalfDilPix; dX++){
//            for (int y=0; y<ExtractProc->IhProcAngio; y++)
//                for (int x=OSC_MAX(0,-dX); x<OSC_MIN(ExtractProc->IwProcAngio,ExtractProc->IwProcAngio-dX); x++)
//                    if (ExtractPicture->ImLabS[y*ExtractProc->IwProcAngio+x+dX] > 1)
//                        Buf[y*ExtractProc->IwProcAngio+x] = 1;
//        }
//        memset(DilatedMasks[k-ExtractSequence->CardiacCycleStart], 0, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(unsigned char));
//        for (int dY = -sizeHalfDilPix; dY <= sizeHalfDilPix; dY++){
//            for (int y=OSC_MAX(0,-dY); y<OSC_MIN(ExtractProc->IhProcAngio,ExtractProc->IhProcAngio-dY); y++)
//                for (int x=0; x<ExtractProc->IwProcAngio; x++)
//                    DilatedMasks[k-ExtractSequence->CardiacCycleStart][y*ExtractProc->IwProcAngio+x] 
//                        = OSC_MAX(DilatedMasks[k-ExtractSequence->CardiacCycleStart][y*ExtractProc->IwProcAngio+x],
//                                Buf[(y+dY)*ExtractProc->IwProcAngio+x]);
//        }
//    }
//        
//    for (int k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
//        COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);
//            
//        memset(ExtractSequence->LabNbElts[k-ExtractSequence->CardiacCycleStart], 0, (OSC_MAXLABELSNB+1)*sizeof(float));
//        
//        memset(nbEltsLab, 0, (OSC_MAXLABELSNB+1)*sizeof(int));
//        for (int y=0; y<ExtractProc->IhProcAngio; y++)
//            for (int x=0; x<ExtractProc->IwProcAngio; x++){
//                if (ExtractPicture->ImLabS[y*ExtractProc->IwProcAngio+x] > 1){
//                    if (ExtractPicture->ImLabS[y*ExtractProc->IwProcAngio+x]-1 >= OSC_MAXLABELSNB+1){
//                        printf("trop haut mec %d\n", ExtractPicture->ImLabS[y*ExtractProc->IwProcAngio+x]-1); fflush(NULL);
//                    }
//
//                    nbEltsLab[ExtractPicture->ImLabS[y*ExtractProc->IwProcAngio+x]-1] ++;
//                }
//            }
//
//        for (int l=ExtractSequence->CardiacCycleStart; l<=ExtractSequence->CardiacCycleEnd; l++){
//            if (k!=l){
//                int dX = ExtractSequence->dXAngioToAngio[k][l], dY = ExtractSequence->dYAngioToAngio[k][l];
//                memset(nbEltsCoveredLab, 0, (OSC_MAXLABELSNB+1)*sizeof(int));
//                    
//                for (int y=OSC_MAX(0,-dY); y<OSC_MIN(ExtractProc->IhProcAngio,ExtractProc->IhProcAngio-dY); y++)
//                    for (int x=OSC_MAX(0,-dX); x<OSC_MIN(ExtractProc->IwProcAngio,ExtractProc->IwProcAngio-dX); x++){
//                        if ( (ExtractPicture->ImLabS[(y+dY)*ExtractProc->IwProcAngio+x+dX] > 1) 
//                            && (DilatedMasks[l-ExtractSequence->CardiacCycleStart][y*ExtractProc->IwProcAngio+x]) ){
//                            nbEltsCoveredLab[ExtractPicture->ImLabS[(y+dY)*ExtractProc->IwProcAngio+x+dX]-1] ++;
//                        }
//                    }
//                for (int i=0; i<OSC_MAXLABELSNB+1; i++)
//                    if (nbEltsLab[i] > 0){
//                        ExtractSequence->LabNbElts[k-ExtractSequence->CardiacCycleStart][i] += ((float)nbEltsCoveredLab[i]) / nbEltsLab[i];
//                    }
//                ////if (k == ExtractSequence->CardiacCycleStart)
//                //{ 
//                //    MxOpen(); MxSetVisible(1);
//                //    short *ImTmp = (short*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));
//                //    memset(ImTmp,0,ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));
//                //    for (int y=OSC_MAX(0,dY); y<OSC_MIN(ExtractProc->IhProcAngio,ExtractProc->IhProcAngio+dY); y++)
//                //            for (int x=OSC_MAX(0,dX); x<OSC_MIN(ExtractProc->IwProcAngio,ExtractProc->IwProcAngio+dX); x++)
//                //                ImTmp[y*ExtractProc->IwProcAngio+x] = ExtractPicture->ImLabS[(y-dY)*ExtractProc->IwProcAngio+x-dX];
//
//                //    MxPutMatrixShort(ExtractPicture->ImLabS, ExtractProc->IwProcAngio,ExtractProc->IhProcAngio, "ImLabRef");
//                //    MxPutMatrixShort(ImTmp, ExtractProc->IwProcAngio,ExtractProc->IhProcAngio, "ImLabRefWarped");
//                //    MxPutMatrixUC(DilatedMasks[l-ExtractSequence->CardiacCycleStart], ExtractProc->IwProcAngio
//                //        ,ExtractProc->IhProcAngio, "DilatedMasks");
//                //    
//                //    memset(ImTmp,0,ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(short));
//                //    for (int y=OSC_MAX(0,-dY); y<OSC_MIN(ExtractProc->IhProcAngio,ExtractProc->IhProcAngio-dY); y++)
//                //            for (int x=OSC_MAX(0,-dX); x<OSC_MIN(ExtractProc->IwProcAngio,ExtractProc->IwProcAngio-dX); x++)
//                //                ImTmp[y*ExtractProc->IwProcAngio+x] = ExtractPicture->ImLabS[(y+dY)*ExtractProc->IwProcAngio+x+dX];
//                //    
//                //    MxPutMatrixShort(ImTmp, ExtractProc->IwProcAngio,ExtractProc->IhProcAngio, "ImLabRefWarped2");
//
//                //                                
//                //    MxCommand("figure(2); subplot(1,2,1); imagesc(double(DilatedMasks' > 0) + double(ImLabRefWarped'> 1) ); colormap gray; axis image; axis off;");
//                //    MxCommand("figure(2); subplot(1,2,2); imagesc(double(DilatedMasks' > 0) + double(ImLabRefWarped2'> 1) ); colormap gray; axis image; axis off;");
//                //    //MxCommand("figure(3); subplot(1,1,1); imagesc(double(ImLab2' > 1) + double(ImLabRefWarped2'> 1) ); colormap gray; axis image; axis off;");
//
//                //    MxCommand("figure(1); subplot(2,2,1); imagesc(ImLabRef'); colormap gray; axis image; axis off;");
//                //    MxCommand("figure(1); subplot(2,2,3); imagesc(DilatedMasks'); colormap gray; axis image; axis off;");
//                //    MxCommand("figure(1); subplot(2,2,4); imagesc(double(DilatedMasks' > 0) + double(ImLabRefWarped'> 1) ); colormap gray; axis image; axis off;");
//                //    MxCommand("figure(1); subplot(2,2,2); imagesc(ImLabRefWarped'); colormap gray; axis image; axis off;");
//
//                //    free(ImTmp);
//                //}
//            }
//        }
//    }
//
//    for (int k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
//        free( DilatedMasks[k-ExtractSequence->CardiacCycleStart] );
//    }
//    free(Buf);
//
//
//    ExtractSequence->SSCumX = ExtractProc->IwProcAngio / OSC_CONSOLIDATE_CYCLE_IW;
//    ExtractSequence->SSCumY = ExtractProc->IhProcAngio / OSC_CONSOLIDATE_CYCLE_IH;
//
//    // * Compute consolidated map
//    memset(ExtractSequence->MapCumulatedCycle, 0, OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH*sizeof(unsigned char));
//	for (int k=ExtractSequence->CardiacCycleStart; k<=ExtractSequence->CardiacCycleEnd; k++){
//		COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);
//
//        for (int yCum=0; yCum<OSC_CONSOLIDATE_CYCLE_IH; yCum++){
//            for (int xCum=0; xCum<OSC_CONSOLIDATE_CYCLE_IW; xCum++){
//                bool valFound = false;
//
//                for (int y=yCum*ExtractSequence->SSCumY; y<OSC_MIN(ExtractProc->IhProcAngio,(yCum+1)*ExtractSequence->SSCumY); y++){
//                    for (int x=xCum*ExtractSequence->SSCumX; x<OSC_MIN(ExtractProc->IwProcAngio,(xCum+1)*ExtractSequence->SSCumX); x++){
//                        if (ExtractPicture->ImLabS[y*ExtractProc->IwProcAngio+x] > 1){
//                            ExtractSequence->MapCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] ++;
//                            valFound = true;
//                            break;
//                        }
//                    }
//                    if (valFound)
//                        break;
//                }
//            }
//        }
//    }
//
//    // * Connected components
//    unsigned char maxCumCycle = 0;
//    for (int yCum=0; yCum<OSC_CONSOLIDATE_CYCLE_IH; yCum++)
//        for (int xCum=0; xCum<OSC_CONSOLIDATE_CYCLE_IW; xCum++){
//            maxCumCycle = OSC_MAX(maxCumCycle, ExtractSequence->MapCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] );
//        }
//
//    unsigned char minValForLab = (unsigned char)OSC_RND( OSC_MAX(0, OSC_CONSOLIDATE_CYCLE_RATIO_MAX * maxCumCycle) );
//    short currentLab = 1;
//    memset(ExtractSequence->LabCumulatedCycle, 0, OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH*sizeof(short));
//    // 4-neighboor labeling of significant elements
//    for (int yCum=0; yCum<OSC_CONSOLIDATE_CYCLE_IH; yCum++)
//        for (int xCum=0; xCum<OSC_CONSOLIDATE_CYCLE_IW; xCum++){
//
//            if ((ExtractSequence->MapCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] > minValForLab) 
//                && (ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] == 0)){
//                    
//                OscSubFunc4NeighborLabeling(ExtractSequence->MapCumulatedCycle, OSC_CONSOLIDATE_CYCLE_IW, OSC_CONSOLIDATE_CYCLE_IH, minValForLab
//                                            , xCum, yCum, currentLab, ExtractSequence->LabCumulatedCycle);
//                currentLab++;
//            }
//        }
//    ExtractSequence->nbLabCumulatedCycle = currentLab-1;
//    // Closest neighbor for remaining elements
//    int nbEltsNotLabelledPrev, nbEltsNotlabelled = 0;
//    for (int yCum=0; yCum<OSC_CONSOLIDATE_CYCLE_IH; yCum++)
//        for (int xCum=0; xCum<OSC_CONSOLIDATE_CYCLE_IW; xCum++){
//            if ( (ExtractSequence->MapCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] > 0) && 
//                (ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] == 0)){
//                    nbEltsNotlabelled++;
//            }
//        }
//    nbEltsNotLabelledPrev = nbEltsNotlabelled+1;
//    while (nbEltsNotlabelled < nbEltsNotLabelledPrev){
//        nbEltsNotLabelledPrev = nbEltsNotlabelled;
//        nbEltsNotlabelled = 0;
//        for (int yCum=0; yCum<OSC_CONSOLIDATE_CYCLE_IH; yCum++)
//            for (int xCum=0; xCum<OSC_CONSOLIDATE_CYCLE_IW; xCum++){
//                if ( (ExtractSequence->MapCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] > 0) && 
//                    (ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] == 0)){
//                        bool unlabelled = true;
//                        if (ExtractSequence->LabCumulatedCycle[OSC_MAX(0,yCum-1)*OSC_CONSOLIDATE_CYCLE_IW+xCum] >0){
//                            ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] 
//                                = ExtractSequence->LabCumulatedCycle[OSC_MAX(0,yCum-1)*OSC_CONSOLIDATE_CYCLE_IW+xCum];
//                            unlabelled = false;
//                        }
//                        if (ExtractSequence->LabCumulatedCycle[OSC_MIN(OSC_CONSOLIDATE_CYCLE_IH-1,yCum+1)*OSC_CONSOLIDATE_CYCLE_IW+xCum] >0){
//                            ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] 
//                                = ExtractSequence->LabCumulatedCycle[OSC_MIN(OSC_CONSOLIDATE_CYCLE_IH-1,yCum+1)*OSC_CONSOLIDATE_CYCLE_IW+xCum];
//                            unlabelled = false;
//                        }
//                        if (ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+OSC_MAX(0,xCum-1)] >0){
//                            ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] 
//                                = ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+OSC_MAX(0,xCum-1)];
//                            unlabelled = false;
//                        }
//                        if (ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+OSC_MIN(OSC_CONSOLIDATE_CYCLE_IW-1,xCum+1)] >0){
//                            ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] 
//                                = ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+OSC_MIN(OSC_CONSOLIDATE_CYCLE_IW-1,xCum+1)];
//                            unlabelled = false;
//                        }
//                        if (unlabelled)
//                            nbEltsNotlabelled++;
//                }
//            }
//    }
//    for (int lab =1; lab<currentLab; lab++){
//        ExtractSequence->CumulatedIndicatorPerLab[lab-1] = 0;
//        for (int p=0; p<OSC_CONSOLIDATE_CYCLE_IH*OSC_CONSOLIDATE_CYCLE_IW; p++)
//            if (ExtractSequence->LabCumulatedCycle[p] == lab){
//                ExtractSequence->CumulatedIndicatorPerLab[lab-1] = OSC_MAX(ExtractSequence->CumulatedIndicatorPerLab[lab-1], 
//                                                                            ExtractSequence->MapCumulatedCycle[p]);
//            }
//    }
//
//    // * Final decision
//
//    // 1- Regions with max indicator
//    memset(ExtractSequence->MaskCumulatedCycle, 0, OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH*sizeof(unsigned char));
//    for (int p=0; p<OSC_CONSOLIDATE_CYCLE_IW*OSC_CONSOLIDATE_CYCLE_IH; p++)
//        if (ExtractSequence->CumulatedIndicatorPerLab[ExtractSequence->LabCumulatedCycle[p]-1] >= OSC_CONSOLIDATE_CYCLE_THRES_H * maxCumCycle)
//           ExtractSequence->MaskCumulatedCycle[p] = 1; 
//
//    // 2- Neighboring regioins with sufficiently high indicator
//    bool updatedMap = true;
//    while (updatedMap){
//        updatedMap = false;
//        for (int yCum=0; yCum<OSC_CONSOLIDATE_CYCLE_IH; yCum++)
//            for (int xCum=0; xCum<OSC_CONSOLIDATE_CYCLE_IW; xCum++)
//                if ( (ExtractSequence->CumulatedIndicatorPerLab[ExtractSequence->LabCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum]-1] 
//                                >= OSC_CONSOLIDATE_CYCLE_THRES_L * maxCumCycle)
//                    && (ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum] == 0) ){
//
//                        if (ExtractSequence->MaskCumulatedCycle[OSC_MAX(0,yCum-1)*OSC_CONSOLIDATE_CYCLE_IW+xCum] == 1){
//                            ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum]  = 1; updatedMap = true;
//                        }
//                        if (ExtractSequence->MaskCumulatedCycle[OSC_MIN(OSC_CONSOLIDATE_CYCLE_IH-1,yCum+1)*OSC_CONSOLIDATE_CYCLE_IW+xCum] == 1){
//                            ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum]  = 1; updatedMap = true;
//                        }
//                        if (ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+OSC_MAX(0,xCum-1)] == 1){
//                            ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum]  = 1; updatedMap = true;
//                        }
//                        if (ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+OSC_MIN(OSC_CONSOLIDATE_CYCLE_IW-1,xCum+1)] == 1){
//                            ExtractSequence->MaskCumulatedCycle[yCum*OSC_CONSOLIDATE_CYCLE_IW+xCum]  = 1; updatedMap = true;
//                        }
//                    }
//    }
//    
//
//
//    //{
//    //    MxOpen(); MxSetVisible(1);
//    //    MxPutMatrixUC(ExtractSequence->MapCumulatedCycle, OSC_CONSOLIDATE_CYCLE_IW, OSC_CONSOLIDATE_CYCLE_IH, "MapCumulatedCycle");
//    //    MxPutMatrixShort(ExtractSequence->LabCumulatedCycle, OSC_CONSOLIDATE_CYCLE_IW, OSC_CONSOLIDATE_CYCLE_IH, "LabCumulatedCycle");
//    //    
//    //    short *ImS = (short*)malloc(OSC_CONSOLIDATE_CYCLE_IH*OSC_CONSOLIDATE_CYCLE_IW*sizeof(short));
//    //    for (int p=0; p<OSC_CONSOLIDATE_CYCLE_IH*OSC_CONSOLIDATE_CYCLE_IW; p++)
//    //        if (ExtractSequence->LabCumulatedCycle[p] == 0)
//    //            ImS[p] = 0;
//    //        else
//    //            ImS[p] = ExtractSequence->CumulatedIndicatorPerLab[ExtractSequence->LabCumulatedCycle[p]-1];
//    //    MxPutMatrixShort(ImS, OSC_CONSOLIDATE_CYCLE_IW, OSC_CONSOLIDATE_CYCLE_IH, "IndicatorCumulatedCycle");
//
//    //    MxCommand("figure(1); subplot(1,3,1); imagesc(MapCumulatedCycle'); colormap gray; axis image; axis off;");
//    //    MxCommand("figure(1); subplot(1,3,2); imagesc(LabCumulatedCycle'); colormap gray; axis image; axis off;");
//    //    MxCommand("figure(1); subplot(1,3,3); imagesc(IndicatorCumulatedCycle'); colormap gray; axis image; axis off;");
//
//    //    free(ImS);
//    //}
//
//    return 0;
//}
//
//void OscSubFunc4NeighborLabeling(unsigned char *Map, int Iw, int Ih, unsigned char threshold, int x, int y, short currLab, short *Lab){
//
//    if (Map[y*Iw+x] > threshold && Lab[y*Iw+x] == 0){
//        Lab[y*Iw+x]  = currLab;
//        if (x<Iw-1)
//            OscSubFunc4NeighborLabeling(Map, Iw, Ih, threshold, x+1, y, currLab, Lab);
//        if (y<Ih-1)
//            OscSubFunc4NeighborLabeling(Map, Iw, Ih, threshold, x, y+1, currLab, Lab);
//        if (x>0)
//            OscSubFunc4NeighborLabeling(Map, Iw, Ih, threshold, x-1, y, currLab, Lab);
//        if (y>0)
//            OscSubFunc4NeighborLabeling(Map, Iw, Ih, threshold, x, y-1, currLab, Lab);
//    }
//    
//    //if (Map[y*Iw+x] > 0)
//    //    Lab[y*Iw+x]  = currLab;
//
//}
