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
    class NON_PAGED_SECTION GmmGen8CachePolicy :
        public GmmCachePolicyCommon
    {
        protected:
            /* Function prototypes */
            GMM_PRIVATE_PAT GetPrivatePATEntry(uint32_t PATIdx);
            bool            SetPrivatePATEntry(uint32_t PATIdx, GMM_PRIVATE_PAT Entry);
            bool            SelectNewPATIdx(GMM_GFX_MEMORY_TYPE WantedMT, GMM_GFX_TARGET_CACHE WantedTC,
                                            GMM_GFX_MEMORY_TYPE MT1, GMM_GFX_TARGET_CACHE TC1,
                                            GMM_GFX_MEMORY_TYPE MT2, GMM_GFX_TARGET_CACHE TC2);
            bool            GetUsagePTEValue(GMM_CACHE_POLICY_ELEMENT  CachePolicy,
                uint32_t                     Usage,
                uint64_t                    *pPTEDwordValue);

            /* Virtual function prototypes */
            virtual uint32_t BestMatchingPATIdx(const GMM_CACHE_POLICY_ELEMENT &CachePolicy);

        public:
            /* Constructors */
            GmmGen8CachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicyContext, Context *pGmmLibContext)
                : GmmCachePolicyCommon(pCachePolicyContext, pGmmLibContext)            
	    {
                #if(defined(__GMM_KMD__))
                //if (GFX_IS_SKU(pGmmLibContext, FtrIA32eGfxPTEs))
                {
                    // Set the WA's needed for Private PAT initialization
                    SetPATInitWA();
                    SetupPAT();
                }
                #endif
            }

            virtual ~GmmGen8CachePolicy()
            {
            }

            /* Function prototypes */
            GMM_STATUS InitCachePolicy();
            GMM_STATUS SetPATInitWA();
            GMM_STATUS SetupPAT();
            uint8_t GMM_STDCALL CachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage);
    };
}
#endif // #ifdef __cplusplus
