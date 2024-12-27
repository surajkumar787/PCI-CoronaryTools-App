/*!
* \file Platform.h
*
* \author
* Emmanuel Attia (emmanuel.attia@philips.com)
* The REACT Project - MedISys Research Lab - Philips HealthCare
*
* \brief
* REACT internal productivity tool: Platform (machine and compiler) specific includes.
*
* Consider including this file in order to use SIMD extensions.
* Currently only supports Intel Pentium/NetBurst/Core architecture.
* The value of PLATFORM_INTEL_SSE specifies the level of SIMD supported:
* -  5 for MMX
* - 10 for SSE
* - 20 for SSE2
* - 30 for SSE3
* - 35 for SSSE3
* - 41 for SSE4.1
* - 42 for SSE4.2
*
* \date 19 june 2008: Initial header
*
*/
//TICS -*
#pragma once

#include <assert.h>

#ifdef __GNUC__

    #include <stdint.h>

    // In gcc or clang, we determine this by the command-line only (-mssexx)
    #ifdef PLATFORM_INTEL_SSE
        #undef PLATFORM_INTEL_SSE
    #endif
    #ifdef PLATFORM_INTEL_SSE_COMPILABLE
        #undef PLATFORM_INTEL_SSE_COMPILABLE
    #endif

    #if    defined(__SSE4_2__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    42
    #elif  defined(__SSE4_1__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    41
    #elif  defined(__SSSE3__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    35
    #elif  defined(__SSE3__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    30
    #elif  defined(__SSE2__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    20
    #elif  defined(__SSE__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    10
    #elif  defined(__MMX__)
    #define PLATFORM_INTEL_SSE_COMPILABLE    05
    #endif
    #define PLATFORM_INTEL_SSE PLATFORM_INTEL_SSE_COMPILABLE

#else /* !__GNUC__ */

#ifndef PLATFORM_INTEL_SSE
    #define PLATFORM_INTEL_SSE    35
#endif

#endif /* __GNUC__ */

#ifdef _MSC_VER
    #if _MSC_VER < 1600
        #include "stdint.h"
    #else
        // Supported from Visual C++ 2010
        #include <stdint.h>
    #endif

#ifdef _M_AMD64
#define PLATFORM_INTEL_ARCH 64
#else
#define PLATFORM_INTEL_ARCH 32
#endif

    // MSVC mode: CL or ICL
    #define FORCEINLINE         __forceinline
    #define NOINLINE            __declspec(noinline)
    #define RECURSIVEINLINE     __forceinline
    #define PLATFORM_ALIGNED(x) __declspec(align(x))

    #ifdef __ICL

    #ifndef PLATFORM_INTEL_SSE_COMPILABLE
    #if __ICL >= 1100
    // Intel C++ 11.xx
    #define PLATFORM_INTEL_SSE_COMPILABLE    50
    #elif __ICL >= 1000
    // Intel C++ 10.xx
    #define PLATFORM_INTEL_SSE_COMPILABLE    42
    #elif __ICL >= 900
    // Intel C++ 9.xx
    #define PLATFORM_INTEL_SSE_COMPILABLE    35
    #elif __ICL >= 800
    // Intel C++ 8.xx
    #define PLATFORM_INTEL_SSE_COMPILABLE    30
    #else
    #define PLATFORM_INTEL_SSE_COMPILABLE     0
    #endif
    #endif

    #pragma warning(disable:111) // statement is unreachable
    #pragma warning(disable:373) // "..." is inaccessible. <= For some reason with IC 9.xx Microsoft STL header triggers that warning
    #pragma warning(disable:869) // parameter "..." was never referenced
    //#pragma warning(disable:444) // destructor for base class is not virtual
    #pragma warning(disable:981) // operands are evaluated in unspecified order
    //#pragma warning(disable:383) // value copied to temporary, reference to temporary used <= we know that
    #pragma warning(disable:1572) // floating operands are evaluated in unspecified orderg-point equality and inequality comparisons are unreliable <= we don't agree, 0 is binary 0
    #pragma warning(disable:2196 ) // routine is both "inline" and "noinline" ("noinline" assumed)

    #else /* !__ICL */

    #pragma warning(disable:4714) // Some templates recursion with __forceinline keywork will fail the inlining at the terminal condition (but the non inlined function is empty so it's not an issue). I.e: PX_CopyArray
    #pragma warning(disable:4127) // Because we know that we will use conditionnal constant expression a LOT in template instantiation
    #pragma warning(disable:4100) // Unreferenced formal parameters is an annoying warning

    #ifndef PLATFORM_INTEL_SSE_COMPILABLE
    #if _MSC_VER >= 0x0A00
    // Visual C++ 2010
    #define PLATFORM_INTEL_SSE_COMPILABLE    42
    #elif _MSC_VER >= 1500
    // Visual C++ 2008
    #define PLATFORM_INTEL_SSE_COMPILABLE    40
    #elif _MSC_VER >= 1300
    // Visual C++ 2002/2003/2005
    #define PLATFORM_INTEL_SSE_COMPILABLE    20
    #else
    #define PLATFORM_INTEL_SSE_COMPILABLE     0
    #endif
    #endif

    #endif /* __ICL */

    // Prevent inclusion of <xvec.h>
#define _DVEC_H_INCLUDED
#define _FVEC_H_INCLUDED
#define _IVEC_H_INCLUDED
#define DVEC_H_INCLUDED
#define FVEC_H_INCLUDED
#define IVEC_H_INCLUDED

#elif defined(__GNUC__)

    // GCC mode: GCC or ICC or clang
    #define FORCEINLINE          __inline__
    #define RECURSIVEINLINE      inline
    #define NOINLINE             __attribute__((noinline))
    #define PLATFORM_ALIGNED(x)  __attribute__((aligned(x)))

#else /* !__GNUC__ && !_MSC_VER */

    #error "Please adapt 'Platform.h' to your compiler."

#endif

#ifndef PLATFORM_INTEL_SSE_COMPILABLE
#define PLATFORM_INTEL_SSE_COMPILABLE 99
#endif  /* PLATFORM_INTEL_SSE_COMPILABLE */

#ifdef PLATFORM_INTEL_SSE

#if PLATFORM_INTEL_SSE > PLATFORM_INTEL_SSE_COMPILABLE
#pragma message("Warning: Downgrading SSE.")
#undef  PLATFORM_INTEL_SSE
#define PLATFORM_INTEL_SSE PLATFORM_INTEL_SSE_COMPILABLE
#endif /* PLATFORM_INTEL_SSE > PLATFORM_INTEL_SSE_COMPILABLE */

#if PLATFORM_INTEL_SSE_COMPILABLE >= 50
/* AVX */
#include <immintrin.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 42
/* SSE4.2 */
#include <nmmintrin.h>
#include <wmmintrin.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 40
/* SSE4 aka SSE4.1 "Vectorizing Compiler and Media Accelerators" */
#include <smmintrin.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 35
/* SSSE3: Core(r) 2 Duo */
#include <tmmintrin.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 30
/* SSE3: Pentium(r) 4 775 */
#include <pmmintrin.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 20
/* SSE2: Pentium(r) IV */
#include <emmintrin.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 10
/* SSE: Pentium(r) III */
#include <xmmintrin.h>
#include <sse2mmx.h>
#elif PLATFORM_INTEL_SSE_COMPILABLE >= 5
/* MMX: Pentium(r) MMX and Pentium(r) II */
#include <mmintrin.h>
#endif

#endif /* PLATFORM_INTEL_SSE */

#ifndef _MM_ALIGN16
#define _MM_ALIGN16 PLATFORM_ALIGNED(16)
#endif

//TICS -NAM#008
#ifndef __max
#define __max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef __min
#define __min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef __abs
#define __abs(a)  __max((a),(-(a)))
#endif
//TICS +NAM#008

#if defined(min) || defined(max)
#include <algorithm>

// Avoids collisions between STL and <windows.h>
#ifdef min
#undef min
using ::std::min;
#endif

#ifdef max
#undef max
using ::std::max;
#endif
#endif

#include "PlatformCPUDetect.h"
#ifndef _M_CEE
// Following packages not available when compiled in managed C++
#include "PlatformSSEmul.h"
#include "PXVec.h"
#include "PXUtil.h"
#endif
