/*==============================================================================
Copyright(c) 2017 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files(the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and / or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
============================================================================*/

/*
File Name:  AssertTracer.h

Abstract:
    These functions enables reporting asserts to system log in the debug
    driver build.

Notes:

\*****************************************************************************/
#ifndef _ASSERT_TRACER_H_
#define _ASSERT_TRACER_H_

#if defined( _WIN32 ) && (defined( _DEBUG ) || defined(_RELEASE_INTERNAL))

#if !defined( __GMM_KMD__ ) && !defined( STATIC_DRIVER_MODEL ) && !defined( LHDM )
#include <windows.h>
// Windows.h defines MemoryFence as _mm_mfence, but this conflicts with llvm::sys::MemoryFence
#undef MemoryFence
#include <stdio.h>
#include <stdlib.h>
#endif
#undef REPORT_ASSERT_MSG
#define REPORT_ASSERT_MSG( expr, msg )                                            \
    if ( !( expr ) )                                                              \
    {                                                                             \
        ReportAssert( #expr, __FILE__, __FUNCTION__, __LINE__, #msg );            \
    }
#undef REPORT_ASSERT
#define REPORT_ASSERT( expr )                                                     \
    if ( !( expr ) )                                                              \
    {                                                                             \
        ReportAssert( #expr, __FILE__, __FUNCTION__, __LINE__, "" );              \
    }

#undef REPORT_ASSERT_MSG_ETW
#define REPORT_ASSERT_MSG_ETW( compId, compMsk, expr, msg )                       \
    if (!(expr))                                                                  \
    {                                                                             \
        ReportAssertETW( compId, compMsk, #expr, __FILE__, __FUNCTION__, __LINE__, #msg ); \
    }
#undef REPORT_ASSERT_ETW
#define REPORT_ASSERT_ETW( compId, compMsk, expr )                                \
    if (!(expr))                                                                  \
    {                                                                             \
        ReportAssertETW( compId, compMsk, #expr, __FILE__, __FUNCTION__, __LINE__, "" );  \
    }

#ifdef __cplusplus
extern "C"
    {
#endif
void __stdcall ReportAssert( const char *expr,
                   const char *file,
                   const char *func,
                   const unsigned long line,
                   const char *msg );

void __stdcall ReportAssertETW(const unsigned short compId,
                    const unsigned long compMsk,
                    const char *expr,
                    const char *file,
                    const char *func,
                    const unsigned long line,
                    const char *msg );


#ifdef __cplusplus
    }
#endif

#elif defined( __linux__ ) && defined( _RELEASE_INTERNAL ) && !defined( __ANDROID__ )
// do while() is missing ";" at the end and this is intentional
// As invoking assert looks like this: assert(expr); So semicolon will
// be stuck to do.. while() and that way sorting out possible
// problems when assert is used as block in one liner conditions
#define REPORT_ASSERT( expr )                                          \
    do {                                                               \
        if( !( expr ) )                                                \
        {                                                              \
            LogAssertion( __FUNCTION__, __FILE__, __LINE__, #expr );   \
        }                                                              \
    } while( 0 )
#define REPORT_ASSERT_MSG( expr, msg ) REPORT_ASSERT( expr )
#define REPORT_ASSERT_ETW( CompId, compMsk, expr)
void LogAssertion( const char *function_name, const char *file_name, unsigned int line_number, const char *expr );
#else
#define REPORT_ASSERT_MSG( expr, msg )
#define REPORT_ASSERT( expr )
#define REPORT_ASSERT_MSG_ETW( compMsk, expr, msg )
#define REPORT_ASSERT_ETW( CompId, compMsk, expr )
#endif // defined( _WIN32 ) && defined( _DEBUG )
#endif  //_ASSERT_TRACER_H_
