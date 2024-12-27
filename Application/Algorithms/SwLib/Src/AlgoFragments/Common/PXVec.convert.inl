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

template <typename TYPE_OUT, typename HALF_TYPE_IN>
TYPE_OUT convert(Vec2x<HALF_TYPE_IN> const & src)
{
    return TYPE_OUT(convert<typename TYPE_OUT::HALF_TYPE>(src.half(0)), convert<typename TYPE_OUT::HALF_TYPE>(src.half(1)));
}

template <>
FORCEINLINE Is16vec16 convert<Is16vec16, Iu8vec16>(Iu8vec16 const & src)
{
    return Is16vec16(unpack(src, Iu8vec16::zero()));
}

template <>
FORCEINLINE F64vec4 convert<F64vec4, F32vec4>(F32vec4 const & src)
{
    return F64vec4(_mm_cvtps_pd(src), _mm_cvtps_pd(shiftreg_right<2>(src)));
}

template <>
FORCEINLINE F32vec4 convert<F32vec4, F64vec4>(F64vec4 const & src)
{
    return F32vec4(_mm_shuffle_ps(_mm_cvtpd_ps(src.half(0)), _mm_cvtpd_ps(src.half(1)), _MM_SHUFFLE(1, 0, 1, 0)));
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
    return F32vec8(convert<F32vec4>(src.half(0)), convert<F32vec4>(src.half(1)));
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
FORCEINLINE Is16vec8 convert<Is16vec8, F32vec4>(F32vec4 const & input)
{
    return sat_pack(convert<Is32vec4>(input), Is32vec4::undefined());
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
FORCEINLINE Is16vec8 convert<Is16vec8, Is32vec4>(Is32vec4 const & src)
{
    return convert<Is16vec8>(sat_pack(src, Is32vec4::undefined()));
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
FORCEINLINE Is32vec4 convert<Is32vec4, Is16vec8>(Is16vec8 const & src)
{
    return convert<0, Is32vec4, Is16vec8>(src);
}

template <>
FORCEINLINE Iu32vec4 convert<0, Iu32vec4, Is16vec8>(Is16vec8 const & src)
{
    return Iu32vec4(_mm_unpacklo_epi16(_mm_max_epi16(_mm_setzero_si128(), src), _mm_setzero_si128()));
}

template <>
FORCEINLINE Iu32vec4 convert<1, Iu32vec4, Is16vec8>(Is16vec8 const & src)
{
    return Iu32vec4(_mm_unpackhi_epi16(_mm_max_epi16(_mm_setzero_si128(), src), _mm_setzero_si128()));
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
FORCEINLINE Is32vec4 convert<1, Is32vec4, Is32vec8>(Is32vec8 const & src)
{
    return src.half(1);
}

template <>
FORCEINLINE Is32vec4 convert<0, Is32vec4, Is32vec8>(Is32vec8 const & src)
{
    return src.half(0);
}
template <>
FORCEINLINE F32vec4 convert<1, F32vec4, F32vec8>(F32vec8 const & src)
{
    return src.half(1);
}

template <>
FORCEINLINE F32vec4 convert<0, F32vec4, F32vec8>(F32vec8 const & src)
{
    return src.half(0);
}


template <>
FORCEINLINE F32vec4 convert<1, F32vec4, Is32vec8>(Is32vec8 const & src)
{
    return convert<F32vec4>(src.half(1));
}

template <>
FORCEINLINE F32vec4 convert<0, F32vec4, Is32vec8>(Is32vec8 const & src)
{
    return convert<F32vec4>(src.half(0));
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
        return _mm_movemask_pd(F64vec2(input));
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
        return _mm256_movemask_pd(F64vec4(input));
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
        return convert_to_mask(Is16vec8(_mm_packs_epi16(Is16vec8(input.half(0)), Is16vec8(input.half(1)))));
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
