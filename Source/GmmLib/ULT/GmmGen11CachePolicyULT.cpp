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

#include "GmmGen11CachePolicyULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Cache Policy fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
/// It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen11CachePolicy::SetUpTestCase()
{
    GfxPlatform.eProductFamily    = IGFX_ICELAKE;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN11_CORE;
    AllocateAdapterInfo();

    pGfxAdapterInfo->SystemInfo.L3CacheSizeInKb                         = 3072;
    pGfxAdapterInfo->SystemInfo.LLCCacheSizeInKb                        = 2 * 1024;   //2 MB
    pGfxAdapterInfo->SystemInfo.EdramSizeInKb                           = 128 * 1024; //128 MB
    const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrEDram = 1;

    CommonULT::SetUpTestCase();

    printf("%s\n", __FUNCTION__);
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen11CachePolicy::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

void CTestGen11CachePolicy::CheckL3CachePolicy()
{
    const uint32_t L3_WB_CACHEABLE = 0x3;
    const uint32_t L3_UNCACHEABLE  = 0x1;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT     ClientRequest   = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        uint32_t                     AssignedMocsIdx = ClientRequest.MemoryObjectOverride.Gen11.Index;
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs            = pGmmULTClientContext->GetCachePolicyTlbElement(AssignedMocsIdx);

        EXPECT_EQ(0, Mocs.L3.ESC) << "Usage# " << Usage << ": ESC is non-zero";
        EXPECT_EQ(0, Mocs.L3.SCC) << "Usage# " << Usage << ": SCC is non-zero";
        EXPECT_EQ(0, Mocs.L3.Reserved) << "Usage# " << Usage << ": Reserved field is non-zero";

        // Check if Mocs Index is not greater than GMM_GEN9_MAX_NUMBER_MOCS_INDEXES
        EXPECT_GT(GMM_MAX_NUMBER_MOCS_INDEXES, AssignedMocsIdx) << "Usage# " << Usage << ": MOCS Index greater than MAX allowed (62)";

        // Check of assigned Index setting is appropriate for HDCL1 setting
        if(ClientRequest.HDCL1)
        {
            EXPECT_GE(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
        }
        else
        {
            if(Usage == GMM_RESOURCE_USAGE_MOCS_62)
            {
                EXPECT_EQ(AssignedMocsIdx, 62) << "Usage# " << Usage << ": Incorrect Index for MOCS62 usage";
            }
            else if(Usage == GMM_RESOURCE_USAGE_L3_EVICTION)
            {
                EXPECT_EQ(AssignedMocsIdx, 63) << "Usage# " << Usage << ": Incorrect Index for MOCS63 usage";
            }
            else
            {
                EXPECT_LT(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
            }
        }

        if(ClientRequest.L3)
        {
            EXPECT_EQ(L3_WB_CACHEABLE, Mocs.L3.Cacheability) << "Usage# " << Usage << ": Incorrect L3 cachebility setting";
        }
        else
        {
            EXPECT_EQ(L3_UNCACHEABLE, Mocs.L3.Cacheability) << "Usage# " << Usage << ": Incorrect L3 cachebility setting";
        }
    }
}


TEST_F(CTestGen11CachePolicy, TestL3CachePolicy)
{
    CheckL3CachePolicy();
}


void CTestGen11CachePolicy::CheckLlcEdramCachePolicy()
{
    const uint32_t TargetCache_LLC = 1;

    const uint32_t LeCC_UNCACHEABLE  = 0x1;
    const uint32_t LeCC_WB_CACHEABLE = 0x3;
    const uint32_t LeCC_WT_CACHEABLE = 0x2;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT     ClientRequest   = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        uint32_t                     AssignedMocsIdx = ClientRequest.MemoryObjectOverride.Gen11.Index;
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs            = pGmmULTClientContext->GetCachePolicyTlbElement(AssignedMocsIdx);

        // Check for unused fields
        EXPECT_EQ(0, Mocs.LeCC.AOM) << "Usage# " << Usage << ": AOM is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.CoS) << "Usage# " << Usage << ": CoS is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.PFM) << "Usage# " << Usage << ": PFM is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.SCC) << "Usage# " << Usage << ": SCC is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.SCF) << "Usage# " << Usage << ": SCF is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.ESC) << "Usage# " << Usage << ": ESC is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.Reserved) << "Usage# " << Usage << ": Reserved field is non-zero";

        // Check for age
        EXPECT_EQ(ClientRequest.AGE, Mocs.LeCC.LRUM) << "Usage# " << Usage << ": Incorrect AGE settings";

        // Check for Snoop Setting
        EXPECT_EQ(ClientRequest.SSO, Mocs.LeCC.SelfSnoop) << "Usage# " << Usage << ": Self Snoop is non-zero";

        // Check if Mocs Index is not greater than GMM_GEN9_MAX_NUMBER_MOCS_INDEXES
        EXPECT_GT(GMM_MAX_NUMBER_MOCS_INDEXES, AssignedMocsIdx) << "Usage# " << Usage << ": MOCS Index greater than MAX allowed (62)";

        // Check of assigned Index setting is appropriate for HDCL1 setting
        if(ClientRequest.HDCL1)
        {
            EXPECT_GE(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
        }
        else
        {
            if(Usage == GMM_RESOURCE_USAGE_MOCS_62)
            {
                EXPECT_EQ(AssignedMocsIdx, 62) << "Usage# " << Usage << ": Incorrect Index for MOCS62 usage";
            }
            else if(Usage == GMM_RESOURCE_USAGE_L3_EVICTION)
            {
                EXPECT_EQ(AssignedMocsIdx, 63) << "Usage# " << Usage << ": Incorrect Index for MOCS63 usage";
            }
            else
            {
                EXPECT_LT(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
            }
        }

        if(ClientRequest.LLC) // LLC only
        {
            EXPECT_EQ(TargetCache_LLC, Mocs.LeCC.TargetCache) << "Usage# " << Usage << ": Incorrect target cache setting";

            EXPECT_EQ(LeCC_WB_CACHEABLE, Mocs.LeCC.Cacheability) << "Usage# " << Usage << ": Incorrect LLC/eDRAM cachebility setting";
        }
        else
        {
            EXPECT_EQ(LeCC_UNCACHEABLE, Mocs.LeCC.Cacheability) << "Usage# " << Usage << ": Incorrect LLC cachebility setting";
        }
    }
}

TEST_F(CTestGen11CachePolicy, TestLlcEdramCachePolicy)
{
    CheckLlcEdramCachePolicy();
}
