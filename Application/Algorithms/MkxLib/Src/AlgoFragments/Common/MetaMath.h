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

template <int X> struct Sign { enum { Value = X >= 0 ? 1 : -1 }; };

template <int X> struct Log2 { enum { Value = Log2<__max(1, X/2)>::Value + 1 }; };
template <> struct Log2<1> { enum { Value = 0 }; };

template <int N, int K>
struct Combinations { enum { Value = Combinations<N-1,K-1>::Value + Combinations<N-1, K>::Value }; };
template <>      struct Combinations<0, 0> { enum { Value = 1 }; };
template <int K> struct Combinations<0, K> { enum { Value = 0 }; };
template <int N> struct Combinations<N, 0> { enum { Value = 1 }; };

template <int X, int EXP> struct Pow { enum { Value = Pow<X, EXP - 1>::Value * (X) }; };
template <int X> struct Pow<X, 0> { enum { Value = 1 }; };

        }
    }
}
