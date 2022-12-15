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
#include <stdlib.h>

ADAPTER_INFO *      MACommonULT::pGfxAdapterInfo[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
PLATFORM            MACommonULT::GfxPlatform[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_LIB_CONTEXT *   MACommonULT::pLibContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_CLIENT_CONTEXT *MACommonULT::pGmmULTClientContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
PFNGMMINIT          MACommonULT::pfnGmmInit[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
PFNGMMDESTROY       MACommonULT::pfnGmmDestroy[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_INIT_IN_ARGS    MACommonULT::InArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_INIT_OUT_ARGS   MACommonULT::OutArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
GMM_DLL_HANDLE      MACommonULT::hGmmLib[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];

// Comman array for all thread related ULTs to supply Adapter-Client Context input.
ThreadInParams MACommonULT::InParams[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER];

// Array to store the adapter BDFs from simulated UMD, Save the adapter bdf in an array.
ADAPTER_BDF MACommonULT::AdpaterSaved[MAX_NUM_ADAPTERS];

MACommonULT::MACommonULT()
{
}

MACommonULT::~MACommonULT()
{
}

void MACommonULT::SetUpTestCase()
{
}

void MACommonULT::TearDownTestCase()
{
}

CTestMA::CTestMA()
{
    uint32_t i = 0;

    memset(AdpaterSaved, 0, sizeof(AdpaterSaved));
    // Save the random Generated bdf value in an array during intantiation itself
    // These value remain live for each of the ULT lifetime.
    for(i = 0; i < MAX_NUM_ADAPTERS; i++)
    {
        AdpaterSaved[i].Bus      = rand() / 100;
        AdpaterSaved[i].Device   = rand() / 100;
        AdpaterSaved[i].Function = rand() / 100;
        AdpaterSaved[i].Reserved = 0;
    }

    // Validate the generated BDfs are unique
    // No two Adapter's BDF should be equal on a PCI bus.
    for(i = 0; i < MAX_NUM_ADAPTERS; i++)
    {
        for(uint32_t j = 0; j < MAX_NUM_ADAPTERS; j++)
        {
            if(i != j)
            {
                if(AdpaterSaved[i].Bus == AdpaterSaved[j].Bus)
                {
                    if(AdpaterSaved[i].Device == AdpaterSaved[j].Device)
                    {
                        if(AdpaterSaved[i].Function == AdpaterSaved[j].Function)
                        {
                            // OOps! Generated BDFs are equal.
                            // Lets change any one field to make it unique
                            // Lets increment Bus.
                            AdpaterSaved[j].Bus++;
                        }
                    }
                }
            }
        }
    }
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
    ASSERT_TRUE(AdapterIdx < MAX_NUM_ADAPTERS);
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

// This member function creates MaxClientThreads number of threads
// MaxClientsThreads to represent total numbers of UMD Clients
// MaxClientsThreads = MAX_NUM_ADAPTERS for ULT TestMTLoadMultipleAdapters
// MaxClientsThreads = MAX_COUNT_PER_ADAPTER for ULT TestMTLoadAdaptersMultipleTimes
// MaxClientsThreads = MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER, for ULT TestMTLoadMultipleAdaptersMultipleTimes
void MACommonULT::CreateMAThread(uint32_t MaxClientThreads)
{
    // Spawn all threads upto MaxClientThreads and wait for them all at once
    uint32_t  i = 0;
    int       Status;                                              /* return value                           */
    pthread_t thread_id[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER]; /* thread's ID (just an integer)          */

    /* MaxClientsThreads to represent MAX_NUM_ADAPTERS *MAX_COUNT_PER_ADAPTER Clients */
    for(i = 0; i < (MaxClientThreads); i++)
    {
        Status = pthread_create(&thread_id[i], NULL, Thread1, (void *)&InParams[i]);
        ASSERT_TRUE((!Status));
    }

    /* wait for threads to terminate */
    for(i = 0; i < (MaxClientThreads); i++)
    {
        Status = pthread_join(thread_id[i], NULL);
        ASSERT_TRUE((!Status));
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
/*
 To simulate the real time scenario between the Gmmlib and the UMD clients
 Folowing ULTs assume:
 MAX_NUM_ADAPTERS = Number of GPU Adapters (BDFs) available on a system at a given point of time.
 MAX_COUNT_PER_ADAPTER = Number of UMD clients that can be simulated per Adapter
 So Total clients simulated = MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER
 Where,
 The LibContext for an Adapter is equal across all Clients for that adapter
 The ClientConetxt is unique(Not equal) across all Clients for that adapter
*/

// Load multiple Adapters in the same process with the Limit up to MAX_NUM_ADAPTERS
// Increase MAX_NUM_ADAPTERS > 32 if needed
TEST_F(CTestMA, DISABLED_TestLoadMultipleAdapters)
{
    uint32_t AdapterCount = 0;
    uint32_t i            = 0;

    // Load the dll for all available adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        LoadGmmDll(AdapterCount, 0);
    }

    // Initilize the dll for all available adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        GmmInitModule(AdapterCount, 0);
    }

    // Check the Libcontext for each of the adapter is different or not
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(i = 0; i < MAX_NUM_ADAPTERS; i++)
        {
            if(AdapterCount != i)
            {
                EXPECT_NE(pLibContext[AdapterCount][0], pLibContext[i][0]);
            }
        }
    }

    // Un-Initilize the dll for all loaded adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        GmmDestroyModule(AdapterCount, 0);
    }

    // Unload the dll for all loaded adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        UnLoadGmmDll(AdapterCount, 0);
    }
}

/// Load all adapters(MAX_NUM_ADAPTERS) multiple times up to MAX_COUNT_PER_ADAPTER in same process
TEST_F(CTestMA, DISABLED_TestLoadAdapterMultipleTimes)
{
    uint32_t AdapterCount = 0, RefCount = 0;

    // Load the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            LoadGmmDll(AdapterCount, RefCount);
        }
    }

    // Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmInitModule(AdapterCount, RefCount);
        }
    }
    // For each adapter upto MAX_NUM_ADAPTERS Check the LibContext for all instances upto
    // MAX_COUNT_PER_ADAPTER to be equal
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER - 1; RefCount++)
        {
            EXPECT_EQ(pLibContext[AdapterCount][RefCount], pLibContext[AdapterCount][RefCount + 1]);
        }
    }

    // Un-Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    // The destroy/unload can be out of order
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmDestroyModule(AdapterCount, RefCount);
        }
    }

    // Unload the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            UnLoadGmmDll(AdapterCount, RefCount);
        }
    }
}

/// Test Init-Destroy multiple times Upto MAX_COUNT_PER_ADAPTER before Unloading DLL, on Same Adapter upto MAX_NUM_ADAPTERS
TEST_F(CTestMA, DISABLED_TestInitDestroyMultipleTimesOnSameAdapter)
{
    uint32_t AdapterCount = 0, RefCount = 0;

    // Load the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            LoadGmmDll(AdapterCount, RefCount);
        }
    }

    // Initilize and destroy module upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    // For each adapter(AdapterCount <  MAX_NUM_ADAPTERS) Check the LibContext for all instances to be equal
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        // Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each adapter
        // In reality the init and destroy can occurs any number of time on a particular adapter, so for simplcity treating that UMD
        // will load already loaded lib again for MAX_COUNT_PER_ADAPTER times.
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmInitModule(AdapterCount, RefCount);
        }

        // Check the LibContext for all instances on a same adapter to be equal
        // It might also seems that LibContext pointer value on next adapters is same as previous pointer value returned in previous adapter init.
        // This is the OS Memory Manager's role to avoid fragmentation in the process VA space
        // Also our ULT is a Light-Weight process due to which the freed memory not assigned to other processes may get assigned again.
        // But mind that this is possible only when the previous libcontext intialized is compulsorily inactive and destroyed.
        // otherwise the same secnario as in TestLoadMultipleAdapters occurs .i.e different pointer value is returned on new adpater bdf.
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER - 1; RefCount++)
        {
            EXPECT_EQ(pLibContext[AdapterCount][RefCount], pLibContext[AdapterCount][RefCount + 1]);
        }

        // Un-Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each adapter
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmDestroyModule(AdapterCount, RefCount);
        }
    }

    // Unload the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            UnLoadGmmDll(AdapterCount, RefCount);
        }
    }
}

/// Test Init-Destroy multiple times before Unloading DLL, on Multiple Adapters
TEST_F(CTestMA, DISABLED_TestInitDestroyMultipleTimesOnMultiAdapter)
{
    uint32_t AdapterCount = 0, RefCount = 0;

    // Load the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            LoadGmmDll(AdapterCount, RefCount);
        }
    }

    // Initilize and destroy the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    // For each adapter(AdapterCount <  MAX_NUM_ADAPTERS) Check the LibContext for all instances to is unique
    // This is similar to TestInitDestroyMultipleTimesOnSameAdapter ULT apart from the order of adapter initialization.
    for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
    {
        // Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each adapter
        for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
        {
            GmmInitModule(AdapterCount, RefCount);
        }
        // Check the LibContext for each of the adpater(upto MAX_NUM_ADAPTERS) to be unique
        // whereas LibContext for all instances on a same adapter is to be equal
        for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS - 1; AdapterCount++)
        {
            EXPECT_NE(pLibContext[AdapterCount][RefCount], pLibContext[AdapterCount + 1][RefCount]);
        }

        for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
        {
            GmmDestroyModule(AdapterCount, RefCount);
        }
    }

    // Unload the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            UnLoadGmmDll(AdapterCount, RefCount);
        }
    }
}

/*
Following ULT's Exhibit the multitasking behaviour of UMDs considering that all UMDs loads and unloads dll
in parallel and in random order.
*/

// Load Multiple Adapters upto MAX_NUM_ADAPTERS on multiple threads in same process at the same time
// Here the number of client per adapter is 1 .i.e 0th count Index
TEST_F(CTestMA, DISABLED_TestMTLoadMultipleAdapters)
{
    uint32_t AdapterCount = 0;

    memset(InParams, 0, sizeof(InParams));

    //Populate the Inparams array with the MAX_NUM_ADAPTERS indices
    for(AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        InParams[AdapterCount].AdapterIdx = AdapterCount;
        InParams[AdapterCount].CountIdx   = 0;
    }

    // Create threads to load all Adapters upto MAX_NUM_ADAPTERS for a single client each
    CreateMAThread(MAX_NUM_ADAPTERS);
}

// Load a Single Adapter multiple times upto MAX_COUNT_PER_ADAPTER on multiple threads in same process
TEST_F(CTestMA, DISABLED_TestMTLoadAdaptersMultipleTimes)
{
    uint32_t RefCount = 0;

    memset(InParams, 0, sizeof(InParams));

    //Populate the Inparams array with MAX_COUNT_PER_ADAPTER indices
    for(RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
    {
        InParams[RefCount].AdapterIdx = 0;
        InParams[RefCount].CountIdx   = RefCount;
    }
    // Create threads to load all clients i.e MAX_COUNT_PER_ADAPTER on single adpater
    CreateMAThread(MAX_COUNT_PER_ADAPTER);
}

// Load Multiple Adapters upto MAX_NUM_ADAPTERS, multiple times upto MAX_COUNT_PER_ADAPTER on multiple threads in same process
TEST_F(CTestMA, DISABLED_TestMTLoadMultipleAdaptersMultipleTimes)
{
    uint32_t i = 0, j = 0, k = 0;
    uint32_t AdapterCount = 0, RefCount = 0;

    memset(InParams, 0, sizeof(InParams));

    //Populate the Inparams array with the MAX_NUM_ADAPTERS*MAX_COUNT_PER_ADAPTER indices
    //Such that Each Adapter corresponds to its max mumber of clients in a sequential order
    for(i = 0; i < (MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER); i++)
    {
        for(j = 0; j < MAX_NUM_ADAPTERS; j++)
        {
            for(k = 0; k < MAX_COUNT_PER_ADAPTER; k++)
            {
                InParams[i].AdapterIdx = AdapterCount;
                InParams[i].CountIdx   = RefCount++;
                i++;
            }
            RefCount = 0;
            AdapterCount++;
        }
    }

    // Create threads to load MAX_NUM_ADAPTERS, MAX_COUNT_PER_ADAPTER times
    // Thread Count = MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER
    CreateMAThread(MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER);
}

#endif // GMM_LIB_DLL_MA

