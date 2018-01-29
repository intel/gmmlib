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


#include "Internal/Common/GmmLibInc.h"
#include "External/Common/GmmClientContext.h"


#ifdef GMM_OCL
//===========================================================================
// Global Variable:
//      pGmmOclClientContext
//
// Description:
//     Handle to global GmmClientContext for OCL.
//
//----------------------------------------------------------------------------
GMM_CLIENT_CONTEXT *pGmmOclClientContext = NULL;

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for creating Global GmmClientContext for OCL Client
/// @see        Class GMM_CLIENT_CONTEXT
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmCreateGlobalOCLClientContext()
{
    GMM_STATUS Status    = GMM_OUT_OF_MEMORY;
    pGmmOclClientContext = GmmCreateClientContext(GMM_OCL_VISTA);
    if(pGmmOclClientContext)
    {
        Status = GMM_SUCCESS;
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for destroying Global GmmClientContext for OCL Client
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmDestroyGlobalOCLClientContext()
{
    if(pGmmOclClientContext)
    {
        GmmDeleteClientContext(pGmmOclClientContext);
        pGmmOclClientContext = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for Creating ResInfo for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GmmResCreateForOCL(GMM_RESCREATE_PARAMS *pCreateParams)
{
    GMM_RESOURCE_INFO *pRes = NULL;
    if(pGmmOclClientContext)
    {
        pRes = pGmmOclClientContext->CreateResInfoObject(pCreateParams);
    }

    return pRes;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for Destrying ResInfo for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmResFreeForOCL(GMM_RESOURCE_INFO *pRes)
{
    if(pGmmOclClientContext)
    {
        pGmmOclClientContext->DestroyResInfoObject(pRes);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for ResInfo Copy for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GmmResCopyForOCL(GMM_RESOURCE_INFO *pSrcRes)
{
    GMM_RESOURCE_INFO *pRes = NULL;
    if(pGmmOclClientContext)
    {
        pRes = pGmmOclClientContext->CopyResInfoObject(pSrcRes);
    }

    return pRes;
}

#endif

#if _WIN32

#if !__GMM_KMD__

#ifdef GMM_OCL
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for ResUpdateCacheability for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResUpdateCacheability(HANDLE                 hAdapter,
                                             HANDLE                 hDevice,
                                             D3DKMT_HANDLE          hAllocation,
                                             GMM_RESOURCE_INFO *    pGmmResInfo,
                                             GMM_GPU_CACHE_SETTINGS CacheSettings,
                                             GMM_ESCAPE_FUNC_TYPE   pfnEscape)
{
    uint8_t                  Status = 0;
    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.DxHandle  = hAdapter;
    GmmDevice.DxHandle   = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;
    Status               = pGmmResInfo->UpdateCacheability(GmmAdapter, GmmDevice, hAllocation, CacheSettings, GmmEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmRes32bitAlias(GMM_ESCAPE_HANDLE    hAdapter,
                                     GMM_ESCAPE_HANDLE    hDevice,
                                     D3DKMT_HANDLE        hAllocation,
                                     GMM_RESOURCE_INFO *  pGmmResInfo,
                                     uint32_t             Size,
                                     uint32_t *           pAliasAddress,
                                     GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t                  Status = 0;
    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;
    Status               = pGmmResInfo->Alias32bit(GmmAdapter, GmmDevice, hAllocation, Size, pAliasAddress, GmmEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmConfigureDeviceAddressSpace(GMM_ESCAPE_HANDLE    hAdapter,
                                                   GMM_ESCAPE_HANDLE    hDevice,
                                                   GMM_ESCAPE_FUNC_TYPE pfnEscape,
                                                   GMM_GFX_SIZE_T       SvmSize,
                                                   uint8_t              FaultableSvm,
                                                   uint8_t              SparseReady,
                                                   uint8_t              BDWL3Coherency,
                                                   GMM_GFX_SIZE_T       SizeOverride,
                                                   GMM_GFX_SIZE_T       SlmGfxSpaceReserve)
{
    uint8_t                  Status = 0;
    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->ConfigureDeviceAddressSpace(GmmAdapter,
                                                                   GmmDevice,
                                                                   GmmEscape,
                                                                   SvmSize,
                                                                   FaultableSvm,
                                                                   SparseReady,
                                                                   BDWL3Coherency,
                                                                   SizeOverride,
                                                                   SlmGfxSpaceReserve);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_PARTITIONING GMM_STDCALL GmmOverrideGfxPartition(GMM_GFX_PARTITIONING *GfxPartition,
                                                         GMM_ESCAPE_HANDLE     hAdapter,
                                                         GMM_ESCAPE_HANDLE     hDevice,
                                                         GMM_ESCAPE_FUNC_TYPE  pfnEscape)
{
    GMM_GFX_PARTITIONING ProcessVA = {0};

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        ProcessVA = pGmmOclClientContext->OverrideGfxPartition(GfxPartition,
                                                               GmmAdapter,
                                                               GmmDevice,
                                                               GmmEscape);
    }

    return ProcessVA;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmEnhancedBufferMap(GMM_ESCAPE_HANDLE         hAdapter,
                                         GMM_ESCAPE_HANDLE         hDevice,
                                         GMM_ESCAPE_FUNC_TYPE      pfnEscape,
                                         D3DKMT_HANDLE             hOriginalAllocation,
                                         GMM_ENHANCED_BUFFER_INFO *pEnhancedBufferInfo[GMM_MAX_DISPLAYS])
{
    uint8_t                  Status = 0;
    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->EnhancedBufferMap(GmmAdapter,
                                                         GmmDevice,
                                                         GmmEscape,
                                                         hOriginalAllocation,
                                                         pEnhancedBufferInfo);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmGetHeap32Base(GMM_ESCAPE_HANDLE    hAdapter,
                                             GMM_ESCAPE_HANDLE    hDevice,
                                             uint32_t *           pHeapSize,
                                             GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_GFX_ADDRESS Heap32Base = 0;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Heap32Base = pGmmOclClientContext->GetHeap32Base(GmmAdapter, GmmDevice, pHeapSize, GmmEscape);
    }

    return Heap32Base;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmGetTrashPageGfxAddress(GMM_ESCAPE_HANDLE    hAdapter,
                                                      GMM_ESCAPE_HANDLE    hDevice,
                                                      GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_GFX_ADDRESS GfxAddress = 0;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        GfxAddress = pGmmOclClientContext->GetTrashPageGfxAddress(GmmAdapter, GmmDevice, GmmEscape);
    }

    return GfxAddress;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmPigmsReserveGpuVA(GMM_ESCAPE_HANDLE    hAdapter,
                                                 GMM_ESCAPE_HANDLE    hDevice,
                                                 GMM_GFX_SIZE_T       Size,
                                                 GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_GFX_ADDRESS GfxAddress = 0;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        GfxAddress = pGmmOclClientContext->PigmsReserveGpuVA(GmmAdapter, GmmDevice, Size, GmmEscape);
    }

    return GfxAddress;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmPigmsMapGpuVA(GMM_ESCAPE_HANDLE    hAdapter,
                                     GMM_ESCAPE_HANDLE    hDevice,
                                     GMM_GFX_ADDRESS      GfxAddress,
                                     D3DKMT_HANDLE        hAllocation,
                                     GMM_RESOURCE_INFO *  pGmmResInfo,
                                     GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t                  Status = 0;
    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->PigmsMapGpuVA(GmmAdapter,
                                                     GmmDevice,
                                                     GfxAddress,
                                                     hAllocation,
                                                     pGmmResInfo,
                                                     GmmEscape);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmPigmsFreeGpuVa(GMM_ESCAPE_HANDLE    hAdapter,
                                      GMM_ESCAPE_HANDLE    hDevice,
                                      GMM_GFX_ADDRESS      GfxAddress,
                                      GMM_GFX_SIZE_T       Size,
                                      GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t                  Status = 0;
    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->PigmsFreeGpuVa(GmmAdapter,
                                                      GmmDevice,
                                                      GfxAddress,
                                                      Size,
                                                      GmmEscape);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_HEAP *GMM_STDCALL GmmUmSetupHeap(GMM_ESCAPE_HANDLE    hAdapter,
                                     GMM_ESCAPE_HANDLE    hDevice,
                                     GMM_GFX_ADDRESS      GfxAddress,
                                     GMM_GFX_SIZE_T       Size,
                                     uint32_t             Flags,
                                     GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_HEAP *pHeapObj = NULL;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        pHeapObj = pGmmOclClientContext->UmSetupHeap(GmmAdapter, GmmDevice, GfxAddress, Size, Flags, GmmEscape);
    }

    return pHeapObj;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmUmDestroypHeap(GMM_ESCAPE_HANDLE    hAdapter,
                                         GMM_ESCAPE_HANDLE    hDevice,
                                         GMM_HEAP **          pHeapObj,
                                         GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_STATUS Status = GMM_ERROR;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->UmDestroypHeap(GmmAdapter, GmmDevice, pHeapObj, GmmEscape);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmAllocateHeapVA(GMM_HEAP *     pHeapObj,
                                              GMM_GFX_SIZE_T AllocSize)
{
    GMM_GFX_ADDRESS GfxAddress = 0;

    if(pGmmOclClientContext)
    {
        GfxAddress = pGmmOclClientContext->AllocateHeapVA(pHeapObj, AllocSize);
    }

    return GfxAddress;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmFreeHeapVA(GMM_HEAP *      pHeapObj,
                                     GMM_GFX_ADDRESS AllocVA,
                                     GMM_GFX_SIZE_T  AllocSize)
{
    GMM_STATUS Status = GMM_ERROR;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->FreeHeapVA(pHeapObj, AllocVA, AllocSize);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResDestroySvmAllocation(GMM_ESCAPE_HANDLE    hAdapter,
                                               GMM_ESCAPE_HANDLE    hDevice,
                                               D3DKMT_HANDLE        hAllocation,
                                               GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->ResDestroySvmAllocation(GmmAdapter, GmmDevice, hAllocation, GmmEscape);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmResUpdateAfterSharedOpen(HANDLE               hAdapter,
                                                   HANDLE               hDevice,
                                                   D3DKMT_HANDLE        hAllocation,
                                                   GMM_RESOURCE_INFO *  pGmmResInfo,
                                                   GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_STATUS Status = GMM_ERROR;

    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmOclClientContext)
    {
        Status = pGmmOclClientContext->ResUpdateAfterSharedOpen(hAdapter, hDevice, hAllocation, pGmmResInfo, GmmEscape);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmKmdResGetSetHardwareProtection(GMM_ESCAPE_HANDLE    hAdapter,
                                                      GMM_ESCAPE_HANDLE    hDevice,
                                                      D3DKMT_HANDLE        hAllocation,
                                                      GMM_RESOURCE_INFO *  pGmmResource,
                                                      uint8_t              SetIsEncrypted,
                                                      uint8_t *            pGetIsEncrypted,
                                                      GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;

    GMM_ESCAPE_HANDLE_EXT    GmmAdapter;
    GMM_ESCAPE_HANDLE_EXT    GmmDevice;
    GMM_ESCAPE_FUNC_TYPE_EXT GmmEscape;

    GmmAdapter.KmtHandle = hAdapter;
    GmmDevice.KmtHandle  = hDevice;
    GmmEscape.pfnKmtFunc = pfnEscape;

    if(pGmmResource)
    {
        Status = pGmmResource->KmdGetSetHardwareProtection(GmmAdapter, GmmDevice, hAllocation, SetIsEncrypted, pGetIsEncrypted, GmmEscape);
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Instantiates GmmPageTableMgr, allocating memory for root-tables, copies provided
/// device-callback and Batch-buffer function pointers
///
/// @param[in]  DeviceCB: pointer sharing device-callback function pointers
/// @param[in]  TTFlags: Flags specifying which PageTables are required by client
/// @param[in]  TTCB: pointer sharing Batch Buffer function pointers
/// @return     GmmPageTableMgr*
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::GmmPageTableMgr(GMM_DEVICE_CALLBACKS *DeviceCB, uint32_t TTFlags)
{
    GMM_PAGETABLE_MGR *      ptr         = NULL;
    GMM_DEVICE_CALLBACKS_INT OclDeviceCb = {0};

    OclDeviceCb.Adapter.KmtHandle                   = DeviceCB->Adapter;
    OclDeviceCb.hDevice.KmtHandle                   = DeviceCB->hDevice;
    OclDeviceCb.PagingFence                         = DeviceCB->PagingFence;
    OclDeviceCb.PagingQueue                         = DeviceCB->PagingQueue;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnAllocate     = DeviceCB->pfnAllocate;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnDeallocate   = DeviceCB->pfnDeallocate;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnEscape       = DeviceCB->pfnEscape;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnEvict        = DeviceCB->pfnEvict;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnFreeGPUVA    = DeviceCB->pfnFreeGPUVA;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnLock         = DeviceCB->pfnLock;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnMakeResident = DeviceCB->pfnMakeResident;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnMapGPUVA     = DeviceCB->pfnMapGPUVA;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnReserveGPUVA = DeviceCB->pfnReserveGPUVA;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnUnLock       = DeviceCB->pfnUnLock;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnUpdateGPUVA  = DeviceCB->pfnUpdateGPUVA;
    OclDeviceCb.DevCbPtrs.KmtCbPtrs.pfnWaitFromCpu  = DeviceCB->pfnWaitFromCpu;

    ptr = new GmmPageTableMgr(&OclDeviceCb, TTFlags, pGmmOclClientContext);

    *this = *ptr;

    delete ptr;
    ptr = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Instantiates GmmPageTableMgr, allocating memory for root-tables, copies provided
/// device-callback and Batch-buffer function pointers
///
/// @param[in]  DeviceCB: pointer sharing device-callback function pointers
/// @param[in]  TTFlags: Flags specifying which PageTables are required by client
/// @param[in]  TTCB: pointer sharing Batch Buffer function pointers
/// @return     GmmPageTableMgr*
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::GmmPageTableMgr(GMM_DEVICE_CALLBACKS *DeviceCB, uint32_t TTFlags, GMM_TRANSLATIONTABLE_CALLBACKS *TTCB)
    : GmmPageTableMgr(DeviceCB, TTFlags)
{
    memcpy_s(&TTCb, sizeof(GMM_TRANSLATIONTABLE_CALLBACKS), TTCB, sizeof(GMM_TRANSLATIONTABLE_CALLBACKS));
}

#else

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for ResUpdateCacheability for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResUpdateCacheability(HANDLE                 hAdapter,
                                             HANDLE                 hDevice,
                                             D3DKMT_HANDLE          hAllocation,
                                             GMM_RESOURCE_INFO *    pGmmResInfo,
                                             GMM_GPU_CACHE_SETTINGS CacheSettings,
                                             GMM_ESCAPE_FUNC_TYPE   pfnEscape)
{
    uint8_t Status = 0;
    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(hAllocation);
    UNREFERENCED_PARAMETER(pGmmResInfo);
    UNREFERENCED_PARAMETER(CacheSettings);
    UNREFERENCED_PARAMETER(pfnEscape);
    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmRes32bitAlias(GMM_ESCAPE_HANDLE    hAdapter,
                                     GMM_ESCAPE_HANDLE    hDevice,
                                     D3DKMT_HANDLE        hAllocation,
                                     GMM_RESOURCE_INFO *  pGmmResInfo,
                                     uint32_t             Size,
                                     uint32_t *           pAliasAddress,
                                     GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;
    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(hAllocation);
    UNREFERENCED_PARAMETER(pGmmResInfo);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(pAliasAddress);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmConfigureDeviceAddressSpace(GMM_ESCAPE_HANDLE    hAdapter,
                                                   GMM_ESCAPE_HANDLE    hDevice,
                                                   GMM_ESCAPE_FUNC_TYPE pfnEscape,
                                                   GMM_GFX_SIZE_T       SvmSize,
                                                   uint8_t              FaultableSvm,
                                                   uint8_t              SparseReady,
                                                   uint8_t              BDWL3Coherency,
                                                   GMM_GFX_SIZE_T       SizeOverride,
                                                   GMM_GFX_SIZE_T       SlmGfxSpaceReserve)
{
    uint8_t Status = 0;
    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(SvmSize);
    UNREFERENCED_PARAMETER(FaultableSvm);
    UNREFERENCED_PARAMETER(SparseReady);
    UNREFERENCED_PARAMETER(BDWL3Coherency);
    UNREFERENCED_PARAMETER(SizeOverride);
    UNREFERENCED_PARAMETER(SlmGfxSpaceReserve);
    UNREFERENCED_PARAMETER(pfnEscape);
    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_PARTITIONING GMM_STDCALL GmmOverrideGfxPartition(GMM_GFX_PARTITIONING *GfxPartition,
                                                         GMM_ESCAPE_HANDLE     hAdapter,
                                                         GMM_ESCAPE_HANDLE     hDevice,
                                                         GMM_ESCAPE_FUNC_TYPE  pfnEscape)
{
    GMM_GFX_PARTITIONING ProcessVA = {0};

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(GfxPartition);
    UNREFERENCED_PARAMETER(pfnEscape);

    return ProcessVA;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmEnhancedBufferMap(GMM_ESCAPE_HANDLE         hAdapter,
                                         GMM_ESCAPE_HANDLE         hDevice,
                                         GMM_ESCAPE_FUNC_TYPE      pfnEscape,
                                         D3DKMT_HANDLE             hOriginalAllocation,
                                         GMM_ENHANCED_BUFFER_INFO *pEnhancedBufferInfo[GMM_MAX_DISPLAYS])
{
    uint8_t Status = 0;
    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(hOriginalAllocation);
    UNREFERENCED_PARAMETER(pEnhancedBufferInfo);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmGetHeap32Base(GMM_ESCAPE_HANDLE    hAdapter,
                                             GMM_ESCAPE_HANDLE    hDevice,
                                             uint32_t *           pHeapSize,
                                             GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_GFX_ADDRESS Heap32Base = 0;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(pHeapSize);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Heap32Base;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmGetTrashPageGfxAddress(GMM_ESCAPE_HANDLE    hAdapter,
                                                      GMM_ESCAPE_HANDLE    hDevice,
                                                      GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_GFX_ADDRESS GfxAddress = 0;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(pfnEscape);

    return GfxAddress;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmPigmsReserveGpuVA(GMM_ESCAPE_HANDLE    hAdapter,
                                                 GMM_ESCAPE_HANDLE    hDevice,
                                                 GMM_GFX_SIZE_T       Size,
                                                 GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_GFX_ADDRESS GfxAddress = 0;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(pfnEscape);

    return GfxAddress;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmPigmsMapGpuVA(GMM_ESCAPE_HANDLE    hAdapter,
                                     GMM_ESCAPE_HANDLE    hDevice,
                                     GMM_GFX_ADDRESS      GfxAddress,
                                     D3DKMT_HANDLE        hAllocation,
                                     GMM_RESOURCE_INFO *  pGmmResInfo,
                                     GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;
    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(GfxAddress);
    UNREFERENCED_PARAMETER(hAllocation);
    UNREFERENCED_PARAMETER(pGmmResInfo);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmPigmsFreeGpuVa(GMM_ESCAPE_HANDLE    hAdapter,
                                      GMM_ESCAPE_HANDLE    hDevice,
                                      GMM_GFX_ADDRESS      GfxAddress,
                                      GMM_GFX_SIZE_T       Size,
                                      GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;
    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(GfxAddress);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_HEAP *GMM_STDCALL GmmUmSetupHeap(GMM_ESCAPE_HANDLE    hAdapter,
                                     GMM_ESCAPE_HANDLE    hDevice,
                                     GMM_GFX_ADDRESS      GfxAddress,
                                     GMM_GFX_SIZE_T       Size,
                                     uint32_t             Flags,
                                     GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_HEAP *pHeapObj = NULL;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(GfxAddress);
    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(pfnEscape);

    return pHeapObj;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmUmDestroypHeap(GMM_ESCAPE_HANDLE    hAdapter,
                                         GMM_ESCAPE_HANDLE    hDevice,
                                         GMM_HEAP **          pHeapObj,
                                         GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_STATUS Status = GMM_ERROR;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(pHeapObj);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmAllocateHeapVA(GMM_HEAP *     pHeapObj,
                                              GMM_GFX_SIZE_T AllocSize)
{
    GMM_GFX_ADDRESS GfxAddress = 0;

    UNREFERENCED_PARAMETER(pHeapObj);
    UNREFERENCED_PARAMETER(AllocSize);

    return GfxAddress;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmFreeHeapVA(GMM_HEAP *      pHeapObj,
                                     GMM_GFX_ADDRESS AllocVA,
                                     GMM_GFX_SIZE_T  AllocSize)
{
    GMM_STATUS Status = GMM_ERROR;

    UNREFERENCED_PARAMETER(pHeapObj);
    UNREFERENCED_PARAMETER(AllocSize);
    UNREFERENCED_PARAMETER(AllocVA);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResDestroySvmAllocation(GMM_ESCAPE_HANDLE    hAdapter,
                                               GMM_ESCAPE_HANDLE    hDevice,
                                               D3DKMT_HANDLE        hAllocation,
                                               GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(hAllocation);
    UNREFERENCED_PARAMETER(pfnEscape);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmResUpdateAfterSharedOpen(HANDLE               hAdapter,
                                                   HANDLE               hDevice,
                                                   D3DKMT_HANDLE        hAllocation,
                                                   GMM_RESOURCE_INFO *  pGmmResInfo,
                                                   GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    GMM_STATUS Status = GMM_ERROR;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(hAllocation);
    UNREFERENCED_PARAMETER(pfnEscape);
    UNREFERENCED_PARAMETER(pGmmResInfo);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmRes32bitAlias for OCL Client using pGmmOclClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmKmdResGetSetHardwareProtection(GMM_ESCAPE_HANDLE    hAdapter,
                                                      GMM_ESCAPE_HANDLE    hDevice,
                                                      D3DKMT_HANDLE        hAllocation,
                                                      GMM_RESOURCE_INFO *  pGmmResource,
                                                      uint8_t              SetIsEncrypted,
                                                      uint8_t *            pGetIsEncrypted,
                                                      GMM_ESCAPE_FUNC_TYPE pfnEscape)
{
    uint8_t Status = 0;

    UNREFERENCED_PARAMETER(hAdapter);
    UNREFERENCED_PARAMETER(hDevice);
    UNREFERENCED_PARAMETER(hAllocation);
    UNREFERENCED_PARAMETER(pfnEscape);
    UNREFERENCED_PARAMETER(pGmmResource);
    UNREFERENCED_PARAMETER(SetIsEncrypted);
    UNREFERENCED_PARAMETER(pGetIsEncrypted);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Instantiates GmmPageTableMgr, allocating memory for root-tables, copies provided
/// device-callback and Batch-buffer function pointers
///
/// @param[in]  DeviceCB: pointer sharing device-callback function pointers
/// @param[in]  TTFlags: Flags specifying which PageTables are required by client
/// @param[in]  TTCB: pointer sharing Batch Buffer function pointers
/// @return     GmmPageTableMgr*
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::GmmPageTableMgr(GMM_DEVICE_CALLBACKS *DeviceCB, uint32_t TTFlags, GMM_TRANSLATIONTABLE_CALLBACKS *TTCB)
{
    UNREFERENCED_PARAMETER(DeviceCB);
    UNREFERENCED_PARAMETER(TTFlags);
    UNREFERENCED_PARAMETER(TTCB);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Instantiates GmmPageTableMgr, allocating memory for root-tables, copies provided
/// device-callback and Batch-buffer function pointers
///
/// @param[in]  DeviceCB: pointer sharing device-callback function pointers
/// @param[in]  TTFlags: Flags specifying which PageTables are required by client
/// @param[in]  TTCB: pointer sharing Batch Buffer function pointers
/// @return     GmmPageTableMgr*
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::GmmPageTableMgr(GMM_DEVICE_CALLBACKS *DeviceCB, uint32_t TTFlags)
{
    UNREFERENCED_PARAMETER(DeviceCB);
    UNREFERENCED_PARAMETER(TTFlags);
}


#endif
#endif

#endif //#if _WIN32
