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
#include "../inc/External/Common/GmmClientContext.h"

#define MAX_COUNT_PER_ADAPTER 5

#ifdef _WIN32
#define GMM_DLL_HANDLE      HINSTANCE
#else
#define GMM_DLL_HANDLE      void*
#endif

typedef struct GmmULTInfo_Rec
{
    ADAPTER_INFO       *pGfxAdapterInfo;
    PLATFORM            GfxPlatform;
    GMM_LIB_CONTEXT    *pLibContext;
    GMM_CLIENT_CONTEXT *pGmmULTClientContext;
    GMM_INIT_IN_ARGS    InArgs;
    GMM_INIT_OUT_ARGS   OutArgs;
} GmmULTInfo;

class MACommonULT : public testing::Test
{
public:
    MACommonULT();
    ~MACommonULT();

    void LoadGmmDll();
    void UnLoadGmmDll();
    void GmmInitModule(uint32_t AdapterIdx, uint32_t CountIdx);
    void GmmDestroyModule(uint32_t AdapterIdx, uint32_t CountIdx);

    PRODUCT_FAMILY GetProductFamily(uint32_t AdapterIdx);
    GFXCORE_FAMILY GetRenderCoreFamily(uint32_t AdapterIdx);
    GMM_CLIENT     GetClientType(uint32_t CountIdx);
    ADAPTER_BDF    GetAdapterBDF(uint32_t AdapterIdx);

protected:
    GmmULTInfo GmmTestInfo[MAX_NUM_ADAPTERS][MAX_COUNT_PER_ADAPTER];

    GMM_DLL_HANDLE hGmmLib;
    PFNGMMINIT     pfnGmmInit;
    PFNGMMDESTROY  pfnGmmDestroy;

    // Array to store the adapter BDFs from simulated UMD, Save the adapter bdf in an array
    ADAPTER_BDF AdapterSaved[MAX_NUM_ADAPTERS];
};

typedef MACommonULT GMM_MA_ULT_CONTEXT;

typedef struct ThreadInParams_Rec
{
    uint32_t AdapterIdx;
    uint32_t CountIdx;

    GMM_MA_ULT_CONTEXT *MATestObj;
} ThreadInParams;

class CTestMA : public MACommonULT
{
public:
    CTestMA();
    ~CTestMA();

    void SetUp() override;
    void TearDown() override;

    static void SetUpTestCase();
    static void TearDownTestCase();
};

#ifdef _WIN32
DWORD WINAPI MAULTThreadEntryFunc(LPVOID lpParam);
#else
void *MAULTThreadEntryFunc(void *lpParam);
#endif
