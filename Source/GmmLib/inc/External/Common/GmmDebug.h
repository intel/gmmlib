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
#pragma once

// ------------------------------------------------------------------------
// This block determines where GMM gets definitions for debug assert and
// print functions. Location differs depending who is linking GMM lib.
// ------------------------------------------------------------------------
#if ( __GMM_KMD__ || GMM_OCL)
#include "../../Common/AssertTracer/AssertTracer.h"
#include "../../util/gfxDebug.h"
#else
#include "GmmCommonExt.h"
#include "GmmLog/GmmLog.h"
//===================== Debug Message Levels========================
#define GFXDBG_OFF                      (0x00000000)
#define GFXDBG_CRITICAL                 (0x00000001)
#define GFXDBG_NORMAL                   (0x00000002)
#define GFXDBG_VERBOSE                  (0x00000004)
#define GFXDBG_FUNCTION                 (0x80000000)
#define GFXDBG_NONCRITICAL              (0x00000010)
#define GFXDBG_CRITICAL_DEBUG           (0x00000020)
#define GFXDBG_VERBOSE_VERBOSITY        (0x00000040)
#define GFXDBG_PROTOCAL                 (0x00000100)
#define GFXDBG_FUNCTION_ENTRY           (0x80000000)
#define GFXDBG_FUNCTION_EXIT            (0x80000000)
#define GFXDBG_FUNCTION_ENTRY_VERBOSE   (0x20000000)
#define GFXDBG_FUNCTION_EXIT_VERBOSE    (0x20000000)

#define VOIDRETURN
#endif


#if (__GMM_KMD__ || GMM_OCL)

#if defined (GMM_OCL)
#include "../../3d/common/iStdLib/osinlines.h"
#endif

// Enable GMM_ASSERTS and GMM_DEBUG only for Debug builds similar to what UMD clients used to do earlier
#if (_DEBUG) //(_DEBUG || _RELEASE_INTERNAL)
    #define GMM_DPF             GMMDebugMessage     //gfxDebug.h
    #define GMM_ASSERT          GMMASSERT           //gfxDebug.h
    #define GMM_ASSERTPTR       GMMASSERTPTR        //gfxDebug.h
#else
    #define GMMDebugMessage(...)
    #define GMM_DPF             GMMDebugMessage
    #define GMM_ASSERT(expr)
    #define GMM_ASSERTPTR(expr, ret)
#endif

#define __GMM_ASSERT        GMM_ASSERT
#define __GMM_ASSERTPTR     GMM_ASSERTPTR

#else

// Enable GMM_ASSERTS and GMM_DEBUG only for Debug builds similar to what UMD clients used to do earlier
#if (_DEBUG) //(_DEBUG || _RELEASE_INTERNAL)

#if defined(_MSC_VER)
#define GMM_DBG_BREAK __debugbreak()
#elif defined(__GNUC__)
#define GMM_DBG_BREAK __builtin_trap()
#else
#include <assert.h>
#define GMM_DBG_BREAK assert(0)
#endif

#define GMMLibDebugMessage(DebugLevel, message, ...)                                \
{                                                                                   \
    if(DebugLevel == GFXDBG_CRITICAL)                                               \
    {                                                                               \
        GMM_LOG_ERROR(message, ##__VA_ARGS__);                                      \
    }                                                                               \
    else if(DebugLevel == GFXDBG_VERBOSE)                                           \
    {                                                                               \
        GMM_LOG_TRACE(message, ##__VA_ARGS__);                                      \
    }                                                                               \
    else if(DebugLevel == GFXDBG_OFF)                                               \
    {                                                                               \
        GMM_LOG_TRACE_IF(0, message, ##__VA_ARGS__)                             \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        GMM_LOG_INFO(message, ##__VA_ARGS__);                                       \
    }                                                                               \
}

#define GMM_LIB_ASSERT(expr)                                    \
{                                                               \
    if(!(expr) )                                                \
    {                                                           \
        GMM_DBG_BREAK;                                          \
    }                                                           \
}

#define GMM_LIB_ASSERTPTR(expr, ret)                            \
{                                                               \
    GMM_LIB_ASSERT(expr);                                       \
    if( !expr )                                                 \
    {                                                           \
        return ret;                                             \
    }                                                           \
}

#else

#define GMMLibDebugMessage(...)
#define GMM_LIB_ASSERT(expr)
#define GMM_LIB_ASSERTPTR(expr, ret)                            \
{                                                               \
    if (!expr)                                                  \
    {                                                           \
        return ret;                                             \
    }                                                           \
}                                                               \

#endif // (_DEBUG) //_DEBUG || _RELEASE_INTERNAL

#define GMM_DPF             GMMLibDebugMessage
#define __GMM_ASSERT        GMM_LIB_ASSERT
#define __GMM_ASSERTPTR     GMM_LIB_ASSERTPTR

#endif


// ------------------------------------------------------------------------
// This block defines various debug print macros
// ------------------------------------------------------------------------
#define GMM_DPF_ENTER   \
    GMM_DPF(GFXDBG_FUNCTION_ENTRY, "%s-->\n", __FUNCTION__);

#define GMM_DPF_EXIT    \
    GMM_DPF(GFXDBG_FUNCTION_EXIT, "%s<--\n", __FUNCTION__);

#define GMM_DPF_CRITICAL(Message)    \
    GMM_DPF(GFXDBG_CRITICAL, "%s:%d: %s\n", __FUNCTION__, __LINE__, (Message));

#ifndef GMM_ASSERTDPF
    #define GMM_ASSERTDPF(Expression, Message)  \
    {                                           \
        if (!(Expression))                      \
        {                                       \
            GMM_DPF_CRITICAL(Message);          \
            __GMM_ASSERT(0);                    \
        }                                       \
    }
#endif
