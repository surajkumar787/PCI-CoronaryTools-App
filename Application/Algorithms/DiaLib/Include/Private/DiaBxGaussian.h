//Copyright (c)2014 Koninklijke Philips Electronics N.V.,
//All Rights Reserved.
//
//This source code and any compilation or derivative thereof is the
//proprietary information of Koninklijke Philips Electronics N.V.
//and is confidential in nature.
//Under no circumstances is this software to be combined with any Open Source
//Software in any way or placed under an Open Source License of any type 
//without the express written permission of Koninklijke Philips 
//Electronics N.V.

#ifndef		_DIABXGAUSSIAN_INCLUDE
#define		_DIABXGAUSSIAN_INCLUDE

short DiaBxFastRoundShort(double val);

int DiaBxIsoGaussFilterFloat2D(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int DiaBxAniGaussFilterFloat2D(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int DiaBxIsoGaussFilterFloat3D(float  * in, float * out, float  sigma, int dimx, int dimy,int dimz) ;
int DiaBxAniGaussFilterFloat3D(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int DiaBxIsoGaussFilterShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int DiaBxAniGaussFilterShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int DiaBxIsoGaussFilterShort3D(short  * in, short * out, float  sigma, int dimx, int dimy,int dimz) ;
int DiaBxAniGaussFilterShort3D(short  * in, short * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int DiaBxIsoGaussFilterFastShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int DiaBxAniGaussFilterFastShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;

int DiaBxIsoGaussFilterFloat2D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int DiaBxAniGaussFilterFloat2D_SSE(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int DiaBxIsoGaussFilterFloat3D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy, int dimz) ;
int DiaBxAniGaussFilterFloat3D_SSE(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

#endif
