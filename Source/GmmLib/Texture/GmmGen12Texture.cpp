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


#include "Internal/Common/GmmLibInc.h"
#include "Internal/Common/Texture/GmmGen10TextureCalc.h"
#include "Internal/Common/Texture/GmmGen11TextureCalc.h"
#include "Internal/Common/Texture/GmmGen12TextureCalc.h"

GMM_MIPTAIL_SLOT_OFFSET MipTailSlotOffset1DSurface[15][5] = GEN11_MIPTAIL_SLOT_OFFSET_1D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET MipTailSlotOffset2DSurface[15][5] = GEN11_MIPTAIL_SLOT_OFFSET_2D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET MipTailSlotOffset3DSurface[15][5] = GEN11_MIPTAIL_SLOT_OFFSET_3D_SURFACE;

/////////////////////////////////////////////////////////////////////////////////////
/// Calculates height of the 2D mip layout on Gen9
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///
/// @return     height of 2D mip layout
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen12TextureCalc::Get2DMipMapHeight(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t BlockHeight, MipHeight;
    uint32_t HeightLinesLevel0, HeightLinesLevel1, HeightLinesLevel2;
    uint32_t i, MipLevel, VAlign, CompressHeight, CompressWidth, CompressDepth;
    uint8_t  Compressed;
    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    Compressed = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);
    MipHeight  = pTexInfo->BaseHeight;
    MipLevel   = pTexInfo->MaxLod;
    VAlign     = pTexInfo->Alignment.VAlign;
    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    HeightLinesLevel0 = __GMM_EXPAND_HEIGHT(this, MipHeight, VAlign, pTexInfo);

    if(Compressed)
    {
        HeightLinesLevel0 /= CompressHeight;
    }

    // Mip0 height...
    BlockHeight = HeightLinesLevel0;

    if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
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

            if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
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
        if(!(pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)))
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
/// Calculates Linear CCS size from main surface size
///
/// @param[in]  pSurf: ptr to ::GMM_TEXTURE_INFO of main surface
/// @param[in]  pAuxTexInfo: ptr to ::GMM_TEXTURE_INFO of Aux surface
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmGen12TextureCalc::FillTexCCS(GMM_TEXTURE_INFO *pSurf,
                                                   GMM_TEXTURE_INFO *pAuxTexInfo)
{
    if(pGmmLibContext->GetSkuTable().FtrFlatPhysCCS && !pSurf->Flags.Gpu.ProceduralTexture)
    {
        //No CCS allocation for lossless compression (exclude AMFS CCS).
        return GMM_SUCCESS;
    }    
    else if(pAuxTexInfo->Flags.Gpu.__NonMsaaLinearCCS)
    {
        GMM_TEXTURE_INFO         Surf      = *pSurf;
        uint32_t                 Depth;	
        const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pSurf, pGmmLibContext);
        pAuxTexInfo->Flags.Info.TiledW     = 0;
        pAuxTexInfo->Flags.Info.TiledYf    = 0;
        pAuxTexInfo->Flags.Info.TiledX     = 0;
        pAuxTexInfo->Flags.Info.Linear     = 1;
        GMM_SET_64KB_TILE(pAuxTexInfo->Flags, 0, pGmmLibContext);
        GMM_SET_4KB_TILE(pAuxTexInfo->Flags, 0, pGmmLibContext);

        pAuxTexInfo->ArraySize    = Surf.ArraySize;
        pAuxTexInfo->Alignment    = {0};	
        pAuxTexInfo->BitsPerPixel = 8;
        Depth                     = (Surf.Depth > 0) ? Surf.Depth : 1; // Depth = 0 needs to be handled gracefully
        uint32_t ExpandedArraySize =
        GFX_MAX(Surf.ArraySize, 1) *
        ((Surf.Type == RESOURCE_CUBE) ? 6 : 1) *        // Cubemaps simply 6-element, 2D arrays.
        ((Surf.Type == RESOURCE_3D) ? Depth : 1) * // 3D's simply 2D arrays for sizing.
        ((Surf.Flags.Gpu.Depth || Surf.Flags.Gpu.SeparateStencil ||
          GMM_IS_64KB_TILE(Surf.Flags) || Surf.Flags.Info.TiledYf) ?
         1 :
         Surf.MSAA.NumSamples) *                                                                                         // MSAA (non-Depth/Stencil) RT samples stored as array planes.
        ((GMM_IS_64KB_TILE(Surf.Flags) && !pGmmLibContext->GetSkuTable().FtrTileY && !pGmmLibContext->GetSkuTable().FtrXe2PlusTiling && (Surf.MSAA.NumSamples == 16)) ? 4 : // MSAA x8/x16 stored as pseudo array planes each with 4x samples
         (GMM_IS_64KB_TILE(Surf.Flags) && !pGmmLibContext->GetSkuTable().FtrTileY && !pGmmLibContext->GetSkuTable().FtrXe2PlusTiling && (Surf.MSAA.NumSamples == 8)) ? 2 :
                                                                                                                                                                       1);

        if(GMM_IS_64KB_TILE(Surf.Flags) || Surf.Flags.Info.TiledYf)
        {
            ExpandedArraySize = GFX_ALIGN(ExpandedArraySize, pPlatform->TileInfo[Surf.TileMode].LogicalTileDepth);
        }

        if(GmmIsUVPacked(Surf.Format))
        {
            uint64_t YCcsSize = GFX_ALIGN((Surf.OffsetInfo.Plane.Y[GMM_PLANE_U] * Surf.Pitch), GMM_KBYTE(16)) >> 8;
            YCcsSize          = GFX_ALIGN(YCcsSize, PAGE_SIZE);

            uint64_t PlanarSize = (Surf.ArraySize > 1) ? (Surf.OffsetInfo.Plane.ArrayQPitch) : Surf.Size;

            uint64_t UVCcsSize = GFX_ALIGN(PlanarSize - (Surf.OffsetInfo.Plane.Y[GMM_PLANE_U] * Surf.Pitch), GMM_KBYTE(16)) >> 8;
            if(UVCcsSize == 0)
            {
                //GMM_ASSERTDPF(UVCcsSize != 0, "Incorrect Planar Surface Size"); //Redescription of Yf/Ys planar surface P010 hits it (debug required?)
                UVCcsSize = 1;
            }
            UVCcsSize = GFX_ALIGN_NP2(UVCcsSize, PAGE_SIZE);

            pAuxTexInfo->OffsetInfo.Plane.X[GMM_PLANE_Y] = 0;
            pAuxTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y] = 0;
            pAuxTexInfo->OffsetInfo.Plane.X[GMM_PLANE_U] = YCcsSize; //Being Linear CCS, fill X-offset - Test GetAuxOffset UV_CCS is proper
            pAuxTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] = 0;
            pAuxTexInfo->OffsetInfo.Plane.X[GMM_PLANE_V] = YCcsSize; //Being Linear CCS, fill X-offset
            pAuxTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V] = 0;

            pAuxTexInfo->OffsetInfo.Plane.ArrayQPitch = YCcsSize + UVCcsSize;
            pAuxTexInfo->Size                         = pAuxTexInfo->OffsetInfo.Plane.ArrayQPitch * ((Surf.ArraySize > 1) ? (Surf.ArraySize) : 1);
            pAuxTexInfo->Alignment.QPitch = GFX_ULONG_CAST(pAuxTexInfo->Size) / ExpandedArraySize;
        }
        else if(GmmIsPlanar(Surf.Format))
        {
            //Doesn't require separate Aux surfaces since not displayable. Page-alignment ensures
            //each hybrid plane is 4k-aligned, hence gets unique AuxT.L1e
            uint64_t PlanarSize = (Surf.ArraySize > 1) ? (Surf.OffsetInfo.Plane.ArrayQPitch) : Surf.Size;
            uint64_t CcsSize    = GFX_ALIGN(PlanarSize, GMM_KBYTE(16)) >> 8;
            CcsSize             = GFX_ALIGN(CcsSize, PAGE_SIZE);

            pAuxTexInfo->OffsetInfo.Plane.X[GMM_PLANE_Y] = 0;
            pAuxTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y] = 0;
            pAuxTexInfo->OffsetInfo.Plane.X[GMM_PLANE_U] = GFX_ALIGN(Surf.OffsetInfo.Plane.Y[GMM_PLANE_U] * Surf.Pitch, GMM_KBYTE(16)) >> 8; //Being Linear CCS, fill X-offset - Test GetAuxOffset UV_CCS is proper
            pAuxTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] = 0;
            pAuxTexInfo->OffsetInfo.Plane.X[GMM_PLANE_V] = GFX_ALIGN(Surf.OffsetInfo.Plane.Y[GMM_PLANE_V] * Surf.Pitch, GMM_KBYTE(16)) >> 8; //Being Linear CCS, fill X-offset
            pAuxTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V] = 0;

            pAuxTexInfo->OffsetInfo.Plane.ArrayQPitch = CcsSize;
            pAuxTexInfo->Size                         = pAuxTexInfo->OffsetInfo.Plane.ArrayQPitch *
                                ((Surf.ArraySize > 1) ? (Surf.ArraySize) : 1);
            pAuxTexInfo->Alignment.QPitch = GFX_ULONG_CAST(pAuxTexInfo->Size) / ExpandedArraySize;
        }
        else
        {
       	    pAuxTexInfo->Size = (GFX_ALIGN(Surf.Size, GMM_KBYTE(16)) >> 8);

            uint32_t qPitch;
            if(ExpandedArraySize > 1)
            {
                uint64_t sliceSize = ((GFX_ALIGN(Surf.Pitch * Surf.Alignment.QPitch, GMM_KBYTE(16)) >> 8));
                qPitch             = GFX_ULONG_CAST(sliceSize); //HW doesn't use QPitch for Aux except MCS, how'd AMFS get sw-filled non-zero QPitch?
               

                if(Surf.MSAA.NumSamples && !pGmmLibContext->GetSkuTable().FtrTileY)
                {
                    //MSAA Qpitch is sample-distance, multiply NumSamples in a tile
                    qPitch *= (pGmmLibContext->GetSkuTable().FtrXe2PlusTiling ? Surf.MSAA.NumSamples : GFX_MIN(Surf.MSAA.NumSamples, 4));
                }
            }
            else
            {
                qPitch = GFX_ULONG_CAST(pAuxTexInfo->Size);
            }
            
            pAuxTexInfo->Alignment.QPitch = qPitch;
        }
        __GMM_ASSERT(ExpandedArraySize || (pAuxTexInfo->Size == 0));
        pAuxTexInfo->Pitch                   = 0;
        pAuxTexInfo->Type                    = RESOURCE_BUFFER;
        pAuxTexInfo->Alignment.BaseAlignment = GMM_KBYTE(4);                            //TODO: TiledResource?
        pAuxTexInfo->Size                    = GFX_ALIGN(pAuxTexInfo->Size, PAGE_SIZE); //page-align final size

        if(pAuxTexInfo->Flags.Gpu.TiledResource)
        {
            pAuxTexInfo->Alignment.BaseAlignment = GMM_KBYTE(64);                               //TODO: TiledResource?
            pAuxTexInfo->Size                    = GFX_ALIGN(pAuxTexInfo->Size, GMM_KBYTE(64)); //page-align final size
        }

        //Clear compression request in CCS
        pAuxTexInfo->Flags.Info.RenderCompressed = 0;
        pAuxTexInfo->Flags.Info.MediaCompressed  = 0;
        pAuxTexInfo->Flags.Info.NotCompressed    = 1;
        pAuxTexInfo->Flags.Info.RedecribedPlanes = 0;
        SetTileMode(pAuxTexInfo);

        return GMM_SUCCESS;
    }
    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Allocates the 2D mip layout for surface state programming.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: ptr to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen12TextureCalc::FillTex2D(GMM_TEXTURE_INFO * pTexInfo,
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

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    BitsPerPixel = pTexInfo->BitsPerPixel;
    //TODO: Deprecate TileY usage
    if((pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs))
    {
        // Aux Surfaces are 8bpp.
        BitsPerPixel = 8;
    }

    Height = pTexInfo->BaseHeight;
    Width  = GFX_ULONG_CAST(pTexInfo->BaseWidth);

    if((pTexInfo->Format == GMM_FORMAT_R8G8B8_UINT) && (pTexInfo->Flags.Info.Linear || pTexInfo->Flags.Info.TiledX))
    {
        Width += GFX_CEIL_DIV(Width, 63);
    }

    pTexInfo->MSAA.NumSamples = GFX_MAX(pTexInfo->MSAA.NumSamples, 1);

    if(pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags))
    {
        FindMipTailStartLod(pTexInfo);
    }

    ExpandedArraySize =
    GFX_MAX(pTexInfo->ArraySize, 1) *
    ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1) *             // Cubemaps simply 6-element, 2D arrays.
    ((pTexInfo->Type == RESOURCE_3D) ? pTexInfo->Depth : 1) * // 3D's simply 2D arrays for sizing.
    ((pTexInfo->Flags.Gpu.Depth || pTexInfo->Flags.Gpu.SeparateStencil ||
      (GMM_IS_64KB_TILE(pTexInfo->Flags) || pTexInfo->Flags.Info.TiledYf)) ? // MSAA Ys/Yf samples are ALSO stored as array planes, calculate size for single sample and expand it later.
     1 :
     pTexInfo->MSAA.NumSamples) *                                                                                              // MSAA (non-Depth/Stencil) RT samples stored as array planes.
    ((pTexInfo->Flags.Gpu.Depth || pTexInfo->Flags.Gpu.SeparateStencil) ? // Depth/Stencil MSAA surface is expanded through Width and Depth
     1 :
     ((GMM_IS_64KB_TILE(pTexInfo->Flags) && !pGmmLibContext->GetSkuTable().FtrTileY && !pGmmLibContext->GetSkuTable().FtrXe2PlusTiling && (pTexInfo->MSAA.NumSamples == 16)) ? 4 : // MSAA x8/x16 stored as pseudo array planes each with 4x samples
      (GMM_IS_64KB_TILE(pTexInfo->Flags) && !pGmmLibContext->GetSkuTable().FtrTileY && !pGmmLibContext->GetSkuTable().FtrXe2PlusTiling && (pTexInfo->MSAA.NumSamples == 8)) ? 2 :
                                                                                                                                                                              1));
    if(GMM_IS_64KB_TILE(pTexInfo->Flags) || pTexInfo->Flags.Info.TiledYf)
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
            pTexInfo->Flags.Gpu.ColorSeparation     = 0;
            pTexInfo->Flags.Gpu.ColorSeparationRGBX = 0;
        }
    }

    HAlign = pTexInfo->Alignment.HAlign;
    VAlign = pTexInfo->Alignment.VAlign;
    DAlign = pTexInfo->Alignment.DAlign;

    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    Compress = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);

    /////////////////////////////////
    // Calculate Block Surface Height
    /////////////////////////////////

    if(ExpandedArraySize > 1)
    {
        uint32_t Alignment = VAlign;
        if((pTexInfo->Type == RESOURCE_3D && !pTexInfo->Flags.Info.Linear) ||
           (pTexInfo->Flags.Gpu.S3dDx && pGmmLibContext->GetSkuTable().FtrDisplayEngineS3d) ||
	   (pTexInfo->Flags.Wa.MediaPipeUsage))
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
        else
        {
            BlockHeight = ScaleTextureHeight(pTexInfo, BlockHeight);
        }

        BlockHeight *= ExpandedArraySize;
    }
    else
    {
        pTexInfo->Alignment.QPitch = 0;

        BlockHeight = Get2DMipMapHeight(pTexInfo);
        BlockHeight = ScaleTextureHeight(pTexInfo, BlockHeight);
    }

    ///////////////////////////////////
    // Calculate Pitch
    ///////////////////////////////////

    AlignedWidth = __GMM_EXPAND_WIDTH(this, Width, HAlign, pTexInfo);

    // Calculate special pitch case of small dimensions where LOD1 + LOD2 widths
    // are greater than LOD0. e.g. dimensions 4x4 and MinPitch == 1
    if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
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
    else
    {
        AlignedWidth = ScaleTextureWidth(pTexInfo, AlignedWidth);
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

    // For Non-planar surfaces, the alignment is done on the entire height of the allocation
    if(pGmmLibContext->GetWaTable().WaAlignYUVResourceToLCU &&
       GmmIsYUVFormatLCUAligned(pTexInfo->Format) &&
       !GmmIsPlanar(pTexInfo->Format))
    {
        BlockHeight = GFX_ALIGN(BlockHeight, GMM_SCANLINES(GMM_MAX_LCU_SIZE));
    }

    // Align height to even row to avoid hang if HW over-fetch
    BlockHeight = GFX_ALIGN(BlockHeight, __GMM_EVEN_ROW);

    // slice pitch padding to 64KB
    if (pTexInfo->Flags.Wa.SlicePitchPadding64KB && (ExpandedArraySize > 1))
    {
	BlockHeight                = GFX_CEIL_DIV(GFX_ALIGN_NP2(BlockHeight * Pitch, GMM_KBYTE(64)), Pitch);
        pTexInfo->Alignment.QPitch = BlockHeight;
    }

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
/// This function will Setup a planar surface allocation.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions.
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen12TextureCalc::FillTexPlanar(GMM_TEXTURE_INFO * pTexInfo,
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

                Height                                = YHeight + 2 * VHeight; // One VHeight for V and one for U.
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;
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

                Height                                = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;
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

                Height                                = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;
                break;
            }
        case GMM_FORMAT_BGRP:
        case GMM_FORMAT_RGBP:
        {
            //For RGBP linear Tile keep resource Offset non aligned and for other Tile format to be 16-bit aligned
            if(pTexInfo->Flags.Info.Linear)
            {
                VHeight = YHeight;

                Height                                = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;
            }
            else
            {
                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                VHeight = YHeight;

                Height                                = YHeight + 2 * VHeight;
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;
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
            UVPacked                              = true;
            pTexInfo->OffsetInfo.Plane.NoOfPlanes = 2;
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
                WidthBytesPhysical                    = GFX_ALIGN(WidthBytesPhysical, 2); // If odd YWidth, pitch bumps-up to fit rounded-up U/V planes.
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 2;
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
            pTexInfo->Flags.Info.TiledYf          = 0;
            pTexInfo->Flags.Info.TiledX           = 0;
            pTexInfo->Flags.Info.Linear           = 1;
            pTexInfo->OffsetInfo.Plane.NoOfPlanes = 1;
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

        pTexInfo->OffsetInfo.Plane.IsTileAlignedPlanes = true;

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
            pTexInfo->OffsetInfo.Plane.ArrayQPitch = ElementSizeBytes;
            pTexInfo->Size                         = LargeSize;
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

GMM_STATUS GMM_STDCALL GmmLib::GmmGen12TextureCalc::GetCCSScaleFactor(GMM_TEXTURE_INFO *pTexInfo,
                                                                      CCS_UNIT &        ScaleFactor)
{
    GMM_STATUS           Status     = GMM_SUCCESS;
    GMM_TEXTURE_ALIGN_EX TexAlignEx = static_cast<PlatformInfoGen12 *>(pGmmLibContext->GetPlatformInfoObj())->GetExTextureAlign();
    uint32_t             CCSModeIdx = 0;

    if(pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) //pTexInfo is RT Surf
    {
        CCSModeIdx = CCS_MODE(pTexInfo->TileMode);
        __GMM_ASSERT(pTexInfo->TileMode < GMM_TILE_MODES);
    }
    else //pTexInfo is CCS Surf
    {
        CCSModeIdx = pTexInfo->CCSModeAlign;
    }

    if(!(CCSModeIdx < CCS_MODES))
    {
        __GMM_ASSERT(0); //indicates something wrong w/ H/V/D Align Filling function or Wrong TileMode set
        return GMM_ERROR;
    }

    ScaleFactor = TexAlignEx.CCSEx[CCSModeIdx];

    return (Status);
}

GMM_STATUS GMM_STDCALL GmmLib::GmmGen12TextureCalc::GetCCSExMode(GMM_TEXTURE_INFO *AuxSurf)
{
    if(GMM_IS_4KB_TILE(AuxSurf->Flags) || GMM_IS_64KB_TILE(AuxSurf->Flags) || AuxSurf->Flags.Info.Linear)
    {
        if(pGmmLibContext->GetSkuTable().FtrLinearCCS)
 	{
            AuxSurf->Flags.Gpu.__NonMsaaLinearCCS = 1;
        }
        else
        {
            AuxSurf->Flags.Gpu.__NonMsaaTileYCcs = 1;
            //CCS is always 2D, even for 3D surface
            if(AuxSurf->Type == RESOURCE_CUBE)
            {
                AuxSurf->ArraySize = 6;
            }
            AuxSurf->Type = RESOURCE_2D;
        }
        if(AuxSurf->Flags.Gpu.__NonMsaaTileYCcs)
        {
            AuxSurf->CCSModeAlign = 0;
            SetTileMode(AuxSurf);
            /*if (AuxSurf->Flags.Gpu.UnifiedAuxSurface)*/
            {
                AuxSurf->CCSModeAlign = CCS_MODE(AuxSurf->TileMode);
            }
            AuxSurf->TileMode = TILE_NONE;

            __GMM_ASSERT(AuxSurf->CCSModeAlign < CCS_MODES);
            return (AuxSurf->CCSModeAlign < CCS_MODES) ? GMM_SUCCESS : GMM_INVALIDPARAM;
        }
    }
    return GMM_SUCCESS;
}

uint32_t GMM_STDCALL GmmLib::GmmGen12TextureCalc::ScaleTextureHeight(GMM_TEXTURE_INFO *pTexInfo, uint32_t Height)
{
    uint32_t ScaledHeight = Height;
    if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        CCS_UNIT ScaleFactor;
        GetCCSScaleFactor(pTexInfo, ScaleFactor);

        ScaledHeight /= ScaleFactor.Downscale.Height;
    }

    return ScaledHeight;
}

uint32_t GMM_STDCALL GmmLib::GmmGen12TextureCalc::ScaleTextureWidth(GMM_TEXTURE_INFO *pTexInfo, uint32_t Width)
{
    uint32_t ScaledWidth = Width;

    if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        CCS_UNIT ScaleFactor;
        GetCCSScaleFactor(pTexInfo, ScaleFactor);


        if(ScaleFactor.Downscale.Width < 0)
        {
            ScaledWidth *= ((-1) * ScaleFactor.Downscale.Width);
        }
        else
        {
            ScaledWidth /= ScaleFactor.Downscale.Width;
        }
    }
    else if(pTexInfo->Flags.Gpu.ColorSeparation)
    {
        ScaledWidth *= pTexInfo->ArraySize;
        __GMM_ASSERT(0 == (ScaledWidth % GMM_COLOR_SEPARATION_WIDTH_DIVISION));
        ScaledWidth /= GMM_COLOR_SEPARATION_WIDTH_DIVISION;
    }
    else if(pTexInfo->Flags.Gpu.ColorSeparationRGBX)
    {
        ScaledWidth *= pTexInfo->ArraySize;
        __GMM_ASSERT(0 == (ScaledWidth % GMM_COLOR_SEPARATION_RGBX_WIDTH_DIVISION));
        ScaledWidth /= GMM_COLOR_SEPARATION_RGBX_WIDTH_DIVISION;
    }

    return ScaledWidth;
}

uint32_t GMM_STDCALL GmmLib::GmmGen12TextureCalc::ScaleFCRectHeight(GMM_TEXTURE_INFO *pTexInfo, uint32_t Height)
{
    uint32_t ScaledHeight = Height;
    if(pTexInfo->Flags.Gpu.CCS)
    {
        CCS_UNIT *FCRectAlign = static_cast<PlatformInfoGen12 *>(pGmmLibContext->GetPlatformInfoObj())->GetFCRectAlign();
	uint8_t   index       = FCMaxModes;
        if((index = FCMode(pTexInfo->TileMode, pTexInfo->BitsPerPixel)) < FCMaxModes)
        {
            ScaledHeight = GFX_ALIGN(ScaledHeight, FCRectAlign[index].Align.Height);
            ScaledHeight /= FCRectAlign[index].Downscale.Height;
        }
        else
        {
            __GMM_ASSERT(0);
        }
    }

    return ScaledHeight;
}

uint64_t GMM_STDCALL GmmLib::GmmGen12TextureCalc::ScaleFCRectWidth(GMM_TEXTURE_INFO *pTexInfo, uint64_t Width)
{
    uint64_t ScaledWidth = Width;
    if(pTexInfo->Flags.Gpu.CCS)
    {
        CCS_UNIT *FCRectAlign = static_cast<PlatformInfoGen12 *>(pGmmLibContext->GetPlatformInfoObj())->GetFCRectAlign();	    
        uint8_t   index       = FCMaxModes;
        if((index = FCMode(pTexInfo->TileMode, pTexInfo->BitsPerPixel)) < FCMaxModes)
        {
            ScaledWidth = GFX_ALIGN(ScaledWidth, FCRectAlign[index].Align.Width);
            ScaledWidth /= FCRectAlign[index].Downscale.Width;
        }
        else
        {
            //Unsupported tiling-type for FastClear
            __GMM_ASSERT(0);
        }
    }

    return ScaledWidth;
}


uint64_t GMM_STDCALL GmmLib::GmmGen12TextureCalc::Get2DFCSurfaceWidthFor3DSurface(GMM_TEXTURE_INFO *pTexInfo,
                                                                                  uint64_t          Width)
{
    uint64_t Width2D = Width;
    if (pTexInfo->Flags.Gpu.CCS)
    {
        CCS_UNIT *FCRectAlign = static_cast<PlatformInfoGen12 *>(pGmmLibContext->GetPlatformInfoObj())->GetFCRectAlign();
        uint8_t   index       = FCMaxModes;
        if ((index = FCMode(pTexInfo->TileMode, pTexInfo->BitsPerPixel)) < FCMaxModes)
        {
            Width2D = GFX_ALIGN(Width2D, FCRectAlign[index].Align.Width);
            Width2D *= FCRectAlign[index].Downscale.Width;
        }
        else
        {
            
            __GMM_ASSERT(0);
        }
    }
    return Width2D;
}
uint64_t GMM_STDCALL GmmLib::GmmGen12TextureCalc::Get2DFCSurfaceHeightFor3DSurface(GMM_TEXTURE_INFO *pTexInfo,
                                                                                   uint32_t          Height,
                                                                                   uint32_t          Depth)
{
    uint64_t Height2D = Height;
    uint32_t Depth3D  = Depth;

    if (pTexInfo->Flags.Gpu.CCS && (Depth > 1))
    {
        CCS_UNIT *FCRectAlign = static_cast<PlatformInfoGen12 *>(pGmmLibContext->GetPlatformInfoObj())->GetFCRectAlign();
        uint8_t   index       = FCMaxModes;
        if ((index = FCMode(pTexInfo->TileMode, pTexInfo->BitsPerPixel)) < FCMaxModes)
        {
            Height2D = GFX_ALIGN(Height2D, FCRectAlign[index].Align.Height);
            Height2D *= FCRectAlign[index].Downscale.Height;
            Depth3D = GFX_ALIGN(Depth3D, FCRectAlign[index].Align.Depth) / FCRectAlign[index].Align.Depth;
            Height2D *= Depth3D;
        }
        else
        {
            
            __GMM_ASSERT(0);
        }
    }
    return Height2D;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function does any special-case conversion from client-provided pseudo creation
/// parameters to actual parameters for CCS.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen12TextureCalc::MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS Status = GMM_SUCCESS;

    if(pTexInfo->MSAA.NumSamples > 1 && (pTexInfo->Flags.Gpu.MCS)) // CCS for MSAA Compression
    {
        Status = MSAACompression(pTexInfo);
    }
    else // Non-MSAA CCS Use (i.e. Render Target Fast Clear)
    {
        if(!pTexInfo->Flags.Info.TiledW &&
           (!pTexInfo->Flags.Info.TiledX) &&
           ((GMM_IS_4KB_TILE(pTexInfo->Flags) || GMM_IS_64KB_TILE(pTexInfo->Flags) ||
             (pTexInfo->Type == RESOURCE_BUFFER && pTexInfo->Flags.Info.Linear)))) //!Yf - deprecate Yf)
        {
            // For non-MSAA CCS usage, the Doc has four tables of
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
            // (!) For all the above, the doc has separate entries for
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

            GetCCSExMode(pTexInfo);
        }
        else
        {
            GMM_ASSERTDPF(0, "Illegal CCS creation parameters!");
            Status = GMM_ERROR;
        }
    }
    return Status;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip offset of given LOD in Mip Tail
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: mip-map level
///
/// @return     offset value of LOD in bytes
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen12TextureCalc::GetMipTailByteOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                           uint32_t          MipLevel)
{
    uint32_t ByteOffset = 0, Slot = 0xff;

    GMM_DPF_ENTER;

    if(pGmmLibContext->GetSkuTable().FtrTileY)
    {
        return GmmGen11TextureCalc::GetMipTailByteOffset(pTexInfo, MipLevel);
    }
    // 3D textures follow the Gen10 mip tail format
    if(!pGmmLibContext->GetSkuTable().FtrStandardMipTailFormat)
    {
        return GmmGen9TextureCalc::GetMipTailByteOffset(pTexInfo, MipLevel);
    }

    // Til64 is the only format which supports MipTail on FtrTileY disabled platforms
    __GMM_ASSERT(pTexInfo->Flags.Info.Tile64);
    // Mipped MSAA is not supported for Tile64
    __GMM_ASSERT(pTexInfo->MSAA.NumSamples <= 1);

    if((pTexInfo->Type == RESOURCE_1D) || (pTexInfo->Type == RESOURCE_3D) || (pTexInfo->Type == RESOURCE_2D || pTexInfo->Type == RESOURCE_CUBE))
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod;
    }

    // Miptail Slot layout in Tile64: as per specifications
    // Byteoffset varies based on bpp for tile64 format, so any caller who needs to use byteoffset needs to call cpuswizzle with corresponding geomteric offsets
    // Returning ByteOffset as 0 for Tile64 always

    // GMM_DPF_CRITICAL("Miptail byte offset requested for Tile64 \r\n");
    GMM_DPF_EXIT;

    // return ByteOffset=0, i.e return start of miptail for any address within packed miptail
    return (ByteOffset);
}

void GmmLib::GmmGen12TextureCalc::GetMipTailGeometryOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                           uint32_t          MipLevel,
                                                           uint32_t *        OffsetX,
                                                           uint32_t *        OffsetY,
                                                           uint32_t *        OffsetZ)
{
    uint32_t ArrayIndex = 0;
    uint32_t Slot       = 0;

    GMM_DPF_ENTER;

    if(pGmmLibContext->GetSkuTable().FtrTileY)
    {
        return GmmGen11TextureCalc::GetMipTailGeometryOffset(pTexInfo, MipLevel, OffsetX, OffsetY, OffsetZ);
    }

    // Til64 is the only format which supports MipTail on FtrTileY disabled platforms
    __GMM_ASSERT(pTexInfo->Flags.Info.Tile64);
    // Mipped MSAA is not supported for Tile64
    __GMM_ASSERT(pTexInfo->MSAA.NumSamples <= 1);

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


    // FtrTileY disabled platforms: platforms which support Tile4/Tile64 tiled formats
    if(pTexInfo->Type == RESOURCE_1D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod;

        *OffsetX = MipTailSlotOffset1DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = MipTailSlotOffset1DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = MipTailSlotOffset1DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_2D || pTexInfo->Type == RESOURCE_CUBE)
    {
        // Mipped MSAA is not supported on Tile64, so need not account for MSAA here
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod;

        *OffsetX = MipTailSlotOffset2DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = MipTailSlotOffset2DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = MipTailSlotOffset2DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_3D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod;

        *OffsetX = MipTailSlotOffset3DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = MipTailSlotOffset3DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = MipTailSlotOffset3DSurface[Slot][ArrayIndex].Z;
    }

    GMM_DPF_EXIT;
    return;
}

