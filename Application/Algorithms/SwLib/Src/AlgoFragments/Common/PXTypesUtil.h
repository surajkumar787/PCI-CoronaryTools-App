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

// Part of PXUtil that is needed by PXVec, some stuff might be redundant with C++11 but it works with classic C++

namespace
{
    // int literal
    template <int N>
    struct $I { enum { _ = N }; };

#define PX_IMPL_IS_TRUE typedef $I<1> Type; enum { Result = true }
#define PX_IMPL_IS_FALSE typedef $I<0> Type; enum { Result = false }

    template <typename TYPE>
    struct PX_IsArray
    {
        PX_IMPL_IS_FALSE;
    };

    template <typename TYPE, size_t N>
    struct PX_IsArray<TYPE[N]>
    {
        PX_IMPL_IS_TRUE;
    };

#define PX_ISARRAY(TYPE) PX_IsArray<(TYPE)>::Result
#define PX_ISARRAY_TYPE(TYPE) typename PX_IsArray<(TYPE)>::Type

    template <typename TYPE>
    struct PX_IsVoid
    {
        PX_IMPL_IS_FALSE;
    };

    template <>
    struct PX_IsVoid<void>
    {
        PX_IMPL_IS_TRUE;
    };

#define PX_ISVOID(TYPE) PX_IsVoid<(TYPE)>::Result
#define PX_ISVOID_TYPE(TYPE) typename PX_IsVoid<(TYPE)>::Type

#undef PX_IMPL_IS_FALSE
#undef PX_IMPL_IS_TRUE



    template <bool COND, typename TRUE_TYPE, typename FALSE_TYPE>
    struct PX_TypeSelect
    {
        typedef TRUE_TYPE ResultType;
    };

    template <typename TRUE_TYPE, typename FALSE_TYPE>
    struct PX_TypeSelect<false, TRUE_TYPE, FALSE_TYPE>
    {
        typedef FALSE_TYPE ResultType;
    };

#define PX_TYPESELECT(CND,Y,N) PX_TypeSelect<(CND),Y,N>::ResultType

    template <typename T1, typename T2>
    struct PX_TypeCompare
    {
        enum { Result = false };
    };

    template <typename T1>
    struct PX_TypeCompare<T1, T1>
    {
        enum { Result = true };
    };

#define PX_TYPECOMPARE(T1,T2) PX_TypeCompare<T1,T2>::Result

    template <typename T1>
    struct PX_TypeUnsign
    {
        typedef T1 ResultType;
    };

    template <typename T1>
    struct PX_TypeSign
    {
        typedef T1 ResultType;
    };

#define PX_TYPEUNSIGN_SETTYPE(x, y) \
    template <> struct PX_TypeUnsign<x> { typedef y ResultType; }; \
    template <> struct PX_TypeSign<y> { typedef x ResultType; };

    PX_TYPEUNSIGN_SETTYPE(int8_t , uint8_t);
    PX_TYPEUNSIGN_SETTYPE(int16_t, uint16_t);
    PX_TYPEUNSIGN_SETTYPE(int32_t, uint32_t);
    PX_TYPEUNSIGN_SETTYPE(int64_t, uint64_t);

#define PX_TYPEUNSIGN(T1) PX_TypeUnsign<T1>::ResultType
#define PX_TYPESIGN(T1) PX_TypeSign<T1>::ResultType

    /*
    template <typename T1>
    struct PX_TypeSign<true, T1>
    {
        typedef typename PXVEC_TYPE_FORSIZE(typename PX_TYPESIGN(typename T1::SCALAR_TYPE), T1::SCALAR_COUNT) ResultType;
    };

    template <typename T1>
    struct PX_TypeUnsign<true, T1>
    {
        typedef typename PXVEC_TYPE_FORSIZE(typename PX_TYPEUNSIGN(typename T1::SCALAR_TYPE), T1::SCALAR_COUNT) ResultType;
    };
    */

    /*

    template <typename T1>
    struct PX_TypeUnsign<true, T1>
    {
        typedef typename PXVEC_TYPE_FORSIZE(typename PX_TYPEUNSIGN(typename T1::SCALAR_TYPE), T1::SCALAR_COUNT) ResultType;
    };
    */

    template <int BYTES>
    struct impl_PX_intxx_t { enum { MAX = 8 }; typedef typename impl_PX_intxx_t<__min(MAX, BYTES + 1)>::SType SType; typedef typename impl_PX_intxx_t<__min(MAX, BYTES + 1)>::UType UType; };

#define PX_IMPLEMENT_INTXX(SIGNED, UNSIGNED) \
    template <> struct impl_PX_intxx_t<sizeof(SIGNED)> { typedef SIGNED SType; typedef UNSIGNED UType; };

PX_IMPLEMENT_INTXX(int64_t, uint64_t)
PX_IMPLEMENT_INTXX(int32_t, uint32_t)
PX_IMPLEMENT_INTXX(int16_t, uint16_t)
PX_IMPLEMENT_INTXX(int8_t, uint8_t)

#define PX_INTXX(N) impl_PX_intxx_t<N>::SType
#define PX_UINTXX(N) impl_PX_intxx_t<N>::UType

}