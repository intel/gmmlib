/*==============================================================================
Copyright(c) 2022 Intel Corporation
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

#define GMM_NUM_PAT_ENTRIES_Xe_LPG (16)

namespace GmmLib
{
    class NON_PAGED_SECTION GmmXe_LPGCachePolicy :
        public GmmGen12CachePolicy
    {
        protected:
            uint32_t               CurrentMaxPATIndex;

        public:

            /* Constructors */
            GmmXe_LPGCachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicyContext, Context *pGmmLibContext)
                : GmmGen12CachePolicy(pCachePolicyContext, pGmmLibContext)
            {
                NumPATRegisters    = GMM_NUM_PAT_ENTRIES_Xe_LPG;
                CurrentMaxPATIndex = 0;
            }
            virtual ~GmmXe_LPGCachePolicy()
            {
            }

            /* Function prototypes */
            GMM_STATUS InitCachePolicy();
            uint32_t GMM_STDCALL CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable);
            GMM_STATUS SetupPAT();
            void       SetUpMOCSTable();
            void GMM_STDCALL                                SetL1CachePolicy(uint32_t Usage);
            virtual uint32_t GMM_STDCALL                    GetSurfaceStateL1CachePolicy(GMM_RESOURCE_USAGE_TYPE Usage);
            virtual MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage);

};

}
#endif // #ifdef __cplusplus
