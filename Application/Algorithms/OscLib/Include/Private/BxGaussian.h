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


#ifndef		_BXGAUSSIAN_INCLUDE
#define		_BXGAUSSIAN_INCLUDE

#ifdef __cplusplus
    extern "C" {
#endif

int BxIsoGaussFilterFloat2D(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterFloat2D(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int BxIsoGaussFilterFloat3D(float  * in, float * out, float  sigma, int dimx, int dimy,int dimz) ;
int BxAniGaussFilterFloat3D(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int BxIsoGaussFilterShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int BxIsoGaussFilterShort2DROI(short * in, short* out, float sigma, int dimx, int dimy, int xMin, int xMax, int yMin, int yMax);
int BxAniGaussFilterShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int BxAniGaussFilterShort2DROI(short * in, short* out, float sigmax, float sigmay, int dimx, int dimy, int xMin, int xMax, int yMin, int yMax);
int BxIsoGaussFilterShort3D(short  * in, short * out, float  sigma, int dimx, int dimy,int dimz) ;
int BxAniGaussFilterShort3D(short  * in, short * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;
int BxAniGaussFilterShort3DInROI(short * in, short* out, float sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz, unsigned char *RoiMask);

int BxIsoGaussFilterFastShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterFastShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;

int BxIsoGaussFilterFloat2D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int BxAniGaussFilterFloat2D_SSE(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int BxIsoGaussFilterFloat3D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy, int dimz) ;
int BxAniGaussFilterFloat3D_SSE(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

#ifdef __cplusplus
	}
#endif

#endif



