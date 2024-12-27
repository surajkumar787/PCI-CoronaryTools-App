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

namespace
{
    template <typename TYPE>
    struct PX_IsVoid
    {
        enum { Result = false };
    };

    template <>
    struct PX_IsVoid<void>
    {
        enum { Result = true };
    };

#define PX_ISVOID(TYPE) PX_IsVoid<(TYPE)>::Result

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

}