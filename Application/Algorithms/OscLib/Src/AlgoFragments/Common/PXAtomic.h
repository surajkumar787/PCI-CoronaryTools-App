#pragma once

// For better implementations
// see http://locklessinc.com/articles/locks/

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max

namespace Philips
{
    namespace Medisys
    {
        namespace PXAtomic
        {

typedef __int32 int32_t;
typedef int32_t spinlock_t;
typedef int32_t atomic_counter_t;

static FORCEINLINE int32_t atomic_inc(int32_t * ptr)
{
    return int32_t(_InterlockedIncrement((long *)ptr));
}

static FORCEINLINE int32_t atomic_add(int32_t * ptr, int32_t value)
{
    return (_InterlockedExchangeAdd((long *)ptr, (long)value) + value);
}

static FORCEINLINE int32_t atomic_exchange(int32_t * ptr, int32_t value)
{
    return (_InterlockedExchange((long *)ptr, (long)value));
}

static FORCEINLINE int32_t atomic_dec(int32_t * ptr)
{
    return int32_t(_InterlockedDecrement((long *)ptr));
}

static FORCEINLINE int32_t atomic_compare_exchange(int32_t * ptr, int32_t exchange, int32_t comparand)
{
    return int32_t(_InterlockedCompareExchange((long *)ptr, exchange, comparand));
}

#define SPINLOCK_FREE  0
#define SPINLOCK_TAKEN 1

static FORCEINLINE void spinlock_init(spinlock_t * ptr)
{
    *ptr = SPINLOCK_FREE;
}

static FORCEINLINE bool spinlock_trylock(spinlock_t * ptr)
{
    return (_InterlockedCompareExchange((long *)ptr, SPINLOCK_TAKEN, SPINLOCK_FREE) == SPINLOCK_FREE);
}

static FORCEINLINE bool spinlock_islocked(spinlock_t * ptr)
{
    return (_InterlockedCompareExchange((long *)ptr, SPINLOCK_TAKEN, SPINLOCK_TAKEN) == SPINLOCK_TAKEN);
}

static FORCEINLINE void spinlock_lock(spinlock_t * ptr)
{
    //while (*ptr);
    while (!spinlock_trylock(ptr)) _mm_pause();
}

static FORCEINLINE void spinlock_unlock(spinlock_t * ptr)
{
    //*ptr = SPINLOCK_FREE;
    _InterlockedExchange((long *)ptr, SPINLOCK_FREE);
}

static FORCEINLINE bool nested_spinlock_trylock(spinlock_t * ptr, int threadNumber)
{
    threadNumber++;
    long previousValue = _InterlockedCompareExchange((long *)ptr, threadNumber, SPINLOCK_FREE);
    return (previousValue == SPINLOCK_FREE || previousValue == threadNumber);
}

static FORCEINLINE bool nested_spinlock_islocked(spinlock_t * ptr, int threadNumber)
{
    threadNumber++;
    return (_InterlockedCompareExchange((long *)ptr, threadNumber, threadNumber) == threadNumber);
}

static FORCEINLINE void nested_spinlock_lock(spinlock_t * ptr, int threadNumber)
{
    while (!nested_spinlock_trylock(ptr, threadNumber));
}


static FORCEINLINE void nested_spinlock_unlock(spinlock_t * ptr)
{
//    *ptr = SPINLOCK_FREE;
    _InterlockedExchange((long *)ptr, SPINLOCK_FREE);
}

template <int LOG2SIZE, typename TYPE>
class AtomicStack
{
    enum { SIZE = 1 << LOG2SIZE };
    spinlock_t m_lock;
    int  m_iSize;
    TYPE m_iItems[SIZE];

public:
    FORCEINLINE AtomicStack()
    {
        spinlock_init(&m_lock);
        m_iSize = 0;
    }

    FORCEINLINE bool Available() const
    {
        return m_iSize > 0;
    }

    FORCEINLINE void Push(TYPE const & tItem)
    {
        spinlock_lock(&m_lock);
        m_iItems[m_iSize++] = tItem;
        spinlock_unlock(&m_lock);
    }

    FORCEINLINE TYPE Pop()
    {
        TYPE tResult;
        spinlock_lock(&m_lock);
        tResult = m_iItems[--m_iSize];
        spinlock_unlock(&m_lock);
        return tResult;
    }
};

template <int LOG2SIZE, typename TYPE>
class AtomicQueue
{
    enum { SIZE = 1 << LOG2SIZE };

    spinlock_t m_lock;
    int  m_iBegin;
    int  m_iEnd;
    TYPE m_iItems[SIZE];

public:
    FORCEINLINE AtomicQueue()
    {
        spinlock_init(&m_lock);
        m_iBegin = 0;
        m_iEnd = 0;
    }

    FORCEINLINE void Clear()
    {
        spinlock_init(&m_lock);
        m_iBegin = 0;
        m_iEnd = 0;
    }

    FORCEINLINE void PushBack(TYPE const & tItem)
    {
        spinlock_lock(&m_lock);
        m_iItems[m_iEnd] = tItem;
        m_iEnd = (m_iEnd + 1) & (SIZE - 1);
        spinlock_unlock(&m_lock);
    }

    FORCEINLINE void PushFront(TYPE const & tItem)
    {
        spinlock_lock(&m_lock);
        m_iBegin = (m_iBegin - 1) & (SIZE - 1);
        m_iItems[m_iBegin] = tItem;
        spinlock_unlock(&m_lock);
    }

    FORCEINLINE bool TryPop(TYPE & tItem)
    {
        bool bResult;
        if (!spinlock_trylock(&m_lock)) return false;
        bResult = (m_iBegin != m_iEnd);
        if (bResult)
        {
            tItem = m_iItems[m_iBegin];
            m_iBegin = (m_iBegin + 1) & (SIZE - 1);
        }
        spinlock_unlock(&m_lock);
        return bResult;
    }

    FORCEINLINE bool Pop(TYPE & tItem)
    {
        bool bResult;
        spinlock_lock(&m_lock);
        bResult = (m_iBegin != m_iEnd);
        if (bResult)
        {
            tItem = m_iItems[m_iBegin];
            m_iBegin = (m_iBegin + 1) & (SIZE - 1);
        }
        spinlock_unlock(&m_lock);
        return bResult;
    }
};
        }
    }
}

#endif
