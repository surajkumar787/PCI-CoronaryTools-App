// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong , Vincent Auvray                                  *
// ***************************************************************************
//SWLib for Width Based Sternal Wire Library

#define __STDC_LIMIT_MACROS
#include "SWLibDev.h"
#include <mutex> 

//#include "MxLab.h"
//#define SW_TIMING_DETAILS

#ifdef SW_TIMING_DETAILS
#ifndef SW_TIMING
#define SW_TIMING
#endif
#endif

#ifdef SW_TIMING
#include "omp.h"
#endif

/*************************************************************************************/
// SWLib: mask extraction
/*************************************************************************************/
int SWExtract(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize, bool consolidate, CSWRes * SWRes)
{
    CSW* hdl = (CSW*)Hdl->Pv;

    if ( (Iw > hdl->IwAlloc) || (Ih > hdl->IhAlloc) )
        return -1;
    if ( (t > 0) && ( (Iw != hdl->Iw) || (Ih != hdl->Ih) ) )
        return -2;

    SWRes->advanced.Rdg                 = hdl->Ctr;
    SWRes->advanced.SWMaskTight         = hdl->MaskSeqTight;
    SWRes->advanced.SWMaskLoose         = hdl->MaskSeqLoose;
    SWRes->advanced.SWMaskFinalTight    = SWRes->advanced.SWMaskTight;
    SWRes->advanced.SWMaskFinalLoose    = SWRes->advanced.SWMaskLoose;
    SWRes->SWMask                       = SWRes->advanced.SWMaskLoose;

#ifdef SW_TIMING
  double t0, t1, t2, t3;
#endif

    // * Update parameters
    SWUpdateParameters(Hdl, Iw, Ih, RoiCoord, t, pixSize);

    // * Frame-based computation
#ifdef SW_TIMING
  t0 = omp_get_wtime();
#endif
    SWExtractFrameBased(Hdl, Ori);

#ifdef SW_TIMING
  t1 = omp_get_wtime();
#endif

    // * Temporal consolidation
    SWExtractTemporalConsolidationMin(Hdl);

#ifdef SW_TIMING
  t2 = omp_get_wtime();
#endif

    // * Connected components consolidation
    if (consolidate){
        hdl->nbPixMask =  SWConsolidate(Hdl);

        // * Output

        SWRes->advanced.SWMaskFinalTight = hdl->MaskTightFinal;
        SWRes->advanced.SWMaskFinalLoose = hdl->MaskLooseFinal;

        SWRes->advanced.nbLabsBeforeConsolidation = hdl->nbLabsConsolidate;
        SWRes->advanced.nbPixPerLabBeforeConsolidation    = hdl->WkBufI2;
        SWRes->advanced.xBaryRegionBeforeCons = (float*)hdl->WVectF1; SWRes->advanced.yBaryRegionBeforeCons = (float*)hdl->WVectF2;
        SWRes->advanced.vDir1RegionBeforeCons = (float*)hdl->WVectFDoubleLength1; SWRes->advanced.vDir2RegionBeforeCons = (float*)hdl->WVectFDoubleLength2;
        SWRes->advanced.xMin1BeforeCons = (float*)hdl->WVectF3; SWRes->advanced.xMax1BeforeCons = (float*)hdl->WVectF4;
        SWRes->advanced.yMin2BeforeCons = (float*)hdl->WVectF5; SWRes->advanced.yMax2BeforeCons = (float*)hdl->WVectF6;
        SWRes->advanced.validationStatus = (unsigned char*)hdl->WVectUC;
    } else 
    {
        SWRes->advanced.SWMaskFinalTight = SWRes->advanced.SWMaskTight;
        SWRes->advanced.SWMaskFinalLoose = SWRes->advanced.SWMaskLoose;
        
        hdl->nbPixMask = 0;
        for (int y=hdl->Roi.YMin; y<=hdl->Roi.YMax; y++)
            for (int x=hdl->Roi.XMin; x<=hdl->Roi.XMax; x++)
                if (SWRes->advanced.SWMaskFinalTight [y*hdl->Iw+x] > 0)
                    hdl->nbPixMask ++ ;
        
        SWRes->advanced.nbLabsBeforeConsolidation = 0;
    }

    // * Outputs
    SWRes->advanced.Rdg                 = hdl->Ctr;
    SWRes->advanced.SWMaskTight         = hdl->MaskSeqTight;
    SWRes->advanced.SWMaskLoose         = hdl->MaskSeqLoose;
    SWRes->advanced.medianRdg           = hdl->medianRdg;

    SWRes->SWMask                       = SWRes->advanced.SWMaskFinalLoose;
    SWRes->Iw                           = Iw;
    SWRes->Ih                           = Ih;
    SWRes->nbPixMask                    = hdl->nbPixMask;

#ifdef SW_TIMING
    t3 = omp_get_wtime();
    printf("*** FrameBased %.1f Temporal min %.1f Consolidation %.1f\n", 1000.0 * (t1-t0), 1000.0 * (t2-t1), 1000.0 * (t3-t2));
    hdl->durationFrameBased = 1000.0 * (t1-t0); SWRes->advanced.durationFrameBased  = hdl->durationFrameBased ;
    hdl->durationTemporalMin = 1000.0 * (t2-t1); SWRes->advanced.durationTemporalMin = hdl->durationTemporalMin;
    hdl->durationConsolidation = 1000.0 * (t3-t2); SWRes->advanced.durationConsolidation = hdl->durationConsolidation;
    SWRes->advanced.durationRdg = hdl->durationRdg;
#endif

    return 0;
}

/*************************************************************************************/
int SWExtractFrameBased(CSWHdl *Hdl, const short* Ori)
{
  CSW*          hdl = (CSW*)Hdl->Pv;
  int           Iw = hdl->Iw, Ih = hdl->Ih;
  int           xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;
  std::mutex    mutex;
#ifdef SW_TIMING
  double        t0 = omp_get_wtime(), t1;
#endif

    // * Multiscale ridge computation - multithreaded
    for (int k = 0; k < Iw*Ih; k++)    hdl->fOri[k] = (float)Ori[k];
    memset(hdl->Ctr, 0, Iw*Ih*sizeof(float));
    for (int p = 0; p < Iw*Ih; p++){ hdl->Wid[p] = hdl->TabSig[0]; }
    
    hdl->pool->execute([&mutex,&hdl](int threadIndex, int threadCount)
    {
        // Set of sigmas used by this thread
        int     Iw = hdl->Iw, Ih = hdl->Ih;

        int     firstSigmaThread = (int)( (float)threadIndex * SW_NB_WIDTH / threadCount +0.5);
        int     lastSigmaThread = (int)( (float)(threadIndex+1) * SW_NB_WIDTH / threadCount +0.5);
        int     nbSigmasThread = lastSigmaThread-firstSigmaThread;

        float sigmasCore[16];
        for (int i=0; i<nbSigmasThread; i++){
            sigmasCore[i] = hdl->TabSig[firstSigmaThread+i];
        }
        
        // Ridge computations
        //MultiscaleHessianNoDir(hdl->fOri, Iw, Ih, &hdl->Roi, (const float*)sigmasCore, nbSigmasThread, SW_GAMMA, hdl->WidCores[threadIndex], hdl->CtrCores[threadIndex]
        //                        , hdl->WkBuf1Cores[threadIndex], hdl->WkBuf2Cores[threadIndex]);
        MultiscaleHessianNoDirSSEFloat(hdl->fOri, Iw, Ih, &hdl->Roi, (const float*)sigmasCore, nbSigmasThread, SW_GAMMA, hdl->WidCores[threadIndex], hdl->CtrCores[threadIndex]
                                , hdl->WkBuf1Cores[threadIndex], hdl->WkBuf2Cores[threadIndex]);

        
        // Merging the results of the different threads
        std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
        lock.lock();
        {
            for (int p = 0; p < Iw*Ih; p++)
            {
                if (hdl->CtrCores[threadIndex][p]>hdl->Ctr[p])
                {
                    hdl->Ctr[p] = hdl->CtrCores[threadIndex][p];
                    hdl->Wid[p] = hdl->WidCores[threadIndex][p];
                }
            }
        }
        lock.unlock();
    });
#ifdef SW_TIMING
    t1 = omp_get_wtime();
#endif
    
    // * Pixel selections based on ridge thickness and intensity

    // Threshold on ridgeness
    hdl->medianRdg = SWMedianPosRidgeByHisto(hdl->Ctr, Iw, Ih, xMin, xMax, yMin, yMax, hdl->histoBuf, SW_HISTO_NB_BINS);
    hdl->RdgThres = SW_LIB_MAX(SW_LAMBDA_THRES, SW_COEFF_LAMBDA_THRES * hdl->medianRdg); // SW_LAMBDA_THRES

    // Condition enforcement: above threshold, and in given sigma ranges
    // The admissible sigma range is larger for the loose energy
    memset(hdl->EnergyTight, 0, Iw*Ih*sizeof(float));
    memset(hdl->EnergyLoose, 0, Iw*Ih*sizeof(float));
    for (int y = yMin; y <= yMax; y++)
        for (int x = xMin; x <= xMax; x++)
            if (hdl->Ctr[y*Iw+x] > hdl->RdgThres){
                if (hdl->Wid[y*Iw+x] >= hdl->WidLowTight && hdl->Wid[y*Iw+x] <= hdl->WidHighTight){
                    hdl->EnergyLoose[y*Iw+x] = hdl->Ctr[y*Iw+x];

                    if ( (x >= xMin + hdl->MarginShutters) && (x <= xMax - hdl->MarginShutters) &&
                        (y >= yMin + hdl->MarginShutters) && (y <= yMax - hdl->MarginShutters) )
                        hdl->EnergyTight[y*Iw+x] = hdl->Ctr[y*Iw+x]; 
                } else {
                    if (hdl->Wid[y*Iw+x] >= hdl->WidLowLoose && hdl->Wid[y*Iw+x] <= hdl->WidHighLoose){
                        hdl->EnergyLoose[y*Iw+x] = hdl->Ctr[y*Iw+x];
                    }
                }
            }

    // * Vertical weighting
    VertiWeight(hdl->EnergyTight, Iw, Ih, hdl->VertiWeights, hdl->VertiSigmaPix);
    for (int y = yMin; y <= yMax; y++)
        for (int x = xMin; x <= xMax; x++)
                hdl->EnergyTight[x + y*Iw] *= hdl->VertiWeights[x];

    VertiWeight(hdl->EnergyLoose, Iw, Ih, hdl->VertiWeights, hdl->VertiSigmaPix);
    for (int y = yMin; y <= yMax; y++)
        for (int x = xMin; x <= xMax; x++)
                hdl->EnergyLoose[x + y*Iw] *= hdl->VertiWeights[x];

    // * Second thresholding based on the vertically weighted ridges)
    memset(hdl->MaskLiveTight, 0, Iw*Ih*sizeof(unsigned char));
    memset(hdl->MaskLiveLoose, 0, Iw*Ih*sizeof(unsigned char));
    for (int y = yMin; y <= yMax; y++)
        for (int x = xMin; x <= xMax; x++){
            if (hdl->EnergyTight[x + y*Iw] > SW_COEFF_AFTER_VERTI_WEIGHT*hdl->RdgThres)
                hdl->MaskLiveTight[x + y*Iw] = 1;
            if (hdl->EnergyLoose[x + y*Iw] > SW_COEFF_AFTER_VERTI_WEIGHT*hdl->RdgThres)
                hdl->MaskLiveLoose[x + y*Iw] = 1;
        }

    //{
    //    MxOpen(); MxSetVisible(1);
    //    MxPutMatrixFloat(hdl->Ctr,Iw,Ih,"Ctr");
    //    MxPutMatrixFloat(hdl->EnergyLoose,Iw,Ih,"EnergyLoose");
    //    MxPutMatrixFloat(hdl->EnergyTight,Iw,Ih,"EnergyTight");
    //    MxPutMatrixUC(hdl->MaskLiveTight,Iw,Ih,"MaskLiveTight");
    //    MxPutMatrixUC(hdl->MaskLiveLoose,Iw,Ih,"MaskLiveLoose");
    //    MxPutFloat(hdl->RdgThres,"RdgThres");
    //    MxCommand("figure(2); subplot('Position',[0,0.5,0.33,0.5]); Show(Ctr');");
    //    MxCommand("figure(2); subplot('Position',[0,0,0.33,0.5]); Show(Ctr' > RdgThres);");
    //    MxCommand("figure(2); subplot('Position',[0.33,0.5,0.33,0.5]); Show(EnergyLoose');");
    //    MxCommand("figure(2); subplot('Position',[0.67,0.5,0.33,0.5]); Show(EnergyTight');");
    //    MxCommand("figure(2); subplot('Position',[0.33,0.,0.33,0.5]); Show(MaskLiveLoose');");
    //    MxCommand("figure(2); subplot('Position',[0.67,0.,0.33,0.5]); Show(MaskLiveTight');");
    //} 

  //register mask image according to current image
#ifdef SW_TIMING_DETAILS
  double t2 = omp_get_wtime();
  printf("Ridge computation %.1f Computations1 %.1f ", 1000.0 * (t1-t0), 1000.0 * (t2-t1));
#endif

#ifdef SW_TIMING
  hdl->durationRdg = 1000.0 * (t1-t0);
#endif

  return 0;
}

/*************************************************************************************/
int SWExtractTemporalConsolidationMin(CSWHdl *Hdl)
{
  CSW* hdl = (CSW*)Hdl->Pv;
  int Iw = hdl->Iw, Ih = hdl->Ih, t = hdl->t;
  int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;

  int tx = 0, ty = 0;
  if (t > 0){
        // * Registration
        SWEstimateGlobalTranslation(hdl->MaskSeqTight, hdl->EnergyTight, Iw, Ih, xMin, xMax, yMin, yMax, 0, 0, hdl->SearchRange, hdl->SearchRange, hdl->pool, &tx, &ty);

        // * Warping
        memset(hdl->MaskRegTight, 0, Iw*Ih*sizeof(unsigned char)); 
        int xIniRef = SW_LIB_MAX(xMin,xMin-tx);
        int width = SW_LIB_MIN(xMax,xMax-tx)-SW_LIB_MAX(xMin,xMin-tx)+1;
        for (int y = SW_LIB_MAX(yMin,yMin-ty); y <= SW_LIB_MIN(yMax,yMax-ty); y++)
            memcpy(&(hdl->MaskRegTight[y*Iw + xIniRef]), &(hdl->MaskSeqTight[(y + ty)*Iw + xIniRef + tx]), width*sizeof(unsigned char));

        memset(hdl->MaskRegLoose, 0, Iw*Ih*sizeof(unsigned char));
        for (int y = SW_LIB_MAX(yMin,yMin-ty); y <= SW_LIB_MIN(yMax,yMax-ty); y++)
            memcpy(&(hdl->MaskRegLoose[y*Iw + xIniRef]), &(hdl->MaskSeqLoose[(y + ty)*Iw + xIniRef + tx]), width*sizeof(unsigned char));

        // * Temporal consolidation
        for (int y = yMin; y <= yMax; y++)
            for (int x = xMin; x <= xMax; x++)
                hdl->MaskSeqTight[y*Iw+x] = SW_LIB_MIN(hdl->MaskLiveTight[y*Iw+x], hdl->MaskRegTight[y*Iw+x]);

        ImDilate(hdl->MaskRegLoose, Iw, Ih, xMin, xMax, yMin, yMax, hdl->MaskRegLoose, 1, hdl->WkBufUC2);//3);
        for (int y = yMin; y <= yMax; y++)
            for (int x = xMin; x <= xMax; x++)
                hdl->MaskSeqLoose[y*Iw+x] = SW_LIB_MIN(hdl->MaskLiveLoose[y*Iw+x], hdl->MaskRegLoose[y*Iw+x]);
  }else {
      memcpy(hdl->MaskSeqTight, hdl->MaskLiveTight, Iw*Ih*sizeof(unsigned char));
      memcpy(hdl->MaskSeqLoose, hdl->MaskLiveLoose, Iw*Ih*sizeof(unsigned char));
  }


    //{
    //    MxOpen(); MxSetVisible(1);
    //    MxPutMatrixUC(hdl->MaskLiveTight,Iw,Ih,"MaskLiveTight");
    //    MxPutMatrixUC(hdl->MaskRegTight,Iw,Ih,"MaskRegTight");
    //    MxPutMatrixUC(hdl->MaskSeqTight,Iw,Ih,"MaskSeqTight");
    //    MxPutMatrixUC(hdl->MaskLiveLoose,Iw,Ih,"MaskLiveLoose");
    //    MxPutMatrixUC(hdl->MaskRegLoose,Iw,Ih,"MaskRegLoose");
    //    MxPutMatrixUC(hdl->MaskSeqLoose,Iw,Ih,"MaskSeqLoose");
    //    MxCommand("figure(2); subplot('Position',[0,0.5,0.33,0.5]); Show(MaskLiveTight');");
    //    MxCommand("figure(2); subplot('Position',[0.33,0.5,0.33,0.5]); Show(MaskRegTight');");
    //    MxCommand("figure(2); subplot('Position',[0.67,0.5,0.33,0.5]); Show(MaskSeqTight');");
    //    MxCommand("figure(2); subplot('Position',[0,0.,0.33,0.5]); Show(MaskLiveLoose');");
    //    MxCommand("figure(2); subplot('Position',[0.33,0.,0.33,0.5]); Show(MaskRegLoose');");
    //    MxCommand("figure(2); subplot('Position',[0.67,0.,0.33,0.5]); Show(MaskSeqLoose');");
    //} 


  return 0;
}


/*************************************************************************************/
int SWConsolidate(CSWHdl *Hdl)
{
    CSW* hdl = (CSW*)(Hdl->Pv);
    int Iw = hdl->Iw, Ih = hdl->Ih;
    int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;

    int *LabTight       = hdl->WkBufI1;
    int *nbPixPerLab    = hdl->WkBufI2;
    int *LabLoose       = hdl->WkBufI3;
    int nbLabs, nbPixInMask = 0;

#ifdef SW_TIMING_DETAILS
  double t0 = omp_get_wtime(), t1, t2, t3, t4, t5;
#endif

    // * Connected components on loose mask
    //SWConnectedComponents((const unsigned char *)hdl->MaskSeqLoose, Iw, Ih, xMin, xMax, yMin, yMax, hdl->RangeSameRegion
    //                      , hdl->WkBuf1S, hdl->WkBuf2S, hdl->WkBuf3S, hdl->WkBuf4S, LabLoose, nbPixPerLab, &nbLabs);
    SWConnectedComponentsAndDistance(Hdl, LabLoose, nbPixPerLab, &nbLabs);
    nbLabs = SW_LIB_MIN(nbLabs, SW_MAX_NB_REGIONS-1);
    

    //if (hdl->t == 5){
    //    short *ImTmp = (short*)malloc(Iw*Ih*sizeof(short));

    //    MxOpen(); MxSetVisible(1);
    //    MxPutMatrixUC(hdl->MaskSeqLoose, Iw, Ih, "MaskSeqLoose");
    //    for (int p=0; p<Iw*Ih; p++)
    //        ImTmp[p] = (short)LabLoose[p];
    //    MxPutMatrixShort(ImTmp, Iw, Ih, "LabLoose");

    //    MxCommand("figure(3); subplot('Position',[0.,0.5,0.5,0.5]); Show(MaskSeqLoose');");
    //    MxCommand("figure(3); subplot('Position',[0.5,0.5,0.5,0.5]); Show(LabLoose');");

    //    free(ImTmp);
    //}

#ifdef SW_TIMING_DETAILS
  t1 = omp_get_wtime();
#endif
  
    // * Determine how these connected components translate into Tight, and check which resulting region
    // should be accepted/rejected based on its shape (dimensions projected over principal directions) and surface
    unsigned char* valid = (unsigned char*)hdl->WVectUC;
    SWGeometricalConstraintsOnConnectedComponents(Hdl, (const int*)LabLoose, nbLabs, LabTight, nbPixPerLab, valid);
#ifdef SW_TIMING_DETAILS
  t2 = omp_get_wtime();
#endif

    // * Compute final mask
    // -->Loose mask under (a dilated version of) Tight
    memset(hdl->MaskTightFinal, 0, Iw*Ih*sizeof(unsigned char));
    for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++){
            if ( (LabTight[y*Iw+x] > 1) && (valid[LabTight[y*Iw+x]-2] >= 10) ){
                hdl->MaskTightFinal[y*Iw+x] = 1;
                nbPixInMask++;
            }
        }
#ifdef SW_TIMING_DETAILS
  t3 = omp_get_wtime();
#endif

    unsigned char * MaskDil = hdl->WkBufUC1, *Buf = hdl->WkBufUC2;
    ImDilate(hdl->MaskTightFinal, Iw, Ih, xMin, xMax, yMin, yMax, MaskDil, hdl->dilSizeMaxDistToLoose, Buf);
#ifdef SW_TIMING_DETAILS
  t4 = omp_get_wtime();
#endif
    memset(hdl->MaskLooseFinal, 0, Iw*Ih*sizeof(unsigned char));
    for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++){
            if ( (MaskDil[y*Iw+x] > 0) &&(LabLoose[y*Iw+x] > 1) ){
                hdl->MaskLooseFinal[y*Iw+x] = 1;
            }
        }

    //if (hdl->t == 5){
    //    short *ImTmp = (short*)malloc(Iw*Ih*sizeof(short));

    //    MxPutMatrixUC(hdl->MaskTightFinal, Iw, Ih, "MaskTightFinal");
    //    MxPutMatrixUC(hdl->MaskLooseFinal, Iw, Ih, "MaskLooseFinal");
    //    
    //    MxCommand("figure(3); subplot('Position',[0,0,0.5,0.5]); Show(MaskTightFinal');");
    //    MxCommand("figure(3); subplot('Position',[0.5,0,0.5,0.5]); Show(MaskLooseFinal');");
    //    

    //    free(ImTmp);
    //}
        

#ifdef SW_TIMING_DETAILS
  t5 = omp_get_wtime();
  printf("Connected components %.1f Decision 1 %.1f Validation %.1f Dilatation %.1f Output mask computation %.1f\n", 1000.0 * (t1-t0), 1000.0 * (t2-t1), 1000.0 * (t3-t2), 1000.0 * (t4-t3), 1000.0 * (t5-t4));
#endif
  
    hdl->nbLabsConsolidate = nbLabs;

    return nbPixInMask;
}
