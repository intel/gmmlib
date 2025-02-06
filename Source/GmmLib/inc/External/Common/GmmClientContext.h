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

#pragma once
#include "GmmCommonExt.h"
#include "GmmInfo.h"

//////////////////////////////////////////////////////////////////
// Memory Allocators and DeAllocators for Gmm Create Objects
// This is needed for Clients who want to construct using their
// own memory allocators
//////////////////////////////////////////////////////////////////
typedef void* (GMM_STDCALL *PFN_ClientAllocationFunction)(
    void*                                       pUserData,
    uint32_t                                    size,
    uint32_t                                    alignment);

typedef void (GMM_STDCALL *PFN_ClientFreeFunction)(
    void*                                       pUserData,
    void*                                       pMemory);

typedef struct _GmmClientAllocationCallbacks_
{
    void*                                   pUserData;
    uint32_t                                size;
    uint32_t                                alignment;
    PFN_ClientAllocationFunction            pfnAllocation;
    PFN_ClientFreeFunction                  pfnFree;
} GmmClientAllocationCallbacks;

//===========================================================================
// typedef:
//      GMM_DEVICE_OPERATION
//
// Description:
//     Decribes compoents required for device operations.
//---------------------------------------------- ------------------------------
typedef struct _GMM_DEVICE_INFO_REC
{
    GMM_DEVICE_CALLBACKS_INT     *pDeviceCb;
}GMM_DEVICE_INFO;

#ifdef __cplusplus
#include "GmmMemAllocator.hpp"

/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmClientContext.h
/// @brief This file contains the functions and members of GmmClientContext that is
///       common for both Linux and Windows.
///
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains functions and members that are common between Linux and
    /// Windows implementation.  This class members will hold data that
    /// are specific to each client.
    /////////////////////////////////////////////////////////////////////////
    class Context;
    class GMM_LIB_API NON_PAGED_SECTION GmmClientContext : public GmmMemAllocator
    {
    protected:
        GMM_CLIENT                       ClientType;
        ///< Placeholders for storing UMD context. Actual UMD context that needs to be stored here is 
        union
        {
            void *pUmdAdapter;
            GMM_AIL_STRUCT *pClientContextAilFlags; //To store the UMD AIL flags. This is applicable for each client. Used to populate the corresponding LibContextAilFlags
        };
	GMM_UMD_CONTEXT                  *pGmmUmdContext;
        GMM_DEVICE_CALLBACKS_INT          DeviceCB;       //OS-specific defn: Will be used by Clients to send as input arguments.
        // Flag to indicate Device_callbacks received.
        uint8_t             IsDeviceCbReceived;
        Context *pGmmLibContext;

    public:
        /* Constructor */
        GmmClientContext(GMM_CLIENT ClientType);
        GmmClientContext(GMM_CLIENT ClientType, Context* pLibContext);

        /* Virtual destructor */
        virtual ~GmmClientContext();

        /* Inline functions */
        /////////////////////////////////////////////////////////////////////////////////////
        /// Returns the GMM_CLIENT Type that has created this ClientContext.
        /// @return     GMM_CLIENT
        /////////////////////////////////////////////////////////////////////////////////////
        GMM_INLINE_VIRTUAL GMM_INLINE_EXPORTED GMM_CLIENT GMM_STDCALL  GetClientType()
        {
            return (ClientType);
        }

	GMM_INLINE_VIRTUAL GMM_LIB_CONTEXT *GetLibContext()
        {
            return pGmmLibContext;
        }

        /* Function prototypes */
        /* CachePolicy Related Exported Functions from GMM Lib */
        GMM_VIRTUAL MEMORY_OBJECT_CONTROL_STATE         GMM_STDCALL CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage);
        GMM_VIRTUAL GMM_PTE_CACHE_CONTROL_BITS          GMM_STDCALL CachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage);
        GMM_VIRTUAL MEMORY_OBJECT_CONTROL_STATE         GMM_STDCALL CachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL CachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL GetSurfaceStateL1CachePolicy(GMM_RESOURCE_USAGE_TYPE Usage);
	GMM_VIRTUAL uint32_t                            GMM_STDCALL CachePolicyGetMaxMocsIndex();
        GMM_VIRTUAL uint32_t                            GMM_STDCALL CachePolicyGetMaxL1HdcMocsIndex();
        GMM_VIRTUAL uint32_t                            GMM_STDCALL CachePolicyGetMaxSpecialMocsIndex();
        GMM_VIRTUAL GMM_CACHE_POLICY_ELEMENT*           GMM_STDCALL GetCachePolicyUsage();
        GMM_VIRTUAL void                                GMM_STDCALL GetCacheSizes(GMM_CACHE_SIZES *pCacheSizes);
        GMM_VIRTUAL GMM_CACHE_POLICY_ELEMENT            GMM_STDCALL GetCachePolicyElement(GMM_RESOURCE_USAGE_TYPE Usage);
        GMM_VIRTUAL GMM_CACHE_POLICY_TBL_ELEMENT        GMM_STDCALL GetCachePolicyTlbElement(uint32_t  MocsIdx);
        GMM_VIRTUAL GMM_PLATFORM_INFO&                  GMM_STDCALL GetPlatformInfo();
        GMM_VIRTUAL const SKU_FEATURE_TABLE&            GMM_STDCALL GetSkuTable();
        GMM_VIRTUAL void                                GMM_STDCALL GetExtendedTextureAlign(uint32_t Mode, ALIGNMENT &UnitAlign);

        GMM_VIRTUAL uint8_t                             GMM_STDCALL IsPlanar(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL IsP0xx(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL IsUVPacked(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL IsCompressed(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL IsYUVPacked(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL GMM_SURFACESTATE_FORMAT             GMM_STDCALL GetSurfaceStateFormat(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL GetSurfaceStateCompressionFormat(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint8_t                             GMM_STDCALL GetMediaSurfaceStateCompressionFormat(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL GMM_E2ECOMP_FORMAT                  GMM_STDCALL GetLosslessCompressionType(GMM_RESOURCE_FORMAT Format);
        GMM_VIRTUAL uint64_t                            GMM_STDCALL GetInternalGpuVaRangeLimit();

        /* ResourceInfo Creation and Destroy API's */
        GMM_VIRTUAL GMM_RESOURCE_INFO* GMM_STDCALL       CreateResInfoObject(GMM_RESCREATE_PARAMS *pCreateParams);
        GMM_VIRTUAL GMM_RESOURCE_INFO* GMM_STDCALL       CopyResInfoObject(GMM_RESOURCE_INFO *pSrcRes);
        GMM_VIRTUAL void GMM_STDCALL                     ResMemcpy(void *pDst, void *pSrc);
        GMM_VIRTUAL void  GMM_STDCALL                    DestroyResInfoObject(GMM_RESOURCE_INFO    *pResInfo);
        /* PageTableMgr Creation and Destroy API's */
        GMM_VIRTUAL GMM_PAGETABLE_MGR* GMM_STDCALL      CreatePageTblMgrObject(GMM_DEVICE_CALLBACKS_INT* pDevCb, uint32_t TTFlags);
        GMM_VIRTUAL GMM_PAGETABLE_MGR* GMM_STDCALL      CreatePageTblMgrObject(uint32_t TTFlags);
        /* PageTableMgr Creation and Destroy API's */
        GMM_VIRTUAL void GMM_STDCALL                    DestroyPageTblMgrObject(GMM_PAGETABLE_MGR* pPageTableMgr);

        
#ifdef GMM_LIB_DLL
        /* ResourceInfo and PageTableMgr Create and Destroy APIs with Client provided Memory Allocators */
        GMM_VIRTUAL GMM_RESOURCE_INFO* GMM_STDCALL       CreateResInfoObject(GMM_RESCREATE_PARAMS *pCreateParams,
                                                                             GmmClientAllocationCallbacks *pAllocCbs);
        GMM_VIRTUAL void  GMM_STDCALL                    DestroyResInfoObject(GMM_RESOURCE_INFO    *pResInfo,
                                                                              GmmClientAllocationCallbacks *pAllocCbs);
#endif

        GMM_VIRTUAL GMM_PAGETABLE_MGR* GMM_STDCALL      CreatePageTblMgrObject(
                                                        GMM_DEVICE_CALLBACKS_INT* pDevCb,
                                                        uint32_t TTFlags,
                                                        GmmClientAllocationCallbacks* pAllocCbs);
        GMM_VIRTUAL GMM_PAGETABLE_MGR* GMM_STDCALL      CreatePageTblMgrObject(
                                                        uint32_t TTFlags,
                                                        GmmClientAllocationCallbacks* pAllocCbs);
        GMM_VIRTUAL void GMM_STDCALL                    DestroyPageTblMgrObject(GMM_PAGETABLE_MGR* pPageTableMgr,
                                                        GmmClientAllocationCallbacks* pAllocCbs);
        GMM_VIRTUAL GMM_STATUS GMM_STDCALL              GmmSetDeviceInfo(GMM_DEVICE_INFO* DeviceInfo);
        GMM_VIRTUAL GMM_RESOURCE_INFO* GMM_STDCALL      CreateCustomResInfoObject(GMM_RESCREATE_CUSTOM_PARAMS* pCreateParams);

#ifndef __GMM_KMD__
        GMM_VIRTUAL GMM_RESOURCE_INFO *GMM_STDCALL      CreateCustomResInfoObject_2(GMM_RESCREATE_CUSTOM_PARAMS_2 *pCreateParams);
#endif
	GMM_VIRTUAL uint32_t GMM_STDCALL CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable);
        GMM_VIRTUAL const SWIZZLE_DESCRIPTOR *GMM_STDCALL GetSwizzleDesc(EXTERNAL_SWIZZLE_NAME ExternalSwizzleName, EXTERNAL_RES_TYPE ResType, uint8_t bpe, bool isStdSwizzle = false);
	
	GMM_VIRTUAL void GMM_STDCALL            GmmSetAIL(GMM_AIL_STRUCT *pAilFlags);
	GMM_VIRTUAL const uint64_t *GMM_STDCALL GmmGetAIL();
    };
}

typedef GmmLib::GmmClientContext    GMM_CLIENT_CONTEXT;

#else
struct GmmClientContext;
typedef struct GmmClientContext GMM_CLIENT_CONTEXT;

#endif

#ifdef __cplusplus
extern "C" {
#endif

    /* ClientContext will be unique to each client */
    GMM_CLIENT_CONTEXT *GMM_STDCALL GmmCreateClientContextForAdapter(GMM_CLIENT ClientType, 
		                               ADAPTER_BDF sBdf, const void *_pSkuTable);
    void GMM_STDCALL GmmDeleteClientContext(GMM_CLIENT_CONTEXT *pGmmClientContext);

#if GMM_LIB_DLL
#ifdef _WIN32

    GMM_STATUS GMM_STDCALL GmmCreateLibContext(const PLATFORM           Platform,
                                               const SKU_FEATURE_TABLE *pSkuTable,
                                               const WA_TABLE *         pWaTable,
                                               const GT_SYSTEM_INFO *   pGtSysInfo,
                                               ADAPTER_BDF              sBdf);
#else

    GMM_STATUS GMM_STDCALL GmmCreateLibContext(const PLATFORM Platform,
                                               const void *   pSkuTable,
                                               const void *   pWaTable,
                                               const void *   pGtSysInfo,
                                               ADAPTER_BDF    sBdf,
                                               const GMM_CLIENT ClientType);
#endif

    void GMM_STDCALL GmmLibContextFree(ADAPTER_BDF sBdf);
    GMM_LIB_API_CONSTRUCTOR void GmmCreateMultiAdapterContext();
    GMM_LIB_API_DESTRUCTOR void GmmDestroyMultiAdapterContext();

#endif //GMM_LIB_DLL

#ifdef __cplusplus
}
#endif

