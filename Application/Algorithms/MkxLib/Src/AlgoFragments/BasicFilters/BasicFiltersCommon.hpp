#pragma once

#include <Platform.h>
#include <MetaUnroller.h>

#include <cmath>
#include <cassert>
#include <limits>

#define CONCAT_X(t1, t2)t1##t2
#define CONCAT2(t1, t2)CONCAT_X(t1, t2)

#define AlgoFragmentsImpl CONCAT2(AlgoFragmentsImpl, PLATFORM_INTEL_SSE)

namespace // Private namespace to this compiland
{
    namespace AlgoFragmentsImpl
    {

using namespace ::std;
using namespace ::Philips::Medisys::PXVec;
using namespace ::Philips::Medisys::Meta;

FORCEINLINE int MirrorIndex(int current, int left, int right)
{
    //int iLength = iMax - iMin;
    //int iPeriodisedCurrent = ((iCurrent - iMin) % (2*iLength) + (2*iLength)) % (2*iLength);
    //return iMin + __min(iLength - 1, __min(iPeriodisedCurrent, 2*iLength - 1 - iPeriodisedCurrent));

    if (EXPECT(current < left, false))
        current = left - current - 1;
    if (EXPECT(current >= right, false))
        current = 2 * right - current - 1;
    return current;
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

template <typename REGION>
static inline REGION Region2D_CounterRotate(REGION const & input)
{
    REGION result;
    result.left   = 1-input.bottom;
    result.top    = input.left;
    result.right  = 1-input.top;
    result.bottom = input.right;
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

    FORCEINLINE bool IsAligned(int x, int iPadding = PLATFORM_DEFAULT_ALIGN) const
    {
        return ((LineStride % iPadding) == 0)
            && (((((BufferAtOrigin + x) - (TYPE *)NULL) * sizeof(TYPE)) % iPadding) == 0);
    }
};

struct Region3D
{
    int firstX;
    int firstY;
    int firstZ;
    int lastX;
    int lastY;
    int lastZ;
};

template <typename TYPE>
struct Image3D
{
    typedef TYPE Type;
    TYPE * BufferAtOrigin;
    int StrideY;
    int StrideYZ;

    FORCEINLINE TYPE * PointerAt(int x, int y, int z) const
    {
        return BufferAtOrigin + StrideYZ * z + StrideY * y + x;
    }

    FORCEINLINE bool IsAligned(int x, int iPadding = PLATFORM_DEFAULT_ALIGN) const
    {
        return ((StrideY % iPadding) == 0)
            && (((((BufferAtOrigin + x) - (TYPE *)NULL) * sizeof(TYPE)) % iPadding) == 0);
    }
};

template <typename TYPE>
struct Image3D_Planes
{
    typedef TYPE Type;
    TYPE ** Planes;
    bool PlanesAligned;
    int StrideY;

    FORCEINLINE TYPE * PointerAt(int x, int y, int z) const
    {
        return Planes[z] + StrideY * y + x;
    }

    FORCEINLINE bool IsAligned(int x, int iPadding = PLATFORM_DEFAULT_ALIGN) const
    {
        return PlanesAligned && ((StrideY % iPadding) == 0) && ((x % iPadding) == 0);
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

    FORCEINLINE TYPE * PointerAt(int x, int y) const
    {
        TYPE * result = BufferAtOrigin + x - Left + (((y - Top) + LineCount) % LineCount) * LineStride;
        return result;
    }

    FORCEINLINE TYPE * SingleLinePointerAt(int x, int y) const
    {
        assert(LineCount == 1);
        return BufferAtOrigin + x - Left;
    }

    //inline bool IsAligned(int x, int iPadding = PLATFORM_DEFAULT_ALIGN) const
    //{
    //    return ((LineStride % iPadding) == 0)
    //        && (((((BufferAtOrigin + x) - (TYPE *)NULL) * sizeof(TYPE)) % iPadding) == 0);
    //}
};

template <int LINE_COUNT, typename TYPE>
struct FixedLinesCircularBuffer
{
    typedef TYPE Type;
    
    enum { LineCount = LINE_COUNT };
    int LineStride;
    int Left;
    int Top;
    TYPE * BufferAtOrigin;

    FORCEINLINE TYPE * PointerAt(int x, int y) const
    {
        TYPE * result = BufferAtOrigin + x - Left + (((y - Top) + LineCount) % LineCount) * LineStride;
        return result;
    }

    template <typename T>
    FORCEINLINE void IncrementY(T& result) const
    {
        result += LineStride;
        if (EXPECT(result + LineStride > BufferAtOrigin + LineCount * LineStride, false))
            result -= LineCount * LineStride;
    }

    FORCEINLINE TYPE * SingleLinePointerAt(int x, int y) const
    {
        assert(LineCount == 1);
        return BufferAtOrigin + x - Left;
    }

    //inline bool IsAligned(int x, int iPadding = PLATFORM_DEFAULT_ALIGN) const
    //{
    //    return ((LineStride % iPadding) == 0)
    //        && (((((BufferAtOrigin + x) - (TYPE *)NULL) * sizeof(TYPE)) % iPadding) == 0);
    //}
};

// worstCase is useful to implement GetRequiredPoolSize functions
template <typename C>
FORCEINLINE void AddCircularBufferToPoolEx(C & circularBuffer, void *& pool, int left, int right, int alignAt, int lineCount, int vectorSize, bool worstCase = false)
{
    ((int8_t *&)pool) += PLATFORM_DEFAULT_ALIGN;

    intptr_t poolBytes = (intptr_t)((int8_t *)pool - (int8_t *)NULL);
    int misAlign = poolBytes + ((alignAt - left) * sizeof(typename C::Type)) & (PLATFORM_DEFAULT_ALIGN - 1);
    if (misAlign > 0)
        misAlign = PLATFORM_DEFAULT_ALIGN - misAlign;

    if (worstCase)
        misAlign += PLATFORM_DEFAULT_ALIGN;

    circularBuffer.BufferAtOrigin = (typename C::Type *)(poolBytes + misAlign);
    circularBuffer.LineStride     = idiv_ceil<int>(right - left, vectorSize) * vectorSize;
    circularBuffer.Left           = left;
    circularBuffer.Top            = 0;

    // This is specific to Intel to prevent 4K-aliasing
    if ((popcount((uint32_t)circularBuffer.LineStride) == 1) || worstCase)
    {
        circularBuffer.LineStride += 3 * vectorSize;
    }

    ((int8_t *&)pool) += PLATFORM_DEFAULT_ALIGN + lineCount * circularBuffer.LineStride * sizeof(circularBuffer.BufferAtOrigin[0]);
}

template <int LINE_COUNT, typename TYPE>
FORCEINLINE void AddCircularBufferToPoolEx(FixedLinesCircularBuffer<LINE_COUNT, TYPE> & circularBuffer, void *& pool, int left, int right, int alignAt, int vectorSize, bool worstCase = false)
{
    AddCircularBufferToPoolEx<FixedLinesCircularBuffer<LINE_COUNT, TYPE> >(circularBuffer, pool, left, right, alignAt, LINE_COUNT, vectorSize, worstCase);
}

template <typename TYPE>
FORCEINLINE void AddCircularBufferToPoolEx(CircularBuffer<TYPE> & circularBuffer, void *& pool, int left, int right, int alignAt, int lineCount, int vectorSize, bool worstCase = false)
{
    circularBuffer.LineCount      = lineCount;
    AddCircularBufferToPoolEx<CircularBuffer<TYPE> >(circularBuffer, pool, left, right, alignAt, lineCount, vectorSize, worstCase);
}


template <int LINE_COUNT, typename TYPE>
FORCEINLINE void AddCircularBufferToPool(FixedLinesCircularBuffer<LINE_COUNT, TYPE> & circularBuffer, void *& pool, int left, int right, int vectorSize, bool worstCase = false)
{
    AddCircularBufferToPoolEx<FixedLinesCircularBuffer<LINE_COUNT, TYPE> >(circularBuffer, pool, left, right, /* align at */ left, LINE_COUNT, vectorSize, worstCase);
}

template <typename TYPE>
FORCEINLINE void AddCircularBufferToPool(CircularBuffer<TYPE> & circularBuffer, void *& pool, int left, int right, int lineCount, int vectorSize, bool worstCase = false)
{
    circularBuffer.LineCount      = lineCount;
    AddCircularBufferToPoolEx<CircularBuffer<TYPE> >(circularBuffer, pool, left, right, /* align at */ left, lineCount, vectorSize, worstCase);
}

template <typename C>
FORCEINLINE void AddBufferToPool(C & circularBuffer, void *& pool, int left, int right, int top, int bottom, int vectorSize)
{
    ((int8_t *&)pool) += PLATFORM_DEFAULT_ALIGN;

    circularBuffer.LineStride     = idiv_ceil<int>(right - left, vectorSize) * vectorSize;
    circularBuffer.BufferAtOrigin = ((typename C::Type *)pool);
    circularBuffer.LineCount      = bottom - top;
    circularBuffer.Left           = left;
    circularBuffer.Top            = top;

    ((int8_t *&)pool) += PLATFORM_DEFAULT_ALIGN + circularBuffer.LineCount * circularBuffer.LineStride * sizeof(circularBuffer.BufferAtOrigin[0]);
}

template <typename TYPE>
FORCEINLINE size_t RequiredBufferPoolSize(int maxWidth, int lineCount, int vectorSize)
{
    size_t lineStride = idiv_ceil<int>(maxWidth, vectorSize) * vectorSize;
    return 2 * vectorSize + lineCount * lineStride * sizeof(TYPE);
}

template <int LEFT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyLeftBorder(int leftAvailable,
                                        VEC_TYPE & left, VEC_TYPE & center)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    if (EXPECT(leftAvailable < LEFT_NEEDED, false))
    {
        if (EXPECT(leftAvailable == 0, true))
        {
            left = reverse(center);
        }
        else
        {
            PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) TYPE sBuffer[2*VectorSize];

            storea(sBuffer     , left  );
            storea(sBuffer + VectorSize, center);

            for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - leftAvailable); iLeftOffset++)
            {
                sBuffer[VectorSize - leftAvailable - iLeftOffset - 1] = sBuffer[VectorSize - leftAvailable + iLeftOffset];
            }

            left   = loada<VEC_TYPE>(sBuffer    );
            center = loada<VEC_TYPE>(sBuffer + VectorSize);
        }
    }
}

template <int LEFT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyLeftBorder(int leftAvailable,
                                        typename VEC_TYPE::SCALAR_TYPE & sLeft, VEC_TYPE & center)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) TYPE sBuffer[2*VectorSize];

    if (leftAvailable >= LEFT_NEEDED)
        return;

    sBuffer[VectorSize-1] = sLeft;
    storea(sBuffer +  VectorSize, center);

    for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - leftAvailable); iLeftOffset++)
    {
        sBuffer[VectorSize - leftAvailable - iLeftOffset - 1] = sBuffer[VectorSize - leftAvailable + iLeftOffset];
    }

     sLeft = sBuffer[VectorSize-1];
     center = loada<VEC_TYPE>(sBuffer +  VectorSize);
}

template <int LEFT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyLeftBorder(int leftAvailable,
                                        typename VEC_TYPE::SCALAR_TYPE & sLeft, VEC_TYPE & center, VEC_TYPE & right)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) TYPE sBuffer[3*VectorSize];

    if (leftAvailable >= LEFT_NEEDED)
        return;

    sBuffer[VectorSize-1] = sLeft;
    storea(sBuffer +  VectorSize, center);
    storea(sBuffer +  2*VectorSize, right );

    for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - leftAvailable); iLeftOffset++)
    {
        sBuffer[VectorSize - leftAvailable - iLeftOffset - 1] = sBuffer[VectorSize - leftAvailable + iLeftOffset];
    }

     sLeft = sBuffer[VectorSize-1];
     center = loada<VEC_TYPE>(sBuffer +  VectorSize);
     right  = loada<VEC_TYPE>(sBuffer +  2*VectorSize);
}

template <int LEFT_NEEDED>
static FORCEINLINE void ApplyLeftBorder(int leftAvailable,
                                        Is16vec8 & left, Is16vec8 & center, Is16vec8 & right)
{
    if (EXPECT(leftAvailable < LEFT_NEEDED, false))
    {
        if (EXPECT(leftAvailable == 0, true))
        {
            left = reverse(center);
        }
        else
        {
            short _MM_ALIGN16 sBuffer[24];

            storea(sBuffer     , left  );
            storea(sBuffer +  8, center);
            storea(sBuffer + 16, right );

            for (int iLeftOffset = 0; iLeftOffset != (LEFT_NEEDED - leftAvailable); iLeftOffset++)
            {
                sBuffer[8 - leftAvailable - iLeftOffset - 1] = sBuffer[8 - leftAvailable + iLeftOffset];
            }

            left   = loada<Is16vec8>(sBuffer     );
            center = loada<Is16vec8>(sBuffer +  8);
            right  = loada<Is16vec8>(sBuffer + 16);
        }
    }
}

template <int RIGHT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyRightBorder(int count,
                                         int rightAvailable,
                                         VEC_TYPE & left, VEC_TYPE & center, VEC_TYPE & right)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    if (EXPECT(rightAvailable < RIGHT_NEEDED, false))
    {
        if (EXPECT(rightAvailable == 0 && count == VectorSize, true))
        {
            right = reverse(center);
        }
        else
        {
            PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) TYPE sBuffer[3*VectorSize];

            storea(sBuffer               , left  );
            storea(sBuffer +  VectorSize , center);
            storea(sBuffer + 2*VectorSize, right );

            for (int iRight = rightAvailable; iRight != RIGHT_NEEDED; iRight++)
            {
                sBuffer[count + iRight + VectorSize] = sBuffer[count + 2 * rightAvailable - iRight - 1 + VectorSize];
            }

             left   = loada<VEC_TYPE>(sBuffer     );
             center = loada<VEC_TYPE>(sBuffer +   VectorSize);
             right  = loada<VEC_TYPE>(sBuffer + 2*VectorSize);
        }
    }
}

template <int RIGHT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyRightBorder(int count,
                                         int rightAvailable,
                                         typename VEC_TYPE::SCALAR_TYPE & left, VEC_TYPE & center, VEC_TYPE & right)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) TYPE sBuffer[3*VectorSize];

    if (rightAvailable >= RIGHT_NEEDED)
        return;

    sBuffer[VectorSize-1] = left;
    storea(sBuffer +  VectorSize, center);
    storea(sBuffer + 2*VectorSize, right );

    for (int iRight = rightAvailable; iRight != RIGHT_NEEDED; iRight++)
    {
        sBuffer[count + iRight + VectorSize] = sBuffer[count + 2 * rightAvailable - iRight - 1 + VectorSize];
    }

     left = sBuffer[VectorSize-1];
     center = loada<VEC_TYPE>(sBuffer +  VectorSize);
     right  = loada<VEC_TYPE>(sBuffer + 2*VectorSize);
}

template <int RIGHT_NEEDED>
static FORCEINLINE void ApplyRightBorder(int count,
                                         int rightAvailable,
                                         Is16vec8 & center, Is16vec8 & right)
{
    short _MM_ALIGN16 sBuffer[16];

    if (rightAvailable >= RIGHT_NEEDED)
        return;

    storea(sBuffer     , center);
    storea(sBuffer +  8, right );

    for (int iRight = rightAvailable; iRight != RIGHT_NEEDED; iRight++)
    {
        sBuffer[count + iRight] = sBuffer[count + 2 * rightAvailable - iRight - 1];
    }

     center = loada<Is16vec8>(sBuffer);
     right  = loada<Is16vec8>(sBuffer + 8);
}

template <int RIGHT_NEEDED, typename VEC_TYPE>
static FORCEINLINE void ApplyRightBorder(int count,
                                         int rightAvailable,
                                         VEC_TYPE & center, VEC_TYPE & right)
{
    typedef typename VEC_TYPE::SCALAR_TYPE TYPE;
    enum { VectorSize = VEC_TYPE::SCALAR_COUNT };

    if (EXPECT(rightAvailable < RIGHT_NEEDED, false))
    {
        if (EXPECT(rightAvailable == 0 && count == VectorSize, true))
        {
            right = reverse(center);
        }
        else
        {
            PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) TYPE sBuffer[16];

            storea(sBuffer             , center);
            storea(sBuffer + VectorSize, right );

            for (int iRight = rightAvailable; iRight != RIGHT_NEEDED; iRight++)
            {
                sBuffer[count + iRight] = sBuffer[count + 2 * rightAvailable - iRight - 1];
            }

             center = loada<VEC_TYPE>(sBuffer);
             right  = loada<VEC_TYPE>(sBuffer + VectorSize);
        }
    }
}
    }
}

namespace Philips
{
    namespace Medisys
    {
        namespace AlgoFragments
        {
            using ::AlgoFragmentsImpl::Image2D;
            using ::AlgoFragmentsImpl::Image3D;
            using ::AlgoFragmentsImpl::Image3D_Planes;
            using ::AlgoFragmentsImpl::Region3D;
            using ::AlgoFragmentsImpl::Region2D;
            using ::AlgoFragmentsImpl::Region2D_Make;
            using ::AlgoFragmentsImpl::Region2D_Rotate;
            using ::AlgoFragmentsImpl::Region2D_CounterRotate;
            using ::AlgoFragmentsImpl::ClampIndex;
            using ::AlgoFragmentsImpl::MirrorIndex;
            using ::AlgoFragmentsImpl::ApplyLeftBorder;
            using ::AlgoFragmentsImpl::ApplyRightBorder;
            using ::AlgoFragmentsImpl::CircularBuffer;
            using ::AlgoFragmentsImpl::FixedLinesCircularBuffer;
            using ::AlgoFragmentsImpl::RequiredBufferPoolSize;
        }
    }
}
