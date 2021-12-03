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

#ifdef __cplusplus
#include "../inc/External/Common/GmmMemAllocator.hpp"
#include "../inc/External/Common/GmmTextureExt.h"

namespace GmmLib {
    
    class Context;
    class NON_PAGED_SECTION PlatformInfo : public GmmMemAllocator
    {
    public:
        static int32_t RefCount;
        static int32_t OverrideRefCount;

    protected:
        GMM_PLATFORM_INFO Data;
        Context *         pGmmLibContext;

    public:
        PlatformInfo(PLATFORM &Platform, Context* pGmmLibContext);
        virtual ~PlatformInfo()
        {
        }

        const GMM_PLATFORM_INFO& GetData()
        {
            return Data;
        }

        virtual void ApplyExtendedTexAlign(uint32_t CCSMode, ALIGNMENT& UnitAlign)
        {
            GMM_UNREFERENCED_PARAMETER(CCSMode);
            GMM_UNREFERENCED_PARAMETER(UnitAlign);
        }

        virtual uint8_t OverrideCompressionFormat(GMM_RESOURCE_FORMAT Format, uint8_t IsMC)
        {
            GMM_UNREFERENCED_PARAMETER(Format);
            GMM_UNREFERENCED_PARAMETER(IsMC);
            return 0;
        }

        void SetDataSurfaceMaxSize(uint64_t Size)
        {
            Data.SurfaceMaxSize = Size;
        }
        void SetDataFBCRequiredStolenMemorySize(uint32_t Size)
        {
            Data.FBCRequiredStolenMemorySize = Size;
        }
        void SetDataNumberFenceRegisters(uint32_t Number)
        {
            Data.NumberFenceRegisters = Number;
        }
        virtual void SetCCSFlag(GMM_RESOURCE_FLAG &Flags);
        virtual uint8_t ValidateMMC(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t ValidateCCS(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t ValidateUnifiedAuxSurface(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t CheckFmtDisplayDecompressible(GMM_TEXTURE_INFO &Surf,
                                                      bool IsSupportedRGB64_16_16_16_16,
                                                      bool IsSupportedRGB32_8_8_8_8,
                                                      bool IsSupportedRGB32_2_10_10_10,
                                                      bool IsSupportedMediaFormats);

    };
}
#define GMM_PLATFORM_INFO_CLASS GmmLib::PlatformInfo
#else
typedef struct PlatformInfo PlatformInfo;
#define GMM_PLATFORM_INFO_CLASS PlatformInfo
#endif

//***************************************************************************
//
//                      GMM_PLATFORM_INFO Internl API
//
//***************************************************************************
#define SET_TILE_MODE_INFO(Mode, _Width, _Height, _Depth, _MtsWidth, _MtsHeight, _MtsDepth) \
{                                                                                           \
    Data.TileInfo[Mode].LogicalTileWidth      = GMM_BYTES(_Width);                          \
    Data.TileInfo[Mode].LogicalTileHeight     = GMM_SCANLINES(_Height);                     \
    Data.TileInfo[Mode].LogicalTileDepth      = (_Depth);                                   \
    Data.TileInfo[Mode].LogicalSize           = (_Width) * (_Height) * (_Depth);            \
    Data.TileInfo[Mode].MaxPitch              = GMM_KBYTE(256);                             \
    Data.TileInfo[Mode].MaxMipTailStartWidth  = (_MtsWidth);                                \
    Data.TileInfo[Mode].MaxMipTailStartHeight = (_MtsHeight);                               \
    Data.TileInfo[Mode].MaxMipTailStartDepth  = (_MtsDepth);                                \
}

#if __cplusplus
extern "C" {
#endif

    const GMM_PLATFORM_INFO *GMM_STDCALL __GmmGetPlatformInfo(void *pLibContext);
    void GMM_STDCALL __SetNumberFenceRegisters(void *pLibContext, uint32_t Number);

#if __cplusplus
}
#endif
