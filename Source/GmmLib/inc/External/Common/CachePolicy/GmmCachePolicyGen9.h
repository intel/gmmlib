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

#ifdef __cplusplus
#include "../GmmCachePolicyCommon.h"

namespace GmmLib
{
    class NON_PAGED_SECTION GmmGen9CachePolicy :
        public GmmGen8CachePolicy
    {
        public:
            uint32_t CurrentMaxMocsIndex;
            uint32_t CurrentMaxL1HdcMocsIndex;
            /* Constructors */
            GmmGen9CachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicy) :GmmGen8CachePolicy(pCachePolicy)
            {
            }
            virtual ~GmmGen9CachePolicy()
            {
            }

            /* Function prototypes */
            GMM_STATUS InitCachePolicy();
            GMM_STATUS SetupPAT();
    };
}
#endif // #ifdef __cplusplus