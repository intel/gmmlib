/*==============================================================================
Copyright(c) 2020 Intel Corporation

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
    class NON_PAGED_SECTION GmmGen12dGPUCachePolicy :
        public GmmGen12CachePolicy
    {
        public:

            /* Constructors */
            GmmGen12dGPUCachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicy):GmmGen12CachePolicy(pCachePolicy)
            {
            }
            virtual ~GmmGen12dGPUCachePolicy()
            {
            }

            /* Function prototypes */
            GMM_STATUS InitCachePolicy();
            void       SetUpMOCSTable();
            int32_t    IsSpecialMOCSUsage(GMM_RESOURCE_USAGE_TYPE Usage, bool& UpdateMOCS);

    };
}
#endif // #ifdef __cplusplus
