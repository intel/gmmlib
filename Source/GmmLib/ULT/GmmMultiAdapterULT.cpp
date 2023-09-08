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

MACommonULT::MACommonULT()
{
    // Ideally this should be in the SetUp function for each test case, but since there are
    // wrapper classes around this for the test suites and SetUp does not get called on this
    // class, it needs to go in the constructor which gets created for each test case.

    //reset all test info
    memset(GmmTestInfo, 0, sizeof(GmmTestInfo));
    memset(AdapterSaved, 0, sizeof(AdapterSaved));

    hGmmLib       = NULL;
    pfnGmmDestroy = NULL;
    pfnGmmInit    = NULL;

    //set a seed value so the test is always reproducible
    srand(0);

    // Save the random Generated bdf value in an array during intantiation itself
    // These value remain live for each of the ULT lifetime.
    for (int i = 0; i < MAX_NUM_ADAPTERS; i++)
    {
        AdapterSaved[i].Bus      = rand() / 100;
        AdapterSaved[i].Device   = rand() / 100;
        AdapterSaved[i].Function = rand() / 100;
        AdapterSaved[i].Reserved = 0;
    }

    // Validate the generated BDfs are unique
    // No two Adapter's BDF should be equal on a PCI bus.
    for (int i = 0; i < MAX_NUM_ADAPTERS; i++)
    {
        for (int j = 0; j < MAX_NUM_ADAPTERS; j++)
        {
            if (i != j)
            {
                if (AdapterSaved[i].Bus == AdapterSaved[j].Bus)
                {
                    if (AdapterSaved[i].Device == AdapterSaved[j].Device)
                    {
                        if (AdapterSaved[i].Function == AdapterSaved[j].Function)
                        {
                            // OOps! Generated BDFs are equal.
                            // Lets change any one field to make it unique
                            // Lets increment Bus.
                            AdapterSaved[j].Bus++;
                        }
                    }
                }
            }
        }
    }
}

MACommonULT::~MACommonULT()
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

void CTestMA::SetUp()
{
    LoadGmmDll();
}

void CTestMA::TearDown()
{
    UnLoadGmmDll();
}

void MACommonULT::LoadGmmDll()
{
    hGmmLib = dlopen(GMM_UMD_DLL, RTLD_LAZY);
    ASSERT_TRUE(hGmmLib);

    *(void **)(&pfnGmmInit)    = dlsym(hGmmLib, "InitializeGmm");
    *(void **)(&pfnGmmDestroy) = dlsym(hGmmLib, "GmmAdapterDestroy");

    ASSERT_TRUE(pfnGmmInit);
    ASSERT_TRUE(pfnGmmDestroy);
}

void MACommonULT::UnLoadGmmDll()
{
    if (hGmmLib)
    {	    
        dlclose(hGmmLib);

        hGmmLib       = NULL;
        pfnGmmInit    = NULL;
        pfnGmmDestroy = NULL;
    }
}

// Lets test with the recent GPUs till 32nd adapters and take IGFX_COFFEELAKE for 32+ adapters
// Only for our ULT to supply dummy ProductFamily
PRODUCT_FAMILY MACommonULT::GetProductFamily(uint32_t AdapterIdx)
{
    switch (AdapterIdx)
    {
    case 0:
        return IGFX_DG1;
    case 1:
        return IGFX_ICELAKE;
    case 2:
        return IGFX_TIGERLAKE_LP;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        return IGFX_DG2;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        return IGFX_XE_HP_SDV;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
        return IGFX_PVC;
    case 30:
    case 31:
    default:
        break;
    }
    return IGFX_COFFEELAKE;
}

// Lets test with the recent GPUs till 32nd adpater and take IGFX_GEN9_CORE for 32+ adapters
// Only for our ULT to supply dummy GFXCORE_FAMILY
GFXCORE_FAMILY MACommonULT::GetRenderCoreFamily(uint32_t AdapterIdx)
{
    switch (AdapterIdx)
    {
    case 0:
        return IGFX_XE_HP_CORE;
    case 1:
        return IGFX_GEN11LP_CORE;
    case 2:
        return IGFX_GEN12LP_CORE;
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        return IGFX_XE_HPG_CORE;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        return IGFX_XE_HP_CORE;
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
        return IGFX_XE_HPC_CORE;
    case 30:
    case 31:
    default:
        break;
    }
    return IGFX_GEN9_CORE;
}

// To simulate the UMDs/ClinentContexts per adapter i.e MAX_COUNT_PER_ADAPTER
// Increase MAX_COUNT_PER_ADAPTER value if there are more UMDs involved
GMM_CLIENT MACommonULT::GetClientType(uint32_t CountIdx)
{
    switch (CountIdx)
    {
    case 0:
        return GMM_D3D9_VISTA;
    case 1:
        return GMM_D3D10_VISTA;
    case 2:
        return GMM_D3D12_VISTA;
    case 3:
        return GMM_EXCITE_VISTA;
    case 4:
        return GMM_OCL_VISTA;
    default:
        break;
    }
    return GMM_D3D9_VISTA;
}

// Returns the AdapterSaved array value, Adapter BDFs based on input AdapterIdx
ADAPTER_BDF MACommonULT::GetAdapterBDF(uint32_t AdapterIdx)
{
    ADAPTER_BDF AdapterBDF = {0, 2, 0, 0};

    if (AdapterIdx < MAX_NUM_ADAPTERS)
    {
        AdapterBDF.Bus      = AdapterSaved[AdapterIdx].Bus;
        AdapterBDF.Device   = AdapterSaved[AdapterIdx].Device;
        AdapterBDF.Function = AdapterSaved[AdapterIdx].Function;
    }
    return AdapterBDF;
}

void MACommonULT::GmmInitModule(uint32_t AdapterIdx, uint32_t CountIdx)
{
    ASSERT_TRUE(AdapterIdx < MAX_NUM_ADAPTERS);
    GMM_STATUS  Status                                              = GMM_SUCCESS;
    ADAPTER_BDF AdapterBDF                                          = GetAdapterBDF(AdapterIdx);
    GmmTestInfo[AdapterIdx][CountIdx].GfxPlatform.eProductFamily    = GetProductFamily(AdapterIdx);
    GmmTestInfo[AdapterIdx][CountIdx].GfxPlatform.eRenderCoreFamily = GetRenderCoreFamily(AdapterIdx);

    if (!GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo)
    {
        GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo = (ADAPTER_INFO *)malloc(sizeof(ADAPTER_INFO));
        if (!GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo)
        {
            ASSERT_TRUE(false);
            return;
        }
        memset(GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo, 0, sizeof(ADAPTER_INFO));

        GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrTileY = 1; // Legacy TileY

        if (GmmTestInfo[AdapterIdx][CountIdx].GfxPlatform.eRenderCoreFamily >= IGFX_GEN12_CORE)
        {

            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrWddm2GpuMmu              = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrUserModeTranslationTable = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrE2ECompression           = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrLinearCCS                = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrTileY                    = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrLLCBypass                = 0;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrWddm2Svm                 = 0; // keep this disabled for pagetablemgr testing
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrSVM                      = 1; // keep this enabled for pagetablemgr testing
            //GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.WaUntypedBufferCompression   = 1;
        }

        if (IGFX_DG1 == GmmTestInfo[AdapterIdx][CountIdx].GfxPlatform.eProductFamily)
        {
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrLinearCCS             = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrStandardMipTailFormat = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrTileY                 = 0;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrLocalMemory           = 1;
            GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable.FtrWddm2_1_64kbPages     = 1;
        }
    }


    GmmTestInfo[AdapterIdx][CountIdx].InArgs.ClientType = GetClientType(CountIdx);
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.pGtSysInfo = &GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SystemInfo;
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.pSkuTable  = &GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->SkuTable;
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.pWaTable   = &GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo->WaTable;
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.Platform   = GmmTestInfo[AdapterIdx][CountIdx].GfxPlatform;
#if LHDM
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.stAdapterBDF       = AdapterBDF;
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.DeviceRegistryPath = NULL;
#else
    GmmTestInfo[AdapterIdx][CountIdx].InArgs.FileDescriptor = AdapterBDF.Data;
#endif

    Status = pfnGmmInit(&GmmTestInfo[AdapterIdx][CountIdx].InArgs, &GmmTestInfo[AdapterIdx][CountIdx].OutArgs);
    EXPECT_EQ(Status, GMM_SUCCESS);

    GmmTestInfo[AdapterIdx][CountIdx].pGmmULTClientContext = GmmTestInfo[AdapterIdx][CountIdx].OutArgs.pGmmClientContext;
    ASSERT_TRUE(GmmTestInfo[AdapterIdx][CountIdx].pGmmULTClientContext);

    GmmTestInfo[AdapterIdx][CountIdx].pLibContext = GmmTestInfo[AdapterIdx][CountIdx].pGmmULTClientContext->GetLibContext();
    ASSERT_TRUE(GmmTestInfo[AdapterIdx][CountIdx].pLibContext);
}

void MACommonULT::GmmDestroyModule(uint32_t AdapterIdx, uint32_t CountIdx)
{
    GmmTestInfo[AdapterIdx][CountIdx].OutArgs.pGmmClientContext = GmmTestInfo[AdapterIdx][CountIdx].pGmmULTClientContext;
    pfnGmmDestroy(&GmmTestInfo[AdapterIdx][CountIdx].OutArgs);
    GmmTestInfo[AdapterIdx][CountIdx].pGmmULTClientContext = NULL;
    GmmTestInfo[AdapterIdx][CountIdx].pLibContext          = NULL;
    if (GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo)
    {
        free(GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo);
        GmmTestInfo[AdapterIdx][CountIdx].pGfxAdapterInfo = NULL;
    }
}

// This member function creates MaxClientThreads number of threads
// MaxClientsThreads to represent total numbers of UMD Clients
// MaxClientsThreads = MAX_NUM_ADAPTERS for ULT TestMTLoadMultipleAdapters
// MaxClientsThreads = MAX_COUNT_PER_ADAPTER for ULT TestMTLoadAdaptersMultipleTimes
// MaxClientsThreads = MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER, for ULT TestMTLoadMultipleAdaptersMultipleTimes
static void CreateMAThread(uint32_t MaxClientThreads, ThreadInParams *InParams)
{
    // Spawn all threads upto MaxClientThreads and wait for them all at once
    uint32_t  i = 0;
    int       Status;                                              /* return value                           */
    pthread_t thread_id[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER]; /* thread's ID (just an integer)          */

    /* MaxClientsThreads to represent MAX_NUM_ADAPTERS *MAX_COUNT_PER_ADAPTER Clients */
    for (i = 0; i < (MaxClientThreads); i++)
    {
        Status = pthread_create(&thread_id[i], NULL, MAULTThreadEntryFunc, (void *)&InParams[i]);
        ASSERT_TRUE((!Status));
    }

    /* wait for threads to terminate */
    for (i = 0; i < (MaxClientThreads); i++)
    {
        Status = pthread_join(thread_id[i], NULL);
        ASSERT_TRUE((!Status));
    }
}

void *MAULTThreadEntryFunc(void *lpParam)
{
    ThreadInParams *pInParams = (ThreadInParams *)(lpParam);

    pInParams->MATestObj->GmmInitModule(pInParams->AdapterIdx, pInParams->CountIdx);
    pInParams->MATestObj->GmmDestroyModule(pInParams->AdapterIdx, pInParams->CountIdx);

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
TEST_F(CTestMA, TestLoadMultipleAdapters)
{
    uint32_t AdapterCount = 0;
    uint32_t i            = 0;

    // Initilize the dll for all available adapters
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        GmmInitModule(AdapterCount, 0);
    }

    // Check the Libcontext for each of the adapter is different or not
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for (i = 0; i < MAX_NUM_ADAPTERS; i++)
        {
            if (AdapterCount != i)
            {
                EXPECT_NE(GmmTestInfo[AdapterCount][0].pLibContext, GmmTestInfo[i][0].pLibContext);
            }
        }
    }

    // Un-Initilize the dll for all loaded adapters
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        GmmDestroyModule(AdapterCount, 0);
    }
}

/// Load all adapters(MAX_NUM_ADAPTERS) multiple times up to MAX_COUNT_PER_ADAPTER in same process
TEST_F(CTestMA, TestLoadAdapterMultipleTimes)
{
    uint32_t AdapterCount = 0, RefCount = 0;

    // Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmInitModule(AdapterCount, RefCount);
        }
    }
    // For each adapter upto MAX_NUM_ADAPTERS Check the LibContext for all instances upto
    // MAX_COUNT_PER_ADAPTER to be equal
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER - 1; RefCount++)
        {
            EXPECT_EQ(GmmTestInfo[AdapterCount][RefCount].pLibContext, GmmTestInfo[AdapterCount][RefCount + 1].pLibContext);
        }
    }

    // Un-Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    // The destroy/unload can be out of order
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmDestroyModule(AdapterCount, RefCount);
        }
    }
}

/// Test Init-Destroy multiple times Upto MAX_COUNT_PER_ADAPTER before Unloading DLL, on Same Adapter upto MAX_NUM_ADAPTERS
TEST_F(CTestMA, TestInitDestroyMultipleTimesOnSameAdapter)
{
    uint32_t AdapterCount = 0, RefCount = 0;

    // Initilize and destroy module upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    // For each adapter(AdapterCount <  MAX_NUM_ADAPTERS) Check the LibContext for all instances to be equal
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        // Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each adapter
        // In reality the init and destroy can occurs any number of time on a particular adapter, so for simplcity treating that UMD
        // will load already loaded lib again for MAX_COUNT_PER_ADAPTER times.
        for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmInitModule(AdapterCount, RefCount);
        }

        // Check the LibContext for all instances on a same adapter to be equal
        // It might also seems that LibContext pointer value on next adapters is same as previous pointer value returned in previous adapter init.
        // This is the OS Memory Manager's role to avoid fragmentation in the process VA space
        // Also our ULT is a Light-Weight process due to which the freed memory not assigned to other processes may get assigned again.
        // But mind that this is possible only when the previous libcontext intialized is compulsorily inactive and destroyed.
        // otherwise the same secnario as in TestLoadMultipleAdapters occurs .i.e different pointer value is returned on new adpater bdf.
        for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER - 1; RefCount++)
        {
            EXPECT_EQ(GmmTestInfo[AdapterCount][RefCount].pLibContext, GmmTestInfo[AdapterCount][RefCount + 1].pLibContext);
        }

        // Un-Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each adapter
        for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
        {
            GmmDestroyModule(AdapterCount, RefCount);
        }
    }
}

/// Test Init-Destroy multiple times before Unloading DLL, on Multiple Adapters
TEST_F(CTestMA, TestInitDestroyMultipleTimesOnMultiAdapter)
{
    uint32_t AdapterCount = 0, RefCount = 0;

    // Initilize and destroy the dll upto MAX_COUNT_PER_ADAPTER times for each of MAX_NUM_ADAPTERS adapters
    // For each adapter(AdapterCount <  MAX_NUM_ADAPTERS) Check the LibContext for all instances to is unique
    // This is similar to TestInitDestroyMultipleTimesOnSameAdapter ULT apart from the order of adapter initialization.
    for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
    {
        // Initilize the dll upto MAX_COUNT_PER_ADAPTER times for each adapter
        for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
        {
            GmmInitModule(AdapterCount, RefCount);
        }
        // Check the LibContext for each of the adpater(upto MAX_NUM_ADAPTERS) to be unique
        // whereas LibContext for all instances on a same adapter is to be equal
        for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS - 1; AdapterCount++)
        {
            EXPECT_NE(GmmTestInfo[AdapterCount][RefCount].pLibContext, GmmTestInfo[AdapterCount + 1][RefCount].pLibContext);
        }

        for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
        {
            GmmDestroyModule(AdapterCount, RefCount);
        }
    }
}

/*
Following ULT's Exhibit the multitasking behaviour of UMDs considering that all UMDs loads and unloads dll
in parallel and in random order.
*/

// Load Multiple Adapters upto MAX_NUM_ADAPTERS on multiple threads in same process at the same time
// Here the number of client per adapter is 1 .i.e 0th count Index
TEST_F(CTestMA, TestMTLoadMultipleAdapters)
{
    uint32_t       AdapterCount = 0;
    ThreadInParams InParams[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER];

    memset(InParams, 0, sizeof(InParams));

    //Populate the Inparams array with the MAX_NUM_ADAPTERS indices
    for (AdapterCount = 0; AdapterCount < MAX_NUM_ADAPTERS; AdapterCount++)
    {
        InParams[AdapterCount].AdapterIdx = AdapterCount;
        InParams[AdapterCount].CountIdx   = 0;
        InParams[AdapterCount].MATestObj  = this;
    }

    // Create threads to load all Adapters upto MAX_NUM_ADAPTERS for a single client each
    CreateMAThread(MAX_NUM_ADAPTERS, InParams);
}

// Load a Single Adapter multiple times upto MAX_COUNT_PER_ADAPTER on multiple threads in same process
TEST_F(CTestMA, TestMTLoadAdaptersMultipleTimes)
{
    uint32_t       RefCount = 0;
    ThreadInParams InParams[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER];

    memset(InParams, 0, sizeof(InParams));

    //Populate the Inparams array with MAX_COUNT_PER_ADAPTER indices
    for (RefCount = 0; RefCount < MAX_COUNT_PER_ADAPTER; RefCount++)
    {
        InParams[RefCount].AdapterIdx = 0;
        InParams[RefCount].CountIdx   = RefCount;
        InParams[RefCount].MATestObj  = this;
    }
    // Create threads to load all clients i.e MAX_COUNT_PER_ADAPTER on single adpater
    CreateMAThread(MAX_COUNT_PER_ADAPTER, InParams);
}

// Load Multiple Adapters upto MAX_NUM_ADAPTERS, multiple times upto MAX_COUNT_PER_ADAPTER on multiple threads in same process
TEST_F(CTestMA, TestMTLoadMultipleAdaptersMultipleTimes)
{
    uint32_t       i = 0, j = 0, k = 0;
    uint32_t       AdapterCount = 0, RefCount = 0;
    ThreadInParams InParams[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER];

    memset(InParams, 0, sizeof(InParams));

    //Populate the Inparams array with the MAX_NUM_ADAPTERS*MAX_COUNT_PER_ADAPTER indices
    //Such that Each Adapter corresponds to its max mumber of clients in a sequential order
    for (i = 0; i < (MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER); i++)
    {
        for (j = 0; j < MAX_NUM_ADAPTERS; j++)
        {
            for (k = 0; k < MAX_COUNT_PER_ADAPTER; k++)
            {
                InParams[i].AdapterIdx = AdapterCount;
                InParams[i].CountIdx   = RefCount++;
                InParams[i].MATestObj  = this;
                i++;
            }
            RefCount = 0;
            AdapterCount++;
        }
    }

    // Create threads to load MAX_NUM_ADAPTERS, MAX_COUNT_PER_ADAPTER times
    // Thread Count = MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER
    CreateMAThread(MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER, InParams);
}

#endif // GMM_LIB_DLL_MA

