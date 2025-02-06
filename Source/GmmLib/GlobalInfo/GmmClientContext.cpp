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

#if !__GMM_KMD__ && LHDM
#include "..\..\inc\common\gfxEscape.h"
#include "..\..\..\miniport\LHDM\inc\gmmEscape.h"
#include "Internal\Windows\GmmResourceInfoWinInt.h"
#include "../TranslationTable/GmmUmdTranslationTable.h"
#endif

extern GMM_MA_LIB_CONTEXT *pGmmMALibContext;

/////////////////////////////////////////////////////////////////////////////////////
/// Overloaded Constructor to zero initialize the GmmLib::GmmClientContext object
/// This Construtor takes pointer to GmmLibCOntext as input argumnet and initiaizes
/// ClientContext's GmmLibContext with this value
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmClientContext::GmmClientContext(GMM_CLIENT ClientType, Context *pLibContext)
    : ClientType(),
      pClientContextAilFlags(),
      pGmmUmdContext(),
      DeviceCB(),
      IsDeviceCbReceived(0)
{
    this->ClientType     = ClientType;
    this->pGmmLibContext = pLibContext;
    
    if (NULL != (pClientContextAilFlags = (GMM_AIL_STRUCT *)malloc(sizeof(GMM_AIL_STRUCT))))
    {
        memset(pClientContextAilFlags, 0, sizeof(GMM_AIL_STRUCT));
    }
    else
    {
        pClientContextAilFlags = NULL;
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/// Destructor to free  GmmLib::GmmClientContext object memory
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmClientContext::~GmmClientContext()
{
    pGmmLibContext = NULL;
    if (pClientContextAilFlags)
    {
        free(pClientContextAilFlags);
	pClientContextAilFlags = NULL;
    }    
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
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetMemoryObject(pResInfo, Usage);
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
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetPteType(Usage);
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
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetOriginalMemoryObject(pResInfo);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning
/// PAT Index for a given Resource Usage Type
///
/// @param[in]  GMM_RESOURCE_INFO       : Pointer to ResInfo object
/// @param[in]  GMM_RESOURCE_USAGE_TYPE : Resource Usage Type
/// @return     PAT index for the resource of "Usage" type.
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable)
{
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyGetPATIndex(pResInfo, Usage, pCompressionEnable, IsCpuCacheable);
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
    return pGmmLibContext->GetCachePolicyObj()->CachePolicyIsUsagePTECached(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class to return L1 Cache Control on DG2 for a
/// given resource usage type
///
/// @param[in]  GMM_RESOURCE_USAGE_TYPE : Resource Usage Type
/// @return     Value of L1 Cache control
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::GetSurfaceStateL1CachePolicy(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmLibContext->GetCachePolicyObj()->GetSurfaceStateL1CachePolicy(Usage);
}

////////////////////////////////////////////////////////////////////////////////////
/// Member function to get the AIL flags associated with Client Context
/// @param[in] None
/// @return    GMM_AIL_STRUCT associated with the ClientContext

const uint64_t* GMM_STDCALL GmmLib::GmmClientContext::GmmGetAIL()
{
    return (uint64_t*)(this->pClientContextAilFlags);
}

////////////////////////////////////////////////////////////////////////////////////
/// Member function to Set the AIL flags associated with Client Context
///
/// @param[in] GMM_AIL_STRUCT: Pointer to AIL struct
/// @return    void
void GMM_STDCALL GmmLib::GmmClientContext::GmmSetAIL(GMM_AIL_STRUCT* pAilFlags)
{
    //Cache the AilXe2CompressionRequest value
    bool IsClientAilXe2Compression = this->pClientContextAilFlags->AilDisableXe2CompressionRequest;

    memcpy(this->pClientContextAilFlags, pAilFlags, sizeof(GMM_AIL_STRUCT));

    // Update the Current ClientContext flags with whatever was cached earlier before copy
    this->pClientContextAilFlags->AilDisableXe2CompressionRequest = IsClientAilXe2Compression;

    return;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class to return Swizzle Descriptor
/// given Swizzle name , ResType and bpe
///
/// @param[in] EXTERNAL_SWIZZLE_NAME
/// @param[in] EXTERNAL_RES_TYPE
/// @param[in] bpe
/// @return  SWIZZLE_DESCRIPTOR*
/////////////////////////////////////////////////////////////////////////////////////
const SWIZZLE_DESCRIPTOR *GMM_STDCALL GmmLib::GmmClientContext::GetSwizzleDesc(EXTERNAL_SWIZZLE_NAME ExternalSwizzleName, EXTERNAL_RES_TYPE ResType, uint8_t bpe, bool isStdSwizzle)
{
    const SWIZZLE_DESCRIPTOR *pSwizzleDesc;
    pSwizzleDesc = NULL;
    /*#define SWITCH_SWIZZLE(Layout, res, bpe) \
        pSwizzleDesc = &Layout##_##res##bpe;*/

#define CASE_BPP(Layout, Tile, msaa, xD, bpe)       \
    case bpe:                                       \
        pSwizzleDesc = &Layout##_##Tile##msaa##bpe; \
        break;

#define SWITCH_SWIZZLE(Layout, Tile, msaa, bpe) \
    switch (bpe)                                \
    {                                           \
        CASE_BPP(Layout, Tile, msaa, xD, 8);    \
        CASE_BPP(Layout, Tile, msaa, xD, 16);   \
        CASE_BPP(Layout, Tile, msaa, xD, 32);   \
        CASE_BPP(Layout, Tile, msaa, xD, 64);   \
        CASE_BPP(Layout, Tile, msaa, xD, 128);  \
    }
#define SWIZZLE_DESC(pGmmLibContext, ExternalSwizzleName, ResType, bpe, pSwizzleDesc) \
    switch (ExternalSwizzleName)                                                      \
    {                                                                                 \
    case TILEX:                                                                       \
        pSwizzleDesc = &INTEL_TILE_X;                                                 \
        break;                                                                        \
    case TILEY:                                                                       \
        if (GmmGetSkuTable(pGmmLibContext)->FtrTileY)                                 \
            pSwizzleDesc = &INTEL_TILE_Y;                                             \
        else                                                                          \
            pSwizzleDesc = &INTEL_TILE_4;                                             \
        break;                                                                        \
    case TILEYS:                                                                      \
        if (GmmGetSkuTable(pGmmLibContext)->FtrTileY || isStdSwizzle)                 \
        {                                                                             \
            switch (ResType)                                                          \
            {                                                                         \
            case 0:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_YS, , , bpe);                               \
                break;                                                                \
            case 1:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_YS, 3D_, , bpe);                            \
                break;                                                                \
            case 2:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_YS, , MSAA2_, bpe);                         \
                break;                                                                \
            case 3:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_YS, , MSAA4_, bpe);                         \
                break;                                                                \
            case 4:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_YS, , MSAA8_, bpe);                         \
                break;                                                                \
            case 5:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_YS, , MSAA16_, bpe);                        \
                break;                                                                \
            }                                                                         \
        }                                                                             \
        else if (GmmGetSkuTable(pGmmLibContext)->FtrXe2PlusTiling)                    \
        {                                                                             \
            switch (ResType)                                                          \
            {                                                                         \
            case 0:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64, , , bpe);                               \
                break;                                                                \
            case 1:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64_V2, 3D_, , bpe);                         \
                break;                                                                \
            case 2:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64_V2, , MSAA2_, bpe);                      \
                break;                                                                \
            case 3:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64_V2, , MSAA4_, bpe);                      \
                break;                                                                \
            case 4:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64_V2, , MSAA8_, bpe);                      \
                break;                                                                \
            case 5:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64_V2, , MSAA16_, bpe);                     \
                break;                                                                \
            }                                                                         \
        }                                                                             \
        else                                                                          \
        {                                                                             \
            switch (ResType)                                                          \
            {                                                                         \
            case 0:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64, , , bpe);                               \
                break;                                                                \
            case 1:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64, 3D_, , bpe);                            \
                break;                                                                \
            case 2:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64, , MSAA2_, bpe);                         \
                break;                                                                \
            case 3:                                                                   \
            case 4:                                                                   \
            case 5:                                                                   \
                SWITCH_SWIZZLE(INTEL_TILE_64, , MSAA_, bpe);                          \
                break;                                                                \
            }                                                                         \
        }                                                                             \
    case TILEW:                                                                       \
    case TILEYF:                                                                      \
    default: break;                                                                   \
    }                                                                                 \

    SWIZZLE_DESC(pGmmLibContext, ExternalSwizzleName, ResType, bpe, pSwizzleDesc);
    return pSwizzleDesc;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning Max MOCS index used
/// on a platform
///
/// @return     Max MOCS Index
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmClientContext::CachePolicyGetMaxMocsIndex()
{
    GMM_CACHE_POLICY *          pCachePolicy = pGmmLibContext->GetCachePolicyObj();
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
    GMM_CACHE_POLICY *          pCachePolicy = pGmmLibContext->GetCachePolicyObj();
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
    GMM_CACHE_POLICY *pCachePolicy = pGmmLibContext->GetCachePolicyObj();
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
    return (pGmmLibContext->GetCachePolicyUsage());
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for populating GMM_CACHE_SIZES
/// available on a platform
///
/// @return     In/Out GMM_CACHE_SIZES Populated Caches sizes
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::GetCacheSizes(GMM_CACHE_SIZES *pCacheSizes)
{
    return GmmGetCacheSizes(pGmmLibContext, pCacheSizes);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_CACHE_POLICY_ELEMENT
/// for a given Resource Usage Type
///
/// @return     GMM_CACHE_POLICY_ELEMENT
/////////////////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_ELEMENT GMM_STDCALL GmmLib::GmmClientContext::GetCachePolicyElement(GMM_RESOURCE_USAGE_TYPE Usage)
{
    return pGmmLibContext->GetCachePolicyElement(Usage);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_CACHE_POLICY_TBL_ELEMENT
/// for a given Mocs Index
///
/// @return     GMM_CACHE_POLICY_TBL_ELEMENT
/////////////////////////////////////////////////////////////////////////////////////
GMM_CACHE_POLICY_TBL_ELEMENT GMM_STDCALL GmmLib::GmmClientContext::GetCachePolicyTlbElement(uint32_t MocsIdx)
{
    return pGmmLibContext->GetCachePolicyTlbElement()[MocsIdx];
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning GMM_PLATFORM_INFO data
///
/// @return     GMM_PLATFORM_INFO&
/////////////////////////////////////////////////////////////////////////////////////
GMM_PLATFORM_INFO &GMM_STDCALL GmmLib::GmmClientContext::GetPlatformInfo()
{
    return pGmmLibContext->GetPlatformInfo();
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for getting Alignment info
///
/// @return     void
//////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::GetExtendedTextureAlign(uint32_t Mode, ALIGNMENT &UnitAlign)
{
    ALIGNMENT AlignInfo;
    pGmmLibContext->GetPlatformInfoObj()->ApplyExtendedTexAlign(Mode, AlignInfo);
    UnitAlign = AlignInfo;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning SKU_FEATURE_TABLE data
///
/// @return     SKU_FEATURE_TABLE&
/////////////////////////////////////////////////////////////////////////////////////
const SKU_FEATURE_TABLE &GMM_STDCALL GmmLib::GmmClientContext::GetSkuTable()
{
    return pGmmLibContext->GetSkuTable();
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
           pGmmLibContext->GetPlatformInfo().FormatTable[Format].Compressed;
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
           pGmmLibContext->GetPlatformInfo().FormatTable[Format].SurfaceStateFormat :
           GMM_SURFACESTATE_FORMAT_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning
/// RENDER_SURFACE_STATE::CompressionFormat
///
/// @return     uint8_t
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::GetSurfaceStateCompressionFormat(GMM_RESOURCE_FORMAT Format)
{
    __GMM_ASSERT((Format > GMM_FORMAT_INVALID) && (Format < GMM_RESOURCE_FORMATS));
    return pGmmLibContext->GetPlatformInfo().FormatTable[Format].CompressionFormat.AuxL1eFormat;

}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning
/// MEDIA_SURFACE_STATE::CompressionFormat
///
/// @return     uint8_t
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmClientContext::GetMediaSurfaceStateCompressionFormat(GMM_RESOURCE_FORMAT Format)
{
    __GMM_ASSERT((Format > GMM_FORMAT_INVALID) && (Format < GMM_RESOURCE_FORMATS));

    return pGmmLibContext->GetPlatformInfoObj()->OverrideCompressionFormat(Format, (uint8_t)0x1);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for returning E2E compression format
///
/// @return     GMM_E2ECOMP_FORMAT
/////////////////////////////////////////////////////////////////////////////////////
GMM_E2ECOMP_FORMAT GMM_STDCALL GmmLib::GmmClientContext::GetLosslessCompressionType(GMM_RESOURCE_FORMAT Format)
{
    // ToDo: Remove the definition of GmmGetLosslessCompressionType(Format)
    __GMM_ASSERT((Format > GMM_FORMAT_INVALID) && (Format < GMM_RESOURCE_FORMATS));

    return pGmmLibContext->GetPlatformInfo().FormatTable[Format].CompressionFormat.AuxL1eFormat;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class to return InternalGpuVaMax value
/// stored in pGmmLibContext
///
/// @return    GMM_SUCCESS
/////////////////////////////////////////////////////////////////////////////////////
uint64_t GMM_STDCALL GmmLib::GmmClientContext::GetInternalGpuVaRangeLimit()
{
    return pGmmLibContext->GetInternalGpuVaRangeLimit();
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of Custiom ResourceInfo Object .
/// @see        GmmLib::GmmResourceInfoCommon::Create()
///
/// @param[in] pCreateParams: Flags which specify what sort of resource to create
/// @return     Pointer to GmmResourceInfo class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmLib::GmmClientContext::CreateCustomResInfoObject(GMM_RESCREATE_CUSTOM_PARAMS *pCreateParams)
{
    GMM_RESOURCE_INFO *pRes             = NULL;
    GmmClientContext * pClientContextIn = NULL;

    pClientContextIn = this;

    if((pRes = new GMM_RESOURCE_INFO(pClientContextIn)) == NULL)
    {
        GMM_ASSERTDPF(0, "Allocation failed!");
        goto ERROR_CASE;
    }

    if(pRes->CreateCustomRes(*pGmmLibContext, *pCreateParams) != GMM_SUCCESS)
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

#ifndef __GMM_KMD__
/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of Custiom ResourceInfo Object .
/// @see        GmmLib::GmmResourceInfoCommon::CreateCustomResInfoObject_2()
///
/// @param[in] pCreateParams: Flags which specify what sort of resource to create
/// @return     Pointer to GmmResourceInfo class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmLib::GmmClientContext::CreateCustomResInfoObject_2(GMM_RESCREATE_CUSTOM_PARAMS_2 *pCreateParams)
{
    GMM_RESOURCE_INFO *pRes             = NULL;
    GmmClientContext * pClientContextIn = NULL;

    pClientContextIn = this;

    if((pRes = new GMM_RESOURCE_INFO(pClientContextIn)) == NULL)
    {
        GMM_ASSERTDPF(0, "Allocation failed!");
        goto ERROR_CASE;
    }

    if(pRes->CreateCustomRes_2(*pGmmLibContext, *pCreateParams) != GMM_SUCCESS)
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
#endif

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
    pClientContextIn = pGmmLibContext->pGmmGlobalClientContext;
#else
    pClientContextIn = this;
#endif

    GMM_DPF_ENTER;

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

    if(pRes->Create(*pGmmLibContext, *pCreateParams) != GMM_SUCCESS)
    {
        goto ERROR_CASE;
    }

    GMM_DPF_EXIT;

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
    pClientContextIn = pGmmLibContext->pGmmGlobalClientContext;
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

    // Set the GmmLibContext for newly created DestResInfo object
    pResCopy->SetGmmLibContext(pGmmLibContext);

    *pResCopy = *pSrcRes;

    // Set the client type to the client for which this resinfo is created
    pResCopy->SetClientType(GetClientType());

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
    pClientContextIn = pGmmLibContext->pGmmGlobalClientContext;
#else
    pClientContextIn = this;
#endif

    GMM_RESOURCE_INFO *pResSrc = reinterpret_cast<GMM_RESOURCE_INFO *>(pSrc);
    // Init memory correctly, in case the pointer is a raw memory pointer
    GMM_RESOURCE_INFO *pResDst = new(pDst) GMM_RESOURCE_INFO(pClientContextIn);

    // Set the GmmLibContext for newly created DestResInfo object
    pResDst->SetGmmLibContext(pGmmLibContext);

    *pResDst = *pResSrc;

    // Set the client type to the client for which this resinfo is created
    pResDst->SetClientType(GetClientType());
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

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of PAgeTableMgr Object .
/// @see        GmmLib::GMM_PAGETABLE_MGR::GMM_PAGETABLE_MGR
///
/// @param[in] TTFags
/// @return     Pointer to GMM_PAGETABLE_MGR class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_PAGETABLE_MGR* GMM_STDCALL GmmLib::GmmClientContext::CreatePageTblMgrObject(uint32_t TTFlags)
{
    if (!IsDeviceCbReceived)
    {
        GMM_ASSERTDPF(0, "Device_callbacks not set");
        return NULL;
    }

    return CreatePageTblMgrObject(&DeviceCB, TTFlags);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of PAgeTableMgr Object .
/// @see        GmmLib::GMM_PAGETABLE_MGR::GMM_PAGETABLE_MGR
///
/// @param[in] pDevCb: Pointer to GMM_DEVICE_CALLBACKS_INT
/// @param[in] TTFags
/// @return     Pointer to GMM_PAGETABLE_MGR class.
//  move the code to new overloaded the API and remove this API once all clients are moved to new API.
/////////////////////////////////////////////////////////////////////////////////////
GMM_PAGETABLE_MGR* GMM_STDCALL GmmLib::GmmClientContext::CreatePageTblMgrObject(GMM_DEVICE_CALLBACKS_INT* pDevCb,
                                                                                uint32_t TTFlags)
{
    GMM_PAGETABLE_MGR* pPageTableMgr = NULL;

    pPageTableMgr = new GMM_PAGETABLE_MGR(pDevCb, TTFlags, this);

    return pPageTableMgr;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for destroy of PageTableMgr Object .
///
/// @param[in] pPageTableMgr: Pointer to GMM_PAGETABLE_MGR
/// @return     void.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::DestroyPageTblMgrObject(GMM_PAGETABLE_MGR* pPageTableMgr)
{
    if (pPageTableMgr)
    {
        delete pPageTableMgr;
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

        if(pRes->Create(*pGmmLibContext, *pCreateParams) != GMM_SUCCESS)
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
/// Member function of ClientContext class for creation of PAgeTableMgr Object .
/// @see        GmmLib::GMM_PAGETABLE_MGR::GMM_PAGETABLE_MGR
///
/// @param[in] TTFags
/// @return     Pointer to GMM_PAGETABLE_MGR class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_PAGETABLE_MGR* GMM_STDCALL GmmLib::GmmClientContext::CreatePageTblMgrObject(uint32_t TTFlags,
                                                         GmmClientAllocationCallbacks* pAllocCbs)
{
    if (!IsDeviceCbReceived)
    {
        GMM_ASSERTDPF(0, "Device_callbacks not set");
        return NULL;
    }
    return CreatePageTblMgrObject(
        &DeviceCB,
        TTFlags,
        pAllocCbs);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for creation of PAgeTableMgr Object .
/// @see        GmmLib::GMM_PAGETABLE_MGR::GMM_PAGETABLE_MGR
///
/// @param[in] pDevCb: Pointer to GMM_DEVICE_CALLBACKS_INT
/// @param[in] TTFags
/// @return     Pointer to GMM_PAGETABLE_MGR class.
/// move the code to new overloaded the API and remove this API once all clients are moved to new API.
/////////////////////////////////////////////////////////////////////////////////////
GMM_PAGETABLE_MGR* GMM_STDCALL GmmLib::GmmClientContext::CreatePageTblMgrObject(
                                                         GMM_DEVICE_CALLBACKS_INT* pDevCb,
                                                         uint32_t                      TTFlags,
                                                         GmmClientAllocationCallbacks* pAllocCbs)
{
    if (!pAllocCbs || !pAllocCbs->pfnAllocation)
    {
        return CreatePageTblMgrObject(
            pDevCb,
            TTFlags);
    }
    else
    {
        GMM_PAGETABLE_MGR* pPageTableMgr = NULL;
        return pPageTableMgr;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for destroy of PageTableMgr Object .
///
/// @param[in] pPageTableMgr: Pointer to GMM_PAGETABLE_MGR
/// @return     void.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::GmmClientContext::DestroyPageTblMgrObject(GMM_PAGETABLE_MGR* pPageTableMgr,
    GmmClientAllocationCallbacks* pAllocCbs)
{
    if (!pAllocCbs || !pAllocCbs->pfnFree)
    {
        return DestroyPageTblMgrObject(pPageTableMgr);
    }
}

////////////////////////////////////////////////////////////////////////////////////
/// Member function of ClientContext class for doing device specific operations.
/// Clients must call it before any Gfx resource (incl. svm)
/// is mapped, must happen before any use of GfxPartition, or PageTableMgr init.
/// @param[in]  DeviceInfo : Pointer to info related to Device Operations.
/// @return     GMM_STATUS.
//////////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmClientContext::GmmSetDeviceInfo(GMM_DEVICE_INFO* DeviceInfo)
{
    GMM_STATUS Status = GMM_SUCCESS;

    if (DeviceInfo == NULL || DeviceInfo->pDeviceCb == NULL)
    {
        return GMM_INVALIDPARAM;
    }

    DeviceCB = *(DeviceInfo->pDeviceCb);
    IsDeviceCbReceived = 1;
    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Gmm lib DLL C wrapper for creating GmmLib::GmmClientContext object
/// This C wrapper is used for Multi-Adapter scenarios to take in Adapter's BDF as
/// additional input argument to derive its correspodning GmmLibContext
///
/// @see        Class GmmLib::GmmClientContext
///
/// @param[in]  ClientType : describles the UMD clients such as OCL, DX, OGL, Vulkan etc
/// @param[in]  sBDF: Adapter's BDF info@param[in]  sBDF: Adapter's BDF info
/// @param[in]  _pSkuTable: SkuTable Pointer
///
/// @return     Pointer to GmmClientContext, if Context is created
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_CLIENT_CONTEXT *GMM_STDCALL GmmCreateClientContextForAdapter(GMM_CLIENT ClientType,
                                                                            ADAPTER_BDF sBdf,
                                                                            const void *_pSkuTable)
{
    GMM_CLIENT_CONTEXT *pGmmClientContext = nullptr;
    GMM_LIB_CONTEXT *   pLibContext       = pGmmMALibContext->GetAdapterLibContext(sBdf);
    SKU_FEATURE_TABLE *pSkuTable;

    if (pLibContext)
    {
        pGmmClientContext = new GMM_CLIENT_CONTEXT(ClientType, pLibContext);
	
	if (pGmmClientContext)
	{
	    pSkuTable = (SKU_FEATURE_TABLE *)_pSkuTable;
            if (GFX_GET_CURRENT_RENDERCORE(pLibContext->GetPlatformInfo().Platform) >= IGFX_XE2_HPG_CORE && pLibContext->GetSkuTable().FtrXe2Compression && !pSkuTable->FtrXe2Compression)
            {

                GMM_AIL_STRUCT *pClientAilFlags = (GMM_AIL_STRUCT *)pGmmClientContext->GmmGetAIL();

                pClientAilFlags->AilDisableXe2CompressionRequest = true;
            }
        }

    }
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
