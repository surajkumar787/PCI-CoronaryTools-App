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

//#define OSC_TIMING

#include <mutex> 



// *************************************************
// Oversegmentation in regions of same direction
// Keep labels with significant mean ridgeness
// Threshold for selection tentatively decreased, as long as it allows to agregate regions significant in size

int OscRdgCleanVasculatureImage(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);

	unsigned char		*ImMask = ExtractProc->ImMask;
	float				*ImFrg = ExtractProc->ImFrg;
	float				*ImDir = ExtractProc->ImDir;
	int 				*ImLab = ExtractProc->ImLab;

	int					*ImLabOverSeg	= ExtractProc->BufsI[0];
	int					*nbPixPerLab	= ExtractProc->BufsI[1];
    
	float				*meanRdgPerLab	= ExtractProc->BufsF[0];
    float               *CumRdgTmp	= ExtractProc->BufsF[1]; 
    float               *ImFrgFiltTmp = ExtractProc->BufsF[2];
    
	short               *stack_x			= ExtractProc->BufsS[0];
	short               *stack_y			= ExtractProc->BufsS[1];
	short               *stack_x0			= ExtractProc->BufsS[2];
	short               *stack_y0			= ExtractProc->BufsS[3];
	short               *nbIterOnMaskIn	    = ExtractProc->BufsS[4];

	int					Err, ErrorCodeOut = 0, nbLabels, Iw = ExtractProc->IwProcAngio, Ih = ExtractProc->IhProcAngio, p;
        
	// *** Oversegmentation in portions of exact same directions
    // And labels statistics
    Err =  OscClean_OverSegmentAndStats(OscHdl, ImMask, ImFrg, ImDir, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, CumRdgTmp, ImFrgFiltTmp
                                        , stack_x, stack_y, stack_x0, stack_y0, nbIterOnMaskIn
                                        , ImLabOverSeg, &nbLabels, nbPixPerLab, meanRdgPerLab);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscClean_OverSegmentAndStats", "OscRdgCleanVasculatureImage");

    // *** Compute optimal mean ridge threshold
    ExtractProc->storedThresholds[ExtractProc->t] = OscClean_ComputeMeanRdgThresholdImageWise(nbPixPerLab, meanRdgPerLab, nbLabels);

    if (ExtractProc->t >= ExtractProc->nbThresholdsForMedianComputation-1){
        // Compute limit
        for (int i=0; i<ExtractProc->nbThresholdsForMedianComputation; i++)
            ExtractProc->storedThresholdsBuf1[i] = ExtractProc->storedThresholds[ExtractProc->t-ExtractProc->nbThresholdsForMedianComputation+1+i];

        float medianThresStored = 0.f;
        Err = OscMedian(This, ExtractProc->storedThresholdsBuf1, ExtractProc->storedThresholdsBuf2
                                            , 0, ExtractProc->nbThresholdsForMedianComputation/2, ExtractProc->nbThresholdsForMedianComputation-1, &medianThresStored);
        if(Err)
	    {
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "OscRdgCleanVasculatureImage", "OscMedian", 0, Err);
		    return Err;
	    }

        for (int i=0; i<ExtractProc->nbThresholdsForMedianComputation; i++){
            ExtractProc->storedThresholdsBuf1[i] = OSC_ABS_F(ExtractProc->storedThresholds[ExtractProc->t-ExtractProc->nbThresholdsForMedianComputation+1+i]
                                                        - medianThresStored);
        }
        float medianStdThresStored = 0.f;
        Err = OscMedian(This, ExtractProc->storedThresholdsBuf1, ExtractProc->storedThresholdsBuf2
                                            , 0, ExtractProc->nbThresholdsForMedianComputation/2, ExtractProc->nbThresholdsForMedianComputation-1, &medianStdThresStored);
        if(Err)
	    {
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "OscRdgCleanVasculatureImage", "OscMedian", 0, Err);
		    return Err;
	    }

        float limitThresholdRdg = medianThresStored - OSC_CLEAN_COEFFSTDFORTHRES_SEQ * medianStdThresStored;
        
        // Final decision
        ExtractProc->minRdgThresholdClean =  OSC_MAX(ExtractProc->storedThresholds[ExtractProc->t],limitThresholdRdg);
    } else {
        ExtractProc->minRdgThresholdClean = ExtractProc->storedThresholds[ExtractProc->t];
    }

	// Conclusion
	OSC_MEMZERO(ImMask, Iw*Ih);
	for (p=0; p<Iw*Ih; p++)
		if ((ImLabOverSeg[p] > 1) && (meanRdgPerLab[ImLabOverSeg[p]-2] > ExtractProc->minRdgThresholdClean)){
            ImMask[p] = 1; 
        } else {
            if (ImLab[p] > 0)
                ImLab[p] = 1;
        }

	return 0;
}

// ************************************************************************
int OscClean_OverSegmentAndStats(COscHdl OscHdl, unsigned char *ImMaskIn, float *ImFrgIn, float *ImDirIn, int Iw, int Ih
                                 , float *CumRdgTmp, float *ImFrgFiltTmp, short *stack_x, short *stack_y, short *stack_x0, short *stack_y0, short *nbIterOnMaskIn
                                  , int *ImLabOut, int *nbLabelsOut, int *nbPixPerLabOut, float *meanRdgOut){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
    int                 Err, ErrorCodeOut = 0;
    
    Err = OscRdgDirectionalLabelingFromSeedDetailed(OscHdl, ImMaskIn, ImMaskIn, ImFrgIn, ImDirIn, Iw, Ih
                                                    , OSC_CLEAN_RANGE_LABELING, OSC_CLEAN_LIM_DIR
                                                    , stack_x, stack_y, stack_x0, stack_y0, nbIterOnMaskIn
													, ImLabOut, nbLabelsOut, nbPixPerLabOut, CumRdgTmp);
	OSC_ERR_REPORT_FROM_CALLER(Err, ErrorCodeOut, "OscRdgDirectionalLabelingFromSeed", "OscClean_OverSegmentAndStats");

	// Filtering: max per label
	OscImMaxFiltPerLab(ImLabOut, ImFrgIn, ImFrgFiltTmp, Iw, Ih, OSC_CLEAN_MAXFILSIZE);

	// *** Mean ridgeness per label
	OSC_MEMZERO(meanRdgOut, *nbLabelsOut);
		
	for (int p=0; p<Iw*Ih; p++)
		if (ImLabOut[p] > 1){
			meanRdgOut[ImLabOut[p]-2] += ImFrgFiltTmp[p];
		}

	for (int l=0; l<*nbLabelsOut; l++)
		if (nbPixPerLabOut[l] > 0)
			meanRdgOut[l] /= nbPixPerLabOut[l];
		else
			meanRdgOut[l] = 0;

    return ErrorCodeOut;
}

// ************************************************************************
float OscClean_ComputeMeanRdgThresholdImageWise(int *nbPixPerLab, float *meanRdgPerLab, int nbLabels){

    float maxRdgLab, thrRdgCent, deltaThres;
    int sizeMaxLab, sizeTree;
    bool ok;

	maxRdgLab = 0; sizeMaxLab = 0;
	for (int l=0; l<nbLabels; l++)
		if (sizeMaxLab < nbPixPerLab[l]){
			sizeMaxLab = nbPixPerLab[l];
			maxRdgLab = meanRdgPerLab[l];
		}

	// Decision on which labels are kept

	// Initial thresholds
	thrRdgCent = OSC_CLEAN_INICOEFFTHRES * maxRdgLab;
	deltaThres = OSC_CLEAN_COEFFTHRESSTEP * maxRdgLab;
	sizeTree = 0;
	for (int l=0; l<nbLabels; l++)
		if (meanRdgPerLab[l] > thrRdgCent)
			sizeTree += nbPixPerLab[l];
	
	// Threshold on ridges is progressively decreased, as long as it allows to agregate large enough branches
	ok = true;
	while (ok){
		int sizeAdded = 0;
		for (int l=0; l<nbLabels; l++)
			if ( (meanRdgPerLab[l] > thrRdgCent - deltaThres) && (meanRdgPerLab[l] <= thrRdgCent) )
				sizeAdded += nbPixPerLab[l];

		if (sizeAdded > OSC_CLEAN_THRSIZEADDED * sizeTree){
			thrRdgCent -= deltaThres;
			sizeTree += sizeAdded;
		} else
			ok = 0;
	}

    return thrRdgCent;
}


// ************************************************************************
// Prefiltering: local max over considered label

void OscImMaxFiltPerLab(int * ImLab, float * Rdg, float * RdgFilt, int Iw, int Ih, int sizeFilt){

	int x, y, xx, yy, sizeFilt_2 = sizeFilt / 2;

	OSC_MEMZERO(RdgFilt, Iw*Ih);

	for (y=0; y<Ih; y++)
		for (x=0; x<Iw; x++)
			if (ImLab[y*Iw+x] > 1){
				RdgFilt[y*Iw+x] = Rdg[y*Iw+x];
				for (yy = OSC_MAX(0,y-sizeFilt_2); yy <= OSC_MIN(Ih-1,y+sizeFilt_2); yy++)
					for (xx = OSC_MAX(0,x-sizeFilt_2); xx <= OSC_MIN(Iw-1,x+sizeFilt_2); xx++)
						if ( (ImLab[yy*Iw+xx] == ImLab[y*Iw+x]) && (Rdg[yy*Iw+xx] > RdgFilt[y*Iw+x]) )
							RdgFilt[y*Iw+x] = Rdg[yy*Iw+xx];
			}

}

// ************************************************************************
int OscRdgCleanVasculatureSequenceConsolidation(COscHdl OscHdl, int firstImage, int lastImage){
    int                 ErrorCodeOut = 0;

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc     = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
    int                 IwProc = ExtractProc->IwProcAngio;
    COscRoiCoord *RoiCoord = &ExtractProc->ShuttersRoiSafe;
#ifdef OSC_TIMING
	double t0 = omp_get_wtime(), t1, t2, t3;
#endif
    

    // *** Consolidated threshold
    int cardiacCycleLength = lastImage-firstImage+1;
    float * buf1 = (float*)malloc((cardiacCycleLength+1)*sizeof(float))
        , * buf2 = (float*)malloc((cardiacCycleLength+1)*sizeof(float));

    for (int i=0; i<=cardiacCycleLength; i++){
        buf1[i] = ExtractProc->storedThresholds[firstImage+i];
    }
    float medianThresCycle = 0.f;
    int Err = OscMedian(This, buf1, buf2, 0, cardiacCycleLength/2, cardiacCycleLength, &medianThresCycle);
    if(Err)
	{
        free(buf1); free(buf2);
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscRdgCleanVasculatureSequenceConsolidation", "OscMedian", 0, Err);
		return Err;
	}

    for (int i=0; i<=cardiacCycleLength; i++){
        buf1[i] = OSC_ABS_F(ExtractProc->storedThresholds[firstImage+i] - medianThresCycle);
    }
    float medianStdThresCycle = 0.f;
    Err = OscMedian(This, buf1, buf2, 0, cardiacCycleLength/2, cardiacCycleLength, &medianStdThresCycle);
    if(Err)
	{
        free(buf1); free(buf2);
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscRdgCleanVasculatureSequenceConsolidation", "OscMedian", 0, Err);
		return Err;
	}

    float ThresholdMeanRdg = medianThresCycle - OSC_CLEAN_COEFFSTDFORTHRES_SEQ * medianStdThresCycle;

    free(buf1); free(buf2);

#ifdef OSC_TIMING
	t1 = omp_get_wtime();
#endif

    // *** Cleaning the sequence
    short ** ImLabsStored = ExtractSequence->storeCycleShort;//(short**)malloc((lastImage-firstImage+1)*sizeof(short*));
    unsigned char *ImCumDil = ExtractProc->BufsUC[0];

    std::mutex          mutex;
	ExtractProc->pool->execute([&mutex,&OscHdl,&firstImage,&lastImage, &ImCumDil, &ThresholdMeanRdg, &ImLabsStored](int threadIndex, int threadCount)
	{
	    COsc                *This = (COsc*)OscHdl.Pv;
	    COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
	    COscExtractProc     *ExtractProc     = &(This->ExtractProc);

        int                 IwProc = ExtractProc->IwProcAngio, IhProc = ExtractProc->IhProcAngio;
        COscRoiCoord        *RoiCoord = &ExtractProc->ShuttersRoiSafe;

        int firstImageThread    = firstImage + (int)OSC_RND((float)threadIndex * (lastImage-firstImage+1) / threadCount);
        int lastImageThread     = firstImage + (int)OSC_RND((float)(threadIndex+1) * (lastImage-firstImage+1) / threadCount);

        unsigned char		*ImMask, *ImDil, *ImToDil, *BufMask;
        float	        	*ImFrg, *ImDir, *meanRdgPerLab,*CumRdgTmp,*ImFrgFiltTmp;
        int					*ImLabOverSeg, *nbPixPerLab;
        short               *stack_x, *stack_y, *stack_x0, *stack_y0, *nbIterOnMaskIn;
        int                 nbLabelsOverSeg;

        ImMask  = ExtractProc->BufsUC[4*threadIndex+1];
        ImDil   = ExtractProc->BufsUC[4*threadIndex+2];
        ImToDil = ExtractProc->BufsUC[4*threadIndex+3];
        BufMask = ExtractProc->BufsUC[4*threadIndex+4];

        ImFrg = ExtractProc->BufsF[5*threadIndex];
        ImDir = ExtractProc->BufsF[5*threadIndex+1];
        meanRdgPerLab = ExtractProc->BufsF[5*threadIndex+2];
        CumRdgTmp = ExtractProc->BufsF[5*threadIndex+3];
        ImFrgFiltTmp = ExtractProc->BufsF[5*threadIndex+4];

        ImLabOverSeg = ExtractProc->BufsI[2*threadIndex];
        nbPixPerLab = ExtractProc->BufsI[2*threadIndex+1];

        stack_x = ExtractProc->BufsS[5*threadIndex];
        stack_y = ExtractProc->BufsS[5*threadIndex+1];
        stack_x0 = ExtractProc->BufsS[5*threadIndex+2];
        stack_y0 = ExtractProc->BufsS[5*threadIndex+3];
        nbIterOnMaskIn = ExtractProc->BufsS[5*threadIndex+4];
        
        for (int k=firstImageThread; k<lastImageThread; k++){
            COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);

            short *ImLabOverSeqAfterRdgThres, *ImLabLargestRegions, *ImLabAfterClustering;
            unsigned char *BufUC1, *BufUC2, *BufUC3;
            int *nbPixPerLabAfterRdgThres;

            OSC_CALLOC(ImLabOverSeqAfterRdgThres, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
            OSC_CALLOC(ImLabLargestRegions, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
            OSC_CALLOC(ImLabAfterClustering, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
            OSC_CALLOC(BufUC1, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
            OSC_CALLOC(BufUC2, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
            OSC_CALLOC(BufUC3, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
        
            // Converting images
            OSC_MEMZERO(ImMask, IwProc*IhProc);
            for (int p=0; p<IwProc*IhProc; p++){
                if (ExtractPicture->ImLabS[p] >1)
                    ImMask[p] = 1;

                ImFrg[p] = ExtractPicture->ImFrgUC[p] * (ExtractPicture->maxFrg-ExtractPicture->minFrg)/255.f + ExtractPicture->minFrg;

                ImDir[p] = (float)((ExtractPicture->ImDirUC[p] -128.f) * OSC_PI / 254.f);
            }
        
	        // Oversegmentation in portions of exact same directions and labels statistics
            OscClean_OverSegmentAndStats(OscHdl, ImMask, ImFrg, ImDir, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, CumRdgTmp, ImFrgFiltTmp
                                        , stack_x, stack_y, stack_x0, stack_y0, nbIterOnMaskIn
                                        , ImLabOverSeg, &nbLabelsOverSeg, nbPixPerLab, meanRdgPerLab);

            OSC_CALLOC(nbPixPerLabAfterRdgThres, nbLabelsOverSeg);
	        for (int p=0; p<IwProc*IhProc; p++){
		        if ((ImLabOverSeg[p] > 1)){
                    if (meanRdgPerLab[ImLabOverSeg[p]-2] > OSC_MAX(ExtractProc->storedThresholds[k],ThresholdMeanRdg)){
                        ImLabOverSeqAfterRdgThres[p] = ImLabOverSeg[p];
                    }
                }
            }
            for (int lab=0; lab<nbLabelsOverSeg; lab++)
                if (meanRdgPerLab[lab] <= OSC_MAX(ExtractProc->storedThresholds[k],ThresholdMeanRdg))
                    nbPixPerLab[lab] = 0;

            // Keep longest 
            int nbLabsLongest = OscClean_RemoveShortestLabels(ImLabOverSeqAfterRdgThres, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio
                                            , nbPixPerLab, nbLabelsOverSeg, OSC_CLEAN_NBREGIONS_BYLENGTH, ImLabLargestRegions, nbPixPerLabAfterRdgThres);
        
            
            // Cluster by distance
            int nbClusters = OscClean_ClusteringByDistance(ImLabLargestRegions, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, RoiCoord
                                                        , nbPixPerLabAfterRdgThres, nbLabsLongest
                                                        , OSC_CLEAN_DISTFORCLUSTERING_MM / (ExtractProc->SSFactor*ExtractProc->isoCenterPixSize)
                                                        , BufUC1, BufUC2, BufUC3, ImLabAfterClustering);
            if (nbClusters == -1){
                free(ImLabOverSeqAfterRdgThres); free(ImLabLargestRegions); free(ImLabAfterClustering);
                free(BufUC1); free(BufUC2); free(BufUC3);
                free(nbPixPerLabAfterRdgThres);
                continue;
            }

            OscClean_ComputeFinalMapFromClusters(ImLabAfterClustering, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, RoiCoord, nbClusters
                                              , ExtractProc->SSFactor*ExtractProc->isoCenterPixSize, OSC_CLEAN_MAXDISTONSAMEBORDER_MM
                                              , OSC_CLEAN_MINDISTTOINCLUDEBRANCHWITHBORDERCONTACT_MM, ExtractPicture->ImLabS);
        
        

            // Compute cumulated dilatation image
            for (int p=0; p<IwProc*IhProc; p++){
                if (ExtractPicture->ImLabS[p] > 1)
                    ImToDil[p] = 1;
                else
                    ImToDil[p] = 0;
            }
            int limPixHalf = OSC_RND( OSC_CLEAN_DILSIZE_TEMPCONS_MM / (2*ExtractProc->isoCenterPixSize * ExtractProc->SSFactor) );
            OscDilationRect(ImToDil, IwProc, IhProc, RoiCoord, limPixHalf, limPixHalf, BufMask, ImDil);
            
		    std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
            lock.lock();
		    {
                for (int p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++){
                    ImCumDil[p] += ImDil[p];
                }
                
                // Store ImLabLargestRegions for final consolidation
                OSC_MEMCPY(ImLabsStored[k-firstImage], ImLabLargestRegions, IwProc*IhProc);
		    }
		    lock.unlock();

#ifdef OSC_CLEAN_DEBUG
            OSC_MEMCPY(ExtractSequence->ImLabOverSeq[k-firstImage]              , ImLabOverSeg              , IwProc*IhProc);
            OSC_MEMCPY(ExtractSequence->ImLabOverSeqAfterRdgThres[k-firstImage] , ImLabOverSeqAfterRdgThres , IwProc*IhProc);
            OSC_MEMCPY(ExtractSequence->ImLabLargestRegions[k-firstImage]       , ImLabLargestRegions       , IwProc*IhProc);
            OSC_MEMCPY(ExtractSequence->ImLabAfterClustering[k-firstImage]      , ExtractPicture->ImLabS    , IwProc*IhProc);
#endif 
            free(ImLabOverSeqAfterRdgThres); free(ImLabLargestRegions); free(ImLabAfterClustering);
            free(BufUC1); free(BufUC2); free(BufUC3);
            free(nbPixPerLabAfterRdgThres);
        }
    });
    
#ifdef OSC_TIMING
	t2 = omp_get_wtime();
#endif

    // Final consolidation
    int * nbPixPerRegion, *nbPixConveredPerRegion;
    unsigned char* regionsToKeep;
    OSC_CALLOC(nbPixPerRegion, OSC_CLEAN_NBREGIONS_BYLENGTH);
    OSC_CALLOC(nbPixConveredPerRegion, OSC_CLEAN_NBREGIONS_BYLENGTH);
    OSC_CALLOC(regionsToKeep, OSC_CLEAN_NBREGIONS_BYLENGTH);

    for (int k=firstImage; k<=lastImage; k++){
        COscExtractPicture *ExtractPicture = &(ExtractSequence->ExtractPictures[k]);
        short *ImLabCandidates = ImLabsStored[k-firstImage];

        OSC_MEMZERO(nbPixPerRegion, OSC_CLEAN_NBREGIONS_BYLENGTH);
        OSC_MEMZERO(nbPixConveredPerRegion, OSC_CLEAN_NBREGIONS_BYLENGTH);
        for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
            for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){
                if (ImLabCandidates[y*IwProc+x] > 1){
                    nbPixPerRegion[ImLabCandidates[y*IwProc+x]-2] ++;
                    nbPixConveredPerRegion[ImLabCandidates[y*IwProc+x]-2] += ImCumDil[y*IwProc+x];
                }
            }

        int nbPixelsSelectedRegion = 0, nbPixelsSelectedRegionCovered = 0;
        for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
            for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){
                if (ExtractPicture->ImLabS[y*IwProc+x] > 1){
                    nbPixelsSelectedRegion++;
                    nbPixelsSelectedRegionCovered += ImCumDil[y*IwProc+x];
                }
            }
            
        if (nbPixelsSelectedRegionCovered > OSC_CLEAN_REFERENCE_RATIO_COVERED * nbPixelsSelectedRegion * cardiacCycleLength){
            // Correct overlap (dilation enough to take into account the motion)

            OSC_MEMZERO(regionsToKeep, OSC_CLEAN_NBREGIONS_BYLENGTH);
            for (int lab=0; lab<OSC_CLEAN_NBREGIONS_BYLENGTH; lab++){
                if (nbPixConveredPerRegion[lab] > OSC_CLEAN_RATIO_COVERED_TO_REMOVE * nbPixPerRegion[lab] * cardiacCycleLength){
                    regionsToKeep[lab] = 1;
                }
            }

            for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
                for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){
                    if ( (ExtractPicture->ImLabS[y*IwProc+x] > 1) 
                        &&(ImLabCandidates[y*IwProc+x] > 1) && (regionsToKeep[ImLabCandidates[y*IwProc+x]-2] == 0) ){
                        ExtractPicture->ImLabS[y*IwProc+x] = OSC_MIN(ExtractPicture->ImLabS[y*IwProc+x],1);
                    }
                }
        }
    }
    free(nbPixPerRegion); free(nbPixConveredPerRegion); free(regionsToKeep);

#ifdef OSC_TIMING
	t3 = omp_get_wtime();
    printf("Cleaning: Consolidated threshold %.1f ms, images reprocessing %.1f ms, final consolidation %.1f ms\n", 1000.0 * (t1-t0), 1000.0 * (t2-t1), 1000.0 * (t3-t2));
#endif

    return ErrorCodeOut;
}

// *****************************************************************************
int OscClean_RemoveShortestLabels(short *ImLabIn, int Iw, int Ih, int *nbPixPerLab, int nbLabelsIn, int nbLabelsToKeep
                                  , short *ImLabOut, int *nbPixByLabelsOut){

    int *LabelsToKeep = (int*)malloc(nbLabelsToKeep*sizeof(int));
    int ind;

    OSC_MEMCPY(nbPixByLabelsOut, nbPixPerLab, nbLabelsIn);

    for (ind=0; ind<nbLabelsToKeep; ind++){
        int maxLengthHere = 0, indMaxLengthHere = -1;

        for (int j=0; j<nbLabelsIn; j++){
            if (nbPixByLabelsOut[j] > maxLengthHere){
                maxLengthHere = nbPixPerLab[j]; indMaxLengthHere = j;
            }
        }

        if (maxLengthHere > 0){
            LabelsToKeep[ind] = indMaxLengthHere;
            nbPixByLabelsOut[indMaxLengthHere] = 0;
        } else {
            break;
        }
    }
    
    OSC_MEMZERO(ImLabOut, Iw*Ih);
    for (int p=0; p<Iw*Ih; p++){
        if (ImLabIn[p] > 1){

            for (int i=0; i<ind; i++)
                if (ImLabIn[p]-2 == LabelsToKeep[i]){
                    ImLabOut[p] = i+2;
                }
        }
    }

    OSC_MEMZERO(nbPixByLabelsOut, nbLabelsIn);
    for (int i=0; i<ind; i++)
        nbPixByLabelsOut[i] = nbPixPerLab[LabelsToKeep[i]];

    free(LabelsToKeep);

    return ind;
}

// *****************************************************************************
int OscClean_ClusteringByDistance(short *ImLabIn, int Iw, int Ih, COscRoiCoord *RoiCoord, int *lengthRegionsIn, int nbLabsIn, float maxDistPix
                                  , unsigned char *BufMask, unsigned char *BufMask2, unsigned char *BufMask3, short *ImClustOut){

    int *clustAssociatedToLabels, *currentLabels, *clustAssociatedToLabels2, *lengthClusters;
    int nbCurrentLabels;

    OSC_CALLOC(clustAssociatedToLabels, nbLabsIn);
    OSC_CALLOC(currentLabels, nbLabsIn);

    bool ongoing = true;
    int currClust = 1;
    while (ongoing){
        // New largest label
        for (int i=0; i<nbLabsIn; i++){
            if (clustAssociatedToLabels[i] == 0){
                currentLabels[0] = i; 
                nbCurrentLabels = 1; 
                clustAssociatedToLabels[i] = currClust;
                break;
            }
        }

        // Iterate merging as long as new regions are included
        bool ongoingMerging = true;
        int limPix = (int)ceil(maxDistPix);
        float maxDistPix2 = maxDistPix*maxDistPix;

        while (ongoingMerging){
            ongoingMerging = false;

            // Computing Mask
            unsigned char *MaskIn = BufMask, *MaskDil = BufMask3;
            OSC_MEMZERO(MaskIn, Iw*Ih);
            for (int p=0; p<Iw*Ih; p++){
                if (ImLabIn[p] > 1){
                    for (int j=0; j<nbCurrentLabels; j++){
                        if (ImLabIn[p]-2 == currentLabels[j]){
                            MaskIn[p] = 1; continue;
                        }
                    }
                }
            }

            OscDilationRect(BufMask, Iw, Ih, RoiCoord, limPix, limPix, BufMask2, MaskDil);

            for (int p=0; p<Iw*Ih; p++){
                if (ImLabIn[p] > 1){
                    if (MaskIn[p] == 1){
                        MaskDil[p] = 0;  // No need to discuss labels already part of the 
                    }
                }
            }

            // Need to futher merge?
            bool alreadyInCluster;
            int p;
            for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
                for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++){
                    p = y*Iw+x;
                    if ((MaskDil[p] == 1) && (ImLabIn[p] > 1)){

                        alreadyInCluster = false;
                        for (int k=0; k<nbCurrentLabels; k++){
                            if (ImLabIn[p] -2 == currentLabels[k]){
                                alreadyInCluster = true;
                            }
                        }
                        if (alreadyInCluster)
                            continue;

                        for (int yy = OSC_MAX(RoiCoord->YMin,y-limPix); yy < OSC_MIN(RoiCoord->YMax,y+limPix); yy++){
                            bool merge = false;
                            for (int xx = OSC_MAX(RoiCoord->XMin,x-limPix); xx < OSC_MIN(RoiCoord->XMax,x+limPix); xx++){
                                if (MaskIn[yy*Iw+xx] && ((xx-x)*(xx-x) + (yy-y)*(yy-y) < maxDistPix2) ){

                                    // Merge new label
                                    int lab = ImLabIn[p]-2;

                                    if (clustAssociatedToLabels[lab] == 0){
                                        clustAssociatedToLabels[lab] = currClust;
                                        currentLabels[nbCurrentLabels] = lab;
                                        nbCurrentLabels ++;
                                        ongoingMerging = true;
                                        merge = true;
                                        break;
                                    } else {
                                        // Label was already clustered - include the corresponding cluster
                                        int oldClust = clustAssociatedToLabels[lab];
                                        if (oldClust  != currClust){
                                            for (int llab=0; llab<nbLabsIn; llab++){
                                                if (clustAssociatedToLabels[llab] == oldClust){
                                                    clustAssociatedToLabels[llab] = currClust;
                                                    currentLabels[nbCurrentLabels] = llab;
                                                    nbCurrentLabels ++;
                                                    ongoingMerging = true;
                                                    merge = true;
                                                }
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                            if (merge)
                                break;
                        }
                    }
                }
        }

        // Loop if there remain unclustered regions
        ongoing = false; currClust ++;
        for (int i=0; i<nbLabsIn; i++){
            if (clustAssociatedToLabels[i] == 0){
                ongoing = true; 
            }
        }
    }

    // Relabeling by length
    OSC_CALLOC(lengthClusters, currClust);
    if (lengthClusters == NULL){
        printf("OscClean_ClusteringByDistance: could not allocate lengthClusters\n");
		return -1;
    }

    for (int lab=0; lab <nbLabsIn; lab++){
        lengthClusters[ clustAssociatedToLabels[lab] ] += lengthRegionsIn[lab];
    }

    OSC_CALLOC(clustAssociatedToLabels2, nbLabsIn);
    if (clustAssociatedToLabels2 == NULL){
        printf("OscClean_ClusteringByDistance: could not allocate clustAssociatedToLabels2\n");
		return -1;
    }
    int currentClust = 1;

    for (int it=0; it<currClust; it++){

        int maxClustLength = 0;
        int currentLongestClust = -1;
        for (int j=0; j<currClust; j++){
            if (lengthClusters[j] > maxClustLength){
                maxClustLength = lengthClusters[j] ;
                currentLongestClust = j;
            }
        }

        if (currentLongestClust == -1){
            break;
        } else {
            for (int i=0; i<nbLabsIn; i++){
                if (clustAssociatedToLabels[i] == currentLongestClust){
                    clustAssociatedToLabels2[i] = currentClust;
                }
            }
            currentClust++;
            lengthClusters[currentLongestClust] = 0;
        }
    }

    OSC_MEMZERO(ImClustOut, Iw*Ih);
    for (int p=0; p<Iw*Ih; p++){
        if (ImLabIn[p] > 1){
            ImClustOut[p] = clustAssociatedToLabels2[ImLabIn[p]-2];
        }
    }

    int nbClustFinal = 0;
    for (int i=0; i<nbLabsIn; i++){
        nbClustFinal = OSC_MAX(nbClustFinal, clustAssociatedToLabels2[i]);
    }

    free(clustAssociatedToLabels); free(currentLabels); free(lengthClusters); free(clustAssociatedToLabels2);

    return nbClustFinal;
}

void OscClean_ComputeFinalMapFromClusters(short *ImClust, int Iw, int Ih, COscRoiCoord *RoiCoord, int nbClusters
                                          , float pixSize, float minDistBorderToMerge, float minLengthToMergeIfSameBorder
                                          , short *ImLabFinal){

    unsigned char* clustersToInclude;
    OSC_CALLOC(clustersToInclude, nbClusters);
    if (nbClusters > 0){clustersToInclude[0] = 1;}

    // Clusters contact with border
    unsigned char* contactWithBorder;
    short * minContact, *maxContact;
    OSC_CALLOC(contactWithBorder, 4*nbClusters);
    OSC_CALLOC(minContact, 4*nbClusters);
    OSC_CALLOC(maxContact, 4*nbClusters);

    for (int i=0; i<nbClusters; i++){
        minContact[4*i] = Iw; minContact[4*i+1] = Iw;
        minContact[4*i+2] = Ih; minContact[4*i+3] = Ih;
        maxContact[4*i] = 0; maxContact[4*i+1] = 0;
        maxContact[4*i+2] = 0; maxContact[4*i+3] = 0;
    }
    
    for (int y=RoiCoord->YMin; y<RoiCoord->YMin + OSC_CLEAN_MARGIN_CONTACT_BORDER_PIX; y++)
        for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
            if (ImClust[y*Iw+x] > 0){
                contactWithBorder[4*(ImClust[y*Iw+x]-1)] = 1;
                minContact[4*(ImClust[y*Iw+x]-1)]  = OSC_MIN(minContact[4*(ImClust[y*Iw+x]-1)], x);
                maxContact[4*(ImClust[y*Iw+x]-1)]  = OSC_MAX(maxContact[4*(ImClust[y*Iw+x]-1)], x);
            }
    for (int y=RoiCoord->YMax-OSC_CLEAN_MARGIN_CONTACT_BORDER_PIX+1; y<=RoiCoord->YMax; y++)
        for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
            if (ImClust[y*Iw+x] > 0){
                contactWithBorder[4*(ImClust[y*Iw+x]-1)+1] = 1;
                minContact[4*(ImClust[y*Iw+x]-1)+1]  = OSC_MIN(minContact[4*(ImClust[y*Iw+x]-1)+1], x);
                maxContact[4*(ImClust[y*Iw+x]-1)+1]  = OSC_MAX(maxContact[4*(ImClust[y*Iw+x]-1)+1], x);
            }
    for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
        for (int x=RoiCoord->XMin; x<RoiCoord->XMin + OSC_CLEAN_MARGIN_CONTACT_BORDER_PIX; x++)
            if (ImClust[y*Iw+x] > 0){
                contactWithBorder[4*(ImClust[y*Iw+x]-1)+2] = 1;
                minContact[4*(ImClust[y*Iw+x]-1)+2]  = OSC_MIN(minContact[4*(ImClust[y*Iw+x]-1)+2], y);
                maxContact[4*(ImClust[y*Iw+x]-1)+2]  = OSC_MAX(maxContact[4*(ImClust[y*Iw+x]-1)+2], y);
            }
    for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
        for (int x=RoiCoord->XMax-OSC_CLEAN_MARGIN_CONTACT_BORDER_PIX+1; x<=RoiCoord->XMax; x++)
            if (ImClust[y*Iw+x] > 0){
                contactWithBorder[4*(ImClust[y*Iw+x]-1)+3] = 1;
                minContact[4*(ImClust[y*Iw+x]-1)+3]  = OSC_MIN(minContact[4*(ImClust[y*Iw+x]-1)+3], y);
                maxContact[4*(ImClust[y*Iw+x]-1)+3]  = OSC_MAX(maxContact[4*(ImClust[y*Iw+x]-1)+3], y);
            }

    // Discussing the clusters touching the same border than the largest cluster (#1)
    for (int border = 0; border <4; border++){

        if (contactWithBorder[border] == 1){
            
            // The largest cluster touches that border
            for (int clust = 1; clust <nbClusters; clust++){
                if (contactWithBorder[4*clust+border] == 1){
                    // Touches same border

                    if (contactWithBorder[4*clust] + contactWithBorder[4*clust+1] + contactWithBorder[4*clust+2] + contactWithBorder[4*clust+3] > 1){
                        // Rouches at least another border
                        clustersToInclude[clust] = 2;
                    } else {

                        float dist = OSC_MIN( OSC_ABS(minContact[4*clust+border]-maxContact[border])
                                            , OSC_ABS(maxContact[4*clust+border]-minContact[border]) ) * pixSize;

                        if (dist < minDistBorderToMerge){

                            int xMin = Iw, xMax = 0, yMin = Ih, yMax = 0;

                            for (int y=RoiCoord->YMin; y<=RoiCoord->YMax; y++)
                                for (int x=RoiCoord->XMin; x<=RoiCoord->XMax; x++)
                                    if (ImClust[y*Iw+x] -1 == clust){
                                        xMin = OSC_MIN(xMin, x); xMax = OSC_MAX(xMax, x);
                                        yMin = OSC_MIN(yMin, y); yMax = OSC_MAX(yMax, y);
                                    }

                            if ( OSC_MAX(xMax-xMin+1,yMax-yMin+1) * pixSize > minLengthToMergeIfSameBorder)
                                clustersToInclude[clust] = 2;
                        }
                    }
                }
            }
        }
    }

    // Final result
    OSC_MEMZERO(ImLabFinal, Iw*Ih);
    for (int p=0; p<Iw*Ih; p++)
        if (ImClust[p] > 0){
            ImLabFinal[p] = 1+ clustersToInclude[ImClust[p]-1];
        }

    free(contactWithBorder); free(clustersToInclude); free(minContact); free(maxContact);
}
