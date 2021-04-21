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

#include "GmmGen12CachePolicyULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Cache Policy fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
/// It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen12CachePolicy::SetUpTestCase()
{
    GfxPlatform.eProductFamily    = IGFX_TIGERLAKE_LP;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN12_CORE;

    AllocateAdapterInfo();

    pGfxAdapterInfo->SystemInfo.L3CacheSizeInKb = 3072;

    const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrEDram     = false;
    const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLLCBypass = 0;

    CommonULT::SetUpTestCase();

    printf("%s\n", __FUNCTION__);
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen12CachePolicy::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

void CTestGen12CachePolicy::CheckL3CachePolicy()
{
    const uint32_t L3_WB_CACHEABLE = 0x3;
    const uint32_t L3_UNCACHEABLE  = 0x1;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT     ClientRequest   = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        uint32_t                     AssignedMocsIdx = ClientRequest.MemoryObjectOverride.Gen12.Index;
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs            = pGmmULTClientContext->GetCachePolicyTlbElement(AssignedMocsIdx);

        //printf("Usage: %d --> Index: [%d]\n", Usage, AssignedMocsIdx);

        EXPECT_EQ(0, Mocs.L3.ESC) << "Usage# " << Usage << ": ESC is non-zero";
        EXPECT_EQ(0, Mocs.L3.SCC) << "Usage# " << Usage << ": SCC is non-zero";
        EXPECT_EQ(0, Mocs.L3.Reserved) << "Usage# " << Usage << ": Reserved field is non-zero";

        // Check if Mocs Index is not greater than GMM_MAX_NUMBER_MOCS_INDEXES
        EXPECT_GT(GMM_MAX_NUMBER_MOCS_INDEXES, AssignedMocsIdx) << "Usage# " << Usage << ": MOCS Index greater than MAX allowed (63)";

        if(ClientRequest.L3Eviction == 0x2) //63
        {
            if((GMM_RESOURCE_USAGE_TYPE)Usage == GMM_RESOURCE_USAGE_L3_EVICTION)
            {
                EXPECT_EQ(AssignedMocsIdx, 63) << "Usage# " << Usage << ": Incorrect Index for L3Eviction type# " << ClientRequest.L3Eviction;
                EXPECT_EQ(0, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for L3Eviction type# " << ClientRequest.L3Eviction;
            }
            else
            {
                EXPECT_NE(AssignedMocsIdx, 63) << "Usage# " << Usage << ": Incorrect Index for L3Eviction type# " << ClientRequest.L3Eviction;
                EXPECT_EQ(1, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for L3Eviction type# " << ClientRequest.L3Eviction;
            }
        }
        else if(ClientRequest.L3Eviction == 0x3) //61
        {
            EXPECT_EQ(AssignedMocsIdx, 61) << "Usage# " << Usage << ": Incorrect Index for L3Eviction type# " << ClientRequest.L3Eviction;
            EXPECT_EQ(1, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for L3Eviction type# " << ClientRequest.L3Eviction;
        }
        else if(Usage == GMM_RESOURCE_USAGE_CCS) //60
        {
            EXPECT_EQ(AssignedMocsIdx, 60) << "Usage# " << Usage << ": Incorrect Index for CCS";
            EXPECT_EQ(0, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for CCS";
        }
        else if(Usage == GMM_RESOURCE_USAGE_MOCS_62) //62
        {
            EXPECT_EQ(AssignedMocsIdx, 62) << "Usage# " << Usage << ": Incorrect Index for MOCS_62";
            EXPECT_EQ(0, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for MOCS#62";
        }
        // Check of assigned Index setting is appropriate for HDCL1 setting
        else if(ClientRequest.HDCL1)
        {
            EXPECT_GE(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
        }
        else
        {
            EXPECT_LT(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
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


TEST_F(CTestGen12CachePolicy, TestL3CachePolicy)
{
    CheckL3CachePolicy();
}


void CTestGen12CachePolicy::CheckLlcEdramCachePolicy()
{
    const uint32_t TargetCache_LLC = 1;

    const uint32_t LeCC_UNCACHEABLE    = 0x0;
    const uint32_t LeCC_WC_UNCACHEABLE = 0x1;
    const uint32_t LeCC_WB_CACHEABLE   = 0x3;
    const uint32_t LeCC_WT_CACHEABLE   = 0x2;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT     ClientRequest   = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        uint32_t                        AssignedMocsIdx = ClientRequest.MemoryObjectOverride.Gen12.Index;
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs            = pGmmULTClientContext->GetCachePolicyTlbElement(AssignedMocsIdx);

        // Check for unused fields
        EXPECT_EQ(0, Mocs.LeCC.AOM) << "Usage# " << Usage << ": AOM is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.CoS) << "Usage# " << Usage << ": CoS is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.PFM) << "Usage# " << Usage << ": PFM is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.SCC) << "Usage# " << Usage << ": SCC is non-zero";
        // SCF field might be set for LKF/Gen12+ platforms;
        EXPECT_EQ(0, Mocs.LeCC.SCF & !const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLLCBypass) << "Usage# " << Usage << ": SCF is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.ESC) << "Usage# " << Usage << ": ESC is non-zero";
        EXPECT_EQ(0, Mocs.LeCC.Reserved) << "Usage# " << Usage << ": Reserved field is non-zero";

        // Check for age
        EXPECT_EQ(ClientRequest.AGE, Mocs.LeCC.LRUM) << "Usage# " << Usage << ": Incorrect AGE settings";

        // Check for Snoop Setting
        EXPECT_EQ(ClientRequest.SSO, Mocs.LeCC.SelfSnoop) << "Usage# " << Usage << ": Self Snoop is non-zero";

        // Check if Mocs Index is not greater than GMM_MAX_NUMBER_MOCS_INDEXES
        EXPECT_GT(GMM_MAX_NUMBER_MOCS_INDEXES, AssignedMocsIdx) << "Usage# " << Usage << ": MOCS Index greater than MAX allowed (63)";

        if(ClientRequest.L3Eviction == 0x2) //63
        {
            GMM_CACHE_POLICY_ELEMENT MOCS63 = pGmmULTClientContext->GetCachePolicyElement(GMM_RESOURCE_USAGE_L3_EVICTION);
            if((GMM_RESOURCE_USAGE_TYPE)Usage == GMM_RESOURCE_USAGE_L3_EVICTION)
            {
                EXPECT_EQ(AssignedMocsIdx, 63) << "Usage# " << Usage << ": Incorrect Index for L3Eviction type# " << ClientRequest.L3Eviction;
            }
            else
            {
                MOCS63.L3 = 1; //Override L3 to test , since Hw forces it to L3-uncached
                EXPECT_NE(AssignedMocsIdx, 63) << "Usage# " << Usage << ": Incorrect Index for L3Eviction type# " << ClientRequest.L3Eviction;
                EXPECT_EQ(MOCS63.Value, ClientRequest.Value) << "Usage# " << Usage << ": Incorrect usage for L3Eviction type# " << ClientRequest.L3Eviction;
            }
        }
        else if(ClientRequest.L3Eviction == 0x3) //61
        {
            GMM_CACHE_POLICY_ELEMENT MOCS61 = pGmmULTClientContext->GetCachePolicyElement(GMM_RESOURCE_USAGE_L3_EVICTION_SPECIAL);
            EXPECT_EQ(AssignedMocsIdx, 61) << "Usage# " << Usage << ": Incorrect Index for L3Eviction type# " << ClientRequest.L3Eviction;
            EXPECT_EQ(0, ClientRequest.LLC) << "Usage# " << Usage << ": Incorrect LLC cacheability for L3Eviction type# " << ClientRequest.L3Eviction;
            EXPECT_EQ(MOCS61.Value, ClientRequest.Value) << "Usage# " << Usage << ": Incorrect usage for L3Eviction type# " << ClientRequest.L3Eviction;
        }
        else if(Usage == GMM_RESOURCE_USAGE_CCS) //60
        {
            EXPECT_EQ(AssignedMocsIdx, 60) << "Usage# " << Usage << ": Incorrect Index for CCS";
        }
        else if(Usage == GMM_RESOURCE_USAGE_MOCS_62) //62
        {
            EXPECT_EQ(AssignedMocsIdx, 62) << "Usage# " << Usage << ": Incorrect Index for MOCS_62";
        }
        // Check of assigned Index setting is appropriate for HDCL1 setting
        else if(ClientRequest.HDCL1)
        {
            EXPECT_GE(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
        }
        else
        {
            EXPECT_LT(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
        }

        if(!ClientRequest.LLC && !ClientRequest.ELLC) // Uncached
        {
            EXPECT_EQ(LeCC_WC_UNCACHEABLE, Mocs.LeCC.Cacheability) << "Usage# " << Usage << ": Incorrect LLC/eDRAM cachebility setting";
        }
        else
        {
            if(ClientRequest.LLC) // LLC only
            {
                EXPECT_EQ(TargetCache_LLC, Mocs.LeCC.TargetCache) << "Usage# " << Usage << ": Incorrect target cache setting";

                EXPECT_EQ(LeCC_WB_CACHEABLE, Mocs.LeCC.Cacheability) << "Usage# " << Usage << ": Incorrect LLC cachebility setting";
            }
            else
            {
                EXPECT_EQ(TargetCache_LLC, Mocs.LeCC.TargetCache) << "Usage# " << Usage << ": Incorrect target cache setting";

                EXPECT_EQ(LeCC_WC_UNCACHEABLE, Mocs.LeCC.Cacheability) << "Usage# " << Usage << ": Incorrect LLC cachebility setting";
            }
        }
    }
}

TEST_F(CTestGen12CachePolicy, TestLlcEdramCachePolicy)
{
    CheckLlcEdramCachePolicy();
}
