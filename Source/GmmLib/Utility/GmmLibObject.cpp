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
#include "Internal/Common/Platform/GmmGen12Platform.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen10.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen11.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen12.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen12dGPU.h"
#include "Internal/Common/Texture/GmmTextureCalc.h"
#include "Internal/Common/Texture/GmmGen10TextureCalc.h"
#include "Internal/Common/Texture/GmmGen11TextureCalc.h"
#include "Internal/Common/Texture/GmmGen12TextureCalc.h"

/////////////////////////////////////////////////////////////////////////////////////
/// Static function to return a PlatformInfo object based on input platform
///
/// @param[in]  Platform: Specify the information about the PlatformInfo object to be created
/// @param[in]  Override: Indicates if Override should be used (KMD and Debug/Release-Internal Only)
/// @return     PlatformInfo object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::PlatformInfo *GmmLib::PlatformInfo::Create(PLATFORM Platform, bool Override)
{
    GMM_DPF_ENTER;

    if(Override == false)
    {
        GmmLib::PlatformInfo::IncrementRefCount();
        if(pGmmGlobalContext->GetPlatformInfoObj() != NULL)
        {
            return pGmmGlobalContext->GetPlatformInfoObj();
        }
    }
#if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
    else
    {
        if(pGmmGlobalContext->GetOverridePlatformInfoObj() != NULL)
        {
            delete pGmmGlobalContext->GetOverridePlatformInfoObj();
            pGmmGlobalContext->SetOverridePlatformInfoObj(NULL);
        }
    }
#endif
    GMM_DPF_EXIT;
    switch(GFX_GET_CURRENT_RENDERCORE(Platform))
    {
        case IGFX_GEN12LP_CORE:
        case IGFX_GEN12_CORE:
        case IGFX_XE_HP_CORE:
            return new GmmLib::PlatformInfoGen12(Platform);
            break;
        case IGFX_GEN11_CORE:
            return new GmmLib::PlatformInfoGen11(Platform);
            break;
        case IGFX_GEN10_CORE:
            return new GmmLib::PlatformInfoGen10(Platform);
            break;
        case IGFX_GEN9_CORE:
            return new GmmLib::PlatformInfoGen9(Platform);
            break;
        default:
            return new GmmLib::PlatformInfoGen8(Platform);
            break;
    }

}

/////////////////////////////////////////////////////////////////////////////////////
/// This function creates the GmmCachePolicy object
///
/// @return        GmmCachePolicyCommon
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmCachePolicyCommon *GmmLib::GmmCachePolicyCommon::Create()
{
    GMM_CACHE_POLICY *        pGmmCachePolicy = NULL;
    GMM_CACHE_POLICY_ELEMENT *CachePolicy     = NULL;
    CachePolicy                               = pGmmGlobalContext->GetCachePolicyUsage();

    IncrementRefCount();
    if(pGmmGlobalContext->GetCachePolicyObj())
    {
        return pGmmGlobalContext->GetCachePolicyObj();
    }

        switch(GFX_GET_CURRENT_RENDERCORE(pGmmGlobalContext->GetPlatformInfo().Platform))
        {
            case IGFX_GEN12LP_CORE:
            case IGFX_GEN12_CORE:
            case IGFX_XE_HP_CORE:
                if(pGmmGlobalContext->GetSkuTable().FtrLocalMemory)
                {
                    pGmmCachePolicy = new GmmLib::GmmGen12dGPUCachePolicy(CachePolicy);
                }
                else
                {
                    pGmmCachePolicy = new GmmLib::GmmGen12CachePolicy(CachePolicy);
                }
                break;
            case IGFX_GEN11_CORE:
                pGmmCachePolicy = new GmmLib::GmmGen11CachePolicy(CachePolicy);
                break;
            case IGFX_GEN10_CORE:
                pGmmCachePolicy = new GmmLib::GmmGen10CachePolicy(CachePolicy);
                break;
            case IGFX_GEN9_CORE:
                pGmmCachePolicy = new GmmLib::GmmGen9CachePolicy(CachePolicy);
                break;
            default:
                pGmmCachePolicy = new GmmLib::GmmGen8CachePolicy(CachePolicy);
                break;
        }

    if(!pGmmCachePolicy)
    {
        GMM_DPF_CRITICAL("unable to allocate memory for CachePolicy Object");
    }

    return pGmmCachePolicy;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Creates GmmLib-wide global TextureCalc object based on PlatformID, which will
/// be used by GmmResourceInfo* class for populating GMM_TEXTURE_INFO
///
/// @param[in]  Platform: PLATFORM contains platform ID
///
/// @return     Returns an instance of GmmTextureCalc's derived class
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmTextureCalc *GmmLib::GmmTextureCalc::Create(PLATFORM Platform, uint8_t Override)
{
    if(!Override)
    {
        IncrementRefCount();
        if(pGmmGlobalContext->GetTextureCalc())
        {
            return pGmmGlobalContext->GetTextureCalc();
        }
    }

    switch(GFX_GET_CURRENT_RENDERCORE(Platform))
    {
        case IGFX_GEN7_CORE:
        case IGFX_GEN7_5_CORE:
            return new GmmGen7TextureCalc();
            break;
        case IGFX_GEN8_CORE:
            return new GmmGen8TextureCalc();
            break;
        case IGFX_GEN9_CORE:
            return new GmmGen9TextureCalc();
            break;
        case IGFX_GEN10_CORE:
            return new GmmGen10TextureCalc();
            break;
        case IGFX_GEN11_CORE:
            return new GmmGen11TextureCalc();
            break;
        case IGFX_GEN12LP_CORE:
        case IGFX_GEN12_CORE:
        case IGFX_XE_HP_CORE:
        default:
            return new GmmGen12TextureCalc();
            break;
    }
}
