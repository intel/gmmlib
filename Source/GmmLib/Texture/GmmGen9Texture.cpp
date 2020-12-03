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
/// Calculates the mip offset of given LOD in 1D mip layout
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     offset value in bytes
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GmmLib::GmmGen9TextureCalc::Get1DTexOffsetAddressPerMip(GMM_TEXTURE_INFO *pTexInfo,
                                                                       uint32_t          MipLevel)
{
    uint32_t       AlignedMipWidth, MipWidth, __MipLevel;
    uint32_t       i, HAlign;
    GMM_GFX_SIZE_T MipOffset = 0;
    uint8_t        Compressed;
    uint32_t       CompressHeight, CompressWidth, CompressDepth;

    GMM_DPF_ENTER;

    HAlign   = pTexInfo->Alignment.HAlign;
    MipWidth = GFX_ULONG_CAST(pTexInfo->BaseWidth);

    __MipLevel =
    (pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) ?
    GFX_MIN(MipLevel, pTexInfo->Alignment.MipTailStartLod) :
    MipLevel;

    Compressed = GmmIsCompressed(pTexInfo->Format);
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    for(i = 1; i <= __MipLevel; i++)
    {
        AlignedMipWidth = __GMM_EXPAND_WIDTH(this, MipWidth, HAlign, pTexInfo);

        if(Compressed)
        {
            AlignedMipWidth /= CompressWidth;
        }

        MipOffset += AlignedMipWidth;

        MipWidth = GFX_ULONG_CAST(GmmTexGetMipWidth(pTexInfo, i));
    }

    MipOffset *= (pTexInfo->BitsPerPixel >> 3);

    if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
       (MipLevel >= pTexInfo->Alignment.MipTailStartLod))
    {
        MipOffset += GetMipTailByteOffset(pTexInfo, MipLevel);
    }

    GMM_DPF_EXIT;

    return (MipOffset);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the address offset for each mip map of 1D texture and store them into
/// the GMM_TEXTURE_INFO for surf state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen9TextureCalc::Fill1DTexOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t i;

    GMM_DPF_ENTER;

    pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender =
    pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock =
    pTexInfo->Alignment.QPitch * pTexInfo->BitsPerPixel >> 3;

    for(i = 0; i <= pTexInfo->MaxLod; i++)
    {
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[i] = Get1DTexOffsetAddressPerMip(pTexInfo, i);
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
GMM_STATUS GMM_STDCALL GmmLib::GmmGen9TextureCalc::FillTex1D(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   ArraySize, BitsPerPixel, HAlign, i, Width, MipWidth;
    int64_t    Size;
    GMM_STATUS Status = GMM_SUCCESS;
    uint8_t    Compressed;
    uint32_t   CompressHeight, CompressWidth, CompressDepth;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);
    __GMM_ASSERT(pTexInfo->Flags.Info.Linear ||
                 pTexInfo->Flags.Info.TiledYf ||
                 GMM_IS_64KB_TILE(pTexInfo->Flags));

    pTexInfo->Flags.Info.Linear = 1;
    pTexInfo->Flags.Info.TiledW = 0;
    pTexInfo->Flags.Info.TiledX = 0;
    GMM_SET_4KB_TILE(pTexInfo->Flags, 0);

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    ArraySize    = GFX_MAX(pTexInfo->ArraySize, 1);
    BitsPerPixel = pTexInfo->BitsPerPixel;
    HAlign       = pTexInfo->Alignment.HAlign;

    Compressed = GmmIsCompressed(pTexInfo->Format);
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    if(pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags))
    {
        FindMipTailStartLod(pTexInfo);
    }

    /////////////////////////////
    // Calculate Surface QPitch
    /////////////////////////////

    Width    = __GMM_EXPAND_WIDTH(this, GFX_ULONG_CAST(pTexInfo->BaseWidth), HAlign, pTexInfo);
    MipWidth = Width;

    if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
       ((pTexInfo->Alignment.MipTailStartLod == 0) || (pTexInfo->MaxLod == 0)))
    {
        // Do nothing. Width is already aligned.
    }
    else
    {
        for(i = 1; i <= pTexInfo->MaxLod; i++)
        {
            uint32_t AlignedMipWidth;

            if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
               (i == pTexInfo->Alignment.MipTailStartLod))
            {
                Width += pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth;
                break;
            }
            else
            {
                MipWidth = GFX_ULONG_CAST(GmmTexGetMipWidth(pTexInfo, i));

                AlignedMipWidth = __GMM_EXPAND_WIDTH(this, MipWidth, HAlign, pTexInfo);

                if(Compressed)
                {
                    AlignedMipWidth /= CompressWidth;
                }

                Width += AlignedMipWidth;
            }
        }
    }

    pTexInfo->Alignment.QPitch = GFX_ALIGN((ArraySize > 1) ? Width : 0, HAlign); // in pixels

    pTexInfo->Pitch = 0;

    ///////////////////////////
    // Calculate Surface Size
    ///////////////////////////

    Width *= BitsPerPixel >> 3;

    Size = GFX_ALIGN((uint64_t)Width * ArraySize, PAGE_SIZE);

    if(Size <= pPlatform->SurfaceMaxSize)
    {
        pTexInfo->Size = Size;

        Fill1DTexOffsetAddress(pTexInfo);
    }
    else
    {
        GMM_ASSERTDPF(0, "Surface too large!");
        Status = GMM_ERROR;
    }

    //////////////////////
    // Surface Alignment
    //////////////////////

    if(!pTexInfo->Alignment.BaseAlignment || __GMM_IS_ALIGN(pRestrictions->Alignment, pTexInfo->Alignment.BaseAlignment))
    {
        pTexInfo->Alignment.BaseAlignment = pRestrictions->Alignment;
    }
    else if(__GMM_IS_ALIGN(pTexInfo->Alignment.BaseAlignment, pRestrictions->Alignment))
    {
        // Do nothing: pTexInfo->Alignment.BaseAlignment is properly aligned
    }
    else
    {
        pTexInfo->Alignment.BaseAlignment = pTexInfo->Alignment.BaseAlignment * pRestrictions->Alignment;
        GMM_ASSERTDPF(0,
                      "Client requested alignment that is not properly aligned to HW requirements."
                      "Alignment is going to be much higher to match both client and HW requirements.\r\n");
    }

    GMM_DPF_EXIT;

    return (Status);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates height of the 2D mip layout on Gen9
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     height of 2D mip layout
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen9TextureCalc::Get2DMipMapHeight(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t BlockHeight, MipHeight;
    uint32_t HeightLinesLevel0, HeightLinesLevel1, HeightLinesLevel2;
    uint32_t i, MipLevel, VAlign, CompressHeight, CompressWidth, CompressDepth;
    uint8_t  Compressed;
    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    Compressed = GmmIsCompressed(pTexInfo->Format);
    MipHeight  = pTexInfo->BaseHeight;
    MipLevel   = pTexInfo->MaxLod;
    VAlign     = pTexInfo->Alignment.VAlign;
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    HeightLinesLevel0 = __GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo);

    if(Compressed)
    {
        HeightLinesLevel0 /= CompressHeight;
    }
    else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
    {
        HeightLinesLevel0 /= 2;
    }
    else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        HeightLinesLevel0 /= 16;
    }

    // Mip0 height...
    BlockHeight = HeightLinesLevel0;

    if((pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs) &&
       ((pTexInfo->Alignment.MipTailStartLod == 0) || (pTexInfo->MaxLod == 0)))
    {
        // Do nothing. Height is already aligned.
    }
    else
    {
        // Height of Mip1 and Mip2..n needed later...
        HeightLinesLevel1 = HeightLinesLevel2 = 0;
        for(i = 1; i <= MipLevel; i++)
        {
            uint32_t AlignedHeightLines;

            if((pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs) &&
               (i == pTexInfo->Alignment.MipTailStartLod))
            {
                AlignedHeightLines = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;

                if(i == 1)
                {
                    HeightLinesLevel1 = AlignedHeightLines;
                }
                else
                {
                    HeightLinesLevel2 += AlignedHeightLines;
                }

                break;
            }
            else
            {
                MipHeight = GmmTexGetMipHeight(pTexInfo, i);

                AlignedHeightLines = __GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo);

                if(Compressed)
                {
                    AlignedHeightLines /= CompressHeight;
                }
                else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
                {
                    AlignedHeightLines /= 2;
                }
                else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
                {
                    AlignedHeightLines /= 16;
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
        }

        // If Mip1 height covers all others, then that is all we need...
        if(!(pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs))
        {
            if(HeightLinesLevel1 >= HeightLinesLevel2)
            {
                BlockHeight += GFX_ALIGN(HeightLinesLevel1, VAlign);
            }
            else
            {
                BlockHeight += GFX_ALIGN(HeightLinesLevel2, VAlign);
            }
        }
        else
        {
            //TR mode- requires TileMode height alignment
            BlockHeight += (HeightLinesLevel1 >= HeightLinesLevel2) ? HeightLinesLevel1 : HeightLinesLevel2;
            BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
        }
    }

    GMM_DPF_EXIT;

    return (BlockHeight);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates total height of an arrayed 2D/3D mip layout
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     height of arrayed 2D/3D  mip layout
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen9TextureCalc::Get2DMipMapTotalHeight(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t BlockHeight, MipHeight;
    uint32_t HeightLinesLevel0, HeightLinesLevel1, HeightLinesLevel2;
    uint32_t i, MipLevel, VAlign;
    uint32_t AlignedHeightLines;

    GMM_DPF_ENTER;

    MipHeight = pTexInfo->BaseHeight;
    MipLevel  = pTexInfo->MaxLod;
    VAlign    = pTexInfo->Alignment.VAlign;

    MipLevel =
    (pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) ?
    GFX_MIN(MipLevel, pTexInfo->Alignment.MipTailStartLod) :
    MipLevel;


    HeightLinesLevel0 = __GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo);

    // Mip0 height...
    BlockHeight = HeightLinesLevel0;

    // Height of Mip1 and Mip2..n needed later...
    HeightLinesLevel1 = HeightLinesLevel2 = 0;
    for(i = 1; i <= MipLevel; i++)
    {
        MipHeight = GmmTexGetMipHeight(pTexInfo, i);

        AlignedHeightLines = __GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo);

        if(i == 1)
        {
            HeightLinesLevel1 = AlignedHeightLines;
        }
        else
        {
            HeightLinesLevel2 += AlignedHeightLines;
        }
    }

    // If Mip1 height covers all others, then that is all we need...
    if(HeightLinesLevel1 >= HeightLinesLevel2)
    {
        BlockHeight += HeightLinesLevel1;
    }
    else
    {
        BlockHeight += HeightLinesLevel2;
    }

    GMM_DPF_EXIT;

    return (BlockHeight);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the mip offset of given LOD in 2D/3D mip layout
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     ::GMM_GFX_SIZE_T  offset value in bytes
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GmmLib::GmmGen9TextureCalc::Get2DTexOffsetAddressPerMip(GMM_TEXTURE_INFO *pTexInfo,
                                                                       uint32_t          MipLevel)
{
    uint32_t       AlignedMipHeight, i, OffsetHeight;
    uint8_t        Compressed;
    uint32_t       HAlign, VAlign, __MipLevel;
    uint32_t       CompressHeight, CompressWidth, CompressDepth;
    uint32_t       MipHeight;
    GMM_GFX_SIZE_T MipOffset;

    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    HAlign     = pTexInfo->Alignment.HAlign;
    VAlign     = pTexInfo->Alignment.VAlign;
    Compressed = GmmIsCompressed(pTexInfo->Format);

    MipHeight    = pTexInfo->BaseHeight;
    OffsetHeight = 0;

    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    __MipLevel =
    (pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) ?
    GFX_MIN(MipLevel, pTexInfo->Alignment.MipTailStartLod) :
    MipLevel;

    if(__MipLevel < 2) // LOD0 and LOD1 are on the left edge...
    {
        MipOffset = 0;
    }
    else // LOD2 and beyond are to the right of LOD1...
    {
        uint32_t MipWidth     = GFX_ULONG_CAST(GmmTexGetMipWidth(pTexInfo, 1));
        uint32_t BitsPerPixel = pTexInfo->BitsPerPixel;

        MipWidth = __GMM_EXPAND_WIDTH(this, MipWidth, HAlign, pTexInfo);

        if(Compressed)
        {
            MipWidth /= CompressWidth;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
        {
            MipWidth *= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
        {
            BitsPerPixel = 8; // Aux Surfaces are 8bpp

            switch(pTexInfo->BitsPerPixel)
            {
                case 32:
                    MipWidth /= 8;
                    break;
                case 64:
                    MipWidth /= 4;
                    break;
                case 128:
                    MipWidth /= 2;
                    break;
                default:
                    __GMM_ASSERT(0);
            }
        }

        MipOffset = (GMM_GFX_SIZE_T)MipWidth * BitsPerPixel >> 3;
    }

    for(i = 1; i <= __MipLevel; i++)
    {
        AlignedMipHeight = GFX_ULONG_CAST(__GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo));

        if(Compressed)
        {
            AlignedMipHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
        {
            AlignedMipHeight /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
        {
            AlignedMipHeight /= 16;
        }

        OffsetHeight += ((i != 2) ? AlignedMipHeight : 0);

        MipHeight = GmmTexGetMipHeight(pTexInfo, i);
    }
    OffsetHeight *= GFX_MAX(pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileDepth, 1);

    MipOffset += OffsetHeight * GFX_ULONG_CAST(pTexInfo->Pitch);

    if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
       (MipLevel >= pTexInfo->Alignment.MipTailStartLod))
    {
        MipOffset += GetMipTailByteOffset(pTexInfo, MipLevel);
    }

    GMM_DPF_EXIT;
    return (MipOffset);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates the address offset for each mip map of 2D texture and store them into
/// the GMM_TEXTURE_INFO for surf state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen9TextureCalc::Fill2DTexOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t i;
    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    // QPitch: Array Element-to-Element, or Cube Face-to-Face Pitch...
    if((pTexInfo->ArraySize <= 1) &&
       (pTexInfo->Type != RESOURCE_3D) &&
       (pTexInfo->Type != RESOURCE_CUBE) &&
       !(pTexInfo->Flags.Gpu.ColorSeparation ||
         pTexInfo->Flags.Gpu.ColorSeparationRGBX))
    {
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender =
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock = 0;
    }
    else
    {
        uint32_t ArrayQPitch, Alignment;

        Alignment = pTexInfo->Alignment.VAlign;
        if((pTexInfo->Type == RESOURCE_3D && !pTexInfo->Flags.Info.Linear) ||
           (pTexInfo->Flags.Gpu.S3dDx && pGmmGlobalContext->GetSkuTable().FtrDisplayEngineS3d))
        {
            Alignment = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        }

        // Calculate the overall Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
        ArrayQPitch = Get2DMipMapTotalHeight(pTexInfo);
        ArrayQPitch = GFX_ALIGN_NP2(ArrayQPitch, Alignment);
	    
	// Color Surf with MSAA Enabled Mutiply 4
        if((pTexInfo->Flags.Info.TiledYs) && (!pGmmGlobalContext->GetSkuTable().FtrTileY) &&
           ((pTexInfo->MSAA.NumSamples == 8) && (pTexInfo->MSAA.NumSamples == 16)) &&
           ((pTexInfo->Flags.Gpu.Depth == 0) && (pTexInfo->Flags.Gpu.SeparateStencil == 0)))
        {
            ArrayQPitch *= 4; /* Aligned height of 4 samples */
        }
	    
        pTexInfo->Alignment.QPitch = ArrayQPitch;

        if(GmmIsCompressed(pTexInfo->Format))
        {
            uint32_t CompressWidth, CompressHeight, CompressDepth;

            GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

            ArrayQPitch /= CompressHeight;

            if((pTexInfo->Type == RESOURCE_3D) && !pTexInfo->Flags.Info.Linear)
            {
                ArrayQPitch = GFX_ALIGN(ArrayQPitch, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
            }
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
        {
            ArrayQPitch /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
        {
            ArrayQPitch /= 16;
        }

        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender =
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock = ArrayQPitch * pTexInfo->Pitch;
    }

    for(i = 0; i <= pTexInfo->MaxLod; i++)
    {
        pTexInfo->OffsetInfo.Texture2DOffsetInfo.Offset[i] = Get2DTexOffsetAddressPerMip(pTexInfo, i);
    }

    GMM_DPF_EXIT;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the aligned block height of the 3D surface on Gen9
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             BlockHeight: Unaligned block height
///             ExpandedArraySize:  adjusted array size for MSAA, cube faces, etc.
///
/// @return     Aligned BlockHeight
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen9TextureCalc::GetAligned3DBlockHeight(GMM_TEXTURE_INFO *pTexInfo,
                                                             uint32_t          BlockHeight,
                                                             uint32_t          ExpandedArraySize)
{
    GMM_DPF_ENTER;

    GMM_UNREFERENCED_PARAMETER(ExpandedArraySize);
    __GMM_ASSERTPTR(pTexInfo, 0);

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    if((pTexInfo->Type == RESOURCE_3D) && !pTexInfo->Flags.Info.Linear)
    {
        BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
    }

    GMM_DPF_EXIT;

    return BlockHeight;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Allocates the 2D mip layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen9TextureCalc::FillTex2D(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   Width, Height, BitsPerPixel;
    uint32_t   HAlign, VAlign, DAlign, CompressHeight, CompressWidth, CompressDepth;
    uint32_t   AlignedWidth, BlockHeight, ExpandedArraySize, Pitch;
    uint8_t    Compress = 0;
    GMM_STATUS Status;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    BitsPerPixel = pTexInfo->BitsPerPixel;
    if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        // Aux Surfaces are 8bpp.
        BitsPerPixel = 8;
    }

    Height = pTexInfo->BaseHeight;
    Width  = GFX_ULONG_CAST(pTexInfo->BaseWidth);

    pTexInfo->MSAA.NumSamples = GFX_MAX(pTexInfo->MSAA.NumSamples, 1);

    if(pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs)
    {
        FindMipTailStartLod(pTexInfo);
    }

    ExpandedArraySize =
    GFX_MAX(pTexInfo->ArraySize, 1) *
    ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1) *             // Cubemaps simply 6-element, 2D arrays.
    ((pTexInfo->Type == RESOURCE_3D) ? pTexInfo->Depth : 1) * // 3D's simply 2D arrays.
    ((pTexInfo->Flags.Gpu.Depth || pTexInfo->Flags.Gpu.SeparateStencil ||
      (pTexInfo->Flags.Info.TiledYs || pTexInfo->Flags.Info.TiledYf)) ? // MSAA Ys samples are NOT stored as array planes.
     1 :
     pTexInfo->MSAA.NumSamples); // MSAA (non-Depth/Stencil) RT samples stored as array planes.

    if(pTexInfo->Flags.Info.TiledYs || pTexInfo->Flags.Info.TiledYf)
    {
        ExpandedArraySize = GFX_CEIL_DIV(ExpandedArraySize, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileDepth);
    }

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
            pTexInfo->Flags.Gpu.ColorSeparation     = false;
            pTexInfo->Flags.Gpu.ColorSeparationRGBX = false;
        }
    }

    HAlign = pTexInfo->Alignment.HAlign;
    VAlign = pTexInfo->Alignment.VAlign;
    DAlign = pTexInfo->Alignment.DAlign;
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    Compress = GmmIsCompressed(pTexInfo->Format);

    /////////////////////////////////
    // Calculate Block Surface Height
    /////////////////////////////////

    if(ExpandedArraySize > 1)
    {
        uint32_t Alignment = VAlign;
        if((pTexInfo->Type == RESOURCE_3D && !pTexInfo->Flags.Info.Linear) ||
           (pTexInfo->Flags.Gpu.S3dDx && pGmmGlobalContext->GetSkuTable().FtrDisplayEngineS3d))
        {
            Alignment = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        }

        // Calculate the overall Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
        BlockHeight = Get2DMipMapTotalHeight(pTexInfo);
        BlockHeight = GFX_ALIGN_NP2(BlockHeight, Alignment);

        // GMM internally uses QPitch as the logical distance between slices, but translates
        // as appropriate to service client queries in GmmResGetQPitch.
        pTexInfo->Alignment.QPitch = BlockHeight;

        if(Compress)
        {
            BlockHeight = GFX_CEIL_DIV(BlockHeight, CompressHeight);

            BlockHeight = GetAligned3DBlockHeight(pTexInfo, BlockHeight, ExpandedArraySize);
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
        {
            BlockHeight /= 2;
        }
        else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
        {
            BlockHeight /= 16;
        }

        BlockHeight *= ExpandedArraySize;
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
    if((pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs) &&
       (pTexInfo->Alignment.MipTailStartLod < 2))
    {
        // Do nothing -- all mips are in LOD0/LOD1, which is already width aligned.
    }
    else if(pTexInfo->MaxLod >= 2)
    {
        uint32_t AlignedWidthLod1, AlignedWidthLod2;

        AlignedWidthLod1 = __GMM_EXPAND_WIDTH(this, Width >> 1, HAlign, pTexInfo);
        AlignedWidthLod2 = __GMM_EXPAND_WIDTH(this, Width >> 2, HAlign, pTexInfo);

        AlignedWidth = GFX_MAX(AlignedWidth, AlignedWidthLod1 + AlignedWidthLod2);
    }

    if(Compress)
    {
        AlignedWidth = GFX_CEIL_DIV(AlignedWidth, CompressWidth);
    }
    else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
    {
        AlignedWidth *= 2;
    }
    else if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
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
    }

    GMM_DPF_EXIT;

    return (Status);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip offset of given LOD in Mip Tail
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: given LOD #
///
/// @return     offset value of LOD in bytes
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen9TextureCalc::GetMipTailByteOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                          uint32_t          MipLevel)
{
    uint32_t ByteOffset = 0, Slot;

    GMM_DPF_ENTER;

    if((pTexInfo->Type == RESOURCE_1D) || (pTexInfo->Type == RESOURCE_3D))
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
               (pTexInfo->Flags.Info.TiledYf ? 4 : 0);

        switch(Slot)
        {
            case 0:
                ByteOffset = GMM_KBYTE(32);
                break;
            case 1:
                ByteOffset = GMM_KBYTE(16);
                break;
            case 2:
                ByteOffset = GMM_KBYTE(8);
                break;
            case 3:
                ByteOffset = GMM_KBYTE(4);
                break;
            case 4:
                ByteOffset = GMM_KBYTE(2);
                break;
            case 5:
                ByteOffset = GMM_KBYTE(1);
                break;
            case 6:
                ByteOffset = GMM_BYTES(768);
                break;
            case 7:
                ByteOffset = GMM_BYTES(512);
                break;
            case 8:
                ByteOffset = GMM_BYTES(448);
                break;
            case 9:
                ByteOffset = GMM_BYTES(384);
                break;
            case 10:
                ByteOffset = GMM_BYTES(320);
                break;
            case 11:
                ByteOffset = GMM_BYTES(256);
                break;
            case 12:
                ByteOffset = GMM_BYTES(192);
                break;
            case 13:
                ByteOffset = GMM_BYTES(128);
                break;
            case 14:
                ByteOffset = GMM_BYTES(64);
                break;
            case 15:
                ByteOffset = GMM_BYTES(0);
                break;
            default:
                __GMM_ASSERT(0);
        }
    }
    else if(pTexInfo->Type == RESOURCE_2D || pTexInfo->Type == RESOURCE_CUBE)
    {
        // clang-format off
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
                    // TileYs
                   ((pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples == 16) ? 4 :
                    (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples ==  8) ? 3 :
                    (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples ==  4) ? 2 :
                    (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples ==  2) ? 1 :
                    (pTexInfo->Flags.Info.TiledYs                                   ) ? 0 :
                    // TileYf
                    (pTexInfo->Flags.Info.TiledYf                                   ) ?  4: 0);
        // clang-format on

        switch(Slot)
        {
            case 0:
                ByteOffset = GMM_KBYTE(32);
                break;
            case 1:
                ByteOffset = GMM_KBYTE(16);
                break;
            case 2:
                ByteOffset = GMM_KBYTE(8);
                break;
            case 3:
                ByteOffset = GMM_KBYTE(4);
                break;
            case 4:
                ByteOffset = GMM_KBYTE(2);
                break;
            case 5:
                ByteOffset = GMM_BYTES(1536);
                break;
            case 6:
                ByteOffset = GMM_BYTES(1280);
                break;
            case 7:
                ByteOffset = GMM_BYTES(1024);
                break;
            case 8:
                ByteOffset = GMM_BYTES(768);
                break;
            case 9:
                ByteOffset = GMM_BYTES(512);
                break;
            case 10:
                ByteOffset = GMM_BYTES(256);
                break;
            case 11:
                ByteOffset = GMM_BYTES(192);
                break;
            case 12:
                ByteOffset = GMM_BYTES(128);
                break;
            case 13:
                ByteOffset = GMM_BYTES(64);
                break;
            case 14:
                ByteOffset = GMM_BYTES(0);
                break;
            default:
                __GMM_ASSERT(0);
        }
    }

    GMM_DPF_EXIT;

    return (ByteOffset);
}

GMM_MIPTAIL_SLOT_OFFSET Gen9MipTailSlotOffset1DSurface[16][5] = GEN9_MIPTAIL_SLOT_OFFSET_1D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET Gen9MipTailSlotOffset2DSurface[15][5] = GEN9_MIPTAIL_SLOT_OFFSET_2D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET Gen9MipTailSlotOffset3DSurface[16][5] = GEN9_MIPTAIL_SLOT_OFFSET_3D_SURFACE;
/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip-map offset in geometric OffsetX, Y, Z
//  for a given LOD in Mip Tail.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: mip-map level
///             OffsetX: ptr to Offset in X direction (in bytes)
///             OffsetY: ptr to Offset in Y direction (in pixels)
///             OffsetZ: ptr to Offset in Z direction (in pixels)
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen9TextureCalc::GetMipTailGeometryOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                          uint32_t          MipLevel,
                                                          uint32_t *        OffsetX,
                                                          uint32_t *        OffsetY,
                                                          uint32_t *        OffsetZ)
{
    uint32_t ArrayIndex = 0;
    uint32_t Slot       = 0;

    GMM_DPF_ENTER;

    switch(pTexInfo->BitsPerPixel)
    {
        case 128:
            ArrayIndex = 0;
            break;
        case 64:
            ArrayIndex = 1;
            break;
        case 32:
            ArrayIndex = 2;
            break;
        case 16:
            ArrayIndex = 3;
            break;
        case 8:
            ArrayIndex = 4;
            break;
        default:
            __GMM_ASSERT(0);
            break;
    }

    if(pTexInfo->Type == RESOURCE_1D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
               (pTexInfo->Flags.Info.TiledYf ? 4 : 0);

        *OffsetX = Gen9MipTailSlotOffset1DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen9MipTailSlotOffset1DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen9MipTailSlotOffset1DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_2D || pTexInfo->Type == RESOURCE_CUBE)
    {
        // clang-format off
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
                    // TileYs
                   ((pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples == 16) ? 4 :
                    (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples ==  8) ? 3 :
                    (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples ==  4) ? 2 :
                    (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples ==  2) ? 1 :
                    (pTexInfo->Flags.Info.TiledYs                                   ) ? 0 :
                    // TileYf
                    (pTexInfo->Flags.Info.TiledYf                                   ) ?  4: 0);
        // clang-format on

        *OffsetX = Gen9MipTailSlotOffset2DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen9MipTailSlotOffset2DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen9MipTailSlotOffset2DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_3D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
               (pTexInfo->Flags.Info.TiledYf ? 4 : 0);

        *OffsetX = Gen9MipTailSlotOffset3DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen9MipTailSlotOffset3DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen9MipTailSlotOffset3DSurface[Slot][ArrayIndex].Z;
    }

    GMM_DPF_EXIT;
    return;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Allocates the 3D mip layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen9TextureCalc::FillTex3D(GMM_TEXTURE_INFO * pTexInfo,
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
GMM_STATUS GMM_STDCALL GmmLib::GmmGen9TextureCalc::FillTexCube(GMM_TEXTURE_INFO * pTexInfo,
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
GMM_STATUS GMM_STDCALL GmmLib::GmmGen9TextureCalc::MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS Status = GMM_SUCCESS;

    if(pTexInfo->MSAA.NumSamples > 1) // CCS for MSAA Compression
    {
        Status = MSAACompression(pTexInfo);
    }
    else // Non-MSAA CCS Use (i.e. Render Target Fast Clear)
    {
        if(!pTexInfo->Flags.Info.TiledW &&
           (!pTexInfo->Flags.Info.TiledX) &&
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
