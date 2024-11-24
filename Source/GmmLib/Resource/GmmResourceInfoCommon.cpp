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
/// Returns indication of whether resource is eligible for 64KB pages or not.
/// On Windows, UMD must call this api after GmmResCreate()
/// @return     1/0
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::Is64KBPageSuitable()
{
    bool Ignore64KBPadding = false;
    //!!!! DO NOT USE GetSizeSurface() as it returns the padded size and not natural size.
    GMM_GFX_SIZE_T Size = Surf.Size + AuxSurf.Size + AuxSecSurf.Size;

    __GMM_ASSERT(Size);

    // All ESM resources and VirtuaPadding are exempt from 64KB paging
    if(Surf.Flags.Info.ExistingSysMem ||
       Surf.Flags.Info.XAdapter ||
       Surf.Flags.Gpu.CameraCapture ||
       Surf.Flags.Info.KernelModeMapped ||
       (Surf.Flags.Gpu.S3d && !Surf.Flags.Gpu.S3dDx &&
        !GetGmmLibContext()->GetSkuTable().FtrDisplayEngineS3d)
#if(LHDM)
       || (Surf.Flags.Info.AllowVirtualPadding &&
           ExistingSysMem.hParentAllocation)
#endif
       )
    {
        Ignore64KBPadding = true;
    }

    if(GetGmmLibContext()->GetSkuTable().FtrLocalMemory)
     {
        Ignore64KBPadding |= (Surf.Flags.Info.Shared && !Surf.Flags.Info.NotLockable);
        Ignore64KBPadding |= ((GetGmmLibContext()->GetSkuTable().FtrLocalMemoryAllows4KB) && Surf.Flags.Info.NoOptimizationPadding);
        Ignore64KBPadding |= ((GetGmmLibContext()->GetSkuTable().FtrLocalMemoryAllows4KB || Surf.Flags.Info.NonLocalOnly) && (((Size * (100 + (GMM_GFX_SIZE_T)GetGmmLibContext()->GetAllowedPaddingFor64KbPagesPercentage())) / 100) < GFX_ALIGN(Size, GMM_KBYTE(64))));
    }
    else
    {
        // The final padded size cannot be larger then a set percentage of the original size
        if((Surf.Flags.Info.NoOptimizationPadding && !GFX_IS_ALIGNED(Size, GMM_KBYTE(64))) /*Surface is not 64kb aligned*/ ||
           (!Surf.Flags.Info.NoOptimizationPadding && (((Size * (100 + (GMM_GFX_SIZE_T)GetGmmLibContext()->GetAllowedPaddingFor64KbPagesPercentage())) / 100) < GFX_ALIGN(Size, GMM_KBYTE(64)))) /*10% padding TBC */)
        {
            Ignore64KBPadding |= true;
        }
    }

    // If 64KB paging is enabled pad out the resource to 64KB alignment
    if(GetGmmLibContext()->GetSkuTable().FtrWddm2_1_64kbPages &&
       // Ignore the padding for the above VirtualPadding or ESM cases
       (!Ignore64KBPadding) &&
       // Resource must be 64KB aligned
       (GFX_IS_ALIGNED(Surf.Alignment.BaseAlignment, GMM_KBYTE(64)) ||
        // Or must be aligned to a factor of 64KB
        (Surf.Alignment.BaseAlignment == GMM_KBYTE(32)) ||
        (Surf.Alignment.BaseAlignment == GMM_KBYTE(16)) ||
        (Surf.Alignment.BaseAlignment == GMM_KBYTE(8)) ||
        (Surf.Alignment.BaseAlignment == GMM_KBYTE(4))))
    {
        return 1;
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Allows clients to "create" any type of resource. This function does not
/// allocate any memory for the resource. It just calculates the various parameters
/// which are useful for the client and can be queried using other functions.
///
/// @param[in]  GmmLib Context: Reference to ::GmmLibContext
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmResourceInfoCommon::Create(GMM_RESCREATE_PARAMS &CreateParams)
{
    GMM_STATUS Status = GMM_ERROR;
    // ToDo: Only Vk is using this Create API directly. Derive the GmmLibCOntext from the ClientContext stored in
    // ResInfo object.
    Status = Create(*(reinterpret_cast<GMM_CLIENT_CONTEXT *>(pClientContext)->GetLibContext()), CreateParams);

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Allows clients to "create"  Custom memory layout received from the App as user pointer or DMABUF
// This function does not allocate any memory for the resource. It just calculates/ populates the various parameters
/// which are useful for the client and can be queried using other functions.
///
/// @param[in]  GmmLib Context: Reference to ::GmmLibContext
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmResourceInfoCommon::CreateCustomRes(Context &GmmLibContext, GMM_RESCREATE_CUSTOM_PARAMS &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    GMM_STATUS               Status       = GMM_ERROR;
    GMM_TEXTURE_CALC *       pTextureCalc = NULL;
    uint32_t                 BitsPerPixel, i;


    GMM_DPF_ENTER;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);
    pGmmUmdLibContext = reinterpret_cast<uint64_t>(&GmmLibContext);
    __GMM_ASSERTPTR(pGmmUmdLibContext, GMM_ERROR);

    if((CreateParams.Format > GMM_FORMAT_INVALID) &&
       (CreateParams.Format < GMM_RESOURCE_FORMATS))
    {
        BitsPerPixel = GetGmmLibContext()->GetPlatformInfo().FormatTable[CreateParams.Format].Element.BitsPer;
    }
    else
    {
        GMM_ASSERTDPF(0, "Format Error");
        Status = GMM_INVALIDPARAM;
        goto ERROR_CASE;
    }

    pPlatform    = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    Surf.Type                    = CreateParams.Type;
    Surf.Format                  = CreateParams.Format;
    Surf.BaseWidth               = CreateParams.BaseWidth64;
    Surf.BaseHeight              = CreateParams.BaseHeight;
    Surf.Flags                   = CreateParams.Flags;
    Surf.CachePolicy.Usage       = CreateParams.Usage;
    Surf.Pitch                   = CreateParams.Pitch;
    Surf.Size                    = CreateParams.Size;
    Surf.Alignment.BaseAlignment = CreateParams.BaseAlignment;
    Surf.MaxLod                  = 1;
    Surf.ArraySize               = 1;
    Surf.CpTag                   = CreateParams.CpTag;

#if(_DEBUG || _RELEASE_INTERNAL)
    Surf.Platform = GetGmmLibContext()->GetPlatformInfo().Platform;
#endif
    Surf.BitsPerPixel     = BitsPerPixel;
    Surf.Alignment.QPitch = (GMM_GLOBAL_GFX_SIZE_T)(Surf.Pitch * Surf.BaseHeight);

    pTextureCalc->SetTileMode(&Surf);

    if(GmmIsPlanar(Surf.Format))
    {
        pTextureCalc->SetPlanarOffsetInfo(&Surf, CreateParams);

        if (Surf.ArraySize > 1)
        {
            //Surf.OffsetInfo.Plane.ArrayQPitch = Surf.Size;  //Not required as this new interface doesn't support arrayed surfaces.
        }

        UpdateUnAlignedParams();
    }

    switch(Surf.Type)
    {
        case RESOURCE_1D:
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

        if (Surf.ArraySize > 1)
        {
           // Surf.OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender =
           // Surf.OffsetInfo.Texture2DOffsetInfo.ArrayQPitchLock = Surf.Pitch * Surf.BaseHeight;  //Not required as this new interface doesn't support arrayed surfaces.
        }

            for(i = 0; i <= Surf.MaxLod; i++)
            {
                Surf.OffsetInfo.Texture2DOffsetInfo.Offset[i] = 0;
            }

            break;
        }
        default:
        {
            GMM_ASSERTDPF(0, "GmmTexAlloc: Unknown surface type!");
            Status = GMM_INVALIDPARAM;
            goto ERROR_CASE;
            ;
        }
    };

    GMM_DPF_EXIT;
    return GMM_SUCCESS;

ERROR_CASE:
    //Zero out all the members
    new(this) GmmResourceInfoCommon();

    GMM_DPF_EXIT;
    return Status;
}

#ifndef __GMM_KMD__
/////////////////////////////////////////////////////////////////////////////////////
/// Allows clients to "create"  Custom memory layout received from the App as user pointer or DMABUF
// This function does not allocate any memory for the resource. It just calculates/ populates the various parameters
/// which are useful for the client and can be queried using other functions.
///
/// @param[in]  GmmLib Context: Reference to ::GmmLibContext
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmResourceInfoCommon::CreateCustomRes_2(Context &GmmLibContext, GMM_RESCREATE_CUSTOM_PARAMS_2 &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    GMM_STATUS               Status       = GMM_ERROR;
    GMM_TEXTURE_CALC *       pTextureCalc = NULL;
    uint32_t                 BitsPerPixel, i;


    GMM_DPF_ENTER;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);
    pGmmUmdLibContext = reinterpret_cast<uint64_t>(&GmmLibContext);
    __GMM_ASSERTPTR(pGmmUmdLibContext, GMM_ERROR);


    if((CreateParams.Format > GMM_FORMAT_INVALID) &&
       (CreateParams.Format < GMM_RESOURCE_FORMATS))
    {
        BitsPerPixel = GetGmmLibContext()->GetPlatformInfo().FormatTable[CreateParams.Format].Element.BitsPer;
    }
    else
    {
        GMM_ASSERTDPF(0, "Format Error");
        Status = GMM_INVALIDPARAM;
        goto ERROR_CASE;
    }

    pPlatform    = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    Surf.Type                    = CreateParams.Type;
    Surf.Format                  = CreateParams.Format;
    Surf.BaseWidth               = CreateParams.BaseWidth64;
    Surf.BaseHeight              = CreateParams.BaseHeight;
    Surf.Flags                   = CreateParams.Flags;
    Surf.CachePolicy.Usage       = CreateParams.Usage;
    Surf.Pitch                   = CreateParams.Pitch;
    Surf.Size                    = CreateParams.Size;
    Surf.Alignment.BaseAlignment = CreateParams.BaseAlignment;
    Surf.MaxLod                  = 1;
    Surf.ArraySize               = 1;
    Surf.CpTag                   = CreateParams.CpTag;

#if(_DEBUG || _RELEASE_INTERNAL)
    Surf.Platform = GetGmmLibContext()->GetPlatformInfo().Platform;
#endif
    Surf.BitsPerPixel     = BitsPerPixel;
    Surf.Alignment.QPitch = (GMM_GLOBAL_GFX_SIZE_T)(Surf.Pitch * Surf.BaseHeight);

    pTextureCalc->SetTileMode(&Surf);

    if(GmmIsPlanar(Surf.Format))
    {
        pTextureCalc->SetPlanarOffsetInfo_2(&Surf, CreateParams);

        if(Surf.ArraySize > 1)
        {
            //Not required as this new interface doesn't support arrayed surfaces.
        }

        UpdateUnAlignedParams();
    }

    switch(Surf.Type)
    {
        case RESOURCE_1D:
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
            if(Surf.ArraySize > 1)
            {
                //Not required as this new interface doesn't support arrayed surfaces.
            }

            for(i = 0; i <= Surf.MaxLod; i++)
            {
                Surf.OffsetInfo.Texture2DOffsetInfo.Offset[i] = 0;
            }

            break;
        }
        default:
        {
            GMM_ASSERTDPF(0, "GmmTexAlloc: Unknown surface type!");
            Status = GMM_INVALIDPARAM;
            goto ERROR_CASE;
            ;
        }
    };

    if(Surf.Flags.Gpu.UnifiedAuxSurface || Surf.Flags.Gpu.CCS)
    {

        if(GetGmmLibContext()->GetSkuTable().FtrLinearCCS)
        {
            AuxSurf.Flags.Gpu.__NonMsaaLinearCCS = 1;
        }

        AuxSurf.Flags.Info.TiledW  = 0;
        AuxSurf.Flags.Info.TiledYf = 0;
        AuxSurf.Flags.Info.TiledX  = 0;
        AuxSurf.Flags.Info.Linear  = 1;
        GMM_SET_64KB_TILE(AuxSurf.Flags, 0, GetGmmLibContext());
        GMM_SET_4KB_TILE(AuxSurf.Flags, 0, GetGmmLibContext());

        AuxSurf.ArraySize    = 1;
        AuxSurf.BitsPerPixel = 8;

        if(GmmIsPlanar(CreateParams.Format) || GmmIsUVPacked(CreateParams.Format))
        {
            AuxSurf.OffsetInfo.Plane.X[GMM_PLANE_Y] = CreateParams.AuxSurf.PlaneOffset.X[GMM_PLANE_Y];
            AuxSurf.OffsetInfo.Plane.Y[GMM_PLANE_Y] = CreateParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_Y];
            AuxSurf.OffsetInfo.Plane.X[GMM_PLANE_U] = CreateParams.AuxSurf.PlaneOffset.X[GMM_PLANE_U];
            AuxSurf.OffsetInfo.Plane.Y[GMM_PLANE_U] = CreateParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_U];
            AuxSurf.OffsetInfo.Plane.X[GMM_PLANE_V] = CreateParams.AuxSurf.PlaneOffset.X[GMM_PLANE_V];
            AuxSurf.OffsetInfo.Plane.Y[GMM_PLANE_V] = CreateParams.AuxSurf.PlaneOffset.Y[GMM_PLANE_V];
            AuxSurf.OffsetInfo.Plane.ArrayQPitch    = CreateParams.AuxSurf.Size;
        }

        AuxSurf.Size = CreateParams.AuxSurf.Size;

        AuxSurf.Pitch     = CreateParams.AuxSurf.Pitch;
        AuxSurf.Type      = RESOURCE_BUFFER;
        AuxSurf.Alignment = {0};

        AuxSurf.Alignment.QPitch        = GFX_ULONG_CAST(AuxSurf.Size);
        AuxSurf.Alignment.BaseAlignment = CreateParams.AuxSurf.BaseAlignment; //TODO: TiledResource?
        AuxSurf.Size                    = GFX_ALIGN(AuxSurf.Size, PAGE_SIZE); //page-align final size

        if(AuxSurf.Flags.Gpu.TiledResource)
        {
            AuxSurf.Alignment.BaseAlignment = GMM_KBYTE(64);                          //TODO: TiledResource?
            AuxSurf.Size                    = GFX_ALIGN(AuxSurf.Size, GMM_KBYTE(64)); //page-align final size
        }

        //Clear compression request in CCS
        AuxSurf.Flags.Info.RenderCompressed = 0;
        AuxSurf.Flags.Info.MediaCompressed  = 0;
        AuxSurf.Flags.Info.RedecribedPlanes = 0;
        pTextureCalc->SetTileMode(&AuxSurf);
        AuxSurf.UnpaddedSize = AuxSurf.Size;
    }
    GMM_DPF_EXIT;
    return GMM_SUCCESS;

ERROR_CASE:
    //Zero out all the members
    new(this) GmmResourceInfoCommon();

    GMM_DPF_EXIT;
    return Status;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// Allows clients to "create" any type of resource. This function does not
/// allocate any memory for the resource. It just calculates the various parameters
/// which are useful for the client and can be queried using other functions.
///
/// @param[in]  GmmLib Context: Reference to ::GmmLibContext
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmResourceInfoCommon::Create(Context &GmmLibContext, GMM_RESCREATE_PARAMS &CreateParams)
{
    const GMM_PLATFORM_INFO *pPlatform;
    GMM_STATUS               Status       = GMM_ERROR;
    GMM_TEXTURE_CALC *       pTextureCalc = NULL;

    GMM_DPF_ENTER;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);
    pGmmUmdLibContext = reinterpret_cast<uint64_t>(&GmmLibContext);
    __GMM_ASSERTPTR(pGmmUmdLibContext, GMM_ERROR);

    if(CreateParams.Flags.Info.ExistingSysMem &&
       (CreateParams.Flags.Info.TiledW ||
        CreateParams.Flags.Info.TiledX ||
        GMM_IS_4KB_TILE(CreateParams.Flags) ||
        GMM_IS_64KB_TILE(CreateParams.Flags)))
    {
        GMM_ASSERTDPF(0, "Tiled System Accelerated Memory not supported.");
        Status = GMM_INVALIDPARAM;
        goto ERROR_CASE;
    }

    if(!CopyClientParams(CreateParams))
    {
        Status = GMM_INVALIDPARAM;
        goto ERROR_CASE;
    }

    pPlatform    = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    if (!pTextureCalc)
    {
        Status = GMM_ERROR;
        GMM_ASSERTDPF(0, "Texture Calculation pointer is NULL.");
        goto ERROR_CASE;
    }

#if defined(__GMM_KMD__) || !defined(_WIN32)
    if(!CreateParams.Flags.Info.ExistingSysMem)
#else
    // TiledResource uses a private gfx alloc, which doesn't receive a  WDDM CreateAllocation
    if(!CreateParams.Flags.Info.ExistingSysMem &&
       (CreateParams.NoGfxMemory || CreateParams.Flags.Gpu.TiledResource))
#endif
    {
        if(!ValidateParams())
        {
            GMM_ASSERTDPF(0, "Invalid parameter!");
            Status = GMM_INVALIDPARAM;
            goto ERROR_CASE;
        }

        if(GMM_SUCCESS != pTextureCalc->AllocateTexture(&Surf))
        {
            GMM_ASSERTDPF(0, "GmmTexAlloc failed!");
            goto ERROR_CASE;
        }

        if(Surf.Flags.Gpu.UnifiedAuxSurface)
        {
            GMM_GFX_SIZE_T TotalSize;
            uint32_t       Alignment;

            if(GMM_SUCCESS != pTextureCalc->FillTexCCS(&Surf, (AuxSecSurf.Type != RESOURCE_INVALID ? &AuxSecSurf : &AuxSurf)))
            {
                GMM_ASSERTDPF(0, "GmmTexAlloc failed!");
                goto ERROR_CASE;
            }

            if(AuxSurf.Size == 0 && AuxSurf.Type != RESOURCE_INVALID && GMM_SUCCESS != pTextureCalc->AllocateTexture(&AuxSurf))
            {
                GMM_ASSERTDPF(0, "GmmTexAlloc failed!");
                goto ERROR_CASE;
            }

            AuxSurf.UnpaddedSize = AuxSurf.Size;

            if(Surf.Flags.Gpu.IndirectClearColor ||
               Surf.Flags.Gpu.ColorDiscard)
            {
                if(GetGmmLibContext()->GetSkuTable().FtrFlatPhysCCS && AuxSurf.Type == RESOURCE_INVALID)
                {
                    //ie only AuxType is CCS, doesn't exist with FlatCCS, enable it for CC
                    if (!GetGmmLibContext()->GetSkuTable().FtrXe2Compression || (GetGmmLibContext()->GetSkuTable().FtrXe2Compression && (Surf.MSAA.NumSamples > 1)))
                    {
                        AuxSurf.Type = Surf.Type;
                    }
                }
                if (!Surf.Flags.Gpu.TiledResource)
                {
                    if (!GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
                    {
                        AuxSurf.CCSize = PAGE_SIZE; // 128bit Float Value + 32bit RT Native Value + Padding.
                        AuxSurf.Size += PAGE_SIZE;
                    }
                    else
                    {

                        if (Surf.MSAA.NumSamples > 1)
                        {
                            AuxSurf.UnpaddedSize += PAGE_SIZE;
                            AuxSurf.Size += PAGE_SIZE;              // Clear Color stored only for MSAA surfaces
                        }
                    }
                }
                else
                {
                    if (!GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
                    {
                        AuxSurf.CCSize = GMM_KBYTE(64); // 128bit Float Value + 32bit RT Native Value + Padding.
                        AuxSurf.Size += GMM_KBYTE(64);
                    }
                    else
                    {
                        if (Surf.MSAA.NumSamples > 1)
                        {
                            AuxSurf.UnpaddedSize += GMM_KBYTE(64);
                            AuxSurf.Size += GMM_KBYTE(64);              // Clear Color stored only for MSAA surfaces, stored as part of MCS
                        }
                    }
                }
            }
				    
  	    if(Surf.Flags.Gpu.ProceduralTexture)
            {
                //Do not require main surface access either in GPUVA/physical space.
                Surf.Size = 0;
            }

            TotalSize = Surf.Size + AuxSurf.Size; //Not including AuxSecSurf size, multi-Aux surface isn't supported for displayables
            Alignment = GFX_ULONG_CAST(Surf.Pitch * pPlatform->TileInfo[Surf.TileMode].LogicalTileHeight);

            // We need to pad the aux size to the size of the paired surface's tile row (i.e. Pitch * TileHeight) to
            // ensure the entire surface can be described with a constant pitch (for GGTT aliasing, clean FENCE'ing and
            // AcquireSwizzlingRange, even though the aux isn't intentionally part of such fencing).
            if(Surf.Flags.Gpu.FlipChain &&
               (!__GMM_IS_ALIGN(TotalSize, Alignment)))
            {
                AuxSurf.Size += (GFX_ALIGN_NP2(TotalSize, Alignment) - TotalSize);
            }

            if((Surf.Size + AuxSurf.Size + AuxSecSurf.Size) > (GMM_GFX_SIZE_T)(pPlatform->SurfaceMaxSize))
            {
                GMM_ASSERTDPF(0, "Surface too large!");
                goto ERROR_CASE;
            }
        }
    }

    if(Surf.Flags.Info.ExistingSysMem)
    {
        Surf.ExistingSysMem.IsGmmAllocated =
        (CreateParams.pExistingSysMem &&
         CreateParams.ExistingSysMemSize) ?
        false :
        true;

        if(!Surf.ExistingSysMem.IsGmmAllocated)
        {
            Surf.ExistingSysMem.IsPageAligned =
            (((CreateParams.pExistingSysMem & (PAGE_SIZE - 1)) == 0) &&
             (((CreateParams.pExistingSysMem + CreateParams.ExistingSysMemSize) & (PAGE_SIZE - 1)) == 0)) ?
            true :
            false;
        }

        if(!ValidateParams())
        {
            GMM_ASSERTDPF(0, "Invalid parameter!");
            goto ERROR_CASE;
        }

        // Get surface Gfx memory size required.
        if(GMM_SUCCESS != pTextureCalc->AllocateTexture(&Surf))
        {
            GMM_ASSERTDPF(0, "GmmTexAlloc failed!");
            goto ERROR_CASE;
        }

        if(CreateParams.pExistingSysMem &&
           CreateParams.ExistingSysMemSize)
        {
            // Client provided own memory and is not assumed to be Gfx aligned
            ExistingSysMem.IsGmmAllocated = 0;

            ExistingSysMem.pExistingSysMem = CreateParams.pExistingSysMem;
            ExistingSysMem.Size            = CreateParams.ExistingSysMemSize;

            // An upper dword of 0xffffffff is invalid and may mean the address
            // was sign extended or came from a rogue UMD. In either case
            // we can truncate the address down to 32 bits prevent attempts
            // to access an invalid address range.
            if((ExistingSysMem.pExistingSysMem & (0xffffffff00000000ull)) == (0xffffffff00000000ull))
            {
                ExistingSysMem.pExistingSysMem &= 0xffffffff;
            }

            //Align the base address to new ESM requirements.
            if(!Surf.ExistingSysMem.IsPageAligned)
            {
                if(GMM_SUCCESS != ApplyExistingSysMemRestrictions())
                {
                    GMM_ASSERTDPF(0, "Malloc'ed Sys Mem too small for gfx surface!");
                    goto ERROR_CASE;
                }
            }
            else
            {
                ExistingSysMem.pVirtAddress =
                ExistingSysMem.pGfxAlignedVirtAddress = CreateParams.pExistingSysMem;
            }

            if((ExistingSysMem.pVirtAddress + Surf.Size) >
               (CreateParams.pExistingSysMem + ExistingSysMem.Size))
            {
                GMM_ASSERTDPF(0, "Malloc'ed Sys Mem too small for gfx surface");
                goto ERROR_CASE;
            }
        }
        else
        {
            __GMM_BUFFER_TYPE Restrictions = {0};

            ExistingSysMem.IsGmmAllocated     = 1;
            Surf.ExistingSysMem.IsPageAligned = 1;

            // Adjust memory size to compensate for Gfx alignment.
            pTextureCalc->GetResRestrictions(&Surf, Restrictions);
            ExistingSysMem.Size = Restrictions.Alignment + Surf.Size;

            ExistingSysMem.pVirtAddress = (uint64_t)GMM_MALLOC(GFX_ULONG_CAST(ExistingSysMem.Size));
            if(!ExistingSysMem.pVirtAddress)
            {
                GMM_ASSERTDPF(0, "Failed to allocate System Accelerated Memory.");
                goto ERROR_CASE;
            }
            else
            {
                ExistingSysMem.pGfxAlignedVirtAddress = (uint64_t)GFX_ALIGN(ExistingSysMem.pVirtAddress, Restrictions.Alignment);
            }
        }
    }

    if(Is64KBPageSuitable() && GetGmmLibContext()->GetSkuTable().FtrLocalMemory)
    {
        // BaseAlignment can be greater than 64KB and needs to be aligned to 64KB
        Surf.Alignment.BaseAlignment = GFX_MAX(GFX_ALIGN(Surf.Alignment.BaseAlignment, GMM_KBYTE(64)), GMM_KBYTE(64));
    }

    GMM_DPF_EXIT;
    return GMM_SUCCESS;

ERROR_CASE:
    //Zero out all the members
    new(this) GmmResourceInfoCommon();

    if(CreateParams.pPreallocatedResInfo)
    {
        this->GetResFlags().Info.__PreallocatedResInfo = 1; // Set flag if PreAllocated ResInfo has been set by the Client.
    }

    GMM_DPF_EXIT;
    return Status;
}

void GmmLib::GmmResourceInfoCommon::UpdateUnAlignedParams()
{
    uint32_t          YHeight = 0, VHeight = 0, Height = 0;
    uint32_t          WidthBytesPhysical = GFX_ULONG_CAST(Surf.BaseWidth) * Surf.BitsPerPixel >> 3;
    GMM_TEXTURE_CALC *pTextureCalc       = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    __GMM_ASSERTPTR(((Surf.TileMode < GMM_TILE_MODES) && (Surf.TileMode >= TILE_NONE)), VOIDRETURN);
    GMM_DPF_ENTER;

    Height = Surf.BaseHeight;

    switch(Surf.Format)
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
                YHeight = GFX_ALIGN(Surf.BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                VHeight = GFX_ALIGN(GFX_CEIL_DIV(Surf.BaseHeight, 2), GMM_IMCx_PLANE_ROW_ALIGNMENT);

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
                YHeight = GFX_ALIGN(Surf.BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                VHeight = GFX_ALIGN(GFX_CEIL_DIV(Surf.BaseHeight, 4), GMM_IMCx_PLANE_ROW_ALIGNMENT);

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
                YHeight = GFX_ALIGN(Surf.BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

                VHeight = GFX_ALIGN(Surf.BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

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

            __GMM_ASSERT((Surf.Pitch & 1) == 0);

            YHeight = GFX_ALIGN(Surf.BaseHeight, GMM_IMCx_PLANE_ROW_ALIGNMENT);

            VHeight = GFX_CEIL_DIV(YHeight, 2);

            break;
        }
        case GMM_FORMAT_I420: // I420 = IYUV
        case GMM_FORMAT_IYUV: // I420/IYUV = YV12 with Swapped U/V
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

            YSize = GFX_ULONG_CAST(Surf.Pitch) * Surf.BaseHeight;

            // YVU9 has one U/V pixel for each 4x4 Y block.
            // The others have one U/V pixel for each 2x2 Y block.

            // YVU9 has a Y:V size ratio of 16 (4x4 --> 1).
            // The others have a ratio of 4 (2x2 --> 1).
            YVSizeRShift = (Surf.Format != GMM_FORMAT_YVU9) ? 2 : 4;

            // If a Y plane isn't fully-aligned to its Y-->U/V block size, the
            // extra/unaligned Y pixels still need corresponding U/V pixels--So
            // for the purpose of computing the UVSize, we must consider a
            // dimensionally "rounded-up" YSize. (E.g. a 13x5 YVU9 Y plane would
            // require 4x2 U/V planes--the same UVSize as a fully-aligned 16x8 Y.)
            YSizeForUVPurposesDimensionalAlignment = (Surf.Format != GMM_FORMAT_YVU9) ? 2 : 4;
            YSizeForUVPurposes =
            GFX_ALIGN(GFX_ULONG_CAST(Surf.Pitch), YSizeForUVPurposesDimensionalAlignment) *
            GFX_ALIGN(Surf.BaseHeight, YSizeForUVPurposesDimensionalAlignment);

            VSize = (YSizeForUVPurposes >> YVSizeRShift);

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
        {
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // YYYYYYYY
            // [UV-Packing]
            YHeight = GFX_ALIGN(Height, __GMM_EVEN_ROW);

            if((Surf.Format == GMM_FORMAT_NV12) ||
               (Surf.Format == GMM_FORMAT_NV21) ||
               (Surf.Format == GMM_FORMAT_P010) ||
               (Surf.Format == GMM_FORMAT_P012) ||
               (Surf.Format == GMM_FORMAT_P016))
            {
                VHeight = GFX_CEIL_DIV(Height, 2);
            }
            else
            {
                VHeight = YHeight; // U/V plane is same as Y
            }

            break;
        }
        default:
        {
            GMM_ASSERTDPF(0, "Unknown Video Format U\n");
            break;
        }
    }

    pTextureCalc->SetPlaneUnAlignedTexOffsetInfo(&Surf, YHeight, VHeight);

}
/////////////////////////////////////////////////////////////////////////////////////
/// Returns downscaled width for fast clear of given subresource
/// @param[in]  uint32_t : MipLevel
/// @return     Width
/////////////////////////////////////////////////////////////////////////////////////
uint64_t GmmLib::GmmResourceInfoCommon::GetFastClearWidth(uint32_t MipLevel)
{
    uint64_t width      = 0;
    uint64_t mipWidth   = GetMipWidth(MipLevel);
    uint32_t numSamples = GetNumSamples();

    GMM_TEXTURE_CALC *pTextureCalc;
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    if(numSamples == 1)
    {
        width = pTextureCalc->ScaleFCRectWidth(&Surf, mipWidth);
    }
    else if(numSamples == 2 || numSamples == 4)
    {
        if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
        {
            width = GFX_ALIGN(mipWidth, 64) / 64;
        }
        else
        {
            width = GFX_ALIGN(mipWidth, 8) / 8;
        }
    }
    else if(numSamples == 8)
    {
        if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
        {
            width = GFX_ALIGN(mipWidth, 16) / 16;
        }
        else
        {
            width = GFX_ALIGN(mipWidth, 2) / 2;
        }
    }
    else // numSamples == 16
    {
        if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
        {
            width = GFX_ALIGN(mipWidth, 8) / 8;
        }
        else
        {
            width = mipWidth;
        }
    }

    return width;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Returns downscaled height for fast clear of given subresource
/// @param[in]  uint32_t : MipLevel
/// @return     height
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmResourceInfoCommon::GetFastClearHeight(uint32_t MipLevel)
{
    uint32_t height     = 0;
    uint32_t mipHeight  = GetMipHeight(MipLevel);
    uint32_t numSamples = GetNumSamples();

    GMM_TEXTURE_CALC *pTextureCalc;
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    if(numSamples == 1)
    {
        height = pTextureCalc->ScaleFCRectHeight(&Surf, mipHeight);
    }
    else
    {
        if (GetGmmLibContext()->GetSkuTable().FtrXe2Compression)
        {
            height = GFX_ALIGN(mipHeight, 4) / 4;
        }
        else
        {
            height = GFX_ALIGN(mipHeight, 2) / 2;
        }
    }

    return height;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Returns 2D Surface width to be used for fast clear for a given 3D surface
/// @param[in]  uint32_t : MipLevel
/// @return     height
/////////////////////////////////////////////////////////////////////////////////////
uint64_t GmmLib::GmmResourceInfoCommon::Get2DFastClearSurfaceWidthFor3DSurface(uint32_t MipLevel)
{
    uint64_t width    = 0;
    uint64_t mipWidth = GetMipWidth(MipLevel);

    GMM_TEXTURE_CALC *pTextureCalc;
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());
    width        = pTextureCalc->Get2DFCSurfaceWidthFor3DSurface(&Surf, mipWidth);
    return width;
}


uint64_t GmmLib::GmmResourceInfoCommon::Get2DFastClearSurfaceHeightFor3DSurface(uint32_t MipLevel)
{
    uint64_t          height    = 0;
    uint32_t          mipHeight = GetMipHeight(MipLevel);
    uint32_t          mipDepth  = GetMipDepth(MipLevel);
    GMM_TEXTURE_CALC *pTextureCalc;

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());
    height       = pTextureCalc->Get2DFCSurfaceHeightFor3DSurface(&Surf, mipHeight, mipDepth);
    return height;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the Platform info.  If Platform has been overriden by the clients, then
/// it returns the overriden Platform Info struct.
/// @return     Reference to the relevent ::GMM_PLATFORM_INFO
/////////////////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO &GmmLib::GmmResourceInfoCommon::GetPlatformInfo()
{
#if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
    if(GFX_GET_CURRENT_RENDERCORE(Surf.Platform) != GFX_GET_CURRENT_RENDERCORE(((Context *)pGmmKmdLibContext)->GetPlatformInfo().Platform))
    {
        return ((Context *)pGmmKmdLibContext)->GetOverridePlatformInfo();
    }
    else
    {
        return ((Context *)pGmmKmdLibContext)->GetPlatformInfo();
    }
#else
    return ((Context *)pGmmUmdLibContext)->GetPlatformInfo();
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns width padded to HAlign. Only called for special flags. See asserts in
/// function for which surfaces are supported.
///
/// @param[in]  MipLevel Mip level for which the width is requested
/// @return     Padded Width
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetPaddedWidth(uint32_t MipLevel)
{
    GMM_TEXTURE_CALC *pTextureCalc;
    uint32_t          AlignedWidth;
    GMM_GFX_SIZE_T    MipWidth;
    uint32_t          HAlign;

    __GMM_ASSERT(MipLevel <= Surf.MaxLod);

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    // This shall be called for Depth and Separate Stencil main surface resource
    // This shall be called for the Aux surfaces (MCS, CCS and Hiz) too.
    // MCS will have Surf.Flags.Gpu.CCS set
    // Hiz will have Surf.Flags.Gpu.HiZ set
    __GMM_ASSERT(Surf.Flags.Gpu.Depth || Surf.Flags.Gpu.SeparateStencil ||
                 Surf.Flags.Gpu.CCS || Surf.Flags.Gpu.HiZ ||
                 AuxSurf.Flags.Gpu.__MsaaTileMcs ||
                 AuxSurf.Flags.Gpu.CCS || AuxSurf.Flags.Gpu.__NonMsaaTileYCcs);

    MipWidth = pTextureCalc->GmmTexGetMipWidth(&Surf, MipLevel);

    HAlign = Surf.Alignment.HAlign;
    if(AuxSurf.Flags.Gpu.CCS && AuxSurf.Flags.Gpu.__NonMsaaTileYCcs)
    {
        HAlign = AuxSurf.Alignment.HAlign;
    }

    AlignedWidth = __GMM_EXPAND_WIDTH(pTextureCalc,
                                      GFX_ULONG_CAST(MipWidth),
                                      HAlign,
                                      &Surf);

    if(Surf.Flags.Gpu.SeparateStencil)
    {
        if(Surf.Flags.Info.TiledW)
        {
            AlignedWidth *= 2;
        }

        // Reverse MSAA Expansion ////////////////////////////////////////////////
        // It might seem strange that we ExpandWidth (with consideration for MSAA)
        // only to "reverse" the MSAA portion of the expansion...It's an order-of-
        // operations thing--The intention of the reversal isn't to have
        // disregarded the original MSAA expansion but to produce a width, that
        // when MSAA'ed will match the true physical width (which requires MSAA
        // consideration to compute).
        switch(Surf.MSAA.NumSamples)
        {
            case 1:
                break;
            case 2: // Same as 4x...
            case 4:
                AlignedWidth /= 2;
                break;
            case 8: // Same as 16x...
            case 16:
                AlignedWidth /= 4;
                break;
            default:
                __GMM_ASSERT(0);
        }
    }

    // CCS Aux surface, Aligned width needs to be scaled based on main surface bpp
    if(AuxSurf.Flags.Gpu.CCS && AuxSurf.Flags.Gpu.__NonMsaaTileYCcs)
    {
        AlignedWidth = pTextureCalc->ScaleTextureWidth(&AuxSurf, AlignedWidth);
    }

    return AlignedWidth;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns height padded to VAlign. Only called for special flags. See asserts in
/// function for which surfaces are supported.
///
/// @param[in]  MipLevel Mip level for which the height is requested
/// @return     Padded height
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetPaddedHeight(uint32_t MipLevel)
{
    GMM_TEXTURE_CALC *pTextureCalc;
    uint32_t          AlignedHeight, MipHeight;
    uint32_t          VAlign;

    __GMM_ASSERT(MipLevel <= Surf.MaxLod);

    // See note in GmmResGetPaddedWidth.
    __GMM_ASSERT(Surf.Flags.Gpu.Depth || Surf.Flags.Gpu.SeparateStencil ||
                 Surf.Flags.Gpu.CCS || Surf.Flags.Gpu.HiZ ||
                 AuxSurf.Flags.Gpu.__MsaaTileMcs ||
                 AuxSurf.Flags.Gpu.CCS || AuxSurf.Flags.Gpu.__NonMsaaTileYCcs);

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    MipHeight = pTextureCalc->GmmTexGetMipHeight(&Surf, MipLevel);

    VAlign = Surf.Alignment.VAlign;
    if(AuxSurf.Flags.Gpu.CCS && AuxSurf.Flags.Gpu.__NonMsaaTileYCcs)
    {
        VAlign = AuxSurf.Alignment.VAlign;
    }

    AlignedHeight = __GMM_EXPAND_HEIGHT(pTextureCalc,
                                        MipHeight,
                                        VAlign,
                                        &Surf);

    if(Surf.Flags.Gpu.SeparateStencil)
    {
        if(Surf.Flags.Info.TiledW)
        {
            AlignedHeight /= 2;
        }

        // Reverse MSAA Expansion ////////////////////////////////////////////////
        // See note in GmmResGetPaddedWidth.
        switch(Surf.MSAA.NumSamples)
        {
            case 1:
                break;
            case 2:
                break; // No height adjustment for 2x...
            case 4:    // Same as 8x...
            case 8:
                AlignedHeight /= 2;
                break;
            case 16:
                AlignedHeight /= 4;
                break;
            default:
                __GMM_ASSERT(0);
        }
    }

    // CCS Aux surface, AlignedHeight needs to be scaled by 16
    if(AuxSurf.Flags.Gpu.CCS && AuxSurf.Flags.Gpu.__NonMsaaTileYCcs)
    {
        AlignedHeight = pTextureCalc->ScaleTextureHeight(&AuxSurf, AlignedHeight);
    }

    return AlignedHeight;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns pitch padded to VAlign. Only called for special flags. See asserts in
/// function for which surfaces are supported.
///
/// @param[in]  MipLevel Mip level for which the pitch is requested
/// @return     Padded pitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetPaddedPitch(uint32_t MipLevel)
{
    uint32_t AlignedWidth;
    uint32_t AlignedPitch;
    uint32_t BitsPerPixel;

    __GMM_ASSERT(MipLevel <= Surf.MaxLod);

    // See note in GetPaddedWidth.
    AlignedWidth = GetPaddedWidth(MipLevel);

    BitsPerPixel = Surf.BitsPerPixel;
    if(AuxSurf.Flags.Gpu.CCS && AuxSurf.Flags.Gpu.__NonMsaaTileYCcs)
    {
        BitsPerPixel = 8; //Aux surface are 8bpp
    }

    AlignedPitch = AlignedWidth * BitsPerPixel >> 3;

    return AlignedPitch;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns resource's QPitch.
///
/// @return     QPitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetQPitch()
{
    const GMM_PLATFORM_INFO *pPlatform;
    uint32_t                 QPitch;

    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());

    __GMM_ASSERT(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN8_CORE);
    __GMM_ASSERT((Surf.Type != RESOURCE_3D) ||
                 (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE));

    // 2D/CUBE    ==> distance in rows between array slices
    // 3D         ==> distance in rows between R-slices
    // Compressed ==> one row contains a complete compression block vertically
    // HiZ        ==> HZ_PxPerByte * HZ_QPitch
    // Stencil    ==> logical, i.e. not halved

    if((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE) &&
       GmmIsCompressed(GetGmmLibContext(), Surf.Format))
    {
        QPitch = Surf.Alignment.QPitch / GetCompressionBlockHeight();

        if((Surf.Type == RESOURCE_3D) && !Surf.Flags.Info.Linear)
        {
            const GMM_TILE_MODE TileMode = Surf.TileMode;
            __GMM_ASSERT(TileMode < GMM_TILE_MODES);
            QPitch = GFX_ALIGN(QPitch, pPlatform->TileInfo[TileMode].LogicalTileHeight);
        }
    }
    else if(Surf.Flags.Gpu.HiZ)
    {
        QPitch = Surf.Alignment.QPitch * pPlatform->HiZPixelsPerByte;
    }
    else
    {
        QPitch = Surf.Alignment.QPitch;
    }

    return QPitch;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns offset information to a particular mip map or plane.
///
/// @param[in][out] Has info about which offset client is requesting. Offset is also
///                 passed back to the client in this parameter.
/// @return         ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetOffset(GMM_REQ_OFFSET_INFO &ReqInfo)
{
    GMM_TEXTURE_CALC *pTextureCalc;

    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    __GMM_ASSERT((pTextureCalc != NULL));

    if(Surf.Flags.Info.RedecribedPlanes)
    {
        uint8_t RestoreReqStdLayout = ReqInfo.ReqStdLayout ? 1 : 0;

        // Lock and Render offsets do not require additional handling
        if(ReqInfo.ReqLock || ReqInfo.ReqRender)
        {
            ReqInfo.ReqStdLayout = 0;
            GmmTexGetMipMapOffset(&Surf, &ReqInfo, GetGmmLibContext());
            ReqInfo.ReqStdLayout = RestoreReqStdLayout;
        }

        if(ReqInfo.ReqStdLayout)
        {
            GMM_REQ_OFFSET_INFO TempReqInfo[GMM_MAX_PLANE] = {0};
	    GMM_TEXTURE_INFO    TexInfo[GMM_MAX_PLANE];
            uint32_t            Plane, TotalPlanes = GmmLib::Utility::GmmGetNumPlanes(Surf.Format);

            // Caller must specify which plane they need the offset into if not
            // getting the whole surface size
            if(ReqInfo.Plane >= GMM_MAX_PLANE ||
               (ReqInfo.StdLayout.Offset != -1 && !ReqInfo.Plane))
            {
                __GMM_ASSERT(0);
                return GMM_ERROR;
            }

            TempReqInfo[GMM_PLANE_Y]         = *&ReqInfo;
            TempReqInfo[GMM_PLANE_Y].Plane   = GMM_NO_PLANE;
            TempReqInfo[GMM_PLANE_Y].ReqLock = TempReqInfo[GMM_PLANE_Y].ReqRender = 0;

            TempReqInfo[GMM_PLANE_V] = TempReqInfo[GMM_PLANE_U] = TempReqInfo[GMM_PLANE_Y];
            
	    pTextureCalc->GetRedescribedPlaneParams(&Surf, GMM_PLANE_Y, &TexInfo[GMM_PLANE_Y]);
            pTextureCalc->GetRedescribedPlaneParams(&Surf, GMM_PLANE_U, &TexInfo[GMM_PLANE_U]);
            pTextureCalc->GetRedescribedPlaneParams(&Surf, GMM_PLANE_V, &TexInfo[GMM_PLANE_V]);

            if(GMM_SUCCESS != GmmTexGetMipMapOffset(&TexInfo[GMM_PLANE_Y], &TempReqInfo[GMM_PLANE_Y], GetGmmLibContext()) ||
               GMM_SUCCESS != GmmTexGetMipMapOffset(&TexInfo[GMM_PLANE_U], &TempReqInfo[GMM_PLANE_U], GetGmmLibContext()) ||
               GMM_SUCCESS != GmmTexGetMipMapOffset(&TexInfo[GMM_PLANE_V], &TempReqInfo[GMM_PLANE_V], GetGmmLibContext()))
	    {
    		    __GMM_ASSERT(0);
                return GMM_ERROR;
            }

            ReqInfo.StdLayout.TileDepthPitch = TempReqInfo[ReqInfo.Plane].StdLayout.TileDepthPitch;
            ReqInfo.StdLayout.TileRowPitch   = TempReqInfo[ReqInfo.Plane].StdLayout.TileRowPitch;

            if(ReqInfo.StdLayout.Offset == -1)
            {
                // Special request to get the StdLayout size
                ReqInfo.StdLayout.Offset = TempReqInfo[ReqInfo.Plane].StdLayout.Offset;

                if(!ReqInfo.Plane)
                {
                    for(Plane = GMM_PLANE_Y; Plane <= TotalPlanes; Plane++)
                    {
                        ReqInfo.StdLayout.Offset += TempReqInfo[Plane].StdLayout.Offset;
                    }
                }
            }
            else
            {
                ReqInfo.StdLayout.Offset = TempReqInfo[ReqInfo.Plane].StdLayout.Offset;

                for(Plane = GMM_PLANE_Y; Plane < (uint32_t)ReqInfo.Plane; Plane++)
                {
                    // Find the size of the previous planes and add it to the offset
                    TempReqInfo[Plane].StdLayout.Offset = -1;
                
                    if(GMM_SUCCESS != GmmTexGetMipMapOffset(&TexInfo[Plane], &TempReqInfo[Plane], GetGmmLibContext()))
                    {
                 
                        __GMM_ASSERT(0);
                        return GMM_ERROR;
                    }

                    ReqInfo.StdLayout.Offset += TempReqInfo[Plane].StdLayout.Offset;
                }
            }
        }

        return GMM_SUCCESS;
    }
    else
    {
        return GmmTexGetMipMapOffset(&Surf, &ReqInfo, GetGmmLibContext());
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Performs a CPU BLT between a specified GPU resource and a system memory surface,
/// as defined by the GMM_RES_COPY_BLT descriptor.
///
/// @param[in]  pBlt: Describes the blit operation. See ::GMM_RES_COPY_BLT for more info.
/// @return     1 if succeeded, 0 otherwise
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::CpuBlt(GMM_RES_COPY_BLT *pBlt)
{
#define REQUIRE(e)       \
    if(!(e))             \
    {                    \
        __GMM_ASSERT(0); \
        Success = 0;     \
        goto EXIT;       \
    }

    const GMM_PLATFORM_INFO *pPlatform;
    uint8_t                  Success = 1;
    GMM_TEXTURE_INFO *       pTexInfo;
    GMM_TEXTURE_CALC *       pTextureCalc;
    GMM_TEXTURE_INFO         RedescribedPlaneInfo;

    __GMM_ASSERTPTR(pBlt, 0);

    pPlatform    = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    __GMM_ASSERT(
    Surf.Type == RESOURCE_1D ||
    Surf.Type == RESOURCE_2D ||
    Surf.Type == RESOURCE_PRIMARY ||
    Surf.Type == RESOURCE_CUBE ||
    Surf.Type == RESOURCE_3D);
    __GMM_ASSERT(pBlt->Gpu.MipLevel <= Surf.MaxLod);
    __GMM_ASSERT(Surf.MSAA.NumSamples <= 1);                          // Supported by CpuSwizzleBlt--but not yet this function.
    __GMM_ASSERT(!Surf.Flags.Gpu.Depth || Surf.MSAA.NumSamples <= 1); // MSAA depth currently ends up with a few exchange swizzles--CpuSwizzleBlt could support with expanded XOR'ing, but probably no use case.
    __GMM_ASSERT(!(
    pBlt->Blt.Upload &&
    Surf.Flags.Gpu.Depth &&
    (Surf.BitsPerPixel == 32) &&
    (pBlt->Sys.PixelPitch == 4) &&
    (pBlt->Blt.BytesPerPixel == 3))); // When uploading D24 data from D24S8 to D24X8, no harm in copying S8 to X8 and upload will then be faster.

    pTexInfo = &(Surf);

    // YUV Planar surface
    if(pTextureCalc->IsTileAlignedPlanes(pTexInfo) && GmmIsPlanar(Surf.Format))
    {
        uint32_t PlaneId = GMM_NO_PLANE;
	
	pTextureCalc->GetPlaneIdForCpuBlt(pTexInfo, pBlt, &PlaneId);
        
	if(PlaneId == GMM_MAX_PLANE)
        {
            // TODO BLT rect should not overlap between planes.
            {
                // __GMM_ASSERT(0); // decide later, for now blt it
                //return 0;
            }

            // BLT monolithic surface per plane and remove padding due to tiling.
            for(PlaneId = GMM_PLANE_Y; PlaneId <= pTextureCalc->GetNumberOfPlanes(pTexInfo); PlaneId++)
            {
                pTextureCalc->GetBltInfoPerPlane(pTexInfo, pBlt, PlaneId);
                CpuBlt(pBlt);
            }
        }
        // else  continue below
    }

    // UV packed planar surfaces will have different tiling geometries for the
    // Y and UV planes. Blts cannot span across the tiling boundaries and we
    // must select the proper mode for each plane. Non-UV packed formats will
    // have a constant tiling mode, and so do not have the same limits
    if(Surf.Flags.Info.RedecribedPlanes &&
       GmmIsUVPacked(Surf.Format))
    {
        if(!((pBlt->Gpu.OffsetY >= pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U]) ||
             ((pBlt->Gpu.OffsetY + pBlt->Blt.Height) <= pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U])))
        {
            __GMM_ASSERT(0);
            return false;
        }

        if(pBlt->Gpu.OffsetY < pTexInfo->OffsetInfo.Plane.Y[GMM_PLANE_U])
        {
	    pTextureCalc->GetRedescribedPlaneParams(pTexInfo, GMM_PLANE_Y, &RedescribedPlaneInfo);
            // Y Plane
            pTexInfo = &RedescribedPlaneInfo;
        }
        else
        {
            // UV Plane
	    pTextureCalc->GetRedescribedPlaneParams(pTexInfo, GMM_PLANE_U, &RedescribedPlaneInfo);
            pTexInfo = &RedescribedPlaneInfo;
        }
    }

    if(pBlt->Blt.Slices > 1)
    {
        GMM_RES_COPY_BLT SliceBlt = *pBlt;
        uint32_t         Slice;

        SliceBlt.Blt.Slices = 1;
        for(Slice = pBlt->Gpu.Slice;
            Slice < (pBlt->Gpu.Slice + pBlt->Blt.Slices);
            Slice++)
        {
            SliceBlt.Gpu.Slice      = Slice;
            SliceBlt.Sys.pData      = (void *)((char *)pBlt->Sys.pData + (Slice - pBlt->Gpu.Slice) * pBlt->Sys.SlicePitch);
            SliceBlt.Sys.BufferSize = pBlt->Sys.BufferSize - GFX_ULONG_CAST((char *)SliceBlt.Sys.pData - (char *)pBlt->Sys.pData);
            CpuBlt(&SliceBlt);
        }
    }
    else // Single Subresource...
    {
        uint32_t            ResPixelPitch = pTexInfo->BitsPerPixel / CHAR_BIT;
        uint32_t            BlockWidth, BlockHeight, BlockDepth;
        uint32_t            __CopyWidthBytes, __CopyHeight, __OffsetXBytes, __OffsetY;
        GMM_REQ_OFFSET_INFO GetOffset = {0};

        pTextureCalc->GetCompressionBlockDimensions(pTexInfo->Format, &BlockWidth, &BlockHeight, &BlockDepth);

#if(LHDM)
        if(pTexInfo->MsFormat == D3DDDIFMT_G8R8_G8B8 ||
           pTexInfo->MsFormat == D3DDDIFMT_R8G8_B8G8)
        {
            BlockWidth    = 2;
            ResPixelPitch = 4;
        }
#endif

        { // __CopyWidthBytes...
            uint32_t Width;

            if(!pBlt->Blt.Width) // i.e. "Full Width"
            {
                __GMM_ASSERT(!GmmIsPlanar(pTexInfo->Format)); // Caller must set Blt.Width--GMM "auto-size on zero" not supported with planars since multiple interpretations would confuse more than help.

                Width = GFX_ULONG_CAST(pTextureCalc->GmmTexGetMipWidth(pTexInfo, pBlt->Gpu.MipLevel));

                __GMM_ASSERT(Width >= pBlt->Gpu.OffsetX);
                Width -= pBlt->Gpu.OffsetX;
                __GMM_ASSERT(Width);
            }
            else
            {
                Width = pBlt->Blt.Width;
            }

            if(((pBlt->Sys.PixelPitch == 0) ||
                (pBlt->Sys.PixelPitch == ResPixelPitch)) &&
               ((pBlt->Blt.BytesPerPixel == 0) ||
                (pBlt->Blt.BytesPerPixel == ResPixelPitch)))
            {
                // Full-Pixel BLT...
                __CopyWidthBytes =
                GFX_CEIL_DIV(Width, BlockWidth) * ResPixelPitch;
            }
            else // Partial-Pixel BLT...
            {
                __GMM_ASSERT(BlockWidth == 1); // No partial-pixel support for block-compressed formats.

                // When copying between surfaces with different pixel pitches,
                // specify CopyWidthBytes in terms of unswizzled surface
                // (convenient convention used by CpuSwizzleBlt).
                __CopyWidthBytes =
                Width *
                (pBlt->Sys.PixelPitch ?
                 pBlt->Sys.PixelPitch :
                 ResPixelPitch);
            }
        }

        {                         // __CopyHeight...
            if(!pBlt->Blt.Height) // i.e. "Full Height"
            {
                __GMM_ASSERT(!GmmIsPlanar(pTexInfo->Format)); // Caller must set Blt.Height--GMM "auto-size on zero" not supported with planars since multiple interpretations would confuse more than help.

                __CopyHeight = pTextureCalc->GmmTexGetMipHeight(pTexInfo, pBlt->Gpu.MipLevel);
                __GMM_ASSERT(__CopyHeight >= pBlt->Gpu.OffsetY);
                __CopyHeight -= pBlt->Gpu.OffsetY;
                __GMM_ASSERT(__CopyHeight);
            }
            else
            {
                __CopyHeight = pBlt->Blt.Height;
            }

            __CopyHeight = GFX_CEIL_DIV(__CopyHeight, BlockHeight);
        }

        __GMM_ASSERT((pBlt->Gpu.OffsetX % BlockWidth) == 0);
        __OffsetXBytes = (pBlt->Gpu.OffsetX / BlockWidth) * ResPixelPitch + pBlt->Gpu.OffsetSubpixel;

        __GMM_ASSERT((pBlt->Gpu.OffsetY % BlockHeight) == 0);
        __OffsetY = (pBlt->Gpu.OffsetY / BlockHeight);

        { // Get pResData Offsets to this subresource...
            GetOffset.ReqLock      = pTexInfo->Flags.Info.Linear;
            GetOffset.ReqStdLayout = !GetOffset.ReqLock && pTexInfo->Flags.Info.StdSwizzle;
            GetOffset.ReqRender    = !GetOffset.ReqLock && !GetOffset.ReqStdLayout;
            GetOffset.MipLevel     = pBlt->Gpu.MipLevel;
            switch(pTexInfo->Type)
            {
                case RESOURCE_1D:
                case RESOURCE_2D:
                case RESOURCE_PRIMARY:
                {
                    GetOffset.ArrayIndex = pBlt->Gpu.Slice;
                    break;
                }
                case RESOURCE_CUBE:
                {
                    GetOffset.ArrayIndex = pBlt->Gpu.Slice / 6;
                    GetOffset.CubeFace   = (GMM_CUBE_FACE_ENUM)(pBlt->Gpu.Slice % 6);
                    break;
                }
                case RESOURCE_3D:
                {
                    GetOffset.Slice = (GMM_IS_64KB_TILE(pTexInfo->Flags) || pTexInfo->Flags.Info.TiledYf) ?
                                      (pBlt->Gpu.Slice / pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileDepth) :
                                      pBlt->Gpu.Slice;
                    break;
                }
                default:
                    __GMM_ASSERT(0);
            }

            REQUIRE(this->GetOffset(GetOffset) == GMM_SUCCESS);
        }

        if(pTexInfo->Flags.Info.Linear)
        {
            char *   pDest, *pSrc;
            uint32_t DestPitch, SrcPitch;
            uint32_t y;

            __GMM_ASSERT( // Linear-to-linear subpixel BLT unexpected--Not implemented.
            (!pBlt->Sys.PixelPitch || (pBlt->Sys.PixelPitch == ResPixelPitch)) &&
            (!pBlt->Blt.BytesPerPixel || (pBlt->Blt.BytesPerPixel == ResPixelPitch)));

            if(pBlt->Blt.Upload)
            {
                pDest     = (char *)pBlt->Gpu.pData;
                DestPitch = GFX_ULONG_CAST(pTexInfo->Pitch);

                pSrc     = (char *)pBlt->Sys.pData;
                SrcPitch = pBlt->Sys.RowPitch;
            }
            else
            {
                pDest     = (char *)pBlt->Sys.pData;
                DestPitch = pBlt->Sys.RowPitch;

                pSrc     = (char *)pBlt->Gpu.pData;
                SrcPitch = GFX_ULONG_CAST(pTexInfo->Pitch);
            }

            __GMM_ASSERT(GetOffset.Lock.Offset < pTexInfo->Size);
            pDest += GetOffset.Lock.Offset + (__OffsetY * DestPitch + __OffsetXBytes);

            for(y = 0; y < __CopyHeight; y++)
            {
// Memcpy per row isn't optimal, but doubt this linear-to-linear path matters.

#if _WIN32
#ifdef __GMM_KMD__
                GFX_MEMCPY_S
#else
                memcpy_s
#endif
                (pDest, __CopyWidthBytes, pSrc, __CopyWidthBytes);
#else
                memcpy(pDest, pSrc, __CopyWidthBytes);
#endif
                pDest += DestPitch;
                pSrc += SrcPitch;
            }
        }
        else // Swizzled BLT...
        {
            CPU_SWIZZLE_BLT_SURFACE LinearSurface = {0}, SwizzledSurface;
            uint32_t                ZOffset       = 0;

            __GMM_ASSERT(GetOffset.Render.Offset64 < pTexInfo->Size);

            ZOffset = (pTexInfo->Type == RESOURCE_3D &&
                       (GMM_IS_64KB_TILE(pTexInfo->Flags) || pTexInfo->Flags.Info.TiledYf)) ?
                      (pBlt->Gpu.Slice % pPlatform->TileInfo[pTexInfo->TileMode].LogicalTileDepth) :
                      0;

            if(pTexInfo->Flags.Info.StdSwizzle == 1)
            {
                SwizzledSurface.pBase   = (char *)pBlt->Gpu.pData + GFX_ULONG_CAST(GetOffset.StdLayout.Offset);
                SwizzledSurface.OffsetX = __OffsetXBytes;
                SwizzledSurface.OffsetY = __OffsetY;
                SwizzledSurface.OffsetZ = ZOffset;

                uint32_t MipWidth  = GFX_ULONG_CAST(pTextureCalc->GmmTexGetMipWidth(pTexInfo, pBlt->Gpu.MipLevel));
                uint32_t MipHeight = pTextureCalc->GmmTexGetMipHeight(pTexInfo, pBlt->Gpu.MipLevel);

                pTextureCalc->AlignTexHeightWidth(pTexInfo, &MipHeight, &MipWidth);
                SwizzledSurface.Height = MipHeight;
                SwizzledSurface.Pitch  = MipWidth * ResPixelPitch;
            }
            else
            {
                SwizzledSurface.pBase   = (char *)pBlt->Gpu.pData + GFX_ULONG_CAST(GetOffset.Render.Offset64);
                SwizzledSurface.Pitch   = GFX_ULONG_CAST(pTexInfo->Pitch);
                SwizzledSurface.OffsetX = GetOffset.Render.XOffset + __OffsetXBytes;
                SwizzledSurface.OffsetY = GetOffset.Render.YOffset + __OffsetY;
                SwizzledSurface.OffsetZ = GetOffset.Render.ZOffset + ZOffset;
                SwizzledSurface.Height  = GFX_ULONG_CAST(pTexInfo->Size / pTexInfo->Pitch);
            }

            SwizzledSurface.Element.Pitch = ResPixelPitch;

            LinearSurface.pBase = pBlt->Sys.pData;
            LinearSurface.Pitch = pBlt->Sys.RowPitch;
            LinearSurface.Height =
            pBlt->Sys.BufferSize /
            (pBlt->Sys.RowPitch ?
             pBlt->Sys.RowPitch :
             pBlt->Sys.BufferSize);
            LinearSurface.Element.Pitch =
            pBlt->Sys.PixelPitch ?
            pBlt->Sys.PixelPitch :
            ResPixelPitch;
            LinearSurface.Element.Size =
            SwizzledSurface.Element.Size =
            pBlt->Blt.BytesPerPixel ?
            pBlt->Blt.BytesPerPixel :
            ResPixelPitch;

            SwizzledSurface.pSwizzle = NULL;

            if(pTexInfo->Flags.Info.TiledW)
            {
                SwizzledSurface.pSwizzle = &INTEL_TILE_W;

                // Correct for GMM's 2x Pitch handling of stencil...
                // (Unlike the HW, CpuSwizzleBlt handles TileW as a natural,
                // 64x64=4KB tile, so the pre-Gen10 "double-pitch/half-height"
                // kludging to TileY shape must be reversed.)
                __GMM_ASSERT((SwizzledSurface.Pitch % 2) == 0);
                SwizzledSurface.Pitch /= 2;
                SwizzledSurface.Height *= 2;
            }
            else if(GMM_IS_4KB_TILE(pTexInfo->Flags) &&
                    !(pTexInfo->Flags.Info.TiledYf ||
                      GMM_IS_64KB_TILE(pTexInfo->Flags)))
            {
                if(GetGmmLibContext()->GetSkuTable().FtrTileY)
                 {
                    SwizzledSurface.pSwizzle = &INTEL_TILE_Y;
                }
                else
                {
                    SwizzledSurface.pSwizzle = &INTEL_TILE_4;
                }
            }
            else if(pTexInfo->Flags.Info.TiledX)
            {
                SwizzledSurface.pSwizzle = &INTEL_TILE_X;
            }
            else // Yf/s...
            {
// clang-format off
                #define NA

                #define CASE(Layout, Tile, msaa, xD, bpe)                               \
                    case bpe:                                                           \
                        SwizzledSurface.pSwizzle = &Layout##_##Tile##_##msaa##xD##bpe;  \
                        break

                #define SWITCH_BPP(Layout, Tile, msaa, xD)    \
                    switch(pTexInfo->BitsPerPixel)            \
                    {                                         \
                        CASE(Layout, Tile, msaa, xD, 8);      \
                        CASE(Layout, Tile, msaa, xD, 16);     \
                        CASE(Layout, Tile, msaa, xD, 32);     \
                        CASE(Layout, Tile, msaa, xD, 64);     \
                        CASE(Layout, Tile, msaa, xD, 128);    \
                    }

                #define SWITCH_MSAA_TILE64(Layout, Tile, xD)     \
                {\
                    switch(pTexInfo->MSAA.NumSamples)           \
                    {                                           \
                        case 0:                                 \
                            SWITCH_BPP(Layout, TILE_64, , xD);    \
                            break;                              \
                        case 1:                                 \
                            SWITCH_BPP(Layout, TILE_64, , xD);    \
                            break;                              \
                        case 2:                                 \
                            if(GetGmmLibContext()->GetSkuTable().FtrXe2PlusTiling)\
                            { \
                                SWITCH_BPP(Layout, TILE_64_V2, MSAA2_, xD); \
                            }\
                            else\
                            { \
                                SWITCH_BPP(Layout, TILE_64, MSAA2_, xD); \
                            } \
                            break;                              \
                        case 4:                                 \
                            if(GetGmmLibContext()->GetSkuTable().FtrXe2PlusTiling)\
                            { \
                                SWITCH_BPP(Layout, TILE_64_V2, MSAA4_, xD); \
                            }\
                            else\
                            { \
                                SWITCH_BPP(Layout, TILE_64, MSAA_, xD); \
                            } \
                            break; \
                        case 8:                                 \
                            if(GetGmmLibContext()->GetSkuTable().FtrXe2PlusTiling)\
                            { \
                                SWITCH_BPP(Layout, TILE_64_V2, MSAA8_, xD); \
                            }\
                            else\
                            { \
                                SWITCH_BPP(Layout, TILE_64, MSAA_, xD); \
                            } \
                            break; \
                        case 16:                                \
                            if(GetGmmLibContext()->GetSkuTable().FtrXe2PlusTiling)\
                            { \
                                SWITCH_BPP(Layout, TILE_64_V2, MSAA16_, xD); \
                            }\
                            else\
                            { \
                                SWITCH_BPP(Layout, TILE_64, MSAA_, xD); \
                            } \
                            break; \
                    }                                           \
                } \

                #define SWITCH_MSAA(Layout, Tile, xD)           \
                {\
                    switch(pTexInfo->MSAA.NumSamples)           \
                    {                                           \
                        case 0:                                 \
                            SWITCH_BPP(Layout, Tile, , xD);     \
                            break;                              \
                        case 1:                                 \
                            SWITCH_BPP(Layout, Tile, , xD);     \
                            break;                              \
                        case 2:                                 \
                            SWITCH_BPP(Layout, Tile, MSAA2_, xD);  \
                            break;                              \
                        case 4:                                 \
                            SWITCH_BPP(Layout, Tile, MSAA4_, xD);  \
                            break;                              \
                        case 8:                                 \
                            SWITCH_BPP(Layout, Tile, MSAA8_, xD);     \
                            break;                              \
                        case 16:                                \
                            SWITCH_BPP(Layout, Tile, MSAA16_, xD);    \
                            break;                              \
                    }\
                }
                // clang-format on

                if(pTexInfo->Type == RESOURCE_3D)
                {
                    if(pTexInfo->Flags.Info.TiledYf)
                    {
                        SWITCH_BPP(INTEL, TILE_YF, , 3D_);
                    }
                    else if(GMM_IS_64KB_TILE(pTexInfo->Flags))
                    {
                        if(GetGmmLibContext()->GetSkuTable().FtrTileY)
                        {
                            SWITCH_BPP(INTEL, TILE_YS, , 3D_);
                        }
                        else
                        {
                            if (GetGmmLibContext()->GetSkuTable().FtrXe2PlusTiling)
                            {
                                SWITCH_BPP(INTEL, TILE_64_V2, , 3D_);
                            }
                            else
                            {
                                SWITCH_BPP(INTEL, TILE_64, , 3D_);
                            }
                        }
                    }
                }
                else // 2D/Cube...
                {
                    if(pTexInfo->Flags.Info.TiledYf)
                    {
                        SWITCH_MSAA(INTEL, TILE_YF, );
                    }
                    else if(GMM_IS_64KB_TILE(pTexInfo->Flags))
                    {
                        if(GetGmmLibContext()->GetSkuTable().FtrTileY)
                        {
                            SWITCH_MSAA(INTEL, TILE_YS, );
                        }
                        else
                        {
                            SWITCH_MSAA_TILE64(INTEL, TILE_64, );
                        }
                    }
                }
            }
            __GMM_ASSERT(SwizzledSurface.pSwizzle);

            if(pBlt->Blt.Upload)
            {
                CpuSwizzleBlt(&SwizzledSurface, &LinearSurface, __CopyWidthBytes, __CopyHeight);
            }
            else
            {
                CpuSwizzleBlt(&LinearSurface, &SwizzledSurface, __CopyWidthBytes, __CopyHeight);
            }
        }
    }

EXIT:

    return Success;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Helper function that helps UMDs map in the surface in a layout that
/// our HW understands. Clients call this function in a loop until it
/// returns failure. Clients will get back information in pMapping->Span,
/// which they can use to map Span.Size bytes to Span.VirtualOffset gfx
/// address with Span.PhysicalOffset physical page.
///
/// @param[in]  pMapping: Clients call the function with initially zero'd out GMM_GET_MAPPING.
/// @return      1 if more span descriptors to report, 0 if all mapping is done
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetMappingSpanDesc(GMM_GET_MAPPING *pMapping)
{
    const GMM_PLATFORM_INFO *pPlatform;
    uint8_t                  WasFinalSpan = 0;
    GMM_TEXTURE_INFO *       pTexInfo;
    GMM_TEXTURE_CALC *       pTextureCalc;
    GMM_TEXTURE_INFO         RedescribedPlaneInfo;
    bool Aux = false;
    
    __GMM_ASSERT(Surf.Flags.Info.StdSwizzle);

    pPlatform    = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());

    __GMM_ASSERT(pTextureCalc != NULL);
    pTexInfo = &Surf;

    if((pMapping->Type == GMM_MAPPING_YUVPLANAR_AUX || pMapping->Type == GMM_MAPPING_YUVPLANAR) && GmmIsPlanar(Surf.Format))    
    {
        uint32_t            Plane;
        GMM_REQ_OFFSET_INFO ReqInfo = {0}, NextSpanReqInfo = {0};
        GMM_GFX_SIZE_T      SpanPhysicalOffset, SpanVirtualOffset;

        if(pMapping->Type == GMM_MAPPING_YUVPLANAR_AUX)
        {
            // Unpack GMM_MAPPING_YUVPLANAR and Aux from caller function.
            // Applicalble only for Aux mapping for Y/UV Plane
            // GMM_MAPPING_YUVPLANAR_AUX is unpacked as (GMM_MAPPING_YUVPLANAR , Aux)
            Aux = true;
        }

	if(Aux)
        {
            memset(pMapping, 0, sizeof(*pMapping));
            pMapping->Type                  = GMM_MAPPING_YUVPLANAR;
            WasFinalSpan                    = 1;
            SpanPhysicalOffset              = GetSizeMainSurfacePhysical();
            SpanVirtualOffset               = GetSizeMainSurface();
            NextSpanReqInfo.Lock.Offset64   = SpanPhysicalOffset + GetSizeAuxSurface(GMM_AUX_SURF);
            NextSpanReqInfo.Render.Offset64 = GetSizeSurface();
        }
        else
        {
            if(pMapping->Scratch.Plane == GMM_NO_PLANE)
            {
                uint32_t ArrayIndex = pMapping->Scratch.Slice;
                memset(pMapping, 0, sizeof(*pMapping));
                pMapping->Type = GMM_MAPPING_YUVPLANAR;
		pMapping->Scratch.Plane      = GMM_PLANE_Y;
                pMapping->Scratch.Slice = ArrayIndex;

                SpanPhysicalOffset = SpanVirtualOffset = 0;
                if(GmmLib::Utility::GmmGetNumPlanes(Surf.Format) == GMM_PLANE_V)
                {
                    pMapping->Scratch.LastPlane = GMM_PLANE_V;
                }
                else
                {
                    pMapping->Scratch.LastPlane = GMM_PLANE_U;
                }

                Plane = pMapping->Scratch.Plane;
            }
            else
            {
                // If we've crossed into a new plane then need to reset
                // the current mapping info and adjust the mapping
                // params accordingly

                Plane                   = pMapping->Scratch.Plane + 1;
                GMM_YUV_PLANE LastPlane = pMapping->Scratch.LastPlane;
                SpanPhysicalOffset      = pMapping->__NextSpan.PhysicalOffset;
                SpanVirtualOffset       = pMapping->__NextSpan.VirtualOffset;
                uint32_t ArrayIndex     = pMapping->Scratch.Slice;
                memset(pMapping, 0, sizeof(*pMapping));

                pMapping->Type               = GMM_MAPPING_YUVPLANAR;
                pMapping->Scratch.Plane      = GMM_YUV_PLANE(Plane);
                pMapping->Scratch.LastPlane  = LastPlane;
                pMapping->Scratch.Slice     = ArrayIndex;
            }
            {
                if(pMapping->Scratch.Plane == GMM_PLANE_Y)
                {
                    ReqInfo.ReqRender = ReqInfo.ReqLock = 1;
                    ReqInfo.Plane                       = GMM_YUV_PLANE(Plane);
                    ReqInfo.ArrayIndex                  = pMapping->Scratch.Slice;
                    this->GetOffset(ReqInfo);
                    SpanPhysicalOffset = ReqInfo.Lock.Offset64;
                    SpanVirtualOffset  = ReqInfo.Render.Offset64;
                }
                if(GMM_YUV_PLANE(Plane) < pMapping->Scratch.LastPlane)
                {
                    NextSpanReqInfo.ReqRender = NextSpanReqInfo.ReqLock = 1;
                    NextSpanReqInfo.Plane                               = GMM_YUV_PLANE(Plane + 1);
                    NextSpanReqInfo.ArrayIndex                          = pMapping->Scratch.Slice;
                    this->GetOffset(NextSpanReqInfo);
                }
                else // last plane of that array
                {
                    NextSpanReqInfo.Lock.Offset64   = (GetSizeMainSurfacePhysical() / GFX_MAX(Surf.ArraySize, 1)) * (pMapping->Scratch.Slice + 1);
                    NextSpanReqInfo.Render.Offset64 = (GetSizeMainSurface() / GFX_MAX(Surf.ArraySize, 1)) * (pMapping->Scratch.Slice + 1);
                    WasFinalSpan                    = 1;
                }
            }
        }
        // Plane offsets
        pMapping->Span.PhysicalOffset       = SpanPhysicalOffset;
        pMapping->Span.VirtualOffset        = SpanVirtualOffset;
        pMapping->__NextSpan.PhysicalOffset = NextSpanReqInfo.Lock.Offset64;
        pMapping->__NextSpan.VirtualOffset  = NextSpanReqInfo.Render.Offset64;
        pMapping->Span.Size                 = pMapping->__NextSpan.PhysicalOffset - pMapping->Span.PhysicalOffset;

    }
    else if(pMapping->Type == GMM_MAPPING_GEN9_YS_TO_STDSWIZZLE)
    {
        __GMM_ASSERT(Surf.Flags.Info.StdSwizzle);
    
        const uint32_t TileSize = GMM_KBYTE(64);

        __GMM_ASSERT(Surf.Flags.Info.TiledYs);
        __GMM_ASSERT(
        (Surf.Type == RESOURCE_2D) ||
        (Surf.Type == RESOURCE_3D) ||
        (Surf.Type == RESOURCE_CUBE));
        __GMM_ASSERT(Surf.Flags.Gpu.Depth == 0); // TODO(Minor): Proper StdSwizzle exemptions?
        __GMM_ASSERT(Surf.Flags.Gpu.SeparateStencil == 0);

        __GMM_ASSERT(AuxSurf.Size == 0);       // TODO(Medium): Support not yet implemented, but DX12 UMD not using yet.
        __GMM_ASSERT(Surf.Flags.Gpu.MMC == 0); // TODO(Medium): Support not yet implemented, but not yet needed for DX12.

        // For planar surfaces we need to reorder the planes into what HW expects.
        // OS will provide planes in [Y0][Y1][U0][U1][V0][V1] order while
        // HW requires them to be in [Y0][U0][V0][Y1][U1][V1] order
        if(Surf.Flags.Info.RedecribedPlanes)
        {
            if(pMapping->Scratch.Plane == GMM_NO_PLANE)
            {
                pMapping->Scratch.Plane = GMM_PLANE_Y;
                if(GmmLib::Utility::GmmGetNumPlanes(Surf.Format) == GMM_PLANE_V)
                {
                    pMapping->Scratch.LastPlane = GMM_PLANE_V;
                }
                else
                {
                    pMapping->Scratch.LastPlane = GMM_PLANE_U;
                }
            }
            else if(pMapping->Scratch.Row == pMapping->Scratch.Rows)
            {
                // If we've crossed into a new plane then need to reset
                // the current mapping info and adjust the mapping
                // params accordingly
                GMM_REQ_OFFSET_INFO ReqInfo   = {0};
                uint32_t            Plane     = pMapping->Scratch.Plane + 1;
                GMM_YUV_PLANE       LastPlane = pMapping->Scratch.LastPlane;

                memset(pMapping, 0, sizeof(*pMapping));

                pMapping->Type              = GMM_MAPPING_GEN9_YS_TO_STDSWIZZLE;
                pMapping->Scratch.Plane     = GMM_YUV_PLANE(Plane);
                pMapping->Scratch.LastPlane = LastPlane;

                ReqInfo.ReqRender = ReqInfo.ReqStdLayout = 1;
                ReqInfo.Plane                            = GMM_YUV_PLANE(Plane);

                this->GetOffset(ReqInfo);

                pMapping->__NextSpan.PhysicalOffset = ReqInfo.StdLayout.Offset;
                pMapping->__NextSpan.VirtualOffset  = ReqInfo.Render.Offset64;
            }

	    pTextureCalc->GetRedescribedPlaneParams(pTexInfo, GMM_PLANE_Y, &RedescribedPlaneInfo);
            pTexInfo = &RedescribedPlaneInfo;

        }

        // Initialization of Mapping Params...
        if(pMapping->Scratch.Element.Width == 0) // i.e. initially zero'ed struct.
        {
            uint32_t BytesPerElement = pTexInfo->BitsPerPixel / CHAR_BIT;

            pMapping->Scratch.EffectiveLodMax = GFX_MIN(pTexInfo->MaxLod, pTexInfo->Alignment.MipTailStartLod);

            pTextureCalc->GetCompressionBlockDimensions(
            pTexInfo->Format,
            &pMapping->Scratch.Element.Width,
            &pMapping->Scratch.Element.Height,
            &pMapping->Scratch.Element.Depth);

            { // Tile Dimensions...
                GMM_TILE_MODE TileMode = pTexInfo->TileMode;
                __GMM_ASSERT(TileMode < GMM_TILE_MODES);

                // Get Tile Logical Tile Dimensions (i.e. uncompressed pixels)...
                pMapping->Scratch.Tile.Width =
                (pPlatform->TileInfo[TileMode].LogicalTileWidth / BytesPerElement) *
                pMapping->Scratch.Element.Width;

                pMapping->Scratch.Tile.Height =
                pPlatform->TileInfo[TileMode].LogicalTileHeight *
                pMapping->Scratch.Element.Height;

                pMapping->Scratch.Tile.Depth =
                pPlatform->TileInfo[TileMode].LogicalTileDepth *
                pMapping->Scratch.Element.Depth;

                pMapping->Scratch.RowPitchVirtual =
                GFX_ULONG_CAST(pTexInfo->Pitch) *
                pPlatform->TileInfo[TileMode].LogicalTileHeight *
                pPlatform->TileInfo[TileMode].LogicalTileDepth;
            }

            { // Slice...
                uint32_t Lod;
                uint32_t LodsPerSlice =
                (pTexInfo->Type != RESOURCE_3D) ?
                pMapping->Scratch.EffectiveLodMax + 1 :
                1; // 3D Std Swizzle traverses slices before MIP's.

                if(pMapping->Scratch.Plane)
                {
                    // If planar then we need the parent descriptors planar pitch
                    pMapping->Scratch.SlicePitch.Virtual =
                    GFX_ULONG_CAST(Surf.OffsetInfo.Plane.ArrayQPitch) *
                    (pMapping->Scratch.Tile.Depth / pMapping->Scratch.Element.Depth);
                }
                else
                {
                    pMapping->Scratch.SlicePitch.Virtual =
                    GFX_ULONG_CAST(pTexInfo->OffsetInfo.Texture2DOffsetInfo.ArrayQPitchRender) *
                    (pMapping->Scratch.Tile.Depth / pMapping->Scratch.Element.Depth);
                }

                // SlicePitch.Physical...
                __GMM_ASSERT(pMapping->Scratch.SlicePitch.Physical == 0);
                for(Lod = 0; Lod < LodsPerSlice; Lod++)
                {
                    uint32_t       MipCols, MipRows;
                    GMM_GFX_SIZE_T MipWidth;
                    uint32_t       MipHeight;

                    MipWidth  = pTextureCalc->GmmTexGetMipWidth(pTexInfo, Lod);
                    MipHeight = pTextureCalc->GmmTexGetMipHeight(pTexInfo, Lod);

                    MipCols = GFX_ULONG_CAST(
                    GFX_CEIL_DIV(
                    MipWidth,
                    pMapping->Scratch.Tile.Width));
                    MipRows =
                    GFX_CEIL_DIV(
                    MipHeight,
                    pMapping->Scratch.Tile.Height);

                    pMapping->Scratch.SlicePitch.Physical +=
                    MipCols * MipRows * TileSize;
                }
            }

            { // Mip0...
                if(pTexInfo->Type != RESOURCE_3D)
                {
                    pMapping->Scratch.Slices =
                    GFX_MAX(pTexInfo->ArraySize, 1) *
                    ((pTexInfo->Type == RESOURCE_CUBE) ? 6 : 1);
                }
                else
                {
                    pMapping->Scratch.Slices =
                    GFX_CEIL_DIV(pTexInfo->Depth, pMapping->Scratch.Tile.Depth);
                }

                if(pTexInfo->Pitch ==
                   (GFX_ALIGN(pTexInfo->BaseWidth, pMapping->Scratch.Tile.Width) /
                    pMapping->Scratch.Element.Width * BytesPerElement))
                {
                    // Treat Each LOD0 MIP as Single, Large Mapping Row...
                    pMapping->Scratch.Rows = 1;

                    pMapping->__NextSpan.Size =
                    GFX_CEIL_DIV(pTexInfo->BaseWidth, pMapping->Scratch.Tile.Width) *
                    GFX_CEIL_DIV(pTexInfo->BaseHeight, pMapping->Scratch.Tile.Height) *
                    TileSize;
                }
                else
                {
                    pMapping->Scratch.Rows =
                    GFX_CEIL_DIV(pTexInfo->BaseHeight, pMapping->Scratch.Tile.Height);

                    pMapping->__NextSpan.Size =
                    GFX_CEIL_DIV(pTexInfo->BaseWidth, pMapping->Scratch.Tile.Width) *
                    TileSize;
                }
            }
        }

        // This iteration's span descriptor...
        pMapping->Span = pMapping->__NextSpan;

        // Prepare for Next Iteration...
        //  for(Lod = 0; Lod <= EffectiveLodMax; Lod += 1)
        //  for(Row = 0; Row < Rows; Row += 1)
        //  for(Slice = 0; Slice < Slices; Slice += 1)
        if((pMapping->Scratch.Slice += 1) < pMapping->Scratch.Slices)
        {
            pMapping->__NextSpan.PhysicalOffset += pMapping->Scratch.SlicePitch.Physical;
            pMapping->__NextSpan.VirtualOffset += pMapping->Scratch.SlicePitch.Virtual;
        }
        else
        {
            pMapping->Scratch.Slice = 0;

            if((pMapping->Scratch.Row += 1) < pMapping->Scratch.Rows)
            {
                pMapping->__NextSpan.PhysicalOffset =
                pMapping->Scratch.Slice0MipOffset.Physical += pMapping->Span.Size;

                pMapping->__NextSpan.VirtualOffset =
                pMapping->Scratch.Slice0MipOffset.Virtual += pMapping->Scratch.RowPitchVirtual;
            }
            else if((pMapping->Scratch.Lod += 1) <= pMapping->Scratch.EffectiveLodMax)
            {
                GMM_REQ_OFFSET_INFO GetOffset = {0};
                GMM_GFX_SIZE_T      MipWidth;
                uint32_t            MipHeight, MipCols;

                MipWidth  = pTextureCalc->GmmTexGetMipWidth(pTexInfo, pMapping->Scratch.Lod);
                MipHeight = pTextureCalc->GmmTexGetMipHeight(pTexInfo, pMapping->Scratch.Lod);

                MipCols = GFX_ULONG_CAST(
                GFX_CEIL_DIV(
                MipWidth,
                pMapping->Scratch.Tile.Width));

                pMapping->Scratch.Row = 0;
                pMapping->Scratch.Rows =
                GFX_CEIL_DIV(
                MipHeight,
                pMapping->Scratch.Tile.Height);

                if(pTexInfo->Type != RESOURCE_3D)
                {
                    pMapping->__NextSpan.PhysicalOffset =
                    pMapping->Scratch.Slice0MipOffset.Physical += pMapping->Span.Size;
                }
                else
                {
                    uint32_t MipDepth;

                    MipDepth = pTextureCalc->GmmTexGetMipDepth(pTexInfo, pMapping->Scratch.Lod);

                    // 3D Std Swizzle traverses slices before MIP's...
                    pMapping->Scratch.Slice0MipOffset.Physical =
                    pMapping->__NextSpan.PhysicalOffset += pMapping->Span.Size;

                    pMapping->Scratch.Slices =
                    GFX_CEIL_DIV(
                    MipDepth,
                    pMapping->Scratch.Tile.Depth);

                    pMapping->Scratch.SlicePitch.Physical =
                    MipCols * pMapping->Scratch.Rows * TileSize;
                }

                GetOffset.ReqRender = 1;
                GetOffset.MipLevel  = pMapping->Scratch.Lod;
                this->GetOffset(GetOffset);

                pMapping->__NextSpan.VirtualOffset =
                pMapping->Scratch.Slice0MipOffset.Virtual =
                GFX_ALIGN_FLOOR(GetOffset.Render.Offset64, TileSize); // Truncate for packed MIP Tail.

                pMapping->__NextSpan.Size = MipCols * TileSize;
            }
            else
            {
                // If the resource was a planar surface then need to iterate over the remaining planes
                WasFinalSpan = pMapping->Scratch.Plane == pMapping->Scratch.LastPlane;
            }
        }
    }
    else
    {
        __GMM_ASSERT(0);
    }

    return !WasFinalSpan;
}

//=============================================================================
//
// Function: GetTiledResourceMipPacking
//
// Desc: Get number of packed mips and total #tiles for packed mips
//
// Parameters:
//      See function arguments.
//
// Returns:
//      void
//-----------------------------------------------------------------------------
void GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetTiledResourceMipPacking(uint32_t *pNumPackedMips,
                                                                           uint32_t *pNumTilesForPackedMips)
{
    if(GetMaxLod() == 0)
    {
        *pNumPackedMips         = 0;
        *pNumTilesForPackedMips = 0;
        return;
    }

    if(GetResFlags().Info.TiledYf ||
       GMM_IS_64KB_TILE(GetResFlags()))
    {
        if(Surf.Alignment.MipTailStartLod == GMM_TILED_RESOURCE_NO_MIP_TAIL)
        {
            *pNumPackedMips         = 0;
            *pNumTilesForPackedMips = 0;
        }
        else
        {
            *pNumPackedMips = GetMaxLod() -
                              Surf.Alignment.MipTailStartLod + 1;
            *pNumTilesForPackedMips = 1;
        }
    }
    else
    {
        // Error, unsupported format.
        __GMM_ASSERT(false);
    }
}

//=============================================================================
//
// Function: GetPackedMipTailStartLod
//
// Desc: Get Lod of first packed Mip.
//
// Parameters:
//      See function arguments.
//
// Returns:
//      Lod of first packed Mip
//-----------------------------------------------------------------------------
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetPackedMipTailStartLod()

{
    uint32_t NumPackedMips = 0, NumTilesForPackedMips = 0;

    const GMM_PLATFORM_INFO *pPlatform = GMM_OVERRIDE_PLATFORM_INFO(&Surf, GetGmmLibContext());

    GetTiledResourceMipPacking(&NumPackedMips,
                               &NumTilesForPackedMips);

    return (GetMaxLod() == 0) ?
           pPlatform->MaxLod :
           GetMaxLod() - NumPackedMips + 1; //GetMaxLod srarts at index 0, while NumPackedMips is just
                                            //the number of mips. So + 1 to bring them to same units.
}

/////////////////////////////////////////////////////////////////////////////////////
/// Verifies if all mips are RCC-aligned
/// @return    true/false
/////////////////////////////////////////////////////////////////////////////////////
bool GMM_STDCALL GmmLib::GmmResourceInfoCommon::IsMipRCCAligned(uint8_t &MisAlignedLod)
{
    const uint8_t RCCCachelineWidth  = 32;
    const uint8_t RCCCachelineHeight = 4;

    for(uint8_t lod = 0; lod <= GetMaxLod(); lod++)
    {
        if(!(GFX_IS_ALIGNED(GetMipWidth(lod), RCCCachelineWidth) &&
             GFX_IS_ALIGNED(GetMipHeight(lod), RCCCachelineHeight)))
        {
            MisAlignedLod = lod;
            return false;
        }
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Return the logical width of mip level
/// @param[in] MipLevel: Mip level for which the info is needed
/// @return    Mip width
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetMipWidth(uint32_t MipLevel)
{
    GMM_TEXTURE_CALC *pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());
    return pTextureCalc->GmmTexGetMipWidth(&Surf, MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Return the logical height of mip level
/// @param[in] MipLevel: Mip level for which the info is needed
/// @return    Mip width
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetMipHeight(uint32_t MipLevel)
{
    GMM_TEXTURE_CALC *pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());
    return pTextureCalc->GmmTexGetMipHeight(&Surf, MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Return the logical depth of mip level
/// @param[in] MipLevel Mip level for which the info is needed
/// @return    Mip width
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmResourceInfoCommon::GetMipDepth(uint32_t MipLevel)
{
    GMM_TEXTURE_CALC *pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(&Surf, GetGmmLibContext());
    return pTextureCalc->GmmTexGetMipDepth(&Surf, MipLevel);
}
