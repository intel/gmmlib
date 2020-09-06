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

Description: This file contains the class definitions for GmmPageTablePool
             PageTable, and low-level Tables for user-mode PageTable management,
             that is common for both Linux and Windows.

======================= end_copyright_notice ==================================*/
#pragma once
#include "External/Common/GmmPageTableMgr.h"

#ifdef __linux__
#include <pthread.h>
#include <string.h>

// Internal Linux version of MSDK APIs.
static inline void InitializeCriticalSection(pthread_mutex_t *mutex)
{
    pthread_mutexattr_t Attr;
    pthread_mutexattr_init(&Attr);
    pthread_mutexattr_settype(&Attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(mutex, &Attr);
}

static inline void DeleteCriticalSection(pthread_mutex_t *mutex)
{
    pthread_mutex_destroy(mutex);
}

static inline void EnterCriticalSection(pthread_mutex_t *mutex)
{
    pthread_mutex_lock(mutex);
}

static inline void LeaveCriticalSection(pthread_mutex_t *mutex)
{
    pthread_mutex_unlock(mutex);
}

#ifndef _BitScanForwardDefined
static inline int _BitScanForward(uint32_t *index, uint32_t mask)
{
    int i;

#ifdef __ANDROID__
    i = ffs(mask);
#else
    i = ffsl(mask);
#endif

    if(i > 0)
    {
        *index = (uint32_t)(i - 1);
        return i;
    }
    return 0;
}
#endif
#endif

#define GMM_L1_SIZE(TTType, pGmmLibContext)       GMM_AUX_L1_SIZE(pGmmLibContext) 
#define GMM_L1_SIZE_DWORD(TTType, pGmmLibContext) GMM_AUX_L1_SIZE_DWORD(pGmmLibContext)
#define GMM_L2_SIZE(TTType)                       GMM_AUX_L2_SIZE
#define GMM_L2_SIZE_DWORD(TTType)                 GMM_AUX_L2_SIZE_DWORD 
#define GMM_L3_SIZE(TTType)                       GMM_AUX_L3_SIZE 
#define GMM_L1_ENTRY_IDX(TTType, GfxAddress, pGmmLibContext) GMM_AUX_L1_ENTRY_IDX((GfxAddress), (pGmmLibContext)) 
#define GMM_L2_ENTRY_IDX(TTType, GfxAddress)                 GMM_AUX_L2_ENTRY_IDX((GfxAddress)) 
#define GMM_L3_ENTRY_IDX(TTType, GfxAddress)                 GMM_AUX_L3_ENTRY_IDX((GfxAddress)) 

#ifdef GMM_ULT
#define GMM_L1_ENTRY_IDX_EXPORTED(TTType, GfxAddress, WA64KEx)    GMM_AUX_L1_ENTRY_IDX_EXPORTED((GfxAddress), WA64KEx) 
#endif

#ifdef __cplusplus
#include "External/Common/GmmMemAllocator.hpp"

//HW provides single-set of TR/Aux-TT registers for non-privileged programming
//Engine-specific offsets are HW-updated with programmed values.
#define GET_L3ADROFFSET(TRTT, L3AdrOffset) \
           L3AdrOffset = 0x4200;            


#define ASSIGN_POOLNODE(Pool, NodeIdx, PerTableNodes)    {       \
    (Pool)->GetNodeUsageAtIndex((NodeIdx) / (32 *(PerTableNodes))) |= __BIT(((NodeIdx) / (PerTableNodes)) % 32);  \
    (Pool)->GetNodeBBInfoAtIndex(NodeIdx) = SyncInfo();                              \
    (Pool)->GetNumFreeNode() -= (PerTableNodes);                      \
                                          }

#define DEASSIGN_POOLNODE(PageTableMgr, UmdContext, Pool, NodeIdx, PerTableNodes)  {            \
    (Pool)->GetNodeUsageAtIndex((NodeIdx) / (32 * (PerTableNodes))) &= ~__BIT(((NodeIdx) / (PerTableNodes)) % 32 );  \
    (Pool)->GetNumFreeNode() += (PerTableNodes);                      \
    if((Pool)->GetNumFreeNode() == PAGETABLE_POOL_MAX_NODES) {        \
    PageTableMgr->__ReleaseUnusedPool((UmdContext));              \
                                                    }             \
                                          }

namespace GmmLib
{
#define PAGETABLE_POOL_MAX_NODES         512                               //Max. number of L2/L1 tables pool contains
#define PAGETABLE_POOL_SIZE_IN_DWORD     PAGETABLE_POOL_MAX_NODES / 32
#define PAGETABLE_POOL_SIZE              PAGETABLE_POOL_MAX_NODES * PAGE_SIZE   //Pool for L2/L1 table allocation
#define AUX_L2TABLE_SIZE_IN_POOLNODES    8                                 //Aux L2 is 32KB
#define AUX_L1TABLE_SIZE_IN_POOLNODES    2                                 //Aux L1 is 8KB
#define PAGETABLE_POOL_MAX_UNUSED_SIZE   GMM_MBYTE(16)                     //Max. size of unused pool, driver keeps resident


    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Contains functions and members for GmmPageTablePool. 
    /// PageTablePool is a Linked-list, provides common location for both Aux TT and TR-TT pages
    /// Separate NodePool (linked-list element) kept for each PoolType, for cleaner management in 
    /// per-table size
    /////////////////////////////////////////////////////////////////////////////////////////////
    class GmmPageTablePool
    {
    private:
                                       //PageTablePool allocation descriptor
        GMM_RESOURCE_INFO* pGmmResInfo;   
        HANDLE             PoolHandle;
        GMM_GFX_ADDRESS    PoolGfxAddress;
        GMM_GFX_ADDRESS    CPUAddress;              //LMEM-cpuvisible adr

        POOL_TYPE         PoolType;       //Separate Node-pools for TR-L2, TR-L1, Aux-L2, Aux-L1 usages-  

                                      //PageTablePool usage descriptors
        int              NumFreeNodes;    //has value {0 to Pool_Max_nodes}
        uint32_t*           NodeUsage;       //destined node state (updated during node assignment and removed based on destined state of L1/L2 Table 
                                          //that used the pool node) 
                                          //Aux-Pool node-usage tracked at every eighth/second node(for L2 vs L1) 
                                          //ie 1b per node for TR-table, 1b per 8-nodes for Aux-L2table, 1b per 2-nodes for AuxL1-table
                                          //array size= POOL_SIZE_IN_DWORD for TR, =POOL_SIZE_IN_DWORD/8 for AuxL2, POOL_SIZE_IN_DWORD/2 for AuxL1

        SyncInfo*        NodeBBInfo;      //BB info for pending Gpu usage of each pool node
                                          //array of size MaxPoolNodes for TR, =MaxPoolNodes / 8 for Aux, MaxPoolNodes / 2 for AuxL1

        SyncInfo         PoolBBInfo;      //BB info for Gpu usage of the Pool (most recent of pool node BB info)

        GmmPageTablePool* NextPool;       //Next node-Pool in the LinkedList
        GmmClientContext    *pClientContext;    ///< ClientContext of the client creating this Object
    public:
        GmmPageTablePool() :
            pGmmResInfo(NULL),
            PoolHandle(),
            PoolGfxAddress(0x0),
            CPUAddress(0x0),
            PoolType(POOL_TYPE_TRTTL1),
            NumFreeNodes(PAGETABLE_POOL_MAX_NODES),
            NodeUsage(NULL),
            NodeBBInfo(NULL),
            PoolBBInfo(),
            NextPool(NULL),
            pClientContext(NULL)
        {

        }
        GmmPageTablePool(HANDLE hAlloc, GMM_RESOURCE_INFO* pGmmRes, GMM_GFX_ADDRESS SysMem, POOL_TYPE Type) :
            GmmPageTablePool()
        {
            PoolHandle = hAlloc;
            pGmmResInfo = pGmmRes;
            PoolGfxAddress = SysMem;
            CPUAddress = PoolGfxAddress;
            NextPool = NULL;
            NumFreeNodes = PAGETABLE_POOL_MAX_NODES;
            PoolType = Type;
            int DwordPoolSize = (Type == POOL_TYPE_AUXTTL1) ? PAGETABLE_POOL_SIZE_IN_DWORD / AUX_L1TABLE_SIZE_IN_POOLNODES
                                        : (Type == POOL_TYPE_AUXTTL2) ? PAGETABLE_POOL_SIZE_IN_DWORD / AUX_L2TABLE_SIZE_IN_POOLNODES
                                        : PAGETABLE_POOL_SIZE_IN_DWORD;
            NodeUsage = new uint32_t[DwordPoolSize]();
            NodeBBInfo = new SyncInfo[DwordPoolSize * 32]();
            if (pGmmResInfo)
            {
                pClientContext = pGmmResInfo->GetGmmClientContext();
            }
        }
        GmmPageTablePool(HANDLE hAlloc, GMM_RESOURCE_INFO* pGmmRes, GMM_GFX_ADDRESS GfxAdr, GMM_GFX_ADDRESS CPUAdr, POOL_TYPE Type) :
            GmmPageTablePool(hAlloc, pGmmRes, GfxAdr, Type)
        {
            CPUAddress = (CPUAdr != GfxAdr) ? CPUAdr : GfxAdr;
        }
        ~GmmPageTablePool()
        {
            delete[] NodeUsage;
            delete[] NodeBBInfo;
        }

        GmmPageTablePool* InsertInList(GmmPageTablePool* NewNode)
        {
            GmmPageTablePool *Node = this;
            while (Node->NextPool)
            {
                Node = Node->NextPool;
            }
            Node->NextPool = NewNode;
            return Node->NextPool;
        }

        GmmPageTablePool* InsertInListAtBegin(GmmPageTablePool* NewNode)
        {
            GmmPageTablePool *Node = this;
            NewNode->NextPool = Node;
            return NewNode;
        }

        GmmPageTablePool* &GetNextPool() { return NextPool; }
        HANDLE& GetPoolHandle() { return PoolHandle; }
        POOL_TYPE& GetPoolType() { return PoolType; }
        int& GetNumFreeNode() { return NumFreeNodes; }
        SyncInfo& GetPoolBBInfo() { return PoolBBInfo; }
        uint32_t& GetNodeUsageAtIndex(int j) { return NodeUsage[j]; }
        SyncInfo& GetNodeBBInfoAtIndex(int j)
        {
            int BBInfoNodeIdx = (PoolType == POOL_TYPE_AUXTTL1) ? j / AUX_L1TABLE_SIZE_IN_POOLNODES
                : (PoolType == POOL_TYPE_AUXTTL2) ? j / AUX_L2TABLE_SIZE_IN_POOLNODES
                : j;
            return NodeBBInfo[BBInfoNodeIdx];
        }
        GMM_GFX_ADDRESS GetGfxAddress() { return PoolGfxAddress; }
        GMM_GFX_ADDRESS GetCPUAddress() { return CPUAddress; }
        GMM_RESOURCE_INFO* &GetGmmResInfo() { return pGmmResInfo; }
        bool IsPoolInUse(SyncInfo BBInfo) {
            if (NumFreeNodes < PAGETABLE_POOL_MAX_NODES ||
                (PoolBBInfo.BBQueueHandle == BBInfo.BBQueueHandle &&
                    PoolBBInfo.BBFence == BBInfo.BBFence + 1)) //Pool will be used by next BB submission, freeing it will cause page fault
            {
                return true;
            }
            return false;
        }
        bool __IsUnusedTRTTPoolOverLimit(GMM_GFX_SIZE_T * OverLimitSize);
        void ClearBBReference(void * BBQHandle);
        GMM_STATUS __DestroyPageTablePool(void * DeviceCallbacks,HANDLE hCsr);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Contains functions and members for Table. 
    /// Table defines basic building block for tables at different page-table levels
    /////////////////////////////////////////////////////////////////////////////////////////////
    class Table
    {
    protected:
        GMM_PAGETABLEPool *PoolElem;              //L2 Pool ptr different for L2Tables when Pool_nodes <512
        int             PoolNodeIdx;              //pool node idx used for L2 Table
        SyncInfo             BBInfo;              //BB Handle/fence using Table
        uint32_t*          UsedEntries;              //Tracks which L1/L2 entries are being used
                                                  //array size GMM_L1_SIZE_DWORD(TT-type) for LastLevelTable, MidLeveltable(??)
                                                  //array of 1024/32=32 DWs for TR-table, 4096/32 =512 for Aux-Table
    public:
        Table() :
            PoolElem(NULL),
            PoolNodeIdx(),
            BBInfo(),
            UsedEntries(NULL)
        {
        }
        int& GetNodeIdx() { return  PoolNodeIdx; }
        GmmPageTablePool* &GetPool() { return PoolElem; }
        GMM_GFX_ADDRESS GetCPUAddress() { return (PoolElem->GetCPUAddress() + (PoolNodeIdx * PAGE_SIZE)); }
        SyncInfo& GetBBInfo() { return BBInfo; }
        uint32_t* &GetUsedEntries() { return UsedEntries; }
        bool TrackTableUsage(TT_TYPE Type, bool IsL1, GMM_GFX_ADDRESS TileAdr, bool NullMapped);
        bool IsTableNullMapped(TT_TYPE Type, bool IsL1, GMM_GFX_ADDRESS TileAdr);
        void UpdatePoolFence(GMM_UMD_SYNCCONTEXT * UmdContext, bool ClearNode);
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Contains functions and members for LastLevelTable. 
    /// LastLevelTable defines leaf level tables in multi-level pageTable structure
    /////////////////////////////////////////////////////////////////////////////////////////////
    class LastLevelTable : public Table
    {
    private:
        uint32_t         L2eIdx;
        LastLevelTable *pNext;

    public:
        LastLevelTable() : Table(),
            L2eIdx()                             //Pass in Aux vs TR table's GMM_L2_SIZE and initialize L2eIdx?
        {
            pNext = NULL;
        }

        LastLevelTable(GMM_PAGETABLEPool* Elem, int NodeIdx, int DwordL1e, int L2eIndex)
            : LastLevelTable()
        {
            PoolElem = Elem;
            PoolNodeIdx = NodeIdx;
            BBInfo = Elem->GetNodeBBInfoAtIndex(NodeIdx);
            L2eIdx = L2eIndex;
            pNext = NULL;
            UsedEntries = new uint32_t[DwordL1e]();
        }
        ~LastLevelTable()
        {
            delete[] UsedEntries;
        }

        int GetL2eIdx() {
            return L2eIdx;
        }

        LastLevelTable* &Next() {
            return pNext;
        }
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Contains functions and members for MidLevelTable. 
    /// MidLevelTable defines secondary level tables in multi-level pageTable structure
    /////////////////////////////////////////////////////////////////////////////////////////////
    class MidLevelTable : public Table
    {
    private:
        LastLevelTable  *pTTL1;                    //linked list of L1 tables

    public:
        MidLevelTable() :Table()
        {
            pTTL1 = NULL;
        }
        MidLevelTable(GMM_PAGETABLEPool *Pool, int NodeIdx, SyncInfo Info) : MidLevelTable()
        {
            PoolElem = Pool;
            BBInfo = Info;
            PoolNodeIdx = NodeIdx;
        }
        ~MidLevelTable()
        {
            if (pTTL1)
            {
                LastLevelTable* item = pTTL1;

                while (item)
                {
                    LastLevelTable* nextItem = item->Next();
                    delete item;
                    item = nextItem;
                }

                pTTL1 = NULL;
            }
        }
        LastLevelTable* GetL1Table(GMM_GFX_SIZE_T L2eIdx, LastLevelTable** Prev = NULL)
        {
            LastLevelTable* pL1Tbl = pTTL1;
            LastLevelTable* PrevL1Tbl = NULL;

            while (pL1Tbl)
            {
                if (pL1Tbl->GetL2eIdx() == L2eIdx)
                {
                    break;
                }
                PrevL1Tbl = pL1Tbl;
                pL1Tbl = pL1Tbl->Next();
            }

            //if requested, save previous node in linked-list
            if (Prev)
            {
                *Prev = PrevL1Tbl;
            }
            return pL1Tbl;
        }
        void InsertInList(LastLevelTable* pL1Tbl)
        {
            LastLevelTable* Prev = pTTL1;

            //Insert at end
            while (Prev && Prev->Next())
            {
                Prev = Prev->Next();
            }

            if (Prev)
            {
                Prev->Next() = pL1Tbl;
            }
            else
            {
                pTTL1 = pL1Tbl;
            }
        }
        void DeleteFromList(LastLevelTable* pL1Tbl, LastLevelTable* PrevL1Tbl)
        {
            //Save next L1Table in list, before deleting current one
            if (pL1Tbl)
            {
                if (PrevL1Tbl)
                {
                    PrevL1Tbl->Next() = pL1Tbl->Next();
                }
                else
                {
                    pTTL1 = pL1Tbl->Next();
                }

                delete pL1Tbl;
            }
        }
    };

    /////////////////////////////////////////////////////
    /// Contains functions and members for PageTable. 
    /// PageTable defines multi-level pageTable 
    /////////////////////////////////////////////////////
    class PageTable :
        public GmmMemAllocator
    {
    protected:
        const TT_TYPE TTType;                      //PageTable is AuxTT
        const int NodesPerTable;                   //Aux L2/L3 has 32KB size, Aux L1 has 4KB -can't use as selector for PageTable is AuxTT
                                                   // 1 node for TR-table, 8 nodes for Aux-Table L2, 2 nodes for Aux-table L1
        //Root Table structure
        struct RootTable
        {
            GMM_RESOURCE_INFO* pGmmResInfo;
            HANDLE         L3Handle;
            GMM_GFX_ADDRESS  GfxAddress;              //L3 Table Adr CPU equivalent GPU addr
            GMM_GFX_ADDRESS  CPUAddress;              //LMEM-cpuvisible adr
            bool        NeedRegisterUpdate;        //True @ L3 allocation, False when L3AdrRegWrite done
            SyncInfo        BBInfo;
            RootTable() : pGmmResInfo(NULL), L3Handle(NULL), GfxAddress(0), CPUAddress(0), NeedRegisterUpdate(false), BBInfo() {}
        } TTL3;

        MidLevelTable*   pTTL2;                      //array of L2-Tables

    public:
#ifdef _WIN32
        CRITICAL_SECTION    TTLock;                  //synchronized access of PageTable obj
#elif defined __linux__
        pthread_mutex_t TTLock;
#endif

        GmmPageTableMgr*  PageTableMgr;
        GmmClientContext    *pClientContext;

        PageTable(int Size, int NumL3e, TT_TYPE flag) :
            NodesPerTable(Size / PAGE_SIZE),
            TTType(flag)
        {
            InitializeCriticalSection(&TTLock);

            pTTL2 = new MidLevelTable[NumL3e];
        }

        ~PageTable()
        {
            delete[] pTTL2;

            DeleteCriticalSection(&TTLock);
        }
       
        GMM_GFX_ADDRESS GetL3Address() { return TTL3.GfxAddress; }
        bool &GetRegisterStatus() { return TTL3.NeedRegisterUpdate; }
        GMM_STATUS AllocateL3Table(uint32_t L3TableSize, uint32_t L3AddrAlignment);
        GMM_STATUS DestroyL3Table();
        void AllocateL1L2Table(GMM_GFX_ADDRESS TileAddr, GMM_GFX_ADDRESS * L1TableAdr, GMM_GFX_ADDRESS * L2TableAdr);
        void AllocateDummyTables(GmmLib::Table **L2Table, GmmLib::Table **L1Table);
        void GetL1L2TableAddr(GMM_GFX_ADDRESS TileAddr, GMM_GFX_ADDRESS * L1TableAdr, GMM_GFX_ADDRESS* L2TableAdr);
        uint8_t GetMappingType(GMM_GFX_ADDRESS GfxVA, GMM_GFX_SIZE_T Size, GMM_GFX_ADDRESS& LastAddr);
        HANDLE GetL3Handle() { return TTL3.L3Handle; }
    };

    //////////////////////////////////////////////////////////////////////////////////////////////
    /// Contains functions and members for AuxTable. 
    /// AuxTable defines PageTable for translating VA->AuxVA, ie defines page-walk to get address
    /// of CCS-cacheline containing auxiliary data (compression tag, etc) for some resource
    /////////////////////////////////////////////////////////////////////////////////////////////
    class AuxTable : public PageTable
    {
    public:
        const int L1Size;
        Table* NullL2Table;
        Table* NullL1Table;
        GMM_GFX_ADDRESS NullCCSTile;
        AuxTable() : PageTable(8 * PAGE_SIZE, GMM_AUX_L3_SIZE, TT_TYPE::AUXTT),
            L1Size(2 * PAGE_SIZE)
        {
            NullL2Table = nullptr;
            NullL1Table = nullptr;
            NullCCSTile = 0;
        }
        GMM_STATUS InvalidateTable(GMM_UMD_SYNCCONTEXT * UmdContext, GMM_GFX_ADDRESS BaseAdr, GMM_GFX_SIZE_T Size, uint8_t DoNotWait);

        GMM_STATUS MapValidEntry(GMM_UMD_SYNCCONTEXT *UmdContext, GMM_GFX_ADDRESS BaseAdr, GMM_GFX_SIZE_T BaseSize,
                                 GMM_RESOURCE_INFO* BaseResInfo, GMM_GFX_ADDRESS AuxVA, GMM_RESOURCE_INFO* AuxResInfo, uint64_t PartialData, uint8_t DoNotWait);

        GMM_STATUS MapNullCCS(GMM_UMD_SYNCCONTEXT *UmdContext, GMM_GFX_ADDRESS BaseAdr, GMM_GFX_SIZE_T Size, uint64_t PartialL1e, uint8_t DoNotWait);

        GMM_AUXTTL1e CreateAuxL1Data(GMM_RESOURCE_INFO* BaseResInfo);
        GMM_GFX_ADDRESS GMM_INLINE __GetCCSCacheline(GMM_RESOURCE_INFO* BaseResInfo, GMM_GFX_ADDRESS BaseAdr, GMM_RESOURCE_INFO* AuxResInfo,
                                                     GMM_GFX_ADDRESS AuxVA, GMM_GFX_SIZE_T AdrOffset);

    };

typedef struct _GMM_DEVICE_ALLOC {
    uint32_t            Size;
    uint32_t            Alignment;
    HANDLE              Handle;
    GMM_GFX_ADDRESS     GfxVA;
    GMM_GFX_ADDRESS     CPUVA;
    void *              Priv;
    HANDLE              hCsr;
} GMM_DEVICE_ALLOC;

typedef struct _GMM_DEVICE_DEALLOC {
    HANDLE              Handle;
    GMM_GFX_ADDRESS     GfxVA;
    void *              Priv;
    HANDLE              hCsr;
} GMM_DEVICE_DEALLOC;

GMM_STATUS __GmmDeviceAlloc(GmmClientContext            *pClientContext,
                            GMM_DEVICE_CALLBACKS_INT    *pDeviceCbInt,
                            GMM_DEVICE_ALLOC            *pAlloc);

GMM_STATUS __GmmDeviceDealloc(GMM_CLIENT                ClientType,
                              GMM_DEVICE_CALLBACKS_INT  *DeviceCb,
                              GMM_DEVICE_DEALLOC        *pDealloc);

}
#endif  // #ifdef __cplusplus

