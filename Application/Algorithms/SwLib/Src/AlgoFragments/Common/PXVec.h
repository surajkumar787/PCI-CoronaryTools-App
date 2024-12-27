/*
 * Copyright (c)2009-2016 Koninklijke Philips Electronics N.V.,
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

//TICS -*
#pragma once

/*
 * Note about conversion from / to native SSE/AVX types.
 *
 * To avoid ambiguous conversion between types.
 *
 * Every class should implement operator ONLY to its native type.
 * ie: Is32vec4/Is16vec8/... => __m128i
 *     F32vec4 => __m128
 *     F32vec8 (AVX) => __m256
 *     F32vec8 (SSE) => __m128[2]
 *     Is32vec8 (SSE) => __m128i[2]
 *
 * Every class should implement constructor for any convertible type:
 * ie: Is32vec4/F32vec4 => constructs from __m128, __m128i, __m128d
 *     F32vec8 => constructs from __m128[2], __m128i[2], __m256, __m256i, etc...
 *
 * To pass a vector class to a native intrinsic, cast it first to the corresponding vector class.
 * ie: 
 *      Is32vec8 mask;
 *      F32vec8 value;
 *      Is32vec8 result = Is32vec8(_mm256_and_ps(F32vec8(mask), value));
 *
 */

namespace // Private namespace to this compiland
{
    namespace PXVecImpl
    {

// Forward definitions
template <typename HALF_TYPE>
struct Vec2x;

template <typename DOUBLE_TYPE>
struct FakeHalf;

struct I128vec1;

struct F64vec4;
struct F64vec2;

struct F32vec4;
struct F32vec8;

struct Iu64vec2;
struct Iu64vec4;

struct Is64vec2;
struct Is64vec4;

struct Iu32vec4;
struct Iu32vec8;

struct Is32vec4;
struct Is32vec8;

struct Iu16vec8;
struct Iu16vec16;

struct Is16vec8;
struct Is16vec16;

struct Iu8vec16;
struct Iu8vec32;

struct Is8vec16;
struct Is8vec32;

// Types defined here is not exhaustive
typedef FakeHalf<Is16vec8> Is16vec4;

typedef Vec2x<F32vec8> F32vec16;
typedef Vec2x<Is32vec8> Is32vec16;
typedef Vec2x<F64vec4> F64vec8;
typedef Vec2x<Is64vec4> Is64vec8;
typedef Vec2x<Iu64vec4> Iu64vec8;
typedef Vec2x<Iu32vec8> Iu32vec16;
typedef Vec2x<Is16vec16> Is16vec32;
typedef Vec2x<Iu32vec16> Iu32vec32;

#include "PXVec.macros.inl"
#include "PXVec.classes.inl"
#include "PXVec.typesel.inl"

// Used to do: vec = other.vec, no matter if vec is __m128i, __m128i[2], __m128i[2][2], etc...

template <typename HALF_TYPE>
FORCEINLINE void copy_native_vec_(HALF_TYPE(&out)[2], HALF_TYPE const (&in)[2], $I<1>)
{
    copy_native_vec(out[0], in[0]);
    copy_native_vec(out[1], in[1]);
}

template <typename TYPE>
FORCEINLINE void copy_native_vec_(TYPE & out, TYPE const & in, $I<0>)
{
    out = in;
}

template <typename TYPE>
FORCEINLINE void copy_native_vec(TYPE & out, TYPE const & in)
{
    copy_native_vec_(out, in, typename PX_IsArray<TYPE>::Type());
}


/////////////////////////////////////////////////////
/// Macros for type selection (PXVEC_TYPE etc...)
/////////////////////////////////////////////////////


FORCEINLINE Is8vec16 sat_pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_packs_epi16(a, b); }
FORCEINLINE Is8vec16 sat_pack(Is16vec16 const & a) { return _mm_packs_epi16(a.half(0), a.half(1)); }
FORCEINLINE Iu8vec16 usat_pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_packus_epi16(a, b); }
FORCEINLINE Iu8vec16 usat_pack(Is16vec16 const & a) { return _mm_packus_epi16(a.half(0), a.half(1)); }
FORCEINLINE Is16vec8 sat_pack(Is32vec8 const & a) { return _mm_packs_epi32(a.half(0), a.half(1)); }
FORCEINLINE Is16vec8 sat_pack(Is32vec4 const & a, Is32vec4 const & b) { return _mm_packs_epi32(a, b); }

#if PLATFORM_INTEL_SSE >= 60

FORCEINLINE Is32vec8 mul_add(Is16vec16 const & a, Is16vec16 const & b) { return _mm256_madd_epi16(a, b); }
FORCEINLINE Is16vec16 sat_pack_halves(Is32vec8 const & a, Is32vec8 const & b) { return _mm256_packs_epi32(a, b); }
FORCEINLINE Is16vec16 sat_pack(Is32vec8 const & a, Is32vec8 const & b) { return _mm256_permute4x64_epi64(_mm256_packs_epi32(a, b), _MM_SHUFFLE(3, 1, 2, 0)); }

#else

FORCEINLINE Is32vec8 mul_add(Is16vec16 const & a, Is16vec16 const & b) { return Is32vec8(mul_add(a.half(0), b.half(0)), mul_add(a.half(1), b.half(1))); }
FORCEINLINE Is16vec16 sat_pack_halves(Is32vec8 const & a, Is32vec8 const & b) { return Is16vec16(_mm_packs_epi32(a.half(0), b.half(0)), _mm_packs_epi32(a.half(1), b.half(1))); }
FORCEINLINE Is16vec16 sat_pack(Is32vec8 const & a, Is32vec8 const & b) { return Is16vec16(_mm_packs_epi32(a.half(0), a.half(1)), _mm_packs_epi32(b.half(0), b.half(1))); }

#endif

#if PLATFORM_INTEL_SSE >= 40
FORCEINLINE Iu16vec8 usat_pack(Is32vec8 const & a) { return _mm_packus_epi32(a.half(0), a.half(1)); }
FORCEINLINE Iu16vec8 usat_pack(Is32vec4 const & a, Is32vec4 const & b) { return _mm_packus_epi32(a, b); }
FORCEINLINE Iu16vec8 usat_pack(Iu32vec4 const & a, Iu32vec4 const & b) { return _mm_packus_epi32(a, b); }
#endif

template <typename OUT_LO>
struct impl_pack
{
    template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
    static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b)
    {
        return OUT_VEC_TYPE(pack<typename OUT_VEC_TYPE::HALF_TYPE>(a), pack<typename OUT_VEC_TYPE::HALF_TYPE>(b));
    }

    template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
    static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a)
    {
        return pack<OUT_VEC_TYPE>(a.half(0), a.half(1));
    }

    template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
    static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b)
    {
        return OUT_VEC_TYPE(odd_pack<typename OUT_VEC_TYPE::HALF_TYPE>(a), odd_pack<typename OUT_VEC_TYPE::HALF_TYPE>(b));
    }

    template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
    static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a)
    {
        return odd_pack<OUT_VEC_TYPE>(a.half(0), a.half(1));
    }
};

#define PXVEC_IMPLEMENT_PACK(OUT_VEC, IN_VEC, INTRIN, ODD_INTRIN) \
    template <> struct impl_pack<OUT_VEC::LO> { \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b) { return OUT_VEC_TYPE(INTRIN(IN_VEC(a), IN_VEC(b))); } \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b) { return OUT_VEC_TYPE(ODD_INTRIN(IN_VEC(a), IN_VEC(b))); } \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a) { return impl_pack<void>::go<OUT_VEC_TYPE>(a); } \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a) { return impl_pack<void>::go_odd<OUT_VEC_TYPE>(a); } };

#define PXVEC_IMPLEMENT_PACK2(OUT_VEC, IN_VEC, INTRIN, ODD_INTRIN, INTRIN2, ODD_INTRIN2) \
    template <> struct impl_pack<OUT_VEC::LO> { \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b) { return OUT_VEC_TYPE(INTRIN(IN_VEC(a), IN_VEC(b))); } \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b) { return OUT_VEC_TYPE(ODD_INTRIN(IN_VEC(a), IN_VEC(b))); } \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a) { return OUT_VEC_TYPE(INTRIN2(IN_VEC::DOUBLE_TYPE(a))); } \
        template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a) { return OUT_VEC_TYPE(ODD_INTRIN2(IN_VEC::DOUBLE_TYPE(a))); } };

#if PLATFORM_INTEL_SSE < 60

PXVEC_IMPLEMENT_PACK(Is8vec16, Is16vec8, _mm_pack_epi16, _mm_oddpack_epi16)
PXVEC_IMPLEMENT_PACK(Is16vec8, Is32vec4, _mm_pack_epi32, _mm_oddpack_epi32)

#else

PXVEC_IMPLEMENT_PACK2(Is8vec16, Is16vec8, _mm_pack_epi16, _mm_oddpack_epi16, _mm256_pack_epi16_si128, _mm256_oddpack_epi16_si128)
PXVEC_IMPLEMENT_PACK2(Is16vec8, Is32vec4, _mm_pack_epi32, _mm_oddpack_epi32, _mm256_pack_epi32_si128, _mm256_oddpack_epi32_si128)

#endif

PXVEC_IMPLEMENT_PACK(Is32vec4, Is64vec2, _mm_pack_epi64, _mm_oddpack_epi64)

#if PLATFORM_INTEL_SSE >= 60

PXVEC_IMPLEMENT_PACK(Is8vec32, Is16vec16, _mm256_pack_epi16, _mm256_oddpack_epi16)
PXVEC_IMPLEMENT_PACK(Is16vec16, Is32vec8, _mm256_pack_epi32, _mm256_oddpack_epi32)

#endif

#if PLATFORM_INTEL_SSE >= 50

PXVEC_IMPLEMENT_PACK(Is32vec8, Is64vec4 , _mm256_pack_epi64, _mm256_oddpack_epi64)

#endif

template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
FORCEINLINE OUT_VEC_TYPE pack(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b)
{
    return impl_pack<typename OUT_VEC_TYPE::LO>::go<OUT_VEC_TYPE>(a, b);
}

template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
FORCEINLINE OUT_VEC_TYPE pack(IN_VEC_TYPE const & a)
{
    return impl_pack<typename OUT_VEC_TYPE::LO>::go<OUT_VEC_TYPE>(a);
}

template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
FORCEINLINE OUT_VEC_TYPE odd_pack(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b)
{
    return impl_pack<typename OUT_VEC_TYPE::LO>::go_odd<OUT_VEC_TYPE>(a, b);
}

template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE>
FORCEINLINE OUT_VEC_TYPE odd_pack(IN_VEC_TYPE const & a)
{
    return impl_pack<typename OUT_VEC_TYPE::LO>::go_odd<OUT_VEC_TYPE>(a);
}

#define PXVEC_IMPLEMENT_2PACK(OUT_VEC, IN_VEC) \
template <> struct impl_pack<OUT_VEC::LO> { \
    template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b) { return OUT(pack<OUT::HALF_TYPE>(a.half(0), a.half(1)), pack<OUT::HALF_TYPE>(b.half(0), b.half(1))); } \
    template <typename OUT_VEC_TYPE, typename IN_VEC_TYPE> static FORCEINLINE OUT_VEC_TYPE go_odd(IN_VEC_TYPE const & a, IN_VEC_TYPE const & b){ return OUT(pack<OUT::HALF_TYPE>(a.half(0), a.half(1)), odd_pack<OUT::HALF_TYPE>(b.half(0), b.half(1))); } };

#if PLATFORM_INTEL_SSE < 60

PXVEC_IMPLEMENT_2PACK(Is16vec16, Is32vec8)
PXVEC_IMPLEMENT_2PACK(Is8vec32, Is16vec16)

#endif

template <typename VEC_LO>
struct impl_uninterleave
{
    template <typename IN_VEC_TYPE>
    static FORCEINLINE IN_VEC_TYPE go(IN_VEC_TYPE const & src)
    {
        typedef PXVEC_TYPE_FORELEMSIZE(2 * sizeof(typename IN_VEC_TYPE::SCALAR_TYPE), IN_VEC_TYPE::SCALAR_COUNT / 2) InDblType;
        typedef typename IN_VEC_TYPE::HALF_TYPE HType; // TODO: with C++11 replace this by typedef decltype(src.half(0)) HVType
        return IN_VEC_TYPE(pack<HType>(InDblType(src)), odd_pack<HType>(InDblType(src)));
    }
};

#if PLATFORM_INTEL_SSE < 50
/*
template <>
struct impl_uninterleave<F32vec8::LO>
{
    template <typename IN_VEC_TYPE>
    static FORCEINLINE IN_VEC_TYPE go(IN_VEC_TYPE const & src)
    {
        return unpack_low(src.half(0), src.half(1));
    }
};
*/
#else

template <>
struct impl_uninterleave<F32vec16::LO>
{
    template <typename IN_VEC_TYPE>
    static FORCEINLINE IN_VEC_TYPE go(IN_VEC_TYPE const & src)
    {
        __m256 a = F32vec8(src.half(0));
        __m256 b = F32vec8(src.half(1));

        __m256 a1b0 = _mm256_permute2f128_ps(a, b, _MM_SHUFFLE(0, 2, 0, 1));
        __m256 a0b0 = _mm256_blend_ps(a1b0, a, (1 << 4) - 1);
        __m256 a1b1 = _mm256_blend_ps(b, a1b0, (1 << 4) - 1);

        return IN_VEC_TYPE(_mm256_shuffle_ps(a0b0, a1b1, _MM_SHUFFLE(2, 0, 2, 0)),
                           _mm256_shuffle_ps(a0b0, a1b1, _MM_SHUFFLE(3, 1, 3, 1)));
    }
};

#endif

template <>
struct impl_uninterleave<Is16vec8::LO>
{
    template <typename IN_VEC_TYPE>
    static FORCEINLINE IN_VEC_TYPE go(IN_VEC_TYPE const & src)
    {
        //return unpack_low(src, shiftreg_right<IN_VEC_TYPE::SCALAR_COUNT / 2>(src));
        return shuffle<0, 2, 4, 6, 1, 3, 5, 7>(src);
    }
};

template <typename IN_VEC_TYPE>
FORCEINLINE IN_VEC_TYPE uninterleave(IN_VEC_TYPE const & src)
{
    return impl_uninterleave<typename IN_VEC_TYPE::LO>::go(src);
}

FORCEINLINE Is32vec8 mul(Is16vec8 const & a, Is16vec8 const & b)
{
    Is16vec8 lo = _mm_mullo_epi16(a, b), hi = _mm_mulhi_epi16(a, b);
    return Is32vec8(_mm_unpacklo_epi16(lo, hi), _mm_unpackhi_epi16(lo, hi));
}

template <typename TYPE>
FORCEINLINE TYPE fast_sqrt(TYPE const & a)
{
    return a * rsqrt(a);
}

template <typename TYPE>
FORCEINLINE void impl_loada(Vec2x<TYPE> & res, typename TYPE::SCALAR_TYPE const * ptr)
{
    return Vec2x<TYPE>(loada<typename TYPE::HALF_TYPE>(ptr), loada<typename TYPE::HALF_TYPE>(ptr + TYPE::SCALAR_COUNT));
}

template <typename TYPE>
FORCEINLINE void impl_loadu(Vec2x<TYPE> & res, typename TYPE::SCALAR_TYPE const * ptr)
{
    return Vec2x<TYPE>(loadu<typename TYPE::HALF_TYPE>(ptr), loadu<typename TYPE::HALF_TYPE>(ptr + TYPE::SCALAR_COUNT));
}

template <typename TYPE>
FORCEINLINE void impl_lddqu(Vec2x<TYPE> & res, typename TYPE::SCALAR_TYPE const * ptr)
{
    return Vec2x<TYPE>(lddqu<typename TYPE::HALF_TYPE>(ptr), lddqu<typename TYPE::HALF_TYPE>(ptr + TYPE::SCALAR_COUNT));
}

template <typename TYPE>
FORCEINLINE void storea(typename TYPE::SCALAR_TYPE * ptr, Vec2x<TYPE> const & a)
{
    storea(ptr, a.half(0));
    storea(ptr + TYPE::SCALAR_COUNT, a.half(1));
}

template <typename TYPE>
FORCEINLINE void storeu(typename TYPE::SCALAR_TYPE * ptr, Vec2x<TYPE> const & a)
{
    storeu(ptr, a.half(0));
    storeu(ptr + TYPE::SCALAR_COUNT, a.half(1));
}

template <typename TYPE>
FORCEINLINE void streama(typename TYPE::SCALAR_TYPE * ptr, Vec2x<TYPE> const & a)
{
    streama(ptr, a.half(0));
    streama(ptr + TYPE::SCALAR_COUNT, a.half(1));
}

template <typename TYPE>
FORCEINLINE TYPE loadu(typename TYPE::SCALAR_TYPE const * ptr)
{
    TYPE res;
    impl_loadu(res, ptr);
    return res;
}

template <typename TYPE>
FORCEINLINE TYPE lddqu(typename TYPE::SCALAR_TYPE const * ptr)
{
    TYPE res;
    impl_lddqu(res, ptr);
    return res;
}

template <typename TYPE>
FORCEINLINE TYPE loada(typename TYPE::SCALAR_TYPE const * ptr)
{
    TYPE res;
    impl_loada(res, ptr);
    return res;
}

template <typename TYPE>
FORCEINLINE TYPE load_low(typename TYPE::SCALAR_TYPE const * ptr)
{
    TYPE res;
    impl_load_low(res, ptr);
    return res;
}

template <typename TYPE>
TYPE load_scalar(typename TYPE::SCALAR_TYPE const & ptr);

template <typename TYPE>
FORCEINLINE TYPE load_partial_zero(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    PLATFORM_ALIGNED_DEF typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT] = {};
    for (int i = 0; i < count; i++)
        tmp[i] = ptr[i];
    return loada<TYPE>(tmp);
}

template <typename TYPE>
FORCEINLINE TYPE load_partial(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    PLATFORM_ALIGNED_DEF typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
    for (int i = 0; i < count; i++)
        tmp[i] = ptr[i];
    return loada<TYPE>(tmp);
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE TYPE load(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    if (count == TYPE::SCALAR_COUNT)
    {
        return load<ALIGNED, TYPE>(ptr);
    }
    else
    {
        return load_partial<TYPE>(ptr, count);
    }
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE TYPE load(typename TYPE::SCALAR_TYPE const * ptr)
{
    if (ALIGNED) return loada<TYPE>(ptr);
    else         return loadu<TYPE>(ptr);
}

template <typename TYPE>
FORCEINLINE TYPE load_clamp(typename TYPE::SCALAR_TYPE const * ptr, typename TYPE::SCALAR_TYPE const * ptrMin, typename TYPE::SCALAR_TYPE const * ptrMax)
{
    PLATFORM_ALIGNED_DEF typename TYPE::SCALAR_TYPE clamped[TYPE::SCALAR_COUNT];
    for (int i = 0; i < TYPE::SCALAR_COUNT; i++)
    {
        clamped[i] = std::max(ptrMin, std::min(ptrMax - 1, ptr + i))[0];
    }
    return loada<TYPE>(clamped);
}

template <typename TYPE>
void streama(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec);

template <typename TYPE>
void storeu(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec);

// WARNING: storea must be used without explicit template parameter type
// Not storea<VecType>(..., VecType()) but storea(..., VecType())
template <typename TYPE>
void storea(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec);

template <typename TYPE>
typename TYPE::SCALAR_TYPE store_scalar(TYPE const & vec);

template <typename TYPE, int SIZE>
struct partial_mask_impl
{
    FORCEINLINE TYPE operator () (int count) const;
};

/* Generated with

    int const N = 33;

    std::cout << "static uint8_t const left_mask_" << (N - 1) * 8 << "[" << N << "][" << N - 1 << "] = " << std::endl << "{ ";

    for (int j = 0; j < N; j++)
    {
        if (j > 0)
            std::cout << ", ";

        std::cout << "{ ";
        for (int i = 0; i < N - 1; i++)
        {
            if (i > 0)
                std::cout << ", ";

            if (i >= j)
                std::cout << "0x00";
            else
                std::cout << "0xff";
        }
        std::cout << " }";
        if (j < N - 1) std::cout << std::endl;
    }
    std::cout << " };" << std::endl;

 */

PLATFORM_ALIGNED_DEF
static uint8_t const left_mask_128[17][16] =
{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

PLATFORM_ALIGNED_DEF
static uint8_t const left_mask_256[33][32] =
{ { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00 }
, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };

template <typename TYPE>
struct partial_mask_impl<TYPE, 16>
{
    FORCEINLINE TYPE operator () (int count) const
    {
        return load<true, TYPE>((typename TYPE::SCALAR_TYPE const *)&left_mask_128[count * sizeof(typename TYPE::SCALAR_TYPE)]);
    }
};

template <typename TYPE>
struct partial_mask_impl<TYPE, 32>
{
    FORCEINLINE TYPE operator () (int count) const
    {
        return load<true, TYPE>((typename TYPE::SCALAR_TYPE const *)&left_mask_256[count * sizeof(typename TYPE::SCALAR_TYPE)]);
    }
};


template <typename TYPE>
FORCEINLINE TYPE partial_mask(int count)
{
    assert(count >= 0 && count <= TYPE::SCALAR_COUNT);
    return partial_mask_impl<TYPE, sizeof(TYPE)>()(count);
}

template <typename TYPE>
FORCEINLINE TYPE apply_partial_mask(TYPE const & input, int count)
{
    if (count == TYPE::SCALAR_COUNT)
        return input;
    return partial_mask<TYPE>(count) & input;
}

template <typename TYPE, int SIZE>
struct store_partial_impl
{
    FORCEINLINE TYPE operator () (typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count) const
    {
        PLATFORM_ALIGNED_DEF typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
        storea(tmp, vec);
        for (int i = 0; i < count; i++)
            ptr[i] = tmp[i];
    }
};

template <typename TYPE>
struct store_partial_impl<TYPE, 16>
{
    FORCEINLINE void operator () (typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count) const
    {
        if (count == TYPE::SCALAR_COUNT / 2)
            store_low(ptr, vec);
        else
            _mm_maskmoveu_si128(I128vec1(vec), I128vec1(partial_mask<TYPE>(count)), (char *)ptr);
    }
};

#if PLATFORM_INTEL_SSE >= 50

template <typename TYPE>
struct store_partial_impl<TYPE, 32>
{
    FORCEINLINE void operator () (typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count) const
    {
        if (count == TYPE::SCALAR_COUNT / 2)
            storeu(ptr, vec.half(0));
        else
            store_mask(ptr, vec, partial_mask<TYPE>(count));
    }
};

#endif

template <typename TYPE>
FORCEINLINE void store_partial(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count)
{
    store_partial_impl<TYPE, sizeof(TYPE)>()(ptr, vec, count);
}

template <typename TYPE>
void store_low(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec);

template <typename TYPE>
void store_high(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec);

template <typename TYPE>
void store_mask(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, TYPE const & mask);

template <bool ALIGNED, typename TYPE>
FORCEINLINE void store(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec)
{
    if (ALIGNED) storea(ptr, vec);
    else         storeu(ptr, vec);
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE void stream(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec)
{
    if (ALIGNED) streama(ptr, vec);
    else         storeu(ptr, vec);
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE void store(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count)
{
    if (count == TYPE::SCALAR_COUNT) store<ALIGNED>(ptr, vec);
    else                             store_partial(ptr, vec, count);
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE void stream(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count)
{
    if (count == TYPE::SCALAR_COUNT) stream<ALIGNED>(ptr, vec);
    else                             store_partial(ptr, vec, count);
}

template <bool ALIGNED, bool NONTEMPORAL, typename TYPE>
FORCEINLINE void store(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count)
{
    if (NONTEMPORAL) stream<ALIGNED>(ptr, vec, count);
    else             store<ALIGNED>(ptr, vec, count);
}

//////////////////////////////////////////////////
// Load / store specializations
//////////////////////////////////////////////////

PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu64vec2)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Is64vec2)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu32vec4)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Is32vec4)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu16vec8)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Is16vec8)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu8vec16)
PXVEC_IMPLEMENT_LOADSTORE_M128I(Is8vec16)
PXVEC_IMPLEMENT_LOADSTORE_M128F(F32vec4 )
PXVEC_IMPLEMENT_LOADSTORE_M128D(F64vec2 )

#if PLATFORM_INTEL_SSE < 60

PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu64vec4)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is64vec4)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu32vec8)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is32vec8)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu16vec16)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is16vec16)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu8vec32)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is8vec32)

#else

PXVEC_IMPLEMENT_LOADSTORE_M256I(Iu64vec4)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Is64vec4)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Iu32vec8)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Is32vec8)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Iu16vec16)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Is16vec16)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Iu8vec32)
PXVEC_IMPLEMENT_LOADSTORE_M256I(Is8vec32)

#endif

#if PLATFORM_INTEL_SSE < 50

PXVEC_IMPLEMENT_LOADSTORE_2xM128X(F32vec8)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(F64vec4)

#else

PXVEC_IMPLEMENT_LOADSTORE_M256F(F32vec8)
PXVEC_IMPLEMENT_LOADSTORE_M256D(F64vec4)

#endif

//////////////////////////////////////////////////
// Scalar load / store specializations
//////////////////////////////////////////////////

PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu64vec2, _mm_cvtsi64_si128, _mm_cvtsi128_si64)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is64vec2, _mm_cvtsi64_si128, _mm_cvtsi128_si64)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is32vec4, _mm_cvtsi32_si128, _mm_cvtsi128_si32)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu32vec4, _mm_cvtsi32_si128, _mm_cvtsi128_si32)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is16vec8, _mm_cvtsi32_si128, _mm_cvtsi128_si32)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu16vec8, _mm_cvtsi32_si128, _mm_cvtsi128_si32)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is8vec16, _mm_cvtsi32_si128, _mm_cvtsi128_si32)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu8vec16, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD(F32vec4, _mm_load_ss, _mm_store_ss)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD(F64vec2, _mm_load_sd, _mm_store_sd)

PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Iu64vec4 )
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Is64vec4 )
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Iu32vec8 )
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Is32vec8 )
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Iu16vec16)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Is16vec16)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Iu8vec32 )
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(Is8vec32 )

PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(F32vec8)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(F64vec4)

template <int HALF, typename TYPE>
FORCEINLINE TYPE unpack(TYPE const & a, TYPE const & b)
{
    if (HALF == 0)
        return unpack_low(a, b);
    return unpack_high(a, b);
}

template <int HALF, typename TYPE>
FORCEINLINE Vec2x<TYPE> unpack(Vec2x<TYPE> const & a, Vec2x<TYPE> const & b)
{
    return Vec2x<TYPE>(unpack_low(a.half(HALF), b.half(HALF)), unpack_high(a.half(HALF), b.half(HALF)));
}

FORCEINLINE float rcp(float a)
{
    return store_scalar(rcp(load_scalar<F32vec4>(a)));
}

PXVEC_IMPLEMENT_UNPACK(Is8vec16, _mm_unpacklo_epi8 , _mm_unpackhi_epi8 )
PXVEC_IMPLEMENT_UNPACK(Iu8vec16, _mm_unpacklo_epi8 , _mm_unpackhi_epi8 )
PXVEC_IMPLEMENT_UNPACK(Is16vec8, _mm_unpacklo_epi16, _mm_unpackhi_epi16)
PXVEC_IMPLEMENT_UNPACK(Iu16vec8, _mm_unpacklo_epi16, _mm_unpackhi_epi16)
PXVEC_IMPLEMENT_UNPACK(Is32vec4, _mm_unpacklo_epi32, _mm_unpackhi_epi32)
PXVEC_IMPLEMENT_UNPACK(Iu32vec4, _mm_unpacklo_epi32, _mm_unpackhi_epi32)
PXVEC_IMPLEMENT_UNPACK(Is64vec2, _mm_unpacklo_epi64, _mm_unpackhi_epi64)
PXVEC_IMPLEMENT_UNPACK(Iu64vec2, _mm_unpacklo_epi64, _mm_unpackhi_epi64)

PXVEC_IMPLEMENT_UNPACK_CAST(F32vec4, F32vec8, Is32vec4, _mm_unpacklo_epi32, _mm_unpackhi_epi32)
PXVEC_IMPLEMENT_UNPACK_CAST(F64vec2, F64vec4, Is64vec2, _mm_unpacklo_epi64, _mm_unpackhi_epi64)

#if PLATFORM_INTEL_SSE >= 50

PXVEC_IMPLEMENT_UNPACK_HALVES(F32vec8, _mm256_unpacklo_ps, _mm256_unpackhi_ps)
PXVEC_IMPLEMENT_UNPACK_HALVES(F64vec4, _mm256_unpacklo_pd, _mm256_unpackhi_pd)

#else

PXVEC_IMPLEMENT_UNPACK_HALVES2(F32vec8)
PXVEC_IMPLEMENT_UNPACK_HALVES2(F64vec4)

#endif

#if PLATFORM_INTEL_SSE >= 60

PXVEC_IMPLEMENT_UNPACK_HALVES(Is8vec32, _mm256_unpacklo_epi8 , _mm256_unpackhi_epi8 )
PXVEC_IMPLEMENT_UNPACK_HALVES(Iu8vec32, _mm256_unpacklo_epi8 , _mm256_unpackhi_epi8 )
PXVEC_IMPLEMENT_UNPACK_HALVES(Is16vec16, _mm256_unpacklo_epi16, _mm256_unpackhi_epi16)
PXVEC_IMPLEMENT_UNPACK_HALVES(Iu16vec16, _mm256_unpacklo_epi16, _mm256_unpackhi_epi16)
PXVEC_IMPLEMENT_UNPACK_HALVES(Is32vec8, _mm256_unpacklo_epi32, _mm256_unpackhi_epi32)
PXVEC_IMPLEMENT_UNPACK_HALVES(Iu32vec8, _mm256_unpacklo_epi32, _mm256_unpackhi_epi32)
PXVEC_IMPLEMENT_UNPACK_HALVES(Is64vec4, _mm256_unpacklo_epi64, _mm256_unpackhi_epi64)
PXVEC_IMPLEMENT_UNPACK_HALVES(Iu64vec4, _mm256_unpacklo_epi64, _mm256_unpackhi_epi64)

#else /* PLATFORM_INTEL_SSE < 60 */

PXVEC_IMPLEMENT_UNPACK_HALVES2(Is8vec32)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Iu8vec32)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Is16vec16)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Iu16vec16)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Is32vec8)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Iu32vec8)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Is64vec4)
PXVEC_IMPLEMENT_UNPACK_HALVES2(Iu64vec4)

#endif /* PLATFORM_INTEL_SSE >= 60 */

template <int HALF, int ELEM_SIZE, int VECTOR_SIZE>
struct unpack_halves_impl;

#define PXVEC_IMPLEMENT_unpack_halves_impl(TYPE) \
    template <int HALF> struct unpack_halves_impl<HALF, sizeof(TYPE::SCALAR_TYPE), sizeof(TYPE)> { \
        template <typename T> FORCEINLINE T operator()(T const & a, T const & b) const { \
            if (HALF == 0) return T(unpack_halves_low(TYPE(a), TYPE(b))); else return T(unpack_halves_high(TYPE(a), TYPE(b))); } };

PXVEC_IMPLEMENT_unpack_halves_impl(F32vec8)
PXVEC_IMPLEMENT_unpack_halves_impl(F64vec4)
PXVEC_IMPLEMENT_unpack_halves_impl(Iu8vec32)
PXVEC_IMPLEMENT_unpack_halves_impl(Iu16vec16)

template <int HALF, int ELEM_COUNT, typename TYPE>
FORCEINLINE TYPE unpack_halves(TYPE const & a, TYPE const & b)
{
    enum { ELEM_SIZE = ELEM_COUNT*sizeof(typename TYPE::SCALAR_TYPE) };
    return TYPE(unpack_halves_impl<HALF, ELEM_SIZE, sizeof(TYPE)>().operator()<TYPE>(a, b));
}

// Unpacks halves independently
template <int HALF, int ELEM_COUNT, typename TYPE>
FORCEINLINE TYPE unpack_halves(TYPE const & a)
{
    return unpack_halves<HALF, ELEM_COUNT>(a, a);
}

template <int HALF, int ELEM_SIZE, int VECTOR_SIZE>
struct unpack_impl;
/*
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        if (HALF == 0)
            return unpack_low(a, b);
        return unpack_high(a, b);
    }
};
*/

#define PXVEC_IMPLEMENT_unpack_impl(TYPE) \
    template <int HALF> struct unpack_impl<HALF, sizeof(TYPE::SCALAR_TYPE), sizeof(TYPE)> { \
        template <typename T> FORCEINLINE TYPE operator()(T const & a, T const & b) const { \
            if (HALF == 0) return unpack_low(TYPE(a), TYPE(b)); else return unpack_high(TYPE(a), TYPE(b)); } };

PXVEC_IMPLEMENT_unpack_impl(Iu8vec16)
PXVEC_IMPLEMENT_unpack_impl(Iu16vec8)
PXVEC_IMPLEMENT_unpack_impl(Iu32vec4)
PXVEC_IMPLEMENT_unpack_impl(Iu64vec2)

template <int HALF, int ELEM_SIZE>
struct unpack_impl<HALF, ELEM_SIZE, 32>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        return TYPE(unpack_impl<0, ELEM_SIZE, 16>()(a.half(HALF), b.half(HALF)), unpack_impl<1, ELEM_SIZE, 16>()(a.half(HALF), b.half(HALF)));
    }
};

#if PLATFORM_INTEL_SSE < 50

template <int HALF>
struct unpack_impl<HALF, 16, 32>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        return TYPE(a.half(HALF), b.half(HALF));
    }
};

#else /* PLATFORM_INTEL_SSE >= 50 */

//PXVEC_IMPLEMENT_unpack_impl(F32vec8)
//PXVEC_IMPLEMENT_unpack_impl(F64vec4)

template <int HALF>
struct unpack_impl<HALF, 16, 32>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        if (HALF == 0)
            return TYPE(_mm256_unpacklo_ps128(F32vec8(a), F32vec8(b)));
        return TYPE(_mm256_unpackhi_ps128(F32vec8(a), F32vec8(b)));
    }
};

template <int HALF>
struct unpack_impl<HALF, 8, 32>
{
    // For AVX, two cases depending on if we are working on __m256 variable or __m128i[2]
#if PLATFORM_INTEL_SSE < 60
    FORCEINLINE F32vec8 operator()(F32vec8 const & a, F32vec8 const & b) const
    {
        __m256 tmp = (HALF == 0) ? _mm256_unpacklo_ps128(F32vec8(a), F32vec8(b)) : _mm256_unpackhi_ps128(F32vec8(a), F32vec8(b));
        __m256 tmp2 = _mm256_permute2f128_ps(tmp, tmp, 1);
        __m256 tmp3 = _mm256_castpd_ps(_mm256_shuffle_pd(_mm256_castps_pd(tmp2), _mm256_castps_pd(tmp2), 1 | 4));
        return _mm256_castpd_ps(_mm256_blend_pd(_mm256_castps_pd(tmp), _mm256_castps_pd(tmp3), (1 << 1) | (1 << 2) /* positions to take from tmp3 */));
    }

    template <typename TYPE>
    FORCEINLINE F64vec4 operator()(F64vec4 const & a, F64vec4 const & b) const
    {
        return operator()(F32vec8(a), F32vec8(b));
    }
#endif /* PLATFORM_INTEL_SSE < 60 */

    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
#if PLATFORM_INTEL_SSE >= 60
        __m256 tmp = (HALF == 0) ? _mm256_unpacklo_ps128(F32vec8(a), F32vec8(b)) : _mm256_unpackhi_ps128(F32vec8(a), F32vec8(b));
        return TYPE(_mm256_permute4x64_epi64(_mm256_castps_si256(tmp), _MM_SHUFFLE(3, 1, 2, 0)));
#else
        // tmp  = A0A1B0B1 = A0XXXXB1
        // tmp2 = B0B1A0A1
        // tmp3 = B1B0A1A0 = XXB0A1XX

        // --- blend tmp & tmp3 --> A0B0A1B1

        Is32vec4 c = Is32vec8(a).half(HALF);
        Is32vec4 d = Is32vec8(b).half(HALF);

        return TYPE(Is32vec8(shuffle<0, 0>(c, d), shuffle<1, 1>(c, d)));
#endif
    }
};

#endif

template <int HALF, int ELEM_COUNT, typename TYPE>
FORCEINLINE Vec2x<TYPE> unpack(Vec2x<TYPE> const & a, Vec2x<TYPE> const & b)
{
    enum { ELEM_SIZE = ELEM_COUNT * sizeof(typename TYPE::SCALAR_TYPE) };
    return Vec2x<TYPE>(unpack_impl<0, ELEM_SIZE, sizeof(TYPE)>()(a.half(HALF), b.half(HALF)),
                          unpack_impl<1, ELEM_SIZE, sizeof(TYPE)>()(a.half(HALF), b.half(HALF)));
}

template <int HALF, int ELEM_COUNT, typename TYPE>
FORCEINLINE TYPE unpack(TYPE const & a, TYPE const & b)
{
    enum { ELEM_SIZE = ELEM_COUNT * sizeof(typename TYPE::SCALAR_TYPE) };
    return TYPE(unpack_impl<HALF, ELEM_SIZE, sizeof(TYPE)>()(a, b));
}

template <int HALF, int ELEM_COUNT, typename TYPE>
FORCEINLINE TYPE unpack(TYPE const & a)
{
    return unpack<HALF, ELEM_COUNT>(a, a);
}

template <int VECTOR_SIZE>
struct permute_halves_impl;

#if PLATFORM_INTEL_SSE >= 50

template <>
struct permute_halves_impl<32>
{
    template <int LOW_INDEX, int LOW_HALF, int HIGH_INDEX, int HIGH_HALF, typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        enum { PERMUTE_MASK_0 = LOW_INDEX < 0 ? 8 : ((LOW_INDEX & 1) << 1)  | (LOW_HALF & 1) };
        enum { PERMUTE_MASK_1 = HIGH_INDEX < 0 ? 8 : ((HIGH_INDEX & 1) << 1)  | (HIGH_HALF & 1) };

        return TYPE(_mm256_permute2f128_ps(F32vec8(a), F32vec8(b), PERMUTE_MASK_0 | (PERMUTE_MASK_1 << 4)));
    }
};

#endif

// Index = -1 to put 0 in an half
template <int LOW_INDEX, int LOW_HALF, int HIGH_INDEX, int HIGH_HALF, typename TYPE>
FORCEINLINE TYPE permute_halves(TYPE const & a, TYPE const & b)
{
    return permute_halves_impl<sizeof(TYPE)>().operator()<LOW_INDEX, LOW_HALF, HIGH_INDEX, HIGH_HALF>(a, b);
}

template <int N, typename TYPE, int SCALAR_SIZE, int VECTOR_SIZE>
struct shiftreg_right_impl;

template <int N, typename TYPE, int SCALAR_SIZE, int VECTOR_SIZE>
struct shiftreg_left_impl;

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_right_impl<N, TYPE, SCALAR_SIZE, 16>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        return TYPE(_mm_srli_si128(I128vec1(a), N * SCALAR_SIZE));
    }
};

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_left_impl<N, TYPE, SCALAR_SIZE, 16>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        return TYPE(_mm_slli_si128(I128vec1(a), N * SCALAR_SIZE));
    }
};

#if PLATFORM_INTEL_SSE >= 60

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_right_impl<N, TYPE, SCALAR_SIZE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        return TYPE(_mm256_alignr_ex_epi8(_mm256_setzero_si256(), I256vec1(a), N * SCALAR_SIZE));
    }
};

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_left_impl<N, TYPE, SCALAR_SIZE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        return TYPE(_mm256_alignr_ex_epi8(I256vec1(a), _mm256_setzero_si256(), 32 - N * SCALAR_SIZE));
    }
};

#else

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_right_impl<N, TYPE, SCALAR_SIZE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        if (N * SCALAR_SIZE < 16)
        {
            return TYPE(_mm_alignr_epi8(I128vec1(a.half(1)), I128vec1(a.half(0)), (N * SCALAR_SIZE) & 15), 
                _mm_srli_si128(I128vec1(a.half(0)), (N * SCALAR_SIZE) & 15));
        }
        else
        {
            return TYPE(_mm_srli_si128(I128vec1(a.half(1)), (N * SCALAR_SIZE) & 15), _mm_setzero_si128());
        }
    }
};

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_left_impl<N, TYPE, SCALAR_SIZE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        if (N * SCALAR_SIZE < 16)
        {
            return TYPE(_mm_slli_si128(I128vec1(a.half(0)), (N * SCALAR_SIZE) & 15), _mm_alignr_epi8(I128vec1(a.half(1)), I128vec1(a.half(0)), (16 - N * SCALAR_SIZE) & 15));
        }
        else
        {
            return TYPE(_mm_setzero_si128(), _mm_slli_si128(I128vec1(a.half(0)), (N * SCALAR_SIZE) & 15));
        }
    }
};

#if PLATFORM_INTEL_SSE >= 50

#if PLATFORM_INTEL_SSE >= 60

#define shiftreg_4_32_template , typename TYPE
#define shiftreg_4_32_TYPE       TYPE

#else /* !PLATFORM_INTEL_SSE >= 60 */

#define shiftreg_4_32_template 
#define shiftreg_4_32_TYPE       F32vec8

#endif /* PLATFORM_INTEL_SSE >= 60 */

template <int N shiftreg_4_32_template>
struct shiftreg_right_impl<N, shiftreg_4_32_TYPE, 4, 32>
{
    FORCEINLINE shiftreg_4_32_TYPE operator()(shiftreg_4_32_TYPE const & a) const
    {
        return shiftreg_4_32_TYPE(_mm256_alignr_ex_ps(_mm256_setzero_ps(), F32vec8(a), N));
    }
};

template <int N shiftreg_4_32_template>
struct shiftreg_left_impl<N, shiftreg_4_32_TYPE, 4, 32>
{
    FORCEINLINE shiftreg_4_32_TYPE operator()(shiftreg_4_32_TYPE const & a) const
    {
        return shiftreg_4_32_TYPE(_mm256_alignr_ex_ps(F32vec8(a), _mm256_setzero_ps(), 8 - N));
    }
};

#undef shiftreg_4_32_template
#undef shiftreg_4_32_TYPE

#endif /* PLATFORM_INTEL_SSE >= 50 */


#endif


template <int N, typename TYPE>
FORCEINLINE TYPE shiftreg_right(TYPE const & a) { return shiftreg_right_impl<N, TYPE, sizeof(typename TYPE::SCALAR_TYPE), sizeof(TYPE)>()(a); }

template <int N, typename TYPE>
FORCEINLINE TYPE shiftreg_left(TYPE const & a) { return shiftreg_left_impl<N, TYPE, sizeof(typename TYPE::SCALAR_TYPE), sizeof(TYPE)>()(a); }

template <int mask>
FORCEINLINE F32vec4 dotprod(F32vec4 const & a, F32vec4 const & b)
{
    return _mm_dp_ps(a, b, mask);
}

FORCEINLINE float dotprod(F32vec4 const & a, F32vec4 const & b)
{
    float res;
    _mm_store_ss(&res, _mm_dp_ps(a, b, 0xf1));
    return res;
}

FORCEINLINE float dotprod3(F32vec4 const & a, F32vec4 const & b)
{
    float res;
    _mm_store_ss(&res, _mm_dp_ps(a, b, 0x71));
    return res;
}

FORCEINLINE float dotprod(F32vec8 const & a, F32vec8 const & b)
{
    float res;
    _mm_store_ss(&res, _mm_add_ss(_mm_dp_ps(a.half(0), b.half(0), 0xf1), _mm_dp_ps(a.half(1), b.half(1), 0xf1)));
    return res;
}

FORCEINLINE float dotprod3(F32vec8 const & a, F32vec8 const & b)
{
    float res;
    _mm_store_ss(&res, _mm_add_ss(_mm_dp_ps(a.half(0), b.half(0), 0x71), _mm_dp_ps(a.half(1), b.half(1), 0x71)));
    return res;
}

template <int N, typename TYPE>
struct insert_impl;

template <int N, typename TYPE>
struct extract_impl;

#define PXVEC_IMPLEMENT_INSERTEXTRACT(TYPE, INSERT_FUN, EXTRACT_FUN) \
    template <int N> struct insert_impl <N, TYPE> { FORCEINLINE TYPE operator()(TYPE const & a, typename TYPE::SCALAR_TYPE t) { return INSERT_FUN(a, t, N); }  }; \
    template <int N> struct extract_impl<N, TYPE> { FORCEINLINE typename TYPE::SCALAR_TYPE operator()(TYPE const & a) { return (typename TYPE::SCALAR_TYPE)EXTRACT_FUN(a, N); } };

#define PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(TYPE, INSERT_FUN, EXTRACT_FUN) \
    template <int N> struct insert_impl <N, TYPE> { FORCEINLINE TYPE operator()(TYPE const & a, typename TYPE::SCALAR_TYPE t) { if (N >= TYPE::SCALAR_COUNT / 2) return TYPE(a.half(0), INSERT_FUN(a.half(1), t, N & (TYPE::SCALAR_COUNT / 2 - 1))); \
                                                                                                                    else                             return TYPE(INSERT_FUN(a.half(0), t, N & (TYPE::SCALAR_COUNT / 2 - 1)), a.half(1)); }  }; \
    template <int N> struct extract_impl<N, TYPE> { FORCEINLINE typename TYPE::SCALAR_TYPE operator()(TYPE const & a) { if (N >= TYPE::SCALAR_COUNT / 2) return (typename TYPE::SCALAR_TYPE)EXTRACT_FUN(a.half(1), N & (TYPE::SCALAR_COUNT / 2 - 1)); \
                                                                                                            else                             return (typename TYPE::SCALAR_TYPE)EXTRACT_FUN(a.half(0), N & (TYPE::SCALAR_COUNT / 2 - 1)); } };

template <int N, typename TYPE>
FORCEINLINE TYPE insert(TYPE const & a, typename TYPE::SCALAR_TYPE b)
{
    return insert_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>()(a, b);
}

template <int N, typename TYPE>
FORCEINLINE typename TYPE::SCALAR_TYPE extract(TYPE const & a)
{
    return extract_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>()(a);
}

template <int N, typename TYPE>
FORCEINLINE Vec2x<TYPE> insert(Vec2x<TYPE> const & a, typename TYPE::SCALAR_TYPE b)
{
    Vec2x<TYPE> res = a;
    res.set_half(N >= TYPE::SCALAR_COUNT, insert_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>()(a.half(N >= TYPE::SCALAR_COUNT), b));
    return res;
}

template <int N, typename TYPE>
FORCEINLINE typename Vec2x<TYPE>::SCALAR_TYPE extract(Vec2x<TYPE> const & a)
{
    return extract_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>()(a.half(N >= TYPE::SCALAR_COUNT));
}

PXVEC_IMPLEMENT_INSERTEXTRACT(Is8vec16, _mm_insert_epi8 , _mm_extract_epi8 )
PXVEC_IMPLEMENT_INSERTEXTRACT(Iu8vec16, _mm_insert_epi8 , _mm_extract_epi8 )
PXVEC_IMPLEMENT_INSERTEXTRACT(Is16vec8, _mm_insert_epi16, _mm_extract_epi16)
PXVEC_IMPLEMENT_INSERTEXTRACT(Iu16vec8, _mm_insert_epi16, _mm_extract_epi16)
PXVEC_IMPLEMENT_INSERTEXTRACT(Is32vec4, _mm_insert_epi32, _mm_extract_epi32)
PXVEC_IMPLEMENT_INSERTEXTRACT(Iu32vec4, _mm_insert_epi32, _mm_extract_epi32)
PXVEC_IMPLEMENT_INSERTEXTRACT(Is64vec2, _mm_insert_epi64, _mm_extract_epi64)
PXVEC_IMPLEMENT_INSERTEXTRACT(F32vec4, _mm_insertf_ps, _mm_extractf_ps)
PXVEC_IMPLEMENT_INSERTEXTRACT(F64vec2, _mm_insert_pd, _mm_extract_pd)

PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(F32vec8, _mm_insertf_ps, _mm_extractf_ps)
PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(F64vec4, _mm_insertf_pd, _mm_extractf_pd)

PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Is64vec4, _mm_insert_epi64, _mm_extract_epi64)
PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Iu64vec4, _mm_insert_epi64, _mm_extract_epi64)

PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Is32vec8, _mm_insert_epi32, _mm_extract_epi32)
PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Iu32vec8, _mm_insert_epi32, _mm_extract_epi32)

PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Is16vec16, _mm_insert_epi16, _mm_extract_epi16)
PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Iu16vec16, _mm_insert_epi16, _mm_extract_epi16)

PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Is8vec32, _mm_insert_epi8, _mm_extract_epi8)
PXVEC_IMPLEMENT_INSERTEXTRACT_DOUBLE(Iu8vec32, _mm_insert_epi8, _mm_extract_epi8)


template <int VECTOR_SIZE, int SCALAR_SIZE>
struct extract_soft_impl;

#if PLATFORM_INTEL_SSE >= 35

#define extract_soft_impl_16_1_indices_(x) \
    _mm_set_epi8(~0, ~0, ~0, ~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, x)

#define extract_soft_impl_16_2_indices_(x) \
    _mm_set_epi8((int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, 2 * x + 1, 2 * x)

#define extract_soft_impl_16_4_indices_(x) \
    _mm_set_epi8((int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, (int8_t)~0, 4 * x + 3, 4 * x + 2, 4 * x + 1, 4 * x)

#define extract_soft_impl_16_1_indices_unroll_(x) x(0), x(1), x(2), x(3), x(4), x(5), x(6), x(7), x(8), x(9), x(10), x(11), x(12), x(13), x(14), x(15)
#define extract_soft_impl_16_2_indices_unroll_(x) x(0), x(1), x(2), x(3), x(4), x(5), x(6), x(7)
#define extract_soft_impl_16_4_indices_unroll_(x) x(0), x(1), x(2), x(3)

template <>
struct extract_soft_impl<16, 1>
{
    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE operator()(int i, TYPE const & a)
    {
        static __m128i extract_soft_impl_16_1_indices[] = { extract_soft_impl_16_1_indices_unroll_(extract_soft_impl_16_1_indices_) };
        return store_scalar(TYPE(_mm_shuffle_epi8(I128vec1(a), extract_soft_impl_16_1_indices[i])));
    }
};

template <>
struct extract_soft_impl<16, 2>
{
    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE operator()(int i, TYPE const & a)
    {
        static __m128i extract_soft_impl_16_2_indices[] = { extract_soft_impl_16_2_indices_unroll_(extract_soft_impl_16_2_indices_) };
        return store_scalar(TYPE(_mm_shuffle_epi8(I128vec1(a), extract_soft_impl_16_2_indices[i])));
    }
};

template <>
struct extract_soft_impl<16, 4>
{
    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE operator()(int i, TYPE const & a)
    {
        static __m128i extract_soft_impl_16_4_indices[] = { extract_soft_impl_16_4_indices_unroll_(extract_soft_impl_16_4_indices_) };
        return store_scalar(TYPE(_mm_shuffle_epi8(I128vec1(a), extract_soft_impl_16_4_indices[i])));
    }
};

#undef extract_soft_impl_16_1_indices_
#undef extract_soft_impl_16_1_indices_unroll_

#undef extract_soft_impl_16_2_indices_
#undef extract_soft_impl_16_2_indices_unroll_

#undef extract_soft_impl_16_4_indices_
#undef extract_soft_impl_16_4_indices_unroll_

#endif

#if PLATFORM_INTEL_SSE >= 60

template <>
struct extract_soft_impl<32, 4>
{
    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE operator()(int i, TYPE const & a)
    {

#define extract_soft_impl_256_32_4_indices_(x) \
    { x, 0, 0, 0, 0, 0, 0, 0 }

#define extract_soft_impl_256_32_4_indices_unroll_(x) x(0), x(1), x(2), x(3), x(4), x(5), x(6), x(7)

        static PLATFORM_ALIGNED_DEF uint32_t extract_soft_impl_32_4_indices[8][8] = {
            extract_soft_impl_256_32_4_indices_unroll_(extract_soft_impl_256_32_4_indices_)
        };

#undef extract_soft_impl_256_32_4_indices_unroll_
#undef extract_soft_impl_256_32_4_indices_

        return extract<0>(TYPE(_mm256_permutevar8x32_epi32(I256vec1(a), loada<Iu32vec8>(extract_soft_impl_32_4_indices[i]))));
    }
};

#undef extract_soft_impl_32_4_indices_
#undef extract_soft_impl_32_4_indices_unroll_

#elif PLATFORM_INTEL_SSE >= 50

template <>
struct extract_soft_impl<32, 4>
{
    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE v(int i, TYPE const & a)
    {
        //static __m256i extract_soft_impl_32_4_indices[] = { extract_soft_impl_32_4_indices_unroll_(extract_soft_impl_32_4_indices_) };

#define extract_soft_impl_256_32_4_indices_(x) \
    { x, 0, 0, 0, x, 0, 0, 0 }

#define extract_soft_impl_256_32_4_indices_unroll_(x) x(0), x(1), x(2), x(3)

        static PLATFORM_ALIGNED_DEF uint32_t extract_soft_impl_256_32_4_indices[8][8] = {
            extract_soft_impl_256_32_4_indices_unroll_(extract_soft_impl_256_32_4_indices_),
            extract_soft_impl_256_32_4_indices_unroll_(extract_soft_impl_256_32_4_indices_)
        };

#undef extract_soft_impl_256_32_4_indices_unroll_
#undef extract_soft_impl_256_32_4_indices_

        __m256i permute_mask = _mm256_load_si256((__m256i const *)&extract_soft_impl_256_32_4_indices[i]);
        if (i < 4)
        {
            //return extract<0>(TYPE(_mm256_permutevar_ps(F32vec8(a), permute_mask)));
            float result;
            _mm_store_ss(&result, _mm256_extractf128_ps(_mm256_permutevar_ps(F32vec8(a), permute_mask), 0));
            return result;
        }
        else
        {
            float result;
            _mm_store_ss(&result, _mm256_extractf128_ps(_mm256_permutevar_ps(F32vec8(a), permute_mask), 1));
            return result;
        }
    }
    
    FORCEINLINE float operator()(int i, F32vec8 const & a)
    {
        return v(i, a);
    }

    FORCEINLINE double operator()(int i, F64vec4 const & a)
    {
        return v(i, a);
    }

    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE operator()(int i, TYPE const & a)
    {
#define extract_soft_impl_128_32_4_indices_(x) \
    { 4 * x, 4 * x + 1, 4 * x + 2, 4 * x + 3, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0 }

#define extract_soft_impl_128_32_4_indices_unroll_(x) x(0), x(1), x(2), x(3)

        static PLATFORM_ALIGNED_DEF uint8_t extract_soft_impl_128_32_4_indices[8][16] = {
            extract_soft_impl_128_32_4_indices_unroll_(extract_soft_impl_128_32_4_indices_),
            extract_soft_impl_128_32_4_indices_unroll_(extract_soft_impl_128_32_4_indices_)
        };

#undef extract_soft_impl_128_32_4_indices_unroll_
#undef extract_soft_impl_128_32_4_indices_

        if (i < 4)
            return extract<0>(TYPE(_mm_shuffle_epi8(I128vec1(a.half(0)), loada<Iu8vec16>(extract_soft_impl_128_32_4_indices[i]))));
        return extract<0>(TYPE(_mm_shuffle_epi8(I128vec1(a.half(1)), loada<Iu8vec16>(extract_soft_impl_128_32_4_indices[i]))));
    }
};

#undef extract_soft_impl_32_4_indices_
#undef extract_soft_impl_32_4_indices_unroll_

#else

template <>
struct extract_soft_impl<32, 4>
{
    template <typename TYPE>
    FORCEINLINE typename TYPE::SCALAR_TYPE operator()(int i, TYPE const & a)
    {
#define extract_soft_impl_128_32_4_indices_(x) \
    { 4 * x, 4 * x + 1, 4 * x + 2, 4 * x + 3, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0, ~0 }

#define extract_soft_impl_128_32_4_indices_unroll_(x) x(0), x(1), x(2), x(3)

        static PLATFORM_ALIGNED_DEF uint8_t extract_soft_impl_128_32_4_indices[8][16] = {
            extract_soft_impl_128_32_4_indices_unroll_(extract_soft_impl_128_32_4_indices_),
            extract_soft_impl_128_32_4_indices_unroll_(extract_soft_impl_128_32_4_indices_)
        };

#undef extract_soft_impl_128_32_4_indices_unroll_
#undef extract_soft_impl_128_32_4_indices_

        if (i < 4)
            return extract<0>(TYPE(_mm_shuffle_epi8(I128vec1(a.half(0)), loada<Iu8vec16>(extract_soft_impl_128_32_4_indices[i]))));
        return extract<0>(TYPE(_mm_shuffle_epi8(I128vec1(a.half(1)), loada<Iu8vec16>(extract_soft_impl_128_32_4_indices[i]))));
    }
};

#undef extract_soft_impl_32_4_indices_
#undef extract_soft_impl_32_4_indices_unroll_

#endif

template <typename TYPE>
FORCEINLINE Vec2x<TYPE> insert(int N, Vec2x<TYPE> const & a, typename TYPE::SCALAR_TYPE b)
{
    Vec2x<TYPE> res = a;
    res.set_half(N >= TYPE::SCALAR_COUNT, insert_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>()(a.half(N >= TYPE::SCALAR_COUNT), b));
    return res;
}

template <typename TYPE>
FORCEINLINE typename Vec2x<TYPE>::SCALAR_TYPE extract(int N, Vec2x<TYPE> const & a)
{
    return extract(N & (TYPE::SCALAR_COUNT - 1), a.half(N >= TYPE::SCALAR_COUNT));
}

template <typename TYPE>
FORCEINLINE typename TYPE::SCALAR_TYPE extract(int i, TYPE const & a)
{
    return extract_soft_impl<sizeof(a), sizeof(typename TYPE::SCALAR_TYPE)>()(i, a);
}

template <int N, typename TYPE, int VECTOR_SIZE>
struct combine_impl;

template <int N, typename TYPE>
struct combine_impl<N, TYPE, 16>
{
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        TYPE res;
        if (N == 0)                                          res = a;
        else if (N*sizeof(typename TYPE::SCALAR_TYPE) == 16) res = b;
        else if (sizeof(typename TYPE::SCALAR_TYPE) == 4 && N == 2) res = shuffle<2, 3, 0, 1>(a, b);
        else res = TYPE(I128vec1(_mm_alignr_epi8(I128vec1(b), I128vec1(a), N*sizeof(typename TYPE::SCALAR_TYPE))));
        return res;
    }
};

#if PLATFORM_INTEL_SSE >= 50

template <int N>
struct combine_impl<N, F32vec8, 32>
{
    FORCEINLINE F32vec8 operator()(F32vec8 const & a, F32vec8 const & b) const
    {
        return _mm256_alignr_ex_ps(b, a, (N&7));
    }
};

#endif /* PLATFORM_INTEL_SSE >= 50 */

#if PLATFORM_INTEL_SSE >= 60

template <int N, typename TYPE>
struct combine_impl<N, TYPE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        return _mm256_alignr_ex_epi8(I256vec1(b), I256vec1(a), ((N * sizeof(typename TYPE::SCALAR_TYPE)) & 31));
    }
};

#else  /* PLATFORM_INTEL_SSE < 60 */

template <int N, typename TYPE>
struct combine_impl<N, TYPE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a, TYPE const & b) const
    {
        TYPE res;
        Is32vec8 _a(a), _b(b);
        int const N_BYTES = N*sizeof(typename TYPE::SCALAR_TYPE);
        if (N == 0)             res = a;
        else if (N_BYTES == 32) res = b;
        else if (N_BYTES == 16) res = TYPE(a.half(1), b.half(0));
        else if (N_BYTES <  16)
        {
            res = TYPE(I128vec1(_mm_alignr_epi8(_a.half(1), _a.half(0), (N_BYTES < 16) * N_BYTES)),
                       I128vec1(_mm_alignr_epi8(_b.half(0), _a.half(1), (N_BYTES < 16) * N_BYTES)));
        }
        else
        {
            res = TYPE(I128vec1(_mm_alignr_epi8(_b.half(0), _a.half(1), (N_BYTES >= 16) * (N_BYTES-16))),
                       I128vec1(_mm_alignr_epi8(_b.half(1), _b.half(0), (N_BYTES >= 16) * (N_BYTES-16))));
        }
        return res;
    }
};

#endif /* PLATFORM_INTEL_SSE >= 60 */

template <int N, typename TYPE>
FORCEINLINE TYPE combine(TYPE const & a, TYPE const & b)
{
    assert(__abs(N) <= TYPE::SCALAR_COUNT);
    const int M = __min(TYPE::SCALAR_COUNT, __max(0, N < 0 ? N + TYPE::SCALAR_COUNT : N));
    return combine_impl<M, TYPE, sizeof(TYPE)>()(a, b);
}

template <typename TYPE>
FORCEINLINE TYPE combine(TYPE const & a, typename TYPE::SCALAR_TYPE const & b)
{
    return insert<TYPE::SCALAR_COUNT - 1>(shiftreg_right<1>(a), b);
}

template <typename TYPE>
FORCEINLINE TYPE combine(typename TYPE::SCALAR_TYPE const & a, TYPE const & b)
{
    return insert<0>(shiftreg_left<1>(b), a);
}

template <int N, typename TYPE>
FORCEINLINE TYPE combine(TYPE const & a, TYPE const & b, TYPE const & c)
{
    //assert(__abs(N) <= TYPE::SCALAR_COUNT);
    const int M = N < 0 ? N + TYPE::SCALAR_COUNT : N;
    return TYPE(combine<M, TYPE>((N < 0) ? a : b, (N < 0) ? b : c));
}

template <bool FIRST, typename TYPE>
FORCEINLINE void cumulate(TYPE & a, TYPE const & b)
{
    if (FIRST)
        a = b;
    else
        a += b;
}

FORCEINLINE F64vec2 sum02_13(F32vec4 const & a)
{
    return F64vec2(_mm_cvtps_pd(a + shiftreg_right<2>(a)));
}

template <typename TYPE>
FORCEINLINE TYPE sat_add(TYPE const & a, TYPE const & b)
{
    // Default behavior valid only for floating types
    assert(std::numeric_limits<typename TYPE::SCALAR_TYPE>::is_integer == false);
    return a + b;
}

template <typename TYPE>
FORCEINLINE TYPE sat_sub(TYPE const & a, TYPE const & b)
{
    // Default behavior valid only for floating types
    assert(std::numeric_limits<typename TYPE::SCALAR_TYPE>::is_integer == false);
    return a - b;
}

template <typename TYPE>
FORCEINLINE TYPE sat_x2(TYPE const & a)
{
    return sat_add(a, a);
}

template <int N, typename TYPE>
FORCEINLINE TYPE sat_shift_left(TYPE const & a)
{
    if (N <= 0) return a;
    if (N == 1) return sat_add(a, a);
    TYPE b = shift_left<__max(0, N - 1)>(a);
    return sat_add(b, b);
}

template <int N, typename VECTOR>
struct cumsum_impl
{
    VECTOR operator()(VECTOR const & a);
};

template <typename VECTOR>
struct cumsum_impl<4, VECTOR>
{
    FORCEINLINE VECTOR operator()(VECTOR const & a)
    {
        VECTOR b = combine<-1>(VECTOR::zero(), a);
        VECTOR c = a + b;
        VECTOR d = combine<-2>(VECTOR::zero(), c);
        return c + d;
    }
};

template <typename VECTOR>
struct cumsum_impl<8, VECTOR>
{
    FORCEINLINE VECTOR operator()(VECTOR const & a)
    {
        VECTOR b = combine<-1>(VECTOR::zero(), a);
        VECTOR c = a + b;
        VECTOR d = combine<-2>(VECTOR::zero(), c);
        VECTOR e = c + d;
        VECTOR f = combine<-4>(VECTOR::zero(), e);
        return e + f;
    }
};

template <typename VECTOR>
FORCEINLINE VECTOR cumsum(VECTOR const & a)
{
    return cumsum_impl<VECTOR::SCALAR_COUNT, VECTOR>()(a);
}

template <typename TYPE, int VECTOR_SIZE>
struct select_impl { TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE select_impl(TYPE const & mask, TYPE const & on, TYPE const & off) {
        res = andnot(mask, off) | (mask & on);
    }
};

#if PLATFORM_INTEL_SSE >= 40

template <typename TYPE>
struct select_impl<TYPE, 16> { TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE select_impl(TYPE const & mask, TYPE const & on, TYPE const & off) {
        res = TYPE(_mm_blendv_epi8(I128vec1(off), I128vec1(on), I128vec1(mask)));
    }
};

#if PLATFORM_INTEL_SSE < 50

template <typename TYPE>
struct select_impl<TYPE, 32> { TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE select_impl(TYPE const & mask, TYPE const & on, TYPE const & off) {
        res =  TYPE(_mm_blendv_epi8(I128vec1(off.half(0)), I128vec1(on.half(0)), I128vec1(mask.half(0))),
                    _mm_blendv_epi8(I128vec1(off.half(1)), I128vec1(on.half(1)), I128vec1(mask.half(1))));
    }
};

#endif /* PLATFORM_INTEL_SSE < 50 */

#endif /* PLATFORM_INTEL_SSE >= 40 */

template <typename TYPE>
FORCEINLINE TYPE select(TYPE const & mask, TYPE const & on, TYPE const & off)
{
    return select_impl<TYPE, sizeof(TYPE)>(mask, on, off);
}

#include "PXVec.convert.inl"

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a)
{
    return TYPE(_mm_shuffle_epi32(I128vec1(a), _MM_SHUFFLE(A3&3, A2&3, A1&3, A0&3)));
}

#if PLATFORM_INTEL_SSE >= 35
template <int A0, int A1, int A2, int A3, int A4, int A5, int A6, int A7, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a)
{
    return TYPE(_mm_shuffle_epi8(I128vec1(a), _mm_set_epi8(2 * A7 + 1, 2 * A7,
                                                           2 * A6 + 1, 2 * A6,
                                                           2 * A5 + 1, 2 * A5,
                                                           2 * A4 + 1, 2 * A4,
                                                           2 * A3 + 1, 2 * A3,
                                                           2 * A2 + 1, 2 * A2,
                                                           2 * A1 + 1, 2 * A1,
                                                           2 * A0 + 1, 2 * A0)));
}

template <int A0, int A1, int A2, int A3, int A4, int A5, int A6, int A7, int A8, int A9, int A10, int A11, int A12, int A13, int A14, int A15, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a)
{
    return TYPE(_mm_shuffle_epi8(I128vec1(a), _mm_set_epi8(A15, A14, A13, A12,
                                                           A11, A10,  A9,  A8,
                                                            A7,  A6,  A5,  A4,
                                                            A3,  A2,  A1,  A0)));
}

#endif

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE TYPE shuffle_low(TYPE const & a)
{
    return TYPE(_mm_shufflelo_epi16(I128vec1(a), _MM_SHUFFLE(A3&3, A2&3, A1&3, A0&3)));
}

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE TYPE shuffle_high(TYPE const & a)
{
    return TYPE(_mm_shufflehi_epi16(I128vec1(a), _MM_SHUFFLE(A3&3, A2&3, A1&3, A0&3)));
}

template <int A0, int A1, int B2, int B3, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a, TYPE const & b)
{
    return TYPE(_mm_shuffle_ps(F32vec4(a), F32vec4(b), _MM_SHUFFLE(B3&3, B2&3, A1&3, A0&3)));
}

template <int A0, int B1, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a, TYPE const & b)
{
    return TYPE(_mm_shuffle_pd(F64vec2(a), F64vec2(b), _MM_SHUFFLE2(B1&1, A0&1)));
}

template <int A0, int B1, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a)
{
    return TYPE(_mm_shuffle_pd(F64vec2(a), F64vec2(a), _MM_SHUFFLE2(B1&1, A0&1)));
}
/*
template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE typename TYPE::HALF_TYPE shuffle(TYPE const & a)
{
    PX_STATICASSERT((A0 >> 2) == (A1 >> 2) && (A2 >> 2) == (A3 >> 2) && __abs(A0 >> 2) <= 1 && __abs(A2 >> 2) <= 1);
    return TYPE::HALF_TYPE(shuffle<A0, A1, A2, A3>(a.half(A0>>2), a.half(A2>>2)));
}
*/

template <int ELEM_SIZE, int ELEM_COUNT>
struct swap_inside_impl;

#define PXVEC_SW(X) X == I ? J : X == J ? I : X

template <>
struct swap_inside_impl<2, 8>
{
    template <int I, int J, typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        return TYPE(shuffle<PXVEC_SW(0), PXVEC_SW(1), PXVEC_SW(2), PXVEC_SW(3), PXVEC_SW(4), PXVEC_SW(5), PXVEC_SW(6), PXVEC_SW(7)>(in));
    }
};

template <>
struct swap_inside_impl<1, 16>
{
    template <int I, int J, typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        return TYPE(shuffle<PXVEC_SW(0), PXVEC_SW(1), PXVEC_SW(2), PXVEC_SW(3), PXVEC_SW(4), PXVEC_SW(5), PXVEC_SW(6), PXVEC_SW(7), PXVEC_SW(8), PXVEC_SW(9), PXVEC_SW(10), PXVEC_SW(11), PXVEC_SW(12), PXVEC_SW(13), PXVEC_SW(14), PXVEC_SW(15)>(in.half(1)));
    }
};

template <>
struct swap_inside_impl<4, 4>
{
    template <int I, int J, typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        return TYPE(shuffle<PXVEC_SW(0), PXVEC_SW(1), PXVEC_SW(2), PXVEC_SW(3)>(in));
    }
};

#undef PXVEC_SW

template <int I, int J, typename TYPE>
FORCEINLINE TYPE swap_inside(TYPE const & a)
{
    return swap_inside_impl<sizeof(TYPE::SCALAR_TYPE), TYPE::SCALAR_COUNT>().operator()<I, J>(a);
}

template <int ELEM_SIZE, int ELEM_COUNT>
struct reverse_impl;


template <>
struct reverse_impl<2, 8>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
#if PLATFORM_INTEL_SSE >= 35
        return TYPE(shuffle<7, 6, 5, 4, 3, 2, 1, 0>(in));
#else
        return TYPE(shuffle_low<3, 2, 1, 0>(shuffle_high<3, 2, 1, 0>(shuffle<3, 2, 1, 0>(in))));
#endif
    }
};

template <>
struct reverse_impl<1, 16>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        return TYPE(shuffle<15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0>(in.half(1)));
    }
};

template <>
struct reverse_impl<4, 4>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        return TYPE(shuffle<3, 2, 1, 0>(in));
    }
};

#if PLATFORM_INTEL_SSE >= 60

template <>
struct reverse_impl<2, 16>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        __m256i tmp = _mm256_shuffle_epi8(I256vec1(in), _mm256_set_epi8(1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14, 1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14));
        return TYPE(_mm256_permute2f128_si256(tmp, tmp, 1));
    }
};

template <>
struct reverse_impl<4, 8>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        __m256i tmp = _mm256_shuffle_epi32(I256vec1(in), _MM_SHUFFLE(0, 1, 2, 3));
        return TYPE(_mm256_permute2f128_si256(tmp, tmp, 1));
    }
};

template <>
struct reverse_impl<8, 4>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        __m256d tmp = _mm256_shuffle_pd(F64vec4(in), F64vec4(in), _MM_SHUFFLE2(0, 1));
        return TYPE(_mm256_permute2f128_ps(tmp, tmp, 1));
    }
};

#else

template <>
struct reverse_impl<4, 8>
{
    template <typename TYPE>
    FORCEINLINE TYPE operator()(TYPE const & in)
    {
        return TYPE(shuffle<3, 2, 1, 0>(in.half(1)), shuffle<3, 2, 1, 0>(in.half(0)));
    }
};

#endif

template <typename TYPE>
FORCEINLINE TYPE reverse(TYPE const & a)
{
    return reverse_impl<sizeof(TYPE::SCALAR_TYPE), TYPE::SCALAR_COUNT>()(a);
}

template <int N, typename TYPE>
FORCEINLINE TYPE broadcast(TYPE const & a, Is16vec8::LO)
{
    enum { N_M4 = __max(0, N - 4) };
    if (N >= 4)
        return shuffle<2, 2, 2, 2>(shuffle_high<N_M4, N_M4, N_M4, N_M4>(a));
    return shuffle<0, 0, 0, 0>(shuffle_low<N, N, N, N>(a));
}

template <int N, typename TYPE>
FORCEINLINE TYPE broadcast(TYPE const & a, Is32vec4::LO)
{
    return shuffle<N, N, N, N>(a);
}

template <int N, typename TYPE>
FORCEINLINE TYPE broadcast(TYPE const & a, Is64vec2::LO)
{
    return shuffle<N, N>(a, a);
}

template <int N, typename TYPE, typename TYPE_LO>
FORCEINLINE TYPE broadcast(TYPE const & a, TYPE_LO)
{
    return TYPE(extract<N>(a));
}

template <int N, typename TYPE>
FORCEINLINE TYPE broadcast(TYPE const & a)
{
    return broadcast<N>(a, TYPE::LO());
}

template <typename TYPE>
FORCEINLINE typename TYPE::HALF_TYPE sum_halves(TYPE const & a)
{
    return a.half(0) + a.half(1);
}

template <typename TYPE>
FORCEINLINE int vec_to_mask(TYPE const & a)
{
    return vec_to_mask((typename TYPE::NATIVE_VEC_TYPE const &)(a));
}

#if PLATFORM_INTEL_SSE >= 50

template <>
FORCEINLINE int vec_to_mask<__m256>(__m256 const & a)
{
    return _mm256_movemask_ps(a);
}

#endif

template <int N>
struct vec_equal_impl
{
    template <typename TYPE>
    FORCEINLINE bool operator()(TYPE const & a, TYPE const & b) const;
};

template <>
struct vec_equal_impl<16>
{
    template <typename TYPE>
    FORCEINLINE bool operator()(TYPE const & a, TYPE const & b) const
    {
/*
#if PLATFORM_INTEL_SSE >= 40
        return _mm_testc_si128(Is32vec4(a), Is32vec4(b)) != 0;
#else
        */
#if PLATFORM_INTEL_SSE >= 50
        return _mm_movemask_ps(_mm_cmp_ps(F32vec4(a), F32vec4(b), _CMP_NEQ_OQ)) == 0;
#else
        return _mm_movemask_ps(_mm_cmpneq_ps(F32vec4(a), F32vec4(b))) == 0;
#endif
//#endif
    }
};

template <>
struct vec_equal_impl<32>
{
    template <typename TYPE>
    FORCEINLINE bool operator()(TYPE const & a, TYPE const & b) const
    {
#if PLATFORM_INTEL_SSE >= 50
        return _mm256_movemask_ps(_mm256_cmp_ps(F32vec8(a), F32vec8(b), _CMP_NEQ_OQ)) == 0;
#else
        return vec_equal_impl<16>()(a.half(0), b.half(0)) && vec_equal_impl<16>()(a.half(1), b.half(1));
#endif
    }
};

template <typename TYPE>
FORCEINLINE bool vec_equal(TYPE const & a, TYPE const & b)
{
    return vec_equal_impl<sizeof(TYPE)>()(a, b);
}

template <>
FORCEINLINE int vec_to_mask<__m128d>(__m128d const & a)
{
    return _mm_movemask_pd(a);
}

template <>
FORCEINLINE int vec_to_mask<__m128>(__m128 const & a)
{
    return _mm_movemask_ps(a);
}

template <>
FORCEINLINE int vec_to_mask<__m128i>(__m128i const & a)
{
    return _mm_movemask_epi8(a);
}

#if PLATFORM_INTEL_SSE < 50

template <>
FORCEINLINE int vec_to_mask<F32vec8>(F32vec8 const & a)
{
    return (vec_to_mask(a.half(0)) | (vec_to_mask(a.half(1))<<F32vec8::HALF_TYPE::SCALAR_COUNT));
}

#endif

template <typename TYPE>
FORCEINLINE bool vec_is_zero(TYPE const & a)
{
    return vec_is_zero((typename TYPE::NATIVE_VEC_TYPE const &)(a));
}

template <typename TYPE>
FORCEINLINE bool vec_is_zero(Vec2x<TYPE> const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

#if PLATFORM_INTEL_SSE >= 50

template <>
FORCEINLINE bool vec_is_zero<__m256>(__m256 const & a)
{
    return !!_mm256_testz_si256(_mm256_castps_si256(a), _mm256_castps_si256(a));
}

template <>
FORCEINLINE bool vec_is_zero<__m256d>(__m256d const & a)
{
    return !!_mm256_testz_si256(_mm256_castpd_si256(a), _mm256_castpd_si256(a));
}

#endif


#if PLATFORM_INTEL_SSE >= 60

template <>
FORCEINLINE bool vec_is_zero<__m256i>(__m256i const & a)
{
    return !!_mm256_testz_si256(a, a);
}


#endif

#if PLATFORM_INTEL_SSE >= 40

template <>
FORCEINLINE bool vec_is_zero<__m128d>(__m128d const & a)
{
    return !!_mm_testz_si128(_mm_castpd_si128(a), _mm_castpd_si128(a));
}

template <>
FORCEINLINE bool vec_is_zero<__m128>(__m128 const & a)
{
    return !!_mm_testz_si128(_mm_castps_si128(a), _mm_castps_si128(a));
}

template <>
FORCEINLINE bool vec_is_zero<__m128i>(__m128i const & a)
{
    return !!_mm_testz_si128(a, a);
}

#else

template <>
FORCEINLINE bool vec_is_zero<__m128d>(__m128d const & a)
{
    return _mm_movemask_pd(_mm_cmpeq_pd(a, _mm_setzero_pd())) == 0;
}

template <>
FORCEINLINE bool vec_is_zero<__m128>(__m128 const & a)
{
    return _mm_movemask_ps(_mm_cmpeq_ps(a, _mm_setzero_ps())) == 0;
}

template <>
FORCEINLINE bool vec_is_zero<__m128i>(__m128i const & a)
{
    return _mm_movemask_epi8(_mm_cmpeq_epi8(a, _mm_setzero_si128())) == 0;
}

#endif

#if PLATFORM_INTEL_SSE < 50

template <>
FORCEINLINE bool vec_is_zero<F32vec8>(F32vec8 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

#endif

#if PLATFORM_INTEL_SSE < 60

template <>
FORCEINLINE bool vec_is_zero<Is32vec8>(Is32vec8 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

template <>
FORCEINLINE bool vec_is_zero<Iu32vec8>(Iu32vec8 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

#endif

template <>
FORCEINLINE bool vec_is_zero<Is16vec16>(Is16vec16 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

template <typename TYPE>
FORCEINLINE bool vec_any(TYPE const & a)
{
    return !!convert_to_mask_impl<sizeof(TYPE), sizeof(TYPE::SCALAR_TYPE), TYPE>()(a);
}


template <typename TYPE>
FORCEINLINE bool vec_none(TYPE const & a)
{
    return !convert_to_mask_impl<sizeof(TYPE), sizeof(TYPE::SCALAR_TYPE), TYPE>()(a);
}

template <typename TYPE>
FORCEINLINE bool vec_all(TYPE const & a)
{
    return vec_none(a == TYPE::zero());
}

// For compatibility with scalar code
FORCEINLINE bool vec_any(int a)
{
    return !!a;
}

// For compatibility with scalar code
FORCEINLINE bool vec_none(int a)
{
    return !a;
}

// For compatibility with scalar code
FORCEINLINE bool vec_all(int a)
{
    return !!a;
}
template <int N ,typename TYPE>
FORCEINLINE void vec_print(TYPE const & t)
{
    enum { NEXT_N = __min(N + 1, TYPE::SCALAR_COUNT - 1) };

    int n = N;
    char tmp[16];
    if (std::numeric_limits<typename TYPE::SCALAR_TYPE>::is_integer)
    {
        sprintf_s(tmp, "%%0%dx", sizeof(typename TYPE::SCALAR_TYPE) * 2);
    }
    else
    {
        strcpy_s(tmp, "%f");
    }

    printf(tmp, (PX_TYPEUNSIGN(typename TYPE::SCALAR_TYPE))extract<N>(t));

    if (N < NEXT_N)
    {
        printf(", ");
        return vec_print<NEXT_N>(t);
    }
    else
    {
        printf("\n");
    }
}

template <typename TYPE>
FORCEINLINE bool vec_is_nonzero(TYPE const & t)
{
    return !!convert_to_mask(t != t.zero());
}

template <typename TYPE>
FORCEINLINE void vec_print(TYPE const & t)
{
    vec_print<0>(t);
}

template <typename HALF_TYPE>
FORCEINLINE int convert_to_mask(Vec2x<HALF_TYPE> const & a)
{
    return convert_to_mask<HALF_TYPE>(a.half(0)) | (convert_to_mask<HALF_TYPE>(a.half(1)) << HALF_TYPE::SCALAR_COUNT);
}

// Put on the lowest element the min element
template <typename TYPE>
FORCEINLINE TYPE min_inside(TYPE const & a)
{
    TYPE minValue = a;
    if (TYPE::SCALAR_COUNT >= 32) minValue = min(minValue, shiftreg_right<16>(minValue));
    if (TYPE::SCALAR_COUNT >= 16) minValue = min(minValue, shiftreg_right<8>(minValue));
    if (TYPE::SCALAR_COUNT >= 8) minValue = min(minValue, shiftreg_right<4>(minValue));
    if (TYPE::SCALAR_COUNT >= 4) minValue = min(minValue, shiftreg_right<2>(minValue));
    if (TYPE::SCALAR_COUNT >= 2) minValue = min(minValue, shiftreg_right<1>(minValue));
    return minValue;
}

// Put on the lowest element the max element
template <typename TYPE>
FORCEINLINE TYPE max_inside(TYPE const & a)
{
    TYPE maxValue = a;
    if (TYPE::SCALAR_COUNT >= 32) maxValue = max(maxValue, shiftreg_right<16>(maxValue));
    if (TYPE::SCALAR_COUNT >= 16) maxValue = max(maxValue, shiftreg_right<8>(maxValue));
    if (TYPE::SCALAR_COUNT >= 8)  maxValue = max(maxValue, shiftreg_right<4>(maxValue));
    if (TYPE::SCALAR_COUNT >= 4)  maxValue = max(maxValue, shiftreg_right<2>(maxValue));
    if (TYPE::SCALAR_COUNT >= 2)  maxValue = max(maxValue, shiftreg_right<1>(maxValue));
    return maxValue;
}

// Put on the lowest element the min element
template <typename TYPE>
FORCEINLINE typename TYPE::SCALAR_TYPE min_inside_scalar(TYPE const & a)
{
    return store_scalar(min_inside(a));
}

// Put on the lowest element the max element
template <typename TYPE>
FORCEINLINE typename TYPE::SCALAR_TYPE max_inside_scalar(TYPE const & a)
{
    return store_scalar(max_inside(a));
}

template <typename TYPE>
FORCEINLINE bool find_first(unsigned int & index, TYPE const & a, TYPE const & b)
{
    bool result = (_BitScanForward((unsigned long *)&index, vec_to_mask(a == b)) != 0);
    index /= sizeof(TYPE::SCALAR_TYPE);
    return result;
}

template <typename TYPE>
FORCEINLINE bool find_last(unsigned int & index, TYPE const & a, TYPE const & b)
{
    bool result = (_BitScanReverse((unsigned long *)&index, vec_to_mask(a == b)) != 0);
    index /= sizeof(TYPE::SCALAR_TYPE);
    return result;
}

template <typename TYPE>
FORCEINLINE bool find_first(unsigned int & index, TYPE const & compareMask)
{
    bool result = (_BitScanForward((unsigned long *)&index, vec_to_mask(compareMask)) != 0);
    index /= sizeof(TYPE::SCALAR_TYPE);
    return result;
}

template <typename TYPE>
FORCEINLINE bool find_last(unsigned int & index, TYPE const & compareMask)
{
    bool result = (_BitScanReverse((unsigned long *)&index, vec_to_mask(compareMask)) != 0);
    index /= sizeof(TYPE::SCALAR_TYPE);
    return result;
}

template <typename TYPE>
FORCEINLINE bool find_first_not(unsigned int & index, TYPE const & a, TYPE const & b)
{
    int mask = ~vec_to_mask(a == b) & ((1 << sizeof(TYPE)) - 1);
    if (mask == 0)
        return false;
    bool result = (_BitScanForward((unsigned long *)&index, mask) != 0);
    index /= sizeof(TYPE::SCALAR_TYPE);
    return result;
}

template <typename TYPE>
FORCEINLINE bool find_last_not(unsigned int & index, TYPE const & a, TYPE const & b)
{
    bool result = (_BitScanReverse((unsigned long *)&index, (((1 << sizeof(TYPE)) - 1)  & ~vec_to_mask(a == b))) != 0);
    index /= sizeof(TYPE::SCALAR_TYPE);
    return result;
}

template <typename TYPE>
FORCEINLINE TYPE argmin_inside(unsigned int & minIndex, TYPE const & a)
{
    TYPE minValue = broadcast<0>(min_inside(a));
    _BitScanReverse((unsigned long *)&minIndex, vec_to_mask(a == minValue));
    minIndex /= sizeof(TYPE::SCALAR_TYPE);
    return minValue;
}

template <typename TYPE>
FORCEINLINE TYPE argmax_inside(unsigned int & maxIndex, TYPE const & a)
{
    TYPE maxValue = broadcast<0>(max_inside(a));
    _BitScanReverse((unsigned long *)&maxIndex, vec_to_mask(a == maxValue));
    maxIndex /= sizeof(TYPE);
    return maxValue;
}

template <typename TYPE>
FORCEINLINE TYPE sign(TYPE const & a)
{
    return select(a >= TYPE::zero(), TYPE((typename TYPE::SCALAR_TYPE)1), TYPE((typename TYPE::SCALAR_TYPE)-1));
}

enum
{
    RoundingMode_Nearest = 0,
    RoundingMode_Floor = 1,
    RoundingMode_Ceil = 2,
    RoundingMode_Truncate = 3
};

#ifdef _DEBUG
template <int ROUNDING_MODE>
void assert_roundingmode()
{
    switch (ROUNDING_MODE)
    {
    case RoundingMode_Floor:
        assert(_MM_GET_ROUNDING_MODE() == _MM_ROUND_DOWN);
        break;
    case RoundingMode_Ceil:
        assert(_MM_GET_ROUNDING_MODE() == _MM_ROUND_UP);
        break;
    case RoundingMode_Truncate:
        assert(_MM_GET_ROUNDING_MODE() == _MM_ROUND_TOWARD_ZERO);
        break;
    default:
        assert(_MM_GET_ROUNDING_MODE() == _MM_ROUND_NEAREST);
        break;
    }
};
#else
template <int ROUNDING_MODE>
void assert_roundingmode()
{
}
#endif

template <int ROUNDING_MODE>
class vec_roundingmode
{
    int m_previousMode;

public:
    FORCEINLINE ~vec_roundingmode()
    {
        if (m_previousMode != ROUNDING_MODE)
            _MM_SET_ROUNDING_MODE(m_previousMode);
    }

    FORCEINLINE vec_roundingmode()
    {
        m_previousMode = _MM_GET_ROUNDING_MODE();
        if (m_previousMode != ROUNDING_MODE)
        {
            switch (ROUNDING_MODE)
            {
            case RoundingMode_Floor:
                _MM_SET_ROUNDING_MODE(_MM_ROUND_DOWN);
                break;
            case RoundingMode_Ceil:
                _MM_SET_ROUNDING_MODE(_MM_ROUND_UP);
                break;
            case RoundingMode_Truncate:
                _MM_SET_ROUNDING_MODE(_MM_ROUND_TOWARD_ZERO);
                break;
            default:
                _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);
                break;
            }
        }
    }
};

FORCEINLINE int sum(Is16vec8 const & a) {
    return -sum(mul_add(a, Is16vec8::minus_one()));
}

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN,
          int SCALAR_OUT_SIZE, int SCALAR_IN_SIZE, int VECTOR_SIZE>
struct pack_mask_impl;

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 1, 2, 16>
{
    // 2 mask vectors of 16 bits to 1 mask vectors of 8 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(pack(Is16vec8(a[0]), Is16vec8(a[1])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 1, 4, 16>
{
    // 4 mask vectors of 32 bits to 1 mask vectors of 8 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(pack(
                pack(Is32vec4(a[0]), Is32vec4(a[1])),
                pack(Is32vec4(a[2]), Is32vec4(a[3]))));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 1, 8, 16>
{
    // 8 mask vectors of 64 bits to 1 mask vectors of 8 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(pack(
            pack(
                Is32vec4(shuffle<1, 3, 1, 3>(a[0], a[1])),
                Is32vec4(shuffle<1, 3, 1, 3>(a[2], a[3]))),
            pack(
                Is32vec4(shuffle<1, 3, 1, 3>(a[4], a[5])),
                Is32vec4(shuffle<1, 3, 1, 3>(a[6], a[7])))));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 2, 4, 16>
{
    // 2 mask vectors of 32 bits to 1 mask vectors of 16 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(pack(Is32vec4(a[0]), Is32vec4(a[1])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 2, 8, 16>
{
    // 4 mask vectors of 64 bits to 1 mask vectors of 16 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(pack(
                Is32vec4(shuffle<1, 3, 1, 3>(a[0], a[1])),
                Is32vec4(shuffle<1, 3, 1, 3>(a[2], a[3]))));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 4, 8, 16>
{
    // 2 mask vectors of 64 bits to 1 mask vectors of 32 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(shuffle<1, 3, 1, 3>(a[0], a[1]));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
struct pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN, 1, 1, 16>
{
    // 2 mask vectors of 64 bits to 1 mask vectors of 32 bits
    static TYPE_OUT FORCEINLINE go(ARRAY_IN const & a)
    {
        return TYPE_OUT(a[0]);
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_IN>
FORCEINLINE TYPE_OUT pack_mask(ARRAY_IN const & a, TYPE_IN const & dummy)
{
    return TYPE_OUT(pack_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_IN,
        sizeof(typename TYPE_OUT::SCALAR_TYPE),
        sizeof(typename TYPE_IN::SCALAR_TYPE),
        sizeof(TYPE_OUT)>::go(a));
}

/*!
 * Packs any array (C array of adapter implementing the [) operator)
 * of vector masks to a shorter vector mask.
 *
 * Currently supported:
 * -------------------
 * Xx16vec8[2] => Xx8vec16
 * Xx32vec4[4] => Xx8vec16
 * Xx64vec2[8] => Xx8vec16
 *
 * Xx32vec4[2] => Xx16vec8
 * Xx64vec2[4] => Xx16vec8
 *
 * Xx64vec2[2] => Xx32vec4
 *
 */
template <typename TYPE_OUT, typename ARRAY_IN>
FORCEINLINE TYPE_OUT pack_mask(ARRAY_IN const & a)
{
    return pack_mask<TYPE_OUT>(a, a[0]);
}


template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN,
int SCALAR_OUT_SIZE, int SCALAR_IN_SIZE, int VECTOR_SIZE>
struct cast_mask_impl;

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN, int N>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, N, N, 16>
{
    // 2 mask vectors of 16X bits from 1 mask vectors of 8X bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(b[0]);
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 2, 1, 16>
{
    // 2 mask vectors of 16X bits from 1 mask vectors of 8X bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(unpack_low (Is8vec16(b[0]), Is8vec16(b[0])));
        a[1] = TYPE_OUT(unpack_high(Is8vec16(b[0]), Is8vec16(b[0])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 4, 2, 16>
{
    // 2 mask vectors of 16X bits from 1 mask vectors of 8X bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(unpack_low (Is16vec8(b[0]), Is16vec8(b[0])));
        a[1] = TYPE_OUT(unpack_high(Is16vec8(b[0]), Is16vec8(b[0])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 8, 4, 16>
{
    // 2 mask vectors of 16X bits from 1 mask vectors of 8X bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(unpack_low (Is32vec4(b[0]), Is32vec4(b[0])));
        a[1] = TYPE_OUT(unpack_high(Is32vec4(b[0]), Is32vec4(b[0])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 4, 1, 16>
{
    // 4 mask vectors of 32X bits from 1 mask vectors of 8X bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        Is16vec8 c[2];

        c[0] = Is16vec8(unpack_low (Is8vec16(b[0]), Is8vec16(b[0])));
        c[1] = Is16vec8(unpack_high(Is8vec16(b[0]), Is8vec16(b[0])));

        a[0] = TYPE_OUT(unpack_low (c[0], c[0]));
        a[1] = TYPE_OUT(unpack_high(c[0], c[0]));
        a[2] = TYPE_OUT(unpack_low (c[1], c[1]));
        a[3] = TYPE_OUT(unpack_high(c[1], c[1]));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 8, 2, 16>
{
    // 4 mask vectors of 32X bits from 1 mask vectors of 8X bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        Is32vec4 c[2];

        c[0] = Is32vec4(unpack_low (Is16vec8(b[0]), Is16vec8(b[0])));
        c[1] = Is32vec4(unpack_high(Is16vec8(b[0]), Is16vec8(b[0])));

        a[0] = TYPE_OUT(unpack_low (c[0], c[0]));
        a[1] = TYPE_OUT(unpack_high(c[0], c[0]));
        a[2] = TYPE_OUT(unpack_low (c[1], c[1]));
        a[3] = TYPE_OUT(unpack_high(c[1], c[1]));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 8, 1, 16>
{
    // 8 mask vectors of 64X bits from 1 mask vectors of 8 bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        Is16vec8 c[2];

        c[0] = Is16vec8(unpack_low (Is8vec16(b[0]), Is8vec16(b[0])));
        c[1] = Is16vec8(unpack_high(Is8vec16(b[0]), Is8vec16(b[0])));

        Is32vec4 d[4];

        d[0] = Is32vec4(unpack_low (c[0], c[0]));
        d[1] = Is32vec4(unpack_high(c[0], c[0]));
        d[2] = Is32vec4(unpack_low (c[1], c[1]));
        d[3] = Is32vec4(unpack_high(c[1], c[1]));

        a[0] = TYPE_OUT(unpack_low (d[0], d[0]));
        a[1] = TYPE_OUT(unpack_high(d[0], d[0]));
        a[2] = TYPE_OUT(unpack_low (d[1], d[1]));
        a[3] = TYPE_OUT(unpack_high(d[1], d[1]));
        a[4] = TYPE_OUT(unpack_low (d[2], d[2]));
        a[5] = TYPE_OUT(unpack_high(d[2], d[2]));
        a[6] = TYPE_OUT(unpack_low (d[3], d[3]));
        a[7] = TYPE_OUT(unpack_high(d[3], d[3]));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 1, 4, 16>
{
    // 4 mask vectors of 32 bits to 1 mask vectors of 8 bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(sat_pack(
            sat_pack(Is32vec4(b[0]), Is32vec4(b[1])),
            sat_pack(Is32vec4(b[2]), Is32vec4(b[3]))));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 1, 8, 16>
{
    // 8 mask vectors of 64 bits to 1 mask vectors of 8 bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(sat_pack(
            sat_pack(
            Is32vec4(shuffle<1, 3, 1, 3>(Is8vec16(b[0]), Is8vec16(b[1]))),
            Is32vec4(shuffle<1, 3, 1, 3>(Is8vec16(b[2]), Is8vec16(b[3])))),
            sat_pack(
            Is32vec4(shuffle<1, 3, 1, 3>(Is8vec16(b[4]), Is8vec16(b[5]))),
            Is32vec4(shuffle<1, 3, 1, 3>(Is8vec16(b[6]), Is8vec16(b[7]))))));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 2, 4, 16>
{
    // 2 mask vectors of 32 bits to 1 mask vectors of 16 bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(sat_pack(Is32vec4(b[0]), Is32vec4(b[1])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 2, 8, 16>
{
    // 4 mask vectors of 64 bits to 1 mask vectors of 16 bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(sat_pack(
            Is32vec4(shuffle<1, 3, 1, 3>(Is64vec2(b[0]), Is64vec2(b[1]))),
            Is32vec4(shuffle<1, 3, 1, 3>(Is64vec2(b[2]), Is64vec2(b[3])))));
    }
};

template <typename TYPE_OUT, typename TYPE_IN,
typename ARRAY_OUT, typename ARRAY_IN>
struct cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN, 4, 8, 16>
{
    // 2 mask vectors of 64 bits to 1 mask vectors of 32 bits
    static void FORCEINLINE go(ARRAY_OUT & a, ARRAY_IN const & b)
    {
        a[0] = TYPE_OUT(shuffle<1, 3, 1, 3>(Is64vec2(b[0]), Is64vec2(b[1])));
    }
};

template <typename TYPE_OUT, typename TYPE_IN, typename ARRAY_OUT, typename ARRAY_IN>
FORCEINLINE void cast_mask(ARRAY_OUT & b, ARRAY_IN const & a, TYPE_OUT & dummy1, TYPE_IN const & dummy2)
{
    cast_mask_impl<TYPE_OUT, TYPE_IN, ARRAY_OUT, ARRAY_IN,
        sizeof(typename TYPE_OUT::SCALAR_TYPE),
        sizeof(typename TYPE_IN::SCALAR_TYPE),
        sizeof(TYPE_OUT)>::go(b, a);
}

// Usefull to do parallel transport of types of different size
template <typename ARRAY_OUT, typename ARRAY_IN>
FORCEINLINE void cast_mask(ARRAY_OUT & b, ARRAY_IN const & a)
{
    cast_mask(b, a, b[0], a[0]);
}

template <int SIZE>
struct bittest_impl
{
    template <typename TYPE>
    static FORCEINLINE int test(TYPE const & a, int bit)
    {
        return (a >> bit) & 1;
    }

    template <typename TYPE>
    static FORCEINLINE int testandset(TYPE & a, int bit)
    {
        int res = (a >> bit) & 1;
        a |= (1 << bit);
        return res;
    }

    template <typename TYPE>
    static FORCEINLINE int testandreset(TYPE & a, int bit)
    {
        int res = (a >> bit) & 1;
        a &= ~(1 << bit);
        return res;
    }
};

template <>
struct bittest_impl<4>
{
    template <typename TYPE>
    static FORCEINLINE int test(TYPE const & a, int bit)
    {
        return (int)_bittest((long const *)&a, (long)bit);
    }

    template <typename TYPE>
    static FORCEINLINE int testandset(TYPE & a, int bit)
    {
        return (int)_bittestandset((long *)&a, (long)bit);
    }

    template <typename TYPE>
    static FORCEINLINE int testandreset(TYPE & a, int bit)
    {
        return (int)_bittestandreset((long *)&a, (long)bit);
    }
};

#if PLATFORM_INTEL_ARCH >= 64

template <>
struct bittest_impl<8>
{
    template <typename TYPE>
    static FORCEINLINE int test(TYPE const & a, int bit)
    {
        return (int)_bittest64((long long const *)&a, (long long)bit);
    }

    template <typename TYPE>
    static FORCEINLINE int testandset(TYPE & a, int bit)
    {
        return (int)_bittestandset64((long long *)&a, (long long)bit);
    }

    template <typename TYPE>
    static FORCEINLINE int testandreset(TYPE & a, int bit)
    {
        return (int)_bittestandreset64((long long *)&a, (long long)bit);
    }
};

#endif

template <typename TYPE>
FORCEINLINE int bittest(TYPE const & a, int bit)
{
    return bittest_impl<sizeof(TYPE)>::test(a, bit);
}

template <typename TYPE>
FORCEINLINE int bittestandset(TYPE & a, int bit)
{
    return bittest_impl<sizeof(TYPE)>::testandset(a, bit);
}

template <typename TYPE>
FORCEINLINE int bittestandreset(TYPE & a, int bit)
{
    return bittest_impl<sizeof(TYPE)>::testandreset(a, bit);
}

FORCEINLINE uint32_t popcount(uint32_t a)
{
#ifdef PLATFORM_HAS_POPCNT
    return __popcnt(a);
#else
    static PLATFORM_ALIGNED(16) uint8_t POPCOUNT_4bit[16] = {
            /* 0 */ 0,
            /* 1 */ 1,
            /* 2 */ 1,
            /* 3 */ 2,
            /* 4 */ 1,
            /* 5 */ 2,
            /* 6 */ 2,
            /* 7 */ 3,
            /* 8 */ 1,
            /* 9 */ 2,
            /* a */ 2,
            /* b */ 3,
            /* c */ 2,
            /* d */ 3,
            /* e */ 3,
            /* f */ 4
    };

    return POPCOUNT_4bit[a&15]
         + POPCOUNT_4bit[(a>>4)&15]
         + POPCOUNT_4bit[(a>>8)&15] 
         + POPCOUNT_4bit[(a>>12)&15]
         + POPCOUNT_4bit[(a>>16)&15]
         + POPCOUNT_4bit[(a>>20)&15]
         + POPCOUNT_4bit[(a>>24)&15]
         + POPCOUNT_4bit[(a>>28)&15];
#endif
}

FORCEINLINE uint64_t popcount(uint64_t a)
{
#if defined(PLATFORM_HAS_POPCNT)
    return __popcnt64(a);
#else
    return popcount((uint32_t)(a >> 32LL)) + popcount((uint32_t)a);
#endif
}

// Static array that can be use with any vector type above
// ALWAYS PREFER THIS THAN DECLARING IXXvecY variable[N] !!
template <typename TYPE, int N>
struct VecArray
{
    typedef TYPE VECTOR_TYPE;
    typedef typename TYPE::NATIVE_VEC_TYPE NATIVE_VEC_TYPE;
#ifdef _DEBUG
    union
    {
        NATIVE_VEC_TYPE vec[N];
        typename TYPE::SCALAR_TYPE s[N*TYPE::SCALAR_COUNT];
        typename TYPE::SCALAR_TYPE t[N][TYPE::SCALAR_COUNT];
    };
#else
    NATIVE_VEC_TYPE vec[N];
#endif

    enum { COUNT = N };

#if 0
    struct ElemRef
    {
        NATIVE_VEC_TYPE * m_vec;

        FORCEINLINE operator TYPE() const
        {
            return TYPE(*m_vec);
        }

        FORCEINLINE operator NATIVE_VEC_TYPE &()
        {
            return *m_vec;
        }
        
        FORCEINLINE operator NATIVE_VEC_TYPE const & () const
        {
            return *m_vec;
        }

        FORCEINLINE ElemRef & operator= (TYPE const & c)
        {
            //memcpy(&m_vec, &c, sizeof(c));
            copy_native_vec(*m_vec, (NATIVE_VEC_TYPE const &)c);
            return (*this);
        }

        FORCEINLINE ElemRef & operator= (ElemRef const & ref)
        {
            copy_native_vec(*m_vec, ref.m_vec);
            return (*this);
        }

    private:
        FORCEINLINE ElemRef(NATIVE_VEC_TYPE & vec)
        : m_vec(&vec)
        {
        }
    };
#endif

    template <int I> FORCEINLINE TYPE & at() { return (TYPE &)(vec[I]); }
    template <int I> FORCEINLINE TYPE const at() const { return TYPE(vec[I]); }
    FORCEINLINE TYPE const operator[](int i) const { return TYPE   (vec[i]); }
    FORCEINLINE void set(int i, NATIVE_VEC_TYPE const & c) const { copy_native_vec(m_vec[i], c); }
    FORCEINLINE TYPE & operator[](int i) { return (TYPE &)(vec[i]); }
    //FORCEINLINE ElemRef operator[](int i)       { return ElemRef(vec[i]); }
};

// This array can hold mplex<TYPE> but also fake complex<TYPE> with only a real part (and sizeof(fake complex<TYPE>) == sizeof(TYPE)).
template <typename TYPE, typename _COMPLEX_TYPE, int N>
struct ComplexVecArray
{
    typedef _COMPLEX_TYPE COMPLEX_TYPE;
    enum { Factor = sizeof(COMPLEX_TYPE)/sizeof(TYPE) };
#ifdef _DEBUG
    union
    {
        typename TYPE::NATIVE_VEC_TYPE vec[Factor*N];
        COMPLEX_TYPE cvec[Factor*N];
        typename TYPE::SCALAR_TYPE s[Factor*N*TYPE::SCALAR_COUNT];
        typename TYPE::SCALAR_TYPE t[N][Factor][TYPE::SCALAR_COUNT];
    };
#else
    typename TYPE::NATIVE_VEC_TYPE vec[Factor*N];
#endif
    struct ElemRef
    {
        typename TYPE::NATIVE_VEC_TYPE & m_vec1;
        typename TYPE::NATIVE_VEC_TYPE & m_vec2;
        FORCEINLINE ElemRef(
            typename TYPE::NATIVE_VEC_TYPE & vec1,
            typename TYPE::NATIVE_VEC_TYPE & vec2)
        : m_vec1(vec1), m_vec2(vec2)
        {
        }

        FORCEINLINE operator COMPLEX_TYPE() const
        {
            return COMPLEX_TYPE(m_vec1, m_vec2);
        }

        FORCEINLINE ElemRef & operator= (COMPLEX_TYPE const & c)
        {
            m_vec1 = c.real();
            if (Factor > 1) m_vec2 = c.imag();
            return (*this);
        }
    };

    FORCEINLINE COMPLEX_TYPE const operator[](int i) const { return COMPLEX_TYPE(vec[Factor*i], vec[Factor*i+(Factor-1)]); }
    FORCEINLINE void Set(int i, COMPLEX_TYPE const & c) {
        vec[Factor*i  ] = c.real();
        vec[Factor*i+(Factor-1)] = c.imag();
    }
    FORCEINLINE ElemRef      operator[](int i)       { return ElemRef(vec[Factor*i], vec[Factor*i+(Factor-1)]); }
};

template <typename TYPE, int N>
struct RefVecArray
{
    typename TYPE::NATIVE_VEC_TYPE (& vec)[N];

    FORCEINLINE RefVecArray(typename TYPE::NATIVE_VEC_TYPE (& _vec)[N]) : vec(_vec) { }

    FORCEINLINE TYPE const operator[](int i) const { assert(i < N); return TYPE(vec[i]); }
    FORCEINLINE TYPE &  operator[](int i)       { assert(i < N); return (TYPE &)(vec[i]); }
private:
    RefVecArray & operator =(RefVecArray other);
};

template <typename TYPE, int N>
struct ConstRefVecArray
{
    typename TYPE::NATIVE_VEC_TYPE const (& vec)[N];

    FORCEINLINE ConstRefVecArray(typename TYPE::NATIVE_VEC_TYPE const (& _vec)[N]) : vec(_vec) { }

    FORCEINLINE TYPE const operator[](int i) const { assert(i < N); return TYPE   (vec[i]); }
};

template <typename TYPE, int N, int M>
struct VecArray2
{
#ifdef _DEBUG
    union
    {
        typename TYPE::NATIVE_VEC_TYPE vec[N][M];
        typename TYPE::SCALAR_TYPE s[TYPE::SCALAR_COUNT * N * M];
        typename TYPE::SCALAR_TYPE t[N][M][TYPE::SCALAR_COUNT];
    };
#else
    typename TYPE::NATIVE_VEC_TYPE vec[N][M];
#endif

    FORCEINLINE ConstRefVecArray<TYPE, M> operator[](int i) const { assert(i < N); return ConstRefVecArray<TYPE, M>(vec[i]); }
    FORCEINLINE      RefVecArray<TYPE, M> operator[](int i)       { assert(i < N); return      RefVecArray<TYPE, M>(vec[i]); }
};

template <typename VECTOR, bool IS_VECTORIAL>
struct log_impl
{
    FORCEINLINE VECTOR operator()(VECTOR const & in) const
    {
        typedef typename VECTOR::SCALAR_TYPE TYPE;
        typedef typename PXVEC_TYPE_FORSIZE(int32_t, VECTOR::SCALAR_COUNT) VECTOR_INT;
        // Work only with floats (F32vecX)

        VECTOR const one = VECTOR(1.0f);
        VECTOR const half = VECTOR(0.5f);
        VECTOR const minNormPos = VECTOR(VECTOR_INT(0x00800000));
        VECTOR const approx_log_p0 = 7.0376836292E-2f;
        VECTOR const approx_log_p1 = -1.1514610310E-1f;
        VECTOR const approx_log_p2 = 1.1676998740E-1f;
        VECTOR const approx_log_p3 = -1.2420140846E-1f;
        VECTOR const approx_log_p4 = 1.4249322787E-1f;
        VECTOR const approx_log_p5 = -1.6668057665E-1f;
        VECTOR const approx_log_p6 = 2.0000714765E-1f;
        VECTOR const approx_log_p7 = - 2.4999993993E-1f;
        VECTOR const approx_log_p8 = 3.3333331174E-1f;
        VECTOR const approx_log_q1 = -2.12194440e-4f;
        VECTOR const approx_log_q2 = 0.693359375f;

        VECTOR invalidMask = (in <= VECTOR::zero());

        VECTOR inNormalized = max(in, minNormPos);
        VECTOR_INT exponent = shift_right<23>(VECTOR_INT(inNormalized));

        VECTOR const invMantissaMask = VECTOR(VECTOR_INT(~0x7f800000));

        VECTOR frac = (inNormalized & invMantissaMask) | VECTOR(half);

        VECTOR underSqrthf = frac < VECTOR(0.707106781186547524f);
        VECTOR exponentF = convert<VECTOR>(exponent - VECTOR_INT(0x7f)) + andnot(underSqrthf, one);

        VECTOR x = (frac & underSqrthf) + frac - one;

        VECTOR z = x * x;

        VECTOR y = (((((((((approx_log_p0 * x + approx_log_p1) * x + approx_log_p2) * x + approx_log_p3) * x + approx_log_p4 * x) + approx_log_p5 * x) + approx_log_p6 * x) + approx_log_p7 * x) + approx_log_p8) * x * z
            + exponentF * approx_log_q1
            - z * half
            + x + exponentF * approx_log_q2) | invalidMask;

        return y;
    }
};

template <typename VECTOR>
struct log_impl<VECTOR, false>
{
    FORCEINLINE VECTOR operator()(VECTOR const & in) const
    {
        return std::log(in);
    }
};

template <typename VECTOR>
FORCEINLINE VECTOR log(VECTOR const & in)
{
    return log_impl<VECTOR, PXVEC_ISVECTYPE(VECTOR)>()(in);
}

template <typename VECTOR, bool IS_VECTORIAL>
struct exp_impl
{
    FORCEINLINE VECTOR operator()(VECTOR const & in) const
    {
        typedef typename VECTOR::SCALAR_TYPE TYPE;
        typedef typename PXVEC_TYPE_FORSIZE(int32_t, VECTOR::SCALAR_COUNT) VECTOR_INT;
        typedef typename PXVEC_TYPE_FORSIZE(TYPE, 8) VECTOR8;

//Bench done with RVTest.exe rev >~ 1089
//                             Function / Call Stack	INST_RETIRED.ANY	CPU_CLK_UNHALTED.THREAD	CPU_CLK_UNHALTED.REF_TSC	L1D_PEND_MISS.PENDING	OFFCORE_RESPONSE:request=DEMAND_RFO:response=L3_HIT.HITM_OTHER_CORE	CYCLE_ACTIVITY.STALLS_LDM_PENDING	CYCLE_ACTIVITY.CYCLES_NO_EXECUTE	CYCLE_ACTIVITY.STALLS_L1D_PENDING	OFFCORE_REQUESTS_OUTSTANDING.CYCLES_WITH_DEMAND_DATA_RD	OFFCORE_REQUESTS_OUTSTANDING.DEMAND_DATA_RD	MEM_LOAD_UOPS_RETIRED.L3_HIT_PS	MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HIT_PS	DTLB_LOAD_MISSES.STLB_HIT	DTLB_LOAD_MISSES.WALK_DURATION	DTLB_STORE_MISSES.STLB_HIT	MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HITM_PS	MEM_LOAD_UOPS_RETIRED.L1_MISS	RS_EVENTS.EMPTY_CYCLES	RESOURCE_STALLS.SB	DTLB_STORE_MISSES.WALK_DURATION	IDQ.MS_UOPS	ILD_STALL.LCP	UOPS_EXECUTED.CYCLES_GE_1_UOPS_EXEC	UOPS_EXECUTED.CYCLES_GE_2_UOPS_EXEC	UOPS_EXECUTED.CYCLES_GE_3_UOPS_EXEC	IDQ.ALL_MITE_CYCLES_4_UOPS	IDQ.ALL_MITE_CYCLES_ANY_UOPS	IDQ.ALL_DSB_CYCLES_4_UOPS	IDQ.ALL_DSB_CYCLES_ANY_UOPS	IDQ_UOPS_NOT_DELIVERED.CYCLES_0_UOPS_DELIV.CORE	BR_MISP_RETIRED.ALL_BRANCHES_PS	ICACHE.MISSES	IDQ_UOPS_NOT_DELIVERED.CORE	ITLB_MISSES.WALK_DURATION	L1D.REPLACEMENT	L2_LINES_IN.ALL	LD_BLOCKS.STORE_FORWARD	LD_BLOCKS_PARTIAL.ADDRESS_ALIAS	MACHINE_CLEARS.MASKMOV	MACHINE_CLEARS.MEMORY_ORDERING	MACHINE_CLEARS.SMC	MEM_UOPS_RETIRED.ALL_STORES_PS	MEM_UOPS_RETIRED.SPLIT_LOADS_PS	MEM_UOPS_RETIRED.SPLIT_STORES_PS	UOPS_ISSUED.ANY	UOPS_RETIRED.RETIRE_SLOTS	INT_MISC.RECOVERY_CYCLES	CPU_CLK_UNHALTED.THREAD_P	CYCLE_ACTIVITY.STALLS_L2_PENDING	MEM_LOAD_UOPS_RETIRED.L3_MISS_PS	ICACHE.IFETCH_STALL	LD_BLOCKS.NO_SR	MACHINE_CLEARS.COUNT	RS_EVENTS.EMPTY_END	IDQ.MS_SWITCHES	BACLEARS.ANY	DSB2MITE_SWITCHES.PENALTY_CYCLES	MEM_LOAD_UOPS_RETIRED.HIT_LFB	ITLB_MISSES.STLB_HIT	MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_MISS_PS	OTHER_ASSISTS.ANY_WB_ASSIST	Module	Function (Full)	Source File	Start Address
//PXVEC_EXP_COMPACTCONSTS OFF| Philips::Medisys::RealView60::ComputePixelVec	9,526,014,289	10,150,015,225	10,204,015,306	450,000,675	0	1,950,002,925	2,250,003,375	0	5,670,008,505	3,090,004,635	750,315	0	30,000,045	30,000,045	1,500,045	0	21,000,630	120,000,180	30,000,045	1,500,045	1,290,001,935	0	6,390,009,585	3,780,005,670	1,560,002,340	1,710,002,565	3,690,005,535	180,000,270	240,000,360	720,001,080	0	12,000,180	4,020,006,030	6,000,180	120,000,180	1,500,045	0	117,003,510	0	0	0	1,170,001,755	3,000,090	0	9,960,014,940	9,390,014,085	0	10,140,015,210	990,001,485	0	30,000,045	0	0	6,000,090	300,000,450	0	120,000,180	25,500,765	0	0	0	rvengine.dll	Philips::Medisys::RealView60::ComputePixelVec	RVEngine-Impl.inl	0x10028914
//PXVEC_EXP_COMPACTCONSTS ON | Philips::Medisys::RealView60::ComputePixelVec	13,308,019,962	12,880,019,320	13,530,020,295	270,000,405	0	2,190,003,285	2,280,003,420	60,000,090	8,100,012,150	3,390,005,085	1,500,630	0	30,000,045	0	9,000,270	0	15,000,450	270,000,405	660,000,990	3,000,090	1,200,001,800	0	10,380,015,570	5,580,008,370	2,010,003,015	2,220,003,330	4,980,007,470	60,000,090	210,000,315	600,000,900	0	9,000,135	4,920,007,380	0	90,000,135	22,500,675	0	27,000,810	0	0	0	1,950,002,925	12,000,360	0	17,550,026,325	16,740,025,110	0	12,000,018,000	990,001,485	0	90,000,135	1,500,045	0	24,000,360	360,000,540	0	180,000,270	19,500,585	0	0	0	rvengine.dll	Philips::Medisys::RealView60::ComputePixelVec	RVEngine-Impl.inl	0x100299bc

//PXVEC_EXP_COMPACTCONSTS OFF| Philips::Medisys::RealView40::ComputePixelVec	19,388,029,082	14,954,022,431	13,132,019,698	150,000,225	0	2,610,003,915	2,610,003,915	0	11,100,016,650	7,290,010,935	2,250,945	0	30,000,045	30,000,045	1,500,045	0	1,500,045	90,000,135	0	0	0	0	11,910,017,865	6,180,009,270	2,850,004,275	2,940,004,410	4,980,007,470	870,001,305	930,001,395	540,000,810	0	0	3,780,005,670	3,000,090	0	6,000,180	0	31,500,945	0	0	0	2,460,003,690	0	0	20,310,030,465	20,040,030,060	0	15,840,023,760	1,320,001,980	0	60,000,090	0	0	0	30,000,045	0	180,000,270	3,000,090	0	300,165	0	rvengine.dll	Philips::Medisys::RealView40::ComputePixelVec	RVEngine-Impl.inl	0x10031bf0
//PXVEC_EXP_COMPACTCONSTS ON | Philips::Medisys::RealView40::ComputePixelVec	23,550,035,325	17,716,026,574	15,880,023,820	60,000,090	0	2,070,003,105	2,550,003,825	30,000,045	11,820,017,730	8,010,012,015	750,315	0	0	0	3,000,090	0	4,500,135	150,000,225	30,000,045	1,500,045	30,000,045	0	14,610,021,915	8,340,012,510	3,540,005,310	3,690,005,535	6,780,010,170	810,001,215	1,050,001,575	720,001,080	0	6,000,090	4,680,007,020	1,500,045	30,000,045	6,000,180	0	49,501,485	0	0	0	3,510,005,265	1,500,045	0	26,310,039,465	25,830,038,745	0	17,520,026,280	1,020,001,530	0	60,000,090	0	0	3,000,045	0	0	150,000,225	12,000,360	0	300,165	3,000,090	rvengine.dll	Philips::Medisys::RealView40::ComputePixelVec	RVEngine-Impl.inl	0x100326f0

//#define PXVEC_EXP_COMPACTCONSTS
#ifdef PXVEC_EXP_COMPACTCONSTS
        VECTOR8 consts_exp_hi_lo_logef_c1_c2(88.3762626647949f, -88.3762626647949f, 1.44269504088896341f, 0.5f, 0.693359375f, -2.12194440e-4f, 0, 0);

        VECTOR x = min(extract<0>(consts_exp_hi_lo_logef_c1_c2), max(extract<1>(consts_exp_hi_lo_logef_c1_c2), in));

        /* express exp(x) as exp(g + n*log(2)) */
        VECTOR preflooredx = x * extract<2>(consts_exp_hi_lo_logef_c1_c2) + extract<3>(consts_exp_hi_lo_logef_c1_c2);
#else
        // Work only with floats (F32vecX)
        VECTOR const exp_hi = 88.3762626647949f;
        VECTOR const exp_lo =-88.3762626647949f;

        VECTOR const approx_LOG2EF = (TYPE)1.44269504088896341;
        VECTOR const approx_exp_C1 = (TYPE)0.693359375;
        VECTOR const approx_exp_C2 = (TYPE)-2.12194440e-4;

        VECTOR const approx_exp_p0 = (TYPE)1.9875691500e-4f;
        VECTOR const approx_exp_p1 = (TYPE)1.3981999507e-3f;
        VECTOR const approx_exp_p2 = (TYPE)8.3334519073e-3f;
        VECTOR const approx_exp_p3 = (TYPE)4.1665795894e-2f;
        VECTOR const approx_exp_p4 = (TYPE)1.6666665459e-1f;
        VECTOR const approx_exp_p5 = (TYPE)5.0000001201e-1f;

        VECTOR const  one = (TYPE)1.0f;

        VECTOR x = min(exp_hi, max(exp_lo, in));

        /* express exp(x) as exp(g + n*log(2)) */
        VECTOR preflooredx = x * approx_LOG2EF + VECTOR(0.5f);
#endif

        /* how to perform a floorf with SSE: just below */
#if PLATFORM_INTEL_SSE >= 40
        VECTOR flooredx = floor(preflooredx);
        VECTOR_INT intx = convert<VECTOR_INT>(flooredx);
#else
        VECTOR_INT intx = convert<VECTOR_INT>(preflooredx);
        VECTOR flooredx = convert<VECTOR>(intx);
        VECTOR fixFloorMask = (flooredx > preflooredx); // if rounding mode is not to floor
        intx += VECTOR_INT(fixFloorMask); // x = mask ? x - 1 : x;
        flooredx -= fixFloorMask & VECTOR(1.0f);
#endif


#ifdef PXVEC_EXP_COMPACTCONSTS
        x = x - flooredx * extract<4>(consts_exp_hi_lo_logef_c1_c2) - flooredx * extract<5>(consts_exp_hi_lo_logef_c1_c2);

        VECTOR8 consts_p0(1.9875691500e-4, 1.3981999507e-3, 8.3334519073e-3, 4.1665795894e-2, 1.6666665459e-1, 5.0000001201e-1, 1.0, 0);
        VECTOR y = ((((((extract<0>(consts_p0) * x + extract<1>(consts_p0)) * x + extract<2>(consts_p0)) * x + extract<3>(consts_p0)) * x + extract<4>(consts_p0)) * x
            + extract<5>(consts_p0)) * square(x)) + x + extract<6>(consts_p0);
#else
        x = x - flooredx * approx_exp_C1 - flooredx * approx_exp_C2;
        VECTOR y = ((((((approx_exp_p0 * x + approx_exp_p1) * x + approx_exp_p2) * x + approx_exp_p3) * x + approx_exp_p4) * x + approx_exp_p5) * square(x)) + x + one;
#endif

        VECTOR pow2n = VECTOR(shift_left<23>(intx + VECTOR_INT(0x7f)));

        return y * pow2n;
    }
};

template <typename VECTOR>
struct exp_impl<VECTOR, false>
{
    FORCEINLINE VECTOR operator()(VECTOR const & in) const
    {
        return std::exp(in);
    }
};

template <typename VECTOR>
FORCEINLINE VECTOR exp(VECTOR const & in)
{
    return exp_impl<VECTOR, PXVEC_ISVECTYPE(VECTOR)>()(in);
}

template <typename VECTOR>
FORCEINLINE VECTOR pow(VECTOR const & x, VECTOR const & y)
{
    return (x > VECTOR::zero()) & exp(y * log(x));
}

template <bool FAST, typename TYPE>
static FORCEINLINE TYPE atan(TYPE const & x)
{
    TYPE const range1 = (TYPE)2.414213562373095;
    TYPE const range2 = (TYPE)0.4142135623730950;
    TYPE const c9 = (TYPE)8.05374449538e-2;
    TYPE const c7 = (TYPE)1.38776856032e-1; 
    TYPE const c5 = (TYPE)1.99777106478e-1;
    TYPE const c3 = (TYPE)3.33329491539e-1;
    TYPE const epsilon = ::std::numeric_limits<typename TYPE::SCALAR_TYPE>::min();
    TYPE const one = (TYPE)1.0;
    TYPE const minus_one = (TYPE)-1.0;
    TYPE const pi_ov_2 = (TYPE)1.5707963267948966192313216916398;
    TYPE const pi_ov_4 = (TYPE)0.78539816339744830961566084581988;

    /* make argument positive and save the sign */
    TYPE sign = signbit(x);
    TYPE ax = x ^ sign;

    /* range reduction */
    TYPE mask1 = ax > range1;
    TYPE mask2 = ax > range2;
    TYPE notmask3 = mask2; 
    mask2 ^= mask1;

    TYPE z1 = minus_one * rcp(ax + epsilon);
    TYPE z2 = (ax - one) * rcp(ax + one);
    ax = (mask1 & z1) | (mask2 & z2) | andnot(notmask3, ax);

    TYPE y = (mask1 & pi_ov_2) | (mask2 & pi_ov_4);

    TYPE ax2 = ax * ax;
    if (FAST)
        y += (c5 * ax2 - c3) * ax2 * ax + ax;
    else
        y += (((c9 * ax2 - c7) * ax2 + c5) * ax2 - c3) * ax2 * ax + ax;

    y ^= sign;
    return y;
}

template <typename TYPE>
FORCEINLINE TYPE atan(TYPE const & x)
{
    return atan<false>(x);
}

template <typename TYPE>
FORCEINLINE TYPE fast_atan(TYPE const & x)
{
    return atan<true>(x);
}

template <bool FAST, typename TYPE>
FORCEINLINE TYPE atan2(TYPE const & y, TYPE const & x)
{
    TYPE const epsilon = ::std::numeric_limits<typename TYPE::SCALAR_TYPE>::min();
    TYPE sign = signbit(x);
    TYPE signy = signbit(y);
    TYPE xpos = x >= TYPE::zero();
    TYPE res = atan<FAST>(y * rcp(sign ^ (epsilon + x ^ sign)));
    TYPE const pi = (TYPE)3.1415926535897932384626433832795;
    
    //return select(xpos, res, res + signy ^ pi);
    return res + andnot(xpos, signy ^ pi);
}

template <typename TYPE>
FORCEINLINE TYPE atan2(TYPE const & y, TYPE const & x)
{
    return atan2<false>(y, x);
}

template <typename TYPE>
FORCEINLINE TYPE fast_atan2(TYPE const & y, TYPE const & x)
{
    return atan2<true>(y, x);
}

//using ::std::sin;
//using ::std::cos;
//using ::std::tan;
//using ::std::atan;
//using ::std::atan2;

template <typename TYPE>
FORCEINLINE TYPE sin(TYPE const & _x)
{ // any x
    TYPE const minus_cephes_DP1 = -0.78515625;
    TYPE const minus_cephes_DP2 = -2.4187564849853515625e-4;
    TYPE const minus_cephes_DP3 = -3.77489497744594108e-8;
    TYPE const sincof_p0 = -1.9515295891E-4;
    TYPE const sincof_p1 = 8.3321608736E-3;
    TYPE const sincof_p2 = -1.6666654611E-1;
    TYPE const coscof_p0 = 2.443315711809948E-005;
    TYPE const coscof_p1 = -1.388731625493765E-003;
    TYPE const coscof_p2 = 4.166664568298827E-002;
    TYPE const cephes_FOPI = 1.27323954473516; // 4 / M_PI

    TYPE xmm1, xmm2, xmm3, y;
    typedef PXVEC_TYPE_FORSIZE(int32_t, TYPE::SCALAR_COUNT) INT_TYPE;
    INT_TYPE emm0, emm2;

    /* take the absolute value */
    TYPE x = abs(_x);
    /* extract the sign bit (upper one) */
    TYPE sign_bit = signbit(_x);

    /* scale by 4/Pi */
    y = (x * cephes_FOPI);

    /* store the integer part of y in mm0 */
    emm2 = convert<INT_TYPE>(y);
    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 = (emm2 + INT_TYPE(1));
    emm2 = (emm2 & INT_TYPE(~1));
    y = convert<TYPE>(emm2);

    /* get the swap sign flag */
    emm0 = (emm2 & INT_TYPE(4));
    emm0 = (shift_left<29>(emm0));
    /* get the polynom selection mask
       there is one polynom for 0 <= x <= Pi/4
       and another one for Pi/4<x<=Pi/2

       Both branches will be computed.
       */
    emm2 = (emm2 & INT_TYPE(2));
    emm2 = (emm2 == INT_TYPE::zero());

    TYPE swap_sign_bit = TYPE(emm0);
    TYPE poly_mask = TYPE(emm2);
    sign_bit = (sign_bit ^ swap_sign_bit);

    /* The magic pass: "Extended precision modular arithmetic"
       x = ((x - y * DP1) - y * DP2) - y * DP3; */
    xmm1 = minus_cephes_DP1;
    xmm2 = minus_cephes_DP2;
    xmm3 = minus_cephes_DP3;
    xmm1 = (y * xmm1);
    xmm2 = (y * xmm2);
    xmm3 = (y * xmm3);
    x = (x + xmm1);
    x = (x + xmm2);
    x = (x + xmm3);

    /* Evaluate the first polynom  (0 <= x <= Pi/4) */
    y = coscof_p0;
    TYPE z = (x * x);

    y = (y * z);
    y = (y + coscof_p1);
    y = (y * z);
    y = (y + coscof_p2);
    y = (y * z);
    y = (y * z);
    TYPE tmp = (z * TYPE(0.5f));
    y = (y - tmp);
    y = (y + TYPE(1.0f));

    /* Evaluate the second polynom  (Pi/4 <= x <= 0) */
    TYPE y2 = sincof_p0;
    y2 = (y2 * z);
    y2 = (y2 * sincof_p1);
    y2 = (y2 * z);
    y2 = (y2 + sincof_p2);
    y2 = (y2 * z);
    y2 = (y2 * x);
    y2 = (y2 + x);

    /* select the correct result from the two polynoms */
    xmm3 = poly_mask;
    y2 = (poly_mask & y2); //, xmm3);
    y = andnot(xmm3, y);
    y = (y + y2);
    /* update the sign */
    y = (y ^ sign_bit);

    return y;
}

template <typename TYPE>
FORCEINLINE TYPE cos(TYPE const & _x)
{
    return sqrt(TYPE(1.0) - square(sin(_x)));
}

template <typename TYPE>
FORCEINLINE void sincos(TYPE & s, TYPE & c, TYPE const & x)
{ // any x
    s = sin(x);
    c = sqrt(TYPE(1.0) - square(s));
}

template <typename TYPE>
FORCEINLINE TYPE tan(TYPE const & x)
{ // any x
    TYPE s = sin(x);
    return s * rsqrt(TYPE(1.0) - square(s));
}

template <typename TYPE>
FORCEINLINE void prefetchw(TYPE ptr)
{
    _m_prefetchw((char const *)ptr);
}

template <typename TYPE>
FORCEINLINE void prefetch(TYPE ptr)
{
    _mm_prefetch((char const *)ptr, _MM_HINT_T2);
}

template <typename TYPE>
FORCEINLINE void prefetch_nt(TYPE ptr)
{
    _mm_prefetch((char const *)ptr, _MM_HINT_NTA);
}

template <typename VECTOR_TYPE>
FORCEINLINE VECTOR_TYPE is_pos(VECTOR_TYPE const & a)
{
    return a > VECTOR_TYPE::zero();
}

template <typename VECTOR_TYPE>
FORCEINLINE VECTOR_TYPE is_nonzero(VECTOR_TYPE const & a)
{
    return a != VECTOR_TYPE::zero();
}

template <typename VECTOR_TYPE>
FORCEINLINE VECTOR_TYPE is_neg(VECTOR_TYPE const & a)
{
    return a < VECTOR_TYPE::zero();
}

template <typename VECTOR_TYPE>
FORCEINLINE VECTOR_TYPE increment(VECTOR_TYPE const & a)
{
    if (std::numeric_limits<typename VECTOR_TYPE::SCALAR_TYPE>::is_integer)
        return a - VECTOR_TYPE::minus_one();
    else
        return a + VECTOR_TYPE(1.0);
}

template <typename VECTOR_TYPE>
FORCEINLINE VECTOR_TYPE decrement(VECTOR_TYPE const & a)
{
    if (std::numeric_limits<typename VECTOR_TYPE::SCALAR_TYPE>::is_integer)
        return a + VECTOR_TYPE::minus_one();
    else
        return a - VECTOR_TYPE(1.0);
}

// aligned array (on the heap, with _aligned_malloc)
template <typename TYPE, int ALIGN = PLATFORM_DEFAULT_ALIGN>
class al_array
{
    TYPE * ptr_;
    size_t size_;
public:
    FORCEINLINE size_t size() const { return size_; }
    FORCEINLINE operator TYPE *() { return ptr_; }
    FORCEINLINE operator TYPE const *() const { return ptr_; }

    FORCEINLINE al_array() : ptr_(NULL), size_(0)
    {
    }

    FORCEINLINE al_array(size_t s) : size_(s), ptr_((TYPE *)_aligned_malloc(s * sizeof(TYPE), PLATFORM_DEFAULT_ALIGN))
    {
        if (ptr_ == NULL) throw std::bad_alloc();
    }

    FORCEINLINE void resize(size_t newSize, bool noCopy = false)
    {
        if (newSize > size_)
        {
            al_array newArray(newSize);
            if (!noCopy) std::copy(ptr_, ptr_ + size_, newArray.ptr_);
            this->~al_array();
            ptr_ = newArray.ptr_; newArray.ptr_ = NULL;
            size_ = newArray.size_;
        }
    }

    FORCEINLINE ~al_array()
    {
        if (ptr_ != NULL)
        {
            _aligned_free(ptr_);
            ptr_ = NULL;
        }
    }
};

    }

    using namespace ::PXVecImpl;

    PX_TYPEUNSIGN_SETTYPE(Is64vec2, Iu64vec2)
    PX_TYPEUNSIGN_SETTYPE(Is32vec4, Iu32vec4)
    PX_TYPEUNSIGN_SETTYPE(Is16vec8, Iu16vec8)
    PX_TYPEUNSIGN_SETTYPE(Is8vec16, Iu8vec16)
    PX_TYPEUNSIGN_SETTYPE(Is64vec4, Iu64vec4)
    PX_TYPEUNSIGN_SETTYPE(Is32vec8, Iu32vec8)
    PX_TYPEUNSIGN_SETTYPE(Is16vec16, Iu16vec16)
    PX_TYPEUNSIGN_SETTYPE(Is8vec32, Iu8vec32)

}

namespace Philips
{
    namespace Medisys
    {
        namespace PXVec = ::PXVecImpl;
    }
}