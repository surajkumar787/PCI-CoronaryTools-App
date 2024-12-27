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

#include "SWLibDev.h"


void SWGeometricalConstraintsOnConnectedComponents(CSWHdl *Hdl, const int *LabLoose, int nbLabs, int * LabTight, int *nbPixPerLab, unsigned char * valid){

    CSW* hdl = (CSW*)(Hdl->Pv);
    int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;
    
    // * Initialization
    int *xMinLab = (int*)hdl->WVectI1, *xMaxLab = (int*)hdl->WVectI2, *yMinLab = (int*)hdl->WVectI3, *yMaxLab = (int*)hdl->WVectI4;
    float *xBary = (float*)hdl->WVectF1, *yBary = (float*)hdl->WVectF2, *xMinEig1 = (float*)hdl->WVectF3, *xMaxEig1 = (float*)hdl->WVectF4
        , *yMinEig2 = (float*)hdl->WVectF5, *yMaxEig2 = (float*)hdl->WVectF6;

    // * Translate labelling from Loose to Tight mask
    // And compute bounding boxes and barycenters of the regions
    SWTransferLabelToTightAndComputeLimitsAndBary(Hdl, LabLoose, nbLabs, LabTight, nbPixPerLab, xMinLab, xMaxLab, yMinLab, yMaxLab, xBary, yBary);

    // * Compute regions extension, projected over the region's principal components
    SWComputeRegionExtensionInPrincipalDirections(Hdl, LabTight, nbLabs, (const int *)xMinLab, (const int*)xMaxLab, (const int*)yMinLab, (const int*)yMaxLab, (const float *)xBary, (const float*)yBary
                                                   , (const int*)nbPixPerLab, xMinEig1, xMaxEig1, yMinEig2, yMaxEig2);

    // * Finally, discussion
    // If does not touch border, condition on extensions on first and second vector, and on ratio of extensions ;
    // If touches border, condition on region surface. If long enough, further conditions on extensions and their ratio.

    // Codes:   - 0 never considered
    //          - 1 touches border and too small surface
    //          - 2 touches border and does not respect some geometrical condition
    //          - 3 does not touche border and does not respect some geometrical condition
    //          - 4 does not touche border, respects some geometrical condition but too small surface
    //          - 8 does touches border and intermediate surface, final decision rejection
    //          - 9 touches border and intermediate surface, final decision rejection
    //          - 10 directly accepted (large enough, respects boundary conditions, can or cannot touch border)
    //          - 11 touches border and intermediate surface, final decision acceptation
    //          - 12 does not touch border and intermediate surface, final decision acceptation

    for (int lab=0; lab < nbLabs; lab++){

        if (nbPixPerLab[lab] == 0){
            valid[lab] = 0;
            continue;
        }

        valid[lab] = 10;
        float ratioEigenValues = SW_LIB_MIN(xMaxEig1[lab]-xMinEig1[lab], yMaxEig2[lab]-yMinEig2[lab]) / ((float)SW_LIB_MAX(xMaxEig1[lab]-xMinEig1[lab], yMaxEig2[lab]-yMinEig2[lab]));

        if ( (xMinLab[lab] <= xMin + hdl->DistToTouchBorder) || (xMaxLab[lab] >= xMax - hdl->DistToTouchBorder)
            || (yMinLab[lab] <= yMin + hdl->DistToTouchBorder) || (yMaxLab[lab] >= yMax - hdl->DistToTouchBorder) ){
                // Touches border
                if (nbPixPerLab[lab] < hdl->MinSurfaceTouchesBorder_2){
                    valid[lab] = 1;
                } else {
                    if (nbPixPerLab[lab] < hdl->MinSurfaceTouchesBorder_1){
                        valid[lab] = 9;
                    }
                }

                if ( (xMinLab[lab] <= xMin + hdl->DistToTouchBorder) && (xMaxLab[lab] >= xMin + hdl->DistToTouchBorder + hdl->MinDistSWX/2)   // Touches on left, but long enough
                    && (yMinLab[lab] >= yMin + hdl->DistToTouchBorder) && (yMaxLab[lab] <= yMax - hdl->DistToTouchBorder)                     // And does not touch in Y
                    && ( (yMaxEig2[lab]-yMinEig2[lab] < hdl->MinDistSWY) || ( ratioEigenValues < SW_MIN_RATIO_EIGENVALUES) ) )
                        valid[lab] = 2;

                if ( (xMaxLab[lab] >= xMax - hdl->DistToTouchBorder) && (xMinLab[lab] <= xMax - hdl->DistToTouchBorder - hdl->MinDistSWX/2)   // Touches on right, but long enough
                    && (yMinLab[lab] >= yMin + hdl->DistToTouchBorder) && (yMaxLab[lab] <= yMax - hdl->DistToTouchBorder)                     // And does not touch in Y
                    && ( (yMaxEig2[lab]-yMinEig2[lab] < hdl->MinDistSWY) || ( ratioEigenValues < SW_MIN_RATIO_EIGENVALUES) ) )
                        valid[lab] = 2;

                if ( (yMinLab[lab] <= yMin + hdl->DistToTouchBorder) && (yMaxLab[lab] >= yMin + hdl->DistToTouchBorder + hdl->MinDistSWY/2)   // Touches on Top, but long enough
                    && (xMinLab[lab] >= xMin + hdl->DistToTouchBorder) && (xMaxLab[lab] <= xMax - hdl->DistToTouchBorder)                     // And does not touch in X
                    && ( (xMaxEig1[lab]-xMinEig1[lab] < hdl->MinDistSWX) || ( ratioEigenValues < SW_MIN_RATIO_EIGENVALUES) ) )
                        valid[lab] = 2;

                if ( (yMaxLab[lab] >= yMax - hdl->DistToTouchBorder) && (yMinLab[lab] <= yMax - hdl->DistToTouchBorder-hdl->MinDistSWY/2)     // Touches on Bottom, but long enough
                    && (xMinLab[lab] >= xMin + hdl->DistToTouchBorder) && (xMaxLab[lab] <= xMax - hdl->DistToTouchBorder)                     // And does not touch in X
                    && ( (xMaxEig1[lab]-xMinEig1[lab] < hdl->MinDistSWX) || ( ratioEigenValues < SW_MIN_RATIO_EIGENVALUES) ) )
                        valid[lab] = 2;

        } else {
            float ratioEigenValues = SW_LIB_MIN(xMaxEig1[lab]-xMinEig1[lab], yMaxEig2[lab]-yMinEig2[lab]) / ((float)SW_LIB_MAX(xMaxEig1[lab]-xMinEig1[lab], yMaxEig2[lab]-yMinEig2[lab]));

            // Does not touch border
            if ( (xMaxEig1[lab]-xMinEig1[lab] < hdl->MinDistSWX) || (yMaxEig2[lab]-yMinEig2[lab] < hdl->MinDistSWY)
                || ( ratioEigenValues < SW_MIN_RATIO_EIGENVALUES) ){

                valid[lab] = 3;
            } else {
                if (nbPixPerLab[lab] < hdl->MinSurface_2){
                    valid[lab] = 4;
                } else {
                    if (nbPixPerLab[lab] < hdl->MinSurface_1){
                        valid[lab] = 8;
                    }
                }
            }
        }
    }

    // * Finally, the dubious regions (labelled 2) are either rejected if they are alone, or definitely accepted
    unsigned char oneValid = 0;
    for (int lab=0; lab < nbLabs; lab++){
        if (valid[lab] == 10)
            oneValid = 1;
    }
    if (oneValid== 1){
        for (int lab=0; lab < nbLabs; lab++){
            if (valid[lab] == 9)
                valid[lab]  = 11;
            if (valid[lab] == 8)
                valid[lab]  = 12;
        }
    }
}

// *****************************************************************************************
void SWTransferLabelToTightAndComputeLimitsAndBary(CSWHdl *Hdl, const int *LabLoose, int nbLabs, int * LabTight, int *nbPixPerLab
                                                   , int *xMinLab, int*xMaxLab, int*yMinLab, int*yMaxLab, float *xBary, float*yBary){

    CSW* hdl = (CSW*)(Hdl->Pv);
    int Iw = hdl->Iw, Ih = hdl->Ih;
    int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;
    
    // * Initialization
    for (int lab=0; lab < nbLabs; lab++){
        xMinLab[lab] = Iw-1; xMaxLab[lab] = 0;
        yMinLab[lab] = Ih-1; yMaxLab[lab] = 0;
    }

    memset(nbPixPerLab,0,nbLabs*sizeof(int));
    memset(LabTight,0,Iw*Ih*sizeof(int));

    // * Translate labelling from Loose to Tight mask
    // And compute bounding boxes and barycenters of the regions
    memset(xBary,0,nbLabs*sizeof(float));
    memset(yBary,0,nbLabs*sizeof(float));

    for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++){
            if (hdl->MaskSeqTight[y*Iw+x] > 0 && LabLoose[y*Iw+x] > 1){
                LabTight[y*Iw+x] = LabLoose[y*Iw+x];
                
                int ind = LabTight[y*Iw+x]-2;
                xMinLab[ind] = SW_LIB_MIN(x, xMinLab[ind]);
                xMaxLab[ind] = SW_LIB_MAX(x, xMaxLab[ind]);
                yMinLab[ind] = SW_LIB_MIN(y, yMinLab[ind]);
                yMaxLab[ind] = SW_LIB_MAX(y, yMaxLab[ind]);
                nbPixPerLab[ind]++;

                xBary[ind] += (float)x; yBary[ind] += (float)y;
            }
        }
    for (int lab=0; lab <nbLabs; lab++)
        if (nbPixPerLab[lab] > 0){
            xBary[lab] = xBary[lab] / nbPixPerLab[lab];
            yBary[lab] = yBary[lab] / nbPixPerLab[lab];
        }
}

// *****************************************************************************************
void SWComputeRegionExtensionInPrincipalDirections(CSWHdl *Hdl, const int *LabTight, int nbLabs
                                                   , const int *xMinLab, const int*xMaxLab, const int* yMinLab, const int*yMaxLab, const float *xBary, const float*yBary, const int*nbPixPerLab
                                                   , float *xMinEig1, float *xMaxEig1, float *yMinEig2, float *yMaxEig2){

    CSW* hdl = (CSW*)(Hdl->Pv);
    int Iw = hdl->Iw;
    int xMin = hdl->Roi.XMin, xMax = hdl->Roi.XMax, yMin = hdl->Roi.YMin, yMax = hdl->Roi.YMax;
    float *Txx = (float*)hdl->WVectF7, *Tyy = (float*)hdl->WVectF8, *Txy = (float*)hdl->WVectF9
        , *u1 = (float*)hdl->WVectFDoubleLength1, *u2 = (float*)hdl->WVectFDoubleLength2;

    // * Computing main directions of the labelled regions
    // Eigenvectors are computed, normalized, and exchanged so the 1 is closest to ex

    // Inertia tensor for each region
    memset(Txx, 0, nbLabs*sizeof(float));
    memset(Tyy, 0, nbLabs*sizeof(float));
    memset(Txy, 0, nbLabs*sizeof(float));
    for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++)
            if (LabTight[y*Iw+x] > 1){
                int ind = LabTight[y*Iw+x]-2;

                Txx[ind] += (x-xBary[ind])*(x-xBary[ind]);
                Tyy[ind] += (y-yBary[ind])*(y-yBary[ind]);
                Txy[ind] += (x-xBary[ind])*(y-yBary[ind]);
            }

    // Corresponding eigenvectors (potentially exchanged)
    for (int lab=0; lab <nbLabs; lab++){
        float T = Txx[lab] + Tyy[lab], D = Txx[lab]*Tyy[lab] - Txy[lab]*Txy[lab];
        float l1 = T/2 + static_cast<float>(sqrt(T*T/4-D)), l2 = T/2 - static_cast<float>(sqrt(T*T/4-D));
        if (Txy[lab] != 0){
            u1[2*lab] = l1-Tyy[lab]; u1[2*lab+1] = Txy[lab];
            u2[2*lab] = l2-Tyy[lab]; u2[2*lab+1] = Txy[lab];

            float norm = static_cast<float>(sqrt(u1[2*lab]*u1[2*lab] + u1[2*lab+1]*u1[2*lab+1]));
            u1[2*lab] /= norm; u1[2*lab+1] /= norm;
            norm = static_cast<float>(sqrt(u2[2*lab]*u2[2*lab] + u2[2*lab+1]*u2[2*lab+1]));
            u2[2*lab] /= norm; u2[2*lab+1] /= norm;

            if (SW_LIB_ABS_F(u1[2*lab]) < SW_LIB_ABS_F(u2[2*lab])){                 // 1st direction has to be closest to X
                float buf = u1[2*lab]; u1[2*lab] = u2[2*lab]; u2[2*lab] = buf;
                buf = u1[2*lab+1]; u1[2*lab+1] = u2[2*lab+1]; u2[2*lab+1] = buf;
            }
        } else {
            u1[2*lab] = 1; u1[2*lab+1] = 0;
            u2[2*lab] = 0; u2[2*lab+1] = 1;
        }
    }

    // * Computing max extension of the region in the new basis (extrema of projections)
   for (int lab=0; lab <nbLabs; lab++){
       if (nbPixPerLab[lab] > 0){
           xMinEig1[lab] = FLT_MAX; xMaxEig1[lab] = -FLT_MAX;
           yMinEig2[lab] = FLT_MAX; yMaxEig2[lab] = -FLT_MAX;
        } else {
           xMinEig1[lab] = 0.f; xMaxEig1[lab] = 0.f;
           yMinEig2[lab] = 0.f; yMaxEig2[lab] = 0.f;
       }
   }

   for (int y=yMin; y<=yMax; y++)
        for (int x=xMin; x<=xMax; x++)
            if (LabTight[y*Iw+x] > 1){
                int ind = LabTight[y*Iw+x]-2;

                float px = (x-xBary[ind])*u1[2*ind] + (y-yBary[ind])*u1[2*ind+1];
                float py = (x-xBary[ind])*u2[2*ind] + (y-yBary[ind])*u2[2*ind+1];

                xMinEig1[ind] = SW_LIB_MIN(xMinEig1[ind], px);
                xMaxEig1[ind] = SW_LIB_MAX(xMaxEig1[ind], px);
                yMinEig2[ind] = SW_LIB_MIN(yMinEig2[ind], py);
                yMaxEig2[ind] = SW_LIB_MAX(yMaxEig2[ind], py);
            }
}