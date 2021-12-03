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

#define ISPRODUCT(X)  (GFX_GET_CURRENT_PRODUCT(pGmmLibContext->GetPlatformInfo().Platform) == IGFX_##X)

#define FROMPRODUCT(X)  (GFX_GET_CURRENT_PRODUCT(pGmmLibContext->GetPlatformInfo().Platform) >= IGFX_##X)

#define SKU(FtrXxx) (pGmmLibContext->GetSkuTable().FtrXxx != 0)

#define WA(WaXxx)   (pGmmLibContext->GetWaTable().WaXxx != 0)

// Underscored to prevent name collision with the GMM_CACHE_POLICY_ELEMENT fields named L3 and LLC
#define _L3           (pGmmLibContext->GetGtSysInfo()->L3CacheSizeInKb)
#define _LLC          (pGmmLibContext->GetGtSysInfo()->LLCCacheSizeInKb)
#define _ELLC         (pGmmLibContext->GetGtSysInfo()->EdramSizeInKb)
#define CAM$          (SKU(FtrCameraCaptureCaching))

// Units are already in KB in the system information, so these helper macros need to account for that
#define KB(N)         (N)
#define MB(N)         (1024 * KB(N))
