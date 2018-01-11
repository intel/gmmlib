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
    class NON_PAGED_SECTION GmmClientContext : public GmmMemAllocator
    {
    protected:
        GMM_CLIENT                       ClientType;
        ///< Placeholders for storing UMD context. Actual UMD context that needs to be stored here is TBD
        void                             *pUmdAdapter;
        GMM_UMD_CONTEXT                  *pGmmUmdContext;

    public:
        /* Constructor */
        GmmClientContext(GMM_CLIENT ClientType);

        /* Virtual destructor */
        virtual ~GmmClientContext();

        /* Inline functions */
        /////////////////////////////////////////////////////////////////////////////////////
        /// Returns the GMM_CLIENT Type that has created this ClientContext.
        /// @return     GMM_CLIENT
        /////////////////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CLIENT GMM_STDCALL  GetClientType()
        {
            return (ClientType);
        }

        /* Function prototypes */
        /* CachePolicy Related Exported Functions from GMM Lib */
        MEMORY_OBJECT_CONTROL_STATE         GMM_STDCALL CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage);
        GMM_PTE_CACHE_CONTROL_BITS          GMM_STDCALL CachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage);
        MEMORY_OBJECT_CONTROL_STATE         GMM_STDCALL CachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo);
        uint8_t                             GMM_STDCALL CachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage);
        uint32_t                            GMM_STDCALL CachePolicyGetMaxMocsIndex();
        uint32_t                            GMM_STDCALL CachePolicyGetMaxL1HdcMocsIndex();
        uint32_t                            GMM_STDCALL CachePolicyGetMaxSpecialMocsIndex();
        const GMM_CACHE_POLICY_ELEMENT*     GMM_STDCALL GetCachePolicyUsage();
        void                                GMM_STDCALL GetCacheSizes(GMM_CACHE_SIZES *pCacheSizes);
        uint8_t                             GMM_STDCALL GetUseGlobalGtt(GMM_HW_COMMAND_STREAMER cs,
                                                                        GMM_HW_COMMAND Command,
                                                                        D3DDDI_PATCHLOCATIONLIST_DRIVERID *pDriverId);
        GMM_CACHE_POLICY_ELEMENT            GMM_STDCALL GetCachePolicyElement(GMM_RESOURCE_USAGE_TYPE Usage);
        GMM_CACHE_POLICY_TBL_ELEMENT        GMM_STDCALL GetCachePolicyTlbElement(uint32_t  MocsIdx);
        GMM_PLATFORM_INFO&                  GMM_STDCALL GetPlatformInfo();

        uint8_t                             GMM_STDCALL IsPlanar(GMM_RESOURCE_FORMAT Format);
        uint8_t                             GMM_STDCALL IsP0xx(GMM_RESOURCE_FORMAT Format);
        uint8_t                             GMM_STDCALL IsUVPacked(GMM_RESOURCE_FORMAT Format);
        uint8_t                             GMM_STDCALL IsCompressed(GMM_RESOURCE_FORMAT Format);
        uint8_t                             GMM_STDCALL IsYUVPacked(GMM_RESOURCE_FORMAT Format);

        /* ResourceInfo Creation and Destroy API's */
        GMM_RESOURCE_INFO* GMM_STDCALL       CreateResInfoObject(GMM_RESCREATE_PARAMS *pCreateParams);
        GMM_RESOURCE_INFO* GMM_STDCALL       CopyResInfoObject(GMM_RESOURCE_INFO *pSrcRes);
        void GMM_STDCALL                     ResMemcpy(void *pDst, void *pSrc);
        void  GMM_STDCALL                    DestroyResInfoObject(GMM_RESOURCE_INFO    *pResInfo);

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
    GMM_CLIENT_CONTEXT* GMM_STDCALL GmmCreateClientContext(GMM_CLIENT ClientType);
    void GMM_STDCALL GmmDeleteClientContext(GMM_CLIENT_CONTEXT *pGmmClientContext);

#ifdef __cplusplus
}
#endif

