/*
* Copyright (c)2012-2014 Koninklijke Philips Electronics N.V.,
* All Rights Reserved.
*
* This source code and any compilation or derivative thereof is the
* proprietary information of Koninklijke Philips Electronics N.V.
* and is confidential in nature.
* Under no circumstances is this software to be combined with any Open Source
* Software in any way or placed under an Open Source License of any type 
* without the express written permission of Koninklijke Philips 
* Electronics N.V.
*
* Author: Emmanuel Attia - emmanuel.attia@philips.com - Philips Research Paris - Medisys
*
*/
#pragma once

#include "Platform.h"
#include "MetaMath.h"
#include <vector>

namespace
{
    namespace PVT_Common_Impl
    {
        using namespace std; 
        using namespace ::Philips::Medisys::PXVec;
        using namespace ::Philips::Medisys::Meta;

        enum Operation
        {
            Operation_LT = 0,
            Operation_LE = 1,
            Operation_EQ = 2,
            Operation_NE = 3,
            Operation_GT = 4,
            Operation_GE = 5,
            Operation_Not = 0x10,
        };

        template <int, typename T1, typename T2, typename T3>
        struct operator_applier_impl;

#define IMPLEMENT_OPERATOR_APPLIER(opcode, op) \
    template <typename T1, typename T2, typename T3> \
        struct operator_applier_impl<opcode, T1, T2, T3> { \
        FORCEINLINE operator_applier_impl(T1 & t1, T2 const & t2, T3 const & t3) { t1 = t2 op t3; } };

        IMPLEMENT_OPERATOR_APPLIER(Operation_LT, <)
        IMPLEMENT_OPERATOR_APPLIER(Operation_LE, <=)
        IMPLEMENT_OPERATOR_APPLIER(Operation_GT, >)
        IMPLEMENT_OPERATOR_APPLIER(Operation_GE, >=)
        IMPLEMENT_OPERATOR_APPLIER(Operation_EQ, ==)
        IMPLEMENT_OPERATOR_APPLIER(Operation_NE, !=)

#undef IMPLEMENT_OPERATOR_APPLIER

        template <int OP, typename T1, typename T2, typename T3>
        FORCEINLINE void operator_applier(T1 & t1, T2 const & t2, T3 const & t3)
        {
            enum {
                ACTUAL_OP = (OP & Operation_Not) ? (Operation_GE - OP) & (~Operation_Not)
                : OP & (~Operation_Not)
            };

            operator_applier_impl<ACTUAL_OP, T1, T2, T3>(t1, t2, t3);
        }

        template <int DIMENSIONS_COUNT, typename MASK_TYPE = uintptr_t>
        class BitBuffer
        {
        public:
            typedef MASK_TYPE MaskType;
        private:
            intptr_t strides[DIMENSIONS_COUNT];
            int dimensions[DIMENSIONS_COUNT];
            MaskType * buffer;
            enum {
                BitPerUnit = 8 * sizeof(MaskType),
                Log2BitPerUnit = Log2<BitPerUnit>::Value
            };
        public:
            BitBuffer(int const * _dimensions, bool clear = true)
            {
                for (int i = 0; i < DIMENSIONS_COUNT; i++)
                {
                    if (i == 0)
                        strides[0] = idiv_ceil<int>(_dimensions[0], BitPerUnit);
                    else
                        strides[i] = strides[i - 1] * _dimensions[i];
                    dimensions[i] = _dimensions[i];
                }

                buffer = (MaskType *)new MaskType[strides[DIMENSIONS_COUNT - 1]];
                if (clear) Clear();
            }

            ~BitBuffer()
            {
                delete [] buffer;
            }

            class PositionIterator : public iterator<forward_iterator_tag, int>
            {
                BitBuffer & parent;
                intptr_t indexBegin;
                intptr_t indexEnd;
                int x;
                MaskType mask;

            public:
                FORCEINLINE bool operator ==(PositionIterator & other)
                {
                    assert(&parent == &other.parent);
                    return (x == other.x);
                }

                FORCEINLINE bool operator !=(PositionIterator & other)
                {
                    assert(&parent == &other.parent);
                    return !((*this) == other);
                }

                FORCEINLINE PositionIterator(BitBuffer & parent, intptr_t indexBegin, intptr_t indexEnd)
                    : parent(parent), mask(0), indexBegin(indexBegin), indexEnd(indexEnd), x(-1)
                {
                    (*this).operator++();
                }

                FORCEINLINE PositionIterator & operator++()
                {
                    if (mask == 0)
                    {
                        int X = (x + BitPerUnit) >> Log2BitPerUnit;

                        for (;;)
                        {
                            if (indexBegin + X >= indexEnd)
                            {
                                x = -1;
                                return (*this);
                            }

                            mask = parent.buffer[indexBegin + X];
                            if (mask == 0)
                            {
                                X++;
                                continue;
                            }

                            break;
                        }

                        x = X * BitPerUnit - 1;
                    }

                    unsigned int dx;
                    bsf(dx, mask);
                    x += dx + 1;
                    mask >>= (MaskType)dx;
                    mask >>= 1;

                    return (*this);
                }

                FORCEINLINE PositionIterator operator++(int)
                {
                    PositionIterator result = *this;
                    ++(*this);
                    return result;
                }

                FORCEINLINE int operator * () const
                {
                    return x;
                }
            };

            FORCEINLINE PositionIterator BeginLine(intptr_t offset)
            {
                return PositionIterator(*this, offset, offset + strides[0]);
            }

            FORCEINLINE PositionIterator EndLine(intptr_t offset)
            {
                return PositionIterator(*this, offset + strides[0], offset + strides[0]);
            }

        public:
            FORCEINLINE void Clear()
            {
                PX_ZeroArray(buffer, strides[DIMENSIONS_COUNT - 1]);
            }

            template <int OPERATION, bool ALIGNED, typename TYPE>
            FORCEINLINE void PutLine(TYPE const * inputLine, TYPE comparand, intptr_t lineOffset)
            {
                typename typedef PXVEC_TYPE(TYPE) VectorType;

                struct PutLineVector
                {
                    FORCEINLINE PutLineVector(MASK_TYPE * dest, VectorType const & left, VectorType const & right, int & bit, intptr_t & index)
                    {
                        VectorType input;
                        operator_applier<OPERATION & (~Operation_Not)>(input, left, right);
                        MaskType mask = convert_to_mask(input);
                        if (OPERATION & Operation_Not) mask = ~mask;
                        mask &= ((1 << VectorType::SCALAR_COUNT) - 1);
                        dest[index] |= (mask << bit);
                        bit += VectorType::SCALAR_COUNT;
                        if (bit >= BitPerUnit)
                        {
                            bit -= BitPerUnit;
                            index++;
                        }
                    }
                };

                intptr_t index = lineOffset;
                int bit = 0;
                int x = 0;

                VectorType const compareVector = VectorType(comparand);

                for (; x <= dimensions[0] - VectorType::SCALAR_COUNT; x += VectorType::SCALAR_COUNT)
                {
                    VectorType input = load<ALIGNED, VectorType>(inputLine + x);
                    PutLineVector(buffer, input, compareVector, bit, index);
                }

                if (x < dimensions[0])
                {
                    VectorType input(0);
                    PX_CopyArray((TYPE *)&input, inputLine + x, dimensions[0] - x);
                    PutLineVector(buffer, input, compareVector, bit, index);
                }
            }

            FORCEINLINE void Unset(int x, intptr_t lineOffset = 0)
            {
                intptr_t index = (x >> Log2BitPerUnit) + lineOffset;
                int bit = x & (BitPerUnit - 1);
                bittestandreset(buffer[index], bit);
            }
            
            FORCEINLINE void Prefetch(int x, intptr_t lineOffset = 0) const
            {
                intptr_t index = (x >> Log2BitPerUnit) + lineOffset;
                prefetch(&buffer[index]);
            }

            FORCEINLINE void Set(int x, intptr_t lineOffset = 0)
            {
                intptr_t index = (x >> Log2BitPerUnit) + lineOffset;
                int bit = x & (BitPerUnit - 1);
                bittestandset(buffer[index], bit);
            }

            FORCEINLINE int Get(int x, intptr_t lineOffset = 0) const
            {
                intptr_t index = (x >> Log2BitPerUnit) + lineOffset;
                int bit = x & (BitPerUnit - 1);
                return bittest(buffer[index], bit);
            }

            FORCEINLINE intptr_t GetLineOffset2D(int y) const
            {
                return y * strides[0];
            }

            FORCEINLINE intptr_t GetLineOffset3D(int y, int z) const
            {
                return y * strides[0] + z * strides[1];
            }

            FORCEINLINE intptr_t GetLineOffset(int const * position) const
            {
                intptr_t result = 0;
                for (int i = 1; i < DIMENSIONS_COUNT; i++)
                    result += strides[i-1] * position[i];
                return result;
            }

            template <int MASK, int POS>
            FORCEINLINE void ReadNeighbors3D(int & result, intptr_t index, int bit)
            {
                result |= ((buffer[index - strides[0] - strides[1]] >> bit) & MASK) << (0 + POS);
                result |= ((buffer[index              - strides[1]] >> bit) & MASK) << (3 + POS);
                result |= ((buffer[index + strides[0] - strides[1]] >> bit) & MASK) << (6 + POS);
                result |= ((buffer[index - strides[0]             ] >> bit) & MASK) << (9 + POS);
                result |= ((buffer[index                          ] >> bit) & MASK) << (12 + POS);
                result |= ((buffer[index + strides[0]             ] >> bit) & MASK) << (15 + POS);
                result |= ((buffer[index - strides[0] + strides[1]] >> bit) & MASK) << (18 + POS);
                result |= ((buffer[index              + strides[1]] >> bit) & MASK) << (21 + POS);
                result |= ((buffer[index + strides[0] + strides[1]] >> bit) & MASK) << (24 + POS);
            }

            // count includes the center
            FORCEINLINE int GetNeighbors3D(int & count, int x, intptr_t lineOffset)
            {
                int result = 0;

                intptr_t index = (x >> Log2BitPerUnit) + lineOffset;
                int bit = x & (BitPerUnit - 1);

                ReadNeighbors3D<7, 0>(result, index, __max(0, bit - 1));

                if (bit == 0)
                {
                    result <<= 1;
                    result &= ~(1 | (1<<3) | (1<<6) | (1<<9) | (1<<12) | (1<<15) | (1<<18) | (1<<21) | (1<<24));
                    ReadNeighbors3D<1, 0>(result, index - 1, BitPerUnit - 1);
                }

                if (bit == (BitPerUnit - 1))
                {
                    result &= ~((1<<2) | (1<<5) | (1<<8) | (1<<11) | (1<<14) | (1<<17) | (1<<20) | (1<<23) | (1<<26));
                    ReadNeighbors3D<1, 2>(result, index + 1, 0);
                }

                count = popcount((uint32_t)result); // 32 bits is enough to hold 27-bit neighborhood

                return result;
            }
        };
    }
}

namespace Philips
{
    namespace Medisys
    {
        namespace Common
        {
            using PVT_Common_Impl::BitBuffer;
            using PVT_Common_Impl::Operation_LT;
            using PVT_Common_Impl::Operation_LE;
            using PVT_Common_Impl::Operation_EQ;
            using PVT_Common_Impl::Operation_NE;
            using PVT_Common_Impl::Operation_GT;
            using PVT_Common_Impl::Operation_GE;
            using PVT_Common_Impl::Operation_Not;
        }
    }
}
