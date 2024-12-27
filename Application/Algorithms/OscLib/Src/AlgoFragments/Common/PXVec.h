//TICS -*
#pragma once

namespace Philips
{
    namespace Medisys
    {
        namespace PXVec
        {

// To define a = b + c AND a += b
#define PXVEC_IMPLEMENT_BINARYOP(OP, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) {return SELF_TYPE(INTRIN(a, b)); }\
    FORCEINLINE SELF_TYPE & operator OP##=(SELF_TYPE const & a) { return *this = SELF_TYPE(INTRIN(*this, a)); }

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_COMPARE(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b)); }

// To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_COMPARE_OPCODE(OP, FUN, INTRIN, OPCODE) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b, OPCODE)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(a, b, OPCODE)); }

#define PXVEC_IMPLEMENT_COMPARE_REV(OP, FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b, a)); }\
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(INTRIN(b, a)); }

// To define a = class::f()
#define PXVEC_IMPLEMENT_CONSTANT(FUN, INTRIN) \
    static FORCEINLINE SELF_TYPE FUN() { return SELF_TYPE(INTRIN()); }

// To define a = f(b)
#define PXVEC_IMPLEMENT_UNARYFUN(FUN, INTRIN) \
    friend FORCEINLINE SELF_TYPE FUN(SELF_TYPE const & a) { return SELF_TYPE(INTRIN(a)); }

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

#define PXVEC_IMPLEMENT_LOADSTORE_M128I(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_load_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE load_low<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_si128((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_si128((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, __m128i(a)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(__m128i(a), mask, (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128F(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_load_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadu_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE load_low<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_ps((float *)ptr, __m128(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_ps((float *)ptr, __m128(a)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, I128vec1(__m128(a))); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, I128vec1(__m128(a))); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(I128vec1(__m128(a)), I128vec1(__m128(mask)), (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_M256F(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_load_ps((float const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm256_loadu_ps((float const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_store_ps((float *)ptr, __m256(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm256_storeu_ps((float *)ptr, __m256(a)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm256_maskstore_ps(ptr, _mm256_castps_si256(mask), __m256(a)); }

#define PXVEC_IMPLEMENT_LOADSTORE_M128D(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_load_pd((double const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadu_pd((double const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE lddqu<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_lddqu_si128((__m128i const *)ptr)); }\
    template <> FORCEINLINE SELF_TYPE load_low<SELF_TYPE>(SCALAR_TYPE const * ptr) { return SELF_TYPE(_mm_loadl_epi64((__m128i const *)ptr)); }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_store_pd((double *)ptr, __m128d(a)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeu_pd((double *)ptr, __m128d(a)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storel_epi64((__m128i *)ptr, I128vec1(__m128d(a))); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { _mm_storeh_epi64((__m128i *)ptr, I128vec1(__m128d(a))); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { _mm_maskmoveu_si128(I128vec1(__m128d(a)), I128vec1(__m128d(mask)), (char *)ptr); }

#define PXVEC_IMPLEMENT_LOADSTORE_2xM128X(SELF_TYPE, SCALAR_TYPE) \
    template <> FORCEINLINE SELF_TYPE loada<SELF_TYPE>(SCALAR_TYPE const * ptr) { SELF_TYPE res; res.set_half(0, loada<SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, loada<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); return res; }\
    template <> FORCEINLINE SELF_TYPE loadu<SELF_TYPE>(SCALAR_TYPE const * ptr) { SELF_TYPE res; res.set_half(0, loadu<SELF_TYPE::HALF_TYPE>(ptr)); res.set_half(1, loadu<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT))); return res; }\
    template <> FORCEINLINE void storea<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storea<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); storea<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    template <> FORCEINLINE void storeu<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); storeu<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::HALF_TYPE::SCALAR_COUNT), a.half(1)); }\
    template <> FORCEINLINE void store_low<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(0)); }\
    template <> FORCEINLINE void store_high<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a) { storeu<SELF_TYPE::HALF_TYPE>(ptr, a.half(1)); }\
    template <> FORCEINLINE SELF_TYPE partial_mask<SELF_TYPE>(int count) { return SELF_TYPE(partial_mask<SELF_TYPE::HALF_TYPE>(__min(count, (SELF_TYPE::SCALAR_COUNT>>1))), partial_mask<SELF_TYPE::HALF_TYPE>(__max(0, count-(SELF_TYPE::SCALAR_COUNT>>1)))); }\
    template <> FORCEINLINE void store_partial<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, int count) { store_partial<SELF_TYPE::HALF_TYPE>(ptr, a.half(0), __min(count, (SELF_TYPE::SCALAR_COUNT>>1))); if (count > (SELF_TYPE::SCALAR_COUNT>>1)) store_partial<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), count-(SELF_TYPE::SCALAR_COUNT>>1)); }\
    template <> FORCEINLINE void store_mask<SELF_TYPE>(SCALAR_TYPE * ptr, SELF_TYPE const & a, SELF_TYPE const & mask) { store_mask<SELF_TYPE::HALF_TYPE>(ptr, a.half(0), mask.half(0)); store_mask<SELF_TYPE::HALF_TYPE>(ptr+(SELF_TYPE::SCALAR_COUNT>>1), a.half(1), mask.half(1)); }

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(SELF_TYPE, SCALAR_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)\
    template <> FORCEINLINE SELF_TYPE load_scalar<SELF_TYPE>(SCALAR_TYPE const & a) { return SELF_TYPE(INTRIN_LOADSCALAR(a)); }\
    template <> FORCEINLINE SCALAR_TYPE store_scalar<SELF_TYPE>(SELF_TYPE const & a) { return (SCALAR_TYPE)INTRIN_STORESCALAR(a); }

#define PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD(SELF_TYPE, SCALAR_TYPE, INTRIN_LOADSCALAR, INTRIN_STORESCALAR)\
    template <> FORCEINLINE SELF_TYPE load_scalar<SELF_TYPE>(SCALAR_TYPE const & a) { return SELF_TYPE(INTRIN_LOADSCALAR(&a)); }\
    template <> FORCEINLINE SCALAR_TYPE store_scalar<SELF_TYPE>(SELF_TYPE const & a) { SCALAR_TYPE res; INTRIN_STORESCALAR(&res, a); return res; }

#define PXVEC_IMPLEMENT_UNARYMINUS() \
    FORCEINLINE SELF_TYPE operator -() const { return (zero() - *this); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128_NOSCALAR(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD) \
    FORCEINLINE SELF_TYPE() { }\
    FORCEINLINE SELF_TYPE(__m128 const & a) { vec = FROM_PS(a); }\
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec = FROM_PD(a); }\
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec = FROM_EPI(a); }

#define PXVEC_IMPLEMENT_CONSTRUCTORS_M128(SELF_TYPE, SCALAR_TYPE, FROM_EPI, FROM_PS, FROM_PD, FROM_SCALAR, SCALAR_CAST)\
    FORCEINLINE SELF_TYPE() { }\
    FORCEINLINE SELF_TYPE(__m128 const & a) { vec = FROM_PS(a); }\
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec = FROM_PD(a); }\
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec = FROM_EPI(a); }\
    FORCEINLINE SELF_TYPE(SCALAR_TYPE const & a) { vec = FROM_SCALAR(static_cast<SCALAR_CAST>(a)); }

#if PLATFORM_INTEL_SSE < 50
#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256_TO_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD)
#else
#define PXVEC_IMPLEMENT_CONSTRUCTORS_M256_TO_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD) \
    FORCEINLINE SELF_TYPE(__m256 const & a) { vec[0] = FROM_PS(_mm256_castps256_ps128(a)); vec[1] = FROM_PS(_mm256_extractf128_ps(a, 1)); } \
    FORCEINLINE SELF_TYPE(__m256d const & a) { vec[0] = FROM_PD(_mm256_castpd256_pd128(a)); vec[1] = FROM_PD(_mm256_extractf128_pd(a, 1)); } \
    FORCEINLINE SELF_TYPE(__m256i const & a) { vec[0] = FROM_EPI(_mm256_castsi256_si128(a)); vec[1] = FROM_EPI(_mm256_extractf128_si256(a, 1)); } \

#endif

#define PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(SELF_TYPE, SCALAR_TYPE, FROM_EPI, FROM_PS, FROM_PD, FROM_SCALAR, SCALAR_CAST) \
    FORCEINLINE SELF_TYPE() { }\
    FORCEINLINE SELF_TYPE(__m128  const (& a)[2]) { vec[0] =  FROM_PS(a[0]); vec[1] =  FROM_PS(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128d const (& a)[2]) { vec[0] =  FROM_PD(a[0]); vec[1] =  FROM_PD(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128i const (& a)[2]) { vec[0] = FROM_EPI(a[0]); vec[1] = FROM_EPI(a[1]); }\
    FORCEINLINE SELF_TYPE(__m128  const & a, __m128  const & b) { vec[0] =  FROM_PS(a); vec[1] =  FROM_PS(b); }\
    FORCEINLINE SELF_TYPE(__m128d const & a, __m128d const & b) { vec[0] =  FROM_PD(a); vec[1] =  FROM_PD(b); }\
    FORCEINLINE SELF_TYPE(__m128i const & a, __m128i const & b) { vec[0] = FROM_EPI(a); vec[1] = FROM_EPI(b); }\
    FORCEINLINE SELF_TYPE(__m128  const & a) { vec[0] = vec[1] =  FROM_PS(a); }\
    FORCEINLINE SELF_TYPE(__m128d const & a) { vec[0] = vec[1] =  FROM_PD(a); }\
    FORCEINLINE SELF_TYPE(__m128i const & a) { vec[0] = vec[1] = FROM_EPI(a); }\
    FORCEINLINE SELF_TYPE(SCALAR_TYPE const & a) { vec[0] = vec[1] = FROM_SCALAR(static_cast<SCALAR_CAST>(a)); } \
    PXVEC_IMPLEMENT_CONSTRUCTORS_M256_TO_2xM128(SELF_TYPE, FROM_EPI, FROM_PS, FROM_PD)

#ifndef _DEBUG
#define PXVEC_IMPLEMENT_DATA() \
private: \
    VECTOR_TYPE vec; \
public: \
    FORCEINLINE operator VECTOR_TYPE const & () const { return vec; }
#else
#define PXVEC_IMPLEMENT_DATA()\
    private: PLATFORM_ALIGNED(16) union { VECTOR_TYPE vec; SCALAR_TYPE s[SCALAR_COUNT]; }; public:\
    FORCEINLINE operator VECTOR_TYPE const & () const { return vec; }
#endif

#define PXVEC_IMPLEMENT_UNPACK(SELF_TYPE, DOUBLE_SELF_TYPE, INTRIN_LOW, INTRIN_HIGH)\
    template <> FORCEINLINE SELF_TYPE unpack_low<SELF_TYPE>(SELF_TYPE const & a, SELF_TYPE const & b) { return INTRIN_LOW(a, b); }\
    template <> FORCEINLINE SELF_TYPE unpack_high<SELF_TYPE>(SELF_TYPE const & a, SELF_TYPE const & b) { return INTRIN_HIGH(a, b); }\
    template <> FORCEINLINE DOUBLE_SELF_TYPE unpack<SELF_TYPE>(SELF_TYPE const & a, SELF_TYPE const & b) { return DOUBLE_SELF_TYPE(INTRIN_LOW(a, b), INTRIN_HIGH(a, b)); }

#define PXVEC_IMPLEMENT_2HALF()\
    FORCEINLINE HALF_TYPE half(int part) const { return HALF_TYPE(vec[part&1]); } \
    FORCEINLINE void set_half(int part, HALF_TYPE const & value) { vec[part&1] = value; }

struct I128vec1;

struct F64vec4;
struct F64vec2;
struct F32vec4;
struct F32vec8;

struct Is64vec2;
struct Is64vec4;

struct Is32vec4;
struct Is32vec8;

struct Is16vec8;
struct Is16vec16;

struct Is8vec16;
struct Is8vec32;

struct Iu8vec16;
struct Iu8vec32;

struct Iu16vec8;
struct Iu16vec16;

struct Iu32vec4;
struct Iu32vec8;

struct I128vec1
{
    typedef I128vec1 SELF_TYPE;
    typedef __m128i  SCALAR_TYPE;
    typedef __m128i  VECTOR_TYPE;
    enum { SCALAR_COUNT = 1 };
    
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128_NOSCALAR(I128vec1, , _mm_castps_si128, _mm_castpd_si128)
    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

//    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_si128, _mm_srli_si128, _mm_srli_si128)
};

struct Is64vec2
{
    typedef Is64vec2 SELF_TYPE;
    typedef Is64vec4 DOUBLE_TYPE;
    typedef int64_t  SCALAR_TYPE;
    typedef __m128i  VECTOR_TYPE;
    enum { SCALAR_COUNT = 2 };
    
    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Is64vec2, int64_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi64_int64, int64_t)

    FORCEINLINE Is64vec2(int64_t a0, int64_t a1) { vec = _mm_set_epi64_int64(a1, a0); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi64)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi64)

    PXVEC_IMPLEMENT_UNARYMINUS()

    friend FORCEINLINE int64_t sum(Is64vec2 const & a) {
        return _mm_sum_epi64(a);
    }
};

struct Is64vec4
{
    typedef Is64vec4 SELF_TYPE;
    typedef Is64vec2 HALF_TYPE;
    typedef int64_t  SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE[2];
    enum { SCALAR_COUNT = 4 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Is64vec4, int64_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi64_int64, int64_t)

    FORCEINLINE Is64vec4(int64_t a0, int64_t a1, int64_t a2, int64_t a3) { vec[0] = _mm_set_epi64_int64(a1, a0); vec[1] = _mm_set_epi64_int64(a2, a3);  }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi64)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi64)

    PXVEC_IMPLEMENT_UNARYMINUS()

    friend FORCEINLINE int64_t sum(Is64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }

    PXVEC_IMPLEMENT_2HALF()
};

struct Is32vec4
{
    typedef Is32vec4 SELF_TYPE;
    typedef Is32vec8 DOUBLE_TYPE;
    typedef int32_t  SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE;
    enum { SCALAR_COUNT = 4 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Is32vec4, int32_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi32, int32_t)

        FORCEINLINE Is32vec4(int32_t a0, int32_t a1, int32_t a2, int32_t a3) 
        { vec = _mm_set_epi32(a3, a2, a1, a0); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm_hadd_epi32)
    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi32)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi32)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epi32)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epi32)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_slli_epi32)
    PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi32 , _mm_sra_epi32 , _mm_sll_epi32)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi32)
    PXVEC_IMPLEMENT_COMPARE(< , cmplt, _mm_cmplt_epi32)
    PXVEC_IMPLEMENT_COMPARE(> , cmpgt, _mm_cmpgt_epi32)

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

struct Is32vec8
{
    typedef Is32vec8 SELF_TYPE;
    typedef Is32vec4 HALF_TYPE;
    typedef int32_t SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE[2];
    enum { SCALAR_COUNT = 8 };

    PXVEC_IMPLEMENT_DATA()

#if PLATFORM_INTEL_SSE >= 50
    FORCEINLINE operator __m256i const &() const
    {
        return _mm256_insertf128_si256(_mm256_castsi128_si256(vec[0]), vec[1], 1);
    }
#endif

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Is32vec8, int32_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi32, int32_t)

    FORCEINLINE Is32vec8(int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7) 
        { vec[0] = _mm_set_epi32(a3, a2, a1, a0); vec[1] = _mm_set_epi32(a7, a6, a5, a4); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mullo_epi32)
    PXVEC_IMPLEMENT_2BINARYFUN(horz_add, _mm_hadd_epi32)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(mul_low , _mm_mullo_epi32)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_high , _mm_mulhi_epi32)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi32)
    PXVEC_IMPLEMENT_2COMPARE    (< , cmplt, _mm_cmplt_epi32)
    PXVEC_IMPLEMENT_2COMPARE_REV(<=, cmple, _mm_cmpgt_epi32)
    PXVEC_IMPLEMENT_2COMPARE    (> , cmpgt, _mm_cmpgt_epi32)
    PXVEC_IMPLEMENT_2COMPARE_REV(>=, cmpge, _mm_cmplt_epi32)

    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_epi32)

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epi32)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epi32)

    PXVEC_IMPLEMENT_2HALF()

    friend FORCEINLINE Is32vec4 horz_add(SELF_TYPE const & a) { return horz_add(a.half(0), a.half(1)); }
    friend FORCEINLINE int32_t sum(Is32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
    friend Is16vec8 sat_pack(SELF_TYPE const & a);
};

struct Iu32vec4
{
    typedef Iu32vec4 SELF_TYPE;
    typedef Iu32vec8 DOUBLE_TYPE;
    typedef uint32_t SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE;
    enum { SCALAR_COUNT = 4 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Iu32vec4, uint32_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi32, int32_t)

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi32)

    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epu32)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epu32)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi32, _mm_srli_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi32, _mm_srl_epi32, _mm_srl_epi32)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi32)

    friend FORCEINLINE uint32_t sum(Iu32vec4 const & a) { return _mm_sum_epi32(a); }

//    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epu32)


//    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epu32)
};

struct Iu32vec8
{
    typedef Iu32vec8 SELF_TYPE;
    typedef Iu32vec4 HALF_TYPE;
    typedef uint32_t SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE[2];
    enum { SCALAR_COUNT = 8 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Iu32vec8, uint32_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi32, int32_t)

    FORCEINLINE Iu32vec8(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7) 
        { vec[0] = _mm_set_epi32(a3, a2, a1, a0); vec[1] = _mm_set_epi32(a7, a6, a5, a4); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi32)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi32)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi32)

    PXVEC_IMPLEMENT_2HALF()

    friend FORCEINLINE uint32_t sum(Iu32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is16vec8
{
    typedef Is16vec8 SELF_TYPE;
    typedef Is16vec16 DOUBLE_TYPE;
    typedef int16_t SCALAR_TYPE;
    enum { SCALAR_COUNT = 8 };
    typedef __m128i VECTOR_TYPE;

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Is16vec8, int16_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi16, int16_t)

    FORCEINLINE Is16vec8(int16_t a0, int16_t a1, int16_t a2, int16_t a3, int16_t a4, int16_t a5, int16_t a6, int16_t a7) 
        { vec = _mm_set_epi16(a7, a6, a5, a4, a3, a2, a1, a0); }

    FORCEINLINE Is16vec8(int16_t a0, int16_t a1) 
        { vec = _mm_set_epi16(a1, a0, a1, a0, a1, a0, a1, a0); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    static FORCEINLINE Is16vec8 zero_to_n() { return Is16vec8(0, 1, 2, 3, 4, 5, 6, 7); }

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

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

    friend FORCEINLINE Is32vec8 mul(SELF_TYPE const & a, SELF_TYPE const & b)
    {
        Is16vec8 lo = _mm_mullo_epi16(a, b), hi = _mm_mulhi_epi16(a, b);
        return Is32vec8(_mm_unpacklo_epi16(lo, hi), _mm_unpackhi_epi16(lo, hi));
    }

    friend FORCEINLINE Is32vec4 mul_add(SELF_TYPE const & a, SELF_TYPE const & b) { return Is32vec4(_mm_madd_epi16(a, b)); };

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi16, _mm_srai_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi16 , _mm_sra_epi16, _mm_srl_epi16)

    PXVEC_IMPLEMENT_COMPARE    (==, cmpeq, _mm_cmpeq_epi16)
    PXVEC_IMPLEMENT_COMPARE    (< , cmplt, _mm_cmplt_epi16)
    PXVEC_IMPLEMENT_COMPARE    (> , cmpgt, _mm_cmpgt_epi16)
    PXVEC_IMPLEMENT_COMPARE_REV(>=, cmpge, _mm_cmplt_epi16)
    PXVEC_IMPLEMENT_COMPARE_REV(<=, cmple, _mm_cmpgt_epi16)

    friend FORCEINLINE SELF_TYPE operator != (SELF_TYPE const & a, SELF_TYPE const & b)
    {
        return ((a == b) == zero());
    }

    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm_abs_epi16)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(avg, _mm_avg_epi16)
};

struct Iu16vec8
{
    typedef Iu16vec8 SELF_TYPE;
    typedef Iu16vec16 DOUBLE_TYPE;
    typedef uint16_t SCALAR_TYPE;
    enum { SCALAR_COUNT = 8 };
    typedef __m128i VECTOR_TYPE;

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Iu16vec8, uint16_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi16, int16_t)

    FORCEINLINE Iu16vec8(
        uint16_t a0, uint16_t a1, uint16_t a2, uint16_t a3,
        uint16_t a4, uint16_t a5, uint16_t a6, uint16_t a7) 
        { vec = _mm_set_epi16(static_cast<int16_t>(a7),
                              static_cast<int16_t>(a6),
                              static_cast<int16_t>(a5),
                              static_cast<int16_t>(a4),
                              static_cast<int16_t>(a3),
                              static_cast<int16_t>(a2),
                              static_cast<int16_t>(a1),
                              static_cast<int16_t>(a0)); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi16)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi16)

    PXVEC_IMPLEMENT_COMPARE(==, cmpeq, _mm_cmpeq_epi16)

    PXVEC_IMPLEMENT_BINARYFUN(mul_low , _mm_mullo_epi16)
    PXVEC_IMPLEMENT_BINARYFUN(mul_high , _mm_mulhi_epu16)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi16, _mm_srli_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi16 , _mm_srl_epi16, _mm_srl_epi16)

    //    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epu16)
//    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epu16)
};

FORCEINLINE Is16vec8 sat_pack(Is32vec8 const & a) { return _mm_packs_epi32(a.half(0), a.half(1)); }
FORCEINLINE Is16vec8 sat_pack(Is32vec4 const & a, Is32vec4 const & b) { return _mm_packs_epi32(a, b); }
FORCEINLINE Iu16vec8 usat_pack(Is32vec8 const & a) { return _mm_packus_epi32(a.half(0), a.half(1)); }
FORCEINLINE Iu16vec8 usat_pack(Is32vec4 const & a, Is32vec4 const & b) { return _mm_packus_epi32(a, b); }
FORCEINLINE Is16vec8 pack(Is32vec8 const & a) { return _mm_pack_epi32(a.half(0), a.half(1)); }
FORCEINLINE Is16vec8 pack(Is32vec4 const & a, Is32vec4 const & b) { return _mm_packs_epi32(a, b); }

struct Is16vec16
{
    typedef Is16vec16 SELF_TYPE;
    typedef Is16vec8  HALF_TYPE;
    typedef int16_t SCALAR_TYPE;
    enum { SCALAR_COUNT = 16 };
    typedef __m128i VECTOR_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Is16vec16, int16_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi16, int16_t)

#if PLATFORM_INTEL_SSE >= 50
    FORCEINLINE operator __m256i const &() const
    {
        return _mm256_insertf128_si256(_mm256_castsi128_si256(vec[0]), vec[1], 1);
    }
#endif

    FORCEINLINE Is16vec16(int16_t a0, int16_t a1, int16_t a2, int16_t a3, int16_t a4, int16_t a5, int16_t a6, int16_t a7,
                          int16_t a8, int16_t a9, int16_t aa, int16_t ab, int16_t ac, int16_t ad, int16_t ae, int16_t af) 
    {
        vec[0] = _mm_set_epi16(a7, a6, a5, a4, a3, a2, a1, a0);
        vec[1] = _mm_set_epi16(af, ae, ad, ac, ab, aa, a9, a8);
    }

    FORCEINLINE Is16vec16(int16_t a0, int16_t a1) 
    {
        vec[0] = vec[1] = _mm_set_epi16(a1, a0, a1, a0, a1, a0, a1, a0);
    }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    static FORCEINLINE Is16vec16 zero_to_n() { return Is16vec16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); }

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi16)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mullo_epi16)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_hrs, _mm_mulhrs_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_low , _mm_mullo_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(mul_high , _mm_mulhi_epi16)

    friend FORCEINLINE Is32vec8 mul_add(SELF_TYPE const & a, SELF_TYPE const & b) { return Is32vec8(mul_add(a.half(0), b.half(0)), mul_add(a.half(1), b.half(1))); };

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi16, _mm_srai_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi16 , _mm_sra_epi16 , _mm_srl_epi16)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi16)
    PXVEC_IMPLEMENT_2COMPARE    (< , cmplt, _mm_cmplt_epi16)
    PXVEC_IMPLEMENT_2COMPARE    (> , cmpgt, _mm_cmpgt_epi16)

    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_epi16)

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epi16)
    PXVEC_IMPLEMENT_2BINARYFUN(avg, _mm_avg_epi16)

    PXVEC_IMPLEMENT_2HALF()
};

struct Iu16vec16
{
    typedef Iu16vec16 SELF_TYPE;
    typedef Iu16vec8 HALF_TYPE;
    typedef uint16_t SCALAR_TYPE;
    enum { SCALAR_COUNT = 16 };
    typedef __m128i VECTOR_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_2HALF()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Iu16vec16, uint16_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi16, int16_t)

#if PLATFORM_INTEL_SSE >= 50
    FORCEINLINE operator __m256i const &() const
    {
        return _mm256_insertf128_si256(_mm256_castsi128_si256(vec[0]), vec[1], 1);
    }
#endif

    FORCEINLINE Iu16vec16(
        uint16_t a0, uint16_t a1, uint16_t a2, uint16_t a3,
        uint16_t a4, uint16_t a5, uint16_t a6, uint16_t a7,
        uint16_t a8, uint16_t a9, uint16_t aa, uint16_t ab,
        uint16_t ac, uint16_t ad, uint16_t ae, uint16_t af) 
    {
            vec[0] = _mm_set_epi16(static_cast<int16_t>(a7),
                                   static_cast<int16_t>(a6),
                                   static_cast<int16_t>(a5),
                                   static_cast<int16_t>(a4),
                                   static_cast<int16_t>(a3),
                                   static_cast<int16_t>(a2),
                                   static_cast<int16_t>(a1),
                                   static_cast<int16_t>(a0));
            vec[1] = _mm_set_epi16(static_cast<int16_t>(af),
                                   static_cast<int16_t>(ae),
                                   static_cast<int16_t>(ad),
                                   static_cast<int16_t>(ac),
                                   static_cast<int16_t>(ab),
                                   static_cast<int16_t>(aa),
                                   static_cast<int16_t>(a9),
                                   static_cast<int16_t>(a8));
    }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi16)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi16)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi16)

    PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi16, _mm_srli_epi16, _mm_srli_epi16)
    PXVEC_IMPLEMENT_2SHIFT (_mm_sll_epi16 , _mm_srl_epi16 , _mm_srl_epi16)
};

struct Is8vec16
{
    typedef Is8vec16 SELF_TYPE;
    typedef Is8vec32 DOUBLE_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE;
    enum { SCALAR_COUNT = 16 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Is8vec16, int8_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi8, int8_t)

    FORCEINLINE Is8vec16(int8_t a0, int8_t a1, int8_t a2, int8_t a3, int8_t a4, int8_t a5, int8_t a6, int8_t a7,
                         int8_t a8, int8_t a9, int8_t aa, int8_t ab, int8_t ac, int8_t ad, int8_t ae, int8_t af)
    {
        vec = _mm_set_epi8(af, ae, ad, ac, ab, aa, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0);
    }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_COMPARE    (==, cmpeq, _mm_cmpeq_epi8)
    PXVEC_IMPLEMENT_COMPARE    (< , cmplt, _mm_cmplt_epi8)
    PXVEC_IMPLEMENT_COMPARE    (> , cmpgt, _mm_cmpgt_epi8)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm_adds_epi8)
    PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm_subs_epi8)
    PXVEC_IMPLEMENT_UNARYFUN (abs    , _mm_abs_epi8 )
};

struct Iu8vec16
{
    typedef Iu8vec16 SELF_TYPE;
    typedef Iu8vec32 DOUBLE_TYPE;
    typedef uint8_t  SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE;
    enum { SCALAR_COUNT = 16 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(Iu8vec16, uint8_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi8, int8_t)

    FORCEINLINE Iu8vec16(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7,
                         uint8_t a8, uint8_t a9, uint8_t aa, uint8_t ab, uint8_t ac, uint8_t ad, uint8_t ae, uint8_t af)
    {
        vec = _mm_set_epi8(af, ae, ad, ac, ab, aa, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0);
    }

    static FORCEINLINE Iu8vec16 zero_to_n() { return Iu8vec16(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15); }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi8, _mm_srli_epi8, _mm_srli_epi8)
    //PXVEC_IMPLEMENT_SHIFT (_mm_sll_epi8 , _mm_srl_epi8)

    PXVEC_IMPLEMENT_COMPARE    (==, cmpeq, _mm_cmpeq_epi8)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi8)
};

struct Is8vec32
{
    typedef Is8vec32 SELF_TYPE;
    typedef Is8vec16 HALF_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE[2];
    enum { SCALAR_COUNT = 32 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_2HALF()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Is8vec32, int8_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi8, int8_t)

    FORCEINLINE Is8vec32(int8_t a0, int8_t a1, int8_t a2, int8_t a3, int8_t a4, int8_t a5, int8_t a6, int8_t a7,
                         int8_t a8, int8_t a9, int8_t aa, int8_t ab, int8_t ac, int8_t ad, int8_t ae, int8_t af,
                         int8_t b0, int8_t b1, int8_t b2, int8_t b3, int8_t b4, int8_t b5, int8_t b6, int8_t b7,
                         int8_t b8, int8_t b9, int8_t ba, int8_t bb, int8_t bc, int8_t bd, int8_t be, int8_t bf)
    {
        vec[0] = _mm_set_epi8(af, ae, ad, ac, ab, aa, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0);
        vec[1] = _mm_set_epi8(bf, be, bd, bc, bb, ba, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0);
    }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi8)
    PXVEC_IMPLEMENT_2COMPARE    (< , cmplt, _mm_cmplt_epi8)
    PXVEC_IMPLEMENT_2COMPARE    (> , cmpgt, _mm_cmpgt_epi8)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epi8)
    PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epi8)
    PXVEC_IMPLEMENT_2UNARYFUN (abs    , _mm_abs_epi8 )
};

FORCEINLINE Is8vec16 sat_pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_packs_epi16(a, b); }
FORCEINLINE Is8vec16 sat_pack(Is16vec16 const & a) { return _mm_packs_epi16(a.half(0), a.half(1)); }
FORCEINLINE Iu8vec16 usat_pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_packus_epi16(a, b); }
FORCEINLINE Iu8vec16 usat_pack(Is16vec16 const & a) { return _mm_packus_epi16(a.half(0), a.half(1)); }
FORCEINLINE Is8vec16 pack(Is16vec8 const & a, Is16vec8 const & b) { return _mm_pack_epi16(a, b); }
FORCEINLINE Is8vec16 pack(Is16vec16 const & a) { return _mm_pack_epi16(a.half(0), a.half(1)); }

struct Iu8vec32
{
    typedef Iu8vec32 SELF_TYPE;
    typedef Iu8vec16 HALF_TYPE;
    typedef uint8_t  SCALAR_TYPE;
    typedef __m128i VECTOR_TYPE[2];
    enum { SCALAR_COUNT = 32 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_2HALF()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(Iu8vec32, uint8_t, , _mm_castps_si128, _mm_castpd_si128, _mm_set1_epi8, int8_t)

    FORCEINLINE Iu8vec32(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7,
                         uint8_t a8, uint8_t a9, uint8_t aa, uint8_t ab, uint8_t ac, uint8_t ad, uint8_t ae, uint8_t af,
                         uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7,
                         uint8_t b8, uint8_t b9, uint8_t ba, uint8_t bb, uint8_t bc, uint8_t bd, uint8_t be, uint8_t bf)
    {
        vec[0] = _mm_set_epi8(af, ae, ad, ac, ab, aa, a9, a8, a7, a6, a5, a4, a3, a2, a1, a0);
        vec[1] = _mm_set_epi8(bf, be, bd, bc, bb, ba, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0);
    }

    PXVEC_IMPLEMENT_CONSTANT(zero, _mm_setzero_si128)
    PXVEC_IMPLEMENT_CONSTANT(minus_one, _mm_minusone_si128)

    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_si128)
    PXVEC_IMPLEMENT_2BINARYOP(|, _mm_or_si128 )
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_si128)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_si128)

    PXVEC_IMPLEMENT_2COMPARE    (==, cmpeq, _mm_cmpeq_epi8)

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi8)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi8)

    PXVEC_IMPLEMENT_UNARYMINUS()
};

struct F32vec4
{
    typedef F32vec8 DOUBLE_TYPE;
    typedef F32vec4 SELF_TYPE;
    typedef float   SCALAR_TYPE;
    typedef __m128  VECTOR_TYPE;
    enum { SCALAR_COUNT = 4 };

    PXVEC_IMPLEMENT_DATA()
    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(F32vec4, float, _mm_castsi128_ps, , _mm_castpd_ps, _mm_set1_ps, float)

    FORCEINLINE F32vec4(float a0, float a1, float a2, float a3)
    {
        vec = _mm_set_ps(a3, a2, a1, a0);
    }

    static FORCEINLINE F32vec4 zero() { return _mm_setzero_ps(); }
    static FORCEINLINE F32vec4 zero_to_n() { return F32vec4(0, 1, 2, 3); }
    static FORCEINLINE F32vec4 minus_one() { return F32vec4(_mm_minusone_si128()); }

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

    PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm_rsqrt_ps)
    PXVEC_IMPLEMENT_UNARYFUN(sqrt , _mm_sqrt_ps )
    PXVEC_IMPLEMENT_UNARYFUN(rcp  , _mm_rcp_ps  )
    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm_abs_ps  )

    friend FORCEINLINE float sum(F32vec4 const & a) { return _mm_sum_ps(a); }
    friend FORCEINLINE F64vec2 sum02_13(F32vec4 const & a);

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_ps)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_ps)
};

struct F64vec2
{
    typedef F64vec2 SELF_TYPE;
    typedef double   SCALAR_TYPE;
    typedef __m128d  VECTOR_TYPE;
    enum { SCALAR_COUNT = 2 };

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_M128(F64vec2, double, _mm_castsi128_pd, _mm_castps_pd, , _mm_set1_pd, double)

    FORCEINLINE F64vec2(double a0, double a1) { vec = _mm_set_pd(a1, a0); }

    static FORCEINLINE F64vec2 zero() { return _mm_setzero_pd(); }
    static FORCEINLINE F64vec2 zero_to_n() { return F64vec2(0.0, 1.0); }

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

#if PLATFORM_INTEL_SSE >= 50

struct F32vec8
{
    typedef F32vec8 SELF_TYPE;
    typedef float   SCALAR_TYPE;
    typedef __m256  VECTOR_TYPE;
    enum { SCALAR_COUNT = 8 };

    PXVEC_IMPLEMENT_DATA()

    FORCEINLINE F32vec8() { }

    FORCEINLINE F32vec8(__m128 const & a, __m128 const & b)
    {
        vec = _mm256_insertf128_ps(_mm256_castps128_ps256(a), (b), 0x1);
    }
    
    FORCEINLINE F32vec8(__m128i const & a, __m128i const & b)
    {
        vec = _mm256_insertf128_ps(_mm256_castps128_ps256(_mm_castsi128_ps(a)), _mm_castsi128_ps(b), 0x1);
    }

    FORCEINLINE F32vec8(__m256  const & a) { vec = a; }
    FORCEINLINE F32vec8(__m256i const & a) { vec = _mm256_castsi256_ps(a); }
    FORCEINLINE F32vec8(__m256d const & a) { vec = _mm256_castpd_ps(a); }

    FORCEINLINE F32vec8(float a) { vec = _mm256_broadcast_ss(&a); }
    FORCEINLINE F32vec8(float a0, float a1, float a2, float a3,
                        float a4, float a5, float a6, float a7)
    {
        vec = _mm256_set_ps(a7, a6, a5, a4, a3, a2, a1, a0);
    }

    static FORCEINLINE F32vec8 zero() { return F32vec8(_mm256_setzero_ps()); }

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

    PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm256_rsqrt_ps)
    PXVEC_IMPLEMENT_UNARYFUN(sqrt , _mm256_sqrt_ps )
    PXVEC_IMPLEMENT_UNARYFUN(rcp  , _mm256_rcp_ps  )
    PXVEC_IMPLEMENT_UNARYFUN(abs  , _mm256_abs_ps  )

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_ps)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_ps)

    friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }

    FORCEINLINE F32vec4 half(int i) const
    {
        if (!!(i & 1))
            return F32vec4(_mm256_castps256_ps128(vec));
        else
            return F32vec4(_mm256_extractf128_ps(vec, 1));
    }
};

#else

struct F32vec8
{
    typedef F32vec8 SELF_TYPE;
    typedef F32vec4 HALF_TYPE;
    typedef float   SCALAR_TYPE;
    enum { SCALAR_COUNT = 8 };
    typedef __m128  VECTOR_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

    PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128(F32vec8, float, _mm_castsi128_ps, , _mm_castpd_ps, _mm_set1_ps, float)

    FORCEINLINE F32vec8(float a0, float a1, float a2, float a3,
                        float a4, float a5, float a6, float a7)
    {
        vec[0] = _mm_set_ps(a3, a2, a1, a0);
        vec[1] = _mm_set_ps(a7, a6, a5, a4);
    }

    static FORCEINLINE F32vec8 zero() { return F32vec8(_mm_setzero_ps(), _mm_setzero_ps()); }
    static FORCEINLINE F32vec8 zero_to_n() { return F32vec8(0, 1, 2, 3, 4, 5, 6, 7); }

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

    PXVEC_IMPLEMENT_2UNARYFUN(rsqrt, _mm_rsqrt_ps)
    PXVEC_IMPLEMENT_2UNARYFUN(sqrt , _mm_sqrt_ps )
    PXVEC_IMPLEMENT_2UNARYFUN(rcp  , _mm_rcp_ps  )
    PXVEC_IMPLEMENT_2UNARYFUN(abs  , _mm_abs_ps  )

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_ps)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_ps)

    friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }

    PXVEC_IMPLEMENT_2HALF()
};

#endif

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
FORCEINLINE TYPE load_partial(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    PLATFORM_ALIGNED(16) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
    for (int i = 0; i < count; i++)
        tmp[i] = ptr[i];
    return loada<TYPE>(tmp);
}

template <bool ALIGNED, typename TYPE>
FORCEINLINE TYPE load(typename TYPE::SCALAR_TYPE const * ptr, int count)
{
    if (count == TYPE::SCALAR_COUNT)
        return load<ALIGNED, TYPE>(ptr);
    PLATFORM_ALIGNED(16) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
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
        return TYPE(left_mask_256[count*sizeof(typename TYPE::SCALAR_TYPE)]);
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
        PLATFORM_ALIGNED(16) typename TYPE::SCALAR_TYPE tmp[TYPE::SCALAR_COUNT];
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
        // FIXME: Will work only for F32vec8 / F64vec4 not for Is16vec16 (AVX2)
        _mm256_maskstore_ps((float *)ptr, _mm256_castps_si256(partial_mask<TYPE>(count)), F32vec8(vec));
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
FORCEINLINE void store(typename TYPE::SCALAR_TYPE * ptr, TYPE const & vec, int count)
{
    if (count == TYPE::SCALAR_COUNT) store<ALIGNED>(ptr, vec);
    else                              store_partial(ptr, vec, count);
}

PXVEC_IMPLEMENT_LOADSTORE_M128I(Is32vec4, int32_t)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is32vec4, int32_t, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu32vec4, uint32_t)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu32vec4, uint32_t, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_M128I(Is16vec8, int16_t)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is16vec8, int16_t, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu16vec8, uint16_t)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu16vec8, uint16_t, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_M128I(Is8vec16, int8_t)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Is8vec16, int8_t, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_M128I(Iu8vec16, uint8_t)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128I(Iu8vec16, uint8_t, _mm_cvtsi32_si128, _mm_cvtsi128_si32)

PXVEC_IMPLEMENT_LOADSTORE_M128F(F32vec4, float)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD(F32vec4, float, _mm_load_ss, _mm_store_ss)

PXVEC_IMPLEMENT_LOADSTORE_M128D(F64vec2, double)
PXVEC_IMPLEMENT_LOADSTORE_SCALAR_M128FD(F64vec2, double, _mm_load_sd, _mm_store_sd)

PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is8vec32 , int8_t  )
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu8vec32 , uint8_t )
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is16vec16, int16_t )
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu16vec16, uint16_t)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is32vec8 , int32_t )
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Iu32vec8 , uint32_t)
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(Is64vec4 , int64_t)

#if PLATFORM_INTEL_SSE < 50
PXVEC_IMPLEMENT_LOADSTORE_2xM128X(F32vec8, float)
#else
PXVEC_IMPLEMENT_LOADSTORE_M256F(F32vec8, float)
#endif

template <typename TYPE>
typename TYPE::DOUBLE_TYPE unpack(TYPE const & a, TYPE const & b);

template <typename TYPE>
TYPE unpack_low(TYPE const & a, TYPE const & b);

template <typename TYPE>
TYPE unpack_high(TYPE const & a, TYPE const & b);

FORCEINLINE float rcp(float a)
{
    return store_scalar(rcp(load_scalar<F32vec4>(a)));
}

PXVEC_IMPLEMENT_UNPACK(Is8vec16, Is8vec32 , _mm_unpacklo_epi8 , _mm_unpackhi_epi8 )
PXVEC_IMPLEMENT_UNPACK(Is16vec8, Is16vec16, _mm_unpacklo_epi16, _mm_unpackhi_epi16)
PXVEC_IMPLEMENT_UNPACK(Is32vec4, Is32vec8 , _mm_unpacklo_epi32, _mm_unpackhi_epi32)
PXVEC_IMPLEMENT_UNPACK(Is64vec2, Is64vec4 , _mm_unpacklo_epi64, _mm_unpackhi_epi64)

PXVEC_IMPLEMENT_UNPACK(Iu8vec16, Iu8vec32 , _mm_unpacklo_epi8 , _mm_unpackhi_epi8 )
PXVEC_IMPLEMENT_UNPACK(Iu16vec8, Iu16vec16, _mm_unpacklo_epi16, _mm_unpackhi_epi16)
PXVEC_IMPLEMENT_UNPACK(Iu32vec4, Iu32vec8 , _mm_unpacklo_epi32, _mm_unpackhi_epi32)
//PXVEC_IMPLEMENT_UNPACK(Iu16vec8, Iu16vec16, _mm_unpacklo_epi16, _mm_unpackhi_epi16)

template <int N, typename TYPE>
FORCEINLINE TYPE shiftreg_right(TYPE const & a) { return _mm_srli_si128(Is8vec16(a), N * sizeof(typename TYPE::SCALAR_TYPE)); }

template <int N, typename TYPE>
FORCEINLINE TYPE shiftreg_left(TYPE const & a) { return _mm_slli_si128(Is8vec16(a), N * sizeof(typename TYPE::SCALAR_TYPE)); }

template <int mask>
FORCEINLINE F32vec4 dotprod(F32vec4 const & a, F32vec4 const & b)
{
    return _mm_dp_ps(a, b, mask);
}

template <int N, typename TYPE>
struct insert_impl;

template <int N, typename TYPE>
struct extract_impl;

#define PXVEC_IMPLEMENT_INSERTEXTRACT(TYPE, INSERT_FUN, EXTRACT_FUN) \
    template <int N> struct insert_impl <N, TYPE> { TYPE res; FORCEINLINE insert_impl(TYPE const & a, typename TYPE::SCALAR_TYPE t) { res = INSERT_FUN(a, t, N); } FORCEINLINE operator TYPE() { return res; } }; \
    template <int N> struct extract_impl<N, TYPE> { typename TYPE::SCALAR_TYPE res; FORCEINLINE extract_impl(TYPE const & a) { res = (typename TYPE::SCALAR_TYPE)EXTRACT_FUN(a, N); } FORCEINLINE operator typename TYPE::SCALAR_TYPE() { return res; } };

template <int N, typename TYPE>
FORCEINLINE TYPE insert(TYPE const & a, typename TYPE::SCALAR_TYPE b)
{
    return insert_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>(a, b);
}

template <int N, typename TYPE>
FORCEINLINE typename TYPE::SCALAR_TYPE extract(TYPE const & a)
{
    return extract_impl<N & (TYPE::SCALAR_COUNT - 1), TYPE>(a);
}

PXVEC_IMPLEMENT_INSERTEXTRACT(Is8vec16, _mm_insert_epi8 , _mm_extract_epi8 )
PXVEC_IMPLEMENT_INSERTEXTRACT(Iu8vec16, _mm_insert_epi8 , _mm_extract_epi8 )
PXVEC_IMPLEMENT_INSERTEXTRACT(Is16vec8, _mm_insert_epi16, _mm_extract_epi16)
PXVEC_IMPLEMENT_INSERTEXTRACT(Iu16vec8, _mm_insert_epi16, _mm_extract_epi16)
PXVEC_IMPLEMENT_INSERTEXTRACT(Is32vec4, _mm_insert_epi32, _mm_extract_epi32)
PXVEC_IMPLEMENT_INSERTEXTRACT(Iu32vec4, _mm_insert_epi32, _mm_extract_epi32)
PXVEC_IMPLEMENT_INSERTEXTRACT(Is64vec2, _mm_insert_epi64, _mm_extract_epi64)
PXVEC_IMPLEMENT_INSERTEXTRACT(F32vec4, _mm_insert_ps, _mm_extract_ps)
PXVEC_IMPLEMENT_INSERTEXTRACT(F64vec2, _mm_insert_pd, _mm_extract_pd)

template <int N, typename TYPE, int VECTOR_SIZE>
struct combine_impl;

template <int N, typename TYPE>
struct combine_impl<N, TYPE, 16>
{
    TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE combine_impl(TYPE const & a, TYPE const & b)
    {
        if (N == 0)                                          res = a;
        else if (N*sizeof(typename TYPE::SCALAR_TYPE) == 16) res = b;
        else res = TYPE(I128vec1(_mm_alignr_epi8(I128vec1(b), I128vec1(a), N*sizeof(typename TYPE::SCALAR_TYPE))));
    }
};

#if 0

template <int N, typename TYPE>
struct combine_impl<N, TYPE, 32>
{
    TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE combine_impl(TYPE const & a, TYPE const & b)
    {
        Is32vec8 _a(a), _b(b);
        int const N_BYTES = N*sizeof(typename TYPE::SCALAR_TYPE);
        int const N_FLOAT = N_BYTES / 4;
        if (N == 0)             res = a;
        else if (N_BYTES == 32) res = b;
        /*else if (N_BYTES == 16) res = TYPE(a.half(1), b.half(0));
        else if (N_BYTES <  16)
        {
            res = TYPE(I128vec1(_mm_alignr_epi8(_a.half(1), _a.half(0), (N_BYTES < 16) * N_BYTES)),
                       I128vec1(_mm_alignr_epi8(_b.half(0), _a.half(1), (N_BYTES < 16) * N_BYTES)));
        }*/
        else
        {
            //res = TYPE(I128vec1(_mm_alignr_epi8(_b.half(0), _a.half(1), (N_BYTES >= 16) * (N_BYTES-16))),
            //           I128vec1(_mm_alignr_epi8(_b.half(1), _b.half(0), (N_BYTES >= 16) * (N_BYTES-16))));
            res = TYPE(_mm256_permute2_ps(b, a, _mm256_set_epi32(
                N_FLOAT + 7, N_FLOAT + 6, N_FLOAT + 5, N_FLOAT + 4, 
                N_FLOAT + 3, N_FLOAT + 2, N_FLOAT + 1,  N_FLOAT + 0), 0));
        }
    }
};

#else /* if PLATFORM_INTEL_SSE < 50 */

template <int N, typename TYPE>
struct combine_impl<N, TYPE, 32>
{
    TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE combine_impl(TYPE const & a, TYPE const & b)
    {
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
    }
};

#endif /* PLATFORM_INTEL_SSE >= 50 */

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
    const int M = __min(TYPE::SCALAR_COUNT, __max(0, N < 0 ? N + TYPE::SCALAR_COUNT : N));
    return combine_impl<M, TYPE, sizeof(TYPE)>(a, b);
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
    assert(__abs(N) <= TYPE::SCALAR_COUNT);
    const int M = N < 0 ? N + TYPE::SCALAR_COUNT : N;
    return TYPE(combine<M, TYPE>((N < 0) ? a : b, (N < 0) ? b : c));
}

FORCEINLINE F64vec2 sum02_13(F32vec4 const & a)
{
    return F64vec2(_mm_cvtps_pd(a + shiftreg_right<2>(a)));
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

template <typename TYPE>
struct select_impl<TYPE, 32> { TYPE res; FORCEINLINE operator TYPE() const { return res; }
    FORCEINLINE select_impl(TYPE const & mask, TYPE const & on, TYPE const & off) {
        res =  TYPE(_mm_blendv_epi8(I128vec1(off.half(0)), I128vec1(on.half(0)), I128vec1(mask.half(0))),
                    _mm_blendv_epi8(I128vec1(off.half(1)), I128vec1(on.half(1)), I128vec1(mask.half(1))));
    }
};
#endif /* PLATFORM_INTEL_SSE >= 40 */

template <typename TYPE>
FORCEINLINE TYPE select(TYPE const & mask, TYPE const & on, TYPE const & off)
{
    return select_impl<TYPE, sizeof(TYPE)>(mask, on, off);
}

template <typename TYPE_OUT, typename TYPE_IN>
TYPE_OUT convert(TYPE_IN const & src);

#if PLATFORM_INTEL_SSE >= 50

// 8xFloat32 to 8xInt32
template <>
FORCEINLINE Is32vec8 convert<Is32vec8, F32vec8>(F32vec8 const & src)
{
    return Is32vec8(_mm256_cvtps_epi32(src));
}

// 8xInt32 to 8xFloat32
template <>
FORCEINLINE F32vec8 convert<F32vec8, Is32vec8>(Is32vec8 const & src)
{
    return F32vec8(_mm256_cvtepi32_ps(src));
}

#else

// 8xFloat32 to 8xInt32
template <>
FORCEINLINE Is32vec8 convert<Is32vec8, F32vec8>(F32vec8 const & src)
{
    return Is32vec8(_mm_cvtps_epi32(src.half(0)),
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
FORCEINLINE F32vec8 convert<F32vec8, F32vec8>(F32vec8 const & src)
{
    return src;
}

template <>
FORCEINLINE Iu32vec8 convert<Iu32vec8, Iu16vec8>(Iu16vec8 const & src)
{
    return Iu32vec8(_mm_unpacklo_epi16(src, _mm_setzero_si128()),
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

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a)
{
    return TYPE(_mm_shuffle_epi32(I128vec1(a), _MM_SHUFFLE(A3, A2, A1, A0)));
}

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE TYPE shuffle_low(TYPE const & a)
{
    return TYPE(_mm_shufflelo_epi16(I128vec1(a), _MM_SHUFFLE(A3, A2, A1, A0)));
}

template <int A0, int A1, int A2, int A3, typename TYPE>
FORCEINLINE TYPE shuffle_high(TYPE const & a)
{
    return TYPE(_mm_shufflehi_epi16(I128vec1(a), _MM_SHUFFLE(A3, A2, A1, A0)));
}

template <int A0, int A1, int B2, int B3, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a, TYPE const & b)
{
    return TYPE(_mm_shuffle_ps(F32vec4(a), F32vec4(b), _MM_SHUFFLE(B3, B2, A1, A0)));
}

template <int A0, int B1, typename TYPE>
FORCEINLINE TYPE shuffle(TYPE const & a, TYPE const & b)
{
    return TYPE(_mm_shuffle_pd(F64vec2(a), F64vec2(b), _MM_SHUFFLE(0, 0, B1, A0)));
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
    return vec_to_mask((typename TYPE::VECTOR_TYPE const &)(a));
}

#if PLATFORM_INTEL_SSE < 50

template <>
FORCEINLINE int vec_to_mask<F32vec8>(F32vec8 const & a)
{
    return (vec_to_mask(a.half(0)) | (vec_to_mask(a.half(1))<<16));
}

#endif

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

template <typename TYPE>
FORCEINLINE bool vec_is_zero(TYPE const & a)
{
    return vec_is_zero((typename TYPE::VECTOR_TYPE const &)(a));
}

#if PLATFORM_INTEL_SSE < 50

template <>
FORCEINLINE bool vec_is_zero<F32vec8>(F32vec8 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

template <>
FORCEINLINE bool vec_is_zero<Is32vec8>(Is32vec8 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
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

template <>
FORCEINLINE bool vec_is_zero<Is16vec16>(Is16vec16 const & a)
{
    return vec_is_zero(a.half(0)) && vec_is_zero(a.half(1));
}

template <int SCALAR_COUNT, int SCALAR_SIZE, typename TYPE>
class convert_to_mask_impl
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input);
};

template <int SCALAR_COUNT, typename TYPE>
struct convert_to_mask_impl<SCALAR_COUNT, 1, TYPE>
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input)
    {
        mask = vec_to_mask(input);
    }
};

#if 0

template <int SCALAR_SIZE, typename TYPE>
struct convert_to_mask_impl<2, SCALAR_SIZE, TYPE>
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input)
    {
        mask = vec_to_mask(input);
        mask = (mask & 1) | (mask >> (sizeof(TYPE::SCALAR_TYPE) - 1) & 2);
    }
};


template <int SCALAR_SIZE, typename TYPE>
struct convert_to_mask_impl<4, SCALAR_SIZE, TYPE>
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input)
    {
        /*
        mask = vec_to_mask(input);
        mask = (mask & 1) | (mask >> (sizeof(TYPE::SCALAR_TYPE) - 1) & 2)
                          | (mask >> (2 * sizeof(TYPE::SCALAR_TYPE) - 2) & 4)
                          | (mask >> (3 * sizeof(TYPE::SCALAR_TYPE) - 3) & 8);
                          */
        mask = _mm_movemask_ps(input);
    }
};
#endif

template <typename TYPE>
struct convert_to_mask_impl<2, 8, TYPE>
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input)
    {
        /*
        mask = vec_to_mask(input);
        mask = (mask & 1) | (mask >> (sizeof(TYPE::SCALAR_TYPE) - 1) & 2)
                          | (mask >> (2 * sizeof(TYPE::SCALAR_TYPE) - 2) & 4)
                          | (mask >> (3 * sizeof(TYPE::SCALAR_TYPE) - 3) & 8);
                          */
        mask = _mm_movemask_pd(input);
    }
};

template <typename TYPE>
struct convert_to_mask_impl<4, 4, TYPE>
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input)
    {
        /*
        mask = vec_to_mask(input);
        mask = (mask & 1) | (mask >> (sizeof(TYPE::SCALAR_TYPE) - 1) & 2)
                          | (mask >> (2 * sizeof(TYPE::SCALAR_TYPE) - 2) & 4)
                          | (mask >> (3 * sizeof(TYPE::SCALAR_TYPE) - 3) & 8);
                          */
        mask = _mm_movemask_ps(input);
    }
};

template <int SCALAR_SIZE, typename TYPE>
struct convert_to_mask_impl<8, SCALAR_SIZE, TYPE>
{
    FORCEINLINE convert_to_mask_impl(int & mask, TYPE const & input)
    {
        mask = vec_to_mask(input);
        mask = (mask & 1) | (mask >> (sizeof(TYPE::SCALAR_TYPE) - 1) & 2)
                          | (mask >> (2 * sizeof(TYPE::SCALAR_TYPE) - 2) & 4)
                          | (mask >> (3 * sizeof(TYPE::SCALAR_TYPE) - 3) & 8)
                          | (mask >> (4 * sizeof(TYPE::SCALAR_TYPE) - 4) & 16)
                          | (mask >> (5 * sizeof(TYPE::SCALAR_TYPE) - 5) & 32)
                          | (mask >> (6 * sizeof(TYPE::SCALAR_TYPE) - 6) & 64)
                          | (mask >> (7 * sizeof(TYPE::SCALAR_TYPE) - 7) & 128);
    }
};

template <typename TYPE>
FORCEINLINE int convert_to_mask(TYPE const & a)
{
    int result;
    convert_to_mask_impl<TYPE::SCALAR_COUNT, sizeof(TYPE::SCALAR_TYPE), TYPE>(result, a);
    return result;
}

// Put on the lowest element the min element
template <typename TYPE>
FORCEINLINE TYPE min_inside(TYPE const & a)
{
    TYPE minValue = a;
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
    if (TYPE::SCALAR_COUNT >= 16) maxValue = max(maxValue, shiftreg_right<8>(maxValue));
    if (TYPE::SCALAR_COUNT >= 8)  maxValue = max(maxValue, shiftreg_right<4>(maxValue));
    if (TYPE::SCALAR_COUNT >= 4)  maxValue = max(maxValue, shiftreg_right<2>(maxValue));
    if (TYPE::SCALAR_COUNT >= 2)  maxValue = max(maxValue, shiftreg_right<1>(maxValue));
    return maxValue;
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

enum
{
    RoundingMode_Nearest = 0,
    RoundingMode_Floor = 1,
    RoundingMode_Ceil = 2,
    RoundingMode_Truncate = 3
};

template <int ROUNDING_MODE>
class vec_roundingmode
{
    int m_previousMode;

public:
    FORCEINLINE ~vec_roundingmode()
    {
        _MM_SET_ROUNDING_MODE(m_previousMode);
    }

    FORCEINLINE vec_roundingmode()
    {
        m_previousMode = _MM_GET_ROUNDING_MODE();
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
#ifdef _DEBUG
    union
    {
        typename TYPE::VECTOR_TYPE vec[N];
        typename TYPE::SCALAR_TYPE s[N*TYPE::SCALAR_COUNT];
        typename TYPE::SCALAR_TYPE t[N][TYPE::SCALAR_COUNT];
    };
#else
    typename TYPE::VECTOR_TYPE vec[N];
#endif

    FORCEINLINE TYPE   operator[](int i) const { return TYPE   (vec[i]); }
    FORCEINLINE TYPE & operator[](int i)       { return (TYPE &)vec[i]; }
};

// This array can hold complex<TYPE> but also fake complex<TYPE> with only a real part (and sizeof(fake complex<TYPE>) == sizeof(TYPE)).
template <typename TYPE, typename _COMPLEX_TYPE, int N>
struct ComplexVecArray
{
    typedef _COMPLEX_TYPE COMPLEX_TYPE;
    enum { Factor = sizeof(COMPLEX_TYPE)/sizeof(TYPE) };
#ifdef _DEBUG
    union
    {
        typename TYPE::VECTOR_TYPE vec[Factor*N];
        COMPLEX_TYPE cvec[Factor*N];
        typename TYPE::SCALAR_TYPE s[Factor*N*TYPE::SCALAR_COUNT];
        typename TYPE::SCALAR_TYPE t[N][Factor][TYPE::SCALAR_COUNT];
    };
#else
    typename TYPE::VECTOR_TYPE vec[Factor*N];
#endif
    struct ElemRef
    {
        typename TYPE::VECTOR_TYPE & m_vec1;
        typename TYPE::VECTOR_TYPE & m_vec2;
        FORCEINLINE ElemRef(
            typename TYPE::VECTOR_TYPE & vec1,
            typename TYPE::VECTOR_TYPE & vec2)
        : m_vec1(vec1), m_vec2(vec2)
        {
        }

        operator COMPLEX_TYPE() const
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

    FORCEINLINE COMPLEX_TYPE operator[](int i) const { return COMPLEX_TYPE(vec[Factor*i], vec[Factor*i+(Factor-1)]); }
    FORCEINLINE void Set(int i, COMPLEX_TYPE const & c) {
        vec[Factor*i  ] = c.real();
        vec[Factor*i+(Factor-1)] = c.imag();
    }
    FORCEINLINE ElemRef      operator[](int i)       { return ElemRef(vec[Factor*i], vec[Factor*i+(Factor-1)]); }
};

template <typename TYPE, int N>
struct RefVecArray
{
    typename TYPE::VECTOR_TYPE (& vec)[N];

    FORCEINLINE RefVecArray(typename TYPE::VECTOR_TYPE (& _vec)[N]) : vec(_vec) { }

    FORCEINLINE TYPE    operator[](int i) const { assert(i < N); return TYPE(vec[i]); }
    FORCEINLINE TYPE &  operator[](int i)       { assert(i < N); return (TYPE &)(vec[i]); }
private:
    RefVecArray & operator =(RefVecArray other);
};

template <typename TYPE, int N>
struct ConstRefVecArray
{
    typename TYPE::VECTOR_TYPE const (& vec)[N];

    FORCEINLINE ConstRefVecArray(typename TYPE::VECTOR_TYPE const (& _vec)[N]) : vec(_vec) { }

    FORCEINLINE TYPE   operator[](int i) const { assert(i < N); return TYPE   (vec[i]); }
};

template <typename TYPE, int N, int M>
struct VecArray2
{
#ifdef _DEBUG
    union
    {
        typename TYPE::VECTOR_TYPE vec[N][M];
        typename TYPE::SCALAR_TYPE s[TYPE::SCALAR_COUNT * N * M];
        typename TYPE::SCALAR_TYPE t[N][M][TYPE::SCALAR_COUNT];
    };
#else
    typename TYPE::VECTOR_TYPE vec[N][M];
#endif

    FORCEINLINE ConstRefVecArray<TYPE, M> operator[](int i) const { assert(i < N); return ConstRefVecArray<TYPE, M>(vec[i]); }
    FORCEINLINE      RefVecArray<TYPE, M> operator[](int i)       { assert(i < N); return      RefVecArray<TYPE, M>(vec[i]); }
};

template <typename SCALAR_TYPE>
struct VecType;
template <typename SCALAR_TYPE, int N>
struct VecTypeForSize;

#define PXVEC_IMPLEMENT_VECTYPE(_TYPE) \
    template <> struct VecTypeForSize<_TYPE::SCALAR_TYPE, _TYPE::SCALAR_COUNT> { typedef _TYPE TYPE; };

#define PXVEC_IMPLEMENT_BEST_VECTYPE(_TYPE) \
    template <> struct VecType<_TYPE::SCALAR_TYPE> { typedef _TYPE TYPE; }; \
    PXVEC_IMPLEMENT_VECTYPE(_TYPE)


PXVEC_IMPLEMENT_BEST_VECTYPE(Is8vec16)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu8vec16)
PXVEC_IMPLEMENT_BEST_VECTYPE(Is16vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu16vec8)
PXVEC_IMPLEMENT_BEST_VECTYPE(Is32vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(Iu32vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(Is64vec2)
//PXVEC_IMPLEMENT_VECTYPE(uint64_t, Iu64vec2)

PXVEC_IMPLEMENT_VECTYPE(Is32vec8)
PXVEC_IMPLEMENT_VECTYPE(Is16vec16)

#if PLATFORM_INTEL_SSE >= 50
PXVEC_IMPLEMENT_BEST_VECTYPE(F32vec8)
//PXVEC_IMPLEMENT_BEST_VECTYPE(F64vec4)
PXVEC_IMPLEMENT_VECTYPE(F32vec4)
PXVEC_IMPLEMENT_VECTYPE(F64vec2)
#else
PXVEC_IMPLEMENT_VECTYPE(F32vec8)
//PXVEC_IMPLEMENT_VECTYPE(F64vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(F32vec4)
PXVEC_IMPLEMENT_BEST_VECTYPE(F64vec2)
#endif

#undef PXVEC_IMPLEMENT_VECTYPE
#undef PXVEC_IMPLEMENT_BEST_VECTYPE

#define PXVEC_TYPE(SCALAR_TYPE) VecType<SCALAR_TYPE>::TYPE
#define PXVEC_TYPE_FORSIZE(SCALAR_TYPE, N) VecTypeForSize<SCALAR_TYPE, N>::TYPE
#define PXVEC_TYPE_COUNT(SCALAR_TYPE) VecType<SCALAR_TYPE>::TYPE::SCALAR_COUNT

        }
    }
}
