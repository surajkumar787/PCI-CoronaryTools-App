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

#include <stdio.h>
#include <stdlib.h>

#include <SWMkxGaussian.h>


short SWMkxFastRoundShort(double val)
{
	val += DOUBLE2FIXMAGIC ;
	return ((short*)&val)[1] ; 
}

int SWMkxGaussInitOrder3(float sigma, double * pB, double * pb1, double * pb2, double * pb3, double * M)
{
	double m0 = 1.16680, m1 = 1.10783, m2 = 1.40586;
	double m1sq = m1*m1, m2sq = m2*m2;
	double B, b0, b1, b2, b3 ; 
	double q, qsq, scale ; 

	if(sigma < 3.556)
		q = -0.2568 + 0.5784 * sigma + 0.0561 * sigma * sigma;
	else
		q = 2.5091  + 0.9804 * (sigma - 3.556);

	qsq = q*q;
	b0 = (m0 + q) * (m1sq + m2sq + 2*m1*q + qsq);
	b1 = q * (2*m0*m1 + m1sq + m2sq + (2*m0 + 4*m1) * q + 3*qsq) / b0;
	b2 = -qsq * (m0 + 2*m1 + 3*q) / b0;
	b3 = qsq * q / b0;
	B = (m0 * (m1sq + m2sq))/b0;
	scale = 1.0/((1.0+b1-b2+b3)*(1.0-b1-b2-b3)*(1.0+b2+(b1-b3)*b3));
    M[0] = scale*(-b3*b1+1.0-b3*b3-b2);
    M[1] = scale*(b3+b1)*(b2+b3*b1);
    M[2] = scale*b3*(b1+b3*b2);
    M[3] = scale*(b1+b3*b2);
    M[4] = -scale*(b2-1.0)*(b2+b3*b1);
    M[5] = -scale*b3*(b3*b1+b3*b3+b2-1.0);
    M[6] = scale*(b3*b1+b2+b1*b1-b2*b2);
    M[7] = scale*(b1*b2+b3*b2*b2-b1*b3*b3-b3*b3*b3-b3*b2+b3);
    M[8] = scale*b3*(b1+b3*b2);

	*pB  = B ;
	*pb1 = b1 ;
	*pb2 = b2 ;
	*pb3 = b3 ; 

	return 0 ;
}

int SWMkxAniGaussFilterFloat2D(float * in, float* out, float sigmax, float sigmay, int dimx, int dimy)
{
	int x, y ; 
	double M[9] ;
	double bound ; 
	double* vswap ; 
	double* v0    = (double*) malloc( dimx * sizeof(double) ) ;
	double* v1    = (double*) malloc( dimx * sizeof(double) ) ;
	double* v2    = (double*) malloc( dimx * sizeof(double) ) ;
	double* v3    = (double*) malloc( dimx * sizeof(double) ) ;
	double* vbound  = (double*) malloc( dimx * sizeof(double) ) ;
	double* end ; 
	float*  begin ; 
	double u0, u1, u2, u3, p1, p2, p3 ; 
	double B, b1, b2, b3    ;

	SWMkxGaussInitOrder3(sigmax, &B, &b1, &b2, &b3, M) ; 

	for (y=0; y<dimy; y++)
	{
		bound = in[dimx-1] ;
		begin = out ; 
		end   = v0+dimx ; 
		u3 = u2 = u1 = in[0]  ;

		for (;v0<end;in++,v0++)
		{
			v0[0] = u0 =  B * in[0] + b1 * u1 + b2 * u2 + b3 * u3 ;
			u3 = u2 ; u2 = u1 ; u1 = u0 ;
		}
		v0--  ;

		u1 -= bound ; u2 -= bound ;	u3 -= bound ;
 
		p1 =  B * ( M[0] * u1 + M[1] * u2 + M[2] * u3 ) + bound  ;
		p2 =  B * ( M[3] * u1 + M[4] * u2 + M[5] * u3 ) + bound  ; 
		p3 =  B * ( M[6] * u1 + M[7] * u2 + M[8] * u3 ) + bound  ;

		out += dimx-1 ;
		*out = (float) p1 ; 

		u1 = p1 ; u2 = p2 ;	u3 = p3 ;

		v0--  ;
		out-- ;
		
		for (;out>=begin;out--,v0--)
		{
			out[0] = (float) ( u0 =  B * v0[0] + b1 * u1 + b2 * u2 + b3 * u3 ) ;
			u3 = u2 ; u2 = u1 ; u1 = u0 ; 
		}

		out += dimx+1 ; 
		v0++ ; 
	}

	out -= dimx*dimy ; 

	SWMkxGaussInitOrder3(sigmay, &B, &b1, &b2, &b3, M) ; 

	for (x=0; x<dimx; x++)
	{
		v1[x] = v2[x] = v3[x] = out[x] ;
		vbound[x] = out[(dimy-1)*dimx+x] ; 
	}

	for (y=0; y<dimy; y++)
	{
		for (x=0; x<dimx; x++)
			out[x] = (float) ( v0[x] = B * out[x] + b1 * v1[x] + b2 * v2[x] + b3 * v3[x] ) ;
		
		out += dimx  ;
		vswap = v3 ; v3 = v2 ; v2 = v1 ; v1 = v0 ; v0 = vswap ; 
	}

	out -= dimx ; 

	for (x=0; x<dimx; x++)
	{
		bound = vbound[x] ; 
		u1 = v1[x] - bound ; 
		u2 = v2[x] - bound ; 
		u3 = v3[x] - bound ; 

		v1[x] = B * ( M[0] * u1 + M[1] * u2 + M[2] * u3 ) + bound  ;
		v2[x] = B * ( M[3] * u1 + M[4] * u2 + M[5] * u3 ) + bound  ;
		v3[x] = B * ( M[6] * u1 + M[7] * u2 + M[8] * u3 ) + bound  ;

		out[x] = (float) v1[x] ; 
	}

	out -= dimx ; 

	for (y=dimy-2; y>=0; y--)
	{
		for (x=0; x<dimx; x++)
			out[x] = (float) ( v0[x] = B * out[x] + b1 * v1[x] + b2 * v2[x] + b3 * v3[x] ) ;

		out -= dimx  ;
		vswap = v3 ; v3 = v2 ; v2 = v1 ; v1 = v0 ; v0 = vswap ; 
	}

	free(v0) ;
	free(v1) ;
	free(v2) ; 
	free(v3) ;
	free(vbound) ; 
	return 0 ; 

}

int SWMkxAniGaussFilterFloat2D_SSE(float * in, float* out, float sigmax, float sigmay, int dimx, int dimy)
{
	int x, y ; 
	double M[9] ;
	double bound ; 
	double* vswap ; 
	double* v0      = (double*) _aligned_malloc( dimx * sizeof(double), 16 ) ;
	double* v1      = (double*) _aligned_malloc( dimx * sizeof(double), 16 ) ;
	double* v2      = (double*) _aligned_malloc( dimx * sizeof(double), 16 ) ;
	double* v3      = (double*) _aligned_malloc( dimx * sizeof(double), 16 ) ;
	double* vbound  = (double*) _aligned_malloc( dimx * sizeof(double), 16 ) ;
	double* end ; 
	float*  begin ; 
	double u0, u1, u2, u3, p1, p2, p3 ; 
	double B, b1, b2, b3    ;

	__m128d vb1, vb2, vb3 ; 
	__m128d vreg1, vreg2, vreg3 ; 
	__m128  vmem, vmem2 ; 
	__m128d  vB ; 

	SWMkxGaussInitOrder3(sigmax, &B, &b1, &b2, &b3, M) ; 
	vB  = _mm_set1_pd(B)   ;
	vb2 = _mm_set1_pd(b2)  ;
	vb3 = _mm_set1_pd(b3)  ;

	for (y=0; y<dimy; y++)
	{
		bound = in[dimx-1] ;
		begin = out ; 
		end   = v0+dimx ; 
		vreg1 = vreg2 = _mm_set1_pd((double)in[0]) ; 
		vb1   = _mm_set_pd(0, b1) ; 

		for (;v0<end;v0+=4,in+=4)
		{
			vmem   = _mm_load_ps( in ) ; 
			vreg3  = _mm_cvtps_pd(vmem) ; 
			vreg3  = _mm_mul_pd(vreg3,vB) ; 
			vreg2  = _mm_mul_pd(vreg2,vb3) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg1,vb2) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vreg2  = _mm_shuffle_pd(vreg1,vreg1,1) ; 
			vreg2  = _mm_mul_pd(vreg2,vb1) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg3,vb1) ; 
			vreg2  = _mm_shuffle_pd(vreg2,vreg2,1) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			_mm_store_pd(v0, vreg3);

			vreg2  = _mm_shuffle_pd(vreg1,vreg3,1) ;
			vmem   = _mm_shuffle_ps(vmem,vmem,_MM_SHUFFLE(0,0,3,2)) ;
			vreg1  = _mm_cvtps_pd(vmem) ; 
			vreg1  = _mm_mul_pd(vreg1,vB) ; 
			vreg2  = _mm_mul_pd(vreg2,vb3) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg3,vb2) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vreg2  = _mm_shuffle_pd(vreg3,vreg3,1) ; 
			vreg2  = _mm_mul_pd(vreg2,vb1) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg1,vb1) ; 
			vreg2  = _mm_shuffle_pd(vreg2,vreg2,1) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			_mm_store_pd(v0+2, vreg1);
			vreg2    = _mm_shuffle_pd(vreg3,vreg1,1) ;
		}

		v0-- ; 

		u1 = v0[0] - bound ;
		u2 = v0[-1] - bound ;
		u3 = v0[-2] - bound ;
 
		p1 =  B * ( M[0] * u1 + M[1] * u2 + M[2] * u3 ) + bound  ;
		p2 =  B * ( M[3] * u1 + M[4] * u2 + M[5] * u3 ) + bound  ; 
		p3 =  B * ( M[6] * u1 + M[7] * u2 + M[8] * u3 ) + bound  ;

		u1 = p1 ;
		u2 = p2 ;
		u3 = p3 ; 		  
		u0 = ( u1 - B * v0[0] - b1 * u2 - b2 * u3 ) / b3 ; 

		vreg1 = _mm_set_pd(u3,u2) ; 
		vreg2 = _mm_set_pd(u0,u3) ; 
		vb1   = _mm_set_pd(b1, 0) ; 

		v0 -= 3 ; 
		out += dimx-4 ;

		for (;out>=begin;out-=4,v0-=4)
		{
			vreg3  = _mm_load_pd(v0+2);
			vreg3  = _mm_mul_pd(vreg3,vB) ; 
			vreg2  = _mm_mul_pd(vreg2,vb3) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg1,vb2) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vreg2  = _mm_shuffle_pd(vreg1,vreg1,1) ; 
			vreg2  = _mm_mul_pd(vreg2,vb1) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg3,vb1) ; 
			vreg2  = _mm_shuffle_pd(vreg2,vreg2,1) ; 
			vreg3  = _mm_add_pd(vreg3,vreg2) ; 
			vmem2  = _mm_cvtpd_ps(vreg3) ; 
			vreg2  = _mm_shuffle_pd(vreg3,vreg1,1) ;
			vreg1  = _mm_load_pd(v0);
			vreg1  = _mm_mul_pd(vreg1,vB) ; 
			vreg2  = _mm_mul_pd(vreg2,vb3) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg3,vb2) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vreg2  = _mm_shuffle_pd(vreg3,vreg3,1) ; 
			vreg2  = _mm_mul_pd(vreg2,vb1) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vreg2  = _mm_mul_pd(vreg1,vb1) ; 
			vreg2  = _mm_shuffle_pd(vreg2,vreg2,1) ; 
			vreg1  = _mm_add_pd(vreg1,vreg2) ; 
			vmem   = _mm_cvtpd_ps(vreg1) ; 
			vmem2  = _mm_shuffle_ps(vmem2,vmem2,_MM_SHUFFLE(1,0,2,2)) ;
			vmem   = _mm_or_ps(vmem,vmem2) ;
			_mm_store_ps(out,vmem) ; 
			vreg2    = _mm_shuffle_pd(vreg1,vreg3,1) ;
		}

		v0  += 4 ;
		out += dimx+4 ; 
	}

	out -= dimx*dimy ; 

	SWMkxGaussInitOrder3(sigmay, &B, &b1, &b2, &b3, M) ; 

	vB  = _mm_set1_pd(B)   ;
	vb1 = _mm_set1_pd(b1)  ; 
	vb2 = _mm_set1_pd(b2)  ;
	vb3 = _mm_set1_pd(b3)  ;

	for (x=0; x<dimx; x++)
	{
		v1[x] = v2[x] = v3[x] = out[x] ;
		vbound[x] = out[(dimy-1)*dimx+x] ; 
	}

	for (y=0; y<dimy; y++)
	{
		for (x=0; x<dimx; x+=4)
		{
			vmem  = _mm_load_ps(out+x)     ;
			vmem2 = _mm_shuffle_ps(vmem,vmem,_MM_SHUFFLE(0,0,3,2)) ;
			vreg1 = _mm_cvtps_pd(vmem)      ; 
			vreg1 = _mm_mul_pd(vreg1,vB)    ;
			vreg2 = _mm_load_pd(v1+x)       ;
			vreg2 = _mm_mul_pd(vreg2,vb1)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v2+x)       ;
			vreg2 = _mm_mul_pd(vreg2,vb2)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v3+x)       ;
			vreg2 = _mm_mul_pd(vreg2,vb3)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			_mm_store_pd(v0+x,vreg1)        ;
			vmem  = _mm_cvtpd_ps(vreg1)     ;
			vreg1 = _mm_cvtps_pd(vmem2)     ; 
			vreg1 = _mm_mul_pd(vreg1,vB)    ;
			vreg2 = _mm_load_pd(v1+x+2)     ;
			vreg2 = _mm_mul_pd(vreg2,vb1)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v2+x+2)     ;
			vreg2 = _mm_mul_pd(vreg2,vb2)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v3+x+2)     ;
			vreg2 = _mm_mul_pd(vreg2,vb3)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			_mm_store_pd(v0+x+2,vreg1)      ;
			vmem2  = _mm_cvtpd_ps(vreg1)    ;
			vmem2  = _mm_shuffle_ps(vmem2,vmem2,_MM_SHUFFLE(1,0,2,2)) ;
			vmem   = _mm_or_ps(vmem,vmem2) ;
			_mm_store_ps(out+x,vmem)       ;
		}

		out += dimx  ;
		vswap = v3 ; v3 = v2 ; v2 = v1 ; v1 = v0 ; v0 = vswap ; 
	}

	out -= dimx ; 

	for (x=0; x<dimx; x++)
	{
		bound = vbound[x] ; 
		u1 = v1[x] - bound ; 
		u2 = v2[x] - bound ; 
		u3 = v3[x] - bound ; 

		v1[x] = B * ( M[0] * u1 + M[1] * u2 + M[2] * u3 ) + bound  ;
		v2[x] = B * ( M[3] * u1 + M[4] * u2 + M[5] * u3 ) + bound  ;
		v3[x] = B * ( M[6] * u1 + M[7] * u2 + M[8] * u3 ) + bound  ;

		out[x] = (float) v1[x] ; 
	}

	out -= dimx ; 

	for (y=dimy-2; y>=0; y--)
	{
		for (x=0; x<dimx; x+=4)
		{
			vmem  = _mm_load_ps(out+x)     ;
			vmem2 = _mm_shuffle_ps(vmem,vmem,_MM_SHUFFLE(0,0,3,2)) ;
			vreg1 = _mm_cvtps_pd(vmem)      ; 
			vreg1 = _mm_mul_pd(vreg1,vB)    ;
			vreg2 = _mm_load_pd(v1+x)       ;
			vreg2 = _mm_mul_pd(vreg2,vb1)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v2+x)       ;
			vreg2 = _mm_mul_pd(vreg2,vb2)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v3+x)       ;
			vreg2 = _mm_mul_pd(vreg2,vb3)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			_mm_store_pd(v0+x,vreg1)        ;
			vmem  = _mm_cvtpd_ps(vreg1)     ;
			vreg1 = _mm_cvtps_pd(vmem2)     ; 
			vreg1 = _mm_mul_pd(vreg1,vB)    ;
			vreg2 = _mm_load_pd(v1+x+2)     ;
			vreg2 = _mm_mul_pd(vreg2,vb1)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v2+x+2)     ;
			vreg2 = _mm_mul_pd(vreg2,vb2)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			vreg2 = _mm_load_pd(v3+x+2)     ;
			vreg2 = _mm_mul_pd(vreg2,vb3)   ;
			vreg1 = _mm_add_pd(vreg1,vreg2) ;
			_mm_store_pd(v0+x+2,vreg1)      ;
			vmem2  = _mm_cvtpd_ps(vreg1)    ;
			vmem2  = _mm_shuffle_ps(vmem2,vmem2,_MM_SHUFFLE(1,0,2,2)) ;
			vmem   = _mm_or_ps(vmem,vmem2) ;
			_mm_store_ps(out+x,vmem)       ;
		}
			
		out -= dimx  ;
		vswap = v3 ; v3 = v2 ; v2 = v1 ; v1 = v0 ; v0 = vswap ; 
	}

	_aligned_free(v0) ;
	_aligned_free(v1) ;
	_aligned_free(v2) ; 
	_aligned_free(v3) ;
	_aligned_free(vbound) ; 

	//printf("in: %x out: %x\n",(int)(float*)in, (int)(float*)out);

	return 0 ; 

}

int SWMkxAniGaussFilterShort2D(short * in, short* out, float sigmax, float sigmay, int dimx, int dimy)
{
	int x, y ; 
	double M[9] ;
	double bound ; 
	double* vswap ; 
	double* v0    = (double*) malloc( dimx * sizeof(double) ) ;
	double* v1    = (double*) malloc( dimx * sizeof(double) ) ;
	double* v2    = (double*) malloc( dimx * sizeof(double) ) ;
	double* v3    = (double*) malloc( dimx * sizeof(double) ) ;
	double* vbound  = (double*) malloc( dimx * sizeof(double) ) ;
	double* end ; 
	short*  begin ; 
	double u0, u1, u2, u3, p1, p2, p3 ; 
	double B, b1, b2, b3 ;

	SWMkxGaussInitOrder3(sigmax, &B, &b1, &b2, &b3, M) ; 

	for (y=0; y<dimy; y++)
	{
		bound = in[dimx-1] ;
		begin = out ; 
		end   = v0+dimx ; 
		u3 = u2 = u1 = in[0]  ;

		for (;v0<end;in++,v0++)
		{
			v0[0] = u0 =  B * in[0] + b1 * u1 + b2 * u2 + b3 * u3 ;
			u3 = u2 ; u2 = u1 ; u1 = u0 ;
		}
		v0--  ;

		u1 -= bound ; u2 -= bound ;	u3 -= bound ;
			
		p1 =  B * ( M[0] * u1 + M[1] * u2 + M[2] * u3 ) + bound  ;
		p2 =  B * ( M[3] * u1 + M[4] * u2 + M[5] * u3 ) + bound  ; 
		p3 =  B * ( M[6] * u1 + M[7] * u2 + M[8] * u3 ) + bound  ;
		
		out += dimx-1 ;
		out[0] = SWMkxFastRoundShort(p1) ; 

		u1 = p1 ; u2 = p2 ;	u3 = p3 ;

		v0--  ;
		out-- ;

		for (;out>=begin;out--,v0--)
		{
			u0 =  B * v0[0] + b1 * u1 + b2 * u2 + b3 * u3 ; 
			out[0] = SWMkxFastRoundShort(u0) ;  
			u3 = u2 ; u2 = u1 ; u1 = u0 ; 
		}

		out += dimx+1 ; 
		v0++ ; 
	}

	out -= dimx*dimy ; 

	SWMkxGaussInitOrder3(sigmay, &B, &b1, &b2, &b3, M) ; 

	for (x=0; x<dimx; x++)
	{
		v1[x] = v2[x] = v3[x] = out[x] ;
		vbound[x] = out[(dimy-1)*dimx+x] ; 
	}

	for (y=0; y<dimy; y++)
	{
		for (x=0; x<dimx; x++)
		{
			v0[x] = B * out[x] + b1 * v1[x] + b2 * v2[x] + b3 * v3[x] ; 
			out[x] = SWMkxFastRoundShort(v0[x]) ;
		}
		
		out += dimx  ;
		vswap = v3 ; v3 = v2 ; v2 = v1 ; v1 = v0 ; v0 = vswap ; 
	}

	out -= dimx ; 

	for (x=0; x<dimx; x++)
	{
		bound = vbound[x] ; 
		u1 = v1[x] - bound ; 
		u2 = v2[x] - bound ; 
		u3 = v3[x] - bound ; 

		v1[x] = B * ( M[0] * u1 + M[1] * u2 + M[2] * u3 ) + bound  ;
		v2[x] = B * ( M[3] * u1 + M[4] * u2 + M[5] * u3 ) + bound  ;
		v3[x] = B * ( M[6] * u1 + M[7] * u2 + M[8] * u3 ) + bound  ;

		out[x] = SWMkxFastRoundShort(v1[x]) ; 
	}

	out -= dimx ; 

	for (y=dimy-2; y>=0; y--)
	{
		for (x=0; x<dimx; x++)
		{
			v0[x] = B * out[x] + b1 * v1[x] + b2 * v2[x] + b3 * v3[x] ; 
			out[x] = SWMkxFastRoundShort(v0[x]) ;
		}

		out -= dimx  ;
		vswap = v3 ; v3 = v2 ; v2 = v1 ; v1 = v0 ; v0 = vswap ; 
	}

	free(v0) ;
	free(v1) ;
	free(v2) ; 
	free(v3) ;
	free(vbound) ; 
	return 0 ; 
}

int SWMkxAniGaussFilterFastShort2D(short * in, short* out, float sigmax, float sigmay, int dimx, int dimy)
{
	short   *AddInR, *AddOut;
	short   *AddInRU1, *AddInRU2, *AddInRU3;
	short   *AddInRD1, *AddInRD2, *AddInRD3;
	short   ssTmpa, ssTmpb;
	int     x,y;
	double   Norm ; 
	double   Kernel[4] ; 

	__m128i  Vss1, Vss2, Vss3, Vss4;

	__m128i  VssUnit, VssGHor;
	__m128i  VssGVerL, VssGVer, VssGVerR;
	__m128i  VssKer0x, VssKer1x, VssKer2x, VssKer3x;
	__m128i  VssKer0y, VssKer1y, VssKer2y, VssKer3y;

	/* Registers loading ---------------------------------------------------------------*/
	/*----------------------------------------------------------------------------------*/
	

	VssUnit  = _mm_set1_epi16(1);

	Kernel[0] = 1 ; 
	Norm = 1 ; 
	for (x=1;x<4;x++)
	{
		Kernel[x] = exp( - 0.5*x*x/(sigmax*sigmax) ) ; 
		Norm     += 2.0 * Kernel[x] ;
	}

	VssKer0x  = _mm_set1_epi16( (short) ( Kernel[0]/ Norm * (1<<15) + 0.5 )  ) ;
	VssKer1x  = _mm_set1_epi16( (short) ( Kernel[1]/ Norm * (1<<15) + 0.5 )  ) ;
	VssKer2x  = _mm_set1_epi16( (short) ( Kernel[2]/ Norm * (1<<15) + 0.5 )  ) ;
	VssKer3x  = _mm_set1_epi16( (short) ( Kernel[3]/ Norm * (1<<15) + 0.5 )  ) ;

	Kernel[0] = 1 ; 
	Norm = 1 ; 
	for (y=1;y<4;y++)
	{
		Kernel[y] = exp( - 0.5*y*y/(sigmay*sigmay) ) ; 
		Norm     += 2.0 * Kernel[y] ;
	}

	VssKer0y  = _mm_set1_epi16( (short) ( Kernel[0]/ Norm * (1<<15) + 0.5 )  ) ;
	VssKer1y  = _mm_set1_epi16( (short) ( Kernel[1]/ Norm * (1<<15) + 0.5 )  ) ;
	VssKer2y  = _mm_set1_epi16( (short) ( Kernel[2]/ Norm * (1<<15) + 0.5 )  ) ;
	VssKer3y  = _mm_set1_epi16( (short) ( Kernel[3]/ Norm * (1<<15) + 0.5 )  ) ;

	in  += 3*dimx ;
	out += 3*dimx ; 
	for (y=3; y<dimy-3; y++, in+=dimx, out+=dimx)
	{
		AddInR   = in + 8;
		AddOut   = out;
		AddInRU1 = AddInR   - dimx;
		AddInRU2 = AddInRU1 - dimx;
		AddInRU3 = AddInRU2 - dimx;
		AddInRD1 = AddInR   + dimx;
		AddInRD2 = AddInRD1 + dimx;
		AddInRD3 = AddInRD2 + dimx;

		Vss1     = _mm_load_si128((__m128i *) &AddInR[-8]);      // Vss1 = In(i, j)                    (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer0y, VssGVer, Vss2, Vss3)  // VssGVer = Ker0*In(i,j)

		Vss1     = _mm_load_si128((__m128i *) &AddInRU1[-8]);    // Vss1 = In(i-1, j)                  (Vss:1)
		Vss2     = _mm_load_si128((__m128i *) &AddInRD1[-8]);    // Vss2 = In(i+1, j)                  (Vss:1,2)
		Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In1 = In(i-1, j)+In(i+1,j)  (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer1y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker1*In1                    (Vss:1)
		VssGVer  = _mm_adds_epi16(VssGVer, Vss1);                // VssGVer += Ker1*In1  

		Vss1     = _mm_load_si128((__m128i *) &AddInRU2[-8]);    // Vss1 = In(i-2, j)                  (Vss:1)
		Vss2     = _mm_load_si128((__m128i *) &AddInRD2[-8]);    // Vss2 = In(i+2, j)                  (Vss:1,2)
		Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In2 = In(i-2, j)+In(i+2,j)  (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer2y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker2*In2                    (Vss:1)
		VssGVer  = _mm_adds_epi16(VssGVer, Vss1);                // VssGVer += Ker2*In2 

		Vss1     = _mm_load_si128((__m128i *) &AddInRU3[-8]);    // Vss1 = In(i-3, j)                  (Vss:1)
		Vss2     = _mm_load_si128((__m128i *) &AddInRD3[-8]);    // Vss2 = In(i+3, j)                  (Vss:1,2)
		Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In3 = In(i-3, j)+In(i+3,j)  (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer3y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker3*In3                    (Vss:1)
		VssGVer  = _mm_adds_epi16(VssGVer, Vss1);                // VssGVer += Ker3*In3  

		Vss1     = _mm_load_si128((__m128i *) AddInR);           // Vss1 = In(i, j+8)                  (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer0y, VssGVerR, Vss2, Vss3)  // VssGVerR = Ker0*In(i,j+8)

		Vss1     = _mm_load_si128((__m128i *) AddInRU1);         // Vss1 = In(i-1, j+8)                (Vss:1)
		Vss2     = _mm_load_si128((__m128i *) AddInRD1);         // Vss2 = In(i+1, j+8)                (Vss:1,2)
		Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In1=In(i-1, j+8)+In(i+1,j+8)(Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer1y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker1*In1                    (Vss:1)
		VssGVerR = _mm_adds_epi16(VssGVerR, Vss1);               // VssGVerR += Ker1*In1  

		Vss1     = _mm_load_si128((__m128i *) AddInRU2);         // Vss1 = In(i-2, j+8)                (Vss:1)
		Vss2     = _mm_load_si128((__m128i *) AddInRD2);         // Vss2 = In(i+2, j+8)                (Vss:1,2)
		Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In2=In(i-2, j+8)+In(i+2,j+8)(Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer2y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker2*In2                    (Vss:1)
		VssGVerR = _mm_adds_epi16(VssGVerR, Vss1);               // VssGVerR += Ker2*In2  

		Vss1     = _mm_load_si128((__m128i *) AddInRU3);         // Vss1 = In(i-3, j+8)                (Vss:1)
		Vss2     = _mm_load_si128((__m128i *) AddInRD3);         // Vss2 = In(i+3, j+8)                (Vss:1,2)
		Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In3=In(i-3, j+8)+In(i+3,j+8)(Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer3y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker3*In3                    (Vss:1)
		VssGVerR = _mm_adds_epi16(VssGVerR, Vss1);               // VssGVerR += Ker3*In3 

		VEC_MULTIPLY_ROUND(VssGVer, VssKer0x, VssGHor, Vss2, Vss3)// VssGHor = Ker0*GV(i,j)     (Vss:1)
		
		Vss1     = _mm_srli_si128(VssGVer, 2);
		ssTmpa   = _mm_extract_epi16(VssGVerR, 0);
		Vss1     = _mm_insert_epi16(Vss1, ssTmpa, 7);            // Vss1  = GV(i,j+1)          (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer1x, Vss1, Vss2, Vss3)      // Vss1  = Ker1*GV(i,j+1)     (Vss:1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker1*GV(i,j+1)	
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker1*GV(i,j+1)

		Vss1     = _mm_srli_si128(VssGVer, 4);
		Vss2     = _mm_slli_si128(VssGVerR, 12);
		Vss1     = _mm_or_si128(Vss1, Vss2);                     // Vss1  = GV(i,j+2)          (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer2x, Vss1, Vss2, Vss3)      // Vss1  = Ker2*GV(i,j+2)     (Vss:1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker2*GV(i,j+2)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker2*GV(i,j+2)

		Vss1     = _mm_srli_si128(VssGVer, 6);
		Vss2     = _mm_slli_si128(VssGVerR, 10);
		Vss1     = _mm_or_si128(Vss1, Vss2);                     // Vss1  = GV(i,j+3)          (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer3x, Vss1, Vss2, Vss3)      // Vss1  = Ker3*GV(i,j+3)     (Vss:1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker3*GV(i,j+3)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker3*GV(i,j+3)

		_mm_store_si128((__m128i *) AddOut, VssGHor);            // VssGHor -> *AddOut

		VssGVerL = VssGVer;
		VssGVer  = VssGVerR;

		/* Process a Line on a Vector Basis ------------------------------------------------*/
		/* (NbIter = PixelCount/NbPixelInVector = PixelCount/8) ----------------------------*/
		/*----------------------------------------------------------------------------------*/
		for (x=8; x<dimx-8; x+=8)
		{
			Vss1     = _mm_load_si128((__m128i *) &AddInR[x]);     // Vss1 = In(i, j+8)                  (Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer0y, VssGVerR, Vss2, Vss3)  // VssGVerR = Ker0*In(i,j+8)

			Vss1     = _mm_load_si128((__m128i *) &AddInRU1[x]);   // Vss1 = In(i-1, j+8)                (Vss:1)
			Vss2     = _mm_load_si128((__m128i *) &AddInRD1[x]);   // Vss2 = In(i+1, j+8)                (Vss:1,2)
			Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In1=In(i-1, j+8)+In(i+1,j+8)(Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer1y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker1*In1                    (Vss:1)
			VssGVerR = _mm_adds_epi16(VssGVerR, Vss1);               // VssGVerR += Ker1*In1  

			Vss1     = _mm_load_si128((__m128i *) &AddInRU2[x]);   // Vss1 = In(i-2, j+8)                (Vss:1)
			Vss2     = _mm_load_si128((__m128i *) &AddInRD2[x]);   // Vss2 = In(i+2, j+8)                (Vss:1,2)
			Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In2=In(i-2, j+8)+In(i+2,j+8)(Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer2y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker2*In2                    (Vss:1)
			VssGVerR = _mm_adds_epi16(VssGVerR, Vss1);               // VssGVerR += Ker2*In2  

			Vss1     = _mm_load_si128((__m128i *) &AddInRU3[x]);   // Vss1 = In(i-3, j+8)                (Vss:1)
			Vss2     = _mm_load_si128((__m128i *) &AddInRD3[x]);   // Vss2 = In(i+3, j+8)                (Vss:1,2)
			Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = In3=In(i-3, j+8)+In(i+3,j+8)(Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer3y, Vss1, Vss2, Vss3)   	 // Vss1 = Ker3*In3                    (Vss:1)
			VssGVerR = _mm_adds_epi16(VssGVerR, Vss1);               // VssGVerR += Ker3*In3 

			VEC_MULTIPLY_ROUND(VssGVer, VssKer0x, VssGHor, Vss2, Vss3)// VssGHor = Ker0*GV(i,j)

			COMBINE_VEC_1(VssGVer, VssGVerR, VssGVer, VssGVerL, Vss1, Vss2, ssTmpa, ssTmpb)
																	// Vss1 = GV(i+1,j), Vss2 = GV(i-1,j) (Vss:1,2)		
			Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = GV1 = GV(i-1,j) + GV(i+1,j) (Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer1x, Vss1, Vss2, Vss3)   	 // Vss1 = Ker1*GV1                    (Vss:1)
			VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VssGHor += Ker1*GV1 

			COMBINE_VEC_2(VssGVer, VssGVerR, VssGVer, VssGVerL, Vss1, Vss2, Vss3, Vss4)
																	// Vss1 = GV(i+2,j), Vss2 = GV(i-2,j) (Vss:1,2)		
			Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = GV2 = GV(i-2,j) + GV(i+2,j) (Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer2x, Vss1, Vss2, Vss3)   	 // Vss1 = Ker2*GV2                    (Vss:1)
			VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VssGHor += Ker2*GV2 	
			
			COMBINE_VEC_3(VssGVer, VssGVerR, VssGVer, VssGVerL, Vss1, Vss2, Vss3, Vss4)
																	// Vss1 = GV(i+3,j), Vss2 = GV(i-3,j) (Vss:1,2)		
			Vss1     = _mm_adds_epi16(Vss1, Vss2);                   // Vss1 = GV3 = GV(i-3,j) + GV(i+3,j) (Vss:1)
			VEC_MULTIPLY_ROUND(Vss1, VssKer3x, Vss1, Vss2, Vss3)   	 // Vss1 = Ker3*GV3                    (Vss:1)
			VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VssGHor += Ker3*GV3 	

			_mm_store_si128((__m128i *) &AddOut[x], VssGHor);      // VssGHor -> *AddOut
			
			VssGVerL = VssGVer;
			VssGVer  = VssGVerR;

		}

		VEC_MULTIPLY_ROUND(VssGVer, VssKer0x, VssGHor, Vss2, Vss3)// VssGHor = Ker0*GV(i,j)     (Vss:1)
		
		Vss1     = _mm_slli_si128(VssGVer, 2);
		ssTmpb   = _mm_extract_epi16(VssGVerL, 7);
		Vss1     = _mm_insert_epi16(Vss1, ssTmpb, 0);            // Vss1  = GV(i,j-1)          (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer1x, Vss1, Vss2, Vss3)      // Vss1  = Ker1*GV(i,j-1)     (Vss:1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker1*GV(i,j-1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker1*GV(i,j-1)

		Vss1     = _mm_slli_si128(VssGVer, 4);
		Vss2     = _mm_srli_si128(VssGVerL, 12);
		Vss1     = _mm_or_si128(Vss1, Vss2);                     // Vss1  = GV(i,j-2)          (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer2x, Vss1, Vss2, Vss3)      // Vss1  = Ker2*GV(i,j-2)     (Vss:1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker2*GV(i,j-2)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker2*GV(i,j-2)

		Vss1     = _mm_slli_si128(VssGVer, 6);
		Vss2     = _mm_srli_si128(VssGVerL, 10);
		Vss1     = _mm_or_si128(Vss1, Vss2);                     // Vss1  = GV(i,j-3)          (Vss:1)
		VEC_MULTIPLY_ROUND(Vss1, VssKer3x, Vss1, Vss2, Vss3)      // Vss1  = Ker3*GV(i,j-3)     (Vss:1)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker3*GV(i,j-3)
		VssGHor  = _mm_adds_epi16(VssGHor, Vss1);                // VGHor += Ker3*GV(i,j-3)

		_mm_store_si128((__m128i *) &AddOut[x], VssGHor);      // VssGHor -> *AddOut
	}

	return(0);
}



extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterShort2D(short * in, short* out, float sigma, int dimx, int dimy)
{
	return SWMkxAniGaussFilterShort2D(in,out,sigma,sigma,dimx,dimy) ; 
}

extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterFloat2D(float * in, float* out, float sigma, int dimx, int dimy)
{
	return SWMkxAniGaussFilterFloat2D(in,out,sigma,sigma,dimx,dimy) ; 
}

extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterFastShort2D(short * in, short* out, float sigma, int dimx, int dimy)
{
	return SWMkxAniGaussFilterFastShort2D(in,out,sigma,sigma,dimx,dimy) ; 
}

extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterFloat2D_SSE(float * in, float* out, float sigma, int dimx, int dimy)
{
	return SWMkxAniGaussFilterFloat2D_SSE(in, out, sigma, sigma, dimx, dimy) ;
}






