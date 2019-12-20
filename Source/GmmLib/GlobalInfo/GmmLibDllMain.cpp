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


#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <iostream>
#include <stddef.h>

#ifdef GMM_LIB_DLL

#include "Internal/Common/GmmLibInc.h"
#include "External/Common/GmmClientContext.h"
#include "External/Common/GmmLibDll.h"

#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////////////
/// DLL entry point function is needed only on Windows
///
/////////////////////////////////////////////////////////////////////////////////////
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD Reason, LPVOID Reserved)
{
    UNREFERENCED_PARAMETER(Reserved);
    UNREFERENCED_PARAMETER(hInst);

    BOOL result = TRUE;

    switch(Reason)
    {
        case DLL_PROCESS_ATTACH:
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }

    return result;
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/// First Call to GMM Lib DLL/so to get the DLL/so exported fucntion pointers
///
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API GMM_STATUS GMM_STDCALL OpenGmm(GmmExportEntries *pm_GmmFuncs)
{
    GMM_STATUS Status = GMM_SUCCESS;
    if(pm_GmmFuncs)
    {
        pm_GmmFuncs->pfnCreateSingletonContext  = GmmCreateSingletonContext;
        pm_GmmFuncs->pfnDestroySingletonContext = GmmDestroySingletonContext;
        pm_GmmFuncs->pfnCreateClientContext     = GmmCreateClientContext;
        pm_GmmFuncs->pfnDeleteClientContext     = GmmDeleteClientContext;
    }
    else
    {
        Status = GMM_INVALIDPARAM;
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// First Call to GMM Lib DLL/so to initialize singleton global context
/// and create client context
///
/////////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
extern "C" GMM_LIB_API GMM_CLIENT_CONTEXT *GMM_STDCALL GmmInit(const PLATFORM           Platform,
                                                               const SKU_FEATURE_TABLE *pSkuTable,
                                                               const WA_TABLE *         pWaTable,
                                                               const GT_SYSTEM_INFO *   pGtSysInfo,
                                                               GMM_CLIENT               ClientType)
#else
extern "C" GMM_LIB_API GMM_CLIENT_CONTEXT *GMM_STDCALL GmmInit(const PLATFORM Platform,
                                                               const void *   pSkuTable,
                                                               const void *   pWaTable,
                                                               const void *   pGtSysInfo,
                                                               GMM_CLIENT     ClientType)
#endif
{
    GMM_STATUS          Status         = GMM_SUCCESS;
    GMM_CLIENT_CONTEXT *pClientContext = NULL;

    Status = GmmCreateSingletonContext(Platform, pSkuTable, pWaTable, pGtSysInfo);

    if(Status == GMM_SUCCESS)
     {
        pClientContext = GmmCreateClientContext(ClientType);
     }

    return pClientContext;
}
/////////////////////////////////////////////////////////////////////////////////////
// First Call to GMM Lib DLL/so to initialize singleton global context
// and create client context
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API GMM_STATUS GMM_STDCALL InitializeGmm(GMM_INIT_IN_ARGS *pInArgs, GMM_INIT_OUT_ARGS *pOutArgs)
{
    GMM_STATUS Status = GMM_ERROR;

    if(pInArgs && pOutArgs)
    {

        Status = GmmCreateSingletonContext(pInArgs->Platform, pInArgs->pSkuTable, pInArgs->pWaTable, pInArgs->pGtSysInfo);

        if(Status == GMM_SUCCESS)
        {
            pOutArgs->pGmmClientContext = GmmCreateClientContext(pInArgs->ClientType);
        }
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Destroys singleton global context and client context
///
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API void GMM_STDCALL GmmDestroy(GMM_CLIENT_CONTEXT *pGmmClientContext)
{
    GmmDestroySingletonContext();
    GmmDeleteClientContext(pGmmClientContext);
}

/////////////////////////////////////////////////////////////////////////////////////
// Destroys singleton global context and client context
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API void GMM_STDCALL GmmAdapterDestroy(GMM_INIT_OUT_ARGS *pInArgs)
{
    if(pInArgs && pInArgs->pGmmClientContext)
    {
        GmmDeleteClientContext(pInArgs->pGmmClientContext);
        GmmDestroySingletonContext();
    }
}
#endif // GMM_LIB_DLL
