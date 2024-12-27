// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <BxWarp.h>
#include <BxSpline.h>
#include <BxGaussian.h>
#include <float.h>

#define BXWARP_SHIFTED_INTERPOL_VALUE 0.18f

#define BXWARP_DEFAULT_AUTOCENTER    0
#define BXWARP_DEFAULT_ZOOM          1.f
#define BXWARP_DEFAULT_SCALE_FACTOR  0.04f
#define BXWARP_DEFAULT_MODEL         BXWARP_PIECEWISE_SMOOTH
#define BXWARP_DEFAULT_INTERPOLATION BXWARP_LINEAR_INTERPOL 
#define BXWARP_DEFAULT_MAX_DISTANCE  FLT_MAX

#define BXWARP_MAX_NWINDOWS          48
#define BXWARP_WINDOW_SPACING        2
#define BXWARP_QUICK_CAST_METHOD     0

typedef struct {

	int      dimx          ;
	int      dimy          ; 
	float    zoom          ; 
	int      autoCenter    ; 
	float    scale         ;
	int      motionModel   ;
	int      interpolation ; 
	float    maxDistance   ; 

	int*     voronoi       ;
	float*   distance      ;
	float*   buffer        ;
	float*   dx            ;
	float*   dy            ;
	
	BxCurve        curve       ;

}   BxWarp ;

int BxWarpDefaultParameters( BxWarpParameters * params ) 
{
	params->autoCenter    = BXWARP_DEFAULT_AUTOCENTER    ;
	params->zoom          = BXWARP_DEFAULT_ZOOM          ;
	params->scale         = BXWARP_DEFAULT_SCALE_FACTOR  ;
	params->motionModel   = BXWARP_DEFAULT_MODEL         ;
	params->interpolation = BXWARP_DEFAULT_INTERPOLATION ; 
	params->maxDistance   = BXWARP_DEFAULT_MAX_DISTANCE  ;
	return 0 ;
}

int BxWarpCreate( void** handle, BxCurve * warpCurve, BxWarpParameters * params, int dimx, int dimy)
{
	BxWarp * that = (BxWarp*) malloc( sizeof(BxWarp) ) ; 

	(*handle) = (void*) that ; 

	that->dimx          = dimx ;
	that->dimy          = dimy ; 
	that->zoom          = params->zoom ; 
	that->autoCenter    = params->autoCenter ;
	that->scale         = params->scale * (dimx + dimy) / 2 ;
	that->motionModel   = params->motionModel ;
	that->interpolation = params->interpolation ; 
	that->maxDistance   = params->maxDistance ; 

	that->voronoi     = (int*)   malloc(dimx*dimy*sizeof(int)) ; 
	that->distance    = (float*) malloc(dimx*dimy*sizeof(float)) ; 
	that->buffer      = (float*) malloc(dimx*dimy*sizeof(float)) ; 
	that->dx          = (float*) malloc(dimx*dimy*sizeof(float)) ; 
	that->dy          = (float*) malloc(dimx*dimy*sizeof(float)) ; 
	
	return BxWarpChange((void*)that,warpCurve) ;
}

int BxWarpDelete( void* handle )
{
	BxWarp * that = (BxWarp*) handle ;

	if (handle > 0)
	{
		if (that->voronoi != NULL)
			free(that->voronoi) ; 

		if (that->distance != NULL)
			free(that->distance) ; 

		if (that->buffer != NULL)
			free(that->buffer) ; 

		if (that->dx != NULL)
			free(that->dx) ; 
		
		if (that->dy != NULL)
			free(that->dy) ; 

		free(that) ; 
		return 0 ;
	}

	return -1 ; 
}

int BxWarpGetVoronoi( void* handle, int** voronoi )
{
	BxWarp * that = (BxWarp*) handle ;
	(*voronoi) = that->voronoi  ;
	return 0 ; 
}

short BxWarpFastRoundShort(double val)
{
	#if (BXWARP_QUICK_CAST_METHOD == 1)
		val += 68719476736*1.5+0.5 ;
		return ((short*)&val)[1] ; 
	#else
		return (short) (val + 0.5) ; 
	#endif
}

int BxWarpFastRoundInt(double val)
{
	#if (BXWARP_QUICK_CAST_METHOD == 1)
		val += 68719476736*1.5+0.5 ;
		return ((short*)&val)[1] ; 
	#else
		return (int) (val + 0.5) ; 
	#endif
}

int BxWarpFastFloorInt(double val)
{
	#if (BXWARP_QUICK_CAST_METHOD == 1)
		val += 68719476736*1.5 ;
		return ((short*)&val)[1] ; 
	#else
		return (int) val ; 
	#endif
}

void  BxWarpBicubicCoefficients(int *y, float *y1, float *y2, float *y12, float *c)
{
	// static: not thread-safe, but no Pb since read-only
	static int wt[16][16] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
														0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
													 -3, 0, 0, 3, 0, 0, 0, 0,-2, 0, 0,-1, 0, 0, 0, 0, 
														2, 0, 0,-2, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 
														0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
														0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 
														0, 0, 0, 0,-3, 0, 0, 3, 0, 0, 0, 0,-2, 0, 0,-1, 
														0, 0, 0, 0, 2, 0, 0,-2, 0, 0, 0, 0, 1, 0, 0, 1, 
													 -3, 3, 0, 0,-2,-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
														0, 0, 0, 0, 0, 0, 0, 0,-3, 3, 0, 0,-2,-1, 0, 0, 
														9,-9, 9,-9, 6, 3,-3,-6, 6,-6,-3, 3, 4, 2, 1, 2, 
													 -6, 6,-6, 6,-4,-2, 2, 4,-3, 3, 3,-3,-2,-1,-1,-2, 
														2,-2, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
														0, 0, 0, 0, 0, 0, 0, 0, 2,-2, 0, 0, 1, 1, 0, 0, 
													 -6, 6,-6, 6,-3,-3, 3, 3,-4, 4, 2,-2,-2,-2,-1,-1, 
														4,-4, 4,-4, 2, 2,-2,-2, 2,-2,-2, 2, 1, 1, 1, 1
													};
	int   l, k, j, i;
	float xx, cl[16], x[16];

	for(i=0; i<4; i++)
	{
		x[i]    = (float)y[i];
		x[i+4]  = y1[i];
		x[i+8]  = y2[i];
		x[i+12] = y12[i];
	}

	for(i=0; i<16; i++)
	{
		xx = (float)0;
		for(k=0; k<16; k++)
			xx += wt[i][k] * x[k];
		cl[i] = xx;
	}

	l = 0;
	for(i=0; i<4; i++)
		for(j=0; j<4; j++)
			c[i*4+j] = cl[l++];

}

float BxWarpBicubicInterpolationShort(short * im, int dimx, int dimy, float x, float y)
{
	int   i, j, l, Y[4];
	float dx, dy, Y1[4], Y2[4], Y12[4], C[16], Res;

	dimy = 0; // To keep compiler happy

	j = (int)x;
	dx = x - (float)j;
	i = (int)y;
	dy = y - (float)i;

	Y[0] = im[i*dimx+j];
	Y[1] = im[i*dimx+j+1];
	Y[2] = im[(i+1)*dimx+j+1];
	Y[3] = im[(i+1)*dimx+j];

	Y1[0] = (im[i*dimx+j+1]     - im[i*dimx+j-1])    * (float).5;
	Y1[1] = (im[i*dimx+j+2]     - im[i*dimx+j ] )    * (float).5;
	Y1[2] = (im[(i+1)*dimx+j+2] - im[(i+1)*dimx+j])  * (float).5;
	Y1[3] = (im[(i+1)*dimx+j+1] - im[(i+1)*dimx+j-1]) * (float).5;

	Y2[0] = (im[(i+1)*dimx+j]   - im[(i-1)*dimx+j])  * (float).5;
	Y2[1] = (im[(i+1)*dimx+j+1] - im[(i-1)*dimx+j+1])* (float).5;
	Y2[2] = (im[(i+2)*dimx+j+1] - im[i*dimx+j+1])    * (float).5;
	Y2[3] = (im[(i+2)*dimx+j]   - im[i*dimx+j])      * (float).5;

	Y12[0] = (im[(i+1)*dimx+j+1] + im[(i-1)*dimx+j-1] - im[(i+1)*dimx+j-1] - im[(i-1)*dimx+j+1]) * (float).25;
	Y12[1] = (im[(i+1)*dimx+j+2] + im[(i-1)*dimx+j]   - im[(i+1)*dimx+j]   - im[(i-1)*dimx+j+2]) * (float).25;
	Y12[2] = (im[(i+2)*dimx+j+2] + im[i*dimx+j  ]     - im[(i+2)*dimx+j]   - im[i*dimx+j+2])     * (float).25;
	Y12[3] = (im[(i+2)*dimx+j+1] + im[i*dimx+j-1]     - im[(i+2)*dimx+j-1] - im[i*dimx+j+1])     * (float).25;

	BxWarpBicubicCoefficients(Y, Y1, Y2, Y12, C);

	Res = (float)0;
	for(l=3; l>=0; l--)
		Res = dx * Res + ((C[l*4+3] * dy + C[l*4+2]) * dy + C[l*4+1]) * dy + C[l*4];

	return Res;
}

float BxWarpBicubicInterpolationFloat(float * im, int dimx, int dimy, float x, float y)
{
	int   i, j, l, Y[4];
	float dx, dy, Y1[4], Y2[4], Y12[4], C[16], Res;

	dimy = 0; // To keep compiler happy

	j = (int)x;
	dx = x - (float)j;
	i = (int)y;
	dy = y - (float)i;

	Y[0] = (int)im[i*dimx+j];
	Y[1] = (int)im[i*dimx+j+1];
	Y[2] = (int)im[(i+1)*dimx+j+1];
	Y[3] = (int)im[(i+1)*dimx+j];

	Y1[0] = (im[i*dimx+j+1]     - im[i*dimx+j-1])    * (float).5;
	Y1[1] = (im[i*dimx+j+2]     - im[i*dimx+j ] )    * (float).5;
	Y1[2] = (im[(i+1)*dimx+j+2] - im[(i+1)*dimx+j])  * (float).5;
	Y1[3] = (im[(i+1)*dimx+j+1] - im[(i+1)*dimx+j-1]) * (float).5;

	Y2[0] = (im[(i+1)*dimx+j]   - im[(i-1)*dimx+j])  * (float).5;
	Y2[1] = (im[(i+1)*dimx+j+1] - im[(i-1)*dimx+j+1])* (float).5;
	Y2[2] = (im[(i+2)*dimx+j+1] - im[i*dimx+j+1])    * (float).5;
	Y2[3] = (im[(i+2)*dimx+j]   - im[i*dimx+j])      * (float).5;

	Y12[0] = (im[(i+1)*dimx+j+1] + im[(i-1)*dimx+j-1] - im[(i+1)*dimx+j-1] - im[(i-1)*dimx+j+1]) * (float).25;
	Y12[1] = (im[(i+1)*dimx+j+2] + im[(i-1)*dimx+j]   - im[(i+1)*dimx+j]   - im[(i-1)*dimx+j+2]) * (float).25;
	Y12[2] = (im[(i+2)*dimx+j+2] + im[i*dimx+j  ]     - im[(i+2)*dimx+j]   - im[i*dimx+j+2])     * (float).25;
	Y12[3] = (im[(i+2)*dimx+j+1] + im[i*dimx+j-1]     - im[(i+2)*dimx+j-1] - im[i*dimx+j+1])     * (float).25;

	BxWarpBicubicCoefficients(Y, Y1, Y2, Y12, C);

	Res = (float)0;
	for(l=3; l>=0; l--)
		Res = dx * Res + ((C[l*4+3] * dy + C[l*4+2]) * dy + C[l*4+1]) * dy + C[l*4];

	return Res;
}

int BxWarpDistanceMap(BxWarp * that)
{
	float min_distance ; 
	int best_s ; 
	int s ; 
	float a, b ; 
	float dist ; 
	int x, y ;
	BxCurve * curve = &that->curve    ; 
	int* voronoi    =  that->voronoi  ; 
	float* distance =  that->distance ; 
	int dimx        =  that->dimx     ;
	int dimy        =  that->dimy     ;

	for (y=0; y<dimy; y++)
	for (x=0; x<dimx; x++)
	{
		min_distance = FLT_MAX ;
		best_s = 0 ; 

		for ( s=1; s<curve->npoints-1; s++)
		{
			a = (x-curve->x[s]) ;
			b = (y-curve->y[s]) ;
			dist = a*a + b*b ; 
			if ( dist < min_distance )
			{
				min_distance = dist ;
				best_s = s ;
			}
		}

		distance[y*dimx+x] = min_distance ; 
		if (min_distance <= that->maxDistance*that->maxDistance)
			voronoi [y*dimx+x] = best_s ; 
		else
			voronoi [y*dimx+x] = -1 ; 
	}

	return 0 ; 
}

int BxWarpGetDistanceMap ( void* handle, float * map ) 
{
	BxWarp * that = (BxWarp*) handle ;
	memcpy(map,that->distance,that->dimx*that->dimy*sizeof(float)) ; 
	return 0 ; 
}

int BxWarpGetWarpCurve   ( void* handle, BxCurve * warpCurve ) 
{
	BxWarp * that = (BxWarp*) handle ;
	BxCurveCopy(&that->curve,warpCurve) ; 
	return 0 ;
}

int BxWarpChange( void* handle, BxCurve * warpCurve ) 
{
	BxWarp * that = (BxWarp*) handle ;
	int wdimx, wdimy ; 
	
	wdimx = that->dimx ; 
	wdimy = that->dimy ; 

	BxCurveCopy(warpCurve,&that->curve) ; 

	if (that->autoCenter==1) 
		BxCurveCenterInImage(&that->curve,wdimx,wdimy) ; 
	
	BxCurveZoom(&that->curve,that->zoom,((float)(wdimx-1))/2,((float)(wdimy-1))/2) ;	

	/*if (1)
	{
		float xt, yt ; 
		MxOpen() ;
		MxSetVisible(1) ;
		MxCommand(" range = 8 ; xt = single( 2 * range * (rand()-0.5) ) ; yt = single( 2 * range * (rand()-0.5) ) ; ") ;
		MxGetFloat(&xt,"xt") ;
		MxGetFloat(&yt,"yt") ;
		BxCurveTranslate(&that->curve,xt,yt) ;	
	}*/

	BxWarpDistanceMap(that) ; 

	return 0 ;
}

int BxWarpChangeWithCenter( void* handle, BxCurve * warpCurve, float xCenter, float yCenter ) 
{

	BxWarp * that = (BxWarp*) handle ;
	int wdimx, wdimy ; 
	//float xMean, yMean ; 

	wdimx = that->dimx ; 
	wdimy = that->dimy ; 

	BxCurveCopy(warpCurve, &that->curve) ; 
	BxCurveZoom(&that->curve,that->zoom,xCenter,yCenter) ;	
	BxWarpDistanceMap(that) ; 

	return 0 ; 
}


int BxWarpMotionField( void* handle, float * dx, float * dy, BxCurve * curve ) 
{
	BxWarp * that = (BxWarp*) handle ;

	int x, y, s, ind ; 	
	float a ; 
	float b ;
	float ex, ey ; 
	float vx, vy ;
	float cosinus[BXCURVE_MAX_NPOINTS] ; 
	float sinus[BXCURVE_MAX_NPOINTS] ;
	float ds ; 
	int wdimx = that->dimx ; 
	int wdimy = that->dimy ; 
	int pairing[BXCURVE_MAX_NPOINTS] ; 

	BxCurvePointWisePairingPiecewiseLinear (&(that->curve), curve, pairing) ;

	for (s=0; s<that->curve.npoints; s++)
	{
		ex = curve->dx[pairing[s]] ;
		ey = curve->dy[pairing[s]] ;
		ds = curve->ds[pairing[s]]; 
		if (ds>0)
		{
			ex = ex / ds ;
			ey = ey / ds ;
		}

		vx = that->curve.dx[s] ;
		vy = that->curve.dy[s] ;
		ds = that->curve.ds[s] ; 
		if (ds>0)
		{
			vx = vx / ds ;
			vy = vy / ds ;
		}
	
		cosinus[s] = ( vx * ex + vy * ey ) / that->zoom  ;
		sinus[s]   = ( vx * ey - vy * ex ) / that->zoom  ;

		if (cosinus[s] > +1) cosinus[s] = +1.f ; 
		if (cosinus[s] < -1) cosinus[s] = -1.f ; 
		if (sinus[s] > +1)   sinus[s]   = +1.f ; 
		if (sinus[s] < -1)   sinus[s]   = -1.f ; 
	}

	for (y=0; y<wdimy; y++)
	for (x=0; x<wdimx; x++)
	{
		ind = y*wdimx+x ; 
		s = that->voronoi[ind] ; 

		if (s > 0)
		{
			a = (x-that->curve.x[s])  ;
			b = (y-that->curve.y[s])  ;
			dx[ind] = curve->x[pairing[s]]-x + cosinus[s] * a - sinus[s]   * b  ; 
			dy[ind] = curve->y[pairing[s]]-y + sinus[s]   * a + cosinus[s] * b  ; 
		}
		else
		{
			dx[ind] = 0 ; 
			dy[ind] = 0 ; 
		}
	}

	return 0 ; 
}

float RadialFunction( float r2, float scale)
{
	return ( 1.f / (r2+scale*scale) ) ; 
}

int BxWarpLocallyRigidMotionField( void* handle, float * dx, float * dy, BxCurve * curve, float scale ) 
{
	BxWarp * that = (BxWarp*) handle ;

	int x, y, s, ind, n ; 	
	float a ; 
	float b ;
	float ex, ey ; 
	float vx, vy ;
	float cosinus[BXCURVE_MAX_NPOINTS] ; 
	float sinus  [BXCURVE_MAX_NPOINTS] ;
	int pairing  [BXCURVE_MAX_NPOINTS] ; 
	float ds ; 
	int wdimx = that->dimx ; 
	int wdimy = that->dimy ; 
	float weight, weights, r2 , sf ;  
	int nwindows ; 
	float sstep  ;
	
	nwindows = (int) ( that->curve.length / ( BXWARP_WINDOW_SPACING * that->scale ) ) + 1 ; 
	if (nwindows > BXWARP_MAX_NWINDOWS)	nwindows = BXWARP_MAX_NWINDOWS ; 
	sstep    = ((float) that->curve.npoints-1) / (nwindows-1) ; 

	BxCurvePointWisePairingPiecewiseLinear (&(that->curve), curve, pairing) ;

	/*for (s=0; s<that->curve.nlandmarks; s++)
		pairing[s] = s ; */

	for (s=0; s<that->curve.npoints; s++)
	{
		ex = curve->dx[pairing[s]] ;
		ey = curve->dy[pairing[s]] ;
		ds = curve->ds[pairing[s]]; 
		if (ds>0)
		{
			ex = ex / ds ;
			ey = ey / ds ;
		}

		vx = that->curve.dx[s] ;
		vy = that->curve.dy[s] ;
		ds = that->curve.ds[s] ; 
		if (ds>0)
		{
			vx = vx / ds ;
			vy = vy / ds ;
		}
	
		cosinus[s] = ( vx * ex + vy * ey ) / that->zoom  ;
		sinus[s]   = ( vx * ey - vy * ex ) / that->zoom  ;

		if (cosinus[s] > +1) cosinus[s] = +1.f ; 
		if (cosinus[s] < -1) cosinus[s] = -1.f ; 
		if (sinus[s] > +1)   sinus[s]   = +1.f ; 
		if (sinus[s] < -1)   sinus[s]   = -1.f ; 
			
	}

	for (y=0; y<wdimy; y++)
	for (x=0; x<wdimx; x++)
	{
		
		ind = y*wdimx+x ; 

		weights = 0 ; 
		vx = 0 ;
		vy = 0 ; 

		sf = 0 ; 
		for (n=0 ; n<nwindows; sf+= sstep, n++)
		{	
			s = BxWarpFastFloorInt(sf) ; 

			a = (x-that->curve.x[s])  ;
			b = (y-that->curve.y[s])  ;

			r2  = a*a + b*b ;
			weight = RadialFunction(r2,scale) ; 

			if (weight > 0)
			{
				a = (x-that->curve.x[s])  ;
				b = (y-that->curve.y[s])  ;
		
				vx += weight * ( curve->x[pairing[s]]-x + cosinus[s] * a - sinus[s]   * b  ) ; 
				vy += weight * ( curve->y[pairing[s]]-y + sinus[s]   * a + cosinus[s] * b  ) ; 

				weights += weight ;
			}
		}

		if (weights > 0)
		{
			dx[ind] = vx / weights ;
			dy[ind] = vy / weights ;
		}
		else
		{
			dx[ind] = 0 ;
			dy[ind] = 0 ; 
		}

	}

	return 0 ; 
}

int BxWarpDoWarp( void* handle, short * image, int dimx, int dimy, BxCurve * curve, short * warped )
{
	BxWarp *that = (BxWarp*) handle ;

	int x, y ; 
	int ori_x, ori_y ; 
	float ori_xr, ori_yr ; 
	float * input  ;
	float * buffer ; 
	float *dx  = that->dx ;
	float *dy  = that->dy ; 
	int ind, ind1, ind2, ind3, ind4 ; 
	float value ;
	float af, bf ;

	int wdimx = that->dimx ; 
	int wdimy = that->dimy ; 

	input   = (float*) malloc(dimx  *  dimy * sizeof(float)) ; 
	if (that->interpolation == BXWARP_SHIFTED_INTERPOL)
	{
		buffer = (float*) malloc(dimx  *  dimy*sizeof(float)) ; 
		for (x=0; x<dimx*dimy; x++)
			buffer[x] = (float) image[x] ; 
		BxShiftLinearPreFilter2D(buffer,input,dimx,dimy,BXWARP_SHIFTED_INTERPOL_VALUE) ; 
		free(buffer) ; 
	}
	else
	{
		for (x=0; x<dimx*dimy; x++)
			input[x] = (float) image[x] ; 
	}

	memset(warped,0,wdimx*wdimy*sizeof(short)) ; 

	if (that->motionModel == BXWARP_PIECEWISE_RIGID)
	{
		BxWarpMotionField(handle,dx,dy,curve) ; 
	}
	else if (that->motionModel == BXWARP_PIECEWISE_SMOOTH)
	{
		BxWarpMotionField(handle,dx,dy,curve) ; 
		BxIsoGaussFilterFloat2D(dx,dx,that->scale,wdimx,wdimy) ;
		BxIsoGaussFilterFloat2D(dy,dy,that->scale,wdimx,wdimy) ;
	}
	else if (that->motionModel == BXWARP_LOCALLY_RIGID)
	{
		BxWarpLocallyRigidMotionField(handle,dx,dy,curve,that->scale);
	}
	else
	{
		memset(dx,0,wdimx*wdimy*sizeof(float)) ; 
		memset(dy,0,wdimx*wdimy*sizeof(float)) ; 
	}

	for (y=0; y<wdimy; y++)
	for (x=0; x<wdimx; x++)
	{
		ind =y*wdimx+x ; 
		if (that->voronoi[ind] > 0)
		{
			ori_xr = dx[ind] + x ;  
			ori_yr = dy[ind] + y ; 

			if ((ori_xr>=1) && (ori_yr>=1) && (ori_xr<dimx-1) && (ori_yr<dimy-1))
			{
				if (that->interpolation == BXWARP_NEAREST_INTERPOL)
				{
					ori_x = BxWarpFastRoundInt(ori_xr) ;
					ori_y = BxWarpFastRoundInt(ori_yr) ;
					warped[y*wdimx+x] = BxWarpFastRoundShort(input[ori_y*dimx+ori_x]) ;
				}
				else if ( (that->interpolation == BXWARP_SHIFTED_INTERPOL) || (that->interpolation == BXWARP_LINEAR_INTERPOL) )
				{
					if (that->interpolation == BXWARP_SHIFTED_INTERPOL)
					{
						ori_xr -= BXWARP_SHIFTED_INTERPOL_VALUE ;
						ori_yr -= BXWARP_SHIFTED_INTERPOL_VALUE ;
					}

					ori_x = BxWarpFastFloorInt(ori_xr)  ;
					ori_y = BxWarpFastFloorInt(ori_yr)  ;

					af = ori_xr - (float) ori_x ;
					bf = ori_yr - (float) ori_y ;

					ind1 =  ori_y    * dimx +  ori_x    ;
					ind2 =  ori_y    * dimx + (ori_x+1) ;
					ind3 = (ori_y+1) * dimx +  ori_x    ;
					ind4 = (ori_y+1) * dimx + (ori_x+1) ;

					value = input[ind1] ;
					value += af    * ( input[ind2] - input[ind1] ) ;
					value += bf    * ( input[ind3] - input[ind1] ) ; 
					value += af*bf * ( input[ind1] + input[ind4] - input[ind2] - input[ind3] ) ;	
					warped[ind] = BxWarpFastRoundShort(value) ;

				}
				else if (that->interpolation == BXWARP_CUBIC_INTERPOL) 
				{
					warped[ind] = BxWarpFastRoundShort( BxWarpBicubicInterpolationShort(image,dimx,dimy,ori_xr,ori_yr) ) ;
				}
			}
		}
	}

	free(input) ; 
	return 0 ;
}


int BxWarpDoWarpFloat( void* handle, short * image, int dimx, int dimy, BxCurve * curve, float * warped )
{
	BxWarp * that = (BxWarp*) handle ;

	int x, y, ind, ind1, ind2, ind3, ind4 ; 
	int ori_x, ori_y ; 
	float ori_xr, ori_yr ; 
	float value ;
	float af, bf ;
	float * input  ;
	float * buffer ; 
	float *dx = that->dx ;
	float *dy = that->dy ;

	int wdimx = that->dimx ; 
	int wdimy = that->dimy ; 

	input   = (float*) malloc(dimx*dimy * sizeof(float)) ;
	if (that->interpolation == BXWARP_SHIFTED_INTERPOL)
	{
		buffer = (float*) malloc(dimx*dimy*sizeof(float)) ; 
		for (x=0; x<dimx*dimy; x++)
			buffer[x] = (float) image[x] ; 
		BxShiftLinearPreFilter2D(buffer,input,dimx,dimy,BXWARP_SHIFTED_INTERPOL_VALUE) ; 
		free(buffer) ; 
	}
	else
	{
		for (x=0; x<dimx*dimy; x++)
			input[x] = (float) image[x] ; 
	}

	memset(warped,0,wdimx*wdimy*sizeof(float)) ; 

	if (that->motionModel == BXWARP_PIECEWISE_RIGID)
	{
		BxWarpMotionField(handle,dx,dy,curve) ; 
	}
	else if (that->motionModel == BXWARP_PIECEWISE_SMOOTH)
	{
		BxWarpMotionField(handle,dx,dy,curve) ; 
		BxIsoGaussFilterFloat2D(dx,dx,that->scale,wdimx,wdimy) ;
		BxIsoGaussFilterFloat2D(dy,dy,that->scale,wdimx,wdimy) ;
	}
	else if (that->motionModel == BXWARP_LOCALLY_RIGID)
	{
		BxWarpLocallyRigidMotionField(handle,dx,dy,curve,that->scale);
	}
	else
	{
		memset(dx,0,wdimx*wdimy*sizeof(float)) ; 
		memset(dy,0,wdimx*wdimy*sizeof(float)) ; 
	}

	for (y=0; y<wdimy; y++)
	for (x=0; x<wdimx; x++)
	{
		ind =y*wdimx+x ; 
		if (that->voronoi[ind] > 0)
		{
			ori_xr = dx[ind] + x ;  
			ori_yr = dy[ind] + y ; 

			if ((ori_xr>=1) && (ori_yr>=1) && (ori_xr<dimx-1) && (ori_yr<dimy-1))
			{
				
				if (that->interpolation == BXWARP_NEAREST_INTERPOL)
				{
					ori_x = BxWarpFastRoundInt(ori_xr) ;
					ori_y = BxWarpFastRoundInt(ori_yr) ;
					warped[y*wdimx+x] = input[ori_y*dimx+ori_x] ;
				}
				else if ( (that->interpolation == BXWARP_SHIFTED_INTERPOL) || (that->interpolation == BXWARP_LINEAR_INTERPOL) )
				{
					if (that->interpolation == BXWARP_SHIFTED_INTERPOL)
					{
						ori_xr -= BXWARP_SHIFTED_INTERPOL_VALUE ;
						ori_yr -= BXWARP_SHIFTED_INTERPOL_VALUE ;
					}

					ori_x = BxWarpFastFloorInt(ori_xr)  ;
					ori_y = BxWarpFastFloorInt(ori_yr)  ;

					af = ori_xr - (float) ori_x ;
					bf = ori_yr - (float) ori_y ;

					ind1 =  ori_y    * dimx +  ori_x    ;
					ind2 =  ori_y    * dimx + (ori_x+1) ;
					ind3 = (ori_y+1) * dimx +  ori_x    ;
					ind4 = (ori_y+1) * dimx + (ori_x+1) ;

					value = input[ind1] ;
					value += af    * ( input[ind2] - input[ind1] ) ;
					value += bf    * ( input[ind3] - input[ind1] ) ; 
					value += af*bf * ( input[ind1] + input[ind4] - input[ind2] - input[ind3] ) ;	
					warped[ind] = value ;
				}
				else if (that->interpolation == BXWARP_CUBIC_INTERPOL) 
				{
					warped[ind] = BxWarpBicubicInterpolationShort(image,dimx,dimy,ori_xr,ori_yr) ;
				}
			}
		}
	}

	free(input) ; 
	return 0 ;
}

int BxWarpDoWarpFloat2Float( void* handle, float * image, int dimx, int dimy, BxCurve * curve, float * warped )
{
	BxWarp * that = (BxWarp*) handle ;

	int x, y, ind, ind1, ind2, ind3, ind4 ; 
	int ori_x, ori_y ; 
	float ori_xr, ori_yr ; 
	float * input  ;
	float value ;
	float af, bf ;
	float *dx = that->dx ;
	float *dy = that->dy ;

	int wdimx = that->dimx ; 
	int wdimy = that->dimy ; 

	dx      = (float*) malloc(wdimx * wdimy * sizeof(float)) ; 
	dy      = (float*) malloc(wdimx * wdimy * sizeof(float)) ; 

	if (that->interpolation == BXWARP_SHIFTED_INTERPOL)
	{
		input   = (float*) malloc(dimx  *  dimy * sizeof(float)) ; 
		BxShiftLinearPreFilter2D(image,input,dimx,dimy,BXWARP_SHIFTED_INTERPOL_VALUE) ; 
	}
	else
		input = image ; 

	memset(warped,0,wdimx*wdimy*sizeof(float)) ; 

	if (that->motionModel == BXWARP_PIECEWISE_RIGID)
	{
		BxWarpMotionField(handle,dx,dy,curve) ; 
	}
	else if (that->motionModel == BXWARP_PIECEWISE_SMOOTH)
	{
		BxWarpMotionField(handle,dx,dy,curve) ; 
		BxIsoGaussFilterFloat2D(dx,dx,that->scale,wdimx,wdimy) ;
		BxIsoGaussFilterFloat2D(dy,dy,that->scale,wdimx,wdimy) ;
	}
	else if (that->motionModel == BXWARP_LOCALLY_RIGID)
	{
		BxWarpLocallyRigidMotionField(handle,dx,dy,curve,that->scale);
	}
	else
	{
		memset(dx,0,wdimx*wdimy*sizeof(float)) ; 
		memset(dy,0,wdimx*wdimy*sizeof(float)) ; 
	}

	for (y=0; y<wdimy; y++)
	for (x=0; x<wdimx; x++)
	{
		ind =y*wdimx+x ; 
		if (that->voronoi[ind] > 0)
		{
			ori_xr = dx[ind] + x ;  
			ori_yr = dy[ind] + y ; 

			if ((ori_xr>=1) && (ori_yr>=1) && (ori_xr<dimx-1) && (ori_yr<dimy-1))
			{
				if (that->interpolation == BXWARP_NEAREST_INTERPOL)	
				{
					ori_x = BxWarpFastRoundInt(ori_xr) ;
					ori_y = BxWarpFastRoundInt(ori_yr) ;
					warped[y*wdimx+x] = input[ori_y*dimx+ori_x] ;
				}
				else if ((that->interpolation == BXWARP_SHIFTED_INTERPOL) ||(that->interpolation == BXWARP_LINEAR_INTERPOL))
				{
					if (that->interpolation == BXWARP_SHIFTED_INTERPOL)
					{
						ori_xr -= BXWARP_SHIFTED_INTERPOL_VALUE ;
						ori_yr -= BXWARP_SHIFTED_INTERPOL_VALUE ;
					}

					ori_x = BxWarpFastFloorInt(ori_xr)  ;
					ori_y = BxWarpFastFloorInt(ori_yr)  ;

					af = ori_xr - (float) ori_x ;
					bf = ori_yr - (float) ori_y ;

					ind1 =  ori_y    * dimx +  ori_x    ;
					ind2 =  ori_y    * dimx + (ori_x+1) ;
					ind3 = (ori_y+1) * dimx +  ori_x    ;
					ind4 = (ori_y+1) * dimx + (ori_x+1) ;

					value = input[ind1] ;
					value += af    * ( input[ind2] - input[ind1] ) ;
					value += bf    * ( input[ind3] - input[ind1] ) ; 
					value += af*bf * ( input[ind1] + input[ind4] - input[ind2] - input[ind3] ) ;	
					warped[ind] = value ;
				}
				else if (that->interpolation == BXWARP_CUBIC_INTERPOL)
					warped[ind] = BxWarpBicubicInterpolationFloat(image,dimx,dimy,ori_xr,ori_yr) ;

			}
		}
	}

	free(input) ; 
	return 0 ;
}

int BxWarpModelDrivenNormalisedCorrelation( void* handle, float * refImage, short * image, int dimx, int dimy, BxCurve * curve, float * correlation )
{
	BxWarp * that = (BxWarp*) handle ; 
	float var1, var2, corr, mean1, mean2 ; 
	int k ; 
	float a, b ; 
	int wdimx = that->dimx ; 
	int wdimy = that->dimy ; 
	float * warped = that->buffer ;
	int npixels  ; 

	BxWarpDoWarpFloat(handle,image,dimx,dimy,curve,warped) ; 

	corr = var1 = var2 = mean1 = mean2 = 0 ; 
	npixels = 0 ; 
	for (k=0; k<wdimx*wdimy; k++)
	{
		if (that->voronoi[k] > 0)
		{
			mean1 += refImage[k] ;
			mean2 += warped[k]   ; 
			npixels++ ; 
		}
	}

	if (npixels <= 0)
	{
		*correlation = 0 ; 
		return -1 ; 
	}

	mean1 /= npixels ; 
	mean2 /= npixels ; 

	for (k=0; k<wdimx*wdimy; k++)
	{
		if (that->voronoi[k] > 0)
		{
			a = (refImage[k]-mean1) ; 
			b = (warped[k]  -mean2) ;
			var1 += a * a ;
			var2 += b * b ; 
			corr += a * b ; 
		}

		
	}

	var1 /= npixels ; 
	var2 /= npixels ; 
	corr /= npixels ; 

	if (var1*var2>0)
		corr /= (float) sqrt(var1*var2) ; 

	(*correlation) = corr ; 

	return 0 ; 
}



/*int BxWarpDistanceMapCoarseToFine(BxCurve * curve, int* voronoi, float* distance, int dimx, int dimy, int nbLevels)
{
	float min_distance ; 
	int best_s ; 
	int s ; 
	float a, b ; 
	float dist ; 
	int x, y ;
	int level ; 
	int step  ; 
	int range ; 

	for (y=0; y<dimy; y++)
	for (x=0; x<dimx; x++)
	{
		a = (x-curve->x[0]) ;
		b = (y-curve->y[0]) ;
		distance[y*dimx+x] = a*a + b*b ; 
		voronoi [y*dimx+x] = 0 ; 
	}

	step = 1 ; 
	for (level=1; level<nbLevels; level++)
		step = 2*step ; 

	for (y=0; y<dimy; y++)
	for (x=0; x<dimx; x++)
	{
		min_distance = distance[y*dimx+x] ;
		best_s       = voronoi [y*dimx+x] ; 

		for ( s=0; s<curve->npoints; s+=step)
		{
			a = (x-curve->x[s]) ;
			b = (y-curve->y[s]) ;
			dist = a*a + b*b ; 
			if ( dist < min_distance )
			{
				min_distance = dist ;
				best_s = s ;
			}
		}

		voronoi [y*dimx+x] = best_s ; 
		distance[y*dimx+x] = min_distance ; 
	}

	for (level = 1; level<nbLevels; level++)
	{
		range = step ; 
		step /= 2 ; 
		for (y=0; y<dimy; y++)
		for (x=0; x<dimx; x++)
		{
			min_distance = distance[y*dimx+x] ; 
			best_s       = voronoi [y*dimx+x] ; 

			for ( s=best_s-range; s<=best_s+range; s+=step)
			{
				if ((s>=0) && (s<curve->npoints))
				{
					a = (x-curve->x[s]) ;
					b = (y-curve->y[s]) ;
					dist = a*a + b*b ; 
					if ( dist < min_distance )
					{
						min_distance = dist ;
						best_s = s ;
					}
				}
			}

			voronoi [y*dimx+x] = best_s ; 
			distance[y*dimx+x] = min_distance ; 
		}
	}

	return 0 ; 
}*/

