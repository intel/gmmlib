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

#if defined(D3D12_GMM)
    #include "..\..\d3d12\PCH\D3D12DebugGMM.h"
#elif defined(D3D10_GMM)
    #include "..\..\d3d10\Imola\ID3D10DebugGMM.h"
#elif defined(DX9IMOLA_GMM)
    #include "ID3D9Debug.h"
#elif defined(DXVA11_GMM)
    #include "..\..\media\ddi\lhdm_d3d11\dxva11_debug_gmm.h"
#else
    #include "../../Common/AssertTracer/AssertTracer.h"
    #include "../../util/gfxDebug.h"
#endif

// ------------------------------------------------------------------------
// This block determines defines anything that GMM clients don't provide.
// ------------------------------------------------------------------------
#if defined (__KLOCWORK__)

    #define GMM_DPF GMMDebugMessage
    #ifndef __GMM_ASSERT
        #define __GMM_ASSERT(expr) do { if (!(expr)) abort(); } while (0)
        #define __GMM_ASSERTPTR(expr, ret) do { if (!(expr)) return ret; } while (0)
    #endif
#elif defined (D3D12_GMM)

    #define GMM_DPF         GMMDebugMessage     //D3D12Debug.h
    #ifndef __GMM_ASSERT
        #define __GMM_ASSERT    GMM_ASSERT          //D3D12Debug.h
        #define __GMM_ASSERTPTR GMM_ASSERTPTR       //D3D12Debug.h
    #endif
#elif defined (D3D10_GMM)

    #define GMM_DPF         GMMDebugMessage     //D3D10Debug.h
    #ifndef __GMM_ASSERT
        #define __GMM_ASSERT    GMM_ASSERT          //D3D10Debug.h
        #define __GMM_ASSERTPTR GMM_ASSERTPTR       //D3D10Debug.h
    #endif
#elif defined (DX9IMOLA_GMM)

    #define GMM_DPF         GMMPrintMessage     // ID3D9Debug.h
    #ifndef __GMM_ASSERT
        #define __GMM_ASSERT    GMM_ASSERT          // ID3D9Debug.h
        #define __GMM_ASSERTPTR GMM_ASSERT_PTR      // ID3D9Debug.h
    #endif
#elif defined (DXVA11_GMM)

    #define GMM_DPF         GMMDebugMessage     // dxva11_debug_gmm.h
    #ifndef __GMM_ASSERT
        #define __GMM_ASSERT    GMM_ASSERT          // dxva11_debug_gmm.h
        #define __GMM_ASSERTPTR GMM_ASSERTPTR       // dxva11_debug_gmm.h
    #endif
#elif defined (GMM_OGL)

    #if _DEBUG && !defined(__GMM_ASSERT)
        #ifdef __cplusplus
            extern"C"
        #endif
        void __cdecl GMMDebugBreak( const char* file, const char* function, const int line );

        #define GMM_DPF GMMDebugMessage
        #define GMM_DEBUG_BREAK GMMDebugBreak(__FILE__,__FUNCTION__,__LINE__)

        #define __GMM_ASSERT( expr )                \
            if( !(expr) )                           \
            {                                       \
                GMM_DEBUG_BREAK;                    \
            }
        #define __GMM_ASSERTPTR( expr, ret )        \
        {                                           \
            if (!(expr))                            \
            {                                       \
                GMM_DEBUG_BREAK;                    \
                return ret;                         \
            }                                       \
        }

    #else
        #define GMM_DPF(...)
        #define __GMM_ASSERT(expr)
        #define __GMM_ASSERTPTR(expr, ret)
    #endif

#else
    #if defined (GMM_OCL)
        #include "../../3d/common/iStdLib/osinlines.h"
    #endif
    #define GMM_DPF         GMMDebugMessage     //gfxDebug.h
    #ifndef __GMM_ASSERT
        #define __GMM_ASSERT    GMMASSERT           //gfxDebug.h
        #define __GMM_ASSERTPTR GMMASSERTPTR        //gfxDebug.h
    #endif
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
