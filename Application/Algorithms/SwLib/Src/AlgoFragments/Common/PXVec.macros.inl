/*
* Copyright (c)2009-2015 Koninklijke Philips Electronics N.V.,
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

// To define a = b + c AND a += b
#define PXVEC_IMPLEMENT_BINARYOP(OP, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) {return SELF_TYPE(INTRIN(a, b)); }\
    FORCEINLINE SELF_TYPE & operator OP##=(SELF_TYPE const & a) { return *this = SELF_TYPE(INTRIN(*this, a)); }

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_COMPARE(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }

#define PXVEC_COMPARE_EMULATE_ONLYSTRICT() \
    friend FORCEINLINE SELF_TYPE operator <=(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (a > b); }\
    friend FORCEINLINE SELF_TYPE cmple(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (a > b); }\
    friend FORCEINLINE SELF_TYPE operator >=(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (a < b); }\
    friend FORCEINLINE SELF_TYPE cmpge(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE::minus_one() ^ (a < b); }

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

#define PXVEC_IMPLEMENT_NUMLIMIT_CONSTANTS() \
    PXVEC_IMPLEMENT_CONSTANT(epsilon, ::std::numeric_limits<SCALAR_TYPE>::epsilon) \
    PXVEC_IMPLEMENT_CONSTANT(min, ::std::numeric_limits<SCALAR_TYPE>::min) \
    PXVEC_IMPLEMENT_CONSTANT(max, ::std::numeric_limits<SCALAR_TYPE>::max)

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
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_load_si128((__m128i const *)ptr)); } \
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_loadu_si128((__m128i const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); } \
    FORCEINLINE void impl_load_low(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_si128((__m128i *)ptr, __m128i(a)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_stream_si128((__m128i *)ptr, __m128i(a)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_si128((__m128i *)ptr, __m128i(a)); }\
    FORCEINLINE void store_low(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, __m128i(a)); }\
    FORCEINLINE void store_high(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, __m128i(a)); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(__m128i(a), mask, (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128F_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_load_ps((float const *)ptr)); }\
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_loadu_ps((float const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); } \
    FORCEINLINE void impl_load_low(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_ps((float *)ptr, __m128(a)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_stream_ps((float *)ptr, __m128(a)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_ps((float *)ptr, __m128(a)); }\
    FORCEINLINE void store_low(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, I128vec1(__m128(a))); }\
    FORCEINLINE void store_high(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, I128vec1(__m128(a))); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(I128vec1(__m128(a)), I128vec1(__m128(mask)), (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128D_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_load_pd((double const *)ptr)); }\
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_loadu_pd((double const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); } \
    FORCEINLINE void impl_load_low(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_pd((double *)ptr, __m128d(a)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_pd((double *)ptr, __m128d(a)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_stream_pd((double *)ptr, __m128d(a)); }\
    FORCEINLINE void store_low(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, I128vec1(__m128d(a))); }\
    FORCEINLINE void store_high(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, I128vec1(__m128d(a))); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(I128vec1(__m128d(a)), I128vec1(__m128d(mask)), (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256F_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_load_ps((float const *)ptr)); }\
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_loadu_ps((float const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_ps((float *)ptr, __m256(a)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_ps((float *)ptr, __m256(a)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_ps((float *)ptr, __m256(a)); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_ps(ptr, _mm256_castps_si256(mask), __m256(a)); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256D_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_load_pd((double const *)ptr)); }\
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_loadu_pd((double const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_pd((double *)ptr, __m256d(a)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_pd((double *)ptr, __m256d(a)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_pd((double *)ptr, __m256d(a)); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_pd(ptr, _mm256_castpd_si256(mask), __m256d(a)); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256I_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_load_si256((__m256i const *)ptr)); }\
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_loadu_si256((__m256i const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_si256((__m256i *)ptr, __m256i(a)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_si256((__m256i *)ptr, __m256i(a)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_si256((__m256i *)ptr, __m256i(a)); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_emul(ptr, mask, a); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128I(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M128I_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M128F(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M128F_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M128D(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M128D_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)

#define PXVEC_IMPLEMENT_LOADSTORE_M256I(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M256I_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M256F(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M256F_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)
#define PXVEC_IMPLEMENT_LOADSTORE_M256D(SELF_TYPE) PXVEC_IMPLEMENT_LOADSTORE_M256D_(SELF_TYPE, SELF_TYPE::SCALAR_TYPE)

#if PLATFORM_INTEL_SSE >= 50

#define PXVEC_IMPLEMENT_LOADSTORE_2xM128X_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_load_ps((float const *)ptr)); } \
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_loadu_ps((float const *)ptr)); } \
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res = SELF_TYPE(_mm256_lddqu_si256((__m256i const *)ptr)); } \
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_ps((float *)ptr, __m256(F32vec8(a))); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_ps((float *)ptr, __m256(F32vec8(a))); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_stream_ps((float *)ptr, __m256(F32vec8(a))); }\
    FORCEINLINE void store_low(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu(ptr, a.half(0)); }\
    FORCEINLINE void store_high(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu(ptr, a.half(1)); }\
    template <> FORCEINLINE SELF_TYPE partial_mask<SELF_TYPE>(int count) { return SELF_TYPE(partial_mask<typename SELF_TYPE::HALF_TYPE>(__min(count, (SELF_TYPE::SCALAR_COUNT>>1))), partial_mask<typename SELF_TYPE::HALF_TYPE>(__max(0, count-(SELF_TYPE::SCALAR_COUNT>>1)))); }\
    FORCEINLINE void store_partial(SCALAR_TYPE * ptr, SELF_TYPE const & a, int count) { store_partial(ptr, a.half(0), __min(count, (SELF_TYPE::SCALAR_COUNT>>1))); if (count > (SELF_TYPE::SCALAR_COUNT>>1)) store_partial(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), count-(SELF_TYPE::SCALAR_COUNT>>1)); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_ps((float *)ptr, _mm256_castps_si256(F32vec8(mask)), __m256(F32vec8(a))); }

#else

#define PXVEC_IMPLEMENT_LOADSTORE_2xM128X_(SELF_TYPE, SCALAR_TYPE) \
    FORCEINLINE void impl_loada(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res.set_half(0, loada<typename SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, loada<typename SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); }\
    FORCEINLINE void impl_loadu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res.set_half(0, loadu<typename SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, loadu<typename SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); }\
    FORCEINLINE void impl_lddqu(SELF_TYPE & res, SCALAR_TYPE const * ptr) { res.set_half(0, lddqu<typename SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, lddqu<typename SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); }\
    FORCEINLINE void storea(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storea(ptr, a.half(0)); storea(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    FORCEINLINE void storeu(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu(ptr, a.half(0)); storeu(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    FORCEINLINE void streama(SCALAR_TYPE * ptr, SELF_TYPE const & a) { streama(ptr, a.half(0)); streama(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    FORCEINLINE void store_low(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu(ptr, a.half(0)); }\
    FORCEINLINE void store_high(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu(ptr, a.half(1)); }\
    template <> FORCEINLINE SELF_TYPE partial_mask<SELF_TYPE>(int count) { return SELF_TYPE(partial_mask<typename SELF_TYPE::HALF_TYPE>(__min(count, (SELF_TYPE::SCALAR_COUNT>>1))), partial_mask<typename SELF_TYPE::HALF_TYPE>(__max(0, count-(SELF_TYPE::SCALAR_COUNT>>1)))); }\
    FORCEINLINE void store_partial(SCALAR_TYPE * ptr, SELF_TYPE const & a, int count) { store_partial(ptr, a.half(0), __min(count, (SELF_TYPE::SCALAR_COUNT>>1))); if (count > (SELF_TYPE::SCALAR_COUNT>>1)) store_partial(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), count-(SELF_TYPE::SCALAR_COUNT>>1)); }\
    FORCEINLINE void store_mask(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { store_mask(ptr, a.half(0), mask.half(0)); store_mask(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), mask.half(1)); }

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

template <int SIZE, int COUNT>
struct VecLayout
{
    enum { Size = SIZE, Count = COUNT };
};

#ifndef _DEBUG
#define PXVEC_IMPLEMENT_DATA_GEN(TEMPLATE_PREFIX,TYPENAME,X) \
private: \
    NATIVE_VEC_TYPE vec; \
public: \
    enum { SCALAR_COUNT = sizeof(NATIVE_VEC_TYPE) / sizeof(SCALAR_TYPE) }; \
    typedef TYPENAME VecLayout<sizeof(SCALAR_TYPE), SCALAR_COUNT> LO; \
    typedef TYPENAME PX_TYPESIGN(SCALAR_TYPE) SIGNED_SCALAR_TYPE; \
    FORCEINLINE operator NATIVE_VEC_TYPE const & () const { return vec; } \
    FORCEINLINE SCALAR_TYPE const * scalars() const { return (SCALAR_TYPE const *)&vec; } \
    FORCEINLINE SCALAR_TYPE * scalars() { return (SCALAR_TYPE *)&vec; } \
    PXVEC_IMPLEMENT_BASICDEF(TEMPLATE_PREFIX,TYPENAME)
#else
#define PXVEC_IMPLEMENT_DATA_GEN(TEMPLATE_PREFIX,TYPENAME,X) \
    enum { SCALAR_COUNT = sizeof(NATIVE_VEC_TYPE) / sizeof(SCALAR_TYPE) }; \
    typedef TYPENAME VecLayout<sizeof(SCALAR_TYPE), SCALAR_COUNT> LO; \
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

