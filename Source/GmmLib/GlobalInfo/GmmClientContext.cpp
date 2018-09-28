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
#include "External/Common/GmmClientContext.h"

#if !__GMM_KMD__ && _WIN32
#include "..\..\inc\common\gfxEscape.h"
#include "..\..\..\miniport\LHDM\inc\gmmEscape.h"
#include "Internal\Windows\GmmResourceInfoWinInt.h"
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// Constructor to zero initialize the GmmLib::GmmClientContext object and create
/// Utility class object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmClientContext::GmmClientContext(GMM_CLIENT ClientType)
    : ClientType(),
      pUmdAdapter(),
      pGmmUmdContext()
{
    this->ClientType = ClientType;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Destructor to free  GmmLib::GmmClientContext object memory
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmClientContext::~GmmClientContext()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning
/// MEMORY_OBJECT_CONTROL_STATE for a given Resource Usage Type
///
/// @param[in]  GMM_RESOURCE_INFO       : Pointer to ResInfo object
/// @param[in]  GMM_RESOURCE_USAGE_TYPE : Resource Usage Type
/// @return     MEMORY_OBJECT_CONTROL_STATE for the resource of "Usage" type.
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyGetMemoryObject(pResInfo, Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning
/// GMM_PTE_CACHE_CONTROL_BITS for a given Resource Usage Type
///
/// @param[in]  GMM_RESOURCE_USAGE_TYPE : Resource Usage Type
/// @return     GMM_PTE_CACHE_CONTROL_BITS for the resource of "Usage" type.
/////////////////////////////////////////////////////////////////////////////////////
GMM_PTE_CACHE_CONTROL_BITS GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyGetPteType(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning
/// MEMORY_OBJECT_CONTROL_STATE for a given ResInfo Object
///
/// @param[in]  GMM_RESOURCE_INFO       : Pointer to ResInfo object
/// @return     MEMORY_OBJECT_CONTROL_STATE for the ResInfo object
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyGetOriginalMemoryObject(pResInfo);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for checking if PTE is cached  for a
/// given resource usage type
///
/// @param[in]  GMM_RESOURCE_USAGE_TYPE : Resource Usage Type
/// @return     True if PTE cached, else false
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::CachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyObj()->CachePolicyIsUsagePTECached(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning Max MOCS index used
/// on a platform
///
/// @return     Max MOCS Index
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetMaxMocsIndex()
{
    GMM_CACHE_POLICY *          pCachePolicy = pGmmGlobalContext->GetCachePolicyObj();
    GmmLib::GmmGen9CachePolicy *ptr          = static_cast<GmmLib::GmmGen9CachePolicy *>(pCachePolicy);
    return ptr->CurrentMaxMocsIndex;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning Max L1 HDC MOCS index used
/// on a platform
///
/// @return     Max L1 HDC MOCS Index
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetMaxL1HdcMocsIndex()
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
uint32_t GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetMaxSpecialMocsIndex(void)
{
    GMM_CACHE_POLICY *pCachePolicy = pGmmGlobalContext->GetCachePolicyObj();
    return pCachePolicy->GetMaxSpecialMocsIndex();
}
/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_CACHE_POLICY_ELEMENT
/// Table defiend for a platform
///
/// @return     Const GMM_CACHE_POLICY_ELEMENT Table
/////////////////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_ELEMENT *GMM_STDCALL GmmLib::GmmClientContext::GetCachePolicyUsage()
{
    return (pGmmGlobalContext->GetCachePolicyUsage());
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for populating GMM_CACHE_SIZES
/// available on a platform
///
/// @return     In/Out GMM_CACHE_SIZES Populated Caches sizes
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::GetCacheSizes(GMM_CACHE_SIZES *pCacheSizes)
{
    return GmmGetCacheSizes(pCacheSizes);
}


/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_CACHE_POLICY_ELEMENT
/// for a given Resource Usage Type
///
/// @return     GMM_CACHE_POLICY_ELEMENT
/////////////////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_ELEMENT GMM_STDCALL GmmLib::GmmClientContext::GetCachePolicyElement(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmGlobalContext->GetCachePolicyElement(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_CACHE_POLICY_TBL_ELEMENT
/// for a given Mocs Index
///
/// @return     GMM_CACHE_POLICY_TBL_ELEMENT
/////////////////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_TBL_ELEMENT GMM_STDCALL GmmLib::GmmClientContext::GetCachePolicyTlbElement(uint32_t MocsIdx)
{
    return pGmmGlobalContext->GetCachePolicyTlbElement()[MocsIdx];
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_PLATFORM_INFO data
///
/// @return     GMM_PLATFORM_INFO&
/////////////////////////////////////////////////////////////////////////////////////
GMM_PLATFORM_INFO &GMM_STDCALL GmmLib::GmmClientContext::GetPlatformInfo()
{
    return pGmmGlobalContext->GetPlatformInfo();
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for getting Alignment info
///
/// @return     void
//////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::GetExtendedTextureAlign(uint32_t Mode, ALIGNMENT &UnitAlign)
{
    ALIGNMENT AlignInfo;
    pGmmGlobalContext->GetPlatformInfoObj()->ApplyExtendedTexAlign(Mode, AlignInfo);
    UnitAlign = AlignInfo;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning SKU_FEATURE_TABLE data
///
/// @return     SKU_FEATURE_TABLE&
/////////////////////////////////////////////////////////////////////////////////////
const SKU_FEATURE_TABLE &GMM_STDCALL GmmLib::GmmClientContext::GetSkuTable()
{
    return pGmmGlobalContext->GetSkuTable();
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning whether the given Resource
/// format is Planar
///
/// @return     True if the Given format is planar
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::IsPlanar(GMM_RESOURCE_FORMAT Format)
{
    return GmmIsPlanar(Format);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning whether the given Resource
/// format is P0xx
///
/// @return     True if the Given format is P0xx
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::IsP0xx(GMM_RESOURCE_FORMAT Format)
{
    return GmmIsP0xx(Format);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning whether the given Resource
/// format is UV Packed plane
///
/// @return     True if the Given format is UV packed
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::IsUVPacked(GMM_RESOURCE_FORMAT Format)
{
    return GmmIsUVPacked(Format);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning whether the given Resource
/// format is Compressed
///
/// @return     True if the Given format is Compressed
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::IsCompressed(GMM_RESOURCE_FORMAT Format)
{
    return (Format > GMM_FORMAT_INVALID) &&
           (Format < GMM_RESOURCE_FORMATS) &&
           pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].Compressed;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning whether the given Resource
/// format is YUV Packed plane
///
/// @return     True if the Given format is YUV packed
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::IsYUVPacked(GMM_RESOURCE_FORMAT Format)
{
    return GmmIsYUVPacked(Format);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning its GMM_SURFACESTATE_FORMAT
/// for the given equivalent GMM_RESOURCE_FORMAT type
///
/// @return     GMM_SURFACESTATE_FORMAT for the given format type
/////////////////////////////////////////////////////////////////////////////////////
GMM_SURFACESTATE_FORMAT GMM_STDCALL GmmLib::GmmClientContext::GetSurfaceStateFormat(GMM_RESOURCE_FORMAT Format)
{
    // ToDo: Remove the definition of GmmGetSurfaceStateFormat(Format)
    return ((Format > GMM_FORMAT_INVALID) &&
            (Format < GMM_RESOURCE_FORMATS)) ?
           pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].SurfaceStateFormat :
           GMM_SURFACESTATE_FORMAT_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class to return InternalGpuVaMax value
/// stored in pGmmGlobalContext
///
/// @return    GMM_SUCCESS
/////////////////////////////////////////////////////////////////////////////////////
uint64_t GMM_STDCALL GmmLib::GmmClientContext::GetInternalGpuVaRangeLimit()
{
    return pGmmGlobalContext->GetInternalGpuVaRangeLimit();
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of ResourceInfo Object .
/// @see        GmmLib::GmmResourceInfoCommon::Create()
///
/// @param[in] pCreateParams: Flags which specify what sort of resource to create
/// @return     Pointer to GmmResourceInfo class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmLib::GmmClientContext::CreateResInfoObject(GMM_RESCREATE_PARAMS *pCreateParams)
{
    GMM_RESOURCE_INFO *pRes             = NULL;
    GmmClientContext * pClientContextIn = NULL;

#if(!defined(GMM_UNIFIED_LIB))
    pClientContextIn = pGmmGlobalContext->pGmmGlobalClientContext;
#else
    pClientContextIn = this;
#endif

    // GMM_RESOURCE_INFO...
    if(pCreateParams->pPreallocatedResInfo)
    {
        pRes = new(pCreateParams->pPreallocatedResInfo) GmmLib::GmmResourceInfo(pClientContextIn); // Use preallocated memory as a class
        pCreateParams->Flags.Info.__PreallocatedResInfo =
        pRes->GetResFlags().Info.__PreallocatedResInfo = 1; // Set both in case we can die before copying over the flags.
    }
    else
    {
        if((pRes = new GMM_RESOURCE_INFO(pClientContextIn)) == NULL)
        {
            GMM_ASSERTDPF(0, "Allocation failed!");
            goto ERROR_CASE;
        }
    }

    if(pRes->Create(*pCreateParams) != GMM_SUCCESS)
    {
        goto ERROR_CASE;
    }

    return (pRes);

ERROR_CASE:
    if(pRes)
    {
        DestroyResInfoObject(pRes);
    }

    return (NULL);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of ResourceInfo Object from
/// already created Src ResInfo object
/// @see        GmmLib::GmmResourceInfoCommon::Create()
///
/// @param[in] pSrcRes: Existing ResInfoObj
/// @return     Pointer to GmmResourceInfo class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmLib::GmmClientContext::CopyResInfoObject(GMM_RESOURCE_INFO *pSrcRes)
{
    GMM_RESOURCE_INFO *pResCopy         = NULL;
    GmmClientContext * pClientContextIn = NULL;

#if(!defined(GMM_UNIFIED_LIB))
    pClientContextIn = pGmmGlobalContext->pGmmGlobalClientContext;
#else
    pClientContextIn = this;
#endif

    __GMM_ASSERTPTR(pSrcRes, NULL);

    pResCopy = new GMM_RESOURCE_INFO(pClientContextIn);
    if(!pResCopy)
    {
        GMM_ASSERTDPF(0, "Allocation failed.");
        return NULL;
    }

    *pResCopy = *pSrcRes;

    // We are allocating new class, flag must be false to avoid leak at DestroyResource
    pResCopy->GetResFlags().Info.__PreallocatedResInfo = 0;

    return (pResCopy);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for copy of ResourceInfo Object from
/// already created Src ResInfo object
/// @see        GmmLib::GmmResourceInfoCommon::Create()
///
/// @param[in] pDst: Pointer to memory when pSrc will be copied
/// @param[in] pSrc: Pointer to GmmResourceInfo class that needs to be copied
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::ResMemcpy(void *pDst, void *pSrc)
{
    GmmClientContext *pClientContextIn = NULL;

#if(!defined(GMM_UNIFIED_LIB))
    pClientContextIn = pGmmGlobalContext->pGmmGlobalClientContext;
#else
    pClientContextIn = this;
#endif

    GMM_RESOURCE_INFO *pResSrc = reinterpret_cast<GMM_RESOURCE_INFO *>(pSrc);
    // Init memory correctly, in case the pointer is a raw memory pointer
    GMM_RESOURCE_INFO *pResDst = new(pDst) GMM_RESOURCE_INFO(pClientContextIn);

    *pResDst = *pResSrc;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for Destroying ResInfoObject
///
/// @param[in] pResInfo: Pointer to ResInfoObject
/// @return     void.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::DestroyResInfoObject(GMM_RESOURCE_INFO *pResInfo)
{
    __GMM_ASSERTPTR(pResInfo, VOIDRETURN);

    if(pResInfo->GetResFlags().Info.__PreallocatedResInfo)
    {
        *pResInfo = GmmLib::GmmResourceInfo();
    }
    else
    {
        delete pResInfo;
        pResInfo = NULL;
    }
}

#ifdef GMM_LIB_DLL
/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of ResourceInfo Object .
/// @see        GmmLib::GmmResourceInfoCommon::Create()
///
/// @param[in] pCreateParams: Flags which specify what sort of resource to create
/// @return     Pointer to GmmResourceInfo class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmLib::GmmClientContext::CreateResInfoObject(GMM_RESCREATE_PARAMS *        pCreateParams,
                                                                             GmmClientAllocationCallbacks *pAllocCbs)
{
    if(!pAllocCbs || !pAllocCbs->pfnAllocation)
    {
        return CreateResInfoObject(pCreateParams);
    }
    else
    {
        GMM_RESOURCE_INFO *pRes   = NULL;
        void *             pConst = NULL;

        // GMM_RESOURCE_INFO...
        if(pCreateParams->pPreallocatedResInfo)
        {
            pRes = new(pCreateParams->pPreallocatedResInfo) GmmLib::GmmResourceInfo(this); // Use preallocated memory as a class
            pCreateParams->Flags.Info.__PreallocatedResInfo =
            pRes->GetResFlags().Info.__PreallocatedResInfo = 1; // Set both in case we can die before copying over the flags.
        }
        else
        {
            pConst = pAllocCbs->pfnAllocation(pAllocCbs->pUserData,
                                              sizeof(GMM_RESOURCE_INFO),
                                              alignof(GMM_RESOURCE_INFO));
            if(pConst == NULL)
            {
                GMM_ASSERTDPF(0, "Allocation failed!");
                goto ERROR_CASE;
            }
            else
            {
                pRes = new(pConst) GMM_RESOURCE_INFO(this);
            }
        }

        if(pRes->Create(*pCreateParams) != GMM_SUCCESS)
        {
            goto ERROR_CASE;
        }

        return (pRes);

    ERROR_CASE:
        if(pRes)
        {
            if(pAllocCbs->pfnFree)
            {
#ifdef _WIN32
                pRes->~GmmResourceInfoWin();
#else
                pRes->~GmmResourceInfoLin();
#endif

                pAllocCbs->pfnFree(pAllocCbs->pUserData, (void *)pRes);
            }
        }

        return (NULL);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for Destroying ResInfoObject
///
/// @param[in] pResInfo: Pointer to ResInfoObject
/// @return     void.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::DestroyResInfoObject(GMM_RESOURCE_INFO *           pResInfo,
                                                                GmmClientAllocationCallbacks *pAllocCbs)
{
    __GMM_ASSERTPTR(pResInfo, VOIDRETURN);

    if(!pAllocCbs || !pAllocCbs->pfnFree)
    {
        return DestroyResInfoObject(pResInfo);
    }
    else
    {
        if(pResInfo->GetResFlags().Info.__PreallocatedResInfo)
        {
            *pResInfo = GmmLib::GmmResourceInfo();
        }
        else
        {
#ifdef _WIN32
            pResInfo->~GmmResourceInfoWin();
#else
            pResInfo->~GmmResourceInfoLin();
#endif
            pAllocCbs->pfnFree(pAllocCbs->pUserData, (void *)pResInfo);
            pResInfo = NULL;
        }
    }
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// Gmm lib DLL exported C wrapper for creating GmmLib::GmmClientContext object
/// @see        Class GmmLib::GmmClientContext
///
/// @param[in]  ClientType : describles the UMD clients such as OCL, DX, OGL, Vulkan etc
///
/// @return     Pointer to GmmClientContext, if Context is created
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_CLIENT_CONTEXT *GMM_STDCALL GmmCreateClientContext(GMM_CLIENT ClientType)
{
    GMM_CLIENT_CONTEXT *pGmmClientContext = nullptr;

    pGmmClientContext = new GMM_CLIENT_CONTEXT(ClientType);

    return pGmmClientContext;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Gmm lib DLL exported C wrapper for deleting GmmLib::GmmClientContext object
/// @see        Class GmmLib::GmmClientContext
///
/// @param[in]  GMM_CLIENT_CONTEXT * : Pointer to ClientContext object
/// @return     Void
/////////////////////////////////////////////////////////////////////////////////////
extern "C" void GMM_STDCALL GmmDeleteClientContext(GMM_CLIENT_CONTEXT *pGmmClientContext)
{
    if(pGmmClientContext)
    {
        delete pGmmClientContext;
        pGmmClientContext = NULL;
    }
}
