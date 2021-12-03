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
#include "Internal/Common/Platform/GmmGen10Platform.h"
#include "Internal/Common/Platform/GmmGen11Platform.h"

GmmLib::PlatformInfoGen11::PlatformInfoGen11(PLATFORM &Platform, Context *pGmmLibContext)
    : PlatformInfoGen10(Platform, pGmmLibContext)
{

    Data.SurfaceMaxSize                      = GMM_GBYTE(16384);
    Data.MaxGpuVirtualAddressBitsPerResource = 44;

    //Override the Height VP9 VdEnc requirement for Gen11 8k resolution.
    Data.ReconMaxHeight = GMM_KBYTE(32);

    //Override CCS MaxPitch requirement
    if(GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_ICELAKE)
    {
        Data.TexAlign.CCS.MaxPitchinTiles = 1024;
    }

    if(GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_LAKEFIELD)
    {
        Data.SurfaceMaxSize                      = GMM_GBYTE(64);
        Data.MaxGpuVirtualAddressBitsPerResource = 36;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the MMC parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfoGen11::ValidateMMC(GMM_TEXTURE_INFO &Surf)
{

    if(Surf.Flags.Gpu.MMC && //For Media Memory Compression --
       ((!(GMM_IS_4KB_TILE(Surf.Flags) || GMM_IS_64KB_TILE(Surf.Flags))) ||
        Surf.ArraySize > GMM_MAX_MMC_INDEX))
    {
        return 0;
    }

    if(GFX_GET_CURRENT_PRODUCT(pGmmLibContext->GetPlatformInfo().Platform) == IGFX_LAKEFIELD)
    {
        if(Surf.Flags.Gpu.MMC &&
           Surf.Flags.Gpu.UnifiedAuxSurface &&
           !(Surf.Flags.Info.TiledY &&
             (Surf.Format == GMM_FORMAT_NV12 || GmmIsP0xx(Surf.Format))))
        {
            GMM_ASSERTDPF(0, "Invalid MMC usage for LKF!");
            return 0;
        }
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
uint8_t GmmLib::PlatformInfoGen11::ValidateUnifiedAuxSurface(GMM_TEXTURE_INFO &Surf)
{
    if((Surf.Flags.Gpu.UnifiedAuxSurface) &&
       !( //--- Legitimate UnifiedAuxSurface Case ------------------------------------------
       Surf.Flags.Gpu.CCS &&
       ((Surf.MSAA.NumSamples <= 1 && (Surf.Flags.Gpu.RenderTarget || Surf.Flags.Gpu.Texture))) ||
       ((GFX_GET_CURRENT_PRODUCT(pGmmLibContext->GetPlatformInfo().Platform) == IGFX_LAKEFIELD) && Surf.Flags.Gpu.MMC &&
        (Surf.MSAA.NumSamples <= 1))))
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
uint8_t GmmLib::PlatformInfoGen11::CheckFmtDisplayDecompressible(GMM_TEXTURE_INFO &Surf,
                                                                 bool              IsSupportedRGB64_16_16_16_16,
                                                                 bool              IsSupportedRGB32_8_8_8_8,
                                                                 bool              IsSupportedRGB32_2_10_10_10,
                                                                 bool              IsSupportedMediaFormats)
{
    bool IsRenderCompressed = false;
    bool IsMediaCompressed  = false;
    GMM_UNREFERENCED_PARAMETER(IsSupportedMediaFormats);

    if(GFX_GET_CURRENT_PRODUCT(pGmmLibContext->GetPlatformInfo().Platform) == IGFX_LAKEFIELD)
    {
        if(Surf.Flags.Gpu.MMC &&
           Surf.Flags.Info.TiledY &&
           (Surf.Format == GMM_FORMAT_NV12 ||
            Surf.Format == GMM_FORMAT_P010))
        {
            IsMediaCompressed = true;
        }

        if(IsSupportedRGB64_16_16_16_16 || //RGB64 16:16 : 16 : 16 FP16
           IsSupportedRGB32_8_8_8_8 ||     //RGB32 8 : 8 : 8 : 8
           IsSupportedRGB32_2_10_10_10)    //RGB32 2 : 10 : 10 : 10
        {
            IsRenderCompressed = true;
        }
    }
    else
    {
        // Pre-LKF1
        if(IsSupportedRGB32_8_8_8_8 || //RGB32 8 : 8 : 8 : 8
           (GFX_GET_CURRENT_PRODUCT(pGmmLibContext->GetPlatformInfo().Platform) == IGFX_ICELAKE &&
            IsSupportedRGB64_16_16_16_16)) //RGB64 16:16 : 16 : 16 FP16
        {
            IsRenderCompressed = true;
        }
    }
    return IsRenderCompressed || IsMediaCompressed;
}
