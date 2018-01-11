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

/*****************************************************************************
** Optimization :
[1] Avoid constant allocation __GMM_MAX_NUM_TIMES_ALLOC for block
/*****************************************************************************/

#ifdef __GMM_KMD__
#include "..\..\..\miniport\LHDM\KmGmm\inc\gmminc.h"


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmCreateNodeMgmt

Description:
    Create a Node mgmt structure and initialized it

Arguments:
    pGmmContext ==> ptr to pGmmContext not used but just needed for clarity
                    with other
    pNodeMgmt ==> ptr to Node Mgmt
    NodeSize ==> True Node size
    NumNodes ==> Number of Items

Return:
    N/A

Notes:
    N/A
---------------------------------------------------------------------------*/
void __GmmCreateNodeMgmt(GMM_CONTEXT *pGmmContext, GMM_NODE_MGMT *pNodeMgmt,
                         uint32_t NodeSize, uint32_t NumNodes)
{
    __GMM_ASSERTPTR(pNodeMgmt, VOIDRETURN);
    __GMM_ASSERT(NodeSize >= sizeof(GMM_NODE));    // Assert if the size is smaller
    __GMM_ASSERT(NumNodes);

    GMMDebugMessage(GFXDBG_FUNCTION_ENTRY,"__GmmCreateNodeMgmt entered\r\n");

    pNodeMgmt->NumAddr    = 0;
    pNodeMgmt->NodeSize   = GFX_ALIGN(NodeSize, MEMORY_ALLOCATION_ALIGNMENT);
    pNodeMgmt->NumNodes   = NumNodes;
    KeInitializeSpinLock(&pNodeMgmt->NodeLock);

    pNodeMgmt->pFreeNodeList = (SLIST_HEADER *) GFX_ALIGN((uintptr_t) &pNodeMgmt->__FreeNodeListMemory, MEMORY_ALLOCATION_ALIGNMENT);
    __GMM_ASSERT((void *)(pNodeMgmt->pFreeNodeList + 1) < (void *)(&pNodeMgmt->__FreeNodeListMemory + 1));
    InitializeSListHead(pNodeMgmt->pFreeNodeList);
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmDestroyNodeMgmt

Description:
    Destroy the node mgmt

Arguments:
    pGmmContext ==> ptr to pGmmContext
    pNodeMgmt ==> ptr to Node Mgmt

Return:
    N/A

Notes:
    N/A
---------------------------------------------------------------------------*/
void __GmmDestroyNodeMgmt(GMM_CONTEXT *pGmmContext, GMM_NODE_MGMT *pNodeMgmt)
{
// TODO: Function actually called twice--Don't think that's harmful, but...?
    int32_t    i;

    // dbg Assert if the NodeMgmt is NULL
    __GMM_ASSERTPTR(pNodeMgmt, VOIDRETURN);

    GMMDebugMessage(GFXDBG_FUNCTION_ENTRY,"__GmmDestroyNodeMgmt entered\r\n");

    for (i=0; i < (int32_t)pNodeMgmt->NumAddr; i++)
    {
        OSFreeMem(pGmmContext->pHwDevExt, pNodeMgmt->pNodeBlockAddr[i], GFX_COMPONENT_GMM_TAG);
        pNodeMgmt->pNodeBlockAddr[i] = NULL;
    }

    //cleanup
    pNodeMgmt->NumAddr = 0;
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Function:
    __GmmGrowFreeNode

Description:
    Grows free node

Arguments:
    pGmmContext ==> ptr to pGmmContext
    pNodeMgmt ==> ptr to Node Mgmt

Return:
    GMM_SUCCESSFUL if it was successful in growing node
    error code otherwise

Notes:
    N/A
---------------------------------------------------------------------------*/
uint32_t __GmmGrowFreeNode(GMM_CONTEXT *pGmmContext, GMM_NODE_MGMT *pNodeMgmt)
{
#define REQUIRE(e)                      \
    if(!(e))                            \
    {                                   \
        __GMM_ASSERT(0);                \
        Status = STATUS_UNSUCCESSFUL;   \
        goto EXIT;                      \
    }

    void                *pNodeBlock = NULL;
    int32_t               AllocSize, SizeLeft;
    GMM_NODE            *pNode;
    KIRQL               OldIrql;
    KLOCK_QUEUE_HANDLE  LockHandle;
    uint32_t               Status = STATUS_SUCCESS;

    __GMM_ASSERTPTR(pNodeMgmt, STATUS_UNSUCCESSFUL);

    GMMDebugMessage(GFXDBG_FUNCTION_ENTRY,"__GmmGrowFreeNode entered\r\n");

    GMM_ENTER_CRITICAL_SECTION(OldIrql, &pNodeMgmt->NodeLock, &LockHandle);

    REQUIRE(pNodeMgmt->NumAddr < __GMM_MAX_NUM_TIMES_ALLOC);

    AllocSize = pNodeMgmt->NodeSize * pNodeMgmt->NumNodes;

    //Align Alloc Size to page size so that we allocated full page all the time
    AllocSize= GFX_ALIGN(AllocSize, __GMM_NODE_PAGE_SIZE);

    REQUIRE(
        pNodeBlock =
            OSAllocateMem(
                pGmmContext->pHwDevExt,
                MM_ZERO_MEMORY,
                AllocSize,
                NON_PAGED,
                GFX_COMPONENT_GMM_TAG));

    pNodeMgmt->pNodeBlockAddr[pNodeMgmt->NumAddr] = pNodeBlock;
    pNodeMgmt->NumAddr++;

    pNode = (GMM_NODE *)(GFX_ALIGN((uintptr_t) pNodeBlock, MEMORY_ALLOCATION_ALIGNMENT));
    SizeLeft = AllocSize - GFX_VOID_PTR_DIFF(pNode, pNodeBlock);

    //Create as many node as you can
    //Optimization: Remove extra operation by using curptr and tailptr

    while (SizeLeft >= (int32_t) pNodeMgmt->NodeSize)
    {
        __GmmFreeNode(pGmmContext, pNodeMgmt, pNode);
        pNode = GFX_VOID_PTR_INC(pNode, pNodeMgmt->NodeSize);
        SizeLeft -= pNodeMgmt->NodeSize;
    }

EXIT:
    GMM_EXIT_CRITICAL_SECTION(OldIrql, &pNodeMgmt->NodeLock, &LockHandle);

    return Status;

#undef REQUIRE
}
#endif