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

#if (IGFX_GEN >= IGFX_GEN10)

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
                                                        uint32_t            MipLevel)
{
    uint32_t               ByteOffset = 0, Slot = 0xff;

    GMM_DPF_ENTER;

    // 3D textures follow the Gen9 mip tail format
    if(!pGmmGlobalContext->GetSkuTable().FtrStandardMipTailFormat ||
        pTexInfo->Type == RESOURCE_3D)
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
    }

    switch (Slot)
    {
        case  0:  ByteOffset = GMM_KBYTE(   32 );  break;
        case  1:  ByteOffset = GMM_KBYTE(   16 );  break;
        case  2:  ByteOffset = GMM_KBYTE(    8 );  break;
        case  3:  ByteOffset = GMM_KBYTE(    4 );  break;
        case  4:  ByteOffset = GMM_KBYTE(    2 );  break;
        case  5:  ByteOffset = GMM_BYTES( 1536 );  break;
        case  6:  ByteOffset = GMM_BYTES( 1280 );  break;
        case  7:  ByteOffset = GMM_BYTES( 1024 );  break;
        case  8:  ByteOffset = GMM_BYTES(  768 );  break;
        case  9:  ByteOffset = GMM_BYTES(  512 );  break;
        case 10:  ByteOffset = GMM_BYTES(  256 );  break;
        case 11:  ByteOffset = GMM_BYTES(  192 );  break;
        case 12:  ByteOffset = GMM_BYTES(  128 );  break;
        case 13:  ByteOffset = GMM_BYTES(   64 );  break;
        case 14:  ByteOffset = GMM_BYTES(    0 );  break;
        default:  __GMM_ASSERT(0);
    }

    GMM_DPF_EXIT;

    return(ByteOffset);
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
                                                           uint32_t            MipLevel,
                                                           uint32_t*           OffsetX,
                                                           uint32_t*           OffsetY,
                                                           uint32_t*           OffsetZ)
{
    uint32_t ArrayIndex = 0;
    uint32_t Slot = 0;

    GMM_DPF_ENTER;

    // 3D textures follow the Gen9 mip tail format
    if (!pGmmGlobalContext->GetSkuTable().FtrStandardMipTailFormat ||
        pTexInfo->Type == RESOURCE_3D)
    {
        return GmmGen9TextureCalc::GetMipTailGeometryOffset(pTexInfo, MipLevel, OffsetX, OffsetY, OffsetZ);
    }

    switch (pTexInfo->BitsPerPixel)
    {
    case 128: ArrayIndex = 0; break;
    case 64:  ArrayIndex = 1; break;
    case 32:  ArrayIndex = 2; break;
    case 16:  ArrayIndex = 3; break;
    case 8:   ArrayIndex = 4; break;
    default:
        __GMM_ASSERT(0);
        break;
    }

    if (pTexInfo->Type == RESOURCE_1D)
    {
        Slot = MipLevel - pTexInfo->Alignment.MipTailStartLod +
               (pTexInfo->Flags.Info.TiledYf ? 4 : 0);

        *OffsetX = Gen10MipTailSlotOffset1DSurface[Slot][ArrayIndex].X * pTexInfo->BitsPerPixel / 8;
        *OffsetY = Gen10MipTailSlotOffset1DSurface[Slot][ArrayIndex].Y;
        *OffsetZ = Gen10MipTailSlotOffset1DSurface[Slot][ArrayIndex].Z;
    }
    else if(pTexInfo->Type == RESOURCE_2D || pTexInfo->Type == RESOURCE_CUBE)
    {
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
uint32_t   GmmLib::GmmGen10TextureCalc::GetAligned3DBlockHeight(GMM_TEXTURE_INFO*    pTexInfo,
                                                                         uint32_t BlockHeight,
                                                                         uint32_t ExpandedArraySize)
{
    uint32_t               DAlign, CompressHeight, CompressWidth, CompressDepth;
    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, 0);

    const GMM_PLATFORM_INFO* pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

    DAlign = pTexInfo->Alignment.DAlign;

    GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

    if (pTexInfo->Type == RESOURCE_3D)
    {
        ExpandedArraySize = GFX_ALIGN_NP2(ExpandedArraySize, DAlign) / CompressDepth;

        if (!pTexInfo->Flags.Info.Linear)
        {
            BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
        }
    }

    GMM_DPF_EXIT;

    return BlockHeight;
}

#endif // #if (IGFX_GEN >= IGFX_GEN10)

