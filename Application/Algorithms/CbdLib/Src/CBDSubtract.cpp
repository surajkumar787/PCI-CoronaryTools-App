#include "CBDDev.h"

// *** Subtractionwith the tube model
int CBDSubtractCBDTube(short *Im, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                        , CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults, short *ImOut){

    // When model has been identified
    if (PtrCBDResults->detectedCath >= 2){

        // Initializations
        CCBD * This = (CCBD*)PtrCBDHdl->Pv;
        float RInside = This->CBD_radiusInside, ROutside = This->CBD_radiusOutside; 
        float *alphaInside = This->CBD_attenuationInside, *alphaTube = This->CBD_attenuationTube;
        int *contrastYCP = This->CBD_contrastCPY;

        int yMin, yMax;
        yMin = ShutterPos[3];
        for (int y=ShutterPos[2]; y<=ShutterPos[3]; y++){
            float xCT = CBDComputeFromPolynom((float) y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);
            if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) ){
                yMin = y; break;
            }
        }
        yMax = ShutterPos[2];
        for (int y=ShutterPos[3]; y>=ShutterPos[2]; y--){
            float xCT = CBDComputeFromPolynom((float) y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);
            if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) ){
                yMax = y; break;
            }
        }
        memcpy(ImOut, Im, Iw*Ih*sizeof(short));

        if (This->CBD_nbEstimatedContrasts == 1){
            // * Case with one contrast for whole CBD
            for (int y=yMin; y<=yMax; y++){
                float xs = CBDComputeFromPolynom((float)y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);

                for (int x=CBD_MAX(0,(int)floor(xs-ROutside/isoPixSize)); x<=CBD_MIN(Iw-1,(int)ceil(xs+ROutside/isoPixSize)); x++){
                    float d = CBD_ABS_F((xs-x)*isoPixSize);
                    float coeff1 = sqrt(CBD_MAX(0.f,RInside*RInside - d*d));
                    float coeff2 = sqrt(CBD_MAX(0.f,ROutside*ROutside - d*d))-coeff1;
                    ImOut[y*Iw+x] = Im[y*Iw+x] + (short)CBD_RND(alphaInside[0]*coeff1)
                        + (short)CBD_RND(alphaTube[0]*coeff2);
                }
            }
        } else {

            // * Case with different contrasts for whole CBD
            if (alphaInside[0] > 0){
                float contrastInside = alphaInside[0], contrastTube = alphaTube[0];
                for (int y=yMin; y<contrastYCP[0]; y++){
                    float xs = CBDComputeFromPolynom((float)y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);

                    for (int x=CBD_MAX(ShutterPos[0],(int)floor(xs-ROutside/isoPixSize)); 
                            x<=CBD_MIN(ShutterPos[1],(int)ceil(xs+ROutside/isoPixSize)); x++){
                        float d = CBD_ABS_F((xs-x)*isoPixSize);
                        float coeff1 = sqrt(CBD_MAX(0.f,RInside*RInside - d*d));
                        float coeff2 = sqrt(CBD_MAX(0.f,ROutside*ROutside - d*d))-coeff1;
                        ImOut[y*Iw+x] = Im[y*Iw+x] + (short)CBD_RND(contrastInside*coeff1) + (short)CBD_RND(contrastTube*coeff2);
                    }
                }
            }
            for (int j=0; j<This->CBD_nbEstimatedContrasts-1; j++){
                int yMinHere = contrastYCP[j], yMaxHere = CBD_MIN(contrastYCP[j+1], yMax+1);
                
                if (alphaInside[j] > 0){
                    
                    if (alphaInside[j+1] > 0){

                        for (int y=yMinHere; y<yMaxHere; y++){
                            float dYCP = (y-yMinHere)/(float)(yMaxHere-yMinHere);
                            float contrastInside = (1-dYCP)*alphaInside[j] + dYCP*alphaInside[j+1]
                                , contrastTube = (1-dYCP)*alphaTube[j] + dYCP*alphaTube[j+1];
                            float xs = CBDComputeFromPolynom((float)y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);

                            for (int x=CBD_MAX(ShutterPos[0],(int)floor(xs-ROutside/isoPixSize)); 
                                    x<=CBD_MIN(ShutterPos[1],(int)ceil(xs+ROutside/isoPixSize)); x++){
                                float d = CBD_ABS_F((xs-x)*isoPixSize);
                                float coeff1 = sqrt(CBD_MAX(0.f,RInside*RInside - d*d));
                                float coeff2 = sqrt(CBD_MAX(0.f,ROutside*ROutside - d*d))-coeff1;
                                ImOut[y*Iw+x] = Im[y*Iw+x] + (short)CBD_RND(contrastInside*coeff1) + (short)CBD_RND(contrastTube*coeff2);
                            }
                        }
                    } else {

                        float contrastInside = alphaInside[j], contrastTube = alphaTube[j];
                        for (int y=yMinHere; y<yMaxHere; y++){
                            float xs = CBDComputeFromPolynom((float)y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);

                            for (int x=CBD_MAX(ShutterPos[0],(int)floor(xs-ROutside/isoPixSize)); 
                                x<=CBD_MIN(ShutterPos[1],(int)ceil(xs+ROutside/isoPixSize)); x++){
                                float d = CBD_ABS_F((xs-x)*isoPixSize);
                                float coeff1 = sqrt(CBD_MAX(0.f,RInside*RInside - d*d));
                                float coeff2 = sqrt(CBD_MAX(0.f,ROutside*ROutside - d*d))-coeff1;
                                ImOut[y*Iw+x] = Im[y*Iw+x] + (short)CBD_RND(contrastInside*coeff1) + (short)CBD_RND(contrastTube*coeff2);
                            }
                        }
                    }

                } else {

                    if (alphaInside[j+1] > 0){
                        float contrastInside = alphaInside[j+1], contrastTube = alphaTube[j+1];
                        for (int y=yMinHere; y<yMaxHere; y++){
                            float xs = CBDComputeFromPolynom((float)y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);

                            for (int x=CBD_MAX(ShutterPos[0],(int)floor(xs-ROutside/isoPixSize)); 
                                    x<=CBD_MIN(ShutterPos[1],(int)ceil(xs+ROutside/isoPixSize)); x++){
                                float d = CBD_ABS_F((xs-x)*isoPixSize);
                                float coeff1 = sqrt(CBD_MAX(0.f,RInside*RInside - d*d));
                                float coeff2 = sqrt(CBD_MAX(0.f,ROutside*ROutside - d*d))-coeff1;
                                ImOut[y*Iw+x] = Im[y*Iw+x] + (short)CBD_RND(contrastInside*coeff1) + (short)CBD_RND(contrastTube*coeff2);
                            }
                        }
                    }

                }
            }

            if (contrastYCP[This->CBD_nbEstimatedContrasts-1] > 0){
                float contrastInside = alphaInside[This->CBD_nbEstimatedContrasts-1], contrastTube = alphaTube[This->CBD_nbEstimatedContrasts-1];
                for (int y=contrastYCP[This->CBD_nbEstimatedContrasts-1]; y<=yMax; y++){
                    
                    float xs = CBDComputeFromPolynom((float)y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);

                    for (int x=CBD_MAX(ShutterPos[0],(int)floor(xs-ROutside/isoPixSize)); 
                            x<=CBD_MIN(ShutterPos[1],(int)ceil(xs+ROutside/isoPixSize)); x++){
                        float d = CBD_ABS_F((xs-x)*isoPixSize);
                        float coeff1 = sqrt(CBD_MAX(0.f,RInside*RInside - d*d));
                        float coeff2 = sqrt(CBD_MAX(0.f,ROutside*ROutside - d*d))-coeff1;
                        ImOut[y*Iw+x] = Im[y*Iw+x] + (short)CBD_RND(contrastInside*coeff1) + (short)CBD_RND(contrastTube*coeff2);
                    }
                }
            }
        }
    }

    return 0;
}

// *** Conditions on the CBD orientation, and its minimal length
int CBD_ValidateCBD(float *coeffPolynom, int degreePolynome, float yCBDNorm, int ShutPos[4]){

    int nbInROI = 0;
    int yMin = ShutPos[2], yMax = ShutPos[3];
    float xMin = (float)ShutPos[1], xMax = (float)ShutPos[0];
    for (int y=yMin; y<=yMax; y++){
        float xCT = CBDComputeFromPolynom((float) y, coeffPolynom, degreePolynome, yCBDNorm);
        if ( (xCT >= ShutPos[0]) && (xCT <= ShutPos[1]) ){
            nbInROI++;
            xMin = CBD_MIN(xMin, xCT); xMax = CBD_MAX(xMax, xCT);
        }
    }
    float orientation = (xMax - xMin) / (yMax - yMin);

    if ( (CBD_ABS_F(orientation) >= CBD_ORIENTATION_LIMIT) || (nbInROI < CBD_MIN_RELATIVE_LENGTH * (yMax-yMin+1) ) )
        return -1;
    else
        return 0;
}

// *** Compute distance between centerlines
float CBD_DistanceBetweenCenterlines(float *coeffsPolynomPrev, float *coeffsPolynom, int degreePolynom, float yNorm
                                        , int ShutterPos[4], float isoPixSize, float maxDeltaX, float maxDeltaY){

    int yMin = ShutterPos[2], yMax = ShutterPos[3];
    float deltaSpatialStepX = (2*maxDeltaX) / CBD_DIST_NBSAMPLESDISTANCE,deltaSpatialStepY = (2*maxDeltaY) / CBD_DIST_NBSAMPLESDISTANCE;
    float *CTRef = (float*)malloc((yMax-yMin+1)*sizeof(float)), *CTTest = (float*)malloc((yMax-yMin+1)*sizeof(float))
        , *coeffsPolynomTested = (float*)malloc((degreePolynom+1)*sizeof(float))
        , *coeffsPolynom1 = (float*)malloc((degreePolynom+1)*sizeof(float))
        , *coeffsPolynom2 = (float*)malloc((degreePolynom+1)*sizeof(float));
    //int minLength = CBD_RND(0.33f*(yMax-yMin));

    // *** Choice reference (the largest)

    // 1
    int yMinInside1 = yMin, yMaxInside1 = yMax;
    for (int y=yMin; y<=yMax; y++){
        float xCT = CBDComputeFromPolynom((float) y, coeffsPolynom, degreePolynom, yNorm);
        if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) )
            break;
        yMinInside1++;
    }
    for (int y=yMax; y>=yMinInside1; y--){
        float xCT = CBDComputeFromPolynom((float) y, coeffsPolynom, degreePolynom, yNorm);
        if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) ) 
            break;
        yMaxInside1--;
    }
    
    // 2
    int yMinInside2 = yMin, yMaxInside2 = yMax;
    for (int y=yMin; y<=yMax; y++){
        float xCT = CBDComputeFromPolynom((float) y, coeffsPolynomPrev, degreePolynom, yNorm);
        if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) )
            break;
        yMinInside2++;
    }
    for (int y=yMax; y>=yMinInside2; y--){
        float xCT = CBDComputeFromPolynom((float) y, coeffsPolynomPrev, degreePolynom, yNorm);
        if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) ) 
            break;
        yMaxInside2--;
    }

    // Conclude on reference
    int yMinInside, yMaxInside;
    if (yMaxInside1-yMinInside1 >= yMaxInside2-yMinInside2){
        memcpy(coeffsPolynom1, coeffsPolynom, (degreePolynom+1)*sizeof(float));
        memcpy(coeffsPolynom2, coeffsPolynomPrev, (degreePolynom+1)*sizeof(float));
        yMinInside = yMinInside1; yMaxInside = yMaxInside1;
    } else {
        memcpy(coeffsPolynom1, coeffsPolynomPrev, (degreePolynom+1)*sizeof(float));
        memcpy(coeffsPolynom2, coeffsPolynom, (degreePolynom+1)*sizeof(float));
        yMinInside = yMinInside2; yMaxInside = yMaxInside2;
    }

    for (int y=yMinInside; y<=yMaxInside; y++){
        float xCT = CBDComputeFromPolynom((float) y, coeffsPolynom1, degreePolynom, yNorm);
        CTRef[y-yMinInside] = xCT;
    }


    // Test shifts
    float bestDiff = FLT_MAX;
#ifdef DISPLAY_DETAILS_DISTANCE_CBDS
    float bestdX, bestdY;
#endif
    for (float dY = -maxDeltaY; dY<=maxDeltaY; dY+=deltaSpatialStepY){

        // Testing shifted centerline
        memset(coeffsPolynomTested, 0, (degreePolynom+1)*sizeof(float));

        // Shifting by deltaY
        for (int i=degreePolynom; i>=0; i--){
            for (int j=0; j<=i; j++)
                coeffsPolynomTested[j] += CBD_Combination(j,i) * static_cast<float>(pow(dY/yNorm, i-j)) * coeffsPolynom2[i];
            // Resulting change over polynom coefficients
        }
        
        //int yMinInside2 = yMinInside, yMaxInside2 = yMaxInside;
        //if (yMaxInside - yMinInside > minLength){
        //    for (int y=yMinInside; y<=yMaxInside; y++){
        //        float xCT = ComputeFromPolynom((float) y, coeffsPolynomTested, degreePolynom, yNorm);
        //        if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) )
        //            break;
        //        yMinInside2++;
        //    }
        //    for (int y=yMaxInside; y>=yMinInside2; y--){
        //        float xCT = ComputeFromPolynom((float) y, coeffsPolynomTested, degreePolynom, yNorm);
        //        if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) )
        //            break;
        //        yMaxInside2--;
        //    }
        //    if (yMaxInside2 - yMinInside2 <= minLength){
        //        yMinInside2 = yMinInside2-(minLength-(yMaxInside2 - yMinInside2+1))/2;
        //        yMaxInside2 = yMaxInside2-(minLength-(yMaxInside2 - yMinInside2+1))/2;

        //        if (yMinInside2 < yMinInside){
        //            yMaxInside2 = OSC_MIN(yMaxInside,yMaxInside2+yMinInside-yMinInside2);
        //            yMinInside2 = yMinInside;
        //        }
        //        if (yMaxInside2 > yMaxInside){
        //            yMinInside2 = OSC_MAX(yMinInside,yMinInside2-(yMaxInside2-yMaxInside));
        //            yMaxInside2 = yMaxInside;
        //        }
        //    }
        //}

        // Shifted centermine
        for (int y=yMinInside; y<=yMaxInside; y++){
            float xCT = CBDComputeFromPolynom((float) y, coeffsPolynomTested, degreePolynom, yNorm);
            CTTest[y-yMinInside] = xCT;
        }
        
        // Distance between both
        for (float dX = -maxDeltaX; dX<=maxDeltaX; dX+=deltaSpatialStepX){
            float diff = 0.f;
            for (int i=0; i<=yMaxInside-yMinInside; i++)
                diff += CBD_ABS_F(CTRef[i]-CTTest[i]-dX);
            if (yMinInside == yMaxInside)
                diff = FLT_MAX;
            else
                diff /= (float)(yMaxInside-yMinInside+1);
#ifdef DISPLAY_DETAILS_DISTANCE_CBDS    
            if (diff < bestDiff){
                bestDiff = diff;
                bestdX = dX; bestdY = dY;
            }
#else
            bestDiff = CBD_MIN(bestDiff, diff); 
#endif
        }
    }
    bestDiff = bestDiff * isoPixSize;

#ifdef DISPLAY_DETAILS_DISTANCE_CBDS 
    MxOpen(); MxSetVisible(1);
    MxPutInt(yMin, "yMin"); MxPutInt(yMax, "yMax"); 
    MxPutInt(yMinInside, "yMinInside"); MxPutInt(yMaxInside, "yMaxInside"); 
    MxPutVectorFloat(CTRef, yMaxInside-yMinInside+1, "CTRef");

    for (int y=yMin; y<=yMax; y++){
        CTRef[y-yMin] = CBDComputeFromPolynom((float) y, coeffsPolynom1, degreePolynom, yNorm);
    }
    MxPutVectorFloat(CTRef, yMax-yMin+1, "CTRefFull");

    for (int y=yMin; y<=yMax; y++){
        CTTest[y-yMin] = CBDComputeFromPolynom((float) y, coeffsPolynom2, degreePolynom, yNorm);
    }
    MxPutVectorFloat(CTTest, yMax-yMin+1, "CTEstFull");

    // Testing shifted centerline
    memset(coeffsPolynomTested, 0, (degreePolynom+1)*sizeof(float));

    // Shifting by deltaY
    for (int i=degreePolynom; i>=0; i--){
        for (int j=0; j<=i; j++)
            coeffsPolynomTested[j] += CBD_Combination(j,i) * pow(bestdY/yNorm, i-j) * coeffsPolynom2[i];
    }
        
    for (int y=yMinInside; y<=yMaxInside; y++){
        float xCT = CBDComputeFromPolynom((float) y, coeffsPolynomTested, degreePolynom, yNorm)+bestdX;
        CTTest[y-yMinInside] = xCT;
    }
    MxPutVectorFloat(CTTest, yMaxInside-yMinInside+1, "CTEst");

    MxCommand("figure(4); subplot(1,1,1); imagesc(ImSub'); colormap gray; axis image; axis off;");
    MxCommand("hold on; plot(CTRefFull,yMin:yMax,'b-'); plot(CTRef(1:yMaxInside-yMinInside+1),yMinInside:yMaxInside,'r-');");
    MxCommand("plot(CTEstFull,yMin:yMax,'g-'); plot(CTEst(1:yMaxInside-yMinInside+1),yMinInside:yMaxInside,'m-'); hold off;");
#endif 


    free(CTRef); free(CTTest); free(coeffsPolynomTested); free(coeffsPolynom1); free(coeffsPolynom2);

    return bestDiff;
}

// *** Outputs X centerline corresponding to Y (in the referential of the potenial image)
float CBDGetCBDCenterlineX(CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults, float y){
    CCBD * This = (CCBD*)PtrCBDHdl->Pv;

    if (PtrCBDResults->detectedCath)
        return CBDComputeFromPolynom(y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);
    else
        return 0.f;
}

// **** Outputs (outside) radius in mm
float CBDGetRadius(CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults){
    CCBD * This = (CCBD*)PtrCBDHdl->Pv;

    if (PtrCBDResults->detectedCath)
        return This->CBD_radiusOutside ;
    else
        return 0.f;
}