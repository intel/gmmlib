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

//===========================================================================
// Global Variable:
//      pGmmGlobalContext
//
// Description:
//     Handle to global GMM structure containing GMM context and platform info.
//
//----------------------------------------------------------------------------
GMM_GLOBAL_CONTEXT* pGmmGlobalContext = NULL;

#if defined( __ghs__)
std::atomic<int> GmmLib::Context::RefCount = 0;
#else
int32_t GmmLib::Context::RefCount = 0;
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for creating GmmLib::Context object
/// @see        Class GmmLib::Context
///
/// @param[in]  Platform: platform variable. Includes product family (Haswell, Cherryview,
///                       Broxton) with related render and display core revision (GEN3,
//                        ..., GEN10)
/// @param[in]  pSkuTable: Pointer to the sku feature table. Set of capabilities to
///                        allow code paths to be feature based and GEN agnostic.
/// @param[in]  pWaTable:  Pointer to the work around table. A set of anti-features,
///                        often in early/prototype silicon that require work-arounds
///                        until they are resolved to allow code paths to be GEN agnostic.
/// @param[in]  pGtSysInfo: Pointer to the GT system info. Contains various GT System
///                        Information such as EU counts, Thread Counts, Cache Sizes etc.
/// @param[in]  ClientType : describles the UMD clients such as OCL, DX, OGL, Vulkan etc
/// @return     GMM_SUCCESS if Context is created, GMM_ERROR otherwise
/////////////////////////////////////////////////////////////////////////////////////
#if defined(LINUX)
GMM_STATUS GMM_STDCALL GmmInitGlobalContext(const PLATFORM Platform,
                                            const void* pSkuTable,
                                            const void* pWaTable,
                                            const void* pGtSysInfo,
                                            GMM_CLIENT ClientType)
#else
GMM_STATUS GMM_STDCALL GmmInitGlobalContext(const PLATFORM Platform,
                                            const SKU_FEATURE_TABLE* pSkuTable,
                                            const WA_TABLE* pWaTable,
                                            const GT_SYSTEM_INFO* pGtSysInfo,
                                            GMM_CLIENT ClientType)
#endif
{
    __GMM_ASSERTPTR(pSkuTable, GMM_ERROR);
    __GMM_ASSERTPTR(pWaTable, GMM_ERROR);
    __GMM_ASSERTPTR(pGtSysInfo, GMM_ERROR);
    SKU_FEATURE_TABLE *skuTable;
    WA_TABLE          *waTable;
    GT_SYSTEM_INFO    *sysInfo;

    skuTable = (SKU_FEATURE_TABLE *)pSkuTable;
    waTable  = (WA_TABLE *) pWaTable;
    sysInfo  = (GT_SYSTEM_INFO *)pGtSysInfo;

    int32_t ContextRefCount = GmmLib::Context::IncrementRefCount();
    if (ContextRefCount)
    {
        return GMM_SUCCESS;
    }

    pGmmGlobalContext = new GMM_GLOBAL_CONTEXT();
    if (!pGmmGlobalContext)
    {
        GmmLib::Context::DecrementRefCount();
        return GMM_ERROR;
    }

    return (pGmmGlobalContext->InitContext(Platform, skuTable, waTable, sysInfo, ClientType));
}


/////////////////////////////////////////////////////////////////////////////////////
/// This function frees the GMM context memory.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmDestroyGlobalContext(void)
{
    __GMM_ASSERTPTR(pGmmGlobalContext, VOIDRETURN);

    int32_t ContextRefCount = GmmLib::Context::DecrementRefCount();
    if (!ContextRefCount && pGmmGlobalContext)
    {
        pGmmGlobalContext->DestroyContext();
        delete pGmmGlobalContext;
        pGmmGlobalContext = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Constructor to zero initialize the GmmLib::Context object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::Context::Context() :
                    ClientType(),
                    pPlatformInfo(),
                    pTextureCalc(),
                    SkuTable(),
                    WaTable(),
                    GtSysInfo(),
                #if(defined(__GMM_KMD__))
                    GttContext(),
                #endif
                    pGmmKmdContext(),
                    pGmmUmdContext(),
                    pKmdHwDev(),
                    pUmdAdapter(),
                    pGmmCachePolicy()
                #if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
                    ,Override()
                #endif
                    #if(defined(__GMM_KMD__))
                    , IA32ePATTable()
                    #endif
{
    memset(CachePolicy, 0, sizeof(CachePolicy));
    memset(CachePolicyTbl, 0, sizeof(CachePolicyTbl));
#if(defined(__GMM_KMD__))
    memset(PrivatePATTable, 0, sizeof(PrivatePATTable));
    memset(PrivatePATTableMemoryType, 0, sizeof(PrivatePATTableMemoryType));
#endif

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
    memset(FaultingInfo, 0, sizeof(FaultingInfo));
#endif

    //Default initialize 64KB Page padding percentage.
    AllowedPaddingFor64KbPagesPercentage = 10;
    InternalGpuVaMax = 0;

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
    uint32_t RegKey = 0;
    if (GMM_REGISTRY_READ("SOFTWARE\\Intel\\GMM", AllowedPaddingFor64KbPagesPercentage, RegKey))
    {
        AllowedPaddingFor64KbPagesPercentage = RegKey;
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// Destructor to release  GmmLib::Context object memory
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::Context::~Context()
{

}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function to initialize the GmmLib::Context object with cache policy,
/// platform info, Texture calculator etc.
/// @param[in]  Platform: ref to platform
/// @param[in]  pSkuTable: ptr to sku table
/// @param[in]  pWaTable: ptr to workaround table
/// @param[in]  pGtSysInfo: ptr to gt system info
/// @param[in]  ClientType: client type such as dx, ogl, ocl etc
/// @return   GMM_SUCCESS if init is success, GMM_ERROR otherwise
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::Context::InitContext(
                                            const PLATFORM& Platform,
                                            const SKU_FEATURE_TABLE* pSkuTable,
                                            const WA_TABLE* pWaTable,
                                            const GT_SYSTEM_INFO* pGtSysInfo,
                                            GMM_CLIENT ClientType)
{
    this->ClientType = ClientType;

    // Save the SKU and WA
    this->SkuTable = *pSkuTable;
    this->WaTable = *pWaTable;
    this->GtSysInfo = *pGtSysInfo;

    pGmmGlobalContext->pPlatformInfo = GmmLib::PlatformInfo::Create(Platform, false);

    this->pGmmCachePolicy = GmmLib::GmmCachePolicyCommon::Create();
    if (this->pGmmCachePolicy == NULL)
    {
        return GMM_ERROR;
    }

    this->pGmmCachePolicy->InitCachePolicy();

#if _WIN32 && (_DEBUG || _RELEASE_INTERNAL)

#if !__GMM_KMD__
    __GmmInitSurfaceFaultingInfo();
#endif
#endif

    this->pTextureCalc = GmmLib::GmmTextureCalc::Create(Platform, false);
    if (this->pTextureCalc == NULL)
    {
        return GMM_ERROR;
    }

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function to deallcoate the GmmLib::Context's cache policy, platform info,
/// Texture calculator etc.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::Context::DestroyContext()
{
    if (this->pGmmCachePolicy)
    {
        int32_t CachePolicyObjRefCount = GmmLib::GmmCachePolicyCommon::DecrementRefCount();
        if (!CachePolicyObjRefCount)
        {
            delete this->pGmmCachePolicy;
            this->pGmmCachePolicy = NULL;
        }
    }

    if (this->pTextureCalc)
    {
        int32_t TextureCalcObjRefCount = GmmLib::GmmTextureCalc::DecrementRefCount();
        if (!TextureCalcObjRefCount)
        {
            delete this->pTextureCalc;
            this->pTextureCalc = NULL;
        }
    }

#if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
    if (this->Override.pTextureCalc)
    {
        delete this->Override.pTextureCalc;
        this->Override.pTextureCalc = NULL;
    }
#endif

    if (pGmmGlobalContext->pPlatformInfo)
    {
        int32_t PlatformInfoRefCount = GmmLib::PlatformInfo::DecrementRefCount();
        if (!PlatformInfoRefCount)
        {
            delete pGmmGlobalContext->pPlatformInfo;
            pGmmGlobalContext->pPlatformInfo = NULL;
        }
    }

#if(defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL))
    if (pGmmGlobalContext->Override.pPlatformInfo)
    {
        delete pGmmGlobalContext->Override.pPlatformInfo;
        pGmmGlobalContext->Override.pPlatformInfo = NULL;
    }
#endif
}

#ifdef __GMM_KMD__ /*LINK CONTEXT TO GLOBAL*/
//=============================================================================
// Function:
//      GmmLinkKmdContextToGlobalInfo
//
// Description:
//      Links KMD GMM_CONTEXT to GMM Global struct
//-----------------------------------------------------------------------------
void GMM_STDCALL GmmLinkKmdContextToGlobalInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_CONTEXT *pGmmKmdContext)
{
    __GMM_ASSERTPTR(pGmmKmdContext, VOIDRETURN);
    pGmmLibContext->SetGmmKmdContext(pGmmKmdContext);
}
#endif /*__GMM_KMD__ LINK CONTEXT TO GLOBAL*/

//C - Wrappers
/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the PlatformInfo ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const PlatformInfo ptr
/////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO* GMM_STDCALL GmmGetPlatformInfo(GMM_GLOBAL_CONTEXT* pGmmLibContext)
{
    return (&pGmmLibContext->GetPlatformInfo());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the cache policy element array ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const cache policy elment ptr
/////////////////////////////////////////////////////////////////////////
const GMM_CACHE_POLICY_ELEMENT* GmmGetCachePolicyUsage(GMM_GLOBAL_CONTEXT* pGmmLibContext)
{
    return (pGmmLibContext->GetCachePolicyUsage());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the texture calculation object ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   TextureCalc ptr
/////////////////////////////////////////////////////////////////////////
GMM_TEXTURE_CALC* GmmGetTextureCalc(GMM_GLOBAL_CONTEXT* pGmmLibContext)
{
    return (pGmmLibContext->GetTextureCalc());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the sku table ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const SkuTable ptr
/////////////////////////////////////////////////////////////////////////
const SKU_FEATURE_TABLE* GmmGetSkuTable(GMM_GLOBAL_CONTEXT* pGmmLibContext)
{
    return (&pGmmLibContext->GetSkuTable());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the Wa table ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const WaTable ptr
/////////////////////////////////////////////////////////////////////////
const WA_TABLE* GmmGetWaTable(GMM_GLOBAL_CONTEXT* pGmmLibContext)
{
    return (&pGmmLibContext->GetWaTable());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the GT system info ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const GtSysInfo ptr
/////////////////////////////////////////////////////////////////////////
const GT_SYSTEM_INFO* GmmGetGtSysInfo(GMM_GLOBAL_CONTEXT* pGmmLibContext)
{
    return (pGmmLibContext->GetGtSysInfoPtr());
}

#ifdef __GMM_KMD__

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the private PAT table memory type for a given PAT type
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @param[in]  PatIndex: PAT index
/// @return   PAT Memory type
/////////////////////////////////////////////////////////////////////////
int32_t  GmmGetPrivatePATTableMemoryType(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_GFX_PAT_TYPE PatType)
{
    return (pGmmLibContext->GetPrivatePATTableMemoryType(PatType));
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the private PAT table entry for a given PAT index
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @param[in]  PatIndex: PAT index
/// @return   PAT entry
/////////////////////////////////////////////////////////////////////////
GMM_PRIVATE_PAT  GmmGetPrivatePATEntry(GMM_GLOBAL_CONTEXT *pGmmLibContext, uint32_t  PatIndex)
{
    GMM_PRIVATE_PAT NullPAT = { 0 };

    if (PatIndex >= GMM_NUM_PAT_ENTRIES)
    {
        GMM_ASSERTDPF(false, "CRITICAL ERROR: INVALID PAT IDX");
        return NullPAT;
    }

    return pGmmLibContext->GetPrivatePATEntry(PatIndex);
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the gmm kmd context ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   GmmKmdContext ptr
/////////////////////////////////////////////////////////////////////////
GMM_CONTEXT* GmmGetGmmKmdContext(GMM_GLOBAL_CONTEXT *pGmmLibContext)
{
    return (pGmmLibContext->GetGmmKmdContext());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get gtt context ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   GttContext ptr
/////////////////////////////////////////////////////////////////////////
GMM_GTT_CONTEXT* GmmGetGttContext(GMM_GLOBAL_CONTEXT *pGmmLibContext)
{
    return (pGmmLibContext->GetGttContext());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the  Cache policy tbl element for a given usage type
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @param[in]  Usage: cache policy resource usage type
/// @return   cache policy tbl element
////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_TBL_ELEMENT GmmGetCachePolicyTblElement(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_RESOURCE_USAGE_TYPE Usage)
{
    return (pGmmLibContext->GetCachePolicyTblElement(Usage));
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the Cache policy element for a given usage type
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @param[in]  Usage: cache policy resource usage type
/// @return   cache policy element
////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_ELEMENT GmmGetCachePolicyElement(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_RESOURCE_USAGE_TYPE Usage)
{
    return (pGmmLibContext->GetCachePolicyElement(Usage));
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to reset  the sku Table after  GmmInitContext() could have
/// changed them since original latching
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @param[in]  SkuTable: platform based sku feature table
////////////////////////////////////////////////////////////////////////
void GmmSetSkuTable(GMM_GLOBAL_CONTEXT *pGmmLibContext, SKU_FEATURE_TABLE SkuTable)
{
    pGmmLibContext->SetSkuTable(SkuTable);
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to reset the Wa Table after  GmmInitContext() could have
/// changed them since original latching
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @param[in]  WaTable: platform based workaround table
////////////////////////////////////////////////////////////////////////
void GmmSetWaTable(GMM_GLOBAL_CONTEXT *pGmmLibContext, WA_TABLE WaTable)
{
    pGmmLibContext->SetWaTable(WaTable);
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the Platform info ptr to kmd
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   PlatformInfo ptr
////////////////////////////////////////////////////////////////////////
GMM_PLATFORM_INFO* GmmKmdGetPlatformInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext)
{
    return (&pGmmLibContext->GetPlatformInfo());
}

#if(_DEBUG || _RELEASE_INTERNAL)
/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the override Platform info ptr to kmd
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   override PlatformInfo ptr
////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO* GmmGetOverridePlatformInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext)
{
    return (&pGmmLibContext->GetOverridePlatformInfo());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the override Texture calc ptr to kmd
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   override Texture calc ptr
////////////////////////////////////////////////////////////////////////
GMM_TEXTURE_CALC* GmmGetOverrideTextureCalc(GMM_GLOBAL_CONTEXT *pGmmLibContext)
{
    return (pGmmLibContext->GetOverrideTextureCalc());
}
#endif

#endif
