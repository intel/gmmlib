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


//Abstract:   include file for standard system include files,
//            or project specific include files that are used frequently


#pragma once
#ifndef _WIN32
#include "../../inc/portable_compiler.h"
#endif


#include "targetver.h"
#include <stdio.h>
#include <math.h>
#ifdef _WIN32
#include <tchar.h>
#include <windows.h>
#endif
#include <iostream>
#include "gtest.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "sharedata.h"
#include "../../inc/common/igfxfmid.h"
#include "../../inc/common/sku_wa.h"
#include "../../inc/common/gfxmacro.h"
#include "../inc/External/Common/GmmCommonExt.h"
#include "../inc/External/Common/GmmPlatformExt.h"
#include "../inc/External/Common/GmmCachePolicy.h"
#include "../inc/External/Common/GmmTextureExt.h"
#include "../inc/External/Common/GmmResourceInfoExt.h"
#include "../inc/External/Common/GmmResourceInfo.h"
#include "../inc/External/Common/GmmUtil.h"
#include "../inc/External/Common/GmmInfoExt.h"
#include "../inc/External/Common/GmmInfo.h"
#include "../inc/External/Common/GmmClientContext.h"
#include "../inc/External/Common/GmmPageTableMgr.h"
#include "../inc/External/Common/GmmLibDll.h"
#include "../inc/External/Common/GmmLibDllName.h"

#ifdef __cplusplus
}
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status)                      ((NTSTATUS)(Status) >= 0)
#endif

#ifndef STATUS_UNSUCCESSFUL
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS   ((NTSTATUS)0x00000000L)
#endif

extern int g_argc;
extern char** g_argv;

