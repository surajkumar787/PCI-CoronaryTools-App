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


#include <emmintrin.h>
#include <math.h>

#pragma once

#define DOUBLE2FIXMAGIC 68719476736*1.5+0.5

#define COMBINE_VEC_1(VssCa, VssR8a, VssCb, VssL8b, VssOutR1a, VssOutL1b, ssTmpa, ssTmpb) \
	                                                                                      \
	VssOutR1a = _mm_srli_si128(VssCa, 2);                                                 \
	VssOutL1b = _mm_slli_si128(VssCb, 2);                                                 \
	ssTmpa    = _mm_extract_epi16(VssR8a, 0);                                             \
	ssTmpb    = _mm_extract_epi16(VssL8b, 7);                                             \
	VssOutR1a = _mm_insert_epi16(VssOutR1a, ssTmpa, 7);                                   \
	VssOutL1b = _mm_insert_epi16(VssOutL1b, ssTmpb, 0);

#define COMBINE_VEC_2(VssCa, VssR8a, VssCb, VssL8b, VssOutR2a, VssOutL2b, VssTmpa, VssTmpb) \
	                                                                                        \
	VssOutR2a = _mm_srli_si128(VssCa, 4);                                                   \
	VssOutL2b = _mm_slli_si128(VssCb, 4);                                                   \
	VssTmpa   = _mm_slli_si128(VssR8a, 12);                                                 \
	VssTmpb   = _mm_srli_si128(VssL8b, 12);                                                 \
	VssOutR2a = _mm_or_si128(VssOutR2a, VssTmpa);                                           \
	VssOutL2b = _mm_or_si128(VssOutL2b, VssTmpb);

#define COMBINE_VEC_3(VssCa, VssR8a, VssCb, VssL8b, VssOutR2a, VssOutL2b, VssTmpa, VssTmpb) \
	                                                                                        \
	VssOutR2a = _mm_srli_si128(VssCa, 6);                                                   \
	VssOutL2b = _mm_slli_si128(VssCb, 6);                                                   \
	VssTmpa   = _mm_slli_si128(VssR8a, 10);                                                 \
	VssTmpb   = _mm_srli_si128(VssL8b, 10);                                                 \
	VssOutR2a = _mm_or_si128(VssOutR2a, VssTmpa);                                           \
	VssOutL2b = _mm_or_si128(VssOutL2b, VssTmpb);

#define VEC_MULTIPLY_ROUND(VssA, VssB, VssC, VssTmp1, VssTmp2)                              \
	                                                                                        \
	VssTmp1 = _mm_mulhi_epi16(VssA, VssB);                                                  \
	VssTmp2 = _mm_mullo_epi16(VssA, VssB);                                                  \
	VssTmp1 = _mm_slli_epi16(VssTmp1, 1);                                                   \
	VssTmp2 = _mm_srli_epi16(VssTmp2, 14);                                                  \
	VssTmp2 = _mm_adds_epi16(VssTmp2, VssUnit);                                             \
	VssTmp2 = _mm_srli_epi16(VssTmp2, 1);                                                   \
	VssC    = _mm_adds_epi16(VssTmp1, VssTmp2);

extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterShort2D(short * in, short* out, float sigma, int dimx, int dimy);
extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterFastShort2D(short * in, short* out, float sigma, int dimx, int dimy);
extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterFloat2D(float * in, float* out, float sigma, int dimx, int dimy);
extern "C" __declspec(dllexport) int SWMkxIsoGaussFilterFloat2D_SSE(float * in, float* out, float sigma, int dimx, int dimy);
