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

#if defined(_WIN64 ) || defined(__x86_64__) || defined(__LP64__)
    #define GMM_ENTRY_NAME      "OpenGmm"
    #define GMM_INIT_NAME       "GmmInit"
    #define GMM_DESTROY_NAME    "GmmDestroy"

    #if defined(_WIN64)
        #define GMM_UMD_DLL     "igdgmm64.dll"
    #else
        #define GMM_UMD_DLL     "libigdgmm.so.7"
    #endif
#else
    #define GMM_ENTRY_NAME      "_OpenGmm@4"

    #define GMM_INIT_NAME       "_GmmInit@48"
    #define GMM_DESTROY_NAME    "_GmmDestroy@4"

    #if defined(_WIN32)
        #define GMM_UMD_DLL     "igdgmm32.dll"
    #else
        #define GMM_UMD_DLL     "libigdgmm.so.7"
    #endif
#endif
