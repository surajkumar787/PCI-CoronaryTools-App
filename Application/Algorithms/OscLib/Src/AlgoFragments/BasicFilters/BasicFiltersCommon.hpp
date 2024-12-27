#pragma once

#include "../Common/Platform.h"
#include "../Common/MetaUnroller.h"

#include <cmath>
#include <cassert>
#include <limits>

namespace Philips
{
    namespace Medisys
    {
        namespace AlgoFragments
        {
            namespace Impl
            {

using namespace ::std;
using namespace ::Philips::Medisys::PXVec;
using namespace ::Philips::Medisys::Meta;

FORCEINLINE int MirrorIndex(int iCurrent, int iMin, int iMax)
{
    int iLength = iMax - iMin;
    int iPeriodisedCurrent = ((iCurrent - iMin) % (2*iLength) + (2*iLength)) % (2*iLength);
    return iMin + __min(iLength - 1, __min(iPeriodisedCurrent, 2*iLength - 1 - iPeriodisedCurrent));
}

FORCEINLINE int mod(int a, int b)
{
    if (a >= 0) return a % b;
    return (b - a) % b;
}

FORCEINLINE int MirrorIndexCentered(int current, int minBound, int maxBound)
{
    int length = maxBound - minBound;
    return minBound + ::std::abs(mod(current + length, 2*length)) - length;
}

FORCEINLINE int ClampIndex(int current, int minIndex, int maxIndex)
{
    if (current < minIndex)
        return minIndex;
    if (current >= maxIndex)
        return maxIndex - 1;
    return current;
}

// Utility structures to call BasicFilter outside IpStep environment
struct Region2D
{
    int left;
    int top;
    int right;
    int bottom;

    inline int Width() const { return right - left; }
    inline int Height() const { return bottom - top; }
};

template <typename REGION>
static inline REGION Region2D_Make(int left, int top, int right, int bottom)
{
    REGION result;
    result.left   = left;
    result.top    = top;
    result.right  = right;
    result.bottom = bottom;
    return result;
}

template <typename REGION>
static inline REGION Region2D_Rotate(REGION const & input)
{
    REGION result;
    result.left   = input.top;
    result.top    = 1-input.right;
    result.right  = input.bottom;
    result.bottom = 1-input.left;
    return result;
}

template <typename TYPE>
struct Image2D
{
    typedef TYPE Type;
    TYPE * BufferAtOrigin;
    int LineStride;

    FORCEINLINE TYPE * PointerAt(int x, int y) const
    {
        return BufferAtOrigin + LineStride * y + x;
    }

    FORCEINLINE bool IsAligned(int x, int iPadding = 16) const
    {
        return ((LineStride % iPadding) == 0)
            && (((((BufferAtOrigin + x) - (TYPE *)NULL) * sizeof(TYPE)) % iPadding) == 0);
    }
};

template <typename TYPE>
struct CircularBuffer
{
    typedef TYPE Type;

    int LineCount;
    int LineStride;
    int Left;
    int Top;
    TYPE * BufferAtOrigin;

    inline TYPE * PointerAt(int x, int y) const
    {
        TYPE * result = BufferAtOrigin + x - Left + ((((y - Top) % LineCount) + LineCount) % LineCount) * LineStride;
        return result;
    }

    inline TYPE * SingleLinePointerAt(int x, int y) const
    {
        assert(LineCount == 1);
        return BufferAtOrigin + x - Left;
    }

    //inline bool IsAligned(int x, int iPadding = 16) const
    //{
    //    return ((LineStride % iPadding) == 0)
    //        && (((((BufferAtOrigin + x) - (TYPE *)NULL) * sizeof(TYPE)) % iPadding) == 0);
    //}
};

template <typename C>
FORCEINLINE void AddCircularBufferToPool(C & circularBuffer, void *& pool, int left, int right, int lineCount, int vectorSize)
{
    ((int8_t *&)pool) += 16;

    circularBuffer.BufferAtOrigin = (typename C::Type *)pool;
    circularBuffer.LineStride     = idiv_ceil<int>(right - left, vectorSize) * vectorSize;
    circularBuffer.LineCount      = lineCount;
    circularBuffer.Left           = left;
    circularBuffer.Top            = 0;

    ((int8_t *&)pool) += 16 + lineCount * circularBuffer.LineStride * sizeof(circularBuffer.BufferAtOrigin[0]);
}


template <typename C>
FORCEINLINE void AddBufferToPool(C & circularBuffer, void *& pool, int left, int right, int top, int bottom, int vectorSize)
{
    ((int8_t *&)pool) += 16;

    circularBuffer.LineStride     = idiv_ceil<int>(right - left, vectorSize) * vectorSize;
    circularBuffer.BufferAtOrigin = ((typename C::Type *)pool);
    circularBuffer.LineCount      = bottom - top;
    circularBuffer.Left           = left;
    circularBuffer.Top            = top;

    ((int8_t *&)pool) += 16 + circularBuffer.LineCount * circularBuffer.LineStride * sizeof(circularBuffer.BufferAtOrigin[0]);
}

template <typename TYPE>
FORCEINLINE size_t RequiredBufferPoolSize(int maxWidth, int lineCount, int vectorSize)
{
    size_t lineStride = idiv_ceil<int>(maxWidth, vectorSize) * vectorSize;
    return 2 * vectorSize + lineCount * lineStride * sizeof(TYPE);
}

template <int LEFT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyLeftBorder(int iLeftAvailable,
                                        VEC_TYPE & vecLeft, VEC_TYPE & vecCenter)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    TYPE _MM_ALIGN16 sBuffer[2*VectorSize];

    if (iLeftAvailable >= LEFT_NEEDED)
        return;

    storea(sBuffer     , vecLeft  );
    storea(sBuffer + VectorSize, vecCenter);

    for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - iLeftAvailable); iLeftOffset++)
    {
        sBuffer[VectorSize - iLeftAvailable - iLeftOffset - 1] = sBuffer[VectorSize - iLeftAvailable + iLeftOffset];
    }

     vecLeft   = loada<VEC_TYPE>(sBuffer    );
     vecCenter = loada<VEC_TYPE>(sBuffer + VectorSize);
}

template <int LEFT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyLeftBorder(int iLeftAvailable,
                                        typename VEC_TYPE::SCALAR_TYPE & sLeft, VEC_TYPE & vecCenter)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    TYPE _MM_ALIGN16 sBuffer[2*VectorSize];

    if (iLeftAvailable >= LEFT_NEEDED)
        return;

    sBuffer[VectorSize-1] = sLeft;
    storea(sBuffer +  VectorSize, vecCenter);

    for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - iLeftAvailable); iLeftOffset++)
    {
        sBuffer[VectorSize - iLeftAvailable - iLeftOffset - 1] = sBuffer[VectorSize - iLeftAvailable + iLeftOffset];
    }

     sLeft = sBuffer[VectorSize-1];
     vecCenter = loada<VEC_TYPE>(sBuffer +  VectorSize);
}

template <int LEFT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyLeftBorder(int iLeftAvailable,
                                        typename VEC_TYPE::SCALAR_TYPE & sLeft, VEC_TYPE & vecCenter, VEC_TYPE & vecRight)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    TYPE _MM_ALIGN16 sBuffer[3*VectorSize];

    if (iLeftAvailable >= LEFT_NEEDED)
        return;

    sBuffer[VectorSize-1] = sLeft;
    storea(sBuffer +  VectorSize, vecCenter);
    storea(sBuffer +  2*VectorSize, vecRight );

    for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - iLeftAvailable); iLeftOffset++)
    {
        sBuffer[VectorSize - iLeftAvailable - iLeftOffset - 1] = sBuffer[VectorSize - iLeftAvailable + iLeftOffset];
    }

     sLeft = sBuffer[VectorSize-1];
     vecCenter = loada<VEC_TYPE>(sBuffer +  VectorSize);
     vecRight  = loada<VEC_TYPE>(sBuffer +  2*VectorSize);
}

template <int LEFT_NEEDED>
static FORCEINLINE void ApplyLeftBorder(int iLeftAvailable,
                                        Is16vec8 & vecLeft, Is16vec8 & vecCenter, Is16vec8 & vecRight)
{
    short _MM_ALIGN16 sBuffer[24];

    if (iLeftAvailable >= LEFT_NEEDED)
        return;

    storea(sBuffer     , vecLeft  );
    storea(sBuffer +  8, vecCenter);
    storea(sBuffer + 16, vecRight );

    for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - iLeftAvailable); iLeftOffset++)
    {
        sBuffer[8 - iLeftAvailable - iLeftOffset - 1] = sBuffer[8 - iLeftAvailable + iLeftOffset];
    }

    vecLeft   = loada<Is16vec8>(sBuffer     );
    vecCenter = loada<Is16vec8>(sBuffer +  8);
    vecRight  = loada<Is16vec8>(sBuffer + 16);
}

template <int RIGHT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyRightBorder(int iCount,
                                         int iRightAvailable,
                                         VEC_TYPE & vecLeft, VEC_TYPE & vecCenter, VEC_TYPE & vecRight)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    TYPE _MM_ALIGN16 sBuffer[3*VectorSize];

    if (iRightAvailable >= RIGHT_NEEDED)
        return;

    storea(sBuffer               , vecLeft  );
    storea(sBuffer +  VectorSize , vecCenter);
    storea(sBuffer + 2*VectorSize, vecRight );

    for (int iRight = iRightAvailable; iRight != RIGHT_NEEDED; iRight++)
    {
        sBuffer[iCount + iRight + VectorSize] = sBuffer[iCount + 2 * iRightAvailable - iRight - 1 + VectorSize];
    }

     vecLeft   = loada<VEC_TYPE>(sBuffer     );
     vecCenter = loada<VEC_TYPE>(sBuffer +   VectorSize);
     vecRight  = loada<VEC_TYPE>(sBuffer + 2*VectorSize);
}

template <int RIGHT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyRightBorder(int iCount,
                                         int iRightAvailable,
                                         typename VEC_TYPE::SCALAR_TYPE & vecLeft, VEC_TYPE & vecCenter, VEC_TYPE & vecRight)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    TYPE _MM_ALIGN16 sBuffer[3*VectorSize];

    if (iRightAvailable >= RIGHT_NEEDED)
        return;

    sBuffer[VectorSize-1] = vecLeft;
    storea(sBuffer +  VectorSize, vecCenter);
    storea(sBuffer + 2*VectorSize, vecRight );

    for (int iRight = iRightAvailable; iRight != RIGHT_NEEDED; iRight++)
    {
        sBuffer[iCount + iRight + VectorSize] = sBuffer[iCount + 2 * iRightAvailable - iRight - 1 + VectorSize];
    }

     vecLeft = sBuffer[VectorSize-1];
     vecCenter = loada<VEC_TYPE>(sBuffer +  VectorSize);
     vecRight  = loada<VEC_TYPE>(sBuffer + 2*VectorSize);
}

template <int RIGHT_NEEDED>
static FORCEINLINE void ApplyRightBorder(int iCount,
                                         int iRightAvailable,
                                         Is16vec8 & vecCenter, Is16vec8 & vecRight)
{
    short _MM_ALIGN16 sBuffer[16];

    if (iRightAvailable >= RIGHT_NEEDED)
        return;

    storea(sBuffer     , vecCenter);
    storea(sBuffer +  8, vecRight );

    for (int iRight = iRightAvailable; iRight != RIGHT_NEEDED; iRight++)
    {
        sBuffer[iCount + iRight] = sBuffer[iCount + 2 * iRightAvailable - iRight - 1];
    }

     vecCenter = loada<Is16vec8>(sBuffer);
     vecRight  = loada<Is16vec8>(sBuffer + 8);
}

template <int RIGHT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyRightBorder(int iCount,
                                         int iRightAvailable,
                                         VEC_TYPE & vecCenter, VEC_TYPE & vecRight)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    TYPE _MM_ALIGN16 sBuffer[16];

    if (iRightAvailable >= RIGHT_NEEDED)
        return;

    storea(sBuffer             , vecCenter);
    storea(sBuffer + VectorSize, vecRight );

    for (int iRight = iRightAvailable; iRight != RIGHT_NEEDED; iRight++)
    {
        sBuffer[iCount + iRight] = sBuffer[iCount + 2 * iRightAvailable - iRight - 1];
    }

     vecCenter = loada<VEC_TYPE>(sBuffer);
     vecRight  = loada<VEC_TYPE>(sBuffer + VectorSize);
}

            }

            using Impl::Image2D;
            using Impl::Region2D;
            using Impl::Region2D_Make;
            using Impl::Region2D_Rotate;
            using Impl::ClampIndex;
            using Impl::MirrorIndex;
        }
    }
}
