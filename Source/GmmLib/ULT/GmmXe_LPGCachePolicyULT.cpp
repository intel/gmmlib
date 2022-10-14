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
#include "GmmXe_LPGCachePolicyULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Cache Policy fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
/// It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestXe_LPGCachePolicy::SetUpTestCase()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestXe_LPGCachePolicy::TearDownTestCase()
{
}

void CTestXe_LPGCachePolicy::SetUpXe_LPGVariant(PRODUCT_FAMILY platform)
{
    printf("%s\n", __FUNCTION__);

    GfxPlatform.eProductFamily = platform;

    GfxPlatform.eRenderCoreFamily = IGFX_XE_HPG_CORE;

    pGfxAdapterInfo = (ADAPTER_INFO *)malloc(sizeof(ADAPTER_INFO));
    if(pGfxAdapterInfo)
    {
        memset(pGfxAdapterInfo, 0, sizeof(ADAPTER_INFO));

        pGfxAdapterInfo->SkuTable.FtrLinearCCS             = 1; //legacy y =>0 - test both
        pGfxAdapterInfo->SkuTable.FtrStandardMipTailFormat = 1;
        pGfxAdapterInfo->SkuTable.FtrTileY                 = 0;
        pGfxAdapterInfo->SkuTable.FtrLocalMemory           = 0;
        pGfxAdapterInfo->SkuTable.FtrIA32eGfxPTEs          = 1;
        CommonULT::SetUpTestCase();
    }
}

void CTestXe_LPGCachePolicy::TearDownXe_LPGVariant()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

/***********************XeLPG***********************************/
TEST_F(CTestXe_LPGCachePolicy, TestXe_LPGCachePolicy_FtrL4CacheEnabled)
{
    SetUpXe_LPGVariant(IGFX_METEORLAKE);
    CheckVirtualL3CachePolicy();
    CheckPAT();
    TearDownXe_LPGVariant();
}

void CTestXe_LPGCachePolicy::CheckVirtualL3CachePolicy()
{
    const uint32_t L4_WB_CACHEABLE = 0x0;
    const uint32_t L4_WT_CACHEABLE = 0x1;
    const uint32_t L4_UNCACHEABLE  = 0x3;

    const uint32_t L3_WB_CACHEABLE = 0x3;
    const uint32_t L3_UNCACHEABLE  = 0x1;

    for(uint32_t MocsIndex = 0; MocsIndex < GMM_XE_NUM_MOCS_ENTRIES; MocsIndex++)
    {
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs = pGmmULTClientContext->GetCachePolicyTlbElement(MocsIndex);
        //printf("Xe LPG: MocsIndex: %d --> Global Index: [0x%x]\n", MocsIndex, Mocs.LeCC.Xe_LPG.DwordValue);
        //printf("Xe LPG: MocsIndex: %d --> L3 Index: [0x%x]\n", MocsIndex, Mocs.L3.UshortValue);
    }

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT     ClientRequest   = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        uint32_t                AssignedMocsIdx = ClientRequest.MemoryObjectOverride.Gen12.Index;
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs            = pGmmULTClientContext->GetCachePolicyTlbElement(AssignedMocsIdx);
        uint32_t                     StartMocsIdx    = 0;

        EXPECT_EQ(0, Mocs.L3.ESC) << "Usage# " << Usage << ": ESC is non-zero";
        EXPECT_EQ(0, Mocs.L3.SCC) << "Usage# " << Usage << ": SCC is non-zero";
        EXPECT_EQ(0, Mocs.L3.Reserved) << "Usage# " << Usage << ": Reserved field is non-zero";
        // Check if Mocs Index is not greater than GMM_MAX_NUMBER_MOCS_INDEXES
        EXPECT_GT(GMM_XE_NUM_MOCS_ENTRIES, AssignedMocsIdx) << "Usage# " << Usage << ": MOCS Index greater than MAX allowed (16)";
        EXPECT_EQ(0, Mocs.LeCC.Xe_LPG.Reserved0) << "Usage# " << Usage << ": : Reserved field is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.Xe_LPG.Reserved1) << "Usage# " << Usage << ": : Reserved field is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.Xe_LPG.Reserved2) << "Usage# " << Usage << ": : Reserved field is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.Xe_LPG.Reserved3) << "Usage# " << Usage << ": : Reserved field is non-zero";
        EXPECT_EQ(1, Mocs.LeCC.Xe_LPG.igPAT) << "Usage# " << Usage << ": Incorrect igPAT cachebility setting";

        //printf("Xe LPG: Usage: %d --> Index: [%d]\n", Usage, AssignedMocsIdx);

        //L3
        if(ClientRequest.L3)
        {
            EXPECT_EQ(L3_WB_CACHEABLE, Mocs.L3.Cacheability) << "Usage# " << Usage << ": Incorrect L3 cachebility setting";
        }
        else
        {
            EXPECT_EQ(L3_UNCACHEABLE, Mocs.L3.Cacheability) << "Usage# " << Usage << ": Incorrect L3 cachebility setting";
        }

        //L4
        // ADM memory cache 0: UC, 1:WB, 2: WT
        switch(ClientRequest.L4CC)
        {
            case 0x1:
            {
                EXPECT_EQ(L4_WB_CACHEABLE, Mocs.LeCC.Xe_LPG.L4CC) << "Usage# " << Usage << ": Incorrect L4CC cachebility setting";
                break;
            }
            case 0x2:
            {
                EXPECT_EQ(L4_WT_CACHEABLE, Mocs.LeCC.Xe_LPG.L4CC) << "Usage# " << Usage << ": Incorrect L4CC cachebility setting";
                break;
            }
            default:
                EXPECT_EQ(L4_UNCACHEABLE, Mocs.LeCC.Xe_LPG.L4CC) << "Usage# " << Usage << ": Incorrect L4CC cachebility setting";
        }
    }
}

void CTestXe_LPGCachePolicy::CheckPAT()
{

    // Check Usage PAT index against PAT settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT ClientRequest = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        if(ClientRequest.Initialized == false) // undefined resource in platform
        {
            continue;
        }
        uint32_t PATIndex = pGmmULTClientContext->CachePolicyGetPATIndex(NULL, (GMM_RESOURCE_USAGE_TYPE)Usage, NULL, false);
        EXPECT_NE(PATIndex, GMM_PAT_ERROR) << "Usage# " << Usage << ": No matching PAT Index";
    }
}

