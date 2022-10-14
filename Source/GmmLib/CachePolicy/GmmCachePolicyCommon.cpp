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

/////////////////////////////////////////////////////////////////////////////////////
/// Constructor for the GmmCachePolicyCommon Class, initializes the CachePolicy
/// @param[in]         pCachePolicy
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmCachePolicyCommon::GmmCachePolicyCommon(GMM_CACHE_POLICY_ELEMENT *pCachePolicy, Context *pGmmLibContext)
{
    this->pCachePolicy   = pCachePolicy;
    this->pGmmLibContext = pGmmLibContext;
    NumPATRegisters      = GMM_NUM_PAT_ENTRIES_LEGACY;
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
    if(CachePolicy.WT)
    {
        WantedMemoryType = GMM_GFX_WT;
    }
    else if(!(CachePolicy.LLC || CachePolicy.ELLC))
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
    MEMORY_OBJECT_CONTROL_STATE MOCS = pGmmLibContext->GetCachePolicyElement(GMM_RESOURCE_USAGE_UNKNOWN).MemoryObjectOverride;
   
    if(pResInfo)
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
    __GMM_ASSERT(pGmmLibContext->GetCachePolicyElement(Usage).Initialized);
    CachePolicy = pGmmLibContext->GetCachePolicyUsage();
    // Prevent wrong Usage for XAdapter resources. UMD does not call GetMemoryObject on shader resources but,
    // when they add it someone could call it without knowing the restriction.
    if(pResInfo &&
       pResInfo->GetResFlags().Info.XAdapter &&
       Usage != GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE)
    {
        __GMM_ASSERT(false);
    }

    if(!pResInfo ||
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
///      A simple getter function returning the PAT (cache policy) for a given
///      use Usage of the named resource pResInfo.
///      Typically used to populate PPGTT/GGTT.
///
/// @param[in]     pResInfo: Resource info for resource, can be NULL.
/// @param[in]     Usage: Current usage for resource.
///
/// @return        PATIndex
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmCachePolicyCommon::CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable)
{
    GMM_UNREFERENCED_PARAMETER(pResInfo);
    GMM_UNREFERENCED_PARAMETER(Usage);
    GMM_UNREFERENCED_PARAMETER(pCompressionEnable);
    GMM_UNREFERENCED_PARAMETER(IsCpuCacheable);

    return GMM_PAT_ERROR;
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
    __GMM_ASSERT(pGmmLibContext->GetCachePolicyElement(Usage).Initialized);
    return pGmmLibContext->GetCachePolicyElement(Usage).PTE;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns number of PAT entries possible on that platform
///
/// @return        uint32_t
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmCachePolicyCommon::CachePolicyGetNumPATRegisters()
{
    return NumPATRegisters;
}
