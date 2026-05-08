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
/// Copies parameters or sets flags based on info sent by the client.
///
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
/// @return     false if encountered invalid param. true otherwise.
/////////////////////////////////////////////////////////////////////////////////////
bool GmmLib::GmmResourceInfoCommon::CopyClientParams(GMM_RESCREATE_PARAMS &CreateParams)
{
    uint32_t BitsPerPixel;
    uint8_t  Optimize64KBTile = 0;

    if((CreateParams.Format > GMM_FORMAT_INVALID) &&
       (CreateParams.Format < GMM_RESOURCE_FORMATS))
    {
        BitsPerPixel = GetGmmLibContext()->GetPlatformInfo().FormatTable[CreateParams.Format].Element.BitsPer;
    }
    else
    {
        GMM_ASSERTDPF(0, "Format Error");
        return false;
    }
    {
        if ((GetGmmLibContext()->GetSkuTable().FtrXe2Compression) &&
            (CreateParams.Type == RESOURCE_BUFFER) &&
            (CreateParams.Flags.Info.Linear) &&
            (CreateParams.Flags.Gpu.FlipChain))
        {
            CreateParams.Flags.Info.Linear = false;
            CreateParams.Flags.Info.Tile4  = true;
        }
	
	// Promote tiling options if caller does not provide any.
        // X/Y/W/L are tiling formats, and Yf/Ys are modifiers to the internal
        // ordering for Y and L macro-formats.
        if((CreateParams.Flags.Info.Linear +
            CreateParams.Flags.Info.TiledW +
            CreateParams.Flags.Info.TiledX +
            CreateParams.Flags.Info.TiledY) == 0)
        {
            if(CreateParams.Type == RESOURCE_1D ||
               CreateParams.Type == RESOURCE_BUFFER ||
               CreateParams.Type == RESOURCE_SCRATCH ||
               CreateParams.Flags.Info.ExistingSysMem)
            {
                CreateParams.Flags.Info.Linear = true;

                if ((GetGmmLibContext()->GetSkuTable().FtrXe2Compression) &&
                    CreateParams.Flags.Gpu.FlipChain && (CreateParams.Type == RESOURCE_BUFFER))
                {
                    CreateParams.Flags.Info.Linear = false;
                    CreateParams.Flags.Info.Tile4  = true;
                }
	    }

            if(GetGmmLibContext()->GetSkuTable().FtrTileY)
            {

                CreateParams.Flags.Info.TiledYs |= CreateParams.Flags.Info.StdSwizzle || CreateParams.Flags.Gpu.TiledResource;

                // Propose L+Y by default.
                CreateParams.Flags.Info.Linear = true;
                CreateParams.Flags.Info.TiledY = true;

                // Pre-Gen11 Planar
                if(GmmIsPlanar(CreateParams.Format) && (GFX_GET_CURRENT_RENDERCORE(GetGmmLibContext()->GetPlatformInfo().Platform) < IGFX_GEN11_CORE))
                {
                    CreateParams.Flags.Info.TiledX = true;
                }
            }
            //Auto-tiling selection if not Linear already
            else if(CreateParams.Flags.Info.Linear == 0)
             {
                // Xe_HP onwards.
                if((CreateParams.Flags.Info.TiledYs +
                    CreateParams.Flags.Info.TiledYf +
                    CreateParams.Flags.Info.Tile4 +
                    CreateParams.Flags.Info.Tile64) == 0)
                {
                    GMM_ASSERTDPF(!CreateParams.Flags.Info.StdSwizzle, "StdSwizzle not supported on current platform");

                    if(!GetGmmLibContext()->GetSkuTable().FtrForceTile4) // FtrForceTile4 should never be set by default, used for debug purpose only
                    {
                        // Default Tiling is set to Tile64 on FtrTileY disabled platforms
                        uint8_t IsYUVSurface = GmmIsPlanar(CreateParams.Format) ||
                                                (GmmIsYUVPacked(CreateParams.Format));

                        //YCRCB* formats
                        uint8_t IsYCrCbSurface = ((CreateParams.Format == GMM_FORMAT_YCRCB_NORMAL) ||
                                                  (CreateParams.Format == GMM_FORMAT_YCRCB_SWAPUV) ||
                                                  (CreateParams.Format == GMM_FORMAT_YCRCB_SWAPUVY) || (CreateParams.Format == GMM_FORMAT_YCRCB_SWAPY));

			CreateParams.Flags.Info.Tile4 = ((!GMM_IS_SUPPORTED_BPP_ON_TILE_64_YF_YS(BitsPerPixel)) ||            // 24,48,96 bpps are not supported on Tile64, Tile4 is bpp independent
                                                         ((CreateParams.Type == RESOURCE_3D) && (CreateParams.Flags.Gpu.Depth || CreateParams.Flags.Gpu.SeparateStencil)) ||
                                                         ((!GetGmmLibContext()->GetSkuTable().FtrDisplayDisabled) &&
                                                          (CreateParams.Flags.Gpu.FlipChain || CreateParams.Flags.Gpu.Overlay)
                                                          ) ||
							  IsYUVSurface || IsYCrCbSurface);

			CreateParams.Flags.Info.Tile64 = !CreateParams.Flags.Info.Tile4;
                        // Optimize only when GMM makes tiling decision on behalf of UMD clients.
                        // Defering the memory calculations until GMM_TEXTURE_INFO  is available.
                        if(CreateParams.Flags.Info.Tile64)
                        {
                            Optimize64KBTile = 1;
                        }
		    }
                    else
                    {
                        CreateParams.Flags.Info.Tile64 = (CreateParams.MSAA.NumSamples > 1) || CreateParams.Flags.Gpu.TiledResource; // Colour & Depth/Stencil(IMS) MSAA should use Tile64
                        CreateParams.Flags.Info.Tile4  = !CreateParams.Flags.Info.Tile64;
                    }
                }
                else if((CreateParams.Flags.Info.TiledYs +
                         CreateParams.Flags.Info.TiledYf) > 0)
                {
                    GMM_ASSERTDPF(0, "Tile Yf/Ys not supported on given platform");

                    // Overrides the flags.
                    if(GetGmmLibContext()->GetSkuTable().FtrForceTile4)// FtrForceTile4 should never be set by default, used for debug purpose only.
                    {
                        CreateParams.Flags.Info.Tile64 = CreateParams.Flags.Info.TiledYs ||
                                                         (CreateParams.MSAA.NumSamples > 1) || CreateParams.Flags.Gpu.TiledResource; // Colour & Depth/Stencil(IMS) MSAA should use Tile64

                        CreateParams.Flags.Info.Tile4   = !CreateParams.Flags.Info.Tile64;
                        CreateParams.Flags.Info.TiledYf = 0;
                        CreateParams.Flags.Info.TiledYs = 0;
                    }
                }
            }
        }
        //Convert non linear & non-tiledX tiling selection by client to proper tiling.
        else if(CreateParams.Flags.Info.Linear + CreateParams.Flags.Info.TiledX == 0)
         {
            if(!GetGmmLibContext()->GetSkuTable().FtrTileY)
            {
                __GMM_ASSERT(!(CreateParams.Flags.Info.TiledYs ||
                               CreateParams.Flags.Info.TiledYf ||
                               CreateParams.Flags.Info.TiledY));

                // On Xe_HP onwards translate UMD's TileY/TileYs request to Tile4/Tile64 respectively
                // Exclude TileX, Linear from override
                if((GetGmmLibContext()->GetSkuTable().FtrForceTile4) && (CreateParams.Flags.Info.TiledYs || CreateParams.Flags.Info.TiledY))
                {
                    CreateParams.Flags.Info.Tile64 =
                    CreateParams.Flags.Info.TiledYs ||
                    (CreateParams.MSAA.NumSamples > 1) || CreateParams.Flags.Gpu.TiledResource;

                    CreateParams.Flags.Info.Tile4 = !CreateParams.Flags.Info.Tile64;

                    CreateParams.Flags.Info.TiledY  = 0;
                    CreateParams.Flags.Info.TiledYs = 0;
                    CreateParams.Flags.Info.TiledW  = 0;
                    CreateParams.Flags.Info.TiledYf = 0;
                }

                // Displayable surfaces cannot be Tiled4/64.
                __GMM_ASSERT(!GetGmmLibContext()->GetSkuTable().FtrDisplayYTiling);
		
		if (GFX_GET_CURRENT_RENDERCORE(GetGmmLibContext()->GetPlatformInfo().Platform) >= IGFX_XE3_CORE)
                {
                    if (CreateParams.Flags.Gpu.FlipChain || CreateParams.Flags.Gpu.Overlay ||
                        CreateParams.Flags.Gpu.Presentable)
                    {
                        if (CreateParams.Flags.Info.TiledX == 1)
                        {
                            CreateParams.Flags.Info.TiledX = 0;
                            CreateParams.Flags.Info.Tile4  = 1;
                        }
                    }
                }

                //override displayable surfaces to TileX
                if(GetGmmLibContext()->GetSkuTable().FtrDisplayXTiling)
                {
                    if(CreateParams.Flags.Gpu.FlipChain || CreateParams.Flags.Gpu.Overlay ||
                       CreateParams.Flags.Gpu.Presentable)
                    {
                        CreateParams.Flags.Info.TiledX = 1;
                        CreateParams.Flags.Info.TiledY = 0;
                        CreateParams.Flags.Info.Tile4  = 0;
                        CreateParams.Flags.Info.Tile64 = 0;
                    }
                }
            }
        }

        //ExistingSysMem allocations must be Linear
        __GMM_ASSERT(!CreateParams.Flags.Info.ExistingSysMem ||
                     CreateParams.Flags.Info.Linear);
    }

    if(GetGmmLibContext()->GetSkuTable().FtrMultiTileArch)
    {
        // For Naive apps, UMD does not populate multi tile arch params.
        // Gmm will populate them based on the kmd assigned tile to the umd process
        if(!CreateParams.MultiTileArch.Enable)
        {
            uint32_t GpuTile = 0;
            __GMM_ASSERT(CreateParams.MultiTileArch.GpuVaMappingSet == 0);
            __GMM_ASSERT(CreateParams.MultiTileArch.LocalMemEligibilitySet == 0);
            __GMM_ASSERT(CreateParams.MultiTileArch.LocalMemPreferredSet == 0);

            __GMM_ASSERT(GetGmmLibContext()->GetSkuTable().FtrAssignedGpuTile < 4);

#if !__GMM_KMD__
            GpuTile                                    = GetGmmLibContext()->GetSkuTable().FtrAssignedGpuTile;
            CreateParams.MultiTileArch.GpuVaMappingSet = __BIT(GpuTile);
#else
            GpuTile                                    = 0;
            CreateParams.MultiTileArch.GpuVaMappingSet = GetGmmLibContext()->GetGtSysInfo()->MultiTileArchInfo.TileMask;
#endif

            CreateParams.MultiTileArch.Enable = true;

            if(!CreateParams.Flags.Info.NonLocalOnly)
            {
                CreateParams.MultiTileArch.LocalMemEligibilitySet = __BIT(GpuTile);
                CreateParams.MultiTileArch.LocalMemPreferredSet   = __BIT(GpuTile);
            }
        }
    }

    Surf.Type                      = CreateParams.Type;
    Surf.Format                    = CreateParams.Format;
    Surf.BaseWidth                 = CreateParams.BaseWidth64;
    Surf.BaseHeight                = CreateParams.BaseHeight;
    Surf.Depth                     = CreateParams.Depth;
    Surf.MaxLod                    = CreateParams.MaxLod;
    Surf.ArraySize                 = CreateParams.ArraySize;
    Surf.Flags                     = CreateParams.Flags;
    Surf.MSAA                      = CreateParams.MSAA;
    Surf.Alignment.BaseAlignment   = CreateParams.BaseAlignment;
    Surf.CachePolicy.Usage         = CreateParams.Usage;
    Surf.MSAA.NumSamples           = GFX_MAX(Surf.MSAA.NumSamples, 1);
    Surf.MaximumRenamingListLength = CreateParams.MaximumRenamingListLength;
    Surf.OverridePitch             = CreateParams.OverridePitch;
    Surf.CpTag                     = CreateParams.CpTag;

    Surf.Flags.Info.__PreWddm2SVM =
    Surf.Flags.Info.SVM &&
    !(GetGmmLibContext()->GetSkuTable().FtrWddm2GpuMmu ||
      GetGmmLibContext()->GetSkuTable().FtrWddm2Svm);
    
    if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression && 
		    !(ONE_WAY_COHERENT_COMPRESSION_MODE(GetGmmLibContext()->GetPlatformInfo().Platform.eProductFamily, GetGmmLibContext()->GetWaTable().WaNoCpuCoherentCompression)))
    {
        if (!CreateParams.Flags.Info.NotCompressed && CreateParams.Flags.Info.Cacheable)
        {
            // Disable compression
            Surf.Flags.Info.NotCompressed     = 1;
            Surf.Flags.Gpu.CCS                = 0;
            Surf.Flags.Gpu.UnifiedAuxSurface  = 0;
            Surf.Flags.Gpu.IndirectClearColor = 0;
            Surf.Flags.Gpu.MCS                = 0;
        }
    }    

#if !__GMM_KMD__ && LHDM
    if(GetGmmLibContext()->GetWaTable().WaLLCCachingUnsupported)
    {
        Surf.Flags.Info.GttMapType = (CreateParams.Flags.Info.Cacheable) ?
                                     GMM_GTT_CACHETYPE_VLV_SNOOPED :
                                     GMM_GTT_CACHETYPE_UNCACHED;
    }

    if(GetGmmLibContext()->GetSkuTable().FtrCameraCaptureCaching == false &&
       CreateParams.Flags.Gpu.CameraCapture)
    {
        Surf.Flags.Info.Cacheable = 0;
    }
    Surf.Flags.Wa.ForceStdAllocAlign = 0;
#endif

#if(_DEBUG || _RELEASE_INTERNAL)
    Surf.Platform = GetGmmLibContext()->GetPlatformInfo().Platform;
#endif

    Surf.BitsPerPixel = BitsPerPixel;

    // Get pTextureCalc after surface evaluation
    GMM_TEXTURE_CALC *pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    GetGmmLibContext()->GetPlatformInfoObj()->SetCCSFlag(this->GetResFlags());

    // Moderate down displayable flags if input parameters are not conducive.
    // Reject non displayable tiling modes
    if(Surf.Flags.Gpu.FlipChain || Surf.Flags.Gpu.Overlay)
    {
        if(Surf.Flags.Info.TiledY && !GetGmmLibContext()->GetSkuTable().FtrDisplayYTiling)
        {
            if(Surf.Flags.Gpu.FlipChainPreferred)
            {
                Surf.Flags.Gpu.Overlay   = 0;
                Surf.Flags.Gpu.FlipChain = 0;
            }
            else
            {
                GMM_ASSERTDPF(0, "Y-Tiling mode not displayable");
                //return false;
            }
        }
    }

    // Memory optimization for 64KB tiled Surface.
    if (!GetGmmLibContext()->GetSkuTable().FtrTileY)
    {
        if ((GetGmmLibContext()->GetSkuTable().FtrTile64Optimization) && Optimize64KBTile)
        {
            if ((GetGmmLibContext()->GetWaTable().Wa_14020040029) && (Surf.Flags.Gpu.Depth))
            {
                // if SW uses Tile4 merely to reduce surface size for Depth buffers,
                // then use Tile64 instead
                GMM_SET_64KB_TILE(Surf.Flags, 1, GetGmmLibContext());
                GMM_SET_4KB_TILE(Surf.Flags, 0, GetGmmLibContext());

                //Also update CreateParams, if client reuses the modified struct, it'd see final tile-selection by Gmm.
                //Gmm's auto-tile-selection & tile-mode for size-optimization doesn't work for explicit tile-selection
                GMM_SET_64KB_TILE(CreateParams.Flags, 1, GetGmmLibContext());
                GMM_SET_4KB_TILE(CreateParams.Flags, 0, GetGmmLibContext());
            }
            else
            {
                if (pTextureCalc->SurfaceRequires64KBTileOptimization(&Surf))
                {
                    GMM_SET_64KB_TILE(Surf.Flags, 0, GetGmmLibContext());
                    GMM_SET_4KB_TILE(Surf.Flags, 1, GetGmmLibContext());

                    //Also update CreateParams, if client reuses the modified struct, it'd see final tile-selection by Gmm.
                    //Gmm's auto-tile-selection & tile-mode for size-optimization doesn't work for explicit tile-selection
                    GMM_SET_64KB_TILE(CreateParams.Flags, 0, GetGmmLibContext());
                    GMM_SET_4KB_TILE(CreateParams.Flags, 1, GetGmmLibContext());
                }
            }
        }
    }

    // Convert Any Pseudo Creation Params to Actual...
    if(Surf.Flags.Gpu.UnifiedAuxSurface)
    {
        AuxSurf = Surf;

        if(Surf.Flags.Gpu.Depth && Surf.Flags.Gpu.CCS) //Depth + HiZ+CCS
        {
            //GMM_ASSERTDPF(Surf.Flags.Gpu.HiZ, "Lossless Z compression supported when Depth+HiZ+CCS is unified");
            AuxSecSurf                           = Surf;
            AuxSecSurf.Type                      = GetGmmLibContext()->GetSkuTable().FtrFlatPhysCCS ? RESOURCE_INVALID : AuxSecSurf.Type;
            Surf.Flags.Gpu.HiZ                   = 0; //Its depth buffer, so clear HiZ
            AuxSecSurf.Flags.Gpu.HiZ             = 0;
            AuxSurf.Flags.Gpu.IndirectClearColor = 0; //Clear Depth flags from HiZ, contained with separate/legacy HiZ when Depth isn't compressible.
            AuxSurf.Flags.Gpu.CCS                = 0;
            AuxSurf.Type                         = (AuxSurf.Flags.Gpu.HiZ) ? AuxSurf.Type : RESOURCE_INVALID;
            AuxSurf.Flags.Info.RenderCompressed = AuxSurf.Flags.Info.MediaCompressed = 0;
            AuxSurf.Flags.Info.NotCompressed                                         = 1;
        }
        else if(Surf.Flags.Gpu.Depth && Surf.Flags.Gpu.HiZ && !Surf.Flags.Gpu.CCS) // Depth + HiZ only, CCS is disabled
        {
            // main surface is depth, AuxSurf is HiZ
            Surf.Flags.Gpu.HiZ                   = 0; //depth buffer, clear HiZ
            AuxSurf.Flags.Gpu.IndirectClearColor = 0; //Clear fastClear from HiZ
        }
	else if(Surf.Flags.Gpu.SeparateStencil && Surf.Flags.Gpu.CCS) //Stencil compression
        {
            AuxSurf.Flags.Gpu.SeparateStencil = 0;
            Surf.Flags.Gpu.CCS                = 0;
            if(GMM_SUCCESS != pTextureCalc->PreProcessTexSpecialCases(&Surf))
            {
                return false;
            }
            Surf.Flags.Gpu.CCS = 1;
            AuxSurf.Type       = GetGmmLibContext()->GetSkuTable().FtrFlatPhysCCS ? RESOURCE_INVALID : AuxSurf.Type;
        }
        else if(Surf.MSAA.NumSamples > 1 && Surf.Flags.Gpu.CCS) //MSAA+MCS+CCS
        {
            GMM_ASSERTDPF(Surf.Flags.Gpu.MCS, "Lossless MSAA supported when MSAA+MCS+CCS is unified");
            AuxSecSurf                          = Surf;
            AuxSecSurf.Type                     = GetGmmLibContext()->GetSkuTable().FtrFlatPhysCCS ? RESOURCE_INVALID : AuxSecSurf.Type;
            AuxSecSurf.Flags.Gpu.MCS            = 0;
            AuxSurf.Flags.Gpu.CCS               = 0;
            AuxSurf.Flags.Info.RenderCompressed = AuxSurf.Flags.Info.MediaCompressed = 0;
            AuxSurf.Flags.Info.NotCompressed                                         = 1;
        }
        else if(Surf.Flags.Gpu.CCS)
        {
            AuxSurf.Type = (GetGmmLibContext()->GetSkuTable().FtrFlatPhysCCS && !Surf.Flags.Gpu.ProceduralTexture) ? RESOURCE_INVALID : AuxSurf.Type;
        }

        if(AuxSurf.Type != RESOURCE_INVALID &&
           GMM_SUCCESS != pTextureCalc->PreProcessTexSpecialCases(&AuxSurf))
        {
            return false;
        }
        if(AuxSecSurf.Type != RESOURCE_INVALID &&
           GMM_SUCCESS != pTextureCalc->PreProcessTexSpecialCases(&AuxSecSurf))
        {
            return false;
        }
    }
    else
    {
        if(GMM_SUCCESS != pTextureCalc->PreProcessTexSpecialCases(&Surf))
        {
            return false;
        }
    }

    RotateInfo = CreateParams.RotateInfo;

    if(GetGmmLibContext()->GetSkuTable().FtrMultiTileArch)
    {
        MultiTileArch = CreateParams.MultiTileArch;
    }

    // For Xe2 RenderCompressed and MediaCompressed to be unset
    if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
    {
        //Deny compression
        Surf.Flags.Info.RenderCompressed = 0;
        Surf.Flags.Info.MediaCompressed  = 0;
    }
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Validates the parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::ValidateParams()
{
    __GMM_BUFFER_TYPE              Restrictions             = {0};
    const __GMM_PLATFORM_RESOURCE *pPlatformResource        = NULL;
    GMM_TEXTURE_CALC *             pTextureCalc             = NULL;
    bool                           AllowMaxWidthViolations  = false;
    bool                           AllowMaxHeightViolations = false;
    uint8_t                        Status                   = 0;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(GetGmmLibContext(), 0);

#if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
    //KMD Debug and Release Internal Drivers only
    //if GMM_TEXTURE_INFO's platform type != native platform
    //then initialize the OverridePlatformInfo.

    //If GMM_TEXTURE_INFO's platform type == IGFX_UNKNOWN_CORE then it means that UMD driver is
    //in "Release" version and passes all zeros in Surf.Platform
    if(GFX_GET_CURRENT_RENDERCORE(Surf.Platform) == IGFX_UNKNOWN_CORE)
    {
        Surf.Platform = GetGmmLibContext()->GetPlatformInfo().Platform;
        // If this is a unified surface then make sure the AUX surface has the same platform info
        if(Surf.Flags.Gpu.UnifiedAuxSurface)
        {
            AuxSurf.Platform    = Surf.Platform;
            AuxSecSurf.Platform = Surf.Platform;
        }
    }

    if(GetGmmLibContext()->GetPlatformInfoObj() != NULL &&
       (GFX_GET_CURRENT_RENDERCORE(Surf.Platform) !=
        GFX_GET_CURRENT_RENDERCORE(GetGmmLibContext()->GetPlatformInfo().Platform)) &&
       (GetGmmLibContext()->GetOverridePlatformInfoObj() == NULL ||
        (GFX_GET_CURRENT_RENDERCORE(Surf.Platform) !=
         GFX_GET_CURRENT_RENDERCORE(GetGmmLibContext()->GetOverridePlatformInfo().Platform))))
    {
        //Ensure override is a future platform.
        __GMM_ASSERT(GFX_GET_CURRENT_RENDERCORE(Surf.Platform) >
                     GFX_GET_CURRENT_RENDERCORE(GetGmmLibContext()->GetPlatformInfo().Platform));

        GetGmmLibContext()->SetOverridePlatformInfoObj(GetGmmLibContext()->CreatePlatformInfo(Surf.Platform, true));

        if(GetGmmLibContext()->GetOverrideTextureCalc())
        {
            delete(GetGmmLibContext()->GetOverrideTextureCalc());
            GetGmmLibContext()->SetOverrideTextureCalc(NULL);
        }

	GetGmmLibContext()->SetOverrideTextureCalc(GetGmmLibContext()->CreateTextureCalc(Surf.Platform, true));

    }
#endif

    pPlatformResource = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());
    pTextureCalc      = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    if (!pTextureCalc)
    {
        GMM_ASSERTDPF(0, "Texture Calculation pointer is NULL.");
        goto ERROR_CASE;
    }

    __GMM_ASSERT(!(
    Surf.Flags.Gpu.Query &&
    !Surf.Flags.Info.Cacheable)); // Why query not set as cacheable? If not cacheable, what keeps from stolen memory (because STORE_DWORD/PIPE_CONTROL/etc. targets can't be in stolen)?

    if(Surf.Flags.Gpu.HiZ)
    {
        // HiZ checked in PreProcessTexSpecialCases before legal HZ_Width/Height expansion.
        AllowMaxWidthViolations  = true;
        AllowMaxHeightViolations = true;
    }

    // check bpp
    if(((Surf.BitsPerPixel < 8) || (Surf.BitsPerPixel % GMM_BITS(8))) &&
       !(Surf.BitsPerPixel == 1))
    {
        GMM_ASSERTDPF(0, "Invalid BitsPerPixel!");
        goto ERROR_CASE;
    }

    // Check at least one tile pref set.
    // Yf/Ys checked explicitly here, require one of Y or Linear depending on resource type (e.g 1D-->Linear)
    // that TODO: could be automatically promoted.
    if((Surf.Flags.Info.Linear == 0) &&
       (Surf.Flags.Info.TiledX == 0) &&
       (Surf.Flags.Info.TiledW == 0) &&
       (Surf.Flags.Info.TiledYf == 0) &&
       !GMM_IS_4KB_TILE(Surf.Flags) && !GMM_IS_64KB_TILE(Surf.Flags))
    {
        GMM_ASSERTDPF(0, "No Tile or Linear preference specified!");
        goto ERROR_CASE;
    }

    if(Surf.Flags.Info.Tile64 || Surf.Flags.Info.TiledYf || Surf.Flags.Info.TiledYs)
    {
        if(!GMM_IS_SUPPORTED_BPP_ON_TILE_64_YF_YS(Surf.BitsPerPixel))
        {
            GMM_ASSERTDPF(0, "BPP not supported on selected Tile format!");
            goto ERROR_CASE;
        }
    }

    if(!__CanSupportStdTiling(Surf, GetGmmLibContext()))
    {
        GMM_ASSERTDPF(0, "Invalid TileYf/TileYs usage!");
        goto ERROR_CASE;
    }

    if(Surf.TileMode < TILE_NONE || Surf.TileMode >= GMM_TILE_MODES)
    {
        GMM_ASSERTDPF(0, "Invalid TileMode usage!");
        goto ERROR_CASE;
    }

    if(!((Surf.Format > GMM_FORMAT_INVALID) &&
         (Surf.Format < GMM_RESOURCE_FORMATS)))
    {
        GMM_ASSERTDPF(0, "Invalid Resource Format!");
        goto ERROR_CASE;
    }

    // Check resource format is supported on running platform
    if(IsPresentableformat() == false)
    {
        GMM_ASSERTDPF(0, "Bad Format!");
        goto ERROR_CASE;
    }

    if((Surf.Type == RESOURCE_PRIMARY) &&
       !Surf.Flags.Gpu.FlipChain)
    {
        GMM_ASSERTDPF(0, "Primary does not have FlipChain flag set.");
        goto ERROR_CASE;
    }

    if(Surf.Flags.Gpu.Overlay &&
       Surf.Flags.Gpu.FlipChain)
    {
        GMM_ASSERTDPF(0, "Overlay and FlipChain flags set. S3D logic may fail.");
        goto ERROR_CASE;
    }
    
    // Displayable surfaces must remain Tile4
    if(((!GetGmmLibContext()->GetSkuTable().FtrDisplayDisabled) &&
        (Surf.Flags.Gpu.Overlay || Surf.Flags.Gpu.FlipChain)) &&
       (!(Surf.Flags.Info.Linear || Surf.Flags.Info.TiledX || GMM_IS_4KB_TILE(Surf.Flags))))
    {
        GMM_ASSERTDPF(0, "Unsupported tiling format for displayable resource.");
        goto ERROR_CASE;
    }

    if(GetGmmLibContext()->GetSkuTable().FtrLocalMemory)
    {
        GMM_ASSERTDPF(((Surf.Flags.Info.NonLocalOnly && Surf.Flags.Info.LocalOnly) == 0),
                      "Incorrect segment preference, cannot be both local and system memory.");

	// Before overriding the flags predetermine if compression request is deniable or not.
        if(!Surf.Flags.Info.LocalOnly &&
           (!(Surf.Flags.Gpu.Overlay || Surf.Flags.Gpu.FlipChain)) &&
           !(Surf.Flags.Info.HardwareProtected))
        {
            Surf.Flags.Wa.DeniableLocalOnlyForCompression = 1;
        }

        if(Surf.Flags.Gpu.Overlay ||
           Surf.Flags.Gpu.FlipChain)
        {
            if(Surf.Flags.Info.NonLocalOnly)
            {
                GMM_ASSERTDPF(0, "Overlay and FlipChain cannot be in system memory.");
                goto ERROR_CASE;
            }
            Surf.Flags.Info.LocalOnly    = 1;
            Surf.Flags.Info.NonLocalOnly = 0;
        }
        if(GetGmmLibContext()->GetSkuTable().FtrFlatPhysCCS &&
            !GetGmmLibContext()->GetSkuTable().FtrXe2Compression &&
           (Surf.Flags.Info.RenderCompressed ||
            Surf.Flags.Info.MediaCompressed))
        {
            if(Surf.Flags.Info.NonLocalOnly)
            {
                GMM_ASSERTDPF(0, "Compressible surfaces cannot be in system memory.");
                goto ERROR_CASE;
            }
            Surf.Flags.Info.LocalOnly    = 1;
            Surf.Flags.Info.NonLocalOnly = 0;
        }

        if(!Surf.Flags.Info.NotLockable &&
           Surf.Flags.Info.Shared)
        {
            if(Surf.Flags.Info.LocalOnly)
            {
                GMM_ASSERTDPF(0, "Lockable Shared cannot be in local memory.");
                goto ERROR_CASE;
            }
            Surf.Flags.Info.LocalOnly    = 0;
            Surf.Flags.Info.NonLocalOnly = 1;
        }

        if(Surf.Flags.Gpu.CameraCapture)
        {
            if(Surf.Flags.Info.LocalOnly)
            {
                GMM_ASSERTDPF(0, "CameraCapture cannot be in local memory.");
            }
            Surf.Flags.Info.LocalOnly    = 0;
            Surf.Flags.Info.NonLocalOnly = 1;
        }

        if(GetGmmLibContext()->GetWaTable().Wa64kbMappingAt2mbGranularity &&
           (!GetGmmLibContext()->GetSkuTable().FtrLocalMemoryAllows4KB) &&
           !Surf.Flags.Info.NonLocalOnly)
        {
            Surf.Flags.Info.LocalOnly = true;

        }
    }
    else
    {
        Surf.Flags.Info.LocalOnly = false; //Zero out on iGPU
        if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression && Surf.Flags.Info.XAdapter)
        {
            Surf.Flags.Info.NotCompressed     = 1; // disable compression for XAdapter resources on iGPU,
            Surf.Flags.Gpu.CCS                = 0;
            Surf.Flags.Gpu.UnifiedAuxSurface  = 0;
            Surf.Flags.Gpu.IndirectClearColor = 0;
            Surf.Flags.Gpu.MCS                = 0;
        }
    }
    if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
    {
        if (Surf.Flags.Info.TiledX)
        {
            if (!(Surf.Flags.Gpu.FlipChain || Surf.Flags.Gpu.Overlay))
            {
                GMM_ASSERTDPF(0, "TiledX request for non displayable");
            }
            Surf.Flags.Info.NotCompressed = 1; // disable compression for TileX resources
        }

        if ((Surf.Flags.Gpu.FlipChain || Surf.Flags.Gpu.Overlay) && !Surf.Flags.Info.Tile4)
        {
            Surf.Flags.Info.NotCompressed = 1; //Disable compression if displayable are not tile4
        }
    }

#ifndef __GMM_KMD__
    if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression && (GetGmmClientContext() != NULL))
    {
        if (((GMM_AIL_STRUCT *)(GetGmmClientContext()->GmmGetAIL()))->AilDisableXe2CompressionRequest)
        {
            //Disable Compression at resource level only, However at adapter level FtrXe2Compression could be still enabled.
            //AilDisableXe2CompressionRequest helps us to acheive this.
            Surf.Flags.Info.NotCompressed = 1;
        }
    }
#endif

    if((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) < IGFX_GEN8_CORE) &&
       Surf.Flags.Info.TiledW)
    {
        GMM_ASSERTDPF(0, "Flag not supported on this platform.");
        goto ERROR_CASE;
    }

    if((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) > IGFX_GEN11_CORE) &&
       Surf.Flags.Info.TiledW)
    {
        GMM_ASSERTDPF(0, "Flag not supported on this platform.");
        goto ERROR_CASE;
    }

    if((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) < IGFX_GEN9_CORE) &&
#if(_DEBUG || _RELEASE_INTERNAL)
       !GetGmmLibContext()->GetWaTable().WaDisregardPlatformChecks &&
#endif
       Surf.Flags.Gpu.MMC)
    {
        GMM_ASSERTDPF(0, "Flag not supported on this platform.");
        goto ERROR_CASE;
    }

    //For Media Memory Compression --
    if((Status = GetGmmLibContext()->GetPlatformInfoObj()->ValidateMMC(Surf)) == 0)
    {
        GMM_ASSERTDPF(0, "Invalid flag or array size!");
        goto ERROR_CASE;
    }

    if(!GetGmmLibContext()->GetSkuTable().FtrTileY)
    {
        if(Surf.Flags.Gpu.TiledResource &&
           ((Surf.Flags.Info.Linear && !(Surf.Type == RESOURCE_BUFFER)) || Surf.Flags.Info.TiledYs ||
            Surf.Flags.Info.TiledYf || Surf.Flags.Info.TiledY))
        {
            GMM_ASSERTDPF(0, "Invalid Tile for TiledResource!");
            goto ERROR_CASE;
        }
        if(GMM_IS_64KB_TILE(Surf.Flags) &&
           (Surf.MSAA.NumSamples > 1) &&
           (Surf.MaxLod != 0))
        {
            GMM_ASSERTDPF(0, "Mipped MSAA not supported for Tile64!");
            goto ERROR_CASE;
        }
        // Tile4 does not support MSAA.
        if(GMM_IS_4KB_TILE(Surf.Flags) &&
           (Surf.MSAA.NumSamples > 1))
        {
            GMM_ASSERTDPF(0, "No MSAA support for Tile4!");
            goto ERROR_CASE;
        }
    }

    __GMM_ASSERT(!(GetGmmLibContext()->GetSkuTable().FtrTileY &&
                   (Surf.Flags.Info.Tile4 || Surf.Flags.Info.Tile64)));

    //GMM asserts that ExistingSysMem allocation (whose malloc is outside GmmLib) are either
    //SVM Buffer, Index Buffer, Vertex Buffers, Render Target, Texture
    if(Surf.Flags.Info.ExistingSysMem &&
       !Surf.ExistingSysMem.IsGmmAllocated &&
       !Surf.ExistingSysMem.IsPageAligned)
    {
        if(!(Surf.Flags.Gpu.RenderTarget ||
             Surf.Flags.Gpu.Vertex ||
             Surf.Flags.Gpu.Index ||
             Surf.Flags.Gpu.Texture ||
             Surf.Flags.Info.Linear ||
             Surf.Flags.Info.SVM))
        {
            GMM_ASSERTDPF(0, "Flag not supported by ExistingSysMem alloc!");
            goto ERROR_CASE;
        }

        if(Surf.Type == RESOURCE_3D ||
           Surf.Type == RESOURCE_CUBE ||
           Surf.MaxLod ||
           GmmIsYUVPlanar(Surf.Format))
        {
            GMM_ASSERTDPF(0, "Invalid ExistingSysMem resource Type/Format");
            goto ERROR_CASE;
        }
    }

    pTextureCalc->GetResRestrictions(&Surf, Restrictions);

    // Check array size to make sure it meets HW limits
    if((Surf.ArraySize > Restrictions.MaxArraySize) &&
       ((RESOURCE_1D == Surf.Type) ||
        (RESOURCE_2D == Surf.Type) ||
        (RESOURCE_CUBE == Surf.Type)))
    {
        GMM_ASSERTDPF(0, "Invalid array size!");
        goto ERROR_CASE;
    }

    // Check dimensions to make sure it meets HW max limits
    if(((Surf.BaseHeight > Restrictions.MaxHeight) && !AllowMaxHeightViolations) ||
       ((Surf.BaseWidth > Restrictions.MaxWidth) && !AllowMaxWidthViolations) ||
       (Surf.Depth > Restrictions.MaxDepth)) // Any reason why MaxDepth != 1 for Tex2D
    {
        GMM_ASSERTDPF(0, "Invalid Dimension. Greater than max!");
        goto ERROR_CASE;
    }

    // Check width to make sure it meets Yx requirements
    if(((Surf.Format == GMM_FORMAT_Y8_UNORM_VA) && (Surf.BaseWidth % 4)) ||
       ((Surf.Format == GMM_FORMAT_Y16_UNORM) && (Surf.BaseWidth % 2)) ||
       ((Surf.Format == GMM_FORMAT_Y1_UNORM) && (Surf.BaseWidth % 32)))
    {
        GMM_ASSERTDPF(0, "Invalid width!");
        goto ERROR_CASE;
    }

    // Check dimensions to make sure it meets HW min limits
    if((Surf.BaseHeight < Restrictions.MinHeight) ||
       (Surf.BaseWidth < Restrictions.MinWidth) ||
       (Surf.Depth < Restrictions.MinDepth))
    {
        GMM_ASSERTDPF(0, "Invalid Dimension. Less than min!");
        goto ERROR_CASE;
    }

    // check mip map
    if(Surf.MaxLod > pPlatformResource->MaxLod)
    {
        GMM_ASSERTDPF(0, "Invalid mip map chain specified!");
        goto ERROR_CASE;
    }

    // MIPs are not supported for tiled Yf/Ys planar surfaces
    if((Surf.MaxLod) &&
       GmmIsPlanar(Surf.Format) &&
       (Surf.Flags.Info.TiledYf || GMM_IS_64KB_TILE(Surf.Flags)))
    {
        GMM_ASSERTDPF(0, "Invalid mip map chain specified!");
        goto ERROR_CASE;
    }

    // check depth buffer tilings
    if((Surf.Flags.Gpu.Depth == 1) &&
       (Surf.Flags.Info.TiledX == 1))
    {
        GMM_ASSERTDPF(0, "Invalid Tiling for Depth Buffer!");
        goto ERROR_CASE;
    }

#if DBG
    // Check if stencil buffer gpu flag is set w/ other flags
    if(Surf.Flags.Gpu.SeparateStencil == 1)
    {
        GMM_RESOURCE_FLAG OnlySeparateStencilGpuFlag;

        memset(&OnlySeparateStencilGpuFlag.Gpu, 0, sizeof(OnlySeparateStencilGpuFlag.Gpu));
        OnlySeparateStencilGpuFlag.Gpu.SeparateStencil = 1;

        if(memcmp(&Surf.Flags.Gpu, &OnlySeparateStencilGpuFlag.Gpu, sizeof(OnlySeparateStencilGpuFlag.Gpu)))
        {
            GMM_ASSERTDPF(0,
                          "Should not set w/ other flags b/c vertical alignment is "
                          "unique for separate stencil and incompatible w/ other "
                          "usages such as render target.");
            goto ERROR_CASE;
        }
    }
#endif

    // MSAA Restrictions
    if((Surf.MSAA.NumSamples > 1) &&
       !( //--- Legitimate MSAA Case ------------------------------------------
       (Surf.Type == RESOURCE_2D) &&
       (Surf.Flags.Gpu.Depth ||
        Surf.Flags.Gpu.HiZ ||
        Surf.Flags.Gpu.RenderTarget ||
        Surf.Flags.Gpu.SeparateStencil) &&
       // Single-LOD (pre-SKL)...
       ((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN9_CORE) ||
        (Surf.MaxLod == 0)) &&
       // TileYF cannot be MSAA'ed (pre-Gen10)...
       ((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN10_CORE) ||
        !Surf.Flags.Info.TiledYf) &&
       // Tile64 MSAA (Xe_HP)
       (GetGmmLibContext()->GetSkuTable().FtrTileY ||
        !GMM_IS_64KB_TILE(Surf.Flags) ||
        (Surf.MaxLod == 0)) &&
       // Tile4 does not support MSAA
       (GetGmmLibContext()->GetSkuTable().FtrTileY ||
        !GMM_IS_4KB_TILE(Surf.Flags)) &&
       // Non-Compressed/YUV...
       !GmmIsCompressed(GetGmmLibContext(), Surf.Format) &&
       !GmmIsYUVPacked(Surf.Format) &&
       !GmmIsPlanar(Surf.Format) &&
       // Supported Sample Count for Platform...
       (((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN7_CORE) &&
         ((Surf.MSAA.NumSamples == 4) || (Surf.MSAA.NumSamples == 8))) ||
        ((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN8_CORE) &&
         ((Surf.MSAA.NumSamples == 2))) ||
        ((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN9_CORE) &&
         ((Surf.MSAA.NumSamples == 16))))))
    {
        GMM_ASSERTDPF(0, "Invalid MSAA usage!");
        goto ERROR_CASE;
    }

    // CCS Restrictions
    if(Surf.Flags.Gpu.CCS)
    {
        if((Status = GetGmmLibContext()->GetPlatformInfoObj()->ValidateCCS(Surf)) == 0)
        {
            GMM_ASSERTDPF(0, "Invalid CCS usage!");
            goto ERROR_CASE;
        }

        if(Surf.Flags.Info.RenderCompressed && Surf.Flags.Info.MediaCompressed)
        {
            GMM_ASSERTDPF(0, "Invalid CCS usage - can't be both RC and MC!");
            goto ERROR_CASE;
        }
    }

    // UnifiedAuxSurface Restrictions
    if((Status = GetGmmLibContext()->GetPlatformInfoObj()->ValidateUnifiedAuxSurface(Surf)) == 0)
    {
        GMM_ASSERTDPF(0, "Invalid UnifiedAuxSurface usage!");
        goto ERROR_CASE;
    }

    // IndirectClearColor Restrictions
    if((Surf.Flags.Gpu.IndirectClearColor) &&
       !( //--- Legitimate IndirectClearColor Case ------------------------------------------
        (((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN9_CORE) && Surf.Flags.Gpu.UnifiedAuxSurface) ||
         ((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) > IGFX_GEN11_CORE) && (Surf.Flags.Gpu.HiZ || Surf.Flags.Gpu.SeparateStencil)))))

    {
        GMM_ASSERTDPF(0, "Invalid IndirectClearColor usage!");
        goto ERROR_CASE;
    }

    // CornerTexelMode Restrictions
    if(Surf.Flags.Info.CornerTexelMode &&
       (!( //--- Legitimate CornerTexelMode Case -------------------------------------------
       (GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN11_CORE) &&
       (!GmmIsPlanar(Surf.Format)) &&
       (!Surf.Flags.Info.StdSwizzle))))
    {
        GMM_ASSERTDPF(0, "Flag.Info.CornerTexelMode not supported on this platform.");
        goto ERROR_CASE;
    }

 //MultiTileArch params
    if(GetGmmLibContext()->GetSkuTable().FtrMultiTileArch)
    {
        /*
            MultiTileArch validation criteria 
            - MultiTileArch.Enable must be set.
            - NonLocalOnly alloc must have LocalEligibilitySet and LocalPreferenceSet both zero
            - LocalOnly alloc must have non-zero LocalEligibilitySet
            - GpuVaMappingSet/LocalEligibilitySet must be subset of GtSysInfo.TileMask
            - PreferredSet must be subset of EligibilitySet or zero
        */
        if(!(
           // Legitimate cases
           MultiTileArch.Enable &&
           (Surf.Flags.Info.NonLocalOnly || MultiTileArch.LocalMemEligibilitySet) &&
           ((MultiTileArch.GpuVaMappingSet & GetGmmLibContext()->GetGtSysInfo()->MultiTileArchInfo.TileMask) == MultiTileArch.GpuVaMappingSet) &&
           ((MultiTileArch.LocalMemEligibilitySet & GetGmmLibContext()->GetGtSysInfo()->MultiTileArchInfo.TileMask) == MultiTileArch.LocalMemEligibilitySet) &&
           ((MultiTileArch.LocalMemEligibilitySet & MultiTileArch.LocalMemPreferredSet) == MultiTileArch.LocalMemPreferredSet)))
        {
            GMM_ASSERTDPF(0, "Invalid MultiTileArch allocation params");
            goto ERROR_CASE;
        }
    }

    // check 2D, 3D & Cubemap dimensions
    switch(Surf.Type)
    {
        case RESOURCE_1D:
        case RESOURCE_2D:
        case RESOURCE_3D:
        {
            if(!Surf.BaseWidth || !Surf.BaseHeight)
            {
                GMM_ASSERTDPF(0, "Width or Height is 0!");
                goto ERROR_CASE;
            }
            break;
        }
        case RESOURCE_CUBE:
        {
            if(!Surf.BaseWidth || !Surf.BaseHeight)
            {
                GMM_ASSERTDPF(0, "Cubemap Dimensions invalid!");
                goto ERROR_CASE;
            }
            if(Surf.BaseWidth != Surf.BaseHeight)
            {
                GMM_ASSERTDPF(0, "Cubemap Dimensions invalid (width != Height)!");
                goto ERROR_CASE;
            }
            break;
        }
        case RESOURCE_SCRATCH:
        case RESOURCE_BUFFER:
        {
            if(Surf.BaseHeight != 1)
            {
                GMM_ASSERTDPF(0, "Linear surface height not 1!");
                goto ERROR_CASE;
            }
            break;
        }
        default:
            if (!Surf.BaseWidth || !Surf.BaseHeight)
            {
                GMM_ASSERTDPF(0, "Width or Height is 0!");
                goto ERROR_CASE;
            }
            break;
    }

    if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression && 
		    !(ONE_WAY_COHERENT_COMPRESSION_MODE(GetGmmLibContext()->GetPlatformInfo().Platform.eProductFamily, GetGmmLibContext()->GetWaTable().WaNoCpuCoherentCompression)) && 
		    Surf.Flags.Info.Cacheable && !Surf.Flags.Info.NotCompressed)
    {
        GMM_ASSERTDPF(0, "Invalid combination Cpu Cacheable and Compression set on a platform");
        goto ERROR_CASE;
    }

    GMM_DPF_EXIT;
    return 1;

ERROR_CASE:
    __GMM_ASSERT(0);
    return 0;
}

//=============================================================================
//
// Function: GetDisplayCompressionSupport
//
// Desc: Returns true if display hw supports lossless render/media decompression
//       else returns false.
//       Umds can call it to decide if full resolve is required
//
// Parameters:
//      See function arguments.
//
// Returns:
//      uint8_t
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetDisplayCompressionSupport()
{
    uint8_t             ComprSupported = 0;
    GMM_RESOURCE_FORMAT Format         = Surf.Format;

    if(Surf.Flags.Gpu.UnifiedAuxSurface)
    {
        bool IsSupportedRGB64_16_16_16_16 = false;
        bool IsSupportedRGB32_8_8_8_8     = false;
        bool IsSupportedRGB32_2_10_10_10  = false;
        bool IsSupportedMediaFormats      = false;

        switch(Format) //RGB64 16:16 : 16 : 16 FP16
        {
            case GMM_FORMAT_R16G16B16A16_FLOAT:
            case GMM_FORMAT_R16G16B16X16_FLOAT:
                IsSupportedRGB64_16_16_16_16 = true;
            default:
                break;
        }

        switch(Format) //RGB32 8 : 8 : 8 : 8
        {
            case GMM_FORMAT_B8G8R8A8_UNORM:
            case GMM_FORMAT_R8G8B8A8_UNORM:
            case GMM_FORMAT_B8G8R8X8_UNORM:
            case GMM_FORMAT_R8G8B8X8_UNORM:
            case GMM_FORMAT_R8G8B8A8_UNORM_SRGB:
            case GMM_FORMAT_B8X8_UNORM_G8R8_SNORM:
            case GMM_FORMAT_X8B8_UNORM_G8R8_SNORM:
            case GMM_FORMAT_A8X8_UNORM_G8R8_SNORM:
            case GMM_FORMAT_B8G8R8A8_UNORM_SRGB:
            case GMM_FORMAT_B8G8R8X8_UNORM_SRGB:
            case GMM_FORMAT_R8G8B8A8_SINT:
            case GMM_FORMAT_R8G8B8A8_SNORM:
            case GMM_FORMAT_R8G8B8A8_SSCALED:
            case GMM_FORMAT_R8G8B8A8_UINT:
            case GMM_FORMAT_R8G8B8A8_USCALED:
            case GMM_FORMAT_R8G8B8X8_UNORM_SRGB:
                IsSupportedRGB32_8_8_8_8 = true;
            default:
                break;
        }

        switch(Format) //RGB32 2 : 10 : 10 : 10
        {
            case GMM_FORMAT_B10G10R10X2_UNORM:
            case GMM_FORMAT_R10G10B10A2_UNORM:
            case GMM_FORMAT_B10G10R10A2_UNORM:
            case GMM_FORMAT_B10G10R10A2_SINT:
            case GMM_FORMAT_B10G10R10A2_SNORM:
            case GMM_FORMAT_B10G10R10A2_SSCALED:
            case GMM_FORMAT_B10G10R10A2_UINT:
            case GMM_FORMAT_B10G10R10A2_UNORM_SRGB:
            case GMM_FORMAT_B10G10R10A2_USCALED:
            case GMM_FORMAT_R10G10B10_FLOAT_A2_UNORM:
            case GMM_FORMAT_R10G10B10_SNORM_A2_UNORM:
            case GMM_FORMAT_R10G10B10A2_SINT:
            case GMM_FORMAT_R10G10B10A2_SNORM:
            case GMM_FORMAT_R10G10B10A2_SSCALED:
            case GMM_FORMAT_R10G10B10A2_UINT:
            case GMM_FORMAT_R10G10B10A2_UNORM_SRGB:
            case GMM_FORMAT_R10G10B10A2_USCALED:
            case GMM_FORMAT_R10G10B10X2_USCALED:
            case GMM_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
                IsSupportedRGB32_2_10_10_10 = true;
            default:
                break;
        }

        switch(Format)
        {
            case GMM_FORMAT_YUY2: //YUV422 8 bpc
            case GMM_FORMAT_NV12: //YUV420 - NV12
            case GMM_FORMAT_P010: //YUV420 - P0xx
            case GMM_FORMAT_P016:
            case GMM_FORMAT_Y210: //YUV422 - Y210, Y212, Y216
            case GMM_FORMAT_Y216:
            case GMM_FORMAT_Y410: //YUV444 - Y410
            case GMM_FORMAT_Y416:
                IsSupportedMediaFormats = true; //YUV444 - Y412, Y416
            default:
                break;
        }

        //Check fmt is display decompressible
        ComprSupported = GetGmmLibContext()->GetPlatformInfoObj()->CheckFmtDisplayDecompressible(Surf, IsSupportedRGB64_16_16_16_16,
                                                                                                IsSupportedRGB32_8_8_8_8, IsSupportedRGB32_2_10_10_10,
                                                                                                IsSupportedMediaFormats);
    }

    return ComprSupported;
}

//=============================================================================
//
// Function: GetDisplayFastClearSupport
//
// Desc: Returns 1 if display hw supports fast clear else returns 0.
//       Umds can call it to decide if FC resolve is required
//
// Parameters:
//      See function arguments.
//
// Returns:
//      uint8_t
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetDisplayFastClearSupport()
{
    uint8_t FCSupported = 0;

    if(GFX_GET_CURRENT_RENDERCORE(GetGmmLibContext()->GetPlatformInfo().Platform) >= IGFX_GEN11_CORE)
    {
        FCSupported = GetDisplayCompressionSupport() && !GmmIsPlanar(Surf.Format);
        FCSupported &= Surf.Flags.Gpu.IndirectClearColor;
    }

    return FCSupported;
}
