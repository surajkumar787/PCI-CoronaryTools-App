// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************

#ifndef		_RDGGAUSSIAN_INCLUDE
#define		_RDGGAUSSIAN_INCLUDE

int RdgIsoGaussFilterFloat2D(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int RdgAniGaussFilterFloat2D(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int RdgIsoGaussFilterFloat3D(float  * in, float * out, float  sigma, int dimx, int dimy,int dimz) ;
int RdgAniGaussFilterFloat3D(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int RdgIsoGaussFilterShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int RdgAniGaussFilterShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int RdgIsoGaussFilterShort3D(short  * in, short * out, float  sigma, int dimx, int dimy,int dimz) ;
int RdgAniGaussFilterShort3D(short  * in, short * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int RdgIsoGaussFilterFastShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int RdgAniGaussFilterFastShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;

int RdgIsoGaussFilterFloat2D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int RdgAniGaussFilterFloat2D_SSE(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int RdgIsoGaussFilterFloat3D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy, int dimz) ;
int RdgAniGaussFilterFloat3D_SSE(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

#endif
