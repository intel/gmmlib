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
#pragma once

#ifdef __linux__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef struct {
    int(*pfnAllocate)(void *bufMgr, size_t size, size_t alignment, void **bo, void **cpuAddr, uint64_t *gpuAddr);
    void(*pfnDeallocate)(void *bo);
    void(*pfnWaitFromCpu)(void *bo);;
} GMM_DEVICE_CB_PTRS;

typedef struct _GMM_DEVICE_CALLBACKS_INT
{
    void *pBufMgr;
    GMM_DEVICE_CB_PTRS DevCbPtrs_;
} GMM_DEVICE_CALLBACKS_INT;


// Add the definition to compatible.
typedef struct GMM_TRANSLATIONTABLE_CALLBACKS_REC
{
    int (*pfPrologTranslationTable)(void *pDeviceHandle);

    int (*pfWriteL1Entries)(void *pDeviceHandle,
                            const uint32_t NumEntries,
                            GMM_GFX_ADDRESS GfxAddress,
                            uint32_t *Data);

    int (*pfWriteL2L3Entry)(void *pDeviceHandle,
                            GMM_GFX_ADDRESS GfxAddress,
                            uint64_t Data);

    int (*pfWriteFenceID)(void *pDeviceHandle,
                          GMM_GFX_ADDRESS GfxAddress,
                          uint64_t Data);

    int (*pfEpilogTranslationTable)(void *pDeviceHandle,
                                    uint8_t ForceFlush);

    int (*pfCopyL1Entry)(void *pDeviceHandle,
                         GMM_GFX_ADDRESS DstGfxAddress,
                         GMM_GFX_ADDRESS SrcGfxAddress);

    int (*pfWriteL3Adr)(void *pDeviceHandle,
                        GMM_GFX_ADDRESS L3GfxAddress,
                        uint64_t RegOffset);
} GMM_TRANSLATIONTABLE_CALLBACKS;

typedef struct _GMM_DEVICE_CALLBACKS
{
    void *pBufferMgr;
    int FuncDevice;
    int(*pfnAllocate)(void *bufMgr, size_t size, size_t alignment, void **bo, void **cpuAddr, uint64_t *gpuAddr);
    void(*pfnDeallocate)(void *bo);
    void(*pfnWaitFromCpu)(void *bo);;
} GMM_DEVICE_CALLBACKS;


// This definition is only for code sharing.
typedef struct GMM_UMD_SYNCCONTEXT_REC
{
    void                *pCommandQueueHandle;   // pointer to command queue handle
    void                *pUpdateGpuVaInfo;
    HANDLE              BBFenceObj;             // BatchBuffer Last Fence, for CPU to wait on before destroying TT pages
    uint64_t            BBLastFence;            // BatchBuffer Last Fence for TT
} GMM_UMD_SYNCCONTEXT;

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__linux__*/
