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


//------------------------------------------------------------------------
// Size macros
//------------------------------------------------------------------------
#define GMM_BITS(a)                                     (a)
#define GMM_BYTES(a)                                    (a)
#define GMM_KBYTE(a)                                    ((a) * 1024)
#define GMM_MBYTE(a)                                    ((a) * 1024 * 1024)
#define GMM_GBYTE(a)                                    (((uint64_t) 1024) * 1024 * 1024 * (a))
#define GMM_TBYTE(a)                                    (((uint64_t) 1024) * 1024 * 1024 * 1024 * (a))
#define GMM_SCANLINES(a)                                (a)
#define GMM_DEPTH(a)                                    (a)
#define GMM_PIXELS(a)                                   (a)
#define GMM_BYTES_TO_MBYTES(a)                          ((a) / (1024*1024))
#define GMM_BYTES_TO_KBYTES(a)                          ((a) / 1024)

//------------------------------------------------------------------------
// Bit manipulation macros
//------------------------------------------------------------------------
#define __GMM_IS_ALIGN(A, B)            (((A) % (B)) == 0)
#define __BIT(x)                        (1UL << (x))
#define __MASKED_BIT(x)                 (__BIT(x) | (__BIT(x) << 16))
#define __BIT64(x)                      ((uint64_t)1 << (x))
#define __GMM_SET_BIT(A, b)             (A |= __BIT(b))
#define __GMM_CLEAR_BIT(A, b)           (A &= ~__BIT(b))
#define __GMM_SET_BIT64(A, b)           (A |= __BIT64(b))
#define __GMM_CLEAR_BIT64(A, b)         (A &= ~__BIT64(b))
#define __GMM_IS_BIT_SET(A, b)          (A & __BIT(b))
#define __GMM_IS_BIT_CLEAR(A, b)        ((A & __BIT(b)) == 0)

#define __SET_VALUE(editDst, editMask, editShift, editValue)        \
                                        (editDst) = (((editDst) & ~(editMask)) |\
                                        (((editValue) << (editShift)) & (editMask)))

#define GMM_UNREFERENCED_PARAMETER(param)                   ((void)(param))
#define GMM_UNREFERENCED_LOCAL_VARIABLE(debug_var)          ((void)(debug_var))


//------------------------------------------------------------------------
// Escape macros
//------------------------------------------------------------------------
#define GMM_ESCAPE_D3D_SET_DEV_CTX(Escape, AdapterHandle, DeviceHandle)       \
    {                                                           \
        Escape.hContext   = NULL;                               \
        Escape.hDevice    = DeviceHandle;                            \
    }

#define GMM_CALL_D3D_ESCAPE(pfnEscape, hAdapter, Escape) (pfnEscape(hAdapter, &Escape))

#define GMM_ESCAPE_OGL_SET_DEV_CTX(Escape, AdapterHandle, DeviceHandle)       \
    {                                                           \
        Escape.hAdapter = (D3DKMT_HANDLE)(uintptr_t)AdapterHandle;              \
        Escape.hContext = (D3DKMT_HANDLE)(uintptr_t)NULL;                  \
        Escape.hDevice  = (D3DKMT_HANDLE)(uintptr_t)DeviceHandle;               \
        Escape.Type     = D3DKMT_ESCAPE_DRIVERPRIVATE;          \
    }
#define GMM_CALL_OGL_ESCAPE(pfnEscape, hAdapter, Escape) (pfnEscape(&Escape))

#define __GMM_RANGE_IN_GMADR(RangeBase, RangeSize,pGmmLibContext)                              \
    (((RangeBase) >= GmmGetGttContext(pGmmLibContext)->GfxAddrRange.Global.Base) &&           \
    (((RangeBase) + (RangeSize)) <=                                             \
        (GmmGetGttContext(pGmmLibContext)->GfxAddrRange.Global.Base +        \
        GmmGetGttContext(pGmmLibContext)->GlobalGfxApertureSize)))

#define __GMM_RANGE_IN_GLOBAL_GTT_SPACE(RangeBase, RangeSize,pGmmLibContext)                   \
    (((RangeBase) >= GmmGetGttContext(pGmmLibContext)->GfxAddrRange.Global.Base) &&           \
    (((RangeBase) + (RangeSize)) <=                                             \
        (GmmGetGttContext(pGmmLibContext)->GfxAddrRange.Global.Base +        \
         GmmGetGttContext(pGmmLibContext)->GfxAddrRange.Global.Size)))

#define __GMM_RANGE_IN_PPGTT_SPACE(RangeBase, RangeSize,pGmmLibContext)                        \
    (((RangeBase) >= GmmGetGttContext(pGmmLibContext)->GfxAddrRange.PP.Base) &&               \
    (((RangeBase) + (RangeSize)) <=                                             \
        (GmmGetGttContext(pGmmLibContext)->GfxAddrRange.PP.Base +                             \
         GmmGetGttContext(pGmmLibContext)->GfxAddrRange.PP.Size)))

#define GMM_INLINE __inline

#if(defined(__GMM_KMD__))
#define GMM_OVERRIDE_SIZE_64KB_ALLOC(pGmmLibContext)  if(GmmGetSkuTable(pGmmLibContext)->FtrPpgtt64KBWalkOptimization){ return (this->GetSizeAllocation());}
#else 
#define GMM_OVERRIDE_SIZE_64KB_ALLOC(pGmmLibContext)  if(((GmmClientContext*)pClientContext)->GetSkuTable().FtrPpgtt64KBWalkOptimization){ return (this->GetSizeAllocation());}
#endif

