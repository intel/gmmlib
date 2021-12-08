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

#define MAX_COUNT_PER_ADAPTER       5

#ifdef _WIN32
#define GMM_DLL_HANDLE      HINSTANCE
#else
#define GMM_DLL_HANDLE      void*
#endif

class MACommonULT : public testing::Test
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void LoadGmmDll(uint32_t AdapterIdx, uint32_t CountIdx);
    static void UnLoadGmmDll(uint32_t AdapterIdx, uint32_t CountIdx);
    static void GmmInitModule(uint32_t AdapterIdx, uint32_t CountIdx);
    static void GmmDestroyModule(uint32_t AdapterIdx, uint32_t CountIdx);

    static ADAPTER_INFO *           pGfxAdapterInfo[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static PLATFORM                 GfxPlatform[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_LIB_CONTEXT *        pLibContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_CLIENT_CONTEXT *     pGmmULTClientContext[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static PFNGMMINIT               pfnGmmInit[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static PFNGMMDESTROY            pfnGmmDestroy[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_INIT_IN_ARGS         InArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_INIT_OUT_ARGS        OutArgs[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];
    static GMM_DLL_HANDLE           hGmmLib[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];

    static PRODUCT_FAMILY GetProductFamily(uint32_t AdapterIdx)
    {
        switch (AdapterIdx)
        {
        case 0:   return IGFX_DG1;
        case 1:   return IGFX_ICELAKE;
        case 2:   return IGFX_TIGERLAKE_LP;
        default: break;
        }
        return IGFX_COFFEELAKE;
    }

    static GFXCORE_FAMILY GetRenderCoreFamily(uint32_t AdapterIdx)
    {
        switch (AdapterIdx)
        {
        case 0:   return IGFX_XE_HP_CORE;
        case 1:   return IGFX_GEN11LP_CORE;
        case 2:   return IGFX_GEN12LP_CORE;
        default: break;
        }
        return IGFX_GEN9_CORE;
    }

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

    static ADAPTER_BDF GetAdapterBDF(uint32_t AdapterIdx)
    {
        ADAPTER_BDF AdapterBDF={0,2,0,0};

        switch (AdapterIdx)
        {
        case 0:
            AdapterBDF.Bus      = 2;
            AdapterBDF.Device   = 0;
            AdapterBDF.Function = 0;
            break;
        case 1:
            AdapterBDF.Bus      = 1;
            AdapterBDF.Device   = 0;
            AdapterBDF.Function = 0;
            break;
        case 2:
            AdapterBDF.Bus      = 2;
            AdapterBDF.Device   = 1;
            AdapterBDF.Function = 0;
            break;
        default: break;
        }
        return AdapterBDF;
    }
};

typedef struct ThreadInParams_Rec
{
    uint32_t AdapterIdx; 
    uint32_t CountIdx;
}ThreadInParams;

class CTestMA : public MACommonULT
{
public:
    CTestMA();
    ~CTestMA();

    static void SetUpTestCase();
    static void TearDownTestCase();

};

