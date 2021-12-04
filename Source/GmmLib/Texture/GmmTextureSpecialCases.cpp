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
/// This function does any special-case conversion from client-provided pseudo creation
/// parameters to actual parameters for Hiz, CCS, SeparateStencil and Depth buffers.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmTextureCalc::PreProcessTexSpecialCases(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS               Status    = GMM_SUCCESS;
    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);

    if(!pTexInfo->Flags.Gpu.CCS &&
       !pTexInfo->Flags.Gpu.MCS &&
       !pTexInfo->Flags.Gpu.HiZ &&
       !pTexInfo->Flags.Gpu.SeparateStencil &&
       !pTexInfo->Flags.Gpu.MMC)
    {
        // Fast-out for non-special-cases.
    }
    else if(pTexInfo->Flags.Gpu.HiZ) // ######################################
    {
        // With HiZ surface creation, clients send the size/etc. parameters of
        // the associated Depth Buffer--and here we convert to the appropriate
        // HiZ creation parameters...

        if((pTexInfo->BaseWidth > 0) &&
           (pTexInfo->BaseWidth <= pPlatform->HiZ.MaxWidth) &&
           (pTexInfo->BaseHeight > 0) &&
           (pTexInfo->BaseHeight <= pPlatform->HiZ.MaxHeight) &&
           (pTexInfo->Depth <= ((pTexInfo->Type == RESOURCE_3D) ?
                                pPlatform->HiZ.MaxDepth :
                                1)) &&
           (pTexInfo->ArraySize <= ((pTexInfo->Type == RESOURCE_3D) ?
                                    1 :
                                    (pTexInfo->Type == RESOURCE_CUBE) ?
                                    pPlatform->HiZ.MaxArraySize / 6 :
                                    pPlatform->HiZ.MaxArraySize)) &&
           // SKL+ does not support HiZ surfaces for 1D and 3D surfaces
           ((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) < IGFX_GEN9_CORE) ||
            (pTexInfo->Type != RESOURCE_1D && pTexInfo->Type != RESOURCE_3D)))
        {
            uint32_t Z_Width, Z_Height, Z_Depth;

            // Latch Z_[Width/Height/Depth]...
            Z_Width  = GFX_ULONG_CAST(pTexInfo->BaseWidth);
            Z_Height = pTexInfo->BaseHeight;
            if((pTexInfo->Type == RESOURCE_1D) ||
               (pTexInfo->Type == RESOURCE_2D))
            {
                Z_Depth = GFX_MAX(pTexInfo->ArraySize, 1);
            }
            else if(pTexInfo->Type == RESOURCE_3D)
            {
                Z_Depth = pTexInfo->Depth;
            }
            else if(pTexInfo->Type == RESOURCE_CUBE)
            {
                // HW doesn't allow HiZ cube arrays, but GMM is allowing because
                // clients will redescribe depth/HiZ cube arrays as 2D arrays.
                Z_Depth = 6 * GFX_MAX(pTexInfo->ArraySize, 1);
            }
            else
            {
                __GMM_ASSERT(0); // Illegal--Should have caught at upper IF check.
                Z_Depth = 0;
            }

            // HZ_[Width/Height/QPitch] Calculation...
            {
                uint32_t h0, h1, hL, i, NumSamples, QPitch, Z_HeightL;
                uint32_t HZ_HAlign = 16, HZ_VAlign = 8;
                uint8_t  HZ_DepthRows = pPlatform->HiZPixelsPerByte;

                // HZ operates in pixel space starting from SKL. So, it does not care
                // whether the depth buffer is in MSAA mode or not.
                NumSamples =
                (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE) ?
                1 :
                pTexInfo->MSAA.NumSamples;

                pTexInfo->BaseWidth = ExpandWidth(Z_Width, HZ_HAlign, NumSamples);

                h0 = ExpandHeight(Z_Height, HZ_VAlign, NumSamples);

                Z_Height = GmmTexGetMipHeight(pTexInfo, 1);
                h1       = ExpandHeight(Z_Height, HZ_VAlign, NumSamples);

                if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN8_CORE)
                {
                    if(pTexInfo->Type == RESOURCE_3D)
                    {
                        for(i = 0, Z_HeightL = 0; i <= pTexInfo->MaxLod; i++)
                        {
                            Z_Height = GmmTexGetMipHeight(pTexInfo, i);
                            hL       = ExpandHeight(Z_Height, HZ_VAlign, NumSamples);
                            Z_HeightL += (hL * GFX_MAX(1, (Z_Depth / GFX_2_TO_POWER_OF(i))));
                        }

                        pTexInfo->ArraySize  = 0;
                        pTexInfo->BaseHeight = Z_HeightL / 2;
                    }
                    else
                    {
                        for(i = 2, Z_HeightL = 0; i <= pTexInfo->MaxLod; i++)
                        {
                            Z_Height = GmmTexGetMipHeight(pTexInfo, i);
                            Z_HeightL += ExpandHeight(Z_Height, HZ_VAlign, NumSamples);
                        }

                        QPitch =
                        (pTexInfo->MaxLod > 0) ?
                        (h0 + GFX_MAX(h1, Z_HeightL)) :
                        h0;
                        QPitch /= HZ_DepthRows;
                        pTexInfo->ArraySize  = Z_Depth;
                        pTexInfo->BaseHeight = QPitch;
                    }

                    pTexInfo->Alignment.HAlign = HZ_HAlign;
                    pTexInfo->Alignment.VAlign = HZ_VAlign / HZ_DepthRows;
                }
                else //if (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN7_CORE)
                {
                    if(pTexInfo->Type == RESOURCE_3D)
                    {
                        for(i = 0, Z_HeightL = 0; i <= pTexInfo->MaxLod; i++)
                        {
                            hL = ExpandHeight(Z_Height >> i, HZ_VAlign, NumSamples);
                            Z_HeightL += (hL * GFX_MAX(1, (Z_Depth / GFX_2_TO_POWER_OF(i))));
                        }

                        pTexInfo->BaseHeight = Z_HeightL / 2;
                    }
                    else
                    {
                        QPitch = h0 + h1 + 12 * HZ_VAlign;

                        pTexInfo->BaseHeight = GFX_CEIL_DIV((QPitch * Z_Depth / 2), 8) * 8;
                    }

                    pTexInfo->ArraySize = 1;
                }
            }

            /// Native HZ Params //////////////////////////////////////////////////
            pTexInfo->BitsPerPixel       = 8;
            pTexInfo->Depth              = 1;
            pTexInfo->Format             = GMM_FORMAT_GENERIC_8BIT;
            pTexInfo->MaxLod             = 0;
            pTexInfo->MSAA.NumSamples    = 1;
            pTexInfo->MSAA.SamplePattern = GMM_MSAA_DISABLED;
            pTexInfo->Type               = RESOURCE_2D;

            // HiZ Always Tile-Y
            pTexInfo->Flags.Info.Linear  = 0;
            pTexInfo->Flags.Info.TiledW  = 0;
            pTexInfo->Flags.Info.TiledX  = 0;
            pTexInfo->Flags.Info.TiledYf = 0;
            GMM_SET_64KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);
            GMM_SET_4KB_TILE(pTexInfo->Flags, 1, pGmmLibContext);

        }
        else
        {
            GMM_ASSERTDPF(0, "Illegal HiZ creation parameters!");
            Status = GMM_ERROR;
        }
    } // HiZ
    else if(pTexInfo->Flags.Gpu.CCS ||
            pTexInfo->Flags.Gpu.MCS) // ######################################
    {
        // With CCS surface creation, clients send height, width, depth, etc. of
        // the associated RenderTarget--and here we convert to the appropriate CCS
        // creation parameters...
        __GMM_ASSERT((!pGmmLibContext->GetSkuTable().FtrTileY ||
                      (pTexInfo->Flags.Info.Linear + pTexInfo->Flags.Info.TiledW + pTexInfo->Flags.Info.TiledX + pTexInfo->Flags.Info.TiledY)) == 1);

        __GMM_ASSERT((pGmmLibContext->GetSkuTable().FtrTileY || (pTexInfo->Flags.Info.Linear + pTexInfo->Flags.Info.Tile4 + pTexInfo->Flags.Info.Tile64)) == 1);

        __GMM_ASSERT((pTexInfo->MSAA.NumSamples == 1) || (pTexInfo->MSAA.NumSamples == 2) || (pTexInfo->MSAA.NumSamples == 4) ||
                     (pTexInfo->MSAA.NumSamples == 8) || (pTexInfo->MSAA.NumSamples == 16));

        Status = pGmmLibContext->GetTextureCalc()->MSAACCSUsage(pTexInfo);

        if(!pTexInfo->Flags.Gpu.__NonMsaaLinearCCS)
        {
            // CCS Always Tile-Y (Even for Non-MSAA FastClear.)
            pTexInfo->Flags.Info.Linear  = 0;
            pTexInfo->Flags.Info.TiledW  = 0;
            pTexInfo->Flags.Info.TiledX  = 0;
            pTexInfo->Flags.Info.TiledYf = 0;

            GMM_SET_64KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);
            GMM_SET_4KB_TILE(pTexInfo->Flags, 1, pGmmLibContext);

            //Clear compression request in CCS
            pTexInfo->Flags.Info.RenderCompressed = 0;
            pTexInfo->Flags.Info.MediaCompressed  = 0;
        }

    }                                            // CCS
    else if(pTexInfo->Flags.Gpu.SeparateStencil) // ##########################
    {
        // Seperate stencil sizing is based on the associated depth buffer
        // size, however UMD manages this sizing, and GMM will allocate any
        // arbitrarily sized stencil. Stencils do have specific tiling
        // requirements however, which is handled below.

        if((pTexInfo->BaseWidth > 0) &&
           (pTexInfo->BaseHeight > 0))
        {
            __GMM_ASSERT(pTexInfo->BitsPerPixel == 8);

            if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) < IGFX_GEN7_CORE)
            {
                GMM_ASSERTDPF((pTexInfo->MaxLod == 0), "Stencil Buffer LOD's not supported!");
            }

            if(pGmmLibContext->GetSkuTable().FtrTileY)
            {
                // Separate Stencil Tile-W Gen8-Gen11, otherwise Tile-Y
                pTexInfo->Flags.Info.Linear  = 0;
                pTexInfo->Flags.Info.TiledX  = 0;
                pTexInfo->Flags.Info.TiledYf = 0;
                pTexInfo->Flags.Info.TiledW  = 0;
                GMM_SET_4KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);
                GMM_SET_64KB_TILE(pTexInfo->Flags, 0, pGmmLibContext);

                if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN8_CORE &&
                   GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) <= IGFX_GEN11_CORE)
                {
                    pTexInfo->Flags.Info.TiledW = 1;
                }
                else
                {
                    GMM_SET_4KB_TILE(pTexInfo->Flags, 1, pGmmLibContext);
                }
            }
            else
            {
                __GMM_ASSERT(pTexInfo->Flags.Info.Tile4 + pTexInfo->Flags.Info.Tile64 == 1);
            }
        }
        else
        {
            GMM_ASSERTDPF(0, "Illegal Separate Stencil creation parameters!");
            Status = GMM_ERROR;
        }
    } // Separate Stencil
    else if(pTexInfo->Flags.Gpu.MMC && pTexInfo->Flags.Gpu.UnifiedAuxSurface)
    {
        pTexInfo->Flags.Gpu.__NonMsaaLinearCCS = 1;
        pTexInfo->Flags.Info.Linear            = 1;
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function performs rough estimate of memory requirement between 4KB Tile vs
/// 64KB Tile surfaces and if the memory wastage due to padding/alignment exceeds
/// configured threshold, then optimize to demote the surface to 4KB Tile.
///
/// @param[in]  pTexInfo: Reference to ::GMM_TEXTURE_INFO
///             returns 1 if optimization to demote to 4KB tile is required otherwise 0
///
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::GmmTextureCalc::SurfaceRequires64KBTileOptimization(GMM_TEXTURE_INFO *pTexInfo)
{
    GMM_STATUS               Status    = GMM_SUCCESS;
    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo, pGmmLibContext);
    uint32_t                 Size4KbTile, Size64KbTile;

    // Discard the surface if not eligible for 4KB Tile.
    // All YUV formats restricted with default Tile64 across clients
    if((pTexInfo->MSAA.NumSamples > 1) ||
       pTexInfo->Flags.Gpu.TiledResource ||
       pTexInfo->Flags.Gpu.HiZ ||
       (!pTexInfo->Flags.Info.Tile64))
    {
        return 0;
    }

    // Calc Surf size for 64KB Tile.
    // Ignoring the CCS/AuxSurf dimensions since its proportional to main surface size
    {
        GMM_TEXTURE_INFO Surf = {};
        uint32_t         ExpandedArraySize, BitsPerPixel;
        uint32_t         SliceHeight, SliceWidth, Pitch;
        uint32_t         BlockHeight = 0;
        uint32_t         HAlign, VAlign, DAlign, CompressHeight, CompressWidth, CompressDepth;

        Surf = *pTexInfo;

        //Get HAlign/VAlign
        if((Status = __GmmTexFillHAlignVAlign(&Surf, pGmmLibContext)) != GMM_SUCCESS)
        {
            __GMM_ASSERT(0);
            return 0;
        }

        HAlign = Surf.Alignment.HAlign;
        VAlign = Surf.Alignment.VAlign;
        DAlign = Surf.Alignment.DAlign;

        // Set Tile Mode
        SetTileMode(&Surf);
        BitsPerPixel = Surf.BitsPerPixel;

        ExpandedArraySize =
        GFX_MAX(Surf.ArraySize, 1) *
        ((Surf.Type == RESOURCE_CUBE) ? 6 : 1) *                   // Cubemaps simply 6-element, 2D arrays.
        ((Surf.Type == RESOURCE_3D) ? GFX_MAX(Surf.Depth, 1) : 1); // 3D's simply 2D arrays for sizing.

        if(GMM_IS_64KB_TILE(Surf.Flags))
        {
            ExpandedArraySize = GFX_CEIL_DIV(ExpandedArraySize, pPlatform->TileInfo[Surf.TileMode].LogicalTileDepth);
        }

        // For Mipped Surface, Approx SliceHeight = VAlign(Lod0Height) * Mipped ? 1.5 : 1;
        SliceHeight = GFX_ALIGN(Surf.BaseHeight, VAlign);

        if(Surf.MaxLod > 1)
        {
            SliceHeight = (GFX_ALIGN(Surf.BaseHeight, VAlign) * 3) / 2;
        }

        uint8_t Compress = GmmIsCompressed(pGmmLibContext, Surf.Format);
        GetCompressionBlockDimensions(Surf.Format, &CompressWidth, &CompressHeight, &CompressDepth);


        SliceWidth  = __GMM_EXPAND_WIDTH(this, GFX_ULONG_CAST(Surf.BaseWidth), HAlign, &Surf);
        BlockHeight = SliceHeight * ExpandedArraySize;

        if(Compress)
        {
            SliceWidth  = GFX_CEIL_DIV(SliceWidth, CompressWidth);
            BlockHeight = GFX_CEIL_DIV(BlockHeight, CompressHeight);
        }

        // Default pitch
        Pitch = SliceWidth * BitsPerPixel >> 3;

        if(GMM_IS_TILED(pPlatform->TileInfo[Surf.TileMode]))
        {
            Pitch       = GFX_ALIGN(Pitch, pPlatform->TileInfo[Surf.TileMode].LogicalTileWidth);
            BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[Surf.TileMode].LogicalTileHeight);
        }

        // Calculate Tile aligned size.
        Size64KbTile = BlockHeight * Pitch;

	if(pTexInfo->Type == RESOURCE_3D && !pTexInfo->Flags.Info.Linear)
        {
            Size64KbTile *= pPlatform->TileInfo[Surf.TileMode].LogicalTileDepth;
        }
    }

    // Calc Surf size for 4KB Tile
    // Ignoring the CCS/AuxSurf dimensions since its proportional to main surface size
    {
        GMM_TEXTURE_INFO Surf = {};
        uint32_t         ExpandedArraySize, BitsPerPixel;
        uint32_t         SliceHeight, SliceWidth, Pitch;
        uint32_t         BlockHeight = 0;
        uint32_t         HAlign, VAlign, DAlign, CompressHeight, CompressWidth, CompressDepth;

        Surf                   = *pTexInfo;
        Surf.Flags.Info.Tile4  = 1;
        Surf.Flags.Info.Tile64 = 0;

        //Get HAlign/VAlign
        if((Status = __GmmTexFillHAlignVAlign(&Surf, pGmmLibContext)) != GMM_SUCCESS)
        {
            Status = GMM_ERROR;
            return Status;
        }

        HAlign = Surf.Alignment.HAlign;
        VAlign = Surf.Alignment.VAlign;
        DAlign = Surf.Alignment.DAlign;

        // Set Tile Mode
        SetTileMode(&Surf);
        BitsPerPixel = Surf.BitsPerPixel;

        ExpandedArraySize =
        GFX_MAX(Surf.ArraySize, 1) *
        ((Surf.Type == RESOURCE_CUBE) ? 6 : 1) *                   // Cubemaps simply 6-element, 2D arrays.
        ((Surf.Type == RESOURCE_3D) ? GFX_MAX(Surf.Depth, 1) : 1); // 3D's simply 2D arrays for sizing.

        if(GMM_IS_64KB_TILE(Surf.Flags))
        {
            ExpandedArraySize = GFX_CEIL_DIV(ExpandedArraySize, pPlatform->TileInfo[Surf.TileMode].LogicalTileDepth);
        }

        // For Mipped Surface, Approx SliceHeight = VAlign(Lod0Height) * Mipped ? 1.5 : 1;
        SliceHeight = GFX_ALIGN(Surf.BaseHeight, VAlign);

        if(Surf.MaxLod > 1)
        {
            SliceHeight = (GFX_ALIGN(Surf.BaseHeight, VAlign) * 3) / 2;
        }

        uint8_t Compress = GmmIsCompressed(pGmmLibContext, Surf.Format);
        GetCompressionBlockDimensions(Surf.Format, &CompressWidth, &CompressHeight, &CompressDepth);

        SliceWidth  = __GMM_EXPAND_WIDTH(this, GFX_ULONG_CAST(Surf.BaseWidth), HAlign, &Surf);
        BlockHeight = SliceHeight * ExpandedArraySize;

        if(Compress)
        {
            SliceWidth  = GFX_CEIL_DIV(SliceWidth, CompressWidth);
            BlockHeight = GFX_CEIL_DIV(BlockHeight, CompressHeight);
        }

        // Default pitch
        Pitch = SliceWidth * BitsPerPixel >> 3;

        if(GMM_IS_TILED(pPlatform->TileInfo[Surf.TileMode]))
        {
            Pitch       = GFX_ALIGN(Pitch, pPlatform->TileInfo[Surf.TileMode].LogicalTileWidth);
            BlockHeight = GFX_ALIGN(BlockHeight, pPlatform->TileInfo[Surf.TileMode].LogicalTileHeight);
        }

        // Calculate Tile aligned size.
        Size4KbTile = BlockHeight * Pitch;
    }

    // check if 64KB tiled resource size exceeds memory wastage threshold.
    if(((Size4KbTile * (100 + (GMM_GFX_SIZE_T)pGmmLibContext->GetAllowedPaddingFor64KBTileSurf())) / 100) < Size64KbTile)
    {
        return 1;
    }

    return 0;
}
