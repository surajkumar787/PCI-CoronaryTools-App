/*!
* \file OSWrapper.h
*
* \author
* Emmanuel Attia (emmanuel.attia@philips.com)
* The REACT Project - MedISys Research Lab - Philips HealthCare
*
* \brief
* Compact and usefull OS wrapper. If you want to do simple system calls without being dependant on Windows ...
*
* Assumes that if _WIN32 is defined we are on a Windows system otherwise it is considered as POSIX.
* \remarks MSDN Library says that any Windows compiler should define _WIN32 even in 64 bit mode (in that case _WIN64 is also defined).
*
* \date 04 september 2008: Initial header
*
*/

#ifndef _OSWRAPPER_H_
#define _OSWRAPPER_H_

#ifdef _WIN32

#define _WIN32_WINNT 0x0501
#include <windows.h>
#include <winnt.h>
#include <tchar.h>
#include <stdio.h>

#undef min
#undef max

typedef HMODULE OSW_ModulePtr;
typedef void *  OSW_ProcPtr;

inline void OSW_Sleep(double dbl)
{
    if (dbl == 0)
        SwitchToThread();
    else
        Sleep((DWORD)dbl*1000);
}

inline double OSW_GetPreciseTimer()
{
    static LARGE_INTEGER lFreq;
    static bool bFirstCall = true;

    LARGE_INTEGER lTime;

    if (bFirstCall)
        QueryPerformanceFrequency(&lFreq);
    QueryPerformanceCounter(&lTime);

    return lTime.QuadPart / (double)lFreq.QuadPart;
}

namespace __private_unnamedsymbol_220920091546
{
    static void dummy_symbol_function()
    {
    }

    inline int snprintf(char * f, size_t count, char const * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int s = ::vsnprintf_s(f, count, count, fmt, args);
        va_end(args);
        return s;
    }

    inline int snprintf(wchar_t * f, size_t count, wchar_t const * fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        int s = ::vswprintf_s(f, count, fmt, args);
        va_end(args);
        return s;
    }

    inline int strnicmp(char const * s1, char const * s2, size_t count)
    {
        return ::_strnicmp(s1, s2, count);
    }

    inline int strnicmp(wchar_t const * s1, wchar_t const * s2, size_t count)
    {
        return ::_wcsnicmp(s1, s2, count);
    }

    inline bool strncpy(char * s1, size_t dest_count, char const * s2, size_t count)
    {
        return (::strncpy_s(s1, dest_count, s2, count) == 0);
    }

    inline bool strncpy(wchar_t * s1, size_t dest_count, wchar_t const * s2, size_t count)
    {
        return (::wcsncpy_s(s1, dest_count, s2, count) == 0);
    }

    inline char const * strrchr(char const * s1, int c)
    {
        return ::strrchr(s1, c);
    }

    inline wchar_t const * strrchr(wchar_t const * s1, int c)
    {
        return ::wcsrchr(s1, c);
    }

    inline size_t strlen(char const * s1)
    {
        return ::lstrlenA(s1);
    }

    inline size_t strlen(wchar_t const * s1)
    {
        return ::lstrlenW(s1);
    }

    inline bool IsValidLibraryFileHandle(HANDLE hFile)
    {
        if (hFile == INVALID_HANDLE_VALUE)
            return false;

        IMAGE_DOS_HEADER image_dos_header;
        DWORD dwRead;

        ReadFile(hFile, &image_dos_header, sizeof(IMAGE_DOS_HEADER), &dwRead, NULL);

        if (dwRead != sizeof(IMAGE_DOS_HEADER))
            return false;

        if (image_dos_header.e_magic != IMAGE_DOS_SIGNATURE)
            return false;

        IMAGE_NT_HEADERS image_nt_header;

        SetFilePointer(hFile, image_dos_header.e_lfanew, NULL, FILE_BEGIN);
        ReadFile(hFile, &image_nt_header, sizeof(IMAGE_NT_HEADERS), &dwRead, NULL);

        if (dwRead != sizeof(IMAGE_NT_HEADERS))
            return false;

        if (image_nt_header.Signature != IMAGE_NT_SIGNATURE)
            return false;


		if (sizeof(void *) == sizeof(__int32))
		{
			if (image_nt_header.FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
				return false;
		}
        else if (sizeof(void *) == sizeof(__int64))
		{
			if (image_nt_header.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64)
				return false;
		}
		else
		{
			__debugbreak();
		}

        return true;
    }

    inline HANDLE LoadLibrary(char const * s, bool check)
    {
        HANDLE hResult = NULL;
        if (check)
        {
            HANDLE hFile = CreateFileA(s, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                if (IsValidLibraryFileHandle(hFile))
                    hResult = (HANDLE)1;
                CloseHandle(hFile);
            }
        }
        else
        {
            hResult = ::LoadLibraryA(s);
        }
        return hResult;
    }

    inline HANDLE LoadLibrary(wchar_t const * s, bool check)
    {
        HANDLE hResult = NULL;
        if (check)
        {
            HANDLE hFile = CreateFileW(s, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                if (IsValidLibraryFileHandle(hFile))
                    hResult = (HANDLE)1;
                CloseHandle(hFile);
            }
        }
        else
        {
            hResult = ::LoadLibraryW(s);
        }
        return hResult;
    }

    template <typename CHAR_TYPE>
    class Str;

    template <>
    class Str<char>
    {
    public:
        char const * DLLEXT;
        char const * DLLEXT_FORMAT;

        Str()
        {
            DLLEXT = ".dll";
            DLLEXT_FORMAT = "%s.dll";
        }
    };

    template <>
    class Str<wchar_t>
    {
    public:
        wchar_t const * DLLEXT;
        wchar_t const * DLLEXT_FORMAT;

        Str()
        {
            DLLEXT = L".dll";
            DLLEXT_FORMAT = L"%s.dll";
        }
    };
};

#define OSW_Private __private_unnamedsymbol_220920091546

static void OSW_private_dummy_symbol_function()
{
}

inline void OSW_GetCurrentModuleFileName(size_t sLength, char * szBuffer)
{
    HMODULE hModule;
    GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (char *)&OSW_Private::dummy_symbol_function, &hModule);
    GetModuleFileNameA(hModule, szBuffer, (DWORD)sLength);
}

inline void OSW_GetCurrentModuleFileName(size_t sLength, wchar_t * szBuffer)
{
    HMODULE hModule;
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
        | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (wchar_t *)&OSW_Private::dummy_symbol_function, &hModule);
    GetModuleFileNameW(hModule, szBuffer, (DWORD)sLength);
}

inline void OSW_GetCurrentDirectory(size_t sLength, char * szBuffer)
{
    GetCurrentDirectoryA((DWORD)sLength, szBuffer);
}

inline void OSW_GetCurrentDirectory(size_t sLength, wchar_t * szBuffer)
{
    GetCurrentDirectoryW(static_cast<DWORD>(sLength), szBuffer);
}

inline bool OSW_SetCurrentDirectory(char const * szBuffer)
{
    return (SetCurrentDirectoryA(szBuffer) != FALSE);
}

inline bool OSW_SetCurrentDirectory(wchar_t const * szBuffer)
{
    return (SetCurrentDirectoryW(szBuffer) != FALSE);
}

inline void OSW_GetLastErrorString(size_t sLength, char * szBuffer)
{
    DWORD dwLastError = GetLastError();
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, szBuffer, (DWORD)sLength, NULL) == 0)
    {
        lstrcpynA(szBuffer, "Unknown error.", (int)sLength);
    }
}

inline void OSW_GetLastErrorString(size_t sLength, wchar_t * szBuffer)
{
    DWORD dwLastError = GetLastError();
    if (FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, szBuffer, (DWORD)sLength, NULL) == 0)
    {
        lstrcpynW(szBuffer, L"Unknown error.", (int)sLength);
    }
}

// if bCheck is true, we just parse the dll file to see if it match the running architecture (faster debugging)
template <typename CHAR_TYPE>
inline OSW_ModulePtr OSW_LoadModule(const CHAR_TYPE * pszFileName, bool bCheck)
{
    bool bHasExtension = false;
    size_t sLen = OSW_Private::strlen(pszFileName);
    if (sLen > 4)
        bHasExtension = (OSW_Private::strnicmp(pszFileName + sLen - 4, OSW_Private::Str<CHAR_TYPE>().DLLEXT, 4) == 0);

    if (!bHasExtension)
    {
        CHAR_TYPE * pszFileNameWithExtension = new CHAR_TYPE[sLen+5];
        OSW_Private::snprintf(pszFileNameWithExtension, sLen+5, OSW_Private::Str<CHAR_TYPE>().DLLEXT_FORMAT, pszFileName);
        pszFileName = pszFileNameWithExtension;
    }

    OSW_ModulePtr result = NULL;

    CHAR_TYPE const * pszSlash = __max(OSW_Private::strrchr(pszFileName, '/'), OSW_Private::strrchr(pszFileName, '\\'));
    CHAR_TYPE * pszDirectoryName;
    CHAR_TYPE * pszFileNameWithoutDirectory;

    // If the directory is specified, we change directory in order to load correctly all the dependants modules.
    if (pszSlash != NULL)
    {
        size_t sLen = OSW_Private::strlen(pszFileName);
        size_t sLenDirecory = pszSlash - pszFileName;
        size_t sLenFileName = sLen - (pszSlash - pszFileName) - 1;

        pszDirectoryName = new CHAR_TYPE[sLenDirecory + 1];
        OSW_Private::strncpy(pszDirectoryName, sLenDirecory + 1, pszFileName, sLenDirecory);
        pszDirectoryName[sLenDirecory] = 0;
        pszFileNameWithoutDirectory = new CHAR_TYPE[sLenFileName + 1];
        OSW_Private::strncpy(pszFileNameWithoutDirectory, sLenFileName + 1, pszSlash + 1, sLenFileName);
        pszFileNameWithoutDirectory[sLenFileName] = 0;

        CHAR_TYPE * pszOldDirectory = new CHAR_TYPE[MAX_PATH];
        OSW_GetCurrentDirectory(MAX_PATH, pszOldDirectory);
        OSW_SetCurrentDirectory(pszDirectoryName);
        result = static_cast<OSW_ModulePtr>(OSW_Private::LoadLibrary(pszFileNameWithoutDirectory, bCheck));
        OSW_SetCurrentDirectory(pszOldDirectory);

        delete [] pszOldDirectory;
        delete [] pszDirectoryName;
        delete [] pszFileNameWithoutDirectory;
    }
    else
    {
        result = static_cast<OSW_ModulePtr>(OSW_Private::LoadLibrary(pszFileName, bCheck));
    }

    if (!bHasExtension)
    {
        delete [] pszFileName;
    }

    return result;
}

inline bool OSW_FreeModule(OSW_ModulePtr ptr)
{
    if (ptr == (OSW_ModulePtr)1)
        return true;
    return (FreeLibrary(static_cast<HMODULE>(ptr)) != FALSE);
}

inline OSW_ProcPtr OSW_AddrOfProc(OSW_ModulePtr ptr, const char * pszProcName)
{
    return reinterpret_cast<OSW_ProcPtr>(GetProcAddress(static_cast<HMODULE>(ptr), pszProcName));
}

class OSW_SemaphoreArray
{
    HANDLE * m_phSemaphores;
    unsigned long m_ulNumberOfSemaphore;
public:
    static FORCEINLINE OSW_SemaphoreArray * Create(unsigned long ulNumberOfSemaphore, long lInitialCount = 1, long lMaximumCount = 1)
    {
        OSW_SemaphoreArray * pResult = new OSW_SemaphoreArray();
        pResult->m_phSemaphores = new HANDLE[ulNumberOfSemaphore];
        pResult->m_ulNumberOfSemaphore = ulNumberOfSemaphore;

        for (unsigned long ulIndex = 0; ulIndex != ulNumberOfSemaphore; ulIndex++)
            pResult->m_phSemaphores[ulIndex] = CreateSemaphore(NULL, lInitialCount, lMaximumCount, NULL);
        return pResult;
    }

    template <typename ACCESSOR>
    static FORCEINLINE OSW_SemaphoreArray * CreateEx(unsigned long ulNumberOfSemaphore, ACCESSOR const & aInitialCount, ACCESSOR const & aMaximumCount)
    {
        OSW_SemaphoreArray * pResult = new OSW_SemaphoreArray();
        pResult->m_phSemaphores = new HANDLE[ulNumberOfSemaphore];
        pResult->m_ulNumberOfSemaphore = ulNumberOfSemaphore;

        for (unsigned long ulIndex = 0; ulIndex != ulNumberOfSemaphore; ulIndex++)
            pResult->m_phSemaphores[ulIndex] = CreateSemaphore(NULL, aInitialCount[ulIndex], aMaximumCount[ulIndex], NULL);

        return pResult;
    }

    FORCEINLINE ~OSW_SemaphoreArray()
    {
        for (unsigned long ulIndex = 0; ulIndex != m_ulNumberOfSemaphore; ulIndex++)
            CloseHandle(m_phSemaphores[ulIndex]);

        delete [] m_phSemaphores;
    }

    enum { Infinite = INFINITE, TimeOut = INFINITE };

    FORCEINLINE bool Release(unsigned long ulIndex, long lReleaseCount = 1, long * lPreviousCount = NULL)
    {
        return (ReleaseSemaphore(m_phSemaphores[ulIndex], lReleaseCount, lPreviousCount) != FALSE);
    }

    FORCEINLINE void ReleaseAll(long lReleaseCount = 1)
    {
        for (unsigned long ulIndex = 0; ulIndex != m_ulNumberOfSemaphore; ulIndex++)
            ReleaseSemaphore(m_phSemaphores[ulIndex], lReleaseCount, NULL);
    }

    FORCEINLINE bool WaitForOne(unsigned long ulIndex, unsigned long ulTimeOut = Infinite)
    {
        return (WaitForSingleObject(m_phSemaphores[ulIndex], ulTimeOut) == WAIT_OBJECT_0);
    }

    FORCEINLINE bool WaitForAll(unsigned long ulTimeOut = Infinite)
    {
        for (unsigned long ulIndex = 0; ulIndex < m_ulNumberOfSemaphore; ulIndex += MAXIMUM_WAIT_OBJECTS)
        {
            DWORD ulToWait = __min(MAXIMUM_WAIT_OBJECTS, m_ulNumberOfSemaphore - ulIndex);
            if (WaitForMultipleObjects(ulToWait, m_phSemaphores + ulIndex, TRUE, ulTimeOut) != WAIT_OBJECT_0)
                return false;
        }
        return true;
    }

    FORCEINLINE bool WaitForAny(unsigned long & ulIndex, unsigned long ulTimeOut = Infinite)
    {
        if (m_ulNumberOfSemaphore <= MAXIMUM_WAIT_OBJECTS)
        {
            DWORD dwResult = WaitForMultipleObjects(m_ulNumberOfSemaphore, m_phSemaphores, FALSE, ulTimeOut);
            if (dwResult == WAIT_TIMEOUT)
            {
                ulIndex = ~0;
                return false;
            }
            ulIndex = (dwResult - WAIT_OBJECT_0);
            return true;
        }
        else
        {
            bool bTick = (ulTimeOut != 0) && (ulTimeOut != Infinite);

            DWORD dw = bTick ? GetTickCount() : 0;
            unsigned long ulOffset = 0;

            for (;;)
            {
                DWORD ulToWait = MAXIMUM_WAIT_OBJECTS;
                if (m_ulNumberOfSemaphore < ulOffset + MAXIMUM_WAIT_OBJECTS)
                {
                    ulToWait = m_ulNumberOfSemaphore - ulOffset;
                }

                DWORD dwResult = WaitForMultipleObjects(ulToWait, m_phSemaphores + ulOffset, FALSE, 0);

                if (dwResult != WAIT_TIMEOUT)
                {
                    ulIndex = (dwResult - WAIT_OBJECT_0) + ulOffset;
                    return true;
                }
                if (m_ulNumberOfSemaphore > ulOffset + MAXIMUM_WAIT_OBJECTS)
                {
                    ulOffset = 0;
                }

                DWORD dw2 = bTick ? GetTickCount() : 0;

                if (ulTimeOut == 0)
                    break;
                if (ulTimeOut == Infinite)
                    continue;
                if ((dw2 - dw) >= ulTimeOut)
                    break;

                ulOffset += MAXIMUM_WAIT_OBJECTS;
            }

            return false;
        }
    }
};

class OSW_DirectoryReader
{
    bool m_bIsUnicode;
    union
    {
        WIN32_FIND_DATAA ansi;
        WIN32_FIND_DATAW uni;
    } wfd;
    HANDLE hFind;
public:
    OSW_DirectoryReader(const char * pszDirectory)
        : m_bIsUnicode(false)
    {
        size_t sLen = strlen(pszDirectory) + 5;
        char * pszMask = new char[sLen];
        wsprintfA(pszMask, "%s*.*", pszDirectory);
        hFind = FindFirstFileA(pszMask, &wfd.ansi);
        delete [] pszMask;
        if (hFind == INVALID_HANDLE_VALUE)
        {
            hFind = NULL;
        }
    }

    OSW_DirectoryReader(const wchar_t * pszDirectory)
        : m_bIsUnicode(true)
    {
        size_t sLen = wcslen(pszDirectory) + 5;
        wchar_t * pszMask = new wchar_t[sLen];
        wsprintfW(pszMask, L"%s*.*", pszDirectory);
        hFind = FindFirstFileW(pszMask, &wfd.uni);
        delete [] pszMask;
        if (hFind == INVALID_HANDLE_VALUE)
        {
            hFind = NULL;
        }
    }

    ~OSW_DirectoryReader()
    {
        if (hFind != NULL)
        {
            FindClose(hFind);
        }
    }

    void Next()
    {
        if (m_bIsUnicode)
        {
            if (FindNextFileW(hFind, &wfd.uni) == FALSE)
            {
                FindClose(hFind);
                hFind = NULL;
            }
        }
        else
        {
            if (FindNextFileA(hFind, &wfd.ansi) == FALSE)
            {
                FindClose(hFind);
                hFind = NULL;
            }
        }
    }

    bool Available() const
    {
        return (hFind != NULL);
    }

    void GetFileName(char const *& pszFileName) const
    {
        if (!m_bIsUnicode)
            pszFileName = wfd.ansi.cFileName;
        else
            pszFileName = NULL;
    }

    void GetFileName(wchar_t const *& pszFileName) const
    {
        if (m_bIsUnicode)
            pszFileName = wfd.uni.cFileName;
        else
            pszFileName = NULL;
    }
};

#undef OSW_Private

#else

typedef char TCHAR;
#define _tprintf    printf
#define _tcschr		strchr
#define _tcscpy		strcpy
#define _tcsncmp	strncmp
#define _tcscmp		strcmp
#define _tfopen		fopen
#define _tcsicmp	strcasecmp
#define _tcslen		strlen
#define _tcsncpy	strncpy
#define _fgetts		fgets
#define _tgetenv	getenv
#define _stprintf	sprintf
#define TEXT(x)		x
#define MAX_PATH	260

#include <dlfcn.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <dirent.h>

typedef void * OSW_ModulePtr;
typedef void * OSW_ProcPtr;

inline double OSW_GetPreciseTimer()
{
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (double)now.tv_sec + now.tv_nsec / 1000000000.;
}

inline void OSW_GetCurrentDirectory(size_t sLength, TCHAR * szBuffer)
{
    getcwd(szBuffer, sLength);
}

inline OSW_ModulePtr OSW_LoadModule(const TCHAR * pszFileName)
{
    bool bHasExtension = false;
    size_t sLen = _tcslen(pszFileName);
    if (sLen > 3)
        bHasExtension = (_tcsncmp(pszFileName + sLen - 3, TEXT(".so"), 3) == 0);

    if (!bHasExtension)
    {
        TCHAR * pszFileNameWithExtension = new TCHAR[sLen+4];
        _stprintf(pszFileNameWithExtension, TEXT("%s.so"), pszFileName);
        pszFileName = pszFileNameWithExtension;
    }

    OSW_ModulePtr result = static_cast<OSW_ModulePtr>(dlopen(pszFileName, RTLD_NOW));

    if (!bHasExtension)
    {
        delete [] pszFileName;
    }

    return result;
}

inline bool OSW_FreeModule(OSW_ModulePtr ptr)
{
    return (dlclose(static_cast<void *>(ptr)) == 0);
}

inline OSW_ProcPtr OSW_AddrOfProc(OSW_ModulePtr ptr, const char * pszProcName)
{
    return reinterpret_cast<OSW_ProcPtr>(dlsym(static_cast<void *>(ptr), pszProcName));
}

class OSW_DirectoryReader
{
    DIR * dir;
    struct dirent * entry;
public:
    OSW_DirectoryReader(const TCHAR * pszDirectory)
    {
        dir = opendir(pszDirectory);

        if (dir != NULL)
        {
            Next();
        }
    }

    ~OSW_DirectoryReader()
    {
        if (dir != NULL)
        {
            closedir(dir);
        }
    }

    void Next()
    {
        entry = readdir(dir);
        if (entry == NULL)
        {
            closedir(dir);
            dir = NULL;
        }
    }

    bool Available() const
    {
        return (dir != NULL);
    }

    const TCHAR * FileName() const
    {
        return entry->d_name;
    }

    void GetFileName(TCHAR const * pszFileName) const
    {
        pszFileName = entry->d_name;
    }
};

#endif


#endif /* _OSWRAPPER_H_ */
