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
#include "Internal/Common/Texture/GmmGen10TextureCalc.h"

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip offset of given LOD in Mip Tail
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: given LOD #
///
/// @return     offset value of LOD in bytes
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen10TextureCalc::GetMipTailByteOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                           uint32_t          MipLevel)
{
    uint32_t ByteOffset = 0, Slot = 0xff;

    GMM_DPF_ENTER;

    // 3D textures follow the Gen9 mip tail format
    if(!pGmmLibContext->GetSkuTable().FtrStandardMipTailFormat || pTexInfo->Type == RESOURCE_3D)
    {
        return GmmGen9TextureCalc::GetMipTailByteOffset(pTexInfo, MipLevel);
    }


    if(pTexInfo->Type == RESOURCE_1D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
               (pTexInfo->Flags.Info.TiledYf ? 4 : 0);
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
                    (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples == 16) ? 11:
                    (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples ==  8) ? 10:
                    (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples ==  4) ?  8:
                    (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples ==  2) ?  5:
                    (pTexInfo->Flags.Info.TiledYf                                   ) ?  4: 0);
        // clang-format on
    }

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

    GMM_DPF_EXIT;

    return (ByteOffset);
}

GMM_MIPTAIL_SLOT_OFFSET Gen10MipTailSlotOffset1DSurface[15][5] = GEN10_MIPTAIL_SLOT_OFFSET_1D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET Gen10MipTailSlotOffset2DSurface[15][5] = GEN10_MIPTAIL_SLOT_OFFSET_2D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET Gen10MipTailSlotOffset3DSurface[15][5] = GEN10_MIPTAIL_SLOT_OFFSET_3D_SURFACE;
/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip-map offset in geometric OffsetX, Y, Z for a given LOD in Mip Tail.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: mip-map level
///             OffsetX: ptr to Offset in X direction (in bytes)
///             OffsetY: ptr to Offset in Y direction (in pixels)
///             OffsetZ: ptr to Offset in Z direction (in pixels)
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen10TextureCalc::GetMipTailGeometryOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                           uint32_t          MipLevel,
                                                           uint32_t *        OffsetX,
                                                           uint32_t *        OffsetY,
                                                           uint32_t *        OffsetZ)
{
    uint32_t ArrayIndex = 0;
    uint32_t Slot       = 0;

    GMM_DPF_ENTER;

    // 3D textures follow the Gen9 mip tail format
    if(!pGmmLibContext->GetSkuTable().FtrStandardMipTailFormat ||
       pTexInfo->Type == RESOURCE_3D)
    {
        return GmmGen9TextureCalc::GetMipTailGeometryOffset(pTexInfo, MipLevel, OffsetX, OffsetY, OffsetZ);
    }

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

        *OffsetX = Gen10MipTailSlotOffset1DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen10MipTailSlotOffset1DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen10MipTailSlotOffset1DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_2D || pTexInfo->Type == RESOURCE_CUBE)
    {
        // clang-format off
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
               // TileYs
               ((pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples == 16) ? 4 :
               (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples == 8) ? 3 :
               (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples == 4) ? 2 :
               (pTexInfo->Flags.Info.TiledYs && pTexInfo->MSAA.NumSamples == 2) ? 1 :
               (pTexInfo->Flags.Info.TiledYs) ? 0 :
               // TileYf
               (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples == 16) ? 11 :
               (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples == 8) ? 10 :
               (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples == 4) ? 8 :
               (pTexInfo->Flags.Info.TiledYf && pTexInfo->MSAA.NumSamples == 2) ? 5 :
               (pTexInfo->Flags.Info.TiledYf) ? 4 : 0);
        // clang-format on

        *OffsetX = Gen10MipTailSlotOffset2DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen10MipTailSlotOffset2DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen10MipTailSlotOffset2DSurface[Slot][ArrayIndex].Z;
    }

    GMM_DPF_EXIT;
    return;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the aligned block height of the 3D surface on Gen9
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             BlockHeight:
///             ExpandedArraySize:  adjusted array size for MSAA, cube faces, etc.
///
/// @return     BlockHeight
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen10TextureCalc::GetAligned3DBlockHeight(GMM_TEXTURE_INFO *pTexInfo,
                                                              uint32_t          BlockHeight,
                                                              uint32_t          ExpandedArraySize)
{
    uint32_t DAlign, CompressHeight, CompressWidth, CompressDepth;
    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, 0);

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    DAlign = pTexInfo->Alignment.DAlign;

    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    if(pTexInfo->Type == RESOURCE_3D)
    {
        ExpandedArraySize = GFX_ALIGN_NP2(ExpandedArraySize, DAlign) / CompressDepth;

        if(!pTexInfo->Flags.Info.Linear)
        {
            BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
        }
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
GMM_STATUS GMM_STDCALL GmmLib::GmmGen10TextureCalc::FillTex2D(GMM_TEXTURE_INFO * pTexInfo,
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
	    //Gmm uses TileY for Stencil allocations, having half TileW height (TileY width compensates)
            if(pTexInfo->Flags.Gpu.SeparateStencil && pTexInfo->Flags.Info.TiledW)
            {
                Alignment *= 2;
            }
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

    // For Non - planar surfaces, the alignment is done on the entire height of the allocation
    if(pGmmLibContext->GetWaTable().WaAlignYUVResourceToLCU &&
       GmmIsYUVFormatLCUAligned(pTexInfo->Format))
    {
        AlignedWidth = GFX_ALIGN(AlignedWidth, GMM_SCANLINES(GMM_MAX_LCU_SIZE));
    }

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

    // For Non-planar surfaces, the alignment is done on the entire height of the allocation
    if(pGmmLibContext->GetWaTable().WaAlignYUVResourceToLCU &&
        GmmIsYUVFormatLCUAligned(pTexInfo->Format) &&
       !GmmIsPlanar(pTexInfo->Format))
    {
        BlockHeight = GFX_ALIGN(BlockHeight, GMM_SCANLINES(GMM_MAX_LCU_SIZE));
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
/// This function will Setup a planar surface allocation.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions.
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen10TextureCalc::FillTexPlanar(GMM_TEXTURE_INFO * pTexInfo,
                                                                  __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   WidthBytesPhysical, Height, YHeight, VHeight;
    uint32_t   AdjustedVHeight = 0;
    GMM_STATUS Status;
    bool       UVPacked = false;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);
    __GMM_ASSERT(!pTexInfo->Flags.Info.TiledW);
    pTexInfo->TileMode = TILE_NONE;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    WidthBytesPhysical = GFX_ULONG_CAST(pTexInfo->BaseWidth) * pTexInfo->BitsPerPixel >> 3;
    Height = VHeight = 0;

    YHeight = pTexInfo->BaseHeight;

    switch(pTexInfo->Format)
    {
        case GMM_FORMAT_IMC1: // IMC1 = IMC3 with Swapped U/V
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
                VHeight = GFX_ALIGN(GFX_CEIL_DIV(YHeight, 2), GMM_IMCx_PLANE_ROW_ALIGNMENT);

                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                Height = YHeight + 2 * VHeight; // One VHeight for V and one for U.

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

                Height = YHeight + 2 * VHeight;

                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;

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
        case GMM_FORMAT_BGRP:
        case GMM_FORMAT_RGBP:
        case GMM_FORMAT_MFX_JPEG_YUV444: // Similar to IMC3 but U/V are full size.
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

                Height = YHeight + 2 * VHeight;

                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;

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
                WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical, 2); // If odd YWidth, pitch bumps-up to fit rounded-up U/V planes.
            }
            else //if(pTexInfo->Format == GMM_FORMAT_NV11)
            {
                // Tiling not supported, since YPitch != UVPitch...
                pTexInfo->Flags.Info.TiledY  = 0;
                pTexInfo->Flags.Info.TiledYf = 0;
                pTexInfo->Flags.Info.TiledYs = 0;
                pTexInfo->Flags.Info.TiledX  = 0;
                pTexInfo->Flags.Info.Linear  = 1;
            }

            UVPacked                              = true;
            pTexInfo->OffsetInfo.Plane.NoOfPlanes = 2;
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
            pTexInfo->Flags.Info.TiledY  = 0;
            pTexInfo->Flags.Info.TiledYf = 0;
            pTexInfo->Flags.Info.TiledYs = 0;
            pTexInfo->Flags.Info.TiledX  = 0;
            pTexInfo->Flags.Info.Linear  = 1;

            pTexInfo->OffsetInfo.Plane.NoOfPlanes = 1;
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

    // MMC is not supported for linear formats.
    if(pTexInfo->Flags.Gpu.MMC)
    {
        if(!(pTexInfo->Flags.Info.TiledY || pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs))
        {
            pTexInfo->Flags.Gpu.MMC = 0;
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
    // In case of Planar surfaces, only the last Plane has to be aligned to 64 for LCU access
    if(pGmmLibContext->GetWaTable().WaAlignYUVResourceToLCU && GmmIsYUVFormatLCUAligned(pTexInfo->Format) && VHeight > 0)
    {
        AdjustedVHeight = GFX_ALIGN(VHeight, GMM_SCANLINES(GMM_MAX_LCU_SIZE));
        Height += AdjustedVHeight - VHeight;
    }

    // For Tiled Planar surfaces, the planes must be tile-boundary aligned.
    // Actual alignment is handled in FillPlanarOffsetAddress, but height
    // and width must be adjusted for correct size calculation
    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        uint32_t TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        uint32_t TileWidth  = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileWidth;

        pTexInfo->OffsetInfo.Plane.IsTileAlignedPlanes = true;

        //for separate U and V planes, use U plane unaligned and V plane aligned
        Height = GFX_ALIGN(YHeight, TileHeight) + (UVPacked ? GFX_ALIGN(AdjustedVHeight, TileHeight) :
                                                              (GFX_ALIGN(VHeight, TileHeight) + GFX_ALIGN(AdjustedVHeight, TileHeight)));

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

        if(pTexInfo->Flags.Info.TiledYs || pTexInfo->Flags.Info.TiledYf)
        {
            pTexInfo->Flags.Info.RedecribedPlanes = true;
        }
    }

    // Vary wide planar tiled planar formats do not support MMC pre gen11. All formats do not support
    //Special case LKF MMC compressed surfaces
    if(pTexInfo->Flags.Gpu.MMC &&
       pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
       pTexInfo->Flags.Info.TiledY)
    {
        uint32_t TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;

        Height = GFX_ALIGN(YHeight, TileHeight) + GFX_ALIGN(AdjustedVHeight, TileHeight);
    }

    // Vary wide planar tiled planar formats do not support MMC pre gen11. All formats do not support
    // MMC above 16k bytes wide, while Yf NV12 does not support above 8k - 128 bytes.
    if((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) <= IGFX_GEN10_CORE) &&
       (pTexInfo->Flags.Info.TiledY || pTexInfo->Flags.Info.TiledYf || pTexInfo->Flags.Info.TiledYs))
    {
        if(((pTexInfo->BaseWidth * pTexInfo->BitsPerPixel / 8) >= GMM_KBYTE(16)) ||
           (pTexInfo->Format == GMM_FORMAT_NV12 && pTexInfo->Flags.Info.TiledYf &&
            (pTexInfo->BaseWidth * pTexInfo->BitsPerPixel / 8) >= (GMM_KBYTE(8) - 128)))
        {
            pTexInfo->Flags.Gpu.MMC = 0;
        }
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
