
#pragma once

#include <BasicFiltersCommon.hpp>

namespace // Private namespace to this compiland
{
    namespace AlgoFragmentsImpl
    {

template <typename IMAGE_CLASS>
struct BinomialProcessingContext
{
    IMAGE_CLASS Input;
    IMAGE_CLASS Output;
};

template <int HALFKERSIZE, typename VectorType>
class VecCombiner
{
    VecCombiner & operator=(VecCombiner const &);
public:
    VectorType const & left;
    VectorType const & center;
    VectorType const & right;

    FORCEINLINE VecCombiner(VectorType const & _left, VectorType const & _center, VectorType const & _right)
        : left(_left), center(_center), right(_right)
    {
    }

    template <int I>
    FORCEINLINE VectorType at() const
    {

//            return *(VectorType *)(((VectorType::SCALAR_TYPE *)&left) + 8 + I - HALFKERSIZE);
        return combine<I-HALFKERSIZE>(left, center, right);
        //return loadu<VectorType>((((VectorType::SCALAR_TYPE *)&left) + VectorType::SCALAR_COUNT + I - HALFKERSIZE));
    }
};

template <bool ALIGNED, typename VECTOR_TYPE, typename LINECONTEXT>
class LinesLoader
{
    LinesLoader & operator=(LinesLoader const &);
public:
    LINECONTEXT const & lineContext;
    int                 currentColumn;

    FORCEINLINE LinesLoader(int _currentColumn, LINECONTEXT const & _lineContext)
        : lineContext(_lineContext), currentColumn(_currentColumn)
    {
    }

    template <int I>
    FORCEINLINE VECTOR_TYPE at() const
    {
        return load<ALIGNED, VECTOR_TYPE>(&lineContext.inputLines[I][currentColumn]);
    }
};

template <int PASS, int N>
struct BinomialFunctor
{
    enum { HALFKERSIZE = ((N-1)/2) };

    template <int I, typename T_IN>
    FORCEINLINE void operator()(Is32vec8 & result, T_IN const & vecData)
    {
        if (I == 0)
            result = mul_add(unpack(vecData.at<HALFKERSIZE>(), vecData.at<HALFKERSIZE>()), Combinations<N-1,HALFKERSIZE-I>::Value / 2);
        else
            result += mul_add(unpack(vecData.at<HALFKERSIZE-I>(), vecData.at<HALFKERSIZE+I>()), Combinations<N-1,HALFKERSIZE-I>::Value);
    }

    template <typename T_IN>
    FORCEINLINE Is16vec8 operator()(T_IN const & vecData)
    {
        int const BIT_SHIFT = N - 2 * (PASS == 0);
        int const CARRY = (1<<(BIT_SHIFT-1));

        Is32vec8 result;
        unroller<0, 1+HALFKERSIZE>::go(*this, result, vecData);

        return sat_pack((result + Is32vec8(CARRY)) >> BIT_SHIFT);
    }
};

template <int N>
struct BinomialProcessing
{
    enum { HALFKERSIZE = (N-1)/2 };

    struct LineContext
    {
        int16_t const * inputLines[N];
        int16_t       * outputLine;

        template <typename T1, typename T2>
        FORCEINLINE void Fill(int currentLine, int top, int bottom, T1 const & Input, T2 const & Output)
        {
            outputLine = Output.PointerAt(0, currentLine);

            inputLines[HALFKERSIZE] = Input.PointerAt(0, currentLine);
            for (int I = 1; I != 1+HALFKERSIZE; I++)
            {
                inputLines[HALFKERSIZE-I] = Input.PointerAt(0, MirrorIndex(currentLine - I, top, bottom));
                inputLines[HALFKERSIZE+I] = Input.PointerAt(0, MirrorIndex(currentLine + I, top, bottom));
            }
        }
    };

    template <bool ALIGNED>
    static FORCEINLINE Is16vec8 ProcessVertical(int currentColumn, LineContext const & lineContext)
    {
        return BinomialFunctor<0, N>()(LinesLoader<ALIGNED, Is16vec8, LineContext>(currentColumn, lineContext));
    }

    struct LineWorkspace
    {
        // Not Is16vec8 because for unknown reason the compiler would emit unaligned access
        VecArray<Is16vec8, 2> m_vecBuffer;
    };

    template <bool ALIGNED>
    static FORCEINLINE void BeginProcess(int currentColumn, int leftAvailable,
        LineWorkspace & lineWorkspace, LineContext const & lineContext)
    {
        VecArray<Is16vec8, 2> & vecBuffer = lineWorkspace.m_vecBuffer;

        if (leftAvailable > 0)
            vecBuffer[0] = ProcessVertical<ALIGNED>(currentColumn-8, lineContext);
        vecBuffer[1] = ProcessVertical<ALIGNED>(currentColumn, lineContext);

        ApplyLeftBorder<HALFKERSIZE>(leftAvailable, vecBuffer[0], vecBuffer[1]);
    }

    template <bool ALIGNED, bool INPUT_ALIGNED>
    static FORCEINLINE void ProcessVector(int currentColumn, int count, int rightAvailable,
        LineWorkspace & lineWorkspace, LineContext const & lineContext)
    {
        VecArray<Is16vec8, 2> & vecBuffer = lineWorkspace.m_vecBuffer;

        Is16vec8 vecRight;
        if (count > 8 - HALFKERSIZE) vecRight = ProcessVertical<INPUT_ALIGNED>(currentColumn + 8, lineContext);
        ApplyRightBorder<HALFKERSIZE>(count, rightAvailable, vecBuffer[0], vecBuffer[1], vecRight);

        Is16vec8 output = BinomialFunctor<1, N>()(VecCombiner<HALFKERSIZE, Is16vec8>(vecBuffer[0], vecBuffer[1], vecRight));
        store<ALIGNED>(&lineContext.outputLine[currentColumn], output, count);

        vecBuffer[0] = vecBuffer[1];
        vecBuffer[1] = vecRight;
    }

    template <bool ALIGNED, bool INPUT_ALIGNED>
    static FORCEINLINE void ProcessLine(int left, int right,
                                        int leftAvailable, int rightAvailable,
                                        LineContext const & lineContext)
    {
        int x = left;
        int x_limit = right - 16;

        LineWorkspace lineWorkspace;

        BeginProcess<INPUT_ALIGNED>(x, leftAvailable, lineWorkspace, lineContext);

        for (; x < x_limit; x += 8)
        {
            ProcessVector<ALIGNED, INPUT_ALIGNED>(x, 8, HALFKERSIZE, lineWorkspace, lineContext);
        }

        if (x < right - 8)
        {
            // In some case we can have a border effect in the last full vector
            int rightAvailable2 = rightAvailable + right - 8 - x;
            ProcessVector<ALIGNED, INPUT_ALIGNED>(x, 8, rightAvailable2, lineWorkspace, lineContext);
            x += 8;
        }
        if (x < right)
        {
            ProcessVector<ALIGNED, INPUT_ALIGNED>(x, right - x, rightAvailable, lineWorkspace, lineContext);
        }
    }

    template <bool ALIGNED, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        for (int y = roiOutput.top; y < roiOutput.bottom; y++)
        {
            LineContext CurrentLineContext;
            CurrentLineContext.Fill(y, roiInput.top, roiInput.bottom, context.Input, context.Output);

            ProcessLine<ALIGNED, ALIGNED>(roiOutput.left, roiOutput.right,
                roiOutput.left - roiInput.left, roiInput.right - roiOutput.right, CurrentLineContext);
        }
    }

    template <typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        if (context.Input .IsAligned(roiOutput.left)
         && context.Output.IsAligned(roiOutput.left))
            return ProcessROI<true>(roiOutput, roiInput, context);
        else
            return ProcessROI<false>(roiOutput, roiInput, context);
    }
};

class FilterSepKernel
{
    static double sinc(double a)
    {
        double const Pi = 3.1415926539;
        return fabs(a) < numeric_limits<double>::min() ? 1 : sin(Pi * a) / (Pi * a);
    }
public:
    enum { MaxSize = 15, MaxKerDim = 3 };

    typedef FilterSepKernel SelfType;

    double Values[MaxKerDim][MaxSize];
    bool Symmetric;
    int KernelDim;
    int  HalfKernelSize;

    FilterSepKernel(int kernelDim = 1)
        : KernelDim(__max(1, kernelDim))
    {
    }

    ~FilterSepKernel()
    {
    }

    static SelfType LanczosKernel(double a, double desiredNorm, double s)
    {
        a = fabs(a);
        s = fabs(s);

        SelfType result;

        result.HalfKernelSize = (int)std::ceil(s*a)-1;
        result.Symmetric = true;

        double norm = 1;
        for (int k = 1; k <= result.HalfKernelSize; k++)
        {
            double x = (double)k;
            norm += 2 * sinc(x/s) * sinc(x/(s*a));
        }

        result.Values[0][FilterSepKernel::MaxSize/2] = desiredNorm/norm;
        for (int k = 1; k <= result.HalfKernelSize; k++)
        {
            double x = (double)k;
            double b = sinc(x/s);
            double c = sinc(x/(s*a));
            result.Values[0][FilterSepKernel::MaxSize/2+k] = desiredNorm * b * c / norm;
        }

        return result;
    }

    static SelfType GaussianKernel(double sigma, double desiredNorm, int halfKerSize)
    {
        SelfType result;

        sigma = fabs(sigma);
        result.HalfKernelSize = __min(FilterSepKernel::MaxSize/2, halfKerSize);
        result.Symmetric = true;

        if (sigma == 0)
        {
            for (int k = 0; k <= result.HalfKernelSize; k++)
                result.Values[0][FilterSepKernel::MaxSize/2+k] = desiredNorm * (k == 0);
        }
        else
        {
            double norm = 0;
            
            for (int k = -result.HalfKernelSize; k <= result.HalfKernelSize; k++)
            {
                norm += exp(-0.5*k*k/(sigma*sigma));
            }

            for (int k = 0; k <= result.HalfKernelSize; k++)
            {
                result.Values[0][FilterSepKernel::MaxSize/2+k] = desiredNorm * exp(-0.5*k*k/(sigma*sigma)) / norm;
            }
        }

        return result;
    }

    static SelfType GaussianKernel(double sigma)
    {
        return GaussianKernel(sigma, 1.0, round(fabs(3*sigma)));
    }

    static SelfType GaussianKernel(double sigma, int halfKerSize)
    {
        return GaussianKernel(sigma, 1.0, halfKerSize);
    }

    static SelfType GaussianKernel(double sigma, double desiredNorm)
    {
        return GaussianKernel(sigma, desiredNorm, round(fabs(3*sigma)));
    }

    static SelfType AnisoGaussianKernel(double const * sigmas, double desiredNorm, int kernelDim)
    {
        SelfType result(kernelDim);

        int halfKerSize = 0;
        
        for (int d = 0; d < kernelDim; d++)
        {
            halfKerSize = __max(halfKerSize, round(3 * fabs(sigmas[d])));
        }

        result.HalfKernelSize = __min(FilterSepKernel::MaxSize/2, halfKerSize);
        result.Symmetric = true;

        for (int d = 0; d < kernelDim; d++)
        {
            double sigma_d = sigmas[d];

            PX_ZeroArray(&result.Values[d][0], MaxSize);

            if (sigma_d == 0)
            {
                result.Values[d][FilterSepKernel::MaxSize/2] = desiredNorm;
            }
            else
            {
                double norm = 0;
            
                for (int k = -result.HalfKernelSize; k <= result.HalfKernelSize; k++)
                {
                    norm += exp(-0.5*k*k/(sigma_d*sigma_d));
                }

                for (int k = 0; k <= result.HalfKernelSize; k++)
                {
                    result.Values[d][FilterSepKernel::MaxSize/2+k] = desiredNorm * exp(-0.5*k*k/(sigma_d*sigma_d)) / norm;
                }
            }
        }

        return result;
    }
    
    static SelfType AnisoGaussianKernel(double sigma, double desiredNorm, double const * voxelSizes, int kernelDim)
    {
        double * sigmas = new double[kernelDim];

        for (int d = 0; d < kernelDim; d++)
        {
            sigmas[d] = sigma / voxelSizes[d];
        }

        SelfType res = AnisoGaussianKernel(sigmas, desiredNorm, kernelDim);
        delete [] sigmas;
        return res;
    }

    static SelfType SymletKernel(double desiredNorm)
    {
        SelfType result;
        result.HalfKernelSize = 4;
        result.Symmetric = false;
        int offset = (FilterSepKernel::MaxSize - 2*result.HalfKernelSize + 1) / 2;
        result.Values[0][offset-1] = 0;
        result.Values[0][offset+0] = desiredNorm * -0.032223100604071;
        result.Values[0][offset+1] = desiredNorm * -0.012603967262261;
        result.Values[0][offset+2] = desiredNorm * 0.099219543576935;
        result.Values[0][offset+3] = desiredNorm * 0.297857795605543;
        result.Values[0][offset+4] = desiredNorm * -0.803738751805216;
        result.Values[0][offset+5] = desiredNorm * 0.497618667632459;
        result.Values[0][offset+6] = desiredNorm * 0.029635527645954;
        result.Values[0][offset+7] = desiredNorm * -0.075765714789341;
        return result;
    }

    static SelfType GenericKernel(double const * kernel, int kernelSize, double desiredNorm, int kernelDim = 1)
    {
        SelfType result(kernelDim);

        for (int d = 0; d < kernelDim; d++)
        {
            double norm = 0;
            for (int k = 0; k < kernelSize; k++)
            {
                norm += kernel[d * kernelSize + k];
            }

            int offset = (FilterSepKernel::MaxSize - kernelSize + 1) / 2;
            for (int k = 0; k < FilterSepKernel::MaxSize; k++)
            {
                if (k >= offset && k < (offset + kernelSize))
                {
                    result.Values[d][k] = desiredNorm / norm * kernel[d * kernelSize + (k - offset)];
                }
                else
                {
                    result.Values[d][k] = 0;
                }
            }
        }

        result.KernelDim = kernelDim;
        result.HalfKernelSize = 0;
        result.Symmetric = true;

        int const MaxHalfKerSize = FilterSepKernel::MaxSize/2;

        for (int k = 1; k < MaxHalfKerSize; k++)
        {
            for (int d = 0; d < kernelDim; d++)
            {
                if (result.Values[d][MaxHalfKerSize-k] != 0 || result.Values[d][MaxHalfKerSize+k] != 0)
                {
                    result.HalfKernelSize = k;
                    if (result.Values[d][MaxHalfKerSize-k] != result.Values[d][MaxHalfKerSize+k])
                    {
                        result.Symmetric = false;
                    }
                }
            }
        }

        return result;
    }
};

template <typename IMAGE_CLASS, int KERNEL_DIM = 1>
class FilterSepProcessingContext
{
public:
    IMAGE_CLASS Input;
    IMAGE_CLASS Output;
    FilterSepKernel Kernel;
};

template <int HALFKERSIZE, bool SYMMETRIC, typename VECTOR_TYPE>
struct FilterSepFunctor
{
    template <int I, typename T_IN1, typename T_IN2>
    FORCEINLINE void operator()(VECTOR_TYPE & result, T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        if (I == 0)
            result = VECTOR_TYPE(rKernel[0]) * vecData.at<0>();
        else
            result += VECTOR_TYPE(rKernel[I]) * vecData.at<I>();
    }

    template <typename T_IN1, typename T_IN2>
    FORCEINLINE VECTOR_TYPE operator()(T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        VECTOR_TYPE result;
        unroller<0, 1+2*HALFKERSIZE>::go(*this, result, vecData, rKernel);
        return result;
    }
};

template <int HALFKERSIZE, typename VECTOR_TYPE>
struct FilterSepFunctor<HALFKERSIZE, true, VECTOR_TYPE>
{
    template <int I, typename T_IN1, typename T_IN2>
    FORCEINLINE void operator()(VECTOR_TYPE & result, T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        if (I == 0)
            result = VECTOR_TYPE(rKernel[0]) * vecData.at<HALFKERSIZE>();
        else
            result += VECTOR_TYPE(rKernel[I]) * (vecData.at<HALFKERSIZE+I>() + vecData.at<HALFKERSIZE-I>());
    }

    template <typename T_IN1, typename T_IN2>
    FORCEINLINE VECTOR_TYPE operator()(T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        VECTOR_TYPE result;
        unroller<0, 1+HALFKERSIZE>::go(*this, result, vecData, rKernel);
        return result;
    }
};

template <int HALFKERSIZE>
struct FilterSepFunctor<HALFKERSIZE, true, Is16vec8>
{
    template <int I, typename T_IN1, typename T_IN2>
    FORCEINLINE void operator()(Is16vec8 & result, T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        if (I == 0)
            result = mul_hrs(rKernel[0], vecData.at<HALFKERSIZE>());
        else
            result = sat_add(result, mul_hrs(rKernel[I], sat_add(vecData.at<HALFKERSIZE+I>(), vecData.at<HALFKERSIZE-I>())));
    }
    
    template <typename T_IN1, typename T_IN2>
    FORCEINLINE Is16vec8 operator()(T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        Is16vec8 result;
        unroller<0, 1+HALFKERSIZE>::go(*this, result, vecData, rKernel);
        return result;
    }
};

template <int HALFKERSIZE>
struct FilterSepFunctor<HALFKERSIZE, false, Is16vec8>
{
    template <int I, typename T_IN1, typename T_IN2>
    FORCEINLINE void operator()(Is16vec8 & result, T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        if (I == 0)
            result = mul_hrs(rKernel[0], vecData.at<0>());
        else
            result = sat_add(result, mul_hrs(rKernel[I], vecData.at<I>()));
    }
    
    template <typename T_IN1, typename T_IN2>
    FORCEINLINE Is16vec8 operator()(T_IN1 const & vecData, T_IN2 const & rKernel)
    {
        Is16vec8 result;
        unroller<0, 1+2*HALFKERSIZE>::go(*this, result, vecData, rKernel);
        return result;
    }
};

#define USE_COMPACT_KERNEL

template <int KERNEL_DIM, int HALFKERSIZE, bool SYMMETRIC, typename VALUE_TYPE>
class FilterSepProcessingForSize
{
    enum { KERCOUNT = SYMMETRIC ? 1+HALFKERSIZE : 2*HALFKERSIZE+1 };

    typedef typename PXVEC_TYPE(VALUE_TYPE) OptimalVectorType;
    typedef typename PXVEC_TYPE_FORSIZE(VALUE_TYPE, OptimalVectorType::SCALAR_COUNT <= HALFKERSIZE ? 8 : OptimalVectorType::SCALAR_COUNT) VectorType;
    typedef VALUE_TYPE                                 ValueType;
    typedef FilterSepFunctor<HALFKERSIZE, SYMMETRIC, VectorType> FunctorType;

    enum { VectorSize  = sizeof(VectorType)/sizeof(ValueType) };

public:
    static size_t ProcessROI3D_GetRequiredPoolSize(int maxInputWidth)
    {
        size_t poolSize = 0;

        FixedLinesCircularBuffer<2 * HALFKERSIZE + 1, VALUE_TYPE> intermediary;
        AddCircularBufferToPool(intermediary, (void *&)poolSize, 0, maxInputWidth + 2 * HALFKERSIZE, VectorType::SCALAR_COUNT, true);
        return poolSize;
    }

    class Params
    {
        friend class FilterSepProcessingForSize;
#ifdef USE_COMPACT_KERNEL
        typedef ValueType KernelType[KERNEL_DIM][KERCOUNT];
#else
        typedef VecArray<VectorType, KERNEL_DIM * KERCOUNT> KernelType;
#endif
        KernelType m_kernel;

    public:
        template <typename KERNEL>
        FORCEINLINE void Fill(KERNEL const & kernel)
        {
            int const MaxHalfKerSize = FilterSepKernel::MaxSize / 2;
            //int offset = (FilterSepKernel::MaxSize - 2*HALFKERSIZE + 1) / 2;
            for (int _d = 0; _d < KERNEL_DIM; _d++)
            {
                int d = __min(_d, kernel.KernelDim - 1);
                if (SYMMETRIC)
                {
                    for (int k = 0; k <= HALFKERSIZE; k++)
                    {
                        m_kernel[_d][k] = saturate<ValueType>(dbltofix<ValueType>(kernel.Values[d][MaxHalfKerSize + k]));
                    }
                }
                else
                {
                    for (int k = -HALFKERSIZE; k <= HALFKERSIZE; k++)
                    {
                        m_kernel[_d][HALFKERSIZE + k] = saturate<ValueType>(dbltofix<ValueType>(kernel.Values[d][MaxHalfKerSize + k]));
                    }
                }
            }
        }
    };

    class LineContext
    {
        friend class FilterSepProcessingForSize;
        friend class LinesLoader<true, VectorType, LineContext>;
        friend class LinesLoader<false, VectorType, LineContext>;

        ValueType const * inputLines[2*HALFKERSIZE+1];
        ValueType       * outputLine;

    public:
        template <typename T1, typename T2>
        FORCEINLINE void Fill(int currentLine, int top, int bottom, T1 Input, T2 Output)
        {
            outputLine = Output.PointerAt(0, currentLine);

            inputLines[HALFKERSIZE] = Input.PointerAt(0, currentLine);
            for (int I = 1; I <= HALFKERSIZE; I++)
            {
                inputLines[HALFKERSIZE-I] = Input.PointerAt(0, MirrorIndex(currentLine - I, top, bottom));
                inputLines[HALFKERSIZE+I] = Input.PointerAt(0, MirrorIndex(currentLine + I, top, bottom));
            }
        }

        template <typename T1, typename T2>
        FORCEINLINE void Increment3D_Pass2(int y, int firstY, int lastY, int z, T1 Input, T2 Output)
        {
            outputLine = Output.PointerAt(0, y, z);

            for (int I = 0; I < 2*HALFKERSIZE; I++)
            {
                inputLines[I] = inputLines[I + 1];
            }

            if (EXPECT(y < lastY - HALFKERSIZE, true))
            {
                Input.IncrementY(inputLines[2 * HALFKERSIZE]);
            }
            else
            {
                inputLines[2 * HALFKERSIZE] = Input.PointerAt(0, MirrorIndex(y + HALFKERSIZE, firstY, lastY));
            }
        }

        template <typename T1, typename T2>
        FORCEINLINE void Fill3D_Pass2(int y, int firstY, int lastY, int z, T1 Input, T2 Output)
        {
            // Input should be 2D circular buffer
            // Output is 3D output
            outputLine = Output.PointerAt(0, y, z);

            inputLines[HALFKERSIZE] = Input.PointerAt(0, y);
            for (int I = 1; I <= HALFKERSIZE; I++)
            {
                inputLines[HALFKERSIZE-I] = Input.PointerAt(0, MirrorIndex(y - I, firstY, lastY));
                inputLines[HALFKERSIZE+I] = Input.PointerAt(0, MirrorIndex(y + I, firstY, lastY));
            }
        }

        template <typename T1, typename T2>
        FORCEINLINE void Increment3D_Pass1(int y, int z, int firstZ, int lastZ, T1 Input, T2 Output)
        {
            Output.IncrementY(outputLine);
     
            for (int I = 0; I <= 2*HALFKERSIZE; I++)
            {
                inputLines[I] += Input.StrideY;
            }
        }

        template <typename T1, typename T2>
        FORCEINLINE void Fill3D_Pass1(int y, int z, int firstZ, int lastZ, T1 Input, T2 Output)
        {
            // Output should be 2D circular buffer
            // Input is 3D input
            outputLine = Output.PointerAt(0, y);

            inputLines[HALFKERSIZE] = Input.PointerAt(0, y, z);
            for (int I = 1; I <= HALFKERSIZE; I++)
            {
                inputLines[HALFKERSIZE-I] = Input.PointerAt(0, y, MirrorIndex(z - I, firstZ, lastZ));
                inputLines[HALFKERSIZE+I] = Input.PointerAt(0, y, MirrorIndex(z + I, firstZ, lastZ));
            }
        }
    };

    template <int CUR_KERNEL_DIM, bool ALIGNED, bool VERTICAL>
    static FORCEINLINE VectorType ProcessVertical(int currentColumn, Params const & lineParams, LineContext const & lineContext)
    {
        if (VERTICAL)
        {
            return FunctorType()(LinesLoader<ALIGNED, VectorType, LineContext>(currentColumn, lineContext), lineParams.m_kernel[__min(CUR_KERNEL_DIM + 1, KERNEL_DIM - 1)]);
        }
        return load<ALIGNED, VectorType>(lineContext.outputLine + currentColumn);
    }

    struct LineWorkspace
    {
        VectorType previous;
        VectorType current;
        VectorType next;
    };

    template <int CUR_KERNEL_DIM, bool ALIGNED, bool VERTICAL, bool HORIZONTAL>
    static FORCEINLINE void BeginProcess(int currentColumn, int leftAvailable,
        LineWorkspace & lineWorkspace, Params const & lineParams, LineContext const & lineContext)
    {
        if (HORIZONTAL)
        {
            if (HALFKERSIZE > 0)
            {
                if (leftAvailable > 0)
                    lineWorkspace.previous = ProcessVertical<CUR_KERNEL_DIM, ALIGNED, VERTICAL>(currentColumn - VectorSize, lineParams, lineContext);
                lineWorkspace.current = ProcessVertical<CUR_KERNEL_DIM, ALIGNED, VERTICAL>(currentColumn, lineParams, lineContext);

                ApplyLeftBorder<HALFKERSIZE>(leftAvailable, lineWorkspace.previous, lineWorkspace.current);
            }
        }
    }

    template <int CUR_KERNEL_DIM, bool NONTEMPORAL, bool ALIGNED, bool INPUT_ALIGNED, bool VERTICAL, bool HORIZONTAL>
    static FORCEINLINE void ProcessVector(int currentColumn, int count, int rightAvailable,
        LineWorkspace & lineWorkspace, Params const & lineParams, LineContext const & lineContext)
    {
        if (VERTICAL || HORIZONTAL)
        {
            VectorType output;

            if (!HORIZONTAL || HALFKERSIZE == 0)
            {
                output = ProcessVertical<CUR_KERNEL_DIM, INPUT_ALIGNED, VERTICAL>(currentColumn, lineParams, lineContext);
            }
            else
            {
                if (count > VectorSize - HALFKERSIZE) lineWorkspace.next = ProcessVertical<CUR_KERNEL_DIM, INPUT_ALIGNED, VERTICAL>(currentColumn + VectorSize, lineParams, lineContext);
                ApplyRightBorder<HALFKERSIZE>(count, rightAvailable, lineWorkspace.previous, lineWorkspace.current, lineWorkspace.next);
                VecCombiner<HALFKERSIZE, VectorType> combiner(lineWorkspace.previous, lineWorkspace.current, lineWorkspace.next);
                output = FunctorType()(combiner, lineParams.m_kernel[__min(KERNEL_DIM - 1, CUR_KERNEL_DIM)]);
            }

            store<ALIGNED>(&lineContext.outputLine[currentColumn], output, count);

            if ((HALFKERSIZE > 0) && HORIZONTAL)
            {
                lineWorkspace.previous = lineWorkspace.current;
                lineWorkspace.current = lineWorkspace.next;
            }
        }
    }

public:
    template <bool NONTEMPORAL, bool OUTPUT_ALIGNED, bool INPUT_ALIGNED, bool PASS_Z, bool PASS_Y, bool PASS_X>
    static NOINLINE void ProcessLine(int left, int right,
                            int leftAvailable, int rightAvailable,
                            Params const & lineParams,
                            LineContext const & lineContextPassZ, LineContext const & lineContextPassYX)
    {
        enum { BorderVectorCount = __max(PASS_X ? 1 : 0, HALFKERSIZE / VectorType::SCALAR_COUNT) };

        int x = left;

        enum { Z_Phase = PASS_Y ? VectorSize : 0 };

        LineWorkspace lineWorkspace;

        enum { INTER_ALIGNED = true };

        if (!PASS_X)
        {
            int x_limit = right;

            if (leftAvailable > 0)
            {
                // In that case leftAvailable is just a number of pixel to process before we are aligned on the circular buffer (case when roiOutput.firstX > intermediaryFirstX)
                ProcessVector<2, false, false, false, PASS_Z, false>(x, leftAvailable, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
                x += leftAvailable;
            }

            if (Z_Phase > 0)
            {
                ProcessVector<2, false, INTER_ALIGNED, INPUT_ALIGNED, PASS_Z, false>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
                x_limit -= Z_Phase;
            }

            for (; x < x_limit; x += VectorSize)
            {
                ProcessVector<2, false, INTER_ALIGNED, INPUT_ALIGNED, PASS_Z, false>(x + Z_Phase, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
                ProcessVector<0, false, OUTPUT_ALIGNED, INTER_ALIGNED, PASS_Y, false>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassYX);
            }

            if (Z_Phase > 0)
            {
                ProcessVector<0, false, OUTPUT_ALIGNED, INTER_ALIGNED, PASS_Y, false>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassYX);
            }
        }
        else /* if (PASS_X) */
        {
            int x_limit = right - HALFKERSIZE - VectorType::SCALAR_COUNT;

            if (leftAvailable > 0)
            {
                ProcessVector<2, false, false, false, PASS_Z, false>(x - leftAvailable, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
            }

            ProcessVector<2, false, INTER_ALIGNED, INPUT_ALIGNED, PASS_Z, false>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
            BeginProcess<0, INPUT_ALIGNED, PASS_Y, true>(x, leftAvailable, lineWorkspace, lineParams, lineContextPassYX);

            for (; x < x_limit; x += VectorSize)
            {
                ProcessVector<2, false, INTER_ALIGNED, INPUT_ALIGNED, PASS_Z, false>(x + VectorSize, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
                ProcessVector<0, false, OUTPUT_ALIGNED, INTER_ALIGNED, PASS_Y, true>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassYX);
            }

            //if (BorderVectorCount > 1)
            {
                int rightAvailable2 = rightAvailable + right - VectorSize - x;

                while (x < right - VectorSize)
                {
                    // In some case we can have a border effect in the last full vector
                    ProcessVector<2, false, INTER_ALIGNED, INPUT_ALIGNED, PASS_Z, false>(x + VectorSize, right - (VectorSize + x), HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
                    ProcessVector<0, false, OUTPUT_ALIGNED, INTER_ALIGNED, PASS_Y, true>(x, VectorSize, rightAvailable2, lineWorkspace, lineParams, lineContextPassYX);

                    x += VectorSize;
                    rightAvailable2 -= VectorSize;
                }
            }

            if (EXPECT(x < right, true))
            {
                int rightAvailable2 = __min(HALFKERSIZE, rightAvailable);
                if (rightAvailable2 > 0)
                {
                    ProcessVector<2, false, INTER_ALIGNED, INPUT_ALIGNED, PASS_Z, false>(x + VectorSize, rightAvailable2, HALFKERSIZE, lineWorkspace, lineParams, lineContextPassZ);
                }
                ProcessVector<0, false, OUTPUT_ALIGNED, INTER_ALIGNED, PASS_Y, true>(x, right - x, rightAvailable, lineWorkspace, lineParams, lineContextPassYX);
            }
        }
    }

    template <bool NONTEMPORAL, bool ALIGNED, bool INPUT_ALIGNED>
    static FORCEINLINE void ProcessLine(int left, int right,
                            int leftAvailable, int rightAvailable,
                            Params const & lineParams,
                            LineContext const & lineContext)
    {
        enum { BorderVectorCount = __max(1, HALFKERSIZE / VectorType::SCALAR_COUNT) };

        int x = left;
        int x_limit = right - BorderVectorCount * VectorSize - HALFKERSIZE;

        LineWorkspace lineWorkspace;

        BeginProcess<0, INPUT_ALIGNED, true, true>(x, leftAvailable, lineWorkspace, lineParams, lineContext);

        for (; x < x_limit; x += VectorSize)
        {
            ProcessVector<0, NONTEMPORAL, ALIGNED, INPUT_ALIGNED, true, true>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContext);
        }

        if (BorderVectorCount * VectorSize + HALFKERSIZE > VectorSize)
        {
            while (x < right - VectorSize)
            {
                int rightAvailable2 = rightAvailable + right - VectorSize - x;
                // In some case we can have a border effect in the last full vector
                ProcessVector<0, NONTEMPORAL, ALIGNED, INPUT_ALIGNED, true, true>(x, VectorSize, rightAvailable2, lineWorkspace, lineParams, lineContext);
                x += VectorSize;
            }
        }

        if (x < right)
        {
            ProcessVector<0, NONTEMPORAL, ALIGNED, INPUT_ALIGNED, true, true>(x, right - x, rightAvailable, lineWorkspace, lineParams, lineContext);
        }
    }

    template <bool ALIGNED, bool INPUT_ALIGNED>
    static FORCEINLINE void ProcessLine(int left, int right,
        int leftAvailable, int rightAvailable,
        Params const & lineParams,
        LineContext const & lineContext)
    {
        return ProcessLine<false, ALIGNED, INPUT_ALIGNED>(left, right, leftAvailable, rightAvailable, lineParams, lineContext);
    }

    template <bool NONTEMPORAL, bool ALIGNED, typename CONTEXT, typename REGION>
    static void ProcessROI3D(REGION const & roiOutput, REGION const & roiInput,
                              CONTEXT const & context, void * pool, void * poolEnd)
    {
        Params currentParams;
        currentParams.Fill(context.Kernel);

        FixedLinesCircularBuffer<2 * HALFKERSIZE + 1, ValueType> intermediary;

        int intermediaryFirstX = std::max<int>(roiInput.firstX, roiOutput.firstX - HALFKERSIZE);
        int intermediaryLastX  = std::min<int>(roiInput.lastX , roiOutput.lastX  + HALFKERSIZE);
        int intermediaryFirstY = std::max<int>(roiInput.firstY, roiOutput.firstY - HALFKERSIZE);
        int intermediaryLastY  = std::min<int>(roiInput.lastY , roiOutput.lastY  + HALFKERSIZE);

        AddCircularBufferToPoolEx(intermediary, pool, intermediaryFirstX, intermediaryLastX, roiOutput.firstX, VectorType::SCALAR_COUNT);
        assert(pool <= poolEnd);

        for (int z = roiOutput.firstZ; z < roiOutput.lastZ; z++)
        {
            LineContext currentLineContextPass1;
            LineContext currentLineContextPass2;

            currentLineContextPass1.Fill3D_Pass1(roiOutput.firstY, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
            currentLineContextPass2.Fill3D_Pass2(roiOutput.firstY, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);

            int y;

            for (y = roiOutput.firstY - HALFKERSIZE; y < roiOutput.firstY; y++)
            {
                //currentLineContextPass1.Fill3D_Pass1(y + HALFKERSIZE, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                ProcessLine<NONTEMPORAL, true, ALIGNED, true, false, false>(intermediaryFirstX, intermediaryLastX,
                    /* pixelToProcessToBeAlignedOnIntermediary = */ roiOutput.firstX - intermediaryFirstX,
                    0, currentParams, currentLineContextPass1, LineContext());

                currentLineContextPass1.Increment3D_Pass1(y + HALFKERSIZE + 1, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
            }

            for (; y < roiOutput.lastY - HALFKERSIZE; y++)
            {
                //currentLineContextPass1.Fill3D_Pass1(y + HALFKERSIZE, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                //currentLineContextPass2.Fill3D_Pass2(y, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);

                ProcessLine<NONTEMPORAL, ALIGNED, ALIGNED, true, true, true>(roiOutput.firstX, roiOutput.lastX,
                    roiOutput.firstX - intermediaryFirstX, intermediaryLastX - roiOutput.lastX, currentParams,
                    currentLineContextPass1, currentLineContextPass2);

                currentLineContextPass1.Increment3D_Pass1(y + HALFKERSIZE + 1, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                currentLineContextPass2.Increment3D_Pass2(y + 1, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);
            }

            for (; y < roiOutput.lastY; y++)
            {
                //currentLineContextPass2.Fill3D_Pass2(y, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);
                //currentLineContextPass1.Fill3D_Pass1(y + HALFKERSIZE, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);

                ProcessLine<NONTEMPORAL, ALIGNED, true, false, true, true>(roiOutput.firstX, roiOutput.lastX,
                    roiOutput.firstX - intermediaryFirstX, intermediaryLastX - roiOutput.lastX, currentParams,
                    LineContext(), currentLineContextPass2);
                
                currentLineContextPass2.Increment3D_Pass2(y + 1, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);
            }
        }
    }
#if 0
    template <bool NONTEMPORAL, bool ALIGNED, typename CONTEXT, typename REGION>
    static void ProcessROI3D(REGION const & roiOutput, REGION const & roiInput,
                              CONTEXT const & context, void * pool, void * poolEnd)
    {
        Params currentParams;
        currentParams.Fill(context.Kernel);

        FixedLinesCircularBuffer<2 * HALFKERSIZE + 1, ValueType> intermediary;

        int intermediaryFirstX = std::max<int>(roiInput.firstX, roiOutput.firstX - HALFKERSIZE);
        int intermediaryLastX  = std::min<int>(roiInput.lastX , roiOutput.lastX  + HALFKERSIZE);
        int intermediaryFirstY = std::max<int>(roiInput.firstY, roiOutput.firstY - HALFKERSIZE);
        int intermediaryLastY  = std::min<int>(roiInput.lastY , roiOutput.lastY  + HALFKERSIZE);

        AddCircularBufferToPool(intermediary, pool, intermediaryFirstX, intermediaryLastX, VectorType::SCALAR_COUNT);
        assert(pool < poolEnd);

        int blockWidth = 32;

        for (int z = roiOutput.firstZ; z < roiOutput.lastZ; z++)
        {
            for (int firstX = intermediaryFirstX; firstX < intermediaryLastX; firstX += blockWidth)
            {
                LineContext currentLineContextPass1;
                LineContext currentLineContextPass2;

                int lastX = min(blockWidth, firstX + blockWidth);

                currentLineContextPass1.Fill3D_Pass1(roiOutput.firstY, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                //currentLineContextPass2.Fill3D_Pass2(roiOutput.firstY, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);

                for (int y = roiOutput.firstY - HALFKERSIZE; y < roiOutput.firstY; y++)
                {
                    //currentLineContextPass1.Fill3D_Pass1(y + HALFKERSIZE, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);

                    ProcessLine<NONTEMPORAL, true, ALIGNED, true, false>(firstX, lastX, 0, 0, currentParams,
                        currentLineContextPass1, LineContext());

                    currentLineContextPass1.Increment3D_Pass1(y + HALFKERSIZE + 1, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                }

                for (int y = roiOutput.firstY; y < roiOutput.lastY - HALFKERSIZE; y++)
                {
                    //currentLineContextPass1.Fill3D_Pass1(y + HALFKERSIZE, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                    //currentLineContextPass2.Fill3D_Pass2(y, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);

                    ProcessLine<NONTEMPORAL, ALIGNED, true, true, false>(roiOutputForBlock.firstX, roiOutputForBlock.lastX, 0, 0, currentParams,
                        currentLineContextPass1, currentLineContextPass2);

                    currentLineContextPass1.Increment3D_Pass1(y + HALFKERSIZE + 1, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                    currentLineContextPass2.Increment3D_Pass2(y + 1, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);
                }

                for (int y = roiOutput.lastY - HALFKERSIZE; y < roiOutput.lastY; y++)
                {
                    //currentLineContextPass2.Fill3D_Pass2(y, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);

                    //currentLineContextPass1.Fill3D_Pass1(y + HALFKERSIZE, z, roiInput.firstZ, roiInput.lastZ, context.Input, intermediary);
                    ProcessLine<NONTEMPORAL, ALIGNED, true, false, false>(roiOutputForBlock.firstX, roiOutputForBlock.lastX, 0, 0, currentParams,
                        LineContext(), currentLineContextPass2);

                    currentLineContextPass2.Increment3D_Pass2(y + 1, intermediaryFirstY, intermediaryLastY, z, intermediary, context.Output);
                }
            }

            for (int y = roiOutput.firstY; y < roiOutput.lastY; y++)
            {

            }
        }
    }


#endif

    template <bool NONTEMPORAL, typename CONTEXT, typename REGION>
    static void ProcessROI3D(REGION const & roiOutput, REGION const & roiInput,
                             CONTEXT const & context, void * pool, void * poolEnd)
    {
        if (context.Input .IsAligned(roiOutput.firstX)
         && context.Output.IsAligned(roiOutput.firstX))
            return ProcessROI3D<NONTEMPORAL, true>(roiOutput, roiInput, context, pool, poolEnd);
        else

            return ProcessROI3D<NONTEMPORAL, false>(roiOutput, roiInput, context, pool, poolEnd);
    }

    template <bool ALIGNED, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput,
                           CONTEXT const & context)
    {
        Params currentParams;
        currentParams.Fill(context.Kernel);

        for (int y = roiOutput.top; y < roiOutput.bottom; y++)
        {
            LineContext currentLineContext;
            currentLineContext.Fill(y, roiInput.top, roiInput.bottom, context.Input, context.Output);

            ProcessLine<false, ALIGNED, ALIGNED>(roiOutput.left, roiOutput.right,
                roiOutput.left - roiInput.left, roiInput.right - roiOutput.right, currentParams, currentLineContext);
        }
    }

    template <typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput,
                           CONTEXT const & context)
    {
#ifndef PLATFORM_ALIGN_DOESNTMATTERS
        if (context.Input .IsAligned(roiOutput.left)
         && context.Output.IsAligned(roiOutput.left))
            return ProcessROI<true>(roiOutput, roiInput, context);
        else
#endif
            return ProcessROI<false>(roiOutput, roiInput, context);
    }
};

template <typename VALUE_TYPE>
class FilterSepProcessing
{
    template <int KERNEL_DIM, bool SYMMETRICAL, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        switch (context.Kernel.HalfKernelSize)
        {
        #define ProcessROI_FORSIZE(I) \
        case I: FilterSepProcessingForSize<KERNEL_DIM, I, SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context); break;

#ifndef BASICFILTERS_FASTCOMPILE
        ProcessROI_FORSIZE(0)
        ProcessROI_FORSIZE(1)
        ProcessROI_FORSIZE(2)
        ProcessROI_FORSIZE(3)
        ProcessROI_FORSIZE(4)
        ProcessROI_FORSIZE(5)
        ProcessROI_FORSIZE(6)
#endif
        default:
        ProcessROI_FORSIZE(7)
        #undef ProcessROI_FORSIZE
        }
    }

    template <bool SYMMETRICAL, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
#ifdef BASICFILTERS_ISOTROPIC_CODEPATH
        if (context.Kernel.KernelDim >= 2)
        {
#endif
            ProcessROI<2, SYMMETRICAL>(roiOutput, roiInput, context);
#ifdef BASICFILTERS_ISOTROPIC_CODEPATH
        }
        else
        {
            ProcessROI<1, SYMMETRICAL>(roiOutput, roiInput, context);
        }
#endif
    }

    template <int KERNEL_DIM, bool SYMMETRICAL, bool NONTEMPORAL, typename CONTEXT, typename REGION>
    static void ProcessROI3D(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context, void * pool, void * poolEnd)
    {
        switch (context.Kernel.HalfKernelSize)
        {
        #define ProcessROI3D_FORSIZE(I) \
            case I: FilterSepProcessingForSize<KERNEL_DIM, I, SYMMETRICAL, VALUE_TYPE>::ProcessROI3D<NONTEMPORAL>(roiOutput, roiInput, context, pool, poolEnd); break;
#ifndef BASICFILTERS_FASTCOMPILE
        ProcessROI3D_FORSIZE(0)
        ProcessROI3D_FORSIZE(1)
        ProcessROI3D_FORSIZE(2)
        ProcessROI3D_FORSIZE(3)
        ProcessROI3D_FORSIZE(4)
        ProcessROI3D_FORSIZE(5)
        ProcessROI3D_FORSIZE(6)
#endif
        default:
        ProcessROI3D_FORSIZE(7)
        #undef ProcessROI3D_FORSIZE
        }
    }

    template <bool SYMMETRICAL, bool NONTEMPORAL, typename CONTEXT, typename REGION>
    static void ProcessROI3D(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context, void * pool, void * poolEnd)
    {
#ifdef BASICFILTERS_ISOTROPIC_CODEPATH
        if (context.Kernel.KernelDim >= 3)
        {
#endif
            ProcessROI3D<3, SYMMETRICAL, NONTEMPORAL>(roiOutput, roiInput, context, pool, poolEnd);
#ifdef BASICFILTERS_ISOTROPIC_CODEPATH
        }
        else
        {
            ProcessROI3D<1, SYMMETRICAL, NONTEMPORAL>(roiOutput, roiInput, context, pool, poolEnd);
        }
#endif
    }
public:
    static size_t ProcessROI3D_GetRequiredPoolSize(int maxInputWidth)
    {
        return FilterSepProcessingForSize<3, 7, false, VALUE_TYPE>::ProcessROI3D_GetRequiredPoolSize(maxInputWidth);
    }
    
    template <bool NONTEMPORAL, typename CONTEXT, typename REGION>
    static void ProcessROI3D(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context, void * pool, void * poolEnd)
    {
        if (context.Kernel.Symmetric)
            ProcessROI3D<true, NONTEMPORAL>(roiOutput, roiInput, context, pool, poolEnd);
        else
            ProcessROI3D<false, NONTEMPORAL>(roiOutput, roiInput, context, pool, poolEnd);
    }

    template <typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        if (context.Kernel.Symmetric)
            ProcessROI<true>(roiOutput, roiInput, context);
        else
            ProcessROI<false>(roiOutput, roiInput, context);
    }
};


template <typename IMAGE_CLASS>
class Filter2ProcessingContext
{
public:
    IMAGE_CLASS Input;
    IMAGE_CLASS Output;
    double const * KernelValues;
    size_t KernelSizeX;
    size_t KernelSizeY;
};

#define FITLER2_FIXEDPOINT 0 // 2x speed up in fixed point but precision issues to solve

template <int HALFKERSIZE, bool SYMMETRIC, typename VALUE_TYPE>
struct Filter2ProcessingForSize
{
    //enum { KERCOUNT = SYMMETRIC ? 1+HALFKERSIZE : 2*HALFKERSIZE+1 };

    typedef typename PXVEC_TYPE_FORSIZE(VALUE_TYPE, 8) VectorType;
    typedef VALUE_TYPE                                 ValueType;

    enum { VectorSize  = sizeof(VectorType)/sizeof(ValueType) };
#if FITLER2_FIXEDPOINT
    enum { CarryShift = 15-(2*log2<(2*HALFKERSIZE+1)>::Value+1) };
    enum { CarryAdd   = (1<<(CarryShift-1)) };
#endif

    struct LineParams
    {
        // Not Is16vec8 because for unknown reason the compiler would emit unaligned access
#if FITLER2_FIXEDPOINT
        VecArray<VectorType, (1+HALFKERSIZE)*(1+2*HALFKERSIZE)> m_kernel;
#else
        VecArray<F32vec4, (1+2*HALFKERSIZE)*(1+2*HALFKERSIZE)> m_kernel;
#endif

        FORCEINLINE void Fill(double const * kernelValues, size_t kerSizeX, size_t kerSizeY)
        {
            int offsetX = (2 * HALFKERSIZE + 1 - kerSizeX) / 2;
            int offsetY = (2 * HALFKERSIZE + 1 - kerSizeY) / 2;

            if (SYMMETRIC)
            {
                // TODO
                /*
                for (int k = 0; k <= HALFKERSIZE; k++)
                {
                    m_kernel[k] = saturate<ValueType>(flttofix<ValueType>(kernel.Values[MaxHalfKerSize + k]));
                }
                */
            }
            else
            {
                //for (int ky = 0; ky < kerSizeY; ky++)
                //    for (int kx = 0; kx < kerSizeX; kx ++)
                //        norm += kernelValues[(ky - offsetY) * kerSizeX + (kx - offsetX)];

                for (int ky = 0; ky <= 2*HALFKERSIZE; ky++)
                {
#if FITLER2_FIXEDPOINT
                    double kernelTemp[2*HALFKERSIZE+2];
                    for (int kx = 0; kx <= 2*HALFKERSIZE+1; kx ++)
                    {
                        if ((ky < offsetY || ky >= (offsetY + kerSizeY)
                            || (kx < offsetX || kx >= (offsetX + kerSizeX))))
                        {
                            kernelTemp[kx] = 0;
                        }
                        else
                        {
                            int toto = CarryShift;
                            int idx = (ky - offsetY) * kerSizeX + (kx - offsetX);
                            kernelTemp[kx] = saturate<ValueType>(kernelValues[idx] * double(1<<(CarryShift)));
                        }
                    }

                    for (int kx = 0; kx <= 2*HALFKERSIZE; kx += 2)
                    {
                        m_kernel[ky * (1 + HALFKERSIZE) + kx/2] = unpack_low(VectorType(kernelTemp[kx]), VectorType(kernelTemp[kx+1]));
                    }
#else
                    for (int kx = 0; kx <= 2*HALFKERSIZE; kx ++)
                    {
                        m_kernel[ky * (1 + 2*HALFKERSIZE) + kx] = kernelValues[(ky - offsetY) * kerSizeX + (kx - offsetX)];
                    }
#endif
                }
            }
        }
    };

    struct LineContext
    {
        ValueType const * inputLines[2*HALFKERSIZE+1];
        ValueType       * outputLine;

        template <typename T1, typename T2>
        FORCEINLINE void Fill(int currentLine, int top, int bottom, T1 Input, T2 Output)
        {
            outputLine = Output.PointerAt(0, currentLine);

            inputLines[HALFKERSIZE] = Input.PointerAt(0, currentLine);
            for (int I = 1; I <= HALFKERSIZE; I++)
            {
                inputLines[HALFKERSIZE-I] = Input.PointerAt(0, MirrorIndex(currentLine - I, top, bottom));
                inputLines[HALFKERSIZE+I] = Input.PointerAt(0, MirrorIndex(currentLine + I, top, bottom));
            }
        }
    };

    template <bool ALIGNED>
    static FORCEINLINE VectorType ProcessVertical(int currentColumn, LineParams const & lineParams, LineContext const & lineContext)
    {
        return FunctorType()(LinesLoader<ALIGNED, VectorType, LineContext>(currentColumn, lineContext), lineParams.m_kernel);
    }

    struct LineWorkspace
    {
        VecArray<VectorType, 2*HALFKERSIZE+1> previous;
        VecArray<VectorType, 2*HALFKERSIZE+1> current;
        VecArray<VectorType, 2*HALFKERSIZE+1> next;
    };

    template <bool ALIGNED>
    static FORCEINLINE void BeginProcess(int currentColumn, int leftAvailable,
        LineWorkspace & lineWorkspace, LineParams const & lineParams, LineContext const & lineContext)
    {
        if (HALFKERSIZE > 0)
        {
            for (int I = 0; I < 2*HALFKERSIZE+1; I++)
            {
                if (EXPECT(leftAvailable > 0, false))
                {
                    for (int I = 0; I < 2*HALFKERSIZE+1; I++)
                        lineWorkspace.previous[I] = load<ALIGNED, VectorType>(&lineContext.inputLines[I][currentColumn - VectorSize]);
                }
                lineWorkspace.current[I] = load<ALIGNED, VectorType>(&lineContext.inputLines[I][currentColumn]);
                ApplyLeftBorder<HALFKERSIZE>(leftAvailable, lineWorkspace.previous[I], lineWorkspace.current[I]);
            }
        }
    }

    struct Accumulator
    {
        template <int K, typename T1, typename T2, typename T3>
        FORCEINLINE void operator() (T1 & accum, T2 const & lineWorkspace, T3 const & lineParams)
        {
#if FITLER2_FIXEDPOINT
            enum { I = K / (1+HALFKERSIZE) };
            enum { J = K - I * (1+HALFKERSIZE) };

            VectorType even_part = combine<2*J-HALFKERSIZE+0>(lineWorkspace.previous[I], lineWorkspace.current[I], lineWorkspace.next[I]);
            VectorType odd_part  = combine<2*J-HALFKERSIZE+1>(lineWorkspace.previous[I], lineWorkspace.current[I], lineWorkspace.next[I]);

            cumulate<K==0>(accum, Is32vec8(mul_add(even_part, lineParams.m_kernel[I*(1+HALFKERSIZE)+J]),
                                           mul_add(odd_part , lineParams.m_kernel[I*(1+HALFKERSIZE)+J])));
#else
            enum { I = K / (1+2*HALFKERSIZE) };
            enum { J = K - I * (1+2*HALFKERSIZE) };

            VectorType contrib = combine<J-HALFKERSIZE>(lineWorkspace.previous[I], lineWorkspace.current[I], lineWorkspace.next[I]);

            cumulate<K==0>(accum, convert<F32vec8>(contrib) * F32vec8(lineParams.m_kernel[I*(1+2*HALFKERSIZE)+J]));
#endif
        }
    };

    template <bool ALIGNED, bool INPUT_ALIGNED>
    static FORCEINLINE void ProcessVector(int currentColumn, int count, int rightAvailable,
        LineWorkspace & lineWorkspace, LineParams const & lineParams, LineContext const & lineContext)
    {
        if (HALFKERSIZE > 0)
        {
            if (count > VectorSize - HALFKERSIZE)
            {
                for (int I = 0; I < 2*HALFKERSIZE+1; I++)
                    lineWorkspace.next[I] = load<ALIGNED, VectorType>(&lineContext.inputLines[I][currentColumn + VectorSize]);
            }

            for (int I = 0; I < 2*HALFKERSIZE+1; I++)
                ApplyRightBorder<HALFKERSIZE>(count, rightAvailable, lineWorkspace.previous[I], lineWorkspace.current[I], lineWorkspace.next[I]);
        }
        else
        {
            lineWorkspace.current[0] = load<ALIGNED, VectorType>(&lineContext.inputLines[0][currentColumn]);
        }

#if FITLER2_FIXEDPOINT
        Is32vec8 accum;

        unroller<0, (1+HALFKERSIZE)*(1+2*HALFKERSIZE)>::go(Accumulator(), accum, lineWorkspace, lineParams);

        VectorType output = sat_pack(accum >> CarryShift);

        output = unpack_low(output, shiftreg_right<4>(output));
#else
        F32vec8 accum;

        unroller<0, (1+2*HALFKERSIZE)*(1+2*HALFKERSIZE)>::go(Accumulator(), accum, lineWorkspace, lineParams);

        VectorType output = convert<VectorType>(accum);
#endif
        store<ALIGNED>(&lineContext.outputLine[currentColumn], output, count);

        if (HALFKERSIZE > 0)
        {
            lineWorkspace.previous = lineWorkspace.current;
            lineWorkspace.current = lineWorkspace.next;
        }
    }

    template <bool ALIGNED, bool INPUT_ALIGNED>
    static void ProcessLine(int left, int right,
                            int leftAvailable, int rightAvailable,
                            LineParams const & lineParams,
                            LineContext const & lineContext)
    {
        int x = left;
        int x_limit = right - 2 * VectorSize;

        LineWorkspace lineWorkspace;

        BeginProcess<INPUT_ALIGNED>(x, leftAvailable, lineWorkspace, lineParams, lineContext);

        for (; x < x_limit; x += VectorSize)
        {
            ProcessVector<ALIGNED, INPUT_ALIGNED>(x, VectorSize, HALFKERSIZE, lineWorkspace, lineParams, lineContext);
        }

        if (x < right - VectorSize)
        {
            // In some case we can have a border effect in the last full vector
            int rightAvailable2 = rightAvailable + right - VectorSize - x;
            ProcessVector<ALIGNED, INPUT_ALIGNED>(x, VectorSize, rightAvailable2, lineWorkspace, lineParams, lineContext);
            x += VectorSize;
        }
        if (x < right)
        {
            ProcessVector<ALIGNED, INPUT_ALIGNED>(x, right - x, rightAvailable, lineWorkspace, lineParams, lineContext);
        }
    }

    template <bool ALIGNED, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput,
                           CONTEXT const & context)
    {
        LineParams CurrentLineParams;
        CurrentLineParams.Fill(context.KernelValues, context.KernelSizeX, context.KernelSizeY);

        for (int y = roiOutput.top; y < roiOutput.bottom; y++)
        {
            LineContext CurrentLineContext;
            CurrentLineContext.Fill(y, roiInput.top, roiInput.bottom, context.Input, context.Output);

            ProcessLine<ALIGNED, ALIGNED>(roiOutput.left, roiOutput.right,
                roiOutput.left - roiInput.left, roiInput.right - roiOutput.right, CurrentLineParams, CurrentLineContext);
        }
    }

    template <typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput,
                           CONTEXT const & context)
    {
        if (context.Input .IsAligned(roiOutput.left)
         && context.Output.IsAligned(roiOutput.left))
            return ProcessROI<true>(roiOutput, roiInput, context);
        else
            return ProcessROI<false>(roiOutput, roiInput, context);
    }
};

template <typename VALUE_TYPE, int MAX_FILTER_SIZE = 3>
class Filter2Processing
{
    template <bool SYMMETRICAL, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        switch (__max(context.KernelSizeX, context.KernelSizeY) / 2)
        {
        case 1:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 1), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        case 2:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 2), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        case 3:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 3), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        case 4:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 4), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        case 5:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 5), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        case 6:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 6), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        case 7:
            Filter2ProcessingForSize<__min(MAX_FILTER_SIZE, 7), SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
        default:
            Filter2ProcessingForSize<0, SYMMETRICAL, VALUE_TYPE>::ProcessROI(roiOutput, roiInput, context);
            break;
        }
    }

public:
    template <typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        //if (context.Kernel.Symmetric)
          //  ProcessROI<true>(roiOutput, roiInput, context);
        //else
            ProcessROI<false>(roiOutput, roiInput, context);
    }
};


enum AddOrSubOrBlendOperations
{
    AddOrSubOrBlend_Add = 0,
    AddOrSubOrBlend_Sub = 1,
    AddOrSubOrBlend_Blend = 2
};    

template <typename IMAGE>
struct AddOrSubOrBlendProcessingContext
{
    IMAGE    First;
    IMAGE    Second;
    IMAGE    Output;
    Is16vec8 BlendFactor;
};

template <int OPERATION, typename TYPE>
struct AddOrSubOrBlendProcessing;

template <int OPERATION>
struct AddOrSubOrBlendProcessing<OPERATION, int16_t>
{
private:
    struct LineContext
    {
        short const  * m_pFirstLine;
        short const  * m_pSecondLine;
        short        * m_pOutput;
        Is16vec8       m_blendFactor;

        template <typename CONTEXT>
        void Fill(int iCurrentLine, CONTEXT const & context)
        {
            m_pFirstLine  = context.First .PointerAt(0, iCurrentLine);
            m_pSecondLine = context.Second.PointerAt(0, iCurrentLine);
            m_pOutput     = context.Output.PointerAt(0, iCurrentLine);
            m_blendFactor = context.BlendFactor;
        }
    };

    template <bool ALIGNED> static FORCEINLINE void ProcessVector(int currentColumn, int count, LineContext const & lineContext)
    {
        Is16vec8 vecOut;
        Is16vec8 vec1 = load<ALIGNED, Is16vec8>(&lineContext.m_pFirstLine [currentColumn]);
        Is16vec8 vec2 = load<ALIGNED, Is16vec8>(&lineContext.m_pSecondLine[currentColumn]);
        if (OPERATION == AddOrSubOrBlend_Add)
        {
            vecOut = sat_add(vec1, vec2);
        }
        else if (OPERATION == AddOrSubOrBlend_Sub)
        {
            vecOut = sat_sub(vec1, vec2);
        }
        else // if (OPERATION == Blend)
        {
            // blend * vec1 + vec2 * (1 - blend)
            vecOut = sat_add(mul_hrs(sat_sub(vec1, vec2), lineContext.m_blendFactor), vec2);
        }
        store<ALIGNED>(&lineContext.m_pOutput[currentColumn], vecOut, count);
    }

    template <bool ALIGNED> static FORCEINLINE void ProcessLine(int left, int right, LineContext const & lineContext)
    {
        int x = left, x_limit = right - 8;
        while (x <= x_limit)
        {
            ProcessVector<ALIGNED>(x, 8, lineContext);
            x += 8;
        }
        if (x < right) ProcessVector<ALIGNED>(x, right - x, lineContext);
    }

    template <bool ALIGNED, typename CONTEXT, typename REGION>
    static void ProcessROI(REGION const & roiOutput, CONTEXT const & context)
    {
        for (int iCurrentLine = roiOutput.top; iCurrentLine != roiOutput.bottom; iCurrentLine++)
        {
            LineContext CurrentLineContext;
            CurrentLineContext.Fill(iCurrentLine, context);
            ProcessLine<ALIGNED>(roiOutput.left, roiOutput.right, CurrentLineContext);
        }
    }

public:
    template <typename CONTEXT, typename REGION>
    static FORCEINLINE void ProcessROI(REGION const & roiOutput, CONTEXT const & context)
    {
        if (   context.First .IsAligned(roiOutput.left)
            && context.Second.IsAligned(roiOutput.left)
            && context.Output.IsAligned(roiOutput.left))
            ProcessROI<true>(roiOutput, context);
        else
            ProcessROI<false>(roiOutput, context);
    }
};

    }
}

namespace Philips
{
    namespace Medisys
    {
        namespace AlgoFragments
        {
            namespace BasicFilters
            {
                using ::AlgoFragmentsImpl::BinomialProcessingContext;
                using ::AlgoFragmentsImpl::BinomialProcessing;

                using ::AlgoFragmentsImpl::FilterSepProcessingContext;
                using ::AlgoFragmentsImpl::FilterSepProcessing;
                using ::AlgoFragmentsImpl::FilterSepProcessingForSize;
                using ::AlgoFragmentsImpl::FilterSepKernel;

                using ::AlgoFragmentsImpl::Filter2ProcessingContext;
                using ::AlgoFragmentsImpl::Filter2Processing;

                using ::AlgoFragmentsImpl::AddOrSubOrBlendProcessing;
                using ::AlgoFragmentsImpl::AddOrSubOrBlendProcessingContext;
                using ::AlgoFragmentsImpl::AddOrSubOrBlend_Add;
                using ::AlgoFragmentsImpl::AddOrSubOrBlend_Sub;
                using ::AlgoFragmentsImpl::AddOrSubOrBlend_Blend;
            }
        }
    }
}
