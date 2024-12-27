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

        PXVEC_IMPLEMENT_NUMLIMIT_CONSTANTS()

        PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_pd)
        PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_pd)
        PXVEC_IMPLEMENT_BINARYOP(*, _mm_mul_pd)
        PXVEC_IMPLEMENT_BINARYOP(/ , _mm_div_pd)
        PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_pd)
        PXVEC_IMPLEMENT_BINARYOP(| , _mm_or_pd)
        PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_pd)
        PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_pd)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_pd)
        PXVEC_IMPLEMENT_COMPARE(!= , cmpneq, _mm_cmpneq_pd)
        PXVEC_IMPLEMENT_COMPARE(<, cmplt, _mm_cmplt_pd)
        PXVEC_IMPLEMENT_COMPARE(<= , cmple, _mm_cmple_pd)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm_cmpgt_pd)
        PXVEC_IMPLEMENT_COMPARE(>= , cmpge, _mm_cmpge_pd)

        PXVEC_IMPLEMENT_UNARYFUN(sqrt, _mm_sqrt_pd)
        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm_abs_pd)

        friend FORCEINLINE double sum(F64vec2 const & a) { return _mm_sum_pd(a); }

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_pd)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_pd)

#if PLATFORM_INTEL_SSE >= 30
        PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm_hadd_pd)
        PXVEC_IMPLEMENT_BINARYFUN(horz_sub, _mm_hsub_pd)
#endif
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

        PXVEC_IMPLEMENT_NUMLIMIT_CONSTANTS()

        PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_ps)
        PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_ps)
        PXVEC_IMPLEMENT_BINARYOP(*, _mm_mul_ps)
        PXVEC_IMPLEMENT_BINARYOP(/ , _mm_div_ps)
        PXVEC_IMPLEMENT_BINARYOP(&, _mm_and_ps)
        PXVEC_IMPLEMENT_BINARYOP(| , _mm_or_ps)
        PXVEC_IMPLEMENT_BINARYOP(^, _mm_xor_ps)
        PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm_andnot_ps)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_ps)
        PXVEC_IMPLEMENT_COMPARE(!= , cmpneq, _mm_cmpneq_ps)
        PXVEC_IMPLEMENT_COMPARE(<, cmplt, _mm_cmplt_ps)
        PXVEC_IMPLEMENT_COMPARE(<= , cmple, _mm_cmple_ps)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm_cmpgt_ps)
        PXVEC_IMPLEMENT_COMPARE(>= , cmpge, _mm_cmpge_ps)

#if PLATFORM_INTEL_SSE >= 40
        PXVEC_IMPLEMENT_UNARYFUN_CST(floor, _mm_round_ps, _MM_FROUND_FLOOR)
        PXVEC_IMPLEMENT_UNARYFUN_CST(ceil, _mm_round_ps, _MM_FROUND_CEIL)
#endif
        PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm_rsqrt_ps)
        PXVEC_IMPLEMENT_UNARYFUN(sqrt, _mm_sqrt_ps)
        PXVEC_IMPLEMENT_UNARYFUN(rcp, _mm_rcp_ps)
        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm_abs_ps)
        PXVEC_IMPLEMENT_UNARYFUN(signbit, _mm_signbit_ps)

        friend FORCEINLINE float sum(F32vec4 const & a) { return _mm_sum_ps(a); }
    friend FORCEINLINE F64vec2 sum02_13(F32vec4 const & a);

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_ps)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_ps)

#if PLATFORM_INTEL_SSE >= 30
        PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm_hadd_ps)
        PXVEC_IMPLEMENT_BINARYFUN(horz_sub, _mm_hsub_ps)
#endif
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
        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi64)
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
        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi64)
#if PLATFORM_INTEL_SSE >= 42
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm_cmpgt_epi64)
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

        PXVEC_IMPLEMENT_BINARYFUN(mul_low, _mm_mullo_epu32)
        PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epu32)

        PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi32, _mm_srli_epi32, _mm_srli_epi32)
        PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi32, _mm_srl_epi32, _mm_srl_epi32)

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi32)

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

        PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm_hadd_epi32)

        PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi32)
        PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi32)
        PXVEC_IMPLEMENT_BINARYOP(*, _mm_mullo_epi32)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_BINARYFUN(mul_low, _mm_mullo_epi32)
        PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epi32)

        PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
        PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi32)
        PXVEC_IMPLEMENT_COMPARE(<, cmplt, _mm_cmplt_epi32)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm_cmpgt_epi32)
        PXVEC_COMPARE_EMULATE_ONLYSTRICT()

        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm_abs_epi32)

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

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi16)

        PXVEC_IMPLEMENT_BINARYFUN(mul_low, _mm_mullo_epi16)
        PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epu16)

        PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi16, _mm_srli_epi16, _mm_srli_epi16)
        PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi16, _mm_srl_epi16, _mm_srl_epi16)

#if PLATFORM_INTEL_SSE >= 40
        PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epu16)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epu16)
#endif
};

struct Is16vec8
{
    typedef Is16vec4 HALF_TYPE;
    typedef Is16vec8 SELF_TYPE;
    typedef Is16vec16 DOUBLE_TYPE;
    typedef int16_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()

        FORCEINLINE HALF_TYPE half(int i) const;

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
        PXVEC_IMPLEMENT_BINARYFUN(mul_low, _mm_mullo_epi16)
        PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm_mulhi_epi16)

        friend FORCEINLINE Is32vec4 mul_add(SELF_TYPE const & a, SELF_TYPE const & b) { return Is32vec4(_mm_madd_epi16(a, b)); };

    PXVEC_IMPLEMENT_SHIFTI(_mm_slli_epi16, _mm_srai_epi16, _mm_srli_epi16)
        PXVEC_IMPLEMENT_SHIFT(_mm_sll_epi16, _mm_sra_epi16, _mm_srl_epi16)

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi16)
        PXVEC_IMPLEMENT_COMPARE(<, cmplt, _mm_cmplt_epi16)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm_cmpgt_epi16)
        PXVEC_COMPARE_EMULATE_ONLYSTRICT()

        friend FORCEINLINE SELF_TYPE operator != (SELF_TYPE const & a, SELF_TYPE const & b)
    {
        return ((a == b) == zero());
    }

    PXVEC_IMPLEMENT_UNARYFUN(abs, _mm_abs_epi16)

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

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi8)

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

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm_cmpeq_epi8)
        PXVEC_IMPLEMENT_COMPARE(<, cmplt, _mm_cmplt_epi8)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm_cmpgt_epi8)
        PXVEC_COMPARE_EMULATE_ONLYSTRICT()

        PXVEC_IMPLEMENT_BINARYOP(+, _mm_add_epi8)
        PXVEC_IMPLEMENT_BINARYOP(-, _mm_sub_epi8)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm_adds_epi8)
        PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm_subs_epi8)
        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm_abs_epi8)

        PXVEC_IMPLEMENT_BINARYFUN(min, _mm_min_epi8)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm_max_epi8)
};

#if PLATFORM_INTEL_SSE >= 60

struct Iu64vec4
{
    typedef Iu64vec4 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi64)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm256_cmpgt_epi64)
        PXVEC_COMPARE_EMULATE_ONLYGT()

        friend FORCEINLINE int64_t sum(Is64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Iu32vec8
{
    typedef Iu32vec8 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi32)

        friend FORCEINLINE uint32_t sum(Iu32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is32vec8
{
    typedef Is32vec8 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_BINARYFUN(mul_low, _mm256_mullo_epi32)
        //PXVEC_IMPLEMENT_BINARYFUN(mul_high , _mm256_mulhi_epi32)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi32)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm256_cmpgt_epi32)
        PXVEC_COMPARE_EMULATE_ONLYGT()

        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm256_abs_epi32)

        PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_epi32)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_epi32)

        PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi32, _mm256_srai_epi32, _mm256_srli_epi32)
        PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi32, _mm256_sra_epi32, _mm256_srl_epi32)

        friend FORCEINLINE int32_t sum(Is32vec8 const & a) { return _mm_sum_epi32(a.half(0)) + _mm_sum_epi32(a.half(1)); }
};

struct Iu16vec16
{
    typedef Iu16vec16 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi16)

        PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi16, _mm256_srli_epi16, _mm256_srli_epi16)
        PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi16, _mm256_srl_epi16, _mm256_srl_epi16)
};

struct Is16vec16
{
    typedef Is16vec16 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_BINARYFUN(mul_low, _mm256_mullo_epi16)
        PXVEC_IMPLEMENT_BINARYFUN(mul_high, _mm256_mulhi_epi16)

        PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi16, _mm256_srai_epi16, _mm256_srli_epi16)
        PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi16, _mm256_sra_epi16, _mm256_srl_epi16)

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi16)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm256_cmpgt_epi16)
        PXVEC_COMPARE_EMULATE_ONLYGT()

        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm256_abs_epi16)

        PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_epi16)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_epi16)
        PXVEC_IMPLEMENT_BINARYFUN(avg, _mm256_avg_epi16)
};

struct Iu8vec32
{
    typedef Iu8vec32 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu8vec16 HALF_TYPE;
    typedef uint8_t  SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
        PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Iu8vec32, _mm256_set1_epi8)
        PXVEC_IMPLEMENT_CONSTRUCTOR_S32(Iu8vec32, _mm256_set_epi8)

        PXVEC_IMPLEMENT_BASIC_OP_M256I()

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi8)

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
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
    typedef Is8vec16 HALF_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m256i NATIVE_VEC_TYPE;

    PXVEC_IMPLEMENT_DATA()
        PXVEC_IMPLEMENT_CONSTRUCTORS_M256I(Is8vec32, _mm256_set1_epi8)
        PXVEC_IMPLEMENT_CONSTRUCTOR_S32(Is8vec32, _mm256_set_epi8)

        PXVEC_IMPLEMENT_BASIC_OP_M256I()

        PXVEC_IMPLEMENT_COMPARE(== , cmpeq, _mm256_cmpeq_epi8)
        PXVEC_IMPLEMENT_COMPARE(>, cmpgt, _mm256_cmpgt_epi8)
        PXVEC_COMPARE_EMULATE_ONLYGT()

        PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_epi8)
        PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_epi8)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_BINARYFUN(sat_add, _mm256_adds_epi8)
        PXVEC_IMPLEMENT_BINARYFUN(sat_sub, _mm256_subs_epi8)
        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm256_abs_epi8)

        PXVEC_IMPLEMENT_SHIFTI(_mm256_slli_epi32, _mm256_srai_epi32, _mm256_srli_epi32)
        PXVEC_IMPLEMENT_SHIFT_M256(_mm256_sll_epi32, _mm256_sra_epi32, _mm256_srl_epi32)
};

#else

struct Iu64vec4
{
    typedef Iu64vec4 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu64vec2 HALF_TYPE;
    typedef uint64_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()

        PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu64vec4, _mm_set1_epi64_int64)
        PXVEC_IMPLEMENT_CONSTRUCTOR_2xS2(Iu64vec4, _mm_set_epi64_int64)

        PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

#if PLATFORM_INTEL_SSE >= 40
        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi64)
#endif

        PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi64)
        PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi64)

        PXVEC_IMPLEMENT_UNARYMINUS()

        friend FORCEINLINE int64_t sum(Iu64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is64vec4
{
    typedef Is64vec4 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi64)
#if PLATFORM_INTEL_SSE >= 42
        PXVEC_IMPLEMENT_2COMPARE(>, cmpgt, _mm_cmpgt_epi64)
        PXVEC_COMPARE_EMULATE_ONLYGT()
#endif
#endif

        PXVEC_IMPLEMENT_UNARYMINUS()

        friend FORCEINLINE int64_t sum(Is64vec4 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Iu32vec8
{
    typedef Iu32vec8 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_2SHIFT(_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi32)

        PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epu32)
        PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epu32)

        friend FORCEINLINE uint32_t sum(Iu32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
};

struct Is32vec8
{
    typedef Is32vec8 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_2BINARYFUN(horz_add, _mm_hadd_epi32)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_2BINARYFUN(mul_low, _mm_mullo_epi32)
        PXVEC_IMPLEMENT_2BINARYFUN(mul_high, _mm_mulhi_epi32)

        PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi32, _mm_srai_epi32, _mm_srli_epi32)
        PXVEC_IMPLEMENT_2SHIFT(_mm_sll_epi32, _mm_sra_epi32, _mm_srl_epi32)

        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi32)
        PXVEC_IMPLEMENT_2COMPARE(<, cmplt, _mm_cmplt_epi32)
        PXVEC_IMPLEMENT_2COMPARE(>, cmpgt, _mm_cmpgt_epi32)
        PXVEC_COMPARE_EMULATE_ONLYSTRICT()

        PXVEC_IMPLEMENT_2UNARYFUN(abs, _mm_abs_epi32)

        PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epi32)
        PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epi32)

        friend FORCEINLINE Is32vec4 horz_add(SELF_TYPE const & a) { return horz_add(a.half(0), a.half(1)); }

    friend FORCEINLINE int32_t sum(Is32vec8 const & a) { return sum(a.half(0)) + sum(a.half(1)); }
    friend Is16vec8 sat_pack(SELF_TYPE const & a);
};

struct Iu16vec16
{
    typedef Iu16vec16 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu16vec8 HALF_TYPE;
    typedef uint16_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
        PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu16vec16, _mm_set1_epi16)
        PXVEC_IMPLEMENT_CONSTRUCTOR_2xS8(Iu16vec16, _mm_set_epi16)

        PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi16)

        PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epu16)
        PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epu16)

        PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi16)
        PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi16)

#if PLATFORM_INTEL_SSE >= 40
        PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epu16)
        PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epu16)
#endif

        PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi16, _mm_srli_epi16, _mm_srli_epi16)
        PXVEC_IMPLEMENT_2SHIFT(_mm_sll_epi16, _mm_srl_epi16, _mm_srl_epi16)
};

struct Is16vec16
{
    typedef Is16vec16 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_2BINARYFUN(mul_low, _mm_mullo_epi16)
        PXVEC_IMPLEMENT_2BINARYFUN(mul_high, _mm_mulhi_epi16)

        PXVEC_IMPLEMENT_2SHIFTI(_mm_slli_epi16, _mm_srai_epi16, _mm_srli_epi16)
        PXVEC_IMPLEMENT_2SHIFT(_mm_sll_epi16, _mm_sra_epi16, _mm_srl_epi16)

        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi16)
        PXVEC_IMPLEMENT_2COMPARE(<, cmplt, _mm_cmplt_epi16)
        PXVEC_IMPLEMENT_2COMPARE(>, cmpgt, _mm_cmpgt_epi16)
        PXVEC_COMPARE_EMULATE_ONLYSTRICT()

        PXVEC_IMPLEMENT_2UNARYFUN(abs, _mm_abs_epi16)

        PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_epi16)
        PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_epi16)
        PXVEC_IMPLEMENT_2BINARYFUN(avg, _mm_avg_epi16)
};

struct Iu8vec32
{
    typedef Iu8vec32 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
    typedef Iu8vec16 HALF_TYPE;
    typedef uint8_t SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
        PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Iu8vec32, _mm_set1_epi8)
        PXVEC_IMPLEMENT_CONSTRUCTOR_2xS16(Iu8vec32, _mm_set_epi8)

        PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi8)

        PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi8)
        PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi8)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epu8)
        PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epu8)
};

struct Is8vec32
{
    typedef Is8vec32 SELF_TYPE;
    typedef Vec2x<SELF_TYPE> DOUBLE_TYPE;
    typedef Is8vec16 HALF_TYPE;
    typedef int8_t  SCALAR_TYPE;
    typedef __m128i NATIVE_VEC_TYPE[2];

    PXVEC_IMPLEMENT_DATA()
        PXVEC_IMPLEMENT_CONSTRUCTORS_2xM128I(Is8vec32, _mm_set1_epi8)
        PXVEC_IMPLEMENT_CONSTRUCTOR_2xS16(Is8vec32, _mm_set_epi8)

        PXVEC_IMPLEMENT_BASIC_OP_2xM128I()

        PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_epi8)
        PXVEC_IMPLEMENT_2COMPARE(<, cmplt, _mm_cmplt_epi8)
        PXVEC_IMPLEMENT_2COMPARE(>, cmpgt, _mm_cmpgt_epi8)
        PXVEC_COMPARE_EMULATE_ONLYSTRICT()

        PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_epi8)
        PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_epi8)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_2BINARYFUN(sat_add, _mm_adds_epi8)
        PXVEC_IMPLEMENT_2BINARYFUN(sat_sub, _mm_subs_epi8)
        PXVEC_IMPLEMENT_2UNARYFUN(abs, _mm_abs_epi8)
};

#endif

#if PLATFORM_INTEL_SSE >= 50

struct F64vec4
{
    typedef F64vec8 DOUBLE_TYPE;
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
        PXVEC_IMPLEMENT_BINARYOP(/ , _mm256_div_pd)
        PXVEC_IMPLEMENT_BINARYOP(&, _mm256_and_pd)
        PXVEC_IMPLEMENT_BINARYOP(| , _mm256_or_pd)
        PXVEC_IMPLEMENT_BINARYOP(^, _mm256_xor_pd)
        PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm256_andnot_pd)

        PXVEC_IMPLEMENT_UNARYMINUS()

        PXVEC_IMPLEMENT_COMPARE_OPCODE(== , cmpeq, _mm256_cmp_pd, _CMP_EQ_OQ)
        PXVEC_IMPLEMENT_COMPARE_OPCODE(!= , cmpneq, _mm256_cmp_pd, _CMP_NEQ_UQ)
        PXVEC_IMPLEMENT_COMPARE_OPCODE(<, cmplt, _mm256_cmp_pd, _CMP_LT_OQ)
        PXVEC_IMPLEMENT_COMPARE_OPCODE(<= , cmple, _mm256_cmp_pd, _CMP_LE_OS)
        PXVEC_IMPLEMENT_COMPARE_OPCODE(>, cmpgt, _mm256_cmp_pd, _CMP_NLE_US)
        PXVEC_IMPLEMENT_COMPARE_OPCODE(>= , cmpge, _mm256_cmp_pd, _CMP_NLT_US)

        //PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm256_rsqrt_pd)
        PXVEC_IMPLEMENT_UNARYFUN(sqrt, _mm256_sqrt_pd)
        //PXVEC_IMPLEMENT_UNARYFUN(rcp  , _mm256_rcp_pd  )
        PXVEC_IMPLEMENT_UNARYFUN(abs, _mm256_abs_pd)
        //PXVEC_IMPLEMENT_UNARYFUN(signbit, _mm256_signbit_pd)

        PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_pd)
        PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_pd)

        //friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

struct F32vec8
{
    typedef Vec2x<F32vec8> DOUBLE_TYPE;
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

    PXVEC_IMPLEMENT_NUMLIMIT_CONSTANTS()

    PXVEC_IMPLEMENT_BINARYFUN(horz_add, _mm256_hadd_ps)
    PXVEC_IMPLEMENT_BINARYFUN(horz_sub, _mm256_hsub_ps)

    PXVEC_IMPLEMENT_BINARYOP(+, _mm256_add_ps)
    PXVEC_IMPLEMENT_BINARYOP(-, _mm256_sub_ps)
    PXVEC_IMPLEMENT_BINARYOP(*, _mm256_mul_ps)
    PXVEC_IMPLEMENT_BINARYOP(/ , _mm256_div_ps)
    PXVEC_IMPLEMENT_BINARYOP(&, _mm256_and_ps)
    PXVEC_IMPLEMENT_BINARYOP(| , _mm256_or_ps)
    PXVEC_IMPLEMENT_BINARYOP(^, _mm256_xor_ps)
    PXVEC_IMPLEMENT_BINARYFUN(andnot, _mm256_andnot_ps)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_COMPARE_OPCODE(== , cmpeq, _mm256_cmp_ps, _CMP_EQ_OQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(!= , cmpneq, _mm256_cmp_ps, _CMP_NEQ_UQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(<, cmplt, _mm256_cmp_ps, _CMP_LT_OQ)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(<= , cmple, _mm256_cmp_ps, _CMP_LE_OS)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(>, cmpgt, _mm256_cmp_ps, _CMP_NLE_US)
    PXVEC_IMPLEMENT_COMPARE_OPCODE(>= , cmpge, _mm256_cmp_ps, _CMP_NLT_US)

    PXVEC_IMPLEMENT_UNARYFUN(ceil, _mm256_ceil_ps)
    PXVEC_IMPLEMENT_UNARYFUN(floor, _mm256_floor_ps)
    PXVEC_IMPLEMENT_UNARYFUN(rsqrt, _mm256_rsqrt_ps)
    PXVEC_IMPLEMENT_UNARYFUN(sqrt, _mm256_sqrt_ps)
    PXVEC_IMPLEMENT_UNARYFUN(rcp, _mm256_rcp_ps)
    PXVEC_IMPLEMENT_UNARYFUN(abs, _mm256_abs_ps)
    PXVEC_IMPLEMENT_UNARYFUN(signbit, _mm256_signbit_ps)

    PXVEC_IMPLEMENT_BINARYFUN(min, _mm256_min_ps)
    PXVEC_IMPLEMENT_BINARYFUN(max, _mm256_max_ps)

    friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

#else

struct F32vec8
{
    typedef Vec2x<F32vec8> DOUBLE_TYPE;
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

    PXVEC_IMPLEMENT_NUMLIMIT_CONSTANTS()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_ps)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_ps)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mul_ps)
    PXVEC_IMPLEMENT_2BINARYOP(/ , _mm_div_ps)
    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_ps)
    PXVEC_IMPLEMENT_2BINARYOP(| , _mm_or_ps)
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_ps)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_ps)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_ps)
    PXVEC_IMPLEMENT_2COMPARE(!= , cmpneq, _mm_cmpneq_ps)
    PXVEC_IMPLEMENT_2COMPARE(<, cmplt, _mm_cmplt_ps)
    PXVEC_IMPLEMENT_2COMPARE(<= , cmple, _mm_cmple_ps)
    PXVEC_IMPLEMENT_2COMPARE(>, cmpgt, _mm_cmpgt_ps)
    PXVEC_IMPLEMENT_2COMPARE(>= , cmpge, _mm_cmpge_ps)

    PXVEC_IMPLEMENT_2UNARYFUN_CST(floor, _mm_round_ps, _MM_FROUND_FLOOR)
    PXVEC_IMPLEMENT_2UNARYFUN_CST(ceil, _mm_round_ps, _MM_FROUND_CEIL)

    PXVEC_IMPLEMENT_2UNARYFUN(rsqrt, _mm_rsqrt_ps)
    PXVEC_IMPLEMENT_2UNARYFUN(sqrt, _mm_sqrt_ps)
    PXVEC_IMPLEMENT_2UNARYFUN(rcp, _mm_rcp_ps)
    PXVEC_IMPLEMENT_2UNARYFUN(abs, _mm_abs_ps)
    PXVEC_IMPLEMENT_2UNARYFUN(signbit, _mm_signbit_ps)

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_ps)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_ps)

#if PLATFORM_INTEL_SSE >= 30
    PXVEC_IMPLEMENT_2BINARYFUN(horz_add, _mm_hadd_ps)
    PXVEC_IMPLEMENT_2BINARYFUN(horz_sub, _mm_hsub_ps)
#endif

    friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

struct F64vec4
{
    typedef F64vec8 DOUBLE_TYPE;
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

    PXVEC_IMPLEMENT_NUMLIMIT_CONSTANTS()

    PXVEC_IMPLEMENT_2BINARYOP(+, _mm_add_pd)
    PXVEC_IMPLEMENT_2BINARYOP(-, _mm_sub_pd)
    PXVEC_IMPLEMENT_2BINARYOP(*, _mm_mul_pd)
    PXVEC_IMPLEMENT_2BINARYOP(/ , _mm_div_pd)
    PXVEC_IMPLEMENT_2BINARYOP(&, _mm_and_pd)
    PXVEC_IMPLEMENT_2BINARYOP(| , _mm_or_pd)
    PXVEC_IMPLEMENT_2BINARYOP(^, _mm_xor_pd)
    PXVEC_IMPLEMENT_2BINARYFUN(andnot, _mm_andnot_pd)

    PXVEC_IMPLEMENT_UNARYMINUS()

    PXVEC_IMPLEMENT_2COMPARE(== , cmpeq, _mm_cmpeq_pd)
    PXVEC_IMPLEMENT_2COMPARE(!= , cmpneq, _mm_cmpneq_pd)
    PXVEC_IMPLEMENT_2COMPARE(<, cmplt, _mm_cmplt_pd)
    PXVEC_IMPLEMENT_2COMPARE(<= , cmple, _mm_cmple_pd)
    PXVEC_IMPLEMENT_2COMPARE(>, cmpgt, _mm_cmpgt_pd)
    PXVEC_IMPLEMENT_2COMPARE(>= , cmpge, _mm_cmpge_pd)

    //PXVEC_IMPLEMENT_2UNARYFUN(rsqrt, _mm_rsqrt_pd)
    PXVEC_IMPLEMENT_2UNARYFUN(sqrt, _mm_sqrt_pd)
    //PXVEC_IMPLEMENT_2UNARYFUN(rcp  , _mm_rcp_pd  )
    PXVEC_IMPLEMENT_2UNARYFUN(abs, _mm_abs_pd)

    PXVEC_IMPLEMENT_2BINARYFUN(min, _mm_min_pd)
    PXVEC_IMPLEMENT_2BINARYFUN(max, _mm_max_pd)

#if PLATFORM_INTEL_SSE >= 30
    PXVEC_IMPLEMENT_2BINARYFUN(horz_add, _mm_hadd_pd)
    PXVEC_IMPLEMENT_2BINARYFUN(horz_sub, _mm_hsub_pd)
#endif
        //friend FORCEINLINE float sum(F32vec8 const & a) { return _mm_sum_ps(a.half(0)) + _mm_sum_ps(a.half(1)); }
};

#endif


template <typename DOUBLE_TYPE>
struct FakeHalf : public DOUBLE_TYPE
{
    typename DOUBLE_TYPE BASE;
    typedef FakeHalf<DOUBLE_TYPE> SELF_TYPE;
    typedef typename DOUBLE_TYPE::SCALAR_TYPE SCALAR_TYPE;
    typedef DOUBLE_TYPE DOUBLE_TYPE;
    enum { SCALAR_COUNT = DOUBLE_TYPE::SCALAR_COUNT / 2 };

    FakeHalf(DOUBLE_TYPE const & vec)
        : BASE(vec)
    {
    }
};
/*
FORCEINLINE Is16vec4 Is16vec8::half(int i) const
{
return Is16vec4((i) ? *this : _mm_slli_si128(*this, sizeof(Is16vec4)));
}

*/
template <typename HALF_TYPE>
struct Vec2x
{
    typedef Vec2x<HALF_TYPE> SELF_TYPE;
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
        copy_native_vec<HNATIVE_VEC_TYPE>(vec[i], (HNATIVE_VEC_TYPE const &)half);
    }

    static FORCEINLINE void set_half(NATIVE_VEC_TYPE & vec, int i, HALF_TYPE const & half)
    {
        copy_native_vec<HNATIVE_VEC_TYPE>(vec[i], (HNATIVE_VEC_TYPE const &)half);
    }

    FORCEINLINE HALF_TYPE half(int i) const
    {
        return HALF_TYPE(vec[i & 1]);
    }

    // Always use set_half to initialize so you don't need to be aware about the data layout
    FORCEINLINE Vec2x() { }
    FORCEINLINE Vec2x(typename HALF_TYPE::NATIVE_VEC_TYPE const & a) { set_half(0, a); set_half(1, a); }
    FORCEINLINE Vec2x(typename HALF_TYPE::NATIVE_VEC_TYPE const & a, typename HALF_TYPE::NATIVE_VEC_TYPE const & b) { set_half(0, a); set_half(1, b); }
    FORCEINLINE Vec2x(NATIVE_VEC_TYPE const & a) { set_half(0, a[0]); set_half(1, a[1]); }
    FORCEINLINE Vec2x(SCALAR_TYPE const & a) { set_half(0, HALF_TYPE(a)); set_half(1, HALF_TYPE(a)); }

    static FORCEINLINE Vec2x<HALF_TYPE> zero_to_n()
    {
        return Vec2x<HALF_TYPE>(HALF_TYPE::zero_to_n(), HALF_TYPE(SCALAR_COUNT) + HALF_TYPE::zero_to_n());
    }

    template <typename OTHER_HALF_TYPE>
    FORCEINLINE Vec2x(Vec2x<OTHER_HALF_TYPE> const & a) { set_half(0, HALF_TYPE(a.half(0))); set_half(1, HALF_TYPE(a.half(1))); }

    static FORCEINLINE Vec2x<HALF_TYPE> zero() { return Vec2x<HALF_TYPE>(HALF_TYPE::zero(), HALF_TYPE::zero()); }
    static FORCEINLINE Vec2x<HALF_TYPE> undefined() { return Vec2x<HALF_TYPE>(HALF_TYPE::undefined(), HALF_TYPE::undefined()); }

#define PXVEC_IMPLEMENT_VecDType_FUN(f) \
    friend FORCEINLINE SELF_TYPE f(SELF_TYPE const & a) { return SELF_TYPE(f(a.half(0)), f(a.half(1))); }
    
#define PXVEC_IMPLEMENT_VecDType_BINFUN(f) \
    friend FORCEINLINE SELF_TYPE f(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(f(a.half(0), b.half(0)), f(a.half(1), b.half(1))); }

    PXVEC_IMPLEMENT_VecDType_FUN(abs)
    PXVEC_IMPLEMENT_VecDType_FUN(sqrt)

    PXVEC_IMPLEMENT_VecDType_FUN(signbit)
    PXVEC_IMPLEMENT_VecDType_BINFUN(andnot)

    // To define a = b + c AND a += b
#define PXVEC_IMPLEMENT_VecDType_BINARYOP(OP) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(a.half(0) OP b.half(0), a.half(1) OP b.half(1)); }\
    FORCEINLINE SELF_TYPE & operator OP##=(SELF_TYPE const & a) { return *this = SELF_TYPE(half(0) OP a.half(0), half(1) OP a.half(1)); }

    PXVEC_IMPLEMENT_VecDType_BINARYOP(+)
    PXVEC_IMPLEMENT_VecDType_BINARYOP(-)
    PXVEC_IMPLEMENT_VecDType_BINARYOP(/ )
    PXVEC_IMPLEMENT_VecDType_BINARYOP(*)

    PXVEC_IMPLEMENT_VecDType_BINARYOP(&)
    PXVEC_IMPLEMENT_VecDType_BINARYOP(| )
    PXVEC_IMPLEMENT_VecDType_BINARYOP(^)

#undef PXVEC_IMPLEMENT_VecDType_BINARYOP

        // To define a = b < c AND a = cmplt(b, c)
#define PXVEC_IMPLEMENT_VecDType_COMPARE(OP) \
    friend FORCEINLINE SELF_TYPE operator OP(SELF_TYPE const & a, SELF_TYPE const & b) { return SELF_TYPE(a.half(0) OP b.half(0), a.half(1) OP b.half(1)); }\

    PXVEC_IMPLEMENT_VecDType_COMPARE(== )
    PXVEC_IMPLEMENT_VecDType_COMPARE(!= )
    PXVEC_IMPLEMENT_VecDType_COMPARE(<= )
    PXVEC_IMPLEMENT_VecDType_COMPARE(>= )
    PXVEC_IMPLEMENT_VecDType_COMPARE(< )
    PXVEC_IMPLEMENT_VecDType_COMPARE(> )


#undef PXVEC_IMPLEMENT_VecDType_COMPARE
};

#define PXVEC_IMPLEMENT_VecDType_SHIFT(FUN) \
    template <int N, typename HALF_VECTOR_TYPE> \
    FORCEINLINE Vec2x<HALF_VECTOR_TYPE> FUN(Vec2x<HALF_VECTOR_TYPE> const & a) { \
    return Vec2x<HALF_VECTOR_TYPE>(FUN<N>(a.half(0)), FUN<N>(a.half(1))); }

PXVEC_IMPLEMENT_VecDType_SHIFT(shift_right_log)
PXVEC_IMPLEMENT_VecDType_SHIFT(shift_left_log)
PXVEC_IMPLEMENT_VecDType_SHIFT(shift_right)
PXVEC_IMPLEMENT_VecDType_SHIFT(shift_left)

#undef PXVEC_IMPLEMENT_VecDType_SHIFT

