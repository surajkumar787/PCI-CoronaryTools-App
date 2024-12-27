// **********************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. * 
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        * 
// * in whole or in part is prohibited without the prior written        * 
// * consent of the copyright owner.                                    * 
// *                                                                         *
// **********************************************************************

#include <memory.h>
#include <math.h>
#include <BxSpline.h>
#include <BxCurve.h>
#include <BxVessel.h>

#define BXVESSEL_USE_SSE_OPTIMIZATION 1

#if BXVESSEL_USE_SSE_OPTIMIZATION
	#include <emmintrin.h>
#endif

int BxVesselGradient( float* in, float* out, int size) 
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

int BxVesselInit( BxVessel* vessel, float* xknot, float* yknot, float* wknot, int nknots, int sampling )
{
	#if BXVESSEL_USE_SSE_OPTIMIZATION

		__declspec(align(16)) float ddx[BXCURVE_MAX_NPOINTS] ;       //PL-080129 used to be static but no thread-safe
		__declspec(align(16)) float ddy[BXCURVE_MAX_NPOINTS] ;		// ""
		__declspec(align(16)) float xcontrol[BXVESSEL_MAX_NKNOTS] ;	// ""
		__declspec(align(16)) float ycontrol[BXVESSEL_MAX_NKNOTS] ;  // ""
		__declspec(align(16)) float len[4];						    // ""

		float * x  = vessel->centerline.x ; 
		float * y  = vessel->centerline.y ; 
		float * dx = vessel->centerline.dx ; 
		float * dy = vessel->centerline.dy ; 
		float * ds = vessel->centerline.ds ; 
		float * curvature = vessel->centerline.curvature ; 
		//float * width = vessel->width ; 
		BxCurve * curve   = &(vessel->centerline) ; 

		int nextMult4 = 0 ;

		__m128 VflTmp1, VflTmp2, VflTmp3, VflTmp4, VflTmp5, VflTmp6;
		__m128 VflLen = _mm_setzero_ps();

		int k = 0 ;

		vessel->nknots   = nknots   ;
		vessel->sampling = sampling ;
		vessel->centerline.npoints  = (vessel->nknots-1)*sampling + vessel->nknots ;
		vessel->centerline.nlandmarks = 0 ; 

		if (xknot != vessel->xknot)	memcpy(vessel->xknot, xknot, nknots*sizeof(float)) ;
		if (yknot != vessel->yknot)	memcpy(vessel->yknot, yknot, nknots*sizeof(float)) ;
		if (wknot != vessel->wknot) memcpy(vessel->wknot, wknot, nknots*sizeof(float)) ;

		BxDirectCubicSplineTransform  (vessel->xknot, xcontrol, vessel->nknots,BXSPLINE_ANTI_MIRROR) ;
		BxDirectCubicSplineTransform  (vessel->yknot, ycontrol, vessel->nknots,BXSPLINE_ANTI_MIRROR) ;

		BxIndirectCubicSplineTransform(xcontrol, x, vessel->nknots, BXSPLINE_ANTI_MIRROR, vessel->sampling+1) ;
		BxIndirectCubicSplineTransform(ycontrol, y, vessel->nknots, BXSPLINE_ANTI_MIRROR, vessel->sampling+1) ;
		BxVesselSetWidth(vessel, vessel->wknot ) ;

		BxVesselGradient(x,  dx,  curve->npoints) ;
		BxVesselGradient(y,  dy,  curve->npoints) ;
		BxVesselGradient(dx, ddx, curve->npoints) ;
		BxVesselGradient(dy, ddy, curve->npoints) ;

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
		__declspec(align(16)) float xcontrol[BXVESSEL_MAX_NKNOTS] ;   // ""
		__declspec(align(16)) float ycontrol[BXVESSEL_MAX_NKNOTS] ;// ""

		int nextMult4 = 0 ;

		float * x  = vessel->centerline.x ; 
		float * y  = vessel->centerline.y ; 
		float * dx = vessel->centerline.dx ; 
		float * dy = vessel->centerline.dy ; 
		float * ds = vessel->centerline.ds ; 
		float * curvature = vessel->centerline.curvature ; 
		float * width = vessel->width ; 
		BxCurve * curve   = &vessel->centerline ; 

		vessel->nknots   = nknots   ;
		vessel->sampling = sampling ;
		curve->npoints  = (vessel->nknots-1)*sampling + vessel->nknots ;
		curve->nlandmarks = 0 ; 

		if (xknot != vessel->xknot)	memcpy(vessel->xknot, xknot, nknots*sizeof(float)) ;
		if (yknot != vessel->yknot)	memcpy(vessel->yknot, yknot, nknots*sizeof(float)) ;
		if (wknot != vessel->wknot) memcpy(vessel->wknot, wknot, nknots*sizeof(float)) ;

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
		
		BxDirectCubicSplineTransform  (vessel->xknot, xcontrol, vessel->nknots,BXSPLINE_ANTI_MIRROR) ;
		BxDirectCubicSplineTransform  (vessel->yknot, ycontrol, vessel->nknots,BXSPLINE_ANTI_MIRROR) ;

		BxIndirectCubicSplineTransform(xcontrol, x, vessel->nknots, BXSPLINE_ANTI_MIRROR, vessel->sampling+1) ;
		BxIndirectCubicSplineTransform(ycontrol, y, vessel->nknots, BXSPLINE_ANTI_MIRROR, vessel->sampling+1) ;
		BxVesselSetWidth(vessel, vessel->wknot ) ;

		BxVesselGradient(x,  dx,  curve->npoints) ;
		BxVesselGradient(y,  dy,  curve->npoints) ;
		BxVesselGradient(dx, ddx, curve->npoints) ;
		BxVesselGradient(dy, ddy, curve->npoints) ;

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

int BxVesselCopy(BxVessel* vessel, BxVessel* newvessel)
{
	newvessel->nknots     = vessel->nknots   ;
	newvessel->sampling   = vessel->sampling ;
	memcpy(newvessel->xknot,vessel->xknot,vessel->nknots*sizeof(float)) ; 
	memcpy(newvessel->yknot,vessel->yknot,vessel->nknots*sizeof(float)) ; 

	memcpy(newvessel->wknot,vessel->wknot,vessel->nknots*sizeof(float)) ; 
	memcpy(newvessel->width,vessel->width,vessel->centerline.npoints*sizeof(float)) ; 

	BxCurveCopy(&vessel->centerline,&newvessel->centerline) ; 

	return 0 ;
}

int BxVesselSetConstantWidth(BxVessel* vessel, float   width ) 
{
	int k ; 
	float wknot[BXVESSEL_MAX_NKNOTS] ; 

	for (k=0; k<vessel->nknots; k++)
		wknot[k] = width ; 

	return BxVesselSetWidth(vessel,wknot) ; 
}

int BxVesselSetWidth(BxVessel* vessel, float * wknot ) 
{
	int nknots = vessel->nknots ; 
	memcpy(vessel->wknot,wknot,nknots*sizeof(float)) ; 
	BxQuadraticSplineInterpolation(wknot, vessel->width, vessel->nknots, vessel->sampling+1, BXSPLINE_CONSTANT) ; 

	return 0 ; 
}

int BxVesselNormalAtKnot(BxVessel* vessel, int knot, float range, float* xn, float* yn, int npoints) 
{	
	return BxCurveNormal(&vessel->centerline, knot * (vessel->sampling + 1), range, xn, yn, npoints) ;
}

int BxVesselSlide(BxVessel* vessel, int knot, float* xslide, float* yslide, int halfnpoints) 
{
	int sknot = knot *  (vessel->sampling + 1) ;
	int s1    = sknot - halfnpoints ;
	int s2    = sknot + halfnpoints ;
	int k     = 0 ;
	int s     = 0 ;
	float x, y ; 

	k = 0 ;
	s = 0 ;
	x = vessel->centerline.x[sknot] ; 
	y = vessel->centerline.y[sknot] ; 

	for (s=s1; s<=s2; s++)
	{
		if ((s >= 0) && (s<=vessel->centerline.npoints-1))
		{
			x = vessel->centerline.x[s] ; 
			y = vessel->centerline.y[s] ; 
		}
		else
		{
			x = vessel->centerline.x[sknot] ; 
			y = vessel->centerline.y[sknot] ; 
		}

		xslide[k] = x ;
		yslide[k] = y ;
		k++ ;
	}

	return 0 ; 
}

int BxVesselFieldOfView(BxVessel* vessel, int extremity, float extension, float viewAngle, float* xf, float* yf, int npoints) 
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
			s_center    = vessel->sampling + 1 ;
			break ;
		case 1 : 
			s_extremity = (vessel->nknots-1) * (vessel->sampling + 1) ;
			s_center    = (vessel->nknots-2) * (vessel->sampling + 1) ;
			break ;
		default: 
			s_extremity = 0 ; 
			s_center    = vessel->sampling + 1 ;
			break ;
	}

	stepAngle = 2 * viewAngle * pi / (npoints-1) ;

	ax = vessel->centerline.x[s_center] ;
	ay = vessel->centerline.y[s_center] ;

	px = vessel->centerline.x[s_extremity] - ax ;
	py = vessel->centerline.y[s_extremity] - ay ;

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


int BxVesselTranslate(BxVessel* vessel, float tx, float ty) 
{
	int k ;

	for (k=0; k<vessel->nknots; k++)
	{
		vessel->xknot[k]    += tx ;
		vessel->yknot[k]    += ty ;
	}

	BxCurveTranslate(&vessel->centerline,tx,ty) ; 

	return 0 ;
}

int BxVesselAffineTransform(BxVessel* vessel, float xc, float yc, float a, float b, float c, float d, float e, float f) 
{
	float xknot[BXVESSEL_MAX_NKNOTS] ;//PL-080129 used to be static but no thread-safe
	float yknot[BXVESSEL_MAX_NKNOTS] ;// ""
	float wknot[BXVESSEL_MAX_NKNOTS] ;// ""

	int k ;

	for (k=0; k<vessel->nknots; k++)
	{
		xknot[k] = xc + a + b * (vessel->xknot[k]-xc) + c * (vessel->yknot[k]-yc)  ;
		yknot[k] = yc + d + e * (vessel->xknot[k]-xc) + f * (vessel->yknot[k]-yc)  ;
	}

	memcpy(wknot,vessel->wknot,vessel->nknots*sizeof(float)) ; 
	BxVesselInit(vessel, xknot, yknot, wknot, vessel->nknots, vessel->sampling) ;

	return 0 ;
}


int BxVesselRotate(BxVessel* vessel, float theta) 
{
	float xc, yc ; 
	int s ; 

	xc = 0 ; yc = 0 ; 
	for (s = 0; s < vessel->centerline.npoints; s++)
	{
		xc += vessel->centerline.x[s] ; 
		yc += vessel->centerline.y[s] ; 
	}

	if (vessel->centerline.npoints > 0)
	{
		xc /= vessel->centerline.npoints ; 
		yc /= vessel->centerline.npoints ; 
		BxVesselAffineTransform(vessel,xc,yc,0,(float)cos(theta),(float)sin(theta),0,-(float)sin(theta),(float)cos(theta)) ; 
	}

	return 0 ; 
}

int BxVesselMarkersTransform(BxVessel* vessel, float x11, float y11, float x12, float y12, float x21, float y21, float x22, float y22) 
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

	BxVesselAffineTransform( vessel, 
		xc1, yc1, 
		xc2-xc1,	
		scale * vx * ex + vy * ey,											
		scale * vx * ey - vy * ex,
		yc2-yc1,
		scale * vy * ex - vx * ey,
		scale * vy * ey + vx * ex ) ; 

	return 0 ;
}

int BxVesselArcLengthResample( BxVessel * vessel, int nknots, int sampling )
{
	int s,k ; 
	float L = vessel->centerline.length ;
	float sknot ;
	float xknot[BXVESSEL_MAX_NKNOTS]  ;
	float yknot[BXVESSEL_MAX_NKNOTS]  ;
	float wknot[BXVESSEL_MAX_NKNOTS] ; 
	int   lastS, minS ;
	float cumS[BXCURVE_MAX_NPOINTS] ; 
	float curE, lastE ;
	BxVessel tempVessel ; 

	BxVesselCopy(vessel,&tempVessel) ; 
	
	cumS[0] = tempVessel.centerline.ds[0] ; 
	for (s=1; s<tempVessel.centerline.npoints; s++)
	{
		cumS[s] = cumS[s-1] + tempVessel.centerline.ds[s-1] ;
	}

	xknot[0] = tempVessel.xknot[0] ;
	yknot[0] = tempVessel.yknot[0] ;
	wknot[0] = tempVessel.wknot[0] ; 
	lastS = 0 ;

	for (k=1; k<nknots-1; k++)
	{
		sknot = k * L / (nknots-1) ;
		lastE = (sknot-cumS[lastS]) * (sknot-cumS[lastS]) ;
		minS  = lastS ;
		for (s=lastS+1; s<tempVessel.centerline.npoints; s++)
		{
			curE = (sknot-cumS[s]) * (sknot-cumS[s]) ;
			if (curE > lastE)
			{
				minS = s-1 ;
				break ;
			}
			lastE = curE ;
		}

		xknot[k] = tempVessel.centerline.x[minS] ;
		yknot[k] = tempVessel.centerline.y[minS] ;
		wknot[k] = vessel->width[minS] ;
		lastS = minS ; 
	}

	xknot[nknots-1] = tempVessel.xknot[tempVessel.nknots-1] ;
	yknot[nknots-1] = tempVessel.yknot[tempVessel.nknots-1] ;
	wknot[nknots-1] = tempVessel.wknot[tempVessel.nknots-1] ;

	BxVesselInit(vessel, xknot, yknot, wknot, nknots, sampling) ;

	return 0 ; 
}

int BxVesselCenterInImage(BxVessel * vessel, int dimx, int dimy)
{
	float xmean, ymean ;
	int k ;

	xmean = 0 ;
	ymean = 0 ;
	for (k=0; k<vessel->centerline.npoints; k++)
	{
		xmean += vessel->centerline.x[k] ;
		ymean += vessel->centerline.y[k] ;
	}
	xmean /= vessel->centerline.npoints ;
	ymean /= vessel->centerline.npoints ;

	return BxVesselTranslate(vessel, ((float)dimx-1)/2 - xmean, ((float)dimy-1)/2 - ymean) ;
}

int BxVesselInsertKnotAtExtremity(BxVessel * vessel, int extremity)
{
	int nknots = vessel->nknots + 1 ;
	int k ; 
	float xknot[BXVESSEL_MAX_NKNOTS] ;
	float yknot[BXVESSEL_MAX_NKNOTS] ;
	float wknot[BXVESSEL_MAX_NKNOTS] ; 

	if (extremity == 0)
	{
		xknot[0] = vessel->xknot[0] ;
		yknot[0] = vessel->yknot[0] ;
		xknot[1] = vessel->centerline.x[ vessel->sampling / 2 ] ;
		yknot[1] = vessel->centerline.y[ vessel->sampling / 2 ] ;
		for (k=2; k<nknots; k++)
		{
			xknot[k] = vessel->xknot[k-1] ;
			yknot[k] = vessel->yknot[k-1] ;
		}
	}
	else if (extremity == 1)
	{
		for (k=0; k<nknots-2; k++)
		{
			xknot[k] = vessel->xknot[k] ;
			yknot[k] = vessel->yknot[k] ;
		}
		xknot[nknots-2] = vessel->centerline.x[ (nknots-3) *  (vessel->sampling + 1) + vessel->sampling / 2 ] ; 
		yknot[nknots-2] = vessel->centerline.y[ (nknots-3) *  (vessel->sampling + 1) + vessel->sampling / 2 ] ; 
		xknot[nknots-1] = vessel->xknot[nknots-2] ; 
		yknot[nknots-1] = vessel->yknot[nknots-2] ; 
	}
	else
		return -1 ;

	memcpy(wknot,vessel->wknot,vessel->nknots*sizeof(float)) ; 
	BxVesselInit(vessel, xknot, yknot, wknot, vessel->nknots, vessel->sampling) ;

	return 0 ; 
}

int BxVesselZoom(BxVessel* vessel, float zoom, float xcenter, float ycenter)
{
	int k ;

	for (k=0; k<vessel->nknots; k++)
	{
		vessel->xknot[k] = (vessel->xknot[k]-xcenter) * zoom + xcenter ;
		vessel->yknot[k] = (vessel->yknot[k]-ycenter) * zoom + ycenter ;
	}

	return BxCurveZoom(&vessel->centerline,zoom,xcenter,ycenter) ;  ;
}


int BxVesselCenteredZoom(BxVessel* vessel, float zoom)
{
	int k ;

	float xcenter = 0, ycenter = 0 ; 
	for (k=0; k<vessel->centerline.npoints; k++)
	{
		xcenter += vessel->centerline.x[k] ; 
		ycenter += vessel->centerline.y[k] ; 
	}

	xcenter /= vessel->centerline.npoints ; 
	ycenter /= vessel->centerline.npoints ; 

	for (k=0; k<vessel->nknots; k++)
	{
		vessel->xknot[k] = (vessel->xknot[k]-xcenter) * zoom + xcenter ;
		vessel->yknot[k] = (vessel->yknot[k]-ycenter) * zoom + ycenter ;
	}

	return BxCurveZoom(&vessel->centerline,zoom,xcenter,ycenter) ;  ;
}

int BxVesselShiftedCardinalSpline(BxVessel* vessel, int knot, float* cardinalSpline, int normalize)
{
	BxShiftedCubicCardinalSpline(cardinalSpline,vessel->nknots,vessel->sampling+1,knot,normalize,BXSPLINE_ANTI_MIRROR) ; 

	return 0 ; 
}