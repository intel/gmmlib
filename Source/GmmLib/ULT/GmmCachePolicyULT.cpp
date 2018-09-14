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

#include "GmmCachePolicyULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Cache Policy fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
/// It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestCachePolicy::SetUpTestCase()
{
    GfxPlatform.eProductFamily    = IGFX_BROADWELL;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN8_CORE;
    AllocateAdapterInfo();

    pGfxAdapterInfo->SystemInfo.L3CacheSizeInKb                         = 768;
    pGfxAdapterInfo->SystemInfo.LLCCacheSizeInKb                        = 2 * 1024;  //2 MB
    pGfxAdapterInfo->SystemInfo.EdramSizeInKb                           = 64 * 1024; //64 MB
    const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrEDram = 1;

    CommonULT::SetUpTestCase();

    printf("%s\n", __FUNCTION__);
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestCachePolicy::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

void CTestCachePolicy::CheckL3CachePolicy()
{
    const uint32_t TargetCache_L3_LLC_ELLC = 0x3;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT    ClientRequest = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        MEMORY_OBJECT_CONTROL_STATE Mocs          = ClientRequest.MemoryObjectOverride;

        // Not check WT/WB/UC since that doesn't really matter for L3
        if(ClientRequest.L3)
        {
            EXPECT_EQ(TargetCache_L3_LLC_ELLC, Mocs.Gen8.TargetCache) << "Usage# " << Usage << ": Incorrect L3 target cache setting";
        }
    }
}


TEST_F(CTestCachePolicy, TestL3CachePolicy)
{
    CheckL3CachePolicy();
}


void CTestCachePolicy::CheckLlcEdramCachePolicy()
{
    const uint32_t TargetCache_ELLC        = 0;
    const uint32_t TargetCache_LLC         = 1;
    const uint32_t TargetCache_LLC_ELLC    = 2;
    const uint32_t TargetCache_L3_LLC_ELLC = 2;

    const uint32_t CC_UNCACHED  = 0x1;
    const uint32_t CC_CACHED_WT = 0x2;
    const uint32_t CC_CACHED_WB = 0x3;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT    ClientRequest = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        MEMORY_OBJECT_CONTROL_STATE Mocs          = ClientRequest.MemoryObjectOverride;

        // Check for age
        EXPECT_EQ(ClientRequest.AGE, Mocs.Gen8.Age) << "Usage# " << Usage << ": Incorrect AGE settings";

        if(ClientRequest.L3)
        {
            continue;
        }

        if(!ClientRequest.LLC && !ClientRequest.ELLC) // Uncached
        {
            EXPECT_EQ(CC_UNCACHED, Mocs.Gen8.CacheControl) << "Usage# " << Usage << ": Incorrect cache control setting";
        }
        else
        {
            if(ClientRequest.WT) // Write-through
            {
                EXPECT_EQ(CC_CACHED_WT, Mocs.Gen8.CacheControl) << "Usage# " << Usage << ": Incorrect cache control setting";
            }
            else // Write-back
            {
                EXPECT_EQ(CC_CACHED_WB, Mocs.Gen8.CacheControl) << "Usage# " << Usage << ": Incorrect cache control setting";
            }

            if(ClientRequest.LLC && !ClientRequest.ELLC) // LLC only
            {
                EXPECT_EQ(TargetCache_LLC, Mocs.Gen8.TargetCache) << "Usage# " << Usage << ": Incorrect target cache setting";
            }
            else if(!ClientRequest.LLC && ClientRequest.ELLC) // eLLC only
            {
                EXPECT_EQ(TargetCache_ELLC, Mocs.Gen8.TargetCache) << "Usage# " << Usage << ": Incorrect target cache setting";
            }
            else if(ClientRequest.LLC && ClientRequest.ELLC)
            {
                EXPECT_EQ(TargetCache_LLC_ELLC, Mocs.Gen8.TargetCache) << "Usage# " << Usage << ": Incorrect target cache setting";
            }
        }
    }
}

TEST_F(CTestCachePolicy, TestLlcEdramCachePolicy)
{
    CheckLlcEdramCachePolicy();
}
