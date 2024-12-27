#include "CBDDev.h"

//#define DISPLAY_DETAILS_TUBEPARAMSEST
//#define DISPLAY_DETAILS_TUBEPARAMSEST_DETAILS

//#define CBD_TIMING_TEMPTRACKANDTUBEMODEL
//#define CBD_TIMING_TEMPTRACKANDTUBEMODEL_ADVANCED

#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL
#include "omp.h"
#endif
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL_ADVANCED
#include "omp.h"
#endif
#ifdef DISPLAY_DETAILS_TUBEPARAMSEST
#include "MxLab.h"
#endif
#ifdef DISPLAY_DETAILS_TUBEPARAMSEST_DETAILS
#include "MxLab.h"
#endif

// *** CBD parameters computation
int CBDTrackAndComputeTubeModel(short *Im, float *ImPot, int Iw, int Ih, int ShutterPos[4], float isocenterPixSize
                                , CCBDHdl * PtrCBDHdl, CCBDResults * PtrCBDResults){

    CCBD * This = (CCBD*)PtrCBDHdl->Pv;
    int Err;
    
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL
    double t0, t1, t2, t3;
	t0 = omp_get_wtime();
#endif

    if (PtrCBDResults->detectedCath == 1){
        // Has just been detected

        int degree = CBD_DEGREEFITTINGCONIQUE;
        float *coeffsPolynom = This->CoeffConiqueCTCBD;

        Err = CBDFitPolynom(This, PtrCBDResults->detectedTrack->X, PtrCBDResults->detectedTrack->Y, PtrCBDResults->detectedTrack->nbElts
                          , coeffsPolynom, degree, &This->CBDYNormFittingConique);
        if (Err) {
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDTrackAndComputeTubeModel", "CBDFitPolynom", 0, Err);
		    return Err;
        }

        PtrCBDResults->detectedCath = 2;
        This->CBDDistWithPrevious = -1.f;
    } else {
        if (PtrCBDResults->detectedCath >= 2){

            // Adapt CBD position
            memcpy(This->CoeffConiqueCTCBDPrev, This->CoeffConiqueCTCBD, (CBD_DEGREEFITTINGCONIQUE+1)*sizeof(float));

            Err = CBDAdaptCBDPosToNewFrame(This, ImPot, Iw, Ih, ShutterPos, isocenterPixSize, This->CoeffConiqueCTCBDPrev
                                , CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique, This->CoeffConiqueCTCBD);
            if (Err) {
		        ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDTrackAndComputeTubeModel", "CBDAdaptCBDPosToNewFrame", 0, Err);
		        return Err;
            }

            PtrCBDResults->detectedCath = 3;
        }
    }

    // Condition on orientation and cumLength
    if ( (PtrCBDResults->detectedCath > 0) && 
        (CBD_ValidateCBD(This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique, ShutterPos) < 0) ){
            PtrCBDResults->detectedCath = 0;
            return 0;
        }
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL
	t1 = omp_get_wtime();
#endif

//    // Condition on difference with previous
//    if (PtrCBDResults->detectedCath == 3){
//#ifdef DISPLAY_DETAILS_DISTANCE_CBDS
//        MxOpen(); MxSetVisible(1);
//        MxPutMatrixShort(Im, Iw, Ih,"ImSub");
//#endif
//        This->CBDDistWithPrevious = 
//            CBD_DistanceBetweenCenterlines(This->CoeffConiqueCTCBDPrev, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE
//                                            , This->CBDYNormFittingConique, ShutterPos, isocenterPixSize
//                                            , CBD_TUBE_MAXDPTX_UPDATE3 / isocenterPixSize, CBD_TUBE_MAXDPTX_UPDATE3 / isocenterPixSize);
//
//        if (This->CBDDistWithPrevious > CBD_DISTANCELIMIT_TOACCEPTCBD){
//            PtrCBDResults->detectedCath = 0;
//            return 0;
//        }
//    }
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL
	t2 = omp_get_wtime();
#endif

    // Estimate tube parameters
    if (PtrCBDResults->detectedCath > 0){
        int yMinHere, yMaxHere;
        yMinHere = ShutterPos[3];
        for (int y=ShutterPos[2]; y<=ShutterPos[3]; y++){
            float xCT = CBDComputeFromPolynom((float) y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);
            if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) ){
                yMinHere = y; break;
            }
        }
        yMaxHere = ShutterPos[2];
        for (int y=ShutterPos[3]; y>=ShutterPos[2]; y--){
            float xCT = CBDComputeFromPolynom((float) y, This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique);
            if ( (xCT >= ShutterPos[0]) && (xCT <= ShutterPos[1]) ){
                yMaxHere = y; break;
            }
        }

        float RMin = CBD_TUBE_RMIN, RMax = CBD_TUBE_RMAX, MinTubeThickness = CBD_TUBE_MIN_TUBE_THICKNESS
            , MaxTubeThickness = CBD_TUBE_MAX_TUBE_THICKNESS;
        if (PtrCBDResults->detectedCath == 3){
            RMin = This->CBD_radiusInside; RMax = This->CBD_radiusInside;
            MinTubeThickness = This->CBD_radiusOutside-This->CBD_radiusInside; MaxTubeThickness = MinTubeThickness;
        }

        return CBDEstimateCBDParametersfromProfile(This, Im, Iw, Ih, ShutterPos, isocenterPixSize
                                                    , This->CoeffConiqueCTCBD, CBD_DEGREEFITTINGCONIQUE, This->CBDYNormFittingConique
                                                    , RMin, RMax, MinTubeThickness, MaxTubeThickness
                                                    , CBD_TUBE_PRECISION, CBD_TUBE_MAXDELTAADJUSTMENT
                                                    , (float)yMinHere, (float)yMaxHere, CBD_MIN((int)(yMaxHere-yMinHere+1), CBD_TUBE_MAXNBSAMPLES_TUBEPARAMSCOMPUTATION)
                                                    , &This->CBD_radiusInside, &This->CBD_radiusOutside
                                                    , This->CBD_attenuationInside, This->CBD_attenuationTube
                                                    , This->CBD_contrastCPY, &This->CBD_nbEstimatedContrasts);

    }
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL
	t3 = omp_get_wtime();
    printf("Time to detect/track: %.1f ms, to compute error wrt previous %.1f ms, to tube parameters %.1f ms\n"
        , 1000.0 * (t1-t0), 1000.0 * (t2-t1), 1000.0 * (t3-t2));
#endif
    return 0;
}


// *** CBD parameters computation
// By model fitting over profiles
int CBDEstimateCBDParametersfromProfile(CCBD * This, short *Im, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                                        , float *coeffsPolynom, int degreePolynom, float yNorm
                                        , float RMin, float RMax, float minThicknessTube, float maxThicknessTube
                                        , float precisionMM, float deltaCentMax
                                        , float yMin, float yMax, int nbSamples
                                        , float *R1Out, float *R2Out, float *alpha1, float *alpha2, int *yCP, int *nbEstimatedContrasts){

    // * Compute profile along centerline
    float yStep = (yMax-yMin) / (nbSamples-1);
    float centerAnalysis = 2*(CBD_TUBE_RMAX+CBD_TUBE_MAX_TUBE_THICKNESS)+deltaCentMax;
    int nbBinsProfile = (int)ceil((2*centerAnalysis)/precisionMM);
    float *profile = (float*)calloc(nbBinsProfile, sizeof(float));
    int *nbEltProfile = (int*)calloc(nbBinsProfile, sizeof(int));
    float *profBkg = (float*)calloc(nbBinsProfile, sizeof(float));
    float *A = (float*)malloc(4*sizeof(float)), *B = (float*)malloc(2*sizeof(float));
    
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL_ADVANCED
    double t0, t1, t2, t3;
	t0 = omp_get_wtime();
#endif

    bool noElts;
    int Err = CBDComputeProfile(This, Im, Iw, Ih, ShutterPos, isoPixSize, coeffsPolynom, degreePolynom, yNorm, precisionMM
                              , (float) yMin, yStep, 0, nbSamples-1, centerAnalysis, nbBinsProfile, &noElts, profile, nbEltProfile, profBkg);
    if(Err)
	{
        free(profBkg); free(profile); free(nbEltProfile); free(A); free(B);
		ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDEstimateCBDParametersfromProfile", "CBDComputeProfile", 0, Err);
		return Err;
	}

    if (noElts){
        // Profile completely out of the ROI
        *nbEstimatedContrasts = 1; *R1Out = RMin; *R2Out = RMin; *alpha1 = 0; *alpha2 = 0; 
        free(A); free(B); free(profile); free(nbEltProfile); free(profBkg);
        return 0;
    }
        
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL_ADVANCED
	t1 = omp_get_wtime();
#endif

    // * Analyze profile
    float bestError = FLT_MAX;
    float bestDeltaCent = 0;

    // 1- First loop on all hypothesis - unitary contrast
    for (float R1 = RMin; R1 <= RMax; R1 += precisionMM)
        for (float R2 = R1+minThicknessTube; R2 <= R1+maxThicknessTube; R2 += precisionMM)
            for (float deltaCentTested = -deltaCentMax; deltaCentTested <= deltaCentMax; deltaCentTested += precisionMM){

                float contIn, contTube, errTested;

                CBDEstimateBestContrasts(profile, nbEltProfile, profBkg, R1, R2, centerAnalysis, deltaCentTested, precisionMM
                                    , A, B, &contIn, &contTube);
                errTested = CBDComputeProfileFittingError(profile, nbEltProfile, profBkg, R1, R2, centerAnalysis
                                                        , deltaCentTested, precisionMM, contIn, contTube);
                
                if (errTested < bestError){
                    bestError = errTested;
                    *R1Out = R1;  *R2Out = R2;
                    alpha1[0] = B[0]; alpha2[0] = B[1];
                    bestDeltaCent = deltaCentTested;
                }
            }
        
#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL_ADVANCED
	t2 = omp_get_wtime();
#endif

#ifdef DISPLAY_DETAILS_TUBEPARAMSEST
        MxOpen(); MxSetVisible(1);

        // Limits of the tested catheter on profile
        int indLimCBDMinus = CBD_RND((centerAnalysis+bestDeltaCent-*R2Out)/precisionMM);
        int indLimCBDPlus = CBD_RND((centerAnalysis+bestDeltaCent+*R2Out)/precisionMM);
        int lengthArea = (indLimCBDPlus-indLimCBDMinus+1);

        float * idealProf = (float*)malloc(nbBinsProfile*sizeof(float));
        for (int j=0; j<nbBinsProfile; j++)
            if (nbEltProfile[j] > 0){
                float d = j*precisionMM-(centerAnalysis+bestDeltaCent);
                idealProf[j] = profBkg[j]-(*alpha1)*sqrt( CBD_MAX(0.f,(*R1Out)*(*R1Out) - d*d))
                        - (*alpha2)*(sqrt(CBD_MAX(0.f,(*R2Out)*(*R2Out) - d*d))-sqrt(CBD_MAX(0.f,(*R1Out)*(*R1Out) - d*d)));
            } else 
                idealProf[j] = 0;

        MxPutVectorFloat(profile, nbBinsProfile, "profile");
        MxPutVectorFloat(profBkg, nbBinsProfile, "profBkg");
        MxPutVectorFloat(idealProf, nbBinsProfile, "idealProf");

        MxCommand("figure(2); subplot(1,1,1); plot(profile,'b-'); hold on; plot(profBkg,'g-'); plot(idealProf,'r-'); hold off");

        free(idealProf);
#endif

    // 2- radii and deltaCT fixed; reestimate local contrasts
    *nbEstimatedContrasts = (int)ceil( CBD_TUBE_MAXNBOFCONTRASTS * (yMax-yMin) / (float) (ShutterPos[3]-ShutterPos[2]) );
    if (*nbEstimatedContrasts > 1){
        for (int j=0; j<*nbEstimatedContrasts; j++){

            // Subprofile
            bool noElts;
            int Err = CBDComputeProfile(This, Im, Iw, Ih, ShutterPos, isoPixSize, coeffsPolynom, degreePolynom, yNorm, precisionMM
                                    , (float) yMin, yStep, j*nbSamples/(*nbEstimatedContrasts), (j+1)*nbSamples/(*nbEstimatedContrasts)-1
                                    , centerAnalysis, nbBinsProfile, &noElts, profile, nbEltProfile, profBkg);
            if(Err)
	        {
                free(profBkg); free(profile); free(nbEltProfile); free(A); free(B);
		        ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDEstimateCBDParametersfromProfile", "CBDComputeProfile", 0, Err);
		        return Err;
	        }

            if (noElts){
                // Profile completely out of the ROI
                alpha1[j] = -1; alpha2[j] = -1; continue;
            }

            // Contrast estimation
            CBDEstimateBestContrasts(profile, nbEltProfile, profBkg, *R1Out, *R2Out, centerAnalysis, bestDeltaCent, precisionMM
                                    , A, B, &(alpha1[j]), &(alpha2[j]));

            yCP[j] = (int) CBD_RND(yMin + j*(yMax-yMin)/(*nbEstimatedContrasts) + 0.5f * (yMax-yMin)/(*nbEstimatedContrasts));

#ifdef DISPLAY_DETAILS_TUBEPARAMSEST_DETAILS
                MxOpen(); MxSetVisible(1);

                // Limits of the tested catheter on profile
                int indLimCBDMinus = CBD_RND((centerAnalysis+bestDeltaCent-*R2Out)/precisionMM);
                int indLimCBDPlus = CBD_RND((centerAnalysis+bestDeltaCent+*R2Out)/precisionMM);
                int lengthArea = (indLimCBDPlus-indLimCBDMinus+1);

                float * idealProf = (float*)malloc(nbBinsProfile*sizeof(float));
                for (int k=0; k<nbBinsProfile; k++)
                    if (nbEltProfile[k] > 0){
                        float d = k*precisionMM-(centerAnalysis+bestDeltaCent);
                        idealProf[k] = profBkg[k]-alpha1[j]*sqrt( CBD_MAX(0.f,(*R1Out)*(*R1Out) - d*d))
                                - alpha2[j]*(sqrt(CBD_MAX(0.f,(*R2Out)*(*R2Out) - d*d))-sqrt(CBD_MAX(0.f,(*R1Out)*(*R1Out) - d*d)));
                    } else 
                        idealProf[k] = 0;

                MxPutVectorFloat(profile, nbBinsProfile, "profile");
                MxPutVectorFloat(profBkg, nbBinsProfile, "profBkg");
                MxPutVectorFloat(idealProf, nbBinsProfile, "idealProf");

                MxCommand("figure(2); subplot(1,1,1); plot(profile,'b-'); hold on; plot(profBkg,'g-'); plot(idealProf,'r-'); hold off");

                free(idealProf);
#endif
        }
    } else {
        yCP[0] = (int)CBD_RND(0.5f*(yMax-yMin));
    }

#ifdef CBD_TIMING_TEMPTRACKANDTUBEMODEL_ADVANCED
	t3 = omp_get_wtime();
    printf("Profile computation: %.1f ms, initial parameters %.1f ms, local contrasts %.1f ms\n"
        , 1000.0 * (t1-t0), 1000.0 * (t2-t1), 1000.0 * (t3-t2));
#endif

    // Centering centerline
    coeffsPolynom[0] += bestDeltaCent;

    free(profBkg); free(profile); free(nbEltProfile); free(A); free(B);

    return 0;
}

// *** Compute integrated profile along the centerline, and estimate profile background
int CBDComputeProfile(CCBD * This, short *Im, int Iw, int Ih, int ShutterPos[4], float isoPixSize
                        , float *coeffsPolynom, int degreePolynom, float yNorm, float precisionMM
                        , float yMinCT, float yStep, int firstSample, int lastSample
                        , float centerAnalysis, int nbBinsProfile, bool *noElts
                        , float *profile, int *nbEltProfile, float *profBkg){

    int nbElts = 0;

    memset(profile, 0, nbBinsProfile*sizeof(float));
    memset(nbEltProfile, 0, nbBinsProfile*sizeof(int));
                            
    for (int i=firstSample; i<=lastSample; i++){
        float y = yMinCT+i*yStep;
        float xCent = CBDComputeFromPolynom(y, coeffsPolynom, degreePolynom, yNorm);

        if ( (y<ShutterPos[2]) || (y>ShutterPos[3]) || (xCent<ShutterPos[0]) || (xCent>ShutterPos[1]) )
            continue;

        for (int j=0; j<nbBinsProfile; j++){
            float x = xCent + (-centerAnalysis + j * precisionMM) / isoPixSize;
            if ( (x>=ShutterPos[0]) && (x<=ShutterPos[1]) )
            {
                profile[j] += CBDInterpBilinear(Im, Iw, Ih, x, y);
                nbEltProfile[j] ++;
                nbElts++;
            }
        }
    }
    if (nbElts == 0){
        *noElts = true;
        return 0;
    } else {
        *noElts = false;

        for (int j=0; j<nbBinsProfile; j++)
            if (nbEltProfile[j] > 0)
                profile[j] /= nbEltProfile[j];

        return CBDEstimationBackgroundProfile(This, profile, nbEltProfile, nbBinsProfile, profBkg);
    }
}

// *** Estimate contrasts expliaing the difference profile/background
// Tube model and least squares
void CBDEstimateBestContrasts(float *profile, int *nbEltsProf, float *profBkg, float R1, float R2, float centerAnalysis, float deltaCent, float precisionMM
                           , float *A, float *B, float *contIn, float *contTube){

    // Limits of the tested catheter on profile
    int indLimCBDMinus = CBD_RND((centerAnalysis+deltaCent-R2)/precisionMM);
    int indLimCBDPlus = CBD_RND((centerAnalysis+deltaCent+R2)/precisionMM);
    int lengthArea = (indLimCBDPlus-indLimCBDMinus+1);

    // Contrasts estimations
    memset(A, 0, 4*sizeof(float)); memset(B, 0, 2*sizeof(float));
    for (int j=indLimCBDMinus; j<=indLimCBDPlus; j++)
        if (nbEltsProf[j] > 0){
            float d = j*precisionMM-(centerAnalysis+deltaCent);    // Dist to center

            float coeff1 = sqrt( CBD_MAX(0.f, R1*R1 - d*d) );
            float coeff2 = sqrt( CBD_MAX(0.f, R2*R2 - d*d) )-coeff1;

            A[0] += coeff1*coeff1; A[1] += coeff1 * coeff2;
            A[2] += coeff1 * coeff2; A[3] += coeff2 * coeff2;

            B[0] += coeff1 * (profBkg[j]-profile[j]);
            B[1] += coeff2 * (profBkg[j]-profile[j]);
        }
    int ret = CBDGaussNewton(A, B, 2);

    *contIn = B[0]; *contTube = B[1];
}

// *** Error between profile-background and tube model
float CBDComputeProfileFittingError(float *profile, int *nbEltProfile, float *profBkg, float R1, float R2, float centerAnalysis
                                 , float deltaCent, float precisionMM, float contIn, float contTube){

    // Limits of the tested catheter on profile
    int indLimCBDMinus = CBD_RND((centerAnalysis+deltaCent-R2)/precisionMM);
    int indLimCBDPlus = CBD_RND((centerAnalysis+deltaCent+R2)/precisionMM);
    int lengthArea = (indLimCBDPlus-indLimCBDMinus+1);

    // Error computation
    float errTested = 0.f;
    for (int j=indLimCBDMinus; j<=indLimCBDPlus; j++)
        if (nbEltProfile[j] > 0){
            float d = j*precisionMM-(centerAnalysis+deltaCent);
            float coeff1 = sqrt( CBD_MAX(0.f, R1*R1 - d*d) );
            float coeff2 = sqrt( CBD_MAX(0.f, R2*R2 - d*d) )-coeff1;
            errTested += static_cast<float>(pow ( contIn*coeff1 + contTube*coeff2-(profBkg[j]-profile[j]) ,2));
        }

    return (errTested / lengthArea);
}

// *** Background profile estimation
// Robust fit of polynom
int CBDEstimationBackgroundProfile(CCBD * This, float * prof, int *nbEltsProf, int nbElts, float *Bkg){

    int degreeBkg = CBD_TUBE_DEGREE_PROFILEBACKGROUND, nbRobIt = CBD_TUBE_NBROBUSTITERATIONS_PROFILEBACKGROUND, nbResidual;
    float *w = (float*)malloc(nbElts * sizeof(float))
        , *A = (float*)malloc((degreeBkg+1) * (degreeBkg+1) * sizeof(float))
        , *B = (float*)malloc((degreeBkg+1) * sizeof(float))
        , *residual = (float*)malloc(nbElts * sizeof(float))
        , *resList = (float*)malloc(nbElts * sizeof(float))
        , *buf = (float*)malloc(nbElts * sizeof(float));
    float idVal;

    if (w == NULL || A == NULL || B ==  NULL || residual == NULL ||resList == NULL || buf == NULL){
        free(w); free(buf); free(residual); free(resList); free(A), free(B);
        ErrStartErrorReportFromLeaf(This->ErrHdl, "CBDEstimationBackgroundProfile", CBD_ERR_MEM_ALLOC, "CBDEstimationBackgroundProfile: allocation error");
        return CBD_ERR_MEM_ALLOC;
    }

    for (int i=0;i<nbElts; i++)
        w[i] = 1.f;

    for (int it=0; it<nbRobIt; it++){

        // Fit background
        for (int k=0; k<degreeBkg+1; k++){
            for (int l=0; l<degreeBkg+1; l++){
                A[k*(degreeBkg+1)+l] = 0.f;
            }
            B[k] = 0.f;
        }
        for (int i=0; i<nbElts; i++)
            if (nbEltsProf[i] > 0){
                for (int k=0; k<degreeBkg+1; k++){
                    for (int l=0; l<degreeBkg+1; l++){
                        A[k*(degreeBkg+1)+l] += w[i]*(static_cast<float>(pow((float)i, k+l)));
                    }
                    B[k] += w[i]*prof[i]* static_cast<float>(pow((float)i, k));
                }
            }
        int ret = CBDGaussNewton(A, B, degreeBkg+1);

        // Update weights
        nbResidual = 0;
        for (int i=0; i<nbElts; i++)
            if (nbEltsProf[i] > 0){
                idVal = B[0];
                for (int k=1; k<=degreeBkg; k++)
                    idVal += B[k]* static_cast<float>(pow((float)i, k));
                residual[i] = CBD_ABS_F( idVal - prof[i] );
                resList[nbResidual] = residual[i];
                nbResidual ++;
            }
        if (nbResidual == 0){
            free(w); free(buf); free(residual); free(resList); free(A), free(B);
            return 0;
        }

        float resMed = 0.f;
        int Err = CBDMedian(This, resList, buf, 0, nbResidual/2, nbResidual-1, &resMed);
        if(Err)
	    {
		    ErrAppendErrorReportFromCaller(This->ErrHdl, "CBDEstimationBackgroundProfile", "CBDMedian", 0, Err);
		    return Err;
	    }
        float C = 1.48f * 2.795f * resMed;

        if (C == 0){
            for (int i=0; i<nbElts; i++)
                if (residual[i] == 0)
                    w[i] = 1.f;
                else
                    w[i] = 0.f;
        } else {    
            for (int i=0; i<nbElts; i++)
                if (nbEltsProf[i] > 0){
                    idVal = B[0];
                    for (int k=1; k<=degreeBkg; k++)
                        idVal += B[k]* static_cast<float>(pow((float)i, k));
                    w[i] = CBDDerivativeTukey( idVal - prof[i], C); // res/c DerivativeTukey(residu, 1) / residu;
                }
        }

        //{
        //    MxOpen(); MxSetVisible(1);
        //    for (int i=0; i<nbElts; i++)
        //        if (nbEltsProf[i] > 0){
        //        Bkg[i] = B[0];
        //        for (int k=1; k<=degreeBkg; k++)
        //            Bkg[i] += B[k]*pow((float)i, k);
        //    }
        //    MxPutVectorFloat(prof, nbElts, "prof");
        //    MxPutVectorFloat(Bkg, nbElts, "Bkg");
        //    MxPutVectorFloat(residual, nbElts, "residual");
        //    MxPutVectorFloat(w, nbElts, "w");
        //    MxCommand("figure(3); plot(prof,'b-'); hold on;  plot(Bkg,'g-'); hold off;");
        //}
    }
    
    // Output
    for (int i=0; i<nbElts; i++)
        if (nbEltsProf[i] > 0){
            Bkg[i] = B[0];
            for (int k=1; k<=degreeBkg; k++)
                Bkg[i] += B[k]* static_cast<float>(pow((float)i, k));
        } else {
            Bkg[i] = 0;
        }


    free(w); free(buf); free(residual); free(resList); free(A), free(B);

    return 0;
}