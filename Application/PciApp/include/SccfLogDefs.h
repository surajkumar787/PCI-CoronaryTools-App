///////////////////////////////////////////////////////////////////////////////
// This is the  header file. Specific items can be         // 
// enabled by adding the *_INCLUDED macros to the preprocessor definitions   //
//                                     //
///////////////////////////////////////////////////////////////////////////////


#ifndef _M_CEE // Not C++/CLI

///////////////////////////////////////////////////////////////////////////////
// Externals

# ifdef SSCF1_INCLUDED
#  include <sscfts1.h>
# endif


// AppEngine Tracing
# ifdef PHILIPS_TRACE_INCLUDED
#include <Trace.h>
#include "TraceWriter.h"


// These additional defines are added because they are missing in the latest 1.3 nuget package.
// When these macros are added to the Philips.Trace package these can be removed
#ifndef TraceMessageEx
# define TraceMessageEx(filename, lineNumber) TraceInternals::Message(fileName, lineNumber, "Info" )
#endif

#ifndef TraceAssert
#  include <assert.h>
#ifdef NDEBUG
# pragma warning (disable : 4127) // Conditional expression is constant.
# define TraceAssert(COND) \
    do { if (!(COND)) TraceError() << __FUNCTION__ << " : " << #COND; } while(false)
#else
# pragma warning (disable : 4127) // Conditional expression is constant.
# define TraceAssert(COND) \
    do { if (!(COND)) TraceError() << __FUNCTION__ << " : " << #COND; assert(COND); } while (false)
#endif
#endif

# endif

#endif // _M_CEE

#pragma once
