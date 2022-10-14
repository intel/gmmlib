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

#if defined (__linux__) && !defined(__i386__)

#ifndef _ISOC11_SOURCE
#define _ISOC11_SOURCE 1
#endif

#include "GmmGen10ResourceULT.h"
#include <stdlib.h>
#include <malloc.h>

#ifndef ALIGN
#define ALIGN(v, a) (((v) + ((a)-1)) & ~((a)-1))
#endif

class CTestAuxTable : public CTestGen10Resource
{

public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    CTestAuxTable();
    ~CTestAuxTable();

    static int  allocCB(void *bufMgr, size_t size, size_t alignment, void **bo, void **cpuAddr, uint64_t *gpuAddr);
    static void freeCB(void *bo);
    static void waitFromCpuCB(void *bo);

    class Surface
    {
    public:
        Surface(unsigned int width, unsigned int height, bool mmc = true)
            : mWidth(width), mHeight(height), mMMC(mmc), mResInfo(0), mBuf(0)
        {
        }

        ~Surface()
        {
            deinit();
        }

        bool init()
        {
            size_t size;
            size_t alignment;

            GMM_RESCREATE_PARAMS gmmParams       = {};
            gmmParams.Type                       = RESOURCE_2D;
            gmmParams.Format                     = GMM_FORMAT_NV12;
            gmmParams.BaseWidth                  = mWidth;
            gmmParams.BaseHeight                 = mHeight;
            gmmParams.Depth                      = 0x1;
            gmmParams.ArraySize                  = 1;
            gmmParams.Flags.Info.TiledY          = 1;
            gmmParams.Flags.Info.MediaCompressed = mMMC ? 1 : 0;
            //gmmParams.Flags.Gpu.CCS               = mmc ? 1 : 0;
            gmmParams.Flags.Gpu.MMC               = mMMC ? 1 : 0;
            gmmParams.Flags.Gpu.Texture           = 1;
            gmmParams.Flags.Gpu.RenderTarget      = 1;
            gmmParams.Flags.Gpu.UnifiedAuxSurface = mMMC ? 1 : 0;
            //gmmParams.Flags.Gpu.Depth             = 1;
            gmmParams.Flags.Gpu.Video = true;

            mResInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            size = mResInfo->GetSizeSurface();

            alignment = mResInfo->GetResFlags().Info.TiledYf ? GMM_KBYTE(16) : GMM_KBYTE(64);

            mBuf = aligned_alloc(alignment, ALIGN(size, alignment));

            if(!mResInfo || !mBuf)
                return false;

            mYBase      = (GMM_GFX_ADDRESS)mBuf;
            mUVBase     = 0;
            mAuxYBase   = mYBase + mResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);
            mAuxUVBase  = 0;
            mYPlaneSize = mResInfo->GetSizeMainSurface();

            if(pGmmULTClientContext->IsPlanar(mResInfo->GetResourceFormat()))
            {
                GMM_REQ_OFFSET_INFO ReqInfo = {0};
                ReqInfo.Plane               = GMM_PLANE_U;
                ReqInfo.ReqRender           = 1;

                mResInfo->GetOffset(ReqInfo);
                mYPlaneSize = ReqInfo.Render.Offset64;

                mUVBase    = mYBase + mYPlaneSize;
                mAuxUVBase = mYBase + mResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_UV_CCS);
            }

            mUVPlaneSize = mResInfo->GetSizeMainSurface() - mYPlaneSize;

            return true;
        }

        void deinit()
        {
            if(mBuf)
            {
                free(mBuf);
                mBuf = NULL;
            }

            if(mResInfo)
            {
                pGmmULTClientContext->DestroyResInfoObject(mResInfo);
                mResInfo = NULL;
            }
        }

        GMM_GFX_ADDRESS getGfxAddress(GMM_YUV_PLANE plane)
        {
            switch(plane)
            {
                case GMM_PLANE_Y:
                    return mYBase;
                case GMM_PLANE_U:
                case GMM_PLANE_V:
                    return mUVBase;
                default:
                    throw;
            }
        }

        GMM_GFX_ADDRESS getAuxGfxAddress(GMM_UNIFIED_AUX_TYPE auxType)
        {
            switch(auxType)
            {
                case GMM_AUX_CCS:
                case GMM_AUX_Y_CCS:
                    return mAuxYBase;
                case GMM_AUX_UV_CCS:
                    return mAuxUVBase;
                default:
                    throw;
            }
        }

        GMM_RESOURCE_INFO *getGMMResourceInfo()
        {
            return mResInfo;
        }

        size_t getSurfaceSize(GMM_YUV_PLANE plane)
        {
            switch(plane)
            {
                case GMM_PLANE_Y:
                    return mYPlaneSize;
                case GMM_PLANE_U:
                case GMM_PLANE_V:
                    return mUVPlaneSize;
                default:
                    throw;
            }
        }

    private:
        unsigned int       mWidth;
        unsigned int       mHeight;
        bool               mMMC;
        GMM_RESOURCE_INFO *mResInfo;
        void *             mBuf;
        GMM_GFX_ADDRESS    mYBase;
        GMM_GFX_ADDRESS    mUVBase;
        GMM_GFX_ADDRESS    mAuxYBase;
        GMM_GFX_ADDRESS    mAuxUVBase;
        size_t             mYPlaneSize;
        size_t             mUVPlaneSize;
    };

    class Walker
    {
    public:
        Walker(GMM_GFX_ADDRESS mainBase, GMM_GFX_ADDRESS auxBase,
               GMM_GFX_ADDRESS l3Base)
        {
            mMainBase = mainBase;
            mAuxBase  = (auxBase >> 6) << 6;
            mL3Base   = (uint64_t *)l3Base;
        }

        GMM_GFX_ADDRESS expected(GMM_GFX_ADDRESS addr)
        {
            uint8_t  Is64KChunk = (const_cast<WA_TABLE &>(pGfxAdapterInfo->WaTable).WaAuxTable16KGranular) ? 0 : 1;
            uint32_t count      = (addr - mMainBase) / (Is64KChunk ? GMM_KBYTE(64) : GMM_KBYTE(16));
            return mAuxBase + (Is64KChunk ? 256 : 64) * count;
        }

        GMM_GFX_ADDRESS walk(GMM_GFX_ADDRESS addr)
        {
            uint64_t  mask   = (const_cast<WA_TABLE &>(pGfxAdapterInfo->WaTable).WaAuxTable16KGranular) ? 0x0000ffffffffffc0 : 0x0000ffffffffff00;
            uint32_t  idx    = l3Index(addr);
            uint64_t *l2Base = (uint64_t *)((mL3Base[idx] >> 15) << 15);
            idx              = l2Index(addr);
            uint64_t *l1Base = (uint64_t *)((l2Base[idx] >> 13) << 13);
            idx              = l1Index(addr);
            uint64_t auxAddr = (uint64_t)(l1Base[idx] & mask);
            return auxAddr;
        }

    public:
        static inline uint32_t l3Index(GMM_GFX_ADDRESS addr)
        {
            return GMM_AUX_L3_ENTRY_IDX(addr);
        }

        static inline uint32_t l2Index(GMM_GFX_ADDRESS addr)
        {
            return GMM_AUX_L2_ENTRY_IDX(addr);
        }

        static inline uint32_t l1Index(GMM_GFX_ADDRESS addr)
        {
            return GMM_AUX_L1_ENTRY_IDX_EXPORTED_2(addr, (const_cast<WA_TABLE &>(pGfxAdapterInfo->WaTable).WaAuxTable64KGranular), (const_cast<WA_TABLE &>(pGfxAdapterInfo->WaTable).WaAuxTable16KGranular));
        }

    private:
        GMM_GFX_ADDRESS mMainBase;
        GMM_GFX_ADDRESS mAuxBase;
        uint64_t *      mL3Base;
    };
};

#endif /* __linux__ */
