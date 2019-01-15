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


/////////////////////////////////////////////////////////////////////////////////////
/// Allocates the 2D mip layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen8TextureCalc::FillTex2D(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   Width, Height, BitsPerPixel;
    uint32_t   HAlign, VAlign;
    uint32_t   CompressHeight, CompressWidth, CompressDepth;
    uint32_t   AlignedWidth, BlockHeight, ExpandedArraySize, Pitch;
    uint8_t    Compress = 0;
    GMM_STATUS Status;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    BitsPerPixel = pTexInfo->BitsPerPixel;
    Height       = pTexInfo->BaseHeight;
    Width        = GFX_ULONG_CAST(pTexInfo->BaseWidth);

    pTexInfo->MSAA.NumSamples = GFX_MAX(pTexInfo->MSAA.NumSamples, 1);

    ExpandedArraySize =
    GFX_MAX(pTexInfo->ArraySize, 1) *
    ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1) * // Cubemaps simply 6-element, 2D arrays.
    ((pTexInfo->Flags.Gpu.Depth || pTexInfo->Flags.Gpu.SeparateStencil) ?
     1 :
     pTexInfo->MSAA.NumSamples); // MSAA (non-Depth/Stencil) RT samples stored as array planes.

    //
    // Check for color separation
    //
    if(pTexInfo->Flags.Gpu.ColorSeparation || pTexInfo->Flags.Gpu.ColorSeparationRGBX)
    {
        bool csRestrictionsMet = (((ExpandedArraySize <= 2) &&
                                   (ExpandedArraySize == pTexInfo->ArraySize) &&
                                   ((pTexInfo->Format == GMM_FORMAT_R8G8B8A8_UNORM) ||
                                    (pTexInfo->Format == GMM_FORMAT_R8G8B8A8_UNORM_SRGB) ||
                                    (pTexInfo->Format == GMM_FORMAT_B8G8R8A8_UNORM) ||
                                    (pTexInfo->Format == GMM_FORMAT_B8G8R8A8_UNORM_SRGB) ||
                                    (pTexInfo->Format == GMM_FORMAT_B8G8R8X8_UNORM) ||
                                    (pTexInfo->Format == GMM_FORMAT_B8G8R8X8_UNORM_SRGB)) &&
                                   ((pTexInfo->Flags.Gpu.ColorSeparation && (Width % 16) == 0) ||
                                    (pTexInfo->Flags.Gpu.ColorSeparationRGBX && (Width % 12) == 0))));

        if(csRestrictionsMet)
        {
            ExpandedArraySize = GMM_COLOR_SEPARATION_ARRAY_SIZE;
        }
        else
        {
            pTexInfo->Flags.Gpu.ColorSeparation     = 0;
            pTexInfo->Flags.Gpu.ColorSeparationRGBX = 0;
        }
    }

    HAlign = pTexInfo->Alignment.HAlign;
    VAlign = pTexInfo->Alignment.VAlign;
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    Compress = GmmIsCompressed(pTexInfo->Format);

    /////////////////////////////////
    // Calculate Block Surface Height
    /////////////////////////////////

    if(ExpandedArraySize > 1)
    {
        uint32_t Height0, Height1, Mip0BlockHeight, Slice0Delta = 0;

        Height0 = __GMM_EXPAND_HEIGHT(this, Height, VAlign, pTexInfo);
        Height1 = __GMM_EXPAND_HEIGHT(this, Height >> 1, VAlign, pTexInfo);

        Mip0BlockHeight = BlockHeight = (pTexInfo->MaxLod > 0) ?
                                        Height0 + Height1 + 12 * VAlign :
                                        Height0;
        BlockHeight -= (pTexInfo->Flags.Wa.CHVAstcSkipVirtualMips) ? Height0 : 0;

        if(pTexInfo->Flags.Gpu.S3dDx && pGmmGlobalContext->GetSkuTable().FtrDisplayEngineS3d)
        {
            BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
        }

        // QPitch for compressed surface must be multiple of BlockHeight and 4...
        if(Compress && (CompressHeight % 4))
        {
            uint32_t LCM    = CompressHeight * ((CompressHeight % 2) ? 4 : 2);
            BlockHeight     = GFX_ALIGN_NP2(BlockHeight, LCM);
            Mip0BlockHeight = GFX_ALIGN_NP2(Mip0BlockHeight, LCM);
        }

        // Gen8 QPitch programming refers to the logical view, not physical.
        pTexInfo->Alignment.QPitch = BlockHeight;

        if(Compress)
        {
            BlockHeight /= CompressHeight;
            Mip0BlockHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            BlockHeight /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS)
        {
            if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
            {
                BlockHeight /= 32;
            }
            else if(pTexInfo->Flags.Gpu.__NonMsaaTileXCcs)
            {
                BlockHeight /= 16;
            }
        }

        Slice0Delta = (pTexInfo->Flags.Wa.CHVAstcSkipVirtualMips) ? (Mip0BlockHeight - BlockHeight) : 0;
        BlockHeight *= ExpandedArraySize;
        BlockHeight += Slice0Delta;
    }
    else
    {
        pTexInfo->Alignment.QPitch = 0;

        BlockHeight = Get2DMipMapHeight(pTexInfo);
    }

    ///////////////////////////////////
    // Calculate Pitch
    ///////////////////////////////////

    AlignedWidth = __GMM_EXPAND_WIDTH(this, Width, HAlign, pTexInfo);

    // Calculate special pitch case of small dimensions where LOD1 + LOD2 widths
    // are greater than LOD0. e.g. dimensions 4x4 and MinPitch == 1
    if(pTexInfo->MaxLod >= 2)
    {
        uint32_t AlignedWidthLod1, AlignedWidthLod2;

        AlignedWidthLod1 = __GMM_EXPAND_WIDTH(this, Width >> 1, HAlign, pTexInfo);
        AlignedWidthLod2 = __GMM_EXPAND_WIDTH(this, Width >> 2, HAlign, pTexInfo);

        if((pGmmGlobalContext->GetWaTable().WaAstcCorruptionForOddCompressedBlockSizeX || pTexInfo->Flags.Wa.CHVAstcSkipVirtualMips) && pPlatform->FormatTable[pTexInfo->Format].ASTC && CompressWidth == 5)
        {
            uint32_t Width1   = (Width == 1) ? 1 : (Width >> 1);
            uint32_t Modulo10 = Width1 % 10;
            if(Modulo10 >= 1 && Modulo10 <= CompressWidth)
            {
                AlignedWidthLod2 += 3 * CompressWidth;
            }
        }
        AlignedWidth = GFX_MAX(AlignedWidth, AlignedWidthLod1 + AlignedWidthLod2);
    }

    if(Compress)
    {
        AlignedWidth /= CompressWidth;
    }
    else if(pTexInfo->Flags.Gpu.SeparateStencil)
    {
        AlignedWidth *= 2;
    }
    else if(pTexInfo->Flags.Gpu.CCS)
    {
        if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
        {
            switch(pTexInfo->BitsPerPixel)
            {
                case 32:
                    AlignedWidth /= 8;
                    break;
                case 64:
                    AlignedWidth /= 4;
                    break;
                case 128:
                    AlignedWidth /= 2;
                    break;
                default:
                    __GMM_ASSERT(0);
            }
        }
        else if(pTexInfo->Flags.Gpu.__NonMsaaTileXCcs)
        {
            switch(pTexInfo->BitsPerPixel)
            {
                case 32:
                    AlignedWidth /= 16;
                    break;
                case 64:
                    AlignedWidth /= 8;
                    break;
                case 128:
                    AlignedWidth /= 4;
                    break;
                default:
                    __GMM_ASSERT(0);
            }
        }
    }
    else if(pTexInfo->Flags.Gpu.ColorSeparation)
    {
        AlignedWidth *= pTexInfo->ArraySize;
        __GMM_ASSERT(0 == (AlignedWidth % GMM_COLOR_SEPARATION_WIDTH_DIVISION));
        AlignedWidth /= GMM_COLOR_SEPARATION_WIDTH_DIVISION;
    }
    else if(pTexInfo->Flags.Gpu.ColorSeparationRGBX)
    {
        AlignedWidth *= pTexInfo->ArraySize;
        __GMM_ASSERT(0 == (AlignedWidth % GMM_COLOR_SEPARATION_RGBX_WIDTH_DIVISION));
        AlignedWidth /= GMM_COLOR_SEPARATION_RGBX_WIDTH_DIVISION;
    }

    // Default pitch
    Pitch = AlignedWidth * BitsPerPixel >> 3;

    // Make sure the pitch satisfy linear min pitch requirment
    Pitch = GFX_MAX(Pitch, pRestrictions->MinPitch);

    // Make sure pitch satisfy alignment restriction
    Pitch = GFX_ALIGN(Pitch, pRestrictions->PitchAlignment);

    ////////////////////
    // Adjust for Tiling
    ////////////////////
    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        Pitch       = GFX_ALIGN(Pitch, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth);
        BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);

        // If Tiled Resource or Undefined64KBSwizzle resource, align to 64KB tile size
        if((pTexInfo->Flags.Gpu.TiledResource || pTexInfo->Flags.Info.Undefined64KBSwizzle) &&
           (pTexInfo->Flags.Info.TiledY))
        {
            uint32_t ColFactor = 0, RowFactor = 0;
            uint32_t TRTileWidth = 0, TRTileHeight = 0;

            GmmGetD3DToHwTileConversion(pTexInfo, &ColFactor, &RowFactor);
            TRTileWidth  = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth * ColFactor;
            TRTileHeight = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight * RowFactor;

            Pitch       = GFX_ALIGN(Pitch, TRTileWidth);
            BlockHeight = GFX_ALIGN(BlockHeight, TRTileHeight);
        }
    }

    GMM_ASSERTDPF(pTexInfo->Flags.Info.LayoutBelow || !pTexInfo->Flags.Info.LayoutRight, "MIPLAYOUT_RIGHT not supported after Gen6!");
    pTexInfo->Flags.Info.LayoutBelow = 1;
    pTexInfo->Flags.Info.LayoutRight = 0;

    // If a texture is YUV packed, 96, or 48 bpp then one row plus 16 bytes of
    // padding needs to be added. Since this will create a none pitch aligned
    // surface the padding is aligned to the next row
    if(GmmIsYUVPacked(pTexInfo->Format) ||
       (pTexInfo->BitsPerPixel == GMM_BITS(96)) ||
       (pTexInfo->BitsPerPixel == GMM_BITS(48)))
    {
        BlockHeight += GMM_SCANLINES(1) + GFX_CEIL_DIV(GMM_BYTES(16), Pitch);
    }

    // Align height to even row to cover for HW over-fetch
    BlockHeight = GFX_ALIGN(BlockHeight, __GMM_EVEN_ROW);

    if((Status = // <-- Note assignment.
        FillTexPitchAndSize(
        pTexInfo, Pitch, BlockHeight, pRestrictions)) == GMM_SUCCESS)
    {
        Fill2DTexOffsetAddress(pTexInfo);

        // Init to no-packed mips. It'll be initialized when app calls to get packed
        // mips. Calculate packed mips here if there's a chance apps won't call to
        // get packed mips.
        pTexInfo->Alignment.PackedMipStartLod = GMM_TILED_RESOURCE_NO_PACKED_MIPS;
    }

    if(pTexInfo->Flags.Wa.CHVAstcSkipVirtualMips)
    {
        uint32_t i             = 0;
        uint64_t SkipMip0Tiles = 0;
        SkipMip0Tiles          = pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[1] /
                        (pTexInfo->Pitch * pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
        SkipMip0Tiles *= pTexInfo->Pitch * pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        pTexInfo->Size -= SkipMip0Tiles;
        for(i = 0; i <= pTexInfo->MaxLod; i++)
        {
            pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[i] -= SkipMip0Tiles;
        }
    }
    GMM_DPF_EXIT;

    return (Status);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the address offset for each mip map of 2D texture and store them into
/// the GMM_TEXTURE_INFO for surf state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen8TextureCalc::Fill2DTexOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t i;

    GMM_DPF_ENTER;

    // QPitch: Array Element-to-Element, or Cube Face-to-Face Pitch...
    if((pTexInfo->ArraySize <= 1) &&
       (pTexInfo->Type != RESOURCE_CUBE) &&
       !(pTexInfo->Flags.Gpu.ColorSeparation ||
         pTexInfo->Flags.Gpu.ColorSeparationRGBX))
    {
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender = 0;
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock   = 0;
    }
    else
    {
        uint32_t ArrayQPitch;
        uint32_t CompressHeight, CompressWidth, CompressDepth;

        GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

        ArrayQPitch = pTexInfo->Alignment.QPitch;

        if(GmmIsCompressed(pTexInfo->Format))
        {
            ArrayQPitch /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            ArrayQPitch /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS)
        {
            if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
            {
                ArrayQPitch /= 32;
            }
            else if(pTexInfo->Flags.Gpu.__NonMsaaTileXCcs)
            {
                ArrayQPitch /= 16;
            }
        }

        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender = ArrayQPitch * pTexInfo->Pitch;
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock   = ArrayQPitch * pTexInfo->Pitch;
    }

    for(i = 0; i <= pTexInfo->MaxLod; i++)
    {
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[i] = Get2DTexOffsetAddressPerMip(pTexInfo, i);
    }

    GMM_DPF_EXIT;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Allocates the 1D mip layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen8TextureCalc::FillTex1D(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    return FillTex2D(pTexInfo, pRestrictions);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the cube layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen8TextureCalc::FillTexCube(GMM_TEXTURE_INFO * pTexInfo,
                                                               __GMM_BUFFER_TYPE *pRestrictions)
{
    return FillTex2D(pTexInfo, pRestrictions);
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function does any special-case conversion from client-provided pseudo creation
/// parameters to actual parameters for CCS.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen8TextureCalc::MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS Status = GMM_SUCCESS;

    if(pTexInfo->MSAA.NumSamples > 1) // CCS for MSAA Compression
    {
        Status = MSAACompression(pTexInfo);
    }
    else // Non-MSAA CCS Use (i.e. Render Target Fast Clear)
    {
        if(!pTexInfo->Flags.Info.TiledW &&
           ((!pTexInfo->Flags.Info.Linear) ||
            (GMM_IS_4KB_TILE(pTexInfo->Flags) || GMM_IS_64KB_TILE(pTexInfo->Flags) ||
             (pTexInfo->Type == RESOURCE_BUFFER && pTexInfo->Flags.Info.Linear))) && //!Yf - deprecate Yf
           ((pTexInfo->BitsPerPixel == 32) ||
            (pTexInfo->BitsPerPixel == 64) ||
            (pTexInfo->BitsPerPixel == 128)))
        {
            // For non-MSAA CCS usage, the four tables of
            // requirements:
            // (1) RT Alignment (GMM Don't Care: Occurs Naturally)
            // (2) ClearRect Alignment
            // (3) ClearRect Scaling (GMM Don't Care: GHAL3D Matter)
            // (4) Non-MSAA CCS Sizing

            // Gen8+:
            // Since mip-mapped and arrayed surfaces are supported, we
            // deal with alignment later at per mip level. Here, we set
            // tiling type only. TileX is not supported on Gen9+.
            // Pre-Gen8:
            // (!) For all the above, there are separate entries for
            // 32/64/128bpp--and then deals with PIXEL widths--Here,
            // though, we will unify by considering 8bpp table entries
            // (unlisted--i.e. do the math)--and deal with BYTE widths.

            // (1) RT Alignment -- The surface width and height don't
            // need to be padded to RT CL granularity. On HSW, all tiled
            // RT's will have appropriate alignment (given 4KB surface
            // base and no mip-map support) and appropriate padding
            // (due to tile padding). On BDW+, GMM uses H/VALIGN that
            // will guarantee the MCS RT alignment for all subresources.

            // (2) ClearRect Alignment -- I.e. FastClears must be done
            // with certain granularity:
            //  TileY:  512 Bytes x 128 Lines
            //  TileX: 1024 Bytes x  64 Lines
            // So a CCS must be sized to match that granularity (though
            // the RT itself need not be fully padded to that
            // granularity to use FastClear).

            // (4) Non-MSAA CCS Sizing -- CCS sizing is based on the
            // size of the FastClear (with granularity padding) for the
            // paired RT. CCS's (byte widths and heights) are scaled
            // down from their RT's by:
            //  TileY: 32 x 32
            //  TileX: 64 x 16

            // ### Example #############################################
            // RT:         800x600, 32bpp, TileY
            // 8bpp:      3200x600
            // FastClear: 3584x640 (for TileY FastClear Granularity of 512x128)
            // CCS:       112x20 (for TileY RT:CCS Sizing Downscale of 32x32)

            pTexInfo->Flags.Gpu.__NonMsaaTileYCcs = pTexInfo->Flags.Info.TiledY || pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs;
            pTexInfo->Flags.Gpu.__NonMsaaTileXCcs = pTexInfo->Flags.Info.TiledX;
        }
        else
        {
            GMM_ASSERTDPF(0, "Illegal CCS creation parameters!");
            Status = GMM_ERROR;
        }
    }
    return Status;
}
