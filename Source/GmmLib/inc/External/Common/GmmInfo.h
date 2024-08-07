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

// GmmConst.h needed for GMM_MAX_NUMBER_MOCS_INDEXES
#include "GmmConst.h"
#include "../../../Platform/GmmPlatforms.h"

#ifdef _WIN32
#define GMM_MUTEX_HANDLE    HANDLE
#else
#include <pthread.h>
#define GMM_MUTEX_HANDLE    pthread_mutex_t
#endif

// Set packing alignment
#pragma pack(push, 8)

//===========================================================================
// Forward Declaration: Defined in GmmResourceInfoExt.h
//---------------------------------------------------------------------------
struct GMM_CONTEXT_REC;
typedef struct GMM_CONTEXT_REC GMM_CONTEXT;

//===========================================================================
// typedef:
//      GMM_UMD_CONTEXT
//
// Description:
//      Struct defines user mode GMM context.
//----------------------------------------------------------------------------
typedef struct GMM_UMD_CONTEXT_REC
{
    uint32_t   TBD1;
    uint32_t   TBD2;
    uint32_t   TBD3;
} GMM_UMD_CONTEXT;


#if (!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
#include "GmmClientContext.h"
#endif

//===========================================================================
// typedef:
//      GMM_GLOBAL_CONTEXT
//
// Description:
//     Struct contains contexts for user mode and kernel mode. It also contains
//     platform information. This struct is initialized in user mode with
//     GmmInitGlobalContext().
//
//----------------------------------------------------------------------------

#ifdef __cplusplus
#include "GmmMemAllocator.hpp"

namespace GmmLib
{
    class NON_PAGED_SECTION Context : public GmmMemAllocator
    {
    private:
#if(!defined(__GMM_KMD__) && !GMM_LIB_DLL_MA)
    	static int32_t                   RefCount;
#endif
#if GMM_LIB_DLL_MA
        int32_t                          RefCount;
#endif //GMM_LIB_DLL_MA
	GMM_CLIENT                       ClientType;
        GMM_PLATFORM_INFO_CLASS*         pPlatformInfo;

        GMM_TEXTURE_CALC*                pTextureCalc;
        SKU_FEATURE_TABLE                SkuTable;
        WA_TABLE                         WaTable;
        GT_SYSTEM_INFO                   GtSysInfo;

    #if(defined(__GMM_KMD__))
        GMM_GTT_CONTEXT              GttContext;
    #endif

        GMM_CONTEXT                      *pGmmKmdContext;
        GMM_UMD_CONTEXT                  *pGmmUmdContext;
        void                             *pKmdHwDev;
        void                             *pUmdAdapter;

        GMM_CACHE_POLICY_ELEMENT         CachePolicy[GMM_RESOURCE_USAGE_MAX];
        GMM_CACHE_POLICY_TBL_ELEMENT     CachePolicyTbl[GMM_MAX_NUMBER_MOCS_INDEXES];
        GMM_CACHE_POLICY                 *pGmmCachePolicy;

    #if(defined(__GMM_KMD__))
        uint64_t           IA32ePATTable;
	GMM_PRIVATE_PAT     PrivatePATTable[GMM_NUM_PAT_ENTRIES];
	int32_t                PrivatePATTableMemoryType[GMM_NUM_GFX_PAT_TYPES];
    #endif

        // Padding Percentage limit on 64KB paged resource
        uint32_t               AllowedPaddingFor64KbPagesPercentage;
        uint64_t               InternalGpuVaMax;
        uint32_t               AllowedPaddingFor64KBTileSurf;

#ifdef GMM_LIB_DLL
        // Mutex Object used for synchronization of ProcessSingleton Context
        static GMM_MUTEX_HANDLE           SingletonContextSyncMutex;
#endif
        GMM_PRIVATE_PAT PrivatePATTable[GMM_NUM_PAT_ENTRIES];
        GMM_MUTEX_HANDLE SyncMutex;         // SyncMutex to protect access of Gmm UMD Lib process Singleton Context
    public :
        //Constructors and destructors
        Context();
        ~Context();

        GMM_STATUS GMM_STDCALL          LockSingletonContextSyncMutex();
        GMM_STATUS GMM_STDCALL          UnlockSingletonContextSyncMutex();

#if GMM_LIB_DLL_MA
        int32_t                         IncrementRefCount();
        int32_t                         DecrementRefCount();
#endif //GMM_LIB_DLL_MA

#if(!defined(__GMM_KMD__) && (!GMM_LIB_DLL_MA))
        static int32_t IncrementRefCount()  // Returns the current RefCount and then increment it
        {
#if defined(_WIN32)
            return(InterlockedIncrement((LONG *)&RefCount) - 1);  //InterLockedIncrement() returns incremented value
#elif defined(__linux__)
            return(__sync_fetch_and_add(&RefCount, 1));
#endif
        }

        static int32_t DecrementRefCount()
        {
            int CurrentValue = 0;
            int TargetValue = 0;
            do
            {
                CurrentValue = RefCount;
                if (CurrentValue > 0)
                {
                    TargetValue = CurrentValue - 1;
                }
                else
                {
                    break;
                }
#if defined(_WIN32)
            } while (!(InterlockedCompareExchange((LONG *)&RefCount, TargetValue, CurrentValue) == CurrentValue));
#elif defined(__linux__)
            } while (!__sync_bool_compare_and_swap(&RefCount, CurrentValue, TargetValue));
#endif

            return TargetValue;
        }
#endif
        GMM_STATUS GMM_STDCALL InitContext(
                                    const PLATFORM& Platform,
                                    const SKU_FEATURE_TABLE* pSkuTable,
                                    const WA_TABLE* pWaTable,
                                    const GT_SYSTEM_INFO* pGtSysInfo,
                                    GMM_CLIENT ClientType);

        void GMM_STDCALL DestroyContext();

#if (!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
        GMM_CLIENT_CONTEXT *pGmmGlobalClientContext;
#endif


        //Inline functions
        /////////////////////////////////////////////////////////////////////////
        /// Returns the client type e.g. DX, OCL, OGL etc.
        /// @return   client type
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CLIENT  GMM_STDCALL  GetClientType()
        {
            return (ClientType);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the PlatformInfo
        /// @return   PlatformInfo
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_PLATFORM_INFO&  GMM_STDCALL  GetPlatformInfo()
        {
            return (const_cast<GMM_PLATFORM_INFO&>(pPlatformInfo->GetData()));
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the cache policy element array ptr
        /// @return   const cache policy elment ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CACHE_POLICY_ELEMENT*  GMM_STDCALL GetCachePolicyUsage()
        {
            return (&CachePolicy[0]);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the cache policy tlb element array ptr
        /// @return   const cache policy elment ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CACHE_POLICY_TBL_ELEMENT*  GMM_STDCALL GetCachePolicyTlbElement()
        {
            return (&CachePolicyTbl[0]);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the texture calculation object ptr
        /// @return   TextureCalc ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_TEXTURE_CALC* GMM_STDCALL GetTextureCalc()
        {
            return (pTextureCalc);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the platform info class object ptr
        /// @return   PlatformInfo class object ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_PLATFORM_INFO_CLASS* GMM_STDCALL GetPlatformInfoObj()
        {
            return (pPlatformInfo);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the cache policy object ptr
        /// @return   TextureCalc ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CACHE_POLICY* GMM_STDCALL GetCachePolicyObj()
        {
            return (pGmmCachePolicy);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the sku table ptr
        /// @return   const SkuTable ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE const SKU_FEATURE_TABLE& GMM_STDCALL GetSkuTable()
        {
            return (SkuTable);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the Wa table ptr
        /// @return    WaTable ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE const WA_TABLE& GMM_STDCALL GetWaTable()
        {
            return (WaTable);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the GT system info ptr
        /// @return   const GtSysInfo ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE const GT_SYSTEM_INFO* GMM_STDCALL GetGtSysInfoPtr()
        {
            return (&GtSysInfo);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the GT system info ptr
        /// @return   GtSysInfo ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GT_SYSTEM_INFO* GMM_STDCALL GetGtSysInfo()
        {
            return (&GtSysInfo);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns Cache policy element for a given usage type
        /// @return   cache policy element
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CACHE_POLICY_ELEMENT GetCachePolicyElement(GMM_RESOURCE_USAGE_TYPE Usage)
        {
            return (CachePolicy[Usage]);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Get padding percentage limit for 64kb pages
        /////////////////////////////////////////////////////////////////////////
        uint32_t GetAllowedPaddingFor64KbPagesPercentage()
        {
            return (AllowedPaddingFor64KbPagesPercentage);
        }

        uint64_t& GetInternalGpuVaRangeLimit()
        {
            return InternalGpuVaMax;
        }
         /////////////////////////////////////////////////////////////////////////
        /// Get padding  limit for 64k pages
        /////////////////////////////////////////////////////////////////////////
        uint32_t GetAllowedPaddingFor64KBTileSurf()
        {
            return (AllowedPaddingFor64KBTileSurf);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Set padding  limit for 64k pages
        /////////////////////////////////////////////////////////////////////////

        GMM_INLINE void SetAllowedPaddingFor64KBTileSurf(uint32_t Value)
        {
            AllowedPaddingFor64KBTileSurf = Value;
        }

    #ifdef GMM_LIB_DLL
        ADAPTER_BDF             sBdf;  // Adpater's Bus, Device and Function info for which Gmm UMD Lib process Singleton Context is created
        #ifdef _WIN32
            // ProcessHeapVA Singleton HeapObj
            GMM_HEAP            *pHeapObj;
            uint32_t            ProcessHeapCounter;
            // ProcessVA Partition Address space
            GMM_GFX_PARTITIONING ProcessVA;
            uint32_t            ProcessVACounter;

            /////////////////////////////////////////////////////////////////////////
            /// Get ProcessHeapVA Singleton HeapObj
            /////////////////////////////////////////////////////////////////////////
            GMM_HEAP* GetSharedHeapObject();

            /////////////////////////////////////////////////////////////////////////
            /// Set ProcessHeapVA Singleton HeapObj
            /////////////////////////////////////////////////////////////////////////
            uint32_t SetSharedHeapObject(GMM_HEAP **pProcessHeapObj);

            /////////////////////////////////////////////////////////////////////////
            /// Get or Sets ProcessGfxPartition VA
            /////////////////////////////////////////////////////////////////////////
            void GetProcessGfxPartition(GMM_GFX_PARTITIONING* pProcessVA);

            /////////////////////////////////////////////////////////////////////////
            /// Get or Sets ProcessGfxPartition VA
            /////////////////////////////////////////////////////////////////////////
            void SetProcessGfxPartition(GMM_GFX_PARTITIONING* pProcessVA);

            /////////////////////////////////////////////////////////////////////////
            /// Destroy Sync Mutex created for Singleton Context access
            /////////////////////////////////////////////////////////////////////////
            static void   DestroySingletonContextSyncMutex()
            {
                if (SingletonContextSyncMutex)
                {
                    ::CloseHandle(SingletonContextSyncMutex);
                    SingletonContextSyncMutex = NULL;
                }
            }
            #if !GMM_LIB_DLL_MA
            /////////////////////////////////////////////////////////////////////////
            /// Acquire Sync Mutex for Singleton Context access
            /////////////////////////////////////////////////////////////////////////
            static GMM_STATUS   LockSingletonContextSyncMutex()
            {
                if (SingletonContextSyncMutex)
                {
                    while (WAIT_OBJECT_0 != ::WaitForSingleObject(SingletonContextSyncMutex, INFINITE));
                    return GMM_SUCCESS;
                }
                else
                {
                    return GMM_ERROR;
                }
            }

            /////////////////////////////////////////////////////////////////////////
            /// Release Sync Mutex for Singleton Context access
            /////////////////////////////////////////////////////////////////////////
            static GMM_STATUS   UnlockSingletonContextSyncMutex()
            {
                if (SingletonContextSyncMutex)
                {
                    ::ReleaseMutex(SingletonContextSyncMutex);
                    return GMM_SUCCESS;
                }
                else
                {
                    return GMM_ERROR;
                }
            }
            #endif //!GMM_LIB_DLL_MA

        #else // Non Win OS

            /////////////////////////////////////////////////////////////////////////
            /// Destroy Sync Mutex created for Singleton Context access
            /////////////////////////////////////////////////////////////////////////
            static void   DestroySingletonContextSyncMutex()
            {
                pthread_mutex_destroy(&SingletonContextSyncMutex);
            }
            #if !GMM_LIB_DLL_MA
            /////////////////////////////////////////////////////////////////////////
            /// Acquire Sync Mutex for Singleton Context access
            /////////////////////////////////////////////////////////////////////////
            static GMM_STATUS   LockSingletonContextSyncMutex()
            {
                pthread_mutex_lock(&SingletonContextSyncMutex);
                return GMM_SUCCESS;
            }

            /////////////////////////////////////////////////////////////////////////
            /// Release Sync Mutex for Singleton Context access
            /////////////////////////////////////////////////////////////////////////
            static GMM_STATUS   UnlockSingletonContextSyncMutex()
            {
                pthread_mutex_unlock(&SingletonContextSyncMutex);
                return GMM_SUCCESS;
            }
            #endif //!GMM_LIB_DLL_MA

        #endif // _WIN32

    #endif // GMM_LIB_DLL

        // KMD specific inline functions
    #ifdef __GMM_KMD__

        /////////////////////////////////////////////////////////////////////////
        /// Returns private PAT table memory type for a given PAT type
        /// @return   PAT Memory type
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE int32_t  GMM_STDCALL GetPrivatePATTableMemoryType(GMM_GFX_PAT_TYPE PatType)
        {
            return (PrivatePATTableMemoryType[PatType]);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns private PAT table memory type array ptr
        /// @return   PAT Memory type array ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE int32_t*  GMM_STDCALL GetPrivatePATTableMemoryType()
        {
            return (PrivatePATTableMemoryType);
        }


        /////////////////////////////////////////////////////////////////////////
        /// Returns private PAT table array ptr
        /// @return   PAT array ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_PRIVATE_PAT* GMM_STDCALL  GetPrivatePATTable()
        {
            return (PrivatePATTable);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns private PAT table entry for a given PAT index
        /// @return   PAT entry
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_PRIVATE_PAT GMM_STDCALL  GetPrivatePATEntry(uint32_t  PatIndex)
        {
            return (PrivatePATTable[PatIndex]);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns gmm kmd context ptr
        /// @return   GmmKmdContext ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CONTEXT* GetGmmKmdContext()
        {
            return (pGmmKmdContext);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Sets gmm kmd context ptr
        /// @return   GmmKmdContext ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE void SetGmmKmdContext(GMM_CONTEXT *pGmmKmdContext)
        {
            this->pGmmKmdContext = pGmmKmdContext;
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns gtt context ptr
        /// @return   GttContext ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_GTT_CONTEXT* GetGttContext()
        {
            return (&GttContext);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns Cache policy tbl element for a given usage type
        /// @return   cache policy tbl element
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_CACHE_POLICY_TBL_ELEMENT GetCachePolicyTblElement(GMM_RESOURCE_USAGE_TYPE Usage)
        {
            return (CachePolicyTbl[Usage]);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Resets  the sku Table after  GmmInitContext() could have changed them
        /// since original latching
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE void SetSkuTable(SKU_FEATURE_TABLE SkuTable)
        {
            this->SkuTable = SkuTable;
        }

        /////////////////////////////////////////////////////////////////////////
        /// Resets  the Wa Table after  GmmInitContext() could have changed them
        /// since original latching
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE void SetWaTable(WA_TABLE WaTable)
        {
            this->WaTable = WaTable;
        }

    #if(_DEBUG || _RELEASE_INTERNAL)

        /////////////////////////////////////////////////////////////////////////
        /// Returns the override platform info class object ptr
        /// @return   PlatformInfo class object ptr
        /////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_PLATFORM_INFO_CLASS* GMM_STDCALL GetOverridePlatformInfoObj()
        {
            return (Override.pPlatformInfo);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the override Platform info ptr to kmd
        /// @return   override PlatformInfo ref
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_PLATFORM_INFO& GMM_STDCALL GetOverridePlatformInfo()
        {
            return (const_cast<GMM_PLATFORM_INFO&>(Override.pPlatformInfo->GetData()));
        }

        /////////////////////////////////////////////////////////////////////////
        /// Set the override platform info calc ptr
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE void SetOverridePlatformInfoObj(GMM_PLATFORM_INFO_CLASS *pPlatformInfoObj)
        {
            Override.pPlatformInfo = pPlatformInfoObj;
        }

        /////////////////////////////////////////////////////////////////////////
        /// Returns the override Texture calc ptr to kmd
        /// @return   override Texture calc ptr
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE GMM_TEXTURE_CALC* GetOverrideTextureCalc()
        {
            return (Override.pTextureCalc);
        }

        /////////////////////////////////////////////////////////////////////////
        /// Set the override Texture calc ptr
        ////////////////////////////////////////////////////////////////////////
        GMM_INLINE void SetOverrideTextureCalc(GMM_TEXTURE_CALC *pTextureCalc)
        {
            Override.pTextureCalc = pTextureCalc;
        }
    #endif // (_DEBUG || _RELEASE_INTERNAL)

    #endif // __GMM_KMD__

    GMM_CACHE_POLICY* GMM_STDCALL CreateCachePolicyCommon();
    GMM_TEXTURE_CALC* GMM_STDCALL CreateTextureCalc(PLATFORM Platform, bool Override);
    GMM_PLATFORM_INFO_CLASS *GMM_STDCALL CreatePlatformInfo(PLATFORM Platform, bool Override);

    private: 
        void GMM_STDCALL OverrideSkuWa();
        
    public:
    /////////////////////////////////////////////////////////////////////////
    /// Returns private PAT table array ptr
    /// @return   PAT array ptr
    /////////////////////////////////////////////////////////////////////////
    GMM_INLINE GMM_PRIVATE_PAT *GMM_STDCALL GetPrivatePATTable()
    {
        return (&PrivatePATTable[0]);
    }
    
    };

// Max number of Multi-Adapters allowed in the system
#define MAX_NUM_ADAPTERS      9
//===========================================================================
// typedef:
//      _GMM_ADAPTER_INFO_
//
// Description:
//      Struct holds Adapter level information.
//----------------------------------------------------------------------------
typedef struct _GMM_ADAPTER_INFO_
{
    Context             *pGmmLibContext;                    // Gmm UMD Lib Context which is process Singleton
    _GMM_ADAPTER_INFO_  *pNext;                             // Linked List Next pointer to point to the Next Adapter node in the List

}GMM_ADAPTER_INFO;
    
////////////////////////////////////////////////////////////////////////////////////
/// Multi Adpater Context to hold data related to Multiple Adapters in the system
/// Contains functions and members that are needed to support Multi-Adapter.
///////////////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmMultiAdapterContext : public GmmMemAllocator
    {
    private:
        GMM_ADAPTER_INFO                AdapterInfo[MAX_NUM_ADAPTERS];// For Static Initialization of adapter.
        GMM_MUTEX_HANDLE                MAContextSyncMutex;         // SyncMutex to protect access of GmmMultiAdpaterContext
        uint32_t                        NumAdapters;
	void*                           pCpuReserveBase;
	uint64_t                        CpuReserveSize;
        GMM_ADAPTER_INFO                *pHeadNode;// For dynamic Initialization of adapter.
                                                   // The Multi-Adapter Initialization is done dynamiclly using a Linked list Vector
                                                   // pHeadNode points to the root node of the linked list and registers the first
                                                   // adapter received from UMD.
        // thread safe functions; these cannot be called within a LockMAContextSyncMutex block
        GMM_ADAPTER_INFO *              GetAdapterNode(ADAPTER_BDF sBdf);   // Replacement for GetAdapterIndex, now get adapter node from the linked list

        // Mutexes which protect the below thread unsafe functions
        GMM_STATUS GMM_STDCALL          LockMAContextSyncMutex();
        GMM_STATUS GMM_STDCALL          UnLockMAContextSyncMutex();

        // thread unsafe functions; these must be protected with LockMAContextSyncMutex
        GMM_ADAPTER_INFO *              GetAdapterNodeUnlocked(ADAPTER_BDF sBdf);
        GMM_ADAPTER_INFO *              AddAdapterNode();
        void                            RemoveAdapterNode(GMM_ADAPTER_INFO *pNode);

    public:
        //Constructors and destructors
        GmmMultiAdapterContext();
        ~GmmMultiAdapterContext();
        /* Function prototypes */
        /* Functions that update MultiAdapterContext members*/
        uint32_t GMM_STDCALL            GetAdapterIndex(ADAPTER_BDF sBdf);
        uint32_t GMM_STDCALL            GetNumAdapters();
        /* Functions that update AdapterInfo*/
        // thread safe functions; these cannot be called within a LockMAContextSyncMutex block
#if LHDM
        GMM_STATUS GMM_STDCALL          AddContext(const PLATFORM           Platform,
                                                   const SKU_FEATURE_TABLE *pSkuTable,
                                                   const WA_TABLE *         pWaTable,
                                                   const GT_SYSTEM_INFO *   pGtSysInfo,
                                                   ADAPTER_BDF              sBdf,
                                                   const char *             DeviceRegistryPath);
#else
        GMM_STATUS GMM_STDCALL          AddContext(const PLATFORM Platform,
                                                   const void *   pSkuTable,
                                                   const void *   pWaTable,
                                                   const void *   pGtSysInfo,
                                                   ADAPTER_BDF    sBdf,
						   const GMM_CLIENT ClientType);
#endif
        GMM_STATUS GMM_STDCALL          RemoveContext(ADAPTER_BDF sBdf);
        Context* GMM_STDCALL            GetAdapterLibContext(ADAPTER_BDF sBdf);
    }; // GmmMultiAdapterContext

} //namespace

typedef GmmLib::Context GMM_GLOBAL_CONTEXT, GMM_LIB_CONTEXT;
typedef GmmLib::GmmMultiAdapterContext GMM_MA_LIB_CONTEXT;

#else
struct GmmLibContext;
typedef struct GmmLibContext GMM_GLOBAL_CONTEXT, GMM_LIB_CONTEXT;
#endif


#ifdef __GMM_KMD__
void GMM_STDCALL GmmLinkKmdContextToGlobalInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_CONTEXT *pGmmKmdContext);
#endif /*__GMM_KMD__*/

//Declare all  GMM global context C interfaces.
#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
	
    const GMM_PLATFORM_INFO* GMM_STDCALL GmmGetPlatformInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    const GMM_CACHE_POLICY_ELEMENT* GmmGetCachePolicyUsage(GMM_GLOBAL_CONTEXT *pGmmLibContext);
          GMM_TEXTURE_CALC*         GmmGetTextureCalc(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    const SKU_FEATURE_TABLE*        GmmGetSkuTable(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    const WA_TABLE*                 GmmGetWaTable(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    const GT_SYSTEM_INFO*           GmmGetGtSysInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext);

#ifdef __GMM_KMD__ 
    int32_t             GmmGetPrivatePATTableMemoryType(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_GFX_PAT_TYPE PatType);
    GMM_CONTEXT*        GmmGetGmmKmdContext(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    GMM_GTT_CONTEXT*    GmmGetGttContext(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    GMM_CACHE_POLICY_TBL_ELEMENT GmmGetCachePolicyTblElement(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_RESOURCE_USAGE_TYPE Usage);
    GMM_CACHE_POLICY_ELEMENT GmmGetCachePolicyElement(GMM_GLOBAL_CONTEXT *pGmmLibContext, GMM_RESOURCE_USAGE_TYPE Usage);
    void                GmmSetSkuTable(GMM_GLOBAL_CONTEXT *pGmmLibContext, SKU_FEATURE_TABLE SkuTable);
    void                GmmSetWaTable(GMM_GLOBAL_CONTEXT *pGmmLibContext, WA_TABLE WaTable);
    GMM_PLATFORM_INFO*  GmmKmdGetPlatformInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext);
#if(_DEBUG || _RELEASE_INTERNAL)
    const GMM_PLATFORM_INFO* GmmGetOverridePlatformInfo(GMM_GLOBAL_CONTEXT *pGmmLibContext);
    GMM_TEXTURE_CALC*   GmmGetOverrideTextureCalc(GMM_GLOBAL_CONTEXT *pGmmLibContext);
#endif

#endif
    GMM_PRIVATE_PAT GmmGetPrivatePATEntry(GMM_LIB_CONTEXT *pGmmLibContext, uint32_t PatIndex);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

    #define GMM_OVERRIDE_PLATFORM_INFO(pTexInfo,pGmmLibContext)    (GmmGetPlatformInfo(pGmmLibContext))
    #define GMM_OVERRIDE_TEXTURE_CALC(pTexInfo,pGmmLibContext)     (GmmGetTextureCalc(pGmmLibContext))

#ifdef __GMM_KMD__
    #define GMM_OVERRIDE_EXPORTED_PLATFORM_INFO(pTexInfo)    GMM_OVERRIDE_PLATFORM_INFO(pTexInfo)
    #define GMM_IS_PLANAR(Format)                            GmmIsPlanar(Format)
#else
    #define GMM_OVERRIDE_EXPORTED_PLATFORM_INFO(pTexInfo,pGmmLibContext)    (&((GmmClientContext*)pClientContext)->GetPlatformInfo())
    #define GMM_IS_PLANAR(Format)                            (pClientContext->IsPlanar(Format))
#endif

#define GMM_IS_1MB_AUX_TILEALIGNEDPLANES(Platform, Surf) \
    ((GFX_GET_CURRENT_PRODUCT(Platform) >= IGFX_METEORLAKE) && Surf.OffsetInfo.PlaneXe_LPG.Is1MBAuxTAlignedPlanes)

// Reset packing alignment to project default
#pragma pack(pop)
