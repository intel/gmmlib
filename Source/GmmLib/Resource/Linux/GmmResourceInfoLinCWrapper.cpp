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

#include "Internal/Common/GmmLibInc.h"

#if defined(__linux__) && !LHDM
#include "Internal/Linux/GmmResourceInfoLinInt.h"

/////////////////////////////////////////////////////////////////////////////////////
/// This is an overloaded function to call DeviceCb Function for CreateAllocation
///
/// @param[in]  ClientType
/// @param[in]  pDeviceCb: Pointer to GMM_DEVICE_CALLBACKS_INT Struct
/// @param[in]  pAllocate: Pointer to GMM_DDI_ALLOCATE Union
/// @return     Status of CreateAllocation call.
/////////////////////////////////////////////////////////////////////////////////////
int GmmDeviceCallback(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DDI_ALLOCATE *pAllocate)
{
    int      Status   = 0;
    void *   pBo      = NULL;
    void *   pCpuAddr = NULL;
    uint64_t gpuAddr  = 0ULL;

    Status = pDeviceCb->DevCbPtrs_.pfnAllocate(pDeviceCb->pBufMgr,
                                               pAllocate->size,
                                               pAllocate->alignment,
                                               &pBo,
                                               &pCpuAddr,
                                               &gpuAddr);
    pAllocate->bo      = pBo;
    pAllocate->cpuAddr = pCpuAddr;
    pAllocate->gfxAddr = gpuAddr;

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This is an overloaded function to call DeviceCb Function for DestroyAllocation
///
/// @param[in]  ClientType
/// @param[in]  pDeviceCb: Pointer to GMM_DEVICE_CALLBACKS_INT Struct
/// @param[in]  pAllocate: Pointer to GMM_DDI_DEALLOCATE Union
/// @return     Status of DestroyAllocation call.
/////////////////////////////////////////////////////////////////////////////////////
int GmmDeviceCallback(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DDI_DEALLOCATE *pDeallocate)
{
    int Status = 0;

    pDeviceCb->DevCbPtrs_.pfnDeallocate(pDeallocate->bo);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This is an overloaded function to call DeviceCb Function for WaitForSyncObjFromCpu
///
/// @param[in]  ClientType
/// @param[in]  pDeviceCb: Pointer to GMM_DEVICE_CALLBACKS_INT Struct
/// @param[in]  pAllocate: Pointer to GMM_DDI_WAITFORSYNCHRONIZATIONOBJECTFROMCPU Union
/// @return     Status of WaitForSyncObjFromCpu call.
/////////////////////////////////////////////////////////////////////////////////////
int GmmDeviceCallback(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DDI_WAITFORSYNCHRONIZATIONOBJECTFROMCPU *pWait)
{
    int Status = 0;

    pDeviceCb->DevCbPtrs_.pfnWaitFromCpu(pWait->bo);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function checks for Null DeviceCb Function pointer
///
/// @param[in]  ClientType
/// @param[in]  pDeviceCb: Pointer to GMM_DEVICE_CALLBACKS_INT Struct
/// @param[in]  CallBackType Enum @GMM_DEVICE_CALLBACKS_TYPE
/// @return     True if not Null.
/////////////////////////////////////////////////////////////////////////////////////
int GmmCheckForNullDevCbPfn(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DEVICE_CALLBACKS_TYPE CallBackType)
{
    int Status = 0;

    switch(CallBackType)
    {
        case GMM_DEV_CB_ALLOC:
            Status = (pDeviceCb->DevCbPtrs_.pfnAllocate != 0);
            break;
        case GMM_DEV_CB_DEALLOC:
            Status = (pDeviceCb->DevCbPtrs_.pfnDeallocate != 0);
            break;
        case GMM_DEV_CB_WAIT_FROM_CPU:
            Status = (pDeviceCb->DevCbPtrs_.pfnWaitFromCpu != 0);
            break;
        default:
            Status = 0;
            break;
    }

    return Status;
}

// Dummy Translation Table Callback for reusing ..
static inline int DummyPrologTranslationTable(void *pDeviceHandle)
{
    return 0;
}

static inline int DummyWriteL1Entries(void *          pDeviceHandle,
                                      const uint32_t  NumEntries,
                                      GMM_GFX_ADDRESS GfxAddress,
                                      uint32_t *      Data)
{
    return 0;
}

static inline int DummyWriteL2L3Entry(void *          pDeviceHandle,
                                      GMM_GFX_ADDRESS GfxAddress,
                                      uint64_t        Data)
{
    return 0;
}

static inline int DummyWriteFenceID(void *          pDeviceHandle,
                                    GMM_GFX_ADDRESS GfxAddress,
                                    uint64_t        Data)
{
    return 0;
}

static inline int DummyEpilogTranslationTable(void *  pDeviceHandle,
                                              uint8_t ForceFlush)
{
    return 0;
}

static inline int DummyCopyL1Entry(void *          pDeviceHandle,
                                   GMM_GFX_ADDRESS DstGfxAddress,
                                   GMM_GFX_ADDRESS SrcGfxAddress)
{
    return 0;
}

static inline int DummyWriteL3Adr(void *          pDeviceHandle,
                                  GMM_GFX_ADDRESS L3GfxAddress,
                                  uint64_t        RegOffset)
{
    return 0;
}

GMM_TRANSLATIONTABLE_CALLBACKS DummyTTCB = {
.pfPrologTranslationTable = DummyPrologTranslationTable,
.pfWriteL1Entries         = DummyWriteL1Entries,
.pfWriteL2L3Entry         = DummyWriteL2L3Entry,
.pfWriteFenceID           = DummyWriteFenceID,
.pfEpilogTranslationTable = DummyEpilogTranslationTable,
.pfCopyL1Entry            = DummyCopyL1Entry,
.pfWriteL3Adr             = DummyWriteL3Adr,
};

#endif /*__linux__*/
