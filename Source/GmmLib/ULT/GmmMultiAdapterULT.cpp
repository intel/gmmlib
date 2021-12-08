/*========================== begin_copyright_notice ============================
Copyright(c) 2021 Intel Corporation

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
============================= end_copyright_notice ===========================*/
#include "GmmMultiAdapterULT.h"
#ifndef _WIN32
#include <dlfcn.h>
#endif

ADAPTER_INFO *      MACommonULT::pGfxAdapterInfo[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
PLATFORM            MACommonULT::GfxPlatform[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_LIB_CONTEXT *   MACommonULT::pLibContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_CLIENT_CONTEXT *MACommonULT::pGmmULTClientContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
PFNGMMINIT          MACommonULT::pfnGmmInit[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
PFNGMMDESTROY       MACommonULT::pfnGmmDestroy[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_INIT_IN_ARGS    MACommonULT::InArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_INIT_OUT_ARGS   MACommonULT::OutArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_DLL_HANDLE      MACommonULT::hGmmLib[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];

void MACommonULT::SetUpTestCase()
{
}

void MACommonULT::TearDownTestCase()
{
}

CTestMA::CTestMA()
{
}

CTestMA::~CTestMA()
{
}

void CTestMA::SetUpTestCase()
{
}

void CTestMA::TearDownTestCase()
{
}

void MACommonULT::LoadGmmDll(uint32_t AdapterIdx, uint32_t CountIdx)
{
    hGmmLib[AdapterIdx][CountIdx] = dlopen(GMM_UMD_DLL, RTLD_LAZY);
    ASSERT_TRUE(hGmmLib[AdapterIdx][CountIdx]);

    *(void **)(&pfnGmmInit[AdapterIdx][CountIdx])    = dlsym(hGmmLib[AdapterIdx][CountIdx], "InitializeGmm");
    *(void **)(&pfnGmmDestroy[AdapterIdx][CountIdx]) = dlsym(hGmmLib[AdapterIdx][CountIdx], "GmmAdapterDestroy");

    ASSERT_TRUE(pfnGmmInit[AdapterIdx][CountIdx]);
    ASSERT_TRUE(pfnGmmDestroy[AdapterIdx][CountIdx]);
}

void MACommonULT::UnLoadGmmDll(uint32_t AdapterIdx, uint32_t CountIdx)
{
    if(hGmmLib[AdapterIdx][CountIdx])
    {
        dlclose(hGmmLib[AdapterIdx][CountIdx]);
        hGmmLib[AdapterIdx][CountIdx] = NULL;
    }
}

void MACommonULT::GmmInitModule(uint32_t AdapterIdx, uint32_t CountIdx)
{
    GMM_STATUS  Status                                  = GMM_SUCCESS;
    ADAPTER_BDF AdapterBDF                              = GetAdapterBDF(AdapterIdx);
    GfxPlatform[AdapterIdx][CountIdx].eProductFamily    = GetProductFamily(AdapterIdx);
    GfxPlatform[AdapterIdx][CountIdx].eRenderCoreFamily = GetRenderCoreFamily(AdapterIdx);

    if(!pGfxAdapterInfo[AdapterIdx][CountIdx])
    {
        pGfxAdapterInfo[AdapterIdx][CountIdx] = (ADAPTER_INFO *)malloc(sizeof(ADAPTER_INFO));
        if(!pGfxAdapterInfo[AdapterIdx][CountIdx])
        {
            ASSERT_TRUE(false);
            return;
        }
        memset(pGfxAdapterInfo[AdapterIdx][CountIdx], 0, sizeof(ADAPTER_INFO));
        pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrTileY = 1; // Legacy TileY
	
	if(GfxPlatform[AdapterIdx][CountIdx].eRenderCoreFamily >= IGFX_GEN12_CORE)
        {

            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrWddm2GpuMmu              = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrUserModeTranslationTable = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrE2ECompression           = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrLinearCCS                = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrTileY                    = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrLLCBypass                = 0;
	    pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrWddm2Svm                 = 0;
        }

        if(IGFX_DG1 == GfxPlatform[AdapterIdx][CountIdx].eProductFamily)
        {
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrLinearCCS             = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrStandardMipTailFormat = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrTileY                 = 0;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrLocalMemory           = 1;
            pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable.FtrWddm2_1_64kbPages     = 1;
        }

    }

    InArgs[AdapterIdx][CountIdx].ClientType = GetClientType(CountIdx);
    InArgs[AdapterIdx][CountIdx].pGtSysInfo = &pGfxAdapterInfo[AdapterIdx][CountIdx]->SystemInfo;
    InArgs[AdapterIdx][CountIdx].pSkuTable  = &pGfxAdapterInfo[AdapterIdx][CountIdx]->SkuTable;
    InArgs[AdapterIdx][CountIdx].pWaTable   = &pGfxAdapterInfo[AdapterIdx][CountIdx]->WaTable;
    InArgs[AdapterIdx][CountIdx].Platform   = GfxPlatform[AdapterIdx][CountIdx];
#if LHDM
    InArgs[AdapterIdx][CountIdx].stAdapterBDF       = AdapterBDF;
    InArgs[AdapterIdx][CountIdx].DeviceRegistryPath = NULL;
#else
    InArgs[AdapterIdx][CountIdx].FileDescriptor = AdapterBDF.Data;
#endif

    Status = pfnGmmInit[AdapterIdx][CountIdx](&InArgs[AdapterIdx][CountIdx], &OutArgs[AdapterIdx][CountIdx]);
    EXPECT_EQ(Status, GMM_SUCCESS);

    pGmmULTClientContext[AdapterIdx][CountIdx] = OutArgs[AdapterIdx][CountIdx].pGmmClientContext;
    ASSERT_TRUE(pGmmULTClientContext[AdapterIdx][CountIdx]);

    pLibContext[AdapterIdx][CountIdx] = pGmmULTClientContext[AdapterIdx][CountIdx]->GetLibContext();
    ASSERT_TRUE(pLibContext[AdapterIdx][CountIdx]);
}

void MACommonULT::GmmDestroyModule(uint32_t AdapterIdx, uint32_t CountIdx)
{
    OutArgs[AdapterIdx][CountIdx].pGmmClientContext = pGmmULTClientContext[AdapterIdx][CountIdx];
    pfnGmmDestroy[AdapterIdx][CountIdx](&OutArgs[AdapterIdx][CountIdx]);
    pGmmULTClientContext[AdapterIdx][CountIdx] = NULL;
    pLibContext[AdapterIdx][CountIdx]          = NULL;
    if(pGfxAdapterInfo[AdapterIdx][CountIdx])
    {
        free(pGfxAdapterInfo[AdapterIdx][CountIdx]);
        pGfxAdapterInfo[AdapterIdx][CountIdx] = NULL;
    }
}

void *Thread1(void *lpParam)
{
    ThreadInParams *pInParams = (ThreadInParams *)(lpParam);

    MACommonULT::LoadGmmDll(pInParams->AdapterIdx, pInParams->CountIdx);
    MACommonULT::GmmInitModule(pInParams->AdapterIdx, pInParams->CountIdx);
    MACommonULT::GmmDestroyModule(pInParams->AdapterIdx, pInParams->CountIdx);
    MACommonULT::UnLoadGmmDll(pInParams->AdapterIdx, pInParams->CountIdx);

    pthread_exit(NULL);
}


#if GMM_LIB_DLL_MA

/// Load multiple Adapters in the same process
TEST_F(CTestMA, TestLoadMultipleAdapters)
{
    LoadGmmDll(0, 0);
    LoadGmmDll(1, 0);
    LoadGmmDll(2, 0);

    GmmInitModule(2, 0);
    GmmInitModule(0, 0);
    GmmInitModule(1, 0);

    EXPECT_NE(pLibContext[0][0], pLibContext[1][0]);
    EXPECT_NE(pLibContext[2][0], pLibContext[1][0]);
    EXPECT_NE(pLibContext[2][0], pLibContext[0][0]);

    GmmDestroyModule(1, 0);
    GmmDestroyModule(2, 0);
    GmmDestroyModule(0, 0);

    UnLoadGmmDll(2, 0);
    UnLoadGmmDll(1, 0);
    UnLoadGmmDll(0, 0);
}

/// Load same Adapter multiple times in same process
TEST_F(CTestMA, TestLoadAdapterMultipleTimes)
{
    LoadGmmDll(0, 0);
    LoadGmmDll(0, 1);
    LoadGmmDll(0, 2);

    GmmInitModule(0, 0);
    GmmInitModule(0, 1);
    GmmInitModule(0, 2);

    EXPECT_EQ(pLibContext[0][0], pLibContext[0][2]);
    EXPECT_EQ(pLibContext[0][0], pLibContext[0][1]);

    GmmDestroyModule(0, 0);
    GmmDestroyModule(0, 1);
    GmmDestroyModule(0, 2);

    UnLoadGmmDll(0, 0);
    UnLoadGmmDll(0, 1);
    UnLoadGmmDll(0, 2);
}

/// Test Init-Destroy multiple times before Unloading DLL, on Same Adapter
TEST_F(CTestMA, TestInitDestroyMultipleTimesOnSameAdapter)
{
    uint32_t i = 0;
    LoadGmmDll(0, 0);
    LoadGmmDll(0, 1);
    LoadGmmDll(0, 2);

    for(i = 0; i < 3; i++)
    {
        GmmInitModule(0, 0);
        GmmInitModule(0, 1);
        GmmInitModule(0, 2);

        EXPECT_EQ(pLibContext[0][0], pLibContext[0][2]);
        EXPECT_EQ(pLibContext[0][0], pLibContext[0][1]);

        GmmDestroyModule(0, 0);
        GmmDestroyModule(0, 1);
        GmmDestroyModule(0, 2);
    }

    UnLoadGmmDll(0, 0);
    UnLoadGmmDll(0, 1);
    UnLoadGmmDll(0, 2);
}

/// Test Init-Destroy multiple times before Unloading DLL, on Multiple Adapters
TEST_F(CTestMA, TestInitDestroyMultipleTimesOnMultiAdapter)
{
    uint32_t i = 0;
    LoadGmmDll(0, 0);
    LoadGmmDll(1, 0);
    LoadGmmDll(2, 0);

    for(i = 0; i < 3; i++)
    {
        GmmInitModule(0, 0);
        GmmInitModule(1, 0);
        GmmInitModule(2, 0);

        //EXPECT_NE for Multi-Adapter support is added to Linux
        EXPECT_NE(pLibContext[0][0], pLibContext[1][0]);
        EXPECT_NE(pLibContext[2][0], pLibContext[1][0]);
        EXPECT_NE(pLibContext[2][0], pLibContext[0][0]);

        GmmDestroyModule(2, 0);
        GmmDestroyModule(1, 0);
        GmmDestroyModule(0, 0);
    }

    UnLoadGmmDll(2, 0);
    UnLoadGmmDll(1, 0);
    UnLoadGmmDll(0, 0);
}


// Load Multiple Adapters on multiple threads in same process
TEST_F(CTestMA, TestMTLoadMultipleAdapters)
{
    ThreadInParams InParams[3];
    uint32_t       i = 0;

    InParams[0].AdapterIdx = 0;
    InParams[0].CountIdx   = 0;

    InParams[1].AdapterIdx = 1;
    InParams[1].CountIdx   = 0;

    InParams[2].AdapterIdx = 2;
    InParams[2].CountIdx   = 0;

    int       Status;       /* return value                           */
    pthread_t thread_id[3]; /* thread's ID (just an integer)          */

    for(i = 0; i < 3; i++)
    {
        Status = pthread_create(&thread_id[i], NULL, Thread1, (void *)&InParams[i]);
        ASSERT_TRUE((!Status));
    }

    /* wait for threads to terminate */
    for(i = 0; i < 3; i++)
    {
        Status = pthread_join(thread_id[i], NULL);
        ASSERT_TRUE((!Status));
    }
}

// Load a single Adapter on multiple threads in same process
TEST_F(CTestMA, TestMTLoadAdaptersMultipleTimes)
{
    ThreadInParams InParams[3];
    uint32_t       i = 0;

    InParams[0].AdapterIdx = 0;
    InParams[0].CountIdx   = 0;

    InParams[1].AdapterIdx = 0;
    InParams[1].CountIdx   = 1;

    InParams[2].AdapterIdx = 0;
    InParams[2].CountIdx   = 2;

    int       Status;       /* return value                           */
    pthread_t thread_id[3]; /* thread's ID (just an integer)          */

    for(i = 0; i < 3; i++)
    {
        Status = pthread_create(&thread_id[i], NULL, Thread1, (void *)&InParams[i]);
        ASSERT_TRUE((!Status));
    }

    /* wait for threads to terminate */
    for(i = 0; i < 3; i++)
    {
        Status = pthread_join(thread_id[i], NULL);
        ASSERT_TRUE((!Status));
    }

}

// Load Multiple Adapters, multiple times on multiple threads in same process
TEST_F(CTestMA, TestMTLoadMultipleAdaptersMultipleTimes)
{
    ThreadInParams InParams[9];
    uint32_t       i = 0;

    InParams[0].AdapterIdx = 0;
    InParams[0].CountIdx   = 0;

    InParams[1].AdapterIdx = 1;
    InParams[1].CountIdx   = 0;

    InParams[2].AdapterIdx = 2;
    InParams[2].CountIdx   = 0;

    InParams[3].AdapterIdx = 0;
    InParams[3].CountIdx   = 1;

    InParams[4].AdapterIdx = 1;
    InParams[4].CountIdx   = 1;

    InParams[5].AdapterIdx = 2;
    InParams[5].CountIdx   = 1;

    InParams[6].AdapterIdx = 0;
    InParams[6].CountIdx   = 2;

    InParams[7].AdapterIdx = 1;
    InParams[7].CountIdx   = 2;

    InParams[8].AdapterIdx = 2;
    InParams[8].CountIdx   = 2;

    int       Status;       /* return value                           */
    pthread_t thread_id[9]; /* thread's ID (just an integer)          */

    for(i = 0; i < 9; i++)
    {
        Status = pthread_create(&thread_id[i], NULL, Thread1, (void *)&InParams[i]);
        ASSERT_TRUE((!Status));
    }

    /* wait for threads to terminate */
    for(i = 0; i < 9; i++)
    {
        Status = pthread_join(thread_id[i], NULL);
        ASSERT_TRUE((!Status));
    }

}

#endif // GMM_LIB_DLL_MA

