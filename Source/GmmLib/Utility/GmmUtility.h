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

#if __cplusplus
namespace GmmLib
{
    namespace Utility
    {

        uint32_t GMM_STDCALL GmmGetNumPlanes(GMM_RESOURCE_FORMAT Format);
        GMM_RESOURCE_FORMAT GMM_STDCALL GmmGetFormatForASTC(uint8_t HDR,
                                                            uint8_t Float,
                                                            uint32_t BlockWidth,
                                                            uint32_t BlockHeight,
                                                            uint32_t BlockDepth);
    }
}
#endif

#ifndef __GMM_KMD__
    #define GMM_MALLOC(size)    malloc(size)
    #define GMM_FREE(p)         free(p)
#endif

void GMM_STDCALL GmmGetCacheSizes(GMM_CACHE_SIZES* pCacheSizes);

/* Internal functions */
uint32_t   __GmmLog2(uint32_t Value);

