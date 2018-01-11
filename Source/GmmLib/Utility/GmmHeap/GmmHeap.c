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

//#include "GmmCommonExt.h"

#if  _WIN32
#include <stdlib.h>
#include "External/Windows/GmmHeap.h"
#include "External/Windows/node.h"
#endif

#ifdef __GMM_KMD__
#include "..\..\..\miniport\LHDM\KmGmm\inc\gmminc.h"
#endif

#ifdef __GMM_KMD__
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmInitHeapNodes
Description:
     This function setup the node mgmt info needed for allocating Heap nodes

Arguments:
    PGMM_CONTEXT ==> ptr to GmmContext

Return:
    void
---------------------------------------------------------------------------*/
void __GmmInitHeapNodes(GMM_CONTEXT *pGmmContext)
{
    GMM_DPF_ENTER;

    __GmmCreateNodeMgmt(pGmmContext,
                        &(pGmmContext->HeapNodeMgmt),
                        sizeof(GMM_HEAPNODE),
                        __GMM_MAX_NUM_OF_FREE_HEAP_NODES);

    GMM_DPF_EXIT;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmDestroyHeapNodes

Description:
     This function destroy the node mgmt information for maintaning heap nodes

Arguments:
    PGMM_CONTEXT ==> ptr to GmmContext

Return:
    void
---------------------------------------------------------------------------*/
void __GmmDestroyHeapNodes(GMM_CONTEXT *pGmmContext)
{
    GMM_DPF_ENTER;

    __GmmDestroyNodeMgmt(pGmmContext, &(pGmmContext->HeapNodeMgmt));

    GMM_DPF_EXIT;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmCreateFencedSubHeap

Description:
    This function creates the sub heap.

    For each sub heap, a fence is enabled. Caller of function is responsible
    for passing Size, Pitch, Alignment parameters that meet platform
    specific fence requirements.

Arguments:
    pGmmContext     ==> ptr to GMM_CONTEXT
    Pitch           ==> Pitch of the subheap that need to be created
    Size            ==> Size of subheap that need to be created
    Alignment       ==> start address alignment requirement of sub heap
    Flags           ==> Indicate type of sub heap to create

Return:
    uint32_t for valid Heap index
---------------------------------------------------------------------------*/
uint32_t __GmmCreateFencedSubHeap(GMM_CONTEXT         *pGmmContext,
                               GMM_GFX_SIZE_T       Pitch,
                               GMM_GFX_SIZE_T       Size,
                               uint32_t                Alignment,
                               uint32_t                Flags)
{
    const __GMM_PLATFORM_RESOURCE     *pPlatformData;
    uint32_t                    FenceIdx;
    GMM_GFX_ADDRESS             GfxAddress;
    uint32_t                    HeapIdx = __GMM_NO_HEAP_FOUND;
    GMM_GFX_SIZE_T              HeapSize;
    GMM_GFX_SIZE_T              HeapPitch;
    GMM_HEAP                    *pLockHeapObj;
    GMM_HEAP                    *pHeapObj;
    GMM_INT_FUNCTBL             *pGmmFnTableInt = NULL;

    __GMM_ASSERTPTR(pGmmContext, __GMM_NO_HEAP_FOUND);
    __GMM_ASSERT(Size);
    __GMM_ASSERT(Alignment);

    GMM_DPF_ENTER;

    pPlatformData  = pGmmContext->pPlatformData;
    pGmmFnTableInt = &(pGmmContext->GmmFnTableInt);

    // Parent Heap...
    pLockHeapObj = &pGmmContext->LockSegmentHeap;

    // We are only intreseted in Tiled flags
    Flags = Flags & GMM_TILED;

    // If no heap space left bail out
    if (pLockHeapObj->FreeSize > 0)
    {
        HeapPitch = Pitch;
        HeapSize  = Size;

        // See comment with GMM's UseAlternateVA setting.
        {
	        GMM_GFX_SIZE_T MinSubheapSize;

            // If asked to exceed the platfrom specific max pitch return failure
            if ( ((Flags & GMM_TILE_Y) && (HeapPitch > pPlatformData->TileInfo[LEGACY_TILE_Y].MaxPitch)) ||
                 ((Flags & GMM_TILE_X) && (HeapPitch > pPlatformData->TileInfo[LEGACY_TILE_X].MaxPitch)) ||
                 ((HeapPitch > pPlatformData->Linear.MaxPitch)))
            {
                __GMM_ASSERT(0);
                return __GMM_NO_HEAP_FOUND;
            }

            if(GMM_REPORTED_SWIZZLE_RANGES_PER_FENCE > 1)
            {
                // Instead of using one fence per locked resource, we create
                // fence regions of at least a given size, whose extra space
                // can then be shared by future locked resources...

                // Seek to distribute fences across lock segment...
                MinSubheapSize =
                    GFX_CEIL_DIV(GMM_GMADR_SIZE_T_CAST(pLockHeapObj->Size), pPlatformData->NumberFenceRegisters);

                if (Flags & GMM_TILED)
                { // Potentially reduce to clean multiple of tile rows...
                    GMM_GFX_SIZE_T TileRowSize =
                        Pitch *
                        ((Flags & GMM_TILE_Y) ?
                            pPlatformData->TileInfo[LEGACY_TILE_Y].LogicalTileHeight :
                            pPlatformData->TileInfo[LEGACY_TILE_X].LogicalTileHeight);

                    MinSubheapSize = GFX_ALIGN_FLOOR_NP2(MinSubheapSize, TileRowSize);
                }
                else
                {
                    MinSubheapSize = GFX_ALIGN_FLOOR_NP2(MinSubheapSize, Pitch);
                }
            }
            else
            {
                MinSubheapSize = HeapSize; // No fence sharing, so no over-allocation
            }

            // Try to alloc ideal heap size
            if (__GmmAllocAlignHeapBlockGfxAddress(pGmmContext,
                                                   pLockHeapObj,
                                                   GFX_MAX(HeapSize, MinSubheapSize),
                                                   Alignment,
                                                   &GfxAddress))
            {
                // May have allocated more than what was asked. Save alloced size
                HeapSize = GFX_MAX(HeapSize, MinSubheapSize);
            }
            // Try original size
            else if (HeapSize < MinSubheapSize)
            {
                if (!__GmmAllocAlignHeapBlockGfxAddress(pGmmContext,
                                                        pLockHeapObj,
                                                        HeapSize,
                                                        Alignment,
                                                        &GfxAddress))
                {
                    return __GMM_NO_HEAP_FOUND;
                }
            }
            else
            {
                return __GMM_NO_HEAP_FOUND;
            }

            // Set the Fence Register with the right values
            __GMM_ASSERT(__GMM_RANGE_IN_GMADR(GfxAddress, HeapSize));
            FenceIdx = pGmmContext->GmmFnTableInt.__pfnGmmSetFenceReg(
                                                        pGmmContext->pHwDevExt,
                                                        Flags,
                                                        __GMM_GET_GMADR_OFFSET(GfxAddress),
                                                        Pitch,
                                                        HeapSize);
            if (FenceIdx == GMM_NO_FENCE_REG)
            {
                __GMM_ASSERT(0);
                __GmmFreeHeapBlockGfxAddress(pGmmContext, pLockHeapObj, GfxAddress, HeapSize);
                return __GMM_NO_HEAP_FOUND;
            }

            HeapIdx = FenceIdx;
        }

        pHeapObj = &pGmmContext->FenceHeap[FenceIdx];

        //Initialize rest of the HeapObj
        pHeapObj->FenceIdx = FenceIdx;

        if (__GmmSetupHeap(pGmmContext, pHeapObj, GfxAddress, HeapSize, HeapPitch, Flags) != GMM_SUCCESS)
        {
            return __GMM_NO_HEAP_FOUND;
        }
    }
    else
    {
        return __GMM_NO_HEAP_FOUND;
    }

    GMM_DPF_EXIT;
    return HeapIdx;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmFreeFencedSubHeap

Description:
    This function frees the particular heap

Arguments:
    pGmmContext     ==> ptr to GMM_CONTEXT
    HeapIdx         ==> Index of the heap that is being free

Return:
    STATUS_SUCCESS
    STATUS_INVALID_PARAMETER
---------------------------------------------------------------------------*/
NTSTATUS __GmmFreeFencedSubHeap(GMM_CONTEXT *pGmmContext,
                                uint32_t       HeapIdx)
{
    GMM_HEAP        *pHeapObj;
    GMM_INT_FUNCTBL *pGmmFnTableInt = NULL;

    __GMM_ASSERTPTR(pGmmContext, STATUS_INVALID_PARAMETER);

    GMM_DPF_ENTER;

    pHeapObj = &pGmmContext->FenceHeap[HeapIdx];

    pGmmFnTableInt = &(pGmmContext->GmmFnTableInt);

    // Check free size and heap size are same
    if (pHeapObj->Size != pHeapObj->FreeSize)
    {
        __GMM_ASSERT(0);
        // Memory Leak Memory is not freed
        return STATUS_INVALID_PARAMETER;
    }

    // Clean up the sentinal list from the heap Obj
    __GmmResetHeap(pGmmContext, pHeapObj);

    // Clear fence register
    pGmmContext->GmmFnTableInt.__pfnGmmClearFenceReg(pGmmContext->pHwDevExt,
                                                     pHeapObj->FenceIdx);

    // Free the Heap block
    __GmmFreeHeapBlockGfxAddress(
        pGmmContext,
        &pGmmContext->LockSegmentHeap,
        pHeapObj->BaseAddress,
        pHeapObj->Size);

    // Mark the Heap as free
    GFX_MEMSET(pHeapObj, 0, sizeof(GMM_HEAP));

    GMM_DPF_EXIT;

    return STATUS_SUCCESS;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
__GmmAllocBlockWithGfxAddress

Description:
This function allocate a block of memory of a given size and address
location with in the heap

Arguments:
pGmmContext  ==> ptr to GmmContext
pHeapObj ==> ptr to Heap object
Size ==> Size of memory block need to be allocated
ReqGfxAddress ==> Requested allocation start adddress

Return:
Success. (Block start address = pReqAddr on success.)
---------------------------------------------------------------------------*/
bool __GmmAllocBlockWithGfxAddress(GMM_CONTEXT *pGmmContext,
    GMM_HEAP    *pHeapObj,
    GMM_GFX_ADDRESS   ReqGfxAddress,
    GMM_GFX_SIZE_T    Size)
{
    GMM_HEAPNODE    *pNode, *pNode1;
    GMM_GFX_ADDRESS EndAddr, ReqAddr;
    KIRQL           OldIrql;
    KLOCK_QUEUE_HANDLE LockHandle;

    __GMM_ASSERT(pGmmContext);
    __GMM_ASSERT(pHeapObj);
    __GMM_ASSERT(Size);

    GMM_DPF_ENTER;

    ReqAddr = ReqGfxAddress;
    EndAddr = ReqAddr + Size;

    if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_ENTER_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }

    pNode = pHeapObj->pFreeHeap->pNext;       //the first node

    while (pNode && !(ReqAddr >= pNode->BlockAddr && EndAddr <= (pNode->BlockAddr + pNode->BlockSize)))
    {
        pNode = pNode->pNext;
    }

    if (!pNode)
    {
        GMM_DPF_CRITICAL("Unable to allocate specific heap addr!");

        if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
        {
            GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
        }

        __GMM_ASSERT(0);
        return(false);      //no free block
    }

    if (ReqAddr == pNode->BlockAddr &&
        EndAddr == (pNode->BlockAddr + pNode->BlockSize))
    {
        //whole block out, eliminate this node
        pNode->pPrev->pNext = pNode->pNext;
        pNode->pNext->pPrev = pNode->pPrev;

        __GmmFreeNode(pGmmContext, &pGmmContext->HeapNodeMgmt, pNode);

        pHeapObj->FreeSize -= Size;

        if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
        {
            GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
        }

        return(true);
    }

    if (ReqAddr == pNode->BlockAddr)
    { /* from start */
        pNode->BlockAddr = pNode->BlockAddr + Size;        //there is space left
        pNode->BlockSize -= Size;

        pHeapObj->FreeSize -= Size;

        if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
        {
            GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
        }

        return(true);
    }

    if (EndAddr == (pNode->BlockAddr + pNode->BlockSize))
    { /* to end */
        pNode->BlockSize -= Size;

        pHeapObj->FreeSize -= Size;

        if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
        {
            GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
        }

        return(true);
    }

    /* between condition where the allocation falls in between the node  */
    pNode1 = (GMM_HEAPNODE *)__GmmAllocNode(pGmmContext,
        &pGmmContext->HeapNodeMgmt);

    if (pNode1 == NULL)
    {
        GMM_DPF_CRITICAL("Can't allocate node!");

        if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
        {
            GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
        }

        __GMM_ASSERT(0);
        return(false);
    }

    pNode1->BlockAddr = EndAddr;
    pNode1->BlockSize = pNode->BlockSize - (EndAddr - pNode->BlockAddr);

    pNode->BlockSize = ReqAddr - pNode->BlockAddr;

    // Insert new node at end of list.

    // Link new node w/ current node next
    pNode1->pNext = pNode->pNext;
    pNode->pNext->pPrev = pNode1;

    // Link new node w/ current node
    pNode->pNext = pNode1;
    pNode1->pPrev = pNode;

    pHeapObj->FreeSize -= Size;

    __GMM_ASSERT(ReqAddr >= pHeapObj->BaseAddress);
    __GMM_ASSERT(ReqAddr < (pHeapObj->BaseAddress + pHeapObj->Size));

    if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }

    GMM_DPF_EXIT;

    return(true);
} // __GmmAllocBlockWithGfxAddress


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmResetHeap

Description:
    The function reset the heap by removing all sentinel nodes

Arguments:
    PGmmContext ==> ptr to GmmContext
    pHeapObj ==> ptr to Heap object

Return:
    N/A

Notes:
N/A
---------------------------------------------------------------------------*/
void __GmmResetHeap(GMM_CONTEXT *pGmmContext,
    GMM_HEAP    *pHeapObj)
{
    GMM_HEAPNODE *pNode, *pNode1;
    KIRQL        OldIrql;
    KLOCK_QUEUE_HANDLE LockHandle;

    __GMM_ASSERTPTR(pGmmContext, VOIDRETURN);
    __GMM_ASSERTPTR(pHeapObj, VOIDRETURN);

    GMM_DPF_ENTER;

    if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_ENTER_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }

    pNode = pHeapObj->pFreeHeap;       //the first node

    while (pNode)
    {
        pNode1 = pNode->pNext;      // save the next pointer

        __GmmFreeNode(pGmmContext, &pGmmContext->HeapNodeMgmt, pNode);

        pHeapObj->pFreeHeap = pNode1;
        pNode = pNode1;
    }

    if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }

    GMM_DPF_EXIT;
}

#else
#if _WIN32
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
__GmmUmGrowFreeNode

Description:
Grows free node

Arguments:

Return:
pHeapNodePool ==> pointer to list of free nodes

Notes:
N/A
---------------------------------------------------------------------------*/
GMM_INLINE void* __GmmUmGrowFreeNode()
{
    GMM_HEAPNODE         *pHeapNodePool = NULL;

    pHeapNodePool = __GmmUmInitHeapNodePool(sizeof(GMM_HEAPNODE), __GMM_MAX_NUM_OF_FREE_HEAP_NODES);

    return pHeapNodePool;
}
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
__GmmUmAllocNode

Description:
Allocation of free node

Arguments:
 pHeapNodePool ==> ptr to list of free nodes

Return:
Void * indicating free nodeReturn
---------------------------------------------------------------------------*/
GMM_INLINE void *__GmmUmAllocNode(GMM_HEAPNODE         **pHeapNodePool)
{
    GMM_HEAPNODE *pFreeNode = NULL;

    __GMM_ASSERTPTR(pHeapNodePool, NULL);

    if ( !(*pHeapNodePool) )
    {
        (*pHeapNodePool) = __GmmUmGrowFreeNode();
    }

    if (*pHeapNodePool)
    {
        pFreeNode = *pHeapNodePool;
        *pHeapNodePool = (*pHeapNodePool)->pNext;
    }

    return(pFreeNode);
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmUmFreeNode

Description:
    Put the node back into free list

Arguments:
    pHeapNodePool ==> Pool of HeapNodes
    pFreeNode ==> ptr to a node to be freed

Return:
    Void
---------------------------------------------------------------------------*/
GMM_INLINE void __GmmUmFreeNode(GMM_HEAPNODE         **pHeapNodePool, GMM_HEAPNODE *pFreeNode)
{
    __GMM_ASSERTPTR(pFreeNode, VOIDRETURN);
    __GMM_ASSERTPTR(pHeapNodePool, VOIDRETURN);

#if DBG || defined _DEBUG
    GFX_MEMSET(pFreeNode, __GMM_NODE_SIGNATURE, sizeof(GMM_HEAPNODE));
#else //!DBG
    GFX_MEMSET(pFreeNode, 0, sizeof(GMM_HEAPNODE));
#endif //DBG

    pFreeNode->pNext = *pHeapNodePool;
    *pHeapNodePool = pFreeNode;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    GmmUmSetupHeap

Description:
    This function sets up the heap for UMD
    Wrapparound to call __GmmSetupHeap

Arguments:
    GfxAddress      ==> Starting address of Heap
    Size            ==> Size of the heap
    Flags           ==> Indicate type of sub heap to create

Return:
    pHeapObj ==> Ptr to HeapObj to be used by UMD
---------------------------------------------------------------------------*/
GMM_HEAP* GMM_STDCALL GmmUmSetupHeap(GMM_ESCAPE_HANDLE        hAdapter,
                                     GMM_ESCAPE_HANDLE        hDevice,
                                     GMM_GFX_ADDRESS          GfxAddress,
                                     GMM_GFX_SIZE_T           Size,
                                     uint32_t                    Flags,
                                     GMM_ESCAPE_FUNC_TYPE     pfnEscape)
{
    GMM_STATUS         Status = GMM_SUCCESS;
    GMM_HEAP            *pHeapObj = NULL;

    if ((Flags & GMM_PROCESS_HEAP))
    {
        pHeapObj = GmmGetSharedHeapObject(hAdapter,
                                          hDevice,
                                          pfnEscape);
    }
    if (!pHeapObj)
    {
        pHeapObj = (GMM_HEAP*)malloc(sizeof(GMM_HEAP));
        if(pHeapObj)
        {
            GFX_MEMSET(pHeapObj, 0, sizeof(GMM_HEAP));
            pHeapObj->pHeapNodePool = __GmmUmInitHeapNodePool(sizeof(GMM_HEAPNODE), __GMM_MAX_NUM_OF_FREE_HEAP_NODES);

            Status = __GmmSetupHeap(NULL, pHeapObj, GfxAddress, Size, 0, Flags);

            if ((Flags & GMM_PROCESS_HEAP))
            {
                GmmSetSharedHeapObject(hAdapter,
                                       hDevice,
                                       pHeapObj,
                                       pfnEscape);
            }
        }
        else
        {
            pHeapObj = NULL;
            Status = GMM_ERROR;
        }
    }

    __GMM_ASSERT(Status == GMM_SUCCESS);

    return pHeapObj;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    GmmUmDestroypHeap

Description:
    This function destroys the heap created by UMD

Arguments:
    pHeapObj ==> Ptr to heapobject to be destroyed

Return:
    Status ==> GMM_SUCCESS or GMM_ERROR
---------------------------------------------------------------------------*/
GMM_STATUS GMM_STDCALL GmmUmDestroypHeap(GMM_ESCAPE_HANDLE        hAdapter,
                                         GMM_ESCAPE_HANDLE        hDevice,
                                         GMM_HEAP                 **pHeapObj,
                                         GMM_ESCAPE_FUNC_TYPE     pfnEscape
                                        )
{
    GMM_STATUS Status = GMM_SUCCESS;

    if( !pHeapObj ||
        (*pHeapObj == NULL))
    {
        Status = GMM_ERROR;
        return Status;
    }

    (*pHeapObj)->NumContexts--;

    if ((*pHeapObj)->NumContexts == 0)
    {
        __GmmUmResetHeap(*pHeapObj);
        __GmmUmDestroyHeapNodePool((*pHeapObj)->pHeapNodePool);
        (*pHeapObj)->pHeapNodePool = NULL;

        if (((*pHeapObj)->HeapType & GMM_PROCESS_HEAP))
        {
            GmmSetSharedHeapObject(hAdapter,
                                   hDevice,
                                   NULL,
                                   pfnEscape);
        }

        DeleteCriticalSection(&((*pHeapObj)->Lock));
        free(*pHeapObj);
        *pHeapObj = NULL;
    }

	return Status;

}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmUmInitHeapNodePool

Description:
    This function initializes Heap nodes pool for umd use

Arguments:
    NodeSize ==> Size of each node
    NumNodes ==> Number of Nodes to be created

Return:
    Status ==> GMM_SUCCESS or GMM_ERROR
---------------------------------------------------------------------------*/
GMM_HEAPNODE* __GmmUmInitHeapNodePool(uint32_t NodeSize,
                                      uint32_t NumNodes)
{
    uint32_t AllocSize = 0, SizeLeft = 0;
    GMM_HEAPNODE* pHeadNode = NULL;
    GMM_HEAPNODE* pHeapNodePool = NULL;

    __GMM_ASSERT(NodeSize >= sizeof(GMM_HEAPNODE));    // Assert if the size is smaller
    __GMM_ASSERT(NumNodes);

    AllocSize = SizeLeft = NodeSize*NumNodes;
    pHeapNodePool = pHeadNode = (GMM_HEAPNODE *)malloc(sizeof(GMM_HEAPNODE));

    if(pHeapNodePool)
    {
        SizeLeft -= NodeSize;
        pHeapNodePool->pPrev = NULL;
        GFX_MEMSET(pHeadNode, 0, NodeSize);

        //build the singly linked list
        while (SizeLeft >= NodeSize)
        {
            pHeadNode->pNext = (GMM_HEAPNODE *)malloc(sizeof(GMM_HEAPNODE));
            if (pHeadNode->pNext)
            {
                SizeLeft -= NodeSize;
                pHeadNode = pHeadNode->pNext;
                GFX_MEMSET(pHeadNode, 0, NodeSize);
            }
            else
            {
                __GMM_ASSERT(0);
                break;
            }
        }
        pHeadNode->pNext = NULL;
    }

    return pHeapNodePool;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
__GmmUmDestroyHeapNodePool

Description:
    This function destroys HeapNodePool

Arguments:
    pHeapNodePool ==> Ptr to HeapHodePool

Return:
    void
---------------------------------------------------------------------------*/
void __GmmUmDestroyHeapNodePool(GMM_HEAPNODE* pHeapNodePool)
{
    GMM_HEAPNODE* pNode;
    __GMM_ASSERTPTR(pHeapNodePool, VOIDRETURN);

    pNode = pHeapNodePool;
    while (pNode)
    {
        pHeapNodePool = pHeapNodePool->pNext;
        free(pNode);
        pNode = pHeapNodePool;
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmUmResetHeap

Description:
    The function resets the heap by removing all sentinel nodes

Arguments:
    pHeapObj ==> Ptr to HeapObj

Return:
    void
---------------------------------------------------------------------------*/
void __GmmUmResetHeap(GMM_HEAP            *pHeapObj)
{
    GMM_HEAPNODE* pNode, *pNode1;

    __GMM_ASSERT(pHeapObj != NULL);

    EnterCriticalSection(&(pHeapObj->Lock));

    pNode = pHeapObj->pFreeHeap;
    while (pNode)
    {
        pNode1 = pNode->pNext;      // save the next pointer

        __GmmUmFreeNode(&(pHeapObj->pHeapNodePool),pNode);

        pHeapObj->pFreeHeap = pNode1;
        pNode = pNode1;
    }

    LeaveCriticalSection(&(pHeapObj->Lock));

}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    GmmAllocateHeapVA

Description:
    The function reserves the VA range for the requested size from the Heap

Arguments:
    pHeapObj ==> Ptr to HeapObj
    AllocSize ==> SizeRequested

Return:
    GMM_GFX_ADDRESS ==> Reserved address for the requested Size, 0 if requsted size not available
---------------------------------------------------------------------------*/
GMM_GFX_ADDRESS GMM_STDCALL GmmAllocateHeapVA(GMM_HEAP* pHeapObj,
                                              GMM_GFX_SIZE_T AllocSize)
{
    GMM_GFX_ADDRESS GfxAddr;
    uint32_t BaseAlignment = GMM_HEAP_ALIGN_SIZE;
    uint32_t HeapType = GMM_OTHER_HEAP;

    if( !pHeapObj || (AllocSize > (pHeapObj->FreeSize)))
    {
        __GMM_ASSERT(0);
        return 0;
    }

    HeapType = pHeapObj->HeapType & HEAP_TYPE_MASK;

    switch (HeapType)
    {
    case GMM_TRVA_HEAP:
        BaseAlignment = GMM_TRVA_HEAP_ALIGN_SIZE;
        break;
    case GMM_FLAT_HEAP:
        BaseAlignment = GMM_FLAT_HEAP_ALIGN_SIZE;
        break;
    default:
        BaseAlignment = GMM_HEAP_ALIGN_SIZE;    //Align to 1B by default
        break;
    }
    __GmmAllocAlignHeapBlockGfxAddress(NULL, pHeapObj, AllocSize, BaseAlignment, &GfxAddr);

    return GfxAddr;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    GmmFreeHeapVA

Description:
    The function frees the previously reserved VA range from the heap

Arguments:
    pHeapObj ==> Ptr to HeapObj
    AllocSize ==> SizeRequested

Return:
    Status ==> GMM_SUCCESS or GMM_ERROR
---------------------------------------------------------------------------*/
GMM_STATUS GMM_STDCALL GmmFreeHeapVA(GMM_HEAP* pHeapObj,
                                     GMM_GFX_ADDRESS AllocVA,
                                     GMM_GFX_SIZE_T AllocSize)
{
	GMM_STATUS Status = GMM_SUCCESS;

    if( !pHeapObj || (AllocSize > (pHeapObj->Size)) )
    {
        __GMM_ASSERT(0);
        Status = GMM_ERROR;
        return Status;
    }

    __GmmFreeHeapBlockGfxAddress(NULL, pHeapObj, AllocVA, AllocSize);
	return Status;
}
#endif
#endif

#if _WIN32
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmSetupHeap

Description:
    This function setup the heap

Arguments:
    pGmmContext     ==> ptr to GMM_CONTEXT
    pHeapObj        ==> ptr to Heap object
    GfxAddress      ==> Starting address of Heap
    Size            ==> Size of the heap
    Pitch           ==> Pitch heap
    Flags           ==> Indicate type of sub heap to create

Return:
    void
---------------------------------------------------------------------------*/
GMM_STATUS __GmmSetupHeap(GMM_CONTEXT *pGmmContext,
                          GMM_HEAP    *pHeapObj,
                          GMM_GFX_ADDRESS GfxAddress,
                          GMM_GFX_SIZE_T  Size,
                          GMM_GFX_SIZE_T  Pitch,
                          uint32_t       Flags)
{
    GMM_UNREFERENCED_PARAMETER(Pitch);

    GMM_STATUS Status;
    __GMM_ASSERTPTR(pHeapObj, GMM_ERROR);

    GMM_DPF_ENTER;

    pHeapObj->BaseAddress = GfxAddress;
    pHeapObj->Size = Size;
    pHeapObj->FreeSize = Size;

#if  __GMM_KMD__
    __GMM_ASSERTPTR(pGmmContext, GMM_ERROR);
    pHeapObj->HeapCaps = Flags;
    pHeapObj->Pitch = Pitch;
    KeInitializeSpinLock(&pHeapObj->Lock);
#else
    pHeapObj->HeapType = Flags;
    pHeapObj->NumContexts = 0;
    InitializeCriticalSection(&(pHeapObj->Lock));
#endif

    // setup the linked list and other init stuff.
    if (GMM_SUCCESS ==
        (Status = __GmmInitHeap(pGmmContext, pHeapObj)))
    {
#if !__GMM_KMD__
        pHeapObj->NumContexts++;
#endif
    }

    GMM_DPF_EXIT;

    return Status;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmInitHeap
Description:
    The first free heap node is fill with heap start address
    and total heap size.

Arguments:
    PGMM_CONTEXT ==> ptr to GmmContext
    PGMM_HEAP ==> ptr to Heap object

Return:
    void
---------------------------------------------------------------------------*/
GMM_STATUS __GmmInitHeap(GMM_CONTEXT  *pGmmContext,
                         GMM_HEAP     *pHeapObj)
{
    GMM_STATUS Status;
    GMM_HEAPNODE *pNode;
#ifdef __GMM_KMD__
    KIRQL        OldIrql;
    KLOCK_QUEUE_HANDLE LockHandle;
#endif

    __GMM_ASSERTPTR(pHeapObj, GMM_ERROR);


    GMM_DPF_ENTER;

    Status = GMM_SUCCESS;

    // [1] initalize first dummyblock
#if __GMM_KMD__
    __GMM_ASSERTPTR(pGmmContext, GMM_ERROR);
    if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_ENTER_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }

    pNode = (GMM_HEAPNODE*)__GmmAllocNode(pGmmContext,
        &(pGmmContext->HeapNodeMgmt));
#else
    GMM_UNREFERENCED_PARAMETER(pGmmContext);
    EnterCriticalSection(&(pHeapObj->Lock));
    pNode = (GMM_HEAPNODE*)__GmmUmAllocNode(&(pHeapObj->pHeapNodePool));
#endif
    if (!pNode)
    {
        Status = GMM_OUT_OF_MEMORY;
        goto GMM_INIT_HEAP_EXIT;
    }

    pHeapObj->pFreeHeap = pNode;
    pNode->BlockAddr = 0;
    pNode->BlockSize = 0;
    pNode->pNext = pNode->pPrev = NULL;

    // [2] initialize a node to hold one single node
#if __GMM_KMD__
    pNode = (GMM_HEAPNODE*)__GmmAllocNode(pGmmContext,
        &(pGmmContext->HeapNodeMgmt));
#else
    pNode = (GMM_HEAPNODE*)__GmmUmAllocNode(&(pHeapObj->pHeapNodePool));
#endif
    if (!pNode)
    {
        Status = GMM_OUT_OF_MEMORY;
        goto GMM_INIT_HEAP_EXIT;
    }
    pHeapObj->pFreeHeap->pNext = pNode;    //one node when initialized;

    pNode->BlockAddr = pHeapObj->BaseAddress;
    pNode->BlockSize = pHeapObj->Size;
    pNode->pPrev = pHeapObj->pFreeHeap;
    pNode->pNext = NULL;

    // [3] initalize last dummyblock
#if __GMM_KMD__
    pNode = (GMM_HEAPNODE*)__GmmAllocNode(pGmmContext,
        &(pGmmContext->HeapNodeMgmt));
#else
    pNode = (GMM_HEAPNODE*)__GmmUmAllocNode(&(pHeapObj->pHeapNodePool));
#endif
    if (!pNode)
    {
        Status = GMM_OUT_OF_MEMORY;
        goto GMM_INIT_HEAP_EXIT;
    }

    pHeapObj->pFreeHeap->pNext->pNext = pNode;
    pNode->BlockAddr = GMM_GFX_ADDRESS_MAX;
    pNode->BlockSize = 0;
    pNode->pPrev = pHeapObj->pFreeHeap->pNext;
    pNode->pNext = NULL;

GMM_INIT_HEAP_EXIT:

#if __GMM_KMD__
    if ((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }
#else
    LeaveCriticalSection(&(pHeapObj->Lock));
#endif

    GMM_DPF_EXIT;

    return Status;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmFreeHeapBlockGfxAddress

Description:
     This function free the block of memory sepcified by size and address back
     to its original heap. Function will take care of collesing
     multiple block into single big block if possible

Arguments:
    pGmmContext  ==> ptr to GmmContext
    pHeapObj ==> ptr to Heap object
    GfxAddress ==> Address of the block to be freed.
    Size ==> Size of memory block need to be allocated

Return:
    void
---------------------------------------------------------------------------*/
void __GmmFreeHeapBlockGfxAddress(GMM_CONTEXT *pGmmContext,
                                  GMM_HEAP    *pHeapObj,
                                  GMM_GFX_ADDRESS   GfxAddress,
                                  GMM_GFX_SIZE_T    Size)
{
    GMM_GFX_ADDRESS     Addr, End;
    GMM_HEAPNODE        *pNode;
    GMM_GFX_ADDRESS     CurrBlockStartAddr;
    GMM_GFX_ADDRESS     CurrBlockEndAddr;
    GMM_GFX_ADDRESS     NextBlockStartAddr;
    GMM_GFX_ADDRESS     NextBlockEndAddr;
    GMM_HEAPNODE        *pTmpNode;
    GMM_HEAPNODE        *pNewNode;
    GMM_HEAPNODE        *pNextNode;

#ifdef __GMM_KMD__
    KIRQL               OldIrql;
    KLOCK_QUEUE_HANDLE  LockHandle;
#endif

    __GMM_ASSERT(pHeapObj);
    __GMM_ASSERT(Size);

    GMM_DPF_ENTER;

#if __GMM_KMD__
    __GMM_ASSERT(pGmmContext);
    if((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_ENTER_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }
#else
    GMM_UNREFERENCED_PARAMETER(pGmmContext);
    EnterCriticalSection(&(pHeapObj->Lock));
#endif

    __GMM_ASSERT( // GfxAddress belongs to pHeapObj...
        (GfxAddress >= pHeapObj->BaseAddress) &&
        ((GfxAddress + Size) <= (pHeapObj->BaseAddress + pHeapObj->Size)));

    Addr  = GfxAddress;                  // Free memory block start address
    End   = Addr + Size;                 // Free memory block end addrss
    pNode = pHeapObj->pFreeHeap;         // The first node (pFreeHeap) is "BlockAddr: 0, BlockSize: 0"

    while (pNode)
    {
        CurrBlockStartAddr = pNode->BlockAddr;
        CurrBlockEndAddr   = pNode->BlockAddr + pNode->BlockSize;
        NextBlockStartAddr = 0;
        NextBlockEndAddr   = 0;
        pTmpNode           = NULL;
        pNewNode           = NULL;
        pNextNode          = NULL;

        // Validate free memory range
        // -----------------------------------------
        // | Current Node Mem |    | Next Node Mem |
        // -----------------------------------------
        //            | Free Mem |
        //            ------------
        if (Addr < CurrBlockEndAddr && Addr >= CurrBlockStartAddr)
        {
#if __GMM_KMD__
            GMMReleaseMessage(GFXDBG_CRITICAL, "__GmmFreeHeapBlockGfxAddress(): Invalid Free Memory Range.");
#endif
            __GMM_ASSERT(0);
            break;
        }

        // -----------------------------------------
        // | Current Node Mem |    | Next Node Mem |
        // -----------------------------------------
        //                      | Free Mem |
        //                      ------------
        if (pNode->pNext != NULL)
        {
            NextBlockStartAddr = pNode->pNext->BlockAddr;
            NextBlockEndAddr   = pNode->pNext->BlockAddr + pNode->pNext->BlockSize;

            if (End > NextBlockStartAddr && End <= NextBlockEndAddr)
            {
#if __GMM_KMD__
                GMMReleaseMessage(GFXDBG_CRITICAL, "__GmmFreeHeapBlockGfxAddress(): Invalid Free Memory Range.");
#endif
                __GMM_ASSERT(0);
                break;
            }
        }

        //    ------------------------------------------------------
        // 1) | Current Node Mem | Free Mem |      | Next Node Mem |
        //    ------------------------------------------------------
        if (Addr == CurrBlockEndAddr && pNode->BlockAddr != 0 && pNode->BlockSize != 0)
        {
            pNode->BlockSize += Size;

            //    ----------------------------------------------
            //    | Current Node Mem | Free Mem | Next Node Mem|
            //    ----------------------------------------------
            if (pNode->pNext != NULL && End == NextBlockStartAddr)
            {
                pTmpNode            = pNode->pNext;
                pNode->BlockSize   += pTmpNode->BlockSize;
                pNode->pNext        = pTmpNode->pNext;
                pNode->pNext->pPrev = pNode;

#if __GMM_KMD__
                __GmmFreeNode(pGmmContext, &pGmmContext->HeapNodeMgmt, pTmpNode);
#else
                __GmmUmFreeNode(&(pHeapObj->pHeapNodePool), pTmpNode);
#endif
            }

            pHeapObj->FreeSize += Size;
            break;
        }

        //    -----------------------------------------------------------
        // 2) | Current Node Mem |     | Free Mem |     | Next Node Mem |
        //    -----------------------------------------------------------
        //    OR
        //    -------------------------------------
        //    | Current Node Mem |     | Free Mem |
        //    -------------------------------------
        else if (Addr > CurrBlockEndAddr && (End < NextBlockStartAddr || pNode->pNext == NULL))
        {
#if __GMM_KMD__
            pNewNode = (GMM_HEAPNODE *)__GmmAllocNode(pGmmContext,
                &pGmmContext->HeapNodeMgmt);
#else
            pNewNode = (GMM_HEAPNODE*)__GmmUmAllocNode(&(pHeapObj->pHeapNodePool));
#endif

            if (pNewNode == NULL)
            {
#if __GMM_KMD__
                GMMReleaseMessage(GFXDBG_CRITICAL, "__GmmFreeHeapBlockGfxAddress(): Failed to allocate new node");
#endif
                __GMM_ASSERT(0);
                break;
            }

            pNewNode->BlockAddr = Addr;
            pNewNode->BlockSize = Size;
            pNewNode->pNext     = pNode->pNext;

            if (pNewNode->pNext != NULL)
            {
                pNewNode->pNext->pPrev = pNewNode;
            }

            pNode->pNext    = pNewNode;
            pNewNode->pPrev = pNode;

            pHeapObj->FreeSize += Size;
            break;
        }

        //    ----------------------------------------------------
        // 3) | Current Node Mem |     | Free Mem | Next Node Mem|
        //    ----------------------------------------------------
        else if (pNode->pNext != NULL && End == NextBlockStartAddr)
        {
            pNextNode             = pNode->pNext;
            pNextNode->BlockAddr  = Addr;
            pNextNode->BlockSize += Size;

            pHeapObj->FreeSize += Size;
            break;
        }

        pNode = pNode->pNext;
    }

#if __GMM_KMD__
    if((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }
#else
    LeaveCriticalSection(&(pHeapObj->Lock));
#endif

    GMM_DPF_EXIT;
} // __GmmFreeHeapBlockGfxAddress


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmAllocAlignHeapBlockGfxAddress

Description:
     This function allocate a block of memory of a given size and align that
     with a particular value.

Arguments:
    pGmmContext  ==> ptr to GmmContext
    pHeapObj ==> ptr to Heap object
    Size ==> Size of memory block need to be allocated
    AlignValue ==> Block start alignement
    pGfxAddress ==> Pointer for return of alloc'ed GfxAddress on success.

Return:
    true on success, with the allocated gfx address returned via pGfxAddress;
    false on failure. Note that *pGfxAddress = 0 can be a successful
    allocation for some heaps. Also note that *pGfxAddress is undefined on
    allocation failure.
---------------------------------------------------------------------------*/
bool __GmmAllocAlignHeapBlockGfxAddress(GMM_CONTEXT *pGmmContext,
                                           GMM_HEAP    *pHeapObj,
                                           GMM_GFX_SIZE_T   Size,
                                           uint32_t       AlignValue,
                                           GMM_GFX_ADDRESS  *pGfxAddress)
{
    GMM_HEAPNODE        *pNode, *pNode1;
    GMM_GFX_ADDRESS     BlockAddr, AlignAddr;
#ifdef __GMM_KMD__
    KIRQL               OldIrql;
    KLOCK_QUEUE_HANDLE  LockHandle;
#endif
    bool                Success = true;

    __GMM_ASSERT(pHeapObj);
    __GMM_ASSERT(Size);
    __GMM_ASSERT(AlignValue);
    __GMM_ASSERT(pGfxAddress);

    GMM_DPF_ENTER;

#if __GMM_KMD__
    __GMM_ASSERT(pGmmContext);
    if((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_ENTER_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }
#else
    GMM_UNREFERENCED_PARAMETER(pGmmContext);
    EnterCriticalSection(&(pHeapObj->Lock));
#endif

    { // Get Best-Fit Free Node...
        GMM_HEAPNODE    *pBestNode, *pCurrNode;
        GMM_GFX_SIZE_T  PaddedSize;

		// Validate pFreeHeap before use
		if (pHeapObj->pFreeHeap == NULL)
		{
			Success = false;
			goto End;
		}
		pCurrNode = pHeapObj->pFreeHeap->pNext;
        pBestNode = NULL;
        while (pCurrNode)
        {
            BlockAddr = pCurrNode->BlockAddr;
            PaddedSize = Size + (GFX_ALIGN_NP2(BlockAddr, AlignValue) - BlockAddr);

            // find node which has size larger than requested
            if (pCurrNode->BlockSize >= PaddedSize)
            {
                // First: check if this is first node we found if so, then that is best fit for now
                // Second: If not first time then check if still old node is best fit. if not then
                //         replace it with new node
                if ( (pBestNode == NULL) ||
                     ( (pBestNode != NULL) && (pBestNode->BlockSize >= pCurrNode->BlockSize)))
                {
                    pBestNode = pCurrNode;
                }
            }
            pCurrNode = pCurrNode->pNext;
        }

        pNode = pBestNode;
    }

    if (pNode != NULL)
    {
        BlockAddr = pNode->BlockAddr;
        AlignAddr = GFX_ALIGN_NP2(BlockAddr, AlignValue);
        __GMM_ASSERT((Size + (AlignAddr - BlockAddr)) <= pNode->BlockSize);
    }
    else
    {
        Success = false;
        goto End;
    }

    // Align address Match with start address
    if (AlignAddr == BlockAddr)
    {
        pNode->BlockSize -= Size;       //subtract requested size

        if (pNode->BlockSize > 0)
        {
            pNode->BlockAddr = pNode->BlockAddr + Size;        //there is space left
        }
        else
        {   //whole block out, eliminate this node
			if (pNode->pPrev != NULL)
			{
				pNode->pPrev->pNext = pNode->pNext;
			}

			if (pNode->pNext != NULL)
			{
				pNode->pNext->pPrev = pNode->pPrev;
			}

#if __GMM_KMD__
            __GmmFreeNode(pGmmContext, &pGmmContext->HeapNodeMgmt, pNode);
#else
            __GmmUmFreeNode(&(pHeapObj->pHeapNodePool), pNode);
#endif
        }
    } //it is some place in middle
    else
    {
        // get new node
#if __GMM_KMD__
        pNode1 = (GMM_HEAPNODE *) __GmmAllocNode(pGmmContext,
                                         &pGmmContext->HeapNodeMgmt);
#else
        pNode1 = (GMM_HEAPNODE*)__GmmUmAllocNode(&(pHeapObj->pHeapNodePool));
#endif

        if ( pNode1 == NULL )
        {
            __GMM_ASSERT(0);
            Success = false;
            goto End;
        }

        //start address
        pNode1->BlockAddr = BlockAddr;
        pNode1->BlockSize = AlignAddr - BlockAddr;

        //insert into free heap list
        pNode1->pPrev = pNode->pPrev;
        pNode1->pNext = pNode;
		if (pNode->pPrev != NULL)
		{
			pNode->pPrev->pNext = pNode1;
		}
        pNode->pPrev = pNode1;

        //remove the allocated size
        pNode->BlockSize = pNode->BlockSize - (pNode1->BlockSize + Size);

        if (pNode->BlockSize > 0)
        {
            pNode->BlockAddr = AlignAddr + Size;        //there is space left
        }
        else
        {   //whole block out, eliminate this node
			if (pNode->pPrev != NULL)
			{
				pNode->pPrev->pNext = pNode->pNext;
			}
			if (pNode->pNext != NULL)
			{
				pNode->pNext->pPrev = pNode->pPrev;
			}
#if __GMM_KMD__
            __GmmFreeNode(pGmmContext, &pGmmContext->HeapNodeMgmt, pNode);
#else
            __GmmUmFreeNode(&(pHeapObj->pHeapNodePool), pNode);
#endif
        }
    }

    pHeapObj->FreeSize -= Size;

    __GMM_ASSERT( // Block belongs to pHeapObj...
        (AlignAddr >= pHeapObj->BaseAddress) &&
        ((AlignAddr + Size) <= (pHeapObj->BaseAddress + pHeapObj->Size)));

    *pGfxAddress = AlignAddr;

End:
#if __GMM_KMD__
    if((pHeapObj->HeapCaps & GMM_HEAP_EXTERNAL_SYNC) == 0)
    {
        GMM_EXIT_CRITICAL_SECTION(OldIrql, &pHeapObj->Lock, &LockHandle);
    }
#else
    LeaveCriticalSection(&(pHeapObj->Lock));
#endif

    GMM_DPF_EXIT;

    return(Success);
} // __GmmAllocAlignHeapBlockGfxAddress
#endif