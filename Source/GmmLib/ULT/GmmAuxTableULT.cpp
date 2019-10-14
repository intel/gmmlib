/*==============================================================================
Copyright(c) 2019 Intel Corporation

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

#if defined (__linux__) && !defined(__i386__)

#include "GmmAuxTableULT.h"

using namespace std;
using namespace GmmLib;

static GMM_DEVICE_CALLBACKS_INT DeviceCBInt;

CTestAuxTable::CTestAuxTable()
{
}

CTestAuxTable::~CTestAuxTable()
{
}

int CTestAuxTable::allocCB(void *bufMgr, size_t size, size_t alignment, void **bo, void **cpuAddr, uint64_t *gpuAddr)
{
    if(bufMgr != (void *)0xdeadbeef)
        return -1;

    if(!bo || !cpuAddr || !gpuAddr)
        return -2;

    *cpuAddr = aligned_alloc(alignment, ALIGN(size, alignment));
    if(!*cpuAddr)
        return -3;

    *bo      = *cpuAddr;
    *gpuAddr = (uint64_t)*cpuAddr;

    return 0;
}

void CTestAuxTable::freeCB(void *bo)
{
    ASSERT_TRUE(bo != NULL);

    free(bo);
}

void CTestAuxTable::waitFromCpuCB(void *bo)
{
}

void CTestAuxTable::SetUpTestCase()
{
    GfxPlatform.eProductFamily    = IGFX_TIGERLAKE_LP;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN12_CORE;

    DeviceCBInt.pBufMgr                   = (void *)0xdeadbeef;
    DeviceCBInt.DevCbPtrs_.pfnAllocate    = CTestAuxTable::allocCB;
    DeviceCBInt.DevCbPtrs_.pfnDeallocate  = CTestAuxTable::freeCB;
    DeviceCBInt.DevCbPtrs_.pfnWaitFromCpu = CTestAuxTable::waitFromCpuCB;

    if(GfxPlatform.eProductFamily == IGFX_UNKNOWN ||
       GfxPlatform.eRenderCoreFamily == IGFX_UNKNOWN_CORE)
    {
        GfxPlatform.eProductFamily    = IGFX_BROADWELL;
        GfxPlatform.eRenderCoreFamily = IGFX_GEN8_CORE;
    }

    AllocateAdapterInfo();
    if(pGfxAdapterInfo)
    {
        pGfxAdapterInfo->SkuTable.FtrE2ECompression = true;
        pGfxAdapterInfo->SkuTable.FtrLinearCCS      = true;
    }

    CommonULT::SetUpTestCase();
}

void CTestAuxTable::TearDownTestCase()
{
    CommonULT::TearDownTestCase();
}

TEST_F(CTestAuxTable, TestUpdateAuxTableCompressedSurface)
{
    GmmPageTableMgr *mgr = pGmmULTClientContext->CreatePageTblMgrObject(&DeviceCBInt, TT_TYPE::AUXTT);

    ASSERT_TRUE(mgr != NULL);

    Surface *surf = new Surface(7680, 4320);

    ASSERT_TRUE(surf != NULL && surf->init());

    GMM_DDI_UPDATEAUXTABLE updateReq = {0};

    updateReq.BaseResInfo = surf->getGMMResourceInfo();
    updateReq.BaseGpuVA   = surf->getGfxAddress(GMM_PLANE_Y);
    updateReq.Map         = 1;

    GMM_STATUS res = mgr->UpdateAuxTable(&updateReq);
    ASSERT_TRUE(res == GMM_SUCCESS);

    delete surf;
    pGmmULTClientContext->DestroyPageTblMgrObject(mgr);
}

TEST_F(CTestAuxTable, DISABLED_TestUpdateAuxTableNonCompressedSurface)
{
    GmmPageTableMgr *mgr = pGmmULTClientContext->CreatePageTblMgrObject(&DeviceCBInt, TT_TYPE::AUXTT);

    ASSERT_TRUE(mgr != NULL);

    Surface *surf = new Surface(7680, 4320, false);

    ASSERT_TRUE(surf != NULL && surf->init());

    GMM_DDI_UPDATEAUXTABLE updateReq = {0};

    memset(&updateReq, 0, sizeof(GMM_DDI_UPDATEAUXTABLE));

    updateReq.BaseResInfo = surf->getGMMResourceInfo();
    updateReq.BaseGpuVA   = surf->getGfxAddress(GMM_PLANE_Y);
    updateReq.Map         = 1;

    GMM_STATUS res = mgr->UpdateAuxTable(&updateReq);
    ASSERT_TRUE(res != GMM_SUCCESS);

    delete surf;
    pGmmULTClientContext->DestroyPageTblMgrObject(mgr);
}

TEST_F(CTestAuxTable, TestInvalidateAuxTable)
{
    GmmPageTableMgr *mgr = pGmmULTClientContext->CreatePageTblMgrObject(&DeviceCBInt, TT_TYPE::AUXTT);

    ASSERT_TRUE(mgr != NULL);

    Surface *surf = new Surface(7680, 4320);

    ASSERT_TRUE(surf != NULL && surf->init());

    GMM_DDI_UPDATEAUXTABLE updateReq = {0};

    updateReq.BaseResInfo = surf->getGMMResourceInfo();
    updateReq.BaseGpuVA   = surf->getGfxAddress(GMM_PLANE_Y);
    updateReq.Map         = 1;

    GMM_STATUS res = mgr->UpdateAuxTable(&updateReq);
    ASSERT_TRUE(res == GMM_SUCCESS);

    memset(&updateReq, 0, sizeof(updateReq));

    updateReq.BaseResInfo = surf->getGMMResourceInfo();
    updateReq.Map         = 0;

    res = mgr->UpdateAuxTable(&updateReq);
    ASSERT_TRUE(res == GMM_SUCCESS);

    delete surf;
    pGmmULTClientContext->DestroyPageTblMgrObject(mgr);
}

TEST_F(CTestAuxTable, DISABLED_TestUpdateAuxTableStress)
{
    const int num_surf = 1000;
    Surface * surfaces[num_surf];
    Surface * surf;
    int       i;

    GmmPageTableMgr *mgr = pGmmULTClientContext->CreatePageTblMgrObject(&DeviceCBInt, TT_TYPE::AUXTT);

    ASSERT_TRUE(mgr != NULL);

    for(i = 0; i < num_surf; i++)
    {
        surf        = new Surface(7680, 4320);
        surfaces[i] = surf;

        ASSERT_TRUE(surf != NULL && surf->init());

        GMM_DDI_UPDATEAUXTABLE updateReq = {0};

        updateReq.BaseResInfo = surf->getGMMResourceInfo();
        updateReq.BaseGpuVA   = surf->getGfxAddress(GMM_PLANE_Y);
        updateReq.Map         = 1;

        mgr->UpdateAuxTable(&updateReq);
    }

    for(i = 0; i < num_surf; i++)
    {
        surf = surfaces[i];
        delete surf;
    }

    pGmmULTClientContext->DestroyPageTblMgrObject(mgr);
}

TEST_F(CTestAuxTable, TestAuxTableContent)
{
    GmmPageTableMgr *mgr = pGmmULTClientContext->CreatePageTblMgrObject(&DeviceCBInt, TT_TYPE::AUXTT);

    ASSERT_TRUE(mgr != NULL);

    Surface *surf = new Surface(720, 480);

    ASSERT_TRUE(surf != NULL && surf->init());

    GMM_DDI_UPDATEAUXTABLE updateReq = {0};

    updateReq.BaseResInfo = surf->getGMMResourceInfo();
    updateReq.BaseGpuVA   = surf->getGfxAddress(GMM_PLANE_Y);
    updateReq.Map         = 1;

    GMM_STATUS res = mgr->UpdateAuxTable(&updateReq);
    ASSERT_TRUE(res == GMM_SUCCESS);

    Walker *ywalker = new Walker(surf->getGfxAddress(GMM_PLANE_Y),
                                 surf->getAuxGfxAddress(GMM_AUX_CCS),
                                 mgr->GetAuxL3TableAddr());

    for(size_t i = 0; i < surf->getSurfaceSize(GMM_PLANE_Y); i++)
    {
        GMM_GFX_ADDRESS addr     = surf->getGfxAddress(GMM_PLANE_Y) + i;
        GMM_GFX_ADDRESS val      = ywalker->walk(addr);
        GMM_GFX_ADDRESS expected = ywalker->expected(addr);
        ASSERT_EQ(expected, val);
    }

    Walker *uvwalker = new Walker(surf->getGfxAddress(GMM_PLANE_U),
                                  surf->getAuxGfxAddress(GMM_AUX_UV_CCS),
                                  mgr->GetAuxL3TableAddr());

    for(size_t i = 0; i < surf->getSurfaceSize(GMM_PLANE_U); i++)
    {
        GMM_GFX_ADDRESS addr     = surf->getGfxAddress(GMM_PLANE_U) + i;
        GMM_GFX_ADDRESS val      = uvwalker->walk(addr);
        GMM_GFX_ADDRESS expected = uvwalker->expected(addr);
        ASSERT_EQ(expected, val);
    }

    delete uvwalker;
    delete ywalker;
    delete surf;
    pGmmULTClientContext->DestroyPageTblMgrObject(mgr);
}

#endif /* __linux__ */
