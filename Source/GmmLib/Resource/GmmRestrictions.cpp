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
/// Checks that clients only set Presentable flag during a resource allocation, ONLY 
/// when a platform supported render target is selected in ::GMM_RESOURCE_FORMAT enum.
///
/// @return TRUE if displayable, FALSE otherwise.
/////////////////////////////////////////////////////////////////////////////////////
BOOLEAN GmmLib::GmmResourceInfoCommon::IsPresentableformat()
{
    const GMM_PLATFORM_INFO* pPlatform;
    const GMM_FORMAT_ENTRY*  FormatTable = NULL;

    GMM_DPF_ENTER;
    __GMM_ASSERTPTR(pGmmGlobalContext, FALSE);
    
    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(&Surf);
    FormatTable = &(pPlatform->FormatTable[0]);

    if (Surf.Flags.Gpu.Presentable == FALSE)
    {
        // When Presentable flag is not set, no reason to check for valid RT
        // platform supported format. Safe to return TRUE.
        return TRUE;
    }

    if ((Surf.Format > GMM_FORMAT_INVALID) && 
            (Surf.Format < GMM_RESOURCE_FORMATS))
    {
        if ((FormatTable[Surf.Format].RenderTarget) &&
            (FormatTable[Surf.Format].Supported))
        {
            return TRUE;
        }
        else
        {
            GMM_ASSERTDPF(0,"Present flag can only be set w/ a format!");         
            return FALSE;
        }
    }

    return FALSE;    
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the restrictions that a particular resource must follow on a particular
/// OS or hardware.
///
/// @param[out]  Restrictions: restrictions that this resource must adhere to
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmResourceInfoCommon::GetRestrictions(__GMM_BUFFER_TYPE& Restrictions)
{
    const GMM_PLATFORM_INFO   *pPlatform = NULL;
    GMM_RESOURCE_FLAG          ZeroGpuFlags;

    __GMM_ASSERTPTR(pGmmGlobalContext, VOIDRETURN);

    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(&Surf);

    // Check that at least one usage flag is set for allocations other than
    // Primary/Shadow/Staging.
    memset(&ZeroGpuFlags.Gpu, 0, sizeof(ZeroGpuFlags.Gpu));
    if ((Surf.Type <= RESOURCE_KMD_CHECK_START || 
         Surf.Type >= RESOURCE_KMD_CHECK_END) && 
        !memcmp(&Surf.Flags.Gpu, &ZeroGpuFlags.Gpu, sizeof(ZeroGpuFlags.Gpu)))
    {
        GMM_ASSERTDPF(0,"No GPU Usage specified!"); 
        return;
    }

    __GmmPlatformResetRestrictions(&Restrictions);   //Set to Default

    // Get worst case restrictions that match GPU flags set in resource
    switch(Surf.Type)
    {
        case RESOURCE_1D:
        case RESOURCE_2D:
        case RESOURCE_3D:
        case RESOURCE_CUBE:
        case RESOURCE_BUFFER:
        case RESOURCE_SCRATCH:
        case RESOURCE_GDI:
    #if _WIN32
        case RESOURCE_WGBOX_ENCODE_STATE:
        case RESOURCE_WGBOX_ENCODE_DISPLAY:
        case RESOURCE_WGBOX_ENCODE_REFERENCE:
        case RESOURCE_WGBOX_ENCODE_TFD:
    #endif
            GetGenericRestrictions(&Restrictions);
            break;

        case RESOURCE_HW_CONTEXT:       
        case RESOURCE_TAG_PAGE:
            if (Surf.Flags.Info.TiledW || 
                Surf.Flags.Info.TiledX || 
                Surf.Flags.Info.TiledY)
            {
                GMM_ASSERTDPF(0,"Tiled Pref specified for RESOURCE_LINEAR!"); 
                return;
            }
            GetLinearRestrictions(&Restrictions);
            break;

        case RESOURCE_PRIMARY:
        case RESOURCE_SHADOW:
        case RESOURCE_STAGING:
            GetPrimaryRestrictions(&Restrictions);
            break;

        case RESOURCE_NNDI:
            Restrictions = pPlatform->Nndi;
            break;           

        case RESOURCE_HARDWARE_MBM:
        case RESOURCE_IFFS_MAPTOGTT:
            //Hardware MBM resource request can come for overlay allocation or normal
            //displayable allocation. So get the restrictions accordingly
            if (Surf.Flags.Gpu.Overlay)
            {
                Restrictions = pPlatform->Overlay;
            }
            else
            {
                Restrictions = pPlatform->HardwareMBM;
            }
            break; 

        case RESOURCE_CURSOR:
        case RESOURCE_PWR_CONTEXT:
        case RESOURCE_KMD_BUFFER:
        case RESOURCE_NULL_CONTEXT_INDIRECT_STATE:
        case RESOURCE_PERF_DATA_QUEUE:
        case RESOURCE_GLOBAL_BUFFER:
        case RESOURCE_FBC:
        case RESOURCE_GFX_CLIENT_BUFFER:
            Restrictions = pPlatform->Cursor;
            break;

        case RESOURCE_OVERLAY_DMA:
            Restrictions = pPlatform->NoRestriction;
            break;

        case RESOURCE_GTT_TRANSFER_REGION:
            GetGenericRestrictions(&Restrictions);
            break;

        case RESOURCE_OVERLAY_INTERMEDIATE_SURFACE:
            Restrictions = pPlatform->Overlay;
            break;

        default:
            GetGenericRestrictions(&Restrictions);
            GMM_ASSERTDPF(0,"Unkown Resource type");     
    }

    // Apply any specific WA

    if (((Surf.Flags.Wa.ILKNeedAvcMprRowStore32KAlign)) ||
        ((Surf.Flags.Wa.ILKNeedAvcDmvBuffer32KAlign)))   
    {
        Restrictions.Alignment = GFX_ALIGN(Restrictions.Alignment, GMM_KBYTE(32));
    }

    if (pGmmGlobalContext->GetWaTable().WaAlignContextImage && (Surf.Type == RESOURCE_HW_CONTEXT))
    {
        Restrictions.Alignment = GFX_ALIGN(Restrictions.Alignment, GMM_KBYTE(64));
    }

    if (Surf.Flags.Gpu.S3d &&
        Surf.Flags.Info.Linear &&
        !pGmmGlobalContext->GetSkuTable().FtrDisplayEngineS3d)
    {
        Restrictions.Alignment      = PAGE_SIZE;
        Restrictions.PitchAlignment = PAGE_SIZE;
    }
    
    if (Surf.Flags.Gpu.TiledResource)
    {
        // Need at least 64KB alignment to track tile mappings (h/w or s/w tracking).        
        Restrictions.Alignment = GFX_ALIGN(Restrictions.Alignment, GMM_KBYTE(64));
        
        // Buffer tiled resources are trivially divided into 64KB tiles => Pitch must divide into 64KB tiles
        if (Surf.Type == RESOURCE_BUFFER)
        {
            Restrictions.PitchAlignment = GFX_ALIGN(Restrictions.PitchAlignment, GMM_KBYTE(64));
        }

        if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE)
        {
            pGmmGlobalContext->GetPlatformInfo().SurfaceMaxSize = GMM_TBYTE(1);
        }
    }

    // Each plane of Y0-Y1-UV0-UV1 layout must be 4KB aligned to meet media 
    // version of SURFACE_STATE req. No X,Y offsets. Align to 2KB since height 
    // is alwasy even, b/c valign >= 2. 
    if (Surf.Flags.Info.YUVShaderFriendlyLayout)
    {
        Restrictions.PitchAlignment = GFX_ALIGN(Restrictions.PitchAlignment, GMM_KBYTE(2));
    }

    // SKL TileY Display needs 1MB alignment.
    if(((Surf.Type == RESOURCE_PRIMARY) ||
         Surf.Flags.Gpu.FlipChain) &&
        (Surf.Flags.Info.TiledY ||
         Surf.Flags.Info.TiledYf))
    {
        Restrictions.Alignment = GMM_MBYTE(1);
    }  

    if (Surf.Flags.Info.RenderCompressed || 
        Surf.Flags.Info.MediaCompressed)
    {
        Restrictions.Alignment = GFX_ALIGN(Restrictions.Alignment, GMM_KBYTE(16));
    }
}

  
//=============================================================================
// 
// Function: GmmResGetRestrictions
// 
// Desc: This routine returns resource restrictions
//
// Parameters:
//      pPlatform: ptr to HW_DEVICE_EXTENSION
//      pResourceInfo: ptr to GMM_RESOURCE_INFO
//      pRestrictions: ptr to restrictions
//
// Returns:
//      VOID
//         
//-----------------------------------------------------------------------------
void GMM_STDCALL GmmResGetRestrictions(GMM_RESOURCE_INFO*    pResourceInfo,
                                       __GMM_BUFFER_TYPE*    pRestrictions) 
{
    pResourceInfo->GetRestrictions(*pRestrictions);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the best restrictions by comparing two buffer types. Each buffer type 
/// carries alignment and size restrictions.
///
/// @param[in]  pFirstBuffer: Contains surface alignment and size restrictions
/// @param[in]  pSecondBuffer: Contains surface alignment and size restrictions
///
/// @return     Best Restrictions based on the two parameters passed
/////////////////////////////////////////////////////////////////////////////////////
__GMM_BUFFER_TYPE *GmmLib::GmmResourceInfoCommon::GetBestRestrictions(__GMM_BUFFER_TYPE *pFirstBuffer,
                                                                      const __GMM_BUFFER_TYPE *pSecondBuffer)
{
    GMM_DPF_ENTER;

    if(IsRestrictionInvalid(pFirstBuffer))   //default
    {
        *pFirstBuffer = *pSecondBuffer;
        return pFirstBuffer;
    }
   
    pFirstBuffer->Alignment            = GFX_MAX(pFirstBuffer->Alignment,
                                                 pSecondBuffer->Alignment);

    pFirstBuffer->PitchAlignment       = GFX_MAX(pFirstBuffer->PitchAlignment,
                                                 pSecondBuffer->PitchAlignment);

    pFirstBuffer->RenderPitchAlignment = GFX_MAX(pFirstBuffer->RenderPitchAlignment,
                                                 pSecondBuffer->RenderPitchAlignment);

    pFirstBuffer->LockPitchAlignment   = GFX_MAX(pFirstBuffer->LockPitchAlignment,
                                                 pSecondBuffer->LockPitchAlignment);

    pFirstBuffer->MinPitch             = GFX_MAX(pFirstBuffer->MinPitch,
                                                 pSecondBuffer->MinPitch);

    pFirstBuffer->MinAllocationSize    = GFX_MAX(pFirstBuffer->MinAllocationSize,
                                                 pSecondBuffer->MinAllocationSize);

    pFirstBuffer->MinDepth             = GFX_MAX(pFirstBuffer->MinDepth,
                                                 pSecondBuffer->MinDepth);

    pFirstBuffer->MinHeight            = GFX_MAX(pFirstBuffer->MinHeight,
                                                 pSecondBuffer->MinHeight);

    pFirstBuffer->MinWidth             = GFX_MAX(pFirstBuffer->MinWidth,
                                                 pSecondBuffer->MinWidth);

    pFirstBuffer->MaxDepth             = GFX_MIN(pFirstBuffer->MaxDepth,
                                                 pSecondBuffer->MaxDepth);

    pFirstBuffer->MaxHeight            = GFX_MIN(pFirstBuffer->MaxHeight,
                                                 pSecondBuffer->MaxHeight);

    pFirstBuffer->MaxWidth             = GFX_MIN(pFirstBuffer->MaxWidth,
                                                 pSecondBuffer->MaxWidth);

    pFirstBuffer->NeedPow2LockAlignment   = pFirstBuffer->NeedPow2LockAlignment | 
                                            pSecondBuffer->NeedPow2LockAlignment;   

    GMM_DPF_EXIT;
    return pFirstBuffer;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns restrictions for 1D, 2D, 3D textures depending on how the surface 
/// may possibliy be used.
///
/// @param[out]  pBuff: Restrictions filled in this struct
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmResourceInfoCommon::GetGenericRestrictions(__GMM_BUFFER_TYPE *pBuff)
{
    GMM_DPF_ENTER;
    const GMM_PLATFORM_INFO *pPlatformResource = GMM_OVERRIDE_PLATFORM_INFO(&Surf);

    if(Surf.Flags.Gpu.NoRestriction)
    {
        // Impose zero restrictions. Ignore any other GPU usage flags
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->NoRestriction);
        return;
    }

    if(Surf.Flags.Gpu.Texture)
    {   
        if (Surf.Type == RESOURCE_BUFFER)
        {
            *pBuff = pPlatformResource->BufferType;
        }
        else if (Surf.Type == RESOURCE_CUBE)
        {
            *pBuff = pPlatformResource->CubeSurface;
        }    
        else if (Surf.Type == RESOURCE_3D)
        {
            *pBuff = pPlatformResource->Texture3DSurface;
        }
        else
        {
            *pBuff = pPlatformResource->Texture2DSurface;
            if (Surf.Flags.Info.Linear)
            {
                *pBuff = pPlatformResource->Texture2DLinearSurface;
            }
        }
    }
    if( Surf.Flags.Gpu.RenderTarget || 
        Surf.Flags.Gpu.CCS || 
        Surf.Flags.Gpu.MCS) 
    {
        // Gen7 onwards, bound by SURFACE_STATE constraints.
        if (Surf.Type == RESOURCE_BUFFER)
        {
            *pBuff = pPlatformResource->BufferType;
        }
        else if (Surf.Type == RESOURCE_CUBE)
        {
            *pBuff = pPlatformResource->CubeSurface;
        }    
        else if (Surf.Type == RESOURCE_3D)  
        {
            *pBuff = pPlatformResource->Texture3DSurface;
        }
        else
        {
            *pBuff = pPlatformResource->Texture2DSurface;
            if (Surf.Flags.Info.Linear)
            {
                *pBuff = pPlatformResource->Texture2DLinearSurface;
            }
        }
    }
    if(Surf.Flags.Gpu.Depth)
    {
        // Z
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Depth);
    }
    if(Surf.Flags.Gpu.Vertex)
    {
        // VertexData
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Vertex);
    }
    if(Surf.Flags.Gpu.Index)
    {
        // Index buffer
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Index);
    }
    if(Surf.Flags.Gpu.FlipChain)
    {
        // Async Flip
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->ASyncFlipSurface);
    }
    if(Surf.Flags.Gpu.MotionComp)
    {
        // Media buffer
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->MotionComp);
    }
    if( Surf.Flags.Gpu.State || 
        Surf.Flags.Gpu.InstructionFlat || 
        Surf.Flags.Gpu.ScratchFlat) 
    {
        // indirect state
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Vertex);
    }
    if(Surf.Flags.Gpu.Query ||
       Surf.Flags.Gpu.HistoryBuffer)
    {
        // Query
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->NoRestriction);
    }
    if(Surf.Flags.Gpu.Constant)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Constant);
    }
    if(Surf.Flags.Gpu.Stream)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Stream);
    }
    if(Surf.Flags.Gpu.InterlacedScan)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->InterlacedScan);
    }
    if(Surf.Flags.Gpu.TextApi)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->TextApi);
    }
    if(Surf.Flags.Gpu.SeparateStencil)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Stencil);
    }
    if(Surf.Flags.Gpu.HiZ)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->HiZ);
    }
    if(Surf.Flags.Gpu.Video)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Video);
    }
    if(Surf.Flags.Gpu.StateDx9ConstantBuffer)
    {
        // 
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->StateDx9ConstantBuffer);
    }
    if(Surf.Flags.Gpu.Overlay)
    {
        // Overlay buffer use Async Flip values
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->Overlay);

        if ((Surf.Format == GMM_FORMAT_YUY2)  && (Surf.BaseWidth == 640) )
        {
            // override the pitch alignment
            pBuff->PitchAlignment = 64;
        }
    }
    if(Surf.Flags.Info.XAdapter)
    {
        //Add Cross Adapter resource restriction for hybrid graphics.
        pBuff = GetBestRestrictions(pBuff, &pPlatformResource->XAdapter);
        if (Surf.Type == RESOURCE_BUFFER)
        {
            pBuff->MaxWidth = pPlatformResource->SurfaceMaxSize;
            pBuff->MaxPitch = pPlatformResource->BufferType.MaxPitch;
            pBuff->MaxHeight = 1;
        }
    }

     //Non Aligned ExistingSysMem  Special cases.
    if((Surf.Flags.Info.ExistingSysMem && 
       (!Surf.ExistingSysMem.IsGmmAllocated) &&
       (!Surf.ExistingSysMem.IsPageAligned)))
    {

        if(Surf.Flags.Info.Linear ||
            Surf.Flags.Info.SVM)
        {
            if(Surf.Type == RESOURCE_BUFFER)
            {
                //Use combination of BufferType, NoRestriction to support large buffer with minimal pitch alignment
                *pBuff = pPlatformResource->BufferType;
                pBuff->PitchAlignment       = pPlatformResource->NoRestriction.PitchAlignment;
                pBuff->LockPitchAlignment   = pPlatformResource->NoRestriction.LockPitchAlignment; 
                pBuff->RenderPitchAlignment = pPlatformResource->NoRestriction.LockPitchAlignment;
                pBuff->MinPitch             = pPlatformResource->NoRestriction.MinPitch;
            }
           
            //[To DO] Handle other types when needed!
            
        } 
        /*
        else if(Surf.Flags.Gpu.Texture)
        {
            //Override as and when required
        }
        else if(Surf.Flags.Gpu.RenderTarget)
        {
            //Overide as and when Required
        }*/
    }

    GMM_DPF_EXIT;
}

//=============================================================================
// 
// Function: __GmmPlatformResetRestrictions
// 
// Desc: This routine initializes a __GMM_BUFFER_TYPE. Once this function is
//       called, the caller has a invalid restrictions.
//
// Parameters:
//      pRestriction ==> Restrictions
// Returns:
//      VOID
//         
//-----------------------------------------------------------------------------
void __GmmPlatformResetRestrictions(__GMM_BUFFER_TYPE *pRestriction)
{
    pRestriction->MinDepth = 0xffffffff;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Internal function returns the best restrictions depending on how the surface may 
/// possibly be used.
///
/// @param[in]  pTexInfo: ptr to ::GMM_TEXTURE_INFO,
/// @param[in]  pRestrictions: Reference to surface alignment and size restrictions
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmTextureCalc::GetTexRestrictions(GMM_TEXTURE_INFO*      pTexInfo,
                                                __GMM_BUFFER_TYPE*     pRestrictions)
{
    GMM_RESOURCE_INFO ResourceInfo;
    GMM_DPF_ENTER;

    GMM_RESOURCE_FLAG &Flags = ResourceInfo.GetResFlags();
    Flags = pTexInfo->Flags;
    ResourceInfo.OverrideSurfaceFormat(pTexInfo->Format);
    ResourceInfo.OverrideSurfaceType(pTexInfo->Type);
    ResourceInfo.OverrideGmmLibContext(pGmmGlobalContext);
    
    #if(_DEBUG || _RELEASE_INTERNAL)
        ResourceInfo.OverridePlatform(pTexInfo->Platform);
    #endif
   
    GmmResGetRestrictions(&ResourceInfo, pRestrictions);

    GMM_DPF_EXIT;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Calculates surface size based on Non Aligned ExistingSysMem restrictions.
///
/// @return     ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmResourceInfoCommon::ApplyExistingSysMemRestrictions()
{
    const GMM_PLATFORM_INFO   *pPlatform;
    
    // Handle Minimal Restriction ExistingSysMem Requirements...
    GMM_GFX_SIZE_T AdditionalPaddingBytes =  0;
    GMM_GFX_SIZE_T AdditionalPaddingRows =   0;
    GMM_GFX_SIZE_T BaseAlignment =           1; // 1 = Byte Alignment
    GMM_GFX_SIZE_T EndAlignment =            1; // 1 = Byte Alignment
    GMM_GFX_SIZE_T SizePadding =             1; // 1 = Byte Padding
    uint32_t CompressHeight, CompressWidth, CompressDepth;
    GMM_GFX_SIZE_T Width, Height;
    GMM_TEXTURE_INFO *pTexInfo = &Surf;
    GMM_TEXTURE_CALC *pTextureCalc;

    GMM_DPF_ENTER;

    pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo); 
    pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(pTexInfo);

    Height = pTexInfo->BaseHeight;
    Width = pTexInfo->BaseWidth;

    #define UPDATE_BASE_ALIGNMENT(a)                                        \
    {                                                                       \
        __GMM_ASSERT((GFX_MAX(BaseAlignment, a) % GFX_MIN(BaseAlignment, a)) == 0); /* Revisit if ever have to support complex alignments. */ \
        BaseAlignment = GFX_MAX(BaseAlignment, a);                          \
    }

    #define UPDATE_PADDING(p)                   \
    {                                           \
        SizePadding = GFX_MAX(SizePadding, p);  \
    }

    #define UPDATE_ADDITIONAL_ROWS(r)                               \
    {                                                               \
        AdditionalPaddingRows = GFX_MAX(AdditionalPaddingRows, r);  \
    }

    #define UPDATE_ADDITIONAL_BYTES(b)                                  \
    {                                                                   \
        AdditionalPaddingBytes = GFX_MAX(AdditionalPaddingBytes, b);    \
    }

    #define UPDATE_END_ALIGNMENT(a)                                         \
    {                                                                       \
        __GMM_ASSERT((GFX_MAX(EndAlignment, a) % GFX_MIN(EndAlignment, a)) == 0); /* Revisit if ever have to support complex alignments. */ \
        EndAlignment = GFX_MAX(EndAlignment, a);                            \
    }


    if(!pTexInfo->Pitch) 
    {
        __GMM_ASSERT(pTexInfo->Type == RESOURCE_1D); // Clients can leave pitch zero for 1D, and we'll fill-in...
        pTexInfo->Pitch = Width * (pTexInfo->BitsPerPixel >> 3);
    }

    __GMM_ASSERT( // Currently limiting our support...
        pTexInfo->Flags.Gpu.NoRestriction || 
        pTexInfo->Flags.Gpu.Index || 
        pTexInfo->Flags.Gpu.RenderTarget || 
        pTexInfo->Flags.Gpu.Texture || 
        pTexInfo->Flags.Gpu.Vertex);

    __GMM_ASSERT( // Trivial, Linear Surface...
        ((pTexInfo->Type == RESOURCE_BUFFER) || (pTexInfo->Type == RESOURCE_1D) || (pTexInfo->Type == RESOURCE_2D) ) && 
        (pTexInfo->MaxLod == 0) && 
        !GMM_IS_TILED(pPlatform->TileInfo[pTexInfo->TileMode]) && 
        !GmmIsPlanar(pTexInfo->Format) && 
        ((pTexInfo->ArraySize <= 1) || (pTexInfo->Type == RESOURCE_BUFFER)));

    __GMM_ASSERT( // Valid Surface...
        (Width > 0) && 
        !((pTexInfo->Type == RESOURCE_BUFFER) && GmmIsYUVPacked(pTexInfo->Format)));

    // Convert to compression blocks, if applicable...
    if(GmmIsCompressed(pTexInfo->Format))
    {
        pTextureCalc->GetCompressionBlockDimensions(pTexInfo->Format, &CompressWidth, &CompressHeight, &CompressDepth);

        Width = GFX_CEIL_DIV(Width, CompressWidth);   
        Height = GFX_CEIL_DIV(Height, CompressHeight);
    }

    __GMM_ASSERT( // Valid Surface Follow-Up...
        (pTexInfo->Pitch >=  (Width * (pTexInfo->BitsPerPixel >> 3))));

    if(!pTexInfo->Flags.Gpu.NoRestriction && !pTexInfo->Flags.Info.SVM && !pTexInfo->Flags.Info.Linear) 
    {
        if(pTexInfo->Flags.Gpu.Index) /////////////////////////////////////////////////////////
        {
            __GMM_ASSERT(!(
                pTexInfo->Flags.Gpu.RenderTarget || 
                pTexInfo->Flags.Gpu.Texture || 
                pTexInfo->Flags.Gpu.Vertex)); // Can explore if needed what combo's make sense--and how req's should combine.

            // BSpec.3DSTATE_INDEX_BUFFER...
            UPDATE_BASE_ALIGNMENT(4); // 32-bit worst-case, since GMM doesn't receive element-size from clients.
            if (pGmmGlobalContext->GetWaTable().WaAlignIndexBuffer)
            {
                UPDATE_END_ALIGNMENT(64);
            }
            else
            {
                UPDATE_END_ALIGNMENT(1);
            }
        }

        if(pTexInfo->Flags.Gpu.Vertex) ////////////////////////////////////////////////////////
        {
            __GMM_ASSERT(!(
                pTexInfo->Flags.Gpu.Index || 
                pTexInfo->Flags.Gpu.RenderTarget || 
                pTexInfo->Flags.Gpu.Texture
                )); // Can explore if needed what combo's make sense--and how req's should combine.

            // BSpec.VERTEX_BUFFER_STATE...
            UPDATE_BASE_ALIGNMENT(1); // VB's have member alignment requirements--but it's up to UMD to enforce.
            UPDATE_PADDING(1);
        }
        
        if(pTexInfo->Flags.Gpu.RenderTarget) //////////////////////////////////////////////////
        {
            uint32_t ElementSize;

            // BSpec.SURFACE_STATE...
            ElementSize = (pTexInfo->BitsPerPixel >> 3) * (GmmIsYUVPacked(pTexInfo->Format) ? 2 : 1);
            __GMM_ASSERT((pTexInfo->Pitch % ElementSize) == 0);
            UPDATE_BASE_ALIGNMENT(ElementSize);
            UPDATE_PADDING(pTexInfo->Pitch * 2); // BSpec."Surface Padding Requirements --> Render Target and Media Surfaces"
        }

        if(pTexInfo->Flags.Gpu.Texture) // (i.e. Sampler Surfaces) ///////////////////////////
        {
            UPDATE_BASE_ALIGNMENT(1); // Sampler supports byte alignment (with performance hit if misaligned).

            if (pGmmGlobalContext->GetWaTable().WaNoMinimizedTrivialSurfacePadding)
            {
                if(pTexInfo->Type == RESOURCE_BUFFER) 
                {
                    if (pGmmGlobalContext->GetWaTable().WaNoBufferSamplerPadding)
                    {
                        // Client agreeing to take responsibility for flushing L3 after sampling/etc.
                    } 
                    else 
                    {
                        // GMM currently receives GENERIC_8BIT for 
                        // RESOURCE_BUFFER creations, so we have to assume the 
                        // worst-case sample size of 128-bit (unless we alter 
                        // our interface meaning):
                        uint32_t ElementSize = 16;

                         // BSpec."Surface Padding Requirements --> Sampling Engine Surfaces"
                        UPDATE_PADDING(ElementSize * ((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) == IGFX_GEN8_CORE) ? 512 : 256));
                        UPDATE_ADDITIONAL_BYTES(16);
                    }
                } 
                else // RESOURCE_1D/2D...
                {
                    /* BSpec mentions sampler needs Alignment Unit padding--
                        but sampler arch confirms that's overly conservative 
                        padding--and for trivial (linear, single-subresource) 
                        2D's, even-row (quad-row on BDW.A0) plus additional 
                        64B padding is sufficient. (E.g. pitch overfetch will 
                        be caught by subsequent rows or the additional 64B. */

                    __GMM_ASSERT((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) <= IGFX_GEN8_CORE));

                    if(GmmIsCompressed(pTexInfo->Format)) 
                    {
                        // BSpec: "For compressed textures...padding at the bottom of the surface is to an even compressed row."
                        UPDATE_PADDING(pTexInfo->Pitch * 2); // (Sampler arch confirmed that even-row is sufficient on BDW despite BDW's 4x4 sampling, since this req is from L2 instead of L1.)
                    } 
                    else 
                    {
                        UPDATE_PADDING(pTexInfo->Pitch * ((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) == IGFX_GEN8_CORE) ? 4 : 2)); // Sampler Fetch Rows: BDW ? 4 : 2
                    }

                    // BSpec "For packed YUV, 96 bpt, 48 bpt, and 24 bpt surface formats, additional padding is required."
                    if( GmmIsYUVPacked(pTexInfo->Format) || (pTexInfo->BitsPerPixel == 96) || (pTexInfo->BitsPerPixel == 48) || (pTexInfo->BitsPerPixel == 24)) 
                    {
                        UPDATE_ADDITIONAL_BYTES(16);
                        UPDATE_ADDITIONAL_ROWS(1);
                    }

                    /* BSpec: "For linear surfaces, additional padding of 64 
                        bytes is required at the bottom of the surface." 
                        (Sampler arch confirmed the 64 bytes can overlap with 
                        the other "additional 16 bytes" mentions in that section.) */
                    UPDATE_ADDITIONAL_BYTES(64);
                }
            } 
            else 
            {
                /* BSpec: [DevBDW:B0+]: For SURFTYPE_BUFFER, SURFTYPE_1D, and 
                    SURFTYPE_2D non-array, non-MSAA, non-mip-mapped surfaces in 
                    linear memory, the only padding requirement is to the next 
                    aligned 64-byte boundary beyond the end of the surface. */
                UPDATE_END_ALIGNMENT(64);
            }
        }
    } 
    else // Gpu.NoRestriction...
    {
        // Clients specify NoRestriction at their own risk--e.g. it can be 
        // appropriate when using IA-Coherent L3 combined with L3 being in 
        // unified/"Rest" mode (where there won't be write-->read-only 
        // collisions on unintentionally shared cachelines).
    }

    {    //Finally calculate surf size
        GMM_GFX_SIZE_T OriginalEnd, RequiredSize;

        ExistingSysMem.pVirtAddress    = 
                    (ExistingSysMem.pExistingSysMem  & (PAGE_SIZE - 1))               ?
                        ((GMM_VOIDPTR64) GFX_ALIGN(ExistingSysMem.pExistingSysMem , 
                            BaseAlignment))                                                 :
                        ExistingSysMem.pExistingSysMem ;

        ExistingSysMem.pGfxAlignedVirtAddress = 
            (GMM_VOIDPTR64) GFX_ALIGN(
                (UINT64) ExistingSysMem.pVirtAddress, PAGE_SIZE);

        __GMM_ASSERT((ExistingSysMem.pVirtAddress % BaseAlignment) == 0);

        RequiredSize = pTexInfo->Pitch * Height;

        RequiredSize = 
            GFX_ALIGN(RequiredSize, SizePadding) + 
            (AdditionalPaddingRows * pTexInfo->Pitch) + 
            AdditionalPaddingBytes;

        OriginalEnd = ExistingSysMem.pVirtAddress + RequiredSize;
        RequiredSize += GFX_ALIGN(OriginalEnd, EndAlignment) - OriginalEnd;

        //Ensure sufficient ExistingSysMem available.
        if(ExistingSysMem.Size < RequiredSize)
        {
            return GMM_ERROR; 
        }

        Surf.Size = RequiredSize;
    }

    GMM_DPF_EXIT;

    return GMM_SUCCESS;

}


