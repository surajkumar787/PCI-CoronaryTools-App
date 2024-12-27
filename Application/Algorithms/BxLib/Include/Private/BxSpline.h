// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_BXSPLINE_INCLUDE
#define		_BXSPLINE_INCLUDE

// Boundary conditions : 0 = Mirror, 1 = Anti-Mirror, 2 = Cyclic,  3 = Constant */
#define BXSPLINE_MIRROR        0       
#define BXSPLINE_ANTI_MIRROR   1       
#define BXSPLINE_CYCLIC        2
#define BXSPLINE_CONSTANT      3

#ifdef __cplusplus
extern "C" {
#endif

int BxDirectCubicSplineTransform       ( float * in, float * out, int size, int boundType         ) ;
int BxIndirectCubicSplineTransform     ( float * in, float * out, int size, int boundType, int m  ) ;
int BxDirectQuadraticSplineTransform   ( float * in, float * out, int size, int boundType         ) ;
int BxIndirectQuadraticSplineTransform ( float * in, float * out, int size, int boundType, int m  ) ;
int BxBoundaryExtension                ( float * in, float * out, int size, int boundType, int m  ) ;
int BxMovingAverageFilter              ( float * in, float * out, int m, int size, int shift      ) ;
int BxShiftLinearPreFilter2D           ( float * in, float * out, int dimx, int dimy, float shift ) ;

int BxLinearSplineInterpolation        ( float * in, float * out, int size, int m ) ;
int BxCubicSplineInterpolation         ( float * in, float * out, int size, int m, int boundType ) ;
int BxQuadraticSplineInterpolation     ( float * in, float * out, int size, int m, int boundType ) ;

int BxShiftedCubicCardinalSpline       ( float* cardinalSpline, int size, int m, int shift, int normalize, int boundType ) ; 

#ifdef __cplusplus
}
#endif

#endif