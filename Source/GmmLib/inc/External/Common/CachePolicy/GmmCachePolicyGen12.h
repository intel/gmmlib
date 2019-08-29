/*==============================================================================
Copyright(c) 2019 Intel Corporation

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

#define GMM_GEN12_MAX_NUMBER_MOCS_INDEXES (60) // On TGL last four (#60-#63) are reserved by h/w, few? are sw configurable though (#60)

namespace GmmLib
{
    class NON_PAGED_SECTION GmmGen12CachePolicy :
        public GmmGen11CachePolicy
    {
        public:

            /* Constructors */
            GmmGen12CachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicy) :GmmGen11CachePolicy(pCachePolicy)
            {
#if(defined(__GMM_KMD__))
            {
                // Set the WA's needed for Private PAT initialization
                SetPATInitWA();
                SetupPAT();
            }
#endif
            }
            virtual ~GmmGen12CachePolicy()
            {
            }

            virtual uint32_t GetMaxSpecialMocsIndex()
            {
                return CurrentMaxSpecialMocsIndex;
            }

            int32_t IsSpecialMOCSUsage(GMM_RESOURCE_USAGE_TYPE Usage, bool& UpdateMOCS);

            /* Function prototypes */
            GMM_STATUS InitCachePolicy();
            uint8_t SelectNewPATIdx(GMM_GFX_MEMORY_TYPE WantedMT, GMM_GFX_MEMORY_TYPE MT1, GMM_GFX_MEMORY_TYPE MT2);
            uint32_t BestMatchingPATIdx(GMM_CACHE_POLICY_ELEMENT CachePolicy);
            GMM_STATUS SetPATInitWA();
            GMM_STATUS SetupPAT();
            void       SetUpMOCSTable();
    };
}
#endif // #ifdef __cplusplus