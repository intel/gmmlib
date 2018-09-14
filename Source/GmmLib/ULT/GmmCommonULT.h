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

#pragma once

#include "stdafx.h"

typedef GMM_CLIENT_CONTEXT *(GMM_STDCALL * PFNGMMINIT)
#ifdef _WIN32
    (const PLATFORM,
    const SKU_FEATURE_TABLE *,
    const WA_TABLE *,
    const GT_SYSTEM_INFO *,
    GMM_CLIENT);
#else
    (const PLATFORM Platform,
    const void *   pSkuTable,
    const void *   pWaTable,
    const void *   pGtSysInfo,
    GMM_CLIENT ClientType);
#endif
typedef void(GMM_STDCALL *PFNGMMDESTROY)(GMM_CLIENT_CONTEXT *);

class CommonULT : public testing::Test
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    static void AllocateAdapterInfo();

protected:
    static ADAPTER_INFO *pGfxAdapterInfo;
    static PLATFORM GfxPlatform;

    static GMM_CLIENT_CONTEXT              *pGmmULTClientContext;
    static PFNGMMINIT                       pfnGmmInit;
    static PFNGMMDESTROY                    pfnGmmDestroy;

    #ifdef _WIN32
        static HINSTANCE                     hGmmLib;
    #else
        static void                         *hGmmLib;
    #endif

};
