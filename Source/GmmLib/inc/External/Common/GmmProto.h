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

#pragma once


//////////////////////////////////////////////////////////////////////////////////////
// GmmGtt.c
//////////////////////////////////////////////////////////////////////////////////////
GMM_GTT_ENTRY GMM_STDCALL __GmmGen8MakeGTTEntry(void *pAddress, GMM_MAKE_PTE *pPteType);
GMM_STATUS GMM_STDCALL __GmmGen8GttGetEntryInfo(GMM_GFX_ADDRESS GfxAddress, GMM_PTE_INFO *pPteInfo);
void       GMM_STDCALL GmmFlushGGttTlb(GMM_CONTEXT *pGmmContext);


//////////////////////////////////////////////////////////////////////////////////////
// GmmRestrictions.c
//////////////////////////////////////////////////////////////////////////////////////
void __GmmPlatformResetRestrictions(__GMM_BUFFER_TYPE *pRestriction);
void __GmmGetCubeTexRestrictions(GMM_PLATFORM_INFO*     pPlatformResource,
                                 GMM_RESOURCE_INFO*     pResourceInfo,
                                 __GMM_BUFFER_TYPE*     pBuff);
