// ***************************************************************************
// * Copyright (c) 2002-2014 Philips Medical Research Paris, Philips France. *
// *                                                                         *
// * All rights reserved. Reproduction or disclosure to third parties        *
// * in whole or in part is prohibited without the prior written             *
// * consent of the copyright owner.                                         *
// *                                                                         *
// * Author: Pierre Lelong                                                   *
// ***************************************************************************
//PL_141230

#include <BxSpline.h>
#include <stdlib.h>
#include <memory.h>

#define BXSPLINE_BUFFER_SIZE      4096
#define BXSPLINE_MIN_K0           16
#define BXSPLINE_CUBIC_ALPHA     -0.26794919243112f
#define BXSPLINE_CUBIC_BETA       1.73205080756888f
#define BXSPLINE_QUADRATIC_ALPHA -0.17157287525381f
#define BXSPLINE_QUADRATIC_BETA   1.41421356237309f

/******************************************************************************/
int BxBoundaryExtension(float * in, float * out, int size, int boundType, int m)
{
	int k       = 0 ;
	int off1    = 0 ;
	int off2    = 0 ;
	// int newsize = size + 2*m ;

	memcpy(out+m, in, size * sizeof(float)) ;

	if (boundType == BXSPLINE_ANTI_MIRROR)
	{
		off1 = m+size ;
		off2 = size-2 ;
		for (k=0; k<m; k++)
		{
			out[k]      = 2*in[0] - in[m-k] ;
			out[off1+k] = 2*in[size-1] - in[off2-k] ;
		}
		return 0 ;
	}

	if (boundType == BXSPLINE_CYCLIC)
	{
		off1 = m+size ;
		off2 = size-2 ;
		for (k=0; k<m; k++)
		{
			out[k]      = in[size-m+k-1] ;
			out[off1+k] = in[k+1] ;
		}
		return 0 ;
	}

	if (boundType == BXSPLINE_CONSTANT)
	{
		off1 = m+size ;
		off2 = size-2 ;
		for (k=0; k<m; k++)
		{
			out[k]      = in[0] ;
			out[off1+k] = in[size-1] ;
		}
		return 0 ;
	}

	/* Default : Mirror */
	off1 = m+size ;
	off2 = size-2 ;
	for (k=0; k<m; k++)
	{
		out[k]      = in[m-k] ;
		out[off1+k] = in[off2-k] ;
	}
	return 0 ;
	
}

/********************************************************/
int BxUpsample( float * in, float * out, int m, int size )
{
	int upsize   = m*(size-1)+1 ;
	int k        = 0 ;

	memset(out, 0, upsize*sizeof(float)) ;
	
	for (k=0; k<size; k++)
		out[k*m] = in[k] ;

	return 0 ;
}

/*****************/
int BxIsEven(int m)
{
	if ( m%2 == 0 )
		return 1 ;
	else
		return 0 ;
}

/******************************************************************************/
int BxMovingAverageFilter( float * in, float * out, int m, int size, int shift )
{
	int h       = m >> 1 ;
	int k       = 0 ;
	float inv_m = 1.f / m ;
	int prev    =     h - shift ;
	int next    = m - h + shift ;
	float val ;

	val = 0 ;
	for (k=0;k<h;k++)
		val += in[h-k] ;

	for (k=h;k<m;k++)
		val += in[k-h] ;

	val *= inv_m ;
	out[0] = val ;

	for (k=1; prev>0; k++, prev--, next++)
	{
		val += inv_m * ( in[next] - in[prev] ) ;
		out[k] = val ;
	}

	for (; next<size-1; k++, prev++, next++)
	{
		val += inv_m * ( in[next] - in[prev] ) ;
		out[k] = val ;	
	}

	for (; k<size; k++, prev++, next--)
	{
		val += inv_m * ( in[next] - in[prev] ) ;
		out[k] = val ;	
	}

	return 0 ;
}

#pragma optimize("",off) //EA-PL-141223 rustine pour la discrepancy debug/release
/************************************************************************************/
int BxMovingAverageFilterNoNorm( float * in, float * out, int m, int size, int shift )
{
	int h        = m >> 1 ;
	int k        = 0 ;
	float * prev = in + h - shift ;
	float * next = in + m -h + shift ;
	float * end  = out + size ; 
	float val ;

	val = 0 ;
	for (k=0;k<h;k++)
		val += in[h-k] ;

	for (k=h;k<m;k++)
		val += in[k-h] ;

	*out++ = val ;

	for (;prev>in;)
	{
		val += *next++ - *prev--  ;
		*out++ = val ;
	}

	for (;next<in+size-1;)
	{
		val += *next++ - *prev++ ;
		*out++ = val ;	
	}

	for (;out<end;)
	{
		val += *next-- - *prev++  ;
		*out++ = val ;	
	}

	return 0 ;
}

#pragma optimize("",on)

/**********************************************************************************/
int BxDirectCubicSplineTransform( float * in, float * out, int size, int boundType )
{
	int   k0         = size-1 < BXSPLINE_MIN_K0 ? size-1 : BXSPLINE_MIN_K0 ;
	int   k          = 0 ;
	int   mirsize    = size + 2*k0 ;
	float val ;

	float yp      [BXSPLINE_BUFFER_SIZE] ;//PL-080129 used to be static but no thread-safe
	float ym      [BXSPLINE_BUFFER_SIZE] ;// ""
	float buff    [BXSPLINE_BUFFER_SIZE] ;// ""

	BxBoundaryExtension(in, buff, size, boundType, k0 ) ;

	yp[0] = buff[0] * 1/(1-BXSPLINE_CUBIC_ALPHA) ;
	val = yp[0] ;
	for (k=1; k<mirsize; k++)
	{
		val   = BXSPLINE_CUBIC_ALPHA * val + buff[k]  ;
		yp[k] = val ;
	}

	ym[mirsize-1] = yp[mirsize-1] ;

	val = ym[mirsize-1] ;
	for (k=mirsize-2; k>=0; k--)
	{
		val = BXSPLINE_CUBIC_ALPHA * val + buff[k]  ;
		ym[k] = val ;
	}

	for (k=k0; k<k0+size; k++)
		out[k-k0] = BXSPLINE_CUBIC_BETA * ( ym[k] + yp[k] - buff[k] ) ;

	return 0 ;
}

/**************************************************************************************/
int BxDirectQuadraticSplineTransform( float * in, float * out, int size, int boundType )
{
	int   k0         = size-1 < BXSPLINE_MIN_K0 ? size-1 : BXSPLINE_MIN_K0 ;
	int   k          = 0 ;
	int   mirsize    = size + 2*k0 ;
	float val ;

	float yp      [BXSPLINE_BUFFER_SIZE] ;//PL-080129 used to be static but no thread-safe
	float ym      [BXSPLINE_BUFFER_SIZE] ;// ""
	float buff    [BXSPLINE_BUFFER_SIZE] ;// ""

	BxBoundaryExtension(in, buff, size, boundType, k0 ) ;

	yp[0] = buff[0] * 1/(1-BXSPLINE_QUADRATIC_ALPHA) ;
	val = yp[0] ;
	for (k=1; k<mirsize; k++)
	{
		val   = BXSPLINE_QUADRATIC_ALPHA * val + buff[k]  ;
		yp[k] = val ;
	}

	ym[mirsize-1] = yp[mirsize-1] ;

	val = ym[mirsize-1] ;
	for (k=mirsize-2; k>=0; k--)
	{
		val = BXSPLINE_QUADRATIC_ALPHA * val + buff[k]  ;
		ym[k] = val ;
	}

	for (k=k0; k<k0+size; k++)
		out[k-k0] = BXSPLINE_QUADRATIC_BETA * ( ym[k] + yp[k] - buff[k] ) ;

	return 0 ;
}

/*******************************************************************************************/
int BxIndirectCubicSplineTransform( float * in, float * out, int size, int boundType, int m )
{
	float buff1[BXSPLINE_BUFFER_SIZE] ;//PL-080129 used to be static but no thread-safe
	float buff2[BXSPLINE_BUFFER_SIZE] ;// ""
											  
	int upsize  = size ;
	int mirsize = size ;
	int shift   = 0 ;
	int k ;
	int mirror  = 1 ;
	int offset = mirror*m  ;
	const float norma = 1.f / (6*m*m*m) ; 

	if (m > 1)
	{
		BxBoundaryExtension(in,buff1,size,boundType,mirror) ;
		mirsize = size + 2*mirror ;

		BxUpsample(buff1, buff2, m, mirsize) ;
		upsize = m*(mirsize-1) + 1 ;

		if (BxIsEven(m))
			shift = 1 ;
		
		BxMovingAverageFilterNoNorm(buff2, buff1, m, upsize, shift ) ;
		BxMovingAverageFilterNoNorm(buff1, buff2, m, upsize, 0     ) ;
		BxMovingAverageFilterNoNorm(buff2, buff1, m, upsize, shift ) ;
		BxMovingAverageFilterNoNorm(buff1, buff2, m, upsize, 0     ) ;

	}
	else
	{
		BxBoundaryExtension(in,buff2,size,boundType,mirror) ;
		upsize = size + 2*mirror ;
	}

	for (k=offset; k<offset+ m*(size-1)+1; k++)
			out[k-offset] = norma * ( buff2[k-1] + 4 * buff2[k] + buff2[k+1] ) ;

	return 0 ;
}


/*******************************************************************************************/
int BxIndirectQuadraticSplineTransform( float * in, float * out, int size, int boundType, int m )
{
	float buff1[BXSPLINE_BUFFER_SIZE] ;//PL-080129 used to be static but no thread-safe
	float buff2[BXSPLINE_BUFFER_SIZE] ;// ""
											  
	int upsize  = size ;
	int mirsize = size ;
	int shift   = 0 ;
	int k ;
	int mirror = 1 ; 
	int offset = mirror*m  ;
	const float norma = 1.f / (8*m*m) ; 

	if (m > 1)
	{
		BxBoundaryExtension(in,buff1,size,boundType,mirror) ;
		mirsize = size + 2*mirror ;

		BxUpsample(buff1, buff2, m, mirsize) ;
		upsize = (mirsize-1) * (m-1) + mirsize ;

		if (BxIsEven(m))
			shift = 1 ;
		
		BxMovingAverageFilterNoNorm(buff2, buff1, m, upsize, 0 ) ;
		BxMovingAverageFilterNoNorm(buff1, buff2, m, upsize, shift ) ;
		BxMovingAverageFilterNoNorm(buff2, buff1, m, upsize, 0 ) ;
	}
	else
	{
		BxBoundaryExtension(in,buff1,size,boundType,mirror) ;
		upsize = size + 2*mirror ;
	}

	if (BxIsEven(m))
		for (k=offset; k<offset+(m*(size-1)+1); k++)
				out[k-offset] = 4 * norma * ( buff1[k+1] + buff1[k] ) ;
	else
		for (k=offset; k<offset+(m*(size-1)+1); k++)
				out[k-offset] = norma * ( buff1[k-1] + 6 * buff1[k] + buff1[k+1] ) ;
		


	return 0 ;
}

/**************************************************************************************/
int BxShiftLinearPreFilter2D( float * in, float * out, int dimx, int dimy, float shift )
{
	float a = - shift / (1.f-shift) ; 
	float b =     1.f / (1.f-shift) ; 
	float value ; 
	int ind ; 
	int prec ; 
	int x, y ; 

	for (y=0; y<dimy; y++)
	{
		ind = y*dimx ; 
		out[ind] = in[ind] ;
		value    = in[ind] ; 
		for (x=ind+1; x<ind+dimx; x++)
		{
			value *= a ; 
			value += b * in[x] ; 
			out[x] = value ; 
		}
	}

	for (y=1; y<dimy; y++)
	{
		prec = (y-1)*dimx ;
		for (ind=y*dimx; ind<(y+1)*dimx; ind++, prec++)
		{
			out[ind] = b * out[ind] + a * out[prec] ; 
		}
	}

	return 0 ; 

}

/*************************************************************************/
int BxLinearSplineInterpolation( float * in, float * out, int size, int m )
{
	int knot, k;
	int ind = 0 ; 
	float a ; 

	for (knot = 0 ; knot < size-1 ; knot++)
	{
		for (k=0; k<m; k++, ind++)
		{
			a = ((float)k)/m ; 
			out[ind] = (1-a) * in[knot] + a * in[knot+1];
		}
	}
	out[ind] = in[size-1] ; 
	return 0 ; 
}

/***************************************************************************************/
int BxCubicSplineInterpolation( float * in, float * out, int size, int m, int boundType ) 
{
	float * transform = (float*) malloc( size * sizeof(float) ) ; 

	BxDirectCubicSplineTransform(in,transform,size,boundType) ; 
	BxIndirectCubicSplineTransform(transform,out,size,boundType,m) ; 

	free(transform) ; 
	return 0 ;  
}

/*******************************************************************************************/
int BxQuadraticSplineInterpolation( float * in, float * out, int size, int m, int boundType ) 
{
	float * transform = (float*) malloc( size * sizeof(float) ) ; 

	BxDirectQuadraticSplineTransform(in,transform,size,boundType) ; 
	BxIndirectQuadraticSplineTransform(transform,out,size,boundType,m) ; 

	free(transform) ; 
	return 0 ;
}

/***************************************************************************************************************/
int BxShiftedCubicCardinalSpline(float* cardinalSpline, int size, int m, int shift, int normalize, int boundType)
{
	float * impulse   = (float*) malloc( size * sizeof(float) ) ; 
	int outSize = m*(size-1) + 1 ; 

	if ((shift < 0) || (shift >= size)){
		free(impulse);
		return -1;
	}

	memset(impulse,0,size*sizeof(float)) ; 
	impulse[shift] = 1.0f ; 

	BxCubicSplineInterpolation( impulse, cardinalSpline, size, m, boundType) ;

	if (normalize == 1)
	{
		int s ; 
		float sum = 0 ; 
		for (s=0; s<outSize; s++)
		{
			if (cardinalSpline[s] < 0)
				cardinalSpline[s] = -cardinalSpline[s] ; 
			sum += cardinalSpline[s] ;
		}

		if (sum > 0)
			for (s=0; s<outSize; s++)
				cardinalSpline[s]/=sum ;
	}

	else if (normalize == 2)
	{
		int s ; 
		float max = 0 ; 
		for (s=0; s<outSize; s++)
		{
			if (cardinalSpline[s] < 0)
				cardinalSpline[s] = -cardinalSpline[s] ; 
			if (cardinalSpline[s] > max)
				max = cardinalSpline[s] ; 
		}

		if (max > 0)
			for (s=0; s<outSize; s++)
				cardinalSpline[s]/=max ;
	}

	free(impulse) ; 
	return 0 ; 
}
