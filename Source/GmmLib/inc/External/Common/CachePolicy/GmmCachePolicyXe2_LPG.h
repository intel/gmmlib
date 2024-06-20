/*==============================================================================
Copyright(c) 2024 Intel Corporation
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

#define GMM_XE2_NUM_MOCS_ENTRIES  (16)
#define GMM_XE2_DEFAULT_PAT_INDEX (PAT2)


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef enum GMM_GFX_PHY_L4_MEMORY_TYPE_REC
{
    GMM_GFX_PHY_L4_MT_WB = 0x0,
    GMM_GFX_PHY_L4_MT_WT = 0x1,
    GMM_GFX_PHY_L4_MT_UC = 0x3,
} GMM_GFX_PHY_L4_MEMORY_TYPE;

typedef enum GMM_GFX_PHY_L3_MEMORY_TYPE_REC
{
    GMM_GFX_PHY_L3_MT_WB    = 0x0,
    GMM_GFX_PHY_L3_MT_WB_XD = 0x1, // Transient Flush Display
    GMM_GFX_PHY_L3_MT_UC    = 0x3,
} GMM_GFX_PHY_L3_MEMORY_TYPE;

typedef enum GMM_GFX_PHY_CACHE_COHERENCY_TYPE_REC
{
    GMM_GFX_PHY_NON_COHERENT_NO_SNOOP         = 0x0,
    GMM_GFX_PHY_NON_COHERENT                  = 0x1,
    GMM_GFX_PHY_COHERENT_ONE_WAY_IA_SNOOP     = 0x2,
    GMM_GFX_PHY_COHERENT_TWO_WAY_IA_GPU_SNOOP = 0x3
} GMM_GFX_PHY_CACHE_COHERENCY_TYPE;

typedef union GMM_XE2_PRIVATE_PAT_REC
{
    struct
    {
        uint32_t Coherency            : 2;
        uint32_t L4CC                 : 2;
        uint32_t L3CC                 : 2;
        uint32_t L3CLOS               : 2;
        uint32_t Reserved1            : 1;
        uint32_t LosslessCompressionEn: 1;
        uint32_t Reserved2            : 22;
    } Xe2;
    uint32_t Value;
} GMM_XE2_PRIVATE_PAT;

namespace GmmLib
{
    class NON_PAGED_SECTION GmmXe2_LPGCachePolicy : public GmmXe_LPGCachePolicy
    {
    protected:

    public:
        /* Constructors */
        GmmXe2_LPGCachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicyContext, Context *pGmmLibContext)
            : GmmXe_LPGCachePolicy(pCachePolicyContext, pGmmLibContext)
        {
            NumPATRegisters     = GMM_NUM_PAT_ENTRIES;
            NumMOCSRegisters    = GMM_XE2_NUM_MOCS_ENTRIES;
            CurrentMaxPATIndex  = 0;
            CurrentMaxMocsIndex = 0;
        }
        virtual ~GmmXe2_LPGCachePolicy()
        {
        }

        /* Function prototypes */
        GMM_STATUS           InitCachePolicy();
        GMM_STATUS           SetupPAT();
        void                 SetUpMOCSTable();
        void                 GetL3L4(GMM_CACHE_POLICY_TBL_ELEMENT *pUsageEle, GMM_XE2_PRIVATE_PAT *pUsagePATElement, uint32_t Usage);
        uint32_t GMM_STDCALL CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable);
    };
} // namespace GmmLib
#endif // #ifdef __cplusplus

#ifdef __cplusplus
}
#endif /* end__cplusplus*/
