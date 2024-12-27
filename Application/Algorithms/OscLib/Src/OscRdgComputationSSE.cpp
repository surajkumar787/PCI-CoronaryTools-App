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

//extern "C"
//{
    #include <OscDev.h>
    #include <BxGaussian.h>
//}

#include "AlgoFragments/Common/Platform.h"
#define BASICFILTERS_HIGHPRECISION_SHORT
#include "AlgoFragments/BasicFilters/BasicFiltersProcessing.hpp"

using namespace Philips::Medisys::PXVec;
using namespace Philips::Medisys::AlgoFragments::Impl;

template <typename TYPE> void OscFIRGaussianProcessing(TYPE const * input, TYPE * output, int width, int height, double sigma)
{
	FilterSepProcessingContext<Image2D<TYPE> > context;
    context.Input.BufferAtOrigin = (TYPE *)input;
    context.Input.LineStride = width;
    context.Output.BufferAtOrigin = output;
    context.Output.LineStride = width;
    context.Kernel = FilterSepKernel::GaussianKernel(sigma, 1.0, 7);
    
    Region2D roi = { 0, 0, width, height };
#ifdef OSC_USE_OPENMP_RDGSSE
    Region2D roiOMP = { 0,
        omp_get_thread_num() * height / omp_get_num_threads(),
        width,
        OSC_MIN(height, (1 + omp_get_thread_num()) * height / omp_get_num_threads()) };
    FilterSepProcessingForSize<7, true, TYPE>::ProcessROI(roiOMP, roi, context);
#else
    FilterSepProcessingForSize<7, true, TYPE>::ProcessROI(roi, roi, context);
#endif
}

extern "C" void OscFIRGaussianProcessingShort(short const * input, short * output, int width, int height, double sigma)
{
    return OscFIRGaussianProcessing<short>(input, output, width, height, sigma);
}

extern "C" void OscFIRGaussianProcessingFloat(float const * input, float * output, int width, int height, double sigma)
{
    return OscFIRGaussianProcessing<float>(input, output, width, height, sigma);
}

template <int K, typename VEC_FTYPE, typename VEC_TYPE>
FORCEINLINE void OscRdgCalcGaussHessian2SSE(VEC_FTYPE & lxx, VEC_FTYPE & lyy, VEC_FTYPE & lxy,
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
FORCEINLINE void OscRdgCalcDirVectorSSEPreload(VecArray<VEC_TYPE, 2*K+1> & gauLeft,
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
FORCEINLINE void OscRdgCalcDirVectorSSE(VecArray<VEC_TYPE, 2*K+1> & gauLeft,
                                        VecArray<VEC_TYPE, 2*K+1> & gauCenter,
                                        VecArray<VEC_TYPE, 2*K+1> & gauRight,
                                        float * directionLine,
                                        float * ridgeLine,
                                        short const * gaussianRightLine, int stride, int count)
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

    VEC_FTYPE lxx, lyy, lxy, l1, l2;

    // calculate Gaussian Hessian eigenvalues
    OscRdgCalcGaussHessian2SSE<K>(lxx, lyy, lxy, gauLeft, gauCenter, gauRight);

    VEC_FTYPE det = fast_sqrt(max(numeric_limits<float>::epsilon(), square(lxx - lyy) + square(lxy + lxy)));

    VEC_FTYPE la = VEC_FTYPE(0.5f) * (lxx + lyy - det);
    VEC_FTYPE lb = VEC_FTYPE(0.5f) * (lxx + lyy + det);

    VEC_FTYPE cond = abs(la) < abs(lb);

    l1 = select(cond, lb, la);
    l2 = select(cond, la, lb);

    VEC_FTYPE vx = (lyy - lxx); 
    VEC_FTYPE vy = - 2*lxy; 

    VEC_FTYPE const lambda = VEC_FTYPE(0.33f);

    store<false>(directionLine, VEC_FTYPE(0.5f) * arctan2SSE(vy, vx), count);
    store<false>(ridgeLine    , (l1 > VEC_FTYPE::zero()) & (l1 - l2 * lambda), count);

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
void OscRdgMonoScaleRidgeSSE(short * Ori, short * Gau, float *Rdg, float *Dir, int Iw, int Ih,
                             COscRoiCoord *RoiCoord, float KernelSig)
{
    //int dBo = (int)(OSC_RDG_BORDER_MIN_FACTOR*KernelSig) + 1;
    //int yMin1 = OSC_MAX(RoiCoord->YMin - dBo, OSC_RDG_BORDER_MIN_ABS);
    //int yMax1 = OSC_MIN(RoiCoord->YMax + dBo, Ih-1-OSC_RDG_BORDER_MIN_ABS);
    //int xMin1 = OSC_MAX(RoiCoord->XMin - dBo, OSC_RDG_BORDER_MIN_ABS);
    //int xMax1 = OSC_MIN(RoiCoord->XMax + dBo, Iw-1-OSC_RDG_BORDER_MIN_ABS);
    int yMin1 = OSC_MAX(RoiCoord->YMin, OSC_RDG_BORDER_MIN_ABS)+1;
    int yMax1 = OSC_MIN(RoiCoord->YMax, Ih-1-OSC_RDG_BORDER_MIN_ABS)-1;
    int xMin1 = OSC_MAX(RoiCoord->XMin, OSC_RDG_BORDER_MIN_ABS)+1;
    int xMax1 = OSC_MIN(RoiCoord->XMax, Iw-1-OSC_RDG_BORDER_MIN_ABS)-1;

#ifdef OSC_USE_OPENMP_RDGSSE
#pragma omp parallel
    {
        SetThreadAffinityMask(GetCurrentThread(), 1<<omp_get_thread_num());
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif /* OSC_USE_OPENMP_RDGSSE */

        if (Ori != NULL)
        {
            // calculation of the Gaussian image used to determine the eigenvalues
#ifdef OSC_USE_SSE_RDG_FIR
            OscFIRGaussianProcessingShort(Ori, Gau, Iw, Ih, KernelSig);
#else
            if (BxIsoGaussFilterShort2DROI(Ori, Gau, KernelSig, Iw, Ih, xMin1-1, xMax1+1, yMin1-1, yMax1+1) != 0){
                printf("OscRdgMonoScaleRidgeSSE: allocation error in BxIsoGaussFilterShort2DROI\n"); return ;
            }			
#endif
        }

        typedef PXVEC_TYPE(short) VEC_TYPE;
        typedef PXVEC_TYPE_FORSIZE(float, VEC_TYPE::SCALAR_COUNT) VEC_FTYPE;

        int xMilestone2 = xMax1 - VEC_TYPE::SCALAR_COUNT + 1;

#ifdef OSC_USE_OPENMP_RDGSSE
#pragma omp for schedule(static, (64*1024)/Iw)
#endif /* OSC_USE_OPENMP_RDGSSE */
        for(int y = 0; y < Ih; y++)
        {
            if (y < yMin1 || y > yMax1)
            {
                OSC_MEMZERO(Dir + y * Iw, Iw);
                OSC_MEMZERO(Rdg + y * Iw, Iw);
            }
            else
            {
				enum { K = 1 };
                short const * gaussianLine = Gau + y * Iw;

                VecArray<VEC_TYPE, 2*K+1> gauLeft, gauCenter, gauRight;

                int x;
                for (x = 0; x < xMin1; x++)
                {
                    Dir[x + y * Iw] = 0;
                    Rdg[x + y * Iw] = 0;
                }

                OscRdgCalcDirVectorSSEPreload<K>(gauLeft, gauCenter, gaussianLine + x, Iw);

                for(x = xMin1; x <= xMilestone2; x += VEC_TYPE::SCALAR_COUNT)
                {
                    OscRdgCalcDirVectorSSE<K>(gauLeft, gauCenter, gauRight, Dir + x + y * Iw, Rdg + x + y * Iw, gaussianLine + x + VEC_TYPE::SCALAR_COUNT, Iw, VEC_TYPE::SCALAR_COUNT);
                }

                if (xMax1 + 1 - x > 0)
                {
                    int count = xMax1 + 1 - x;
                    OscRdgCalcDirVectorSSE<K>(gauLeft, gauCenter, gauRight, Dir + x + y * Iw, Rdg + x + y * Iw, gaussianLine + x + VEC_TYPE::SCALAR_COUNT, Iw, count);
                }

                for(x = xMax1 + 1; x < Iw; x++)
                {
                    Dir[x + y * Iw] = 0;
                    Rdg[x + y * Iw] = 0;
                }
            }
        }
#ifdef OSC_USE_OPENMP_RDGSSE
    }
#endif
}
