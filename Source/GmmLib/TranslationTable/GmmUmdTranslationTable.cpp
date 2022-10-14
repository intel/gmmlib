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

Description: Contains functions of internal classes 
             (ie PageTablePool, PageTable, Table), that support
             user mode page table management

============================================================================*/

#include "Internal/Common/GmmLibInc.h"
#include "../TranslationTable/GmmUmdTranslationTable.h"
#include "Internal/Common/Texture/GmmTextureCalc.h"

#if !defined(__GMM_KMD)

#if defined(__linux__)
#include "Internal/Linux/GmmResourceInfoLinInt.h"

#define _aligned_free(ptr) free(ptr)

#endif

//=============================================================================
//
// Function: AllocateL3Table
//
// Desc: Allocates (always resident SVM) memory for AUXTT\L3 Table, and updates AUXTT object
//
// Parameters:
//      pAUXTT_Obj: per-device AUX-TT object. Contains AUXTT node info
//
// Returns:
//      GMM_SUCCESS on success,
//      GMM_INVALIDPARAM on invalid parameter(s)
//      GMM_OUT_OF_MEMORY on memory allocation failure, failure to make resident
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::PageTable::AllocateL3Table(uint32_t L3TableSize, uint32_t L3AddrAlignment)
{
    GMM_STATUS       Status = GMM_SUCCESS;
    GMM_DEVICE_ALLOC Alloc  = {0};

    __GMM_ASSERTPTR(PageTableMgr, GMM_INVALIDPARAM);

    EnterCriticalSection(&TTLock);

    Alloc.Size      = L3TableSize;
    Alloc.Alignment = L3AddrAlignment;
    Alloc.hCsr      = PageTableMgr->hCsr;

    Status = __GmmDeviceAlloc(pClientContext, &PageTableMgr->DeviceCbInt, &Alloc);
    if(Status != GMM_SUCCESS)
    {
        LeaveCriticalSection(&TTLock);
        return Status;
    }

    TTL3.GfxAddress         = GMM_GFX_ADDRESS_CANONIZE(Alloc.GfxVA);
    TTL3.CPUAddress         = Alloc.CPUVA;
    TTL3.NeedRegisterUpdate = true;
    TTL3.L3Handle           = (HANDLE)(uintptr_t)Alloc.Handle;
    TTL3.pGmmResInfo        = (GMM_RESOURCE_INFO *)Alloc.Priv;

    // Invalidate L3e's
    for(int i = 0; i < (GMM_L3_SIZE(TTType)); i++)
    {
        //initialize L3e ie mark all entries with Null tile/invalid value
        ((GMM_AUXTTL3e *)TTL3.CPUAddress)[i].Value = 0;

    }

    LeaveCriticalSection(&TTLock);
    return Status;
}

//=============================================================================
//
// Function: __IsUnusedTRTTPoolOverLimit
//
// Desc: Checks if unused TRTTPools have reached residency limit and must be freed.
//
// Parameters:
//      pTRTT_Obj: per-device TT object. Contains TT node info
//      OverLimitSize: Size in bytes that can be freed
//
// Returns:
//      True, if unused TTPool reached max. residency limit
//      False, otherwise
//-----------------------------------------------------------------------------
bool GmmLib::GmmPageTablePool::__IsUnusedTRTTPoolOverLimit(GMM_GFX_SIZE_T *OverLimitSize)
{
    GMM_GFX_SIZE_T             UnusedTrTTPoolSize = 0;
    GmmLib::GMM_PAGETABLEPool *Pool               = NULL;

    Pool = this;

    while(Pool)
    {
        if(Pool->NumFreeNodes == PAGETABLE_POOL_MAX_NODES)
        {
            UnusedTrTTPoolSize += PAGETABLE_POOL_SIZE;
        }
        Pool = Pool->NextPool;
    }

    *OverLimitSize = (UnusedTrTTPoolSize > PAGETABLE_POOL_MAX_UNUSED_SIZE) ? (UnusedTrTTPoolSize - PAGETABLE_POOL_MAX_UNUSED_SIZE) : 0;
    return (UnusedTrTTPoolSize > PAGETABLE_POOL_MAX_UNUSED_SIZE) ? true : false;
}

//=============================================================================
//
// Function: AllocateL1L2Table
//
// Desc: Assigns pages from AUXTTPool for L1/L2Table for translation of given TRVA
//
// Parameters:
//      pAUXTT_Obj: per-device AUX-TT object. Contains AUXTT node info
//      TileAddr: Tiled Resource Virtual address
//
// Returns:
//     L2Table/L1Table Address
//-----------------------------------------------------------------------------
void GmmLib::PageTable::AllocateL1L2Table(GMM_GFX_ADDRESS TileAddr, GMM_GFX_ADDRESS *L1TableAdr, GMM_GFX_ADDRESS *L2TableAdr)
{
    GMM_GFX_ADDRESS         L3TableAdr = GMM_NO_TABLE;
    uint32_t                L3eIdx     = static_cast<uint32_t>(GMM_L3_ENTRY_IDX(TTType, TileAddr));
    uint32_t                L2eIdx     = static_cast<uint32_t>(GMM_L2_ENTRY_IDX(TTType, TileAddr));
    GmmLib::LastLevelTable *pL1Tbl     = NULL;

    __GMM_ASSERTPTR(pClientContext, VOIDRETURN); // void return

    *L2TableAdr = GMM_NO_TABLE;
    *L1TableAdr = GMM_NO_TABLE;

    if(TTL3.L3Handle)
    {
        L3TableAdr = TTL3.GfxAddress;
    }
    else
    {
        //Should never hit -- L3Table is allocated during device creation
        __GMM_ASSERT(false);
    }

    if(pTTL2[L3eIdx].GetPool())
    {
        GmmLib::GMM_PAGETABLEPool *PoolElem = NULL;
        PoolElem                            = pTTL2[L3eIdx].GetPool();
        *L2TableAdr                         = (PoolElem != NULL) ? PoolElem->GetGfxAddress() + (PAGE_SIZE * pTTL2[L3eIdx].GetNodeIdx()) : GMM_NO_TABLE;
    }
    else
    {
        uint32_t                   PoolNodeIdx = PAGETABLE_POOL_MAX_NODES;
        GmmLib::GMM_PAGETABLEPool *PoolElem    = NULL;
        POOL_TYPE                  PoolType    = POOL_TYPE_AUXTTL2;
        PoolElem                               = PageTableMgr->__GetFreePoolNode(&PoolNodeIdx, PoolType);
        if(PoolElem)
        {
            pTTL2[L3eIdx] = MidLevelTable(PoolElem, PoolNodeIdx, PoolElem->GetNodeBBInfoAtIndex(PoolNodeIdx));
            *L2TableAdr   = PoolElem->GetGfxAddress() + PAGE_SIZE * PoolNodeIdx; //PoolNodeIdx must be multiple of 8 (Aux L2) and multiple of 2 (Aux L1)
            ASSIGN_POOLNODE(PoolElem, PoolNodeIdx, NodesPerTable)
        }
    }

    pL1Tbl = pTTL2[L3eIdx].GetL1Table(L2eIdx);
    if(pL1Tbl)
    {
        GmmLib::GMM_PAGETABLEPool *PoolElem = NULL;
        PoolElem                            = pL1Tbl->GetPool();
        *L1TableAdr                         = (PoolElem != NULL) ? PoolElem->GetGfxAddress() + (PAGE_SIZE * pL1Tbl->GetNodeIdx()) : GMM_NO_TABLE;
    }
    else
    {
        //Allocate L1 Table
        uint32_t                   PoolNodeIdx = PAGETABLE_POOL_MAX_NODES;
        GmmLib::GMM_PAGETABLEPool *PoolElem    = NULL;
        POOL_TYPE                  PoolType    = POOL_TYPE_AUXTTL1;

        PoolElem = PageTableMgr->__GetFreePoolNode(&PoolNodeIdx, PoolType); //Recognize if Aux-L1 being allocated
        if(PoolElem)
        {
            pL1Tbl = new GmmLib::LastLevelTable(PoolElem, PoolNodeIdx, GMM_L1_SIZE_DWORD(TTType, GetGmmLibContext()), L2eIdx); // use TR vs Aux L1_Size_DWORD

            if(pL1Tbl)
            {
                *L1TableAdr = PoolElem->GetGfxAddress() + PAGE_SIZE * PoolNodeIdx; //PoolNodeIdx should reflect 1 node per Tr-table and 2 nodes per AUX L1 TABLE
                if(PoolNodeIdx != PAGETABLE_POOL_MAX_NODES)
                {
                    uint32_t PerTableNodes = (TTType == AUXTT) ? AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetGmmLibContext()) : 1;
		    ASSIGN_POOLNODE(PoolElem, PoolNodeIdx, PerTableNodes)
                }
                pTTL2[L3eIdx].InsertInList(pL1Tbl);
            }
        }
    }
}

//=============================================================================
//
// Function: AllocateDummyTables
//
// Desc: Assigns pages from AUXTTPool for Dummy L1/L2Table
//
// Parameters:
//     L2Table - Ptr to initiatize dummy L2Table
//     L1Table - Ptr to initiatize dummy L1Table
//
// Returns:
//     L2Table/L1Tables
//-----------------------------------------------------------------------------
void GmmLib::PageTable::AllocateDummyTables(GmmLib::Table **L2Table, GmmLib::Table **L1Table)
{
    GMM_GFX_ADDRESS         L3TableAdr = GMM_NO_TABLE;
    GmmLib::LastLevelTable *pL1Tbl     = NULL;

    __GMM_ASSERTPTR(pClientContext, VOIDRETURN);

    if(TTL3.L3Handle)
    {
        L3TableAdr = TTL3.GfxAddress;
    }
    else
    {
        //Should never hit -- L3Table is allocated during device creation
        __GMM_ASSERT(false);
    }

    //Allocate dummy L2Table
    {
        uint32_t                   PoolNodeIdx = PAGETABLE_POOL_MAX_NODES;
        GmmLib::GMM_PAGETABLEPool *PoolElem    = NULL;
        POOL_TYPE                  PoolType    = POOL_TYPE_AUXTTL2;
        PoolElem                               = PageTableMgr->__GetFreePoolNode(&PoolNodeIdx, PoolType);
        if(PoolElem)
        {
            *L2Table = new GmmLib::MidLevelTable(PoolElem, PoolNodeIdx, PoolElem->GetNodeBBInfoAtIndex(PoolNodeIdx));
            ASSIGN_POOLNODE(PoolElem, PoolNodeIdx, NodesPerTable)
        }
    }

    //Allocate dummy L1Table
    {
        uint32_t                   PoolNodeIdx = PAGETABLE_POOL_MAX_NODES;
        GmmLib::GMM_PAGETABLEPool *PoolElem    = NULL;
        POOL_TYPE                  PoolType    = POOL_TYPE_AUXTTL1;

        PoolElem = PageTableMgr->__GetFreePoolNode(&PoolNodeIdx, PoolType); //Recognize if Aux-L1 being allocated
        if(PoolElem)
        {
            *L1Table = new GmmLib::LastLevelTable(PoolElem, PoolNodeIdx, GMM_L1_SIZE_DWORD(TTType, GetGmmLibContext()), 0); // use TR vs Aux L1_Size_DWORD

            if(*L1Table)
            {
                if(PoolNodeIdx != PAGETABLE_POOL_MAX_NODES)
                {
                    uint32_t PerTableNodes = (TTType == AUXTT) ? AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetGmmLibContext()) : 1;
                    ASSIGN_POOLNODE(PoolElem, PoolNodeIdx, PerTableNodes)
                }
            }
        }
    }
}

//=============================================================================
//
// Function: GetL1L2TableAddr
//
// Desc: For given tile address, returns L1/L2 Table address if the table exists
//
// Parameters:
//      pAUXTT_Obj: per-device AUX-TT object. Contains AXUTT node info
//      TileAddr: Tiled Resource Virtual address
//
// Returns:
//     L2Table/L1Table Address
//-----------------------------------------------------------------------------
void GmmLib::PageTable::GetL1L2TableAddr(GMM_GFX_ADDRESS TileAddr, GMM_GFX_ADDRESS *L1TableAdr, GMM_GFX_ADDRESS *L2TableAdr)
{
    GMM_GFX_SIZE_T  L3eIdx, L2eIdx, L1eIdx;
    GMM_GFX_ADDRESS L3TableAdr = GMM_NO_TABLE;
    *L2TableAdr                = GMM_NO_TABLE;
    *L1TableAdr                = GMM_NO_TABLE;

    __GMM_ASSERTPTR(pClientContext, VOIDRETURN);

    L3eIdx = GMM_L3_ENTRY_IDX(TTType, TileAddr);
    L2eIdx = GMM_L2_ENTRY_IDX(TTType, TileAddr);
    L1eIdx = GMM_L1_ENTRY_IDX(TTType, TileAddr, GetGmmLibContext());

    __GMM_ASSERT(TTL3.L3Handle);
    L3TableAdr = TTL3.GfxAddress;
    if(pTTL2[L3eIdx].GetPool())
    {
        GmmLib::GMM_PAGETABLEPool *Pool   = NULL;
        GmmLib::LastLevelTable *   pL1Tbl = NULL;
        Pool                              = pTTL2[L3eIdx].GetPool();

        if(Pool)
        {
            __GMM_ASSERT(Pool->GetNumFreeNode() != PAGETABLE_POOL_MAX_NODES);
            __GMM_ASSERT(pTTL2[L3eIdx].GetNodeIdx() < PAGETABLE_POOL_MAX_NODES);
            __GMM_ASSERT(Pool->GetNodeUsageAtIndex(pTTL2[L3eIdx].GetNodeIdx() / (32 * NodesPerTable)) != 0);

            *L2TableAdr = Pool->GetGfxAddress() + PAGE_SIZE * (pTTL2[L3eIdx].GetNodeIdx());
        }

        pL1Tbl = pTTL2[L3eIdx].GetL1Table(L2eIdx);
        if(pL1Tbl && pL1Tbl->GetPool())
        {
            Pool = NULL;
            Pool = pL1Tbl->GetPool();
            if(Pool)
            {
                uint32_t PerTableNodes = (TTType == AUXTT) ? AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetGmmLibContext()) : 1;
                __GMM_ASSERT(Pool->GetNumFreeNode() != PAGETABLE_POOL_MAX_NODES);
                __GMM_ASSERT(pL1Tbl->GetNodeIdx() < PAGETABLE_POOL_MAX_NODES);
                __GMM_ASSERT(Pool->GetNodeUsageAtIndex(pL1Tbl->GetNodeIdx() / (32 * PerTableNodes)) != 0);

                *L1TableAdr = Pool->GetGfxAddress() + PAGE_SIZE * (pL1Tbl->GetNodeIdx());
            }
        }
    }
}

//=============================================================================
//
// Function: GetMappingType
//
// Desc: For given gfx address and size, returns MappingType (null/non-null or
//       Valid/Invalid) of GfxVA and first gfx address have reverse mapping
//
/// @param[in] GfxVA: Gfx Address whose mapping type is being queried
/// @param[in] Size:  Size of interested Gfx address range
/// @param[out] LastAddr : 1st Gfx Address having reverse mapping type
//
/// @return    1/0 : for non-null/valid vs null/invalid mapping
//-----------------------------------------------------------------------------
uint8_t GmmLib::PageTable::GetMappingType(GMM_GFX_ADDRESS GfxVA, GMM_GFX_SIZE_T Size, GMM_GFX_ADDRESS &LastAddr)
{
    GMM_GFX_SIZE_T  L3eIdx, L2eIdx, L1eIdx, L1EntrySize;
    uint32_t        L1Size, L2Size;
    uint8_t         MapType      = 0; //true for non-null, false for null mapped
    bool            bFoundLastVA = false, bTerminate = false;
    GMM_GFX_ADDRESS TileAddr = GfxVA;

    L3eIdx      = GMM_L3_ENTRY_IDX(TTType, GfxVA);
    L2eIdx      = GMM_L2_ENTRY_IDX(TTType, GfxVA);
    L1eIdx      = GMM_L1_ENTRY_IDX(TTType, GfxVA, GetGmmLibContext());
    L1EntrySize = WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : WA64K(GetGmmLibContext()) ? GMM_KBYTE(64) : GMM_MBYTE(1);

    EnterCriticalSection(&TTLock);
    __GMM_ASSERT(TTL3.L3Handle);

#define GET_NEXT_L1TABLE(L1eIdx, L2eIdx, L3eIdx) \
    {                                            \
        L1eIdx = 0;                              \
        L2eIdx++;                                \
        if(L2eIdx == (GMM_L2_SIZE(TTType)))      \
        {                                        \
            L2eIdx = 0;                          \
            L3eIdx++;                            \
            if(L3eIdx == (GMM_L3_SIZE(TTType)))  \
            {                                    \
                bTerminate = true;               \
            }                                    \
        }                                        \
    }

#define GET_NEXT_L2TABLE(L1eIdx, L2eIdx, L3eIdx) \
    {                                            \
        L1eIdx = L2eIdx = 0;                     \
        L3eIdx++;                                \
        if(L3eIdx == (GMM_L3_SIZE(TTType)))      \
        {                                        \
            bTerminate = true;                   \
        }                                        \
    }

    while(!(bFoundLastVA || bTerminate) && (TileAddr < GfxVA + Size))
    {
        if(pTTL2[L3eIdx].GetPool())
        {
            GmmLib::LastLevelTable *pL1Tbl = NULL;
            pL1Tbl                         = pTTL2[L3eIdx].GetL1Table(L2eIdx);
            if(pL1Tbl && pL1Tbl->GetPool())
            {
                uint32_t LastBit = 0;
                uint32_t i       = static_cast<uint32_t>(L1eIdx) / 32;

                while(!bFoundLastVA && i < (uint32_t)(GMM_L1_SIZE_DWORD(TTType, GetGmmLibContext())))
                {
                    uint32_t UsageDW = pL1Tbl->GetUsedEntries()[i++];
                    uint32_t BitNum  = 31;
                    if(GfxVA == TileAddr)
                    {
                        BitNum  = L1eIdx % 32;
                        MapType = ((UsageDW & __BIT(BitNum)) ? 0x1 : 0x0); //true for non-null, false for null mapped
                        UsageDW = (!MapType) ? UsageDW : ~UsageDW;
                        UsageDW = ((uint64_t)UsageDW >> (BitNum + 1)) << (BitNum + 1); // clear lsb <= BitNum
                    }
                    else
                    {
                        UsageDW = (!MapType) ? UsageDW : ~UsageDW;
                    }

                    if(_BitScanForward((uint32_t *)&LastBit, UsageDW)) // Gets lsb > BitNum, having reverse mapType
                    {
                        bFoundLastVA      = true;
                        uint32_t NumTiles = (GfxVA == TileAddr) ? (LastBit - BitNum) : LastBit;
                        LastAddr          = TileAddr + NumTiles * L1EntrySize;
                    }
                    else
                    {
                        uint32_t NumTiles = (GfxVA == TileAddr) ? (32 - BitNum) : 32;
                        TileAddr += NumTiles * L1EntrySize;
                    }
                }
                if(!bFoundLastVA)
                {
                    GET_NEXT_L1TABLE(L1eIdx, L2eIdx, L3eIdx);
                }
            }
            else //L2Entry is NULL
            {
                if(MapType) //First hit null-map
                {
                    LastAddr     = TileAddr;
                    bFoundLastVA = true;
                }
                else
                {
                    GMM_GFX_SIZE_T NumTiles = GMM_L1_USABLESIZE(TTType, GetGmmLibContext());			
                    if(GfxVA == TileAddr)
                    {
                        MapType = false;
                        NumTiles -= L1eIdx;
                    }
                    TileAddr += NumTiles * L1EntrySize;
                    GET_NEXT_L1TABLE(L1eIdx, L2eIdx, L3eIdx)
                }
            }
        }
        else //L3entry is NULL
        {
            if(MapType) //First hit null-map
            {
                LastAddr     = TileAddr;
                bFoundLastVA = true;
            }
            else
            {
                GMM_GFX_SIZE_T NumTiles = 0;
                if(GfxVA == TileAddr)
                {
                    MapType  = false;
                    L1Size   = GMM_L1_USABLESIZE(TTType, GetGmmLibContext()) - L1eIdx;
                    L2Size   = GMM_L2_SIZE(TTType) - L2eIdx;
                    NumTiles = ((uint64_t)L1Size * L2Size);
                }
                else
                {
                    L1Size   = GMM_L1_USABLESIZE(TTType, GetGmmLibContext());
                    L2Size   = GMM_L2_SIZE(TTType);
                    NumTiles = ((uint64_t)L1Size * L2Size);
                }
                TileAddr += NumTiles * L1EntrySize;
                GET_NEXT_L2TABLE(L1eIdx, L2eIdx, L3eIdx)
            }
        }
    }

    if(!bFoundLastVA)
    {
        LastAddr = TileAddr;
    }

    LeaveCriticalSection(&TTLock);
    return MapType;
}

//=============================================================================
//
// Function: TrackTableUsage
//
// Desc: For given tile address, updates Table Usage.If Table has all Nullmappings
//       then its pool node can be unassigned
//
// Parameters:
//      Type:  Translation Table type (Aux)
//      IsL1:  Is called for L1table or L2 Table
//      TileAddr: Tiled Resource Virtual address
//      NullMapped: true if given tiled adr was null mapped, otherwise false
//
// Returns:
//     true, if Table for given tile adr is all null mapped
//     false,if Table does not exist or has non-null mapping
//-----------------------------------------------------------------------------
bool GmmLib::Table::TrackTableUsage(TT_TYPE Type, bool IsL1, GMM_GFX_ADDRESS TileAdr, bool NullMapped, GMM_LIB_CONTEXT *pGmmLibContext )
{
    uint32_t EntryIdx;
    uint32_t ElemNum = 0, BitNum = 0;

    EntryIdx = IsL1 ? static_cast<uint32_t>(GMM_L1_ENTRY_IDX(Type, TileAdr, pGmmLibContext)) : static_cast<uint32_t>(GMM_L2_ENTRY_IDX(Type, TileAdr));

    ElemNum = EntryIdx / (sizeof(UsedEntries[0]) * 8);
    BitNum  = EntryIdx % (sizeof(UsedEntries[0]) * 8);

    if(NullMapped)
    {
        UsedEntries[ElemNum] &= ~(1 << BitNum);
    }
    else
    {
        UsedEntries[ElemNum] |= (1 << BitNum);
    }

    if(NullMapped)
    {
        int TableDWSize = IsL1 ? static_cast<int>(GMM_L1_SIZE_DWORD(Type,  pGmmLibContext)) : static_cast<int>(GMM_L2_SIZE_DWORD(Type));
        for(int i = 0; i < TableDWSize; i++)
        {
            if(UsedEntries[i])
            {
                return false;
            }
        }
    }
    return NullMapped ? true : false;
}

//=============================================================================
//
// Function: __IsTableNullMapped
//
// Desc: For given tile address, checks if given Table has all Nullmappings
//       then its pool node can be unassigned
//
// Parameters:
//      Type:  Translation Table type (TR or Aux)
//      IsL1:  Is called for L1table or L2 Table
//      TileAddr: Tiled Resource Virtual address
//
// Returns:
//     true, if Table for given tile adr is all null mapped
//     false,if Table has non-null mapping
//-----------------------------------------------------------------------------
bool GmmLib::Table::IsTableNullMapped(TT_TYPE Type, bool IsL1, GMM_GFX_ADDRESS TileAdr, GMM_LIB_CONTEXT *pGmmLibContext)
{
    GMM_UNREFERENCED_PARAMETER(TileAdr);
    int TableDWSize = IsL1 ? static_cast<int>(GMM_L1_SIZE_DWORD(Type, pGmmLibContext)) : static_cast<int>(GMM_L2_SIZE_DWORD(Type));
    for(int i = 0; i < TableDWSize; i++)
    {
        if(UsedEntries[i])
        {
            return false;
        }
    }
    return true;
}

//=============================================================================
//
// Function: __UpdatePoolFence
//
// Desc: Updates AUXTTPool's or Table's BBFenceObj/value with current BB fence
//
// Parameters:
//      pAUXTT_Obj: per-device AUX-TT object. Contains AUXTT node info
//      Table:     L1/L2 table pointer
//      L1Table:   true for L1 Table, else false
//      ClearNode: if true, Fence info is cleared for table
//                    false, Fence info is updated for table and pool
//-----------------------------------------------------------------------------
void GmmLib::Table::UpdatePoolFence(GMM_UMD_SYNCCONTEXT *UmdContext, bool ClearNode)
{
    if(!ClearNode)
    {
        //update both node and pool with current fence/handle
        PoolElem->GetPoolBBInfo().BBQueueHandle =
        BBInfo.BBQueueHandle = UmdContext->BBFenceObj;
        PoolElem->GetPoolBBInfo().BBFence =
        BBInfo.BBFence = UmdContext->BBLastFence + 1; //Save incremented fence value, since DX does it during submission
    }
    else
    {
        //Clear node fence/handle
        BBInfo.BBQueueHandle = 0;
        BBInfo.BBFence       = 0;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Releases all PageTable Pool(s) existing in Linked List
///
/// @param[in]  DeviceCallbacks   pointer to device callbacks structure
/// @return     GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmPageTablePool::__DestroyPageTablePool(void *DeviceCallbacks, HANDLE hCsr)
{
    GMM_STATUS                Status   = GMM_SUCCESS;
    GMM_DEVICE_CALLBACKS_INT *DeviceCb = static_cast<GMM_DEVICE_CALLBACKS_INT *>(DeviceCallbacks);

    GMM_PAGETABLEPool *Node = this, *Next = NULL;
    GMM_CLIENT         ClientType;
    GMM_DEVICE_DEALLOC Dealloc = {0};

    //Evict/Free gpu Va is implictly done by OS when de-allocating
    while(Node)
    {
        Next = Node->NextPool;

        GET_GMM_CLIENT_TYPE(Node->pClientContext, ClientType);

        Dealloc.Handle = Node->PoolHandle;
        Dealloc.GfxVA  = Node->PoolGfxAddress;
        Dealloc.Priv   = Node->pGmmResInfo;
        Dealloc.hCsr   = hCsr;

        Status = __GmmDeviceDealloc(ClientType, DeviceCb, &Dealloc, Node->pClientContext);

        Node->PoolHandle     = NULL;
        Node->PoolGfxAddress = 0;
        delete Node;
        Node = Next;
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Releases memory allocated to PageTable's Root-table
///
/// @return     GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::PageTable::DestroyL3Table()
{
    GMM_STATUS         Status = GMM_SUCCESS;
    uint8_t            hr     = GMM_SUCCESS;
    GMM_CLIENT         ClientType;
    GMM_DEVICE_DEALLOC Dealloc = {0};

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);

    EnterCriticalSection(&TTLock);

    if(TTL3.L3Handle)
    {
        Dealloc.Handle = TTL3.L3Handle;
        Dealloc.GfxVA  = TTL3.GfxAddress;
        Dealloc.Priv   = TTL3.pGmmResInfo;
        Dealloc.hCsr   = PageTableMgr->hCsr;

        Status = __GmmDeviceDealloc(ClientType, &PageTableMgr->DeviceCbInt, &Dealloc, pClientContext);

        TTL3.L3Handle   = NULL;
        TTL3.GfxAddress = 0;
        TTL3.CPUAddress = 0;
    }

    LeaveCriticalSection(&TTLock);
    return Status;
}

#endif //!__GMM_KMD
