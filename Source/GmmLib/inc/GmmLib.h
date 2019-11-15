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

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#ifndef _WIN32
#include <portable_compiler.h>
#endif

#if defined(LHDM) && !defined(__GMM_KMD__) && defined(_WIN32)

#include <WINDOWS.h>
#include <d3d9Types.h>
#include <WTypesbase.h>
#include <d3dumddi.h>

#else
    // Since we are compiled not for WinOS, we don't want to include later any Visual Studio specific files.
    #define VER_H
#endif // LHDM

#include "../../inc/umKmInc/sharedata.h"

// GMM Lib Client Exports
#include "External/Common/GmmCommonExt.h"
#include "External/Common/GmmUtil.h"
#include "External/Common/GmmResourceFlags.h"
#include "External/Common/GmmCachePolicy.h"
#include "External/Common/GmmCachePolicyExt.h"
#include "External/Common/GmmResourceInfoExt.h"
#include "External/Common/GmmPlatformExt.h"
#include "External/Common/GmmTextureExt.h"
#include "External/Common/GmmInfoExt.h"
#include "External/Common/GmmResourceInfo.h"
#include "External/Common/GmmInfo.h"
#include "External/Common/GmmClientContext.h"

#include "External/Common/GmmPageTableMgr.h"

#include "External/Common/GmmLibDll.h"
#include "External/Common/GmmLibDllName.h"

#ifdef __cplusplus
}
#endif /*__cplusplus*/
