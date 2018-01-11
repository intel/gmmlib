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
#include "External/Common/GmmCachePolicy.h"

int32_t GmmLib::GmmCachePolicyCommon::RefCount = 0;

#if _WIN32
/////////////////////////////////////////////////////////////////////////////////////
/// This function will read registry keys and override cache policy
///
/////////////////////////////////////////////////////////////////////////////////////
// OverrideCachePolicy can take over a minute to compile when optimizations
// are enabled. As this is only needed during init time for non-Release builds,
// we can selectively disable optimizations for this function and improve build times
#if( _DEBUG || _RELEASE_INTERNAL )
#pragma optimize("", off)
void GmmLib::GmmCachePolicyCommon::OverrideCachePolicy()
{
    uint32_t UsageCount = 0;

    int32_t  DefaultEnable = 0, DefaultLLC = 0, DefaultELLC = 0, DefaultL3 = 0;
    int32_t  DefaultAge = 0, DefaultWT = 0, DefaultAOM = 0, DefaultLeCC_SCC = 0;
    int32_t  DefaultL3_SCC = 0, DefaultSCF = 0, DefaultHDCL1 = 0, DefaultSSO = 0;
    int32_t  DefaultCoS = 0, DefaultL3Eviction = 0;

    // Variables used in the REG_OVERRIDE macro block
    int32_t  Enable = 0, LLC = -1, ELLC = -1, L3 = -1, Age = -1, WT = -1, AOM = -1, LeCC_SCC = -1, L3_SCC = -1, SCF = -1, SSO = -1, CoS = -1, HDCL1 = -1, L3Eviction = -1;

#define READ_DEFAULT_OVERRIDE(CacheParam)                                             \
{                                                                                     \
    if (REGISTRY_OVERRIDE_READ(OverrideDefaults ,CacheParam) == false)                \
    {                                                                                 \
        CacheParam = -1;                                                              \
    }                                                                                 \
    Default##CacheParam = CacheParam;                                                 \
}

#define OVERRIDE_DEFAULT(Usage)                             \
{                                                           \
    if (DefaultEnable)                                      \
    {                                                       \
        if (DefaultLLC != -1)                               \
        {                                                   \
            pCachePolicy[Usage].LLC = DefaultLLC;           \
        }                                                   \
        if (DefaultELLC != -1)                              \
        {                                                   \
            pCachePolicy[Usage].ELLC = DefaultELLC;         \
        }                                                   \
        if (DefaultL3 != -1)                                \
        {                                                   \
            pCachePolicy[Usage].L3 = DefaultL3;             \
        }                                                   \
        if (DefaultAge != -1)                               \
        {                                                   \
            pCachePolicy[Usage].AGE = DefaultAge;           \
        }                                                   \
        if (DefaultWT != -1)                                \
        {                                                   \
            pCachePolicy[Usage].WT = DefaultWT;             \
        }                                                   \
        if (DefaultAOM != -1)                               \
        {                                                   \
            pCachePolicy[Usage].AOM = DefaultAOM;           \
        }                                                   \
        if (DefaultLeCC_SCC != -1)                          \
        {                                                   \
            pCachePolicy[Usage].LeCC_SCC = DefaultLeCC_SCC; \
        }                                                   \
        if (DefaultL3_SCC != -1)                            \
        {                                                   \
            pCachePolicy[Usage].L3_SCC = DefaultL3_SCC;     \
        }                                                   \
        if (DefaultSCF != -1)                               \
        {                                                   \
            pCachePolicy[Usage].SCF = DefaultSCF;           \
        }                                                   \
        if (DefaultSSO != -1)                               \
        {                                                   \
            pCachePolicy[Usage].SSO = DefaultSSO;           \
        }                                                   \
        if (DefaultCoS != -1)                               \
        {                                                   \
            pCachePolicy[Usage].CoS = DefaultCoS;           \
        }                                                   \
        if (DefaultHDCL1 != -1)                             \
        {                                                   \
            pCachePolicy[Usage].HDCL1 = DefaultHDCL1;       \
        }                                                   \
        if (DefaultL3Eviction != -1)                            \
        {                                                       \
            pCachePolicy[Usage].L3Eviction = DefaultL3Eviction; \
        }                                                       \
    }                                                           \
}

#ifdef __GMM_KMD__
    uint32_t GenerateKeys = 0;
    REGISTRY_OVERRIDE_READ(, GenerateKeys);
#endif
    REGISTRY_OVERRIDE_READ(OverrideDefaults, Enable); DefaultEnable = Enable;

#ifdef __GMM_KMD__
    __GMM_ASSERT(!(DefaultEnable && GenerateKeys));
    // Disable DefaultEnable if GenerateKeys is enabled
    DefaultEnable = GenerateKeys ? 0 : DefaultEnable;
#endif

    if (DefaultEnable)
    {
        READ_DEFAULT_OVERRIDE(LLC);
        READ_DEFAULT_OVERRIDE(ELLC);
        READ_DEFAULT_OVERRIDE(L3);
        READ_DEFAULT_OVERRIDE(Age);
        READ_DEFAULT_OVERRIDE(WT);
        READ_DEFAULT_OVERRIDE(AOM);
        READ_DEFAULT_OVERRIDE(LeCC_SCC);
        READ_DEFAULT_OVERRIDE(L3_SCC);
        READ_DEFAULT_OVERRIDE(SCF);
        READ_DEFAULT_OVERRIDE(SSO);
        READ_DEFAULT_OVERRIDE(CoS);
        READ_DEFAULT_OVERRIDE(HDCL1);
        READ_DEFAULT_OVERRIDE(L3Eviction);
    }

    OVERRIDE_DEFAULT(GMM_RESOURCE_USAGE_UNKNOWN); REG_OVERRIDE(GMM_RESOURCE_USAGE_UNKNOWN);


#define DEFINE_RESOURCE_USAGE(Usage) OVERRIDE_DEFAULT(Usage); REG_OVERRIDE(Usage);
    #include "GmmCachePolicyResourceUsageDefinitions.h"
    #undef DEFINE_RESOURCE_USAGE
    #undef OVERRIDE_DEFAULT
    #undef READ_DEFAULT_OVERRIDE
        __GMM_ASSERT(UsageCount == GMM_RESOURCE_USAGE_MAX);

    #ifdef __GMM_KMD__
        if (GenerateKeys)
        {
            GenerateKeys = 0;
            REGISTRY_OVERRIDE_WRITE(, GenerateKeys, GenerateKeys);
        }
    #endif
}
#pragma optimize("", on)
#endif

#endif // #if _WIN32

/////////////////////////////////////////////////////////////////////////////////////
/// Constructor for the GmmCachePolicyCommon Class, initializes the CachePolicy
/// @param[in]         pCachePolicy
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmCachePolicyCommon::GmmCachePolicyCommon(GMM_CACHE_POLICY_ELEMENT *pCachePolicy)
{
    this->pCachePolicy = pCachePolicy;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the wanted memory type for this usage.
///
/// @param[in]      CachePolicy: cache policy for a usage
///
/// @return         wanted memory type
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_MEMORY_TYPE GmmLib::GmmCachePolicyCommon::GetWantedMemoryType(GMM_CACHE_POLICY_ELEMENT CachePolicy)
{
    GMM_GFX_MEMORY_TYPE WantedMemoryType = GMM_GFX_UC_WITH_FENCE;
    if (CachePolicy.WT)
    {
        WantedMemoryType = GMM_GFX_WT;
    }
    else if (!(CachePolicy.LLC || CachePolicy.ELLC))
    {
        WantedMemoryType = GMM_GFX_UC_WITH_FENCE;
    }
    else
    {
        WantedMemoryType = GMM_GFX_WB;
    }
    return WantedMemoryType;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Generates memory object based on resource usage
///
/// @param[in]     pResInfo: Resource info for resource , can be null
/// @param[in]     Usage: Current usage for resource
///
/// @return        MEMORY_OBJECT_CONTROL_STATE:    Populated memory object
///
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmLib::GmmCachePolicyCommon::CachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo)
{
    MEMORY_OBJECT_CONTROL_STATE MOCS = pGmmGlobalContext->GetCachePolicyElement(GMM_RESOURCE_USAGE_UNKNOWN).MemoryObjectOverride;

    if (pResInfo)
    {
        MOCS = pResInfo->GetMOCS();
    }

    return MOCS;
}

/////////////////////////////////////////////////////////////////////////////////////
///      A simple getter function returning the MOCS (cache policy) for a given
///      use Usage of the named resource pResInfo.
///      Typically used to populate a SURFACE_STATE for a GPU task.
///
/// @param[in]     pResInfo: Resource info for resource, can be NULL.
/// @param[in]     Usage: Current usage for resource.
///
/// @return        MEMORY_OBJECT_CONTROL_STATE: Gen adjusted MOCS structure (cache
///                                             policy) for the given buffer use.
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmLib::GmmCachePolicyCommon::CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage)
{
    const GMM_CACHE_POLICY_ELEMENT *CachePolicy = NULL;
    __GMM_ASSERT(pGmmGlobalContext->GetCachePolicyElement(Usage).Initialized);
    CachePolicy = pGmmGlobalContext->GetCachePolicyUsage();

    // Prevent wrong Usage for XAdapter resources. UMD does not call GetMemoryObject on shader resources but,
    // when they add it someone could call it without knowing the restriction.
    if (pResInfo &&
        pResInfo->GetResFlags().Info.XAdapter &&
        Usage != GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE)
    {
        __GMM_ASSERT(false);
    }

    if (!pResInfo ||
        (CachePolicy[Usage].Override & CachePolicy[pResInfo->GetCachePolicyUsage()].IDCode) ||
        (CachePolicy[Usage].Override == ALWAYS_OVERRIDE))
    {
        return CachePolicy[Usage].MemoryObjectOverride;
    }
    else
    {
        return CachePolicy[Usage].MemoryObjectNoOverride;
    }

    return CachePolicy[GMM_RESOURCE_USAGE_UNKNOWN].MemoryObjectOverride;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Generates PTE based on resource usage
///
/// @param[in]     Usage: type of usage
///
/// @return        GMM_PTE_CACHE_CONTROL_BITS: Populated PTE
/////////////////////////////////////////////////////////////////////////////////////
GMM_PTE_CACHE_CONTROL_BITS GMM_STDCALL GmmLib::GmmCachePolicyCommon::CachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage)
{
    __GMM_ASSERT(pGmmGlobalContext->GetCachePolicyElement(Usage).Initialized);
    return pGmmGlobalContext->GetCachePolicyElement(Usage).PTE;
}
