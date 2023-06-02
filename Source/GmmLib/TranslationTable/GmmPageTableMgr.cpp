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

Description: UMD-TT manager (manages both TR-TT and AUX-TT in user mode space)

============================================================================*/

#include "Internal/Common/GmmLibInc.h"
#include "External/Common/GmmPageTableMgr.h"
#include "../TranslationTable/GmmUmdTranslationTable.h"
#include "External/Common/GmmClientContext.h"

#if defined(__linux__)
#include "Internal/Linux/GmmResourceInfoLinInt.h"
#endif

#define ENTER_CRITICAL_SECTION           \
    if(AuxTTObj)                         \
    {                                    \
        EnterCriticalSection(&PoolLock); \
    }

#define EXIT_CRITICAL_SECTION            \
    if(AuxTTObj)                         \
    {                                    \
        LeaveCriticalSection(&PoolLock); \
    }
extern GMM_MA_LIB_CONTEXT *pGmmMALibContext;
#if defined(__linux__)
GMM_STATUS GmmLib::__GmmDeviceAlloc(GmmClientContext *        pClientContext,
                                    GMM_DEVICE_CALLBACKS_INT *pDeviceCbInt,
                                    GMM_DEVICE_ALLOC *        pAlloc)
{
    GMM_CLIENT       ClientType;
    GMM_DDI_ALLOCATE Alloc = {0};
    int              err;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);

    __GMM_ASSERTPTR(GmmCheckForNullDevCbPfn(ClientType, pDeviceCbInt, GMM_DEV_CB_ALLOC), GMM_INVALIDPARAM);

    if(GmmCheckForNullDevCbPfn(ClientType, pDeviceCbInt, GMM_DEV_CB_ALLOC))
    {
        Alloc.size      = pAlloc->Size;
        Alloc.alignment = pAlloc->Alignment;

        err = GmmDeviceCallback(ClientType, pDeviceCbInt, &Alloc);
        if(err)
        {
            return GMM_OUT_OF_MEMORY;
        }

        pAlloc->GfxVA  = Alloc.gfxAddr;
        pAlloc->CPUVA  = (GMM_GFX_ADDRESS) Alloc.cpuAddr;
        pAlloc->Handle = (HANDLE)Alloc.bo;
    }

    return GMM_SUCCESS;
}

GMM_STATUS GmmLib::__GmmDeviceDealloc(GMM_CLIENT                ClientType,
                                      GMM_DEVICE_CALLBACKS_INT *DeviceCb,
                                      GMM_DEVICE_DEALLOC *      pDealloc,
                                      GmmClientContext *        pClientContext)

{
    GMM_DDI_DEALLOCATE DeAlloc = {0};
    int                err     = 0;

    __GMM_ASSERTPTR(GmmCheckForNullDevCbPfn(ClientType, DeviceCb, GMM_DEV_CB_DEALLOC), GMM_INVALIDPARAM);

    if(GmmCheckForNullDevCbPfn(ClientType, DeviceCb, GMM_DEV_CB_DEALLOC))
    {
        DeAlloc.bo = pDealloc->Handle;

        err = GmmDeviceCallback(ClientType, DeviceCb, &DeAlloc);
    }

    return (err == 0) ? GMM_SUCCESS : GMM_ERROR;
}
#endif 

//=============================================================================
//
// Function: __AllocateNodePool
//
// Desc: Allocates (always resident SVM) memory for new Pool node, and updates PageTableMgr object
//
// Parameters:
//      AddrAlignment: Pool allocation address alignment
//
// Returns:
//      S_OK on success,
//-----------------------------------------------------------------------------
GmmLib::GMM_PAGETABLEPool *GmmLib::GmmPageTableMgr::__AllocateNodePool(uint32_t AddrAlignment, GmmLib::POOL_TYPE Type)
{
    GMM_STATUS         Status      = GMM_SUCCESS;
    GMM_RESOURCE_INFO *pGmmResInfo = NULL;
    GMM_PAGETABLEPool *pTTPool     = NULL;
    HANDLE             PoolHnd     = 0;
    GMM_CLIENT         ClientType;
    GMM_DEVICE_ALLOC   Alloc = {0};

    ENTER_CRITICAL_SECTION

    //Allocate pool, sized PAGETABLE_POOL_MAX_NODES pages, assignable to TR/Aux L1/L2 tables
    //SVM allocation, always resident
    Alloc.Size      = PAGETABLE_POOL_SIZE;
    Alloc.Alignment = AddrAlignment;
    Alloc.hCsr      = hCsr;

    Status = __GmmDeviceAlloc(pClientContext, &DeviceCbInt, &Alloc);

    if(Status != GMM_SUCCESS)
    {
        __GMM_ASSERT(0);
        EXIT_CRITICAL_SECTION
        return NULL;
    }

    PoolHnd     = Alloc.Handle;
    pGmmResInfo = (GMM_RESOURCE_INFO *)Alloc.Priv;

    pTTPool = new GMM_PAGETABLEPool(PoolHnd, pGmmResInfo, Alloc.GfxVA, Alloc.CPUVA, Type);


    if(pTTPool)
    {
        if(pPool)
        {
            NumNodePoolElements++;
            if(Type == POOL_TYPE_TRTTL2) // TRTT-L2 not 1st node in Pool LinkedList, place it at beginning
            {
                pPool = pPool->InsertInListAtBegin(pTTPool);
            }
            else
            {
                pTTPool = pPool->InsertInList(pTTPool);
            }
        }
        else
        {
            NumNodePoolElements = 1;
            pPool               = pTTPool;
        }
    }
    else
    {
        __GMM_ASSERT(0);
        Status = GMM_OUT_OF_MEMORY;
    }

    EXIT_CRITICAL_SECTION
    return (Status == GMM_SUCCESS) ? pTTPool : NULL;
}

//=============================================================================
//
// Function: __ReleaseUnusedPool
//
// Desc: Frees up unused PageTablePools once residency limit is hit
//
// Parameters:
//      UmdContext: pointer to caller thread's context (containing BBHandle/Fence info)
//
//-----------------------------------------------------------------------------
void GmmLib::GmmPageTableMgr::__ReleaseUnusedPool(GMM_UMD_SYNCCONTEXT *UmdContext)
{
    GMM_STATUS                 Status         = GMM_SUCCESS;
    GMM_GFX_SIZE_T             PoolSizeToFree = {0};
    GMM_GFX_SIZE_T             FreedSize      = {0};
    GmmLib::GMM_PAGETABLEPool *Pool = NULL, *PrevPool = NULL;
    uint32_t                   i = 0;
    GMM_CLIENT                 ClientType;
    GMM_DEVICE_DEALLOC         Dealloc;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);

    ENTER_CRITICAL_SECTION
    if(pPool->__IsUnusedTRTTPoolOverLimit(&PoolSizeToFree))
    {
        for(i = 0; i < NumNodePoolElements && FreedSize < PoolSizeToFree; i++)
        {
            Pool = (PrevPool) ? PrevPool->GetNextPool() : pPool;

            if(Pool->IsPoolInUse(UmdContext ? SyncInfo(UmdContext->BBFenceObj, UmdContext->BBLastFence) : SyncInfo()))
            {
                PrevPool = Pool;
                continue;
            }

            if(GmmCheckForNullDevCbPfn(ClientType, &DeviceCbInt, GMM_DEV_CB_WAIT_FROM_CPU))
            {
                GMM_DDI_WAITFORSYNCHRONIZATIONOBJECTFROMCPU Wait = {0};
                Wait.bo                                          = Pool->GetPoolHandle();
                GmmDeviceCallback(ClientType, &DeviceCbInt, &Wait);
            }

            Dealloc.Handle = Pool->GetPoolHandle();
            Dealloc.GfxVA  = Pool->GetGfxAddress();
            Dealloc.Priv   = Pool->GetGmmResInfo();
            Dealloc.hCsr   = hCsr;

            Status = __GmmDeviceDealloc(ClientType, &DeviceCbInt, &Dealloc, pClientContext);

            __GMM_ASSERT(GMM_SUCCESS == Status);

            if(PrevPool)
            {
                PrevPool->GetNextPool() = Pool->GetNextPool();
            }
            else
            {
                pPool = Pool->GetNextPool();
            }
            delete Pool;
            FreedSize += PAGETABLE_POOL_SIZE;
        }
    }
    EXIT_CRITICAL_SECTION
}

//=============================================================================
//
// Function: __GetFreePoolNode
//
// Desc: Finds free node within existing PageTablePool(s), if no such node found,
//       allocates new PageTablePool. Caller should update Pool Node usage
//
// Parameters:
//      FreePoolNodeIdx: pointer to return Pool's free Node index
//      PoolType: AuxTT_L1/L2 pool
//
// Returns:
//     PageTablePool element and FreePoolNodeIdx that should be used for L2/L1 assignment
//     NULL, if no free node exists and new pool allocation failed
//-----------------------------------------------------------------------------
GmmLib::GMM_PAGETABLEPool *GmmLib::GmmPageTableMgr::__GetFreePoolNode(uint32_t *FreePoolNodeIdx, POOL_TYPE PoolType)
{
    uint32_t PoolNode = -1, i = 0, j = 0, DWdivisor = 1, IdxMultiplier = 1;
    bool     PoolNodeFound = false, TRTTPool = false;

    ENTER_CRITICAL_SECTION
    GmmLib::GMM_PAGETABLEPool *Pool = pPool;

    Pool = (PoolType == POOL_TYPE_TRTTL2) ? Pool : //1st pool reserved for TRTT-L2, since TRTT-L2 pruning not supported yet,
           (Pool ? Pool->GetNextPool() : NULL);    //other pools can be TR-L1/Aux-L1/Aux-L2 (and support dynamic pruning)
    TRTTPool      = (PoolType == POOL_TYPE_TRTTL2 || PoolType == POOL_TYPE_TRTTL1) ? true : false;
    DWdivisor     = TRTTPool ? 8 * sizeof(uint32_t) : (PoolType == POOL_TYPE_AUXTTL2) ? 8 * sizeof(uint32_t) * AUX_L2TABLE_SIZE_IN_POOLNODES : 8 * sizeof(uint32_t) * AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetLibContext());
    IdxMultiplier = TRTTPool ? 1 : (PoolType == POOL_TYPE_AUXTTL2) ? AUX_L2TABLE_SIZE_IN_POOLNODES : AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetLibContext());
    //Scan existing PageTablePools for free pool node
    for(i = (PoolType == POOL_TYPE_TRTTL2) ? 0 : 1; Pool && i < NumNodePoolElements; i++)
    {
        if(Pool->GetNumFreeNode() > 0 && Pool->GetPoolType() == PoolType)
        {
            PoolNodeFound    = true;
            *FreePoolNodeIdx = 0;
            for(; j < PAGETABLE_POOL_MAX_NODES / DWdivisor; j++)
            {
                if(_BitScanForward((uint32_t *)&PoolNode, (uint32_t) ~(Pool->GetNodeUsageAtIndex(j)))) // Get LSB that has value 0
                {
                    *FreePoolNodeIdx += PoolNode * IdxMultiplier;
                    PoolNodeFound = true;
                    break;
                }
                PoolNodeFound = false;
                *FreePoolNodeIdx += DWdivisor; //DWORD size in bits
            }
        }
        if(PoolNodeFound)
        {
            __GMM_ASSERT(Pool->GetPoolType() == PoolType);
            EXIT_CRITICAL_SECTION
            return Pool;
        }
        Pool = Pool->GetNextPool();
    }

    //No free pool node, allocate new
    if(!PoolNodeFound)
    {
        GMM_PAGETABLEPool *Pool = NULL;
        if(Pool = __AllocateNodePool(IdxMultiplier * PAGE_SIZE, PoolType))
        {
            __GMM_ASSERT(Pool->GetPoolType() == PoolType);

            *FreePoolNodeIdx = 0;
            EXIT_CRITICAL_SECTION
            return Pool;
        }
    }

    EXIT_CRITICAL_SECTION
    return NULL;
}



/**********************************************************************************
** Class GmmPageTableMgr functions **
***********************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
/// Instantiates GmmPageTableMgr, allocating memory for root-tables, copies provided
/// device-callback function pointers
///
/// @param[in]  DeviceCb: pointer sharing device-callback function pointers
/// @param[in]  TTFlags: Flags specifying which PageTables are required by client
/// @return     GmmPageTableMgr*
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::GmmPageTableMgr(GMM_DEVICE_CALLBACKS_INT *DeviceCB, uint32_t TTFlags, GmmClientContext *pClientContextIn)
    : GmmPageTableMgr()
{
    GMM_PAGETABLE_MGR *ptr    = NULL;
    GMM_STATUS         status = GMM_SUCCESS;
    GMM_CLIENT         ClientType;

    if(pClientContextIn)
    {
        ClientType = pClientContextIn->GetClientType();
    }
    else
    {
        goto ERROR_CASE;
    }

    // this is needed if there is an error case and destructor gets called on ptr
    this->pClientContext = pClientContextIn;

    // Currently coping the code below to GMMOldAPi.cpp for backward compatible.
    // Any changes here should be copied there.
    //Initialize PageTableMgr further, only if PageTable creation succeeded
    try
    {
        ptr                 = new GmmPageTableMgr();
        ptr->pClientContext = pClientContextIn;
        memcpy(&ptr->DeviceCbInt, DeviceCB, sizeof(GMM_DEVICE_CALLBACKS_INT));

        if(pClientContextIn->GetSkuTable().FtrE2ECompression &&
           !pClientContextIn->GetSkuTable().FtrFlatPhysCCS)
        {
            __GMM_ASSERT(TTFlags & AUXTT); //Aux-TT is mandatory
            ptr->AuxTTObj = new AuxTable(pClientContext);
            if(!ptr->AuxTTObj)
            {
                goto ERROR_CASE;
            }
            ptr->AuxTTObj->PageTableMgr   = ptr;
            ptr->AuxTTObj->pClientContext = pClientContextIn;
            status                        = ptr->AuxTTObj->AllocateL3Table(8 * PAGE_SIZE, 8 * PAGE_SIZE);

            if(status != GMM_SUCCESS)
            {
                InitializeCriticalSection(&(ptr->PoolLock));
                goto ERROR_CASE;
            }
        }
    }

    catch(...)
    {
        __GMM_ASSERT(false);
        if(ptr && (AuxTTObj))
        {
            InitializeCriticalSection(&(ptr->PoolLock));
        }
        goto ERROR_CASE;
    }

    if(status == GMM_SUCCESS && !(AuxTTObj))
    {
        if(ptr->AuxTTObj)
        {
            ptr->AuxTTObj->PageTableMgr = this;
        }
        *this = *ptr;
        //Don't initialize PoolLock until any of AuxTable object created
        if(ptr->AuxTTObj )
        {
            InitializeCriticalSection(&PoolLock);

        }
        //Delete temporary ptr, but don't release allocated PageTable Obj.
        ptr->AuxTTObj = NULL;
    }

ERROR_CASE:
    delete ptr;
    ptr = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns Root-table address for Aux-table
///
/// @return     GMM_GFX_ADDRESS if Aux-Table was created; NULL otherwise
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GmmLib::GmmPageTableMgr::GetAuxL3TableAddr()
{
    return AuxTTObj ? AuxTTObj->GetL3Address() : 0ULL;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Queues commands to initialize Aux-Table registers in the HW context image
///
/// @param[in]  initialBBHandle: pointer to BatchBuffer for queuing commands
/// @param[in]  engType: specifes engine on which the context would run
/// @return     GMM_SUCCESS if queuing succeeded; GMM_ERROR otherwise
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmPageTableMgr::InitContextAuxTableRegister(HANDLE CmdQHandle, GMM_ENGINE_TYPE engType)
{
    GMM_GFX_ADDRESS MaskedL3GfxAddress = 0ULL;
    GMM_UNREFERENCED_PARAMETER(engType);

    //Check FtrE2ECompression = 1
    if(GetLibContext()->GetSkuTable().FtrE2ECompression && AuxTTObj != NULL)
    {
        EnterCriticalSection(&AuxTTObj->TTLock);
        if(CmdQHandle)
        {
            //engType = ENGINE_TYPE_RCS;            //use correct offset based on engType (once per-eng offsets known)
            uint64_t RegOffset = 0, L3AdrReg = 0;
            GET_L3ADROFFSET(0, L3AdrReg, GetLibContext());

            RegOffset = (L3AdrReg + sizeof(uint32_t));
            RegOffset = L3AdrReg | (RegOffset << 0x20);

            MaskedL3GfxAddress = AuxTTObj->GetL3Address();

            //TTCb.pfPrologTranslationTable(CmdQHandle);   //MI_FLUSH, TLBInv not required since its called during context-init

            TTCb.pfWriteL3Adr(CmdQHandle, MaskedL3GfxAddress, RegOffset);

            GMM_DPF(GFXDBG_NORMAL, "AuxTT Map Address: GPUVA=0x%016llX\n", MaskedL3GfxAddress);

            //TTCb.pfEpilogTranslationTable(CmdQHandle, 0);

            AuxTTObj->GetRegisterStatus() = 0;
        }
        else
        {
            __GMM_ASSERT(false);
            LeaveCriticalSection(&AuxTTObj->TTLock);
            return GMM_INVALIDPARAM;
        }
        LeaveCriticalSection(&AuxTTObj->TTLock);
    }
    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Updates the Aux-PageTables, for given base resource, with appropriate mappings
///
/// @param[in]  Details of AuxTable update request
/// @return     GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmPageTableMgr::UpdateAuxTable(const GMM_DDI_UPDATEAUXTABLE *UpdateReq)
{
    if(GetAuxL3TableAddr() == 0ULL)
    {
        GMM_ASSERTDPF(0, "Invalid AuxTable update request, AuxTable is not initialized");
        return GMM_INVALIDPARAM;
    }

    if(!((UpdateReq->BaseResInfo->GetResFlags().Info.RenderCompressed ||
          UpdateReq->BaseResInfo->GetResFlags().Info.MediaCompressed) &&
         ((!UpdateReq->AuxResInfo && UpdateReq->BaseResInfo->GetResFlags().Gpu.UnifiedAuxSurface) ||
          (UpdateReq->AuxResInfo && UpdateReq->AuxResInfo->GetResFlags().Gpu.CCS))))
    /*(UpdateReq->BaseResInfo->GetResFlags().Gpu.TiledResource ||
        UpdateReq->BaseResInfo->GetResFlags().Gpu.Depth) */
    //Allow Separate Aux for Depth/TR/MSAA/others?
    {
        GMM_ASSERTDPF(0, "Invalid AuxTable update request");
        return GMM_INVALIDPARAM;
    }

    if(UpdateReq->Map && !(!UpdateReq->BaseResInfo->GetResFlags().Gpu.TiledResource || (UpdateReq->BaseResInfo->GetResFlags().Gpu.TiledResource && UpdateReq->UmdContext && UpdateReq->UmdContext->pCommandQueueHandle)))
    {
        //GMM_DPF_CRITICAL("TiledResources must Gpu-update AuxTable, proceeding with CPU-update...");

        //Allowing CPU-update if requested so..
        if(!UpdateReq->DoNotWait)
        {
            return GMM_INVALIDPARAM;
        }
    }

    ENTER_CRITICAL_SECTION

    if(UpdateReq->Map)
    {
        //Get AuxL1e data (other than CCS-adr) from main surface
        uint64_t   PartialL1e = AuxTTObj->CreateAuxL1Data(UpdateReq->BaseResInfo).Value;
        GMM_STATUS Status     = GMM_SUCCESS;

        if(UpdateReq->BaseResInfo->GetResFlags().Gpu.TiledResource)
        {
            //Aux-TT is sparsely updated, for TRs, upon change in mapping state ie
            // null->non-null must be mapped
            // non-null->null        invalidated on AuxTT
            uint8_t CpuUpdate = UpdateReq->DoNotWait || !(UpdateReq->UmdContext && UpdateReq->UmdContext->pCommandQueueHandle);

            GMM_GFX_ADDRESS AuxVA = UpdateReq->AuxSurfVA;
            if(UpdateReq->BaseResInfo->GetResFlags().Gpu.UnifiedAuxSurface)
            {
                GMM_UNIFIED_AUX_TYPE AuxType = GMM_AUX_CCS;
                AuxType                      = (UpdateReq->BaseResInfo->GetResFlags().Gpu.Depth && UpdateReq->BaseResInfo->GetResFlags().Gpu.CCS) ? GMM_AUX_ZCS : AuxType;
                AuxVA                        = UpdateReq->BaseGpuVA + GmmResGetAuxSurfaceOffset64(UpdateReq->BaseResInfo, AuxType);
            }

        }
        else
        {
            GMM_GFX_ADDRESS AuxVA      = {0};
            GMM_GFX_ADDRESS UVAuxVA    = {0};
            GMM_GFX_SIZE_T  YPlaneSize = 0;
            uint32_t        MaxPlanes  = 1;

            if(!UpdateReq->AuxResInfo && UpdateReq->BaseResInfo->GetResFlags().Gpu.UnifiedAuxSurface)
            {
                GMM_UNIFIED_AUX_TYPE AuxType = GMM_AUX_CCS;
                AuxType                      = (UpdateReq->BaseResInfo->GetResFlags().Gpu.Depth &&
                           UpdateReq->BaseResInfo->GetResFlags().Gpu.CCS) ?
                          GMM_AUX_ZCS :
                          AuxType;

                AuxVA = UpdateReq->BaseGpuVA + GmmResGetAuxSurfaceOffset64(UpdateReq->BaseResInfo, AuxType);

                //For UV Packed, Gen12 e2e compr supported formats have 2 planes per surface
                //Each has distinct Aux surface, Y-plane/UV-plane must be mapped to respective Y/UV Aux surface
                if(GmmIsPlanar(UpdateReq->BaseResInfo->GetResourceFormat()))
                {
                    GMM_REQ_OFFSET_INFO ReqInfo = {0};
                    ReqInfo.Plane               = GMM_PLANE_U;
                    ReqInfo.ReqRender           = 1;

                    MaxPlanes = 2;
                    UpdateReq->BaseResInfo->GetOffset(ReqInfo);
                    YPlaneSize = ReqInfo.Render.Offset64;

                    UVAuxVA = UpdateReq->BaseGpuVA + GmmResGetAuxSurfaceOffset64(UpdateReq->BaseResInfo, GMM_AUX_UV_CCS);
                }
            }

            //Per-plane Aux-TT map called with per-plane base/Aux address/size
            for(uint32_t i = 0; i < MaxPlanes; i++)
            {
                GMM_GFX_SIZE_T SurfSize = (MaxPlanes > 1 && UpdateReq->BaseResInfo->GetArraySize() > 1) ?
                                          (UpdateReq->BaseResInfo->GetQPitchPlanar(GMM_NO_PLANE) * UpdateReq->BaseResInfo->GetRenderPitch()) :
                                          UpdateReq->BaseResInfo->GetSizeMainSurface();
                GMM_GFX_SIZE_T MapSize = (i == 0) ? ((MaxPlanes > 1) ? YPlaneSize : SurfSize) : SurfSize - YPlaneSize;

                GMM_GFX_ADDRESS BaseSurfVA = (UpdateReq->AuxResInfo || i == 0) ? UpdateReq->BaseGpuVA :
                                                                                 UpdateReq->BaseGpuVA + YPlaneSize;
                GMM_GFX_ADDRESS AuxSurfVA = (UpdateReq->AuxResInfo) ? UpdateReq->AuxSurfVA : (i > 0 ? UVAuxVA : AuxVA);

                //Luma plane reset LumaChroma bit
                ((GMM_AUXTTL1e *)&PartialL1e)->LumaChroma = (i == 0) ? 0 : 1;
                uint32_t ArrayEle                         = GFX_MAX(((MaxPlanes > 1) ?
                                             UpdateReq->BaseResInfo->GetArraySize() :
                                             1),
                                            1);

                for(uint32_t j = 0; j < ArrayEle; j++)
                {
                    BaseSurfVA += ((j > 0) ? (UpdateReq->BaseResInfo->GetQPitchPlanar(GMM_PLANE_Y) * UpdateReq->BaseResInfo->GetRenderPitch()) : 0);
                    AuxSurfVA += (UpdateReq->AuxResInfo ?
                                  ((j > 0) ? (UpdateReq->AuxResInfo->GetQPitchPlanar(GMM_PLANE_Y) * UpdateReq->BaseResInfo->GetRenderPitch()) : 0) :
                                  ((j > 0) ? UpdateReq->BaseResInfo->GetAuxQPitch() : 0));

                    //(Flat mapping): Remove main/aux resInfo from params
                    Status = AuxTTObj->MapValidEntry(UpdateReq->UmdContext, BaseSurfVA, MapSize, UpdateReq->BaseResInfo,
                                                     AuxSurfVA, UpdateReq->AuxResInfo, PartialL1e, 1);
                    if(Status != GMM_SUCCESS)
                    {
                        GMM_ASSERTDPF(0, "Insufficient memory, free resources and try again");
                        EXIT_CRITICAL_SECTION
                        return Status;
                    }
                }
            }
        }
    }
    else
    {
        //Invalidate all mappings for given main surface
        AuxTTObj->InvalidateTable(UpdateReq->UmdContext, UpdateReq->BaseGpuVA, UpdateReq->BaseResInfo->GetSizeMainSurface(), UpdateReq->DoNotWait);
    }

    EXIT_CRITICAL_SECTION
    return GMM_SUCCESS;
}

#if defined(__linux__) && !_WIN32
/////////////////////////////////////////////////////////////////////////////////////
/// Gets size of PageTable buffer object (BOs) list
///
/// @param[in]  TTFlags: Flags specifying PageTable-type for which BO-count required
/// @return     non-zero if BO list is created. Zero otherwise.
/////////////////////////////////////////////////////////////////////////////////////
int GmmLib::GmmPageTableMgr::GetNumOfPageTableBOs(uint8_t TTFlags)
{
    int NumBO = 0;

    __GMM_ASSERTPTR(TTFlags & AUXTT, 0);

    ENTER_CRITICAL_SECTION

    if(AuxTTObj && AuxTTObj->GetL3Handle())
        NumBO++;

    NumBO += NumNodePoolElements;

    EXIT_CRITICAL_SECTION

    return NumBO;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Gets size of PageTable buffer object (BOs) list
///
/// @param[in]       TTFlags: Flags specifying PageTable-type for which BO-count required
/// @param[in][out]  BOList: pointer to memory where PageTable BO*(s) must be sent
/// @return     non-zero if BO list is created. Zero otherwise.
/////////////////////////////////////////////////////////////////////////////////////
int GmmLib::GmmPageTableMgr::GetPageTableBOList(uint8_t TTFlags, void *BOList)
{
    int                        NumBO   = GetNumOfPageTableBOs(TTFlags);
    HANDLE *                   Handles = (HANDLE *)BOList;
    GmmLib::GMM_PAGETABLEPool *Pool;

    __GMM_ASSERTPTR(TTFlags & AUXTT, 0);
    __GMM_ASSERTPTR(BOList, 0);
    __GMM_ASSERTPTR(NumBO, 0);

    ENTER_CRITICAL_SECTION

    if(AuxTTObj && AuxTTObj->GetL3Handle())
        Handles[0] = AuxTTObj->GetL3Handle();

    Pool = pPool;

    for(int i = 0; i < NumNodePoolElements; i++)
    {
        if(Pool)
        {
            Handles[i + 1] = Pool->GetPoolHandle();
            Pool           = Pool->GetNextPool();
        }
    }

    EXIT_CRITICAL_SECTION

    return NumBO;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// Releases GmmPageTableMgr, deleting root-tables and existing page-table pools
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::~GmmPageTableMgr()
{
    GMM_CLIENT ClientType;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);



    if(pPool)
    {
        ENTER_CRITICAL_SECTION
        pPool->__DestroyPageTablePool(&DeviceCbInt, hCsr);
        NumNodePoolElements = 0;
        EXIT_CRITICAL_SECTION
    }

    if(AuxTTObj)
    {
        DeleteCriticalSection(&PoolLock);

        if(AuxTTObj)
        {
            if(AuxTTObj->NullL1Table)
            {
                delete AuxTTObj->NullL1Table;
            }
            if(AuxTTObj->NullL2Table)
            {
                delete AuxTTObj->NullL2Table;
            }
            AuxTTObj->DestroyL3Table();
            delete AuxTTObj;
            AuxTTObj = NULL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Instantiates and zeroes out GmmPageTableMgr
///
/// @return     GmmPageTableMgr*
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmPageTableMgr::GmmPageTableMgr()
{
    this->AuxTTObj            = NULL;
    this->pPool               = NULL;
    this->NumNodePoolElements = 0;
    this->pClientContext      = NULL;
    this->hCsr                = NULL;

    memset(&DeviceCb, 0, sizeof(GMM_DEVICE_CALLBACKS));
    memset(&DeviceCbInt, 0, sizeof(GMM_DEVICE_CALLBACKS_INT));
}


