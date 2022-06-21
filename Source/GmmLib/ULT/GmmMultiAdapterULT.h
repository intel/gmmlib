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
#pragma once

#include "stdafx.h"
#include "GmmCommonULT.h"

#define MAX_COUNT_PER_ADAPTER       3

#ifdef _WIN32
#define GMM_DLL_HANDLE      HINSTANCE
#else
#define GMM_DLL_HANDLE      void*
#endif

typedef struct ThreadInParams_Rec
{
    uint32_t AdapterIdx;
    uint32_t CountIdx;
} ThreadInParams;

class MACommonULT : public testing::Test
{
public:
    MACommonULT();
    ~MACommonULT();
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void LoadGmmDll(uint32_t AdapterIdx, uint32_t CountIdx);
    static void UnLoadGmmDll(uint32_t AdapterIdx, uint32_t CountIdx);
    static void GmmInitModule(uint32_t AdapterIdx, uint32_t CountIdx);
    static void GmmDestroyModule(uint32_t AdapterIdx, uint32_t CountIdx);
    static void CreateMAThread(uint32_t MaxClientThreads);

    static ADAPTER_INFO *           pGfxAdapterInfo[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static PLATFORM                 GfxPlatform[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_LIB_CONTEXT *        pLibContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_CLIENT_CONTEXT *     pGmmULTClientContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static PFNGMMINIT               pfnGmmInit[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static PFNGMMDESTROY            pfnGmmDestroy[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_INIT_IN_ARGS         InArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_INIT_OUT_ARGS        OutArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_DLL_HANDLE           hGmmLib[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];

    // Comman Input paramter array for all thread related ULTs to supply Adapter-Client Context input.

    static ThreadInParams           InParams[MAX_NUM_ADAPTERS * MAX_COUNT_PER_ADAPTER];

    // Array to store the adapter BDFs from simulated UMD, Save the adapter bdf in an array
    static ADAPTER_BDF              AdpaterSaved[MAX_NUM_ADAPTERS];
    
    // Only for our ULT to supply dummy ProductFamily
    static PRODUCT_FAMILY GetProductFamily(uint32_t AdapterIdx)
    {
        switch (AdapterIdx)
        {
        case 0:   return IGFX_DG1;
        case 1:   return IGFX_ICELAKE;
        case 2:   return IGFX_TIGERLAKE_LP;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:   return IGFX_DG2;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:   return IGFX_XE_HP_SDV;
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
        case 29:   return IGFX_PVC;
	case 30:
        case 31:
	default: break;
        }
        return IGFX_COFFEELAKE;
    }

    // Only for our ULT to supply dummy GFXCORE_FAMILY
    static GFXCORE_FAMILY GetRenderCoreFamily(uint32_t AdapterIdx)
    {
        switch (AdapterIdx)
        {
        case 0:   return IGFX_XE_HP_CORE;
        case 1:   return IGFX_GEN11LP_CORE;
        case 2:   return IGFX_GEN12LP_CORE;
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:   return IGFX_XE_HPG_CORE;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:   return IGFX_XE_HP_CORE;
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
        case 29:   return IGFX_XE_HPC_CORE;
        case 30:
        case 31:
	default: break;
        }
        return IGFX_GEN9_CORE;
    }

    // To simulate the UMDs/ClinentContexts per adapter i.e MAX_COUNT_PER_ADAPTER
    // Increase MAX_COUNT_PER_ADAPTER value if there are more UMDs involved.
    static GMM_CLIENT GetClientType(uint32_t CountIdx)
    {
        switch (CountIdx)
        {
        case 0:   return GMM_D3D9_VISTA;
        case 1:   return GMM_D3D10_VISTA;
        case 2:   return GMM_D3D12_VISTA;
	case 3:   return GMM_EXCITE_VISTA;
	case 4:   return GMM_OCL_VISTA;
        default: break;
        }
        return GMM_D3D9_VISTA;
    }

    // Returns the AdpaterSaved array value, Adapter BDFs based on input AdapterIdx.

    static ADAPTER_BDF GetAdapterBDF(uint32_t AdapterIdx)
    {
        ADAPTER_BDF AdapterBDF={0,2,0,0};

        if(AdapterIdx < MAX_NUM_ADAPTERS)
        {
            AdapterBDF.Bus      = AdpaterSaved[AdapterIdx].Bus;
            AdapterBDF.Device   = AdpaterSaved[AdapterIdx].Device;
            AdapterBDF.Function = AdpaterSaved[AdapterIdx].Function;
	}
        return AdapterBDF;
    }
};

typedef MACommonULT GMM_MA_ULT_CONTEXT;

class CTestMA : public MACommonULT
{
public:
    CTestMA();
    ~CTestMA();

    static void SetUpTestCase();
    static void TearDownTestCase();

};

#ifdef _WIN32
    DWORD WINAPI Thread1(LPVOID lpParam);
#else
    void *Thread1(void *lpParam);
#endif
