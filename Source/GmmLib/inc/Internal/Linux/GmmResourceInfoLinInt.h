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

#if defined(__linux__) && !LHDM

// GMM Lib Client Exports
#include "External/Common/GmmCommonExt.h"
#include "External/Common/GmmUtil.h"
#include "External/Common/GmmResourceFlags.h"
#include "External/Common/GmmCachePolicy.h"
#include "External/Common/GmmCachePolicyExt.h"
#include "External/Common/GmmResourceInfoExt.h"
#include "External/Common/GmmPlatformExt.h"
#include "External/Common/GmmTextureExt.h"
#include "External/Common/GmmInfoExt.h"
#include "External/Common/GmmResourceInfo.h"
#include "External/Common/GmmInfo.h"

typedef struct {
    size_t size;
    size_t alignment;
    void *bo;
    void *cpuAddr;
    uint64_t gfxAddr;
} GMM_DDI_ALLOCATE;

typedef struct  {
    void *bo;
} GMM_DDI_DEALLOCATE;

typedef struct {
    void *bo;
}GMM_DDI_WAITFORSYNCHRONIZATIONOBJECTFROMCPU;

typedef enum GMM_DEVICE_CALLBACKS_TYPE_REC
{
    GMM_DEV_CB_ALLOC = 0,
    GMM_DEV_CB_DEALLOC,
    GMM_DEV_CB_WAIT_FROM_CPU,
} GMM_DEVICE_CALLBACKS_TYPE;

int GmmDeviceCallback(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DDI_ALLOCATE *pAllocate);
int GmmDeviceCallback(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DDI_DEALLOCATE *pDeallocate);
int GmmDeviceCallback(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DDI_WAITFORSYNCHRONIZATIONOBJECTFROMCPU *pWait);

int GmmCheckForNullDevCbPfn(GMM_CLIENT ClientType, GMM_DEVICE_CALLBACKS_INT *pDeviceCb, GMM_DEVICE_CALLBACKS_TYPE CallBackType);

extern GMM_TRANSLATIONTABLE_CALLBACKS DummyTTCB;

#endif /*__linux__*/
