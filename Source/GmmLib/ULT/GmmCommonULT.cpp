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

ADAPTER_INFO *CommonULT::pGfxAdapterInfo = NULL;
PLATFORM CommonULT::GfxPlatform = {};
GMM_CLIENT_CONTEXT* CommonULT::pGmmULTClientContext = NULL;

void CommonULT::SetUpTestCase()
{
    printf("%s\n", __FUNCTION__);

    if(GfxPlatform.eProductFamily == IGFX_UNKNOWN ||
       GfxPlatform.eRenderCoreFamily == IGFX_UNKNOWN_CORE)
    {
        GfxPlatform.eProductFamily = IGFX_BROADWELL;
        GfxPlatform.eRenderCoreFamily = IGFX_GEN8_CORE;
    }

    pGfxAdapterInfo = (ADAPTER_INFO *) malloc(sizeof(ADAPTER_INFO));

    if(!pGfxAdapterInfo)
    {
        ASSERT_TRUE(false);
        return;
    }
    memset(pGfxAdapterInfo, 0, sizeof(ADAPTER_INFO));

    GMM_STATUS GmmStatus = GmmInitGlobalContext(GfxPlatform,
        &pGfxAdapterInfo->SkuTable,
        &pGfxAdapterInfo->WaTable,
        &pGfxAdapterInfo->SystemInfo,
        GMM_D3D9_VISTA
        );

    ASSERT_EQ(GmmStatus, GMM_SUCCESS);

    pGmmULTClientContext = GmmCreateClientContext(GMM_EXCITE_VISTA);
    ASSERT_TRUE(pGmmULTClientContext);

}

void CommonULT::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);
    GmmDeleteClientContext(pGmmULTClientContext);
    GmmDestroyGlobalContext();
    free(pGfxAdapterInfo);
    pGfxAdapterInfo = NULL;
    GfxPlatform = {};
}
