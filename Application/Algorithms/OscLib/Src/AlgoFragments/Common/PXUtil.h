/*!
* \file PXUtil.h
*
* \author
* Emmanuel Attia (emmanuel.attia@philips.com)
* The REACT Project - MedISys Research Lab - Philips HealthCare
*
* \brief
* REACT internal productivity tool: Utility template functions.
*
* \date 28 july 2010: Initial header.
*
*/
//TICS -*
#pragma once

#define PX_ELEMCOUNT(x) sizeof((x))/sizeof(*(x))

template <typename TYPE>
struct PX_ConstantArray
{
    TYPE t;
    FORCEINLINE PX_ConstantArray(TYPE t)
        : t(t)
    {
    }

    FORCEINLINE TYPE operator [] (int i) const
    {
        return t;
    }
};

template <typename TYPE>
FORCEINLINE PX_ConstantArray<TYPE> PX_Make_ConstantArray(TYPE t)
{
    return PX_ConstantArray<TYPE>(t);
}
template <int N>
FORCEINLINE int PX_AlignByteOffset(const void * ptr)
{
    return (int)(intptr_t)(sizeof(char) * ((char *)ptr - (char *)NULL) & (N - 1));
}

template <typename TYPE> class PX_AssertTypeIsNotVoid { };
template <> class PX_AssertTypeIsNotVoid<void> { private: PX_AssertTypeIsNotVoid() { } };

template <bool> class PX_Assert { };
template <> class PX_Assert<false> { private: PX_Assert() { } };

#define PX_STATICASSERT(condition)  { switch(1) { case 0: case (condition): break; } }

template <int N, typename TYPE>
FORCEINLINE int PX_AlignOffset(TYPE const * ptr)
{
    // This function must not be instantiated with TYPE = void
    PX_AssertTypeIsNotVoid<TYPE>();
    return PX_AlignByteOffset<N>(ptr) / (int)sizeof(TYPE);
}

template <int I, int N, typename TYPE_OUT, typename TYPE_IN>
FORCEINLINE void PX_CopyArray(TYPE_OUT (& dest)[N], TYPE_IN const * src)
{
    int const NEXT_I = __min(I + 1, N - 1);

    dest[I] = (TYPE_OUT)src[I];

    if (I < NEXT_I)
        PX_CopyArray<NEXT_I, N, TYPE_OUT, TYPE_IN>(dest, src);
}

template <int N, typename TYPE_OUT, typename TYPE_IN>
FORCEINLINE void PX_CopyArray(TYPE_OUT (& dest)[N], TYPE_IN const * src)
{
    PX_CopyArray<0, N, TYPE_OUT, TYPE_IN>(dest, src);
}

template <typename TYPE_OUT, typename TYPE_IN>
FORCEINLINE void PX_CopyArray(TYPE_OUT * dest, TYPE_IN const * src, int N)
{
    for (int i = 0; i != N; i++)
        dest[i] = src[i];
}

template <int I, int N, typename TYPE>
FORCEINLINE bool PX_EqualArray(TYPE const (& src1)[N], TYPE const (& src2)[N])
{
    int const NEXT_I = __min(I + 1, N - 1);

    if (src1[I] != src2[I])
        return false;

    if (I < NEXT_I)
        return PX_EqualArray<NEXT_I, N, TYPE>(src1, src2);

    return true;
}

template <int N, typename TYPE>
FORCEINLINE bool PX_EqualArray(TYPE const (& src1)[N], TYPE const (& src2)[N])
{
    return PX_EqualArray<0, N, TYPE>(src1, src2);
}

template <int I, int N, typename TYPE>
FORCEINLINE void PX_SetArray(TYPE (& dest)[N], TYPE const & src)
{
    int const NEXT_I = __min(I+1,N-1);
    dest[I] = src;
    if (I < NEXT_I)
        PX_SetArray<NEXT_I, N, TYPE>(dest, src);
}

template <int N, typename TYPE>
FORCEINLINE void PX_SetArray(TYPE (& dest)[N], TYPE const & src)
{
    PX_SetArray<0, N, TYPE>(dest, src);
}

template <typename TYPE>
FORCEINLINE void PX_SetArray(TYPE * dest, TYPE const & src, int N)
{
    for (int i = 0; i != N; i++)
        dest[i] = src;
}

template <typename T1>
FORCEINLINE void PX_Swap(T1 & a, T1 & b)
{
	T1 c = b;
	b = a;
	a = c;
}

template <typename T>
FORCEINLINE void PX_Wrap(T & val, T min, T max)
{
    while (val  < min) val += (max - min);
    while (val >= max) val -= (max - min);
}

#if defined(_OMPAPI) || defined(__OMP_H)
#include <stdio.h>

struct PX_Timer
{
    char const * m_p;
    double m_begin;
    int m_trialCount;
    bool m_condition;
    PX_Timer(char const * p, int trialCount = 1, bool condition = true) : m_p(p),m_condition(condition) { m_begin = omp_get_wtime(); m_trialCount = trialCount; }
    ~PX_Timer() { double const secToMs = 1000.0; if (m_condition) printf("%s: %.2f ms\n", m_p, secToMs * (omp_get_wtime() - m_begin) / m_trialCount); }
};

#endif

#include <functional>
#include <map>
#include <vector>

template <typename KEY, typename VALUE, int HASH_SIZE = 256>
class PX_HashMap
{
private:
    ::std::vector< ::std::map<KEY, VALUE> > m_container;
public:
    FORCEINLINE PX_HashMap()
        : m_container(HASH_SIZE)
    {
    }

    FORCEINLINE ~PX_HashMap()
    {
    }

    template <typename F>
    FORCEINLINE void for_each(F f)
    {
        for (size_t hashCode = 0; hashCode != m_container.size(); hashCode++)
        {
            for (typename ::std::map<KEY, VALUE>::const_iterator it = m_container[hashCode].begin();
                 it != m_container[hashCode].end(); it++)
            {
                f(it->second);
            }
        }
    }

    template <typename F>
    FORCEINLINE void put(KEY const & key, VALUE const & value, F getHashCode)
    {
        int hashCode = getHashCode(key) & (HASH_SIZE - 1);
        typename ::std::map<KEY, VALUE>::const_iterator it = m_container[hashCode].find(key);
        if (it == m_container[hashCode].end())
            m_container[hashCode].insert(::std::make_pair(key, value));
    }

    template <typename F>
    FORCEINLINE bool get(KEY const & key, VALUE & value, F getHashCode) const
    {
        int hashCode = getHashCode(key) & (HASH_SIZE - 1);
        typename ::std::map<KEY, VALUE>::const_iterator it = m_container[hashCode].find(key);
        if (it != m_container[hashCode].end())
        {
            value = it->second;
            return true;
        }
        return false;
    }
    
    template <typename RESULT_LIST, typename F>
    FORCEINLINE void get_many(KEY const & key, RESULT_LIST & result_list, F getHashCode)
    {
        int hashCode = getHashCode(key) & (HASH_SIZE - 1);
        typename ::std::map<KEY, VALUE>::const_iterator it = m_container[hashCode].find(key);
        if (it != m_container[hashCode].end())
        {
            result_list.push_back(it->second);
        }
    }
};

#define PX_ROUNDUP(VAL, N) ((((VAL) + (N) - 1)/(N))*(N))
#define PX_ROUNDDOWN(VAL, N) ((((VAL))/(N))*(N))
#define PX_ROUND(VAL, N) ((((VAL) + ((N) / 2))/(N))*(N))
#define PX_DIVCEIL(A, B) (((A)+(B)-1)/(A))
