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
#include "Internal/Common/Texture/GmmGen11TextureCalc.h"

/////////////////////////////////////////////////////////////////////////////////////
/// This function calculates the (X,Y) address of each given plane. X is in bytes
/// and Y is in scanlines.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmGen11TextureCalc::FillPlanarOffsetAddress(GMM_TEXTURE_INFO *pTexInfo)
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
    pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_Y] = 0;
    pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y] = 0;

    pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y] = 0;
    pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y] = 0;

    Height = pTexInfo->BaseHeight;
    if(pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        Height = __GMM_EXPAND_HEIGHT(this, Height, pTexInfo->Alignment.VAlign, pTexInfo);
        Height = ScaleTextureHeight(pTexInfo, Height);
        if(pTexInfo->Flags.Gpu.UnifiedAuxSurface)
        {
            pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y] = 0;
        }
    }

    // GMM_PLANE_U/V Planes...
    pUOffsetX = &pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_U];
    pUOffsetY = &pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U];
    pVOffsetX = &pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_V];
    pVOffsetY = &pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V];

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

    pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_Y] = YHeight;
    if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 2)
    {
        pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U] = VHeight;
    }
    else if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 3)
    {
        pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_U] =
        pTexInfo->OffsetInfo.Plane.UnAligned.Height[GMM_PLANE_V] = VHeight;
    }

    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]) || pTexInfo->Flags.Gpu.__NonMsaaTileYCcs)
    {
        GMM_GFX_SIZE_T TileHeight = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        GMM_GFX_SIZE_T TileWidth  = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth;
        GMM_GFX_SIZE_T PhysicalTileHeight = TileHeight;
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
            *pUOffsetY += pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y];
            *pVOffsetY = *pUOffsetY;
        }

        // This is needed for FtrDisplayPageTables
        if(pGmmLibContext->GetSkuTable().FtrDisplayPageTables)
        {
            pTexInfo->OffsetInfo.Plane.Aligned.Height[GMM_PLANE_Y] = GFX_ALIGN(YHeight, TileHeight);
            if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 2)
            {
                pTexInfo->OffsetInfo.Plane.Aligned.Height[GMM_PLANE_U] = GFX_ALIGN(VHeight, TileHeight);
            }
            else if(pTexInfo->OffsetInfo.Plane.NoOfPlanes == 3)
            {
                pTexInfo->OffsetInfo.Plane.Aligned.Height[GMM_PLANE_U] =
                pTexInfo->OffsetInfo.Plane.Aligned.Height[GMM_PLANE_V] = GFX_ALIGN(VHeight, TileHeight);
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

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip offset of given LOD in Mip Tail
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: mip-map level
///
/// @return     offset value of LOD in bytes
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen11TextureCalc::GetMipTailByteOffset(GMM_TEXTURE_INFO *pTexInfo,
                                                           uint32_t          MipLevel)
{
    uint32_t ByteOffset = 0, Slot = 0xff;

    GMM_DPF_ENTER;

    // 3D textures follow the Gen10 mip tail format
    if(!pGmmLibContext->GetSkuTable().FtrStandardMipTailFormat)
    {
        return GmmGen9TextureCalc::GetMipTailByteOffset(pTexInfo, MipLevel);
    }

    if((pTexInfo->Type == RESOURCE_1D) || (pTexInfo->Type == RESOURCE_3D))
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
            ByteOffset = GMM_BYTES(0);
            break;
        case 12:
            ByteOffset = GMM_BYTES(64);
            break;
        case 13:
            ByteOffset = GMM_BYTES(128);
            break;
        case 14:
            ByteOffset = GMM_BYTES(196);
            break;
        default:
            __GMM_ASSERT(0);
    }

    GMM_DPF_EXIT;

    return (ByteOffset);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Returns the mip-map offset in geometric OffsetX, Y, Z for a given LOD in Mip Tail on Gen11.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
///             MipLevel: mip-map level
///             OffsetX: ptr to Offset in X direction (in bytes)
///             OffsetY: ptr to Offset in Y direction (in pixels)
///             OffsetZ: ptr to Offset in Z direction (in pixels)
///
/////////////////////////////////////////////////////////////////////////////////////

GMM_MIPTAIL_SLOT_OFFSET Gen11MipTailSlotOffset1DSurface[15][5] = GEN11_MIPTAIL_SLOT_OFFSET_1D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET Gen11MipTailSlotOffset2DSurface[15][5] = GEN11_MIPTAIL_SLOT_OFFSET_2D_SURFACE;
GMM_MIPTAIL_SLOT_OFFSET Gen11MipTailSlotOffset3DSurface[15][5] = GEN11_MIPTAIL_SLOT_OFFSET_3D_SURFACE;

void GmmLib::GmmGen11TextureCalc::GetMipTailGeometryOffset(GMM_TEXTURE_INFO *pTexInfo,
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
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod;

        *OffsetX = Gen11MipTailSlotOffset1DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen11MipTailSlotOffset1DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen11MipTailSlotOffset1DSurface[Slot][ArrayIndex].Z;
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

        *OffsetX = Gen11MipTailSlotOffset2DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen11MipTailSlotOffset2DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen11MipTailSlotOffset2DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_3D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod;

        *OffsetX = Gen11MipTailSlotOffset3DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen11MipTailSlotOffset3DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen11MipTailSlotOffset3DSurface[Slot][ArrayIndex].Z;
    }

    GMM_DPF_EXIT;
    return;
}

GMM_STATUS GmmLib::GmmGen11TextureCalc::FillLinearCCS(GMM_TEXTURE_INFO * pTexInfo,
                                                      __GMM_BUFFER_TYPE *pRestrictions)
{
    GMM_GFX_SIZE_T           PaddedSize;
    uint32_t                 TileHeight;
    GMM_GFX_SIZE_T           YCcsSize    = 0;
    GMM_GFX_SIZE_T           UVCcsSize   = 0;
    GMM_GFX_SIZE_T           TotalHeight = 0;
    const GMM_PLATFORM_INFO *pPlatform   = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);
    GMM_DPF_ENTER;


    __GMM_ASSERT(pTexInfo->Flags.Gpu.MMC &&
                 pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
                 pTexInfo->Flags.Gpu.__NonMsaaLinearCCS);

    __GMM_ASSERT(pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] > 0);
    TileHeight = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;

    // Vinante : CCS or Tile status buffer is computed by giving 2bit for every 256bytes of origional pixel data.
    // For YUV Planar surfaces, UV Plane follow immediately after Y plane. Y and UV surfaces have their own
    // control surfaces. So AuxSurf will be linear buffer with CCS for Y plane followed by CCS for UV plane.
    // Y and UV control surface must be 4kb base aligned and they store the control data for full tiles covering Y and UV
    // planes respectively.
    // GMM will also allocate cacheline aligned 64-byte to hold the LKF's software controlled media compression state.
    // GMM will calculate YAuxOffset, UVAuxOffset and MediaCompression State offset on the fly. Refer GmmResGetAuxSurfaceOffset().

    YCcsSize = pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] * pTexInfo->Pitch / 1024;
    YCcsSize = GFX_ALIGN(YCcsSize, PAGE_SIZE);

    if(pTexInfo->ArraySize > 1)
    {
        TotalHeight = pTexInfo->OffsetInfo.Plane.ArrayQPitch / pTexInfo->Pitch;
    }
    else
    {
        TotalHeight = pTexInfo->Size / pTexInfo->Pitch;
    }

    UVCcsSize = (TotalHeight - pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]) * pTexInfo->Pitch / 1024;
    UVCcsSize = GFX_ALIGN(UVCcsSize, PAGE_SIZE);

    pTexInfo->Size  = GFX_ALIGN(YCcsSize + UVCcsSize + GMM_MEDIA_COMPRESSION_STATE_SIZE, pRestrictions->MinAllocationSize);
    pTexInfo->Pitch = 0;

    //Store the dimension of linear surface in OffsetInfo.Plane.X.
    pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_Y] = YCcsSize;
    pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_U] =
    pTexInfo->OffsetInfo.Plane.X[GMM_PLANE_V] = UVCcsSize;

    pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_Y] =
    pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U] =
    pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_V] = 0;

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
            return GMM_ERROR;
        }
    }

    return GMM_SUCCESS;
    GMM_DPF_EXIT;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function will Setup a planar surface allocation.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions.
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmGen11TextureCalc::FillTexPlanar(GMM_TEXTURE_INFO * pTexInfo,
                                                                  __GMM_BUFFER_TYPE *pRestrictions)
{
    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    GMM_DPF_ENTER;
    uint32_t   WidthBytesPhysical, Height, YHeight, VHeight;
    uint32_t   AdjustedVHeight = 0;
    GMM_STATUS Status;
    bool       UVPacked = false;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);
    __GMM_ASSERT(!pTexInfo->Flags.Info.TiledW);

    pTexInfo->TileMode = TILE_NONE;

    WidthBytesPhysical = GFX_ULONG_CAST(pTexInfo->BaseWidth) * pTexInfo->BitsPerPixel >> 3;
    Height = VHeight = 0;

    YHeight = pTexInfo->BaseHeight;

    //[History]
    // When planar surfaces are tiled, there are HW alignment
    // restrictions about where the U and V planes can be located.
    // Prior to SURFACE_STATE.X/YOffset support, planes needed to start
    // on tile boundaries; with X/YOffset support, the alignment
    // restrictions were reduced (but not eliminated).
    //
    // Horizontal alignment is only an issue for IMC2/4 surfaces, since
    // the planes of all other formats are always on the left-edge.
    //
    // For IMC1/3 surfaces, we must ensure that both the U/V planes are
    // properly aligned--That is, both the YHeight and VHeight must be
    // properly aligned. For all other surfaces (since the U/V data
    // starts at a common vertical location) only YHeight must be
    // properly aligned.

    // [Current] :
    // For Tiled surfaces, even though h/w supports U and V plane alignment
    // at lower granularities GMM will align all the planes at Tiled boundary
    // to unify the implementation across all platforms and GMM will add
    // handling for removing the extra padding when UMDs request for ResCpuBlt
    // operations.
    // For Linear surfaces, GMM will continue to support minimal aligment restrictions

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
            else //Tiled
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
                WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical, 2); // If odd YWidth, pitch bumps-up to fit rounded-up U/V planes.
                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 2;
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
        uint32_t TileHeight = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        uint32_t TileWidth  = pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth;

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
            pTexInfo->Flags.Info.RedecribedPlanes = 1;
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
    // Very wide planar tiled planar formats do not support MMC pre gen11. All formats do not support
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

    //LKF specific Restrictions
    if(GFX_GET_CURRENT_PRODUCT(pPlatform->Platform) == IGFX_LAKEFIELD)
    {
        // If GMM fall backs TileY to Linear then reset the UnifiedAuxSurface flag.
        if(!pTexInfo->Flags.Gpu.MMC &&
           pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
           !pTexInfo->Flags.Gpu.__NonMsaaLinearCCS)
        {
            GMM_ASSERTDPF(0, "MMC TileY is fallback to Linear surface!");
            pTexInfo->Flags.Gpu.UnifiedAuxSurface = 0;
        }

        if(pTexInfo->Flags.Gpu.MMC &&
           pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
           pTexInfo->Flags.Gpu.__NonMsaaLinearCCS)
        {
            FillLinearCCS(pTexInfo, pRestrictions);
        }
    }

    GMM_DPF_EXIT;

    return GMM_SUCCESS;
}
