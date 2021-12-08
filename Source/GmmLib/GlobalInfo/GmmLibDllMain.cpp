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

/////////////////////////////////////////////////////////////////////////////////////
// First Call to GMM Lib DLL/so to initialize singleton global context
// and create client context
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API GMM_STATUS GMM_STDCALL InitializeGmm(GMM_INIT_IN_ARGS *pInArgs, 
		                                              GMM_INIT_OUT_ARGS *pOutArgs)
{
    GMM_STATUS Status = GMM_ERROR;

    if(pInArgs && pOutArgs)
    {
#if GMM_LIB_DLL_MA
        ADAPTER_BDF stAdapterBDF;

#ifdef _WIN32
        stAdapterBDF = pInArgs->stAdapterBDF;
#else
        stAdapterBDF.Data = pInArgs->FileDescriptor;
#endif

        Status = GmmCreateLibContext(pInArgs->Platform, pInArgs->pSkuTable, pInArgs->pWaTable, 
			                                   pInArgs->pGtSysInfo, stAdapterBDF);

        if(Status == GMM_SUCCESS)
        {
            pOutArgs->pGmmClientContext = GmmCreateClientContextForAdapter(pInArgs->ClientType,
			                                                          stAdapterBDF);
        }

#endif
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Destroys singleton global context and client context
///
/////////////////////////////////////////////////////////////////////////////////////
extern "C" GMM_LIB_API void GMM_STDCALL GmmAdapterDestroy(GMM_INIT_OUT_ARGS *pInArgs)
{
    if(pInArgs && pInArgs->pGmmClientContext)
    {
#if GMM_LIB_DLL_MA
    ADAPTER_BDF stAdapterBDF = pInArgs->pGmmClientContext->GetLibContext()->sBdf;

        GmmDeleteClientContext(pInArgs->pGmmClientContext);
        GmmLibContextFree(stAdapterBDF);
#endif
    }
}
#endif // GMM_LIB_DLL
