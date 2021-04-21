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
#ifdef _WIN32
#include <intrin.h>
#endif

//64KB Undefined Swizzle Descriptors #########################################
// !! TODO (critical): Provide mapping helper function to UMD to map in non-TR
//                     Undefined64KBSwizzle resources. Also check if support
//                     for it is needed in GmmResCpuBlt.
//
// Note: These 64KB swizzles are not really hardware tile swizzles, so leave
//       them out of the CpuSwizzleBlt.c

/*  On systems that do not support Std Swizzle (IGFX_GEN7_5_CORE / IGFX_GEN8_CORE),
we still have to support 64KB tiles. These 64KB tiles will be made of 16 4KB
tiles -- we'll be using TileY to create these 64KB tiles. The table below shows
how the 64KB tile shape changes depending on the bpp and how we need to arrange
(in columns X rows) the 4KB tiles to fit that shape.

    bpp     Tile Size (in pixels)   Tile Size (in bytes)      4K tile config
    ---     --------------------    --------------------      --------------
    8bpp        256x256                 256x256                     2x8
    16bpp       256x128                 512x128                     4x4
    32bpp       128x128                 512x128                     4x4
    64bpp       128x64                  1024x64                     8x2
    128bpp      64x64                   1024x64                     8x2

We need 3 different swizzle pattern to support all configs above.  The swizzle
patterns are:

          |-Rows-|  |-Column-|   |-TileY Swizzle-|
8bpp:       YYY         X          XXXYYYYYXXXX
16/32bpp:   YY          XX         XXXYYYYYXXXX
64/128bpp:  Y           XXX        XXXYYYYYXXXX

The translation of these Xs and Ys to bitmap is shown below */
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_8bpp      = {0x1E0F, 0xE1F0, 0};
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_16_32bpp  = {0x3E0F, 0xC1F0, 0};
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_64_128bpp = {0x7E0F, 0x81F0, 0};
//#############################################################################

//=============================================================================
// Function:
//    GmmIsRedecribedPlanes
//
// Description:
//     Checks if the resource has redescribed planes
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsRedecribedPlanes(GMM_RESOURCE_INFO *pGmmResource)
{
    return pGmmResource->GetResFlags().Info.RedecribedPlanes;
}

//=============================================================================
// Function:
//    GmmIsStdTilingSupported
//
// Description:
//     Checks if the given ResCreateParams is supported for TileYf/Ys
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsStdTilingSupported(GMM_RESCREATE_PARAMS *pCreateParams)
{
    GMM_TEXTURE_INFO Surface;
    Surface.Type                      = pCreateParams->Type;
    Surface.Format                    = pCreateParams->Format;
    Surface.BaseWidth                 = pCreateParams->BaseWidth64;
    Surface.BaseHeight                = pCreateParams->BaseHeight;
    Surface.Depth                     = pCreateParams->Depth;
    Surface.MaxLod                    = pCreateParams->MaxLod;
    Surface.ArraySize                 = pCreateParams->ArraySize;
    Surface.Flags                     = pCreateParams->Flags;
    Surface.MSAA                      = pCreateParams->MSAA;
    Surface.Alignment.BaseAlignment   = pCreateParams->BaseAlignment;
    Surface.CachePolicy.Usage         = pCreateParams->Usage;
    Surface.MSAA.NumSamples           = GFX_MAX(Surface.MSAA.NumSamples, 1);
    Surface.MaximumRenamingListLength = pCreateParams->MaximumRenamingListLength;
    Surface.OverridePitch             = pCreateParams->OverridePitch;

#if(_DEBUG || _RELEASE_INTERNAL)
    Surface.Platform = pGmmGlobalContext->GetPlatformInfo().Platform;
#endif

    if((pCreateParams->Format > GMM_FORMAT_INVALID) &&
       (pCreateParams->Format < GMM_RESOURCE_FORMATS))
    {
        Surface.BitsPerPixel = pGmmGlobalContext->GetPlatformInfo().FormatTable[pCreateParams->Format].Element.BitsPer;
    }
    else
    {
        GMM_ASSERTDPF(0, "Format Error");
        return 0;
    }

    return __CanSupportStdTiling(Surface);
}

//=============================================================================
// Function:
//    GmmIsUVPacked
//
// Description:
//     Checks if format has packed UV plane
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsUVPacked(GMM_RESOURCE_FORMAT Format)
{
    uint8_t Status = 0;

    switch(Format)
    {
        case GMM_FORMAT_NV11:
        case GMM_FORMAT_NV12:
        case GMM_FORMAT_NV21:
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P012:
        case GMM_FORMAT_P016:
        case GMM_FORMAT_P208:
        case GMM_FORMAT_P216:
            Status = 1;
            break;
        default:
            Status = 0;
            break;
    }
    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Checks if format can be accessed by LCU
///
/// @param[in]  pSurf: ptr to ::GMM_TEXTURE_INFO of main surface
/// @param[in]  pAuxTexInfo: ptr to ::GMM_TEXTURE_INFO of Aux surface
///
/////////////////////////////////////////////////////////////////////////////////////
bool GMM_STDCALL GmmIsYUVFormatLCUAligned(GMM_RESOURCE_FORMAT Format)
{
    bool Status = 0;

    switch(Format)
    {
        case GMM_FORMAT_NV12:
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P016:
        case GMM_FORMAT_YUY2:
        case GMM_FORMAT_Y210:
        case GMM_FORMAT_Y410:
        case GMM_FORMAT_Y216:
        case GMM_FORMAT_Y416:
        case GMM_FORMAT_AYUV:
            Status = true;
            break;
        default:
            Status = false;
            break;
    }
    return Status;
}

//=============================================================================
// Function:
//    GmmIsYUVPacked
//
// Description:
//     Checks if format is a YCRCB_xxx format supported by the sampler.
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsYUVPacked(GMM_RESOURCE_FORMAT Format)
{
    uint8_t Status = 0;

    switch(Format)
    {
        // YCRCB_xxx Format Supported by the Sampler...
        case GMM_FORMAT_YUY2:
        case GMM_FORMAT_YVYU:
        case GMM_FORMAT_UYVY:
        case GMM_FORMAT_VYUY:
        case GMM_FORMAT_YUY2_2x1:
        case GMM_FORMAT_YVYU_2x1:
        case GMM_FORMAT_UYVY_2x1:
        case GMM_FORMAT_VYUY_2x1:
        case GMM_FORMAT_Y210:
        case GMM_FORMAT_Y212:
        case GMM_FORMAT_Y216:
        case GMM_FORMAT_Y410:
        case GMM_FORMAT_Y412:
        case GMM_FORMAT_Y416:
        case GMM_FORMAT_AYUV:
            Status = 1;
            break;
        default:
            Status = 0;
            break;
    }
    return Status;
}

//=============================================================================
// Function:
//    GmmIsPlanar
//
// Description:
//     Checks if format is YUV planar
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsPlanar(GMM_RESOURCE_FORMAT Format)
{
    uint8_t Status = 0;

    switch(Format)
    {
        // YUV Planar Formats
        case GMM_FORMAT_BGRP:
        case GMM_FORMAT_IMC1:
        case GMM_FORMAT_IMC2:
        case GMM_FORMAT_IMC3:
        case GMM_FORMAT_IMC4:
        case GMM_FORMAT_I420: //Same as IYUV.
        case GMM_FORMAT_IYUV:
        case GMM_FORMAT_MFX_JPEG_YUV411:
        case GMM_FORMAT_MFX_JPEG_YUV411R:
        case GMM_FORMAT_MFX_JPEG_YUV420:
        case GMM_FORMAT_MFX_JPEG_YUV422H:
        case GMM_FORMAT_MFX_JPEG_YUV422V:
        case GMM_FORMAT_MFX_JPEG_YUV444:
        case GMM_FORMAT_RGBP:
        case GMM_FORMAT_YV12:
        case GMM_FORMAT_YVU9:
        // YUV Hybrid Formats - GMM treats as Planar
        case GMM_FORMAT_NV11:
        case GMM_FORMAT_NV12:
        case GMM_FORMAT_NV21:
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P012:
        case GMM_FORMAT_P016:
        case GMM_FORMAT_P208:
        case GMM_FORMAT_P216:
            Status = 1;
            break;
        default:
            Status = 0;
            break;
    }
    return Status;
}

//=============================================================================
// Function:
//    GmmIsReconstructableSurface
//
// Description:
//     Checks if format is GmmIsReconstructableSurface
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsReconstructableSurface(GMM_RESOURCE_FORMAT Format)
{
    uint8_t Status = 0;

    switch(Format)
    {
        case GMM_FORMAT_AYUV:
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P012:
        case GMM_FORMAT_P016:
        case GMM_FORMAT_Y210:
        case GMM_FORMAT_Y216:
        case GMM_FORMAT_Y212:
        case GMM_FORMAT_Y410:
        case GMM_FORMAT_Y416:
        case GMM_FORMAT_P8:
        case GMM_FORMAT_NV12:
        case GMM_FORMAT_YUY2_2x1:
        case GMM_FORMAT_YUY2:
            Status = 1;
            break;
        default:
            Status = 0;
            break;
    }
    return Status;
}

//=============================================================================
// Function:
//    GmmIsP0xx
//
// Description:
//     Checks if format is P0xx
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsP0xx(GMM_RESOURCE_FORMAT Format)
{
    uint8_t Status = 0;

    switch(Format)
    {
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P012:
        case GMM_FORMAT_P016:
            Status = 1;
            break;
        default:
            Status = 0;
            break;
    }

    return Status;
}

//=============================================================================
// Function:
//    GmmIsCompressed
//
// Description:
//     Checks if format is compressed
//
// Arguments: <Look at Function Header)
//
// Return:
//    1 or 0
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmIsCompressed(GMM_RESOURCE_FORMAT Format)
{
    return (Format > GMM_FORMAT_INVALID) &&
           (Format < GMM_RESOURCE_FORMATS) &&
           pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].Compressed;
}

//=============================================================================
// Function:
//     GmmGetCacheSizes
//
// Description:
//    This function returns the L3, LLC and EDRAM cache sizes.
//
// Arguments:
//    pCacheSizes ==> ptr to GMM_CACHE_SIZES struct
//
// Return:
//    void
//
//-----------------------------------------------------------------------------
void GMM_STDCALL GmmGetCacheSizes(GMM_CACHE_SIZES *pCacheSizes)
{
    const GT_SYSTEM_INFO *pGtSysInfo;
    __GMM_ASSERT(pCacheSizes != NULL);
    __GMM_ASSERT(pGmmGlobalContext != NULL);

    GMM_DPF_ENTER;
    pGtSysInfo                 = pGmmGlobalContext->GetGtSysInfoPtr();
    pCacheSizes->TotalEDRAM    = GMM_KBYTE(pGtSysInfo->EdramSizeInKb);
    pCacheSizes->TotalLLCCache = GMM_KBYTE(pGtSysInfo->LLCCacheSizeInKb);
    pCacheSizes->TotalL3Cache  = GMM_KBYTE(pGtSysInfo->L3CacheSizeInKb);

    GMM_DPF_EXIT;
}
namespace GmmLib
{
    namespace Utility
    {
        //=============================================================================
        // Function:
        //    GmmGetNumPlanes
        //
        // Description:
        //     Returns number of planes for given format
        //
        // Arguments: <Look at Function Header)
        //
        // Return:
        //    uint32_t number of planes
        //-----------------------------------------------------------------------------
        uint32_t GMM_STDCALL GmmGetNumPlanes(GMM_RESOURCE_FORMAT Format)
        {
            uint32_t Planes = 1;

            switch(Format)
            {
                // YUV Planar Formats
                case GMM_FORMAT_BGRP:
                case GMM_FORMAT_IMC1:
                case GMM_FORMAT_IMC2:
                case GMM_FORMAT_IMC3:
                case GMM_FORMAT_IMC4:
                case GMM_FORMAT_I420:
                case GMM_FORMAT_IYUV:
                case GMM_FORMAT_MFX_JPEG_YUV411:
                case GMM_FORMAT_MFX_JPEG_YUV411R:
                case GMM_FORMAT_MFX_JPEG_YUV420:
                case GMM_FORMAT_MFX_JPEG_YUV422H:
                case GMM_FORMAT_MFX_JPEG_YUV422V:
                case GMM_FORMAT_MFX_JPEG_YUV444:
                case GMM_FORMAT_RGBP:
                case GMM_FORMAT_YV12:
                case GMM_FORMAT_YVU9:
                    Planes = 3;
                    break;
                // YUV Hybrid Formats - GMM treats as Planar
                case GMM_FORMAT_NV11:
                case GMM_FORMAT_NV12:
                case GMM_FORMAT_NV21:
                case GMM_FORMAT_P010:
                case GMM_FORMAT_P012:
                case GMM_FORMAT_P016:
                case GMM_FORMAT_P208:
                case GMM_FORMAT_P216:
                    Planes = 2;
                    break;
                default:
                    Planes = 1;
                    break;
            }
            return Planes;
        }

        //==============================================================================
        //
        // Function:
        //      GmmGetFormatForASTC
        //
        // Description: See below.
        //
        // Returns:
        //      GMM_RESOURCE_FORMAT for described ASTC format or GMM_FORMAT_INVALID(0)
        //      if invalid params specified.
        //
        //-----------------------------------------------------------------------------
        GMM_RESOURCE_FORMAT GMM_STDCALL GmmGetFormatForASTC(uint8_t HDR, uint8_t Float, uint32_t BlockWidth, uint32_t BlockHeight, uint32_t BlockDepth)
        {
            // Full enums/etc. in case we ever need to typedef them...
            enum GMM_SURFACESTATE_FORMAT_ASTC_DYNAMIC_RANGE
            {
                GMM_SURFACESTATE_FORMAT_ASTC_DYNAMIC_RANGE_LDR = 0,
                GMM_SURFACESTATE_FORMAT_ASTC_DYNAMIC_RANGE_HDR = 1,
            };

            enum GMM_SURFACESTATE_FORMAT_ASTC_BLOCK_DIMENSION
            {
                GMM_SURFACESTATE_FORMAT_ASTC_BLOCK_DIMENSION_2D = 0,
                GMM_SURFACESTATE_FORMAT_ASTC_BLOCK_DIMENSION_3D = 1,
            };

            enum GMM_SURFACESTATE_FORMAT_ASTC_DECODE_FORMAT
            {
                GMM_SURFACESTATE_FORMAT_ASTC_DECODE_FORMAT_UNORM8_sRGB = 0,
                GMM_SURFACESTATE_FORMAT_ASTC_DECODE_FORMAT_FLOAT16     = 1,
            };

            enum GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE
            {
                GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_4px   = 0,
                GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_5px   = 1,
                GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_6px   = 2,
                __GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_0x3 = 3,
                GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_8px   = 4,
                __GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_0x5 = 5,
                GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_10px  = 6,
                GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_12px  = 7,
            };

            enum GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE
            {
                GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_3px = 0,
                GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_4px = 1,
                GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_5px = 2,
                GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_6px = 3,
            };

            union GMM_SURFACESTATE_FORMAT_ASTC {
                struct // Common Fields...
                {
                    uint32_t Reserved1 : 6;      // [2D/3D-Specific Fields]
                    uint32_t DecodeFormat : 1;   // GMM_SURFACESTATE_FORMAT_ASTC_DECODE_FORMAT
                    uint32_t BlockDimension : 1; // GMM_SURFACESTATE_FORMAT_ASTC_BLOCK_DIMENSION
                    uint32_t DynamicRange : 1;   // GMM_SURFACESTATE_FORMAT_ASTC_DYNAMIC_RANGE
                    uint32_t Reserved2 : 23;
                };
                struct // 2D-Specific Fields...
                {
                    uint32_t BlockHeight2D : 3; // GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE
                    uint32_t BlockWidth2D : 3;  // GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE
                    uint32_t Reserved3 : 3;     // [Common Fields]
                    uint32_t Reserved4 : 23;
                };
                struct // 3D-Specific Fields...
                {
                    uint32_t BlockDepth3D : 2;  // GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE
                    uint32_t BlockHeight3D : 2; // GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE
                    uint32_t BlockWidth3D : 2;  // GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE
                    uint32_t Reserved5 : 3;     // [Common Fields]
                    uint32_t Reserved6 : 23;
                };
                struct
                {
                    uint32_t Value : 9;
                    uint32_t Reserved7 : 23;
                };
            } Format = {0};
            C_ASSERT(sizeof(union GMM_SURFACESTATE_FORMAT_ASTC) == sizeof(uint32_t));

            // TODO(Minor): Verify and fail for invalid HDR/sRGB combinations.

            if(BlockDepth == 0)
                BlockDepth = 1;

            Format.DynamicRange = HDR ?
                                  GMM_SURFACESTATE_FORMAT_ASTC_DYNAMIC_RANGE_HDR :
                                  GMM_SURFACESTATE_FORMAT_ASTC_DYNAMIC_RANGE_LDR;

            Format.DecodeFormat = Float ?
                                  GMM_SURFACESTATE_FORMAT_ASTC_DECODE_FORMAT_FLOAT16 :
                                  GMM_SURFACESTATE_FORMAT_ASTC_DECODE_FORMAT_UNORM8_sRGB;

            // Validate Block Dimensions...
            if(!(
#define GMM_FORMAT_INCLUDE_ASTC_FORMATS_ONLY
#define GMM_FORMAT(Name, bpe, Width, Height, Depth, IsRT, IsASTC, RcsSurfaceFormat, SSCompressionFmt, Availability) \
    ((BlockWidth == (Width)) && (BlockHeight == (Height)) && (BlockDepth == (Depth))) ||
#include "External/Common/GmmFormatTable.h"
               0)) // <-- 0 benignly terminates the chain of OR expressions.
            {
                goto Invalid;
            }

            if(BlockDepth <= 1)
            {
                Format.BlockDimension = GMM_SURFACESTATE_FORMAT_ASTC_BLOCK_DIMENSION_2D;

                // clang-format off
                Format.BlockWidth2D =
                    (BlockWidth ==  4) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_4px :
                    (BlockWidth ==  5) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_5px :
                    (BlockWidth ==  6) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_6px :
                    (BlockWidth ==  8) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_8px :
                    (BlockWidth == 10) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_10px :
                    GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_12px;
                Format.BlockHeight2D =
                    (BlockHeight ==  4) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_4px :
                    (BlockHeight ==  5) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_5px :
                    (BlockHeight ==  6) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_6px :
                    (BlockHeight ==  8) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_8px :
                    (BlockHeight == 10) ? GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_10px :
                    GMM_SURFACESTATE_FORMAT_ASTC_2D_BLOCK_SIZE_12px;
                // clang-format on
            }
            else
            {
                Format.BlockDimension = GMM_SURFACESTATE_FORMAT_ASTC_BLOCK_DIMENSION_3D;
                // clang-format off
                Format.BlockWidth3D =
                    (BlockWidth == 3) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_3px :
                    (BlockWidth == 4) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_4px :
                    (BlockWidth == 5) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_5px :
                    GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_6px;
                Format.BlockHeight3D =
                    (BlockHeight == 3) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_3px :
                    (BlockHeight == 4) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_4px :
                    (BlockHeight == 5) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_5px :
                    GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_6px;
                Format.BlockDepth3D =
                    (BlockDepth == 3) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_3px :
                    (BlockDepth == 4) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_4px :
                    (BlockDepth == 5) ? GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_5px :
                    GMM_SURFACESTATE_FORMAT_ASTC_3D_BLOCK_SIZE_6px;
                // clang-format on
            }

            return ((GMM_RESOURCE_FORMAT)Format.Value);
        Invalid:
            return (GMM_FORMAT_INVALID);
        }


    } // namespace Utility
} // namespace GmmLib

//=============================================================================
//
// Function: __GmmLog2
//
// Desc: Returns Log2 of passed value. Useful for indexing into arrays.
//
// Parameters:
//      Value => Must be power of 2
//
// Returns:
//      See desc.
//      Example Value =  8 => Log2(Value) = 3
//      Example Value = 32 => Log2(Value) = 5
//-----------------------------------------------------------------------------
uint32_t __GmmLog2(uint32_t Value)
{
    uint32_t FirstSetBit = 0; // bit # of first set bit in Bpp.

#if _MSC_VER
    // Check that Value is pow2
    __GMM_ASSERT(__popcnt(Value) <= 1);
    _BitScanReverse((DWORD *)&FirstSetBit, (DWORD)Value);
#else
    // Check that Value is pow2
    __GMM_ASSERT(__builtin_popcount(Value) <= 1);
    FirstSetBit = __builtin_ctz(Value);
#endif

    // Log2(Value) = FirstSetBit.
    return FirstSetBit;
};

// Table for converting the tile layout of DirectX tiled resources
// to TileY.
const uint32_t __GmmTileYConversionTable[5][2] =
{
//WxH 256x256, of 64KB tile in texels, BPP = 8
{2, 8}, // = (W * (BPP >> 3)) / 128, H / 32
//WxH 256x128, BPP = 16
{4, 4},
//WxH 128x128, BPP = 32
{4, 4},
//WxH 128x64,  BPP = 64
{8, 2},
//WxH 64x64,   BPP = 128
{8, 2}};

// With MSAA, modify DirectX tile dimensions
// MSAA  Divide Tile Dimensions (WxH) by
// 1          1x1
// 2          2x1
// 4          2x2
// 8          4x2
//16          4x4
const uint32_t __GmmMSAAConversion[5][2] =
{
// MSAA 1x
{1, 1},
// MSAA 2x
{2, 1},
// MSAA 4x
{2, 2},
// MSAA 8x
{4, 2},
// MSAA 16x
{4, 4}};
