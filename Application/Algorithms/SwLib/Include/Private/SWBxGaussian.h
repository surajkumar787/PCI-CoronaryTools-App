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

int SWBxIsoGaussFilterFloat2D(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int SWBxAniGaussFilterFloat2DROI(float * in, float* out, float sigmax, float sigmay, int dimx, int dimy, int xMin, int xMax, int yMin, int yMax);
int SWBxAniGaussFilterFloat2D(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int SWBxIsoGaussFilterFloat2DROI(float * in, float* out, float sigma, int dimx, int dimy, int xMin, int xMax, int yMin, int yMax);
int SWBxIsoGaussFilterFloat3D(float  * in, float * out, float  sigma, int dimx, int dimy,int dimz) ;
int SWBxAniGaussFilterFloat3D(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

int SWBxIsoGaussFilterShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int SWBxIsoGaussFilterShort2DROI(short * in, short* out, float sigma, int dimx, int dimy, int xMin, int xMax, int yMin, int yMax);
int SWBxAniGaussFilterShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int SWBxAniGaussFilterShort2DROI(short * in, short* out, float sigmax, float sigmay, int dimx, int dimy, int xMin, int xMax, int yMin, int yMax);
int SWBxIsoGaussFilterShort3D(short  * in, short * out, float  sigma, int dimx, int dimy,int dimz) ;
int SWBxAniGaussFilterShort3D(short  * in, short * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;
int SWBxAniGaussFilterShort3DInROI(short * in, short* out, float sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz, unsigned char *RoiMask);

int SWBxIsoGaussFilterFastShort2D(short  * in, short * out, float  sigma, int dimx, int dimy) ;
int SWBxAniGaussFilterFastShort2D(short  * in, short * out, float  sigmax, float sigmay, int dimx, int dimy) ;

int SWBxIsoGaussFilterFloat2D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy) ;
int SWBxAniGaussFilterFloat2D_SSE(float  * in, float * out, float  sigmax, float sigmay, int dimx, int dimy) ;
int SWBxIsoGaussFilterFloat3D_SSE(float  * in, float * out, float  sigma, int dimx, int dimy, int dimz) ;
int SWBxAniGaussFilterFloat3D_SSE(float  * in, float * out, float  sigmax, float sigmay, float sigmaz, int dimx, int dimy, int dimz) ;

#ifdef __cplusplus
	}
#endif

#endif



