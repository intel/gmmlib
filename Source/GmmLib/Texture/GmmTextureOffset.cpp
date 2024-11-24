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
/// GMM Interface to return lock or render aligned offset to a mip map
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO to store offset info
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmTexGetMipMapOffset(GMM_TEXTURE_INFO *   pTexInfo,
                                 GMM_REQ_OFFSET_INFO *pReqInfo,
                                 GMM_LIB_CONTEXT *    pGmmLibContext)
{
    GMM_STATUS        Status           = GMM_SUCCESS;
    bool              RestoreRenderReq = false;
    bool              RestoreLockReq   = false;
    GMM_TEXTURE_CALC *pTextureCalc;

    GMM_DPF_ENTER;
    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pReqInfo, GMM_ERROR);
    __GMM_ASSERT(pReqInfo->CubeFace <= __GMM_NO_CUBE_MAP);

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(pTexInfo, pGmmLibContext);

    if((pReqInfo->Plane >= GMM_MAX_PLANE) ||
       (pReqInfo->Plane < GMM_NO_PLANE) ||
       (pReqInfo->MipLevel >= GMM_MAX_MIPMAP))
    {
        GMM_ASSERTDPF(0, "Invalid parameter!");
        return GMM_ERROR;
    }

    if((pTexInfo->TileMode >= GMM_TILE_MODES) ||
       (pTexInfo->TileMode < TILE_NONE))
    {
        GMM_ASSERTDPF(0, "Invalid parameter!");
        return GMM_ERROR;
    }

    // Retrieve offset info at pReqInfo->MipLevel
    if(pReqInfo->ReqLock)
    {
        if(pReqInfo->ReqRender)
        {
            pReqInfo->ReqRender = 0;
            RestoreRenderReq    = true;
        }

        if(pTextureCalc->GetTexLockOffset(pTexInfo, pReqInfo) != GMM_SUCCESS)
        {
            GMM_ASSERTDPF(0, "ReqLock failed!");
            Status = GMM_ERROR;
        }
    }

    if(RestoreRenderReq == true)
        pReqInfo->ReqRender = 1;

    if(pReqInfo->ReqLock)
    {
        pReqInfo->ReqLock = 0;
        RestoreLockReq    = 1;
    }

    if(pReqInfo->ReqRender)
    {
        if(pTextureCalc->GetTexRenderOffset(pTexInfo, pReqInfo) != GMM_SUCCESS)
        {
            GMM_ASSERTDPF(0, "ReqRender failed!");
            Status = GMM_ERROR;
        }
    }
    
    if(RestoreLockReq)
    {
        pReqInfo->ReqLock = 1;
    }
    
    if(pReqInfo->ReqStdLayout)
    {
        if(pTextureCalc->GetTexStdLayoutOffset(pTexInfo, pReqInfo) != GMM_SUCCESS)
        {
            GMM_ASSERTDPF(0, "ReqStdLayout failed!");
            Status = GMM_ERROR;
        }
    }

    GMM_DPF_EXIT;
    return Status;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates StdLayout offsets and related pitches of
/// subresource..
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO to store offset info
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::GetTexStdLayoutOffset(GMM_TEXTURE_INFO *   pTexInfo,
                                                         GMM_REQ_OFFSET_INFO *pReqInfo)
{
    uint32_t ReqArrayIndex;
    bool     NeedSurfaceSize = false;

    __GMM_ASSERT(pTexInfo);
    __GMM_ASSERT(GMM_IS_64KB_TILE(pTexInfo->Flags) || pTexInfo->Flags.Info.TiledYf);
    __GMM_ASSERT(
    (pTexInfo->Type == RESOURCE_2D) ||
    (pTexInfo->Type == RESOURCE_3D) ||
    (pTexInfo->Type == RESOURCE_CUBE));
    __GMM_ASSERT(GmmIsPlanar(pTexInfo->Format) == false); // Planar not support

    if(pReqInfo->StdLayout.Offset == -1) // Special Req for Surface Size
    {
        NeedSurfaceSize = true;
        ReqArrayIndex   = // TODO(Medium): Add planar support.
        (pTexInfo->ArraySize * ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1));
    }
    else
    {
        ReqArrayIndex =
        (pReqInfo->ArrayIndex * ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1));
    }

    {
        uint32_t TileSize = 0;

        // TileYs (64) and TileYf (4)
        if(GMM_IS_64KB_TILE(pTexInfo->Flags))
        {
            TileSize = GMM_KBYTE(64);
        }
        else if(GMM_IS_4KB_TILE(pTexInfo->Flags))
        {
            TileSize = GMM_KBYTE(4);
        }

        const GMM_PLATFORM_INFO *pPlatform       = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);
        uint32_t                 BytesPerElement = pTexInfo->BitsPerPixel / CHAR_BIT;
        GMM_TILE_MODE            TileMode        = pTexInfo->TileMode;
        struct
        {
            uint32_t Width, Height, Depth;
        } Element, Tile;

        __GMM_ASSERT(TileMode < GMM_TILE_MODES);

        GetCompressionBlockDimensions(
        pTexInfo->Format,
        &Element.Width,
        &Element.Height,
        &Element.Depth);

        Tile.Width =
        (pPlatform->TileInfo[TileMode].LogicalTileWidth / BytesPerElement) *
        Element.Width;

        Tile.Height =
        pPlatform->TileInfo[TileMode].LogicalTileHeight *
        Element.Height;

        Tile.Depth =
        pPlatform->TileInfo[TileMode].LogicalTileDepth *
        Element.Depth;

        {
            GMM_GFX_ADDRESS TargetLodOffset = 0;
            GMM_GFX_SIZE_T  PrevMipSize     = 0;
            GMM_GFX_SIZE_T  SliceOffset     = 0;
            GMM_GFX_SIZE_T  SlicePitch      = 0;
            uint32_t        Lod;
            uint32_t        EffectiveMaxLod =
            (ReqArrayIndex == 0) ?
            pReqInfo->MipLevel :
            GFX_MIN(pTexInfo->MaxLod, pTexInfo->Alignment.MipTailStartLod);

            pReqInfo->StdLayout.Offset = 0;
            for(Lod = 0; Lod <= EffectiveMaxLod; Lod++)
            {
                GMM_GFX_SIZE_T MipWidth  = GmmTexGetMipWidth(pTexInfo, Lod);
                uint32_t       MipHeight = GmmTexGetMipHeight(pTexInfo, Lod);
                uint32_t       MipDepth  = GmmTexGetMipDepth(pTexInfo, Lod);

                uint32_t MipCols = GFX_ULONG_CAST(
                GFX_CEIL_DIV(
                MipWidth,
                Tile.Width));
                uint32_t MipRows =
                GFX_CEIL_DIV(
                MipHeight,
                Tile.Height);
                uint32_t MipDepthTiles =
                GFX_CEIL_DIV(
                MipDepth,
                Tile.Depth);
                uint32_t RowPitch   = MipCols * TileSize; // Bytes from one tile row to the next.
                uint32_t DepthPitch = RowPitch * MipRows; // Bytes from one depth slice of tiles to the next.

                if(Lod <= pTexInfo->Alignment.MipTailStartLod)
                {
                    pReqInfo->StdLayout.Offset += PrevMipSize;
                }

                if(Lod == pReqInfo->MipLevel)
                {
                    TargetLodOffset = pReqInfo->StdLayout.Offset;

                    pReqInfo->StdLayout.TileRowPitch   = RowPitch;
                    pReqInfo->StdLayout.TileDepthPitch = DepthPitch;
                }

                PrevMipSize = (GMM_GFX_SIZE_T)DepthPitch * MipDepthTiles;
                SlicePitch += DepthPitch;
            }

            if(pReqInfo->Slice > 0)
            {
                SliceOffset = SlicePitch * pReqInfo->Slice;
            }

            if(!NeedSurfaceSize && pReqInfo->MipLevel >= pTexInfo->Alignment.MipTailStartLod)
            {
                pReqInfo->StdLayout.Offset += (ReqArrayIndex * (pReqInfo->StdLayout.Offset + PrevMipSize)) +
                                              GetMipTailByteOffset(pTexInfo, pReqInfo->MipLevel);
            }
            else
            {
                pReqInfo->StdLayout.Offset = ReqArrayIndex * (pReqInfo->StdLayout.Offset + PrevMipSize) +
                                             TargetLodOffset;
            }

            pReqInfo->StdLayout.Offset += SliceOffset;
        }
    }

    return GMM_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Calculates offset address of a sub resource(i.e.  Mip Map, Cube face, volume texture)
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO to store offset info
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::GetTexLockOffset(GMM_TEXTURE_INFO *   pTexInfo,
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
        pReqInfo->Lock.Pitch    = Pitch;

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
/// Function used to align width and height of texture so that it satisfy our HW
/// restriction
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pHeight: ptr to height of mip
/// @param[in]  pWidth: ptr to width of mip
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmTextureCalc::AlignTexHeightWidth(GMM_TEXTURE_INFO *pTexInfo,
                                                 uint32_t *        pHeight,
                                                 uint32_t *        pWidth)
{
    uint32_t MipWidth        = 0;
    uint32_t MipHeight       = 0;
    uint32_t UnitAlignHeight = 0;
    uint32_t UnitAlignWidth  = 0;

    uint8_t Compress = 0;

    __GMM_ASSERTPTR(pTexInfo, VOIDRETURN);
    __GMM_ASSERTPTR(pWidth, VOIDRETURN);
    __GMM_ASSERTPTR(pHeight, VOIDRETURN);
    __GMM_ASSERTPTR(pGmmLibContext, VOIDRETURN);

    MipWidth  = *pWidth;
    MipHeight = *pHeight;

    UnitAlignWidth  = pTexInfo->Alignment.HAlign;
    UnitAlignHeight = pTexInfo->Alignment.VAlign;
    Compress        = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);

    MipWidth  = GFX_MAX(MipWidth, UnitAlignWidth);
    MipHeight = GFX_MAX(MipHeight, UnitAlignHeight);

    MipWidth  = GFX_ALIGN(MipWidth, UnitAlignWidth);
    MipHeight = GFX_ALIGN(MipHeight, UnitAlignHeight);

    if(Compress)
    {
        uint32_t CompressHeight, CompressWidth, CompressDepth;
        GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);
        MipWidth /= CompressWidth;
        MipHeight /= CompressHeight;
    }
    else if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
    {
        MipWidth *= 2;
        MipHeight /= 2;
    }

    *pHeight = MipHeight;
    *pWidth  = MipWidth;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Function used to calculate the render aligned offset of a given surface
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::GetTexRenderOffset(GMM_TEXTURE_INFO *   pTexInfo,
                                                      GMM_REQ_OFFSET_INFO *pReqInfo)
{

    const GMM_TILE_INFO *    pTileInfo         = NULL;
    GMM_GFX_SIZE_T           AddressOffset     = 0;
    GMM_GFX_SIZE_T           RenderAlignOffset = 0;
    uint32_t                 OffsetX           = 0;
    uint32_t                 OffsetY           = 0;
    uint32_t                 OffsetZ           = 0;
    const GMM_PLATFORM_INFO *pPlatform         = NULL;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pReqInfo, GMM_ERROR);

    pPlatform     = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);
    pTileInfo     = &pPlatform->TileInfo[pTexInfo->TileMode];
    AddressOffset = GetMipMapByteAddress(pTexInfo, pReqInfo);

    if(GMM_IS_TILED(*pTileInfo))
    {
        uint32_t       TileAlignedOffsetX = 0;
        uint32_t       TileAlignedOffsetY = 0;
        GMM_GFX_SIZE_T MipTailByteOffset  = 0;

        //--- Compute Tile-Aligned Offset, and Corresponding X/Y Offsets -------
        // Render/Tiled-Aligned offsets and corresponding X/Y offsets are used
        // to program the Surface Base Address and X/Y Offset fields of a
        // SURFACE_STATE. For a given subresource, the tiled-aligned offset
        // addresses the tile containing the base of the subresource; the X/Y
        // offsets then give the additional offsets into the tile of the
        // subresource base. (Though in SURFACE_STATE, X Offset is specified in
        // pixels, this function will return the X Offset in bytes. Y Offset is
        // in pixel rows.)

        if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
           (pReqInfo->MipLevel >= pTexInfo->Alignment.MipTailStartLod))
        {
            MipTailByteOffset = GetMipTailByteOffset(pTexInfo, pReqInfo->MipLevel);

            // For MipTail, Offset is really with respect to start of MipTail,
            // so taking out individual Mipoffset within miptail region to get correct Tile aligned offset.
            AddressOffset -= MipTailByteOffset;
        }

        if(!pTexInfo->Flags.Info.RedecribedPlanes)
        {
            GMM_GFX_SIZE_T Pitch = pTexInfo->Pitch;
            if(!pTexInfo->Pitch)
            {
                // If no pitch exists, but the surface is still marked as tiled, then it is a 1D TileYf/Ys surface.
                // Technically no pitch exists for 1D surfaces, but we will fake it to make calculations work below.
                // Since 1D surfaces only have an X-dimension, this Pitch calculation is only used for OffsetX calculation.
                Pitch = pTexInfo->Size;
            }
	    
	    if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
            {
                OffsetX            = GFX_ULONG_CAST(AddressOffset % Pitch);
                TileAlignedOffsetX = GFX_ALIGN_FLOOR(OffsetX, pTileInfo->LogicalTileWidth / 2);
                OffsetX -= TileAlignedOffsetX;
            }
            else
            {
                OffsetX            = GFX_ULONG_CAST(AddressOffset % Pitch);
                TileAlignedOffsetX = GFX_ALIGN_FLOOR(OffsetX, pTileInfo->LogicalTileWidth);
                OffsetX -= TileAlignedOffsetX;
            }

            if(pTexInfo->Pitch)
            {
                if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
                {
                    //Expt: YOffset ignore row-interleave -- verify both 2d/3d mips
                    OffsetY = GFX_ULONG_CAST(AddressOffset / pTexInfo->Pitch);
                    OffsetY *= 2;
                    TileAlignedOffsetY = GFX_ALIGN_FLOOR(OffsetY, pTileInfo->LogicalTileHeight * 2 * pTileInfo->LogicalTileDepth);
                    OffsetY -= TileAlignedOffsetY;
                    TileAlignedOffsetY /= 2;
                }
                else
                {
                    OffsetY            = GFX_ULONG_CAST(AddressOffset / pTexInfo->Pitch);
                    TileAlignedOffsetY = GFX_ALIGN_FLOOR(OffsetY, pTileInfo->LogicalTileHeight * pTileInfo->LogicalTileDepth);
                    OffsetY -= TileAlignedOffsetY;
                }
            }

            RenderAlignOffset =
            TileAlignedOffsetY * pTexInfo->Pitch +
            (TileAlignedOffsetX / pTileInfo->LogicalTileWidth) * pTileInfo->LogicalSize;

            // For Gen9+, Miptail Lods should be reported in a way that
            //      - Base Address equals tile-aligned "Miptail start address"
            //      - OffsetX equals to offset (in bytes) from "Miptail start Lod" to "current Lod" in geometric X direction
            //      - OffsetY and OffsetZ are their pixel distance from "Miptail start Lod" to "current Lod" in geometric Y, Z directions
            // Note: only Tile Yf and TileYs have Miptails and their Mips are always "tile aligned"

            if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
               (pReqInfo->MipLevel >= pTexInfo->Alignment.MipTailStartLod) &&
               // Planar surfaces do not support MIPs
               !GmmIsPlanar(pTexInfo->Format))
            {
                GetMipTailGeometryOffset(pTexInfo, pReqInfo->MipLevel, &OffsetX, &OffsetY, &OffsetZ);
            }
        }
        else
        {
            // Std swizzled and UV packed planes begin at tile-aligned
            // offsets and do not support MIPs, so no adjustment is needed
            RenderAlignOffset = AddressOffset;
            OffsetX = OffsetY = OffsetZ = 0;
        }
    }
    else
    {
        // Linear case make sure Render address is DWORD aligned.
        RenderAlignOffset = GFX_ALIGN_FLOOR(AddressOffset, GMM_BYTES(4));

        if(pTexInfo->Pitch)
        {
            OffsetX = GFX_ULONG_CAST((AddressOffset - RenderAlignOffset) % pTexInfo->Pitch);
            OffsetY = GFX_ULONG_CAST((AddressOffset - RenderAlignOffset) / pTexInfo->Pitch);
        }
        else
        {
            // One-dimensional textures (no height)
            OffsetX = GFX_ULONG_CAST(AddressOffset - RenderAlignOffset);
            OffsetY = 0;
        }
    }

    pReqInfo->Render.Offset64 = RenderAlignOffset;
    pReqInfo->Render.XOffset  = GFX_ULONG_CAST(OffsetX);
    pReqInfo->Render.YOffset  = GFX_ULONG_CAST(OffsetY);
    pReqInfo->Render.ZOffset  = GFX_ULONG_CAST(OffsetZ);

    return GMM_SUCCESS;
} // __GmmGetRenderAlignAddress


/////////////////////////////////////////////////////////////////////////////////////
/// Function used to calculate byte address of a specified mip map
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO
///
/// @return     ::GMM_GFX_SIZE_T byte offset
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GmmLib::GmmTextureCalc::GetMipMapByteAddress(GMM_TEXTURE_INFO *   pTexInfo,
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
            OffsetX     = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.X[Plane]);
            OffsetY     = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.Y[Plane]);
            ArrayQPitch = pTexInfo->OffsetInfo.Plane.ArrayQPitch;
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


/////////////////////////////////////////////////////////////////////////////////////
/// Utility function used to calculate byte address to a mip slice
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO
///
/// @return     byte offset
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GmmLib::GmmTextureCalc::Get3DMipByteAddress(GMM_TEXTURE_INFO *   pTexInfo,
                                                           GMM_REQ_OFFSET_INFO *pReqInfo)
{
    uint32_t            MipsInThisRow, PlaneRows;
    uint32_t            MipHeight, MipWidth;
    uint32_t            UnitAlignHeight, UnitAlignWidth;
    GMM_GFX_SIZE_T      MipMapByteAddress, ExtraBytes;
    uint32_t            Slice, MipLevel, Pitch;
    uint8_t             Compress;
    GMM_RESOURCE_FORMAT GenericFormat;
    uint32_t            CompressHeight, CompressWidth, CompressDepth;

    __GMM_ASSERTPTR(pGmmLibContext, 0);

    GenericFormat = pTexInfo->Format;
    Slice         = pReqInfo->Slice;
    MipLevel      = pReqInfo->MipLevel;
    Pitch         = GFX_ULONG_CAST(pTexInfo->Pitch);

    // For slice 0 for any mip address is simple and stored in table
    if(Slice == 0)
    {
        MipMapByteAddress = pTexInfo->OffsetInfo.Texture3DOffsetInfo.Offset[MipLevel];
    }
    // For any slice
    else
    {
        MipMapByteAddress = pTexInfo->OffsetInfo.Texture3DOffsetInfo.Offset[MipLevel];

        // See how many mip can fit in one row
        MipsInThisRow = GFX_2_TO_POWER_OF(MipLevel);

        PlaneRows = Slice / MipsInThisRow;

        // make sure we get the height and mip of base level
        MipWidth  = GFX_ULONG_CAST(pTexInfo->BaseWidth);
        MipHeight = pTexInfo->BaseHeight;

        MipWidth >>= MipLevel;
        MipHeight >>= MipLevel;

        UnitAlignWidth  = pTexInfo->Alignment.HAlign;
        UnitAlignHeight = pTexInfo->Alignment.VAlign;
        Compress        = GmmIsCompressed(pGmmLibContext, pTexInfo->Format);
        GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

        // clamp such that mip height is at least min height
        MipHeight = GFX_MAX(MipHeight, UnitAlignHeight);
        MipHeight = GFX_ALIGN(MipHeight, UnitAlignHeight);

        // clamp such that mip width is at least min width
        MipWidth = GFX_MAX(MipWidth, UnitAlignWidth);
        MipWidth = GFX_ALIGN(MipWidth, UnitAlignWidth);

        if(Compress)
        {
            MipWidth /= CompressWidth;
            MipHeight /= CompressHeight;
        }
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            MipWidth *= 2;
            MipHeight /= 2;
        }

        ExtraBytes = (GMM_GFX_SIZE_T)PlaneRows * MipHeight * Pitch;

        ExtraBytes += ((GMM_GFX_SIZE_T)(Slice % MipsInThisRow) *
                       MipWidth * pTexInfo->BitsPerPixel) >>
                      3;

        // get address offset
        MipMapByteAddress += ExtraBytes;
    }

    return MipMapByteAddress;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Utility function calculates a byte offset from the base of the allocation
//  to L frame, R frame, or blank region.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
/// @param[in]  pReqInfo: ptr to GMM_REQ_OFFSET_INFO
///
/// @return     byte offset
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmTextureCalc::GetDisplayFrameOffset(GMM_TEXTURE_INFO *   pTexInfo,
                                                       GMM_REQ_OFFSET_INFO *pReqInfo)
{
    uint32_t Offset;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pReqInfo, GMM_ERROR);

    switch(pReqInfo->Frame)
    {
        case GMM_DISPLAY_L:
            Offset = 0;
            break;
        case GMM_DISPLAY_R:
            Offset = pTexInfo->S3d.RFrameOffset;
            break;
        case GMM_DISPLAY_BLANK_AREA:
            Offset = pTexInfo->S3d.BlankAreaOffset;
            break;
        default:
            Offset = 0;
            GMM_ASSERTDPF(0, "Unknown Frame Type!");
            break;
    }

    return Offset;
}

void GmmLib::GmmTextureCalc::SetPlanarOffsetInfo(GMM_TEXTURE_INFO *pTexInfo, GMM_RESCREATE_CUSTOM_PARAMS &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        pTexInfo->OffsetInfo.Plane.IsTileAlignedPlanes = true;
    }
    for(uint8_t i = 1; i <= CreateParams.NoOfPlanes; i++)
    {
        pTexInfo->OffsetInfo.Plane.X[i] = CreateParams.PlaneOffset.X[i];
        pTexInfo->OffsetInfo.Plane.Y[i] = CreateParams.PlaneOffset.Y[i];
    }
    pTexInfo->OffsetInfo.Plane.NoOfPlanes = CreateParams.NoOfPlanes;
}

#ifndef __GMM_KMD__
void GmmLib::GmmTextureCalc::SetPlanarOffsetInfo_2(GMM_TEXTURE_INFO *pTexInfo, GMM_RESCREATE_CUSTOM_PARAMS_2 &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        pTexInfo->OffsetInfo.Plane.IsTileAlignedPlanes = true;
    }
    for(uint8_t i = 1; i <= CreateParams.NoOfPlanes; i++)
    {
        pTexInfo->OffsetInfo.Plane.X[i] = CreateParams.PlaneOffset.X[i];
        pTexInfo->OffsetInfo.Plane.Y[i] = CreateParams.PlaneOffset.Y[i];
    }
    pTexInfo->OffsetInfo.Plane.NoOfPlanes = CreateParams.NoOfPlanes;
}
#endif

void GmmLib::GmmTextureCalc::SetPlaneUnAlignedTexOffsetInfo(GMM_TEXTURE_INFO *pTexInfo, uint32_t YHeight, uint32_t VHeight)
{
    uint32_t UmdUHeight = 0, UmdVHeight = 0;

    pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y] = YHeight;
    if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 2)
    {
        pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U] = VHeight;
        UmdUHeight                                               = (GMM_GLOBAL_GFX_SIZE_T)((pTexInfo->Size / pTexInfo->Pitch) - pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]);
    }
    else if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 3)
    {
        pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U] =
        pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_V] = VHeight;
        UmdUHeight                                               = (GMM_GLOBAL_GFX_SIZE_T)(pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V] - pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]);
        UmdVHeight                                               = (GMM_GLOBAL_GFX_SIZE_T)(((pTexInfo->Size / pTexInfo->Pitch) - pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]) / 2);
        __GMM_ASSERTPTR((UmdUHeight == UmdVHeight), VOIDRETURN);
    }

    __GMM_ASSERTPTR(((pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] == YHeight) && (UmdUHeight == VHeight)), VOIDRETURN);
}

uint32_t GmmLib::GmmTextureCalc::IsTileAlignedPlanes(GMM_TEXTURE_INFO *pTexInfo)
{
    return pTexInfo->OffsetInfo.Plane.IsTileAlignedPlanes;
}

uint32_t GmmLib::GmmTextureCalc::GetNumberOfPlanes(GMM_TEXTURE_INFO *pTexInfo)
{
    return pTexInfo->OffsetInfo.Plane.NoOfPlanes;
}

void GmmLib::GmmTextureCalc::GetPlaneIdForCpuBlt(GMM_TEXTURE_INFO *pTexInfo, GMM_RES_COPY_BLT *pBlt, uint32_t *PlaneId)
{
    uint32_t TotalHeight = 0;

    if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 2)
    {
        TotalHeight = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y] +
                                     pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U]);
    }
    else if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 3)
    {
        TotalHeight = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y] +
                                     pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U] +
                                     pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_V]);
    }
    else
    {
        TotalHeight = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y]); //YV12 exception
    }

    // Determine if BLT rectange is for monolithic surface or contained in specific Y/UV plane
    if(((pBlt->Gpu.OffsetY + pBlt->Blt.Height <= pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]) || pTexInfo->OffsetInfo.Plane.NoOfPlanes == 1) &&
       (pBlt->Gpu.OffsetX + pBlt->Blt.Width <= pTexInfo->BaseWidth))
    {
        *PlaneId = GMM_PLANE_Y;
    }
    else if(pBlt->Gpu.OffsetY >= pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] &&
            (pBlt->Gpu.OffsetY + pBlt->Blt.Height <= (pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] + pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U])) &&
            (pBlt->Gpu.OffsetX + pBlt->Blt.Width <= pTexInfo->BaseWidth))
    {
        *PlaneId = GMM_PLANE_U;
    }
    else if(pBlt->Gpu.OffsetY >= pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V] &&
            (pBlt->Gpu.OffsetY + pBlt->Blt.Height <= (pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V] + pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U])) &&
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

void GmmLib::GmmTextureCalc::GetBltInfoPerPlane(GMM_TEXTURE_INFO *pTexInfo, GMM_RES_COPY_BLT *pBlt, uint32_t PlaneId)
{
    if(PlaneId == GMM_PLANE_Y)
    {
        pBlt->Gpu.OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_Y]);
        pBlt->Gpu.OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y]);
        pBlt->Blt.Height  = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y]);
    }
    else if(PlaneId == GMM_PLANE_U)
    {
        pBlt->Gpu.OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_U]);
        pBlt->Gpu.OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]);

        pBlt->Sys.pData  = (char *)pBlt->Sys.pData + uint32_t(pBlt->Blt.Height * pBlt->Sys.RowPitch);
        pBlt->Blt.Height = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U]);
        if(pTexInfo->Flags.Info.RedecribedPlanes)
        {
            __GMM_ASSERT(0);
        }
    }
    else
    {
        pBlt->Gpu.OffsetX = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_V]);
        pBlt->Gpu.OffsetY = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V]);
        pBlt->Blt.Height  = GFX_ULONG_CAST(pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U]);
        pBlt->Sys.pData   = (char *)pBlt->Sys.pData + uint32_t(pBlt->Blt.Height * pBlt->Sys.RowPitch);
    }
}
