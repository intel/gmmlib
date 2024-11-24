/*==============================================================================
Copyright(c) 2022 Intel Corporation

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
#include "Internal/Common/Texture/GmmGen10TextureCalc.h"
#include "Internal/Common/Texture/GmmGen11TextureCalc.h"
#include "Internal/Common/Texture/GmmGen12TextureCalc.h"
#include "Internal/Common/Texture/GmmXe_LPGTextureCalc.h"


/////////////////////////////////////////////////////////////////////////////////////
/// This function will Setup a planar surface allocation.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions.
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmXe_LPGTextureCalc::FillTexPlanar(GMM_TEXTURE_INFO * pTexInfo,
                                                                   __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   WidthBytesPhysical, Height, YHeight, VHeight;
    uint32_t   AdjustedVHeight = 0;
    GMM_STATUS Status;
    bool       UVPacked = false;
    uint32_t   BitsPerPixel, AlignedWidth;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);
    __GMM_ASSERT(!pTexInfo->Flags.Info.TiledW);
    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    BitsPerPixel = pTexInfo->BitsPerPixel;
    AlignedWidth = GFX_ULONG_CAST(pTexInfo->BaseWidth);

    if(!pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        pTexInfo->TileMode = TILE_NONE;
    }
    else
    {
        pTexInfo->TileMode = LEGACY_TILE_Y;
    }

    WidthBytesPhysical = AlignedWidth * BitsPerPixel >> 3;
    Height = VHeight = 0;

    YHeight = pTexInfo->BaseHeight;

    switch(pTexInfo->Format)
    {
        case GMM_FORMAT_IMC1: // IMC1 = IMC3 with Swapped U/V
        case GMM_FORMAT_IMC3:
        case GMM_FORMAT_MFX_JPEG_YUV420: // Same as IMC3.
        // YYYYYYYY
        // YYYYYYYY
        // YYYYYYYY
        // YYYYYYYY
        // UUUU
        // UUUU
        // VVVV
        // VVVV
        case GMM_FORMAT_MFX_JPEG_YUV422V: // Similar to IMC3 but U/V are full width.
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // UUUUUUUU
            // UUUUUUUU
            // VVVVVVVV
            // VVVVVVVV
            {
                VHeight = GFX_ALIGN(GFX_CEIL_DIV(YHeight, 2), GMM_IMCx_PLANE_ROW_ALIGNMENT);

                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                Height                                      = YHeight + 2 * VHeight; // One VHeight for V and one for U.
                pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 3;
                break;
            }
        case GMM_FORMAT_MFX_JPEG_YUV411R_TYPE: //Similar to IMC3 but U/V are quarther height and full width.
            //YYYYYYYY
            //YYYYYYYY
            //YYYYYYYY
            //YYYYYYYY
            //UUUUUUUU
            //VVVVVVVV
            {
                VHeight = GFX_ALIGN(GFX_CEIL_DIV(YHeight, 4), GMM_IMCx_PLANE_ROW_ALIGNMENT);

                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                Height                                      = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 3;
                break;
            }
        case GMM_FORMAT_MFX_JPEG_YUV411: // Similar to IMC3 but U/V are quarter width and full height.
        // YYYYYYYY
        // YYYYYYYY
        // YYYYYYYY
        // YYYYYYYY
        // UU
        // UU
        // UU
        // UU
        // VV
        // VV
        // VV
        // VV
        case GMM_FORMAT_MFX_JPEG_YUV422H: // Similar to IMC3 but U/V are full height.
        // YYYYYYYY
        // YYYYYYYY
        // YYYYYYYY
        // YYYYYYYY
        // UUUU
        // UUUU
        // UUUU
        // UUUU
        // VVVV
        // VVVV
        // VVVV
        // VVVV
        case GMM_FORMAT_MFX_JPEG_YUV444: // Similar to IMC3 but U/V are full size.
#if _WIN32
        case GMM_FORMAT_WGBOX_YUV444:
        case GMM_FORMAT_WGBOX_PLANAR_YUV444:
#endif
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // UUUUUUUU
            // UUUUUUUU
            // UUUUUUUU
            // UUUUUUUU
            // VVVVVVVV
            // VVVVVVVV
            // VVVVVVVV
            // VVVVVVVV
            {
                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                VHeight = YHeight;

                Height                                      = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 3;
                break;
            }
        case GMM_FORMAT_BGRP:
        case GMM_FORMAT_RGBP:
        {
            //For RGBP linear Tile keep resource Offset non aligned and for other Tile format to be 16-bit aligned
            if(pTexInfo->Flags.Info.Linear)
            {
                VHeight = YHeight;

                Height                                      = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 3;
            }
            else
            {
                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                VHeight = YHeight;

                Height                                      = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 3;
            }
            break;
        }
        case GMM_FORMAT_IMC2: // IMC2 = IMC4 with Swapped U/V
        case GMM_FORMAT_IMC4:
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // UUUUVVVV
            // UUUUVVVV

            YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
            VHeight = GFX_CEIL_DIV(YHeight, 2);

            WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical, 2); // If odd YWidth, pitch bumps-up to fit rounded-up U/V planes.

            Height = YHeight + VHeight;

            // With SURFACE_STATE.XOffset support, the U-V interface has
            // much lighter restrictions--which will be naturally met by
            // surface pitch restrictions (i.e. dividing an IMC2/4 pitch
            // by 2--to get the U/V interface--will always produce a safe
            // XOffset value).
            // Not technically UV packed but sizing works out the same
            // if the resource is std swizzled
            UVPacked                                    = true;
            pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 2;
            break;
        }
        case GMM_FORMAT_NV12:
        case GMM_FORMAT_NV21:
        case GMM_FORMAT_NV11:
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P012:
        case GMM_FORMAT_P016:
        case GMM_FORMAT_P208:
        case GMM_FORMAT_P216:
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // [UV-Packing]

            if((pTexInfo->Format == GMM_FORMAT_NV12) ||
               (pTexInfo->Format == GMM_FORMAT_NV21) ||
               (pTexInfo->Format == GMM_FORMAT_P010) ||
               (pTexInfo->Format == GMM_FORMAT_P012) ||
               (pTexInfo->Format == GMM_FORMAT_P016))
            {
                VHeight = GFX_CEIL_DIV(YHeight, 2); // U/V plane half of Y
                Height  = YHeight + VHeight;
            }
            else
            {
                VHeight = YHeight; // U/V plane is same as Y
                Height  = YHeight + VHeight;
            }

            if((pTexInfo->Format == GMM_FORMAT_NV12) ||
               (pTexInfo->Format == GMM_FORMAT_NV21) ||
               (pTexInfo->Format == GMM_FORMAT_P010) ||
               (pTexInfo->Format == GMM_FORMAT_P012) ||
               (pTexInfo->Format == GMM_FORMAT_P016) ||
               (pTexInfo->Format == GMM_FORMAT_P208) ||
               (pTexInfo->Format == GMM_FORMAT_P216))
            {
                WidthBytesPhysical                          = GFX_ALIGN(WidthBytesPhysical, 2); // If odd YWidth, pitch bumps-up to fit rounded-up U/V planes.
                pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 2;
            }
            else //if(pTexInfo->Format == GMM_FORMAT_NV11)
            {
                // Tiling not supported, since YPitch != UVPitch...
                pTexInfo->Flags.Info.TiledYf = 0;
                pTexInfo->Flags.Info.TiledX  = 0;
                pTexInfo->Flags.Info.Linear  = 1;
                GMM_SET_64KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);
                GMM_SET_4KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);
            }

            UVPacked = true;
            break;
        }
        case GMM_FORMAT_I420: // IYUV & I420: are identical to YV12 except,
        case GMM_FORMAT_IYUV: // U & V pl.s are reversed.
        case GMM_FORMAT_YV12:
        case GMM_FORMAT_YVU9:
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // VVVVVV..  <-- V and U planes follow the Y plane, as linear
            // ..UUUUUU      arrays--without respect to pitch.

            uint32_t YSize, UVSize, YVSizeRShift;
            uint32_t YSizeForUVPurposes, YSizeForUVPurposesDimensionalAlignment;

            YSize = WidthBytesPhysical * YHeight;

            // YVU9 has one U/V pixel for each 4x4 Y block.
            // The others have one U/V pixel for each 2x2 Y block.

            // YVU9 has a Y:V size ratio of 16 (4x4 --> 1).
            // The others have a ratio of 4 (2x2 --> 1).
            YVSizeRShift = (pTexInfo->Format != GMM_FORMAT_YVU9) ? 2 : 4;

            // If a Y plane isn't fully-aligned to its Y-->U/V block size, the
            // extra/unaligned Y pixels still need corresponding U/V pixels--So
            // for the purpose of computing the UVSize, we must consider a
            // dimensionally "rounded-up" YSize. (E.g. a 13x5 YVU9 Y plane would
            // require 4x2 U/V planes--the same UVSize as a fully-aligned 16x8 Y.)
            YSizeForUVPurposesDimensionalAlignment = (pTexInfo->Format != GMM_FORMAT_YVU9) ? 2 : 4;
            YSizeForUVPurposes =
            GFX_ALIGN(WidthBytesPhysical, YSizeForUVPurposesDimensionalAlignment) *
            GFX_ALIGN(YHeight, YSizeForUVPurposesDimensionalAlignment);

            UVSize = 2 * // <-- U + V
                     (YSizeForUVPurposes >> YVSizeRShift);

            Height = GFX_CEIL_DIV(YSize + UVSize, WidthBytesPhysical);

            // Tiling not supported, since YPitch != UVPitch...
            pTexInfo->Flags.Info.TiledYf                = 0;
            pTexInfo->Flags.Info.TiledX                 = 0;
            pTexInfo->Flags.Info.Linear                 = 1;
            pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = 1;
            GMM_SET_64KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);
            GMM_SET_4KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);

            break;
        }
        default:
        {
            GMM_ASSERTDPF(0, "Unexpected format");
            return GMM_ERROR;
        }
    }

    // Align Height to even row to avoid hang if HW over-fetch
    Height = GFX_ALIGN(Height, __GMM_EVEN_ROW);

    SetTileMode(pTexInfo);

    // If the Surface has Odd height dimension, we will fall back to Linear Format.
    // If MMC is enabled, disable MMC during such cases.
    if(pTexInfo->Flags.Gpu.MMC)
    {
        if(!(GMM_IS_4KB_TILE(pTexInfo->Flags) || GMM_IS_64KB_TILE(pTexInfo->Flags)))
        {
            pTexInfo->Flags.Gpu.MMC = 0;
        }
    }

    // If the Surface has Odd height dimension, we will fall back to Linear Format.
    // If MMC is enabled, disable .CCS/UnifiedAuxSurface during such cases.
    if(pTexInfo->Flags.Gpu.CCS)
    {
        if(!(GMM_IS_4KB_TILE(pTexInfo->Flags) || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
           !(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs && GMM_IS_4KB_TILE(pTexInfo->Flags)))
        {
            pTexInfo->Flags.Gpu.MMC               = 0;
            pTexInfo->Flags.Gpu.CCS               = 0;
            pTexInfo->Flags.Gpu.UnifiedAuxSurface = 0;
            pTexInfo->Flags.Gpu.__NonMsaaTileYCcs = 0;
        }
    }

    // Legacy Planar "Linear Video" Restrictions...
    if(pTexInfo->Flags.Info.Linear && !pTexInfo->Flags.Wa.NoLegacyPlanarLinearVideoRestrictions)
    {
        pRestrictions->LockPitchAlignment   = GFX_MAX(pRestrictions->LockPitchAlignment, GMM_BYTES(64));
        pRestrictions->MinPitch             = GFX_MAX(pRestrictions->MinPitch, GMM_BYTES(64));
        pRestrictions->PitchAlignment       = GFX_MAX(pRestrictions->PitchAlignment, GMM_BYTES(64));
        pRestrictions->RenderPitchAlignment = GFX_MAX(pRestrictions->RenderPitchAlignment, GMM_BYTES(64));
    }

    // Multiply overall pitch alignment for surfaces whose U/V planes have a
    // pitch down-scaled from that of Y--Since the U/V pitches must meet the
    // original restriction, the Y pitch must meet a scaled-up multiple.
    if((pTexInfo->Format == GMM_FORMAT_I420) ||
       (pTexInfo->Format == GMM_FORMAT_IYUV) ||
       (pTexInfo->Format == GMM_FORMAT_NV11) ||
       (pTexInfo->Format == GMM_FORMAT_YV12) ||
       (pTexInfo->Format == GMM_FORMAT_YVU9))
    {
        uint32_t LShift =
        (pTexInfo->Format != GMM_FORMAT_YVU9) ?
        1 : // UVPitch = 1/2 YPitch
        2;  // UVPitch = 1/4 YPitch

        pRestrictions->LockPitchAlignment <<= LShift;
        pRestrictions->MinPitch <<= LShift;
        pRestrictions->PitchAlignment <<= LShift;
        pRestrictions->RenderPitchAlignment <<= LShift;
    }

    AdjustedVHeight = VHeight;

    FindMipTailStartLod(pTexInfo);

    // In case of Planar surfaces, only the last Plane has to be aligned to 64 for LCU access
    if(pGmmLibContext->GetWaTable().WaAlignYUVResourceToLCU && GmmIsYUVFormatLCUAligned(pTexInfo->Format) && VHeight > 0)
    {
        AdjustedVHeight = GFX_ALIGN(VHeight, GMM_SCANLINES(GMM_MAX_LCU_SIZE));
        Height += AdjustedVHeight - VHeight;
    }

    // For std swizzled and UV packed tile Ys/Yf cases, the planes
    // must be tile-boundary aligned. Actual alignment is handled
    // in FillPlanarOffsetAddress, but height and width must
    // be adjusted for correct size calculation
    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]) &&
       !pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        uint32_t TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        uint32_t TileWidth  = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileWidth;

        pTexInfo->OffsetInfo.PlaneXe_LPG.IsTileAlignedPlanes = true;

        if(pTexInfo->Flags.Gpu.CCS && !pGmmLibContext->GetSkuTable().FtrFlatPhysCCS) // alignment adjustment needed only for aux tables
        {
            if(GMM_IS_64KB_TILE(pTexInfo->Flags))
            {
                TileHeight *= (!WA64K(pGmmLibContext) && !WA16K(pGmmLibContext)) ? 16 : 1; // For 64Kb Tile mode: Multiply TileHeight by 16 for 1 MB alignment
            }
            else
            {
                TileHeight *= (WA16K(pGmmLibContext) ? 1 : WA64K(pGmmLibContext) ? 4 : 64); // For 4k Tile:  Multiply TileHeight by 4 and Pitch by 4 for 64kb alignment, multiply TileHeight by 64 and Pitch by 4 for 1 MB alignment
            }
        }

        if(pTexInfo->Format == GMM_FORMAT_IMC2 || // IMC2, IMC4 needs even tile columns
           pTexInfo->Format == GMM_FORMAT_IMC4)
        {
            // If the U & V planes are side-by-side then the surface pitch must be
            // padded out so that U and V planes will being on a tile boundary.
            // This means that an odd Y plane width must be padded out
            // with an additional tile. Even widths do not need padding
            uint32_t TileCols = GFX_CEIL_DIV(WidthBytesPhysical, TileWidth);
            if(TileCols % 2)
            {
                WidthBytesPhysical = (TileCols + 1) * TileWidth;
            }
        }

        Height = GFX_ALIGN(YHeight, TileHeight) + (UVPacked ? GFX_ALIGN(AdjustedVHeight, TileHeight) :
                                                              (GFX_ALIGN(VHeight, TileHeight) + GFX_ALIGN(AdjustedVHeight, TileHeight)));

        if(GMM_IS_64KB_TILE(pTexInfo->Flags) || pTexInfo->Flags.Info.TiledYf)
        {
            pTexInfo->Flags.Info.RedecribedPlanes = true;
        }
    }
    else if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        uint32_t TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;

        BitsPerPixel = 8;

        if(pTexInfo->Format == GMM_FORMAT_IMC2 || // IMC2, IMC4 needs even tile columns
           pTexInfo->Format == GMM_FORMAT_IMC4)
        {
// If the U & V planes are side-by-side then the surface pitch must be
// padded out so that U and V planes will being on a tile boundary.
// This means that an odd Y plane width must be padded out
// with an additional tile. Even widths do not need padding

// CCS must use padded main surface width, so get main surface TileWidth
#define CCSMODE_TO_TILEMODE(y) ((y + TILE_YF_2D_8bpe) < TILE_YS_1D_8bpe) ? (y + TILE_YF_2D_8bpe) : \
                                                                           ((y + TILE_YF_2D_8bpe + 5) >= TILE_YS_1D_128bpe) ? (y + TILE_YF_2D_8bpe + 5) : TILE_NONE

            uint32_t BaseTileWidth = pPlatform->TileInfo[CCSMODE_TO_TILEMODE(pTexInfo->CCSModeAlign)].LogicalTileWidth;
            WidthBytesPhysical     = GFX_ALIGN(WidthBytesPhysical, 2 * BaseTileWidth);
        }

        AlignedWidth = GFX_ULONG_CAST(WidthBytesPhysical / (pTexInfo->BitsPerPixel >> 3));

        WidthBytesPhysical = __GMM_EXPAND_WIDTH(this, AlignedWidth, pTexInfo->Alignment.HAlign, pTexInfo);
        WidthBytesPhysical = ScaleTextureWidth(pTexInfo, WidthBytesPhysical); //Should both YAux and UVAux use same CCModeALign (ie using common bpe?)
                                                                              //If different, then copy Aux info from per-plane Aux? HW has separate bpe or common?
        YHeight = __GMM_EXPAND_HEIGHT(this, YHeight, pTexInfo->Alignment.VAlign, pTexInfo);
        YHeight = ScaleTextureHeight(pTexInfo, YHeight);
        YHeight = GFX_ALIGN(YHeight, TileHeight);

        VHeight = __GMM_EXPAND_HEIGHT(this, VHeight, pTexInfo->Alignment.VAlign, pTexInfo);
        VHeight = ScaleTextureHeight(pTexInfo, VHeight);
        VHeight = GFX_ALIGN(VHeight, TileHeight);

        Height = YHeight + VHeight;
    }

    if(pTexInfo->Flags.Info.RedecribedPlanes)
    {
        if(false == RedescribeTexturePlanes(pTexInfo, &WidthBytesPhysical))
        {
            __GMM_ASSERT(false);
        }
    }
    if((Status = // <-- Note assignment.
        FillTexPitchAndSize(
        pTexInfo, WidthBytesPhysical, Height, pRestrictions)) == GMM_SUCCESS)
    {
        FillPlanarOffsetAddress(pTexInfo);
    }

    // Planar & hybrid 2D arrays supported in DX11.1+ spec but not HW. Memory layout
    // is defined by SW requirements; Y plane must be 4KB aligned.
    if(pTexInfo->ArraySize > 1)
    {
        GMM_GFX_SIZE_T ElementSizeBytes = pTexInfo->Size;
        int64_t        LargeSize;

        // Size should always be page aligned.
        __GMM_ASSERT((pTexInfo->Size % PAGE_SIZE) == 0);

        if((LargeSize = (int64_t)ElementSizeBytes * pTexInfo->ArraySize) <= pPlatform->SurfaceMaxSize)
        {
            pTexInfo->OffsetInfo.PlaneXe_LPG.ArrayQPitch = ElementSizeBytes;
            pTexInfo->Size                               = LargeSize;
        }
        else
        {
            GMM_ASSERTDPF(0, "Surface too large!");
            Status = GMM_ERROR;
        }
    }

    GMM_DPF_EXIT;
    return (Status);
} // FillTexPlanar

/////////////////////////////////////////////////////////////////////////////////////
/// This function calculates the (X,Y) address of each given plane. X is in bytes
/// and Y is in scanlines.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmXe_LPGTextureCalc::FillPlanarOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_GFX_SIZE_T *pUOffsetX, *pUOffsetY;
    GMM_GFX_SIZE_T *pVOffsetX, *pVOffsetY;
    uint32_t        YHeight = 0, VHeight = 0;
    bool            UVPacked = false;
    uint32_t        Height;
    uint32_t        WidthBytesPhysical = GFX_ULONG_CAST(pTexInfo->BaseWidth) * pTexInfo->BitsPerPixel >> 3;

#define SWAP_UV()              \
    {                          \
        GMM_GFX_SIZE_T *pTemp; \
                               \
        pTemp     = pUOffsetX; \
        pUOffsetX = pVOffsetX; \
        pVOffsetX = pTemp;     \
                               \
        pTemp     = pUOffsetY; \
        pUOffsetY = pVOffsetY; \
        pVOffsetY = pTemp;     \
    }

    __GMM_ASSERTPTR(pTexInfo, VOIDRETURN);
    __GMM_ASSERTPTR(((pTexInfo->TileMode < GMM_TILE_MODES) && (pTexInfo->TileMode >= TILE_NONE)), VOIDRETURN);
    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    // GMM_PLANE_Y always at (0, 0)...
    pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_Y] = 0;
    pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_Y] = 0;

    pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y] = 0;
    pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y] = 0;

    Height = pTexInfo->BaseHeight;
    if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        Height = __GMM_EXPAND_HEIGHT(this, Height, pTexInfo->Alignment.VAlign, pTexInfo);
        Height = ScaleTextureHeight(pTexInfo, Height);
        if(pTexInfo->Flags.Gpu.UnifiedAuxSurface)
        {
            pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_Y] = 0;
        }
    }

    // GMM_PLANE_U/V Planes...
    pUOffsetX = &pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_U];
    pUOffsetY = &pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U];
    pVOffsetX = &pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_V];
    pVOffsetY = &pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_V];

    switch(pTexInfo->Format)
    {
        case GMM_FORMAT_IMC1:
            SWAP_UV(); // IMC1 = IMC3 with Swapped U/V
        case GMM_FORMAT_IMC3:
        case GMM_FORMAT_MFX_JPEG_YUV420:  // Same as IMC3.
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // UUUU
                                          // UUUU
                                          // VVVV
                                          // VVVV
        case GMM_FORMAT_MFX_JPEG_YUV422V: // Similar to IMC3 but U/V are full width.
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // UUUUUUUU
                                          // UUUUUUUU
                                          // VVVVVVVV
                                          // VVVVVVVV
            {
                *pUOffsetX = 0;
                YHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pUOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                *pVOffsetX = 0;
                VHeight    = GFX_ALIGN(GFX_CEIL_DIV(pTexInfo->BaseHeight, 2), GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pVOffsetY =
                GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT) +
                GFX_ALIGN(GFX_CEIL_DIV(pTexInfo->BaseHeight, 2), GMM_IMCx_PLANE_ROW_ALIGNMENT);

                break;
            }
        case GMM_FORMAT_MFX_JPEG_YUV411R_TYPE: //Similar to IMC3 but U/V are quarther height and full width.
                                               //YYYYYYYY
                                               //YYYYYYYY
                                               //YYYYYYYY
                                               //YYYYYYYY
                                               //UUUUUUUU
                                               //VVVVVVVV
            {
                *pUOffsetX = 0;
                YHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pUOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                *pVOffsetX = 0;
                VHeight    = GFX_ALIGN(GFX_CEIL_DIV(pTexInfo->BaseHeight, 4), GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pVOffsetY =
                GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT) +
                GFX_ALIGN(GFX_CEIL_DIV(pTexInfo->BaseHeight, 4), GMM_IMCx_PLANE_ROW_ALIGNMENT);

                break;
            }
        case GMM_FORMAT_MFX_JPEG_YUV411:  // Similar to IMC3 but U/V are quarter width and full height.
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // UU
                                          // UU
                                          // UU
                                          // UU
                                          // VV
                                          // VV
                                          // VV
                                          // VV
        case GMM_FORMAT_MFX_JPEG_YUV422H: // Similar to IMC3 but U/V are full height.
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // UUUU
                                          // UUUU
                                          // UUUU
                                          // UUUU
                                          // VVVV
                                          // VVVV
                                          // VVVV
                                          // VVVV
        case GMM_FORMAT_MFX_JPEG_YUV444:  // Similar to IMC3 but U/V are full size.
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // YYYYYYYY
                                          // UUUUUUUU
                                          // UUUUUUUU
                                          // UUUUUUUU
                                          // UUUUUUUU
                                          // VVVVVVVV
                                          // VVVVVVVV
                                          // VVVVVVVV
                                          // VVVVVVVV
            {
                *pUOffsetX = 0;
                YHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pUOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                *pVOffsetX = 0;
                VHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pVOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT) * 2;

                break;
            }
        case GMM_FORMAT_BGRP:
        case GMM_FORMAT_RGBP:
        {
            //For RGBP linear Tile keep resource Offset non aligned and for other Tile format to be 16-bit aligned
            if(pTexInfo->Flags.Info.Linear)
            {
                *pUOffsetX = 0;
                YHeight    = pTexInfo->BaseHeight;
                *pUOffsetY = pTexInfo->BaseHeight;

                *pVOffsetX = 0;
                VHeight    = pTexInfo->BaseHeight;
                *pVOffsetY = (GMM_GFX_SIZE_T)pTexInfo->BaseHeight * 2;
            }
            else //Tiled
            {
                *pUOffsetX = 0;
                YHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pUOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                *pVOffsetX = 0;
                VHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                *pVOffsetY = (GMM_GFX_SIZE_T)GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT) * 2;
            }
            break;
        }
        case GMM_FORMAT_IMC2:
            SWAP_UV(); // IMC2 = IMC4 with Swapped U/V
        case GMM_FORMAT_IMC4:
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // UUUUVVVV
            // UUUUVVVV

            __GMM_ASSERT((pTexInfo->Pitch & 1) == 0);

            *pUOffsetX = 0;
            YHeight    = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
            *pUOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

            *pVOffsetX = pTexInfo->Pitch / 2;
            VHeight    = GFX_CEIL_DIV(YHeight, 2);
            *pVOffsetY = GFX_ALIGN(pTexInfo->BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

            // Not technically UV packed but sizing works out the same
            UVPacked = true;

            break;
        }
        case GMM_FORMAT_I420: // I420 = IYUV
        case GMM_FORMAT_IYUV:
            SWAP_UV(); // I420/IYUV = YV12 with Swapped U/V
        case GMM_FORMAT_YV12:
        case GMM_FORMAT_YVU9:
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // VVVVVV..  <-- V and U planes follow the Y plane, as linear
            // ..UUUUUU      arrays--without respect to pitch.

            uint32_t YSize, YVSizeRShift, VSize, UOffset;
            uint32_t YSizeForUVPurposes, YSizeForUVPurposesDimensionalAlignment;

            YSize = GFX_ULONG_CAST(pTexInfo->Pitch) * pTexInfo->BaseHeight;

            // YVU9 has one U/V pixel for each 4x4 Y block.
            // The others have one U/V pixel for each 2x2 Y block.

            // YVU9 has a Y:V size ratio of 16 (4x4 --> 1).
            // The others have a ratio of 4 (2x2 --> 1).
            YVSizeRShift = (pTexInfo->Format != GMM_FORMAT_YVU9) ? 2 : 4;

            // If a Y plane isn't fully-aligned to its Y-->U/V block size, the
            // extra/unaligned Y pixels still need corresponding U/V pixels--So
            // for the purpose of computing the UVSize, we must consider a
            // dimensionally "rounded-up" YSize. (E.g. a 13x5 YVU9 Y plane would
            // require 4x2 U/V planes--the same UVSize as a fully-aligned 16x8 Y.)
            YSizeForUVPurposesDimensionalAlignment = (pTexInfo->Format != GMM_FORMAT_YVU9) ? 2 : 4;
            YSizeForUVPurposes =
            GFX_ALIGN(GFX_ULONG_CAST(pTexInfo->Pitch), YSizeForUVPurposesDimensionalAlignment) *
            GFX_ALIGN(pTexInfo->BaseHeight, YSizeForUVPurposesDimensionalAlignment);

            VSize   = (YSizeForUVPurposes >> YVSizeRShift);
            UOffset = YSize + VSize;

            *pVOffsetX = 0;
            *pVOffsetY = pTexInfo->BaseHeight;

            *pUOffsetX = UOffset % pTexInfo->Pitch;
            *pUOffsetY = UOffset / pTexInfo->Pitch;

            YHeight = GFX_CEIL_DIV(YSize + 2 * VSize, WidthBytesPhysical);

            break;
        }
        case GMM_FORMAT_NV12:
        case GMM_FORMAT_NV21:
        case GMM_FORMAT_NV11:
        case GMM_FORMAT_P010:
        case GMM_FORMAT_P012:
        case GMM_FORMAT_P016:
        case GMM_FORMAT_P208:
        case GMM_FORMAT_P216:
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // [UV-Packing]
            *pUOffsetX = *pVOffsetX = 0;
            YHeight                 = GFX_ALIGN(Height, __GMM_EVEN_ROW);
            *pUOffsetY = *pVOffsetY = YHeight;

            if((pTexInfo->Format == GMM_FORMAT_NV12) ||
               (pTexInfo->Format == GMM_FORMAT_NV21) ||
               (pTexInfo->Format == GMM_FORMAT_P010) ||
               (pTexInfo->Format == GMM_FORMAT_P012) ||
               (pTexInfo->Format == GMM_FORMAT_P016))
            {
                VHeight = GFX_CEIL_DIV(Height, 2);
            }
            else
            {
                VHeight = YHeight; // U/V plane is same as Y
            }

            UVPacked = true;
            break;
        }
        default:
        {
            GMM_ASSERTDPF(0, "Unknown Video Format U\n");
            break;
        }
    }

    pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y] = YHeight;
    if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 2)
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U] = VHeight;
    }
    else if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 3)
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U] =
        pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_V] = VHeight;
    }

    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]) || pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        uint32_t TileHeight         = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        uint32_t TileWidth          = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth;
        uint32_t PhysicalTileHeight = TileHeight;

        if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) > IGFX_GEN11LP_CORE)
        {
            if(pTexInfo->Flags.Gpu.CCS && !pGmmLibContext->GetSkuTable().FtrFlatPhysCCS)
            {
                //U/V must be aligned to AuxT granularity, 4x pitchalign enforces 16K-align for 4KB tile,
                //add extra padding for 64K AuxT, 1MB AuxT
                if(GMM_IS_64KB_TILE(pTexInfo->Flags))
                {
                    TileHeight *= (!WA64K(pGmmLibContext) && !WA16K(pGmmLibContext)) ? 16 : 1; // For 64Kb Tile mode: Multiply TileHeight by 16 for 1 MB alignment
                }
                else
                {
                    PhysicalTileHeight *= (WA16K(pGmmLibContext) ? 1 : WA64K(pGmmLibContext) ? 4 : 1); //  for 1 MB AuxT granularity, we do 1 MB alignment only in VA space and not in physical space, so do not multiply PhysicalTileHeight with 64 here
                    TileHeight *= (WA16K(pGmmLibContext) ? 1 : WA64K(pGmmLibContext) ? 4 : 64);        // For 4k Tile:  Multiply TileHeight by 4 and Pitch by 4 for 64kb alignment, multiply TileHeight by 64 and Pitch by 4 for 1 MB alignment
                }
                if(!WA64K(pGmmLibContext) && !WA16K(pGmmLibContext))
                {
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Is1MBAuxTAlignedPlanes       = true;
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_Y] = GFX_ALIGN(YHeight, PhysicalTileHeight); // confirm tilealignment is needed for physivcal backing, else calculate size from unaligned offset and use the same
                    if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 2)
                    {
                        pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_U] = GFX_ALIGN(VHeight, PhysicalTileHeight);
                    }
                    else if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 3)
                    {
                        pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_U] =
                        pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_V] = GFX_ALIGN(VHeight, PhysicalTileHeight);
                    }
                }
            }
        }

        *pUOffsetX = GFX_ALIGN(*pUOffsetX, TileWidth);
        *pUOffsetY = GFX_ALIGN(*pUOffsetY, TileHeight);
        *pVOffsetX = GFX_ALIGN(*pVOffsetX, TileWidth);
        *pVOffsetY = UVPacked ?
                     GFX_ALIGN(*pVOffsetY, TileHeight) :
                     GFX_ALIGN(YHeight, TileHeight) + GFX_ALIGN(VHeight, TileHeight);

        if(pTexInfo->Flags.Gpu.UnifiedAuxSurface && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
        {
            *pUOffsetY += pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_Y];
            *pVOffsetY = *pUOffsetY;
        }

        // This is needed for FtrDisplayPageTables
        if(pGmmLibContext->GetSkuTable().FtrDisplayPageTables)
        {
            if(pTexInfo->OffsetInfo.PlaneXe_LPG.Is1MBAuxTAlignedPlanes)
            {
                pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_Y] = GFX_ALIGN(YHeight, PhysicalTileHeight); // confirm tilealignment is needed for physivcal backing, else calculate size from unaligned offset and use the same
                if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 2)
                {
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_U] = GFX_ALIGN(VHeight, PhysicalTileHeight);
                }
                else if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 3)
                {
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_U] =
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_V] = GFX_ALIGN(VHeight, PhysicalTileHeight);
                }
            }
            else
            {
                pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_Y] = GFX_ALIGN(YHeight, TileHeight);
                if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 2)
                {
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_U] = GFX_ALIGN(VHeight, TileHeight);
                }
                else if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 3)
                {
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_U] =
                    pTexInfo->OffsetInfo.PlaneXe_LPG.Aligned.Height[GMM_PLANE_V] = GFX_ALIGN(VHeight, TileHeight);
                }
            }
        }
    }

    //Special case LKF MMC compressed surfaces
    if(pTexInfo->Flags.Gpu.MMC &&
       pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
       pTexInfo->Flags.Info.TiledY)
    {
        GMM_GFX_SIZE_T TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        GMM_GFX_SIZE_T TileWidth  = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileWidth;

        *pUOffsetX = GFX_ALIGN(*pUOffsetX, TileWidth);
        *pUOffsetY = GFX_ALIGN(*pUOffsetY, TileHeight);
        *pVOffsetX = GFX_ALIGN(*pVOffsetX, TileWidth);
        *pVOffsetY = GFX_ALIGN(*pVOffsetY, TileHeight);
    }

    GMM_DPF_EXIT;

#undef SWAP_UV
}

////////////////////////////////////////////////////////////////////////////////////
/// Calculates offset address of a sub resource(i.e.  Mip Map, Cube face, volume texture)
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO to store offset info
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmXe_LPGTextureCalc::GetTexLockOffset(GMM_TEXTURE_INFO *   pTexInfo,
                                                          GMM_REQ_OFFSET_INFO *pReqInfo)
{
    GMM_STATUS     Result = GMM_SUCCESS;
    GMM_GFX_SIZE_T AddressOffset;
    uint32_t       Pitch, Slice;
    uint32_t       MipHeight, MipWidth, MipLevel;
    uint32_t       NumberOfMipsInSingleRow, SliceRow;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pReqInfo, GMM_ERROR);

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    // set default value
    AddressOffset = 0;
    Pitch         = GFX_ULONG_CAST(pTexInfo->Pitch);
    MipLevel      = pReqInfo->MipLevel;
    Slice         = pReqInfo->Slice;

    if(GmmIsPlanar(pTexInfo->Format))
    {
        AddressOffset           = GetMipMapByteAddress(pTexInfo, pReqInfo);
        pReqInfo->Lock.Offset64 = AddressOffset;
        pReqInfo->Lock.Pitch    = pTexInfo->OffsetInfo.PlaneXe_LPG.Is1MBAuxTAlignedPlanes ? GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.PhysicalPitch) : Pitch;

        // Adjust returned pitch for non-uniform-pitch U/V queries...
        if((pReqInfo->Plane == GMM_PLANE_U) ||
           (pReqInfo->Plane == GMM_PLANE_V))
        {
            switch(pTexInfo->Format)
            {
                case GMM_FORMAT_I420:
                case GMM_FORMAT_IYUV:
                case GMM_FORMAT_YV12:
                case GMM_FORMAT_NV11:
                    pReqInfo->Lock.Pitch /= 2;
                    break;
                case GMM_FORMAT_YVU9:
                    pReqInfo->Lock.Pitch /= 4;
                    break;
                default:
                    //Cool--Constant pitch across all planes.
                    break;
            }
        }

        return Result;
    }

    switch(pTexInfo->Type)
    {
        case RESOURCE_3D:
        {
            if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE)
            {
                AddressOffset = GFX_ULONG_CAST(GetMipMapByteAddress(pTexInfo, pReqInfo));

                // Bytes from one slice to the next...
                pReqInfo->Lock.Gen9PlusSlicePitch = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock);
            }
            else
            {
                MipHeight = pTexInfo->BaseHeight >> MipLevel;
                MipWidth  = GFX_ULONG_CAST(pTexInfo->BaseWidth) >> MipLevel;

                AlignTexHeightWidth(pTexInfo, &MipHeight, &MipWidth);

                // See how many mip can fit in one row
                NumberOfMipsInSingleRow = GFX_2_TO_POWER_OF(MipLevel);

                SliceRow = Slice / NumberOfMipsInSingleRow;

                // get the base address + Slice pitch
                AddressOffset = pTexInfo->OffsetInfo.Texture3DOffsetInfo.Offset[MipLevel];

                pReqInfo->Lock.Mip0SlicePitch = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Texture3DOffsetInfo.Mip0SlicePitch);

                // Actual address is offset based on requested slice
                AddressOffset += (GMM_GFX_SIZE_T)SliceRow * MipHeight * Pitch;

                // Get to particular slice
                if(Slice % NumberOfMipsInSingleRow)
                {
                    AddressOffset += (((GMM_GFX_SIZE_T)(Slice % NumberOfMipsInSingleRow) *
                                       MipWidth * pTexInfo->BitsPerPixel) >>
                                      3);
                }
            }
            break;
        }
        case RESOURCE_CUBE:
        case RESOURCE_2D:
        case RESOURCE_1D:
        {
            AddressOffset = GetMipMapByteAddress(pTexInfo, pReqInfo);
            break;
        }
        default:
        { // These resources dont' have multiple levels of detail
            AddressOffset = 0;
            break;
        }
    }

    pReqInfo->Lock.Offset64 = AddressOffset;
    pReqInfo->Lock.Pitch    = Pitch;

    return Result;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Function used to calculate byte address of a specified mip map
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO
///
/// @return     ::GMM_GFX_SIZE_T byte offset
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GmmLib::GmmXe_LPGTextureCalc::GetMipMapByteAddress(GMM_TEXTURE_INFO *   pTexInfo,
                                                                  GMM_REQ_OFFSET_INFO *pReqInfo)
{
    GMM_GFX_SIZE_T ArrayQPitch, MipMapByteAddress, Pitch;
    uint32_t       MipLevel;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pReqInfo, GMM_ERROR);
    __GMM_ASSERT(!(pTexInfo->Flags.Gpu.CCS && !pTexInfo->Flags.Gpu.UnifiedAuxSurface));
    __GMM_ASSERT(pReqInfo->Plane < GMM_MAX_PLANE);

    MipLevel    = pReqInfo->MipLevel;
    Pitch       = pTexInfo->Pitch;
    ArrayQPitch = pReqInfo->ReqRender ?
                  pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender :
                  pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(pTexInfo->Type == RESOURCE_3D && !pTexInfo->Flags.Info.Linear)
    {
        ArrayQPitch *= pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileDepth;
    }

    if((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN8_CORE) &&
       ((pTexInfo->MSAA.NumSamples > 1) &&
        !(pTexInfo->Flags.Gpu.Depth ||
          pTexInfo->Flags.Gpu.SeparateStencil ||
          GMM_IS_64KB_TILE(pTexInfo->Flags) ||
          pTexInfo->Flags.Info.TiledYf)))
    {
        ArrayQPitch *= pTexInfo->MSAA.NumSamples;
    }

    if(GmmIsPlanar(pTexInfo->Format))
    {
        uint32_t Plane = pReqInfo->Plane;

        uint32_t OffsetX = 0;
        uint32_t OffsetY = 0;
        if(Plane < GMM_MAX_PLANE)
        {
            if(pReqInfo->ReqLock && pTexInfo->OffsetInfo.PlaneXe_LPG.Is1MBAuxTAlignedPlanes)
            {
                OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_Y]);
                OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_Y]);
                for(uint32_t PlaneId = GMM_PLANE_Y; PlaneId < Plane; PlaneId++)
                {
                    // physical offset calculation: x offset remains same as PLANE_Y, YOffset = sum of physical height of all prev planes
                    OffsetY += GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[PlaneId]);
                }
                Pitch       = pTexInfo->OffsetInfo.PlaneXe_LPG.PhysicalPitch;
                ArrayQPitch = ((pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_Y] + pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_U] + pTexInfo->OffsetInfo.PlaneXe_LPG.Physical.Height[GMM_PLANE_V]) * pTexInfo->OffsetInfo.PlaneXe_LPG.PhysicalPitch); // Physical size of single array
            }
            else
            {
                OffsetX     = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.X[Plane]);
                OffsetY     = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.Y[Plane]);
                ArrayQPitch = pTexInfo->OffsetInfo.PlaneXe_LPG.ArrayQPitch;
            }
        }
        MipMapByteAddress = (OffsetY * Pitch) + OffsetX;

        __GMM_ASSERT(!pReqInfo->ArrayIndex || (pReqInfo->ArrayIndex < pTexInfo->ArraySize));

        MipMapByteAddress += (ArrayQPitch * pReqInfo->ArrayIndex);
    }
    else
    {
        switch(pTexInfo->Type)
        {
            case RESOURCE_CUBE:
            {
                uint32_t CubeFace = pReqInfo->CubeFace;

                GMM_ASSERTDPF( // Validate Cube Map Params...
                (!pReqInfo->ArrayIndex || (pReqInfo->ArrayIndex < pTexInfo->ArraySize)) &&
                (pReqInfo->CubeFace < __GMM_MAX_CUBE_FACE) &&
                (pReqInfo->CubeFace != __GMM_NO_CUBE_MAP) &&
                (pReqInfo->Plane == GMM_NO_PLANE) &&
                (pReqInfo->Slice == 0),
                "Invalid parameter!");

                // Support for CubeMap Arrays using 2D Arrays
                MipMapByteAddress = pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[MipLevel];
                MipMapByteAddress += (ArrayQPitch * ((6 * pReqInfo->ArrayIndex) + CubeFace));
                break;
            }
            case RESOURCE_2D:
            case RESOURCE_1D:
            {
                MipMapByteAddress = pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[MipLevel];

                if(pReqInfo->ArrayIndex)
                {
                    MipMapByteAddress += (ArrayQPitch * pReqInfo->ArrayIndex);
                }
                break;
            }
            case RESOURCE_3D:
            {
                if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE)
                {
                    MipMapByteAddress = pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[MipLevel];

                    if(pReqInfo->Slice)
                    {
                        MipMapByteAddress += (ArrayQPitch * pReqInfo->Slice);
                    }
                }
                else
                {
                    MipMapByteAddress = Get3DMipByteAddress(pTexInfo, pReqInfo);
                }
                break;
            }
            default:
            { // These resources don't have multiple levels of detail
                MipMapByteAddress = 0;
                break;
            }
        }
    }

    MipMapByteAddress += pTexInfo->Flags.Gpu.S3d ?
                         GetDisplayFrameOffset(pTexInfo, pReqInfo) :
                         0;

    return MipMapByteAddress;
}

void GmmLib::GmmXe_LPGTextureCalc::SetPlanarOffsetInfo(GMM_TEXTURE_INFO *pTexInfo, GMM_RESCREATE_CUSTOM_PARAMS &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.IsTileAlignedPlanes = true;
    }
    for(uint8_t i = 1; i <= CreateParams.NoOfPlanes; i++)
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.X[i] = CreateParams.PlaneOffset.X[i];
        pTexInfo->OffsetInfo.PlaneXe_LPG.Y[i] = CreateParams.PlaneOffset.Y[i];
    }
    pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = CreateParams.NoOfPlanes;
}

#ifndef __GMM_KMD__
void GmmLib::GmmXe_LPGTextureCalc::SetPlanarOffsetInfo_2(GMM_TEXTURE_INFO *pTexInfo, GMM_RESCREATE_CUSTOM_PARAMS_2 &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.IsTileAlignedPlanes = true;
    }
    for(uint8_t i = 1; i <= CreateParams.NoOfPlanes; i++)
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.X[i] = CreateParams.PlaneOffset.X[i];
        pTexInfo->OffsetInfo.PlaneXe_LPG.Y[i] = CreateParams.PlaneOffset.Y[i];
    }
    pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes = CreateParams.NoOfPlanes;
}
#endif

void GmmLib::GmmXe_LPGTextureCalc::SetPlaneUnAlignedTexOffsetInfo(GMM_TEXTURE_INFO *pTexInfo, uint32_t YHeight, uint32_t VHeight)
{
    uint32_t UmdUHeight = 0, UmdVHeight = 0;

    pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y] = YHeight;
    if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 2)
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U] = VHeight;
        UmdUHeight                                                     = (GMM_GLOBAL_GFX_SIZE_T)((pTexInfo->Size / pTexInfo->Pitch) - pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U]);
    }
    else if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 3)
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U] =
        pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_V] = VHeight;
        UmdUHeight                                                     = (GMM_GLOBAL_GFX_SIZE_T)(pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_V] - pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U]);
        UmdVHeight                                                     = (GMM_GLOBAL_GFX_SIZE_T)(((pTexInfo->Size / pTexInfo->Pitch) - pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U]) / 2);
        __GMM_ASSERTPTR((UmdUHeight == UmdVHeight), VOIDRETURN);
    }

    __GMM_ASSERTPTR(((pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U] == YHeight) && (UmdUHeight == VHeight)), VOIDRETURN);
}

uint32_t GmmLib::GmmXe_LPGTextureCalc::IsTileAlignedPlanes(GMM_TEXTURE_INFO *pTexInfo)
{
    return pTexInfo->OffsetInfo.PlaneXe_LPG.IsTileAlignedPlanes;
}

uint32_t GmmLib::GmmXe_LPGTextureCalc::GetNumberOfPlanes(GMM_TEXTURE_INFO *pTexInfo)
{
    return pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes;
}

void GmmLib::GmmXe_LPGTextureCalc::GetPlaneIdForCpuBlt(GMM_TEXTURE_INFO *pTexInfo, GMM_RES_COPY_BLT *pBlt, uint32_t *PlaneId)
{
    uint32_t TotalHeight = 0;

    if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 2)
    {
        TotalHeight = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y] +
                                     pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U]);
    }
    else if(pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 3)
    {
        TotalHeight = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y] +
                                     pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U] +
                                     pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_V]);
    }
    else
    {
        TotalHeight = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y]); //YV12 exception
    }

    // Determine if BLT rectange is for monolithic surface or contained in specific Y/UV plane
    if(((pBlt->Gpu.OffsetY + pBlt->Blt.Height <= pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U]) || pTexInfo->OffsetInfo.PlaneXe_LPG.NoOfPlanes == 1) &&
       (pBlt->Gpu.OffsetX + pBlt->Blt.Width <= pTexInfo->BaseWidth))
    {
        *PlaneId = GMM_PLANE_Y;
    }
    else if(pBlt->Gpu.OffsetY >= pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U] &&
            (pBlt->Gpu.OffsetY + pBlt->Blt.Height <= (pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U] + pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U])) &&
            (pBlt->Gpu.OffsetX + pBlt->Blt.Width <= pTexInfo->BaseWidth))
    {
        *PlaneId = GMM_PLANE_U;
    }
    else if(pBlt->Gpu.OffsetY >= pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_V] &&
            (pBlt->Gpu.OffsetY + pBlt->Blt.Height <= (pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_V] + pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U])) &&
            (pBlt->Gpu.OffsetX + pBlt->Blt.Width <= pTexInfo->BaseWidth))
    {
        *PlaneId = GMM_PLANE_V;
    }

    // For smaller surface, BLT rect may fall in Y Plane due to tile alignment but user may have requested monolithic BLT
    if(pBlt->Gpu.OffsetX == 0 &&
       pBlt->Gpu.OffsetY == 0 &&
       pBlt->Blt.Height >= TotalHeight)
    {
        *PlaneId = GMM_MAX_PLANE;
    }
}

void GmmLib::GmmXe_LPGTextureCalc::GetBltInfoPerPlane(GMM_TEXTURE_INFO *pTexInfo, GMM_RES_COPY_BLT *pBlt, uint32_t PlaneId)
{
    if(PlaneId == GMM_PLANE_Y)
    {
        pBlt->Gpu.OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_Y]);
        pBlt->Gpu.OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_Y]);
        pBlt->Blt.Height  = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_Y]);
    }
    else if(PlaneId == GMM_PLANE_U)
    {
        pBlt->Gpu.OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_U]);
        pBlt->Gpu.OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_U]);

        pBlt->Sys.pData  = (char *)pBlt->Sys.pData + uint32_t(pBlt->Blt.Height * pBlt->Sys.RowPitch);
        pBlt->Blt.Height = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U]);
        if(pTexInfo->Flags.Info.RedecribedPlanes)
        {
            __GMM_ASSERT(0);
        }
    }
    else
    {
        pBlt->Gpu.OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.X[GMM_PLANE_V]);
        pBlt->Gpu.OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.Y[GMM_PLANE_V]);
        pBlt->Blt.Height  = GFX_ULONG_CAST(pTexInfo->OffsetInfo.PlaneXe_LPG.UnAligned.Height[GMM_PLANE_U]);
        pBlt->Sys.pData   = (char *)pBlt->Sys.pData + uint32_t(pBlt->Blt.Height * pBlt->Sys.RowPitch);
    }
}
