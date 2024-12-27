// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <BxCurve.h>
#include <BxSpline.h>
#include <math.h>
#include <stdlib.h>
#include <memory.h>
#include <float.h>

int BxCurveSwap(BxCurve* curve) 
{
	int s ;
	int nlandmarks ; 
	int slandmark[BXCURVE_MAX_LANDMARKS] ; 
	float *xBuf, *yBuf;

	xBuf = (float *)malloc(curve->npoints * sizeof(float));
	yBuf = (float *)malloc(curve->npoints * sizeof(float));

	nlandmarks = curve->nlandmarks ; 
	memcpy(slandmark,curve->slandmark,nlandmarks*sizeof(int)) ; 

	for (s=0; s<curve->npoints; s++)
	{
		xBuf[s] = curve->x[s];
		yBuf[s] = curve->y[s];
	}

	for (s=0; s<curve->npoints; s++)
	{
		curve->x[s] = xBuf[curve->npoints - s - 1];
		curve->y[s] = yBuf[curve->npoints - s - 1];
	}

	BxCurveInit(curve,curve->x,curve->y,curve->npoints) ; 

	curve->nlandmarks = nlandmarks ; 
	memcpy(curve->slandmark,slandmark,nlandmarks*sizeof(int)) ; 
 
	free(xBuf);
	free(yBuf);

	return 0 ;
}

short BxCurveFastRoundShort(double val)
{
	val += 68719476736*1.5+0.5 ;
	return ((short*)&val)[1] ; 
}

short BxCurveFastFloorShort(double val)
{
	val += 68719476736*1.5   ;
	return ((short*)&val)[1] ; 
}

int BxCurveGradient( float* in, float* out, int size) 
{
	//int k = 0 ;
	float * prev ;
	float * next ; 
	float * end = out+size ; 

	*out++ = in[1]-in[0] ;
	prev = in ; 
	next = in+2 ; 
	for (;out<end-1;)
		*out++ = 0.5f * ( *next++ - *prev++ ) ;
	next-- ; 
	*out = *next - *prev ;
	
	return 0 ;
}

int BxCurveInit(BxCurve* curve, float* x, float* y, int npoints)
{
	int s ; 
	float dx, dy, ds, ddx, ddy ; 

	if (npoints > BXCURVE_MAX_NPOINTS)
		return -1 ;

	curve->npoints = npoints ; 
	curve->nlandmarks = 0 ; 

	memcpy(curve->x,x,npoints*sizeof(float)) ;
	memcpy(curve->y,y,npoints*sizeof(float)) ; 

	curve->length = 0 ; 
	for (s=0; s<npoints-1; s++)
	{
		dx = x[s+1] - x[s] ;
		dy = y[s+1] - y[s] ; 
		ds = (float) sqrt( dx*dx + dy*dy ) ; 

		curve->dx[s]   = dx ;
		curve->dy[s]   = dy ;
		curve->ds[s]   = ds ; 
		curve->length += ds ; 
	}

	dx = curve->dx[npoints-1] = x[npoints-1]-x[npoints-2] ;
	dy = curve->dy[npoints-1] = y[npoints-1]-y[npoints-2] ;
	ds = curve->ds[npoints-1] = (float) sqrt( dx*dx + dy*dy )   ;
	curve->length       += ds ; 

	for (s=0; s<npoints-1; s++)
	{
		ds  = curve->ds[s] ;
		if (ds>0)
		{
			dx  = curve->dx[s] ;
			dy  = curve->dy[s] ;
			ddx = curve->dx[s+1] - dx ; 
			ddy = curve->dy[s+1] - dy ; 
			curve->curvature[s] = (dx*ddy - dy*ddx) / (ds*ds*ds) ;
		}
		else
			curve->curvature[s] = 0 ; 
	}

	curve->curvature[npoints-1] = curve->curvature[npoints-2] ; 

	return 0 ;
}

int BxCurveInitGenericSpline(BxCurve* curve, float* xknot, float* yknot, int nknots, int sampling, int bounds) 
{
	//int k = 0 ;
	float *xcontrol = (float*) malloc( nknots * sizeof(float) ) ;
	float *ycontrol = (float*) malloc( nknots * sizeof(float) ) ;
	float x[BXCURVE_MAX_NPOINTS] ;
	float y[BXCURVE_MAX_NPOINTS] ; 
	int npoints  = (nknots-1)*sampling + nknots ;
		
	BxDirectCubicSplineTransform  (xknot, xcontrol, nknots, bounds) ;
	BxDirectCubicSplineTransform  (yknot, ycontrol, nknots, bounds) ;

	BxIndirectCubicSplineTransform(xcontrol, x, nknots, bounds, sampling+1) ;
	BxIndirectCubicSplineTransform(ycontrol, y, nknots, bounds, sampling+1) ;

	BxCurveInit(curve,x,y,npoints) ; 

	free(xcontrol) ;
	free(ycontrol) ; 

	return 0 ;
}

int BxCurveInitOpenSpline(BxCurve* curve, float* xknot, float* yknot, int nknots, int sampling) 
{
	return BxCurveInitGenericSpline(curve,xknot,yknot,nknots,sampling,BXSPLINE_ANTI_MIRROR) ; 
}

int BxCurveInitClosedSpline(BxCurve* curve, float* xknot, float* yknot, int nknots, int sampling) 
{
	float *xknot_closed = (float*) malloc( (nknots+1) * sizeof(float) ) ;
	float *yknot_closed = (float*) malloc( (nknots+1) * sizeof(float) ) ;

	memcpy(xknot_closed,xknot,nknots*sizeof(float)) ; 
	memcpy(yknot_closed,yknot,nknots*sizeof(float)) ; 
	xknot_closed[nknots] = xknot[0] ;
	yknot_closed[nknots] = yknot[0] ;

	BxCurveInitGenericSpline(curve,xknot_closed,yknot_closed,nknots+1,sampling,BXSPLINE_CYCLIC) ; 

	free(xknot_closed) ; 
	free(yknot_closed) ; 

	return 0 ; 
}

int BxCurveCopy(BxCurve* curve, BxCurve* newcurve) 
{
	newcurve->npoints    = curve->npoints  ;
	newcurve->length     = curve->length   ;
	newcurve->nlandmarks = curve->nlandmarks ;

	memcpy(newcurve->x,  curve->x,  curve->npoints*sizeof(float)) ; 
	memcpy(newcurve->y,  curve->y,  curve->npoints*sizeof(float)) ; 
	memcpy(newcurve->dx, curve->dx, curve->npoints*sizeof(float)) ; 
	memcpy(newcurve->dy, curve->dy, curve->npoints*sizeof(float)) ; 
	memcpy(newcurve->ds, curve->ds, curve->npoints*sizeof(float)) ; 
	memcpy(newcurve->curvature, curve->curvature, curve->npoints*sizeof(float)) ; 
	memcpy(newcurve->slandmark, curve->slandmark, curve->nlandmarks*sizeof(int)) ; 

	return 0 ;
}

int BxCurveTruncate(BxCurve* curve, BxCurve* newcurve, int first, int last) 
{
	int l ; 

	if (last >= curve->npoints) last = curve->npoints-1 ; 
	if (first < 0)              first = 0 ; 

	BxCurveInit(newcurve, curve->x + first, curve->y + first, last-first+1) ; 

	for (l=0; l<curve->nlandmarks; l++)
		if ((curve->slandmark[l] >= first) && (curve->slandmark[l] <= last))
			BxCurveAddLandmark(newcurve,l-first) ; 

	return 0 ;
}

int BxCurveNormal(BxCurve* curve, int s, float range, float* xn, float* yn, int npoints) 
{
	float nx    ;
	float ny     ;
	float dn    =  0.f ;
	float x, y, ds ; 
	int   k     =  0 ;
	float step  = 2*range / (npoints-1) ;

	if (s < 0)
	{
		x  =  curve->x[0] + s * curve->dx[0] ;
		y  =  curve->y[0] + s * curve->dy[0] ;
		nx = -curve->dy[0] ;
		ny =  curve->dx[0] ;
		ds =  curve->ds[0] ; 
	}
	else if (s > curve->npoints-1)
	{
		x  =  curve->x[curve->npoints-1] + (s-curve->npoints+1) * curve->dx[curve->npoints-1] ;
		y  =  curve->y[curve->npoints-1] + (s-curve->npoints+1) * curve->dy[curve->npoints-1] ;
		nx = -curve->dy[curve->npoints-1] ;
		ny =  curve->dx[curve->npoints-1] ;
		ds =  curve->ds[curve->npoints-1] ;
	}
	else
	{
		x  = curve->x[s]  ;
		y  = curve->y[s]  ;
		nx = -curve->dy[s] ;
		ny =  curve->dx[s] ;
		ds = curve->ds[s] ; 
	}

	if (ds > 0) 
	{
		nx = nx / ds ;
		ny = ny / ds ;
	}

	for (dn=-range, k=0; k<npoints; dn+=step, k++)
	{
		xn[k] = x + dn * nx ;
		yn[k] = y + dn * ny ;
	}

	return 0 ; 
}



int BxCurveTranslate(BxCurve* curve, float tx, float ty) 
{
	int s ; 
	for (s=0; s<curve->npoints; s++)
	{
		curve->x[s] += tx ;
		curve->y[s] += ty ;
	}

	return 0 ;
}

int BxCurveCentroid(BxCurve* curve, float * xmean, float * ymean)
{
	int s ; 

	*xmean = 0 ;
	*ymean = 0 ;
	for (s=0; s<curve->npoints; s++)
	{
		*xmean += curve->x[s] ;
		*ymean += curve->y[s] ;
	}
	*xmean /= curve->npoints ;
	*ymean /= curve->npoints ;

	return 0 ;
}

int BxCurveCenterInImage(BxCurve* curve, int dimx, int dimy)
{
	float xmean, ymean ;

	BxCurveCentroid(curve,&xmean,&ymean) ; 
	BxCurveTranslate(curve, ((float)dimx-1)/2 - xmean, ((float)dimy-1)/2 - ymean) ;

	return 0 ;
}

int BxCurveZoom(BxCurve* curve, float zoom, float xcenter, float ycenter)
{
	int s ;

	for (s=0; s<curve->npoints; s++)
	{
		curve->x[s]   = (curve->x[s]-xcenter)*zoom + xcenter ;
		curve->y[s]   = (curve->y[s]-ycenter)*zoom + ycenter ;
		curve->dx[s] *= zoom ;
		curve->dy[s] *= zoom ;
		curve->ds[s] *= zoom ;
	}

	curve->length *= zoom ;

	return 0 ;
}

int BxCurveAffineTransform(BxCurve* curve, float xc, float yc, float a, float b, float c, float d, float e, float f) 
{
	float x, y ;
	int s ;
	int nlandmarks ; 
	int slandmark[BXCURVE_MAX_LANDMARKS] ; 

	nlandmarks = curve->nlandmarks ; 
	memcpy(slandmark,curve->slandmark,nlandmarks*sizeof(int)) ; 

	for (s=0; s<curve->npoints; s++)
	{
		x = xc + a + b * (curve->x[s]-xc) + c * (curve->y[s]-yc)  ;
		y = yc + d + e * (curve->x[s]-xc) + f * (curve->y[s]-yc)  ;
		curve->x[s] = x ; 
		curve->y[s] = y ; 
	}

	BxCurveInit(curve,curve->x,curve->y,curve->npoints) ; 

	curve->nlandmarks = nlandmarks ; 
	memcpy(curve->slandmark,slandmark,nlandmarks*sizeof(int)) ; 
 
	return 0 ;
}

int BxCurveMarkersTransform(BxCurve* curve, float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22) 
{
	float xc1, yc1, xc2, yc2, ex, ey, vx, vy, norm_e, norm_v, scale, epsil ; 

	xc1 = 0.5f * (x11 + x12) ;
	yc1 = 0.5f * (y11 + y12) ;
	xc2 = 0.5f * (x21 + x22) ;
	yc2 = 0.5f * (y21 + y22) ;

	ex  = x12 - x11 ;
	ey  = y12 - y11 ;

	vx  = x22 - x21 ;
	vy  = y22 - y21 ;

	epsil   = 1e-6f ;
	norm_e = (float) sqrt( ex * ex + ey * ey ) + epsil ;
	norm_v = (float) sqrt( vx * vx + vy * vy ) + epsil ;
	scale  = norm_v / norm_e ;

	ex = ex / norm_e ;
	ey = ey / norm_e ;

	vx = vx / norm_v ;
	vy = vy / norm_v ;

	BxCurveAffineTransform( curve, 
		xc1, yc1, 
		xc2-xc1,	
		scale * vx * ex + vy * ey,											
		scale * vx * ey - vy * ex,
		yc2-yc1,
		scale * vy * ex - vx * ey,
		scale * vy * ey + vx * ex ) ; 

	return 0 ; 
}

int BxCurveIntegral(BxCurve* curve, float* image, int dimx, int dimy, float* integral, int interpol) 
{
	int s ;
	int x, y ;
	float xr, yr, a, b ;
	int out = 0 ;
	float length = 0 ;
	float value = 0 ;

	(*integral) = 0 ;
	
	for (s=0; s<curve->npoints; s++)
	{
		xr = curve->x[s]  ;
		yr = curve->y[s]  ;

		if ((xr>=0) && (yr>=0) && (xr<dimx) && (yr<dimy))
		{
			if (interpol == BXCURVE_LINEAR_INTERPOL)
			{
				x = (int) xr ;
				y = (int) yr ;

				a = xr - (float) x ;
				b = yr - (float) y ;

				value  = (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
				value += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
				value +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
				value +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
				
			}

			if (interpol == BXCURVE_NEAREST_INTERPOL)
			{
				x = (int) ( xr + 0.5 ) ;
				y = (int) ( yr + 0.5 ) ;
				value = image[y*dimx + x] ;
			}

			(*integral)  += curve->ds[s] * value  ;
			length += curve->ds[s] ;
		}
		else
			out = -1 ;
	}

	if (length > 0)
		(*integral) /= length ;

	return out ;
}

int BxCurveProfile(BxCurve* curve, float* image, int dimx, int dimy, float* profile, int interpol) 
{
	int s ;
	int x, y ;
	float xr, yr, a, b ;
	int out     = 0 ;

	memset(profile,0,curve->npoints*sizeof(float)) ;
	
	for (s=0; s<curve->npoints; s++)
	{
		xr = curve->x[s]  ;
		yr = curve->y[s]  ;

		if ((xr>0) && (yr>0) && (xr<dimx-1) && (yr<dimy-1))
		{
			if (interpol == BXCURVE_LINEAR_INTERPOL)
			{
				x = (int) xr ;
				y = (int) yr ;

				a = xr - (float) x ;
				b = yr - (float) y ;

				profile[s]  = (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
				profile[s] += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
				profile[s] +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
				profile[s] +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
				
			}

			if (interpol == BXCURVE_NEAREST_INTERPOL)
			{
				x = (int) ( xr + 0.5 ) ;
				y = (int) ( yr + 0.5 ) ;
				profile[s] = image[y*dimx + x] ;
			}
		}
		else
			out = -1 ;
	}

	return out ;
}


int BxCurveNormalProfile(BxCurve* curve, short* image, int dimx, int dimy, int s,  float* profile, int interpol, float range, int npoints) 
{
	int k ;
	float xc, yc ; 
	short xs, ys ;
	float xr, yr, a, b, ds, nx, ny ;
	float dn = 0.f ; 
	float step  = 2*range / (npoints-1) ;

	memset(profile,0,npoints*sizeof(float)) ;
	
	if (s < 0)
	{
		xc =  curve->x[0] + s * curve->dx[0] ;
		yc =  curve->y[0] + s * curve->dy[0] ;
		nx = -curve->dy[0] ;
		ny =  curve->dx[0] ;
		ds =  curve->ds[0] ; 
	}
	else if (s > curve->npoints-1)
	{
		xc =  curve->x[curve->npoints-1] + (s-curve->npoints+1) * curve->dx[curve->npoints-1] ;
		yc =  curve->y[curve->npoints-1] + (s-curve->npoints+1) * curve->dy[curve->npoints-1] ;
		nx = -curve->dy[curve->npoints-1] ;
		ny =  curve->dx[curve->npoints-1] ;
		ds =  curve->ds[curve->npoints-1] ;
	}
	else
	{
		xc = curve->x[s]  ;
		yc = curve->y[s]  ;
		nx = -curve->dy[s] ;
		ny =  curve->dx[s] ;
		ds = curve->ds[s] ; 
	}

	if (ds > 0) 
	{
		nx = nx / ds ;
		ny = ny / ds ;
	}
	
	for (dn=-range, k=0; k<npoints; dn+=step, k++)
	{
		xr = xc + dn * nx  ;
		yr = yc + dn * ny  ;

		if ((xr>0) && (yr>0) && (xr<dimx-1) && (yr<dimy-1))
		{
			if (interpol == BXCURVE_LINEAR_INTERPOL)
			{
				xs = (short) xr ;
				ys = (short) yr ;

				a = xr - (float) xs ;
				b = yr - (float) ys ;

				profile[k]  = (1-a) * (1-b) * (float) image[   ys   * dimx +   xs   ] ;
				profile[k] += (1-a) *   b   * (float) image[ (ys+1) * dimx +   xs   ] ;
				profile[k] +=   a   * (1-b) * (float) image[   ys   * dimx + (xs+1) ] ;
				profile[k] +=   a   *   b   * (float) image[ (ys+1) * dimx + (xs+1) ] ;
				
			}

			if (interpol == BXCURVE_NEAREST_INTERPOL)
			{
				xs = (short) ( xr + 0.5 ) ;
				ys = (short) ( yr + 0.5 ) ;
				profile[k] = (float) image[ ys * dimx + xs ] ;
			}
		}
	}

	return 0 ;
}

int BxCurveRidgeContrast(BxCurve * curve, short * image, int dimx, int dimy, float * contrast, float cc, float dz, float bk, float maxContrast, int nonLinear, int interpol)
{
	int s;
	float k;
	float halfcc = cc / 2 ;
	float value = 0 ;
	float mean1 = 0 ;
	float mean2 = 0 ;
	//int minv  = 0 ;
	int x, y ;
	float nx, ny, xr, yr, a, b ;
	float local, global ; 
	int icc = 2 * ((int) halfcc) + 1 ; 
	int ibk = ((int) bk) + 1 ; 
	float ccstep = cc / (icc-1) ; 
	float bkstep = bk / (ibk-1) ; 

	global = 0 ;

	for (s=0; s<curve->npoints; s++)
	{	
		nx    = -curve->dy[s] ;
		ny    =  curve->dx[s] ;

		if (curve->ds[s] > 0)	
		{
			nx = nx / curve->ds[s] ; 
			ny = ny / curve->ds[s] ; 
		}
		
		value = 0 ;
		mean1 = 0 ;
		mean2 = 0 ;

		for (k=-halfcc; k<=halfcc; k+=ccstep)
		{
			xr = curve->x[s] + k * nx  ;
			yr = curve->y[s] + k * ny  ;

			if ((xr>=1) && (yr>=1) && (xr<dimx-1) && (yr<dimy-1))
			{
				if (interpol == BXCURVE_LINEAR_INTERPOL)
				{
					x = (int) xr ;
					y = (int) yr ;

					a = xr - (float) x ;
					b = yr - (float) y ;

					value += (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
					value += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
					value +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
					value +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
					
				}

				if (interpol == BXCURVE_NEAREST_INTERPOL)
				{
					x = (int) ( xr + 0.5 ) ;
					y = (int) ( yr + 0.5 ) ;
					value += image[y*dimx + x] ;
				}
			}
		}

		for (k=halfcc+dz; k<=halfcc+dz+bk; k+=bkstep)
		{
			xr = curve->x[s] + k * nx  ;
			yr = curve->y[s] + k * ny  ;

			if ((xr>=1) && (yr>=1) && (xr<dimx-1) && (yr<dimy-1))
			{
				if (interpol == BXCURVE_LINEAR_INTERPOL)
				{
					x = (int) xr ;
					y = (int) yr ;

					a = xr - (float) x ;
					b = yr - (float) y ;

					mean1 += (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
					mean1 += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
					mean1 +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
					mean1 +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
					
				}

				if (interpol == BXCURVE_NEAREST_INTERPOL)
				{
					x = (int) ( xr + 0.5 ) ;
					y = (int) ( yr + 0.5 ) ;
					mean1 += image[y*dimx + x] ;
				}
			}

			xr = curve->x[s] - k * nx  ;
			yr = curve->y[s] - k * ny  ;

			if ((xr>=1) && (yr>=1) && (xr<dimx-1) && (yr<dimy-1))
			{
				if (interpol == BXCURVE_LINEAR_INTERPOL)
				{
					x = (int) xr ;
					y = (int) yr ;

					a = xr - (float) x ;
					b = yr - (float) y ;

					mean2 += (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
					mean2 += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
					mean2 +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
					mean2 +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
					
				}

				if (interpol == BXCURVE_NEAREST_INTERPOL)
				{
					x = (int) ( xr + 0.5 ) ;
					y = (int) ( yr + 0.5 ) ;
					mean2 += image[y*dimx + x] ;
				}
			}
		}

		value /= icc ; 
		mean1 /= ibk ;
		mean2 /= ibk ; 

		if (nonLinear > 0)
		{
			if ( fabs(value-mean1) > fabs(value-mean2) )
				mean1 = mean2 ;
			else
				mean2 = mean1 ;
		}

		local = value  - ( mean1 + mean2 ) / 2 ; 

		if (maxContrast > 0)
		{
			if (local > +maxContrast) local = +maxContrast ; 
			if (local < -maxContrast) local = -maxContrast ; 
		}
		global += local ; 
	}

	if (maxContrast > 0)
		global /= maxContrast ; 

	(*contrast) = global / curve->npoints ;

	return 0 ;
}

int BxCurveEdgeContrast(BxCurve* curve, short* image, int dimx, int dimy, float * contrast, float inbandsize, float deadzone, float outbandsize, float maxContrast, int interpol) 
{
	int s;
	float k;
	//float value = 0 ;
	float mean1 = 0 ;
	float mean2 = 0 ;
	//int minv  = 0 ;
	int x, y ;
	float nx, ny, xr, yr, a, b ;
	float local, global ; 

	global = 0 ;

	for (s=0; s<curve->npoints; s++)
	{	
		nx    = -curve->dy[s] ;
		ny    =  curve->dx[s] ;

		if (curve->ds[s] > 0)	
		{
			nx = nx / curve->ds[s] ; 
			ny = ny / curve->ds[s] ; 
		}
		
		mean1 = 0 ;
		mean2 = 0 ;

		for (k=deadzone; k<=deadzone+inbandsize; k++)
		{
			xr = curve->x[s] + k * nx  ;
			yr = curve->y[s] + k * ny  ;

			if ((xr>=1) && (yr>=1) && (xr<dimx-1) && (yr<dimy-1))
			{
				if (interpol == BXCURVE_LINEAR_INTERPOL)
				{
					x = (int) xr ;
					y = (int) yr ;

					a = xr - (float) x ;
					b = yr - (float) y ;

					mean1 += (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
					mean1 += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
					mean1 +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
					mean1 +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
					
				}

				if (interpol == BXCURVE_NEAREST_INTERPOL)
				{
					x = (int) ( xr + 0.5 ) ;
					y = (int) ( yr + 0.5 ) ;
					mean1 += image[y*dimx + x] ;
				}
			}
		}

		for (k=deadzone; k<=deadzone+outbandsize; k++)
		{

			xr = curve->x[s] - k * nx  ;
			yr = curve->y[s] - k * ny  ;

			if ((xr>=1) && (yr>=1) && (xr<dimx-1) && (yr<dimy-1))
			{
				if (interpol == BXCURVE_LINEAR_INTERPOL)
				{
					x = (int) xr ;
					y = (int) yr ;

					a = xr - (float) x ;
					b = yr - (float) y ;

					mean2 += (1-a) * (1-b) * image[   y   * dimx +   x  ] ;
					mean2 += (1-a) *   b   * image[ (y+1) * dimx +   x  ] ;
					mean2 +=   a   * (1-b) * image[   y   * dimx + (x+1)] ;
					mean2 +=   a   *   b   * image[ (y+1) * dimx + (x+1)] ;
					
				}

				if (interpol == BXCURVE_NEAREST_INTERPOL)
				{
					x = (int) ( xr + 0.5 ) ;
					y = (int) ( yr + 0.5 ) ;
					mean2 += image[y*dimx + x] ;
				}
			}
		}

		mean1 /= (float) inbandsize ;
		mean2 /= (float) outbandsize ; 

		local = mean1 - mean2  ; 

		if (maxContrast > 0)
		{
			if (local > +maxContrast) local = +maxContrast ; 
			if (local < -maxContrast) local = -maxContrast ; 
		}
		
		global += local ; 	

	}

	if (maxContrast > 0)
		global /= maxContrast ; 

	(*contrast) = global / curve->npoints ;

	return 0 ;
}

int BxCurveAddLandmark(BxCurve* curve, int s) 
{
	if (curve->nlandmarks < BXCURVE_MAX_LANDMARKS)
	{
		curve->slandmark[curve->nlandmarks] = s ;
		curve->nlandmarks++ ;
		return 0 ;
	}
	else
		return -1 ;
}

int BxCurveAddLandmarkClosestTo(BxCurve* curve, float x, float y)
{
	int   closest = 0 ;
	float mind, curd ;
	int s ;

	mind = (curve->x[0]-x)*(curve->x[0]-x) + (curve->y[0]-y)*(curve->y[0]-y) ;

	for (s=1; s<curve->npoints; s++)
	{
		curd = (curve->x[s]-x)*(curve->x[s]-x) + (curve->y[s]-y)*(curve->y[s]-y) ;
		if ( curd < mind )
		{
			closest = s ;
			mind = curd ;
		}
	}

	return BxCurveAddLandmark(curve, closest) ; 
}

int BxCurveRemoveAllLandmarks(BxCurve* curve) 
{
	curve->nlandmarks = 0 ; 
	return 0 ; 
}

int BxCurveInflexionPoints(BxCurve* curve, int * sInflex, int * nInflex) 
{
	int s ;
	(*nInflex) = 0 ;

	for (s=1; s<curve->npoints-1; s++)
		if ( curve->curvature[s] * curve->curvature[s-1] < 0 )
		{
			sInflex[(*nInflex)] = s ; 
			(*nInflex)++ ;	
		}

	return 0 ;
}

int BxCurveCenter(BxCurve* curve, float * xCenter, float * yCenter)  
{
	int s ; 

	(*xCenter) = (*yCenter) = 0.f ; 
	for (s=0; s <curve->npoints; s++)
	{
		(*xCenter) += curve->x[s] ; 
		(*yCenter) += curve->y[s] ; 
	}

	(*xCenter) /= curve->npoints ; 
	(*yCenter) /= curve->npoints ; 

	return 0 ; 
}

int BxCurveFindClosest(float * vector, float value, int min, int max, int * index)
{
	float best = (vector[min]-value)*(vector[min]-value) ;
	float current ;
	int k = 0 ;

	(*index) = min ;
	for (k=min+1; k<max; k++)
	{
		current = (vector[k]-value)*(vector[k]-value) ;
		if ( current < best )
		{
			(*index) = k ;
			best = current ;
		}
	}

	return 0 ;
}

int BxCurveLinearPairing(float * ds_source, float * ds_target, int nsource, int ntarget, int * pairing, int offset)
{
	float cumSource[BXCURVE_MAX_NPOINTS] ;
	float cumTarget[BXCURVE_MAX_NPOINTS] ;
	int s = 0 ;
	int lastindex = 0 ;
	float sourceLength = 0 ; 
	float targetLength = 0 ; 

	for (s=0; s<nsource; s++)
		sourceLength += ds_source[s] ;

	for (s=0; s<ntarget; s++)
		targetLength += ds_target[s] ;
	
	if (sourceLength > 0)
	{
		cumSource[0] = 0 ;
		for (s=1; s<nsource; s++)
			cumSource[s] = cumSource[s-1] + ds_source[s] / sourceLength ;
	}

	if (targetLength > 0)
	{
		cumTarget[0] = 0 ;
		for (s=1; s<ntarget; s++)
			cumTarget[s] = cumTarget[s-1] + ds_target[s] / targetLength ;
	}

	for (s=0; s<nsource; s++)
	{
		BxCurveFindClosest(cumTarget,cumSource[s],lastindex,ntarget,&(pairing[s])) ;
		lastindex = pairing[s] ;
	}

	for (s=0; s<nsource; s++)
		pairing[s] += offset ;

	return 0 ;
}


int BxCurvePointWisePairingLinear(BxCurve* curve, BxCurve* reference, int * pairing) 
{
	BxCurveLinearPairing(curve->ds, reference->ds, curve->npoints, reference->npoints, pairing, 0) ;
	return 0 ;
}

int BxCurvePointWisePairingPiecewiseLinear(BxCurve* curve, BxCurve* reference, int * pairing) 
{
	int segment ;
	int curve_last_ind = 0 ; 
	int reference_last_ind = 0 ; 

	if (curve->nlandmarks != reference->nlandmarks)
		return -1 ;

	for (segment=0; segment<curve->nlandmarks; segment++)
	{
		BxCurveLinearPairing( 
			&(curve->ds[curve_last_ind]), 
			&(reference->ds[reference_last_ind]), 
			curve->slandmark[segment]-curve_last_ind+1, 
			reference->slandmark[segment]-reference_last_ind+1, 
			&(pairing[curve_last_ind]),
			reference_last_ind
		) ;

		curve_last_ind = curve->slandmark[segment] ;
		reference_last_ind = reference->slandmark[segment] ;
	}

	BxCurveLinearPairing( 
			&(curve->ds[curve_last_ind]), 
			&(reference->ds[reference_last_ind]), 
			curve->npoints-curve_last_ind, 
			reference->npoints-reference_last_ind, 
			&(pairing[curve_last_ind]),
			reference_last_ind 
		) ;

	return 0 ;
}

int BxCurveHausdorffDistance( BxCurve * c1, BxCurve * c2, float * distance )
{
	int a, b ;
	float curD, minD, maxD ;
	float x1, y1 ; 

	maxD = 0 ; 
	for (a=0; a<c1->npoints; a++)
	{
		x1 = c1->x[a] ; 
		y1 = c1->y[a] ; 
		minD = FLT_MAX ; 
		for (b=0; b<c2->npoints; b++)
		{
			curD = (x1-c2->x[b])*(x1-c2->x[b]) + (y1-c2->y[b])*(y1-c2->y[b]) ;
			if (curD < minD) minD = curD ; 
		}
		if (minD > maxD)
			maxD = minD ; 
	}

	for (b=0; b<c2->npoints; b++)
	{
		x1 = c2->x[b] ; 
		y1 = c2->y[b] ; 
		minD = FLT_MAX ; 
		for (a=0; a<c1->npoints; a++)
		{
			curD = (x1-c1->x[a])*(x1-c1->x[a]) + (y1-c1->y[a])*(y1-c1->y[a]) ;
			if (curD < minD) minD = curD ; 
		}
		if (minD > maxD)
			maxD = minD ; 
	}

	(*distance) = (float) sqrt(maxD) ; 

	return 0 ; 
}

int BxCurveUpsampleCoordinates             (BxCurve* curve, int upsample) 
{
	float x[BXCURVE_MAX_NPOINTS] ;
	float y[BXCURVE_MAX_NPOINTS] ; 
	int s ; 
	int nlandmarks ; 
	int slandmark[BXCURVE_MAX_LANDMARKS] ; 

	nlandmarks = curve->nlandmarks ; 
	memcpy(slandmark,curve->slandmark,nlandmarks*sizeof(int)) ; 

	for (s=0; s<curve->npoints; s++)
	{
		x[s] = upsample * curve->x[s] ; 
		y[s] = upsample * curve->y[s] ; 
	}

	BxCurveInit(curve,x,y,curve->npoints) ; 

	curve->nlandmarks = nlandmarks ; 
	memcpy(curve->slandmark,slandmark,nlandmarks*sizeof(int)) ; 

	return 0 ; 
}

int BxCurveTrim(BxCurve* curve, int trim) 
{
	float x[BXCURVE_MAX_NPOINTS] ;
	float y[BXCURVE_MAX_NPOINTS] ; 
	int s ; 
	int nlandmarks ; 
	int slandmark[BXCURVE_MAX_LANDMARKS] ; 

	if (curve->npoints-2*trim <= 0)
		return -1 ;

	nlandmarks = curve->nlandmarks ; 
	memcpy(slandmark,curve->slandmark,nlandmarks*sizeof(int)) ; 

	for (s=0; s<curve->npoints-2*trim; s++)
	{
		x[s] = curve->x[s+trim] ; 
		y[s] = curve->y[s+trim] ; 
	}

	BxCurveInit(curve,x,y,curve->npoints-2*trim) ; 
	BxCurveRemoveAllLandmarks(curve) ; 

	for (s=0; s<nlandmarks; s++)
		if (( slandmark[s] - trim >= 0 ) && (slandmark[s] - trim < curve->npoints))
			BxCurveAddLandmark(curve,slandmark[s] - trim) ;

	return 0 ; 
}


int BxCurveDownsampleCoordinates           (BxCurve* curve, int downsample) 
{
	float x[BXCURVE_MAX_NPOINTS] ;
	float y[BXCURVE_MAX_NPOINTS] ; 
	int s ; 
	int nlandmarks ; 
	int slandmark[BXCURVE_MAX_LANDMARKS] ; 

	nlandmarks = curve->nlandmarks ; 
	memcpy(slandmark,curve->slandmark,nlandmarks*sizeof(int)) ; 

	for (s=0; s<curve->npoints; s++)
	{
		x[s] = curve->x[s] / downsample ; 
		y[s] = curve->y[s] / downsample ; 
	}

	BxCurveInit(curve,x,y,curve->npoints) ; 

	curve->nlandmarks = nlandmarks ; 
	memcpy(curve->slandmark,slandmark,nlandmarks*sizeof(int)) ; 

	return 0 ; 
}

int BxCurveUnfold(BxCurve* curve) 
{
	float x[BXCURVE_MAX_NPOINTS] ;
	float y[BXCURVE_MAX_NPOINTS] ; 
	float xmean=0, ymean=0, dxmean=0, dymean=0 ; 
	int s ; 
	int nlandmarks ; 
	int slandmark[BXCURVE_MAX_LANDMARKS] ; 

	nlandmarks = curve->nlandmarks ; 
	memcpy(slandmark,curve->slandmark,nlandmarks*sizeof(int)) ; 

	for (s=0; s<curve->npoints; s++)
	{
		xmean  += curve->x[s] ;
		ymean  += curve->y[s] ; 
		if (curve->ds[s] > 0)
		{
			dxmean += curve->dx[s] / curve->ds[s] ; 
			dymean += curve->dy[s] / curve->ds[s] ; 
		}
	}

	if (curve->npoints > 0)
	{
		dxmean /= curve->npoints ; 
		dymean /= curve->npoints ; 
		xmean  /= curve->npoints ; 
		ymean  /= curve->npoints ; 
	}
	else
		return 0 ; 

	dxmean = 0 ;
	dymean = -1 ; 

	xmean -= curve->length / 2 * dxmean ; 
	ymean -= curve->length / 2 * dymean ; 

	for (s=0; s<curve->npoints; s++)
	{
		x[s] = xmean + ((float)s) / (curve->npoints-1) * dxmean * curve->length ; 
		y[s] = xmean + ((float)s) / (curve->npoints-1) * dymean * curve->length ; 
	}

	BxCurveInit(curve,x,y,curve->npoints) ; 

	curve->nlandmarks = nlandmarks ; 
	memcpy(curve->slandmark,slandmark,nlandmarks*sizeof(int)) ; 

	return 0 ; 
}

