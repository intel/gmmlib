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
//
/// @param[in]     pLibContext: pGmmLibContext
/// @param[in]     Usage: type of usage
///
/// @return        GMM_PTE_CACHE_CONTROL_BITS:Populated PTE
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_PTE_CACHE_CONTROL_BITS GMM_STDCALL GmmCachePolicyGetPteType(void *pLibContext, GMM_RESOURCE_USAGE_TYPE Usage)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetPteType(Usage);
}
/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmLib::GmmCachePolicyGetPATIndex
/// @see           GmmLib::GmmCachePolicyCommon::CachePolicyGetPATIndex()
///
/// @param[in]          pLibContext: pGmmLibContext
/// @param[in]          Usage: type of usage
/// @param[Optional]    Usage: for Compression Enable
///
/// @return       uint32_t
///
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetPATIndex(void *pLibContext, GMM_RESOURCE_USAGE_TYPE Usage, bool  *pCompressionEnable, bool IsCpuCacheable)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetPATIndex(NULL, Usage, pCompressionEnable, IsCpuCacheable);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmLib::GmmCachePolicyIsUsagePTECached
/// @see           GmmLib::GmmCachePolicyCommon::CachePolicyIsUsagePTECached()
///
/// @param[in]     pLibContext: pGmmLibContext
/// @param[in]     Usage: type of usage
///
/// @return        1 if the usage PTE entry is set for cached, 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmCachePolicyIsUsagePTECached(void *pLibContext, GMM_RESOURCE_USAGE_TYPE Usage)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyIsUsagePTECached(Usage);
}
/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function to return L1 Cache Control on DG2 for a given resource type
///
/// @param[in]     pLibContext: pGmmLibContext
/// @param[in]     Usage: type of usage
///
/// @return         Value of L1 Cache control.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmGetSurfaceStateL1CachePolicy(void *pLibContext, GMM_RESOURCE_USAGE_TYPE Usage)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return (uint8_t)pGmmLibContext->GetCachePolicyObj()->GetSurfaceStateL1CachePolicy(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function to return L2 Cache Control on MTL for a given resource type
///
/// @param[in]     pLibContext: pGmmLibContext
/// @param[in]     Usage: type of usage
///Value of L2 Cache support
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmGetSurfaceStateL2CachePolicy(void *pLibContext, GMM_RESOURCE_USAGE_TYPE Usage)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyElement(Usage).L2CC;
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
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmCachePolicyGetMemoryObject(void *pLibContext, GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetMemoryObject(pResInfo, Usage);
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
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmCachePolicyGetOriginalMemoryObject(void *pLibContext, GMM_RESOURCE_INFO *pResInfo)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetOriginalMemoryObject(pResInfo);
}
/////////////////////////////////////////////////////////////////////////////////////
/// C Wrapper function for GmmCachePolicy::GmmGetWantedMemoryType.
/// @see            GmmLib::GmmCachePolicy::GetWantedMemoryType()
///
/// @param[in]     pLibContext: pGmmLibContext
/// @param[in]     CachePolicy:cache policy for a usage
///
/// @return         wanted memory type
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_MEMORY_TYPE GmmGetWantedMemoryType(void *pLibContext, GMM_CACHE_POLICY_ELEMENT CachePolicy)
{
    GMM_LIB_CONTEXT *pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    return pGmmLibContext->GetCachePolicyObj()->GetWantedMemoryType(CachePolicy);
}
/////////////////////////////////////////////////////////////////////////////////////
/// Returns count of current MOCS values for MOCS Table programming at GMM boot
///
/// @param[in]     pLibContext: pGmmLibContext
/// @return     uint32_t  no of mocs register required to program
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetMaxMocsIndex(void *pLibContext)
{
    GMM_LIB_CONTEXT *           pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    GMM_CACHE_POLICY *          pCachePolicy   = pGmmLibContext->GetCachePolicyObj();
    GmmLib::GmmGen9CachePolicy *ptr            = static_cast<GmmLib::GmmGen9CachePolicy *>(pCachePolicy);
    return ptr->CurrentMaxMocsIndex;
}
/////////////////////////////////////////////////////////////////////////////////////
/// Returns count of current L1 HDC MOCS values for MOCS Table programming at GMM boot
///
/// @param[in]     pLibContext: pGmmLibContext
/// @return     uint32_t  max L1 hdc mocs index needed to program
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetMaxL1HdcMocsIndex(void *pLibContext)
{
    GMM_LIB_CONTEXT *           pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    GMM_CACHE_POLICY *          pCachePolicy   = pGmmLibContext->GetCachePolicyObj();
    GmmLib::GmmGen9CachePolicy *ptr            = static_cast<GmmLib::GmmGen9CachePolicy *>(pCachePolicy);
    return ptr->CurrentMaxL1HdcMocsIndex;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns count of current Special MOCS values for MOCS Table programming at GMM boot
///
/// @param[in]     pLibContext: pGmmLibContext
/// @return     uint32_t  max special mocs index needed to program
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmCachePolicyGetMaxSpecialMocsIndex(void *pLibContext)
{
    GMM_LIB_CONTEXT * pGmmLibContext = (GMM_LIB_CONTEXT *)pLibContext;
    GMM_CACHE_POLICY *pCachePolicy   = pGmmLibContext->GetCachePolicyObj();
    return pCachePolicy->GetMaxSpecialMocsIndex();
}
