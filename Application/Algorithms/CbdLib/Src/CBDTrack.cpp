#include "CBDDev.h"

//#define DISPLAY_DETAILS_ADAPT2
//#define DISPLAY_DETAILS_ADAPT2_DETAILS

#ifdef DISPLAY_DETAILS_ADAPT2
#include "Mxlab.h"
#endif
#ifdef DISPLAY_DETAILS_ADAPT2_DETAILS
#include "Mxlab.h"
#endif

// *** Moving centerline to new frame
// Locally search local max along centerline, and then robustly fit polynom over it

int CBDAdaptCBDPosToNewFrame(CCBD * This, float *ImPot, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                            , float *coeffsPolynomPrev, int degreePolynom, float yNorm, float *coeffsPolynomOut){

    int yMin = ShutterPos[2], yMax = ShutterPos[3];
    float yStep = (float)(yMax-yMin+1) / CBD_TRACK_NBSAMPLESADAPTATION;
    float * profOut = (float*)malloc(CBD_TRACK_SIZEPROFILE_ADAPTATION*sizeof(float)), 
        * profBuf = (float*)malloc(CBD_TRACK_SIZEPROFILE_ADAPTATION*sizeof(float));
    float * correspondingX = (float*)malloc(CBD_TRACK_SIZEPROFILE_ADAPTATION*sizeof(float));
    float *candidateX = (float*)malloc(CBD_TRACK_NBSAMPLESADAPTATION*CBD_TRACK_SIZEPROFILE_ADAPTATION*sizeof(float))
        , *candidateY = (float*)malloc(CBD_TRACK_NBSAMPLESADAPTATION*CBD_TRACK_SIZEPROFILE_ADAPTATION*sizeof(float));
    float *weights= (float*)malloc(CBD_TRACK_NBSAMPLESADAPTATION*CBD_TRACK_SIZEPROFILE_ADAPTATION*sizeof(float));

    // * Extract candidates from buffer
    int nbPts = 0;
    for (float y=(float)yMin; y<=(float)yMax; y+= yStep){

        float xPrev = CBDComputeFromPolynom(y, coeffsPolynomPrev, degreePolynom, yNorm);

        if ( (xPrev < ShutterPos[0]) || (xPrev > ShutterPos[1]) )
            continue;

        // Local profile (given Y), smoothed
        CBDFilteredImProfileY(ImPot, Iw, Ih, y, xPrev-CBD_TUBE_MAXDPTX_UPDATE2 / isoPixSize, xPrev+CBD_TUBE_MAXDPTX_UPDATE2 / isoPixSize
                            , CBD_TRACK_SIZEPROFILE_ADAPTATION, profOut, correspondingX, profBuf);

        // Global max
        float maxProf = 0.f;
        for (int i=0; i<CBD_TRACK_SIZEPROFILE_ADAPTATION; i++)
            maxProf = CBD_MAX(maxProf, profOut[i]);

        // Local max (and condition relative to global max)
        for (int i=1; i<CBD_TRACK_SIZEPROFILE_ADAPTATION-1; i++)
            if ( (profOut[i] > CBD_TRACK_ADAPTATION_RATIO_MAXPROF*maxProf) && (profOut[i] >profOut[i-1]) && (profOut[i] >profOut[i+1]) ){
                candidateX[nbPts] = correspondingX[i]; candidateY[nbPts] = y; //valProfile[nbPts] = profOut[i]; 
                nbPts ++;
            }
    }

#ifdef DISPLAY_DETAILS_ADAPT2
    // Display
    MxOpen(); MxSetVisible(1);
    MxPutMatrixFloat(ImPot, Iw, Ih, "ImPot");
    MxPutVectorFloat(candidateX, nbPts, "XCP"); MxPutVectorFloat(candidateY, nbPts, "YCP");
    float * xC = (float*)malloc((yMax-yMin+1)*sizeof(float));
    float * yC = (float*)malloc((yMax-yMin+1)*sizeof(float));
    for (int y=yMin; y<=yMax; y++){
        xC[y-yMin] = CBDComputeFromPolynom((float)y, coeffsPolynomPrev, degreePolynom, yNorm);
        yC[y-yMin] = (float)y;
    }
    MxPutVectorFloat(xC, yMax-yMin+1, "xC"); MxPutVectorFloat(yC, yMax-yMin+1, "yC");
    MxCommand("figure(1); subplot('Position',[0,0,0.5,1]); imagesc(ImPot'); colormap gray; axis image; axis off;");
    MxCommand("hold on; plot(XCP+1, YCP+1, 'rx'); hold off;");
    MxCommand("figure(1); subplot('Position',[0.5,0,0.5,1]); imagesc(ImPot'); colormap gray; axis image; axis off;");
    MxCommand("hold on; plot(xC+1, yC+1, 'b-'); plot(XCP+1, YCP+1, 'rx'); hold off;");
#endif

    // Robustly fit polynom
    for (int i=0; i<nbPts; i++)
        weights[i] = 1.f;
    memcpy(coeffsPolynomOut, coeffsPolynomPrev, (degreePolynom+1)*sizeof(float));
    for (int it = 0; it <CBD_TRACK_ADAPTATION_NBROBUSTITERATIONS; it++){

        int Err = CBDUpdateRobustWeights(This, candidateX, candidateY, weights, nbPts, coeffsPolynomOut, degreePolynom, yNorm);
        if(Err)
	    {
            free(profOut); free(profBuf); free(correspondingX); free(candidateX); free(candidateY); free(weights);
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDAdaptCBDPosToNewFrame", "CBDUpdateRobustWeights", 0, Err);
		    return Err;
	    }

        Err = CBDFitPolynomWithWeights(This, candidateX, candidateY, weights, nbPts, coeffsPolynomOut, degreePolynom, yNorm);
        if (Err)
	    {
            free(profOut); free(profBuf); free(correspondingX); free(candidateX); free(candidateY); free(weights);
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDAdaptCBDPosToNewFrame", "CBDFitPolynomWithWeights", 0, Err);
		    return Err;
	    }
        
#ifdef DISPLAY_DETAILS_ADAPT2_DETAILS
        // Display
        for (int y=yMin; y<=yMax; y++)
            xC[y-yMin] = CBDComputeFromPolynom((float)y, coeffsPolynomOut, degreePolynom, yNorm);
        MxPutVectorFloat(xC, yMax-yMin+1, "xC"); MxPutVectorFloat(yC, yMax-yMin+1, "yC");  
        MxPutVectorFloat(weights, nbPts, "w");
        MxCommand("figure(1); subplot('Position',[0.5,0,0.5,1]); imagesc(ImPot'); colormap gray; axis image; axis off;");
        MxCommand("hold on; plot(xC+1, yC+1, 'g-'); plot(XCP+1, YCP+1, 'rx'); hold off;");
#endif
    }
#ifdef DISPLAY_DETAILS_ADAPT2
    for (int y=yMin; y<=yMax; y++)
        xC[y-yMin] = CBDComputeFromPolynom((float)y, coeffsPolynomOut, degreePolynom, yNorm);
    MxPutVectorFloat(xC, yMax-yMin+1, "xC");
    MxCommand("hold on; plot(xC+1, yC+1, 'g-'); hold off;");
#endif

    free(profOut); free(profBuf); free(correspondingX); free(candidateX); free(candidateY); free(weights); //free(valProfile);

    return 0;
}

// ***********************************************************************************************
// Build profile (interpolations) and smooth it
void CBDFilteredImProfileY(float *ImPot, int Iw, int Ih, float y, float xMin, float xMax, int nbElts
                            , float *profOut, float *corrX, float *profBuf){

    float xStep = (xMax-xMin) / (nbElts-1);
    for (int i=0; i<nbElts; i++){
        corrX[i] = xMin+i*xStep;
        profBuf[i] = CBDInterpBilinearF(ImPot, Iw, Ih, xMin+i*xStep, y);
    }

    memset(profOut, 0, nbElts*sizeof(float));
    float coeffs[3] = {0.5f, 1.f, 0.5f};
    for (int i=0; i<nbElts; i++){
        float normCoeff = 0.f;
        for (int j=CBD_MAX(i-1,0); j<=CBD_MIN(i+1,nbElts-1); j++){
            profOut[i] += profBuf[j]*coeffs[j-i+1];
            normCoeff += coeffs[j-i+1];
        }
        profOut[i] /= normCoeff;
    }

    //MxOpen(); MxSetVisible(1);
    //MxPutMatrixFloat(ImPot, Iw, Ih, "ImPot");
    //MxPutVectorFloat(corrX, CBD_TUBE_SIZEPROFILE_ADAPTATION, "corrX");
    //MxPutFloat(y,"y");
    //MxPutVectorFloat(profBuf, CBD_TUBE_SIZEPROFILE_ADAPTATION, "profRaw");
    //MxPutVectorFloat(profOut, CBD_TUBE_SIZEPROFILE_ADAPTATION, "profFiltered");
    //MxCommand("figure(2); subplot('Position',[0,0,0.5,1]); imagesc(ImPot'); colormap gray; axis image; axis off;");
    //MxCommand("hold on; plot(corrX, y*ones(1,length(corrX)), 'bx');");
    //MxCommand("figure(2); subplot('Position',[0.5,0,0.5,1]); plot(corrX, profRaw,'b-'); hold on; plot(corrX, profFiltered,'r-'); hold off;");
}

// ****************************************************************************************************
// Fit polynom over the control points x = f(y)
// By least squares
int CBDFitPolynom(CCBD * This, float *XCPIn, float *YCPIn, int nbCPIn, float *coeffsPolynom, int degreePolynom, float *yNorm){

    float *A = (float*)malloc((degreePolynom+1)*(degreePolynom+1)*sizeof(float));
    if (A == NULL){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDFitPolynom", CBD_ERR_MEM_ALLOC, "CBDFitPolynom: Could not allocate A");
        return CBD_ERR_MEM_ALLOC;
    }

    *yNorm = YCPIn[nbCPIn/2];

    // Establishing system
    memset(A, 0, (degreePolynom+1)*(degreePolynom+1)*sizeof(float));
    memset(coeffsPolynom, 0, (degreePolynom+1)*sizeof(float));

    for (int i=0; i<nbCPIn; i++){
        for (int k=0; k<degreePolynom+1; k++){
            for (int l=0; l<degreePolynom+1; l++){
                A[k*(degreePolynom+1)+l] += static_cast<float>(pow(YCPIn[i]/(*yNorm), k+l));
            }
            coeffsPolynom[k] += XCPIn[i] * static_cast<float>(pow(YCPIn[i]/(*yNorm), k));
        }
    }

    // Solving the system
    if (CBDGaussNewton(A, coeffsPolynom, degreePolynom+1) < 0){
        free(A);
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDFitPolynom", CBD_ERR_INTERNALERROR, "CBDGaussNewton: Ill conditionned");
        return CBD_ERR_INTERNALERROR;
    }

    free(A);
    return 0;
}

// *** Fit polynom over the control points x = f(y), when control points associated with weights
int CBDFitPolynomWithWeights(CCBD * This, float *XCPIn, float *YCPIn, float *w, int nbCPIn, float *coeffsPolynom, int degreePolynom, float yNorm){

    float *A = (float*)malloc((degreePolynom+1)*(degreePolynom+1)*sizeof(float));
    if (A == NULL){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDFitPolynomWithWeights", CBD_ERR_MEM_ALLOC, "CBDFitPolynom: Could not allocate A");
        return CBD_ERR_MEM_ALLOC;
    }

    // Establishing system
    memset(A, 0, (degreePolynom+1)*(degreePolynom+1)*sizeof(float));
    memset(coeffsPolynom, 0, (degreePolynom+1)*sizeof(float));

    for (int i=0; i<nbCPIn; i++){
        for (int k=0; k<degreePolynom+1; k++){
            for (int l=0; l<degreePolynom+1; l++){
                A[k*(degreePolynom+1)+l] += w[i]* static_cast<float>(pow(YCPIn[i]/(yNorm), k+l));
            }
            coeffsPolynom[k] += w[i]*XCPIn[i] * static_cast<float>(pow(YCPIn[i]/(yNorm), k));
        }
    }

    // Solving the system
    if (CBDGaussNewton(A, coeffsPolynom, degreePolynom+1) < 0){
        free(A);
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDFitPolynomWithWeights", CBD_ERR_INTERNALERROR, "CBDGaussNewton: Ill conditionned");
        return CBD_ERR_INTERNALERROR;
    }
    free(A);
    return 0;
}

// *** Update weights
// With tukey function on residuals, normalized by median values (and minimum median value to avoid overfitting)
int CBDUpdateRobustWeights(CCBD * This, float *XCPIn, float *YCPIn, float *w, int nbCPIn, float *coeffsPolynom, int degreePolynom, float yNorm){

    float *residual = (float*)malloc(nbCPIn*sizeof(float)), * buf = (float*)malloc(nbCPIn*sizeof(float));

    if (residual == NULL || buf == NULL){
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDUpdateRobustWeights", CBD_ERR_MEM_ALLOC, "CBDUpdateRobustWeights: could not allocate residual or buf");
        return CBD_ERR_MEM_ALLOC;
    }

    for (int i=0; i<nbCPIn; i++){
        residual[i] = CBD_ABS_F( CBDComputeFromPolynom(YCPIn[i], coeffsPolynom, degreePolynom, yNorm) - XCPIn[i] );
    }

    float resMed; 
    int Err = CBDMedian(This, residual, buf, 0, nbCPIn/2, nbCPIn-1, &resMed);
    if(Err)
	{
        free(residual); free(buf);
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDUpdateRobustWeights", "CBDMedian", 0, Err);
		return Err;
	}

    float C = 1.48f * 2.795f * CBD_MAX(CBD_TRACK_MIN_DIST_FOR_CENTERLINE_WEIGHTS_PIX,resMed);   // To avoid overfitting
    free(residual); free(buf);

    if (C == 0){
        for (int i=0; i<nbCPIn; i++)
            if (residual[i] == 0)
                w[i] = 1.f;
            else
                w[i] = 0.f;
    } else {    
        for (int i=0; i<nbCPIn; i++){
            w[i] = CBDDerivativeTukey( CBDComputeFromPolynom(YCPIn[i], coeffsPolynom, degreePolynom, yNorm) - XCPIn[i], C); // res/c DerivativeTukey(residu, 1) / residu;
        }
    }

    return 0;
}