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
extern "C" {
#endif /*__cplusplus*/

// Set packing alignment
#pragma pack(push, 8)

#define GMM_MAX_MIPMAP      15
#define GMM_MAX_MMC_INDEX   64

//===========================================================================
// typedef:
//        GMM_PLANAR_OFFSET_INFO
//
// Description:
//     This structure stores the offset address of each level
//---------------------------------------------------------------------------
typedef struct GMM_PLANAR_OFFSET_INFO_REC
{
    GMM_GFX_SIZE_T     ArrayQPitch;
    GMM_GFX_SIZE_T     X[GMM_MAX_PLANE];
    GMM_GFX_SIZE_T     Y[GMM_MAX_PLANE];
    struct
    {
        GMM_GFX_SIZE_T     Height[GMM_MAX_PLANE];
    } UnAligned, Aligned;
    uint32_t            NoOfPlanes;
    bool                IsTileAlignedPlanes;
}GMM_PLANAR_OFFSET_INFO;

//===========================================================================
// typedef:
//        GMM_2D_TEXTURE_OFFSET_INFO
//
// Description:
//     This structure stores the offset address of each level
//---------------------------------------------------------------------------
typedef struct GMM_2D_TEXTURE_OFFSET_INFO_REC
{
    GMM_GFX_SIZE_T              ArrayQPitchLock;
    GMM_GFX_SIZE_T              ArrayQPitchRender;
    GMM_GFX_SIZE_T              Offset[GMM_MAX_MIPMAP];
}GMM_2D_TEXTURE_OFFSET_INFO_T;

//===========================================================================
// typedef:
//        GMM_3D_TEXTURE_OFFSET_INFO
//
// Description:
//     This structure stores the offset address of each level
//---------------------------------------------------------------------------
typedef struct GMM_3D_TEXTURE_OFFSET_INFO_REC
{
    GMM_GFX_SIZE_T     Mip0SlicePitch;
    GMM_GFX_SIZE_T     Offset[GMM_MAX_MIPMAP];
}GMM_3D_TEXTURE_OFFSET_INFO_T;

//===========================================================================
// typedef:
//        GMM_OFFSET_INFO
//
// Description:
//     This structure stores the offset address of each level
//---------------------------------------------------------------------------
typedef struct GMM_OFFSET_INFO_REC
{
    union
    {
        GMM_3D_TEXTURE_OFFSET_INFO_T    Texture3DOffsetInfo;
        GMM_2D_TEXTURE_OFFSET_INFO_T    Texture2DOffsetInfo;
        GMM_PLANAR_OFFSET_INFO          Plane;
    };
}GMM_OFFSET_INFO, GMM_OFFSET_INFO_T;

//===========================================================================
// typedef:
//        GMM_TEXTURE_CALC
//
// Description:
//     This struct is used to texture calculator.
//     Forward Declaration: Defined in GmmGenXXTextureCalc*.h
//---------------------------------------------------------------------------
#ifdef __cplusplus
namespace GmmLib
{
    class GmmTextureCalc;
}

typedef GmmLib::GmmTextureCalc GMM_TEXTURE_CALC;
#else
struct GmmTextureCalc;
typedef struct GmmTextureCalc GMM_TEXTURE_CALC;
#endif

//===========================================================================
// typedef:
//      GMM_TEXTURE_INFO_REC
//
// Description:
//      This structure used to request mipmap offset information
//---------------------------------------------------------------------------
typedef struct GMM_TEXTURE_INFO_REC
{
     // Input ----------------------------------
    GMM_RESOURCE_TYPE       Type;
    GMM_RESOURCE_FORMAT     Format;
    uint32_t                BitsPerPixel;
    GMM_RESOURCE_FLAG       Flags;

    uint64_t                BaseWidth;
    uint32_t                BaseHeight;
    uint32_t                Depth;
    uint32_t                MaxLod;
    uint32_t                ArraySize;
    uint32_t                CpTag;

    struct{
        GMM_RESOURCE_USAGE_TYPE Usage;
    }                       CachePolicy;

    GMM_RESOURCE_MSAA_INFO  MSAA;

    // Output ---------------------------------
    GMM_RESOURCE_ALIGNMENT  Alignment;

    uint8_t                 MmcMode[GMM_MAX_MMC_INDEX];
    uint8_t                 MmcHint[GMM_MAX_MMC_INDEX];

    GMM_GFX_SIZE_T          Pitch;
    GMM_GFX_SIZE_T          OverridePitch;   // VirtualPadding

    GMM_GFX_SIZE_T          Size;            // For AuxSurf Size = CCS + CC + Padding
    GMM_GFX_SIZE_T          CCSize;          // 4kb => 128 bit Float + 32bit Native RT + Padding. Usage : Gpu.IndirectClearColor
    GMM_GFX_SIZE_T          UnpaddedSize;    // Unpadded CCS Size for Flags.Gpu.UnifiedAuxSurface only
    GMM_GFX_SIZE_T          SizeReportToOS;  // For Non-aligned ESM

    GMM_OFFSET_INFO         OffsetInfo;      // (X,Y) offsets to each mipmap/plane
    GMM_TILE_MODE           TileMode;
    uint32_t                CCSModeAlign;    // For AUX_CCS, TexAlign.CCSEx index derived from main surface tiling
    uint32_t                LegacyFlags;

    GMM_S3D_INFO            S3d;

    #if(LHDM)
        D3DDDIFORMAT        MsFormat;

    #endif

    struct{
        uint32_t               Seg1    : 8;
        uint32_t               Evict   : 8;
    }                   SegmentOverride; // Used for tuning the Vista driver

    uint32_t               MaximumRenamingListLength;

    #if(_DEBUG || _RELEASE_INTERNAL)
        PLATFORM        Platform;
    #else
        PLATFORM        __Platform; // Do not use--For size preservation only.
    #endif

    struct{
        uint8_t          IsGmmAllocated;
        uint8_t          IsPageAligned;
    }                   ExistingSysMem;
}GMM_TEXTURE_INFO;

//***************************************************************************
//
//                      GMM_TEXTURE API
//
//***************************************************************************
#if(defined(__GMM_KMD__))
GMM_STATUS GmmTexAlloc(GMM_TEXTURE_INFO* pTexInfo);
GMM_STATUS GmmTexLinearCCS(GMM_TEXTURE_INFO* pTexInfo, GMM_TEXTURE_INFO *pAuxTexInfo);
#endif
GMM_STATUS GmmTexGetMipMapOffset(GMM_TEXTURE_INFO* pTexInfo, GMM_REQ_OFFSET_INFO* pReqInfo);

#define GMM_ISNOT_TILED(TileInfo) ((TileInfo).LogicalSize == 0)
#define GMM_IS_TILED(TileInfo)    ((TileInfo).LogicalSize > 0)
#define GMM_CLEAR_TILEINFO(TileInfo)            \
{                                               \
        (TileInfo).LogicalSize = 0;             \
        (TileInfo).LogicalTileHeight = 0;       \
        (TileInfo).LogicalTileWidth = 0;        \
        (TileInfo).MaxPitch = 0;                \
}

#define GMM_IS_4KB_TILE(Flags)  ((Flags).Info.TiledY || (Flags).Info.Tile4)
#define GMM_IS_64KB_TILE(Flags) (Flags.Info.TiledYs || Flags.Info.Tile64)
#define GMM_IS_SUPPORTED_BPP_ON_TILE_64_YF_YS(bpp) ((bpp == 8) || (bpp == 16) || (bpp == 32) || (bpp == 64) || (bpp == 128))

#define GMM_SET_4KB_TILE(Flags, Value) if (pGmmGlobalContext->GetSkuTable().FtrTileY) ((Flags).Info.TiledY = (Value)); else ((Flags).Info.Tile4 = (Value))
#define GMM_SET_64KB_TILE(Flags, Value) if (pGmmGlobalContext->GetSkuTable().FtrTileY) ((Flags).Info.TiledYs = (Value)); else ((Flags).Info.Tile64 = (Value))
#define GMM_SET_4KB_TILE_MODE(TileMode) if (pGmmGlobalContext->GetSkuTable().FtrTileY) (TileMode = LEGACY_TILE_Y); else (TileMode = TILE4)
#define GMM_IS_TILEY (pClientContext->GetSkuTable().FtrTileY)


// Reset packing alignment to project default
#pragma pack(pop)

#ifdef __cplusplus
}
#endif /*__cplusplus*/
