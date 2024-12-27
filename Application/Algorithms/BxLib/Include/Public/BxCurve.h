// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


// Disable warning in bxcurve.h, because algorithm does alignment on purpose
#pragma warning ( disable : 4324 )

#ifndef _BXCURVE_INCLUDE
#define _BXCURVE_INCLUDE

#define BXCURVE_MAX_NPOINTS          1024
#define BXCURVE_MAX_LANDMARKS        64
#define BXCURVE_NEAREST_INTERPOL     0
#define BXCURVE_LINEAR_INTERPOL      1

#define BXCURVE_ALIGNED __declspec(align(16))

typedef struct
{
	BXCURVE_ALIGNED int   npoints                           ;
	BXCURVE_ALIGNED float x        [BXCURVE_MAX_NPOINTS]    ;
	BXCURVE_ALIGNED float y        [BXCURVE_MAX_NPOINTS]    ;
	BXCURVE_ALIGNED float dx       [BXCURVE_MAX_NPOINTS]    ;
	BXCURVE_ALIGNED float dy       [BXCURVE_MAX_NPOINTS]    ;
	BXCURVE_ALIGNED float ds       [BXCURVE_MAX_NPOINTS]    ;
	BXCURVE_ALIGNED float curvature[BXCURVE_MAX_NPOINTS]    ;
	BXCURVE_ALIGNED float length                            ;
	BXCURVE_ALIGNED int   nlandmarks                        ;
	BXCURVE_ALIGNED int   slandmark[BXCURVE_MAX_LANDMARKS]  ;

} BxCurve ;

#ifdef __cplusplus
extern "C" {
#endif

int BxCurveSwap(BxCurve* curve); 

int BxCurveInit                            (BxCurve* curve, float* x, float* y, int npoints) ;
int BxCurveInitOpenSpline                  (BxCurve* curve, float* xknot, float* yknot, int nknots, int sampling) ; 
int BxCurveInitClosedSpline                (BxCurve* curve, float* xknot, float* yknot, int nknots, int sampling) ; 
int BxCurveCopy                            (BxCurve* curve, BxCurve* newcurve) ;
int BxCurveTranslate                       (BxCurve* curve, float tx, float ty) ;
int BxCurveCentroid                        (BxCurve* curve, float* xmean, float* ymean) ;
int BxCurveCenterInImage                   (BxCurve* curve, int dimx, int dimy);
int BxCurveZoom                            (BxCurve* curve, float zoom, float xcenter, float ycenter) ; 
int BxCurveUpsampleCoordinates             (BxCurve* curve, int upsample) ; 
int BxCurveDownsampleCoordinates           (BxCurve* curve, int downsample) ; 
int BxCurveAffineTransform                 (BxCurve* curve, float xc, float yc, float a, float b, float c, float d, float e, float f) ;
int BxCurveMarkersTransform                (BxCurve* curve, float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22) ;
int BxCurveNormal                          (BxCurve* curve, int s, float range, float* xn, float* yn, int npoints) ;
int BxCurveIntegral                        (BxCurve* curve, float* image, int dimx, int dimy, float* integral, int interpol) ;
int BxCurveProfile                         (BxCurve* curve, float* image, int dimx, int dimy, float* profile,  int interpol) ;
int BxCurveNormalProfile                   (BxCurve* curve, short* image, int dimx, int dimy, int s,  float* profile, int interpol, float range, int npoints) ;
int BxCurveEdgeContrast                    (BxCurve* curve, short* image, int dimx, int dimy, float* contrast, float inbandsize, float deadzone, float outbandsize, float maxContrast, int interpol) ;
int BxCurveRidgeContrast                   (BxCurve* curve, short* image, int dimx, int dimy, float* contrast, float cc, float dz, float bk, float maxContrast, int nonLinear, int interpol) ;
int BxCurveAddLandmark                     (BxCurve* curve, int s) ;
int BxCurveAddLandmarkClosestTo            (BxCurve* curve, float x, float y) ;
int BxCurveRemoveAllLandmarks              (BxCurve* curve) ;
int BxCurveInflexionPoints                 (BxCurve* curve, int * sInflex, int * nInflex) ;
int BxCurveCenter                          (BxCurve* curve, float * xCenter, float * yCenter)  ;
int BxCurvePointWisePairingLinear          (BxCurve* curve, BxCurve* reference, int * pairing) ;
int BxCurvePointWisePairingPiecewiseLinear (BxCurve* curve, BxCurve* reference, int * pairing) ;
int BxCurveUnfold                          (BxCurve* curve) ;
int BxCurveTruncate                        (BxCurve* curve, BxCurve* newcurve, int first, int last) ;
int BxCurveTrim                            (BxCurve* curve, int trim) ;
int BxCurveHausdorffDistance               (BxCurve* c1,    BxCurve* c2, float * distance) ;


#ifdef __cplusplus
}
#endif

#endif