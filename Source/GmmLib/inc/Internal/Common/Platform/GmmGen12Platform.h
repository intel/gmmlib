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
#include "GmmGen11Platform.h"

typedef struct __CCS_UNIT
{
    ALIGNMENT Align;
    struct
    {
        //represents downscale factor if msb = 0,
        //             upscale factor if msb = 1,
        //factor value is absolute (+ve)
        int32_t Width;
        int32_t Height;
        uint32_t Depth;  //Depth slices or Samples sharing CCS$line
    } Downscale;
} CCS_UNIT;

//Gen12 CCS supported on Yf/Ys 2D/MSAA/3D tiling
#define CCS_MODES          (GMM_TILE_MODES - TILE_YF_2D_8bpe)
#define CCS_MODE(x)        (x >= TILE_YF_2D_8bpe) ? (x - TILE_YF_2D_8bpe) : CCS_MODES

typedef enum _FC_TileType
{
    FC_TILE_Y,
    FC_TILE_YF,
    FC_TILE_YS,
    FC_TILE_4,
    FC_TILE_64,
    //max equals last supported plus one
    FC_TILE_MAX
} FC_TILE_TYPE;

#define FCTilingType(x)    (((x) == LEGACY_TILE_Y) ? (FC_TILE_Y) : \
                           (((x) == TILE4) ? (FC_TILE_4) : \
                           (((x) >= TILE_YF_2D_8bpe && (x) <= TILE_YF_2D_128bpe) ? (FC_TILE_YF) : \
                           (((x) >= TILE_YS_2D_8bpe && (x) <= TILE_YS_2D_128bpe) ? (FC_TILE_YS) : \
                           (((x) >= TILE__64_2D_8bpe && (x) <= TILE__64_2D_128bpe) ? (FC_TILE_64) : \
                           (FC_TILE_MAX))))))
#define FCMaxBppModes      5
#define FCMaxModes         FC_TILE_MAX * FCMaxBppModes
#define FCBppMode(bpp)     __GmmLog2(bpp) - 3
#define FCMode(TileMode, bpp)  (FCTilingType(TileMode) < FC_TILE_MAX) ? (FCTilingType(TileMode) * FCMaxBppModes + FCBppMode(bpp)) : FCMaxModes

//===========================================================================
// typedef:
//        GMM_TEXTURE_ALIGN_EX
//
// Description:
//      The following struct extends the texture mip map unit alignment
//      required for each map format. The alignment values are platform
//      dependent.
//
//---------------------------------------------------------------------------
typedef struct GMM_TEXTURE_ALIGN_EX_REC
{
    CCS_UNIT CCSEx[CCS_MODES];
}GMM_TEXTURE_ALIGN_EX;

typedef enum GMM_FLATCCS_FORMAT_ENUM
{
    GMM_FLATCCS_FORMAT_R16S = 0,
    GMM_FLATCCS_FORMAT_R16U = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RG16F = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RG16U = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RG16S = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RGBA16S = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RGBA16U = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RGBA16F = GMM_FLATCCS_FORMAT_R16S,

    GMM_FLATCCS_MIN_RC_FORMAT = GMM_FLATCCS_FORMAT_R16S,

    GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_R32S = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_R32U = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RG32F = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RG32S = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RG32U = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RGBA32F = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RGBA32S = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RGBA32U = GMM_FLATCCS_FORMAT_R32F,

    GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RGBA4 = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_B5G6R5 = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_R8S = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_R8U = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RG8S = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RG8U = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RGBA8S = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RGBA8U = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_ML8    = GMM_FLATCCS_FORMAT_RGB5A1,

    GMM_FLATCCS_FORMAT_RGB10A2,
    GMM_FLATCCS_FORMAT_RG11B10,

    GMM_FLATCCS_FORMAT_R32F1,
    GMM_FLATCCS_FORMAT_R32S1 = GMM_FLATCCS_FORMAT_R32F1,
    GMM_FLATCCS_FORMAT_R32U1 = GMM_FLATCCS_FORMAT_R32F1,

    GMM_FLATCCS_FORMAT_R16F1,
    GMM_FLATCCS_FORMAT_R16S1 = GMM_FLATCCS_FORMAT_R16F1,
    GMM_FLATCCS_FORMAT_R16U1 = GMM_FLATCCS_FORMAT_R16F1,

    GMM_FLATCCS_FORMAT_R8S1,
    GMM_FLATCCS_FORMAT_R8U1 = GMM_FLATCCS_FORMAT_R8S1,

    GMM_FLATCCS_MAX_RC_FORMAT = GMM_FLATCCS_FORMAT_R8U1,

	GMM_FLATCCS_MIN_MC_FORMAT = 0x21,               //(0x1 <<5) ie Msb-5th bit turned on to identify MC encoding, to drop before SurfaceState usage
    GMM_FLATCCS_FORMAT_RGBA16_MEDIA = GMM_FLATCCS_MIN_MC_FORMAT,
    GMM_FLATCCS_FORMAT_Y210,
    GMM_FLATCCS_FORMAT_YUY2,
    GMM_FLATCCS_FORMAT_Y410,
    GMM_FLATCCS_FORMAT_Y216,
    GMM_FLATCCS_FORMAT_Y416,
    GMM_FLATCCS_FORMAT_P010,
    GMM_FLATCCS_FORMAT_P016,
    GMM_FLATCCS_FORMAT_AYUV,
    GMM_FLATCCS_FORMAT_ARGB8b,
    GMM_FLATCCS_FORMAT_SWAPY,
    GMM_FLATCCS_FORMAT_SWAPUV,
    GMM_FLATCCS_FORMAT_SWAPUVY,
    GMM_FLATCCS_FORMAT_RGB10b,
    GMM_FLATCCS_FORMAT_NV12,

    GMM_FLATCCS_FORMAT_YCRCB_SWAPUV = GMM_FLATCCS_FORMAT_SWAPUV,
    GMM_FLATCCS_FORMAT_YCRCB_SWAPUVY = GMM_FLATCCS_FORMAT_SWAPUVY,
    GMM_FLATCCS_FORMAT_YCRCB_SWAPY = GMM_FLATCCS_FORMAT_SWAPY,

    GMM_FLATCCS_MAX_MC_FORMAT = GMM_FLATCCS_FORMAT_NV12,    //should always be equal to last format encoding

    GMM_FLATCCS_FORMAT_INVALID,                          //equal to last valid encoding plus one
} GMM_FLATCCS_FORMAT;

#ifdef __cplusplus

namespace GmmLib
{
    class NON_PAGED_SECTION PlatformInfoGen12 : public PlatformInfoGen11
    {
    protected:
        GMM_TEXTURE_ALIGN_EX    TexAlignEx;
        CCS_UNIT                FCTileMode[FCMaxModes];
    public:
        PlatformInfoGen12(PLATFORM &Platform);
        ~PlatformInfoGen12(){};
        virtual GMM_TEXTURE_ALIGN_EX GetExTextureAlign() { return TexAlignEx; }
        virtual void ApplyExtendedTexAlign(uint32_t CCSMode, ALIGNMENT& UnitAlign);
        virtual CCS_UNIT* GetFCRectAlign() { return FCTileMode; }
        virtual void                 SetCCSFlag(GMM_RESOURCE_FLAG &Flags);
        virtual uint8_t              ValidateMMC(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t              ValidateCCS(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t              ValidateUnifiedAuxSurface(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t              CheckFmtDisplayDecompressible(GMM_TEXTURE_INFO &Surf,
                                                                   bool IsSupportedRGB64_16_16_16_16,
                                                                   bool IsSupportedRGB32_8_8_8_8,
                                                                   bool IsSupportedRGB32_2_10_10_10,
                                                                   bool IsSupportedMediaFormats);
        virtual uint8_t OverrideCompressionFormat(GMM_RESOURCE_FORMAT Format, uint8_t IsMC);
    };
}

#endif
