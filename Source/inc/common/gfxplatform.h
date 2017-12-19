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

#ifndef __GFXPLATFORM_H__
#define __GFXPLATFORM_H__

////////////////////////////////////////////////////////////////////////
// ABSTRACT:
//                  Platform-specific constants, #defines, and
//                  #includes.  For the purposes of this header file,
//                  a platform is defined as the combination of:
//
//                    - CPU Architecture
//                    - C/C++ Compiler
////////////////////////////////////////////////////////////////////////

#if _MSC_VER
#pragma inline_depth (255)
#endif // _MSC_VER

#ifndef __S_INLINE
    #define __S_INLINE __inline
#endif

#ifndef __F_INLINE
    #ifdef _DEBUG
        #define __F_INLINE static
    #else
        #define __F_INLINE static __inline
    #endif
#endif

// Component definitions for ASSERT.

#define GFX_GMM	   (uint32_t)0x00000020

#ifdef _DEBUG
  #define GFX_ASSERT(component,expr)        \
                                        {if (!(expr)) {__debugbreak();}}
#else
  #define GFX_ASSERT(component,expr)
#endif

//========================================================================
// From winnt.h
//
// C_ASSERT() can be used to perform many COMPILE-TIME assertions:
//            type sizes, field offsets, etc.
//
// An assertion failure results in error C2118: negative subscript.
//
// When this assertion is to be used in the middle of a code block,
// use it within {} - e.g. {GFX_C_ASSERT (GFX_NUM == 0);}
#ifndef GFX_C_ASSERT
#define GFX_C_ASSERT(e) typedef char GFX_C_ASSERT__[(e)?1:-1]
#endif
//========================================================================
// GFX_MMX_ALIGN:  Alignment boundary (in bytes) for optimal use of
// the MMX instruction set on IA32.

#define GFX_MMX_ALIGN              8

#endif // #ifndef __GFXPLATFORM_H__
