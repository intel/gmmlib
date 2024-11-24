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

Description: AUX-Table management functions 
             (contains functions to assign memory to 
			 AUX-Tables with valid entries, 
			 and update their entries on request)

============================================================================*/

#include "Internal/Common/GmmLibInc.h"
#include "../TranslationTable/GmmUmdTranslationTable.h"

#if !defined(__GMM_KMD__)

//=============================================================================
//
// Function: MapNullCCS
//
// Desc: Maps given resource, with dummy null-ccs chain, on Aux Table
//
// Caller: UpdateAuxTable (map op for null-tiles)
//
// Parameters:
//      UmdContext: Caller-thread specific info (regarding BB for TR-Aux udpate, cmdQ to use etc)
//      BaseAdr: Start adr of main surface
//      Size:   Main-surface size in bytes
//      PartialL1e: Aux-metadata other than AuxVA
//      DoNotWait: 1 for CPU update, 0 for async(Gpu) update
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::AuxTable::MapNullCCS(GMM_UMD_SYNCCONTEXT *UmdContext, GMM_GFX_ADDRESS BaseAdr, GMM_GFX_SIZE_T Size, uint64_t PartialL1e, uint8_t DoNotWait)
{
    GMM_STATUS      Status       = GMM_SUCCESS;
    GMM_GFX_SIZE_T  L1TableSize  = ((GMM_GFX_SIZE_T)GMM_L1_SIZE(AUXTT, GetGmmLibContext())) * (WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : GMM_KBYTE(64)); // TGL and above : L1TableSize =  256x64K OR 16x1M
    GMM_GFX_ADDRESS Addr         = 0;
    GMM_GFX_ADDRESS L3GfxAddress = 0;
    GMM_CLIENT      ClientType;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);

    EnterCriticalSection(&TTLock);

    DoNotWait |= (!UmdContext || !UmdContext->pCommandQueueHandle);

    if(TTL3.L3Handle)
    {
        L3GfxAddress = TTL3.GfxAddress;
    }
    else
    {
        LeaveCriticalSection(&TTLock);
        return GMM_ERROR;
    }

    if(!DoNotWait)
    {
        PageTableMgr->TTCb.pfPrologTranslationTable(
        UmdContext->pCommandQueueHandle);
    }

    // For each L1 table
    for(Addr = GFX_ALIGN_FLOOR(BaseAdr, L1TableSize); // Start at begining of L1 table
        Addr < BaseAdr + Size;
        Addr += L1TableSize) // Increment by 1 L1 table
    {
        GMM_GFX_ADDRESS L1GfxAddress, L2GfxAddress;
        GMM_GFX_ADDRESS L1CPUAddress, L2CPUAddress;
        GMM_GFX_ADDRESS StartAddress = 0;
        GMM_GFX_ADDRESS EndAddress   = 0;
        GMM_GFX_ADDRESS TileAddr     = 0;
        GMM_GFX_SIZE_T  L2eIdx       = 0;

        StartAddress = Addr < BaseAdr ? BaseAdr : Addr;
        EndAddress   = Addr + L1TableSize;
        if(EndAddress > BaseAdr + Size)
        {
            EndAddress = BaseAdr + Size;
        }

        GetL1L2TableAddr(StartAddress,
                         &L1GfxAddress,
                         &L2GfxAddress);

        // If tables are not there, then they are already invalidated as part of
        // AUX-TT initialization or other APIs.
        if(L2GfxAddress == GMM_NO_TABLE ||
           L1GfxAddress == GMM_NO_TABLE)
        {
            //Clear Valid-bit for L3Entry or L2Entry
            uint64_t        Data            = 0;
            GMM_GFX_ADDRESS TableGfxAddress = (L2GfxAddress == GMM_NO_TABLE) ? L3GfxAddress : L2GfxAddress;
            GMM_GFX_ADDRESS TableCPUAddress = (L2GfxAddress == GMM_NO_TABLE) ? TTL3.CPUAddress : pTTL2[GMM_L3_ENTRY_IDX(AUXTT, StartAddress)].GetCPUAddress();
            uint32_t        TableEntryIdx   = (L2GfxAddress == GMM_NO_TABLE) ? static_cast<uint32_t>(GMM_L3_ENTRY_IDX(AUXTT, StartAddress)) : static_cast<uint32_t>(GMM_L2_ENTRY_IDX(AUXTT, StartAddress));
            L2CPUAddress                    = (L2GfxAddress == GMM_NO_TABLE) ? 0 : TableCPUAddress;

            if(!NullL1Table || !NullL2Table)
            {
                AllocateDummyTables(&NullL2Table, &NullL1Table);
                if(!NullL1Table || !NullL2Table)
                {
                    //report error
                    LeaveCriticalSection(&TTLock);
                    return GMM_OUT_OF_MEMORY;
                }
                else
                {
                    //Initialize dummy table entries (one-time)
                    GMM_GFX_ADDRESS TableAddr = NullL2Table->GetCPUAddress();
                    GMM_AUXTTL2e    L2e       = {0};
                    L2e.Valid                 = 1;
                    GMM_TO_AUX_L2e_L1GFXADDR_2((NullL1Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL1Table->GetNodeIdx()), L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext()))) // populate L2e.L1GfxAddr
                    for(int i = 0; i < GMM_AUX_L2_SIZE; i++)
                    {
                        //initialize L2e ie clear Valid bit for all entries
                        ((GMM_AUXTTL2e *)TableAddr)[i].Value = L2e.Value;
                    }

                    TableAddr = NullL1Table->GetCPUAddress();

                    GMM_AUXTTL1e L1e = {0};
                    L1e.Valid        = 1;
                    if(!WA64K(GetGmmLibContext()))
                    {
                        L1e.GfxAddress = (NullCCSTile >> 12); /*********** 4kb-aligned CCS adr *****/
                    }
                    else
                    {
                        L1e.Reserved4  = (NullCCSTile >> 8);  /*********** 4 lsbs of 256B-aligned CCS adr *****/
                        L1e.GfxAddress = (NullCCSTile >> 12); /*********** 4kb-aligned CCS adr *****/
                    }

		    for(int i = 0; i < GMM_AUX_L1_SIZE(GetGmmLibContext()); i++)
                    {
                        //initialize L1e with null ccs tile
                        ((GMM_AUXTTL1e *)TableAddr)[i].Value = L1e.Value;
                    }
                }
            }

            if(L2GfxAddress == GMM_NO_TABLE)
            {
                GMM_AUXTTL3e L3e = {0};
                L3e.Valid        = 1;
                L3e.L2GfxAddr    = (NullL2Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL2Table->GetNodeIdx()) >> 15;
                Data             = L3e.Value;
            }
            else
            {
                GMM_AUXTTL2e L2e = {0};
                L2e.Valid        = 1;
                GMM_TO_AUX_L2e_L1GFXADDR_2((NullL1Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL1Table->GetNodeIdx()), L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
                Data = L2e.Value;
	    }

            if(DoNotWait)
            {
                //Sync update on CPU
                ((GMM_AUXTTL2e *)TableCPUAddress)[TableEntryIdx].Value = Data;
            }
            else
            {
                if(L2GfxAddress != GMM_NO_TABLE)
                {
                    pTTL2[GMM_L3_ENTRY_IDX(AUXTT, StartAddress)].UpdatePoolFence(UmdContext, false);
                }
                PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                    TableGfxAddress + TableEntryIdx * GMM_AUX_L2e_SIZE,
                                                    Data);
            }
            continue;
        }
        else
        {
            uint32_t L3eIdx = static_cast<uint32_t>(GMM_L3_ENTRY_IDX(AUXTT, StartAddress));
            L2CPUAddress    = pTTL2[L3eIdx].GetCPUAddress();

            L2eIdx = GMM_L2_ENTRY_IDX(AUXTT, StartAddress);
            if(DoNotWait)
            {
                //Sync update on CPU
                ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].Valid     = 1; //set Valid bit
                ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].L2GfxAddr = L2GfxAddress >> 15;

                ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx].Valid = 1; //set Valid bit
                GMM_TO_AUX_L2e_L1GFXADDR_2(L1GfxAddress, ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx], (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
	    }
            else
            {
                GMM_AUXTTL3e L3e = {0};
                L3e.Valid        = 1;
                L3e.L2GfxAddr    = L2GfxAddress >> 15;
                PageTableMgr->TTCb.pfWriteL2L3Entry(
                UmdContext->pCommandQueueHandle,
                L3GfxAddress + (L3eIdx * GMM_AUX_L3e_SIZE),
                L3e.Value);

                pTTL2[L3eIdx].UpdatePoolFence(UmdContext, false);

                GMM_AUXTTL2e L2e = {0};
                L2e.Valid        = 1;
                GMM_TO_AUX_L2e_L1GFXADDR_2(L1GfxAddress, L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
		PageTableMgr->TTCb.pfWriteL2L3Entry(
                UmdContext->pCommandQueueHandle,
                L2GfxAddress + (L2eIdx * GMM_AUX_L2e_SIZE),
                L2e.Value);
            }
        }

        // For each 64KB or 16KB of main surface (entry) in L1 table
        for(TileAddr = StartAddress;
            TileAddr < EndAddress;
            TileAddr += (WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : WA64K(GetGmmLibContext()) ? GMM_KBYTE(64) : GMM_MBYTE(1)))
        {
            uint64_t                Data   = PartialL1e | NullCCSTile | __BIT(0);
            GMM_GFX_SIZE_T          L1eIdx = GMM_L1_ENTRY_IDX(AUXTT, TileAddr, GetGmmLibContext());
            GmmLib::LastLevelTable *pL1Tbl = NULL;

            pL1Tbl       = pTTL2[GMM_AUX_L3_ENTRY_IDX(TileAddr)].GetL1Table(L2eIdx, NULL);
            L1CPUAddress = pL1Tbl->GetCPUAddress();
            if(DoNotWait)
            {
                //Sync update on CPU
                ((GMM_AUXTTL1e *)L1CPUAddress)[L1eIdx].Value = Data;
                GMM_DPF(GFXDBG_NORMAL, "##### Null-Map | Table Entry:  TileAddress[0x%llX] L2eIdx[%d]  :: L1eIdx[%d] L1Addr[0x%llX] L1Value[00x%llX]\n", TileAddr, L2eIdx, L1eIdx, &((GMM_AUXTTL1e *)L1CPUAddress)[L1eIdx], Data);
            }
            else
            {
                pL1Tbl->UpdatePoolFence(UmdContext, false);

                /*                PageTableMgr->TTCb.pfWriteL1Entries(
                    UmdContext->pCommandQueueHandle,
                    2,
                    L1GfxAddress + (L1eIdx * GMM_AUX_L1e_SIZE),
                    (uint32_t*)(&Data));*/ //**********REQUIRE UMD CHANGE TO UPDATE 64-bit ENTRY - both DWORDs must be updated atomically*******/
                PageTableMgr->TTCb.pfWriteL2L3Entry(
                UmdContext->pCommandQueueHandle,
                L1GfxAddress + (L1eIdx * GMM_AUX_L1e_SIZE),
                Data);
            }

            if(pL1Tbl->TrackTableUsage(AUXTT, true, TileAddr, true, GetGmmLibContext()))
            { // L1 Table is not being used anymore
                GMM_AUXTTL2e               L2e      = {0};
                GmmLib::GMM_PAGETABLEPool *PoolElem = NULL;
                GmmLib::LastLevelTable *   pL1Tbl = NULL, *Prev = NULL;

                pL1Tbl = pTTL2[GMM_L3_ENTRY_IDX(AUXTT, TileAddr)].GetL1Table(L2eIdx, &Prev);
                // Map L2-entry to Null-L1Table
                L2e.Valid = 1;
                GMM_TO_AUX_L2e_L1GFXADDR_2((NullL1Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL1Table->GetNodeIdx()), L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext()))) // populate L2e.L1GfxAddress/Le2.Reserved2
		if(DoNotWait)
                {
                    //Sync update on CPU
                    ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx].Value = L2e.Value;
                }
                else
                {
                    pTTL2[GMM_L3_ENTRY_IDX(AUXTT, TileAddr)].UpdatePoolFence(UmdContext, false);
                    PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                        L2GfxAddress + L2eIdx * GMM_AUX_L2e_SIZE,
                                                        L2e.Value);
                }
                //Update usage for PoolNode assigned to L1Table, and free L1Tbl
                if(pL1Tbl)
                {
                    PoolElem = pL1Tbl->GetPool();
                    if(PoolElem)
                    {
                        if(pL1Tbl->GetBBInfo().BBQueueHandle)
                        {
                            PoolElem->GetNodeBBInfoAtIndex(pL1Tbl->GetNodeIdx()) = pL1Tbl->GetBBInfo();
                        }
                        DEASSIGN_POOLNODE(PageTableMgr, UmdContext, PoolElem, pL1Tbl->GetNodeIdx(), AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetGmmLibContext()))
                    }
                    pTTL2[GMM_L3_ENTRY_IDX(AUXTT, TileAddr)].DeleteFromList(pL1Tbl, Prev);
                }

                // The L1 table is unused -- meaning everything else in this table is
                // already invalid. So, break early.
                break;
            }
        }
    }

    if(!DoNotWait)
    {
        PageTableMgr->TTCb.pfEpilogTranslationTable(
        UmdContext->pCommandQueueHandle,
        1); // ForceFlush
    }
    LeaveCriticalSection(&TTLock);

    return Status;
}

//=============================================================================
//
// Function: InvalidateTable (InvalidateMappings)
//
// Desc: Unmaps given resource from Aux Table; and marks affected entries as invalid
//
// Caller: UpdateAuxTable (unmap op)
//
// Parameters:
//      UmdContext: Caller-thread specific info (regarding BB for Aux udpate, cmdQ to use etc)
//      BaseAdr: Start adr of main surface
//      Size:   Main-surface size in bytes? (or take GmmResInfo?)
//      DoNotWait: 1 for CPU update, 0 for async(Gpu) update
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::AuxTable::InvalidateTable(GMM_UMD_SYNCCONTEXT *UmdContext, GMM_GFX_ADDRESS BaseAdr, GMM_GFX_SIZE_T Size, uint8_t DoNotWait)
{
    GMM_STATUS      Status       = GMM_SUCCESS;
    GMM_GFX_SIZE_T  L1TableSize  = ((GMM_GFX_SIZE_T)GMM_L1_SIZE(AUXTT, GetGmmLibContext())) * (WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : GMM_KBYTE(64)); //Each AuxTable entry maps 16K main-surface
    GMM_GFX_ADDRESS Addr         = 0;
    GMM_GFX_ADDRESS L3GfxAddress = 0;
    uint8_t         isTRVA       = 0; 

    GMM_CLIENT ClientType;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);

    //NullCCSTile isn't initialized, disable TRVA path
    isTRVA = (NullCCSTile ? isTRVA : 0);

    EnterCriticalSection(&TTLock);

    DoNotWait |= (!UmdContext || !UmdContext->pCommandQueueHandle);

    if(TTL3.L3Handle)
    {
        L3GfxAddress = TTL3.GfxAddress;
    }
    else
    {
        LeaveCriticalSection(&TTLock);
        return GMM_ERROR;
    }

    if(!DoNotWait)
    {
        PageTableMgr->TTCb.pfPrologTranslationTable(
        UmdContext->pCommandQueueHandle);
    }

    // For each L1 table
    for(Addr = GFX_ALIGN_FLOOR(BaseAdr, L1TableSize); // Start at begining of L1 table
        Addr < BaseAdr + Size;
        Addr += L1TableSize) // Increment by 1 L1 table
    {
        GMM_GFX_ADDRESS L1GfxAddress, L2GfxAddress;
        GMM_GFX_ADDRESS L1CPUAddress, L2CPUAddress;
        GMM_GFX_ADDRESS StartAddress = 0;
        GMM_GFX_ADDRESS EndAddress   = 0;
        GMM_GFX_ADDRESS TileAddr     = 0;
        GMM_GFX_SIZE_T  L2eIdx       = 0;

        StartAddress = Addr < BaseAdr ? BaseAdr : Addr;
        EndAddress   = Addr + L1TableSize;
        if(EndAddress > BaseAdr + Size)
        {
            EndAddress = BaseAdr + Size;
        }

        GetL1L2TableAddr(StartAddress,
                         &L1GfxAddress,
                         &L2GfxAddress);

        // If tables are not there, then they are already invalidated as part of
        // AUX-TT initialization or other APIs.
        if(L2GfxAddress == GMM_NO_TABLE ||
           L1GfxAddress == GMM_NO_TABLE)
        {
            //Clear Valid-bit for L3Entry or L2Entry
            GMM_AUXTTL2e    L2e             = {0}; //AUXTT L3e is identical to L2e, reuse.
            GMM_GFX_ADDRESS TableGfxAddress = (L2GfxAddress == GMM_NO_TABLE) ? L3GfxAddress : L2GfxAddress;
            GMM_GFX_ADDRESS TableCPUAddress = (L2GfxAddress == GMM_NO_TABLE) ? TTL3.CPUAddress : pTTL2[GMM_L3_ENTRY_IDX(AUXTT, StartAddress)].GetCPUAddress();
            uint32_t        TableEntryIdx   = (L2GfxAddress == GMM_NO_TABLE) ? static_cast<uint32_t>(GMM_L3_ENTRY_IDX(AUXTT, StartAddress)) : static_cast<uint32_t>(GMM_L2_ENTRY_IDX(AUXTT, StartAddress));
            L2CPUAddress                    = (L2GfxAddress == GMM_NO_TABLE) ? 0 : TableCPUAddress;

            if(isTRVA && NullL2Table && NullL1Table)
            {
                //invalidate if request spans entire stretch ie TileAdr aligns L1TableSize*GMM_L2_SIZE
                uint64_t Data = 0;
                if(L2GfxAddress == GMM_NO_TABLE)
                {
                    GMM_AUXTTL3e L3e = {0};
                    L3e.Valid        = 1;
                    L3e.L2GfxAddr    = (NullL2Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL2Table->GetNodeIdx()) >> 15;
                    Data             = L3e.Value;
                }
                else
                {
                    GMM_AUXTTL2e L2e = {0};
                    L2e.Valid        = 1;
                    GMM_TO_AUX_L2e_L1GFXADDR_2((NullL1Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL1Table->GetNodeIdx()), L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
                    Data = L2e.Value;
                }
                L2e.Value = Data;
            }
            else
            {
                L2e.Valid = 0;
            }
            if(DoNotWait)
            {
                //Sync update on CPU
                ((GMM_AUXTTL2e *)TableCPUAddress)[TableEntryIdx].Value = L2e.Value;
            }
            else
            {
                if(L2GfxAddress != GMM_NO_TABLE)
                {
                    pTTL2[GMM_L3_ENTRY_IDX(AUXTT, StartAddress)].UpdatePoolFence(UmdContext, false);
                }
                PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                    TableGfxAddress + TableEntryIdx * GMM_AUX_L2e_SIZE,
                                                    L2e.Value);
            }
            continue;
        }
        else
        {
            uint32_t L3eIdx = static_cast<uint32_t>(GMM_L3_ENTRY_IDX(AUXTT, StartAddress));
            L2CPUAddress    = pTTL2[L3eIdx].GetCPUAddress();

            L2eIdx = GMM_L2_ENTRY_IDX(AUXTT, StartAddress);
            if(DoNotWait)
            {
                //Sync update on CPU
                ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].Valid     = 1; //set Valid bit
                ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].L2GfxAddr = L2GfxAddress >> 15;

                ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx].Valid = 1; //set Valid bit
                GMM_TO_AUX_L2e_L1GFXADDR_2(L1GfxAddress, ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx], (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
	    }
            else
            {
                GMM_AUXTTL3e L3e = {0};
                L3e.Valid        = 1;
                L3e.L2GfxAddr    = L2GfxAddress >> 15;
                PageTableMgr->TTCb.pfWriteL2L3Entry(
                UmdContext->pCommandQueueHandle,
                L3GfxAddress + (L3eIdx * GMM_AUX_L3e_SIZE),
                L3e.Value);

                pTTL2[L3eIdx].UpdatePoolFence(UmdContext, false);

                GMM_AUXTTL2e L2e = {0};
                L2e.Valid        = 1;
                GMM_TO_AUX_L2e_L1GFXADDR_2(L1GfxAddress, L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
		PageTableMgr->TTCb.pfWriteL2L3Entry(
                UmdContext->pCommandQueueHandle,
                L2GfxAddress + (L2eIdx * GMM_AUX_L2e_SIZE),
                L2e.Value);
            }
        }

        // For each 64KB or 16KB or 1MB of main surface (entry) in L1 table
        for(TileAddr = StartAddress;
            TileAddr < EndAddress;
            TileAddr += (WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : WA64K(GetGmmLibContext()) ? GMM_KBYTE(64) : GMM_MBYTE(1)))
        {
            //Invalidation of requested range irrespective of TRVA
            uint64_t                Data   = GMM_INVALID_AUX_ENTRY;
            GMM_GFX_SIZE_T          L1eIdx = GMM_L1_ENTRY_IDX(AUXTT, TileAddr, GetGmmLibContext());
            GmmLib::LastLevelTable *pL1Tbl = NULL;

            pL1Tbl       = pTTL2[GMM_AUX_L3_ENTRY_IDX(TileAddr)].GetL1Table(L2eIdx, NULL);
            L1CPUAddress = pL1Tbl->GetCPUAddress();
            if(DoNotWait)
            {
                //Sync update on CPU
                ((GMM_AUXTTL1e *)L1CPUAddress)[L1eIdx].Value = Data;
                GMM_DPF(GFXDBG_NORMAL, "~~UnMap | Table Entry: L2addressBase[0x%llX] :: L2Valid[%d] :: L2eidx[%d]  L1addressBase[0x%llX] :: L1eidx[%d]  L1Valid[%d] :: DerivedCCS[0x%llX] ", (GMM_AUXTTL2e *)L2CPUAddress, ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx].Valid, L2eIdx, GMM_L1TABLE_ADDR_FROM_AUX_L2e_L1GFXADDR(((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx], true), L1eIdx, ((GMM_AUXTTL1e *)L1CPUAddress)[L1eIdx].Valid, (((GMM_AUXTTL1e *)L1CPUAddress)[L1eIdx].GfxAddress << 12));

            }
            else
            {
                pL1Tbl->UpdatePoolFence(UmdContext, false);

                /*                PageTableMgr->TTCb.pfWriteL1Entries(
                    UmdContext->pCommandQueueHandle,
                    2,
                    (uint32_t*)(&Data));*/
                //**********REQUIRE UMD CHANGE TO UPDATE 64-bit ENTRY - both DWORDs must be updated atomically*******/
                PageTableMgr->TTCb.pfWriteL2L3Entry(
                UmdContext->pCommandQueueHandle,
                L1GfxAddress + (L1eIdx * GMM_AUX_L1e_SIZE),
                Data);
            }

            if(pL1Tbl->TrackTableUsage(AUXTT, true, TileAddr, true, GetGmmLibContext()))
            { // L1 Table is not being used anymore
                GMM_AUXTTL2e               L2e      = {0};
                GmmLib::GMM_PAGETABLEPool *PoolElem = NULL;
                GmmLib::LastLevelTable *   pL1Tbl = NULL, *Prev = NULL;

                pL1Tbl = pTTL2[GMM_L3_ENTRY_IDX(AUXTT, TileAddr)].GetL1Table(L2eIdx, &Prev);

                if(isTRVA && NullL1Table &&
                   ((TileAddr > GFX_ALIGN_FLOOR(BaseAdr, L1TableSize) && TileAddr < GFX_ALIGN_NP2(BaseAdr, L1TableSize)) ||
                    (TileAddr > GFX_ALIGN_FLOOR(BaseAdr + Size, L1TableSize) && TileAddr < GFX_ALIGN_NP2(BaseAdr + Size, L1TableSize))))
                {
                    //Invalidation affects entries out of requested range, null-map for TR
                    L2e.Valid = 1;
                    GMM_TO_AUX_L2e_L1GFXADDR_2((NullL1Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL1Table->GetNodeIdx()), L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
		}
                else
                {
                    // Clear valid bit of L2 entry
                    L2e.Valid                                    = 0;
                    ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx].Valid = 0;
                }
                if(DoNotWait)
                {
                    //Sync update on CPU
                    ((GMM_AUXTTL2e *)L2CPUAddress)[L2eIdx].Value = L2e.Value;
                }
                else
                {
                    pTTL2[GMM_L3_ENTRY_IDX(AUXTT, TileAddr)].UpdatePoolFence(UmdContext, false);
                    PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                        L2GfxAddress + L2eIdx * GMM_AUX_L2e_SIZE,
                                                        L2e.Value);
                }
                //Update usage for PoolNode assigned to L1Table, and free L1Tbl
                if(pL1Tbl)
                {
                    PoolElem = pL1Tbl->GetPool();
                    if(PoolElem)
                    {
                        if(pL1Tbl->GetBBInfo().BBQueueHandle)
                        {
                            PoolElem->GetNodeBBInfoAtIndex(pL1Tbl->GetNodeIdx()) = pL1Tbl->GetBBInfo();
                        }
                        DEASSIGN_POOLNODE(PageTableMgr, UmdContext, PoolElem, pL1Tbl->GetNodeIdx(), AUX_L1TABLE_SIZE_IN_POOLNODES_2(GetGmmLibContext()))
		    }
                    pTTL2[GMM_L3_ENTRY_IDX(AUXTT, TileAddr)].DeleteFromList(pL1Tbl, Prev);
                }

                // The L1 table is unused -- meaning everything else in this table is
                // already invalid. So, break early.
                break;
            }
        }
    }

    if(!DoNotWait)
    {
        PageTableMgr->TTCb.pfEpilogTranslationTable(
        UmdContext->pCommandQueueHandle,
        1); // ForceFlush
    }

    LeaveCriticalSection(&TTLock);

    return Status;
}

//=============================================================================
//
// Function: MapValidEntry
//
// Desc: Maps given main-surface, on Aux-Table, to get the exact CCS cacheline tied to
//       different 4x4K pages of main-surface
//
// Caller: UpdateAuxTable (map op)
//
// Parameters:
//      UmdContext: ptr to thread-data
//      BaseAdr: Start adr of main-surface
//      BaseSize:   main-surface Size in bytes
//      BaseResInfo: main surface ResInfo
//      AuxVA: Start adr of Aux-surface
//      AuxResInfo: Aux surface ResInfo
//      PartialData: Aux L1 partial data (ie w/o address)
//      DoNotWait: true for CPU update, false for async(Gpu) update
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::AuxTable::MapValidEntry(GMM_UMD_SYNCCONTEXT *UmdContext, GMM_GFX_ADDRESS BaseAdr, GMM_GFX_SIZE_T BaseSize,
                                           GMM_RESOURCE_INFO *BaseResInfo, GMM_GFX_ADDRESS AuxVA, GMM_RESOURCE_INFO *AuxResInfo, uint64_t PartialData, uint8_t DoNotWait)
{
    GMM_STATUS      Status = GMM_SUCCESS;
    GMM_GFX_ADDRESS Addr = 0, L3TableAdr = GMM_NO_TABLE;
    GMM_GFX_SIZE_T  L1TableSize = GMM_AUX_L1_SIZE(GetGmmLibContext()) * (WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : GMM_KBYTE(64)); // L1TableSize maps to 16MB address space for TGL and above: 256x64k | 16x1MB
    GMM_GFX_SIZE_T  CCS$Adr     = AuxVA;
    uint8_t         isTRVA    =0  ;

    GMM_CLIENT ClientType;

    GET_GMM_CLIENT_TYPE(pClientContext, ClientType);

    //NullCCSTile isn't initialized, disable TRVA path
    isTRVA = (NullCCSTile ? isTRVA : 0);

    EnterCriticalSection(&TTLock);
    if(!TTL3.L3Handle || (!DoNotWait && !UmdContext))
    {
        Status = GMM_ERROR;
    }
    else
    {
        L3TableAdr = TTL3.GfxAddress;

        if(!DoNotWait)
        {
            PageTableMgr->TTCb.pfPrologTranslationTable(UmdContext->pCommandQueueHandle);
        }

        // GMM_DPF(GFXDBG_CRITICAL, "Mapping surface: GPUVA=0x%016llX Size=0x%08X Aux_GPUVA=0x%016llX\n", BaseAdr, BaseSize, AuxVA);
        for(Addr = GFX_ALIGN_FLOOR(BaseAdr, L1TableSize); Addr < BaseAdr + BaseSize; Addr += L1TableSize)
        {
            GMM_GFX_ADDRESS StartAdr, EndAdr, TileAdr;
            GMM_GFX_ADDRESS L1TableAdr = GMM_NO_TABLE, L2TableAdr = GMM_NO_TABLE;
            GMM_GFX_ADDRESS L1TableCPUAdr = GMM_NO_TABLE, L2TableCPUAdr = GMM_NO_TABLE;
            GMM_GFX_SIZE_T  L2eIdx     = 0;
            GMM_GFX_SIZE_T  L3eIdx     = 0;
            bool            AllocateL1 = false, AllocateL2 = false;

            EndAdr   = Addr + L1TableSize;
            EndAdr   = EndAdr > BaseAdr + BaseSize ? BaseAdr + BaseSize : EndAdr;
            StartAdr = Addr < BaseAdr ? BaseAdr : Addr;

            L2eIdx = GMM_L2_ENTRY_IDX(AUXTT, StartAdr);
            L3eIdx = GMM_L3_ENTRY_IDX(AUXTT, StartAdr);

            //Allocate L2/L1 Table -- get L2 Table Adr for <StartAdr,EndAdr>
            GetL1L2TableAddr(Addr, &L1TableAdr, &L2TableAdr);
            if(L2TableAdr == GMM_NO_TABLE || L1TableAdr == GMM_NO_TABLE)
            {
                AllocateL1 = GMM_NO_TABLE == L1TableAdr;
                AllocateL2 = GMM_NO_TABLE == L2TableAdr;
                AllocateL1L2Table(Addr, &L1TableAdr, &L2TableAdr);

                if(L2TableAdr == GMM_NO_TABLE || L1TableAdr == GMM_NO_TABLE)
                {
                    LeaveCriticalSection(&TTLock);
                    return GMM_OUT_OF_MEMORY;
                }

                if(AllocateL2)
                {
                    uint32_t     i = 0;
                    GMM_AUXTTL2e InvalidEntry;
                    InvalidEntry.Value = 0;
                    if(isTRVA && NullL1Table)
                    {
                        InvalidEntry.Valid = 1;
                        GMM_TO_AUX_L2e_L1GFXADDR_2((NullL1Table->GetPool()->GetGfxAddress() + PAGE_SIZE * NullL1Table->GetNodeIdx()), InvalidEntry, (!WA16K(GetGmmLibContext()) && !WA64K(GetGmmLibContext())))
		    }

                    if(DoNotWait)
                    {
                        L2TableCPUAdr = pTTL2[L3eIdx].GetCPUAddress();

                        ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].Value     = 0;
                        ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].L2GfxAddr = L2TableAdr >> 15;
                        ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].Valid     = 1;
                        for(i = 0; i < GMM_AUX_L2_SIZE; i++)
                        {
                            //initialize L2e ie clear Valid bit for all entries
                            ((GMM_AUXTTL2e *)L2TableCPUAdr)[i].Value = InvalidEntry.Value;
                        }
                    }
                    else
                    {
                        GMM_AUXTTL3e L3e = {0};
                        L3e.Valid        = 1;
                        L3e.L2GfxAddr    = L2TableAdr >> 15;
                        PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                            L3TableAdr + L3eIdx * GMM_AUX_L3e_SIZE,
                                                            L3e.Value);

                        //initialize L2e ie clear valid bit for all entries
                        for(i = 0; i < GMM_AUX_L2_SIZE; i++)
                        {
                            PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                                L2TableAdr + i * GMM_AUX_L2e_SIZE,
                                                                InvalidEntry.Value);
                        }
                    }
                }

                if(AllocateL1)
                {
                    uint64_t InvalidEntry = (!isTRVA) ? GMM_INVALID_AUX_ENTRY : (NullCCSTile | __BIT(0));
                    uint32_t i            = 0;

                    if(DoNotWait)
                    {
                        GmmLib::LastLevelTable *pL1Tbl = NULL;
                        pL1Tbl                         = pTTL2[L3eIdx].GetL1Table(L2eIdx, NULL);
                        L2TableCPUAdr = pTTL2[L3eIdx].GetCPUAddress();
                        L1TableCPUAdr = pL1Tbl->GetCPUAddress();
                        //Sync update on CPU
                        ((GMM_AUXTTL2e *)L2TableCPUAdr)[L2eIdx].Value = 0;
                        GMM_TO_AUX_L2e_L1GFXADDR_2(L1TableAdr, ((GMM_AUXTTL2e *)L2TableCPUAdr)[L2eIdx], (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext()))) // populate L2e.L1GfxAddr
                        ((GMM_AUXTTL2e *)L2TableCPUAdr)[L2eIdx]
                        .Valid = 1;
                        for(i = 0; i < (uint32_t)GMM_AUX_L1_SIZE(GetGmmLibContext()); i++)
                        {
                            //initialize L1e ie mark all entries with Null tile value
                            ((GMM_AUXTTL1e *)L1TableCPUAdr)[i].Value = InvalidEntry;
                        }
                    }
                    else
                    {
                        GMM_AUXTTL2e L2e = {0};
                        L2e.Valid        = 1;
                        GMM_TO_AUX_L2e_L1GFXADDR_2(L1TableAdr, L2e, (!WA64K(GetGmmLibContext()) && !WA16K(GetGmmLibContext())))
                        pTTL2[L3eIdx]
                        .UpdatePoolFence(UmdContext, false);
                        PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                            L2TableAdr + L2eIdx * GMM_AUX_L2e_SIZE,
                                                            L2e.Value);

                        //initialize all L1e with invalid entries
                        for(i = 0; i < (uint32_t)GMM_AUX_L1_SIZE(GetGmmLibContext()); i++)
                        {
                            PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                                L1TableAdr + i * sizeof(uint64_t),
                                                                InvalidEntry);
                        }
                    }
                }
            }
   
	    //GMM_DPF(GFXDBG_NORMAL, "Mapping surface: GPUVA=0x%016llx Size=0x%08x Aux_GPUVA=0x%016llx", StartAdr, BaseSize, CCS$Adr);
            for(TileAdr = StartAdr; TileAdr < EndAdr; TileAdr += (WA16K(GetGmmLibContext()) ? GMM_KBYTE(16) : WA64K(GetGmmLibContext()) ? GMM_KBYTE(64) : GMM_MBYTE(1)),
            CCS$Adr += (pClientContext->GetLibContext()->GetSkuTable().FtrLinearCCS ?
                        (WA16K(pClientContext->GetLibContext()) ? GMM_BYTES(64) : WA64K(pClientContext->GetLibContext()) ? GMM_BYTES(256) : GMM_KBYTE(4)) :
                        0))
	    {
                GMM_GFX_SIZE_T L1eIdx = GMM_L1_ENTRY_IDX(AUXTT, TileAdr, GetGmmLibContext());
                GMM_AUXTTL1e   L1e    = {0};
                L1e.Value             = PartialData;
                L1e.Valid             = 1;

                if(L1eIdx == 15 || L1eIdx == 14)
                {
                    GMM_DPF(GFXDBG_NORMAL, "\n****** switching over L1*******\n");
                }
		
                CCS$Adr = (pClientContext->GetLibContext()->GetSkuTable().FtrLinearCCS ? CCS$Adr :
				__GetCCSCacheline(BaseResInfo, BaseAdr, AuxResInfo, AuxVA, TileAdr - BaseAdr));

                if(WA16K(GetGmmLibContext()))
                {
                    L1e.Reserved2  = CCS$Adr >> 6;  /*********** 2 lsbs of 64B-aligned CCS adr *****/
                    L1e.Reserved4  = CCS$Adr >> 8;  /*********** 256B-aligned CCS adr *****/
                    L1e.GfxAddress = CCS$Adr >> 12; /*********** 4KB-aligned CCS adr *****/
                }
                else if(WA64K(GetGmmLibContext()))
                {
                    __GMM_ASSERT((CCS$Adr & 0xFF) == 0x0);
                    __GMM_ASSERT(GFX_IS_ALIGNED(CCS$Adr, GMM_BYTES(256)));
                    __GMM_ASSERT(GFX_IS_ALIGNED(TileAdr, GMM_KBYTE(64)));
                    L1e.Reserved4  = CCS$Adr >> 8;  /*********** 4 lsbs of 256B-aligned CCS adr *****/
                    L1e.GfxAddress = CCS$Adr >> 12; /*********** 4KB-aligned CCS adr *****/
                }
                else // 1MB aligned address
                {
                    __GMM_ASSERT((CCS$Adr & 0xFF) == 0x0);
                    __GMM_ASSERT(GFX_IS_ALIGNED(CCS$Adr, GMM_KBYTE(4)));
                    __GMM_ASSERT(GFX_IS_ALIGNED(TileAdr, GMM_MBYTE(1)));
                    L1e.GfxAddress = CCS$Adr >> 12; /*********** 4KB-aligned CCS adr *****/
                }


                GMM_DPF(GFXDBG_NORMAL, "--------------------------------MAP AuxTT Map Address: TileAddr[0x%llX], Size[0x%x], CCSAddr[0x%llX], L3eIdx[%d], L2eIdx[%d], L1eIdx[%d], \n L1CCSAddres[0x%llX] \n", TileAdr, BaseSize, CCS$Adr, L3eIdx, L2eIdx, L1eIdx, L1e.GfxAddress);

                GmmLib::LastLevelTable *pL1Tbl = NULL;

                pL1Tbl        = pTTL2[L3eIdx].GetL1Table(L2eIdx, NULL);
                L1TableCPUAdr = pL1Tbl->GetCPUAddress();
                if(DoNotWait)
                {
                    //Sync update on CPU
                    ((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].Value = L1e.Value;

                }
                else
                {
                    pL1Tbl->UpdatePoolFence(UmdContext, false);
                    PageTableMgr->TTCb.pfWriteL2L3Entry(UmdContext->pCommandQueueHandle,
                                                        L1TableAdr + L1eIdx * GMM_AUX_L1e_SIZE,
                                                        L1e.Value);
                }
                GMM_DPF(GFXDBG_NORMAL, "Map | L3 Table Entry: L3AddressBase[0x%llX] :: L3.L2GfxAddr[0x%llX] :: L3Valid[0x%llX] \n", (GMM_AUXTTL3e *)(TTL3.CPUAddress), ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].L2GfxAddr, ((GMM_AUXTTL3e *)(TTL3.CPUAddress))[L3eIdx].Valid);
                GMM_DPF(GFXDBG_NORMAL, "Map | L2 Table Entry: L2addressBase[0x%llX] :: L2.L1GfxAddr[0x%llX] :: L2Valid[0x%llX] \n", ((GMM_AUXTTL2e *)pTTL2[L3eIdx].GetCPUAddress()), ((GMM_AUXTTL2e *)pTTL2[L3eIdx].GetCPUAddress())[L2eIdx].L1GfxAddr, ((GMM_AUXTTL2e *)pTTL2[L3eIdx].GetCPUAddress())[L2eIdx].Valid);
                GMM_DPF(GFXDBG_NORMAL, "Map | L1 Table Entry: L1addressBase[0x%llX] :: L1.CCSAddr[0x%llX] :: L1ValueReserved4[0x%llX] ::L1ValueReserved2[0x%llX] :: L1Valid[0x%llX] :: DerivedCCS[0x%llX] \n\n", ((GMM_AUXTTL1e *)L1TableCPUAdr), ((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].GfxAddress, ((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].Reserved4, ((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].Reserved2, ((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].Valid, (((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].GfxAddress << 12));
                GMM_DPF(GFXDBG_NORMAL, "**Map | Table Entry: L2addressBase[0x%llX] :: L2Valid[%d] :: L2eidx[%d]  L1addressBase[0x%llX] :: L1eidx[%d]  L1Valid[0x%llX] :: DerivedCCS[0x%llX]", ((GMM_AUXTTL2e *)pTTL2[L3eIdx].GetCPUAddress()), ((GMM_AUXTTL2e *)pTTL2[L3eIdx].GetCPUAddress())[L2eIdx].Valid, L2eIdx, GMM_L1TABLE_ADDR_FROM_AUX_L2e_L1GFXADDR(((GMM_AUXTTL2e *)pTTL2[L3eIdx].GetCPUAddress())[L2eIdx], true), L1eIdx, ((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].Valid, (((GMM_AUXTTL1e *)L1TableCPUAdr)[L1eIdx].GfxAddress << 12));

                // Since we are mapping a non-null entry, no need to check whether
                // L1 table is unused.
                pL1Tbl->TrackTableUsage(AUXTT, true, TileAdr, false, GetGmmLibContext());
            }
        }
        if(!DoNotWait)
        {
            PageTableMgr->TTCb.pfEpilogTranslationTable(
            UmdContext->pCommandQueueHandle,
            1);
        }
    }

    LeaveCriticalSection(&TTLock);

    return Status;
}

GMM_AUXTTL1e GmmLib::AuxTable::CreateAuxL1Data(GMM_RESOURCE_INFO *BaseResInfo)
{
    GMM_RESOURCE_FORMAT Format;
    Format = BaseResInfo->GetResourceFormat();
    Format = ((Format > GMM_FORMAT_INVALID) && (Format < GMM_RESOURCE_FORMATS)) ? Format : GMM_FORMAT_INVALID;

    GMM_FORMAT_ENTRY FormatInfo = pClientContext->GetLibContext()->GetPlatformInfo().FormatTable[Format];
    GMM_AUXTTL1e     L1ePartial = {0};
#define GMM_REGISTRY_UMD_PATH "SOFTWARE\\Intel\\IGFX\\GMM\\"
#define GMM_E2EC_OVERRIDEDEPTH16BPPTO12 "ForceYUV16To12BPP"

    L1ePartial.Mode = BaseResInfo->GetResFlags().Info.RenderCompressed ? 0x1 : 0x0; //MC on VCS supports all compression modes,
                                                                                    //MC on Render pipe only 128B compr (until B-step)
    //Recognize which .MC surfaces needs Render pipe access
    if(pClientContext->GetLibContext()->GetWaTable().WaLimit128BMediaCompr)
    {
        L1ePartial.Mode = 0x1; //Limit media compression to 128B (same as RC) on gen12LP A0
    }

    //L1ePartial.Lossy = 0; // when to set it
    L1ePartial.TileMode = BaseResInfo->GetResFlags().Info.TiledYs ? 0 : 1;

    L1ePartial.Format     = FormatInfo.CompressionFormat.AuxL1eFormat;
    L1ePartial.LumaChroma = GmmIsPlanar(Format);

    if(pClientContext->GetLibContext()->GetWaTable().WaUntypedBufferCompression && BaseResInfo->GetResourceType() == RESOURCE_BUFFER)
    {
        //Gen12LP WA to support untyped raw buffer compression on HDC ie MLC(machine-learning compression)
        L1ePartial.TileMode = 0;
        L1ePartial.Depth    = 0x6;
        L1ePartial.Format   = GMM_E2ECOMP_FORMAT_RGBAFLOAT16;
    }

    __GMM_ASSERT(L1ePartial.Format > GMM_E2ECOMP_MIN_FORMAT && //Are we going to reuse 0x00 for uncompressed indication? CCS contains that info, but only known by HW
                 L1ePartial.Format <= GMM_E2ECOMP_MAX_FORMAT); //Could SW use it as surface-wide uncompressed state indicator? If so, remove teh assert (Need to make sure, all format encodings are correct)

    if(BaseResInfo->GetResFlags().Info.RenderCompressed)
    {
        if(BaseResInfo->GetResourceType() != RESOURCE_BUFFER)
        {
            switch(FormatInfo.Element.BitsPer)
            {
                case 8:
                    L1ePartial.Depth = 0x4;
                    break;
                case 16:
                    L1ePartial.Depth = 0x0;
                    break;
                case 32:
                    L1ePartial.Depth = 0x5;
                    break;
                case 64:
                    L1ePartial.Depth = 0x6;
                    break;
                case 128:
                    L1ePartial.Depth = 0x7;
                    break;
                default:
                    L1ePartial.Depth = 0x3;
            }
        }
    }
    else
    {
        switch(Format)
        {
            case GMM_FORMAT_P012:
            case GMM_FORMAT_Y412:
            case GMM_FORMAT_Y212: //which format encoding for Y212, Y412, P012?
                L1ePartial.Depth = 0x2;
                break;
            case GMM_FORMAT_P010:
            //case GMM_FORMAT_Y410:
            case GMM_FORMAT_Y210: //which format encoding for Y210?
                L1ePartial.Depth = 0x1;
                break;
            case GMM_FORMAT_P016: //per HAS, separate encoding than P010, but a comment says to use P010 in AuxTable?
            case GMM_FORMAT_Y416:
            case GMM_FORMAT_Y216:
                L1ePartial.Depth = 0x0;
                break;
            default:
                L1ePartial.Depth = 0x3; //For MC, bpp got from format encoding
        }

        if(L1ePartial.Format == GMM_E2ECOMP_FORMAT_R10G10B10A2_UNORM)
        {
            L1ePartial.Format = GMM_E2ECOMP_FORMAT_RGB10b;
        }
    }

    return L1ePartial;
}

GMM_GFX_ADDRESS GMM_INLINE GmmLib::AuxTable::__GetCCSCacheline(GMM_RESOURCE_INFO *BaseResInfo, GMM_GFX_ADDRESS BaseAdr,
                                                               GMM_RESOURCE_INFO *AuxResInfo, GMM_GFX_ADDRESS AuxVA, GMM_GFX_SIZE_T AdrOffset)
{
    GMM_GFX_ADDRESS CCSChunkAdr = 0xFFFFFFF0;
    uint32_t        x = 0, y = 0;
    uint32_t        i = 0, j = 0;
    uint32_t        CCSXTile = 0, CCSYTile = 0;
    GMM_UNREFERENCED_PARAMETER(BaseAdr);

    bool     BaseIsYF         = BaseResInfo->GetResFlags().Info.TiledYf ? true : false;
    uint32_t BasePitchInTiles = BaseResInfo->GetRenderPitchTiles();

    //Find YF/YS TileId <x,y> for given main surface 16K-chunk
    //and CCS$Id <i,j> corresponding to main's <x,y>
    AdrOffset >>= 14; //AdrOffset must be 16K-aligned chunk, since mapping unit is 4 YF pages
    if(BaseIsYF)
    {
        uint32_t PitchIn4YF = BasePitchInTiles / 4; //Base Pitch is physically padded to 4x1 YF width
        
	if (PitchIn4YF != 0)
        {
            i = static_cast<uint32_t>(AdrOffset % PitchIn4YF);
            j = static_cast<uint32_t>(AdrOffset / PitchIn4YF);
        }
        else
        {
            __GMM_ASSERT(PitchIn4YF != 0);
            return 0;
        }
    }
    else if(BasePitchInTiles != 0) //TileYs
    {
        x = static_cast<uint32_t>(AdrOffset >> 2); //YS-tile count
        y = x / BasePitchInTiles;                  //YS- tile id <x,y>
        x = x % BasePitchInTiles;
        i = 2 * x;
        j = 2 * y;
        switch(AdrOffset % 4) //YS : XYXY [XYXY YF] ie 2x2 16K-units in Y-major
        {
            case 0:
                break;
            case 1:
                j++;
                break;
            case 2:
                i++;
                break;
            case 3:
                i++;
                j++;
                break;
        }
    }

    //Compute CCS$ address for <i,j>
    CCSXTile = (i >= 8) ? i / 8 : 0; //8x8 CLs make one CCS Tile; get TileOffset
    CCSYTile = (j >= 8) ? j / 8 : 0;
    i %= 8;
    j %= 8;

    uint32_t AuxPitchInTiles = AuxResInfo ? AuxResInfo->GetRenderPitchTiles() : BaseResInfo->GetRenderAuxPitchTiles();
    CCSChunkAdr              = AuxVA + ((CCSXTile + CCSYTile * AuxPitchInTiles) * GMM_KBYTE(4)) + (8 * GMM_BYTES(64) * i) + (GMM_BYTES(64) * j);

    return CCSChunkAdr;
}

#endif /*!__GMM_KMD__*/
