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
GMM_STATUS GMM_STDCALL GmmLib::GmmGen7TextureCalc::FillTex2D(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   Width, Height, BitsPerPixel;
    uint32_t   HAlign, VAlign;
    uint32_t   CompressHeight, CompressWidth, CompressDepth;
    uint32_t   AlignedWidth, BlockHeight, ExpandedArraySize, Pitch;
    uint8_t    Compress = 0;
    GMM_STATUS Status;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);

    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    BitsPerPixel = pTexInfo->BitsPerPixel;
    Height       = pTexInfo->BaseHeight;
    Width        = GFX_ULONG_CAST(pTexInfo->BaseWidth);

    pTexInfo->MSAA.NumSamples = GFX_MAX(pTexInfo->MSAA.NumSamples, 1);

    ExpandedArraySize =
    GFX_MAX(pTexInfo->ArraySize, 1) *
    ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1) * // Cubemaps simply 6-element, 2D arrays.
    ((pTexInfo->Flags.Gpu.Depth || pTexInfo->Flags.Gpu.SeparateStencil) ?
     1 :
     pTexInfo->MSAA.NumSamples); // Gen7 MSAA (non-Depth/Stencil) RT samples stored as array planes.

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

    Compress = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);

    /////////////////////////////////
    // Calculate Block Surface Height
    /////////////////////////////////

    // Adjust total height for arrayed 2D textures
    if(ExpandedArraySize > 1)
    {
        uint32_t Height0, Height1;

        Height0 = __GMM_EXPAND_HEIGHT(this, Height, VAlign, pTexInfo);

        // If not ARYSPC_LOD0-eligible...
        if((pTexInfo->MaxLod > 0) ||
           pTexInfo->Flags.Gpu.Depth || // Depth/HiZ/Stencil buffers not ARYSPC_LOD0-compatible.
           pTexInfo->Flags.Gpu.HiZ ||
           pTexInfo->Flags.Gpu.SeparateStencil)
        {
            Height1 = __GMM_EXPAND_HEIGHT(this, Height >> 1, VAlign, pTexInfo);

            // QPitch = (h0 + h1 + 12j) * pitch
            BlockHeight = Height0 + Height1 + 12 * VAlign;
        }
        else // SURFACE_STATE: Surface Array Spacing: ARYSPC_LOD0
        {
            // QPitch = h0 * pitch
            BlockHeight                               = Height0;
            pTexInfo->Alignment.ArraySpacingSingleLod = true;
        }

        if(Compress)
        {
            BlockHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            BlockHeight /= 2;
        }

        // Compute total array height
        BlockHeight *= ExpandedArraySize;
    }
    else
    {
        BlockHeight = Get2DMipMapHeight(pTexInfo);
    }


    ///////////////////////////////////
    // Calculate Pitch
    ///////////////////////////////////

    AlignedWidth = __GMM_EXPAND_WIDTH(this, Width, HAlign, pTexInfo);

    // Calculate special pitch case of small dimensions where LOD1 + LOD2 widths are greater
    // than LOD0.  e.g. dimensions 4x4 and MinPitch == 1
    if(pTexInfo->MaxLod >= 2)
    {
        uint32_t AlignedWidthLod1, AlignedWidthLod2;

        AlignedWidthLod1 = __GMM_EXPAND_WIDTH(this, Width >> 1, HAlign, pTexInfo);
        AlignedWidthLod2 = __GMM_EXPAND_WIDTH(this, Width >> 2, HAlign, pTexInfo);

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

    // Align height to even row to cover for HW over - fetch
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

    GMM_DPF_EXIT;
    return (Status);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the mip offset of given LOD in 2D mip layout
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     offset value in bytes
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GmmLib::GmmGen7TextureCalc::Get2DTexOffsetAddressPerMip(GMM_TEXTURE_INFO *pTexInfo,
                                                                       uint32_t          MipLevel)
{
    GMM_GFX_SIZE_T MipOffset;
    uint32_t       AlignedMipHeight, i, MipHeight, OffsetHeight;
    uint32_t       HAlign, VAlign;
    uint32_t       CompressHeight, CompressWidth, CompressDepth;
    uint8_t        Compress;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    HAlign = pTexInfo->Alignment.HAlign;
    VAlign = pTexInfo->Alignment.VAlign;

    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    Compress = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);

    MipHeight    = pTexInfo->BaseHeight;
    OffsetHeight = 0;

    // Mips 0 and 1 are on the left edge...
    if(MipLevel < 2)
    {
        MipOffset = 0;
    }
    else // Mip2 and beyond are to the right of Mip1...
    {
        uint32_t Mip1Width = GFX_ULONG_CAST(pTexInfo->BaseWidth) >> 1;

        Mip1Width = __GMM_EXPAND_WIDTH(this, Mip1Width, HAlign, pTexInfo);

        if(Compress)
        {
            Mip1Width /= CompressWidth;
            
	    if((pGmmLibContext->GetWaTable().WaAstcCorruptionForOddCompressedBlockSizeX || pTexInfo->Flags.Wa.CHVAstcSkipVirtualMips) && pPlatform->FormatTable[pTexInfo->Format].ASTC && CompressWidth == 5)
            {
                uint32_t Width1   = (pTexInfo->BaseWidth == 1) ? 1 : (GFX_ULONG_CAST(pTexInfo->BaseWidth) >> 1);
                uint32_t Modulo10 = Width1 % 10;
                if(Modulo10 >= 1 && Modulo10 <= CompressWidth)
                {
                    Mip1Width += 3;
                }
            }
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            Mip1Width *= 2;
        }

        MipOffset = (GMM_GFX_SIZE_T)Mip1Width * pTexInfo->BitsPerPixel >> 3;
    }

    for(i = 1; i <= MipLevel; i++)
    {
        AlignedMipHeight = GFX_ULONG_CAST(__GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo));

        if(Compress)
        {
            AlignedMipHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            AlignedMipHeight /= 2;
        }

        OffsetHeight += ((i != 2) ? AlignedMipHeight : 0);

        MipHeight >>= 1;
    }

    MipOffset += OffsetHeight * GFX_ULONG_CAST(pTexInfo->Pitch);

    GMM_DPF_EXIT;
    return (MipOffset);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates height of the 2D mip layout
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     Height of 2D mip layout
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen7TextureCalc::Get2DMipMapHeight(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t Height, BlockHeight, NumLevels; // Final height for 2D surface
    uint32_t HeightLines, HeightLinesLevel0, HeightLinesLevel1, HeightLinesLevel2;
    uint32_t VAlign, CompressHeight, CompressWidth, CompressDepth;
    uint32_t i;
    uint8_t  Compress;

    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    // Mip 0 height is needed later
    Height      = pTexInfo->BaseHeight;
    Compress    = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);
    NumLevels   = pTexInfo->MaxLod;
    HeightLines = Height;
    VAlign      = pTexInfo->Alignment.VAlign;
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    HeightLinesLevel0 = __GMM_EXPAND_HEIGHT(this, HeightLines, VAlign, pTexInfo);

    if(Compress)
    {
        HeightLinesLevel0 /= CompressHeight;
    }
    else if(pTexInfo->Flags.Gpu.SeparateStencil)
    {
        HeightLinesLevel0 /= 2;
    }

    // Start out with mip0
    BlockHeight = HeightLinesLevel0;

    // Height of mip1 and height of all others mips(2,3,4,5,,) needed later
    HeightLinesLevel1 = HeightLinesLevel2 = 0;
    for(i = 1; i <= NumLevels; i++)
    {
        uint32_t AlignedHeightLines;

        HeightLines >>= 1;

        AlignedHeightLines = __GMM_EXPAND_HEIGHT(this, HeightLines, VAlign, pTexInfo);

        if(Compress)
        {
            AlignedHeightLines /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            AlignedHeightLines /= 2;
        }

        if(i == 1)
        {
            HeightLinesLevel1 = AlignedHeightLines;
        }
        else
        {
            HeightLinesLevel2 += AlignedHeightLines;
        }
    }

    // If mip1 height covers all others then that is all we need
    if(HeightLinesLevel1 >= HeightLinesLevel2)
    {
        BlockHeight += GFX_ALIGN_NP2(HeightLinesLevel1, VAlign);
    }
    else
    {
        BlockHeight += GFX_ALIGN_NP2(HeightLinesLevel2, VAlign);
    }

    GMM_DPF_EXIT;
    return (BlockHeight);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the address offset for each mip map of 2D texture and store them
/// into the GMM_TEXTURE_INFO for surf state programming.
///
/// @param[in]  pTexInfo: pointer to ::GMM_TEXTURE_INFO
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen7TextureCalc::Fill2DTexOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t i;

    __GMM_ASSERTPTR(pTexInfo, VOIDRETURN);

    GMM_DPF_ENTER;

    // QPitch: Array Element-to-Element, or Cube Face-to-Face Pitch...
    // -------------------------------------------------------------------------
    // Note: Gen7 MSAA RT samples stored as contiguous array planes--
    // e.g. MSAA-4X'ed array elements A and B stored: A0 A1 A2 A3 B0 B1 B2 B3.
    // However, for GMM's purposes QPitch is still the distance between
    // elements--not the distance between samples.
    // -------------------------------------------------------------------------
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
        uint32_t Height, Height0, Height1, ArrayQPitch, VAlign;

        Height = pTexInfo->BaseHeight;

        VAlign = pTexInfo->Alignment.VAlign;

        Height0 = __GMM_EXPAND_HEIGHT(this, Height, VAlign, pTexInfo);
        Height1 = __GMM_EXPAND_HEIGHT(this, Height >> 1, VAlign, pTexInfo);

        if(!pTexInfo->Alignment.ArraySpacingSingleLod)
        {
            // QPitch = (h0 + h1 + 12j) * pitch
            ArrayQPitch = Height0 + Height1 + 12 * VAlign;
        }
        else // SURFACE_STATE: Surface Array Spacing: ARYSPC_LOD0
        {
            // QPitch = h0 * pitch
            ArrayQPitch = Height0;
        }

        if(GmmIsCompressed(pGmmLibContext, pTexInfo->Format))
        {
            uint32_t CompressHeight, CompressWidth, CompressDepth;

            GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

            ArrayQPitch /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            ArrayQPitch /= 2;
        }

        if((pTexInfo->MSAA.NumSamples > 1) && !(pTexInfo->Flags.Gpu.Depth || pTexInfo->Flags.Gpu.SeparateStencil))
        {
            // Gen7 MSAA (non-Depth/Stencil) RT samples stored as array planes;
            // QPitch still element-to-element, not sample-to-sample.
            ArrayQPitch *= pTexInfo->MSAA.NumSamples;
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
/// Calculates total height of an arrayed 3D mip layout
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     height of arrayed 3D mip layout
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen7TextureCalc::GetTotal3DHeight(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t          AlignedHeight, BlockHeight, Depth;
    uint8_t           Compressed;
    uint32_t          i, MipsInThisRow, MipLevel, MipRows;
    uint32_t          Total3DHeight = 0, UnitAlignHeight;
    uint32_t          CompressHeight, CompressWidth, CompressDepth;
    GMM_TEXTURE_CALC *pTextureCalc;

    GMM_DPF_ENTER;

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(pTexInfo, pGmmLibContext);

    BlockHeight     = pTexInfo->BaseHeight;
    Depth           = pTexInfo->Depth;
    MipLevel        = pTexInfo->MaxLod;
    UnitAlignHeight = pTexInfo->Alignment.VAlign;

    Compressed = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);
    pTextureCalc->GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    // All mip0s of all planes are stacked together, then mip1s and so on...
    for(i = 0; i <= MipLevel; i++)
    {
        BlockHeight   = GFX_MAX(BlockHeight, UnitAlignHeight);
        AlignedHeight = GFX_ALIGN(BlockHeight, UnitAlignHeight);

        if(Compressed)
        {
            AlignedHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            AlignedHeight /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS)
        {
            if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
            {
                AlignedHeight /= 32;
            }
            else if(pTexInfo->Flags.Gpu.__NonMsaaTileXCcs)
            {
                AlignedHeight /= 16;
            }
        }

        // See how many mip can fit in one row
        MipsInThisRow = GFX_2_TO_POWER_OF(i);

        // calculate if the mips will spill over to multiple rows
        MipRows = GFX_CEIL_DIV(GFX_MAX(1, Depth >> i), MipsInThisRow);

        Total3DHeight += MipRows * AlignedHeight;

        // next level height
        BlockHeight >>= 1;
    }

    GMM_DPF_EXIT;
    return Total3DHeight;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the address offset for each mip map of 3D texture and store them
/// into the GMM_TEXTURE_INFO for surf state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen7TextureCalc::Fill3DTexOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t          AlignedMipHeight, AlignedMipWidth;
    uint32_t          i, Depth;
    uint32_t          MipsInThisRow, MipLevel, MipRows;
    uint32_t          MipHeight, MipWidth;
    uint32_t          UnitAlignHeight, UnitAlignWidth;
    uint32_t          CompressHeight, CompressWidth, CompressDepth;
    uint32_t          OffsetMipRows = 0;
    GMM_GFX_SIZE_T    OffsetValue;
    uint8_t           Compress;
    GMM_TEXTURE_CALC *pTextureCalc;

    __GMM_ASSERT(pTexInfo);

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(pTexInfo, pGmmLibContext);

    // Assign directly to unaligned MipMap dimension variables
    // There isn't a need to save original dimensions
    MipWidth  = GFX_ULONG_CAST(pTexInfo->BaseWidth);
    MipHeight = pTexInfo->BaseHeight;

    // Align before we compress
    UnitAlignWidth  = pTexInfo->Alignment.HAlign;
    UnitAlignHeight = pTexInfo->Alignment.VAlign;
    Compress        = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);
    pTextureCalc->GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    // Aligned MipMap Dimensions
    AlignedMipWidth  = GFX_ALIGN(MipWidth, UnitAlignWidth);
    AlignedMipHeight = GFX_ALIGN(MipHeight, UnitAlignHeight);

    Depth    = pTexInfo->Depth;
    MipLevel = pTexInfo->MaxLod;

    // Start with base offset
    OffsetValue = 0;

    // calculate the offset for each Mip level
    for(i = 0; i <= MipLevel; i++)
    {
        // store the value in blockdesc
        if(Compress)
        {
            // If there is compression, compress after the alignment at each level
            AlignedMipWidth /= CompressWidth;
            AlignedMipHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            AlignedMipWidth *= 2;
            AlignedMipHeight /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS)
        {
            if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
            {
                switch(pTexInfo->BitsPerPixel)
                {
                    case 32:
                        AlignedMipWidth /= 8;
                        break;
                    case 64:
                        AlignedMipWidth /= 4;
                        break;
                    case 128:
                        AlignedMipWidth /= 2;
                        break;
                    default:
                        __GMM_ASSERT(0);
                }

                AlignedMipHeight /= 32;
            }
            else if(pTexInfo->Flags.Gpu.__NonMsaaTileXCcs)
            {
                switch(pTexInfo->BitsPerPixel)
                {
                    case 32:
                        AlignedMipWidth /= 16;
                        break;
                    case 64:
                        AlignedMipWidth /= 8;
                        break;
                    case 128:
                        AlignedMipWidth /= 4;
                        break;
                    default:
                        __GMM_ASSERT(0);
                }

                AlignedMipHeight /= 16;
            }
        }

        pTexInfo->OffsetInfo.Texture3DOffsetInfo.Offset[i] = OffsetValue;

        // See how many mip can fit in one row
        MipsInThisRow = GFX_2_TO_POWER_OF(i);

        // Slice pitch for LOD0
        if(MipsInThisRow == 1)
        {
            pTexInfo->OffsetInfo.Texture3DOffsetInfo.Mip0SlicePitch =
            AlignedMipHeight * pTexInfo->Pitch;
        }

        // calculate if the mips will spill over to multiple rows
        MipRows = GFX_CEIL_DIV(GFX_MAX(1, Depth >> i), MipsInThisRow);

        // Offset in terms of height
        OffsetMipRows += MipRows * AlignedMipHeight;

        // For a particular mip This is offset of a base slice (i.e. Slice 0)
        OffsetValue = OffsetMipRows * pTexInfo->Pitch;

        // next level height
        MipHeight >>= 1;
        // Clamp such that mip height is at least1
        MipHeight        = GFX_MAX(MipHeight, 1);
        AlignedMipHeight = GFX_ALIGN(MipHeight, UnitAlignHeight);

        MipWidth >>= 1;
        // Clamp such that mip width is at least 1
        MipWidth        = GFX_MAX(MipWidth, 1);
        AlignedMipWidth = GFX_ALIGN(MipWidth, UnitAlignWidth);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the 3D offset and QPitch for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen7TextureCalc::FillTex3D(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   AlignedMipWidth;
    uint32_t   BitsPerPixel;
    uint32_t   Depth, Height, Width;
    uint32_t   i, MipsInThisRow, MipWidth;
    uint32_t   RenderPitch = 0, ThisRowPitch;
    uint32_t   UnitAlignWidth;
    uint32_t   Total3DHeight;
    uint32_t   WidthBytesPhysical;
    uint8_t    Compress;
    uint32_t   CompressHeight, CompressWidth, CompressDepth;
    bool       SeparateStencil;
    GMM_STATUS Status;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);

    const __GMM_PLATFORM_RESOURCE *pPlatformResource = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    BitsPerPixel = pTexInfo->BitsPerPixel;
    Height       = pTexInfo->BaseHeight;
    Width        = GFX_ULONG_CAST(pTexInfo->BaseWidth);
    Depth        = pTexInfo->Depth;

    // Align before we compress
    UnitAlignWidth = pTexInfo->Alignment.HAlign;
    Compress       = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);
    SeparateStencil = pTexInfo->Flags.Gpu.SeparateStencil ? true : false;

    // Unaligned MipMap dimension variables
    MipWidth = Width;

    // Aligned MipMap dimension variables
    AlignedMipWidth = GFX_ALIGN(MipWidth, UnitAlignWidth);

    // Calculate the render pitch exactly the same way we do the
    // offset for each Mip level
    for(i = 0; i <= pTexInfo->MaxLod; i++)
    {
        if(Compress)
        {
            // If there is compression, compress after the alignment at each level
            AlignedMipWidth /= CompressWidth;
        }
        else if(SeparateStencil)
        {
            AlignedMipWidth *= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS)
        {
            if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
            {
                switch(BitsPerPixel)
                {
                    case 32:
                        AlignedMipWidth /= 8;
                        break;
                    case 64:
                        AlignedMipWidth /= 4;
                        break;
                    case 128:
                        AlignedMipWidth /= 2;
                        break;
                    default:
                        __GMM_ASSERT(0);
                }
            }
            else if(pTexInfo->Flags.Gpu.__NonMsaaTileXCcs)
            {
                switch(BitsPerPixel)
                {
                    case 32:
                        AlignedMipWidth /= 16;
                        break;
                    case 64:
                        AlignedMipWidth /= 8;
                        break;
                    case 128:
                        AlignedMipWidth /= 4;
                        break;
                    default:
                        __GMM_ASSERT(0);
                }
            }
        }

        // See how many mip can fit in one row
        MipsInThisRow = GFX_2_TO_POWER_OF(i);

        // LOD planes may be less than MipsInThisRow, take the smaller value for pitch
        MipsInThisRow = GFX_MIN(GFX_MAX(1, (Depth >> i)), MipsInThisRow);
        ThisRowPitch  = AlignedMipWidth * MipsInThisRow;

        // Default pitch
        WidthBytesPhysical = ThisRowPitch * BitsPerPixel >> 3;

        if(RenderPitch < WidthBytesPhysical)
        {
            RenderPitch = WidthBytesPhysical;
        }

        MipWidth >>= 1;
        // Clamp such that mip width is at least 1
        MipWidth        = GFX_MAX(MipWidth, 1);
        AlignedMipWidth = GFX_ALIGN(MipWidth, UnitAlignWidth);
    }

    WidthBytesPhysical = RenderPitch;

    // Make sure the pitch satisfy linear min pitch requirment
    WidthBytesPhysical = GFX_MAX(WidthBytesPhysical,
                                 pRestrictions->MinPitch);

    // Make sure pitch satisfy alignment restriction
    WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical,
                                   pRestrictions->PitchAlignment);

    // Get Total height for the entire 3D texture
    Total3DHeight = GetTotal3DHeight(pTexInfo);

    if(GMM_IS_TILED(pPlatformResource->TileInfo[pTexInfo->TileMode]))
    {
        // Align to tile boundary
        Total3DHeight = GFX_ALIGN(Total3DHeight,
                                  pPlatformResource->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
        WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical,
                                       pPlatformResource->TileInfo[pTexInfo->TileMode].LogicalTileWidth);
    }

    // If a texture is YUV packed, 96, or 48 bpp then one row plus 16 bytes of
    // padding needs to be added. Since this will create a none pitch aligned
    // surface the padding is aligned to the next row
    if(GmmIsYUVPacked(pTexInfo->Format) ||
       (pTexInfo->BitsPerPixel == GMM_BITS(96)) ||
       (pTexInfo->BitsPerPixel == GMM_BITS(48)))
    {
        Total3DHeight += GMM_SCANLINES(1) + GFX_CEIL_DIV(GMM_BYTES(16), WidthBytesPhysical);
    }

    Total3DHeight = GFX_ALIGN(Total3DHeight, __GMM_EVEN_ROW);

    if((Status = // <-- Note assignment.
        FillTexPitchAndSize(
        pTexInfo, WidthBytesPhysical, Total3DHeight, pRestrictions)) == GMM_SUCCESS)
    {
        Fill3DTexOffsetAddress(pTexInfo);
    }

    GMM_DPF_EXIT;
    return (Status);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Allocates the 1D mip layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen7TextureCalc::FillTex1D(GMM_TEXTURE_INFO * pTexInfo,
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
GMM_STATUS GMM_STDCALL GmmLib::GmmGen7TextureCalc::FillTexCube(GMM_TEXTURE_INFO * pTexInfo,
                                                               __GMM_BUFFER_TYPE *pRestrictions)
{
    return FillTex2D(pTexInfo, pRestrictions);
}
