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

#include "GmmGen12dGPUCachePolicyULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Cache Policy fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
/// It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen12dGPUCachePolicy::SetUpTestCase()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen12dGPUCachePolicy::TearDownTestCase()
{
}

void CTestGen12dGPUCachePolicy::SetUpGen12dGPUVariant(PRODUCT_FAMILY platform)
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
        pGfxAdapterInfo->SkuTable.FtrLocalMemory           = 1;
        CommonULT::SetUpTestCase();
    }
}

void CTestGen12dGPUCachePolicy::TearDownGen12dGPUVariant()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

TEST_F(CTestGen12dGPUCachePolicy, TestGen12dGPU_DG1CachePolicy)
{
    SetUpGen12dGPUVariant(IGFX_DG1);

    CheckL3Gen12dGPUCachePolicy();

    TearDownGen12dGPUVariant();
}

TEST_F(CTestGen12dGPUCachePolicy, TestGen12dGPU_XE_HP_SDVCachePolicy)
{
    SetUpGen12dGPUVariant(IGFX_XE_HP_SDV);

    CheckL3Gen12dGPUCachePolicy();

    TearDownGen12dGPUVariant();
}

void CTestGen12dGPUCachePolicy::CheckSpecialMocs(uint32_t                    Usage,
                                               uint32_t                    AssignedMocsIdx,
                                               GMM_CACHE_POLICY_ELEMENT ClientRequest)
{
    if(Usage == GMM_RESOURCE_USAGE_CCS) //60
    {
        EXPECT_EQ(AssignedMocsIdx, 60) << "Usage# " << Usage << ": Incorrect Index for CCS";
        EXPECT_EQ(0, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for CCS";
        EXPECT_EQ(0, ClientRequest.UcLookup) << "Usage# " << Usage << ": Incorrect L3 LookUp cacheability for CCS";
    }
    else if(Usage == GMM_RESOURCE_USAGE_CCS_MEDIA_WRITABLE) // 61
    {
        EXPECT_EQ(AssignedMocsIdx, 61) << "Usage# " << Usage << ": Incorrect Index for CCS";
        EXPECT_EQ(0, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for CCS";
        EXPECT_EQ(0, ClientRequest.UcLookup) << "Usage# " << Usage << ": Incorrect L3 LookUp cacheability for CCS";
    }
    else if(Usage == GMM_RESOURCE_USAGE_MOCS_62) //62
    {
        EXPECT_EQ(AssignedMocsIdx, 62) << "Usage# " << Usage << ": Incorrect Index for MOCS_62";
        EXPECT_EQ(0, ClientRequest.L3) << "Usage# " << Usage << ": Incorrect L3 cacheability for MOCS#62";
    }
}

void CTestGen12dGPUCachePolicy::CheckMocsIdxHDCL1(uint32_t                    Usage,
                                                uint32_t                    AssignedMocsIdx,
                                                GMM_CACHE_POLICY_ELEMENT ClientRequest)
{
#define GMM_GEN12_MAX_NUMBER_MOCS_INDEXES (60) // On TGL last four (#60-#63) are reserved by h/w, few? are sw configurable though (#60)

    // Check of assigned Index setting is appropriate for HDCL1 setting
    if(ClientRequest.HDCL1)
    {
        EXPECT_GE(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Index must be greater than " << GMM_GEN10_HDCL1_MOCS_INDEX_START - 1 << " for HDCL1 setting. AssignedMocs: " << AssignedMocsIdx;
        EXPECT_LT(AssignedMocsIdx, GMM_GEN12_MAX_NUMBER_MOCS_INDEXES) << "Usage# " << Usage << ": Index must be less than " << GMM_GEN12_MAX_NUMBER_MOCS_INDEXES << "for  HDCL1 setting";
    }
    else if(AssignedMocsIdx < GMM_GEN12_MAX_NUMBER_MOCS_INDEXES)
    {
        EXPECT_LT(AssignedMocsIdx, GMM_GEN10_HDCL1_MOCS_INDEX_START) << "Usage# " << Usage << ": Incorrect Index for HDCL1 setting";
    }
}

void CTestGen12dGPUCachePolicy::CheckL3Gen12dGPUCachePolicy()
{
    const uint32_t L3_WB_CACHEABLE = 0x3;
    const uint32_t L3_UNCACHEABLE  = 0x1;

    // Check Usage MOCS index against MOCS settings
    for(uint32_t Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT     ClientRequest   = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        uint32_t                        AssignedMocsIdx = ClientRequest.MemoryObjectOverride.Gen12.Index;
        GMM_CACHE_POLICY_TBL_ELEMENT Mocs            = pGmmULTClientContext->GetCachePolicyTlbElement(AssignedMocsIdx);
        uint32_t                     StartMocsIdx    = 1;

        //printf("Usage: %d --> Index: [%d]\n", Usage, AssignedMocsIdx);

	if(GfxPlatform.eProductFamily == IGFX_DG2)
        {
            StartMocsIdx = 0;
        }
        if(StartMocsIdx == 1)
        {
            EXPECT_NE(0, AssignedMocsIdx) << "Usage# " << Usage << ": Misprogramming MOCS - Index 0 is reserved for Error";
        }

        EXPECT_EQ(0, Mocs.L3.ESC) << "Usage# " << Usage << ": ESC is non-zero";
        EXPECT_EQ(0, Mocs.L3.SCC) << "Usage# " << Usage << ": SCC is non-zero";
        EXPECT_EQ(0, Mocs.L3.Reserved) << "Usage# " << Usage << ": Reserved field is non-zero";
        // Check if Mocs Index is not greater than GMM_MAX_NUMBER_MOCS_INDEXES
        EXPECT_GT(GMM_MAX_NUMBER_MOCS_INDEXES, AssignedMocsIdx) << "Usage# " << Usage << ": MOCS Index greater than MAX allowed (63)";

        if(GfxPlatform.eProductFamily <= IGFX_XE_HP_SDV)
        {
            CheckMocsIdxHDCL1(Usage, AssignedMocsIdx, ClientRequest);
        }

        if(GfxPlatform.eProductFamily < IGFX_DG2)
	{
            CheckSpecialMocs(Usage, AssignedMocsIdx, ClientRequest);
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

void CTestXe_HP_CachePolicy::SetUpPlatformVariant(PRODUCT_FAMILY platform)
{
    printf("%s\n", __FUNCTION__);
    CTestGen12dGPUCachePolicy::SetUpGen12dGPUVariant(platform);
}

void CTestXe_HP_CachePolicy::TearDownPlatformVariant()
{
    printf("%s\n", __FUNCTION__);
    CTestGen12dGPUCachePolicy::TearDownGen12dGPUVariant();
}

void CTestXe_HP_CachePolicy::CheckL3CachePolicy()
{
    printf("%s\n", __FUNCTION__);
    CTestGen12dGPUCachePolicy::CheckL3Gen12dGPUCachePolicy();
}

void CTestXe_HP_CachePolicy::SetUpTestCase()
{
}

void CTestXe_HP_CachePolicy::TearDownTestCase()
{
}

TEST_F(CTestXe_HP_CachePolicy, Test_DG2_CachePolicy)
{
    SetUpPlatformVariant(IGFX_DG2);
    CheckL3CachePolicy();
    TearDownPlatformVariant();
}

TEST_F(CTestXe_HP_CachePolicy, Test_PVC_CachePolicy)
{
    SetUpPlatformVariant(IGFX_PVC);
    CheckL3CachePolicy();
    CheckPAT();
    TearDownPlatformVariant();
}

void CTestXe_HP_CachePolicy::CheckPAT()
{
    // Check Usage PAT index against PAT settings
    for(unsigned long Usage = GMM_RESOURCE_USAGE_UNKNOWN; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        GMM_CACHE_POLICY_ELEMENT ClientRequest = pGmmULTClientContext->GetCachePolicyElement((GMM_RESOURCE_USAGE_TYPE)Usage);
        if(ClientRequest.Initialized == false) // undefined resource in platform
        {
            continue;
        }
        unsigned long PATIndex = pGmmULTClientContext->CachePolicyGetPATIndex(NULL, (GMM_RESOURCE_USAGE_TYPE)Usage, NULL, false); 
        //printf("Xe HPG: Usage: %d --> PAT Index: [%d]\n", Usage, PATIndex);
        EXPECT_NE(PATIndex, GMM_PAT_ERROR) << "Usage# " << Usage << ": No matching PAT Index";
    }
}
