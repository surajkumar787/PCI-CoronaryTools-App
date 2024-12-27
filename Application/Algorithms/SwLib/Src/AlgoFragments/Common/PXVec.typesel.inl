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

    PXVEC_IMPLEMENT_VECTYPE(F64vec8)
    PXVEC_IMPLEMENT_VECTYPE_INT(Is64vec8)
    PXVEC_IMPLEMENT_VECTYPE(Iu64vec8)

    PXVEC_IMPLEMENT_VECTYPE(F32vec16)
    PXVEC_IMPLEMENT_VECTYPE_INT(Is32vec16)

    PXVEC_IMPLEMENT_VECTYPE_INT(Is16vec32)

    PXVEC_IMPLEMENT_VECTYPE_FORCOPY(F32vec4)
    PXVEC_IMPLEMENT_VECTYPE_FORCOPY(F32vec8)

    PXVEC_IMPLEMENT_VECTYPE_FORCOPY(F64vec8)

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
