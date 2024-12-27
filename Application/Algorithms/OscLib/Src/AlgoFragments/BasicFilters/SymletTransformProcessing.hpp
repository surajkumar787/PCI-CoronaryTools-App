#pragma once

#include "../BasicFilters/BasicFiltersCommon.hpp"

namespace Philips
{
    namespace Medisys
    {
        namespace AlgoFragments
        {
            namespace Impl
            {

using ::std::min;
using ::std::max;
using namespace PXVec;

template <typename IMAGE_CLASS>
struct SymletTransformContext
{
    IMAGE_CLASS Input;
    IMAGE_CLASS Output;
    mutable double Sum;
    mutable double SquaredSum;
};

template <bool ESTIMATOR>
class Symlet4TransformProcessor
{
    enum {
        // Some constants to improve readibility of the code
        HalfKerSize = 4,
        LeftNeeded = HalfKerSize,
        RightNeeded = HalfKerSize-1,
        KernelSize = LeftNeeded + 1 + RightNeeded,
        Center = 1 + LeftNeeded,
        VecSize = Is16vec8::SCALAR_COUNT,
        VecBufSize = 3,
    };

    struct LineContext
    {
        LineContext()
        {
            m_sum = 0;
            m_squaredSum = 0;
        }

        int16_t const * m_pInputLines[KernelSize];
        int16_t       * m_pOutputLine;
        VecArray<Is16vec8, VecBufSize> m_vecVerticalBuffer;
        mutable F32vec4 m_sum;
        mutable F32vec4 m_squaredSum;
    };

    static FORCEINLINE Is16vec8 VecSymletVert4(VecArray<Is16vec8, KernelSize> const & vecSource)
    {
        Is16vec8 res;

        Is16vec8 const kernelInterleaved[] = {
            Is16vec8(-0.032223100604071 * (1<<15) - 0.5, -0.012603967262261 * (1<<15) - 0.5),
            Is16vec8(0.099219543576935  * (1<<15) + 0.5,  0.297857795605543 * (1<<15) + 0.5),
            Is16vec8(-0.803738751805216 * (1<<15) - 0.5,  0.497618667632459 * (1<<15) + 0.5),
            Is16vec8(0.029635527645954  * (1<<15) + 0.5, -0.075765714789341 * (1<<15) - 0.5) };


        // 14 to 15 bit (shift by 14 instead of 15)
        res = sat_pack((mul_add(Is16vec16(unpack(vecSource[0], vecSource[1])), Is16vec16(kernelInterleaved[0]))
                      + mul_add(Is16vec16(unpack(vecSource[2], vecSource[3])), Is16vec16(kernelInterleaved[1]))
                      + mul_add(Is16vec16(unpack(vecSource[4], vecSource[5])), Is16vec16(kernelInterleaved[2]))
                      + mul_add(Is16vec16(unpack(vecSource[6], vecSource[7])), Is16vec16(kernelInterleaved[3])) + Is32vec8(8192)) >> 14);

        return res;
    }

    static FORCEINLINE Is32vec4 VecSymletHorz4(VecArray<Is16vec8, KernelSize> const & vecSource)
    {
        Is32vec4 res;

        Is16vec8 const kernelInterleaved[] = {
            Is16vec8(-0.032223100604071 * (1<<15) - 0.5, -0.012603967262261 * (1<<15) - 0.5),
            Is16vec8(0.099219543576935  * (1<<15) + 0.5,  0.297857795605543 * (1<<15) + 0.5),
            Is16vec8(-0.803738751805216 * (1<<15) - 0.5,  0.497618667632459 * (1<<15) + 0.5),
            Is16vec8(0.029635527645954  * (1<<15) + 0.5, -0.075765714789341 * (1<<15) - 0.5) };


        // 15 to 14 bit (shift by 16 instead of 15)
        return (mul_add(vecSource[0], kernelInterleaved[0])
              + mul_add(vecSource[2], kernelInterleaved[1])
              + mul_add(vecSource[4], kernelInterleaved[2])
              + mul_add(vecSource[6], kernelInterleaved[3]) + Is32vec4(32768)) >> 16;
    }

    template <bool ALIGNED>
    static FORCEINLINE Is16vec8 ProcessVertical(int iOffset, LineContext const & lineContext)
    {
        VecArray<Is16vec8, KernelSize> vecInput;

        vecInput[0] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[0][iOffset]);
        vecInput[1] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[1][iOffset]);
        vecInput[2] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[2][iOffset]);
        vecInput[3] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[3][iOffset]);
        vecInput[4] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[4][iOffset]);
        vecInput[5] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[5][iOffset]);
        vecInput[6] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[6][iOffset]);
        vecInput[7] = load<ALIGNED, Is16vec8>(&lineContext.m_pInputLines[7][iOffset]);

        return VecSymletVert4(vecInput);
    }

    template <bool ALIGNED>
    static FORCEINLINE void ProcessBegin(int currentColumn, int leftAvailable, LineContext & lineContext)
    {
        if (leftAvailable > 0) lineContext.m_vecVerticalBuffer[1] = ProcessVertical<ALIGNED>(2 * currentColumn - 8, lineContext);
                                lineContext.m_vecVerticalBuffer[2] = ProcessVertical<ALIGNED>(2 * currentColumn, lineContext);

        ApplyLeftBorder<LeftNeeded>(leftAvailable, lineContext.m_vecVerticalBuffer[1], lineContext.m_vecVerticalBuffer[2]);
    }

    template <bool ALIGNED>
    static FORCEINLINE void ProcessVector(int currentColumn, int count, int rightAvailable, LineContext & lineContext)
    {
        //   ------ 0 ----- ----|------- 1 -------|------- 2 ---------
        //   X X X X X -3 -2 -1 | 0 1 2 3 4 5 6 7 | 8 9 10 X X X
        // 0         L  L  L  L | C R R R         |
        // 1               L  L | L L C R R R     |
        // 2                    | L L L L C R R R | 
        // 3                    |     L L L L C R | R R 
        // ==> For count > 2 we need to do another vertical processing
        lineContext.m_vecVerticalBuffer[0] = lineContext.m_vecVerticalBuffer[1];
        lineContext.m_vecVerticalBuffer[1] = lineContext.m_vecVerticalBuffer[2];
        if (count > 3) lineContext.m_vecVerticalBuffer[2] = ProcessVertical<ALIGNED>(2 * currentColumn + 8, lineContext);

        if (rightAvailable < RightNeeded) ApplyRightBorder<RightNeeded>(2 * count, rightAvailable,
            lineContext.m_vecVerticalBuffer[0], lineContext.m_vecVerticalBuffer[1], lineContext.m_vecVerticalBuffer[2]);

        VecArray<Is16vec8, KernelSize> vecSource;


        vecSource[0] = combine<-4>(lineContext.m_vecVerticalBuffer[0], lineContext.m_vecVerticalBuffer[1]);
        vecSource[1] = combine<-3>(lineContext.m_vecVerticalBuffer[0], lineContext.m_vecVerticalBuffer[1]);
        vecSource[2] = combine<-2>(lineContext.m_vecVerticalBuffer[0], lineContext.m_vecVerticalBuffer[1]);
        vecSource[3] = combine<-1>(lineContext.m_vecVerticalBuffer[0], lineContext.m_vecVerticalBuffer[1]);
        vecSource[4] = lineContext.m_vecVerticalBuffer[1];
        vecSource[5] = combine<1>(lineContext.m_vecVerticalBuffer[1], lineContext.m_vecVerticalBuffer[2]);
        vecSource[6] = combine<2>(lineContext.m_vecVerticalBuffer[1], lineContext.m_vecVerticalBuffer[2]);
        vecSource[7] = combine<3>(lineContext.m_vecVerticalBuffer[1], lineContext.m_vecVerticalBuffer[2]);

        // Note that odd vecSource items are not used in precise mode.
        Is32vec4 vecOutput = VecSymletHorz4(vecSource);

        if (ESTIMATOR)
        {
            F32vec4 vecOutputFloat = convert<F32vec4>(vecOutput);
            lineContext.m_squaredSum += vecOutputFloat * vecOutputFloat;
            lineContext.m_sum        += vecOutputFloat;
        }
        else
        {
            Is16vec8 vecOutputInt16 = sat_pack(vecOutput, Is32vec4());
            if (count == 4) store_low    (&lineContext.m_pOutputLine[currentColumn], vecOutputInt16);
            else             store_partial(&lineContext.m_pOutputLine[currentColumn], vecOutputInt16, count);
        }
    }

    template <bool ALIGNED, typename CONTEXT, typename REGION>
    static FORCEINLINE bool ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        Region2D rgnAvailable;
        rgnAvailable.left  = __min(LeftNeeded, 2 * roiOutput.left - roiInput.left);
        if (rgnAvailable.left  < -1) return false;
        rgnAvailable.right = __min(RightNeeded, roiInput.right - 2 * roiOutput.right);
        if (rgnAvailable.right < -1) return false;
        rgnAvailable.top    = __min(LeftNeeded, 2 * roiOutput.top - roiInput.top);
        if (rgnAvailable.top < -1) return false;
        rgnAvailable.bottom = __min(RightNeeded, roiInput.bottom - 2 * roiOutput.bottom);
        if (rgnAvailable.bottom < -1) return false;
        rgnAvailable.bottom += 2 * roiOutput.Height() - 1;

        double accumSum = 0;
        double accumSquaredSum = 0;

        for (int y = roiOutput.top; y < roiOutput.bottom; y++)
        {
            LineContext CurrentLineContext;
            CurrentLineContext.m_pInputLines[0] = context.Input .PointerAt(0, MirrorIndex(2 * y - 4, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[1] = context.Input .PointerAt(0, MirrorIndex(2 * y - 3, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[2] = context.Input .PointerAt(0, MirrorIndex(2 * y - 2, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[3] = context.Input .PointerAt(0, MirrorIndex(2 * y - 1, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[4] = context.Input .PointerAt(0, MirrorIndex(2 * y + 0, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[5] = context.Input .PointerAt(0, MirrorIndex(2 * y + 1, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[6] = context.Input .PointerAt(0, MirrorIndex(2 * y + 2, roiInput.top, roiInput.bottom));
            CurrentLineContext.m_pInputLines[7] = context.Input .PointerAt(0, MirrorIndex(2 * y + 3, roiInput.top, roiInput.bottom));

            if (!ESTIMATOR)
            {
                CurrentLineContext.m_pOutputLine    = context.Output.PointerAt(0, y);
            }

            int x = roiOutput.left;

            ProcessBegin<ALIGNED>(x, rgnAvailable.left, CurrentLineContext);
            int x_limit = roiOutput.right - 4;

            for (; x < x_limit; x += 4)
            {
                ProcessVector<ALIGNED>(x, 4, RightNeeded, CurrentLineContext); 
            }

            ProcessVector<ALIGNED>(x, roiOutput.right - x, rgnAvailable.right, CurrentLineContext);

            if (ESTIMATOR)
            {
                accumSum        += (double)sum(CurrentLineContext.m_sum);
                accumSquaredSum += (double)sum(CurrentLineContext.m_squaredSum);
            }

            rgnAvailable.top    += 2;
            rgnAvailable.bottom -= 2;
        }

        if (ESTIMATOR)
        {
            context.Sum        = accumSum;
            context.SquaredSum = accumSquaredSum;
        }

        return true;
    }

public:
    template <typename CONTEXT, typename REGION>
    static NOINLINE bool ProcessROI(REGION const & roiOutput, REGION const & roiInput, CONTEXT const & context)
    {
        if (context.Input.IsAligned(2*roiOutput.left, 16))
            return ProcessROI<true>(roiOutput, roiInput, context);
        else
            return ProcessROI<false>(roiOutput, roiInput, context);
    }
};

template <typename REGION>
static FORCEINLINE REGION SymletTransformOutputRegionFromInput(REGION const & regionOutput, REGION const & roiInput)
{
    REGION roiOutput;
    roiOutput.left   = max(regionOutput.left,   roiInput.left   / 2 + 4);
    roiOutput.right  = min(regionOutput.right,  roiInput.right  / 2 - 4);
    roiOutput.top    = max(regionOutput.top,    roiInput.top    / 2 + 4);
    roiOutput.bottom = min(regionOutput.bottom, roiInput.bottom / 2 - 4);
    return roiOutput;
}

template <typename REGION>
static FORCEINLINE REGION SymletTransformInputRegionFromOutput(REGION const & regionInput, REGION const & roiOutput)
{
    REGION roiInput;
    roiInput.left   =  max(regionInput.left,   roiOutput.left   * 2 - 4);
    roiInput.right  =  min(regionInput.right,  roiOutput.right  * 2 + 4);
    roiInput.top    =  max(regionInput.top,    roiOutput.top    * 2 - 4);
    roiInput.bottom =  min(regionInput.bottom, roiOutput.bottom * 2 + 4);
    return roiInput;
}

template <typename REGION>
static FORCEINLINE REGION SymletEstimatorOutputRegionFromInput(REGION const & regionOutput, REGION const & roiInput)
{
    REGION roiOutput;
    roiOutput.left   = max(regionOutput.left,   roiInput.left   + 4);
    roiOutput.right  = min(regionOutput.right,  roiInput.right  - 3);
    roiOutput.top    = max(regionOutput.top,    roiInput.top    + 4);
    roiOutput.bottom = min(regionOutput.bottom, roiInput.bottom - 3);
    return roiOutput;
}

template <typename REGION>
static FORCEINLINE REGION SymletEstimatorInputRegionFromOutput(REGION const & regionInput, REGION const & roiOutput)
{
    REGION roiInput;
    roiInput.left   =  max(regionInput.left,   roiOutput.left   - 4);
    roiInput.right  =  min(regionInput.right,  roiOutput.right  + 3);
    roiInput.top    =  max(regionInput.top,    roiOutput.top    - 4);
    roiInput.bottom =  min(regionInput.bottom, roiOutput.bottom + 3);
    return roiInput;
}

            }

            using Impl::SymletTransformContext;
            using Impl::Symlet4TransformProcessor;
            using Impl::SymletEstimatorInputRegionFromOutput;
            using Impl::SymletEstimatorOutputRegionFromInput;
            using Impl::SymletTransformInputRegionFromOutput;
            using Impl::SymletTransformOutputRegionFromInput;
        }
    }
}
