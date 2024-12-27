// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#ifndef		_BXGAUSSIAN_INCLUDE
#define		_BXGAUSSIAN_INCLUDE

int BxIsoGaussFilterFloat2D(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterFloat2D(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int BxIsoGaussFilterFloat3D(float  * in, float * out, float  sigma, int dimx, int dimy,int dimz) ;
int BxAniGaussFilterFloat3D(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int BxIsoGaussFilterShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int BxIsoGaussFilterShort3D(short  * in, short * out, float  sigma, int dimx, int dimy,int dimz) ;
int BxAniGaussFilterShort3D(short  * in, short * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int BxIsoGaussFilterFastShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterFastShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;

int BxIsoGaussFilterFloat2D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterFloat2D_SSE(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int BxIsoGaussFilterFloat3D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy, int dimz) ;
int BxAniGaussFilterFloat3D_SSE(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

#endif



