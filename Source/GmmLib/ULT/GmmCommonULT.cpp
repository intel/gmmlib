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

#include "GmmCommonULT.h"
#ifndef _WIN32
#include <dlfcn.h>
#endif


ADAPTER_INFO *      CommonULT::pGfxAdapterInfo      = NULL;
PLATFORM            CommonULT::GfxPlatform          = {};
GMM_CLIENT_CONTEXT *CommonULT::pGmmULTClientContext = NULL;
PFNGMMINIT          CommonULT::pfnGmmInit           = {0};
PFNGMMDESTROY       CommonULT::pfnGmmDestroy        = {0};
#ifdef _WIN32
    HINSTANCE       CommonULT::hGmmLib              = NULL;
#else
    void           *CommonULT::hGmmLib              = NULL;
#endif

void CommonULT::AllocateAdapterInfo()
{
    if(!pGfxAdapterInfo)
    {
        pGfxAdapterInfo = (ADAPTER_INFO *)malloc(sizeof(ADAPTER_INFO));
        if(!pGfxAdapterInfo)
        {
            ASSERT_TRUE(false);
            return;
        }
        memset(pGfxAdapterInfo, 0, sizeof(ADAPTER_INFO));

        pGfxAdapterInfo->SkuTable.FtrTileY = 1;
        pGfxAdapterInfo->WaTable.WaAuxTable64KGranular = 1; // 64K aux granularity
    }
}

void CommonULT::SetUpTestCase()
{
    printf("%s\n", __FUNCTION__);

    GMM_INIT_IN_ARGS  InArgs;
    GMM_INIT_OUT_ARGS OutArgs;


    if(GfxPlatform.eProductFamily == IGFX_UNKNOWN ||
       GfxPlatform.eRenderCoreFamily == IGFX_UNKNOWN_CORE)
    {
        GfxPlatform.eProductFamily    = IGFX_BROADWELL;
        GfxPlatform.eRenderCoreFamily = IGFX_GEN8_CORE;
    }

    AllocateAdapterInfo();

    InArgs.ClientType = GMM_EXCITE_VISTA;
    InArgs.pGtSysInfo = &pGfxAdapterInfo->SystemInfo;
    InArgs.pSkuTable  = &pGfxAdapterInfo->SkuTable;
    InArgs.pWaTable   = &pGfxAdapterInfo->WaTable;
    InArgs.Platform   = GfxPlatform;
#ifdef _WIN32
    InArgs.stAdapterBDF = {0, 2, 0, 0};
#else
    InArgs.FileDescriptor = 512;  // Initializing BDF to {0,2,0,0} which corresponds to decimal 512 for AdapterBDF.Data
#endif

    hGmmLib = dlopen(GMM_UMD_DLL, RTLD_LAZY);
    ASSERT_TRUE(hGmmLib);

    *(void **)(&pfnGmmInit)    = dlsym(hGmmLib, "InitializeGmm");
    *(void **)(&pfnGmmDestroy) = dlsym(hGmmLib, "GmmAdapterDestroy");

    ASSERT_TRUE(pfnGmmInit);
    ASSERT_TRUE(pfnGmmDestroy);

    pfnGmmInit(&InArgs, &OutArgs);
    pGmmULTClientContext = OutArgs.pGmmClientContext;

    ASSERT_TRUE(pGmmULTClientContext);
}

void CommonULT::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    GMM_INIT_OUT_ARGS OutArgs;
    OutArgs.pGmmClientContext = static_cast<GMM_CLIENT_CONTEXT *>(pGmmULTClientContext);

    pfnGmmDestroy(&OutArgs);

    if(hGmmLib)
    {
        dlclose(hGmmLib);
    }

    hGmmLib              = NULL;
    pGmmULTClientContext = NULL;
    free(pGfxAdapterInfo);
    pGfxAdapterInfo = NULL;
    GfxPlatform     = {};
}
