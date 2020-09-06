/*==============================================================================
Copyright(c) 2019 Intel Corporation

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

Description: This file contains the class definitions for GmmPageTableMgr
             for user-mode PageTable management, that is common for both 
             Linux and Windows.

============================================================================*/

#pragma once
#include "GmmHw.h"

#ifdef __linux__
#include <pthread.h>
#endif

typedef enum _GMM_ENGINE_TYPE
{
    ENGINE_TYPE_RCS = 0,            //RCS
    ENGINE_TYPE_COMPUTE = 1,        //Compute-CS
    ENGINE_TYPE_BCS,                //BLT
    ENGINE_TYPE_VD0,
    ENGINE_TYPE_VD1,
    ENGINE_TYPE_VE0
    //Add all engines supporting  AUX-TT
} GMM_ENGINE_TYPE;

typedef enum TT_Flags
{
    AUXTT = 1,              //Indicate TT request for AUX i.e. e2e compression
} TT_TYPE;


#if !(defined(__GMM_KMD__))
// Shared Structure for both Windows and Linux
typedef struct __GMM_DDI_UPDATEAUXTABLE
{
    GMM_UMD_SYNCCONTEXT * UmdContext;     // [in]  pointer to thread-specific data, specifying BBQHandle/Fence etc
    GMM_RESOURCE_INFO * BaseResInfo;      // [in]  GmmResourceInfo ptr for compressed resource
    GMM_RESOURCE_INFO * AuxResInfo;       // [in]  GmmResourceInfo ptr for separate Auxiliary resource
    GMM_GFX_ADDRESS BaseGpuVA;            // [in]  GPUVA where compressed resource has been mapped
    GMM_GFX_ADDRESS AuxSurfVA;            // [in]  GPUVA where separate Auxiliary resource has been mapped
    uint8_t Map;                          // [in]  specifies if resource is being mapped or unmapped
    uint8_t DoNotWait;                    // [in]  specifies if PageTable update be done on CPU (true) or GPU (false)
}GMM_DDI_UPDATEAUXTABLE;

#ifdef __cplusplus
#include "GmmMemAllocator.hpp"

namespace GmmLib
{
    class SyncInfoLin {                       //dummy class
    public:
        HANDLE BBQueueHandle;
        uint64_t BBFence;
        SyncInfoLin() : BBQueueHandle(NULL), BBFence(0) {}
        SyncInfoLin(HANDLE Handle, uint64_t Fence) : BBQueueHandle(Handle), BBFence(Fence) {}
    };
    typedef class SyncInfoLin  SyncInfo;


     //Forward class declarations
     class AuxTable;
     class GmmPageTablePool;
     typedef class GmmPageTablePool GMM_PAGETABLEPool;

     typedef enum POOL_TYPE_REC
     {
         POOL_TYPE_TRTTL1  = 0,
         POOL_TYPE_TRTTL2  = 1,
         POOL_TYPE_AUXTTL1 = 2,
         POOL_TYPE_AUXTTL2 = 3,
     } POOL_TYPE;

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Contains functions and members for GMM_PAGETABLE_MGR, clients must place its pointer in
    /// their device object. Clients call GmmLib to initialize the instance and use it for mapping
    /// /unmapping on GmmLib managed page tables (TR-TT for SparseResources, AUX-TT for compression)
    //////////////////////////////////////////////////////////////////////////////////////////////
    class GMM_LIB_API NON_PAGED_SECTION GmmPageTableMgr :
                     public GmmMemAllocator
    {
    private:
        GMM_ENGINE_TYPE EngType;             //PageTable managed @ device-level (specifies engine associated with the device)

        AuxTable* AuxTTObj;                  //Auxiliary Translation Table obj

        GMM_PAGETABLEPool *pPool;            //Common page table pool
        uint32_t NumNodePoolElements;
        GmmClientContext    *pClientContext;    ///< ClientContext of the client creating this Object

         //OS-specific defn
#if defined  __linux__
	pthread_mutex_t PoolLock;
#endif
    public:
        GMM_DEVICE_CALLBACKS DeviceCb;       //OS-specific defn: Will be used by Clients to send as input arguments for TR-TT APIs
        GMM_DEVICE_CALLBACKS_INT DeviceCbInt;       //OS-specific defn: Will be used internally GMM lib
        GMM_TRANSLATIONTABLE_CALLBACKS TTCb; //OS-specific defn
        HANDLE hCsr;  // OCL per-device command stream receiver handle for aubcapture
    public:
        GmmPageTableMgr();
        GmmPageTableMgr(GMM_DEVICE_CALLBACKS_INT *, uint32_t TTFlags, GmmClientContext  *pClientContextIn); // Allocates memory for indicate TT’s root-tables, initializes common node-pool
        
        
        //GMM_VIRTUAL GMM_GFX_ADDRESS GetTRL3TableAddr();
        GMM_VIRTUAL GMM_GFX_ADDRESS GetAuxL3TableAddr();

        //Update TT root table address in context-image
        GMM_VIRTUAL GMM_STATUS InitContextAuxTableRegister(HANDLE initialBBHandle, GMM_ENGINE_TYPE engType); //Clients call it to update Aux-Table pointer in context-image, engType reqd. if @ context level

        //Aux TT management API
        GMM_VIRTUAL GMM_STATUS UpdateAuxTable(const GMM_DDI_UPDATEAUXTABLE*);      //new API for updating Aux-Table to point to correct 16B-chunk
                                                                       //for given host page VA  when base/Aux surf is mapped/unmapped
        GMM_VIRTUAL void __ReleaseUnusedPool(GMM_UMD_SYNCCONTEXT *UmdContext);
        GMM_VIRTUAL GMM_PAGETABLEPool * __GetFreePoolNode(uint32_t * FreePoolNodeIdx, POOL_TYPE PoolType);


#if defined __linux__
        //returns number of BOs for indicated TTs = NumNodePoolElements+1 BOs for root table and pools
        GMM_VIRTUAL int GetNumOfPageTableBOs(uint8_t TTFlags);
        //returns BO* list for indicated TT in client allocated memory
        GMM_VIRTUAL int GetPageTableBOList(uint8_t TTFlags, void* BOList);
#endif

        //Destructor
        GMM_VIRTUAL ~GmmPageTableMgr();                         //Clean-up page table structures

        // Inline Functions
        /////////////////////////////////////////////////////////////////////////////////////
        /// Returns GmmClientContext associated with this PageTableMgr
        /// @return ::GmmClientContext
        /////////////////////////////////////////////////////////////////////////////////////
        GMM_INLINE_VIRTUAL GmmClientContext* GetClientContext()
        {
            return pClientContext;
        }

    private:
        GMM_PAGETABLEPool * __AllocateNodePool(uint32_t AddrAlignment, POOL_TYPE Type);

    };

    
}
#endif  // #ifdef __cplusplus
#endif
