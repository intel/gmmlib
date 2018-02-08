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

File Name:  AssertTracer.cpp

Abstract:
    These functions enables reporting asserts to system log in the debug
    driver build.

Notes:

\*****************************************************************************/

#if defined( _WIN32 ) && (defined( _DEBUG ) || defined( _RELEASE_INTERNAL ))
#include "AssertTracer.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

// Windows.h defines MemoryFence as _mm_mfence, but this conflicts with llvm::sys::MemoryFence
#undef MemoryFence
#include <stdio.h>
#include <stdlib.h>

#if defined( __GMM_KMD__ ) || defined( STATIC_DRIVER_MODEL )
#include"BufferedDbg.h"
#include "igdKrnlEtwMacros.h"
#endif //__GMM_KMD__ || STATIC_DRIVER_MODEL

#if _DEBUG
/*****************************************************************************\

Function:
    ReportAsserts

Description:
    Sends message to the system log.

Input:
    const char *expr -
        The expression passed to function.
    const char *file -
        The name of the file that is the origin of the expression.
    const char *func -
        The function from which call to this function was made.
    const unsigned long line -
        The line number from file, which caused this function call.
    const char *msg -
        Message passed to the system log.
Output:
    void - None.

\*****************************************************************************/
void __stdcall ReportAssert(
    const char              *expr,
    const char              *file,
    const char              *func,
    const unsigned long      line,
    const char              *msg )
{
#if !defined( __GMM_KMD__ ) && !defined( STATIC_DRIVER_MODEL )//We are in UMD, use the UMD implementation

#if 0

    HANDLE hEventLog = RegisterEventSourceA( NULL, "GFX Driver AssertTracer" );
    char *wideMessage;
    if ( hEventLog != NULL )
    {
        // Calculate length with hard coded max unsigned long length plus some safe space.
        size_t length = strlen( expr ) + strlen( file ) + strlen( func ) + strlen( msg ) + 15;

        // Checks against maximum string size for ReportEvent lpStrings parameter.
        // Cuts string to the limit size.
        uint32_t maxSizeReached = 0;
        if ( length > 31839 )
        {
            length = 31839;
            maxSizeReached = 1;
        }

        wideMessage = ( char * ) malloc( sizeof( char ) * length );
        if( wideMessage != NULL )
        {
            // snprintf spec: "The resulting character string will be terminated with a null character"
            _snprintf_s( wideMessage, length, length / sizeof( char ), "%s:%lu\n%s\n%s\n%s", file, line, expr, func, msg);

            ReportEventA( hEventLog, EVENTLOG_INFORMATION_TYPE, 0, 0, NULL, 1, 0, ( LPCSTR* ) &wideMessage, NULL );
            free( wideMessage );
        }

        DeregisterEventSource( hEventLog );
    }

#endif //!define( WINDOWS_MOBILE )

    // Windows Mobile has no implementation. Reference the parameters to remove the unreference param warnings.
    (void)expr;
    (void)file;
    (void)func;
    (void)line;
    (void)msg;

#else //We are in KMD, use the KMD implementation
    BufDbgPrint("ASSERT_TRACE: %s::%s:%i : %s : %s\n", file, func, line, expr, msg);
#endif //!defined( __GMM_KMD__ ) && !defined( STATIC_DRIVER_MODEL )
}
#endif

/*****************************************************************************\

Function:
    ReportAssertsETW

Description:
    Sends message to the system log.

Input:
    const unsigned long ComponentMask
        Contains the component id for which raised assert (KMD, MINIPORT..)
    const char *expr -
        The expression passed to function.
    const char *file -
        The name of the file that is the origin of the expression.
    const char *func -
        The function from which call to this function was made.
    const unsigned long line -
        The line number from file, which caused this function call.
    const char *msg -
        Message passed to the system log.
Output:
    void - None.

\*****************************************************************************/

void __stdcall ReportAssertETW( const unsigned short     compId,
                                const unsigned long      compMsk,
                                const char              *expr,
                                const char              *file,
                                const char              *func,
                                const unsigned long     line,
                                const char              *msg)
{
#if !defined( __GMM_KMD__ ) && !defined( STATIC_DRIVER_MODEL ) //We are in UMD, use the UMD implementation
    // TODO: Add UMD code for ETW here.
    // Reference the parameters to remove the unreference param warnings.
    (void)compId;
    (void)compMsk;
    (void)expr;
    (void)file;
    (void)func;
    (void)line;
    (void)msg;
#else //We are in KMD, use the KMD implementation
    // Log event if ETW session is active
    if (g_ulHDGraphics_SessionActive)
    {
        EtwAssertPrint(compId, compMsk, expr, file, func, line, msg);
    }
#endif
}

#elif defined( __linux__ ) && defined( _RELEASE_INTERNAL ) && !defined( __ANDROID__ )
#include <algorithm>
#include <syslog.h>
#include <execinfo.h>
#include <string>
#include <cxxabi.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <fstream>
#define CALL_STACK_REPORT_SIZE 50  // maximal limit of call stack to be reported in case of assertion
#define MAX_FUNCTION_NAME_LENGTH 100 // no worries it can be extended (relocated by driver if needed)
#define MAX_SYSLOG_ENTRY_LENGTH 1000 // Syslog protocol recommends minimum entry length 1KB (ubuntu rsyslog message limit is about 2K)
void LogAssertion( const char *function_name, const char *file_name, unsigned int line_number, const char *expr )
{
    std::string  stack;
    std::string  part1;
    std::string  part2;
    std::size_t  pos;
    void         *buffer[CALL_STACK_REPORT_SIZE];
    char         *function_name_buffer = NULL;
    int          nframes;        // numer of frames to be returned
    char         **strings;
    int          i;
    int          status;
    size_t       length;
    const size_t stringLength = 4096;
    std::string  oclAppCmdLine;

    nframes = backtrace( buffer, CALL_STACK_REPORT_SIZE );

    strings = backtrace_symbols( buffer, nframes );
    if( strings == NULL )
    {
        perror( "Getting backtrace symbols error:" );
        nframes = 0;
    }
    else
    {
        // Get Commandline of process (in that case OCL app)
        // from process info itself eg. /proc/self/cmdline

        oclAppCmdLine.reserve( stringLength );
        std::ifstream fileCmdline( "/proc/self/cmdline", std::ifstream::binary );
        if( fileCmdline.is_open() )
        {
            oclAppCmdLine = std::string( std::istreambuf_iterator< char >( fileCmdline ), std::istreambuf_iterator< char >() );
            if( oclAppCmdLine.size() > 0 )
            {
                // Trim last \0 character
                oclAppCmdLine.resize( oclAppCmdLine.size() - 1 );
                // Format of /proc/self/cmdline is that args are separated with '\0'
                // so for nicer printing we replace zeros with spaces (without terminating 0)
                std::replace( oclAppCmdLine.begin(), oclAppCmdLine.end(), '\0', ' ' );
            }
            else
            {
                fprintf( stderr, "Getting Commandline of OCL app error: Error reading /proc/self/cmdline\n" );
            }
        }
        else
        {
            fprintf( stderr, "Getting Commandline of OCL app error:" );
        }
    }

    // allocation by malloc is suggessted by documentation as abi function may do some relocation
    function_name_buffer = ( char * )malloc( MAX_FUNCTION_NAME_LENGTH );
    if( function_name_buffer == NULL )
    {
        // Not enough memory to get small allocation then do not print stack
        nframes = 0;
    }
    else
    {
        length = MAX_FUNCTION_NAME_LENGTH;
        memset( function_name_buffer, 0, length );
    }

    for( i = 0; i < nframes; ++i )
    {
        // Generate signature of given stack frame eg. #0 #1 etc...
        std::stringstream framePromptBuilder;
        framePromptBuilder << "#" << i << " ";
        const std::string &framePrompt = framePromptBuilder.str();
        // demangle name eg. split stack frame into two pieces
        part1 = strings[i];
        pos   = part1.find( "(" );
        if( pos != std::string::npos )
        {
            // For final level instead of binary print whole commandline
            // if were able to get one
            if( ( i == nframes - 1 ) && ( !oclAppCmdLine.empty() ) )
            {
                //..call stack's part1 contains "(" so we add it here manualy
                stack.append( ( oclAppCmdLine.insert( 0, "COMMANDLINE(" ) ).c_str() );
            }
            else
            {
                // part1 contains everything before section to be demangled
                part1 = part1.substr( 0, pos + 1 );
                stack.append(framePrompt);
                stack.append( part1 );
            }
            // part2 contains string to be demangled
            part2 = strings[i];
            part2 = part2.substr( pos + 1 );
            pos   = part2.find( "+" );
            // Final level may not have any function (static functions are not exposed)
            if( pos != std::string::npos )
            {
                part2                = part2.substr( 0, pos );
                function_name_buffer = abi::__cxa_demangle( part2.c_str(), function_name_buffer, &length, &status );
                // in case of error during demangling attach mangled name
                if( status != 0 )
                {
                    stack.append( part2 );
                }
                else
                {
                    stack.append( function_name_buffer );
                }
                stack.append( "())" );
                free( function_name_buffer );
                function_name_buffer = NULL;

            }
            else
            {
                // if there was no function then attach ")"
                stack.append( ")" );
            }
        }
        else
        {
            if( ( i == nframes - 1 ) && ( !oclAppCmdLine.empty() ) )
            {
                stack.append( ( oclAppCmdLine.insert( 0, "COMMANDLINE(" ) ).c_str() );
                stack.append( ")" );
            }
            else
            {
                stack.append(framePrompt);
                stack.append( strings[i] );
            }
        }
        stack.append( " " );
    }
    //Compose full message..
    std::string fullMsg = "File: ";
    std::string syslogEntry;
    fullMsg += file_name;
    fullMsg += " line: ";
    std::stringstream lineStr;
    lineStr << line_number;
    fullMsg += lineStr.str();
    fullMsg += " function: ";
    fullMsg += function_name;
    fullMsg += " expr: ";
    fullMsg += expr;
    fullMsg += " ";
    fullMsg += stack.c_str();

    // split it into chunks we can send
    openlog( "OpenCL", LOG_PID, LOG_USER );
    pos = 0;
    int numberOfChunks = ( fullMsg.length() / MAX_SYSLOG_ENTRY_LENGTH ) + 1;
    while( pos < fullMsg.length() )
    {
        syslogEntry = fullMsg.substr( pos, MAX_SYSLOG_ENTRY_LENGTH );
        // Add chunk ID / part number and send to syslog
        syslog( LOG_MAKEPRI( LOG_USER,
                             LOG_ERR ), "[%zd/%d]%s", ( pos / MAX_SYSLOG_ENTRY_LENGTH + 1 ), numberOfChunks,
                syslogEntry.c_str() );
        pos += MAX_SYSLOG_ENTRY_LENGTH;
    }
    closelog();

    // backtrace_symbols allocates memory in a malloc like way sop it should be freed
    free( strings );
    strings = NULL;
}
#endif //defined( _WIN32 ) && defined( _DEBUG )
