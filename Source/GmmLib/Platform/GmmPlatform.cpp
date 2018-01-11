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

GmmLib::PlatformInfo::PlatformInfo(PLATFORM &Platform) {
    GMM_DPF_ENTER;

    memset(&Data, 0, sizeof(Data));
    Data.Platform = Platform;

    GMM_RESOURCE_FORMAT GmmFormat;
#define GMM_FORMAT_GEN(X)      (GFX_GET_CURRENT_RENDERCORE(Data.Platform) >= IGFX_GEN##X##_CORE)
#define GMM_FORMAT_SKU(FtrXxx) (pGmmGlobalContext->GetSkuTable().FtrXxx != 0)
#define GMM_FORMAT_WA(WaXxx)   (pGmmGlobalContext->GetWaTable().WaXxx != 0)
#define GMM_FORMAT(Name, bpe, _Width, _Height, _Depth, IsRT, IsASTC, RcsSurfaceFormat, AuxL1Format, Availability)    \
{                                                                                                       \
    GmmFormat = GMM_FORMAT_##Name;                                                                      \
    Data.FormatTable[GmmFormat].ASTC =                (IsASTC);                                              \
    Data.FormatTable[GmmFormat].Element.BitsPer =     (bpe);                                                 \
    Data.FormatTable[GmmFormat].Element.Depth =       (_Depth);                                              \
    Data.FormatTable[GmmFormat].Element.Height =      (_Height);                                             \
    Data.FormatTable[GmmFormat].Element.Width =       (_Width);                                              \
    Data.FormatTable[GmmFormat].RenderTarget =        ((IsRT) != 0);                                         \
    Data.FormatTable[GmmFormat].SurfaceStateFormat =  ((GMM_SURFACESTATE_FORMAT)(RcsSurfaceFormat));         \
    Data.FormatTable[GmmFormat].Reserved 	=         ((uint32_t)(AuxL1Format));                   \
    Data.FormatTable[GmmFormat].Supported =           ((Availability) != 0);                                 \
    if(((_Depth) > 1) || ((_Height) > 1) || ((_Width) > 1))                                             \
    {                                                                                                   \
        Data.FormatTable[GmmFormat].Compressed = 1;                                                     \
    }                                                                                                   \
}

#include "External/Common/GmmFormatTable.h"

}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper to get platform info data pointer (non-override platform info)
///
/// @return Pointer to platform info data
/////////////////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO* GMM_STDCALL __GmmGetPlatformInfo()
{
    __GMM_ASSERTPTR(pGmmGlobalContext, NULL)

    if (pGmmGlobalContext->GetPlatformInfoObj() != NULL)
    {
        return (const GMM_PLATFORM_INFO*)(&(pGmmGlobalContext->GetPlatformInfo()));
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

    if (pGmmGlobalContext != NULL && pGmmGlobalContext->GetPlatformInfoObj() != NULL)
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

    if (pGmmGlobalContext != NULL && pGmmGlobalContext->GetPlatformInfoObj() != NULL)
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
const GMM_PLATFORM_INFO* GMM_STDCALL __GmmGetOverridePlatformInfo()
{
    __GMM_ASSERT(pGmmGlobalContext != NULL)

    if (pGmmGlobalContext != NULL && pGmmGlobalContext->GetOverridePlatformInfoObj() != NULL)
    {
        return (const GMM_PLATFORM_INFO*)(&(pGmmGlobalContext->GetOverridePlatformInfoObj()->GetData()));
    }

    return NULL;
}
#endif

uint32_t GMM_STDCALL GmmPlatformGetBppFromGmmResourceFormat(GMM_RESOURCE_FORMAT Format)
{
    __GMM_ASSERT((Format>GMM_FORMAT_INVALID) && (Format<GMM_RESOURCE_FORMATS));
    __GMM_ASSERT(pGmmGlobalContext);
    __GMM_ASSERT(pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].Element.BitsPer >> 3);
    return pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].Element.BitsPer;
}
