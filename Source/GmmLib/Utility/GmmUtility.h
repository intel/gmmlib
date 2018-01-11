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


#if __cplusplus
namespace GmmLib {
    namespace Utility {

#endif
#if (!defined(__GMM_KMD__) && (_DEBUG || _RELEASE_INTERNAL) && (_WIN32))
        extern bool GmmUMDReadRegistryFullPath(PCTSTR pszSubKeyPath, PCTSTR pszDWORDValueName, uint32_t * pdwData);
#endif

#if __cplusplus
        uint32_t GMM_STDCALL GmmGetNumPlanes(GMM_RESOURCE_FORMAT Format);
        GMM_RESOURCE_FORMAT GMM_STDCALL GmmGetFormatForASTC(uint8_t HDR, 
                                                            uint8_t Float, 
                                                            uint32_t BlockWidth, 
                                                            uint32_t BlockHeight, 
                                                            uint32_t BlockDepth);
#endif
#if __cplusplus
    }
}
#endif

#ifdef __GMM_KMD__
    void* GmmAllocKmdSystemMem(uint32_t Size, uint8_t bPageable, uint32_t Tag);
    void GmmFreeKmdSystemMem(void* pMem, uint32_t Tag);

    #define GMM_MALLOC(size)    GmmAllocKmdSystemMem((size), 0, GFX_COMPONENT_GMM_TAG)
    #define GMM_FREE(p)         GmmFreeKmdSystemMem((p), GFX_COMPONENT_GMM_TAG)
#else
    #define GMM_MALLOC(size)    malloc(size)
    #define GMM_FREE(p)         free(p)
#endif

#ifdef _WIN32
#ifdef __GMM_KMD__
extern NTSTATUS __GmmReadDwordKeyValue(char *pPath, WCHAR *pValueName, ULONG *pValueData);
extern NTSTATUS __GmmWriteDwordKeyValue(char *pCStringPath, WCHAR *pValueName, ULONG DWord);

#define REGISTRY_OVERRIDE_READ(Usage,  CacheParam)                                              \
        (__GmmReadDwordKeyValue(GMM_CACHE_POLICY_OVERRIDE_REGISTY_PATH_REGISTRY_KMD  #Usage ,   \
                                L#CacheParam,                                                   \
                                (ULONG*)&CacheParam) == STATUS_SUCCESS)
#define REGISTRY_OVERRIDE_WRITE(Usage,CacheParam,Value)                                         \
        __GmmWriteDwordKeyValue(GMM_CACHE_POLICY_OVERRIDE_REGISTY_PATH_REGISTRY_KMD #Usage ,    \
                                L#CacheParam,                                                   \
                                Value); 

#define GMM_REGISTRY_READ(Path, RegkeyName,RegkeyValue)                             \
        (__GmmReadDwordKeyValue(Path,                                               \
                                L#RegkeyName,                                       \
                                (ULONG*)&RegkeyValue) == STATUS_SUCCESS)
#else
#define REGISTRY_OVERRIDE_READ(Usage,CacheParam)                                                                \
        GmmLib::Utility::GmmUMDReadRegistryFullPath(GMM_CACHE_POLICY_OVERRIDE_REGISTY_PATH_REGISTRY_UMD #Usage ,\
                                                    #CacheParam,                                                \
                                                    (uint32_t*)&CacheParam)

#define GMM_REGISTRY_READ(Path, RegkeyName, RegkeyValue)                                \
        GmmLib::Utility::GmmUMDReadRegistryFullPath(Path,                               \
                                                    #RegkeyName,                        \
                                                    (uint32_t*)&RegkeyValue)
#endif
#endif

void GMM_STDCALL GmmGetCacheSizes(GMM_CACHE_SIZES* pCacheSizes);

/* Internal functions */
uint32_t   __GmmLog2(uint32_t Value);

