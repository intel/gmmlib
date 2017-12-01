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
#include "GmmUtil.h"

#if __GMM_KMD__
    #define NON_PAGED_SECTION __declspec(code_seg("$kerneltext$"))

    void* GmmAllocKmdSystemMem(uint32_t Size, BOOLEAN bPageable, uint32_t Tag);
    void GmmFreeKmdSystemMem(void* pMem, uint32_t Tag);

    #define GMM_MALLOC(size)    GmmAllocKmdSystemMem((size), FALSE, GFX_COMPONENT_GMM_TAG)
    #define GMM_FREE(p)         GmmFreeKmdSystemMem((p), GFX_COMPONENT_GMM_TAG)
#else
    #include <stdlib.h>

    #define NON_PAGED_SECTION

    #define GMM_MALLOC(size)    malloc(size)
    #define GMM_FREE(p)         free(p)
#endif

/////////////////////////////////////////////////////////////
/// Overrides new() and delete() to work with both user mode 
/// and kernel mode.
/////////////////////////////////////////////////////////////
class NON_PAGED_SECTION GmmMemAllocator
{
    public:
        void* operator new(size_t size) 
        {
            return GMM_MALLOC(size);
        }

        void* operator new(size_t size, void* ptr)
        {
            GMM_UNREFERENCED_PARAMETER(size);
            return ptr;
        }
        
        void operator delete(void *ptr) 
        {       
            GMM_FREE(ptr);
        }

        void operator delete(void *ptr, void *place)
        {
            GMM_UNREFERENCED_PARAMETER(ptr);
            GMM_UNREFERENCED_PARAMETER(place);
            // placement delete -- nothing to do.
        }
};
