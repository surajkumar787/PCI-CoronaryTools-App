// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef _BXWARP_INCLUDE
#define _BXWARP_INCLUDE

#include <BxCurve.h>

#define BXWARP_PIECEWISE_RIGID   0
#define BXWARP_PIECEWISE_SMOOTH  1
#define BXWARP_LOCALLY_RIGID     2

#define BXWARP_NEAREST_INTERPOL       0
#define BXWARP_LINEAR_INTERPOL        1
#define BXWARP_SHIFTED_INTERPOL       2
#define BXWARP_CUBIC_INTERPOL         3

typedef struct 
{
	float zoom          ;
	int   autoCenter    ;
	float scale         ;
	int   motionModel   ;
	int   interpolation ; 
	float maxDistance   ;

}   BxWarpParameters   ;

#ifdef __cplusplus
extern "C" {
#endif


int BxWarpDefaultParameters( BxWarpParameters * params ) ;

int BxWarpCreate         ( void** handle, BxCurve * warpCurve, BxWarpParameters * params, int dimx, int dimy)  ;
int BxWarpDelete         ( void* handle ) ;
int BxWarpChange         ( void* handle, BxCurve * warpCurve ) ;
int BxWarpDoWarp         ( void* handle, short * image, int dimx, int dimy, BxCurve * curve, short * warped ) ;
int BxWarpDoWarpFloat    ( void* handle, short * image, int dimx, int dimy, BxCurve * curve, float * warped ) ;
int BxWarpMotionField    ( void* handle, float * dx, float * dy, BxCurve * curve ) ;
int BxWarpGetDistanceMap ( void* handle, float * map ) ;
int BxWarpGetWarpCurve   ( void* handle, BxCurve * warpCurve ) ;

#ifdef __cplusplus
}
#endif

#endif