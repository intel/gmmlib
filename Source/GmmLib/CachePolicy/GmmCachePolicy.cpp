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
/// C Wrapper function for GmmLib::GmmCachePolicyGetPteType
/// @see           GmmLib::GmmCachePolicyCommon::CachePolicyGetPteType()
///
/// @param[in]     Usage: type of usage
///
/// @return        GMM_PTE_CACHE_CONTROL_BITS:Populated PTE
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_PTE_CACHE_CONTROL_BITS GMM_STDCALL GmmCachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyGetPteType(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmLib::GmmCachePolicyIsUsagePTECached
/// @see           GmmLib::GmmCachePolicyCommon::CachePolicyIsUsagePTECached()
///
/// @param[in]     Usage: type of usage
///
/// @return        1 if the usage PTE entry is set for cached, 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmCachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyIsUsagePTECached(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetCachePolicyUsage.
/// @see        GmmLib::GmmResourceInfoCommon::GetCachePolicyUsage()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @return         Cache policy usage
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_USAGE_TYPE GMM_STDCALL GmmCachePolicyGetResourceUsage(GMM_RESOURCE_INFO *pResInfo)
{
    __GMM_ASSERT(pResInfo);
    return pResInfo->GetCachePolicyUsage();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::OverrideCachePolicyUsage.
/// @see        GmmLib::GmmResourceInfoCommon::OverrideCachePolicyUsage()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]      Cache policy usage
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmCachePolicyOverrideResourceUsage(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage)
{
    pResInfo->OverrideCachePolicyUsage(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmLib::GmmCachePolicyGetMemoryObject
/// @see           GmmLib::GmmCachePolicyCommon::CachePolicyGetMemoryObject()
///
/// param[in]      pResInfo: Resource info for resource, can be NULL.
/// param[in]      Usage: Current usage for resource.
///
/// @return        MEMORY_OBJECT_CONTROL_STATE: Gen adjusted MOCS structure (cache
///                                             policy) for the given buffer use.
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmCachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyGetMemoryObject(pResInfo, Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmLib::GmmCachePolicyGetOriginalMemoryObject
///  @see           GmmLib::GmmCachePolicyCommon::CachePolicyGetOriginalMemoryObject()
///
///  @param[in]     pResInfo: Resource info for resource , can be null
///  @param[in]     Usage: Current usage for resource
///
///  @return        MEMORY_OBJECT_CONTROL_STATE: Populated memory object
///
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmCachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyGetOriginalMemoryObject(pResInfo);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmCachePolicy::GmmGetWantedMemoryType.
/// @see            GmmLib::GmmCachePolicy::GetWantedMemoryType()
///
/// @param[in]      CachePolicy:cache policy for a usage
///
/// @return         wanted memory type
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_MEMORY_TYPE GmmGetWantedMemoryType(GMM_CACHE_POLICY_ELEMENT CachePolicy)
{
    return pGmmGlobalContext->GetCachePolicyObj()->GetWantedMemoryType(CachePolicy);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns count of current MOCS values for MOCS Table programming at GMM boot
///
/// @param[in]  none:
/// @return     uint32_t  no of mocs register required to program
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetMaxMocsIndex(void)
{
    GMM_CACHE_POLICY *          pCachePolicy = pGmmGlobalContext->GetCachePolicyObj();
    GmmLib::GmmGen9CachePolicy *ptr          = static_cast<GmmLib::GmmGen9CachePolicy *>(pCachePolicy);
    return ptr->CurrentMaxMocsIndex;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns count of current L1 HDC MOCS values for MOCS Table programming at GMM boot
///
/// @param[in]  none:
/// @return     uint32_t  max L1 hdc mocs index needed to program
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetMaxL1HdcMocsIndex(void)
{
    GMM_CACHE_POLICY *          pCachePolicy = pGmmGlobalContext->GetCachePolicyObj();
    GmmLib::GmmGen9CachePolicy *ptr          = static_cast<GmmLib::GmmGen9CachePolicy *>(pCachePolicy);
    return ptr->CurrentMaxL1HdcMocsIndex;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns count of current Special MOCS values for MOCS Table programming at GMM boot
///
/// @param[in]  none:
/// @return     uint32_t  max special mocs index needed to program
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetMaxSpecialMocsIndex(void)
{
    GMM_CACHE_POLICY *pCachePolicy = pGmmGlobalContext->GetCachePolicyObj();
    return pCachePolicy->GetMaxSpecialMocsIndex();
}