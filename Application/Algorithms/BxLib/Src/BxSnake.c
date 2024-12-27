// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <BxSpline.h>
#include <BxCurve.h>
#include <BxSnake.h>
#include <memory.h>
#include <math.h>

//new version protected against overflow 2008-07-23

#define BXSNAKE_USE_SSE_OPTIMIZATION 1
#define BXSNAKE_USE_QUADRATIC_SPLINE 0

#if BXSNAKE_USE_SSE_OPTIMIZATION
	#include <emmintrin.h>
#endif

int BxSnakeGradient( float* in, float* out, int size) 
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

int BxSnakeInit( BxSnake* snake, float* xknot, float* yknot, int nknots, int sampling )
{
	#if (BXSNAKE_USE_SSE_OPTIMIZATION == 1)

		__declspec(align(16)) float ddx[BXCURVE_MAX_NPOINTS] ;     //PL-080129 used to be static but no thread-safe
		__declspec(align(16)) float ddy[BXCURVE_MAX_NPOINTS] ;     // ""
		__declspec(align(16)) float xcontrol[BXSNAKE_MAX_NKNOTS] ; // ""
		__declspec(align(16)) float ycontrol[BXSNAKE_MAX_NKNOTS] ; // ""
		__declspec(align(16)) float len[4];                        // ""

		float * x  = snake->curve.x ; 
		float * y  = snake->curve.y ; 
		float * dx = snake->curve.dx ; 
		float * dy = snake->curve.dy ; 
		float * ds = snake->curve.ds ; 
		float * curvature = snake->curve.curvature ; 
		BxCurve * curve   = &(snake->curve) ; 

		int nextMult4 = 0 ;

		__m128 VflTmp1, VflTmp2, VflTmp3, VflTmp4, VflTmp5, VflTmp6;
		__m128 VflLen = _mm_setzero_ps();

		int k = 0 ;

		//PL_080723 protection against overflow
		if(nknots >= BXSNAKE_MAX_NKNOTS || (nknots-1)*sampling + nknots >= BXCURVE_MAX_NPOINTS)
			return 1;

		snake->nknots   = nknots   ;
		snake->sampling = sampling ;
		snake->curve.npoints  = (snake->nknots-1)*sampling + snake->nknots ;
		snake->curve.nlandmarks = 0 ; 

		memcpy(snake->xknot, xknot, nknots*sizeof(float)) ;
		memcpy(snake->yknot, yknot, nknots*sizeof(float)) ;

		#if (BXSNAKE_USE_QUADRATIC_SPLINE == 1)
			BxDirectQuadraticSplineTransform  (snake->xknot, xcontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxDirectQuadraticSplineTransform  (snake->yknot, ycontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxIndirectQuadraticSplineTransform(xcontrol, x, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
			BxIndirectQuadraticSplineTransform(ycontrol, y, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
		#else
			BxDirectCubicSplineTransform  (snake->xknot, xcontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxDirectCubicSplineTransform  (snake->yknot, ycontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxIndirectCubicSplineTransform(xcontrol, x, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
			BxIndirectCubicSplineTransform(ycontrol, y, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
		#endif

		BxSnakeGradient(x,  dx,  curve->npoints) ;
		BxSnakeGradient(y,  dy,  curve->npoints) ;
		BxSnakeGradient(dx, ddx, curve->npoints) ;
		BxSnakeGradient(dy, ddy, curve->npoints) ;

		VflTmp4 = _mm_setzero_ps() ;
		for (k=0; k<curve->npoints;k+=4)
		{
			VflLen  = _mm_add_ps(VflLen, VflTmp4);
			VflTmp1 = _mm_load_ps(dx + k);
			VflTmp2 = _mm_load_ps(dy + k);
			VflTmp3 = _mm_mul_ps(VflTmp1, VflTmp1);
			VflTmp4 = _mm_mul_ps(VflTmp2, VflTmp2);
			VflTmp3 = _mm_add_ps(VflTmp3, VflTmp4);
			VflTmp4 = _mm_rsqrt_ps(VflTmp3);
			VflTmp4 = _mm_rcp_ps(VflTmp4);
			_mm_store_ps(ds + k, VflTmp4);
			VflTmp3 = _mm_mul_ps(VflTmp3, VflTmp4);
			VflTmp3 = _mm_rcp_ps(VflTmp3);
			VflTmp6 = _mm_cmpgt_ps(VflTmp4, _mm_setzero_ps());
			VflTmp5 = _mm_load_ps(ddy + k);
			VflTmp1 = _mm_mul_ps(VflTmp1, VflTmp5);
			VflTmp5 = _mm_load_ps(ddx + k);
			VflTmp2 = _mm_mul_ps(VflTmp2, VflTmp5);
			VflTmp1 = _mm_sub_ps(VflTmp1, VflTmp2);
			VflTmp1 = _mm_mul_ps(VflTmp1, VflTmp3);
			VflTmp1 = _mm_and_ps(VflTmp1, VflTmp6);
			_mm_store_ps(curvature + k, VflTmp1);
		}

		curve->length = 0 ;
		
		_mm_store_ps(len, VflTmp4);	
		for (k-=4; k<curve->npoints; k++)
			curve->length += len[k%4];

		_mm_store_ps(len, VflLen);
		for (k=0; k<4; k++)
			curve->length +=len[k] ;
		
		nextMult4 = ( (curve->npoints+3) / 4 ) * 4 ;
		for (k=curve->npoints; k<nextMult4; k++)
		{
			x[k]  = 0 ;
			y[k]  = 0 ;
			dx[k] = 0 ;
			dy[k] = 0 ;
			ds[k] = 1 ;
		}

		return 0 ;

	#else 

		int k = 0 ;

		float ddx[BXCURVE_MAX_NPOINTS] ;                            //PL-080129 used to be static but no thread-safe
		float ddy[BXCURVE_MAX_NPOINTS] ;							   // ""
		__declspec(align(16)) float xcontrol[BXSNAKE_MAX_NKNOTS] ;   // ""
		__declspec(align(16)) float ycontrol[BXSNAKE_MAX_NKNOTS] ;   // ""
		int nextMult4 = 0 ;												   

		float * x  = snake->curve.x ; 
		float * y  = snake->curve.y ; 
		float * dx = snake->curve.dx ; 
		float * dy = snake->curve.dy ; 
		float * ds = snake->curve.ds ; 
		float * curvature = snake->curve.curvature ; 
		BxCurve * curve   = &snake->curve ; 

		snake->nknots   = nknots   ;
		snake->sampling = sampling ;
		curve->npoints  = (snake->nknots-1)*sampling + snake->nknots ;
		curve->nlandmarks = 0 ; 

		memcpy(snake->xknot, xknot, nknots*sizeof(float)) ;
		memcpy(snake->yknot, yknot, nknots*sizeof(float)) ;

		// SSE 4-Padding needed to vectorize curvilinear contrast measure */
		nextMult4 = ( (curve->npoints+3) / 4 ) * 4 ;
		for (k=curve->npoints; k<nextMult4; k++)
		{
			x[k]  = 0 ;
			y[k]  = 0 ;
			dx[k] = 0 ;
			dy[k] = 0 ;
			ds[k] = 1 ;
		}
		
		#if (BXSNAKE_USE_QUADRATIC_SPLINE == 1)
			BxDirectQuadraticSplineTransform  (snake->xknot, xcontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxDirectQuadraticSplineTransform  (snake->yknot, ycontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxIndirectQuadraticSplineTransform(xcontrol, x, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
			BxIndirectQuadraticSplineTransform(ycontrol, y, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
		#else
			BxDirectCubicSplineTransform  (snake->xknot, xcontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxDirectCubicSplineTransform  (snake->yknot, ycontrol, snake->nknots,BXSPLINE_ANTI_MIRROR) ;
			BxIndirectCubicSplineTransform(xcontrol, x, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
			BxIndirectCubicSplineTransform(ycontrol, y, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;
		#endif

		BxSnakeGradient(x,  dx,  curve->npoints) ;
		BxSnakeGradient(y,  dy,  curve->npoints) ;
		BxSnakeGradient(dx, ddx, curve->npoints) ;
		BxSnakeGradient(dy, ddy, curve->npoints) ;

		curve->length = 0 ;
		for (k=0; k<curve->npoints;k++)
		{
			ds[k] = (float) sqrt( dx[k]*dx[k] + dy[k]*dy[k] ) ;

			if (ds[k] > 0)
				curvature[k] = ( dx[k] * ddy[k] - dy[k] * ddx[k] ) / ( ds[k] * ds[k] * ds[k] ) ;
			else
				curvature[k] = 0 ;

			curve->length += curve->ds[k] ;
		}

		return 0 ;

	#endif
}

int BxSnakeCopy(BxSnake* snake, BxSnake* newsnake)
{
	newsnake->nknots     = snake->nknots   ;
	newsnake->sampling   = snake->sampling ;
	memcpy(newsnake->xknot,snake->xknot,snake->nknots*sizeof(float)) ; 
	memcpy(newsnake->yknot,snake->yknot,snake->nknots*sizeof(float)) ; 
	BxCurveCopy(&snake->curve,&newsnake->curve) ; 

	return 0 ;
}

int BxSnakeNormalAtKnot(BxSnake* snake, int knot, float range, float* xn, float* yn, int npoints) 
{	
	return BxCurveNormal(&snake->curve, knot * (snake->sampling + 1), range, xn, yn, npoints) ;
}

int BxSnakeSlide(BxSnake* snake, int knot, float* xslide, float* yslide) 
{
	int sknot = knot *  (snake->sampling + 1) ;
	int s1    = sknot - (snake->sampling/2  ) ;
	int s2    = sknot + (snake->sampling/2  ) ;
	int k     = 0 ;
	int s     = 0 ;

	k = 0 ;
	s = 0 ;
	for (s=s1; s<=s2; s++)
	{
		xslide[k] = snake->curve.x[s] ;
		yslide[k] = snake->curve.y[s] ;
		k++ ;
	}

	return 0 ; 
}

int BxSnakeFieldOfView(BxSnake* snake, int extremity, float extension, float viewAngle, float* xf, float* yf, int npoints) 
{
	const float pi  = 3.1415927f ;

	int   s_extremity = 0 ;
	int   s_center    = 0 ;
	float stepAngle   = 0 ;
	float angle       = 0 ;
	int   k           = 0 ;
	float diameter    = 0. ;
	float ax, ay, px, py ;
	
	switch(extremity)
	{
		case 0 : 
			s_extremity = 0 ; 
			s_center    = snake->sampling + 1 ;
			break ;
		case 1 : 
			s_extremity = (snake->nknots-1) * (snake->sampling + 1) ;
			s_center    = (snake->nknots-2) * (snake->sampling + 1) ;
			break ;
		default: 
			s_extremity = 0 ; 
			s_center    = snake->sampling + 1 ;
			break ;
	}

	stepAngle = 2 * viewAngle * pi / (npoints-1) ;

	ax = snake->curve.x[s_center] ;
	ay = snake->curve.y[s_center] ;

	px = snake->curve.x[s_extremity] - ax ;
	py = snake->curve.y[s_extremity] - ay ;

	diameter = (float) sqrt(px*px + py*py) ;
	if (diameter>0)
	{
		px = px / diameter ;
		py = py / diameter ;
	}

	diameter += extension ;

	k = 0 ;
	for (angle = -pi*viewAngle ; k<npoints; angle+=stepAngle)
	{
		xf[k] = ax + diameter * ( px  * (float) cos(angle) + py * (float) sin(angle) ) ;
		yf[k] = ay + diameter * ( -px * (float) sin(angle) + py * (float) cos(angle) ) ;
		k++ ;
	}

	return 0 ; 
}


int BxSnakeTranslate(BxSnake* snake, float tx, float ty) 
{
	int k ;

	for (k=0; k<snake->nknots; k++)
	{
		snake->xknot[k]    += tx ;
		snake->yknot[k]    += ty ;
	}

	BxCurveTranslate(&snake->curve,tx,ty) ; 

	return 0 ;
}

int BxSnakeAffineTransform(BxSnake* snake, float xc, float yc, float a, float b, float c, float d, float e, float f) 
{
	float xknot[BXSNAKE_MAX_NKNOTS] ;//PL-080129 used to be static but no thread-safe
	float yknot[BXSNAKE_MAX_NKNOTS] ;// ""
	int k ;									
											
	for (k=0; k<snake->nknots; k++)			
	{
		xknot[k] = xc + a + b * (xknot[k]-xc) + c * (yknot[k]-yc)  ;
		yknot[k] = yc + d + e * (xknot[k]-xc) + f * (yknot[k]-yc)  ;
	}
 
	BxSnakeInit(snake, xknot, yknot, snake->nknots, snake->sampling) ;

	return 0 ;
}

int BxSnakeMarkersTransform(BxSnake* snake, float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22) 
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

	BxSnakeAffineTransform( snake, 
		xc1, yc1, 
		xc2-xc1,	
		scale * vx * ex + vy * ey,											
		scale * vx * ey - vy * ex,
		yc2-yc1,
		scale * vy * ex - vx * ey,
		scale * vy * ey + vx * ex ) ; 

	return 0 ;
}


int BxSnakeArcLengthResample( BxSnake * snake, int nknots, int sampling )
{
	return BxSnakeArcLengthResampleCurve( snake, &snake->curve, nknots, sampling ) ;
}

int BxSnakeArcLengthResampleCurve( BxSnake * snake, BxCurve * curve, int nknots, int sampling )
{
	int s,k ; 
	float L = curve->length ;
	float sknot ;
	float xknot[BXSNAKE_MAX_NKNOTS]  ;
	float yknot[BXSNAKE_MAX_NKNOTS]  ;
	int   lastS, minS ;
	float cumS[BXCURVE_MAX_NPOINTS] ; 
	float curE, lastE ;
	
	cumS[0] = curve->ds[0] ; 
	for (s=1; s<curve->npoints; s++)
	{
		cumS[s] = cumS[s-1] + curve->ds[s-1] ;
	}

	xknot[0] = curve->x[0] ;
	yknot[0] = curve->y[0] ;

	lastS = 0 ;
	for (k=1; k<nknots-1; k++)
	{
		sknot = k * L / (nknots-1) ;
		lastE = (sknot-cumS[lastS]) * (sknot-cumS[lastS]) ;
		minS  = lastS ;
		for (s=lastS+1; s<curve->npoints; s++)
		{
			curE = (sknot-cumS[s]) * (sknot-cumS[s]) ;
			if (curE > lastE)
			{
				minS = s-1 ;
				break ;
			}
			lastE = curE ;
		}

		xknot[k] = curve->x[minS] ;
		yknot[k] = curve->y[minS] ;
		lastS = minS ; 
	}

	xknot[nknots-1] = curve->x[curve->npoints-1] ;
	yknot[nknots-1] = curve->y[curve->npoints-1] ;

	return BxSnakeInit(snake, xknot, yknot, nknots, sampling) ;
}

int BxSnakeCenterInImage(BxSnake * snake, int dimx, int dimy)
{
	float xmean, ymean ;
	int k ;

	xmean = 0 ;
	ymean = 0 ;
	for (k=0; k<snake->curve.npoints; k++)
	{
		xmean += snake->curve.x[k] ;
		ymean += snake->curve.y[k] ;
	}
	xmean /= snake->curve.npoints ;
	ymean /= snake->curve.npoints ;

	return BxSnakeTranslate(snake, ((float)dimx-1)/2 - xmean, ((float)dimy-1)/2 - ymean) ;
}

int BxSnakeInsertKnotAtExtremity(BxSnake * snake, int extremity)
{
	int nknots = snake->nknots + 1 ;
	int k ; 
	float xknot[BXSNAKE_MAX_NKNOTS] ;
	float yknot[BXSNAKE_MAX_NKNOTS] ;

	if (extremity == 0)
	{
		xknot[0] = snake->xknot[0] ;
		yknot[0] = snake->yknot[0] ;
		xknot[1] = snake->curve.x[ snake->sampling / 2 ] ;
		yknot[1] = snake->curve.y[ snake->sampling / 2 ] ;
		for (k=2; k<nknots; k++)
		{
			xknot[k] = snake->xknot[k-1] ;
			yknot[k] = snake->yknot[k-1] ;
		}
	}
	else if (extremity == 1)
	{
		for (k=0; k<nknots-2; k++)
		{
			xknot[k] = snake->xknot[k] ;
			yknot[k] = snake->yknot[k] ;
		}
		xknot[nknots-2] = snake->curve.x[ (nknots-3) *  (snake->sampling + 1) + snake->sampling / 2 ] ; 
		yknot[nknots-2] = snake->curve.y[ (nknots-3) *  (snake->sampling + 1) + snake->sampling / 2 ] ; 
		xknot[nknots-1] = snake->xknot[nknots-2] ; 
		yknot[nknots-1] = snake->yknot[nknots-2] ; 
	}
	else
		return -1 ;

	return BxSnakeInit(snake,xknot,yknot,nknots,snake->sampling) ;
}

int BxSnakeZoom(BxSnake* snake, float zoom, float xcenter, float ycenter)
{
	int k ;

	for (k=0; k<snake->nknots; k++)
	{
		snake->xknot[k] = (snake->xknot[k]-xcenter) * zoom + xcenter ;
		snake->yknot[k] = (snake->yknot[k]-ycenter) * zoom + ycenter ;
	}

	return BxCurveZoom(&snake->curve,zoom,xcenter,ycenter) ;  ;
}

int BxSnakeShiftedCardinalSpline(BxSnake* snake, int knot, float* cardinalSpline, int normalize)
{
	float impulse[BXSNAKE_MAX_NKNOTS] ;
	float transform[BXSNAKE_MAX_NKNOTS] ;

	if ((knot < 0) || (knot >= snake->nknots))
		return -1 ; 

	memset(impulse,0,snake->nknots*sizeof(float)) ; 
	impulse[knot] = 1.0f ; 

	BxDirectCubicSplineTransform   ( impulse,   transform,      snake->nknots, BXSPLINE_ANTI_MIRROR) ;
	BxIndirectCubicSplineTransform ( transform, cardinalSpline, snake->nknots, BXSPLINE_ANTI_MIRROR, snake->sampling+1) ;

	if (normalize == 1)
	{
		int s ; 
		float sum = 0 ; 
		for (s=0; s<snake->curve.npoints; s++)
		{
			cardinalSpline[s] = (float)fabs(cardinalSpline[s]) ; 
			sum += cardinalSpline[s] ;
		}

		if (sum > 0)
			for (s=0; s<snake->curve.npoints; s++)
				cardinalSpline[s]/=sum ;
	}

	else if (normalize == 2)
	{
		int s ; 
		float max = 0 ; 
		for (s=0; s<snake->curve.npoints; s++)
		{
			cardinalSpline[s] = (float)fabs(cardinalSpline[s]) ; 
			if (cardinalSpline[s] > max)
				max = cardinalSpline[s] ; 
		}

		if (max > 0)
			for (s=0; s<snake->curve.npoints; s++)
				cardinalSpline[s]/=max ;
	}

	return 0 ; 
}