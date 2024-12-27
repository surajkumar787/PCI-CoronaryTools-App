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

#define _USE_MATH_DEFINES

#include "SWLibDev.h"
#include <SWBxGaussian.h>
#include "omp.h"

#include "AlgoFragments/Common/Platform.h"
#define BASICFILTERS_HIGHPRECISION_SHORT
#include "AlgoFragments/BasicFilters/BasicFiltersProcessing.hpp"

using namespace Philips::Medisys::PXVec;
using namespace Philips::Medisys::AlgoFragments;//::Impl;
using namespace Philips::Medisys::AlgoFragments::BasicFilters;//::Impl;

#define SW_RDG_BORDER_MIN_ABS   2

template <typename TYPE> void SWFIRGaussianProcessing(TYPE const * input, TYPE * output, int width, int height, double sigma)
{
	FilterSepProcessingContext<Image2D<TYPE> > context;
    context.Input.BufferAtOrigin = (TYPE *)input;
    context.Input.LineStride = width;
    context.Output.BufferAtOrigin = output;
    context.Output.LineStride = width;
    context.Kernel = FilterSepKernel::GaussianKernel(sigma, 1.0, 7);
    
    Region2D roi = { 0, 0, width, height };
    FilterSepProcessingForSize<1, 7, true, TYPE>::ProcessROI(roi, roi, context);
}

extern "C" void SWFIRGaussianProcessingShort(short const * input, short * output, int width, int height, double sigma)
{
    return SWFIRGaussianProcessing<short>(input, output, width, height, sigma);
}

extern "C" void SWFIRGaussianProcessingFloat(float const * input, float * output, int width, int height, double sigma)
{
    return SWFIRGaussianProcessing<float>(input, output, width, height, sigma);
}

template <int K, typename VEC_FTYPE, typename VEC_TYPE>
FORCEINLINE void SWRdgCalcGaussHessian2SSE(VEC_FTYPE & lxx, VEC_FTYPE & lyy, VEC_FTYPE & lxy,
                                            VecArray<VEC_TYPE, 2*K+1> const & gauLeft,
                                            VecArray<VEC_TYPE, 2*K+1> const & gauCenter,
                                            VecArray<VEC_TYPE, 2*K+1> const & gauRight)
{
    enum { UP = 0, UPDIAG = K-1, DOWNDIAG = K+1, CENTER = K, DOWN = 2*K };

    VEC_TYPE center2 = sat_add(gauCenter[CENTER], gauCenter[CENTER]);
    VEC_TYPE left   = combine<-K>(gauLeft[CENTER], gauCenter[CENTER]);
    VEC_TYPE right  = combine<K>(gauCenter[CENTER], gauRight[CENTER]);
    VEC_TYPE top    = gauCenter[UP];
    VEC_TYPE bottom = gauCenter[DOWN];
    VEC_TYPE topLeft  = combine<-1>(gauLeft[UPDIAG], gauCenter[UPDIAG]);
    VEC_TYPE topRight = combine<1>(gauCenter[UPDIAG], gauRight[UPDIAG]);
    VEC_TYPE bottomLeft  = combine<-1>(gauLeft[DOWNDIAG], gauCenter[DOWNDIAG]);
    VEC_TYPE bottomRight = combine<1>(gauCenter[DOWNDIAG], gauRight[DOWNDIAG]);

    lxx = convert<VEC_FTYPE>(sat_sub(sat_add(left, right), center2));
    lyy = convert<VEC_FTYPE>(sat_sub(sat_add(top, bottom), center2));
    lxy = convert<VEC_FTYPE>(sat_sub(sat_add(bottomRight, topLeft), sat_add(topRight, bottomLeft)));

    if (K == 2)
    {
        lxx *= VEC_FTYPE(0.25f);
        lyy *= VEC_FTYPE(0.25f);
    }

    lxy *= VEC_FTYPE(0.25f);
}

template <typename VEC_FTYPE>
FORCEINLINE VEC_FTYPE arctan2SSE(VEC_FTYPE const & y, VEC_FTYPE const & x)
{
    VEC_FTYPE const ONEQTR_PI = VEC_FTYPE(VEC_FTYPE::SCALAR_TYPE(M_PI / 4.0));
    VEC_FTYPE const THRQTR_PI = VEC_FTYPE(VEC_FTYPE::SCALAR_TYPE(3.0 * M_PI / 4.0));

    VEC_FTYPE abs_y = max(abs(y), VEC_FTYPE(::std::numeric_limits<typename VEC_FTYPE::SCALAR_TYPE>::min()));

    VEC_FTYPE cond1 = x < VEC_FTYPE::zero();

    VEC_FTYPE x_p_abs_y = x + abs_y;

    VEC_FTYPE r = select(cond1, x_p_abs_y, x - abs_y) * rcp(select(cond1, abs_y - x, x_p_abs_y));
    VEC_FTYPE angle = select(cond1, THRQTR_PI, ONEQTR_PI) + (VEC_FTYPE(0.1963f) * r * r - VEC_FTYPE(0.9817f)) * r;

    return select(y < VEC_FTYPE::zero(), -angle, angle);
}

template <int K, typename VEC_TYPE>
FORCEINLINE void SWRdgCalcDirVectorSSEPreload(VecArray<VEC_TYPE, 2*K+1> & gauLeft,
                                               VecArray<VEC_TYPE, 2*K+1> & gauCenter,
                                               typename VEC_TYPE::SCALAR_TYPE const * gaussianLine, int stride)
{
    for (int k = -K; k <= K; k++)
    {
        if (K == 2)
        {
            gauLeft[K+k] = insert<-2>(VEC_TYPE()  , gaussianLine[k * stride - 2]);
            gauLeft[K+k] = insert<-1>(gauLeft[K+k], gaussianLine[k * stride - 1]);
        }
        else
        {
            gauLeft[K+k] = insert<-1>(VEC_TYPE()  , gaussianLine[k * stride - 1]);
        }

        gauCenter[K+k] = loadu<VEC_TYPE>(gaussianLine + k * stride);
    }
}

template <int K, typename VEC_TYPE>
FORCEINLINE void SWRdgCalcDirVectorSSE(VecArray<VEC_TYPE, 2*K+1> & gauLeft,
                                        VecArray<VEC_TYPE, 2*K+1> & gauCenter,
                                        VecArray<VEC_TYPE, 2*K+1> & gauRight,
                                        float * ridgeLine,
                                        typename VEC_TYPE::SCALAR_TYPE const * gaussianRightLine, int stride, int count)
{
    typedef PXVEC_TYPE_FORSIZE(float, VEC_TYPE::SCALAR_COUNT) VEC_FTYPE;

    if (K == 2)
    {
        gauRight[K-2] = loadu<VEC_TYPE>(gaussianRightLine - 2 * stride);
        gauRight[K+2] = loadu<VEC_TYPE>(gaussianRightLine + 2 * stride);
    }

    gauRight[K-1] = loadu<VEC_TYPE>(gaussianRightLine - stride);
    gauRight[K  ] = loadu<VEC_TYPE>(gaussianRightLine         );
    gauRight[K+1] = loadu<VEC_TYPE>(gaussianRightLine + stride);

    VEC_FTYPE lxx, lyy, lxy, l1;

    // calculate Gaussian Hessian eigenvalues
    SWRdgCalcGaussHessian2SSE<K>(lxx, lyy, lxy, gauLeft, gauCenter, gauRight);

    //VEC_FTYPE det = fast_sqrt(max(numeric_limits<float>::epsilon(), square(lxx - lyy) + square(lxy + lxy)));
    //VEC_FTYPE det = fast_sqrt(max(numeric_limits<float>::epsilon(), square(lxx - lyy) + VEC_FTYPE(4.f) * mulfix(lxy,lxy)));
    VEC_FTYPE det = sqrt(max(::std::numeric_limits<float>::epsilon(), square(lxx - lyy) + VEC_FTYPE(4.f) * mulfix(lxy,lxy)));
    //VEC_FTYPE det = sqrt(max(numeric_limits<float>::epsilon(), square(lxx - lyy) + VEC_FTYPE(4.f) * lxy*lxy)); //mulfix(lxy,lxy)));

    l1 = VEC_FTYPE(0.5f) * (lxx + lyy + det);

    //VEC_FTYPE const lambda = VEC_FTYPE(0.33f);

    store<false>(ridgeLine    , (l1 > VEC_FTYPE::zero()) & (l1), count);
    //store<false>(ridgeLine    , (l1 > VEC_FTYPE::zero()), count);

    //{
    //enum { CENTER = K };
    //VEC_TYPE center2 = sat_add(gauCenter[CENTER], gauCenter[CENTER]);
    //VEC_TYPE left   = combine<-K>(gauLeft[CENTER], gauCenter[CENTER]);
    //VEC_TYPE right  = combine<K>(gauCenter[CENTER], gauRight[CENTER]);
    //VEC_FTYPE lxx = convert<VEC_FTYPE>(center2);//

    //store<false>(ridgeLine, lxx);
    //}
    if (count == VEC_FTYPE::SCALAR_COUNT)
    {
        if (K == 2)
        {
            gauLeft[K-2] = gauCenter[K-2];
            gauLeft[K+2] = gauCenter[K+2];

            gauCenter[K-2] = gauRight[K-2];
            gauCenter[K+2] = gauRight[K+2];
        }
      
        gauLeft[K-1] = gauCenter[K-1];
        gauLeft[K  ] = gauCenter[K  ];
        gauLeft[K+1] = gauCenter[K+1];

        gauCenter[K-1] = gauRight[K-1];
        gauCenter[K  ] = gauRight[K  ];
        gauCenter[K+1] = gauRight[K+1];
    }
}

/*************************************************************************************/
// Differential ridge (based on determination of kernels from Hessian eigenvalues)
/*************************************************************************************/
void SWHessianNoDirSSEShort(short * Ori, short * Gau, float *Rdg, int Iw, int Ih, CSWRoiCoord *RoiCoord, float KernelSig)
{
    int yMin1 = SW_LIB_MAX(RoiCoord->YMin, SW_RDG_BORDER_MIN_ABS)+1;
    int yMax1 = SW_LIB_MIN(RoiCoord->YMax, Ih-1-SW_RDG_BORDER_MIN_ABS)-1;
    int xMin1 = SW_LIB_MAX(RoiCoord->XMin, SW_RDG_BORDER_MIN_ABS)+1;
    int xMax1 = SW_LIB_MIN(RoiCoord->XMax, Iw-1-SW_RDG_BORDER_MIN_ABS)-1;

    //SWBxIsoGaussFilterShort2D(Ori, Gau, KernelSig, Iw, Ih);
    SWBxIsoGaussFilterShort2DROI(Ori, Gau, KernelSig, Iw, Ih, RoiCoord->XMin, RoiCoord->XMax, RoiCoord->YMin, RoiCoord->YMax);

    typedef PXVEC_TYPE(short) VEC_TYPE;
    typedef PXVEC_TYPE_FORSIZE(float, VEC_TYPE::SCALAR_COUNT) VEC_FTYPE;

    int xMilestone2 = xMax1 - VEC_TYPE::SCALAR_COUNT + 1;

    for(int y = 0; y < Ih; y++)
    {
        if (y < yMin1 || y > yMax1)
        {
            SW_LIB_MEMZERO(Rdg + y * Iw, Iw);
        }
        else
        {
			enum { K = 1 };
            short const * gaussianLine = Gau + y * Iw;

            VecArray<VEC_TYPE, 2*K+1> gauLeft, gauCenter, gauRight;

            int x;
            for (x = 0; x < xMin1; x++)
            {
                Rdg[x + y * Iw] = 0;
            }

            SWRdgCalcDirVectorSSEPreload<K>(gauLeft, gauCenter, gaussianLine + x, Iw);

            for(x = xMin1; x <= xMilestone2; x += VEC_TYPE::SCALAR_COUNT)
            {
                SWRdgCalcDirVectorSSE<K>(gauLeft, gauCenter, gauRight, Rdg + x + y * Iw, gaussianLine + x + VEC_TYPE::SCALAR_COUNT, Iw, VEC_TYPE::SCALAR_COUNT);
            }

            if (xMax1 + 1 - x > 0)
            {
                int count = xMax1 + 1 - x;
                SWRdgCalcDirVectorSSE<K>(gauLeft, gauCenter, gauRight, Rdg + x + y * Iw, gaussianLine + x + VEC_TYPE::SCALAR_COUNT, Iw, count);
            }

            for(x = xMax1 + 1; x < Iw; x++)
            {
                Rdg[x + y * Iw] = 0;
            }
        }
    }
}

void SWHessianNoDirSSEFloat(float * Ori, float * Gau, float *Rdg, int Iw, int Ih, CSWRoiCoord *RoiCoord, float KernelSig)
{
    int yMin1 = SW_LIB_MAX(RoiCoord->YMin, SW_RDG_BORDER_MIN_ABS)+1;
    int yMax1 = SW_LIB_MIN(RoiCoord->YMax, Ih-1-SW_RDG_BORDER_MIN_ABS)-1;
    int xMin1 = SW_LIB_MAX(RoiCoord->XMin, SW_RDG_BORDER_MIN_ABS)+1;
    int xMax1 = SW_LIB_MIN(RoiCoord->XMax, Iw-1-SW_RDG_BORDER_MIN_ABS)-1;

    //SWMkxIsoGaussFilterFloat2D(Ori, Gau, KernelSig, Iw, Ih);
    //SWBxIsoGaussFilterFloat2D(Ori, Gau, KernelSig, Iw, Ih);
    SWBxIsoGaussFilterFloat2DROI(Ori, Gau, KernelSig, Iw, Ih, RoiCoord->XMin, RoiCoord->XMax, RoiCoord->YMin, RoiCoord->YMax);

    //typedef PXVEC_TYPE(short) VEC_TYPE;
    typedef PXVEC_TYPE(float) VEC_TYPE;
    typedef PXVEC_TYPE_FORSIZE(float, VEC_TYPE::SCALAR_COUNT) VEC_FTYPE;

    int xMilestone2 = xMax1 - VEC_TYPE::SCALAR_COUNT + 1;

    for(int y = 0; y < Ih; y++)
    {
        if (y < yMin1 || y > yMax1)
        {
            SW_LIB_MEMZERO(Rdg + y * Iw, Iw);
        }
        else
        {
			enum { K = 1 };
            //short const * gaussianLine = Gau + y * Iw;
            float const * gaussianLine = Gau + y * Iw;

            VecArray<VEC_TYPE, 2*K+1> gauLeft, gauCenter, gauRight;

            int x;
            for (x = 0; x < xMin1; x++)
            {
                Rdg[x + y * Iw] = 0;
            }

            SWRdgCalcDirVectorSSEPreload<K>(gauLeft, gauCenter, gaussianLine + x, Iw);

            for(x = xMin1; x <= xMilestone2; x += VEC_TYPE::SCALAR_COUNT)
            {
                SWRdgCalcDirVectorSSE<K>(gauLeft, gauCenter, gauRight, Rdg + x + y * Iw, gaussianLine + x + VEC_TYPE::SCALAR_COUNT, Iw, VEC_TYPE::SCALAR_COUNT);
            }

            if (xMax1 + 1 - x > 0)
            {
                int count = xMax1 + 1 - x;
                SWRdgCalcDirVectorSSE<K>(gauLeft, gauCenter, gauRight, Rdg + x + y * Iw, gaussianLine + x + VEC_TYPE::SCALAR_COUNT, Iw, count);
            }

            for(x = xMax1 + 1; x < Iw; x++)
            {
                Rdg[x + y * Iw] = 0;
            }
        }
    }
}

// **************************************************************************************************
int MultiscaleHessianNoDirSSEShort(short* Ori, int Iw, int Ih, CSWRoiCoord *RoiCoord, const float* TabSig, int nbSigmas, float Gamma, float* Wid, float* Ctr, float *wkBuf1, float *wkBuf2)
{
    float *Lamb1 = wkBuf1;
    short *Gau = (short*)wkBuf2;
#ifdef SW_TIMING_DETAILS
    double durationRidge = 0, durationReste = 0, t0, t1, t2;
#endif
    
    memset(Lamb1, 0, Iw*Ih*sizeof(Lamb1[0]));
    memset(Ctr, 0, Iw*Ih*sizeof(Ctr[0]));
    memset(Wid, 0, Iw*Ih*sizeof(Wid[0]));

    for (int kSig = 0; kSig < nbSigmas; kSig++){
        float Sig = TabSig[kSig];
        
#ifdef SW_TIMING_DETAILS
        t0 = omp_get_wtime();
#endif
        SWHessianNoDirSSEShort(Ori, Gau, Lamb1, Iw, Ih, RoiCoord, Sig);
#ifdef SW_TIMING_DETAILS
        t1 = omp_get_wtime();
        durationRidge += 1000 * (t1-t0);
#endif

        float Sig_Gamma = pow(Sig, Gamma);
        int xMinHere = RoiCoord->XMin+1, xMaxHere = RoiCoord->XMax - 1;
        int yMinHere = RoiCoord->YMin+1, yMaxHere = RoiCoord->YMax - 1;
        int width = xMaxHere-xMinHere+1, height = yMaxHere-yMinHere+1, stride = Iw-width;

        float *ptrLambda = &Lamb1[yMinHere*Iw+xMinHere];
        float *ptrCtrl = &Ctr[yMinHere*Iw+xMinHere];
        float *ptrWidth = &Wid[yMinHere*Iw+xMinHere];
        for (int y=0; y<height; y++){
            for (int x=0; x<width; x++){
                (*ptrLambda) *= Sig_Gamma;
                if ((*ptrLambda) > (*ptrCtrl)){
                    *ptrCtrl = *ptrLambda;
                    *ptrWidth = Sig;
                }
                ptrLambda++; ptrCtrl++; ptrWidth ++;
            }
            ptrLambda += stride; ptrCtrl += stride; ptrWidth += stride;
        }
#ifdef SW_TIMING_DETAILS
        t2 = omp_get_wtime();
        durationReste += 1000 * (t2-t1);
#endif
    }
    
#ifdef SW_TIMING_DETAILS
    printf("Duration ridges %.1f duration reste %.1f\n", durationRidge, durationReste);
#endif

    return 0;
}

int MultiscaleHessianNoDirSSEFloat(float* Ori, int Iw, int Ih, CSWRoiCoord *RoiCoord, const float* TabSig, int nbSigmas, float Gamma, float* Wid, float* Ctr, float *wkBuf1, float *wkBuf2)
{
    float *Lamb1 = wkBuf1;
    float *Gau = wkBuf2;
#ifdef SW_TIMING_DETAILS
    double durationRidge = 0, durationReste = 0, t0, t1, t2;
#endif
    
    memset(Lamb1, 0, Iw*Ih*sizeof(Lamb1[0]));
    memset(Ctr, 0, Iw*Ih*sizeof(Ctr[0]));
    memset(Wid, 0, Iw*Ih*sizeof(Wid[0]));

    for (int kSig = 0; kSig < nbSigmas; kSig++){
        float Sig = TabSig[kSig];
        
#ifdef SW_TIMING_DETAILS
        t0 = omp_get_wtime();
#endif
        SWHessianNoDirSSEFloat(Ori, Gau, Lamb1, Iw, Ih, RoiCoord, Sig);
#ifdef SW_TIMING_DETAILS
        t1 = omp_get_wtime();
        durationRidge += 1000 * (t1-t0);
#endif

        float Sig_Gamma = pow(Sig, Gamma);
        int xMinHere = RoiCoord->XMin+1, xMaxHere = RoiCoord->XMax - 1;
        int yMinHere = RoiCoord->YMin+1, yMaxHere = RoiCoord->YMax - 1;
        int width = xMaxHere-xMinHere+1, height = yMaxHere-yMinHere+1, stride = Iw-width;

        float *ptrLambda = &Lamb1[yMinHere*Iw+xMinHere];
        float *ptrCtrl = &Ctr[yMinHere*Iw+xMinHere];
        float *ptrWidth = &Wid[yMinHere*Iw+xMinHere];
        for (int y=0; y<height; y++){
            for (int x=0; x<width; x++){
                (*ptrLambda) *= Sig_Gamma;
                if ((*ptrLambda) > (*ptrCtrl)){
                    *ptrCtrl = *ptrLambda;
                    *ptrWidth = Sig;
                }
                ptrLambda++; ptrCtrl++; ptrWidth ++;
            }
            ptrLambda += stride; ptrCtrl += stride; ptrWidth += stride;
        }
#ifdef SW_TIMING_DETAILS
        t2 = omp_get_wtime();
        durationReste += 1000 * (t2-t1);
#endif
    }
    
#ifdef SW_TIMING_DETAILS
    printf("Duration ridges %.1f duration reste %.1f\n", durationRidge, durationReste);
#endif

    return 0;
}
