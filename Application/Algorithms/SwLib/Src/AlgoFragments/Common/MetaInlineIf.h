/*
 * Copyright (c)2011-2014 Koninklijke Philips Electronics N.V.,
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
#pragma once

#include "Platform.h"

namespace Philips
{
    namespace Medisys
    {
        namespace Meta
        {

    template <bool CONDITION>
    struct inline_if
    {
        template <typename F>
        static FORCEINLINE void go(F & f)
        {
            f();
        }

        template <typename F, typename T1>
        static FORCEINLINE void go(F & f, T1 & t1)
        {
            f(t1);
        }

        template <typename F, typename T1, typename T2>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2)
        {
            f(t1, t2);
        }

        template <typename F, typename T1, typename T2, typename T3>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3)
        {
            f(t1, t2, t3);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4)
        {
            f(t1, t2, t3, t4);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5)
        {
            f(t1, t2, t3, t4, t5);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6)
        {
            f(t1, t2, t3, t4, t5, t6);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7)
        {
            f(t1, t2, t3, t4, t5, t6, t7);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9, T10 & t10)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9, T10 & t10, T11 & t11)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
        static FORCEINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9, T10 & t10, T11 & t11, T12 & t12)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12);
        }
    };

    template <>
    struct inline_if<false>
    {
        template <typename F>
        static NOINLINE void go(F & f)
        {
            f();
        }

        template <typename F, typename T1>
        static NOINLINE void go(F & f, T1 & t1)
        {
            f(t1);
        }

        template <typename F, typename T1, typename T2>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2)
        {
            f(t1, t2);
        }

        template <typename F, typename T1, typename T2, typename T3>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3)
        {
            f(t1, t2, t3);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4)
        {
            f(t1, t2, t3, t4);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5)
        {
            f(t1, t2, t3, t4, t5);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6)
        {
            f(t1, t2, t3, t4, t5, t6);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7)
        {
            f(t1, t2, t3, t4, t5, t6, t7);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9, T10 & t10)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9, T10 & t10, T11 & t11)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11);
        }

        template <typename F, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10, typename T11, typename T12>
        static NOINLINE void go(F & f, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9, T10 & t10, T11 & t11, T12 & t12)
        {
            f(t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12);
        }
    };

        }
    }
}
