/*
 * Copyright (c)2009-2014 Koninklijke Philips Electronics N.V.,
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

// To define a = b + c AND a += b
#define PXVEC_IMPLEMENT_BINARYOP(OP, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) {return SELF_TYPE(INTRIN(a, b)); }\
    FORCEINLINE SELF_TYPE & operator OP##=(SELF_TYPE const & a) { return *this = SELF_TYPE(INTRIN(*this, a)); }

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_COMPARE(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }

#define PXVEC_COMPARE_EMULATE_ONLYSTRICT() \
    friend FORCEINLINE SELF_TYPE operator <=(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (b > a); }\
    friend FORCEINLINE SELF_TYPE cmple(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (b > a); }\
    friend FORCEINLINE SELF_TYPE operator >=(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (b < a); }\
    friend FORCEINLINE SELF_TYPE cmpge(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (b < a); }

#define PXVEC_COMPARE_EMULATE_ONLYGT() \
    friend FORCEINLINE SELF_TYPE operator <(SELF_TYPE const & a, SELF_TYPE const & b) { return b > a; }\
    friend FORCEINLINE SELF_TYPE cmplt(SELF_TYPE const & a, SELF_TYPE const & b) { return b > a; }\
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_COMPARE_OPCODE(OP, FUN, INTRIN, OPCODE) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b, OPCODE)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b, OPCODE)); }

#define PXVEC_IMPLEMENT_COMPARE_REV(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b, a)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b, a)); }

#define PXVEC_IMPLEMENT_COMPARE_NOT(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ SELF_TYPE(INTRIN(a, b)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ SELF_TYPE(INTRIN(a, b)); }

// To define a = class::f()
#define PXVEC_IMPLEMENT_CONSTANT(FUN, INTRIN) \
    static FORCEINLINE SELF_TYPE FUN() { return SELF_TYPE(INTRIN()); }

// To define a = f(b)
#define PXVEC_IMPLEMENT_UNARYFUN(FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a) { return SELF_TYPE(INTRIN(a)); }

// To define a = f(b)
#define PXVEC_IMPLEMENT_UNARYFUN_CST(FUN, INTRIN, CST) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a) { return SELF_TYPE(INTRIN(a, CST)); }

// To define a = f(b, c)
#define PXVEC_IMPLEMENT_BINARYFUN(FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }

#define PXVEC_IMPLEMENT_SHIFTI(INTRIN_LEFT, INTRIN_RIGHT, INTRIN_RIGHT_LOG) \
    template <int i> friend FORCEINLINE SELF_TYPE shift_right_log(SELF_TYPE const & a) { return SELF_TYPE(INTRIN_RIGHT_LOG(a, i)); }\
    template <int i> friend FORCEINLINE SELF_TYPE shift_right(SELF_TYPE const & a) { return SELF_TYPE(INTRIN_RIGHT(a, i)); }\
    template <int i> friend FORCEINLINE SELF_TYPE shift_left (SELF_TYPE const & a) { return SELF_TYPE(INTRIN_LEFT(a, i)); }\
    friend FORCEINLINE SELF_TYPE operator>>(SELF_TYPE const & a, int const i) { return SELF_TYPE(INTRIN_RIGHT(a, i)); }\
    friend FORCEINLINE SELF_TYPE operator<<(SELF_TYPE const & a, int const i) { return SELF_TYPE(INTRIN_LEFT(a, i)); }\
    FORCEINLINE SELF_TYPE & operator>>=(int const i) { return *this = SELF_TYPE(INTRIN_RIGHT(*this, i)); }\
    FORCEINLINE SELF_TYPE & operator<<=(int const i) { return *this = SELF_TYPE(INTRIN_LEFT(*this, i)); }

#define PXVEC_IMPLEMENT_SHIFT(INTRIN_LEFT, INTRIN_RIGHT, INTRIN_RIGHT_LOG) \
    friend FORCEINLINE SELF_TYPE shift_right_log(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT_LOG(a, i)); }\
    friend FORCEINLINE SELF_TYPE shift_right(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT(a, i)); }\
    friend FORCEINLINE SELF_TYPE shift_left (SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_LEFT(a, i)); }\
    friend FORCEINLINE SELF_TYPE operator>>(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT(a, i)); }\
    friend FORCEINLINE SELF_TYPE operator<<(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_LEFT(a, i)); }\
    FORCEINLINE SELF_TYPE & operator>>=(SELF_TYPE const & i) { return *this = SELF_TYPE(INTRIN_RIGHT(*this, i)); }\
    FORCEINLINE SELF_TYPE & operator<<=(SELF_TYPE const & i) { return *this = SELF_TYPE(INTRIN_LEFT(*this, i)); }

#define PXVEC_IMPLEMENT_SHIFT_M256(INTRIN_LEFT, INTRIN_RIGHT, INTRIN_RIGHT_LOG) \
    friend FORCEINLINE SELF_TYPE shift_right_log(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT_LOG(a, i.half(0))); }\
    friend FORCEINLINE SELF_TYPE shift_right(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT(a, i.half(0))); }\
    friend FORCEINLINE SELF_TYPE shift_left (SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_LEFT(a, i.half(0))); }\
    friend FORCEINLINE SELF_TYPE operator>>(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT(a, i.half(0))); }\
    friend FORCEINLINE SELF_TYPE operator<<(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_LEFT(a, i.half(0))); }\
    FORCEINLINE SELF_TYPE & operator>>=(SELF_TYPE const & i) { return *this = SELF_TYPE(INTRIN_RIGHT(*this, i.half(0))); }\
    FORCEINLINE SELF_TYPE & operator<<=(SELF_TYPE const & i) { return *this = SELF_TYPE(INTRIN_LEFT(*this, i.half(0))); }

// To define a = b + c AND a += b
#define PXVEC_IMPLEMENT_2BINARYOP(OP, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a.half(0), b.half(0)), INTRIN(a.half(1), b.half(1))); }\
    FORCEINLINE SELF_TYPE & operator OP##=(SELF_TYPE const & a) { return *this = SELF_TYPE(INTRIN(half(0), a.half(0)), INTRIN(half(1), a.half(1))); }

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_2COMPARE(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a.half(0), b.half(0)), INTRIN(a.half(1), b.half(1))); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a.half(0), b.half(0)), INTRIN(a.half(1), b.half(1))); }

// To define a = b < c AND a = cmplt(b, c) but with the intrinsic being cmpge for instance
#define PXVEC_IMPLEMENT_2COMPARE_REV(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b.half(0), a.half(0)), INTRIN(b.half(1), a.half(1))); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b.half(0), a.half(0)), INTRIN(b.half(1), a.half(1))); }

// To define a = f(b)
#define PXVEC_IMPLEMENT_2UNARYFUN(FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a) { return SELF_TYPE(INTRIN(a.half(0)), INTRIN(a.half(1))); }

// To define a = f_c(b)
#define PXVEC_IMPLEMENT_2UNARYFUN_CST(FUN, INTRIN, C) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a) { return SELF_TYPE(INTRIN(a.half(0), C), INTRIN(a.half(1), C)); }

// To define a = f(b, c)
#define PXVEC_IMPLEMENT_2BINARYFUN(FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a.half(0), b.half(0)), INTRIN(a.half(1), b.half(1))); }

#define PXVEC_IMPLEMENT_2SHIFTI(INTRIN_LEFT, INTRIN_RIGHT, INTRIN_RIGHT_LOG) \
    template <int i> friend FORCEINLINE SELF_TYPE shift_right_log(SELF_TYPE const & a) { return SELF_TYPE(INTRIN_RIGHT_LOG(a.half(0), i), INTRIN_RIGHT_LOG(a.half(1), i)); }\
    template <int i> friend FORCEINLINE SELF_TYPE shift_right(SELF_TYPE const & a) { return SELF_TYPE(INTRIN_RIGHT(a.half(0), i), INTRIN_RIGHT(a.half(1), i)); }\
    template <int i> friend FORCEINLINE SELF_TYPE shift_left (SELF_TYPE const & a) { return SELF_TYPE(INTRIN_LEFT(a.half(0), i), INTRIN_LEFT(a.half(1), i)); }\
    friend FORCEINLINE SELF_TYPE operator>>(SELF_TYPE const & a, int const i) { return SELF_TYPE(INTRIN_RIGHT(a.half(0), i), INTRIN_RIGHT(a.half(1), i)); }\
    friend FORCEINLINE SELF_TYPE operator<<(SELF_TYPE const & a, int const i) { return SELF_TYPE(INTRIN_LEFT(a.half(0), i), INTRIN_LEFT(a.half(1), i)); }\
    FORCEINLINE SELF_TYPE & operator>>=(int const i) { return *this = SELF_TYPE(INTRIN_RIGHT(half(0), i), INTRIN_RIGHT(half(1), i)); }\
    FORCEINLINE SELF_TYPE & operator<<=(int const i) { return *this = SELF_TYPE(INTRIN_LEFT(half(0), i), INTRIN_LEFT(half(1), i)); }

// Here we ignore the higher part of i (the shifter register has the form [ 0 .... 0 0 0 i ]
#define PXVEC_IMPLEMENT_2SHIFT(INTRIN_LEFT, INTRIN_RIGHT, INTRIN_RIGHT_LOG) \
    friend FORCEINLINE SELF_TYPE shift_right_log(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT_LOG(a.half(0), i.half(0)), INTRIN_RIGHT_LOG(a.half(1), i.half(0))); }\
    friend FORCEINLINE SELF_TYPE shift_right(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT(a.half(0), i.half(0)), INTRIN_RIGHT(a.half(1), i.half(0))); }\
    friend FORCEINLINE SELF_TYPE shift_left (SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_LEFT(a.half(0), i.half(0)), INTRIN_LEFT(a.half(1), i.half(0))); }\
    friend FORCEINLINE SELF_TYPE operator>>(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_RIGHT(a.half(0), i.half(0)), INTRIN_RIGHT(a.half(1), i.half(0))); }\
    friend FORCEINLINE SELF_TYPE operator<<(SELF_TYPE const & a, SELF_TYPE const & i) { return SELF_TYPE(INTRIN_LEFT(a.half(0), i.half(0)), INTRIN_LEFT(a.half(1), i.half(0))); }\
    FORCEINLINE SELF_TYPE & operator>>=(SELF_TYPE const & i) { return *this = SELF_TYPE(INTRIN_RIGHT(half(0), i.half(0)), INTRIN_RIGHT(half(1), i.half(0))); }\
    FORCEINLINE SELF_TYPE & operator<<=(SELF_TYPE const & i) { return *this = SELF_TYPE(INTRIN_LEFT(half(0), i.half(0)), INTRIN_LEFT(half(1), i.half(0))); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128I_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_load_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE load_low<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_si128((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_stream_si128((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_si128((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(__m128i(a), mask, (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128F_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_load_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadu_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE load_low<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_ps((float *)ptr, __m128(a)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_stream_ps((float *)ptr, __m128(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_ps((float *)ptr, __m128(a)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, I128vec1(__m128(a))); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, I128vec1(__m128(a))); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(I128vec1(__m128(a)), I128vec1(__m128(mask)), (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128D_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_load_pd((double const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadu_pd((double const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE load_low<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_pd((double *)ptr, __m128d(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_pd((double *)ptr, __m128d(a)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_stream_pd((double *)ptr, __m128d(a)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, I128vec1(__m128d(a))); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, I128vec1(__m128d(a))); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(I128vec1(__m128d(a)), I128vec1(__m128d(mask)), (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256F_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_load_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_loadu_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_ps((float *)ptr, __m256(a)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_ps((float *)ptr, __m256(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_ps((float *)ptr, __m256(a)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_ps(ptr, _mm256_castps_si256(mask), __m256(a)); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256D_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_load_pd((double const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_loadu_pd((double const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_pd((double *)ptr, __m256d(a)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_pd((double *)ptr, __m256d(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_pd((double *)ptr, __m256d(a)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_pd(ptr, _mm256_castpd_si256(mask), __m256d(a)); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256I_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_load_si256((__m256i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_loadu_si256((__m256i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_si256((__m256i *)ptr, __m256i(a)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_si256((__m256i *)ptr, __m256i(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_si256((__m256i *)ptr, __m256i(a)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_emul(ptr, mask, a); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128I(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M128I_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M128F(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M128F_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M128D(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M128D_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)

#define PXVEC_IMPLEMENT_LOADSTORE_M256I(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M256I_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M256F(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M256F_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M256D(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M256D_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)

#if PLATFORM_INTEL_SSE >= 50

#define PXVEC_IMPLEMENT_LOADSTORE_2xM128X_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_load_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_loadu_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_ps((float *)ptr, __m256(F32vec8(a))); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_ps((float *)ptr, __m256(F32vec8(a))); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_ps((float *)ptr, __m256(F32vec8(a))); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(1)); }\
    template <> FORCEINLINE SELF_TYPE partial_mask<SELF_TYPE>(int count) { return SELF_TYPE(partial_mask<SELF_TYPE::HALF_TYPE>(__min(count, (SELF_TYPE::SCALAR_COUNT>>1))), partial_mask<SELF_TYPE::HALF_TYPE>(__max(0, count-(SELF_TYPE::SCALAR_COUNT>>1)))); }\
    template <> FORCEINLINE void store_partial<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, int count) { store_partial<SELF_TYPE::HALF_TYPE>(ptr, a.half(0), __min(count, (SELF_TYPE::SCALAR_COUNT>>1))); if (count > (SELF_TYPE::SCALAR_COUNT>>1)) store_partial<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), count-(SELF_TYPE::SCALAR_COUNT>>1)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_ps((float *)ptr, _mm256_castps_si256(F32vec8(mask)), __m256(F32vec8(a))); }

#else

#define PXVEC_IMPLEMENT_LOADSTORE_2xM128X_(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { SELF_TYPE res; res.set_half(0, loada<SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, loada<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); return res; }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { SELF_TYPE res; res.set_half(0, loadu<SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, loadu<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); return res; }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { SELF_TYPE res; res.set_half(0, lddqu<SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, lddqu<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); return res; }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storea<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); storea<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); storeu<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    template <> FORCEINLINE void streama<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { streama<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); streama<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(1)); }\
    template <> FORCEINLINE SELF_TYPE partial_mask<SELF_TYPE>(int count) { return SELF_TYPE(partial_mask<SELF_TYPE::HALF_TYPE>(__min(count, (SELF_TYPE::SCALAR_COUNT>>1))), partial_mask<SELF_TYPE::HALF_TYPE>(__max(0, count-(SELF_TYPE::SCALAR_COUNT>>1)))); }\
    template <> FORCEINLINE void store_partial<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, int count) { store_partial<SELF_TYPE::HALF_TYPE>(ptr, a.half(0), __min(count, (SELF_TYPE::SCALAR_COUNT>>1))); if (count > (SELF_TYPE::SCALAR_COUNT>>1)) store_partial<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), count-(SELF_TYPE::SCALAR_COUNT>>1)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { store_mask<SELF_TYPE::HALF_TYPE>(ptr, a.half(0), mask.half(0)); store_mask<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), mask.half(1)); }

#endif

#define PXVEC_IMPLEMENT_LOADSTORE_2xM128X(SELF_TYPE) \
    PXVEC_IMPLEMENT_LOADSTORE_2xM128X_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X_(SELF_TYPE, SCALAR_TYPE)\
    template <> FORCEINLINE SELF_TYPE load_scalar<SELF_TYPE>(SCALAR_TYPE const & a) { return SELF_TYPE(load_scalar<SELF_TYPE::HALF_TYPE>(a), SELF_TYPE::HALF_TYPE()); }\
    template <> FORCEINLINE SCALAR_TYPE store_scalar<SELF_TYPE>(SELF_TYPE const & a) { return (SCALAR_TYPE)store_scalar(a.half(0)); }

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X(SELF_TYPE)\
    PXVEC_IMPLEMENT_LOADSTORE_SCALAR_2xM128X_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I_(SELF_TYPE, SCALAR_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)\
    template <> FORCEINLINE SELF_TYPE load_scalar<SELF_TYPE>(SCALAR_TYPE const & a) { return SELF_TYPE(INTRIN_LOADSCALAR(a)); }\
    template <> FORCEINLINE SCALAR_TYPE store_scalar<SELF_TYPE>(SELF_TYPE const & a) { return (SCALAR_TYPE)INTRIN_STORESCALAR(a); }

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(SELF_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)\
    PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD_(SELF_TYPE, SCALAR_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)\
    template <> FORCEINLINE SELF_TYPE load_scalar<SELF_TYPE>(SCALAR_TYPE const & a) { return SELF_TYPE(INTRIN_LOADSCALAR(&a)); }\
    template <> FORCEINLINE SCALAR_TYPE store_scalar<SELF_TYPE>(SELF_TYPE const & a) { SCALAR_TYPE res; INTRIN_STORESCALAR(&res, a); return res; }

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD(SELF_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)\
    PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)

#define PXVEC_IMPLEMENT_UNARYMINUS() \
    FORCEINLINE SELF_TYPE operator -() const { return (zero() - *this); }

////////////////////////////////////////////////////////////////////////////////
// Constructor / data member macros
////////////////////////////////////////////////////////////////////////////////

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128_NOSCALAR(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD) \
    FORCEINLINE SELF_TYPE() { }\
    FORCEINLINE SELF_TYPE(__m128 const & a) { vec = FROM_PS(a); }\
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec = FROM_PD(a); }\
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec = FROM_EPI(a); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD, FROM_SCALAR) \
    FORCEINLINE SELF_TYPE() { }\
    FORCEINLINE SELF_TYPE(__m128 const & a) { vec = FROM_PS(a); }\
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec = FROM_PD(a); }\
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec = FROM_EPI(a); }\
    FORCEINLINE SELF_TYPE(SCALAR_TYPE const & a) { vec = FROM_SCALAR((SIGNED_SCALAR_TYPE)(a)); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(SELF_TYPE, , _mm_castps_si128, _mm_castpd_si128, FROM_SCALAR)

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128F(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(SELF_TYPE, _mm_castsi128_ps, , _mm_castpd_ps, FROM_SCALAR)

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128D(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(SELF_TYPE, _mm_castsi128_pd, _mm_castps_pd, , FROM_SCALAR)

/****
 * Cast 128 -> 256 may crash if the register is stored on the stack (will be aligned on 16 boundaries and expected to be on 32 bytes align)
#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256_NOSCALAR(SELF_TYPE, FROM_SI128, FROM_PS128, FROM_PD128, FROM_SI256, FROM_PS256, FROM_PD256, CAST128_256, INSERT128_256, CAST256_128, EXTRACT_128_256) \
    FORCEINLINE SELF_TYPE() { } \
    FORCEINLINE SELF_TYPE(__m128 const  & a) { vec = INSERT128_256(CAST128_256(FROM_PS128(a)), FROM_PS128(a), 1); } \
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec = INSERT128_256(CAST128_256(FROM_SI128(a)), FROM_SI128(a), 1); } \
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec = INSERT128_256(CAST128_256(FROM_PD128(a)), FROM_PD128(a), 1); } \
    FORCEINLINE SELF_TYPE(__m128 const  & a, __m128 const  & b) { vec = INSERT128_256(CAST128_256(FROM_PS128(a)), FROM_PS128(b), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128i const & a, __m128i const & b) { vec = INSERT128_256(CAST128_256(FROM_SI128(a)), FROM_SI128(b), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128d const & a, __m128d const & b) { vec = INSERT128_256(CAST128_256(FROM_PD128(a)), FROM_PD128(b), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128 const  (& a)[2]) { vec = INSERT128_256(CAST128_256(FROM_PS128(a[0])), FROM_PS128(a[1]), 0x1); }\
    FORCEINLINE SELF_TYPE(__m128i const (& a)[2]) { vec = INSERT128_256(CAST128_256(FROM_SI128(a[0])), FROM_SI128(a[1]), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128d const (& a)[2]) { vec = INSERT128_256(CAST128_256(FROM_PD128(a[0])), FROM_PD128(a[1]), 0x1); } \
    FORCEINLINE SELF_TYPE(__m256  const & a) { vec = FROM_PS256(a); } \
    FORCEINLINE SELF_TYPE(__m256i const & a) { vec = FROM_SI256(a); } \
    FORCEINLINE SELF_TYPE(__m256d const & a) { vec = FROM_PD256(a); } \
    FORCEINLINE HALF_TYPE half(int i) const { if (!(i & 1)) return HALF_TYPE(CAST256_128(vec)); else return HALF_TYPE(EXTRACT_128_256(vec, 1)); } \
    FORCEINLINE void set_half(int part, HALF_TYPE const & value) { if (part == 1) vec = INSERT128_256(vec, value, 1); else vec = INSERT128_256(vec, value, 0); } \
    static FORCEINLINE void set_half(NATIVE_VEC_TYPE & vec, int part, HALF_TYPE const & value) { if (part == 1) vec = INSERT128_256(vec, value, 1); else vec = INSERT128_256(vec, value, 0); }
 *
 **********/
#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256_NOSCALAR(SELF_TYPE, FROM_SI128, FROM_PS128, FROM_PD128, FROM_SI256, FROM_PS256, FROM_PD256, UNDEFINED_256, INSERT128_256, CAST256_128, EXTRACT_128_256) \
    FORCEINLINE SELF_TYPE() { } \
    FORCEINLINE SELF_TYPE(__m128 const  & a) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_PS128(a), 0), FROM_PS128(a), 1); } \
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_SI128(a), 0), FROM_SI128(a), 1); } \
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_PD128(a), 0), FROM_PD128(a), 1); } \
    FORCEINLINE SELF_TYPE(__m128 const  & a, __m128 const  & b) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_PS128(a), 0), FROM_PS128(b), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128i const & a, __m128i const & b) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_SI128(a), 0), FROM_SI128(b), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128d const & a, __m128d const & b) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_PD128(a), 0), FROM_PD128(b), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128 const  (& a)[2]) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_PS128(a[0]), 0), FROM_PS128(a[1]), 0x1); }\
    FORCEINLINE SELF_TYPE(__m128i const (& a)[2]) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_SI128(a[0]), 0), FROM_SI128(a[1]), 0x1); } \
    FORCEINLINE SELF_TYPE(__m128d const (& a)[2]) { vec = INSERT128_256(INSERT128_256(UNDEFINED_256(), FROM_PD128(a[0]), 0), FROM_PD128(a[1]), 0x1); } \
    FORCEINLINE SELF_TYPE(__m256  const & a) { vec = FROM_PS256(a); } \
    FORCEINLINE SELF_TYPE(__m256i const & a) { vec = FROM_SI256(a); } \
    FORCEINLINE SELF_TYPE(__m256d const & a) { vec = FROM_PD256(a); } \
    FORCEINLINE HALF_TYPE half(int i) const { if (!(i & 1)) return HALF_TYPE(CAST256_128(vec)); else return HALF_TYPE(EXTRACT_128_256(vec, 1)); } \
    FORCEINLINE void set_half(int part, HALF_TYPE const & value) { if (part == 1) vec = INSERT128_256(vec, value, 1); else vec = INSERT128_256(vec, value, 0); } \
    static FORCEINLINE void set_half(NATIVE_VEC_TYPE & vec, int part, HALF_TYPE const & value) { if (part == 1) vec = INSERT128_256(vec, value, 1); else vec = INSERT128_256(vec, value, 0); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256(SELF_TYPE, FROM_SI128, FROM_PS128, FROM_PD128, FROM_SI256, FROM_PS256, FROM_PD256, UNDEFINED_256, INSERT128_256, CAST256_128, EXTRACT_128_256, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256_NOSCALAR(SELF_TYPE, FROM_SI128, FROM_PS128, FROM_PD128, FROM_SI256, FROM_PS256, FROM_PD256, UNDEFINED_256, INSERT128_256, CAST256_128, EXTRACT_128_256) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a) { vec = FROM_SCALAR((SIGNED_SCALAR_TYPE)a); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256(SELF_TYPE, , _mm_castps_si128, _mm_castpd_si128, , _mm256_castps_si256, _mm256_castpd_si256, _mm256_undefined_si256, _mm256_insertf128_si256, _mm256_castsi256_si128, _mm256_extractf128_si256, FROM_SCALAR)

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256F(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256(SELF_TYPE, _mm_castsi128_ps, , _mm_castpd_ps, _mm256_castsi256_ps, , _mm256_castpd_ps, _mm256_undefined_ps, _mm256_insertf128_ps, _mm256_castps256_ps128, _mm256_extractf128_ps, FROM_SCALAR)

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256D(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256(SELF_TYPE, _mm_castsi128_pd, _mm_castps_pd, , _mm256_castsi256_pd, _mm256_castps_pd, , _mm256_undefined_pd, _mm256_insertf128_pd, _mm256_castpd256_pd128, _mm256_extractf128_pd, FROM_SCALAR)

#if PLATFORM_INTEL_SSE < 50

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256_TO_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD) \
    FORCEINLINE SELF_TYPE(__m128  const (& a)[2]) { vec[0] =  FROM_PS(a[0]); vec[1] =  FROM_PS(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128d const (& a)[2]) { vec[0] =  FROM_PD(a[0]); vec[1] =  FROM_PD(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128i const (& a)[2]) { vec[0] = FROM_EPI(a[0]); vec[1] = FROM_EPI(a[1]); }

#else

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256_TO_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD) \
    FORCEINLINE SELF_TYPE(__m128  const (& a)[2]) { vec[0] =  FROM_PS(a[0]); vec[1] =  FROM_PS(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128d const (& a)[2]) { vec[0] =  FROM_PD(a[0]); vec[1] =  FROM_PD(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128i const (& a)[2]) { vec[0] = FROM_EPI(a[0]); vec[1] = FROM_EPI(a[1]); }\
    FORCEINLINE SELF_TYPE(__m256 const & a)  { vec[0] = FROM_PS(_mm256_castps256_ps128(a)); vec[1] = FROM_PS(_mm256_extractf128_ps(a, 1)); } \
    FORCEINLINE SELF_TYPE(__m256d const & a) { vec[0] = FROM_PD(_mm256_castpd256_pd128(a)); vec[1] = FROM_PD(_mm256_extractf128_pd(a, 1)); } \
    FORCEINLINE SELF_TYPE(__m256i const & a) { vec[0] = FROM_EPI(_mm256_castsi256_si128(a)); vec[1] = FROM_EPI(_mm256_extractf128_si256(a, 1)); }

#endif

#define PXVEC_IMPLEMENT_2xM128_HALF() \
    FORCEINLINE HALF_TYPE half(int part) const { return HALF_TYPE(vec[part&1]); } \
    FORCEINLINE void set_half(int part, HALF_TYPE const & value) { vec[part&1] = value; } \
    static FORCEINLINE void set_half(NATIVE_VEC_TYPE & _vec, int part, HALF_TYPE const & value) { _vec[part&1] = value; }

#define PXVEC_IMPLEMENT_SCALAR_CONSTRUCTOR_2xM128(SELF_TYPE, FROM_SCALAR) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE const & a) { vec[0] = vec[1] = FROM_SCALAR((SIGNED_SCALAR_TYPE)(a)); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD, FROM_SCALAR) \
    FORCEINLINE SELF_TYPE() { } \
    FORCEINLINE SELF_TYPE(__m128  const & a, __m128  const & b) { vec[0] =  FROM_PS(a); vec[1] =  FROM_PS(b); } \
    FORCEINLINE SELF_TYPE(__m128d const & a, __m128d const & b) { vec[0] =  FROM_PD(a); vec[1] =  FROM_PD(b); } \
    FORCEINLINE SELF_TYPE(__m128i const & a, __m128i const & b) { vec[0] = FROM_EPI(a); vec[1] = FROM_EPI(b); } \
    FORCEINLINE SELF_TYPE(__m128  const & a) { vec[0] = vec[1] =  FROM_PS(a); } \
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec[0] = vec[1] =  FROM_PD(a); } \
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec[0] = vec[1] = FROM_EPI(a); } \
    PXVEC_IMPLEMENT_SCALAR_CONSTRUCTOR_2xM128(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256_TO_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD) \
    PXVEC_IMPLEMENT_2xM128_HALF()

#define PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(SELF_TYPE, , _mm_castps_si128, _mm_castpd_si128, FROM_SCALAR)

#define PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128F(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(SELF_TYPE, _mm_castsi128_ps, , _mm_castpd_ps, FROM_SCALAR)

#define PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128D(SELF_TYPE, FROM_SCALAR) \
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(SELF_TYPE, _mm_castsi128_pd, _mm_castps_pd, , FROM_SCALAR)

/////////////////////////////////////////////////////////////////////
// Scalar constructors
/////////////////////////////////////////////////////////////////////

////////////////////////////

#define PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S4(SELF_TYPE) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1) \
    { *this = SELF_TYPE(a0, a1, a0, a1); } \

#define PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S8(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S4(SELF_TYPE) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2, SCALAR_TYPE a3) \
    { *this = SELF_TYPE(a0, a1, a2, a3, a0, a1, a2, a3); } \

#define PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S16(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S8(SELF_TYPE) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2, SCALAR_TYPE a3, SCALAR_TYPE a4, SCALAR_TYPE a5, SCALAR_TYPE a6, SCALAR_TYPE a7) \
    { *this = SELF_TYPE(a0, a1, a2, a3, a4, a5, a6, a7, a0, a1, a2, a3, a4, a5, a6, a7); }

#define PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S32(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S16(SELF_TYPE) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2 , SCALAR_TYPE a3 , SCALAR_TYPE a4 , SCALAR_TYPE a5 , SCALAR_TYPE a6 , SCALAR_TYPE a7 , \
                          SCALAR_TYPE a8, SCALAR_TYPE a9, SCALAR_TYPE a10, SCALAR_TYPE a11, SCALAR_TYPE a12, SCALAR_TYPE a13, SCALAR_TYPE a14, SCALAR_TYPE a15) \
    { *this = SELF_TYPE(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15); }

////////////////////////////

#define PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S2(SELF_TYPE) \
    static FORCEINLINE SELF_TYPE zero_to_n() { return SELF_TYPE((SCALAR_TYPE)0, (SCALAR_TYPE)1); }

#define PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S4(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S4(SELF_TYPE) \
    static FORCEINLINE SELF_TYPE zero_to_n() { return SELF_TYPE((SCALAR_TYPE)0, (SCALAR_TYPE)1, (SCALAR_TYPE)2, (SCALAR_TYPE)3); }

#define PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S8(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S8(SELF_TYPE) \
    static FORCEINLINE SELF_TYPE zero_to_n() { return SELF_TYPE((SCALAR_TYPE)0, (SCALAR_TYPE)1, (SCALAR_TYPE)2, (SCALAR_TYPE)3, (SCALAR_TYPE)4, (SCALAR_TYPE)5, (SCALAR_TYPE)6, (SCALAR_TYPE)7); }

#define PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S16(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S16(SELF_TYPE) \
    static FORCEINLINE SELF_TYPE zero_to_n() { return SELF_TYPE((SCALAR_TYPE)0, (SCALAR_TYPE)1, (SCALAR_TYPE)2, (SCALAR_TYPE)3, (SCALAR_TYPE)4, (SCALAR_TYPE)5, (SCALAR_TYPE)6, (SCALAR_TYPE)7, \
                                                                (SCALAR_TYPE)8, (SCALAR_TYPE)9, (SCALAR_TYPE)10, (SCALAR_TYPE)11, (SCALAR_TYPE)12, (SCALAR_TYPE)13, (SCALAR_TYPE)14, (SCALAR_TYPE)15); }

#define PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S32(SELF_TYPE) \
    PXVEC_IMPLEMENT_CONSTRUCTOR_REP_S32(SELF_TYPE) \
    static FORCEINLINE SELF_TYPE zero_to_n() { return SELF_TYPE((SCALAR_TYPE)0, (SCALAR_TYPE)1, (SCALAR_TYPE)2, (SCALAR_TYPE)3, (SCALAR_TYPE)4, (SCALAR_TYPE)5, (SCALAR_TYPE)6, (SCALAR_TYPE)7, \
                                                                (SCALAR_TYPE)8, (SCALAR_TYPE)9, (SCALAR_TYPE)10, (SCALAR_TYPE)11, (SCALAR_TYPE)12, (SCALAR_TYPE)13, (SCALAR_TYPE)14, (SCALAR_TYPE)15, \
                                                                (SCALAR_TYPE)16, (SCALAR_TYPE)17, (SCALAR_TYPE)18, (SCALAR_TYPE)19, (SCALAR_TYPE)20, (SCALAR_TYPE)21, (SCALAR_TYPE)22, (SCALAR_TYPE)23, \
                                                                (SCALAR_TYPE)24, (SCALAR_TYPE)25, (SCALAR_TYPE)26, (SCALAR_TYPE)27, (SCALAR_TYPE)28, (SCALAR_TYPE)29, (SCALAR_TYPE)30, (SCALAR_TYPE)31); }

////////////////////////////

#define PXVEC_IMPLEMENT_CONSTRUCTOR_S2(SELF_TYPE, FROM_S2) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1) { vec = FROM_S2((SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S2(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_S4(SELF_TYPE, FROM_S4) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2, SCALAR_TYPE a3) { vec = FROM_S4((SIGNED_SCALAR_TYPE)a3, (SIGNED_SCALAR_TYPE)a2, (SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S4(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_S8(SELF_TYPE, FROM_S8) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2, SCALAR_TYPE a3, SCALAR_TYPE a4, SCALAR_TYPE a5, SCALAR_TYPE a6, SCALAR_TYPE a7) \
        { vec = FROM_S8((SIGNED_SCALAR_TYPE)a7, (SIGNED_SCALAR_TYPE)a6, (SIGNED_SCALAR_TYPE)a5, (SIGNED_SCALAR_TYPE)a4, (SIGNED_SCALAR_TYPE)a3, (SIGNED_SCALAR_TYPE)a2, (SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S8(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_S16(SELF_TYPE, FROM_S16) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2 , SCALAR_TYPE a3 , SCALAR_TYPE a4 , SCALAR_TYPE a5 , SCALAR_TYPE a6 , SCALAR_TYPE a7 , \
                          SCALAR_TYPE a8, SCALAR_TYPE a9, SCALAR_TYPE a10, SCALAR_TYPE a11, SCALAR_TYPE a12, SCALAR_TYPE a13, SCALAR_TYPE a14, SCALAR_TYPE a15) \
        { vec = FROM_S16((SIGNED_SCALAR_TYPE)a15, (SIGNED_SCALAR_TYPE)a14, (SIGNED_SCALAR_TYPE)a13, (SIGNED_SCALAR_TYPE)a12, (SIGNED_SCALAR_TYPE)a11, (SIGNED_SCALAR_TYPE)a10, (SIGNED_SCALAR_TYPE)a9, (SIGNED_SCALAR_TYPE)a8, \
                         (SIGNED_SCALAR_TYPE)a7 , (SIGNED_SCALAR_TYPE)a6 , (SIGNED_SCALAR_TYPE)a5 , (SIGNED_SCALAR_TYPE)a4 , (SIGNED_SCALAR_TYPE)a3 , (SIGNED_SCALAR_TYPE)a2 , (SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S16(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_S32(SELF_TYPE, FROM_S32) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0 , SCALAR_TYPE a1 , SCALAR_TYPE a2 , SCALAR_TYPE a3 , SCALAR_TYPE a4 , SCALAR_TYPE a5 , SCALAR_TYPE a6 , SCALAR_TYPE a7 , \
                          SCALAR_TYPE a8 , SCALAR_TYPE a9 , SCALAR_TYPE a10, SCALAR_TYPE a11, SCALAR_TYPE a12, SCALAR_TYPE a13, SCALAR_TYPE a14, SCALAR_TYPE a15, \
                          SCALAR_TYPE a16, SCALAR_TYPE a17, SCALAR_TYPE a18, SCALAR_TYPE a19, SCALAR_TYPE a20, SCALAR_TYPE a21, SCALAR_TYPE a22, SCALAR_TYPE a23, \
                          SCALAR_TYPE a24, SCALAR_TYPE a25, SCALAR_TYPE a26, SCALAR_TYPE a27, SCALAR_TYPE a28, SCALAR_TYPE a29, SCALAR_TYPE a30, SCALAR_TYPE a31) \
        { *this = FROM_S32((SIGNED_SCALAR_TYPE)a31, (SIGNED_SCALAR_TYPE)a30, (SIGNED_SCALAR_TYPE)a29, (SIGNED_SCALAR_TYPE)a28, (SIGNED_SCALAR_TYPE)a27, (SIGNED_SCALAR_TYPE)a26, (SIGNED_SCALAR_TYPE)a25, (SIGNED_SCALAR_TYPE)a24, \
                           (SIGNED_SCALAR_TYPE)a23, (SIGNED_SCALAR_TYPE)a22, (SIGNED_SCALAR_TYPE)a21, (SIGNED_SCALAR_TYPE)a20, (SIGNED_SCALAR_TYPE)a19, (SIGNED_SCALAR_TYPE)a18, (SIGNED_SCALAR_TYPE)a17, (SIGNED_SCALAR_TYPE)a16, \
                           (SIGNED_SCALAR_TYPE)a15, (SIGNED_SCALAR_TYPE)a14, (SIGNED_SCALAR_TYPE)a13, (SIGNED_SCALAR_TYPE)a12, (SIGNED_SCALAR_TYPE)a11, (SIGNED_SCALAR_TYPE)a10, (SIGNED_SCALAR_TYPE)a9 , (SIGNED_SCALAR_TYPE)a8 , \
                           (SIGNED_SCALAR_TYPE)a7 , (SIGNED_SCALAR_TYPE)a6 , (SIGNED_SCALAR_TYPE)a5 , (SIGNED_SCALAR_TYPE)a4 , (SIGNED_SCALAR_TYPE)a3 , (SIGNED_SCALAR_TYPE)a2 , (SIGNED_SCALAR_TYPE)a1 , (SIGNED_SCALAR_TYPE)a0 ); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S32(SELF_TYPE)

//////////////////////////

#define PXVEC_IMPLEMENT_CONSTRUCTOR_2xS2(SELF_TYPE, FROM_S2) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2, SCALAR_TYPE a3) { vec[0] = FROM_S2((SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); vec[1] = FROM_S2((SIGNED_SCALAR_TYPE)a3, (SIGNED_SCALAR_TYPE)a2); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S4(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_2xS4(SELF_TYPE, FROM_S4) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2, SCALAR_TYPE a3, SCALAR_TYPE a4, SCALAR_TYPE a5, SCALAR_TYPE a6, SCALAR_TYPE a7) { \
      vec[0] = FROM_S4((SIGNED_SCALAR_TYPE)a3, (SIGNED_SCALAR_TYPE)a2, (SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); \
      vec[1] = FROM_S4((SIGNED_SCALAR_TYPE)a7, (SIGNED_SCALAR_TYPE)a6, (SIGNED_SCALAR_TYPE)a5, (SIGNED_SCALAR_TYPE)a4); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S8(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_2xS8(SELF_TYPE, FROM_S8) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0, SCALAR_TYPE a1, SCALAR_TYPE a2 , SCALAR_TYPE a3 , SCALAR_TYPE a4 , SCALAR_TYPE a5 , SCALAR_TYPE a6 , SCALAR_TYPE a7 , \
                          SCALAR_TYPE a8, SCALAR_TYPE a9, SCALAR_TYPE a10, SCALAR_TYPE a11, SCALAR_TYPE a12, SCALAR_TYPE a13, SCALAR_TYPE a14, SCALAR_TYPE a15) { \
      vec[0] = FROM_S8((SIGNED_SCALAR_TYPE)a7 , (SIGNED_SCALAR_TYPE)a6 , (SIGNED_SCALAR_TYPE)a5 , (SIGNED_SCALAR_TYPE)a4 , (SIGNED_SCALAR_TYPE)a3 , (SIGNED_SCALAR_TYPE)a2 , (SIGNED_SCALAR_TYPE)a1, (SIGNED_SCALAR_TYPE)a0); \
      vec[1] = FROM_S8((SIGNED_SCALAR_TYPE)a15, (SIGNED_SCALAR_TYPE)a14, (SIGNED_SCALAR_TYPE)a13, (SIGNED_SCALAR_TYPE)a12, (SIGNED_SCALAR_TYPE)a11, (SIGNED_SCALAR_TYPE)a10, (SIGNED_SCALAR_TYPE)a9, (SIGNED_SCALAR_TYPE)a8); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S16(SELF_TYPE)

#define PXVEC_IMPLEMENT_CONSTRUCTOR_2xS16(SELF_TYPE, FROM_S16) \
    FORCEINLINE SELF_TYPE(SCALAR_TYPE a0 , SCALAR_TYPE a1 , SCALAR_TYPE a2 , SCALAR_TYPE a3 , SCALAR_TYPE a4 , SCALAR_TYPE a5 , SCALAR_TYPE a6 , SCALAR_TYPE a7 , \
                          SCALAR_TYPE a8 , SCALAR_TYPE a9 , SCALAR_TYPE a10, SCALAR_TYPE a11, SCALAR_TYPE a12, SCALAR_TYPE a13, SCALAR_TYPE a14, SCALAR_TYPE a15, \
                          SCALAR_TYPE a16, SCALAR_TYPE a17, SCALAR_TYPE a18, SCALAR_TYPE a19, SCALAR_TYPE a20, SCALAR_TYPE a21, SCALAR_TYPE a22, SCALAR_TYPE a23, \
                          SCALAR_TYPE a24, SCALAR_TYPE a25, SCALAR_TYPE a26, SCALAR_TYPE a27, SCALAR_TYPE a28, SCALAR_TYPE a29, SCALAR_TYPE a30, SCALAR_TYPE a31) { \
        vec[1] = FROM_S16((SIGNED_SCALAR_TYPE)a15, (SIGNED_SCALAR_TYPE)a14, (SIGNED_SCALAR_TYPE)a13, (SIGNED_SCALAR_TYPE)a12, (SIGNED_SCALAR_TYPE)a11, (SIGNED_SCALAR_TYPE)a10, (SIGNED_SCALAR_TYPE)a9 , (SIGNED_SCALAR_TYPE)a8 , \
                          (SIGNED_SCALAR_TYPE)a7 , (SIGNED_SCALAR_TYPE)a6 , (SIGNED_SCALAR_TYPE)a5 , (SIGNED_SCALAR_TYPE)a4 , (SIGNED_SCALAR_TYPE)a3 , (SIGNED_SCALAR_TYPE)a2 , (SIGNED_SCALAR_TYPE)a1 , (SIGNED_SCALAR_TYPE)a0); \
        vec[0] = FROM_S16((SIGNED_SCALAR_TYPE)a31, (SIGNED_SCALAR_TYPE)a30, (SIGNED_SCALAR_TYPE)a29, (SIGNED_SCALAR_TYPE)a28, (SIGNED_SCALAR_TYPE)a27, (SIGNED_SCALAR_TYPE)a26, (SIGNED_SCALAR_TYPE)a25, (SIGNED_SCALAR_TYPE)a24, \
                          (SIGNED_SCALAR_TYPE)a23, (SIGNED_SCALAR_TYPE)a22, (SIGNED_SCALAR_TYPE)a21, (SIGNED_SCALAR_TYPE)a20, (SIGNED_SCALAR_TYPE)a19, (SIGNED_SCALAR_TYPE)a18, (SIGNED_SCALAR_TYPE)a17, (SIGNED_SCALAR_TYPE)a16); } \
    PXVEC_IMPLEMENT_CONSTRUCTOR_GEN_S32(SELF_TYPE)

#define PXVEC_IMPLEMENT_BASICDEF(TEMPLATE_PREFIX,TYPENAME) \
    TEMPLATE_PREFIX friend SELF_TYPE min(SELF_TYPE const &, SELF_TYPE const &); \
    TEMPLATE_PREFIX friend SELF_TYPE max(SELF_TYPE const &, SELF_TYPE const &);

#ifndef _DEBUG
#define PXVEC_IMPLEMENT_DATA_GEN(TEMPLATE_PREFIX,TYPENAME,X) \
private: \
    NATIVE_VEC_TYPE vec; \
public: \
    enum { SCALAR_COUNT = sizeof(NATIVE_VEC_TYPE) / sizeof(SCALAR_TYPE) }; \
    typedef TYPENAME PX_TYPESIGN(SCALAR_TYPE) SIGNED_SCALAR_TYPE; \
    FORCEINLINE operator NATIVE_VEC_TYPE const & () const { return vec; } \
    FORCEINLINE SCALAR_TYPE const * scalars() const { return (SCALAR_TYPE const *)&vec; } \
    FORCEINLINE SCALAR_TYPE * scalars() { return (SCALAR_TYPE *)&vec; } \
    PXVEC_IMPLEMENT_BASICDEF(TEMPLATE_PREFIX,TYPENAME)
#else
#define PXVEC_IMPLEMENT_DATA_GEN(TEMPLATE_PREFIX,TYPENAME,X) \
    enum { SCALAR_COUNT = sizeof(NATIVE_VEC_TYPE) / sizeof(SCALAR_TYPE) }; \
    private: union { NATIVE_VEC_TYPE vec; SCALAR_TYPE s[SCALAR_COUNT]; }; public:\
    FORCEINLINE operator NATIVE_VEC_TYPE const & () const { return vec; } \
    FORCEINLINE SCALAR_TYPE const * scalars() const { return (SCALAR_TYPE const *)&vec; } \
    FORCEINLINE SCALAR_TYPE * scalars() { return (SCALAR_TYPE *)&vec; } \
public: \
    typedef TYPENAME PX_TYPESIGN(SCALAR_TYPE) SIGNED_SCALAR_TYPE; \
    PXVEC_IMPLEMENT_BASICDEF(TEMPLATE_PREFIX,TYPENAME)
#endif

#define PXVEC_IMPLEMENT_DATA_T(TEMPLATE_PREFIX) PXVEC_IMPLEMENT_DATA_GEN(TEMPLATE_PREFIX,typename,)
#define PXVEC_IMPLEMENT_DATA() PXVEC_IMPLEMENT_DATA_GEN(,,)

#define PXVEC_IMPLEMENT_UNPACK_GEN(SELF_TYPE, DOUBLE_SELF_TYPE, INTRIN_LOW, INTRIN_HIGH, POSTFIX)\
    FORCEINLINE SELF_TYPE unpack##POSTFIX##_low(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN_LOW(a, b)); }\
    FORCEINLINE SELF_TYPE unpack##POSTFIX##_high(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN_HIGH(a, b)); }\
    FORCEINLINE DOUBLE_SELF_TYPE unpack##POSTFIX (SELF_TYPE const & a, SELF_TYPE const & b) { return DOUBLE_SELF_TYPE(INTRIN_LOW(a, b), INTRIN_HIGH(a, b)); }

#define PXVEC_IMPLEMENT_UNPACK(SELF_TYPE, INTRIN_LOW, INTRIN_HIGH) \
    PXVEC_IMPLEMENT_UNPACK_GEN(SELF_TYPE, SELF_TYPE::DOUBLE_TYPE, INTRIN_LOW, INTRIN_HIGH, )

#define PXVEC_IMPLEMENT_UNPACK_HALVES(SELF_TYPE, INTRIN_LOW, INTRIN_HIGH) \
    PXVEC_IMPLEMENT_UNPACK_GEN(SELF_TYPE, SELF_TYPE::DOUBLE_TYPE, INTRIN_LOW, INTRIN_HIGH, _halves)

#define PXVEC_IMPLEMENT_UNPACK_HALVES2(SELF_TYPE) \
    FORCEINLINE SELF_TYPE unpack_halves_low(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(unpack_low(a.half(0), b.half(0)), unpack_low(a.half(1), b.half(1))); }\
    FORCEINLINE SELF_TYPE unpack_halves_high(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(unpack_high(a.half(0), b.half(0)), unpack_high(a.half(1), b.half(1))); }
    //FORCEINLINE DOUBLE_SELF_TYPE unpack_halves(SELF_TYPE const & a, SELF_TYPE const & b) { return DOUBLE_SELF_TYPE(unpack_low(a, b), unpack_high(a, b)); }

#define PXVEC_IMPLEMENT_UNPACK_CAST(SELF_TYPE, DOUBLE_SELF_TYPE, CAST_TYPE, INTRIN_LOW, INTRIN_HIGH)\
    FORCEINLINE SELF_TYPE unpack_low(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN_LOW(CAST_TYPE(a), CAST_TYPE(b))); }\
    FORCEINLINE SELF_TYPE unpack_high(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN_HIGH(CAST_TYPE(a), CAST_TYPE(b))); }\
    FORCEINLINE DOUBLE_SELF_TYPE unpack(SELF_TYPE const & a, SELF_TYPE const & b) { return DOUBLE_SELF_TYPE(SELF_TYPE(INTRIN_LOW(CAST_TYPE(a), CAST_TYPE(b))), SELF_TYPE(INTRIN_HIGH(CAST_TYPE(a), CAST_TYPE(b)))); }

#define PXVEC_IMPLEMENT_BASIC_OP_M128I() \
    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm_undefined_si128) \
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)        \
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)  \
    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)               \
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )               \
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)               \
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

#if PLATFORM_INTEL_SSE >= 60

#define PXVEC_IMPLEMENT_BASIC_OP_M256I() \
    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm256_undefined_si256)  \
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm256_setzero_si256)         \
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm256_minusone_si256)   \
    PXVEC_IMPLEMENT_BINARYOP(&, _mm256_and_si256)                \
    PXVEC_IMPLEMENT_BINARYOP(|, _mm256_or_si256)                 \
    PXVEC_IMPLEMENT_BINARYOP(^, _mm256_xor_si256)                \
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm256_andnot_si256)

#endif

#define PXVEC_IMPLEMENT_BASIC_OP_2xM128I() \
    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm_undefined_si128) \
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128) \
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128) \
    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128) \
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 ) \
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128) \
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)


template <typename HALF_TYPE>
struct VecDType;

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

typedef VecDType<Iu32vec8> Iu32vec16;
typedef VecDType<Iu32vec16> Iu32vec32;

struct I128vec1
{
    typedef I128vec1 SELF_TYPE;
    typedef __m128i  SCALAR_TYPE;
    typedef __m128i  NATIVE_VEC_TYPE;
    
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128_NOSCALAR(I128vec1, , _mm_castps_si128, _mm_castpd_si128)
    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_BASIC_OP_M128I()
};

#if PLATFORM_INTEL_SSE >= 50

struct I256vec1
{
    typedef I128vec1 HALF_TYPE;
    typedef I256vec1 SELF_TYPE;
    typedef __m256i  SCALAR_TYPE;
    typedef __m256i  NATIVE_VEC_TYPE;
    
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256_NOSCALAR(I256vec1, , _mm_castps_si128, _mm_castpd_si128, , _mm256_castps_si256, _mm256_castpd_si256, _mm256_undefined_si256, _mm256_insertf128_si256, _mm256_castsi256_si128, _mm256_extractf128_si256)
    PXVEC_IMPLEMENT_DATA()

#if PLATFORM_INTEL_SSE >= 60
    PXVEC_IMPLEMENT_BASIC_OP_M256I()
#endif

};

#endif

struct F64vec2
{
    typedef F64vec4 DOUBLE_TYPE;
    typedef F64vec2 SELF_TYPE;
    typedef double   SCALAR_TYPE;
    typedef __m128d  NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128D(F64vec2, _mm_set1_pd)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S2(F64vec2, _mm_set_pd)

    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm_undefined_pd)
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_pd)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_pd)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_pd)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_pd)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mul_pd)
    PXVEC_IMPLEMENT_BINARYOP(/, _mm_div_pd)
    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_pd)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_pd )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_pd)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_pd)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_pd)
    PXVEC_IMPLEMENT_COMPARE(!=, cmpneq, _mm_cmpneq_pd)
    PXVEC_IMPLEMENT_COMPARE(< , cmplt, _mm_cmplt_pd)
    PXVEC_IMPLEMENT_COMPARE(<=, cmple, _mm_cmple_pd)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_pd)
    PXVEC_IMPLEMENT_COMPARE(>=, cmpge, _mm_cmpge_pd)

    PXVEC_IMPLEMENT_UNARYFUN(sqrt , _mm_sqrt_pd )
    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm_abs_pd  )

    friend FORCEINLINE double sum(F64vec2 const & a) { return _mm_sum_pd(a); }

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_pd)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_pd)
};

struct F32vec4
{
    typedef F32vec8 DOUBLE_TYPE;
    typedef F32vec4 SELF_TYPE;
    typedef float   SCALAR_TYPE;
    typedef __m128  NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128F(F32vec4, _mm_set1_ps)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S4(F32vec4, _mm_set_ps)

    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm_undefined_ps)
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_ps)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_ps)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_ps)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_ps)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mul_ps)
    PXVEC_IMPLEMENT_BINARYOP(/, _mm_div_ps)
    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_ps)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_ps )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_ps)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_ps)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_ps)
    PXVEC_IMPLEMENT_COMPARE(!=, cmpneq, _mm_cmpneq_ps)
    PXVEC_IMPLEMENT_COMPARE(< , cmplt, _mm_cmplt_ps)
    PXVEC_IMPLEMENT_COMPARE(<=, cmple, _mm_cmple_ps)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_ps)
    PXVEC_IMPLEMENT_COMPARE(>=, cmpge, _mm_cmpge_ps)

#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_UNARYFUN_CST(floor, _mm_round_ps, _MM_FROUND_FLOOR)
    PXVEC_IMPLEMENT_UNARYFUN_CST(ceil , _mm_round_ps, _MM_FROUND_CEIL)
#endif
    PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm_rsqrt_ps)
    PXVEC_IMPLEMENT_UNARYFUN(sqrt , _mm_sqrt_ps )
    PXVEC_IMPLEMENT_UNARYFUN(rcp  , _mm_rcp_ps  )
    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm_abs_ps  )
    PXVEC_IMPLEMENT_UNARYFUN(signbit, _mm_signbit_ps  )

    friend FORCEINLINE float sum(F32vec4 const & a) { return _mm_sum_ps(a); }
    friend FORCEINLINE F64vec2 sum02_13(F32vec4 const & a);

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_ps)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_ps)
};

struct Iu64vec2
{
    typedef Iu64vec2 SELF_TYPE;
    typedef Iu64vec4 DOUBLE_TYPE;
    typedef uint64_t SCALAR_TYPE;
    typedef __m128i  NATIVE_VEC_TYPE;
    
    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Iu64vec2, _mm_set1_epi64_int64)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S2(Iu64vec2, _mm_set_epi64_int64)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()
    
#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi64)
#endif

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi64)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi64)

    PXVEC_IMPLEMENT_UNARYMINUS()

    friend FORCEINLINE int64_t sum(Iu64vec2 const & a) {
        return _mm_sum_epi64(a);
    }
};

struct Is64vec2
{
    typedef Is64vec2 SELF_TYPE;
    typedef Is64vec4 DOUBLE_TYPE;
    typedef int64_t  SCALAR_TYPE;
    typedef __m128i  NATIVE_VEC_TYPE;
    
    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Is64vec2, _mm_set1_epi64_int64)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S2(Is64vec2, _mm_set_epi64_int64)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi64)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi64)

#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi64)
#if PLATFORM_INTEL_SSE >= 42
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_epi64)
    PXVEC_COMPARE_EMULATE_ONLYGT()
#endif
#endif

    PXVEC_IMPLEMENT_UNARYMINUS()

    friend FORCEINLINE int64_t sum(Is64vec2 const & a) {
        return _mm_sum_epi64(a);
    }
};

struct Iu32vec4
{
    typedef Iu32vec4 SELF_TYPE;
    typedef Iu32vec8 DOUBLE_TYPE;
    typedef uint32_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Iu32vec4, _mm_set1_epi32)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S4(Iu32vec4, _mm_set_epi32)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi32)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi32)

    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epu32)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epu32)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi32, _mm_srli_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi32, _mm_srl_epi32, _mm_srl_epi32)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi32)

    friend FORCEINLINE uint32_t sum(Iu32vec4 const & a) { return _mm_sum_epi32(a); }

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epu32)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epu32)
};

struct Is32vec4
{
    typedef Is32vec4 SELF_TYPE;
    typedef Is32vec8 DOUBLE_TYPE;
    typedef int32_t  SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Is32vec4, _mm_set1_epi32)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S4(Is32vec4, _mm_set_epi32)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm_hadd_epi32)
#endif
    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi32)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi32)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epi32)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epi32)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi32 , _mm_sra_epi32 , _mm_srl_epi32)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi32)
    PXVEC_IMPLEMENT_COMPARE(< , cmplt, _mm_cmplt_epi32)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_epi32)
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm_abs_epi32)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epi32)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epi32)

    friend FORCEINLINE int32_t sum(Is32vec4 const & a) { return _mm_sum_epi32(a); }

    /* Multiply a0 and a2 with b0 and b2 */
    friend FORCEINLINE Is64vec2 mul_even(SELF_TYPE const & a, SELF_TYPE const & b)
    {
        return Is64vec2(_mm_mul_epi32(a, b));
    }
};

struct Iu16vec8
{
    typedef Iu16vec8 SELF_TYPE;
    typedef Iu16vec16 DOUBLE_TYPE;
    typedef uint16_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Iu16vec8, _mm_set1_epi16)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S8(Iu16vec8, _mm_set_epi16)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi16)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi16)

    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm_adds_epu16)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm_subs_epu16)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi16)

    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high , _mm_mulhi_epu16)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi16, _mm_srli_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi16 , _mm_srl_epi16, _mm_srl_epi16)

#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epu16)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epu16)
#endif
};

struct Is16vec8
{
    typedef Is16vec8 SELF_TYPE;
    typedef Is16vec16 DOUBLE_TYPE;
    typedef int16_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Is16vec8, _mm_set1_epi16)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S8(Is16vec8, _mm_set_epi16)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi16)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi16)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm_hadd_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(horz_sat_add, _mm_hadds_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(horz_sub, _mm_hsub_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(horz_sat_sub, _mm_hsubs_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm_adds_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm_subs_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_hrs, _mm_mulhrs_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high , _mm_mulhi_epi16)

    friend FORCEINLINE Is32vec4 mul_add(SELF_TYPE const & a, SELF_TYPE const & b) { return Is32vec4(_mm_madd_epi16(a, b)); };

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi16, _mm_srai_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi16 , _mm_sra_epi16, _mm_srl_epi16)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi16)
    PXVEC_IMPLEMENT_COMPARE(< , cmplt, _mm_cmplt_epi16)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_epi16)
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

    friend FORCEINLINE SELF_TYPE operator != (SELF_TYPE const & a, SELF_TYPE const & b)
    {
        return ((a == b) == zero());
    }

    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm_abs_epi16)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(avg, _mm_avg_epi16)
};

struct Iu8vec16
{
    typedef Iu8vec16 SELF_TYPE;
    typedef Iu8vec32 DOUBLE_TYPE;
    typedef uint8_t  SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Iu8vec16, _mm_set1_epi8)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S16(Iu8vec16, _mm_set_epi8)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi8, _mm_srli_epi8, _mm_srli_epi8)
    //PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi8 , _mm_srl_epi8)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi8)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm_adds_epu8)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm_subs_epu8)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epu8)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epu8)
};

struct Is8vec16
{
    typedef Is8vec16 SELF_TYPE;
    typedef Is8vec32 DOUBLE_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128I(Is8vec16, _mm_set1_epi8)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S16(Is8vec16, _mm_set_epi8)

    PXVEC_IMPLEMENT_BASIC_OP_M128I()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi8)
    PXVEC_IMPLEMENT_COMPARE(< , cmplt, _mm_cmplt_epi8)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_epi8)
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm_adds_epi8)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm_subs_epi8)
    PXVEC_IMPLEMENT_UNARYFUN (abs    , _mm_abs_epi8 )
    
    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epi8)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epi8)
};

#if PLATFORM_INTEL_SSE >= 60

struct Iu64vec4
{
    typedef Iu64vec4 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu64vec2 HALF_TYPE;
    typedef uint64_t SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Iu64vec4, _mm256_set1_epi64x)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S4(Iu64vec4, _mm256_set_epi64x)
    
    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi64)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi64)

    PXVEC_IMPLEMENT_UNARYMINUS()
};

struct Is64vec4
{
    typedef Is64vec4 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is64vec2 HALF_TYPE;
    typedef int64_t  SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Is64vec4, _mm256_set1_epi64x)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S4(Is64vec4, _mm256_set_epi64x)
    
    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi64)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi64)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq , _mm256_cmpeq_epi64)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt , _mm256_cmpgt_epi64)
    PXVEC_COMPARE_EMULATE_ONLYGT()

    friend FORCEINLINE int64_t sum(Is64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Iu32vec8
{
    typedef Iu32vec8 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu32vec4 HALF_TYPE;
    typedef uint32_t SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Iu32vec8, _mm256_set1_epi32)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S8(Iu32vec8, _mm256_set_epi32)

    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi32)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi32)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm256_mullo_epi32)

    PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi32, _mm256_srli_epi32, _mm256_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi32, _mm256_srl_epi32, _mm256_srl_epi32)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_epu32)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_epu32)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm256_cmpeq_epi32)

    friend FORCEINLINE uint32_t sum(Iu32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is32vec8
{
    typedef Is32vec8 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is32vec4 HALF_TYPE;
    typedef int32_t  SCALAR_TYPE;
    typedef __m256i  NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Is32vec8, _mm256_set1_epi32)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S8(Is32vec8, _mm256_set_epi32)

    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi32)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi32)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm256_mullo_epi32)
    PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm256_hadd_epi32)
    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm256_mullo_epi32)
    //PXVEC_IMPLEMENT_BINARYFUN(mul_high , _mm256_mulhi_epi32)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq , _mm256_cmpeq_epi32)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt , _mm256_cmpgt_epi32)
    PXVEC_COMPARE_EMULATE_ONLYGT()

    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm256_abs_epi32)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_epi32)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_epi32)

    PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi32, _mm256_srai_epi32, _mm256_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi32 , _mm256_sra_epi32 , _mm256_srl_epi32)

    friend FORCEINLINE int32_t sum(Is32vec8 const & a) { return _mm_sum_epi32(a.half(0)) + _mm_sum_epi32(a.half(1)); }
};

struct Iu16vec16
{
    typedef Iu16vec16 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu16vec8 HALF_TYPE;
    typedef uint16_t SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Iu16vec16, _mm256_set1_epi16)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S16(Iu16vec16, _mm256_set_epi16)

    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi16)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm256_adds_epu16)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm256_subs_epu16)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_epu16)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_epu16)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm256_cmpeq_epi16)

    PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi16, _mm256_srli_epi16, _mm256_srli_epi16)
    PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi16 , _mm256_srl_epi16 , _mm256_srl_epi16)
};

struct Is16vec16
{
    typedef Is16vec16 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is16vec8  HALF_TYPE;
    typedef int16_t SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Is16vec16, _mm256_set1_epi16)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S16(Is16vec16, _mm256_set_epi16)

    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi16)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi16)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm256_mullo_epi16)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm256_adds_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm256_subs_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_hrs, _mm256_mulhrs_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm256_mullo_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high , _mm256_mulhi_epi16)

    PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi16, _mm256_srai_epi16, _mm256_srli_epi16)
    PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi16 , _mm256_sra_epi16 , _mm256_srl_epi16)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm256_cmpeq_epi16)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm256_cmpgt_epi16)
    PXVEC_COMPARE_EMULATE_ONLYGT()

    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm256_abs_epi16)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(avg, _mm256_avg_epi16)
};

struct Iu8vec32
{
    typedef Iu8vec32 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu8vec16 HALF_TYPE;
    typedef uint8_t  SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Iu8vec32, _mm256_set1_epi8)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S32(Iu8vec32, _mm256_set_epi8)

    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm256_cmpeq_epi8)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi8)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi8)
    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm256_adds_epu8)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm256_subs_epu8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi32, _mm256_srli_epi32, _mm256_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi32, _mm256_srl_epi32, _mm256_srl_epi32)
};

struct Is8vec32
{
    typedef Is8vec32 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is8vec16 HALF_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Is8vec32, _mm256_set1_epi8)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S32(Is8vec32, _mm256_set_epi8)

    PXVEC_IMPLEMENT_BASIC_OP_M256I()

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm256_cmpeq_epi8)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm256_cmpgt_epi8)
    PXVEC_COMPARE_EMULATE_ONLYGT()

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi8)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm256_adds_epi8)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm256_subs_epi8)
    PXVEC_IMPLEMENT_UNARYFUN (abs    , _mm256_abs_epi8 )

    PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi32, _mm256_srai_epi32, _mm256_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi32, _mm256_sra_epi32, _mm256_srl_epi32)
};

#else

struct Iu64vec4
{
    typedef Iu64vec4 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu64vec2 HALF_TYPE;
    typedef uint64_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu64vec4, _mm_set1_epi64_int64)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS2(Iu64vec4, _mm_set_epi64_int64)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()
    
#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_2COMPARE(==, cmpeq, _mm_cmpeq_epi64)
#endif

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi64)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi64)

    PXVEC_IMPLEMENT_UNARYMINUS()

    friend FORCEINLINE int64_t sum(Iu64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is64vec4
{
    typedef Is64vec4 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is64vec2 HALF_TYPE;
    typedef int64_t  SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Is64vec4, _mm_set1_epi64_int64)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS2(Is64vec4, _mm_set_epi64_int64)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi64)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi64)
    
#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_2COMPARE(==, cmpeq, _mm_cmpeq_epi64)
#if PLATFORM_INTEL_SSE >= 42
    PXVEC_IMPLEMENT_2COMPARE(> , cmpgt, _mm_cmpgt_epi64)
    PXVEC_COMPARE_EMULATE_ONLYGT()
#endif
#endif

    PXVEC_IMPLEMENT_UNARYMINUS()

    friend FORCEINLINE int64_t sum(Is64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Iu32vec8
{
    typedef Iu32vec8 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu32vec4 HALF_TYPE;
    typedef uint32_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu32vec8, _mm_set1_epi32)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS4(Iu32vec8, _mm_set_epi32)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mullo_epi32)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi32)

	PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epu32)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epu32)

    friend FORCEINLINE uint32_t sum(Iu32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is32vec8
{
    typedef Is32vec8 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is32vec4 HALF_TYPE;
    typedef int32_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Is32vec8, _mm_set1_epi32)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS4(Is32vec8, _mm_set_epi32)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mullo_epi32)
#if PLATFORM_INTEL_SSE >= 40
    PXVEC_IMPLEMENT_2BINARYFUN(horz_add, _mm_hadd_epi32)
#endif

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(mul_low , _mm_mullo_epi32)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_high , _mm_mulhi_epi32)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi32)
    PXVEC_IMPLEMENT_2COMPARE    (< , cmplt, _mm_cmplt_epi32)
    PXVEC_IMPLEMENT_2COMPARE    (> , cmpgt, _mm_cmpgt_epi32)
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_epi32)

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epi32)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epi32)

#if PLATFORM_INTEL_SSE >= 40
    friend FORCEINLINE Is32vec4 horz_add(SELF_TYPE const & a) { return horz_add(a.half(0), a.half(1)); }
#endif

    friend FORCEINLINE int32_t sum(Is32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
    friend Is16vec8 sat_pack(SELF_TYPE const & a);
};

struct Iu16vec16
{
    typedef Iu16vec16 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu16vec8 HALF_TYPE;
    typedef uint16_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu16vec16, _mm_set1_epi16)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS8(Iu16vec16, _mm_set_epi16)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2COMPARE(==, cmpeq, _mm_cmpeq_epi16)

    PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epu16)
    PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epu16)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi16)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi16, _mm_srli_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi16 , _mm_srl_epi16 , _mm_srl_epi16)
};

struct Is16vec16
{
    typedef Is16vec16 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is16vec8  HALF_TYPE;
    typedef int16_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Is16vec16, _mm_set1_epi16)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS8(Is16vec16, _mm_set_epi16)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi16)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mullo_epi16)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_hrs, _mm_mulhrs_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_low , _mm_mullo_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_high , _mm_mulhi_epi16)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi16, _mm_srai_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi16 , _mm_sra_epi16 , _mm_srl_epi16)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi16)
    PXVEC_IMPLEMENT_2COMPARE    (< , cmplt, _mm_cmplt_epi16)
    PXVEC_IMPLEMENT_2COMPARE    (> , cmpgt, _mm_cmpgt_epi16)
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_epi16)

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(avg, _mm_avg_epi16)
};

struct Iu8vec32
{
    typedef Iu8vec32 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu8vec16 HALF_TYPE;
    typedef uint8_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu8vec32, _mm_set1_epi8)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS16(Iu8vec32, _mm_set_epi8)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi8)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epu8)
    PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epu8)
};

struct Is8vec32
{
    typedef Is8vec32 SELF_TYPE;
    typedef VecDType<SELF_TYPE> DOUBLE_TYPE;
    typedef Is8vec16 HALF_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Is8vec32, _mm_set1_epi8)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS16(Is8vec32, _mm_set_epi8)

    PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi8)
    PXVEC_IMPLEMENT_2COMPARE    (< , cmplt, _mm_cmplt_epi8)
    PXVEC_IMPLEMENT_2COMPARE    (> , cmpgt, _mm_cmpgt_epi8)
    PXVEC_COMPARE_EMULATE_ONLYSTRICT()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epi8)
    PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epi8)
    PXVEC_IMPLEMENT_2UNARYFUN (abs    , _mm_abs_epi8 )
};

#endif

#if PLATFORM_INTEL_SSE >= 50

struct F64vec4
{
    typedef VecDType<F64vec4> DOUBLE_TYPE;
    typedef F64vec4 SELF_TYPE;
    typedef F64vec2 HALF_TYPE;
    typedef double  SCALAR_TYPE;
    typedef __m256d NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256D(F64vec4, _mm256_set1_pd)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S4(F64vec4, _mm256_set_pd)

    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm256_undefined_pd)
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm256_setzero_pd)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm256_minusone_pd)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_pd)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_pd)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm256_mul_pd)
    PXVEC_IMPLEMENT_BINARYOP(/, _mm256_div_pd)
    PXVEC_IMPLEMENT_BINARYOP(&, _mm256_and_pd)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm256_or_pd )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm256_xor_pd)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm256_andnot_pd)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE_OPCODE(==, cmpeq , _mm256_cmp_pd, _CMP_EQ_OQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(!=, cmpneq, _mm256_cmp_pd, _CMP_NEQ_UQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(< , cmplt , _mm256_cmp_pd, _CMP_LT_OQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(<=, cmple , _mm256_cmp_pd, _CMP_LE_OS)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(> , cmpgt , _mm256_cmp_pd, _CMP_NLE_US)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(>=, cmpge , _mm256_cmp_pd, _CMP_NLT_US)

    //PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm256_rsqrt_pd)
    PXVEC_IMPLEMENT_UNARYFUN(sqrt , _mm256_sqrt_pd )
    //PXVEC_IMPLEMENT_UNARYFUN(rcp  , _mm256_rcp_pd  )
    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm256_abs_pd  )
    //PXVEC_IMPLEMENT_UNARYFUN(signbit, _mm256_signbit_pd)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_pd)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_pd)

    //friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

struct F32vec8
{
    typedef VecDType<F32vec8> DOUBLE_TYPE;
    typedef F32vec8 SELF_TYPE;
    typedef F32vec4 HALF_TYPE;
    typedef float   SCALAR_TYPE;
    typedef __m256  NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256F(F32vec8, _mm256_set1_ps)
    PXVEC_IMPLEMENT_CONSTRUCTOR_S8(F32vec8, _mm256_set_ps)

    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm256_undefined_ps)
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm256_setzero_ps)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm256_minusone_ps)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_ps)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_ps)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm256_mul_ps)
    PXVEC_IMPLEMENT_BINARYOP(/, _mm256_div_ps)
    PXVEC_IMPLEMENT_BINARYOP(&, _mm256_and_ps)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm256_or_ps )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm256_xor_ps)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm256_andnot_ps)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE_OPCODE(==, cmpeq , _mm256_cmp_ps, _CMP_EQ_OQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(!=, cmpneq, _mm256_cmp_ps, _CMP_NEQ_UQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(< , cmplt , _mm256_cmp_ps, _CMP_LT_OQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(<=, cmple , _mm256_cmp_ps, _CMP_LE_OS)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(> , cmpgt , _mm256_cmp_ps, _CMP_NLE_US)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(>=, cmpge , _mm256_cmp_ps, _CMP_NLT_US)

    PXVEC_IMPLEMENT_UNARYFUN(ceil , _mm256_ceil_ps )
    PXVEC_IMPLEMENT_UNARYFUN(floor, _mm256_floor_ps)
    PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm256_rsqrt_ps)
    PXVEC_IMPLEMENT_UNARYFUN(sqrt , _mm256_sqrt_ps )
    PXVEC_IMPLEMENT_UNARYFUN(rcp  , _mm256_rcp_ps  )
    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm256_abs_ps  )
    PXVEC_IMPLEMENT_UNARYFUN(signbit, _mm256_signbit_ps)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_ps)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_ps)

    friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

#else

struct F32vec8
{
    typedef VecDType<F32vec8> DOUBLE_TYPE;
    typedef F32vec8 SELF_TYPE;
    typedef F32vec4 HALF_TYPE;
    typedef float   SCALAR_TYPE;
    typedef __m128  NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128F(F32vec8, _mm_set1_ps)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS4(F32vec8, _mm_set_ps)

    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm_undefined_ps)
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_ps)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_ps)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_ps)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_ps)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mul_ps)
    PXVEC_IMPLEMENT_2BINARYOP(/, _mm_div_ps)
    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_ps)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_ps )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_ps)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_ps)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2COMPARE(==, cmpeq, _mm_cmpeq_ps)
    PXVEC_IMPLEMENT_2COMPARE(!=, cmpneq, _mm_cmpneq_ps)
    PXVEC_IMPLEMENT_2COMPARE(< , cmplt, _mm_cmplt_ps)
    PXVEC_IMPLEMENT_2COMPARE(<=, cmple, _mm_cmple_ps)
    PXVEC_IMPLEMENT_2COMPARE(> , cmpgt, _mm_cmpgt_ps)
    PXVEC_IMPLEMENT_2COMPARE(>=, cmpge, _mm_cmpge_ps)

    PXVEC_IMPLEMENT_2UNARYFUN_CST(floor, _mm_round_ps, _MM_FROUND_FLOOR)
    PXVEC_IMPLEMENT_2UNARYFUN_CST(ceil , _mm_round_ps, _MM_FROUND_CEIL)

    PXVEC_IMPLEMENT_2UNARYFUN(rsqrt, _mm_rsqrt_ps)
    PXVEC_IMPLEMENT_2UNARYFUN(sqrt , _mm_sqrt_ps )
    PXVEC_IMPLEMENT_2UNARYFUN(rcp  , _mm_rcp_ps  )
    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_ps  )

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_ps)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_ps)

    friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

struct F64vec4
{
    typedef VecDType<F64vec4> DOUBLE_TYPE;
    typedef F64vec4 SELF_TYPE;
    typedef F64vec2 HALF_TYPE;
    typedef double  SCALAR_TYPE;
    typedef __m128d NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128D(F64vec4, _mm_set1_pd)
    PXVEC_IMPLEMENT_CONSTRUCTOR_2xS2(F64vec4, _mm_set_pd)

    PXVEC_IMPLEMENT_CONSTANT(undefined, _mm_undefined_pd)
    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_pd)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_pd)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_pd)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_pd)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mul_pd)
    PXVEC_IMPLEMENT_2BINARYOP(/, _mm_div_pd)
    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_pd)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_pd )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_pd)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_pd)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2COMPARE(==, cmpeq, _mm_cmpeq_pd)
    PXVEC_IMPLEMENT_2COMPARE(!=, cmpneq, _mm_cmpneq_pd)
    PXVEC_IMPLEMENT_2COMPARE(< , cmplt, _mm_cmplt_pd)
    PXVEC_IMPLEMENT_2COMPARE(<=, cmple, _mm_cmple_pd)
    PXVEC_IMPLEMENT_2COMPARE(> , cmpgt, _mm_cmpgt_pd)
    PXVEC_IMPLEMENT_2COMPARE(>=, cmpge, _mm_cmpge_pd)

    //PXVEC_IMPLEMENT_2UNARYFUN(rsqrt, _mm_rsqrt_pd)
    PXVEC_IMPLEMENT_2UNARYFUN(sqrt , _mm_sqrt_pd )
    //PXVEC_IMPLEMENT_2UNARYFUN(rcp  , _mm_rcp_pd  )
    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_pd  )

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_pd)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_pd)

    //friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

#endif

// Used to do: vec = other.vec, no matter if vec is __m128i, __m128i[2], __m128i[2][2], etc...
template <typename TYPE>
FORCEINLINE void copy_native_vec(TYPE & out, TYPE const & in)
{
    out = in;
}

template <typename HALF_TYPE>
FORCEINLINE void copy_native_vec(HALF_TYPE (& out)[2], HALF_TYPE const (& in)[2])
{
    copy_native_vec(out[0], in[0]);
    copy_native_vec(out[1], in[1]);
}

template <typename HALF_TYPE>
struct VecDType
{
    typedef VecDType<HALF_TYPE> SELF_TYPE;
    typedef HALF_TYPE HALF_TYPE;
    typedef typename HALF_TYPE::SCALAR_TYPE SCALAR_TYPE;
    typedef typename HALF_TYPE::NATIVE_VEC_TYPE NATIVE_VEC_TYPE[2];
    typedef typename HALF_TYPE::NATIVE_VEC_TYPE HNATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA_T(template <typename HALF_TYPE>)
private:
    /*
    template <typename HNATIVE_VEC_TYPE>
    FORCEINLINE void set_half_native(HNATIVE_VEC_TYPE & dest, HNATIVE_VEC_TYPE const & src, HNATIVE_VEC_TYPE)
    {
        dest = src;
    }

    template <int ARRAY_SIZE, typename QNATIVE_VEC_TYPE>
    FORCEINLINE void set_half_native(HNATIVE_VEC_TYPE & dest, HNATIVE_VEC_TYPE const & src, QNATIVE_VEC_TYPE const (&)[ARRAY_SIZE])
    {
        set_half_native(dest[0], src[0], src[0]);
        set_half_native(dest[1], src[1], src[1]);
    }
    */
public:
//
//// To define a = b < c AND a = cmplt(b, c) but with the intrinsic being cmpge for instance
//#define PXVEC_IMPLEMENT_2COMPARE_REV(OP, FUN, INTRIN) \
//    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b.half(0), a.half(0)), INTRIN(b.half(1), a.half(1))); }\
//    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b.half(0), a.half(0)), INTRIN(b.half(1), a.half(1))); }

    FORCEINLINE void set_half(int i, HALF_TYPE const & half)
    {
		copy_native_vec(vec[i][0], ((HNATIVE_VEC_TYPE const &)half)[0]);
		copy_native_vec(vec[i][1], ((HNATIVE_VEC_TYPE const &)half)[1]);
    }
    
    static FORCEINLINE void set_half(NATIVE_VEC_TYPE & vec, int i, HALF_TYPE const & half)
    {
        copy_native_vec(vec[i], (HNATIVE_VEC_TYPE const &)half);
    }

    FORCEINLINE HALF_TYPE half(int i) const
    {
        return HALF_TYPE(vec[i & 1]);
    }

    FORCEINLINE VecDType() { }
    FORCEINLINE VecDType(typename HALF_TYPE::NATIVE_VEC_TYPE const & a) { set_half(0, a); set_half(1, a); }
    FORCEINLINE VecDType(typename HALF_TYPE::NATIVE_VEC_TYPE const & a, typename HALF_TYPE::NATIVE_VEC_TYPE const & b) { set_half(0, a); set_half(1, b); }
    FORCEINLINE VecDType(NATIVE_VEC_TYPE const & a) { set_half(0, a[0]); set_half(1, a[1]); }
    
// To define a = b + c AND a += b
#define PXVEC_IMPLEMENT_VecDType_BINARYOP(OP) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(a.half(0) OP b.half(0), a.half(1) OP b.half(1)); }\
    FORCEINLINE SELF_TYPE & operator OP##=(SELF_TYPE const & a) { return *this = SELF_TYPE(a.half(0) OP b.half(0), a.half(1) OP b.half(1)); }

    PXVEC_IMPLEMENT_VecDType_BINARYOP(+)
    PXVEC_IMPLEMENT_VecDType_BINARYOP(-)
    PXVEC_IMPLEMENT_VecDType_BINARYOP(/)
    PXVEC_IMPLEMENT_VecDType_BINARYOP(*)

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_VecDType_COMPARE(OP) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(a.half(0) OP b.half(0), a.half(1) OP b.half(1)); }\

    PXVEC_IMPLEMENT_VecDType_COMPARE(==)
    PXVEC_IMPLEMENT_VecDType_COMPARE(!=)
    PXVEC_IMPLEMENT_VecDType_COMPARE(<=)
    PXVEC_IMPLEMENT_VecDType_COMPARE(>=)
    PXVEC_IMPLEMENT_VecDType_COMPARE(<)
    PXVEC_IMPLEMENT_VecDType_COMPARE(>)
    PXVEC_IMPLEMENT_VecDType_COMPARE(&)
    PXVEC_IMPLEMENT_VecDType_COMPARE(|)
    PXVEC_IMPLEMENT_VecDType_COMPARE(^)
};

FORCEINLINE Is8vec16 sat_pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_packs_epi16(a, b); }
FORCEINLINE Is8vec16 sat_pack(Is16vec16 const & a) { return _mm_packs_epi16(a.half(0), a.half(1)); }
FORCEINLINE Iu8vec16 usat_pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_packus_epi16(a, b); }
FORCEINLINE Iu8vec16 usat_pack(Is16vec16 const & a) { return _mm_packus_epi16(a.half(0), a.half(1)); }
FORCEINLINE Is8vec16 pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_pack_epi16(a, b); }
FORCEINLINE Is8vec16 pack(Is16vec16 const & a) { return _mm_pack_epi16(a.half(0), a.half(1)); }

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

FORCEINLINE Is16vec8 pack(Is32vec8 const & a) { return _mm_pack_epi32(a.half(0), a.half(1)); }
FORCEINLINE Is16vec8 pack(Is32vec4 const & a, Is32vec4 const & b) { return _mm_packs_epi32(a, b); }

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
TYPE loadu(typename TYPE::SCALAR_TYPE const * ptr);

template <typename TYPE>
TYPE lddqu(typename TYPE::SCALAR_TYPE const * ptr);

template <typename TYPE>
TYPE loada(typename TYPE::SCALAR_TYPE const * ptr);

template <typename TYPE>
TYPE load_low(typename TYPE::SCALAR_TYPE const * ptr);

template <typename TYPE>
TYPE load_scalar(typename TYPE::SCALAR_TYPE const & ptr);

template <typename TYPE>
FORCEINLINE TYPE load_partial_zero(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT] = {};
    for (int i = 0; i < count; i++)
        tmp[i] = ptr[i];
    return loada<TYPE>(tmp);
}

template <typename TYPE>
FORCEINLINE TYPE load_partial(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
    for (int i = 0; i < count; i++)
        tmp[i] = ptr[i];
    return loada<TYPE>(tmp);
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE TYPE load(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    if (count == TYPE::SCALAR_COUNT)
        return load<ALIGNED, TYPE>(ptr);
    PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
    for (int i = 0; i < count; i++)
        tmp[i] = ptr[i];
    return loada<TYPE>(tmp);
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
    PLATFORM_ALIGNED(16) typename TYPE::SCALAR_TYPE clamped[TYPE::SCALAR_COUNT];
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

template <typename TYPE>
void storea(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec);

template <typename TYPE>
typename TYPE::SCALAR_TYPE store_scalar(TYPE const & vec);

#define PXVEC_LEFTMASK_32(N) (N <= 0 ? 0x00000000 : (N == 1 ? 0x000000ff : (N == 2 ? 0x0000ffff : (N == 3 ? 0x00ffffff : 0xffffffff))))
#define PXVEC_LEFTMASK_128(N) \
    _mm_set_epi32( \
        PXVEC_LEFTMASK_32(N - 12), \
        PXVEC_LEFTMASK_32(N - 8), \
        PXVEC_LEFTMASK_32(N - 4), \
        PXVEC_LEFTMASK_32(N))
#define PXVEC_LEFTMASK_256(N) \
    _mm256_set_epi32(             \
        PXVEC_LEFTMASK_32(N-28), \
        PXVEC_LEFTMASK_32(N-24), \
        PXVEC_LEFTMASK_32(N-20), \
        PXVEC_LEFTMASK_32(N-16), \
        PXVEC_LEFTMASK_32(N-12), \
        PXVEC_LEFTMASK_32(N-8), \
        PXVEC_LEFTMASK_32(N-4), \
        PXVEC_LEFTMASK_32(N))

template <typename TYPE, int SIZE>
struct partial_mask_impl
{
    FORCEINLINE TYPE operator () (int count) const;
};

#if 1

template <typename TYPE>
struct partial_mask_impl<TYPE, 16>
{
    FORCEINLINE TYPE operator () (int count) const
    {
        TYPE result = _mm_minusone_si128();
        for (; count < TYPE::SCALAR_COUNT; count++)
            result = shiftreg_right<1>(result);
        return result;
    }
};

#else

template <typename TYPE>
struct partial_mask_impl<TYPE, 16>
{
    FORCEINLINE TYPE operator () (int count) const
    {
        static __m128i const left_mask_128[17] = {
            PXVEC_LEFTMASK_128(0),
            PXVEC_LEFTMASK_128(1),
            PXVEC_LEFTMASK_128(2),
            PXVEC_LEFTMASK_128(3),
            PXVEC_LEFTMASK_128(4),
            PXVEC_LEFTMASK_128(5),
            PXVEC_LEFTMASK_128(6),
            PXVEC_LEFTMASK_128(7),
            PXVEC_LEFTMASK_128(8),
            PXVEC_LEFTMASK_128(9),
            PXVEC_LEFTMASK_128(10),
            PXVEC_LEFTMASK_128(11),
            PXVEC_LEFTMASK_128(12),
            PXVEC_LEFTMASK_128(13),
            PXVEC_LEFTMASK_128(14),
            PXVEC_LEFTMASK_128(15),
            PXVEC_LEFTMASK_128(16)
        };

        return TYPE(left_mask_128[count*sizeof(typename TYPE::SCALAR_TYPE)]);
    }
};

#endif

#if PLATFORM_INTEL_SSE >= 50

template <typename TYPE>
struct partial_mask_impl<TYPE, 32>
{
    FORCEINLINE TYPE operator () (int count) const
    {
        static __m256i const left_mask_256[33] = {
            PXVEC_LEFTMASK_256(0),
            PXVEC_LEFTMASK_256(1),
            PXVEC_LEFTMASK_256(2),
            PXVEC_LEFTMASK_256(3),
            PXVEC_LEFTMASK_256(4),
            PXVEC_LEFTMASK_256(5),
            PXVEC_LEFTMASK_256(6),
            PXVEC_LEFTMASK_256(7),
            PXVEC_LEFTMASK_256(8),
            PXVEC_LEFTMASK_256(9),
            PXVEC_LEFTMASK_256(10),
            PXVEC_LEFTMASK_256(11),
            PXVEC_LEFTMASK_256(12),
            PXVEC_LEFTMASK_256(13),
            PXVEC_LEFTMASK_256(14),
            PXVEC_LEFTMASK_256(15),
            PXVEC_LEFTMASK_256(16),
            PXVEC_LEFTMASK_256(17),
            PXVEC_LEFTMASK_256(18),
            PXVEC_LEFTMASK_256(19),
            PXVEC_LEFTMASK_256(20),
            PXVEC_LEFTMASK_256(21),
            PXVEC_LEFTMASK_256(22),
            PXVEC_LEFTMASK_256(23),
            PXVEC_LEFTMASK_256(24),
            PXVEC_LEFTMASK_256(25),
            PXVEC_LEFTMASK_256(26),
            PXVEC_LEFTMASK_256(27),
            PXVEC_LEFTMASK_256(28),
            PXVEC_LEFTMASK_256(29),
            PXVEC_LEFTMASK_256(30),
            PXVEC_LEFTMASK_256(31),
            PXVEC_LEFTMASK_256(32)
        };
        return TYPE(Is32vec8(left_mask_256[count*sizeof(typename TYPE::SCALAR_TYPE)]));
    }
};

#endif /* PLATFORM_INTEL_SSE >= 50 */

template <typename TYPE>
FORCEINLINE TYPE partial_mask(int count)
{
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
        PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
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
        _mm_maskmoveu_si128(I128vec1(vec), I128vec1(partial_mask<TYPE>(count)), (char *)ptr);
    }
};

#if PLATFORM_INTEL_SSE >= 50

template <typename TYPE>
struct store_partial_impl<TYPE, 32>
{
    FORCEINLINE void operator () (typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count) const
    {
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
TYPE unpack(TYPE const & a, TYPE const & b)
{
    if (HALF == 0)
        return unpack_low(a, b);
    return unpack_high(a, b);
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

#if PLATFORM_INTEL_SSE >= 50

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
FORCEINLINE TYPE unpack(TYPE const & a, TYPE const & b)
{
    enum { ELEM_SIZE = ELEM_COUNT*sizeof(typename TYPE::SCALAR_TYPE) };
    return TYPE(unpack_impl<HALF, ELEM_SIZE, sizeof(TYPE)>()(a, b));
}

template <int HALF, int ELEM_COUNT, typename TYPE>
FORCEINLINE TYPE unpack(TYPE const & a)
{
    return unpack<HALF, ELEM_COUNT>(a, a);
}

template <int VECTOR_SIZE>
struct permute_halves_impl
{
};

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
        return TYPE(_mm_alignr_epi8(I128vec1(a.half(1)), I128vec1(a.half(0)), N * SCALAR_SIZE),
                    _mm_srli_si128 (I128vec1(a.half(1)), N * SCALAR_SIZE));
    }
};

template <int N, typename TYPE, int SCALAR_SIZE>
struct shiftreg_left_impl<N, TYPE, SCALAR_SIZE, 32>
{
    FORCEINLINE TYPE operator()(TYPE const & a) const
    {
        return TYPE(_mm_slli_si128 (I128vec1(a.half(1)), N * SCALAR_SIZE),
                    _mm_alignr_epi8(I128vec1(a.half(1)), I128vec1(a.half(0)), 32 - N * SCALAR_SIZE));
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

        static PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) uint32_t extract_soft_impl_32_4_indices[8][8] = {
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

        static PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) uint32_t extract_soft_impl_256_32_4_indices[8][8] = {
            extract_soft_impl_256_32_4_indices_unroll_(extract_soft_impl_256_32_4_indices_),
            extract_soft_impl_256_32_4_indices_unroll_(extract_soft_impl_256_32_4_indices_)
        };

#undef extract_soft_impl_256_32_4_indices_unroll_
#undef extract_soft_impl_256_32_4_indices_

        __m256i permute_mask = _mm256_load_si256((__m256i const *)&extract_soft_impl_256_32_4_indices[i]);
        if (i < 4)
        {
            return extract<0>(TYPE(_mm256_permutevar_ps(F32vec8(a), permute_mask)));
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

        static PLATFORM_ALIGNED(PLATFORM_DEFAULT_ALIGN) uint8_t extract_soft_impl_128_32_4_indices[8][16] = {
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

#endif /* PLATFORM_INTEL_SSE >= 60 */

template <bool FIRST, typename TYPE>
FORCEINLINE void cumulate(TYPE & a, TYPE const & b)
{
    if (FIRST)
        a = b;
    else
        a += b;
}

template <int N, typename TYPE>
FORCEINLINE TYPE combine(TYPE const & a, TYPE const & b)
{
    assert(__abs(N) <= TYPE::SCALAR_COUNT);
    int const M = __min(TYPE::SCALAR_COUNT, __max(0, N < 0 ? N + TYPE::SCALAR_COUNT : N));
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

FORCEINLINE F64vec2 sum02_13(F32vec4 const & a)
{
    return F64vec2(_mm_cvtps_pd(a + shiftreg_right<2>(a)));
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

template <typename A, typename B>
struct unimplemented_convert_must_be_same_type
{
private:
    unimplemented_convert_must_be_same_type(int);
};

template <typename A>
struct unimplemented_convert_must_be_same_type<A, A>
{
    FORCEINLINE unimplemented_convert_must_be_same_type() {}
};

template <typename TYPE_OUT, typename TYPE_IN>
FORCEINLINE TYPE_OUT convert(TYPE_IN const & src)
{
    unimplemented_convert_must_be_same_type<TYPE_OUT, TYPE_IN>();
    return TYPE_OUT(src);
}

template <int HALF, typename TYPE_OUT, typename TYPE_IN>
TYPE_OUT convert(TYPE_IN const & src);

template <>
FORCEINLINE F64vec4 convert<F64vec4, F32vec4>(F32vec4 const & src)
{
    return F64vec4(_mm_cvtps_pd(src), _mm_cvtps_pd(shiftreg_right<2>(src)));
}

template <>
FORCEINLINE F32vec4 convert<F32vec4, F64vec4>(F64vec4 const & src)
{
    return F32vec4(_mm_shuffle_ps(_mm_cvtpd_ps(src.half(0)), _mm_cvtpd_ps(src.half(1)), _MM_SHUFFLE(3, 2, 1, 0)));
}

template <>
FORCEINLINE Iu32vec8 convert<Iu32vec8, Is32vec8>(Is32vec8 const & src)
{
    return Iu32vec8(src);
}

template <>
FORCEINLINE Is32vec8 convert<Is32vec8, Iu32vec8>(Iu32vec8 const & src)
{
    return Is32vec8(src);
}

template <>
FORCEINLINE Iu16vec8 convert<Iu16vec8, Is16vec8>(Is16vec8 const & src)
{
    return Iu16vec8(src);
}

template <>
FORCEINLINE Is16vec8 convert<Is16vec8, Iu16vec8>(Iu16vec8 const & src)
{
    return Is16vec8(src);
}

#if PLATFORM_INTEL_SSE >= 50

// 8xFloat32 to 8xInt32
template <>
FORCEINLINE Is32vec8 convert<Is32vec8, F32vec8>(F32vec8 const & src)
{
    return Is32vec8(_mm256_cvtps_epi32(src));
}

// 8xFloat32 to 8xInt32
template <>
FORCEINLINE Iu32vec8 convert<Iu32vec8, F32vec8>(F32vec8 const & src)
{
    return Iu32vec8(_mm256_cvtps_epi32(src));
}

// 8xInt32 to 8xFloat32
template <>
FORCEINLINE F32vec8 convert<F32vec8, Is32vec8>(Is32vec8 const & src)
{
    return F32vec8(_mm256_cvtepi32_ps(_mm256_castps_si256(F32vec8(src))));
}

#else

// 8xFloat32 to 8xInt32
template <>
FORCEINLINE Is32vec8 convert<Is32vec8, F32vec8>(F32vec8 const & src)
{
    return Is32vec8(_mm_cvtps_epi32(src.half(0)),
                    _mm_cvtps_epi32(src.half(1)));
}

// 8xFloat32 to 8xInt32
template <>
FORCEINLINE Iu32vec8 convert<Iu32vec8, F32vec8>(F32vec8 const & src)
{
    return Iu32vec8(_mm_cvtps_epi32(src.half(0)),
                    _mm_cvtps_epi32(src.half(1)));
}

// 8xInt32 to 8xFloat32
template <>
FORCEINLINE F32vec8 convert<F32vec8, Is32vec8>(Is32vec8 const & src)
{
    return F32vec8(_mm_cvtepi32_ps(src.half(0)),
                   _mm_cvtepi32_ps(src.half(1)));
}

#endif

template <>
FORCEINLINE Iu32vec8 convert<Iu32vec8, Iu16vec8>(Iu16vec8 const & src)
{
    return Iu32vec8(_mm_unpacklo_epi16(src, _mm_setzero_si128()),
                    _mm_unpackhi_epi16(src, _mm_setzero_si128()));
}

template <>
FORCEINLINE Iu16vec16 convert<Iu16vec16, Iu8vec16>(Iu8vec16 const & src)
{
    return Iu16vec16(_mm_unpacklo_epi8(src, _mm_setzero_si128()),
                     _mm_unpackhi_epi8(src, _mm_setzero_si128()));
}

#if PLATFORM_INTEL_SSE >= 40

template <>
FORCEINLINE Iu16vec16 convert<Iu16vec16, Iu32vec16>(Iu32vec16 const & src)
{
#if PLATFORM_INTEL_SSE >= 60

    return Iu16vec16(_mm256_packus_epi32(src.half(0), src.half(1)));

#else /* PLATFORM_INTEL_SSE < 60 */

    return Iu16vec16(_mm_packus_epi32(src.half(0).half(0), src.half(0).half(1)),
                     _mm_packus_epi32(src.half(1).half(0), src.half(1).half(1)));

#endif /* PLATFORM_INTEL_SSE >= 60 */
}

#endif

template <>
FORCEINLINE Is16vec16 convert<Is16vec16, Iu32vec16>(Iu32vec16 const & src)
{
#if PLATFORM_INTEL_SSE >= 60

    return Is16vec16(_mm256_packs_epi32(src.half(0), src.half(1)));

#else /* PLATFORM_INTEL_SSE < 60 */

    I128vec1 a00 = I128vec1(src.half(0).half(0));
    I128vec1 a01 = I128vec1(src.half(0).half(1));
    I128vec1 a10 = I128vec1(src.half(1).half(0));
    I128vec1 a11 = I128vec1(src.half(1).half(1));

    return Is16vec16(_mm_packs_epi32(a00, a01),
                     _mm_packs_epi32(a10, a11));

#endif /* PLATFORM_INTEL_SSE >= 60 */
}

template <>
FORCEINLINE Is16vec8 convert<Is16vec8, Iu32vec8>(Iu32vec8 const & src)
{
    return Is16vec8(_mm_packs_epi32(src.half(0), src.half(1)));
}

#if PLATFORM_INTEL_SSE >= 40

template <>
FORCEINLINE Iu16vec8 convert<Iu16vec8, Iu32vec8>(Iu32vec8 const & src)
{
    return Iu16vec8(_mm_packus_epi32(src.half(0), src.half(1)));
}

#endif

template <>
FORCEINLINE Iu32vec16 convert<Iu32vec16, Iu16vec16>(Iu16vec16 const & src)
{
#if PLATFORM_INTEL_SSE >= 60

    __m256i x16 = _mm256_permute4x64_epi64(src, _MM_SHUFFLE(3, 1, 2, 0));
    __m256i x0 = _mm256_unpacklo_epi16(x16, _mm256_setzero_si256());
    __m256i x1 = _mm256_unpackhi_epi16(x16, _mm256_setzero_si256());
    return Iu32vec16(x0, x1);

#else /* PLATFORM_INTEL_SSE < 60 */

    return Iu32vec16(convert<Iu32vec8>(src.half(0)), convert<Iu32vec8>(src.half(1)));

#endif /* PLATFORM_INTEL_SSE >= 60 */
}

template <>
FORCEINLINE Iu32vec16 convert<Iu32vec16, Iu8vec16>(Iu8vec16 const & src)
{
    return convert<Iu32vec16>(convert<Iu16vec16>(src));
}

template <>
FORCEINLINE Is32vec8 convert<Is32vec8, Iu16vec8>(Iu16vec8 const & src)
{
    return Is32vec8(_mm_unpacklo_epi16(src, _mm_setzero_si128()),
                    _mm_unpackhi_epi16(src, _mm_setzero_si128()));
}
template <>
FORCEINLINE Is32vec8 convert<Is32vec8, Is16vec8>(Is16vec8 const & src)
{
    return Is32vec8(_mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), src), 16),
                    _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), src), 16));
}

template <>
FORCEINLINE Is16vec8 convert<Is16vec8, Is32vec8>(Is32vec8 const & src)
{
    return Is16vec8(_mm_packs_epi32(src.half(0), src.half(1)));
}

#if PLATFORM_INTEL_SSE >= 40

template <>
FORCEINLINE Iu16vec8 convert<Iu16vec8, Is32vec8>(Is32vec8 const & src)
{
    return Iu16vec8(_mm_packus_epi32(src.half(0), src.half(1)));
}

#else

template <>
FORCEINLINE Iu16vec8 convert<Iu16vec8, Is32vec8>(Is32vec8 const & src)
{
    Is32vec4 off1 = Is32vec4(-32768);
    Is16vec8 off2 = Is16vec8(-32768); // in (u)int16_t, adding -32768 is strictly equivalent to adding 32768
    return Iu16vec8(Is16vec8(_mm_packs_epi32(src.half(0) + off1, src.half(1) + off1)) + off2);
}

#endif

template <>
FORCEINLINE Iu16vec8 convert<Iu16vec8, F32vec8>(F32vec8 const & src)
{
    return convert<Iu16vec8>(convert<Is32vec8>(src));
}

template <>
FORCEINLINE F32vec4 convert<F32vec4, Is32vec4>(Is32vec4 const & src)
{
    return F32vec4(_mm_cvtepi32_ps(src));
}

template <>
FORCEINLINE F32vec4 convert<F32vec4, Iu32vec4>(Iu32vec4 const & src)
{
    __m128  hi = _mm_cvtepi32_ps(_mm_srli_epi32(src, 1));
    return F32vec4(_mm_add_ps(_mm_add_ps(hi, hi), _mm_cvtepi32_ps(_mm_and_si128(src, _mm_set1_epi32(1)))));
}

template <>
FORCEINLINE F32vec8 convert<F32vec8, Iu32vec8>(Iu32vec8 const & src)
{
    return F32vec8(convert<F32vec4, Iu32vec4>(src.half(0)), convert<F32vec4>(src.half(1)));
}

template <>
FORCEINLINE F32vec8 convert<F32vec8, Iu16vec8>(Iu16vec8 const & src)
{
    return F32vec8(convert<F32vec8>(convert<Iu32vec8>(src)));
}

template <>
FORCEINLINE Is32vec4 convert<Is32vec4, F32vec4>(F32vec4 const & src)
{
    return Is32vec4(_mm_cvtps_epi32(src));
}

template <>
FORCEINLINE Iu32vec4 convert<Iu32vec4, F32vec4>(F32vec4 const & src)
{
    return Iu32vec4(_mm_cvtps_epi32(src));
}

template <>
FORCEINLINE Is16vec8 convert<Is16vec8, F32vec8>(F32vec8 const & src)
{
    return convert<Is16vec8>(convert<Is32vec8>(src));
}

template <>
FORCEINLINE Is64vec4 convert<Is64vec4, Is32vec4>(Is32vec4 const & src)
{
    __m128i low = _mm_unpacklo_epi32(src, _mm_setzero_si128());
    low = _mm_or_si128(low, _mm_slli_si128(_mm_cmplt_epi32(low, _mm_setzero_si128()), 4));
    __m128i high = _mm_unpackhi_epi32(src, _mm_setzero_si128());
    high = _mm_or_si128(high, _mm_slli_si128(_mm_cmplt_epi32(high, _mm_setzero_si128()), 4));
    return Is64vec4(low, high);
}

template <>
FORCEINLINE Is64vec2 convert<Is64vec2, Is32vec4>(Is32vec4 const & src)
{
    __m128i low = _mm_unpacklo_epi32(src, _mm_setzero_si128());
    low = _mm_or_si128(low, _mm_slli_si128(_mm_cmplt_epi32(low, _mm_setzero_si128()), 4));
    return Is64vec2(low);
}

template <>
FORCEINLINE Is32vec4 convert<0, Is32vec4, Is16vec8>(Is16vec8 const & src)
{
    return Is32vec4(_mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), src), 16));
}

template <>
FORCEINLINE Is32vec4 convert<1, Is32vec4, Is16vec8>(Is16vec8 const & src)
{
    return Is32vec4(_mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), src), 16));
}

template <>
FORCEINLINE Is32vec4 convert<0, Is32vec4, Iu16vec8>(Iu16vec8 const & src)
{
    return Is32vec4(_mm_unpacklo_epi16(src, _mm_setzero_si128()));
}

template <>
FORCEINLINE Is32vec4 convert<1, Is32vec4, Iu16vec8>(Iu16vec8 const & src)
{
    return Is32vec4(_mm_unpackhi_epi16(src, _mm_setzero_si128()));
}
template <>
FORCEINLINE F32vec4 convert<0, F32vec4, Iu16vec8>(Iu16vec8 const & src)
{
    return convert<F32vec4>(convert<0, Is32vec4>(src));
}

template <>
FORCEINLINE F32vec4 convert<1, F32vec4, Iu16vec8>(Iu16vec8 const & src)
{
    return convert<F32vec4>(convert<1, Is32vec4>(src));
}

template <>
FORCEINLINE F32vec4 convert<0, F32vec4, Is16vec8>(Is16vec8 const & src)
{
    return convert<F32vec4>(convert<0, Is32vec4>(src));
}

template <>
FORCEINLINE F32vec4 convert<1, F32vec4, Is16vec8>(Is16vec8 const & src)
{
    return convert<F32vec4>(convert<1, Is32vec4>(src));
}

template <>
FORCEINLINE F32vec8 convert<F32vec8, Is16vec8>(Is16vec8 const & src)
{
    return F32vec8(convert<F32vec8>(convert<Is32vec8>(src)));
}

template <>
FORCEINLINE Is8vec16 convert<Is8vec16, Is16vec16>(Is16vec16 const & src)
{
    return Is8vec16(_mm_packs_epi16(src.half(0), src.half(1)));
}

template <>
FORCEINLINE Iu8vec16 convert<Iu8vec16, Is16vec16>(Is16vec16 const & src)
{
    return Iu8vec16(_mm_packus_epi16(src.half(0), src.half(1)));
}

/*
template <>
FORCEINLINE Iu32vec8 convert<Iu32vec8, Iu16vec8>(Is16vec8 const & src)
{
    return Iu32vec8(_mm_unpacklo_epi16(_mm_setzero_si128(), src),
                    _mm_unpackhi_epi16(_mm_setzero_si128(), src));
}
*/

// Convert function that work for comparison masks (only casting & resizing)
template <typename OUT_TYPE, typename IN_TYPE>
FORCEINLINE OUT_TYPE convert_mask(IN_TYPE const & in)
{
    typedef ::Philips::Medisys::PXVec::Details::VecTypeForElemSize<sizeof(typename IN_TYPE::SCALAR_TYPE), IN_TYPE::SCALAR_COUNT>::Type InType;
    typedef ::Philips::Medisys::PXVec::Details::VecTypeForElemSize<sizeof(typename OUT_TYPE::SCALAR_TYPE), OUT_TYPE::SCALAR_COUNT>::Type OutType;

    //typedef PXVEC_TYPE_FORELEMSIZE_OR(sizeof(typename IN_TYPE::SCALAR_TYPE), IN_TYPE::SCALAR_COUNT, int) InType;
    //typedef PXVEC_TYPE_FORELEMSIZE_OR(sizeof(typename OUT_TYPE::SCALAR_TYPE), IN_TYPE::SCALAR_COUNT, int) OutType;

    return OUT_TYPE(convert<OutType>(InType(in)));
}

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
    return TYPE(_mm_shuffle_epi8(I128vec1(a), _mm_set_epi8(A15, A7,
                                                           A14, A6,
                                                           A13, A5,
                                                           A12, A4,
                                                           A11, A3,
                                                           A10, A2,
                                                           A9 , A1,
                                                           A8 , A0)));
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

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE typename TYPE::HALF_TYPE shuffle(TYPE const & a)
{
    PX_STATICASSERT((A0 >> 2) == (A1 >> 2) && (A2 >> 2) == (A3 >> 2) && __abs(A0 >> 2) <= 1 && __abs(A2 >> 2) <= 1);
    return TYPE::HALF_TYPE(shuffle<A0, A1, A2, A3>(a.half(A0>>2), a.half(A2>>2)));
}

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
        return TYPE(shuffle<7, 6, 5, 4, 3, 2, 1, 0>(in));
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
FORCEINLINE TYPE broadcast(TYPE const & a)
{
    if (sizeof(TYPE) == 16 && sizeof(TYPE::SCALAR_TYPE) == 2)
    {
        enum { N_M4 = __max(0, N - 4) };
        if (N >= 4)
            return shuffle<2, 2, 2, 2>(shuffle_high<N_M4, N_M4, N_M4, N_M4>(a));
        return shuffle<0, 0, 0, 0>(shuffle_low<N, N, N, N>(a));
    }
    else if (sizeof(TYPE) == 16 && sizeof(TYPE::SCALAR_TYPE) == 4)
    {
        return shuffle<N, N, N, N>(a);
    }
    else if (sizeof(TYPE) == 16 && sizeof(TYPE::SCALAR_TYPE) == 8)
    {
        return shuffle<N, N>(a, a);
    }
    else
    {
        return TYPE(extract<N>(a));
    }
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
        return _mm_movemask_ps(_mm_cmp_ps(F32vec8(a), F32vec8(b), _CMP_NEQ_OQ)) == 0;
#else
        return _mm_movemask_ps(F32vec4(_mm_cmpeq_epi8(Is8vec16(a), Is8vec16(b)))) == 15;
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

#if PLATFORM_INTEL_SSE >= 50

template <>
FORCEINLINE bool vec_is_zero<__m256>(__m256 const & a)
{
    return _mm256_movemask_ps(a) == 0;
}

#endif


template <>
FORCEINLINE bool vec_is_zero<__m128d>(__m128d const & a)
{
    return _mm_movemask_pd(a) == 0;
}

template <>
FORCEINLINE bool vec_is_zero<__m128>(__m128 const & a)
{
    return _mm_movemask_ps(a) == 0;
}

template <>
FORCEINLINE bool vec_is_zero<__m128i>(__m128i const & a)
{
    return _mm_movemask_epi8(a) == 0;
}

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

template <int VECTOR_SIZE, int SCALAR_SIZE, typename TYPE>
class convert_to_mask_impl
{
    FORCEINLINE int operator()(TYPE const & input);
};

template <typename TYPE>
struct convert_to_mask_impl<16, 1, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm_movemask_epi8(input);
    }
};

template <typename TYPE>
struct convert_to_mask_impl<16, 2, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm_movemask_epi8(_mm_packs_epi16(input, _mm_setzero_si128()));
    }
};

//template <typename TYPE>
//struct convert_to_mask_impl<32, 2, TYPE>
//{
//    FORCEINLINE int operator()(TYPE const & input)
//    {
//        return _mm_movemask_epi8(_mm_packs_epi16(input.half(0), input.half(1)));
//    }
//};

template <typename TYPE>
struct convert_to_mask_impl<16, 8, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm_movemask_pd(input);
    }
};

template <typename TYPE>
struct convert_to_mask_impl<16, 4, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm_movemask_ps(F32vec4(input));
    }
};

#if PLATFORM_INTEL_SSE >= 60

template <typename TYPE>
struct convert_to_mask_impl<32, 8, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm256_movemask_pd(F32vec8(input));
    }
};

template <typename TYPE>
struct convert_to_mask_impl<32, 4, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm256_movemask_ps(F32vec8(input));
    }
};

template <typename TYPE>
struct convert_to_mask_impl<32, 2, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm256_movemask_epi8(_mm256_packs_epi16(input, _mm256_setzero_si256()));
    }
};

template <typename TYPE>
struct convert_to_mask_impl<32, 1, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return _mm256_movemask_epi8(input);
    }
};

#else /* PLATFORM_INTEL_SSE < 60 */

#if PLATFORM_INTEL_SSE >= 50

template <>
struct convert_to_mask_impl<32, 4, F32vec8>
{
    FORCEINLINE int operator()(F32vec8 const & input)
    {
        return _mm256_movemask_ps(F32vec8(input));
    }
};


#endif /* PLATFORM_INTEL_SSE >= 50 */

template <typename TYPE>
struct convert_to_mask_impl<32, 4, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return convert_to_mask(Is16vec8(_mm_packs_epi16(input.half(0), input.half(1))));
    }
};

template <int SCALAR_SIZE, typename TYPE>
struct convert_to_mask_impl<32, SCALAR_SIZE, TYPE>
{
    FORCEINLINE int operator()(TYPE const & input)
    {
        return convert_to_mask(input.half(0)) | (convert_to_mask(input.half(1)) << (16 / SCALAR_SIZE));
    }
};

#endif /* PLATFORM_INTEL_SSE >= 60 */

template <typename TYPE>
FORCEINLINE int convert_to_mask(TYPE const & a)
{
    return convert_to_mask_impl<sizeof(TYPE), sizeof(TYPE::SCALAR_TYPE), TYPE>()(a);
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

/////////////////////////////////////////////////////
/// Macros for type selection (PXVEC_TYPE etc...)
/////////////////////////////////////////////////////

#define PXVEC_MAX_VECTOR_SIZE       32

namespace Details
{
    template <typename SCALAR_TYPE>
    struct VecType;

    template <typename SCALAR_TYPE, int N, typename ALTTYPE = void>
    struct VecTypeForSize
    {
        typedef VecTypeForSize<SCALAR_TYPE, __min(PXVEC_MAX_VECTOR_SIZE + 1, N + 1), ALTTYPE> LargerVecTypeForSize;
        typedef typename LargerVecTypeForSize::Type Type;
        enum { Supported = LargerVecTypeForSize::Supported };
    };

    template <typename SCALAR_TYPE, typename ALTTYPE>
    struct VecTypeForSize<SCALAR_TYPE, PXVEC_MAX_VECTOR_SIZE + 1 /* max vector size + 1 */, ALTTYPE>
    {
        typedef ALTTYPE Type;
        enum { Supported = false };
    };

    template <int ELEM_SIZE, int N, typename ALTTYPE = void>
    struct VecTypeForElemSize
    {
        typedef ALTTYPE Type;
        enum { Supported = false };
    };
    template <typename TYPE>
    struct IsVecType { enum { Value = false }; };
    template <int N>
    struct VecTypeForCopy;


#define PXVEC_IMPLEMENT_VECTYPE_FORCOPY(_TYPE) \
    template <> struct VecTypeForCopy<sizeof(_TYPE)> { typedef _TYPE Type; };

#define PXVEC_IMPLEMENT_VECTYPE(_TYPE) \
    template <typename ALTTYPE> struct VecTypeForSize<_TYPE::SCALAR_TYPE, _TYPE::SCALAR_COUNT, ALTTYPE> { typedef _TYPE Type; enum { Supported = true }; }; \
    template <> struct IsVecType<_TYPE> { enum { Value = true }; };

#define PXVEC_IMPLEMENT_VECTYPE_INT(_TYPE) \
    template <typename ALTTYPE> struct VecTypeForElemSize<sizeof(_TYPE::SCALAR_TYPE), _TYPE::SCALAR_COUNT, ALTTYPE> { typedef _TYPE Type; enum { Supported = true }; }; \
    PXVEC_IMPLEMENT_VECTYPE(_TYPE)

#define PXVEC_IMPLEMENT_BEST_VECTYPE(_TYPE) \
    template <> struct VecType<_TYPE::SCALAR_TYPE> { typedef _TYPE Type; }; \
    PXVEC_IMPLEMENT_VECTYPE(_TYPE)

#define PXVEC_IMPLEMENT_BEST_VECTYPE_INT(_TYPE) \
    template <> struct VecType<_TYPE::SCALAR_TYPE> { typedef _TYPE Type; }; \
    PXVEC_IMPLEMENT_VECTYPE_INT(_TYPE)

#if PLATFORM_INTEL_SSE >= 60

PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is8vec32)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu8vec32)
PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is16vec16)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu16vec16)
PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is32vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu32vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is64vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu64vec4)

PXVEC_IMPLEMENT_VECTYPE_INT(Is8vec16)
PXVEC_IMPLEMENT_VECTYPE(Iu8vec16)
PXVEC_IMPLEMENT_VECTYPE_INT(Is16vec8)
PXVEC_IMPLEMENT_VECTYPE(Iu16vec8)
PXVEC_IMPLEMENT_VECTYPE_INT(Is32vec4)
PXVEC_IMPLEMENT_VECTYPE(Iu32vec4)
PXVEC_IMPLEMENT_VECTYPE_INT(Iu64vec2)
PXVEC_IMPLEMENT_VECTYPE(Is64vec2)

#else

PXVEC_IMPLEMENT_VECTYPE_INT(Is8vec32)
PXVEC_IMPLEMENT_VECTYPE(Iu8vec32)
PXVEC_IMPLEMENT_VECTYPE_INT(Is16vec16)
PXVEC_IMPLEMENT_VECTYPE(Iu16vec16)
PXVEC_IMPLEMENT_VECTYPE_INT(Is32vec8)
PXVEC_IMPLEMENT_VECTYPE(Iu32vec8)
PXVEC_IMPLEMENT_VECTYPE_INT(Is64vec4)
PXVEC_IMPLEMENT_VECTYPE(Iu64vec4)

PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is8vec16)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu8vec16)
PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is16vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu16vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is32vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu32vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE_INT(Is64vec2)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu64vec2)

#endif

#if PLATFORM_INTEL_SSE >= 50

PXVEC_IMPLEMENT_BEST_VECTYPE(F32vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE(F64vec4)

PXVEC_IMPLEMENT_VECTYPE(F32vec4)
PXVEC_IMPLEMENT_VECTYPE(F64vec2)

#else

PXVEC_IMPLEMENT_VECTYPE(F32vec8)
PXVEC_IMPLEMENT_VECTYPE(F64vec4)

PXVEC_IMPLEMENT_BEST_VECTYPE(F32vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(F64vec2)

#endif

PXVEC_IMPLEMENT_VECTYPE_FORCOPY(F32vec4)
PXVEC_IMPLEMENT_VECTYPE_FORCOPY(F32vec8)

#undef PXVEC_IMPLEMENT_VECTYPE
#undef PXVEC_IMPLEMENT_BEST_VECTYPE
#undef PXVEC_IMPLEMENT_VECTYPE_INT
#undef PXVEC_IMPLEMENT_BEST_VECTYPE_INT
#undef PXVEC_IMPLEMENT_VECTYPE_FORCOPY

}

#define PXVEC_TYPE_FORCOPY(SIZE) ::Philips::Medisys::PXVec::Details::VecTypeForCopy<SIZE>::Type
#define PXVEC_TYPE(SCALAR_TYPE) ::Philips::Medisys::PXVec::Details::VecType<SCALAR_TYPE>::Type
#define PXVEC_TYPE_FORSIZE(SCALAR_TYPE, N) ::Philips::Medisys::PXVec::Details::VecTypeForSize<SCALAR_TYPE, N>::Type
#define PXVEC_TYPE_FORSIZE_OR(SCALAR_TYPE, N, ALTTYPE) ::Philips::Medisys::PXVec::Details::VecTypeForSize<SCALAR_TYPE, N, ALTTYPE>::Type
#define PXVEC_TYPE_FORSIZE_EXISTS(SCALAR_TYPE, N) ::Philips::Medisys::PXVec::Details::VecTypeForSize<SCALAR_TYPE, N>::Supported
#define PXVEC_TYPE_COUNT(SCALAR_TYPE) ::Philips::Medisys::PXVec::Details::VecType<SCALAR_TYPE>::Type::SCALAR_COUNT
#define PXVEC_TYPE_FORELEMSIZE(ELEMSIZE, COUNT) ::Philips::Medisys::PXVec::Details::VecTypeForElemSize<ELEMSIZE, COUNT>::Type
#define PXVEC_TYPE_FORELEMSIZE_OR(ELEMSIZE, COUNT, ALTTYPE) ::Philips::Medisys::PXVec::Details::VecTypeForElemSize<ELEMSIZE, COUNT, ALTTYPE>::Type

#define PXVEC_ISVECTYPE(TYPE) ::Philips::Medisys::PXVec::Details::IsVecType<TYPE>::Value

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
        VECTOR const cephes_log_p0 = 7.0376836292E-2f;
        VECTOR const cephes_log_p1 = -1.1514610310E-1f;
        VECTOR const cephes_log_p2 = 1.1676998740E-1f;
        VECTOR const cephes_log_p3 = -1.2420140846E-1f;
        VECTOR const cephes_log_p4 = 1.4249322787E-1f;
        VECTOR const cephes_log_p5 = -1.6668057665E-1f;
        VECTOR const cephes_log_p6 = 2.0000714765E-1f;
        VECTOR const cephes_log_p7 = - 2.4999993993E-1f;
        VECTOR const cephes_log_p8 = 3.3333331174E-1f;
        VECTOR const cephes_log_q1 = -2.12194440e-4f;
        VECTOR const cephes_log_q2 = 0.693359375f;

        VECTOR invalidMask = (in < VECTOR::zero());

        VECTOR inNormalized = max(in, minNormPos);
        VECTOR_INT exponent = shift_right<23>(VECTOR_INT(inNormalized));

        VECTOR const invMantissaMask = VECTOR(VECTOR_INT(~0x7f800000));

        VECTOR frac = (inNormalized & invMantissaMask) | VECTOR(half);

        VECTOR underSqrthf = frac < VECTOR(0.707106781186547524f);
        VECTOR exponentF = convert<VECTOR>(exponent - VECTOR_INT(0x7f)) + (underSqrthf & one);

        VECTOR x = (frac & underSqrthf) + frac - one;

        VECTOR z = x * x;

        VECTOR y = (((((((((cephes_log_p0 * x + cephes_log_p1) * x + cephes_log_p2) * x + cephes_log_p3) * x + cephes_log_p4 * x) + cephes_log_p5 * x) + cephes_log_p6 * x) + cephes_log_p7 * x) + cephes_log_p8) * x * z
            + exponentF * cephes_log_q1
            - z * half
            + x + exponentF * cephes_log_q2) | invalidMask;

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
        // Work only with floats (F32vecX)
        VECTOR const exp_hi = 88.3762626647949f;
        VECTOR const exp_lo =-88.3762626647949f;

        VECTOR const cephes_LOG2EF = (TYPE)1.44269504088896341;
        VECTOR const cephes_exp_C1 = (TYPE)0.693359375;
        VECTOR const cephes_exp_C2 = (TYPE)-2.12194440e-4;

        VECTOR const cephes_exp_p0 = (TYPE)1.9875691500E-4;
        VECTOR const cephes_exp_p1 = (TYPE)1.3981999507E-3;
        VECTOR const cephes_exp_p2 = (TYPE)8.3334519073E-3;
        VECTOR const cephes_exp_p3 = (TYPE)4.1665795894E-2;
        VECTOR const cephes_exp_p4 = (TYPE)1.6666665459E-1;
        VECTOR const cephes_exp_p5 = (TYPE)5.0000001201E-1;

        VECTOR const  one = (TYPE)1.0;

        VECTOR x = min(exp_hi, max(exp_lo, in));

        /* express exp(x) as exp(g + n*log(2)) */
        VECTOR flooredx = x * cephes_LOG2EF + VECTOR(0.5f);

        vec_roundingmode<RoundingMode_Floor> rnd;
        /* how to perform a floorf with SSE: just below */
        VECTOR_INT intx = convert<VECTOR_INT>(flooredx);
        flooredx = convert<VECTOR>(intx);

#if 0
        /* if greater, substract 1 */
        v4sf mask = _mm_cmpgt_ps(tmp, fx);
        mask = _mm_and_ps(mask, one);
        fx = _mm_sub_ps(tmp, mask);
#endif

        x = x - flooredx * cephes_exp_C1 - flooredx * cephes_exp_C2;

        VECTOR y = ((((((cephes_exp_p0 * x + cephes_exp_p1) * x + cephes_exp_p2) * x + cephes_exp_p3) * x + cephes_exp_p4) * x + cephes_exp_p5) * square(x)) + x + one;

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

template <typename TYPE>
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
    y += ((( c9 * ax2 - c7) * ax2 + c5) * ax2 - c3) * ax2 * ax + ax;

    y ^= sign;
    return y;
}

template <typename TYPE>
static FORCEINLINE TYPE atan2(TYPE const & y, TYPE const & x)
{
    TYPE const epsilon = ::std::numeric_limits<typename TYPE::SCALAR_TYPE>::min();
    TYPE sign = signbit(x);
    TYPE signy = signbit(y);
    TYPE xpos = x >= TYPE::zero();
    TYPE res = atan(y * rcp(sign ^ (epsilon + x ^ sign)));
    TYPE const pi = (TYPE)3.1415926535897932384626433832795;
    
    //return select(xpos, res, res + signy ^ pi);
    return res + andnot(xpos, signy ^ pi);
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