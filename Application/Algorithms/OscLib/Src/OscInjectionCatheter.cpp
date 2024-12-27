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
#include "float.h"

//#define MXLABDISPLAY

#ifdef MXLABDISPLAY
    #include "MxLab.h"
#endif

// ************************************************************************
// Rescale the ridge image, call the CBDLib routine, and compute the output

int OscCatheterInjectionCatheterROIDetection(COscHdl OscHdl){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);
    int                 tInjBegin, tInjEnd, i, Err;

    Err = OscDetermineInflowBegin(OscHdl, &tInjBegin, &tInjEnd);
    if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscCatheterInjectionCatheterROIDetection", "OscDetermineInflowBegin", 0, Err);
		return Err;
	}
    ExtractProc->tInjBeginEstimated = tInjBegin;
    if (tInjEnd <= tInjBegin){
        ExtractSequence->nbSelectedCathInjROI = 0;
        return 0;
    }


    OscGetCandidateCathInjROIsFrameByFrame(OscHdl, tInjBegin);

    OscGetRepresentativeCathInjCandidatesROI(OscHdl, tInjBegin);

    OscComputeCumulRidgeTouchingROIs(OscHdl, tInjBegin, tInjEnd, ExtractProc->cumRdgRegions);

    Err = OscProcessCumRdgToSelectInjROI(OscHdl, tInjBegin, tInjEnd, ExtractProc->cumRdgRegions, ExtractProc->cumRdgRegionsFiltered
                                    , ExtractProc->BufStack1, ExtractProc->BufStack2, ExtractProc->selectedFinalROIs);
    if(Err)
	{
		ErrAppendErrorReportFromCaller(This->ErrHdl, "OscCatheterInjectionCatheterROIDetection", "OscProcessCumRdgToSelectInjROI", 0, Err);
		return Err;
	}

    ExtractSequence->nbSelectedCathInjROI = 0;
    for (i=0; i<ExtractProc->NbFinalCandidateROI; i++)
        if (ExtractProc->selectedFinalROIs[i]){
            ExtractSequence->SelectedCathInjROI[ExtractSequence->nbSelectedCathInjROI].XMin = ExtractProc->FinalROICandidates[i].XMin;
            ExtractSequence->SelectedCathInjROI[ExtractSequence->nbSelectedCathInjROI].XMax = ExtractProc->FinalROICandidates[i].XMax;
            ExtractSequence->SelectedCathInjROI[ExtractSequence->nbSelectedCathInjROI].YMin = ExtractProc->FinalROICandidates[i].YMin;
            ExtractSequence->SelectedCathInjROI[ExtractSequence->nbSelectedCathInjROI].YMax = ExtractProc->FinalROICandidates[i].YMax;
            ExtractSequence->nbSelectedCathInjROI++;
        }

#ifdef MXLABDISPLAY
     {
         int j;
         MxCommand("figure(1); subplot('Position',[0,0,0.5,1]); Show(ImFrg'); hold on;");
         for (j=0;j<ExtractProc->NbFinalCandidateROI; j++){
             MxPutInt(j+1, "j");
             MxPutInt(ExtractProc->FinalROICandidates[j].XMin, "xMin");
             MxPutInt(ExtractProc->FinalROICandidates[j].XMax, "xMax");
             MxPutInt(ExtractProc->FinalROICandidates[j].YMin, "yMin");
             MxPutInt(ExtractProc->FinalROICandidates[j].YMax, "yMax");
             if (ExtractProc->selectedFinalROIs[j] == 1)
                 MxCommand("plot([xMin, xMin, xMax, xMax, xMin],[yMin, yMax,yMax,yMin,yMin],'-','Color',cmap(j,:),'LineWidth',3);");
             else
                 MxCommand("plot([xMin, xMin, xMax, xMax, xMin],[yMin, yMax,yMax,yMin,yMin],'-','Color',cmap(j,:));");
         }
         MxCommand("saveas(1,['D://Temp//CathInjFromLib_',num2str(it,'%.2d'),'.png']); it = it+1;");
    }
#endif

	return 0;
}

// ************************************************************************
// *** Determine inflow begin

int OscDetermineInflowBegin(COscHdl OscHdl, int *tInjBegin, int *tInjEnd){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

    float               *Vect = ExtractProc->BufVectF1, *VectMedian = ExtractProc->BufVectF2
                        , *Buf1 = ExtractProc->BufStack1, *Buf2 = ExtractProc->BufStack2;
    unsigned char       *alreadyTaken = ExtractProc->BufVectUC;
    int                 *injBeginVect = ExtractProc->BufVectI1, *injEndVect = ExtractProc->BufVectI2;
    int                 frame, p, medianLength2, k, ind, bestInd, Err;
    int                 performFutureExtraction;
    float               bestDeltaCA;

    // Computing cumulated ridges
    memset(Vect, 0, (ExtractSequence->CardiacCycleEnd+1)*sizeof(float));
    for (frame=0; frame<=ExtractSequence->CardiacCycleEnd; frame++){
		COscExtractPicture  *ExtractPic = &This->ExtractSequence.ExtractPictures[frame];

        for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++)
            if (ExtractPic->ImLabS[p] > 1)
                Vect[frame] += ExtractPic->minFrg + (ExtractPic->maxFrg - ExtractPic->minFrg) * ExtractPic->ImFrgUC[p] / 255.f;
    }

    // Indicator filtering
    medianLength2 = (int)floor(0.5*OSC_CATHINJ_MEDIANSIZE_INJINDFILTERING);
    memcpy(VectMedian, Vect, (ExtractSequence->CardiacCycleEnd+1)*sizeof(float));
    for (frame = medianLength2; frame <=ExtractSequence->CardiacCycleEnd-medianLength2; frame++){
        for (k=0; k<2*medianLength2+1; k++)
            Buf1[k] = Vect[frame-medianLength2+k];
        Err = OscMedian(This, Buf1, Buf2, 0, medianLength2, 2*medianLength2, &(VectMedian[frame]));
        if(Err)
	    {
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "OscDetermineInflowBegin", "OscMedian", 0, Err);
		    return Err;
	    }
    }
    if (medianLength2 < ExtractSequence->CardiacCycleEnd){
        for (frame=0; frame<medianLength2; frame++)
            VectMedian[frame] = VectMedian[medianLength2];
        for (frame=ExtractSequence->CardiacCycleEnd-medianLength2-1; frame<=ExtractSequence->CardiacCycleEnd; frame++)
            VectMedian[frame] = VectMedian[ExtractSequence->CardiacCycleEnd-medianLength2];
    }

    // Extracting every possible injection interval
    memset(alreadyTaken, 0, (ExtractSequence->CardiacCycleEnd+1)*sizeof(unsigned char));
    ind = 0; performFutureExtraction = 0;
    while (performFutureExtraction == 0){
        performFutureExtraction = OscFindSlopeLimits(VectMedian, ExtractSequence->CardiacCycleEnd+1, alreadyTaken
                                                    , &(injBeginVect[ind]), &(injEndVect[ind]));
        if (performFutureExtraction == 0)
            ind++;
    }

    // Select the one leading to the most contrast intake
    bestInd = -1; bestDeltaCA = 0;
    for (k=0; k<ind; k++)
        if ( VectMedian[injEndVect[k]] - VectMedian[injBeginVect[k]] > bestDeltaCA){
            bestDeltaCA = VectMedian[injEndVect[k]] - VectMedian[injBeginVect[k]];
            bestInd = k;
        }

#ifdef MXLABDISPLAY
     {
        MxOpen(); MxSetVisible(1);
        MxPutVectorFloat(Vect, ExtractSequence->CardiacCycleEnd+1, "injInd");
        MxPutVectorFloat(VectMedian, ExtractSequence->CardiacCycleEnd+1, "injIndMed");
        MxPutInt((long)injBeginVect[bestInd]+1,"indBeginInj");
        MxPutInt((long)injEndVect[bestInd]+1,"indEndInj");
        MxCommand("figure(1); subplot(1,1,1); subplot('Position',[0.55,0.8,0.45,0.2]); plot(injInd,'k+-'); hold on; plot(injIndMed,'b+-'); plot([indBeginInj,indBeginInj],[min(injIndMed(:)),max(injIndMed(:))],'g-'); plot([indEndInj,indEndInj],[min(injIndMed(:)),max(injIndMed(:))],'g-'); hold off");
    }
#endif

    if (bestDeltaCA > 0){
        *tInjBegin = injBeginVect[bestInd]; *tInjEnd = injEndVect[bestInd];
    } else {
        *tInjBegin = 0; *tInjEnd = 0;
    }
    return 0;
}

int OscFindSlopeLimits(float *injInd, int In, unsigned char* alreadyTaken, int *injBegin, int *injEnd){

    int indiceCenter, nbOfSuccessiveJokersUsed, i;
    float bestSlope;

    // Find point of highest slope
    bestSlope = 0; indiceCenter = 0;
    for (i=1; i<In; i++){
        float slope = injInd[i]-injInd[i-1] ;
        if ((alreadyTaken[i] == 0) && (slope > bestSlope)){
            indiceCenter = i; bestSlope = slope;
        }
    }
    if (indiceCenter == 0)
        return -1;

    // Looking for the injection begin
    nbOfSuccessiveJokersUsed = 0;
    for (i=indiceCenter; i>0; i--){
        if (alreadyTaken[i] == 1){
            *injBegin = OSC_MIN(i+nbOfSuccessiveJokersUsed, indiceCenter); 
            break;
        }
        if (injInd[i]-injInd[i-1] <= OSC_CATHINJ_SLOPERATIOLIMIT * bestSlope){
            if (nbOfSuccessiveJokersUsed < OSC_CATHINJ_NBJOKERS_SEARCHINGFORINJBEGIN)
                nbOfSuccessiveJokersUsed++;
            else {
                *injBegin = OSC_MIN(i+nbOfSuccessiveJokersUsed, indiceCenter); 
                break;
            }
        } else
            nbOfSuccessiveJokersUsed = 0;
    }

    // Looking for the injection end
    nbOfSuccessiveJokersUsed = 0;
    for (i=indiceCenter; i<In; i++){
        if (alreadyTaken[i] == 1){
            *injEnd = OSC_MAX(indiceCenter,i-nbOfSuccessiveJokersUsed); 
            break;
        }
        if (injInd[i]-injInd[i-1] <= OSC_CATHINJ_SLOPERATIOLIMIT * bestSlope){
            if (nbOfSuccessiveJokersUsed < OSC_CATHINJ_NBJOKERS_SEARCHINGFORINJBEGIN)
                nbOfSuccessiveJokersUsed++;
            else {
                *injEnd = OSC_MAX(indiceCenter,i-nbOfSuccessiveJokersUsed);
                break;
            }
        } else
            nbOfSuccessiveJokersUsed = 0;
    }

    if (*injEnd < *injBegin)
        return -1;

    // Updating already taken
    for (i=*injBegin; i<=*injEnd; i++)
        alreadyTaken[i] = 1;

    return 0;
}

// ************************************************************************
void OscGetCandidateCathInjROIsFrameByFrame(COscHdl OscHdl, int tInjBegin){

	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    int                 frame, x, y, lab, lab2, IwProcAngio = ExtractProc->IwProcAngio, IhProcAngio = ExtractProc->IhProcAngio
                        , xMin = ExtractProc->ShuttersRoiSafe.XMin, xMax = ExtractProc->ShuttersRoiSafe.XMax
                        , yMin = ExtractProc->ShuttersRoiSafe.YMin, yMax = ExtractProc->ShuttersRoiSafe.YMax
                        , firstFrame, lastFrame, delta;
    unsigned char       goon, *labSelected = ExtractProc->BufsUC[0];
    short               **FrameROICoordinate = ExtractProc->BufsS;
    int                 *valid = ExtractProc->BufsI[0];
    float               *cumRdg = ExtractProc->BufsF[0];

    // * Initialization
    firstFrame = OSC_MAX(0,tInjBegin+1-OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION); lastFrame = tInjBegin;
    delta = OSC_MIN(OSC_CATHINJ_DELTAFROMINJDETECTION, firstFrame-1);
    firstFrame-= delta; lastFrame -= delta;

    // * Loop on frames
    for (frame = firstFrame; frame <=lastFrame; frame++){
        short* ImLab = This->ExtractSequence.ExtractPictures[frame].ImLabS;

        // * Init
        memset(labSelected, 0, IhProcAngio*IwProcAngio*sizeof(unsigned char));
        memset(cumRdg, 0, IhProcAngio*IwProcAngio*sizeof(float));
        for (lab=0; lab<IhProcAngio*IwProcAngio; lab++){
            FrameROICoordinate[0][lab] = xMax;
            FrameROICoordinate[1][lab] = xMin;
            FrameROICoordinate[2][lab] = yMax;
            FrameROICoordinate[3][lab] = yMin;
        }
        int maxNbLab = 0;
        float maxCumRdg = 0;

        // * Bounding box of each label, cumRdg, and identification of those close enough to the borders
        float coeffRdgConversion = (This->ExtractSequence.ExtractPictures[frame].maxFrg - This->ExtractSequence.ExtractPictures[frame].minFrg) / 255.f;
        float offsetConversion = This->ExtractSequence.ExtractPictures[frame].minFrg;
        for (y=yMin; y<=yMax; y++)
            for (x=xMin; x<=xMax; x++){
                if (ImLab[y*IwProcAngio+x] > 1){
                    lab = ImLab[y*IwProcAngio+x]-2;
                    maxNbLab = OSC_MAX(maxNbLab, lab);
                    cumRdg[lab] += offsetConversion+coeffRdgConversion*This->ExtractSequence.ExtractPictures[frame].ImFrgUC[y*IwProcAngio+x];

                    FrameROICoordinate[0][lab] = OSC_MIN(FrameROICoordinate[0][lab], x);
                    FrameROICoordinate[1][lab] = OSC_MAX(FrameROICoordinate[1][lab], x);
                    FrameROICoordinate[2][lab] = OSC_MIN(FrameROICoordinate[2][lab], y);
                    FrameROICoordinate[3][lab] = OSC_MAX(FrameROICoordinate[3][lab], y);

                    if ( (x<xMin + OSC_CATHINJ_MARGINTOINCLUDELAB) || (x>xMax - OSC_CATHINJ_MARGINTOINCLUDELAB) ||
                        (y<yMin + OSC_CATHINJ_MARGINTOINCLUDELAB) || (y>yMax - OSC_CATHINJ_MARGINTOINCLUDELAB) )
                        labSelected[lab] = 1;
                }
            }

        // Cleaning based on the angulation
        for (lab=0; lab<=maxNbLab; lab++)
            if (labSelected[lab]){

                int topLeft = FrameROICoordinate[1][lab], topRight = FrameROICoordinate[0][lab]
                    , rightBottom = FrameROICoordinate[3][lab], rightTop = FrameROICoordinate[2][lab]
                    , leftBottom = FrameROICoordinate[3][lab], leftTop = FrameROICoordinate[2][lab];

                y = FrameROICoordinate[2][lab];
                for (x = FrameROICoordinate[0][lab]; x <= FrameROICoordinate[1][lab]; x++)
                    if (ImLab[y*IwProcAngio+x] == lab+2){
                        topLeft = OSC_MIN(topLeft, x); topRight = OSC_MAX(topRight, x);
                    }

                x = FrameROICoordinate[0][lab];
                for (y = FrameROICoordinate[2][lab]; y <= FrameROICoordinate[3][lab]; y++)
                    if (ImLab[y*IwProcAngio+x] == lab+2){
                        leftBottom = OSC_MIN(leftBottom, y); leftTop = OSC_MAX(leftTop, y);
                    }

                x = FrameROICoordinate[1][lab];
                for (y = FrameROICoordinate[2][lab]; y <= FrameROICoordinate[3][lab]; y++)
                    if (ImLab[y*IwProcAngio+x] == lab+2){
                        rightBottom = OSC_MIN(rightBottom, y); rightTop = OSC_MAX(rightTop, y);
                    }

                if (OscCathInjROICompatibleWithAngulation(&ExtractProc->ShuttersRoiSafe, OSC_CATHINJ_MARGINTOINCLUDELAB
                                                        , FrameROICoordinate[0][lab], FrameROICoordinate[1][lab]
                                                        , FrameROICoordinate[2][lab], FrameROICoordinate[3][lab]
                                                        , topLeft, topRight, rightBottom, rightTop, leftBottom, leftTop
                                                        , ExtractProc->RotAngio, ExtractProc->AngAngio) == -1)
                    labSelected[lab] = 0;
            }

        // * Cleaning based on the surface
        memset(valid, 0, IhProcAngio*IwProcAngio*sizeof(int));
        for (lab=0; lab<=maxNbLab; lab++)
            if (labSelected[lab] == 1)
                maxCumRdg = OSC_MAX(maxCumRdg, cumRdg[lab]);

        for (lab=0; lab<=maxNbLab; lab++)
            if (cumRdg[lab] > OSC_CATHINJ_RATIOCUMRIDGECLEANING * maxCumRdg)
                valid[lab] = 1;
        //{
        //    int p;
        //    float *ImLabS = (float*)malloc(IwProcAngio*IhProcAngio*sizeof(float));
        //    for (p=0;p<IwProcAngio*IhProcAngio;p++){ ImLabS[p] = (float)ImLab[p];}
        //    MxPutMatrixFloat(ImLabS,IwProcAngio, IhProcAngio,"ImLab");
        //    MxPutMatrixFloat(ImRdg,IwProcAngio, IhProcAngio,"ImRdg");
        //    MxCommand("figure(5); subplot(1,2,2); Show(ImRdg');");
        //    MxCommand("figure(5); subplot(1,2,1); Show(ImLab'); hold on;");
        //    for (lab=0; lab<=maxNbLab; lab++){
        //        MxPutInt(FrameROICoordinate[0][lab], "xMin"); MxPutInt(FrameROICoordinate[1][lab], "xMax");
        //        MxPutInt(FrameROICoordinate[2][lab], "yMin"); MxPutInt(FrameROICoordinate[3][lab], "yMax");
        //        if ( (labSelected[lab]==1) && (valid[lab]==1) )
        //            MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'r-');");
        //        else
        //            MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'b-');");
        //    }
        //    MxPutInt(frame,"frame");
        //    MxCommand("saveas(5,['D://Temp//CathInjFromLib_',num2str(it,'%.2d'),'_FrameExt',num2str(frame,'%.2d'),'A.png']);");
        //    free(ImLabS);
        //}

        // * Extension/merging
        for (lab=0; lab<=maxNbLab; lab++)
            if ( (labSelected[lab] == 1) && (valid[lab] == 1)){

                goon = 1;
                while (goon){
                    goon = 0;
                    for (y=OSC_MAX(yMin, FrameROICoordinate[2][lab]-OSC_CATHINJ_MARGINTOEXTENDTOOTHERLAB);
                            y<=OSC_MIN(yMax, FrameROICoordinate[3][lab]+OSC_CATHINJ_MARGINTOEXTENDTOOTHERLAB); y++)
                        for (x=OSC_MAX(xMin, FrameROICoordinate[0][lab]-OSC_CATHINJ_MARGINTOEXTENDTOOTHERLAB);
                            x<=OSC_MIN(xMax, FrameROICoordinate[1][lab]+OSC_CATHINJ_MARGINTOEXTENDTOOTHERLAB); x++){
                            if ( (ImLab[y*IwProcAngio+x] > 1) && (ImLab[y*IwProcAngio+x] != lab+2) && (valid[ImLab[y*IwProcAngio+x]-2] == 1)){
                                lab2 = ImLab[y*IwProcAngio+x]-2;
                                valid[lab2] = 0;
                                cumRdg[lab] += cumRdg[lab2];

                                FrameROICoordinate[0][lab] = OSC_MIN(FrameROICoordinate[0][lab], FrameROICoordinate[0][lab2]);
                                FrameROICoordinate[1][lab] = OSC_MAX(FrameROICoordinate[1][lab], FrameROICoordinate[1][lab2]);
                                FrameROICoordinate[2][lab] = OSC_MIN(FrameROICoordinate[2][lab], FrameROICoordinate[2][lab2]);
                                FrameROICoordinate[3][lab] = OSC_MAX(FrameROICoordinate[3][lab], FrameROICoordinate[3][lab2]);

                                ImLab[y*IwProcAngio+x] = lab+2;
                                goon = 1;   // ROi has changed, we will loop again to see if further mergin is needed
                            }
                        }
                }
            }

        // * Conclusion
        int ind = 0;
        for (lab=0; lab<=maxNbLab; lab++)
            if ( (labSelected[lab] == 1) && (valid[lab] == 1) && (ind < OSC_CATHINJ_MAXNBOFCANDIDATEROI)){
                ExtractProc->CathInjROICandidatesPerFrame[frame-firstFrame][ind].XMin = FrameROICoordinate[0][lab];
                ExtractProc->CathInjROICandidatesPerFrame[frame-firstFrame][ind].XMax = FrameROICoordinate[1][lab];
                ExtractProc->CathInjROICandidatesPerFrame[frame-firstFrame][ind].YMin = FrameROICoordinate[2][lab];
                ExtractProc->CathInjROICandidatesPerFrame[frame-firstFrame][ind].YMax = FrameROICoordinate[3][lab];
                ExtractProc->CathInjROICandidatesPerFrameCumRdg[frame-firstFrame][ind] = cumRdg[lab];
                ind ++;
            }
        ExtractProc->nbROICandidatesPerFrame[frame-firstFrame] = ind;
    }
}

int OscCathInjROICompatibleWithAngulation(COscRoiCoord *ShutterPos, int marginShut, int xMinROI, int xMaxROI, int yMinROI, int yMaxROI
                                          , int topLeft, int topRight, int rightBottom, int rightTop
                                          , int leftBottom, int leftTop, float Rot, float Ang){

    int valid = -1;

    // * Angulation config 1
    if (Ang < -10){

        // If touches the left border low enough
        if ( (xMinROI < ShutterPos->XMin + marginShut) 
            && (leftTop < ShutterPos->YMin + 0.67f*(ShutterPos->YMax-ShutterPos->YMin) ) )
            valid = 1;

        // If touches the top border on the 2/3rd right
        if ( (yMinROI < ShutterPos->YMin + marginShut) 
            && (topRight < ShutterPos->XMin + 0.67f*(ShutterPos->XMax-ShutterPos->XMin) ) )
            valid = 1;
        
        // If touches the right border low enough
        if ( (xMaxROI > ShutterPos->XMax - marginShut) 
            && (rightTop < ShutterPos->YMin + 0.33f*(ShutterPos->YMax-ShutterPos->YMin) ) )
            valid = 1;
    } else {

        // * Angulation config 2
        if ( (Ang < 10) && (Rot > 20) ){

            // If touches the top border on the 2/3rd left
            if ( (yMinROI < ShutterPos->YMin + marginShut) 
                && (topRight > ShutterPos->XMin + 0.33f*(ShutterPos->XMax-ShutterPos->XMin) ) )
                valid = 1;

        } else { // * Angulation config 3

            // If touches the left border low enough
            if ( (xMinROI < ShutterPos->XMin + marginShut) 
                && (topLeft < ShutterPos->YMin + 0.67f*(ShutterPos->YMax-ShutterPos->YMin) ) )
                valid = 1;

            // If touches the top border on the 1/3rd right
            if ( (yMinROI < ShutterPos->YMin + marginShut) 
                && (topRight < ShutterPos->XMin + 0.33f*(ShutterPos->XMax-ShutterPos->XMin) ) )
                valid = 1;
        }
    }

    return valid;
}


// ************************************************************************
int OscGetRepresentativeCathInjCandidatesROI(COscHdl OscHdl, int tInjBegin){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    float               xs[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION*OSC_CATHINJ_MAXNBOFCANDIDATEROI], 
                        ys[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION*OSC_CATHINJ_MAXNBOFCANDIDATEROI],
                        ws[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION*OSC_CATHINJ_MAXNBOFCANDIDATEROI],
                        xBaryTmp[NB_EXTRACTED_CATHINJROI_MAX+1], yBaryTmp[NB_EXTRACTED_CATHINJROI_MAX+1], 
                        xPart[NB_EXTRACTED_CATHINJROI_MAX+1], yPart[NB_EXTRACTED_CATHINJROI_MAX+1];
    short               bufS[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION*OSC_CATHINJ_MAXNBOFCANDIDATEROI]
                        , labelParticle[OSC_CATHINJ_MAXNBOFCANDIDATEROI];
    int                 bufI[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION*OSC_CATHINJ_MAXNBOFCANDIDATEROI];
    int                 nbPart, ind, frame, i, nbROIIn, nbFrames, lab, lab2
                        , bestNumberOfValidatedClusters, validCluster[NB_EXTRACTED_CATHINJROI_MAX+1];
    float               minDistToMerge = OSC_CATHINJ_MINDISTTOMERGEROI_MM / ExtractProc->isoCenterPixSize
                        , bestCompromise, bestDist, dist, compromiseLab;
    
    COscRoiCoord        listROIs[OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION * OSC_CATHINJ_MAXNBOFCANDIDATEROI];
    COscRoiCoord        currentBestROIs[OSC_CATHINJ_MAXNBOFCANDIDATEROI+1];

    float coveredSurfaceRatio[NB_EXTRACTED_CATHINJROI_MAX+1], missedSurfaceRatio[NB_EXTRACTED_CATHINJROI_MAX+1];
    
#ifdef MXLABDISPLAY
    int bestNbParticles;
    float bestXs[NB_EXTRACTED_CATHINJROI_MAX+1],  bestYs[NB_EXTRACTED_CATHINJROI_MAX+1];
    float bestCoveredSurfaceRatio[NB_EXTRACTED_CATHINJROI_MAX+1], bestMissedSurfaceRatio[NB_EXTRACTED_CATHINJROI_MAX+1];
    short *bestLabels;
    bestNbParticles = 0;
#endif

    // * Computing particle representation
    ind = 0;
    nbFrames = tInjBegin-OSC_MAX(0,tInjBegin+1-OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION) +1;
    for (frame =0; frame < nbFrames; frame++){
        for (i=0; i<ExtractProc->nbROICandidatesPerFrame[frame]; i++){
                xs[ind] = 0.5f*(ExtractProc->CathInjROICandidatesPerFrame[frame][i].XMin+ExtractProc->CathInjROICandidatesPerFrame[frame][i].XMax);
                ys[ind] = 0.5f*(ExtractProc->CathInjROICandidatesPerFrame[frame][i].YMin+ExtractProc->CathInjROICandidatesPerFrame[frame][i].YMax);
                ws[ind] = ExtractProc->CathInjROICandidatesPerFrameCumRdg[frame][i];
                listROIs[ind] = ExtractProc->CathInjROICandidatesPerFrame[frame][i];
                ind++;
        }
    }
    nbROIIn = ind;

#ifdef MXLABDISPLAY
    bestLabels = (short*)malloc(nbROIIn*sizeof(short));
    MxPutMatrixUC(ExtractSequence->ExtractPictures[tInjBegin].ImFrgUC, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImFrg");
    MxCommand("figure(1); subplot('Position',[0.5,0.25,0.3,0.5]); Show(ImFrg'); hold on;");
    for (i=0; i<nbROIIn; i++){
        MxPutShort((short)listROIs[i].XMin,"xMin"); MxPutShort((short)listROIs[i].XMax,"xMax");
        MxPutShort((short)listROIs[i].YMin,"yMin"); MxPutShort((short)listROIs[i].YMax,"yMax");
        MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'w-')");
    }
#endif

    // * Testing different number of clusters
    int bestNbOfConsideredClusters = 0;
    bestNumberOfValidatedClusters = -1;
    for (nbPart = 1; nbPart<=NB_EXTRACTED_CATHINJROI_MAX; nbPart++){

        // Extracting best barycenters
        OscKMeansBarycenterBoxes(xs, ys, ws, nbROIIn, nbPart, OSC_CATHINJ_MEANSHIFT_NBINIT, OSC_CATHINJ_MEANSHIFT_NBITERATIONOPTIM
                              , xBaryTmp, yBaryTmp, bufS, bufI, xPart, yPart, labelParticle);
        
        // Limiting to the OSC_CATHINJ_MAXNBREGIONINLAB largest ROIs
        OscRemoveSmallestRegionslabel(listROIs, labelParticle, nbROIIn, nbPart, OSC_CATHINJ_MAXNBREGIONINLAB, bufI, bufS);

        // Best limits label per label
        int nbValidatedClusters = 0; 
        float compromisePart = 0;
        for (lab=0; lab<nbPart; lab ++){
            validCluster[lab] = OscCathInjProposeBestROIForLabel(lab,listROIs, labelParticle, nbROIIn, nbFrames, &ExtractProc->ShuttersRoiSafe
                                                                , &(currentBestROIs[lab]), &(coveredSurfaceRatio[lab]), &(missedSurfaceRatio[lab])
                                                                , &compromiseLab);

            if (validCluster[lab] == 1){ nbValidatedClusters++; compromisePart += compromiseLab; }
        }

        //{
        //    MxPutInt(nbPart,"nbPart");
        //    MxPutVectorShort(labelParticle, nbROIIn, "labPart");
        //    MxPutVectorFloat(xs, nbPart, "xs");
        //    MxPutVectorFloat(ys, nbPart, "ys");
        //    MxCommand("figure(2); subplot(1,1,1); Show(ImFrg'); hold on; cmap2 = lines(nbPart);");
        //    for (i=0; i<nbROIIn; i++){
        //        MxPutInt(i+1,"i");
        //        MxPutShort((short)listROIs[i].XMin,"xMin"); MxPutShort((short)listROIs[i].XMax,"xMax");
        //        MxPutShort((short)listROIs[i].YMin,"yMin"); MxPutShort((short)listROIs[i].YMax,"yMax");
        //        MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'Color',cmap2(labPart(i)+1,:),'LineWidth',2);");
        //    }
        //    for (lab=0; lab<nbPart; lab ++){
        //        MxPutInt(lab+1,"i");
        //        MxCommand("plot(xs(i), ys(i),'o','MarkerFaceColor',cmap2(i,:),'MarkerEdgeColor',cmap2(i,:),'MarkerSize',10);");
        //    }
        //    MxCommand("saveas(2,['D://Temp//CathInjFromLib_',num2str(it,'%.2d'),'_MeanShift',num2str(nbPart,'%.2d'),'.png']);");
        //    
        //    MxCommand("figure(3); subplot(1,1,1); Show(ImFrg'); hold on; cmap2 = lines(nbPart);");
        //    for (i=0; i<nbROIIn; i++){
        //        MxPutInt(i+1,"i");
        //        MxPutShort((short)listROIs[i].XMin,"xMin"); MxPutShort((short)listROIs[i].XMax,"xMax");
        //        MxPutShort((short)listROIs[i].YMin,"yMin"); MxPutShort((short)listROIs[i].YMax,"yMax");
        //        MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'Color',cmap2(labPart(i)+1,:));");
        //    }
        //    for (lab=0; lab<nbPart; lab ++){
        //        MxPutInt(lab+1,"i");
        //        MxPutShort((short)currentBestROIs[lab].XMin,"xMin"); MxPutShort((short)currentBestROIs[lab].XMax,"xMax");
        //        MxPutShort((short)currentBestROIs[lab].YMin,"yMin"); MxPutShort((short)currentBestROIs[lab].YMax,"yMax");
        //        if (validCluster[lab] == 1)
        //            MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'Color',cmap2(i,:),'LineWidth',4);");
        //        else
        //            MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'--','Color',cmap2(i,:),'LineWidth',4);");
        //        MxPutFloat(coveredSurfaceRatio[lab], "coveredSurfaceRatio");
        //        MxPutFloat(missedSurfaceRatio[lab], "missedSurfaceRatio");
        //        MxCommand("[Ih,Iw] = size(ImFrg); text(double(10), double(Ih-20*(i+1)), ['cov ', num2str(coveredSurfaceRatio,'%.2f'), ' miss ', num2str(missedSurfaceRatio,'%.2f')],'Color',cmap2(i,:));");
        //    }
        //    MxCommand("saveas(3,['D://Temp//CathInjFromLib_',num2str(it,'%.2d'),'_Limits',num2str(nbPart,'%.2d'),'.png']);");
        //    MxCommand("figure(1); subplot('Position',[0.5,0.25,0.3,0.5]); hold on");
        //}

        // Competition between the scenari at different number of particles
        if (nbValidatedClusters > bestNumberOfValidatedClusters){
            bestNumberOfValidatedClusters = nbValidatedClusters;
            bestCompromise = compromisePart;

            ExtractProc->NbFinalCandidateROI = 0;
            for (lab=0; lab<nbPart; lab ++)
                if (validCluster[lab] == 1){
                    // Merge with already existing label?
                    bestDist = FLT_MAX;
                    for (lab2=0;lab2<ExtractProc->NbFinalCandidateROI; lab2++){
                        if ( OSC_MAX(0, OSC_MIN(ExtractProc->FinalROICandidates[lab2].XMax, currentBestROIs[lab].XMax)
                                        -OSC_MAX(ExtractProc->FinalROICandidates[lab2].XMin, currentBestROIs[lab].XMin) )
                            * OSC_MAX(0, OSC_MIN(ExtractProc->FinalROICandidates[lab2].YMax, currentBestROIs[lab].YMax)
                                        -OSC_MAX(ExtractProc->FinalROICandidates[lab2].YMin, currentBestROIs[lab].YMin) ) > 0 ){
                            bestDist = -1.f; break;   // Both ROI intersect
                        } else {
                            dist = (float)OSC_MIN( OSC_MIN(OSC_ABS(ExtractProc->FinalROICandidates[lab2].XMin-currentBestROIs[lab].XMax)
                                                    ,OSC_ABS(currentBestROIs[lab].XMin-ExtractProc->FinalROICandidates[lab2].XMax)),
                                            OSC_MIN(OSC_ABS(ExtractProc->FinalROICandidates[lab2].YMin-currentBestROIs[lab].YMax)
                                                    ,OSC_ABS(currentBestROIs[lab].YMin-ExtractProc->FinalROICandidates[lab2].YMax)) );
                            if (dist < bestDist){ bestDist = dist; }
                        }
                    }

                    if (bestDist < minDistToMerge){
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMin 
                            = OSC_MIN(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMin, currentBestROIs[lab].XMin);
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMax 
                            = OSC_MAX(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMax, currentBestROIs[lab].XMax);
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMin 
                            = OSC_MIN(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMin, currentBestROIs[lab].YMin);
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMax 
                            = OSC_MAX(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMax, currentBestROIs[lab].YMax);
                    } else {
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMin = currentBestROIs[lab].XMin;
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMax = currentBestROIs[lab].XMax;
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMin = currentBestROIs[lab].YMin;
                        ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMax = currentBestROIs[lab].YMax;
                        ExtractProc->NbFinalCandidateROI++;
#ifdef MXLABDISPLAY
                        bestNbParticles = nbPart; memcpy(bestXs, xPart, nbPart*sizeof(float)); memcpy(bestYs, yPart, nbPart*sizeof(float));
                        memcpy(bestLabels, labelParticle, nbROIIn*sizeof(short));
                        memcpy(bestCoveredSurfaceRatio, coveredSurfaceRatio, nbPart*sizeof(float));
                        memcpy(bestMissedSurfaceRatio, missedSurfaceRatio, nbPart*sizeof(float));
#endif
                    }
                }
        } else {
            if ( (nbValidatedClusters == bestNumberOfValidatedClusters) && (compromisePart > bestCompromise)){
                // Same nb of validated clusters --> choose the config reaching the best cumulated compromise
                    
                bestCompromise = compromisePart;

                ExtractProc->NbFinalCandidateROI = 0;
                for (lab=0; lab<nbPart; lab ++)
                    if (validCluster[lab] == 1){
                        // Merge with already existing label?
                        bestDist = FLT_MAX;
                        for (lab2=0;lab2<ExtractProc->NbFinalCandidateROI; lab2++){
                            if ( OSC_MAX(0, OSC_MIN(ExtractProc->FinalROICandidates[lab2].XMax, currentBestROIs[lab].XMax)
                                            -OSC_MAX(ExtractProc->FinalROICandidates[lab2].XMin, currentBestROIs[lab].XMin) )
                                * OSC_MAX(0, OSC_MIN(ExtractProc->FinalROICandidates[lab2].YMax, currentBestROIs[lab].YMax)
                                            -OSC_MAX(ExtractProc->FinalROICandidates[lab2].YMin, currentBestROIs[lab].YMin) ) > 0 ){
                                bestDist = -1.f; break;   // Both ROI intersect
                            } else {
                                dist = (float)OSC_MIN( OSC_MIN(OSC_ABS(ExtractProc->FinalROICandidates[lab2].XMin-currentBestROIs[lab].XMax)
                                                            ,OSC_ABS(currentBestROIs[lab].XMin-ExtractProc->FinalROICandidates[lab2].XMax)),
                                                    OSC_MIN(OSC_ABS(ExtractProc->FinalROICandidates[lab2].YMin-currentBestROIs[lab].YMax)
                                                            ,OSC_ABS(currentBestROIs[lab].YMin-ExtractProc->FinalROICandidates[lab2].YMax)) );
                                if (dist < bestDist){ bestDist = dist; }
                            }
                        }

                        if (bestDist < minDistToMerge){
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMin 
                                = OSC_MIN(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMin, currentBestROIs[lab].XMin);
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMax 
                                = OSC_MAX(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMax, currentBestROIs[lab].XMax);
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMin 
                                = OSC_MIN(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMin, currentBestROIs[lab].YMin);
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMax 
                                = OSC_MAX(ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMax, currentBestROIs[lab].YMax);
                        } else {
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMin = currentBestROIs[lab].XMin;
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].XMax = currentBestROIs[lab].XMax;
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMin = currentBestROIs[lab].YMin;
                            ExtractProc->FinalROICandidates[ExtractProc->NbFinalCandidateROI].YMax = currentBestROIs[lab].YMax;
                            ExtractProc->NbFinalCandidateROI++;
#ifdef MXLABDISPLAY
                            bestNbParticles = nbPart; memcpy(bestXs, xPart, nbPart*sizeof(float)); memcpy(bestYs, yPart, nbPart*sizeof(float));
                            memcpy(bestLabels, labelParticle, nbROIIn*sizeof(short));
                            memcpy(bestCoveredSurfaceRatio, coveredSurfaceRatio, nbPart*sizeof(float));
                            memcpy(bestMissedSurfaceRatio, missedSurfaceRatio, nbPart*sizeof(float));
#endif
                        }
                    }
            }
        }
    }

#ifdef MXLABDISPLAY
    if (bestNbParticles > 0){
        MxPutInt(bestNbParticles,"nbPart");
        MxPutInt(bestNumberOfValidatedClusters,"nbValidated");
        MxCommand("cmap = lines(nbPart); figure(1); subplot('Position',[0.5,0.25,0.3,0.5]); hold on;");
        for (i=0; i<nbROIIn; i++){
            MxPutShort((short)listROIs[i].XMin,"xMin"); MxPutShort((short)listROIs[i].XMax,"xMax");
            MxPutShort((short)listROIs[i].YMin,"yMin"); MxPutShort((short)listROIs[i].YMax,"yMax");
            MxPutShort((short)bestLabels[i],"lab");
            MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'-','Color',cmap(lab+1,:))");
        }
        for (i=0; i<nbPart; i++){
            MxPutFloat(bestXs[i],"xPart"); MxPutFloat(bestYs[i],"yPart");
            MxPutShort((short)i,"lab");
            MxCommand("plot(xPart,yPart,'o','Color',cmap(lab+1,:))");

            MxPutFloat(bestCoveredSurfaceRatio[i], "coveredSurfaceRatio");
            MxPutFloat(bestMissedSurfaceRatio[i], "missedSurfaceRatio");
            MxCommand("[Ih,Iw] = size(ImFrg); text(double(10), double(Ih-20*(lab+1)), ['cov ', num2str(coveredSurfaceRatio,'%.1f'), ' miss ', num2str(missedSurfaceRatio,'%.2f')],'Color',cmap(lab+1,:));");
        }
        for (lab=0; lab<ExtractProc->NbFinalCandidateROI; lab++){
            MxPutShort((short)ExtractProc->FinalROICandidates[lab].XMin,"xMin"); MxPutShort((short)ExtractProc->FinalROICandidates[lab].XMax,"xMax");
            MxPutShort((short)ExtractProc->FinalROICandidates[lab].YMin,"yMin"); MxPutShort((short)ExtractProc->FinalROICandidates[lab].YMax,"yMax");
            MxPutShort((short)lab,"lab");
            MxCommand("plot([xMin,xMin,xMax,xMax,xMin],[yMin,yMax,yMax,yMin,yMin],'-','Color',cmap(lab+1,:),'LineWidth',3)");
        }
        MxCommand("[Ih,Iw] = size(ImFrg); text(0.6*Iw,Ih-20,['Nb Part ',num2str(nbPart,'%d'),' nb validated ',num2str(nbValidated,'%d')],'Color',[0,0,1])");
    }
    free(bestLabels);
#endif

    return 0;
}

// *** K-means over ROIs seen as particle weighting their cumulated ridgeness
int OscKMeansBarycenterBoxes(float *xs, float *ys, float *ws, int nbIn, int nbParticles, int nbInitsKMeans, int nbIterationOptimizations
                              , float *xBary, float *yBary, short *bufS, int *bufI, float *xOut, float *yOut, short *labelParticle){

    int itInit, i, ind, j, itOpt;
    float bestDistGlobal, bestDist, dist;
    int *nbPerBary;

    nbParticles = OSC_MIN(nbParticles, nbIn);

    bestDistGlobal = FLT_MAX;

    for (itInit = 0; itInit < nbInitsKMeans; itInit++){

        // Initialization
        short *alreadyTaken = bufS;
        memset(alreadyTaken, 0, nbIn*sizeof(short));
        for (i=0; i<nbParticles; i++){
            ind = rand() % nbParticles;
            if (alreadyTaken[ind] == 0){
                alreadyTaken[ind] = 1;
                xBary[i] = xs[ind];
                yBary[i] = ys[ind];
            }
        }

        // Optimization
        short *label = bufS;
        for (itOpt = 0; itOpt < nbIterationOptimizations; itOpt++){
            // Labelling
            for (j=0; j<nbIn; j++){
                bestDist = FLT_MAX;
                for (i=0; i<nbParticles; i++){
                    dist = (xs[j]-xBary[i])*(xs[j]-xBary[i]) + (ys[j]-yBary[i])*(ys[j]-yBary[i]);
                    if (dist < bestDist){
                        bestDist = dist; label[j] = i;
                    }
                }
            }

            // Barycenter
            nbPerBary = bufI;
            memset(nbPerBary, 0, nbParticles*sizeof(int));
            memset(xBary, 0, nbParticles*sizeof(float));
            memset(yBary, 0, nbParticles*sizeof(float));
            for (j=0; j<nbIn; j++){
                xBary[ label[j] ] += xs[j];
                yBary[ label[j] ] += ys[j];
                nbPerBary[ label[j] ] ++;
            }

            for (i=0; i<nbParticles; i++)
                if (nbPerBary[i] > 0){
                    xBary[i] /= nbPerBary[i]; yBary[i] /= nbPerBary[i];
                }
        }

        // Finalization
        dist = 0.f;
        for (j=0; j<nbIn; j++){
                dist +=     (xs[j]-xBary[label[j]])*(xs[j]-xBary[label[j]]) + (ys[j]-yBary[label[j]])*(ys[j]-yBary[label[j]]);
        }
        if (bestDistGlobal > dist){
            dist = bestDistGlobal;
            memcpy(xOut, xBary, nbParticles*sizeof(float));
            memcpy(yOut, yBary, nbParticles*sizeof(float));
            memcpy(labelParticle, label, nbIn*sizeof(short));
        }
    }
    return nbParticles;
}

// *** Keeping the maxNrRegions largest regions for each label
void OscRemoveSmallestRegionslabel(COscRoiCoord *listROIs, short *labelParticle, int nrROI, int nbLabs, int maxNrRegions
                                   , int *bufI, short *bufS){

    int i, it, lab, currentMaxSurf, currentBestIndice;
    int *surfRegion = bufI;
    short *alreadyTaken = bufS;

    for (i=0; i<nrROI; i++)
        surfRegion[i] = (listROIs[i].XMax-listROIs[i].XMin)*(listROIs[i].YMax-listROIs[i].YMin);

    for (lab=0; lab<nbLabs; lab++){

        int nbRegionLab = 0;
        for (i=0; i<nrROI; i++)
            if (labelParticle[i] == lab)
                nbRegionLab++;

        if (nbRegionLab > maxNrRegions){
            memset(alreadyTaken, 0, nbLabs*sizeof(short));

            for (it =0; it <maxNrRegions; it ++){
                currentMaxSurf = 0;

                for (i=0; i<nrROI; i++)
                    if ( (labelParticle[i] == lab) && (surfRegion[i] >= currentMaxSurf) && (alreadyTaken[i] == 0) ){
                        currentMaxSurf = surfRegion[i] ; currentBestIndice = i;
                    }
                alreadyTaken[currentBestIndice] = 1;
            }

            for (i=0; i<nrROI; i++)
                if ( (labelParticle[i] == lab) && (alreadyTaken[i] == 0) )
                    labelParticle[i] = -1;
        }
    }
}

// *** Propose an "optimal" ROI to represent the label
// Visit each corner of the labelled ROI, and try to reach a sufficient coverage (surface of the labelled ROI in the represent)
// and an sufficiently small ratio of misses. If both conditions are met, the "best" representative ROI reachs the highest score
// in a compromise coverage - alpha x misses

int OscCathInjProposeBestROIForLabel(int lab, COscRoiCoord * listROIs, short *labelParticle, int nbROIIn, int nbFrames, COscRoiCoord *ShutterCoord
                                     , COscRoiCoord * bestROIsLab, float *bestSurfaceCoveredRatioOut, float *bestMissedSurfaceRatioOut
                                     , float *bestCompromiseOut){

    int surfTested, surfCovered, surfMissed, surfROI, totalSurfaceROIs, totalMissedSurface
        , i1, i2, i3, i4, i, xMinTested, xMaxTested, yMinTested, yMaxTested, valid;
    float globalSurfaceCoveredRatio, globalMissedSurfaceRatio, bestCoverageMissedCompromise;

    bestCoverageMissedCompromise = -FLT_MAX;
    valid = 0;

    // Testing all possible borders
    for (i1=0; i1<nbROIIn; i1 ++)
        if (labelParticle[i1] == lab){
            xMinTested = listROIs[i1].XMin;

            for (i2=0; i2<nbROIIn; i2 ++)
                if (labelParticle[i2] == lab){
                    xMaxTested = listROIs[i2].XMax;

                            
                    for (i3=0; i3<nbROIIn; i3 ++)
                        if (labelParticle[i3] == lab){
                            yMinTested = listROIs[i3].YMin;

                            for (i4=0; i4<nbROIIn; i4 ++)
                                if (labelParticle[i4] == lab){
                                    yMaxTested = listROIs[i4].YMax;

                                    surfTested = OSC_MAX(0, xMaxTested-xMinTested) * OSC_MAX(0, yMaxTested-yMinTested) * nbFrames;
                                    globalSurfaceCoveredRatio = 0;
                                    totalMissedSurface = 0;
                                    totalSurfaceROIs = 0;

                                    if ( (surfTested > 0) 
                                        && (xMaxTested-xMinTested < OSC_CATHINJ_MAXRATIODIMROIVSSHUT * (ShutterCoord->XMax-ShutterCoord->XMin) ) 
                                        && (yMaxTested-yMinTested < OSC_CATHINJ_MAXRATIODIMROIVSSHUT * (ShutterCoord->YMax-ShutterCoord->YMin) ) ){
                                        // Limitation of the ROI extension

                                        for (i=0; i<nbROIIn; i ++)
                                            if (labelParticle[i] == lab){
                                                surfCovered = (OSC_MAX(0, OSC_MIN(xMaxTested, listROIs[i].XMax)
                                                                  -OSC_MAX(xMinTested, listROIs[i].XMin) )
                                                       * OSC_MAX(0, OSC_MIN(yMaxTested, listROIs[i].YMax)
                                                                  -OSC_MAX(yMinTested, listROIs[i].YMin) ) );
                                                surfROI = OSC_MAX(0,listROIs[i].XMax-listROIs[i].XMin)
                                                            * OSC_MAX(0,listROIs[i].YMax-listROIs[i].YMin);
                                                surfMissed = surfROI-surfCovered;

                                                globalSurfaceCoveredRatio += surfCovered/ (float)surfTested;
                                                totalSurfaceROIs += surfROI;
                                                totalMissedSurface += surfMissed;
                                            }
                                    }
                                    globalMissedSurfaceRatio = totalMissedSurface / (float)totalSurfaceROIs;

                                    // Keep best compromise
                                    if (valid == 1){
                                        // Goal number 1: be validated, ie pass two thresholds, one on the coverage, the other on the misses
                                        // Goal number 2: best score coverage - alpha x misses

                                        if ( (globalSurfaceCoveredRatio >= OSC_CATHINJ_MINRATIOCOVERAGE) && (globalMissedSurfaceRatio <= OSC_CATHINJ_MAXMISSED)
                                            && (globalSurfaceCoveredRatio - OSC_WEIGHT_COVERAGEVSMISSED * globalMissedSurfaceRatio > bestCoverageMissedCompromise) ){
                                            bestCoverageMissedCompromise = globalSurfaceCoveredRatio 
                                                                        - OSC_WEIGHT_COVERAGEVSMISSED * globalMissedSurfaceRatio;

                                            bestROIsLab->XMin = xMinTested; bestROIsLab->XMax = xMaxTested; 
                                            bestROIsLab->YMin = yMinTested; bestROIsLab->YMax = yMaxTested;
                                            *bestSurfaceCoveredRatioOut = globalSurfaceCoveredRatio;
                                            *bestMissedSurfaceRatioOut = globalMissedSurfaceRatio;
                                            *bestCompromiseOut = bestCoverageMissedCompromise;
                                        }

                                    } else {
                                        if ( (globalSurfaceCoveredRatio >= OSC_CATHINJ_MINRATIOCOVERAGE) && (globalMissedSurfaceRatio <= OSC_CATHINJ_MAXMISSED) ){
                                            valid = 1;
                                            bestCoverageMissedCompromise = globalSurfaceCoveredRatio - OSC_WEIGHT_COVERAGEVSMISSED * globalMissedSurfaceRatio;
                                            bestROIsLab->XMin = xMinTested; bestROIsLab->XMax = xMaxTested; 
                                            bestROIsLab->YMin = yMinTested; bestROIsLab->YMax = yMaxTested;
                                            *bestSurfaceCoveredRatioOut = globalSurfaceCoveredRatio;
                                            *bestMissedSurfaceRatioOut = globalMissedSurfaceRatio;
                                            *bestCompromiseOut = bestCoverageMissedCompromise;
                                        } else {
                                            if (globalSurfaceCoveredRatio - OSC_WEIGHT_COVERAGEVSMISSED * globalMissedSurfaceRatio > bestCoverageMissedCompromise){
                                                bestCoverageMissedCompromise = globalSurfaceCoveredRatio - OSC_WEIGHT_COVERAGEVSMISSED * globalMissedSurfaceRatio;
                                                bestROIsLab->XMin = xMinTested; bestROIsLab->XMax = xMaxTested; 
                                                bestROIsLab->YMin = yMinTested; bestROIsLab->YMax = yMaxTested;
                                                *bestSurfaceCoveredRatioOut = globalSurfaceCoveredRatio;
                                                *bestMissedSurfaceRatioOut = globalMissedSurfaceRatio;
                                                *bestCompromiseOut = bestCoverageMissedCompromise;
                                            }
                                        }
                                    }
                                }
                        }
                }
        }

    return valid;
}

// ************************************************************************
// ************************************************************************

void OscComputeCumulRidgeTouchingROIs(COscHdl OscHdl, int tInjBegin, int tInjEnd, float ** cumRdgs){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);
	COscExtractSequence *ExtractSequence = &(This->ExtractSequence);

    int firstFrame = OSC_MAX(0,tInjBegin+1-OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION), frame, nbCandidateROI = ExtractProc->NbFinalCandidateROI, j, x, y
        , IwProcAngio = ExtractProc->IwProcAngio, lab, xMinShut = ExtractProc->ShuttersRoi.XMin, xMaxShut = ExtractProc->ShuttersRoi.XMax
        , yMinShut = ExtractProc->ShuttersRoi.YMin, yMaxShut = ExtractProc->ShuttersRoi.YMax;
    float maxFirstRdgTouchingLab;

    float           *cumRdgLab = ExtractProc->BufsF[0];
    unsigned char   *alreadyTaken = ExtractProc->BufsUC[0];

#ifdef MXLABDISPLAY
    {
        int firstFrameHere = OSC_MAX(0,tInjBegin-2), lastFrameHere = OSC_MIN(firstFrameHere+5, tInjEnd);
        int p, nbIm2 = (int)ceil(0.5f*(lastFrameHere-firstFrameHere+1));
        float height = 0.5f/nbIm2-0.01f;
        float * ImLabF = (float*)malloc(ExtractProc->IwProcAngio*ExtractProc->IhProcAngio*sizeof(float));
        
        for (frame = firstFrameHere; frame <= lastFrameHere; frame ++){
            for (p=0; p<ExtractProc->IwProcAngio*ExtractProc->IhProcAngio; p++){ ImLabF[p] = (float)ExtractSequence->ExtractPictures[frame].ImLabS[p];}
            MxPutMatrixFloat(ImLabF, ExtractProc->IwProcAngio, ExtractProc->IhProcAngio, "ImLab");
            MxPutFloat(height,"height");
            if (frame-firstFrameHere < nbIm2){
                MxPutFloat(0.75f-(frame-firstFrameHere+1)*height, "offsetY");
                MxCommand("figure(1); subplot('Position',[0.8,offsetY, 0.095, height]); Show(ImLab');");
            } else {
                MxPutFloat(0.75f-(frame-firstFrameHere-nbIm2+1)*height, "offsetY");
                MxCommand("figure(1); subplot('Position',[0.9,offsetY, 0.095, height]); Show(ImLab');");
            }
        }
        free(ImLabF);
    }
#endif
   
    for (frame = firstFrame; frame <= tInjEnd; frame ++){
        short *ImLab = ExtractSequence->ExtractPictures[frame].ImLabS;
        float coeffRdgConversion = (This->ExtractSequence.ExtractPictures[frame].maxFrg - This->ExtractSequence.ExtractPictures[frame].minFrg) / 255.f;
        float offsetConversion = This->ExtractSequence.ExtractPictures[frame].minFrg;

        // Precompute cumulated ridges
        int nbLabs = 0;
        memset(cumRdgLab, 0, ExtractProc->IwProcAngio*ExtractProc->IhProcAngio);
        for (y = yMinShut; y<= yMaxShut; y++)
            for (x = xMinShut; x<= xMaxShut; x++)
                if (ImLab[y*IwProcAngio+x] > 1){
                    lab = ImLab[y*IwProcAngio+x]-2;
                    nbLabs = OSC_MAX(nbLabs, lab+1);
                    cumRdgLab[lab] += offsetConversion + coeffRdgConversion *This->ExtractSequence.ExtractPictures[frame].ImFrgUC[y*IwProcAngio+x];
                }

        // Loop on candidates ROI
        for (j=0; j<nbCandidateROI; j++){
            
            memset(alreadyTaken, 0, nbLabs*sizeof(unsigned char));
            cumRdgs[j][frame-firstFrame] = 0;

            // For each newly discovered label in the ROI, add the corresponding cumulated ridges
            for (y = ExtractProc->FinalROICandidates[j].YMin; y<= ExtractProc->FinalROICandidates[j].YMax; y++)
                for (x = ExtractProc->FinalROICandidates[j].XMin; x<= ExtractProc->FinalROICandidates[j].XMax; x++)
                    if ( (ImLab[y*IwProcAngio+x] > 1) && (alreadyTaken[ImLab[y*IwProcAngio+x]-2] == 0) ){
                        alreadyTaken[ImLab[y*IwProcAngio+x]-2] = 1;
                        cumRdgs[j][frame-firstFrame] += cumRdgLab[ImLab[y*IwProcAngio+x]-2];
                    }
        }

        //{
        //    int p;
        //    float *ImLabF = (float*)malloc(IwProcAngio*ExtractProc->IhProcAngio*sizeof(float));
        //    for (p=0; p<IwProcAngio*ExtractProc->IhProcAngio; p++){ ImLabF[p] = (float)ImLab[p];}
        //    MxPutMatrixFloat(ImLabF, IwProcAngio, ExtractProc->IhProcAngio, "ImLab");
        //    MxPutMatrixFloat(ImRdg, IwProcAngio, ExtractProc->IhProcAngio, "ImFrg");
        //    MxCommand("figure(3); subplot(1,1,1); subplot('Position',[0,0.33,0.5,0.67]); Show(ImLab');");
        //    MxCommand("figure(3); subplot('Position',[0.5,0.33,0.5,0.67]); Show(ImFrg');");
        //    MxPutInt(nbCandidateROI,"nbROI");
        //    MxCommand("cmap3 = lines(nbROI);");
        //    for (j=0; j<nbCandidateROI; j++){
        //        MxPutInt(j+1,"j");
        //        MxPutFloat(ExtractProc->FinalROICandidates[j].XMin, "xMin"); MxPutFloat(ExtractProc->FinalROICandidates[j].XMax, "xMax");
        //        MxPutFloat(ExtractProc->FinalROICandidates[j].YMin, "yMin"); MxPutFloat(ExtractProc->FinalROICandidates[j].YMax, "yMax");
        //        MxPutVectorFloat(cumRdgs[j], frame-firstFrame+1, "cumRdg");
        //        MxCommand("figure(3); subplot('Position',[0,0.33,0.5,0.67]); hold on; plot([xMin, xMin, xMax, xMax, xMin],[yMin, yMax, yMax, yMin, yMin], '-','Color',cmap3(j,:),'LineWidth',4); hold off;");
        //        MxCommand("figure(3); subplot('Position',[0.5,0.33,0.5,0.67]); hold on; plot([xMin, xMin, xMax, xMax, xMin],[yMin, yMax, yMax, yMin, yMin], '-','Color',cmap3(j,:),'LineWidth',4); hold off;");
        //        MxCommand("figure(3); subplot('Position',[0,0.,1,0.33]); hold on; plot(cumRdg, 'x-', 'Color',cmap3(j,:));");
        //    }
        //    free(ImLabF);
        //    MxPutInt(frame,"frame");
        //    MxCommand("saveas(3,['D://Temp//CathInjFromLib_',num2str(it,'%.2d'),'_CumRdgComp',num2str(frame,'%.2d'),'.png']);");
        //}

    }

    // Normalization
    maxFirstRdgTouchingLab = 0.f;
    for (j=0; j<nbCandidateROI; j++)
        maxFirstRdgTouchingLab = OSC_MAX(maxFirstRdgTouchingLab, cumRdgs[j][0]);
    for (j=0; j<nbCandidateROI; j++){
        float firstVal = cumRdgs[j][0];
        for (frame = firstFrame; frame <= tInjEnd; frame ++)
            cumRdgs[j][frame-firstFrame] = (cumRdgs[j][frame-firstFrame] - firstVal) / maxFirstRdgTouchingLab;
    }
}

// ************************************************************************
// ************************************************************************

int OscProcessCumRdgToSelectInjROI(COscHdl OscHdl, int tInjBegin, int tInjEnd, float ** cumRdgs, float **cumRdgsFiltered
                                    , float *Buf1, float *Buf2, unsigned char *selectedROIsOut){
    
	COsc                *This = (COsc*)OscHdl.Pv;
	COscExtractProc     *ExtractProc = &(This->ExtractProc);

    float               scoreROI[NB_EXTRACTED_CATHINJROI_MAX+1];
    int                 tIntakeROI[NB_EXTRACTED_CATHINJROI_MAX+1];

    int                 i, j, medianLength2, frame, k;
    int                 nbFrames =tInjEnd-OSC_MAX(0,tInjBegin+1-OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION)+1;
    int                 nbCandidateROI = ExtractProc->NbFinalCandidateROI, Err;
    float               maxDeltaIntakeROI[NB_EXTRACTED_CATHINJROI_MAX+1], maxCumRdg, tresholdInjectionSure, tIntake, maxScore;

#ifdef MXLABDISPLAY
    MxPutInt(nbCandidateROI,"nbPart");
    MxCommand("cmap = lines(nbPart);");
#endif

    // * Filtering of the CumRdgs
    medianLength2 = (int)floor(0.5*OSC_CATHINJROI_MEDIANSIZE_CUMRDG);
    maxCumRdg = 0;
    for (j=0; j<nbCandidateROI; j++){
        memcpy(cumRdgsFiltered[j],cumRdgs[j],nbFrames*sizeof(float));
        for (frame = medianLength2; frame <=nbFrames-medianLength2; frame++){
            for (k=0; k<2*medianLength2+1; k++)
                Buf1[k] = cumRdgs[j][frame-medianLength2+k];
            Err =  OscMedian(This, Buf1, Buf2, 0, medianLength2, 2*medianLength2, &(cumRdgsFiltered[j][frame]));
            if(Err)
	        {
		        ErrAppendErrorReportFromCaller(This->ErrHdl, "OscProcessCumRdgToSelectInjROI", "OscMedian", 0, Err);
		        return Err;
	        }
        }
        for (frame = 0; frame <=nbFrames; frame++)
            maxCumRdg = OSC_MAX(maxCumRdg, cumRdgs[j][frame]);

#ifdef MXLABDISPLAY
    MxPutInt(j+1,"j");
    MxPutVectorFloat(cumRdgs[j], nbFrames+1, "cumRdg");
    MxPutVectorFloat(cumRdgsFiltered[j], nbFrames+1, "cumRdgsFiltered");
    MxCommand("figure(1); subplot('Position',[0.55,0.05,0.4,0.2]); hold on; plot(cumRdg,':','Color', cmap(j,:)); plot(cumRdgsFiltered,'+-','Color', cmap(j,:)); hold off;");
#endif
    }

    // * Determining intake for each ROI
    tresholdInjectionSure = OSC_MAX(OSC_CATHINJ_THRESHOLDINJSURE, OSC_CATHINJ_RATIOMAXTHRESHOLDINJSURE*maxCumRdg);
    tIntake = (float)nbFrames;
    for (j=0; j<nbCandidateROI; j++){

        // Time where we are sure the injection has began
        int t = nbFrames+1;
        for (i=0; i<=nbFrames; i++)
            if (cumRdgsFiltered[j][i] > tresholdInjectionSure){
                t = i; break;
            }

        if ( (t > 0) && (t <= nbFrames) ){
            maxDeltaIntakeROI[j] = OSC_MAX(0,cumRdgsFiltered[j][t]-cumRdgsFiltered[j][t-1]);

            // It is progressively decreased until it is under a threshold, and the slope is close to zero (on filtered indicator)
            while ( (t>0) && ( (cumRdgsFiltered[j][t] > OSC_CATHINJ_THRESHOLDINJBEGIN) 
                                || (cumRdgsFiltered[j][t]-cumRdgsFiltered[j][t-1] > OSC_CATHINJ_RATIOSLOPEINTAKE * maxDeltaIntakeROI[j]) ) ){
                maxDeltaIntakeROI[j] = OSC_MAX(maxDeltaIntakeROI[j], cumRdgsFiltered[j][t]-cumRdgsFiltered[j][t-1]);
                t--;
            }

            // Affining (on the non-filtered indicator)
            t = OSC_MIN(t+medianLength2, nbFrames);
            while ( (t>0) && ( (cumRdgs[j][t] > OSC_CATHINJ_THRESHOLDINJBEGIN) 
                                || (cumRdgs[j][t]-cumRdgs[j][t-1] > OSC_CATHINJ_RATIOSLOPEINTAKE * maxDeltaIntakeROI[j]) ) ){
                t--;
            }

            tIntakeROI[j] = t; 
        } else
            tIntakeROI[j] = nbFrames;

        tIntake = OSC_MIN(tIntakeROI[j], tIntake);
    }
    ExtractProc->tInjBeginEstimated = (int)(tIntake + OSC_MAX(0,tInjBegin+1-OSC_CATHINJ_NBFRAMESFORDISCUSSIONINJECTION));

#ifdef MXLABDISPLAY
    MxPutInt((int)tIntake+1,"tIntake"); MxPutFloat(maxCumRdg,"maxCumRdg");
    MxCommand("figure(1); subplot('Position',[0.55,0.05,0.4,0.2]); hold on; plot([tIntake,tIntake], [0,maxCumRdg],'k-'); hold off;");
    for (j=0; j<nbCandidateROI; j++){
        MxPutInt(j+1,"j");
        MxPutInt(tIntakeROI[j]+1,"tIntakeROI");
        MxPutVectorFloat(cumRdgsFiltered[j], nbFrames+1, "cumRdgsFiltered");
        MxCommand("figure(1); subplot('Position',[0.55,0.05,0.4,0.2]); hold on; plot(tIntakeROI, cumRdgsFiltered(tIntakeROI),'o','Color', cmap(j,:)); hold off;");
    }
#endif

    if (tIntake<nbFrames){

        memset(scoreROI, 0, nbCandidateROI*sizeof(float));
        float maxDeltaIntake = 0.f;

        for (j=0; j<nbCandidateROI; j++){

            // Earliest intake rewarded (with a tolerance)
            if (tIntakeROI[j] <= tIntake + OSC_CATHINJ_TOLERANCEFORFIRSTINTAKE){
                scoreROI[j] += OSC_CATHINJ_PTSFORFIRSTINTAKE;
                maxDeltaIntake = OSC_MAX(maxDeltaIntakeROI[j], maxDeltaIntake);
            }
        }

        
        for (j=0; j<nbCandidateROI; j++){

            // Initial jump - detriemental
            if (scoreROI[j] == 0){
                // Only for not early intake
                for (frame = (int)tIntake; frame <= OSC_MIN((int)tIntake + OSC_CATHINJ_NBFRAMESTOSTUDYAFTERINI, nbFrames-1); frame++){
                    if (cumRdgs[j][frame+1]-cumRdgs[j][frame] > OSC_CATHINJ_FACTORFORJUMPINTAKE * maxDeltaIntake)
                        scoreROI[j] += OSC_CATHINJ_PTSFORJUMPATTHEBEGINING;
                    if (cumRdgs[j][frame+1] > tresholdInjectionSure)
                        break;
                }
            }

            // Posterior jumps - detriemental
            unsigned char afterInj = 0;
            for (frame = (int)tIntake; frame <= OSC_MIN((int)tIntake + OSC_CATHINJ_NBFRAMESTOSTUDYAFTERINI, nbFrames-1); frame++){
                if (cumRdgs[j][frame+1] > tresholdInjectionSure)
                    afterInj = 1;
                if ( (afterInj == 1) && cumRdgs[j][frame+1] < OSC_CATHINJ_RATIOFORJUMPAFTERINTAKE *cumRdgs[j][frame] )
                    scoreROI[j] += OSC_CATHINJ_PTSFORJUMPAFTERINTAKE;
            }
        }

        // Assess the dominant track
        for (frame = (int)tIntake; frame <= OSC_MIN((int)tIntake + OSC_CATHINJ_NBFRAMESTOSTUDYAFTERINI, nbFrames); frame++){
            
            maxCumRdg = 0;
            for (j=0; j<nbCandidateROI; j++)
                maxCumRdg = OSC_MAX(maxCumRdg, cumRdgs[j][frame]);

            for (j=0; j<nbCandidateROI; j++)
                if (cumRdgs[j][frame] > maxCumRdg-OSC_CATHINJ_DELTARDGFOREQUIVALENTSCORE)
                    scoreROI[j] += OSC_CATHINJ_PTSFORMOSTINJATTIMET;
        }
    }

    memset(selectedROIsOut, 0, nbCandidateROI*sizeof(unsigned char));
    maxScore = 0;
    for (j=0; j<nbCandidateROI; j++)
        maxScore = OSC_MAX(maxScore, scoreROI[j]);
    for (j=0; j<nbCandidateROI; j++)
        if (scoreROI[j] >= maxScore - OSC_CATHINJ_DELTASCORETOMAXFORSELECTION)
            selectedROIsOut[j] = 1;

#ifdef MXLABDISPLAY
    for (j=0; j<nbCandidateROI; j++){
        MxPutInt(j+1,"j");
        MxPutFloat(scoreROI[j],"scoreROI");
        MxCommand("figure(1); subplot('Position',[0.55,0.05,0.4,0.2]); hold on; text(1.1, double((1-0.15*double(j))*maxCumRdg), ['Score ', num2str(scoreROI,'%d')], 'Color', cmap(j,:)); hold off;");
    }
#endif
    return 0;
}