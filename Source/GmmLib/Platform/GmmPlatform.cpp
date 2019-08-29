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

extern GMM_GLOBAL_CONTEXT *pGmmGlobalContext;

int32_t GmmLib::PlatformInfo::RefCount = 0;

GmmLib::PlatformInfo::PlatformInfo(PLATFORM &Platform)
{
    GMM_DPF_ENTER;

    memset(&Data, 0, sizeof(Data));
    Data.Platform = Platform;

    GMM_RESOURCE_FORMAT GmmFormat;
#define GMM_FORMAT_GEN(X) (GFX_GET_CURRENT_RENDERCORE(Data.Platform) >= IGFX_GEN##X##_CORE)
#define GMM_FORMAT_SKU(FtrXxx) (pGmmGlobalContext->GetSkuTable().FtrXxx != 0)
#define GMM_FORMAT_WA(WaXxx) (pGmmGlobalContext->GetWaTable().WaXxx != 0)
#define GMM_COMPR_FORMAT_INVALID GMM_E2ECOMP_FORMAT_INVALID
#define GMM_FORMAT(Name, bpe, _Width, _Height, _Depth, IsRT, IsASTC, RcsSurfaceFormat, SSCompressionFmt, Availability)        \
                                                                                                                         \
    {                                                                                                                    \
        GmmFormat                                                       = GMM_FORMAT_##Name;                             \
        Data.FormatTable[GmmFormat].ASTC                                = (IsASTC);                                      \
        Data.FormatTable[GmmFormat].Element.BitsPer                     = (bpe);                                         \
        Data.FormatTable[GmmFormat].Element.Depth                       = (_Depth);                                      \
        Data.FormatTable[GmmFormat].Element.Height                      = (_Height);                                     \
        Data.FormatTable[GmmFormat].Element.Width                       = (_Width);                                      \
        Data.FormatTable[GmmFormat].RenderTarget                        = ((IsRT) != 0);                                 \
        Data.FormatTable[GmmFormat].SurfaceStateFormat                  = ((GMM_SURFACESTATE_FORMAT)(RcsSurfaceFormat)); \
        Data.FormatTable[GmmFormat].CompressionFormat.CompressionFormat = (SSCompressionFmt);                            \
        Data.FormatTable[GmmFormat].Supported                           = ((Availability) != 0);                         \
        if(((_Depth) > 1) || ((_Height) > 1) || ((_Width) > 1))                                                          \
        {                                                                                                                \
            Data.FormatTable[GmmFormat].Compressed = 1;                                                                  \
        }                                                                                                                \
    }

#include "External/Common/GmmFormatTable.h"
}

/////////////////////////////////////////////////////////////////////////////////////
/// Copies parameters or sets flags based on info sent by the client.
///
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::PlatformInfo::SetCCSFlag(GMM_RESOURCE_FLAG &Flags)
{
    if(Flags.Gpu.MCS)
    {
        Flags.Gpu.CCS = Flags.Gpu.MCS;
    }
    Flags.Info.RenderCompressed = Flags.Info.MediaCompressed = 0;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the MMC parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfo::ValidateMMC(GMM_TEXTURE_INFO &Surf)
{
    if(Surf.Flags.Gpu.MMC && //For Media Memory Compression --
       ((!(GMM_IS_4KB_TILE(Surf.Flags) || GMM_IS_64KB_TILE(Surf.Flags))) ||
        Surf.ArraySize > GMM_MAX_MMC_INDEX))
    {
        return 0;
    }
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfo::ValidateCCS(GMM_TEXTURE_INFO &Surf)
{
    if(!(                                                             //--- Legitimate CCS Case ----------------------------------------
       ((Surf.Type >= RESOURCE_2D && Surf.Type <= RESOURCE_BUFFER) && //Not supported: 1D; Supported: Buffer, 2D, 3D, cube, Arrays, mip-maps, MSAA, Depth/Stencil
        (Surf.Type <= RESOURCE_CUBE)) ||
       (Surf.Type == RESOURCE_2D && Surf.MaxLod == 0)))
    {
        GMM_ASSERTDPF(0, "Invalid CCS usage!");
        return 0;
    }

    if(Surf.Flags.Info.RenderCompressed && Surf.Flags.Info.MediaCompressed)
    {
        GMM_ASSERTDPF(0, "Invalid CCS usage - can't be both RC and MC!");
        return 0;
    }

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the UnifiedAuxSurface parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfo::ValidateUnifiedAuxSurface(GMM_TEXTURE_INFO &Surf)
{
    if((Surf.Flags.Gpu.UnifiedAuxSurface) &&
       !( //--- Legitimate UnifiedAuxSurface Case ------------------------------------------
       Surf.Flags.Gpu.CCS &&
       (Surf.MSAA.NumSamples <= 1 && (Surf.Flags.Gpu.RenderTarget || Surf.Flags.Gpu.Texture))))
    {
        GMM_ASSERTDPF(0, "Invalid UnifiedAuxSurface usage!");
        return 0;
    }
    return 1;
}

//=============================================================================
//
// Function: CheckFmtDisplayDecompressible
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
uint8_t GmmLib::PlatformInfo::CheckFmtDisplayDecompressible(GMM_TEXTURE_INFO &Surf,
                                                            bool              IsSupportedRGB64_16_16_16_16,
                                                            bool              IsSupportedRGB32_8_8_8_8,
                                                            bool              IsSupportedRGB32_2_10_10_10,
                                                            bool              IsSupportedMediaFormats)
{
    bool IsRenderCompressed = false;
    GMM_UNREFERENCED_PARAMETER(IsSupportedMediaFormats);
    GMM_UNREFERENCED_PARAMETER(IsSupportedRGB64_16_16_16_16);
    GMM_UNREFERENCED_PARAMETER(Surf);

    if(IsSupportedRGB32_8_8_8_8 || //RGB32 8 : 8 : 8 : 8
       (GFX_GET_CURRENT_DISPLAYCORE(pGmmGlobalContext->GetPlatformInfo().Platform) >= IGFX_GEN10_CORE &&
        IsSupportedRGB32_2_10_10_10)) //RGB32 2 : 10 : 10 : 10))
    {
        IsRenderCompressed = true;
    }
    return IsRenderCompressed;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper to get platform info data pointer (non-override platform info)
///
/// @return Pointer to platform info data
/////////////////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO *GMM_STDCALL __GmmGetPlatformInfo()
{
    __GMM_ASSERTPTR(pGmmGlobalContext, NULL)

    if(pGmmGlobalContext->GetPlatformInfoObj() != NULL)
    {
        return (const GMM_PLATFORM_INFO *)(&(pGmmGlobalContext->GetPlatformInfo()));
    }

    return NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper to set FBCRequiredStolenMemorySize
///
/// @param[in] Size: Size of FBC Required Stolen Memory
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL __SetFBCRequiredStolenMemorySize(uint32_t Size)
{
    __GMM_ASSERT(pGmmGlobalContext != NULL)

    if(pGmmGlobalContext != NULL && pGmmGlobalContext->GetPlatformInfoObj() != NULL)
    {
        pGmmGlobalContext->GetPlatformInfoObj()->SetDataFBCRequiredStolenMemorySize(Size);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper to set NumberFenceRegisters
///
/// @param[in] Number: Number of Fence Registers
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL __SetNumberFenceRegisters(uint32_t Number)
{
    __GMM_ASSERT(pGmmGlobalContext != NULL)

    if(pGmmGlobalContext != NULL && pGmmGlobalContext->GetPlatformInfoObj() != NULL)
    {
        pGmmGlobalContext->GetPlatformInfoObj()->SetDataNumberFenceRegisters(Number);
    }
}

#if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper to get override platform info data pointer
///
/// @return Override platfrom info data pointer
/////////////////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO *GMM_STDCALL __GmmGetOverridePlatformInfo()
{
    __GMM_ASSERT(pGmmGlobalContext != NULL)

    if(pGmmGlobalContext != NULL && pGmmGlobalContext->GetOverridePlatformInfoObj() != NULL)
    {
        return (const GMM_PLATFORM_INFO *)(&(pGmmGlobalContext->GetOverridePlatformInfoObj()->GetData()));
    }

    return NULL;
}
#endif

uint32_t GMM_STDCALL GmmPlatformGetBppFromGmmResourceFormat(GMM_RESOURCE_FORMAT Format)
{
    __GMM_ASSERT((Format > GMM_FORMAT_INVALID) && (Format < GMM_RESOURCE_FORMATS));
    __GMM_ASSERT(pGmmGlobalContext);
    __GMM_ASSERT(pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].Element.BitsPer >> 3);
    return pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].Element.BitsPer;
}
