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
#include "External/Common/GmmClientContext.h"

#if(!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for creating Global GmmClientContext for Clients for backward compatible
/// @see        Class GMM_CLIENT_CONTEXT
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmCreateGlobalClientContext(GMM_CLIENT ClientType)
{
    GMM_STATUS Status                          = GMM_OUT_OF_MEMORY;
    pGmmGlobalContext->pGmmGlobalClientContext = GmmCreateClientContext(ClientType);
    if(pGmmGlobalContext->pGmmGlobalClientContext)
    {
        Status = GMM_SUCCESS;
    }

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for destroying Global GmmClientContext for UMD Client
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmDestroyGlobalClientContext()
{
    if(pGmmGlobalContext->pGmmGlobalClientContext)
    {
        GmmDeleteClientContext(pGmmGlobalContext->pGmmGlobalClientContext);
        pGmmGlobalContext->pGmmGlobalClientContext = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for Creating ResInfo for UMD Client using pGmmGlobalClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GmmResCreateThroughClientCtxt(GMM_RESCREATE_PARAMS *pCreateParams)
{
    GMM_RESOURCE_INFO *pRes = NULL;
    if(pGmmGlobalContext->pGmmGlobalClientContext)
    {
        pRes = pGmmGlobalContext->pGmmGlobalClientContext->CreateResInfoObject(pCreateParams);
    }

    return pRes;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for Destrying ResInfo for UMD Client using pGmmGlobalClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmResFreeThroughClientCtxt(GMM_RESOURCE_INFO *pRes)
{
    if(pGmmGlobalContext->pGmmGlobalClientContext)
    {
        pGmmGlobalContext->pGmmGlobalClientContext->DestroyResInfoObject(pRes);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for ResInfo Copy for UMD Client using pGmmGlobalClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GmmResCopyThroughClientCtxt(GMM_RESOURCE_INFO *pSrcRes)
{
    GMM_RESOURCE_INFO *pRes = NULL;
    if(pGmmGlobalContext->pGmmGlobalClientContext)
    {
        pRes = pGmmGlobalContext->pGmmGlobalClientContext->CopyResInfoObject(pSrcRes);
    }

    return pRes;
}


/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for ResMemCopy for UMD Client using pGmmGlobalClientContext
///
/////////////////////////////////////////////////////////////////////////////////////
void GmmResMemcpyThroughClientCtxt(void *pDst, void *pSrc)
{
    if(pGmmGlobalContext->pGmmGlobalClientContext)
    {
        pGmmGlobalContext->pGmmGlobalClientContext->ResMemcpy(pDst, pSrc);
    }
}

#endif // (!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
