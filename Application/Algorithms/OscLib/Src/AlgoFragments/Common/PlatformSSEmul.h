/*!
* \file PlatformSSEmul.h
*
* \author
* Emmanuel Attia (emmanuel.attia@philips.com)
* The REACT Project - MedISys Research Lab - Philips HealthCare
*
* \brief
* REACT internal productivity tool: SSE forward compatibility layer.
*
* \date 19 july 2010: Initial header.
*
*/
//TICS -*
#pragma once

#pragma warning(disable:4512)

/* Former emmextensions.h */
#include <limits>

template <typename TYPE>
FORCEINLINE TYPE square(TYPE const & a)
{
    return a*a;
}

template <typename TYPE>
FORCEINLINE TYPE mulfix(TYPE const & a, TYPE const & b)
{
    return a*b;
}

template <typename TYPE>
FORCEINLINE TYPE mulfix2(TYPE const & a, TYPE const & b)
{
    return 2*a*b;
}

template <typename FLOAT_TYPE>
FORCEINLINE int round_pos(const FLOAT_TYPE & a)
{
    return (a>=::std::numeric_limits<int>::max()-0.5) ? ::std::numeric_limits<int>::max() : ((int)(a + 0.5));
}

template <typename FLOAT_TYPE>
FORCEINLINE int round_neg(const FLOAT_TYPE & a)
{
    return (a<=::std::numeric_limits<int>::min()+0.5) ? ::std::numeric_limits<int>::min() : ((int)(a - 0.5));
}

template <typename FLOAT_TYPE>
FORCEINLINE int round(FLOAT_TYPE const & a)
{
    return (a >= 0) ? round_pos(a) : round_neg(a);
}

//template <typename FLOAT_TYPE>
//FORCEINLINE int round_pos(const FLOAT_TYPE & a)
//{
//    return (int)(a + 0.5);
//}

//template <typename FLOAT_TYPE>
//FORCEINLINE int round_neg(const FLOAT_TYPE & a)
//{
//    return (int)(a - 0.5);
//}

//template <typename FLOAT_TYPE>
//FORCEINLINE int round(const FLOAT_TYPE & a)
//{
//    return (a >= 0) ? (int)(a + 0.5) : (int)(a - 0.5);
//}

template <typename OUT_TYPE, typename FLOAT_TYPE>
struct do_round_if_int
{
    FORCEINLINE OUT_TYPE round_if_int(const FLOAT_TYPE & a)
    {
        return (a >= 0) ? (OUT_TYPE)(a + 0.5) : (OUT_TYPE)(a - 0.5);
    }
};

template <typename FLOAT_TYPE>
struct do_round_if_int<int, FLOAT_TYPE>
{
    static FORCEINLINE int round_if_int(const FLOAT_TYPE & a)
    {
        return round<FLOAT_TYPE>(a);
    }
};

template <>
struct do_round_if_int<float, float>
{
    static FORCEINLINE float round_if_int(const float & a)
    {
        return float(a);
    }
};

template <typename OUT_TYPE, typename FLOAT_TYPE>
FORCEINLINE OUT_TYPE round_if_int(const FLOAT_TYPE & a)
{
    return do_round_if_int<OUT_TYPE, FLOAT_TYPE>::round_if_int(a);
}

//template <typename OUT_TYPE, typename FLOAT_TYPE>
//FORCEINLINE OUT_TYPE round_if_int(const FLOAT_TYPE & a)
//{
//    return (a >= 0) ? (OUT_TYPE)(a + 0.5) : (OUT_TYPE)(a - 0.5);
//}
//
//template <>
//FORCEINLINE float round_if_int<float, float>(const float & a)
//{
//    return float(a);
//}

//template <typename FLOAT_TYPE>
//FORCEINLINE int floor(const FLOAT_TYPE & a)
//{
//    return (a >= 0) ? (int)(a) : (int)(a - 1);
//}

template <typename FLOAT_TYPE>
FORCEINLINE int floor(const FLOAT_TYPE & a)
{
    if (a >= 0)
    {
        return (a >= ::std::numeric_limits<int>::max()) ? ::std::numeric_limits<int>::max() : ((int)(a));
    }
    else
    {
        if (a < ::std::numeric_limits<int>::min()+1) return ::std::numeric_limits<int>::min();
        int ia = ((int)(a));
        return (a == ia) ? ia : (ia-1);
    }
}

template <typename IN_TYPE>
FORCEINLINE IN_TYPE idiv_floor(IN_TYPE a, IN_TYPE b)
{
    return (a >= 0) ? (IN_TYPE)(a / b) : (IN_TYPE)((a - (b - 1)) / b);
}

template <typename IN_TYPE, typename REM_TYPE>
FORCEINLINE IN_TYPE idiv_floor(REM_TYPE & r, IN_TYPE a, IN_TYPE b)
{
    IN_TYPE q  = idiv_floor(a, b);
    r = (REM_TYPE)(a - q * b);
    return q;
}

template <typename IN_TYPE>
FORCEINLINE IN_TYPE idiv_ceil(IN_TYPE a, IN_TYPE b)
{
    return (a <= 0) ? (IN_TYPE)(a / b) : (IN_TYPE)((a + (b - 1)) / b);
}

template <typename IN_TYPE, typename REM_TYPE>
FORCEINLINE IN_TYPE idiv_ceil(REM_TYPE & r, IN_TYPE a, IN_TYPE b)
{
    IN_TYPE q  = idiv_ceil(a, b);
    r = (REM_TYPE)(a - q * b);
    return q;
}

template <typename IN_TYPE>
FORCEINLINE IN_TYPE idiv_round(IN_TYPE a, IN_TYPE b)
{
    return (a <= 0) ? (int)((a - b/2) / b) : (int)((a + (b/2)) / b);
}

template <typename IN_TYPE, typename REM_TYPE>
FORCEINLINE IN_TYPE idiv_round(REM_TYPE & r, IN_TYPE a, IN_TYPE b)
{
    IN_TYPE q  = idiv_round(a, b);
    r = (REM_TYPE)(a - q * b);
    return q;
}

FORCEINLINE bool bsf(unsigned int & index, uint32_t mask)
{
    return (!!_BitScanForward((unsigned long *)&index, (unsigned long)mask));
}

FORCEINLINE bool bsr(unsigned int & index, uint32_t mask)
{
    return (!!_BitScanReverse((unsigned long *)&index, (unsigned long)mask));
}

#if PLATFORM_INTEL_ARCH >= 64

FORCEINLINE bool bsf(unsigned int & index, uint64_t mask)
{
    return (!!_BitScanForward64((unsigned long *)&index, mask));
}

FORCEINLINE bool bsr(unsigned int & index, uint64_t mask)
{
    return (!!_BitScanReverse64((unsigned long *)&index, mask));
}

#endif

template <>
//FORCEINLINE int round<float>(const float & a) // not compatible with GENIAL
FORCEINLINE int round(const float & a)
{
    return (a>=::std::numeric_limits<int>::max()) ? ::std::numeric_limits<int>::max() : ((a<=::std::numeric_limits<int>::min()) ? ::std::numeric_limits<int>::min() : _mm_cvt_ss2si(_mm_load_ss(&a)));
}

template <typename TYPE>
FORCEINLINE TYPE flttofix(const float & a)
{
    return (TYPE)a;
}

template <>
FORCEINLINE int16_t flttofix<int16_t>(const float & a)
{
    return (int16_t)round(((1<<15)-1)*(a));
}

template <typename TYPE>
FORCEINLINE TYPE dbltofix(const double & a)
{
    return (TYPE)a;
}

template <>
FORCEINLINE int16_t dbltofix<int16_t>(const double & a)
{
    return (int16_t)round(((1<<15)-1)*(a));
}

template <typename INT_TYPE, typename TYPE>
FORCEINLINE INT_TYPE saturate(TYPE a)
{
    if (a < (TYPE)std::numeric_limits<INT_TYPE>::max())
    {
        if (a > (TYPE)std::numeric_limits<INT_TYPE>::min())
            return (INT_TYPE)a;
        return std::numeric_limits<INT_TYPE>::min();
    }
    return std::numeric_limits<INT_TYPE>::max();
}

template <>
FORCEINLINE float saturate<float, float>(float a)
{
    return a;
}

template <>
FORCEINLINE double saturate<double, double>(double a)
{
    return a;
}

template <>
FORCEINLINE int16_t mulfix<int16_t>(const int16_t & a, const int16_t & b)
{
    return (((a*b) + (1<<14))>>15);
}

template <>
FORCEINLINE int16_t mulfix2<int16_t>(const int16_t & a, const int16_t & b)
{
    return (((a*b) + (1<<13))>>14);
}

// Returns -1 if x <= 0 otherwise returns floor(log2(x))
template <typename T>
FORCEINLINE T log2_floor(T x)
{
    T result = 0;
    while (x > 0)
    {
        result++;
        x >>= 1;
    }
    return (result-1);
}

// Returns -1 if x <= 0 otherwise returns ceil(log2(x))
template <typename T>
FORCEINLINE T log2_ceil(T x)
{
    T result = log2_floor(x);
    if ((1<<result) < x)
        result++;
    return result;
}

template <typename TYPE, typename SIMDTYPE>
FORCEINLINE int ComputeMisalignment(int iMinLength, TYPE * ptr)
{
    const uintptr_t N = sizeof(SIMDTYPE)/sizeof(TYPE);
    const uintptr_t elementsFromZero = (ptr - (TYPE *)NULL);
    if (iMinLength < N)
        return iMinLength;
    else
        return (int)__min(iMinLength, (N - elementsFromZero) & (N - 1));
}


FORCEINLINE __m128i _mm_minusone_si128_emul()
{
    return _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
}

#define _mm_minusone_si128 _mm_minusone_si128_emul

FORCEINLINE __m128i _mm_pack_epi32(__m128i const & a, __m128i const & b)
{
    return _mm_packs_epi32(_mm_srai_epi32(_mm_slli_epi32( a, 16), 16),
  			               _mm_srai_epi32(_mm_slli_epi32( b, 16), 16));
}

FORCEINLINE __m128i _mm_pack_epi16(__m128i const & a, __m128i const & b)
{
    return _mm_packs_epi16(_mm_srai_epi16(_mm_slli_epi16( a, 8), 8),
  			               _mm_srai_epi16(_mm_slli_epi16( b, 8), 8));
}

//#if PLATFORM_INTEL_SSE_COMPILABLE < 0 // Patrick LAURENT: temporary changed for VC2003, Intel 8
#if PLATFORM_INTEL_SSE_COMPILABLE < 35

// Compiler does not support casting intrinsics

FORCEINLINE __m128 & _mm_castsi128_ps(__m128i & m)
{
    return reinterpret_cast<__m128 &>(m);
}

FORCEINLINE __m128 const & _mm_castsi128_ps(__m128i const & m)
{
    return reinterpret_cast<__m128 const &>(m);
}

FORCEINLINE __m128i & _mm_castps_si128(__m128 & m)
{
    return reinterpret_cast<__m128i &>(m);
}

FORCEINLINE __m128i const & _mm_castps_si128(__m128 const & m)
{
    return reinterpret_cast<__m128i const &>(m);
}


FORCEINLINE __m128d & _mm_castsi128_pd(__m128i & m)
{
    return reinterpret_cast<__m128d &>(m);
}

FORCEINLINE __m128d const & _mm_castsi128_pd(__m128i const & m)
{
    return reinterpret_cast<__m128d const &>(m);
}

FORCEINLINE __m128i const & _mm_castpd_si128(__m128d const & m)
{
    return reinterpret_cast<__m128i const &>(m);
}
FORCEINLINE __m128i & _mm_castpd_si128(__m128d & m)
{
    return reinterpret_cast<__m128i &>(m);

}

FORCEINLINE __m128d & _mm_castps_pd(__m128 & m)
{
    return reinterpret_cast<__m128d &>(m);
}

FORCEINLINE __m128d const & _mm_castps_pd(__m128 const & m)
{
    return reinterpret_cast<__m128d const &>(m);
}

FORCEINLINE __m128 const & _mm_castpd_ps(__m128d const & m)
{
    return reinterpret_cast<__m128 const &>(m);
}
FORCEINLINE __m128 & _mm_castpd_ps(__m128d & m)
{
    return reinterpret_cast<__m128 &>(m);

}

#endif /* PLATFORM_INTEL_SSE_COMPILABLE < 35 */

FORCEINLINE float _mm_sum_ps(__m128 const & a)
{
    float f; __m128 tmp;
    tmp = _mm_add_ps(a, _mm_shuffle_ps(a, a, _MM_SHUFFLE(3, 3, 1, 1)));
    tmp = _mm_add_ps(tmp, _mm_shuffle_ps(tmp, tmp, _MM_SHUFFLE(2, 2, 2, 2)));
    _mm_store_ss(&f, tmp);
    return f;
}

FORCEINLINE double _mm_sum_pd(__m128d const & a)
{
    double f; __m128d tmp;
    tmp = _mm_add_pd(a, _mm_shuffle_pd(a, a, _MM_SHUFFLE2(1, 1)));
    _mm_store_sd(&f, tmp);
    return f;
}

FORCEINLINE int32_t _mm_sum_epi32(__m128i const & a)
{
    __m128i tmp;
    tmp = _mm_add_epi32(a, _mm_srli_si128(a, 4));
    tmp = _mm_add_epi32(tmp, _mm_srli_si128(tmp, 8));
    return _mm_cvtsi128_si32(tmp);
}

FORCEINLINE int64_t _mm_sum_epi64(__m128i const & a)
{
    union
    {
        int64_t f[2];
        __m128i tmp;
    };

    tmp = _mm_add_epi64(a, _mm_srli_si128(a, 8));
    return f[0];
}

FORCEINLINE __m128 _mm_abs_ps_emul(__m128 const & a)
{
    return _mm_castsi128_ps(_mm_andnot_si128(_mm_set1_epi32(0x80000000UL), _mm_castps_si128(a)));
}

#define _mm_abs_ps _mm_abs_ps_emul

FORCEINLINE __m128d _mm_abs_pd_emul(__m128d a)
{
    return _mm_castsi128_pd(_mm_andnot_si128(_mm_set_epi32(0x80000000UL, 0, 0x80000000UL, 0), _mm_castpd_si128(a)));
}

#define _mm_abs_pd _mm_abs_pd_emul

#if PLATFORM_INTEL_SSE >= 50

FORCEINLINE __m256 _mm256_abs_ps_emul(__m256 a)
{
    return _mm256_and_ps(_mm256_castsi256_ps(_mm256_set1_epi32(0x80000000UL)), a);
}

#define _mm256_abs_ps _mm256_abs_ps_emul

FORCEINLINE __m256d _mm256_abs_pd_emul(__m256d a)
{
    return _mm256_and_pd(_mm256_castsi256_pd(_mm256_set_epi32(0x80000000UL, 0, 0x80000000UL, 0, 0x80000000UL, 0, 0x80000000UL, 0)), a);
}

#define _mm256_abs_pd _mm256_abs_pd_emul

#endif

#if PLATFORM_INTEL_SSE < 40

/* Insert / Extract SSE4 emulation */
template <int ndx>
FORCEINLINE int _mm_extract_epi8_emul(__m128i const & src)
{
    if (ndx & 1)
        return _mm_extract_epi16(src, ndx/2) >> 8;
    return _mm_extract_epi16(src, ndx/2) & 0xff;
}

template <int ndx>
FORCEINLINE __m128i _mm_insert_epi8_emul(__m128i const & s1, int s2)
{
    if (ndx & 1)
        return _mm_insert_epi16(s1, (_mm_extract_epi16(s1, ndx/2) & 0xff) | (s2 << 8), ndx/2);
    return _mm_insert_epi16(s1, (_mm_extract_epi16(s1, ndx/2) & 0xff00) | (s2 & 0xff), ndx/2);
}

template <int N>
FORCEINLINE int _mm_extract_epi32_emul(__m128i const & src)
{
    if (N == 0)
        return _mm_cvtsi128_si32(src);
    return _mm_cvtsi128_si32(_mm_srli_si128(src, N*4));
}

template <int N>
FORCEINLINE __m128i _mm_insert_epi32_emul(__m128i const & s1, int s2)
{
    return _mm_insert_epi16(
        _mm_insert_epi16(s1, int16_t(s2), N*2), int16_t(s2 >> 16), N*2+1);
}

// TICS -PRE#002
// TICS bug here
#define   _mm_insert_epi8(s1, s2, N)  _mm_insert_epi8_emul  < (N) >((s1),(s2))
#define  _mm_insert_epi32(s1, s2, N)  _mm_insert_epi32_emul < (N) >((s1),(s2))
#define  _mm_extract_epi8(src, N)     _mm_extract_epi8_emul < (N) >(src)
#define _mm_extract_epi32(src, N)     _mm_extract_epi32_emul< (N) >(src)

// TICS +PRE#002

template <int N>
FORCEINLINE float _mm_extract_ps_emul(__m128 const & src)
{
    int32_t t = _mm_extract_epi32(_mm_castps_si128(src), N);
    return *reinterpret_cast<float *>(&t);
}

template <int N>
FORCEINLINE __m128 _mm_insert_ps_emul(__m128 const & s1, float s2)
{
    return _mm_castsi128_ps(_mm_insert_epi32(_mm_castps_si128(s1), *reinterpret_cast<int32_t *>(&s2), N));
}

#define  _mm_insert_ps(s1, s2, N)  _mm_insert_ps_emul < (N) >((s1),(s2))
#define _mm_extract_ps(src, N)     _mm_extract_ps_emul< (N) >(src)

#endif /* PLATFORM_INTEL_SSE < 40 */

#if PLATFORM_INTEL_SSE < 40 || !defined(_M_X64)

template <int N>
FORCEINLINE int64_t _mm_extract_epi64_emul(__m128i const & src)
{
    int64_t l1 = _mm_extract_epi32(src, N*2);
    int64_t l2 = _mm_extract_epi32(src, N*2+1);
    return l1 | (l2 << 32LL);
}

template <int N>
FORCEINLINE __m128i _mm_insert_epi64_emul(__m128i const & s1, int64_t s2)
{
    return _mm_insert_epi32(
        _mm_insert_epi32(s1, int32_t(s2 >> 32), N*2+1), int32_t(s2), N*2+0);
}

// TICS -PRE#002
#define _mm_extract_epi64(src, N)    _mm_extract_epi64_emul<(N)>(src)
#define _mm_insert_epi64(s1, s2, N)  _mm_insert_epi64_emul<(N)>((s1), (s2))
// TICS +PRE#002

#endif /* PLATFORM_INTEL_SSE < 40 || !defined(_M_X64) */

template <int N>
FORCEINLINE double _mm_extract_pd_emul(__m128d const & src)
{
    int64_t r = _mm_extract_epi64(_mm_castpd_si128(src), N);
    return *(double *)&r;
}

template <int N>
FORCEINLINE __m128i _mm_insert_pd_emul(__m128d const & s1, double s2)
{
    return _mm_castsi128_pd(_mm_insert_epi64(_mm_castpd_si128(s1), *(int64_t *)s2, N));
}

// TICS -PRE#002
#define _mm_extract_pd(src, N)    _mm_extract_pd_emul<(N)>(src);
#define _mm_insert_pd(s1, s2, N)  _mm_insert_pd_emul<(N)>((s1), (s2));
// TICS +PRE#002

#if PLATFORM_INTEL_SSE < 35

template <int N>
FORCEINLINE __m128i _mm_alignr_epi8_emul(__m128i const & a, __m128i const & b)
{
    return _mm_or_si128(_mm_srli_si128(b, N), _mm_slli_si128(a, 16-N));
}

// TICS -PRE#002
#define _mm_alignr_epi8(a, b, n) _mm_alignr_epi8_emul<(n)>(a, b)
// TICS +PRE#002

FORCEINLINE __m128i _mm_mulhrs_epi16_emul(__m128i const & a, __m128i const & b)
{
    const __m128i minus_one = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());

    __m128i hi = _mm_mulhi_epi16(a, b);
    __m128i lo = _mm_mullo_epi16(a, b);

    __m128i carry = 
        _mm_srli_epi16(
        _mm_subs_epi16(
        _mm_srli_epi16(lo, 15-1), minus_one), 1);
    return  
        _mm_adds_epi16(
        _mm_slli_epi16(hi, 16-15), carry);
}

#define _mm_mulhrs_epi16 _mm_mulhrs_epi16_emul

#endif

FORCEINLINE void _mm_lutfullepi16_epi32(__m128i (& vecResult)[2], __m128i const & vecIndices, int32_t const * LUT)
{
#if PLATFORM_INTEL_SSE >= 40
    int32_t val = LUT[_mm_extract_epi16(vecIndices, 0)];
    vecResult[0] = _mm_cvtsi32_si128(val);
    val = LUT[_mm_extract_epi16(vecIndices, 1)];
    vecResult[0] = _mm_insert_epi32(vecResult[0], val, 1);
    val = LUT[_mm_extract_epi16(vecIndices, 2)];
    vecResult[0] = _mm_insert_epi32(vecResult[0], val, 2);
    val = LUT[_mm_extract_epi16(vecIndices, 3)];
    vecResult[0] = _mm_insert_epi32(vecResult[0], val, 3);

    val = LUT[_mm_extract_epi16(vecIndices, 4)];
    vecResult[1] = _mm_cvtsi32_si128(val);
    val = LUT[_mm_extract_epi16(vecIndices, 5)];
    vecResult[1] = _mm_insert_epi32(vecResult[1], val, 1);
    val = LUT[_mm_extract_epi16(vecIndices, 6)];
    vecResult[1] = _mm_insert_epi32(vecResult[1], val, 2);
    val = LUT[_mm_extract_epi16(vecIndices, 7)];
    vecResult[1] = _mm_insert_epi32(vecResult[1], val, 3);
#else /* PLATFORM_INTEL_SSE < 40 */
    // _mm_insert_epi32 is not a native instruction in < SSE4 so we prefer an alternative
    int32_t val = LUT[_mm_extract_epi16(vecIndices, 3)];
    vecResult[0] = _mm_cvtsi32_si128(val);
    val = LUT[_mm_extract_epi16(vecIndices, 2)];
    vecResult[0] = _mm_or_si128(_mm_cvtsi32_si128(val), _mm_slli_si128(vecResult[0], 4));
    val = LUT[_mm_extract_epi16(vecIndices, 1)];
    vecResult[0] = _mm_or_si128(_mm_cvtsi32_si128(val), _mm_slli_si128(vecResult[0], 4));
    val = LUT[_mm_extract_epi16(vecIndices, 0)];
    vecResult[0] = _mm_or_si128(_mm_cvtsi32_si128(val), _mm_slli_si128(vecResult[0], 4));

    val = LUT[_mm_extract_epi16(vecIndices, 7)];
    vecResult[1] = _mm_cvtsi32_si128(val);
    val = LUT[_mm_extract_epi16(vecIndices, 6)];
    vecResult[1] = _mm_or_si128(_mm_cvtsi32_si128(val), _mm_slli_si128(vecResult[1], 4));
    val = LUT[_mm_extract_epi16(vecIndices, 5)];
    vecResult[1] = _mm_or_si128(_mm_cvtsi32_si128(val), _mm_slli_si128(vecResult[1], 4));
    val = LUT[_mm_extract_epi16(vecIndices, 4)];
    vecResult[1] = _mm_or_si128(_mm_cvtsi32_si128(val), _mm_slli_si128(vecResult[1], 4));
#endif /* PLATFORM_INTEL_SSE >= 40 */
}

FORCEINLINE void _mm_lutfullepi16_ps(__m128 (& vecResult)[2], __m128i const & vecIndices, float const * LUT)
{
    _mm_lutfullepi16_epi32(
        reinterpret_cast<__m128i (&) [2]>(vecResult),
        vecIndices,
        reinterpret_cast<int32_t const *>(LUT));
}


FORCEINLINE __m128i _mm_lutfull_epi16(__m128i vecIndices, const int16_t * LUT)
{
    __m128i vecResult;
    uint16_t val, val2;
    uint32_t ulIndex01;

    ulIndex01 = _mm_cvtsi128_si32(vecIndices);
    val = LUT[(uint16_t)ulIndex01];
    val2 = LUT[(uint16_t)(ulIndex01>>16)];
    vecResult = _mm_cvtsi32_si128(val | (val2 << 16));

    vecIndices = _mm_srli_si128(vecIndices, 4);
    ulIndex01 = _mm_cvtsi128_si32(vecIndices);
    val = LUT[(uint16_t)ulIndex01];
    vecResult = _mm_insert_epi16(vecResult, val, 2);
    val = LUT[(uint16_t)(ulIndex01>>16)];
    vecResult = _mm_insert_epi16(vecResult, val, 3);

    vecIndices = _mm_srli_si128(vecIndices, 4);
    ulIndex01 = _mm_cvtsi128_si32(vecIndices);
    val = LUT[(uint16_t)ulIndex01];
    vecResult = _mm_insert_epi16(vecResult, val, 4);
    val = LUT[(uint16_t)(ulIndex01>>16)];
    vecResult = _mm_insert_epi16(vecResult, val, 5);

    vecIndices = _mm_srli_si128(vecIndices, 4);
    ulIndex01 = _mm_cvtsi128_si32(vecIndices);
    val = LUT[(uint16_t)ulIndex01];
    vecResult = _mm_insert_epi16(vecResult, val, 6);
    val = LUT[(uint16_t)(ulIndex01>>16)];
    vecResult = _mm_insert_epi16(vecResult, val, 7);

    return vecResult;
}

template <int POSITION>
FORCEINLINE __m128i _mm_lutitem_s_epi16(__m128i const & mResult, __m128i const & mIndices, const int16_t * LUT)
{
    int16_t val = LUT[(int16_t)_mm_extract_epi16(mIndices, POSITION)];
    return _mm_insert_epi16(_mm_lutitem_s_epi16<POSITION-1>(mResult, mIndices, LUT), val, POSITION);
}

template <>
FORCEINLINE __m128i _mm_lutitem_s_epi16<0>(__m128i const & mResult, __m128i const & mIndices, const int16_t * LUT)
{
    int16_t val = LUT[(int16_t)_mm_extract_epi16(mIndices, 0)];
    return _mm_insert_epi16(mResult, val, 0);
}

FORCEINLINE __m128i _mm_lutfull_s_epi16(__m128i const &  mIndices, const int16_t * LUT)
{
    __m128i mResult;
    return _mm_lutitem_s_epi16<7>(mResult, mIndices, LUT);
}


#if PLATFORM_INTEL_SSE >= 35

// [ (int16)Idx0 (int16)Idx1 ... ] => [ (int8)2*Idx0 (int8)2*Idx0+1 (int8)2*Idx1 (int8)2*Idx1+1 .. ]
FORCEINLINE __m128i _mm_lutidxtobyte_epi16(__m128i const & mIndices)
{
    const __m128i _mm_pack16unlacklo8_epi16 = _mm_set_epi8  (14, 14, 12, 12, 10, 10, 8, 8, 6, 6, 4, 4, 2, 2, 0, 0);
    const __m128i _mm_256_epi16             = _mm_set1_epi16(0x100);

    return _mm_adds_epi16(                           // [ (int8)2*Idx0 (int8)2*Idx0+1 (int8)2*Idx1 (int8)2*Idx1+1 .. ]
        _mm_shuffle_epi8(                    // [ (int8)2*Idx0 (int8)2*Idx0 (int8)2*Idx1 (int8)2*Idx1 .. ]
        _mm_slli_epi16(mIndices, 1),     // [ (int16)2*Idx0 (int16)2*Idx1 .. ]
        _mm_pack16unlacklo8_epi16),
        _mm_256_epi16);
}

// [ (int8)Idx0 (int8)Idx1 ... ] => [ (int8)(Idx0 > 15) ? 0xFF & Idx0 : Idx0 ... ]
FORCEINLINE __m128i _mm_lutsatidx_epi8(__m128i const & mIndices)
{
    const __m128i _mm_15_epi8               = _mm_set1_epi8 (0x0F);
    return _mm_or_si128(_mm_cmpgt_epi8(mIndices, _mm_15_epi8), mIndices);
}

FORCEINLINE __m128i _mm_lut8epi16_epi16(__m128i const & mIndices, __m128i const & mLUT)
{
    return
        _mm_shuffle_epi8(                            // [ (int16)LUT[Idx0] (int16)LUT[Idx1] .. ],
        mLUT,
        _mm_lutidxtobyte_epi16(mIndices)         // [ (int8)2*Idx0 (int8)2*Idx0+1 (int8)2*Idx1 (int8)2*Idx1+1 .. ],
        );
}

template <int I, int N>
FORCEINLINE __m128i _mm_lut8xNepi16_step_epi16(__m128i const & mByteIndices,
                                               __m128i const * mLUT)
{
    __m128i mThisLUT = _mm_shuffle_epi8(                        // [ (16 > Idx0 >= 0) ? LUT[Idx0] : 0, ... ]
        mLUT[I], _mm_lutsatidx_epi8(mByteIndices));
    __m128i const _mm_16_epi8               = _mm_set1_epi8 (0x10);

    if (I == N-1)
        return mThisLUT;
    return
        _mm_or_si128(                               // [ (16 > Idx0 >= 0) ? LUT[Idx0] : NextLUT(Idx0-16), ... ]
        mThisLUT,
        _mm_lut8xNepi16_step_epi16<__min(I+1, N-1), N>(_mm_subs_epi8(mByteIndices, _mm_16_epi8), mLUT));

}

template <int N>
FORCEINLINE __m128i _mm_lut8xNepi16_epi16(__m128i const & mIndices,
                                          __m128i const * mLUT)
{
    __m128i mByteIndices = _mm_lutidxtobyte_epi16(mIndices);
    return _mm_lut8xNepi16_step_epi16<0, N>(mByteIndices, mLUT);
}

FORCEINLINE __m128i _mm_lut16epi8_epi8(__m128i const & mIndices, __m128i const & mLUT)
{
    return _mm_shuffle_epi8(mLUT, mIndices);
}

template <int I, int N>
FORCEINLINE __m128i _mm_lut16xNepi8_step_epi8(__m128i const & mByteIndices,
                                              const __m128i (& mLUT)[N])
{
    __m128i const _mm_16_epi8               = _mm_set1_epi8 (0x10);
    __m128i mThisLUT = _mm_shuffle_epi8(                        // [ (16 > Idx0 >= 0) ? LUT[Idx0] : 0, ... ]
        mLUT[I], _mm_lutsatidx_epi8(mByteIndices));

    if (I == N-1)
        return mThisLUT;
    return
        _mm_or_si128(                               // [ (16 > Idx0 >= 0) ? LUT[Idx0] : NextLUT(Idx0-16), ... ]
        mThisLUT,
        _mm_lut16xNepi8_step_epi8<__min(I+1, N-1), N>(_mm_subs_epi8(mByteIndices, _mm_16_epi8), mLUT));

}

template <int N>
FORCEINLINE __m128i _mm_lut16xNepi8_epi8(__m128i const & mIndices,
                                         const __m128i (& mLUT)[N])
{
    return _mm_lut16xNepi8_step_epi8<0, N>(mIndices, mLUT);
}

#endif

// return (a + b + 1) / 2
FORCEINLINE __m128i _mm_avg_epi16(__m128i const & a, __m128i const & b)
{
#if PLATFORM_INTEL_SSE >= 35
    return _mm_mulhrs_epi16(_mm_adds_epi16(a, b), _mm_set1_epi16(16384));
#else
    const __m128i minus_one = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());
    return _mm_srai_epi16(_mm_adds_epi16(a, _mm_subs_epi16(b, minus_one)), 1);
#endif
}

#if PLATFORM_INTEL_SSE < 40

FORCEINLINE __m128i _mm_max_epi8_emul(__m128i const & a, __m128i const & b)
{
    __m128i mask = _mm_cmpgt_epi8(a, b);
    return _mm_or_si128(_mm_andnot_si128(mask, b), _mm_and_si128(mask, a));
}

FORCEINLINE __m128i _mm_max_epi32_emul(__m128i const & a, __m128i const & b)
{
    __m128i mask = _mm_cmpgt_epi32(a, b);
    return _mm_or_si128(_mm_andnot_si128(mask, b), _mm_and_si128(mask, a));
}

FORCEINLINE __m128i _mm_min_epi8_emul(__m128i const & a, __m128i const & b)
{
    __m128i mask = _mm_cmplt_epi8(a, b);
    return _mm_or_si128(_mm_andnot_si128(mask, b), _mm_and_si128(mask, a));
}

FORCEINLINE __m128i _mm_min_epi32_emul(__m128i const & a, __m128i const & b)
{
    __m128i mask = _mm_cmplt_epi32(a, b);
    return _mm_or_si128(_mm_andnot_si128(mask, b), _mm_and_si128(mask, a));
}

#define _mm_max_epi8  _mm_max_epi8_emul
#define _mm_max_epi32 _mm_max_epi32_emul
#define _mm_min_epi8  _mm_min_epi8_emul
#define _mm_min_epi32 _mm_min_epi32_emul

#endif /* PLATFORM_INTEL_SSE */

#if PLATFORM_INTEL_SSE < 35

FORCEINLINE __m128i _mm_abs_epi8_emul(__m128i const & a)
{
    return _mm_max_epi8(a, _mm_sub_epi8(_mm_setzero_si128(), a));
}

FORCEINLINE __m128i _mm_abs_epi16_emul(__m128i const & a)
{
    return _mm_max_epi16(a, _mm_sub_epi16(_mm_setzero_si128(), a));
}

FORCEINLINE __m128i _mm_abs_epi32_emul(__m128i const & a)
{
    return _mm_max_epi32(a, _mm_sub_epi32(_mm_setzero_si128(), a));
}

#define _mm_abs_epi8  _mm_abs_epi8_emul
#define _mm_abs_epi16 _mm_abs_epi16_emul
#define _mm_abs_epi32 _mm_abs_epi32_emul

#endif /* PLATFORM_INTEL_SSE < 35 */


FORCEINLINE __m128i _mm_mullo_epu32(__m128i const & a, __m128i const & b)
{
    __m128 tmp1 = _mm_castsi128_ps(_mm_mul_epu32(a,b)); /* mul 2,0*/
    __m128 tmp2 = _mm_castsi128_ps(_mm_mul_epu32(_mm_srli_si128(a,4), _mm_srli_si128(b,4))); /* mul 3,1 */
    return _mm_shuffle_epi32(_mm_castps_si128(_mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE (2,0,2,0))), _MM_SHUFFLE(3,1,2,0)); /* shuffle results to [63..0] and pack */
}

FORCEINLINE __m128i _mm_mulhi_epu32(__m128i const & a, __m128i const & b)
{
    __m128 tmp1 = _mm_castsi128_ps(_mm_mul_epu32(a,b)); /* mul 2,0*/
    __m128 tmp2 = _mm_castsi128_ps(_mm_mul_epu32(_mm_srli_si128(a,4), _mm_srli_si128(b,4))); /* mul 3,1 */
    return _mm_shuffle_epi32(_mm_castps_si128(_mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE (3,1,3,1))), _MM_SHUFFLE(3,1,2,0)); /* shuffle results to [63..0] and pack */
}


#if PLATFORM_INTEL_SSE < 40

FORCEINLINE __m128i _mm_mul_epi32_emul(__m128i const & a, __m128i const & b)
{
    __m128i const zero = _mm_setzero_si128();

    __m128i absa = _mm_abs_epi32(a);
    __m128i absb = _mm_abs_epi32(b);
    __m128i absign = _mm_xor_si128(a, b);

    __m128i prod = _mm_mul_epu32(absa, absb);
    __m128i neg_prod = _mm_sub_epi64(zero, prod);

    // the mask need to be extended to 64 bits
    __m128i neg_mask = _mm_shuffle_epi32(_mm_cmplt_epi32(absign, zero), _MM_SHUFFLE(2, 2, 0, 0));

    return _mm_or_si128(_mm_andnot_si128(neg_mask, prod), _mm_and_si128(neg_mask, neg_prod));
}

FORCEINLINE __m128i _mm_mullo_epi32_emul(__m128i const & a, __m128i const & b)
{
    __m128 tmp1 = _mm_castsi128_ps(_mm_mul_epu32(a,b)); /* mul 2,0*/
    __m128 tmp2 = _mm_castsi128_ps(_mm_mul_epu32(_mm_srli_si128(a,4), _mm_srli_si128(b,4))); /* mul 3,1 */
    __m128i res = _mm_castps_si128(_mm_shuffle_ps(tmp1, tmp2, _MM_SHUFFLE (2,0,2,0))); /* shuffle results to [63..0] and pack */
    return _mm_shuffle_epi32(res, _MM_SHUFFLE(3,1,2,0));
}

#define _mm_mul_epi32   _mm_mul_epi32_emul
#define _mm_mullo_epi32 _mm_mullo_epi32_emul

#endif /* PLATFORM_INTEL_SSE < 40 */

FORCEINLINE __m128i _mm_mulhi_epi32(__m128i const & a, __m128i const & b)
{
    __m128i c[2];
    __m128i const mask02 = _mm_set_epi32(0, -1, 0, -1);

    c[0] = _mm_mul_epi32(a, _mm_and_si128(mask02, b));
    c[1] = _mm_mul_epi32(_mm_srli_si128(a, 4), _mm_and_si128(mask02, b));

    c[0] = _mm_srli_si128(c[0], 4);

    return _mm_or_si128(_mm_and_si128(mask02, c[0]), _mm_andnot_si128(mask02, c[1]));
}

#if PLATFORM_INTEL_SSE < 30

#define _mm_lddqu_si128     _mm_loadu_si128

#endif

FORCEINLINE void _mm_storeh_epi64(__m128i * ptr, __m128i const & a)
{
    _mm_storel_epi64(ptr, _mm_srli_si128(a, 8));
}

FORCEINLINE __m128i _mm_set1_epi64_int64(int64_t a)
{
    __m128i res = _mm_insert_epi64(_mm_setzero_si128(), a, 0);
    return _mm_insert_epi64(res, a, 1);
}

FORCEINLINE __m128i _mm_set_epi64_int64(int64_t a1, int64_t a0)
{
    __m128i res = _mm_insert_epi64(_mm_setzero_si128(), a0, 0);
    return _mm_insert_epi64(res, a1, 1);
}

// The following are automatically generated in Intel's but not in Microsoft's compiler.
FORCEINLINE __m128i _mm_slli_epi8_emul(__m128i const & a, int n)
{
    return _mm_and_si128(_mm_set1_epi8(-2 /* 0xfe */), _mm_slli_epi16(a, n));
}

FORCEINLINE __m128i _mm_srli_epi8_emul(__m128i const & a, int n)
{
    return _mm_and_si128(_mm_set1_epi8(127 /* 0x7f */), _mm_srli_epi16(a, n));
}

FORCEINLINE __m128i _mm_srai_epi8_emul(__m128i const & a, int n)
{
    return _mm_or_si128(_mm_and_si128(a, _mm_set1_epi8(-128 /* 0x80 */)),
        _mm_and_si128(_mm_set1_epi8(127 /* 0x7f */), _mm_srli_epi16(a, n)));
}

#define _mm_slli_epi8 _mm_slli_epi8_emul
#define _mm_srli_epi8 _mm_srli_epi8_emul
#define _mm_srai_epi8 _mm_srai_epi8_emul

FORCEINLINE void sseemul_horz_prolog_epi16(__m128i & even, __m128i & odd, __m128i const & a, __m128i const & b)
{
    __m128i c   = _mm_srai_epi32 ( a, 16 );
    __m128i d   = _mm_srai_epi32 ( b, 16 );
    odd = _mm_packs_epi32( c, d );

    c  = _mm_srai_epi32(_mm_slli_si128 ( a, 2 ), 16);
    d  = _mm_srai_epi32(_mm_slli_si128 ( b, 2 ), 16);
    even = _mm_packs_epi32(c, d);
}

#if PLATFORM_INTEL_SSE < 30

FORCEINLINE __m128i _mm_hadd_epi16_emul(__m128i const & a, __m128i const & b)
{
    __m128i even, odd; sseemul_horz_prolog_epi16(even, odd, a, b);
    return _mm_add_epi16(odd, even);
}

#define _mm_hadd_epi16 _mm_hadd_epi16_emul

FORCEINLINE __m128i _mm_hsub_epi16_emul(__m128i const & a, __m128i const & b)
{
    __m128i even, odd; sseemul_horz_prolog_epi16(even, odd, a, b);
    return _mm_sub_epi16(odd, even);
}

#define _mm_hsub_epi16 _mm_hsub_epi16_emul

#endif /* PLATFORM_INTEL_SSE < 30 */

#if PLATFORM_INTEL_SSE < 35

FORCEINLINE __m128i _mm_hadds_epi16_emul(__m128i const & a, __m128i const & b)
{
    __m128i even, odd; sseemul_horz_prolog_epi16(even, odd, a, b);
    return _mm_adds_epi16(odd, even);
}

#define _mm_hadds_epi16 _mm_hadds_epi16_emul

FORCEINLINE __m128i _mm_hsubs_epi16_emul(__m128i const & a, __m128i const & b)
{
    __m128i even, odd; sseemul_horz_prolog_epi16(even, odd, a, b);
    return _mm_subs_epi16(odd, even);
}

#define _mm_hsubs_epi16 _mm_hsubs_epi16_emul

#endif /* PLATFORM_INTEL_SSE < 35 */

#if PLATFORM_INTEL_SSE < 40


template <int mask>
FORCEINLINE __m128 _mm_dp_ps_emul(__m128 const & a, __m128 const & b)
{
    int const highMaskAll   = 128|64|32|16;
    int const highMaskHigh  = 128|64;
    int const highMaskLow   = 32|16;
    int const highMaskEven  = 64|16;
    int const highMaskRed   = 128|16;
    int const highMaskBlack = 64|32;
    int const highMaskOdd   = 128|32;
    int const lowMaskAll    = 8|4|2|1;

    if ((mask & lowMaskAll) == 0 || (mask & highMaskAll) == 0)
        return _mm_setzero_ps();

    __m128 result = _mm_mul_ps(a, b);

    if (((mask & highMaskRed) == highMaskRed) || ((mask & highMaskBlack) == highMaskBlack))
    {
        // all "black" and all "red" case => need to be masked
        __m128 high_mask = _mm_castsi128_ps(_mm_set_epi32((mask & 128) ? 0xffffffff : 0x0, 
                                                          (mask &  64) ? 0xffffffff : 0x0,
                                                          (mask &  32) ? 0xffffffff : 0x0,
                                                          (mask &  16) ? 0xffffffff : 0x0));
        result = _mm_and_ps(result, high_mask);
    }

    if ((mask & highMaskEven) != 0)
    {
        if ((mask & highMaskOdd) != 0)
        {
            // Odd and even
            result = _mm_add_ps(result, _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1)));
        }

        if ((mask & highMaskLow) != 0)
        {
            if ((mask & highMaskHigh) != 0)
            {
                // High and low input part
                result = _mm_add_ps(result, _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(result), 8)));
            }
            result = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(result), _MM_SHUFFLE(0, 0, 0, 0)));
        }
        else
        {
            // High input part only
            result = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(result), _MM_SHUFFLE(2, 2, 2, 2)));
        }
    }
    else
    {
        // Odd only

        if ((mask & highMaskLow) != 0)
        {
            if ((mask & highMaskHigh) != 0)
            {
                // High and low input part
                result = _mm_add_ps(result, _mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(result), 8)));
            }
            result = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(result), _MM_SHUFFLE(1, 1 ,1, 1)));
        }
        else
        {
            // High input part only
            result = _mm_castsi128_ps(_mm_shuffle_epi32(_mm_castps_si128(result), _MM_SHUFFLE(3, 3, 3, 3)));
        }
    }

    if ((mask & lowMaskAll) != lowMaskAll)
    {
        __m128 low_mask  = _mm_castsi128_ps(_mm_set_epi32((mask &   8) ? 0xffffffff : 0x0,
                                                          (mask &   4) ? 0xffffffff : 0x0,
                                                          (mask &   2) ? 0xffffffff : 0x0,
                                                          (mask &   1) ? 0xffffffff : 0x0));
        result = _mm_and_ps(result, low_mask);
    }

    return result;
}

#define _mm_dp_ps(a, b, mask) _mm_dp_ps_emul<(mask)>((a), (b))

#endif /* PLATFORM_INTEL_SSE < 40 */

#pragma warning(default:4512)
