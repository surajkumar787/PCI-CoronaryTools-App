// ***************************************************************************
// * Copyright (c) 2002-2011 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <BxWire.h>
#include <BxSnake.h>
#include <BxGaussian.h>
#include <math.h>
#include <time.h>
#include <float.h>

#define BXWIRE_DEFAULT_SIMILARITY_WEIGHT     50
#define BXWIRE_DEFAULT_CONTRAST_DEADZONE     1.5f
#define BXWIRE_DEFAULT_EXPECTED_CONTRAST     3.f
#define BXWIRE_DEFAULT_INTERNAL_SAMPLING     48 
#define BXWIRE_DEFAULT_OUTPUT_RESOLUTION     0.5f
#define BXWIRE_DEFAULT_INTERNAL_FIELDOFVIEW  0.4f 
#define BXWIRE_DEFAULT_INTERNAL_NSEARCH      64 
#define BXWIRE_DEFAULT_MODEL_TYPE            Recursive
#define BXWIRE_DEFAULT_INIT_SEARCH           1 

#define BXWIRE_MAX_ITERATIONS       8
#define BXWIRE_INIT_NSEARCH_DOWNS   2
#define BXWIRE_INIT_SAMPLING_DOWNS  2

#define BXWIRE_CONTRAST_LIMITER     1.1f 
#define BXWIRE_SSE_OPTIMIZATION     1

#define BXWIRE_ISOTROPIC_CONTRAST   0
#define BXWIRE_LOWPASS_SIGMA        3
#define BXWIRE_HIGHPASS_GAIN        16

#define BXWIRE_RECURSIVE_UPDATE     0.8f

#if (BXWIRE_SSE_OPTIMIZATION == 1)
	#include <emmintrin.h>
	#define COMPUTE_IMOFFSET(vy1, vy2, vx1, vx2, voff1, voff2)  \
	vy1    = _mm_packs_epi32(vy1, vy2)      ;                   \
	vy2    = _mm_mullo_epi16(vy1, vdimx)    ;                   \
	vy1    = _mm_mulhi_epi16(vy1, vdimx)    ;                   \
	voff1  = _mm_unpacklo_epi16( vy2, vy1 ) ;                   \
	voff2  = _mm_unpackhi_epi16( vy2, vy1 ) ;                   \
	voff1  = _mm_add_epi32( voff1, vx1 )    ;                   \
	voff2  = _mm_add_epi32( voff2, vx2 )    ;
#endif

typedef struct { 

	float * curvature ; 
	int   npoints ; 

} BxWireShapeModel ; 

typedef struct {

	float    ExpectedContrast    ;
	float    ContrastDeadZone    ; 
	float    SimilarityWeight    ;
	float    OutputResolution    ;
	int      InternalSampling    ;
	float    InternalFieldOfView ;
	int      InternalNSearch     ;
	int      InitSearch          ;
	
	float Alpha        ;
	float Beta         ; 
	float Gamma        ;
	float TargetLength ; 

	float    InternalRecursion   ; 
	
	BxWireTrackingMode TrackingMode ; 

	BxWireShapeModel * CoarseModel  ;
	BxWireShapeModel * FineModel    ;
	BxWireShapeModel * CurrentModel ; 

} BxWire ;

int BxWireDefaultParameters( BxWireParameters * params, int nBitsDyna )
{
    // _set_SSE2_enable(0);

	params->ExpectedContrast     = (float)              BXWIRE_DEFAULT_EXPECTED_CONTRAST * (float) pow(2,nBitsDyna-8)  ;
	params->SimilarityWeight     = (float)              BXWIRE_DEFAULT_SIMILARITY_WEIGHT     ;
	params->ContrastDeadZone     = (float)              BXWIRE_DEFAULT_CONTRAST_DEADZONE     ; 
	params->InternalSampling     = (int)                BXWIRE_DEFAULT_INTERNAL_SAMPLING     ;
	params->InternalFieldOfView  = (float)              BXWIRE_DEFAULT_INTERNAL_FIELDOFVIEW  ;
	params->InternalNSearch      = (int)                BXWIRE_DEFAULT_INTERNAL_NSEARCH      ;
	params->OutputResolution     = (float)              BXWIRE_DEFAULT_OUTPUT_RESOLUTION     ;
	params->TrackingMode         = (BxWireTrackingMode) BXWIRE_DEFAULT_MODEL_TYPE            ; 
	params->InitSearch           = (int)                BXWIRE_DEFAULT_INIT_SEARCH           ;

	return 0 ;
}

int BxWireCreate( void** handle, BxWireParameters * params )
{
	BxWire* that = (BxWire*) malloc( sizeof(BxWire) ) ;

	that->ExpectedContrast    = params->ExpectedContrast    ;
	that->SimilarityWeight    = params->SimilarityWeight    ;
	that->ContrastDeadZone    = params->ContrastDeadZone    ;
	that->InternalSampling    = params->InternalSampling    ;
	that->InternalFieldOfView = params->InternalFieldOfView ;
	that->InternalNSearch     = params->InternalNSearch     ;
	that->OutputResolution    = params->OutputResolution    ;
	that->InitSearch          = params->InitSearch          ;
	that->TrackingMode        = params->TrackingMode        ;
	that->CurrentModel        = NULL ; 
	that->CoarseModel         = NULL ; 
	that->FineModel           = NULL ; 
	
	that->Alpha        = 0.9f ; 
	that->Beta         = 0.1f ; 
	that->Gamma        = 2    ;

	that->InternalRecursion = 0 ; 

	(*handle) = (void*) that ; 
	return 0 ;
}

int BxWireDelete( void* handle )
{
	BxWire * that = (BxWire*) handle ;

	if (that->CoarseModel != NULL)
	{
		free(that->CoarseModel->curvature) ; 
		free(that->CoarseModel) ; 
		that->CoarseModel = NULL ; 
	}

	if (that->FineModel != NULL)
	{
		free(that->FineModel->curvature) ; 
		free(that->FineModel) ; 
		that->FineModel = NULL ; 
	}

	free(that) ;

	return 0 ;
}

int BxWireContrast( void* handle, BxCurve* wire, short* image, int dimx, int dimy, float* contrast)
{
	BxWire * that = (BxWire*) handle ;
	return BxCurveRidgeContrast(wire, image, dimx, dimy, contrast, 1.0f, that->ContrastDeadZone, 2.0f, that->ExpectedContrast, 0 ,BXCURVE_LINEAR_INTERPOL) ; 
}

int BxWireSetModel( void* handle, BxCurve * wire )
{
	BxWire   * that = (BxWire*) handle ;
	BxSnake  coarseSnake ; 
	BxSnake  fineSnake   ; 
	int s ; 

	if (that->FineModel != NULL)
	{
		free(that->FineModel->curvature) ; 
		free(that->FineModel) ; 
		that->FineModel = NULL ; 
	}

	if (that->CoarseModel != NULL)
	{
		free(that->CoarseModel->curvature) ; 
		free(that->CoarseModel) ; 
		that->CoarseModel = NULL ; 
	}
	
	BxSnakeArcLengthResampleCurve(&fineSnake,wire,5,that->InternalSampling) ;
	that->FineModel            = (BxWireShapeModel*) malloc( sizeof(BxWireShapeModel) ) ;
	that->FineModel->curvature = (float*)            malloc( fineSnake.curve.npoints * sizeof(float) ) ; 
	that->FineModel->npoints = fineSnake.curve.npoints ; 
	for (s=0; s<fineSnake.curve.npoints; s++)
		that->FineModel->curvature[s] = fineSnake.curve.curvature[s] ; 

	BxSnakeArcLengthResampleCurve(&coarseSnake,wire,5,that->InternalSampling / BXWIRE_INIT_SAMPLING_DOWNS) ;
	that->CoarseModel            = (BxWireShapeModel*) malloc( sizeof(BxWireShapeModel) ) ;
	that->CoarseModel->curvature = (float*)            malloc( coarseSnake.curve.npoints * sizeof(float) ) ; 
	that->CoarseModel->npoints = coarseSnake.curve.npoints ; 
	for (s=0; s<coarseSnake.curve.npoints; s++)
		that->CoarseModel->curvature[s] = coarseSnake.curve.curvature[s] ; 

	if (that->TrackingMode == Recursive)
		that->InternalRecursion = BXWIRE_RECURSIVE_UPDATE ; 

	return 0 ;
}

int BxWireUpdateModel( void* handle, BxCurve * wire, float recursive )
{
	BxWire * that = (BxWire*) handle ;
	if (that->FineModel == NULL)
		BxWireSetModel( handle, wire ) ; 
	else
	{
		int s ; 
		BxSnake coarseSnake ; 
		BxSnake fineSnake   ;
		BxSnakeArcLengthResampleCurve(&coarseSnake,wire,5,that->InternalSampling / BXWIRE_INIT_SAMPLING_DOWNS) ;
		BxSnakeArcLengthResampleCurve(&fineSnake,  wire,5,that->InternalSampling ) ;

		for (s=0; s<fineSnake.curve.npoints; s++)
			that->FineModel->curvature[s]   = recursive * that->FineModel->curvature[s] + (1-recursive) * fineSnake.curve.curvature[s] ; 

		for (s=0; s<coarseSnake.curve.npoints; s++)
			that->CoarseModel->curvature[s] = recursive * that->CoarseModel->curvature[s] + (1-recursive) * coarseSnake.curve.curvature[s] ; 
	}

	if (that->TrackingMode == Recursive)
		that->InternalRecursion *= recursive ; 

	return 0 ;
}

int BxWireNormalizedRidgeContrast( BxWire* that, BxCurve* wire, short* image, int dimx, int dimy, float* curvContrast )
{	
	#if (BXWIRE_SSE_OPTIMIZATION == 1)

		int s, k              ; 
		int background[4]     ;
		int central[4]        ;
		int contrast      = 0 ; 
		float maxContrast = that->ExpectedContrast * BXWIRE_CONTRAST_LIMITER ; 
		 __declspec(align(16)) int offc[4], off1[4], off2[4], off3[4], off4[4]  ;//PL-080129 used to be static but no thread-safe
		__m128i vxi1, vyi1, vxi2, vyi2 ;											   
		__m128i voff1, voff2 ;														   
		__m128i  vdimx   = _mm_set1_epi16((short)dimx) ;
		__m128 vnx, vny, vns, vxf, vyf ;
		__m128 vfdz    = _mm_set1_ps((float)(that->ContrastDeadZone+1)) ;
		__m128 vftemp1, vftemp2, vftemp3, vftemp4  ;
		__m128 vfdimx  = _mm_set1_ps((float)(dimx-1)) ;
		__m128 vfdimy  = _mm_set1_ps((float)(dimy-1)) ;
			
		(*curvContrast) = 0 ;

		for (s=0; s<wire->npoints; s+=4)
		{	
			vnx   = _mm_load_ps( wire->dy + s ) ;
			vnx   = _mm_sub_ps ( _mm_setzero_ps(), vnx ) ;
			vny   = _mm_load_ps( wire->dx + s ) ;
			vns   = _mm_load_ps( wire->ds + s ) ;
			vns   = _mm_rcp_ps ( vns ) ;
			vnx   = _mm_mul_ps ( vnx, vns ) ;
			vny   = _mm_mul_ps ( vny, vns ) ;
			vxf   = _mm_load_ps( wire->x + s ) ;
			vyf   = _mm_load_ps( wire->y + s ) ;
			vxf   = _mm_max_ps( vxf, _mm_setzero_ps() ) ;
			vyf   = _mm_max_ps( vyf, _mm_setzero_ps() ) ;
			vxf   = _mm_min_ps( vxf, vfdimx ) ;
			vyf   = _mm_min_ps( vyf, vfdimy ) ;
			vxi1  = _mm_cvtps_epi32(vxf);
			vyi1  = _mm_cvtps_epi32(vyf);
			vyi1  = _mm_packs_epi32(vyi1, _mm_setzero_si128());                     
			vyi2  = _mm_mullo_epi16(vyi1, vdimx) ; 
			vyi1  = _mm_mulhi_epi16(vyi1, vdimx) ;
			voff1 = _mm_unpacklo_epi16( vyi2, vyi1 ) ;
			voff1 = _mm_add_epi32( voff1, vxi1 ) ;   
			_mm_store_si128( (__m128i*) offc, voff1 ) ;
			vftemp1 = _mm_mul_ps( vnx, vfdz ) ;
			vftemp2 = _mm_mul_ps( vny, vfdz ) ;
			vftemp3 = _mm_add_ps( vftemp1, vxf ) ;
			vftemp4 = _mm_add_ps( vftemp2, vyf ) ;
			vftemp3 = _mm_max_ps( vftemp3, _mm_setzero_ps() ) ;
			vftemp4 = _mm_max_ps( vftemp4, _mm_setzero_ps() ) ;
			vftemp3 = _mm_min_ps( vftemp3, vfdimx ) ;
			vftemp4 = _mm_min_ps( vftemp4, vfdimy ) ;
			vxi1    = _mm_cvtps_epi32(vftemp3);
			vyi1    = _mm_cvtps_epi32(vftemp4);
			vftemp3 = _mm_sub_ps( vxf, vftemp1 ) ;
			vftemp4 = _mm_sub_ps( vyf, vftemp2 ) ;
			vftemp3 = _mm_max_ps( vftemp3, _mm_setzero_ps() ) ;
			vftemp4 = _mm_max_ps( vftemp4, _mm_setzero_ps() ) ;
			vftemp3 = _mm_min_ps( vftemp3, vfdimx ) ;
			vftemp4 = _mm_min_ps( vftemp4, vfdimy ) ;
			vxi2    = _mm_cvtps_epi32(vftemp3);
			vyi2    = _mm_cvtps_epi32(vftemp4);
			COMPUTE_IMOFFSET(vyi1, vyi2, vxi1, vxi2, voff1, voff2)
			_mm_store_si128( (__m128i*) off1, voff1 ) ;
			_mm_store_si128( (__m128i*) off2, voff2 ) ;
			vftemp1 = _mm_add_ps( vftemp1, vnx ) ;
			vftemp2 = _mm_add_ps( vftemp2, vny ) ;
			vftemp3 = _mm_add_ps( vftemp1, vxf ) ;
			vftemp4 = _mm_add_ps( vftemp2, vyf ) ;
			vftemp3 = _mm_max_ps( vftemp3, _mm_setzero_ps() ) ;
			vftemp4 = _mm_max_ps( vftemp4, _mm_setzero_ps() ) ;
			vftemp3 = _mm_min_ps( vftemp3, vfdimx ) ;
			vftemp4 = _mm_min_ps( vftemp4, vfdimy ) ;
			vxi1    = _mm_cvtps_epi32(vftemp3);
			vyi1    = _mm_cvtps_epi32(vftemp4);
			vftemp3 = _mm_sub_ps( vxf, vftemp1 ) ;
			vftemp4 = _mm_sub_ps( vyf, vftemp2 ) ;
			vftemp3 = _mm_max_ps( vftemp3, _mm_setzero_ps() ) ;
			vftemp4 = _mm_max_ps( vftemp4, _mm_setzero_ps() ) ;
			vftemp3 = _mm_min_ps( vftemp3, vfdimx ) ;
			vftemp4 = _mm_min_ps( vftemp4, vfdimy ) ;
			vxi2    = _mm_cvtps_epi32(vftemp3);
			vyi2    = _mm_cvtps_epi32(vftemp4);
			COMPUTE_IMOFFSET(vyi1, vyi2, vxi1, vxi2, voff1, voff2)
			_mm_store_si128( (__m128i*) off3, voff1 ) ;
			_mm_store_si128( (__m128i*) off4, voff2 ) ;

			for (k=0; k<4; k++)
			{
				background[k]  = image[ off1[k] ] ;
				background[k] += image[ off2[k] ] ;
				background[k] += image[ off3[k] ] ;
				background[k] += image[ off4[k] ] ;
				central[k]     = image[ offc[k] ] ;

				contrast = central[k] -  background[k] / 4 ;

				if (contrast < 0)
				{
					if ( contrast > - maxContrast )
						(*curvContrast) += contrast / maxContrast ;
					else
						(*curvContrast) += - 1.f ;
				}
				
			}
		}

		(*curvContrast) /= wire->npoints  ;

		return 0 ;

	#else

		int s;
		float k;
		float   dz     = that->ContrastDeadZone;
		int   bk     = 2;
		int   cc     = 1 ;
		float halfcc = ((float)cc-1)/2;
		float maxContrast = that->ExpectedContrast * BXWIRE_CONTRAST_LIMITER ; 
		int value = 0 ;
		int mean1 = 0 ;
		int mean2 = 0 ;
		int minv  = 0 ;
		float nx, ny ;
		int contrast = 0 ; 
		int norma  = 2 * cc *  bk  ;
		int x, y ;

		(*curvContrast) = 0 ;

		for (s=0; s<wire->npoints; s++)
		{	
			nx    = -wire->dy[s] ;
			ny    =  wire->dx[s] ;

			if (wire->ds[s] > 0)	
			{
				nx = nx / wire->ds[s] ; 
				ny = ny / wire->ds[s] ; 
			}
			
			value = 0 ;
			mean1 = 0 ;
			mean2 = 0 ;

			for (k=-halfcc; k<=halfcc; k++)
			{
				x = (int) ( wire->x[s] + k * nx + 0.5f ) ;
				y = (int) ( wire->y[s] + k * ny + 0.5f ) ;

				if ((x>=1) && (y>=1) && (x<dimx-1) && (y<dimy-1))
					value += image[y*dimx+x] ;
			}

			for (k=halfcc+dz+1; k<halfcc+dz+1+bk; k++)
			{
				x = (int) ( wire->x[s] + k * nx + 0.5f ) ;
				y = (int) ( wire->y[s] + k * ny + 0.5f ) ;

				if ((x>=0) && (y>=0) && (x<dimx) && (y<dimy))
					mean1 += image[y*dimx+x] ;

				x = (int) ( wire->x[s] - k * nx + 0.5f ) ;
				y = (int) ( wire->y[s] - k * ny + 0.5f ) ;

				if ((x>=0) && (y>=0) && (x<dimx) && (y<dimy))
					mean2 += image[y*dimx+x] ;
			}

			contrast = value / cc - ( mean1 + mean2 ) / (2*bk) ;

			if ( contrast < 0 )
			{
				if ( contrast > - maxContrast )
					(*curvContrast) += contrast / maxContrast ;
				else
					(*curvContrast) += - 1.f ;
			}

		}

		(*curvContrast) /= wire->npoints ;

		return 0 ;

	#endif
}

int BxWireNormalizedIsotropicContrast(BxCurve* curve, short* image, int dimx, int dimy, float* contrast, int interpol, int maxContrast) 
{
	int s ;
	int x, y ;
	float xr, yr, a, b ;
	int out = 0 ;
	int length = 0 ;
	float value = 0 ;

	(*contrast) = 0 ;
	
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

			if (maxContrast > 0)
			{
				if (value > 0)
					(*contrast) += 0 ;
				else if (value < - maxContrast * BXWIRE_HIGHPASS_GAIN )
					(*contrast) += -1.f ;
				else
					(*contrast) += value / (maxContrast * BXWIRE_HIGHPASS_GAIN) ; 
			}
			else
			{
				(*contrast) += value ; 
			}

			length ++ ;
		}
		else
			out = -1 ;
	}

	if (length > 0)
		(*contrast) /= length ;

	return out ;
}

int BxWireEnergy( BxWire* that, BxCurve* wire, short* image, int dimx, int dimy, float* energy )
{
	float contrast    = 0 ;
	float similarity  = 0 ;
	int k = 0 ;

	#if (BXWIRE_ISOTROPIC_CONTRAST == 1)
		BxWireNormalizedIsotropicContrast(wire, image, dimx, dimy, &contrast, 0, that->ExpectedContrast)  ;
	#else
		BxWireNormalizedRidgeContrast(that, wire, image, dimx, dimy, &contrast) ;
	#endif
	
	if ((that->CurrentModel != NULL)  && (that->CurrentModel->npoints == wire->npoints))
	{
		for (k=0; k<wire->npoints; k++)
			similarity += (float) fabs( that->CurrentModel->curvature[k] - wire->curvature[k] )  ; 
	
		similarity /= wire->npoints ;

		(*energy)   =  contrast + that->SimilarityWeight * (1-that->InternalRecursion) * similarity ;
	}
	else
		(*energy) = contrast ;

	return 0 ;
}

int BxWireSign( int num )
{
	if (num <  0)  return -1 ;
	if (num == 0)  return  0 ;
	if (num >  0)  return +1 ;
	return 0 ; 
}

int BxWireAbs( int num )
{
	if (num < 0) return -num ;
	else         return  num ; 
}

int BxWireFromMarkers( BxWire* that, BxCurve* wire, short* image, int dimx, int dimy, float x1, float y1, float x2, float y2 )
{
	float xknot[BXSNAKE_MAX_NKNOTS] ;
	float yknot[BXSNAKE_MAX_NKNOTS] ;
	
	int   i,j,k ;
	int   best_i, best_j, best_k, mid_i, mid_j, mid_k ;
	float curE, minE, lastMinE ;

	float  fov           = that->InternalFieldOfView ;
	int    nSearch       = 2 * (int) ( ((float) that->InternalNSearch-1) / 2 ) + 1  ;
	int    sampling      = that->InternalSampling    ;
	int    samplingInit  = sampling / BXWIRE_INIT_SAMPLING_DOWNS ;
	float  distance      = (float) sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) ) ; 
	float  searchRange   = 0 ;

	float * xbeg = (float*) malloc(nSearch * sizeof(float)) ;
	float * ybeg = (float*) malloc(nSearch * sizeof(float)) ;
	float * xmid = (float*) malloc(nSearch * sizeof(float)) ;
	float * ymid = (float*) malloc(nSearch * sizeof(float)) ;
	float * xend = (float*) malloc(nSearch * sizeof(float)) ;
	float * yend = (float*) malloc(nSearch * sizeof(float)) ;

	BxSnake curWire  ;
	BxSnake minWire  ;

	int iteration = 0 ;

	if ( fov > 0.25 )
		searchRange = distance / 2 ;
	else
		searchRange = (float) 0.5*distance * (float) tan(fov*3.1415927f) ;

	xknot[0] = (3*x1-x2) / 2 ;	yknot[0] = (3*y1-y2) / 2 ;
	xknot[1] = x1            ;  yknot[1] = y1            ;
	xknot[2] = (x1+x2) / 2   ;	yknot[2] = (y1+y2) / 2   ;
	xknot[3] = x2            ;	yknot[3] = y2            ;
	xknot[4] = (3*x2-x1) / 2 ;	yknot[4] = (3*y2-y1) / 2 ;

	if (BxSnakeInit(&minWire, xknot, yknot, 5, sampling)){
		free(xbeg); free(ybeg); free(xmid); free(ymid); free(xend); free(yend);
		return 1;
	}

	BxSnakeFieldOfView (&minWire,0,0,fov,xbeg,ybeg,nSearch) ;
	BxSnakeFieldOfView (&minWire,1,0,fov,xend,yend,nSearch) ;
	BxSnakeNormalAtKnot(&minWire,2, searchRange, xmid,ymid,nSearch) ;

	best_i = mid_i = (nSearch-1) / 2 ;
	best_j = mid_j = (nSearch-1) / 2 ;
	best_k = mid_k = (nSearch-1) / 2 ;

	if (that->InitSearch == 1)
	{
		// Init Stage (N2, restricted but quasi-exhaustive) 
		that->CurrentModel = that->CoarseModel ; 
		
		xknot[0] = xbeg[best_i] ; yknot[0] = ybeg[best_i] ;
		xknot[2] = xmid[best_j] ; yknot[2] = ymid[best_j] ;
		xknot[4] = xend[best_k] ; yknot[4] = yend[best_k] ;	

		if(BxSnakeInit(&minWire,xknot,yknot,5,samplingInit))return 1 ;
		BxWireEnergy(that,&minWire.curve,image,dimx,dimy,&minE) ;

		for (i=0; i<nSearch; i+=BXWIRE_INIT_NSEARCH_DOWNS)
		for (j=0; j<nSearch; j+=BXWIRE_INIT_NSEARCH_DOWNS)
		{
			if ( BxWireAbs( BxWireSign(i-mid_i)+BxWireSign(j-mid_j)-BxWireSign(best_k-mid_k) ) != 3 )
			{
				xknot[0] = xbeg[i] ;         yknot[0] = ybeg[i] ;
				xknot[2] = xmid[j] ;         yknot[2] = ymid[j] ;
				if(BxSnakeInit(&curWire,xknot,yknot,5,samplingInit))return 1 ;
				BxWireEnergy(that,&curWire.curve,image,dimx,dimy,&curE) ;
				if (curE < minE) 
				{ 
					best_i = i ; 
					best_j = j ;
					minE   = curE  ;	
				}
			}
		}

		xknot[0] = xbeg[best_i] ; yknot[0] = ybeg[best_i] ;
		for (j=0; j<nSearch; j+=BXWIRE_INIT_NSEARCH_DOWNS)
		for (k=0; k<nSearch; k+=BXWIRE_INIT_NSEARCH_DOWNS)
		{
			if ( BxWireAbs( BxWireSign(best_i-mid_i)+BxWireSign(j-mid_j)-BxWireSign(k-mid_k) ) != 3 )
			{
				xknot[2] = xmid[j] ;         yknot[2] = ymid[j] ;
				xknot[4] = xend[k] ;         yknot[4] = yend[k] ;
				if(BxSnakeInit(&curWire,xknot,yknot,5,samplingInit))return 1 ;
				BxWireEnergy(that,&curWire.curve,image,dimx,dimy,&curE) ;
				if (curE < minE) 
				{ 
					best_j = j ; 
					best_k = k ; 
					minE   = curE  ;	
				}
			}
		}		

		xknot[2] = xmid[best_j] ; yknot[2] = ymid[best_j] ;
		for (i=0; i<nSearch; i+=BXWIRE_INIT_NSEARCH_DOWNS)
		for (k=0; k<nSearch; k+=BXWIRE_INIT_NSEARCH_DOWNS)
		{
			if ( BxWireAbs( BxWireSign(best_i-mid_i)+BxWireSign(best_j-mid_j)-BxWireSign(k-mid_k) ) != 3 )
			{
				xknot[0] = xbeg[i] ;         yknot[0] = ybeg[i] ;
				xknot[4] = xend[k] ;         yknot[4] = yend[k] ;
				if(BxSnakeInit(&curWire,xknot,yknot,5,samplingInit))return 1 ;
				BxWireEnergy(that,&curWire.curve,image,dimx,dimy,&curE) ;
				if (curE < minE) 
				{ 
					best_i = i ; 
					best_k = k ;
					minE   = curE  ;	
				}
			}
		}
	}

	// Refine Stage (N1, full range but dimension - wise, hence non-exhaustive)  
	that->CurrentModel = that->FineModel ; 

	xknot[0] = xbeg[best_i] ; yknot[0] = ybeg[best_i] ;
	xknot[2] = xmid[best_j] ; yknot[2] = ymid[best_j] ;
	xknot[4] = xend[best_k] ; yknot[4] = yend[best_k] ;

	if(BxSnakeInit(&minWire, xknot, yknot,5,sampling))return 1 ;
	BxWireEnergy(that,&minWire.curve,image,dimx,dimy,&minE) ;

	lastMinE = minE + 1 ;

	while ((minE < lastMinE) & (iteration < BXWIRE_MAX_ITERATIONS))
	{
		iteration++ ;
		lastMinE = minE ;

		xknot[2] = xmid[best_j] ; yknot[2] = ymid[best_j] ;
		xknot[4] = xend[best_k] ; yknot[4] = yend[best_k] ;
		for (i=0; i<nSearch; i++)
		{
			if ( BxWireAbs( BxWireSign(i-mid_i)+BxWireSign(best_j-mid_j)-BxWireSign(best_k-mid_k) ) != 3 )
			{
				xknot[0] = xbeg[i] ; 
				yknot[0] = ybeg[i] ;
				if(BxSnakeInit(&curWire,xknot,yknot,5,sampling))return 1 ;
				BxWireEnergy(that,&curWire.curve,image,dimx,dimy,&curE) ;
				if (curE < minE ) 
				{ 
					best_i = i ;
					minE   = curE  ;
				}
			}
		}
		
		xknot[0] = xbeg[best_i] ; yknot[0] = ybeg[best_i] ;
		xknot[4] = xend[best_k] ; yknot[4] = yend[best_k] ;
		for (j=0; j<nSearch; j++)
		{
			if ( BxWireAbs( BxWireSign(best_i-mid_i)+BxWireSign(j-mid_j)-BxWireSign(best_k-mid_k) ) != 3 )
			{
				xknot[2] = xmid[j] ; 
				yknot[2] = ymid[j] ;	
				if(BxSnakeInit(&curWire,xknot,yknot,5,sampling))return 1 ;
				BxWireEnergy(that,&curWire.curve,image,dimx,dimy,&curE) ;
				if (curE < minE ) 
				{ 
					best_j = j ;
					minE   = curE  ;
				}
			}
		}

		xknot[0] = xbeg[best_i] ; yknot[0] = ybeg[best_i] ;
		xknot[2] = xmid[best_j] ; yknot[2] = ymid[best_j] ;
		for (k=0; k<nSearch; k++)
		{
			if ( BxWireAbs( BxWireSign(best_i-mid_i)+BxWireSign(best_j-mid_j)-BxWireSign(k-mid_k) ) != 3 )
			{
				xknot[4] = xend[k] ; 
				yknot[4] = yend[k] ;
				if(BxSnakeInit(&curWire,xknot,yknot,5,sampling) )return 1;	
				BxWireEnergy(that,&curWire.curve,image,dimx,dimy,&curE) ;
				if (curE < minE ) 
				{ 
					best_k = k ;
					minE   = curE  ;
				}
			}
		}
	}

	xknot[0] = xbeg[best_i] ; yknot[0] = ybeg[best_i] ;
	xknot[2] = xmid[best_j] ; yknot[2] = ymid[best_j] ;
	xknot[4] = xend[best_k] ; yknot[4] = yend[best_k] ;

	if(BxSnakeInit(&curWire,xknot,yknot,5,sampling))return 1 ;

	if (that->OutputResolution > 0)
	{
		sampling = (int) ( ((curWire.curve.length-5)/4) / that->OutputResolution ) + 1 ; 
		if(BxSnakeInit(&curWire,xknot,yknot,5,sampling))return 1 ;
	}

	BxCurveCopy(&curWire.curve,wire) ; 
	BxCurveAddLandmarkClosestTo( wire, x1, y1) ;
	BxCurveAddLandmarkClosestTo( wire, x2, y2) ;


	free(xbeg) ;
	free(ybeg) ;
	free(xmid) ;
	free(ymid) ;
	free(xend) ;
	free(yend) ;

	return 0 ;
}

static float flatX[5] = {0, 10, 20, 30, 40} ; 
static float flatY[5] = {0, 0, 0, 0, 0} ; 

int BxWireDoExtract( void* handle, BxCurve* wire, short* image, int dimx, int dimy, float x1, float y1, float x2, float y2 )
{
	float contrast ;  
	BxWire*  that  = (BxWire*) handle   ;
	#if (BXWIRE_ISOTROPIC_CONTRAST == 1)
		short * highpass ; 
		int k ; 
	#endif

  // _set_SSE2_enable(0);

	if ((that->TrackingMode == Last) || (that->TrackingMode == Recursive))
	{
		if (that->FineModel == NULL)
		{
			BxCurve flatCurve ; 
			BxCurveInitOpenSpline(&flatCurve,flatX,flatY,5,that->InternalSampling) ;
			BxWireSetModel(handle,&flatCurve) ; 
		}
	}

	#if (BXWIRE_ISOTROPIC_CONTRAST == 1)
		highpass = (short*) malloc(dimx*dimy*sizeof(short)) ; 
		BxIsoGaussFilterShort2D(image,highpass,BXWIRE_LOWPASS_SIGMA,dimx,dimy) ; 
		for (k=0; k<dimx*dimy; k++)
			highpass[k] = (short) BXWIRE_HIGHPASS_GAIN * ( (float) (image[k] - highpass[k]) ) ; 
		if(BxWireFromMarkers(that, wire, highpass, dimx, dimy, x1, y1, x2, y2 ))return 1 ;		
	#else
		if(BxWireFromMarkers(that, wire, image, dimx, dimy, x1, y1, x2, y2 ))return 1 ;		
	#endif

	

	if (that->TrackingMode == Last)
	{
		#if (BXWIRE_ISOTROPIC_CONTRAST == 1)
			BxWireNormalizedIsotropicContrast(wire, highpass, dimx, dimy, &contrast, 1, 0)  ;
			contrast /= BXWIRE_HIGHPASS_GAIN ; 
		#else
			BxCurveRidgeContrast(wire, image, dimx, dimy, &contrast, 1.0f, that->ContrastDeadZone, 2.0f, 0, 0 ,BXCURVE_LINEAR_INTERPOL) ; 
		#endif

		that->ExpectedContrast = -contrast ;
		BxWireSetModel(handle,wire) ; 
	}
	else if (that->TrackingMode == Recursive)
	{
		#if (BXWIRE_ISOTROPIC_CONTRAST == 1)
			BxWireNormalizedIsotropicContrast(wire, highpass, dimx, dimy, &contrast, 1, 0)  ;
			contrast /= BXWIRE_HIGHPASS_GAIN ; 
		#else
			BxCurveRidgeContrast(wire, image, dimx, dimy, &contrast, 1.0f, that->ContrastDeadZone, 2.0f, 0, 0 ,BXCURVE_LINEAR_INTERPOL) ; 
		#endif

		contrast = -contrast ; 
		that->ExpectedContrast = BXWIRE_RECURSIVE_UPDATE * that->ExpectedContrast + (1-BXWIRE_RECURSIVE_UPDATE) * contrast ;
		BxWireUpdateModel(handle,wire,BXWIRE_RECURSIVE_UPDATE) ; 
	}

	#if (BXWIRE_ISOTROPIC_CONTRAST == 1)
		free(highpass) ; 
	#endif

	return 0 ;

}
