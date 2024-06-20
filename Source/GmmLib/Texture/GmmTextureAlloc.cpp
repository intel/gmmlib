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
/// This functions sets the Tile Mode of the graphics surface
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmTextureCalc::SetTileMode(GMM_TEXTURE_INFO *pTexInfo)
{
    const GMM_PLATFORM_INFO *pPlatform;

    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);
    
    pTexInfo->TileMode = TILE_NONE;

    if(pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags))
    {
// clang-format off
        #define SET_TILE_MODE(Tile, Submode)                                                \
        {                                                                                   \
                pTexInfo->TileMode =                                                        \
                    (pTexInfo->BitsPerPixel == 128) ? TILE_##Tile##_##Submode##_128bpe :    \
                    (pTexInfo->BitsPerPixel ==  64) ? TILE_##Tile##_##Submode##_64bpe  :    \
                    (pTexInfo->BitsPerPixel ==  32) ? TILE_##Tile##_##Submode##_32bpe  :    \
                    (pTexInfo->BitsPerPixel ==  16) ? TILE_##Tile##_##Submode##_16bpe  :    \
                                                      TILE_##Tile##_##Submode##_8bpe;       \
        }                                                                                   \

        #define GENERATE_TILE_MODE(T, M1d, M2d, M2d_2x, M2d_4x, M2d_8x, M2d_16x, M3d)            \
        {\
            switch (pTexInfo->Type)\
            {\
                case RESOURCE_1D:\
                    SET_TILE_MODE(T, M1d);\
                    break;\
                case RESOURCE_2D:\
                case RESOURCE_CUBE:\
                    switch (pTexInfo->MSAA.NumSamples)\
                    {\
                    case 1:\
                        SET_TILE_MODE(T, M2d);\
                        break;\
                    case 2:\
                        SET_TILE_MODE(T, M2d_2x);\
                        break;\
                    case 4:\
                        SET_TILE_MODE(T, M2d_4x);\
                        break;\
                    case 8:\
                        SET_TILE_MODE(T, M2d_8x);\
                        break;\
                    case 16:\
                        SET_TILE_MODE(T, M2d_16x);\
                        break;\
                    default:\
                        __GMM_ASSERT(0);\
                    }\
                    break;\
                case RESOURCE_3D:\
                    SET_TILE_MODE(T, M3d);\
                    break;\
                default:\
                    __GMM_ASSERT(0);\
            }\
        }


        // clang-format on
        if(pTexInfo->Flags.Info.TiledYf)
        {
            GENERATE_TILE_MODE(YF, 1D, 2D, 2D_2X, 2D_4X, 2D_8X, 2D_16X, 3D);

            pTexInfo->Flags.Info.TiledYf = 1;
            pTexInfo->Flags.Info.TiledYs = 0;
        }
        else
        {
            if(pGmmLibContext->GetSkuTable().FtrTileY)
            {
                GENERATE_TILE_MODE(YS, 1D, 2D, 2D_2X, 2D_4X, 2D_8X, 2D_16X, 3D);
            }
            else
            {
                if (pGmmLibContext->GetSkuTable().FtrXe2PlusTiling)
                {
                    GENERATE_TILE_MODE(_64, 1D, 2D, 2D_2X, 2D_4X, 2D_8X, 2D_16X, 3D);
                }
                else
                {
                    GENERATE_TILE_MODE(_64, 1D, 2D, 2D_2X, 2D_4X, 2D_4X, 2D_4X, 3D);
                }
            }

            pTexInfo->Flags.Info.TiledYf = 0;
            GMM_SET_64KB_TILE(pTexInfo->Flags, 1, pGmmLibContext);
        }


        GMM_SET_4KB_TILE(pTexInfo->Flags, pGmmLibContext->GetSkuTable().FtrTileY ? 1 : 0, pGmmLibContext);

        pTexInfo->Flags.Info.TiledX = 0;
        pTexInfo->Flags.Info.TiledW = 0;
        pTexInfo->Flags.Info.Linear = 0;
#undef SET_TILE_MODE
    }
    else if(GMM_IS_4KB_TILE(pTexInfo->Flags))
    {
        GMM_SET_4KB_TILE(pTexInfo->Flags, 1, pGmmLibContext);
        pTexInfo->Flags.Info.TiledYf = 0;
        pTexInfo->Flags.Info.TiledYs = 0;
        pTexInfo->Flags.Info.TiledX  = 0;
        pTexInfo->Flags.Info.TiledW  = 0;
        pTexInfo->Flags.Info.Linear  = 0;
        GMM_SET_4KB_TILE_MODE(pTexInfo->TileMode, pGmmLibContext);
    }
    else if(pTexInfo->Flags.Info.TiledX)
    {
        pTexInfo->Flags.Info.TiledY  = 0;
        pTexInfo->Flags.Info.TiledYf = 0;
        pTexInfo->Flags.Info.TiledYs = 0;
        pTexInfo->Flags.Info.TiledX  = 1;
        pTexInfo->Flags.Info.TiledW  = 0;
        pTexInfo->Flags.Info.Linear  = 0;
        pTexInfo->TileMode           = LEGACY_TILE_X;
    }
    else if(pTexInfo->Flags.Info.TiledW)
    {
        pTexInfo->Flags.Info.TiledY  = 0;
        pTexInfo->Flags.Info.TiledYf = 0;
        pTexInfo->Flags.Info.TiledYs = 0;
        pTexInfo->Flags.Info.TiledX  = 0;
        pTexInfo->Flags.Info.TiledW  = 1;
        pTexInfo->Flags.Info.Linear  = 0;
        pTexInfo->TileMode           = LEGACY_TILE_Y;
    }
    else if(pTexInfo->Flags.Info.Linear)
    {
        pTexInfo->Flags.Info.TiledY  = 0;
        pTexInfo->Flags.Info.TiledYf = 0;
        pTexInfo->Flags.Info.TiledYs = 0;
        pTexInfo->Flags.Info.TiledX  = 0;
        pTexInfo->Flags.Info.TiledW  = 0;
        pTexInfo->Flags.Info.Linear  = 1;
        pTexInfo->TileMode           = TILE_NONE;
    }
    else
    {
        GMM_ASSERTDPF(0, "No tiling preference set!");
    }
    
    GMM_ASSERTDPF(pTexInfo->TileMode < GMM_TILE_MODES, "Invalid Tile Mode Set");
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for allocating a mip map or planar surface. The function
/// outputs offset, size and pitch information by enforcing all the h/w alignment
/// and restrictions.
///
/// @param[in]  pTexInfo: Reference to GMM_TEXTURE_INFO
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
#if(defined(__GMM_KMD__))
GMM_STATUS GmmTexAlloc(GMM_LIB_CONTEXT *pGmmLibContext, GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_TEXTURE_CALC *pTextureCalc = pGmmLibContext->GetTextureCalc();
    return (pTextureCalc->AllocateTexture(pTexInfo));
}

GMM_STATUS GmmTexLinearCCS(GMM_LIB_CONTEXT *pGmmLibContext, GMM_TEXTURE_INFO *pTexInfo, GMM_TEXTURE_INFO *pAuxTexInfo)
{
    GMM_TEXTURE_CALC *pTextureCalc = pGmmLibContext->GetTextureCalc();
    return (pTextureCalc->FillTexCCS(pTexInfo, pAuxTexInfo));
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// Top level function for allocating a mip map or planar surface. The function
/// outputs offset, size and pitch information by enforcing all the h/w alignment
/// and restrictions.
///
/// @param[in]  pTexInfo: Reference to GMM_TEXTURE_INFO
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::AllocateTexture(GMM_TEXTURE_INFO *pTexInfo)
{
    __GMM_BUFFER_TYPE Restrictions = {0};
    GMM_STATUS        Status;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pGmmLibContext, GMM_ERROR);

    GMM_DPF_ENTER;

    GetTexRestrictions(pTexInfo, &Restrictions);

    if((Status = __GmmTexFillHAlignVAlign(pTexInfo, pGmmLibContext)) != GMM_SUCCESS)
    {
        return Status;
    }

    // Planar YUV resources treated special. Packed YUV treated like 2D/3D/Cube...
    if(GmmIsPlanar(pTexInfo->Format))
    {
        Status = FillTexPlanar(pTexInfo, &Restrictions);

        if((Status == GMM_SUCCESS) &&
           (ValidateTexInfo(pTexInfo, &Restrictions) == false))
        {
            return GMM_ERROR;
        }
        if(GMM_SUCCESS != FillTexCCS(pTexInfo, pTexInfo))
        {
            return GMM_ERROR;
        }
        return Status;
    }
    else
    {
        SetTileMode(pTexInfo);
    }

    switch(pTexInfo->Type)
    {
        case RESOURCE_2D:
        case RESOURCE_PRIMARY:
        case RESOURCE_SHADOW:
        case RESOURCE_STAGING:
        case RESOURCE_GDI:
        case RESOURCE_NNDI:
        case RESOURCE_HARDWARE_MBM:
        case RESOURCE_OVERLAY_INTERMEDIATE_SURFACE:
        case RESOURCE_IFFS_MAPTOGTT:
#if _WIN32
        case RESOURCE_WGBOX_ENCODE_DISPLAY:
        case RESOURCE_WGBOX_ENCODE_REFERENCE:
#endif
        {
            Status = FillTex2D(pTexInfo, &Restrictions);

            break;
        }
        case RESOURCE_1D:
        {
            Status = FillTex1D(pTexInfo, &Restrictions);

            break;
        }
        case RESOURCE_3D:
        {
            Status = FillTex3D(pTexInfo, &Restrictions);

            break;
        }
        case RESOURCE_CUBE:
        {
            Status = FillTexCube(pTexInfo, &Restrictions);

            break;
        }
        case RESOURCE_SCRATCH:
        case RESOURCE_BUFFER:
        case RESOURCE_FBC:
        case RESOURCE_PWR_CONTEXT:
        case RESOURCE_KMD_BUFFER:
        case RESOURCE_NULL_CONTEXT_INDIRECT_STATE:
        case RESOURCE_PERF_DATA_QUEUE:
        case RESOURCE_HW_CONTEXT:
        case RESOURCE_TAG_PAGE:
        case RESOURCE_OVERLAY_DMA:
        case RESOURCE_GTT_TRANSFER_REGION:
        case RESOURCE_GLOBAL_BUFFER:
        case RESOURCE_CURSOR:
        case RESOURCE_GFX_CLIENT_BUFFER:
#if _WIN32
        case RESOURCE_WGBOX_ENCODE_STATE:
        case RESOURCE_WGBOX_ENCODE_TFD:
#endif
        {
            Status = FillTexBlockMem(pTexInfo, &Restrictions);
            break;
        }
        default:
        {
            GMM_ASSERTDPF(0, "GmmTexAlloc: Unknown surface type!");
            return GMM_INVALIDPARAM;
        }
    };

    if(ValidateTexInfo(pTexInfo, &Restrictions) == false)
    {
        return GMM_ERROR;
    }

    if(GMM_SUCCESS != FillTexCCS(pTexInfo, pTexInfo))
    {
        return GMM_ERROR;
    }

    return Status;
}

GMM_STATUS GmmLib::GmmTextureCalc::FillTexCCS(GMM_TEXTURE_INFO *pBaseSurf, GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_UNREFERENCED_PARAMETER(pBaseSurf);
    GMM_UNREFERENCED_PARAMETER(pTexInfo);
    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function will validate pTexInfo to make sure all the surface creation
/// parameters are valid.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions
///
/// @return     true/false
/////////////////////////////////////////////////////////////////////////////////////
bool GmmLib::GmmTextureCalc::ValidateTexInfo(GMM_TEXTURE_INFO * pTexInfo,
                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    __GMM_ASSERTPTR(pTexInfo, false);
    __GMM_ASSERTPTR(pRestrictions, false);

    GMM_DPF_ENTER;

    if(pTexInfo->Pitch > pRestrictions->MaxPitch)
    {
        GMM_ASSERTDPF(0,
                      "GmmLib::GmmTextureCalc::ValidateTexInfo: Pitch"
                      "exceeds max HW pitch restriction.\r\n");
        return false;
    }

    GMM_DPF_EXIT;
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Sets the tiling  type based on the required alignment parameters.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  WidthBytesPhysical: Width in bytes of the surface
/// @param[in]  Height: Height of the surface
/// @param[in]  pBufferType: Reference to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::FillTexPitchAndSize(GMM_TEXTURE_INFO * pTexInfo,
                                                       GMM_GFX_SIZE_T     WidthBytesPhysical,
                                                       uint32_t           Height,
                                                       __GMM_BUFFER_TYPE *pBufferType)
{
    GMM_STATUS     Status           = GMM_SUCCESS;
    GMM_GFX_SIZE_T WidthBytesRender = 0;
    GMM_GFX_SIZE_T WidthBytesLock   = 0;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pBufferType, GMM_ERROR);

    GMM_DPF_ENTER;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    // Make sure that we meet the minimum HW requirment for that buffer type
    WidthBytesPhysical = GFX_MAX(WidthBytesPhysical, pBufferType->MinPitch);

    if(pTexInfo->TileMode >= GMM_TILE_MODES)
    {
        GMM_ASSERTDPF(0, "Invalid parameter!");
        return GMM_ERROR;
    }

    if(GMM_ISNOT_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        pTexInfo->LegacyFlags |= GMM_LINEAR;

        // For linear surace we need to make sure that physical pitch
        // meet the HW alignment (i.e DWORD or QWORD, ETC)
        WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical,
                                       pBufferType->PitchAlignment);

        WidthBytesRender = WidthBytesPhysical;
        WidthBytesLock   = WidthBytesPhysical;
    }
    else
    {
        if(pTexInfo->Flags.Info.TiledY ||
           pTexInfo->Flags.Info.TiledYf ||
           pTexInfo->Flags.Info.TiledYs)
        {
            pTexInfo->LegacyFlags |= GMM_TILE_Y;
        }
        else if(pTexInfo->Flags.Info.TiledX == 1)
        {
            pTexInfo->LegacyFlags |= GMM_TILE_X;
        }
        else if(pTexInfo->Flags.Info.TiledW == 1)
        {
            pTexInfo->LegacyFlags |= GMM_TILE_W;
        }

        // Align Height to tile height boundary
        Height = GFX_ALIGN(Height, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);

        // Align Width to next tile boundary
        WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical,
                                       pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth);

        if(pTexInfo->Flags.Info.RenderCompressed || pTexInfo->Flags.Info.MediaCompressed)
        {
            if(!GMM_IS_64KB_TILE(pTexInfo->Flags) && !pGmmLibContext->GetSkuTable().FtrFlatPhysCCS) //Ys is naturally aligned to required 4 YF pages
            {
                // Align Pitch to 4-tile boundary
                WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical,
                                               4 * pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth);
            }
        }

        // Calculate Alignment Restriction for rendering on the surface
        // NOTE:
        //  WidthBytesPhysical == true physical pitch used to determine amount
        //                        of Pages need for a surface
        //  WidthBytesRender == HW require pitch of a surface for rendering
        //                      (i.e. power2
        //  WidthBytesLock == Pitch when a surface is visible via Fence region.

        WidthBytesRender = WidthBytesLock = WidthBytesPhysical;

        // Align pitch to meet our HW requirment for each buffer
        WidthBytesRender = GFX_ALIGN(WidthBytesRender,
                                     pBufferType->RenderPitchAlignment);

        // Media Memory Compression : Allocate one memory tile wider than is required...
        pGmmLibContext->GetTextureCalc()->AllocateOneTileThanRequied(pTexInfo, WidthBytesRender,
                                                                        WidthBytesPhysical, WidthBytesLock);

        // check if locking a particular suface need to be power 2 or not
        if(pBufferType->NeedPow2LockAlignment)
        {
            WidthBytesLock = GFX_POW2_SIZE(WidthBytesPhysical);
        }

        // Align pitch to meet our HW requirment for each buffer
        // [1] 8K lock pitch is needed on Gen3 when we internally remap the
        //     display surface in GmmGetDisplayStartAddress ( ). Gen4,
        //     we don't remap due to Persurface tiling and stick to 64byte
        //     lock pitch alignment.
        WidthBytesLock = GFX_ALIGN(WidthBytesLock,
                                   pBufferType->LockPitchAlignment);

        if((pTexInfo->Type == RESOURCE_PRIMARY) || pTexInfo->Flags.Gpu.FlipChain)
        {
            // [2] At creation time, we tell OS the Render size, not
            //     SurfaceSizePhysical like other surfaces. Therefore, we change
            //     the SurfaceSizePhysical to match render size for simplicity.
            WidthBytesPhysical = WidthBytesRender;
        }

        if(pGmmLibContext->GetWaTable().WaMsaa8xTileYDepthPitchAlignment &&
           (pTexInfo->MSAA.NumSamples == 8) &&
           GMM_IS_4KB_TILE(pTexInfo->Flags) &&
           pTexInfo->Flags.Gpu.Depth)
        {
            WidthBytesLock =
            WidthBytesRender =
            WidthBytesPhysical = GFX_ALIGN(WidthBytesLock, GMM_BYTES(256));
        }
    }

    __GMM_ASSERT(WidthBytesLock == WidthBytesPhysical &&
                 WidthBytesRender == WidthBytesPhysical &&
                 WidthBytesLock == WidthBytesRender);
    pTexInfo->Pitch = WidthBytesLock;

    //VirtualPadding override
    if(pTexInfo->Flags.Info.AllowVirtualPadding &&
       pTexInfo->OverridePitch)
    {
        pTexInfo->Pitch = pTexInfo->OverridePitch;
    }

    // When lossless compression is enabled with plane width greater than 3840 and
    // horizontal panning, the surface pitch should be a multiple of 4 tiles. Since
    // GMM doesn't know about lossless compression status at allocation time, here
    // we apply the WA to all unified aux surfaces.
    if(pGmmLibContext->GetWaTable().WaLosslessCompressionSurfaceStride &&
       pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
       (pTexInfo->BaseWidth > 3840))
    {
        pTexInfo->Pitch = GFX_ALIGN(pTexInfo->Pitch, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth * 4);
    }

    // If FBC is enabled with a linear surface, the surface pitch should be a multiple of
    // 8 cache lines (512 bytes). Since GMM doesn't know about FBC status, here we apply
    // the WA to all linear surfaces.
    // Xadapter surfaces has to be 128 Bytes aligned and hence we don't want this 512B alignment
    // for Xadapter. Eventually FBC will be disabled in case of Xadapter Linear surfaces
    if(pGmmLibContext->GetSkuTable().FtrFbc &&
       pGmmLibContext->GetWaTable().WaFbcLinearSurfaceStride &&
       pTexInfo->Flags.Gpu.FlipChain &&
       pTexInfo->Flags.Info.Linear &&
       !pTexInfo->Flags.Info.XAdapter)
    {
        if(pTexInfo->Flags.Gpu.FlipChainPreferred)
        {
            // Moderate down displayable flags if input parameters (.FlipChainPrefered)
            // deprioritise it, over Pitch alignement in this case.
            pTexInfo->Flags.Gpu.FlipChain = __GMM_IS_ALIGN(pTexInfo->Pitch, 512);
        }
        else
        {
            pTexInfo->Pitch = GFX_ALIGN(pTexInfo->Pitch, 512);
        }
    }

    // For CCS Aux Display Surf the surface stride should not exceed 8 times the LogicalTileWidth.
    if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs && pTexInfo->Flags.Gpu.FlipChain &&
       (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE))
    {
        __GMM_ASSERT(pTexInfo->Pitch <= (pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth * 8));
        pTexInfo->Pitch = GFX_MIN(pTexInfo->Pitch, pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth * 8);
    }

    if(pTexInfo->Flags.Gpu.CCS && pTexInfo->Flags.Gpu.__NonMsaaTileYCcs &&
       (pTexInfo->Pitch > pPlatform->TexAlign.CCS.MaxPitchinTiles * pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth))
    {
        GMM_ASSERTDPF(0, "Aux Surface pitch too large!");
        Status = GMM_ERROR;
    }
    
    if(pGmmLibContext->GetWaTable().Wa_15010089951)
    {
        // Default Tiling is set to Tile64 on FtrTileY disabled platforms
        uint8_t IsYUVSurface = ((GmmIsPlanar(pTexInfo->Format) &&
                                 (!((pTexInfo->Format == GMM_FORMAT_BGRP) || (pTexInfo->Format == GMM_FORMAT_RGBP)))) ||
                                (GmmIsYUVPacked(pTexInfo->Format) &&
                                 !((pTexInfo->Format == GMM_FORMAT_YVYU_2x1) || (pTexInfo->Format == GMM_FORMAT_UYVY_2x1) || (pTexInfo->Format == GMM_FORMAT_UYVY_2x1))));

        //YCRCB* formats
        uint8_t IsYCrCbSurface = ((pTexInfo->Format == GMM_FORMAT_YCRCB_NORMAL) ||
                                  (pTexInfo->Format == GMM_FORMAT_YCRCB_SWAPUV) ||
                                  (pTexInfo->Format == GMM_FORMAT_YCRCB_SWAPUVY) || (pTexInfo->Format == GMM_FORMAT_YCRCB_SWAPY));

        // Allocation needs to extend an extra tile in width when pitch is not an odd multiplication
        // of tile width which is 128 for Tile4 (YUV allocation is forced as Tile4).
        if(pTexInfo->Flags.Info.Tile4 && (IsYUVSurface || IsYCrCbSurface) &&
           ((pTexInfo->Pitch / (pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth)) % 2 == 0))
        {
            pTexInfo->Pitch = (pTexInfo->Pitch + (pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth));
        }
    }

    // For NV12 Linear FlipChain surfaces, UV plane distance should be 4k Aligned.
    // Hence make the stride to align to 4k, so that UV distance will be 4k aligned.
    if(pGmmLibContext->GetWaTable().Wa4kAlignUVOffsetNV12LinearSurface &&
       (pTexInfo->Format == GMM_FORMAT_NV12 || GmmIsP0xx(pTexInfo->Format)) && pTexInfo->Flags.Info.Linear &&
       (!pTexInfo->Flags.Info.XAdapter) &&
       ((pTexInfo->Type == RESOURCE_PRIMARY) || pTexInfo->Flags.Gpu.FlipChain))
    {
        if(pTexInfo->Flags.Gpu.FlipChainPreferred)
        {
            // Moderate down displayable flags if input parameters (.FlipChainPrefered)
            // deprioritise it, over Pitch alignement in this case.
            pTexInfo->Flags.Gpu.FlipChain = __GMM_IS_ALIGN(pTexInfo->Pitch, GMM_KBYTE(4));
        }
        else
        {
            pTexInfo->Pitch = GFX_ALIGN(pTexInfo->Pitch, GMM_KBYTE(4));
        }
    }

    if((GFX_GET_CURRENT_PRODUCT(pPlatform->Platform) >= IGFX_METEORLAKE))
    {
        pTexInfo->OffsetInfo.PlaneXe_LPG.PhysicalPitch = pTexInfo->Pitch;
    }

    { // Surface Sizes
        int64_t Size;

        if(pTexInfo->Flags.Gpu.S3d)
        {
            if(pGmmLibContext->GetSkuTable().FtrDisplayEngineS3d) // BDW+ Display Engine S3D (Tiled)
            {
                __GMM_ASSERT(!pTexInfo->Flags.Info.Linear);

                pTexInfo->S3d.BlankAreaOffset = 0;

                if(pTexInfo->Flags.Gpu.S3dDx && (pTexInfo->ArraySize == 2))
                {
                    pTexInfo->S3d.RFrameOffset     = GFX_ULONG_CAST(pTexInfo->Pitch * pTexInfo->Alignment.QPitch);
                    pTexInfo->S3d.TallBufferHeight = Height;
                }
                else
                {
                    if(pTexInfo->Flags.Gpu.Overlay)
                    {
                        pTexInfo->S3d.RFrameOffset = GFX_ULONG_CAST(pTexInfo->Pitch * Height);

                        Height = pTexInfo->S3d.TallBufferHeight = Height * 2;
                    }
                    else if(pTexInfo->Flags.Gpu.FlipChain)
                    {
                        pTexInfo->S3d.RFrameOffset     = 0;
                        pTexInfo->S3d.TallBufferHeight = Height;
                    }
                    else
                    {
                        // Something must be wrong. Not an S3D resource!
                        __GMM_ASSERT(0);
                    }
                }

                __GMM_ASSERT(__GMM_IS_ALIGN(pTexInfo->S3d.RFrameOffset, PAGE_SIZE));
            }
            else if(pTexInfo->Flags.Gpu.S3dDx) // DX S3D (Tiled)
            {
                __GMM_ASSERT(!pTexInfo->Flags.Info.Linear || !pTexInfo->Flags.Gpu.Overlay);
                __GMM_ASSERT(pTexInfo->ArraySize <= 1); // S3D framebuffer arrays are not supported (pre-BDW).

                pTexInfo->S3d.BlankAreaOffset = GFX_ULONG_CAST(pTexInfo->Pitch * pTexInfo->BaseHeight);

                pTexInfo->S3d.RFrameOffset =
                GFX_ULONG_CAST(pTexInfo->Pitch *
                               (pTexInfo->S3d.DisplayModeHeight + pTexInfo->S3d.NumBlankActiveLines));

                Height =
                pTexInfo->S3d.TallBufferHeight =
                GFX_ALIGN(
                (pTexInfo->BaseHeight * 2) + pTexInfo->S3d.NumBlankActiveLines,
                pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileHeight);
            }
            else // Legacy S3D
            {
                __GMM_ASSERT(pTexInfo->Flags.Info.Linear);

                pTexInfo->S3d.BlankAreaOffset = GFX_ULONG_CAST(pTexInfo->Pitch * pTexInfo->BaseHeight);

                pTexInfo->S3d.RFrameOffset =
                GFX_ULONG_CAST(pTexInfo->Pitch *
                               (pTexInfo->S3d.DisplayModeHeight + pTexInfo->S3d.NumBlankActiveLines));

                if(pTexInfo->Flags.Gpu.Overlay)
                {
                    Height =
                    pTexInfo->S3d.TallBufferHeight =
                    pTexInfo->BaseHeight +
                    pTexInfo->S3d.NumBlankActiveLines +
                    pTexInfo->S3d.DisplayModeHeight;
                }
                else if(pTexInfo->Flags.Gpu.FlipChain)
                {
                    __GMM_ASSERT(pTexInfo->S3d.DisplayModeHeight == pTexInfo->BaseHeight);

                    pTexInfo->S3d.TallBufferHeight =
                    (pTexInfo->BaseHeight * 2) +
                    pTexInfo->S3d.NumBlankActiveLines;
                }
                else
                {
                    // Something must be wrong. Not an S3D resource!
                    __GMM_ASSERT(0);
                }

                __GMM_ASSERT(__GMM_IS_ALIGN(pTexInfo->S3d.RFrameOffset, PAGE_SIZE));
                __GMM_ASSERT(__GMM_IS_ALIGN(pTexInfo->S3d.BlankAreaOffset, PAGE_SIZE));
            }

            // Calculate surface size (physical).
            Size = pTexInfo->Pitch * Height;

            // Calculate tall buffer size (virtual).
            pTexInfo->S3d.TallBufferSize = GFX_ULONG_CAST(pTexInfo->Pitch * pTexInfo->S3d.TallBufferHeight);
        }
        else
        {
            Size = (int64_t)pTexInfo->Pitch * Height;

            if(pTexInfo->Type == RESOURCE_3D && !pTexInfo->Flags.Info.Linear)
            {
                Size *= pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileDepth;
            }

            if((pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags)) &&
               (pTexInfo->MSAA.NumSamples > 1) &&
               (pTexInfo->Flags.Gpu.Depth == 0 && pTexInfo->Flags.Gpu.SeparateStencil == 0))
            {
                // For color buffer (meaning not depth or stencil buffer)
                // The width/height for TileYf/Ys MSAA surfaces are not expanded (using GmmExpandWidth/Height functions)
                // because pitch for these surfaces is in their non-expanded dimensions. So, the pitch
                // is also non-expanded units.  That's why, we multiply by the sample size here to get the correct size.
                if(pGmmLibContext->GetSkuTable().FtrTileY)
                {
                    Size *= pTexInfo->MSAA.NumSamples;
                }
                else
                {
                    //XeHP, DG2
                    if (!pGmmLibContext->GetSkuTable().FtrXe2PlusTiling && (pTexInfo->MSAA.NumSamples == 8 || pTexInfo->MSAA.NumSamples == 16))
                    {
                        uint64_t SliceSize = pTexInfo->Pitch * Height;
                        SliceSize *= 4; // multiple by samples per tile
                        Size = (int64_t)SliceSize;
                    }
                    else
                    {
                        Size *= pTexInfo->MSAA.NumSamples;
                    }
                }
            }

            if((pTexInfo->Flags.Info.TiledY && pTexInfo->Flags.Gpu.TiledResource))
            {
                //Pad align surface to 64KB ie Tile size
                Size = GFX_ALIGN(Size, GMM_KBYTE(64));
            }

            if (pGmmLibContext->GetSkuTable().FtrXe2Compression && pTexInfo->Flags.Info.Linear)
            {
                Size = GFX_ALIGN(Size, GMM_BYTES(256)); // for all linear resources starting Xe2, align overall size to compression block size. For subresources, 256B alignment is not needed, needed only for overall resource
                                                        // on older platforms, all linear resources get Halign = 128B which ensures overall size to be a multiple of compression block size of 128B,
                                                        // so this is needed only for linear resources on Xe2 where HAlign continues to be at 128B, but compression block size has doubled to 256B
            }

            // Buffer Sampler Padding...
            if((pTexInfo->Type == RESOURCE_BUFFER) &&
               pGmmLibContext->GetWaTable().WaNoMinimizedTrivialSurfacePadding &&
               !pTexInfo->Flags.Wa.NoBufferSamplerPadding &&
               !pTexInfo->Flags.Info.ExistingSysMem && // <-- Currently using separate padding WA in OCL (and rarity/luck in other UMD's).
               // <-- Never sampled from.
               !pTexInfo->Flags.Gpu.Query &&
               !pTexInfo->Flags.Gpu.HistoryBuffer &&
               !pTexInfo->Flags.Gpu.State &&
               !pTexInfo->Flags.Gpu.StateDx9ConstantBuffer)
            // These can be sampled from, so they need the padding...
            // pTexInfo->Flags.Gpu.Constant
            // pTexInfo->Flags.Gpu.Index
            // pTexInfo->Flags.Gpu.Stream
            // pTexInfo->Flags.Gpu.Vertex

            {
                uint32_t BufferSizeAlignment;
                uint32_t BufferSizePadding;

                // SURFTYPE_BUFFER's that can be sampled from must have their size
                // padded to a multiple of 256 buffer elements and then have an
                // additional 16 bytes of padding beyond that. Currently, the GMM
                // doesn't receive a buffer's element type/size, so (until that's
                // revamped) we'll assume the worst-case of 128-bit elements--which
                // means padding to 256 * 128 / 8 = 4KB and then adding 16 bytes.
                // In the case of BDW:A0, size is padded to a multiple of 512 buffer
                // elements instead of 256--which means padding to 8KB.

                BufferSizeAlignment =
                (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN8_CORE) ?
                8192 :
                4096;

                BufferSizePadding = 16;

                Size = GFX_ALIGN(Size, BufferSizeAlignment) + BufferSizePadding;
            }

            // HiZ Clear Color requires some small data at the end of the allocation to
            // store the color data.
            if(pTexInfo->Flags.Gpu.HiZ && pTexInfo->Flags.Gpu.IndirectClearColor)
            {
                Size += GMM_HIZ_CLEAR_COLOR_SIZE;
            }

            if(pTexInfo->Flags.Info.ExistingSysMem &&
               !pTexInfo->ExistingSysMem.IsGmmAllocated &&
               !pTexInfo->ExistingSysMem.IsPageAligned)
            {
                // Do not modify Size
            }
            else
            {
                Size = GFX_ALIGN(Size, PAGE_SIZE);
            }
        }

        int64_t SurfaceMaxSize = 0;

        if(pTexInfo->Flags.Gpu.NoRestriction)
        {
            SurfaceMaxSize = pPlatform->NoRestriction.MaxWidth;
        }
        else if(pTexInfo->Flags.Gpu.TiledResource)
        {
            SurfaceMaxSize = GMM_TBYTE(1);
        }
        else
        {
            SurfaceMaxSize = pPlatform->SurfaceMaxSize;
        }

        if(Size <= SurfaceMaxSize)
        {
            pTexInfo->Size = Size;
        }
        else
        {
#if defined(__GMM_KMD__) || defined(__linux__)
            GMM_ASSERTDPF(0, "Surface too large!");
#endif
            Status = GMM_ERROR;
        }
    }

    {
        uint64_t TotalAlignment = (((uint64_t)((uint32_t)(pTexInfo->Alignment.BaseAlignment))) * ((uint32_t)(pBufferType->Alignment)));

        if(!pTexInfo->Alignment.BaseAlignment || __GMM_IS_ALIGN(pBufferType->Alignment, pTexInfo->Alignment.BaseAlignment))
        {
            pTexInfo->Alignment.BaseAlignment = pBufferType->Alignment;
        }
        else if(__GMM_IS_ALIGN(pTexInfo->Alignment.BaseAlignment, pBufferType->Alignment))
        {
            // Do nothing: pTexInfo->Alignment.BaseAlignment is properly alighned
        }
        else if(TotalAlignment > 0xFFFFFFFF)
        {
            GMM_ASSERTDPF(0, "Client requested alignment is too high, failing the allocation to match HW requiremnets. \r\n");
            Status = GMM_ERROR;
        }
        else
        {
            pTexInfo->Alignment.BaseAlignment = pTexInfo->Alignment.BaseAlignment * pBufferType->Alignment;
            GMM_ASSERTDPF(0,
                          "Client requested alignment that is not properly aligned to HW requirements."
                          "Alignment is going to be much higher to match both client and HW requirements.\r\n");
        }
    }

    if((pTexInfo->Flags.Gpu.TilePool && (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE)) ||
       (pTexInfo->Flags.Info.Undefined64KBSwizzle) || GMM_IS_64KB_TILE(pTexInfo->Flags))
    {
        pTexInfo->Alignment.BaseAlignment = (GFX_IS_ALIGNED(pTexInfo->Alignment.BaseAlignment, GMM_KBYTE(64))) ? pTexInfo->Alignment.BaseAlignment : GMM_KBYTE(64);
    }

    if(pGmmLibContext->GetWaTable().WaCompressedResourceRequiresConstVA21 && pTexInfo->Flags.Gpu.MMC)
    {
        pTexInfo->Alignment.BaseAlignment = GMM_MBYTE(4);
    }

    GMM_DPF_EXIT;

    return (Status);
} // FillTexPitchAndSize

/////////////////////////////////////////////////////////////////////////////////////
/// This function will Setup a planar surface allocation.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions.
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmTextureCalc::FillTexPlanar(GMM_TEXTURE_INFO * pTexInfo,
                                                             __GMM_BUFFER_TYPE *pRestrictions)
{
    uint32_t   WidthBytesPhysical, Height, YHeight, VHeight;
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

                Height = YHeight + 2 * VHeight;

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

                Height = YHeight + 2 * VHeight;

                pTexInfo->OffsetInfo.Plane.NoOfPlanes = 3;
            }
            else
            {
                YHeight = GFX_ALIGN(YHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);
                VHeight = YHeight;

                Height = YHeight + 2 * VHeight;

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
            YHeight = GFX_ALIGN(pTexInfo->BaseHeight, __GMM_EVEN_ROW);
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
            pTexInfo->Flags.Info.TiledY           = 0;
            pTexInfo->Flags.Info.TiledYf          = 0;
            pTexInfo->Flags.Info.TiledYs          = 0;
            pTexInfo->Flags.Info.TiledX           = 0;
            pTexInfo->Flags.Info.Linear           = 1;
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

    // For Tiled Planar surfaces, the planes must be tile-boundary aligned.
    // Actual alignment is handled in FillPlanarOffsetAddress, but height
    // and width must be adjusted for correct size calculation
    if(GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]))
    {
        uint32_t TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;
        uint32_t TileWidth  = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileWidth;

        pTexInfo->OffsetInfo.Plane.IsTileAlignedPlanes = true;

        Height = GFX_ALIGN(YHeight, TileHeight) + (GFX_ALIGN(VHeight, TileHeight) * (UVPacked ? 1 : 2));

        if(pTexInfo->Format == GMM_FORMAT_IMC2 || // IMC2, IMC4 needs even tile columns
           pTexInfo->Format == GMM_FORMAT_IMC4)
        {
            // If the UV planes are packed then the surface pitch must be
            // padded out so that the tile-aligned UV data will fit.
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

    //Special case LKF MMC compressed surfaces
    if(pTexInfo->Flags.Gpu.MMC &&
       pTexInfo->Flags.Gpu.UnifiedAuxSurface &&
       pTexInfo->Flags.Info.TiledY)
    {
        uint32_t TileHeight = pGmmLibContext->GetPlatformInfo().TileInfo[pTexInfo->TileMode].LogicalTileHeight;

        Height = GFX_ALIGN(YHeight, TileHeight) + GFX_ALIGN(VHeight, TileHeight);
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

/////////////////////////////////////////////////////////////////////////////////////
/// This function calculates the size and pitch for the linear buffer based on h/w
/// alignment and size restrictions.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::FillTexBlockMem(GMM_TEXTURE_INFO * pTexInfo,
                                                   __GMM_BUFFER_TYPE *pRestrictions)
{
    GMM_GFX_SIZE_T WidthBytesPhysical;
    uint32_t       BitsPerPixel;
    GMM_STATUS     Status;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pRestrictions, GMM_ERROR);
    __GMM_ASSERT(pTexInfo->BitsPerPixel == GMM_BITS(8) || (pTexInfo->Flags.Info.AllowVirtualPadding));
    __GMM_ASSERT(pTexInfo->BaseHeight == 1);
    __GMM_ASSERT(pTexInfo->Flags.Info.Linear == 1);
    __GMM_ASSERT(pTexInfo->Flags.Info.TiledW == 0);
    __GMM_ASSERT(pTexInfo->Flags.Info.TiledX == 0);
    __GMM_ASSERT(pTexInfo->Flags.Info.TiledY == 0);
    __GMM_ASSERT(pTexInfo->Flags.Info.TiledYf == 0);
    __GMM_ASSERT(pTexInfo->Flags.Info.TiledYs == 0);

    GMM_DPF_ENTER;

    // Width interpreted in bytes.
    BitsPerPixel       = pTexInfo->BitsPerPixel;
    WidthBytesPhysical = pTexInfo->BaseWidth * BitsPerPixel >> 3;

    Status = GMM_SUCCESS;

    // Clients can allocate Buffers and Structured Buffers by specifying either
    // total size (in BaseWidth) or as an array of structs with the ArraySize
    // and BaseWidth parameters (where BaseWidth = size of the arrayed struct).
    if((pTexInfo->Type == RESOURCE_BUFFER) &&
       (pTexInfo->ArraySize > 1))
    {
        uint64_t __WidthBytesPhysical = WidthBytesPhysical;

        __WidthBytesPhysical *= pTexInfo->ArraySize;

        if(__WidthBytesPhysical <= pRestrictions->MaxPitch)
        {
            WidthBytesPhysical = (GMM_GFX_SIZE_T)__WidthBytesPhysical;
        }
        else
        {
            GMM_ASSERTDPF(0, "Surface too large!");
            Status = GMM_ERROR;
        }
    }

    if(Status == GMM_SUCCESS)
    {
        // Make sure minimum width and alignment is met.
        WidthBytesPhysical = GFX_MAX(WidthBytesPhysical, pRestrictions->MinPitch);
        WidthBytesPhysical = GFX_ALIGN(WidthBytesPhysical, pRestrictions->PitchAlignment);

        Status = FillTexPitchAndSize(pTexInfo, WidthBytesPhysical, pTexInfo->BaseHeight, pRestrictions);
    }

    GMM_DPF_EXIT;
    return (Status);
}


/////////////////////////////////////////////////////////////////////////////////////
/// This function does any special-case conversion from client-provided pseudo creation
/// parameters to actual parameters for CCS.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///
///  @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmTextureCalc::MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS Status = GMM_SUCCESS;
    //const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);

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
           ((pTexInfo->MaxLod == 0) &&
            (pTexInfo->ArraySize <= 1)) &&
           (((pTexInfo->BitsPerPixel == 32) ||
             (pTexInfo->BitsPerPixel == 64) ||
             (pTexInfo->BitsPerPixel == 128))))
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

            uint32_t AlignmentFactor = pGmmLibContext->GetWaTable().WaDoubleFastClearWidthAlignment ? 2 : 1;

            pTexInfo->BaseWidth    = pTexInfo->BaseWidth * pTexInfo->BitsPerPixel / 8;
            pTexInfo->BitsPerPixel = 8;
            pTexInfo->Format       = GMM_FORMAT_R8_UINT;

            if(GMM_IS_4KB_TILE(pTexInfo->Flags)) //-------- Fast Clear Granularity
            {                                    //                       /--- RT:CCS Sizing Downscale
                pTexInfo->BaseWidth  = GFX_ALIGN(pTexInfo->BaseWidth, 512 * AlignmentFactor) / 32;
                pTexInfo->BaseHeight = GFX_ALIGN(pTexInfo->BaseHeight, 128) / 32;
            }
            else //if(pTexInfo->Flags.Info.TiledX)
            {
                pTexInfo->BaseWidth  = GFX_ALIGN(pTexInfo->BaseWidth, 1024 * AlignmentFactor) / 64;
                pTexInfo->BaseHeight = GFX_ALIGN(pTexInfo->BaseHeight, 64) / 16;
            }
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
/// This function does any special-case conversion from client-provided pseudo creation
/// parameters to actual parameters for CCS for MSAA Compression.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///
///  @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::MSAACompression(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS Status = GMM_SUCCESS;

    if((pTexInfo->MSAA.NumSamples == 2) || (pTexInfo->MSAA.NumSamples == 4))
    {
        pTexInfo->BitsPerPixel = 8;
        pTexInfo->Format       = GMM_FORMAT_R8_UINT;
    }
    else if(pTexInfo->MSAA.NumSamples == 8)
    {
        pTexInfo->BitsPerPixel = 32;
        pTexInfo->Format       = GMM_FORMAT_R32_UINT;
    }
    else //if(pTexInfo->MSAA.NumSamples == 16)
    {
        pTexInfo->BitsPerPixel = 64;
        pTexInfo->Format       = GMM_FORMAT_GENERIC_64BIT;
    }

    if((Status = __GmmTexFillHAlignVAlign(pTexInfo, pGmmLibContext)) != GMM_SUCCESS) // Need to get our alignment (matching RT) before overwriting our RT's MSAA setting.
    {
        return Status;
    }
    pTexInfo->MSAA.NumSamples         = 1; // CCS itself isn't MSAA'ed.
    pTexInfo->Flags.Gpu.__MsaaTileMcs = 1;

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
///Allocate one memory tile wider than is required for Media Memory Compression
///
/// @param[in]  See function definition.
///
/// @return     ::
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmTextureCalc::AllocateOneTileThanRequied(GMM_TEXTURE_INFO *pTexInfo,
                                                                    GMM_GFX_SIZE_T &  WidthBytesRender,
                                                                    GMM_GFX_SIZE_T &  WidthBytesPhysical,
                                                                    GMM_GFX_SIZE_T &  WidthBytesLock)
{
    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(pTexInfo->Flags.Gpu.MMC && !pTexInfo->Flags.Gpu.UnifiedAuxSurface)
    {
        WidthBytesRender += pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileWidth;
        WidthBytesPhysical = WidthBytesLock = WidthBytesRender;
    }
}
