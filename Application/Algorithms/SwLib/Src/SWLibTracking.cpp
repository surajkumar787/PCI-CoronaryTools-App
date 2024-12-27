// ***************************************************************************
// * Copyright (c) 2002-2016 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Vincent Auvray                                                  *
// ***************************************************************************
//SWLib for Width Based Sternal Wire Library

#define __STDC_LIMIT_MACROS
#include "SWLibDev.h"
#include <mutex> 

/*************************************************************************************/
int SWTracking(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize, CSWRes * SWRes){
    CSW* hdl = (CSW*)Hdl->Pv;

    SWUpdateParameters(Hdl, Iw, Ih, RoiCoord, t, pixSize);
    return SWTrack(Hdl, Ori, Iw, Ih, RoiCoord, 0, 0, hdl->SearchRange, hdl->SearchRange, SWRes);
}

/*************************************************************************************/
int SWTrackingWithPanningInfo(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int t, float pixSize, float dXPanningMM, float dYPanningMM, CSWRes * SWRes){
    CSW* hdl = (CSW*)Hdl->Pv;

    SWUpdateParameters(Hdl, Iw, Ih, RoiCoord, t, pixSize);
    return SWTrack(Hdl, Ori, Iw, Ih, RoiCoord, SW_LIB_RND(dXPanningMM / hdl->pixSize), SW_LIB_RND(dYPanningMM / hdl->pixSize), hdl->SearchRange, hdl->SearchRange, SWRes);
}
/*************************************************************************************/
int  SWRepositionMask(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, float pixSize
                      , float dXPanningMM, float dYPanningMM, float searchRangeXMM, float searchRangeYMM, CSWRes * SWRes){
    CSW* hdl = (CSW*)Hdl->Pv;

    SWUpdateParameters(Hdl, Iw, Ih, RoiCoord, 0, pixSize);
    return SWTrack(Hdl, Ori, Iw, Ih, RoiCoord, SW_LIB_RND(dXPanningMM / hdl->pixSize), SW_LIB_RND(dYPanningMM / hdl->pixSize)
                    , SW_LIB_RND(searchRangeXMM / hdl->pixSize), SW_LIB_RND(searchRangeYMM / hdl->pixSize), SWRes);
}

/*************************************************************************************/
int SWTrack(CSWHdl *Hdl, const short* Ori, int Iw, int Ih, CSWRoiCoord* RoiCoord, int dXInit, int dYInit, int searchRangeXPix, int searchRangeYPix, CSWRes * SWRes){
    CSW* hdl = (CSW*)Hdl->Pv;
    
    if ( (Iw > hdl->IwAlloc) || (Ih > hdl->IhAlloc) )
        return -1;
    if ( (Iw != hdl->Iw) || (Ih != hdl->Ih) )
        return -2;

    // Ridge filter
    float *Gau = hdl->WkBuf1Cores[0];
    for (int k = 0; k < Iw*Ih; k++)    hdl->fOri[k] = (float)Ori[k];
    SWHessianNoDirSSEFloat(hdl->fOri, Gau, hdl->Ctr, Iw, Ih, RoiCoord, hdl->sigmaTracking);

    // Tracking
    int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax, tx, ty;
    SWEstimateGlobalTranslation(hdl->MaskLooseFinal, hdl->Ctr, Iw, Ih, xMin, xMax, yMin, yMax, dXInit, dYInit, searchRangeXPix, searchRangeYPix, hdl->pool, &tx, &ty);

    // Warping
    memcpy(hdl->WkBufUC1, hdl->MaskLooseFinal, Iw*Ih*sizeof(unsigned char));
    memset(hdl->MaskLooseFinal, 0, Iw*Ih*sizeof(unsigned char));  

    int xIniRef = SW_LIB_MAX(xMin,xMin-tx);
    int width = SW_LIB_MIN(xMax,xMax-tx)-SW_LIB_MAX(xMin,xMin-tx)+1;
    for (int y = SW_LIB_MAX(yMin,yMin-ty); y <= SW_LIB_MIN(yMax,yMax-ty); y++)
        memcpy(&(hdl->MaskLooseFinal[y*Iw + xIniRef]), &(hdl->WkBufUC1[(y + ty)*Iw + xIniRef + tx]), width*sizeof(unsigned char));

    SWRes->SWMask = hdl->MaskLooseFinal;

    return 0;
}

/*************************************************************************************/
unsigned char SWEstimateGlobalTranslation(unsigned char* Maptm1, float* Energyt, int Iw, int Ih, int xMin, int xMax, int yMin, int yMax
                                 , int dXInit, int dYInit, int SearchRangePixX, int SearchRangePixY, IThreadPool *pool, int *TxPix, int* TyPix)
{
    std::mutex    mutex;

    int xMinMask = xMax, xMaxMask = xMin, yMinMask = yMax, yMaxMask = yMin;
    for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++)
            if (Maptm1[y*Iw+x] > 0){
                xMinMask = SW_LIB_MIN(xMinMask, x); xMaxMask = SW_LIB_MAX(xMaxMask, x);
                yMinMask = SW_LIB_MIN(yMinMask, y); yMaxMask = SW_LIB_MAX(yMaxMask, y);
            }

    if ( (xMaxMask < xMinMask) || (yMaxMask < yMinMask) ){
        *TxPix = 0; TyPix = 0; return 1;
    }


    float bestEnergy = 0.f;    
    int txBest = 0, tyBest = 0;
    pool->execute([&mutex, &bestEnergy, &txBest, &tyBest, dXInit, dYInit, SearchRangePixX, SearchRangePixY
                    , xMinMask, xMaxMask, yMinMask, yMaxMask, xMin, xMax, yMin, yMax, Iw, Maptm1, Energyt](int threadIndex, int threadCount)
    {
        int tyMinThread = dYInit-SearchRangePixY + SW_LIB_RND((float)threadIndex*(2*SearchRangePixY+1)/threadCount);
        int tyMaxThread = dYInit-SearchRangePixY + SW_LIB_RND((float)(threadIndex+1)*(2*SearchRangePixY+1)/threadCount);

        float bestEnergyThread = 0.f;
        int txThread, tyThread;
        for (int ty = tyMinThread; ty<tyMaxThread; ty++){
            for (int tx = dXInit-SearchRangePixX; tx<=dXInit+SearchRangePixX; tx++){

                float energyHere = 0.f;

                int xMinHere = SW_LIB_MAX(xMinMask, xMin+tx), xMaxHere = SW_LIB_MIN(xMaxMask, xMax+ty);
                int yMinHere = SW_LIB_MAX(yMinMask, yMin+ty), yMaxHere = SW_LIB_MIN(yMaxMask, yMax+ty);
                int width = xMaxHere-xMinHere+1, height = yMaxHere-yMinHere+1, stride = Iw-width;

                unsigned char* ptrMap = &Maptm1[yMinHere*Iw+xMinHere];
                float* ptrEnergy = &Energyt[(yMinHere-ty)*Iw+xMinHere-tx];
                for (int y=0; y<height; y++){
                    for (int x=0; x<width; x++){
                        energyHere += ((*ptrMap)*(*ptrEnergy));
                        ptrMap++; ptrEnergy++;
                    }
                    ptrMap += stride; ptrEnergy += stride;
                }

                if (energyHere > bestEnergyThread){
                    bestEnergyThread = energyHere;
                    txThread = tx; tyThread = ty;
                }
            }
        }
        // Merging the results of the different threads
        std::unique_lock<std::mutex> lock(mutex,std::defer_lock);
        lock.lock();
        {
            if (bestEnergyThread > bestEnergy){
                bestEnergy = bestEnergyThread;
                txBest = txThread; tyBest = tyThread;
            }
        }
    });
    
    *TxPix = txBest; *TyPix = tyBest;
    return 0;
}

/*************************************************************************************/
void SWGetEstimatedMotion(CSWHdl *Hdl, int *tx, int *ty)
{
  CSW* hdl = (CSW*)(Hdl->Pv);

  *tx = hdl->estTx; *ty = hdl->estTy;
}
