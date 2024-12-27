// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_BXWIRE_INCLUDE
#define		_BXWIRE_INCLUDE

#include <BxCurve.h>

typedef enum {
	None = 0,
	Last,
	Manual,
	Recursive,
} BxWireTrackingMode ;

typedef struct {

	float    ExpectedContrast       ;
	float    ContrastDeadZone       ; 
	float    SimilarityWeight       ;
	float    OutputResolution       ;
	int      InternalSampling       ;
	float    InternalFieldOfView    ;
	int      InternalNSearch        ;
	int      InitSearch             ;

	BxWireTrackingMode TrackingMode ; 

} BxWireParameters ;


#ifdef __cplusplus
extern "C" {
#endif

int BxWireDefaultParameters ( BxWireParameters * params, int nBitsDyn ) ; 

int BxWireCreate    ( void** handle, BxWireParameters * params ) ; 
int BxWireDelete    ( void* handle ) ;
int BxWireDoExtract ( void* handle, BxCurve* wire, short* image, int dimx, int dimy, float x1, float y1, float x2, float y2 ) ;
int BxWireContrast  ( void* handle, BxCurve* wire, short* image, int dimx, int dimy, float* contrast);
int BxWireSetModel  ( void* handle, BxCurve* wire ) ; 

int BxWireGrayLevelBasedRefinement(void* handle, int warpHandle, BxCurve * wire , float * refImage, short * image, int dimx, int dimy, int halfNSearch, float halfSearchRange, int nIterations) ;

#ifdef __cplusplus
}
#endif


#endif



