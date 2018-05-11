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

#ifdef __cplusplus
#include "GmmMemAllocator.hpp"
#include "GmmResourceInfoExt.h"

#if defined(_WIN32)
    #define GMM_DYNAMIC_MOCS_TABLE
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmCachePolicyCommon.h
/// @brief This file contains Gmm Cache Policy functions
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains the Gmm Cache Policy functions  common for Linux and Windows
    /// implementation.  This class is inherited by gen specific class so
    /// so clients shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class  NON_PAGED_SECTION GmmCachePolicyCommon :
        public GmmMemAllocator
    {
        private:
            static int32_t RefCount;

        public:
            GMM_CACHE_POLICY_ELEMENT *pCachePolicy;

            /* Constructor */
            GmmCachePolicyCommon(GMM_CACHE_POLICY_ELEMENT *pCachePolicy);
            static GMM_CACHE_POLICY* Create();

            static void IncrementRefCount()
            {
                #if defined(__GMM_KMD__) || _WIN32
                    InterlockedIncrement((LONG *)&RefCount);
                #elif defined(__linux__)
                    __sync_fetch_and_add(&RefCount, 1);
                #endif
                //TODO[Android]
            }

            static int32_t DecrementRefCount()
            {
                #if defined(__GMM_KMD__) || _WIN32
                    return(InterlockedDecrement((LONG *)&RefCount));
                #elif defined(__linux__)
                    return(__sync_sub_and_fetch(&RefCount, 1));
                #endif
                //TODO[Android]
            }

            /* Function prototypes */
            #if _WIN32
            void OverrideCachePolicy();
            #endif
            GMM_GFX_MEMORY_TYPE GetWantedMemoryType(GMM_CACHE_POLICY_ELEMENT CachePolicy);

            #define DEFINE_CP_ELEMENT(Usage, llc, ellc, l3, wt, age, aom, lecc_scc, l3_scc, scf, sso, cos, hdcl1, l3evict)   \
            do {                                                                                                             \
                    pCachePolicy[Usage].LLC         = (llc);                                                                 \
                    pCachePolicy[Usage].ELLC        = (ellc);                                                                \
                    pCachePolicy[Usage].L3          = (l3);                                                                  \
                    pCachePolicy[Usage].WT          = (wt);                                                                  \
                    pCachePolicy[Usage].AGE         = (age);                                                                 \
                    pCachePolicy[Usage].AOM         = (aom);                                                                 \
                    pCachePolicy[Usage].LeCC_SCC    = (lecc_scc);                                                            \
                    pCachePolicy[Usage].L3_SCC      = (l3_scc);                                                              \
                    pCachePolicy[Usage].SCF         = (scf);                                                                 \
                    pCachePolicy[Usage].SSO         = (sso);                                                                 \
                    pCachePolicy[Usage].CoS         = (cos);                                                                 \
                    pCachePolicy[Usage].HDCL1       = (hdcl1);                                                               \
                    pCachePolicy[Usage].L3Eviction  = (l3evict);                                                             \
                    pCachePolicy[Usage].Initialized = 1;                                                                     \
            } while(0)

            MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL CachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo);
            MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage);
            GMM_PTE_CACHE_CONTROL_BITS GMM_STDCALL CachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage);

            /* Virtual functions prototype*/
            virtual uint8_t GMM_STDCALL CachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage) = 0;
            virtual GMM_STATUS InitCachePolicy() = 0;
            virtual uint32_t GetMaxSpecialMocsIndex()
            {
                return 0;
            }
            virtual ~GmmCachePolicyCommon()
            {
            }
    };
}
#endif // #ifdef __cplusplus