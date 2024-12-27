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

// For better implementations
// see http://locklessinc.com/articles/locks/

#ifdef _WIN32
#include <intrin.h>

namespace
{
    namespace Win32Wrapper
    {
        // Not very nice, but we avoid to be polluted with Win32 symbols
        extern "C" void *   __stdcall CreateEventA          (void *, int32_t, int32_t, char const *);
        extern "C" void *   __stdcall CreateSemaphoreA      (void *, int32_t, int32_t, char const *);
        extern "C" int32_t  __stdcall ReleaseSemaphore      (void *, int32_t, int32_t *);
        extern "C" uint32_t __stdcall WaitForSingleObject   (void *, uint32_t);
        extern "C" uint32_t __stdcall WaitForMultipleObjects(uint32_t, void *, int32_t, uint32_t);
        extern "C" int32_t  __stdcall CloseHandle           (void *);

        enum { WaitAbandoned = 0x80 };
    }
    namespace PXAtomicImpl
    {

typedef __int32 spinlock_t;
typedef __int32 volatile * spinlock_ptr_t;
typedef __int32 atomic_counter_t;

template <int N = 1>
class PLATFORM_NOVTABLE Semaphores
{
   void * ptr[N];
public:
    FORCEINLINE Semaphores()
    {
        for (int i = 0; i < N; i++)
            ptr[i] = Win32Wrapper::CreateSemaphoreA(NULL, 0, 1, NULL);
    }

    FORCEINLINE ~Semaphores()
    {
        for (int i = 0; i < N; i++)
            Win32Wrapper::CloseHandle(ptr[i]);
    }

    FORCEINLINE int Wait(uint32_t timeOut)
    {
        uint32_t res = Win32Wrapper::WaitForMultipleObjects(N, ptr, 0, timeOut);
        if (res >= Win32Wrapper::WaitAbandoned)
            return -1;
        return res;
    }
    
    FORCEINLINE int Wait()
    {
        return Wait(~0);
    }
    
    FORCEINLINE int WaitAll(uint32_t timeOut)
    {
        uint32_t res = Win32Wrapper::WaitForMultipleObjects(N, ptr, 1, timeOut);
        if (res >= Win32Wrapper::WaitAbandoned)
            return -1;
        return res;
    }
    
    FORCEINLINE int WaitAll()
    {
        return WaitOne(~0);
    }

    template <int I>
    FORCEINLINE void Signal()
    {
        ReleaseSemaphore(ptr[I], 1, NULL);
    }

    FORCEINLINE void Signal()
    {
        Signal<0>();
    }

};

typedef Semaphores<1> Semaphore;

// Semaphore-based mutex
class PLATFORM_NOVTABLE Mutex
{
    void * ptr;
public:
    FORCEINLINE Mutex()
    {
        ptr = Win32Wrapper::CreateSemaphoreA(NULL, 1, 1, NULL);
    }

    FORCEINLINE ~Mutex()
    {
        Win32Wrapper::CloseHandle(ptr);
    }

    FORCEINLINE bool Lock(uint32_t timeOut)
    {
        return (Win32Wrapper::WaitForSingleObject(ptr, timeOut) == 0);
    }
    
    FORCEINLINE bool Lock()
    {
        return (Win32Wrapper::WaitForSingleObject(ptr, ~0) == 0);
    }

    FORCEINLINE bool Unlock()
    {
        int32_t prevCount = 0;
        if (Win32Wrapper::ReleaseSemaphore(ptr, 1, &prevCount))
        {
            return prevCount == 0;
        }
        return false;
    }
};

namespace
{
    class PLATFORM_NOVTABLE LightSleepHelper
    {
        void * ptr;
    public:
        FORCEINLINE LightSleepHelper()
        {
            ptr = Win32Wrapper::CreateEventA(NULL, 1, 1, NULL);
        }

        FORCEINLINE void operator()() const
        {
            Win32Wrapper::WaitForSingleObject(ptr, 0);
        }

        FORCEINLINE ~LightSleepHelper()
        {
            Win32Wrapper::CloseHandle(ptr);
        }
    };
};

// Sleep much less than 16 ms (Sleep (1)) but more than 0 ( Sleep(0) keeps the CPU at 100% )
static FORCEINLINE void light_sleep()
{
    static LightSleepHelper helper;
    helper();
}

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

static FORCEINLINE void spinlock_init(spinlock_ptr_t ptr)
{
    *ptr = SPINLOCK_FREE;
}

static FORCEINLINE bool spinlock_trylock(spinlock_ptr_t ptr)
{
    return (_InterlockedCompareExchange((long *)ptr, SPINLOCK_TAKEN, SPINLOCK_FREE) == SPINLOCK_FREE);
}

static FORCEINLINE bool spinlock_islocked(spinlock_ptr_t ptr)
{
    return (_InterlockedCompareExchange((long *)ptr, SPINLOCK_TAKEN, SPINLOCK_TAKEN) == SPINLOCK_TAKEN);
}

static FORCEINLINE void spinlock_lock(spinlock_ptr_t ptr)
{
    //while (*ptr);
    for (;;)
    {
        int i;
        for (i = 128; i >= 0 && (false == spinlock_trylock(ptr)); i--)
        {
            if (*ptr) if (*ptr) if (*ptr) if (*ptr) _mm_pause();
        }
        if (i)
            break;
        light_sleep();
    }
}

static FORCEINLINE void spinlock_passive_lock(spinlock_ptr_t ptr)
{
    //while (*ptr);
    while (!spinlock_trylock(ptr)) if (*ptr) if (*ptr) if (*ptr) if (*ptr) _mm_pause();
}

static FORCEINLINE void spinlock_unlock(spinlock_ptr_t ptr)
{
    //*ptr = SPINLOCK_FREE;
    _InterlockedExchange((long *)ptr, SPINLOCK_FREE);
}

static FORCEINLINE bool nested_spinlock_trylock(spinlock_ptr_t ptr, int threadNumber)
{
    threadNumber++;
    long previousValue = _InterlockedCompareExchange((long *)ptr, threadNumber, SPINLOCK_FREE);
    return (previousValue == SPINLOCK_FREE || previousValue == threadNumber);
}

static FORCEINLINE bool nested_spinlock_islocked(spinlock_ptr_t ptr, int threadNumber)
{
    threadNumber++;
    return (_InterlockedCompareExchange((long *)ptr, threadNumber, threadNumber) == threadNumber);
}

static FORCEINLINE void nested_spinlock_lock(spinlock_ptr_t ptr, int threadNumber)
{
    while (!nested_spinlock_trylock(ptr, threadNumber));
}


static FORCEINLINE void nested_spinlock_unlock(spinlock_ptr_t ptr)
{
//    *ptr = SPINLOCK_FREE;
    _InterlockedExchange((long *)ptr, SPINLOCK_FREE);
}

// Non-nestable spinlock
class PLATFORM_NOVTABLE Spinlock
{
    spinlock_t ptr;
public:
    FORCEINLINE Spinlock()
    {
        spinlock_init(&ptr);
    }
    
    FORCEINLINE bool IsLock()
    {
        return spinlock_islocked(&ptr);
    }

    FORCEINLINE bool Lock()
    {
        spinlock_lock(&ptr);
        return true;
    }

    FORCEINLINE bool Unlock()
    {
        spinlock_unlock(&ptr);
        return true;
    }
};

// Non-nestable spinlock that does not block the CPU (passive) and have almost no creation cost (unlike Windows Semaphores)
class PLATFORM_NOVTABLE PassiveSpinlock
{
    spinlock_t ptr;
public:
    FORCEINLINE PassiveSpinlock()
    {
        spinlock_init(&ptr);
    }
    
    FORCEINLINE bool Lock()
    {
        spinlock_passive_lock(&ptr);
        return true;
    }

    FORCEINLINE bool Unlock()
    {
        spinlock_unlock(&ptr);
        return true;
    }
};

template <int N = 1>
class PLATFORM_NOVTABLE ActiveSemaphores
{
   spinlock_t ptr[N];
public:
    FORCEINLINE ActiveSemaphores()
    {
        for (int i = 0; i < N; i++)
            ptr[i] = SPINLOCK_TAKEN;
    }

    FORCEINLINE ~ActiveSemaphores()
    {
    }

    FORCEINLINE int Wait(uint32_t timeOut)
    {
        if (timeOut == 0)
        {
            for (int i = 0; i < N; i++)
            {
                if (*ptr) if (*ptr) if (*ptr) if (*ptr) _mm_pause();
                if (spinlock_trylock(&ptr[i]))
                    return i;
            }
        }
        else
        {
            for (int trial = 0; ; trial++)
            {
                for (int i = 0; i < N; i++)
                {
                    if (*ptr) if (*ptr) if (*ptr) if (*ptr) _mm_pause();
                    if (spinlock_trylock(&ptr[i]))
                        return i;
                }

                //if ((trial & 127) == 0) light_sleep();

                if (timeOut == 0)
                    break;
            }
        }

        return -1;
    }
    
    FORCEINLINE int Wait()
    {
        return Wait(~0);
    }

    template <int I>
    FORCEINLINE void Signal()
    {
        spinlock_unlock(&ptr[I]);
    }

    FORCEINLINE void Signal()
    {
        Signal<0>();
    }
};

typedef ActiveSemaphores<1> ActiveSemaphore;

template <typename T1, typename T2, unsigned SPINCOUNT = 64>
class WaitOne
{
    T1 & m_t1;
    T2 & m_t2;

    FORCEINLINE int InternalWait()
    {
        if (m_t1.Wait(0) >= 0) return 0;
        if (m_t2.Wait(0) >= 0) return 1;
        return -1;
    }

public:
    FORCEINLINE WaitOne(T1 & t1, T2 & t2)
    : m_t1(t1), m_t2(t2)
    {
    }

    FORCEINLINE int Wait(uint32_t timeOut)
    {
        if (timeOut == 0)
        {
            return InternalWait();
        }
        for (int trial = 0; ; trial++)
        {
            int res = InternalWait();
            if (res != -1)
            {
                return res;
            }
            if (trial >= SPINCOUNT)
            {
                trial = 0;
                light_sleep();
            }
        }
        return -1;
    }
};

template <typename T1, typename T2>
WaitOne<T1, T2> Make_WaitOne(T1 & t1, T2 & t2)
{
    return WaitOne<T1, T2>(t1, t2);
}

template <typename LOCK_TYPE>
class PLATFORM_NOVTABLE ScopeLock
{
    LOCK_TYPE & lock;
public:
    FORCEINLINE ScopeLock(LOCK_TYPE & lock)
        : lock(lock)
    {
        lock.Lock();
    }

    FORCEINLINE  ~ScopeLock()
    {
        lock.Unlock();
    }
};

#define PXATOMIC_SCOPELOCK(Type, Lock) volatile ScopeLock<Type> PXAtomic_ScopeLock_Instance_##__COUNTER__(Lock);

template <int LOG2SIZE, typename TYPE>
class AtomicStack
{
    enum { SIZE = 1 << LOG2SIZE };
    int32_t  m_size;
    TYPE m_items[SIZE];

public:
    FORCEINLINE AtomicStack()
    {
        m_size = 0;
    }

    FORCEINLINE bool Available() const
    {
        return atomic_compare_exchange((int32_t *)&m_size, 0, 0) != 0;
    }

    FORCEINLINE void Push(TYPE const & tItem)
     {
        m_items[atomic_inc(&m_size)] = tItem;
    }

    FORCEINLINE TYPE Pop()
    {
        TYPE tResult;
        tResult = m_items[atomic_dec(&m_size)];
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

namespace Philips
{
    namespace Medisys
    {
        namespace PXAtomic = ::PXAtomicImpl;
    }
}


#endif
