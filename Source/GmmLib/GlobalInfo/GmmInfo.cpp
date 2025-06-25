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

#include "Internal/Common/GmmLibInc.h"

#if(!defined(__GMM_KMD__) && !GMM_LIB_DLL_MA)
int32_t GmmLib::Context::RefCount = 0;
#endif

#ifdef GMM_LIB_DLL

// Create Mutex Object used for syncronization of ProcessSingleton Context
#if !GMM_LIB_DLL_MA
#ifdef _WIN32
GMM_MUTEX_HANDLE GmmLib::Context::SingletonContextSyncMutex = ::CreateMutex(NULL, false, NULL);
#else  //_WIN32
GMM_MUTEX_HANDLE      GmmLib::Context::SingletonContextSyncMutex = PTHREAD_MUTEX_INITIALIZER;
#endif // _WIN32
#endif //!GMM_LIB_DLL_MA

/////////////////////////////////////////////////////////////////////////////////////
/// GMM lib DLL Multi Adapter Functions
/// 1. This is the main holder of the Gmmlib Context
/// 2. There is only one Multi-Adpater Context object created per process.
/// 3. Gmmlib dll is loaded only once per process, This done by the first UMD client
///    requesting Libcontext on a first Adpater to be registered with Gmmlib.
/// 4. Gmmlib dll is Unloaded only once per process, this is done by the last client
///    destroying the LibContext on the only last registered Adapter with Gmmlib.
/// 5. The LibContext for an Adapter is same across all the N clients and is always
///    equal.
/// 6. The ClientContext for all the N clients on particaluar Adapter is unique and
///    never equal.
/// 7  Ex: N UMD clients querying GmmLib for an GPU Adapter's Properties will have the
///    Same/Single LibContext and Unique N ClientContexts for that same adpater, on
///    same process.
/// 8. GmmLib is dynamically scalable for any number of GPU Adapters and any number
///    of Clients per Adapter.
/// 9. In Multiprocessing, for a process, the Gmmlib Multi-Adpater Context
///    object is protected/syncronized using the Lock/UnLockMAContextSyncMutex
/// 9. In Multiprocessing, for a process with Gmmlib Multi-Adpater Context the
///    LibContetxt is protected/syncronized using Lock/unLockSingletonContextSyncMutex
/////////////////////////////////////////////////////////////////////////////////////
GMM_MA_LIB_CONTEXT *pGmmMALibContext = NULL;

/////////////////////////////////////////////////////////////////////////////////////
/// Function to create GmmMultiAdapterContext Object
/// Called only during dll load time.
/// Since Linux doesnt have DLL Main equivalent, adding __attribute__((constructor))
/// for this GmmCreateMultiAdapterContext()
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API_CONSTRUCTOR void GmmCreateMultiAdapterContext()
{
    if(!pGmmMALibContext)
    {
        // This is called only during dll load
        // Initializes the MA context.
	pGmmMALibContext = new GMM_MA_LIB_CONTEXT();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Function to Destroy GmmMultiAdapterContext Object
/// Called Only during Dll Unload.
/// Since Linux doesnt have DLL Main equivalent, adding __attribute__((destructor))
/// for this GmmDestroyMultiAdapterContext()
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API_DESTRUCTOR void GmmDestroyMultiAdapterContext()
{
    if(pGmmMALibContext)
    {
        // Before destroying GmmMultiAdapterContext, check if all the Adapters have
        // their GmmLibContext destroyed.
        // At this point the linked list is empty and pHeadNode = NULL & NumAdapter=0.
	if(!pGmmMALibContext->GetNumAdapters())
        {
            delete pGmmMALibContext;
            pGmmMALibContext = NULL;
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// GMM lib DLL function for creating Singleton Context (GmmLib::Context)
/// object which shall be process singleton across all UMD clients within a process
/// This function creates an object in the global MultiAdapter Context..
/// @see        Class GmmLib::Context
///
/// @param[in]  Platform: platform variable. Includes product family (Haswell, Cherryview,
///                       Broxton) with related render and display core revision (GEN3,
//                        ..., GEN10)
/// @param[in]  pSkuTable: Pointer to the sku feature table. Set of capabilities to
///                        allow code paths to be feature based and GEN agnostic.
/// @param[in]  pWaTable:  Pointer to the work around table. A set of anti-features,
///                        often in early/prototype silicon that require work-arounds
///                        until they are resolved to allow code paths to be GEN agnostic.
/// @param[in]  pGtSysInfo: Pointer to the GT system info. Contains various GT System
///                        Information such as EU counts, Thread Counts, Cache Sizes etc.
/// @param[in]  sBDF: Adapter's BDF info for which SingletonContext has to be created
/// @return     GMM_SUCCESS if Context is created, GMM_ERROR otherwise
/////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
extern "C" GMM_STATUS GMM_STDCALL GmmCreateLibContext(const PLATFORM           Platform,
                                                      const SKU_FEATURE_TABLE *pSkuTable,
                                                      const WA_TABLE *         pWaTable,
                                                      const GT_SYSTEM_INFO *   pGtSysInfo,
                                                      ADAPTER_BDF              sBdf)
#else
extern "C" GMM_STATUS GMM_STDCALL GmmCreateLibContext(const PLATFORM Platform,
                                                      const void *   pSkuTable,
                                                      const void *   pWaTable,
                                                      const void *   pGtSysInfo,
                                                      ADAPTER_BDF    sBdf,
                                                      const GMM_CLIENT ClientType)
#endif
{
    __GMM_ASSERTPTR(pSkuTable, GMM_ERROR);
    __GMM_ASSERTPTR(pWaTable, GMM_ERROR);
    __GMM_ASSERTPTR(pGtSysInfo, GMM_ERROR);

    // If pGmmMALibContext object is NULL, return error as DLL load would have failed
    if(!pGmmMALibContext)
    {
        return GMM_ERROR;
    }

#if LHDM
    return pGmmMALibContext->AddContext(Platform, pSkuTable, pWaTable, pGtSysInfo, sBdf, DeviceRegistryPath);
#else
    return pGmmMALibContext->AddContext(Platform, pSkuTable, pWaTable, pGtSysInfo, sBdf, ClientType);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// GMM lib DLL function for deleting the Singleton Context.
/// Reference Count will be decremented and once the reference count reaches 0,
/// Singleton Context will be freed in memory. This removes a singleton context from
/// the global MultiAdapter Context
///
/// @param[in] sbdf: Adapter's BDF info that needs its SingletonContext to be freed
/////////////////////////////////////////////////////////////////////////////////////
extern "C" void GMM_STDCALL GmmLibContextFree(ADAPTER_BDF sBdf)
{
    if (pGmmMALibContext)
    {
        pGmmMALibContext->RemoveContext(sBdf);
    }
}
/////////////////////////////////////////////////////////////////////////////////////
/// Constructor to zero initialize the GmmLib::GmmMultiAdapterContext object and create
/// GmmMultiAdapterContext class object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmMultiAdapterContext::GmmMultiAdapterContext()
{
    this->NumAdapters = 0;
    pCpuReserveBase   = NULL;
    CpuReserveSize    = 0;
    // The Multi-Adapter Initialization is done dynamiclly using a Single Linked list Vector
    // pHeadNode points to the root node of the linked list and registers the first
    // adapter received from UMD.
    // Initializing to NULL at DLL load.
    this->pHeadNode = NULL;

// Initializes the GmmLib::GmmMultiAdapterContext sync Mutex
// This is required whenever any update has to be done Multiadapter context
// This includes Addition, deletion and search operations of the GMM_ADAPTER_INFO linked list
    MAContextSyncMutex = PTHREAD_MUTEX_INITIALIZER;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Destructor to free  GmmLib::GmmMultiAdapterContext object memory
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GmmMultiAdapterContext::~GmmMultiAdapterContext()
{
    uint32_t i = 0;

// Un-initializes the GmmLib::GmmMultiAdapterContext sync Mutex
    pthread_mutex_destroy(&MAContextSyncMutex);
}

/////////////////////////////////////////////////////////////////////////////////////
/// Multi Adapter member function for creating Singleton Context (GmmLib::Context)
/// object which shall be process singleton across all UMD clients within a process.
/// This function is thread safe for the MultiAdapterContext object.
/// @see        Class GmmLib::Context
///
/// @param[in]  Platform: platform variable. Includes product family (Haswell, Cherryview,
///                       Broxton) with related render and display core revision (GEN3,
//                        ..., GEN10)
/// @param[in]  pSkuTable:  Pointer to the sku feature table. Set of capabilities to
///                         allow code paths to be feature based and GEN agnostic.
/// @param[in]  pWaTable:   Pointer to the work around table. A set of anti-features,
///                         often in early/prototype silicon that require work-arounds
///                         until they are resolved to allow code paths to be GEN agnostic.
/// @param[in]  pGtSysInfo: Pointer to the GT system info. Contains various GT System
///                         Information such as EU counts, Thread Counts, Cache Sizes etc.
/// @param[in]  sBDF:       Adapter's BDF info for which SingletonContext i.e Adapter Node
///                         has to be created.
/// @return     GMM_SUCCESS if Context is created, GMM_ERROR otherwise
/////////////////////////////////////////////////////////////////////////////////////
#if LHDM
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::AddContext(const PLATFORM           Platform,
                                                                  const SKU_FEATURE_TABLE *_pSkuTable,
                                                                  const WA_TABLE          *_pWaTable,
                                                                  const GT_SYSTEM_INFO    *_pGtSysInfo,
                                                                  ADAPTER_BDF              sBdf,
                                                                  const char              *DeviceRegistryPath,
                                                                  uint32_t                 PhysicalAdapterIndex)
#else
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::AddContext(const PLATFORM Platform,
                                                                  const void    *_pSkuTable,
                                                                  const void    *_pWaTable,
                                                                  const void    *_pGtSysInfo,
                                                                  ADAPTER_BDF    sBdf,
                                                                  const GMM_CLIENT ClientType)
#endif
{
    __GMM_ASSERTPTR(_pSkuTable, GMM_ERROR);
    __GMM_ASSERTPTR(_pWaTable, GMM_ERROR);
    __GMM_ASSERTPTR(_pGtSysInfo, GMM_ERROR);

    GMM_STATUS                Status = GMM_SUCCESS;
    SKU_FEATURE_TABLE        *pSkuTable;
    WA_TABLE                 *pWaTable;
    GT_SYSTEM_INFO           *pSysInfo;
    GMM_LIB_CONTEXT          *pGmmLibContext = NULL;
    GmmLib::GMM_ADAPTER_INFO *pNode          = NULL;

    pSkuTable = (SKU_FEATURE_TABLE *)_pSkuTable;
    pWaTable  = (WA_TABLE *)_pWaTable;
    pSysInfo  = (GT_SYSTEM_INFO *)_pGtSysInfo;

    GMM_STATUS SyncLockStatus = LockMAContextSyncMutex();
    if (SyncLockStatus != GMM_SUCCESS)
    {
        return SyncLockStatus;
    }

    // see if a context with the given BDF already exists
    pNode = GetAdapterNodeUnlocked(sBdf);

    if (pNode)
    {
        // The requested adapter context already exists. increment the reference count.
        // No further initialization needed.
        pNode->pGmmLibContext->IncrementRefCount();
        UnLockMAContextSyncMutex();
        return GMM_SUCCESS;
    }

    // No context exists for this bdf. Add a new node to store the context.
    pNode = AddAdapterNode();
    if (pNode == NULL)
    {
        UnLockMAContextSyncMutex();
        return GMM_ERROR;
    }

    // No context for this adapter yet, Lets create a new LibContext
    pGmmLibContext = new GMM_LIB_CONTEXT();
    if (pGmmLibContext == NULL)
    {
        RemoveAdapterNode(pNode);
        pNode = NULL;
        UnLockMAContextSyncMutex();
        return GMM_ERROR;
    }

    pGmmLibContext->IncrementRefCount();

    Status = (pGmmLibContext->InitContext(Platform, pSkuTable, pWaTable, pSysInfo, ClientType));
    if (Status != GMM_SUCCESS)
    {
        //clean everything and return error
        pGmmLibContext->DecrementRefCount();
        pGmmLibContext->DestroyContext();
        // Delete/free the LibContext object
        delete pGmmLibContext;
        RemoveAdapterNode(pNode);
        UnLockMAContextSyncMutex();
        return GMM_ERROR;
    }

#if LHDM
    // Initialize SingletonContext Data.
    // TBD: ProcessHeap creation requires size and GfxAddress parameters. These parameters are constants
    // and are given by GMM lib internally by PageTableMgr. Hence pHeapObj should be created here at the
    // time of SingletonContext creation. But untill all UMD clients have moved to GMM DLL, then we will
    // create this here.
    pGmmLibContext->pHeapObj           = NULL;
    pGmmLibContext->ProcessHeapCounter = 0;

    // TBD: ProcessVA Gfx partition should be created here using VirtualAlloc at the time of SingletonContext
    // creation. But untill all UMD clients have moved to GMM DLL, then we will
    // create this here.
    pGmmLibContext->ProcessVA        = {0};
    pGmmLibContext->ProcessVACounter = 0;

    pGmmLibContext->IsSVMReserved      = 0;
    pGmmLibContext->NullCCSTileAddress = 0;
#endif

    pGmmLibContext->sBdf = sBdf;

    pNode->pGmmLibContext = pGmmLibContext;


    UnLockMAContextSyncMutex();

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// GMM lib DLL function for deleting the Singleton Context.
/// Reference Count will be decremented and once the reference count reaches 0,
/// Singleton Context will be freed in memory
/// This function is thread safe for the MultiAdapterContext object.
///
/// @param[in] sbdf: Adapter's BDF info that needs its SingletonContext to be freed
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::RemoveContext(ADAPTER_BDF sBdf)
{
    GMM_STATUS SyncLockStatus = LockMAContextSyncMutex();

    if (SyncLockStatus != GMM_SUCCESS)
    {
        return SyncLockStatus;
    }

    GmmLib::GMM_ADAPTER_INFO *pNode = GetAdapterNodeUnlocked(sBdf);
    GMM_LIB_ASSERT(pNode);
    if (!pNode)
    {
        UnLockMAContextSyncMutex();
        return GMM_ERROR;
    }

    if (pNode->pGmmLibContext)
    {
        int32_t ContextRefCount = pNode->pGmmLibContext->DecrementRefCount();
        // Refount = 0, It means that it the last client on this adapter
        // Lets free the LibContext and the Adapter Node
        if (!ContextRefCount)
        {
            pNode->pGmmLibContext->DestroyContext();
            // Delete/free the LibContext object
            delete pNode->pGmmLibContext;

            // Delete/free the AdapterNode from the Linked List
            RemoveAdapterNode(pNode);
            pNode = NULL;
        }
    }
    else
    {
        // No context exists for this node. Just remove it.
        RemoveAdapterNode(pNode);
        pNode = NULL;
    }

    UnLockMAContextSyncMutex();

    return GMM_SUCCESS;
}



/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for returning the AdapterIdx
///
/// @param[in]  sBdf       : Adpater Bus, Device and Fucntion details
/// @return     Adpater Idx corresponding the given BDF.
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmMultiAdapterContext::GetAdapterIndex(ADAPTER_BDF sBdf)
{
    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class to add adapter info for a given BDF
/// Adds a new node with the given BDF if it is not yet tracked, otherwise returns the
/// existing node with a matching BDF
/// This function is not thread safe for the MultiAdapterContext object and calls to
/// it must be protected with LockMAContextSyncMutex()
///
/// @param[in]  sBdf       : Adpater Bus, Device and Function details
/// @return     GmmLib::GMM_ADAPTER_INFO pointer for the BDF
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GMM_ADAPTER_INFO *GmmLib::GmmMultiAdapterContext::AddAdapterNode()
{
    GMM_ADAPTER_INFO *pNode = NULL, *pPrev = NULL;

    pNode = this->pHeadNode;

    // walk to the end of the list
    while (pNode)
    {
        pPrev = pNode;
        pNode = pNode->pNext;
    }

    // create a new node
    pNode = (GMM_ADAPTER_INFO *)malloc(sizeof(GMM_ADAPTER_INFO));
    if (!pNode)
    {
        // No memory for new node
        return NULL;
    }

    pNode->pGmmLibContext = NULL;
    pNode->pNext          = NULL;

    if (this->pHeadNode)
    {
        // add it to the end of the list if the list already exists
        pPrev->pNext = pNode;
    }
    else
    {
        // nothing in the list, insert it as the head of the list
        this->pHeadNode = pNode;
    }

    this->NumAdapters++;

    return pNode;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class to remove a given GMM_ADAPTER_INFO node
/// This function is not thread safe for the MultiAdapterContext object and calls to
/// it must be protected with LockMAContextSyncMutex()
///
/// @param[in]  pNode      : The node to remove from the list
/// @return     Void       : Removes and deletes the adapter Node from the list
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::GmmMultiAdapterContext::RemoveAdapterNode(GMM_ADAPTER_INFO *pNode)
{
    GMM_ADAPTER_INFO *pCur = NULL, *pPrev = NULL;

    // Find the node and remove the node from the list
    pCur = this->pHeadNode;
    while (pCur)
    {
        if (pCur == pNode)
        {
            if (pPrev)
            {
                // the node to be removed is in the middle of the list or the tail
                pPrev->pNext = pNode->pNext;
            }
            else if (pNode == this->pHeadNode)
            {
                // the node to be removed is the head of the list
                this->pHeadNode = pNode->pNext;
            }

            // Decrement the Adapter Node count tracker variable
            this->NumAdapters--;

            // Free the node that was removed from the list above.
            free(pNode);
            break;
        }
        pPrev = pCur;
        pCur  = pCur->pNext;
    }
    GMM_ASSERTDPF(pCur != NULL, "CRITICAL ERROR: Node to be released does not exist in list");
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for returning the Adapter Node
/// This function is thread safe for the MultiAdapterContext object
///
/// @param[in]  sBdf       : Adpater Bus, Device and Fucntion details
/// @return     Adpater Node corresponding the given BDF or return NULL if not found
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GMM_ADAPTER_INFO *GmmLib::GmmMultiAdapterContext::GetAdapterNode(ADAPTER_BDF sBdf)
{
    GMM_ADAPTER_INFO *pNode = NULL;

    GMM_STATUS SyncLockStatus = LockMAContextSyncMutex();
    if (SyncLockStatus != GMM_SUCCESS)
    {
        return NULL;
    }

    pNode = GetAdapterNodeUnlocked(sBdf);

    UnLockMAContextSyncMutex();

    return pNode;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for returning the Adapter Node
/// This function is not thread safe for the MultiAdapterContext object and calls to
/// it must be protected with LockMAContextSyncMutex()
///
/// @param[in]  sBdf       : Adpater Bus, Device and Fucntion details
/// @return     Adpater Node corresponding the given BDF or return NULL if not found
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::GMM_ADAPTER_INFO *GmmLib::GmmMultiAdapterContext::GetAdapterNodeUnlocked(ADAPTER_BDF sBdf)
{
    GMM_ADAPTER_INFO *pNode = this->pHeadNode;

    // Search the entire linked list if the Adapter Node with sBdf is found or not
    while (pNode)
    {
        if ((sBdf.Bus == pNode->pGmmLibContext->sBdf.Bus) &&
            (sBdf.Device == pNode->pGmmLibContext->sBdf.Device) &&
            (sBdf.Function == pNode->pGmmLibContext->sBdf.Function))
        {
            // Yes, Found!. This is the Adapter Node
            // pNode != NULL, will be valid pointer.
            return pNode;
        }
        else
        {
            // Not found, Search Next
            // pNode = NULL if traversed till the tail and not found
            pNode = pNode->pNext;
        }
    }

    return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for returning the GmmLibContext
/// This function is thread safe for the MultiAdapterContext object
//
/// @param[in]  sBdf       : Adpater Bus, Device and Fucntion details
/// @return     GmmLibContext corresponding the given BDF.
//////////////////////////////////////////////////////////////////////////////////////
GmmLib::Context *GMM_STDCALL GmmLib::GmmMultiAdapterContext::GetAdapterLibContext(ADAPTER_BDF sBdf)
{
    GMM_ADAPTER_INFO *pNode = NULL;

    //Search the list and get the Adapter Node
    pNode = (GMM_ADAPTER_INFO *)GetAdapterNode(sBdf);
    if(pNode)
    {
        return pNode->pGmmLibContext;
    }
    else
    {
        return NULL; //return Null if not found
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for returning the NumAdapters
/// that are initialized within a process
/// This function is thread safe for the MultiAdapterContext object
///
/// @return     Number of Adpaters that are opened and initialized within a process.
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmMultiAdapterContext::GetNumAdapters()
{
    return this->NumAdapters;
}

#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for Locking MultiAdapter Mutex
/// SyncMutex to protect access of GmmMultiAdpaterContext object
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::LockMAContextSyncMutex()
{
    if(MAContextSyncMutex)
    {
        while(WAIT_OBJECT_0 != ::WaitForSingleObject(MAContextSyncMutex, INFINITE))
            ;
        return GMM_SUCCESS;
    }
    else
    {
        return GMM_ERROR;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for UnLocking MultiAdapter Mutex
/// SyncMutex to protect access of GmmMultiAdpaterContext
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::UnLockMAContextSyncMutex()
{
    if(MAContextSyncMutex)
    {
        ::ReleaseMutex(MAContextSyncMutex);
        return GMM_SUCCESS;
    }
    else
    {
        return GMM_ERROR;
    }
}

#else // Non Win OS

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for Locking MultiAdapter Mutex
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::LockMAContextSyncMutex()
{
    pthread_mutex_lock(&MAContextSyncMutex);
    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of GmmMultiAdapterContext class for UnLocking MultiAdapter Mutex
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::GmmMultiAdapterContext::UnLockMAContextSyncMutex()
{
    pthread_mutex_unlock(&MAContextSyncMutex);
    return GMM_SUCCESS;
}
#endif //#ifdef _WIN32

#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////
/// Get ProcessHeapVA Singleton HeapObj
/////////////////////////////////////////////////////////////////////////
GMM_HEAP *GmmLib::Context::GetSharedHeapObject()
{
    GMM_HEAP *pHeapObjOut = NULL;
    // Obtain ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    LockSingletonContextSyncMutex();
#else
    GmmLib::Context::LockSingletonContextSyncMutex();
#endif

    //Check if the ProcessHeapCounter is 0 or not, if not 0 increment the counter and return the heapObj
    // that is stored in the DLL Singleton context
    if(ProcessHeapCounter)
    {
        ProcessHeapCounter++;
        pHeapObjOut = pHeapObj;
    }

    // Unlock ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    UnlockSingletonContextSyncMutex();
#else
    GmmLib::Context::UnlockSingletonContextSyncMutex();
#endif

    return pHeapObjOut;
}
/////////////////////////////////////////////////////////////////////////
/// Set ProcessHeapVA Singleton HeapObj
/////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::Context::SetSharedHeapObject(GMM_HEAP **pProcessHeapObj)
{
    uint32_t DllClientsCount = 0;
    // Obtain ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    LockSingletonContextSyncMutex();
#else
    GmmLib::Context::LockSingletonContextSyncMutex();
#endif

    if(pProcessHeapObj)
    {
        if(!ProcessHeapCounter)
        {
            // Setting it for the first time
            ProcessHeapCounter++;
            pHeapObj = *pProcessHeapObj;
        }
        else
        {
            ProcessHeapCounter++;
            *pProcessHeapObj = pHeapObj;
        }
    }
    else // Destroy the HeapObj Handle Case
    {
        ProcessHeapCounter--;
        if(!ProcessHeapCounter)
        {
            // When all UMDs clients have called destroy
            pHeapObj = NULL;
        }
    }

    DllClientsCount = ProcessHeapCounter;

    // Unlock ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    UnlockSingletonContextSyncMutex();
#else
    GmmLib::Context::UnlockSingletonContextSyncMutex();
#endif

    return DllClientsCount;
}

/////////////////////////////////////////////////////////////////////////
/// Get ProcessGfxPartition
/////////////////////////////////////////////////////////////////////////
void GmmLib::Context::GetProcessGfxPartition(GMM_GFX_PARTITIONING *pProcessVA)
{
    // Obtain ProcessSingleton Lock
#if GMM_LIB_DLL_MA
   LockSingletonContextSyncMutex();
#else
    GmmLib::Context::LockSingletonContextSyncMutex();
#endif

    //Check if the ProcessVACounter is 0 or not, if not 0 increment the counter and return the ProcessVA
    // that is stored in the DLL Singleton context
    if(ProcessVACounter)
    {
        ProcessVACounter++;
        if(pProcessVA)
        {
            *pProcessVA = ProcessVA;
        }
    }

    // Unlock ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    UnlockSingletonContextSyncMutex();
#else
    GmmLib::Context::UnlockSingletonContextSyncMutex();
#endif
}

/////////////////////////////////////////////////////////////////////////
/// Set ProcessGfxPartition
/////////////////////////////////////////////////////////////////////////
void GmmLib::Context::SetProcessGfxPartition(GMM_GFX_PARTITIONING *pProcessVA)
{
    // Obtain ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    LockSingletonContextSyncMutex();
#else
    GmmLib::Context::LockSingletonContextSyncMutex();
#endif

    if(pProcessVA)
    {
        if(!ProcessVACounter)
        {
            // Setting it for the first time
            ProcessVACounter++;
            ProcessVA = *pProcessVA;
        }
        else
        {
            ProcessVACounter++;
            //Add code to return the stored value of ProcessVA when Escapes are removed
        }
    }
    else // Reset the ProcessVA Case
    {
        ProcessVACounter--;
        if(!ProcessVACounter)
        {
            // When all UMDs clients have called destroy
            ProcessVA = {0};
        }
    }

    // Unlock ProcessSingleton Lock
#if GMM_LIB_DLL_MA
    UnlockSingletonContextSyncMutex();
#else
    GmmLib::Context::UnlockSingletonContextSyncMutex();
#endif
}

#endif // _WIN32

#endif // GMM_LIB_DLL

/////////////////////////////////////////////////////////////////////////////////////
/// Constructor to zero initialize the GmmLib::Context object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::Context::Context()
    : ClientType(),
      pPlatformInfo(),
      pTextureCalc(),
      SkuTable(),
      WaTable(),
      GtSysInfo(),
      pGmmKmdContext(),
      pGmmUmdContext(),
      pKmdHwDev(),
      pUmdAdapter(),
      pGmmCachePolicy()
{
    memset(CachePolicy, 0, sizeof(CachePolicy));
    memset(CachePolicyTbl, 0, sizeof(CachePolicyTbl));

    //Default initialize 64KB Page padding percentage.
    AllowedPaddingFor64KbPagesPercentage = 10;
    InternalGpuVaMax                     = 0;
    AllowedPaddingFor64KBTileSurf        = 10;
    UsageBasedPaddingFor64KBTileSurf         = 50;
    MultiEngineAccessCompressedWAEnable      = 0;

#if(!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
    pGmmGlobalClientContext = NULL;
#endif

#ifdef GMM_LIB_DLL
//Protect this adapter context with a sync mutex. Initialize sync mutex
#ifdef _WIN32
    SyncMutex = ::CreateMutex(NULL, FALSE, NULL);
#else  // !_WIN32
    SyncMutex = PTHREAD_MUTEX_INITIALIZER;
#endif // _WIN32

#if GMM_LIB_DLL_MA
    RefCount = 0;
#endif //GMM_LIB_DLL_MA
#endif //GMM_LIB_DLL
}

/////////////////////////////////////////////////////////////////////////////////////
/// Destructor to release  GmmLib::Context object memory
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::Context::~Context()
{
#ifdef GMM_LIB_DLL
// Close the Mutex protecting this context
    pthread_mutex_destroy(&SyncMutex);
#endif //GMM_LIB_DLL
}

#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////////////
/// Member function of Context class for Locking Singleton Context
/// SyncMutex to protect access of GmmLibContext
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::Context::LockSingletonContextSyncMutex()
{
#ifdef GMM_LIB_DLL
    if(SyncMutex)
    {
        while(WAIT_OBJECT_0 != ::WaitForSingleObject(SyncMutex, INFINITE))
            ;
        return GMM_SUCCESS;
    }
#endif //GMM_LIB_DLL
    return GMM_ERROR;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of Context class for Unlocking Singleton Context
/// SyncMutex to protect access of GmmLibContext
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::Context::UnlockSingletonContextSyncMutex()
{
#ifdef GMM_LIB_DLL
    if(SyncMutex)
    {
        ::ReleaseMutex(SyncMutex);
        return GMM_SUCCESS;
    }
#endif //GMM_LIB_DLL
    return GMM_ERROR;
}

#else //_WIN32

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of Context class for Locking Singleton Context
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::Context::LockSingletonContextSyncMutex()
{
    pthread_mutex_lock(&SyncMutex);
    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of Context class for Unlocking Singleton Context
///
/// @return     GMM_STATUS.
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::Context::UnlockSingletonContextSyncMutex()
{
    pthread_mutex_unlock(&SyncMutex);
    return GMM_SUCCESS;
}

#endif //_WIN32

#if GMM_LIB_DLL_MA

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of Context class for Incrementing Adpater Ref
/// Count
/// RefCount indicates the number of UMD clients using a particaluar Adapter.
/// RefCount is incremented when a client requests LibContext on already registered
/// adapter.
/// RefCount = 0, when no clients are using an adapter's LibContext
/// RefCount > 0, when at least one client is using the adapter's LibContext
///
/// @param1     sBdf        Adpater's Bus, Device and Fucntion
/// @return     Current value of the ref count.
/////////////////////////////////////////////////////////////////////////////////////
int32_t GmmLib::Context::IncrementRefCount()
{
    int32_t *Ref = &RefCount;

#if defined(__linux__)
    // returns 0 only when registering the first client
    return (__sync_fetch_and_add(Ref, 1));
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function of Context class for Decrementing Adpater's Ref
/// Count
/// RefCount indicates the number of UMD clients using a particaluar Adapter.
/// RefCount is decremented when a clients requests LibContext destroy on already
/// registered adapter.
/// RefCount = 0, when no clients are using an adapter's LibContext
/// RefCount > 0, when at least one client is using the adapter's LibContext
///
/// @param1     sBdf        Adpater's Bus, Device and Fucntion
/// @return     Current value of the ref count.
/////////////////////////////////////////////////////////////////////////////////////
int32_t GmmLib::Context::DecrementRefCount()
{

    int32_t *Ref          = &RefCount;
    int      CurrentValue = 0;
    int      TargetValue  = 0;
    do
    {
        CurrentValue = *Ref;
        if(CurrentValue > 0)
        {
            TargetValue = CurrentValue - 1;
        }
        else
        {
            break;
        }
#if defined(__linux__)
    } while(!__sync_bool_compare_and_swap(Ref, CurrentValue, TargetValue));
#endif
    return TargetValue;
}

#endif //GMM_LIB_DLL_MA

/////////////////////////////////////////////////////////////////////////////////////
/// Member function to initialize the GmmLib::Context object with cache policy,
/// platform info, Texture calculator etc.
/// @param[in]  Platform: ref to platform
/// @param[in]  pSkuTable: ptr to sku table
/// @param[in]  pWaTable: ptr to workaround table
/// @param[in]  pGtSysInfo: ptr to gt system info
/// @param[in]  ClientType: client type such as dx, ogl, ocl etc
/// @return   GMM_SUCCESS if init is success, GMM_ERROR otherwise
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmLib::Context::InitContext(
const PLATFORM &         Platform,
const SKU_FEATURE_TABLE *pSkuTable,
const WA_TABLE *         pWaTable,
const GT_SYSTEM_INFO *   pGtSysInfo,
GMM_CLIENT               ClientType)
{
    this->ClientType = ClientType;

    // Save the SKU and WA
    this->SkuTable  = *pSkuTable;
    this->WaTable   = *pWaTable;
    this->GtSysInfo = *pGtSysInfo;
   
    if (GFX_GET_CURRENT_RENDERCORE(Platform) >= IGFX_XE2_HPG_CORE && (pSkuTable->FtrXe2Compression == false))
    {
        this->SkuTable.FtrXe2Compression = true;
        if (!(this->GetSkuTable().FtrFlatPhysCCS) || !(this->GetSkuTable().FtrE2ECompression))
        {
            SkuTable.FtrXe2Compression = false;
        }
    }

    this->pPlatformInfo = CreatePlatformInfo(Platform, false);
    if(this->pPlatformInfo == NULL)
    {
        return GMM_ERROR;
    }

    OverrideSkuWa();

  
    this->pGmmCachePolicy = CreateCachePolicyCommon();
    if(this->pGmmCachePolicy == NULL)
    {
        return GMM_ERROR;
    }

    this->pGmmCachePolicy->InitCachePolicy();

    this->pTextureCalc = CreateTextureCalc(Platform, false);
    if(this->pTextureCalc == NULL)
    {
        return GMM_ERROR;
    }

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Member function to deallcoate the GmmLib::Context's cache policy, platform info,
/// Texture calculator etc.
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmLib::Context::DestroyContext()
{
    if(this->pGmmCachePolicy)
    {
            delete this->pGmmCachePolicy;
            this->pGmmCachePolicy = NULL;
    }

    if(this->pTextureCalc)
    {
            delete this->pTextureCalc;
            this->pTextureCalc = NULL;
    }

    if(this->pPlatformInfo)
    {
            delete this->pPlatformInfo;
            this->pPlatformInfo = NULL;
    }
}

void GMM_STDCALL GmmLib::Context::OverrideSkuWa()
{
    if((GFX_GET_CURRENT_PRODUCT(this->GetPlatformInfo().Platform) < IGFX_XE_HP_SDV))
    {
        SkuTable.FtrTileY = true;
    }

    if(GFX_GET_CURRENT_PRODUCT(this->GetPlatformInfo().Platform) == IGFX_PVC)
    {
        SkuTable.Ftr57bGPUAddressing = true;
    }

    if (GFX_GET_CURRENT_PRODUCT(this->GetPlatformInfo().Platform) >= IGFX_BMG)
    {
        // FtrL3TransientDataFlush is always enabled for XE2 adding GMM Override if UMDs might have reset this.
        SkuTable.FtrL3TransientDataFlush = true;
    }
}

GMM_CACHE_POLICY *GMM_STDCALL GmmLib::Context::CreateCachePolicyCommon()
{
    GMM_CACHE_POLICY *        pGmmCachePolicy = NULL;
    GMM_CACHE_POLICY_ELEMENT *CachePolicy     = NULL;
    CachePolicy                               = GetCachePolicyUsage();
    PRODUCT_FAMILY ProductFamily              = GFX_GET_CURRENT_PRODUCT(GetPlatformInfo().Platform);

    if(GetCachePolicyObj())
    {
        return GetCachePolicyObj();
    }
	
    if(ProductFamily >= IGFX_BMG)
    {
        pGmmCachePolicy = new GmmLib::GmmXe2_LPGCachePolicy(CachePolicy, this);
    }
    else if((ProductFamily == IGFX_METEORLAKE) || (ProductFamily == IGFX_ARROWLAKE))
    {
        pGmmCachePolicy = new GmmLib::GmmXe_LPGCachePolicy(CachePolicy, this);
    }
    else
    {
        switch(GFX_GET_CURRENT_RENDERCORE(this->GetPlatformInfo().Platform))
        {
            case IGFX_XE2_HPG_CORE:
            case IGFX_XE3_CORE:
                pGmmCachePolicy = new GmmLib::GmmXe2_LPGCachePolicy(CachePolicy, this);
                break;
            case IGFX_GEN12LP_CORE:
            case IGFX_GEN12_CORE:
            case IGFX_XE_HP_CORE:
            case IGFX_XE_HPG_CORE:
            case IGFX_XE_HPC_CORE:
                if(GetSkuTable().FtrLocalMemory)
                {
                    pGmmCachePolicy = new GmmLib::GmmGen12dGPUCachePolicy(CachePolicy, this);
                }
                else
                {
                    pGmmCachePolicy = new GmmLib::GmmGen12CachePolicy(CachePolicy, this);
                }
                break;
            case IGFX_GEN11_CORE:
                pGmmCachePolicy = new GmmLib::GmmGen11CachePolicy(CachePolicy, this);
                break;
            case IGFX_GEN10_CORE:
                pGmmCachePolicy = new GmmLib::GmmGen10CachePolicy(CachePolicy, this);
                break;
            case IGFX_GEN9_CORE:
                pGmmCachePolicy = new GmmLib::GmmGen9CachePolicy(CachePolicy, this);
                break;
            default:
                pGmmCachePolicy = new GmmLib::GmmGen8CachePolicy(CachePolicy, this);
                break;
        }
    }
    if(!pGmmCachePolicy)
    {
        GMM_DPF_CRITICAL("unable to allocate memory for CachePolicy Object");
    }

    return pGmmCachePolicy;
}

GMM_TEXTURE_CALC *GMM_STDCALL GmmLib::Context::CreateTextureCalc(PLATFORM Platform, bool Override)
{
    if(!Override)
    {
        if(GetTextureCalc())
        {
            return GetTextureCalc();
        }
    }

    if(GFX_GET_CURRENT_PRODUCT(GetPlatformInfo().Platform) >= IGFX_METEORLAKE)
    {
        return new GmmXe_LPGTextureCalc(this);
    }
    else
    {
        switch(GFX_GET_CURRENT_RENDERCORE(Platform))
        {
            case IGFX_GEN7_CORE:
            case IGFX_GEN7_5_CORE:
                return new GmmGen7TextureCalc(this);
                break;
            case IGFX_GEN8_CORE:
                return new GmmGen8TextureCalc(this);
                break;
            case IGFX_GEN9_CORE:
                return new GmmGen9TextureCalc(this);
                break;
            case IGFX_GEN10_CORE:
                return new GmmGen10TextureCalc(this);
                break;
            case IGFX_GEN11_CORE:
                return new GmmGen11TextureCalc(this);
                break;
            case IGFX_GEN12LP_CORE:
            case IGFX_GEN12_CORE:
            case IGFX_XE_HP_CORE:
            case IGFX_XE_HPG_CORE:
            case IGFX_XE_HPC_CORE:
                 return new GmmGen12TextureCalc(this);
				 break;
            case IGFX_XE2_HPG_CORE:
	    case IGFX_XE3_CORE:
            default:
                return new GmmXe_LPGTextureCalc(this);
                break;
        }
    }
}

GMM_PLATFORM_INFO_CLASS *GMM_STDCALL GmmLib::Context::CreatePlatformInfo(PLATFORM Platform, bool Override)
{
    GMM_DPF_ENTER;

    PRODUCT_FAMILY ProductFamily = GFX_GET_CURRENT_PRODUCT(Platform);

    if(Override == false)
    {
        if(pPlatformInfo != NULL)
        {
            return pPlatformInfo;
        }
    }

    if (ProductFamily >= IGFX_LUNARLAKE)
    {
        return new GmmLib::PlatformInfoGen12(Platform, (GMM_LIB_CONTEXT *)this);
    }
    else
    {
        switch (GFX_GET_CURRENT_RENDERCORE(Platform))
        {
        case IGFX_GEN12LP_CORE:
        case IGFX_GEN12_CORE:
        case IGFX_XE_HP_CORE:
        case IGFX_XE_HPG_CORE:
        case IGFX_XE_HPC_CORE:
        case IGFX_XE2_HPG_CORE:
	case IGFX_XE3_CORE:
            return new GmmLib::PlatformInfoGen12(Platform, (GMM_LIB_CONTEXT *)this);
            break;
        case IGFX_GEN11_CORE:
            return new GmmLib::PlatformInfoGen11(Platform, (GMM_LIB_CONTEXT *)this);
            break;
        case IGFX_GEN10_CORE:
            return new GmmLib::PlatformInfoGen10(Platform, (GMM_LIB_CONTEXT *)this);
            break;
        case IGFX_GEN9_CORE:
            return new GmmLib::PlatformInfoGen9(Platform, (GMM_LIB_CONTEXT *)this);
            break;
        default:
            return new GmmLib::PlatformInfoGen8(Platform, (GMM_LIB_CONTEXT *)this);
            break;
        }
    }    
}

//C - Wrappers
/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the PlatformInfo ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const PlatformInfo ptr
/////////////////////////////////////////////////////////////////////////
const GMM_PLATFORM_INFO *GMM_STDCALL GmmGetPlatformInfo(GMM_LIB_CONTEXT *pGmmLibContext)
{
    return (&pGmmLibContext->GetPlatformInfo());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the cache policy element array ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const cache policy elment ptr
/////////////////////////////////////////////////////////////////////////
const GMM_CACHE_POLICY_ELEMENT *GmmGetCachePolicyUsage(GMM_LIB_CONTEXT *pGmmLibContext)
{
    return (pGmmLibContext->GetCachePolicyUsage());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the texture calculation object ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   TextureCalc ptr
/////////////////////////////////////////////////////////////////////////
GMM_TEXTURE_CALC *GmmGetTextureCalc(GMM_LIB_CONTEXT *pGmmLibContext)
{
    return (pGmmLibContext->GetTextureCalc());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the sku table ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const SkuTable ptr
/////////////////////////////////////////////////////////////////////////
const SKU_FEATURE_TABLE *GmmGetSkuTable(GMM_LIB_CONTEXT *pGmmLibContext)
{
    return (&pGmmLibContext->GetSkuTable());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the Wa table ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const WaTable ptr
/////////////////////////////////////////////////////////////////////////
const WA_TABLE *GmmGetWaTable(GMM_LIB_CONTEXT *pGmmLibContext)
{
    return (&pGmmLibContext->GetWaTable());
}

/////////////////////////////////////////////////////////////////////////
/// C-wrapper to get the GT system info ptr
/// @param[in]  pGmmLibContext: ptr to GMM_GLOBAL_CONTEXT
/// @return   const GtSysInfo ptr
/////////////////////////////////////////////////////////////////////////
const GT_SYSTEM_INFO *GmmGetGtSysInfo(GMM_LIB_CONTEXT *pGmmLibContext)
{
    return (pGmmLibContext->GetGtSysInfoPtr());
}

