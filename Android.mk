# Copyright(c) 2020 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files(the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and / or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libgmm_umd
LOCAL_PROPRIETARY_MODULE := true
LOCAL_SRC_FILES = \
    Source/GmmLib/CachePolicy/GmmCachePolicy.cpp \
    Source/GmmLib/CachePolicy/GmmCachePolicyCommon.cpp \
    Source/GmmLib/CachePolicy/GmmGen8CachePolicy.cpp \
    Source/GmmLib/CachePolicy/GmmGen9CachePolicy.cpp \
    Source/GmmLib/CachePolicy/GmmGen10CachePolicy.cpp \
    Source/GmmLib/CachePolicy/GmmGen11CachePolicy.cpp \
    Source/GmmLib/CachePolicy/GmmGen12CachePolicy.cpp \
    Source/GmmLib/CachePolicy/GmmGen12dGPUCachePolicy.cpp \
    Source/GmmLib/Platform/GmmGen11Platform.cpp \
    Source/GmmLib/Platform/GmmGen12Platform.cpp \
    Source/GmmLib/Platform/GmmGen8Platform.cpp \
    Source/GmmLib/Platform/GmmGen9Platform.cpp \
    Source/GmmLib/Platform/GmmGen10Platform.cpp \
    Source/GmmLib/Platform/GmmPlatform.cpp \
    Source/GmmLib/Resource/GmmResourceInfo.cpp \
    Source/GmmLib/Resource/GmmResourceInfoCommon.cpp \
    Source/GmmLib/Resource/GmmResourceInfoCommonEx.cpp \
    Source/GmmLib/Resource/GmmRestrictions.cpp \
    Source/GmmLib/Resource/Linux/GmmResourceInfoLinCWrapper.cpp \
    Source/GmmLib/Texture/GmmGen7Texture.cpp \
    Source/GmmLib/Texture/GmmGen8Texture.cpp \
    Source/GmmLib/Texture/GmmGen9Texture.cpp \
    Source/GmmLib/Texture/GmmGen10Texture.cpp \
    Source/GmmLib/Texture/GmmGen11Texture.cpp \
    Source/GmmLib/Texture/GmmGen12Texture.cpp \
    Source/GmmLib/Texture/GmmTexture.cpp \
    Source/GmmLib/Texture/GmmTextureAlloc.cpp \
    Source/GmmLib/Texture/GmmTextureSpecialCases.cpp \
    Source/GmmLib/Texture/GmmTextureOffset.cpp \
    Source/GmmLib/GlobalInfo/GmmInfo.cpp \
    Source/GmmLib/Utility/CpuSwizzleBlt/CpuSwizzleBlt.c \
    Source/GmmLib/Utility/GmmLibObject.cpp \
    Source/GmmLib/Utility/GmmLog/GmmLog.cpp \
    Source/GmmLib/Utility/GmmUtility.cpp \
    Source/GmmLib/TranslationTable/GmmAuxTable.cpp \
    Source/GmmLib/TranslationTable/GmmPageTableMgr.cpp \
    Source/GmmLib/TranslationTable/GmmUmdTranslationTable.cpp \
    Source/GmmLib/GlobalInfo/GmmClientContext.cpp \
    Source/GmmLib/GlobalInfo/GmmOldApi.cpp \
    Source/GmmLib/GlobalInfo/GmmLibDllMain.cpp \
    Source/Common/AssertTracer/AssertTracer.cpp

LOCAL_CFLAGS = \
    -DGMM_DYNAMIC_MOCS_TABLE=true \
    -DGMM_LIB_DLL \
    -DGMM_LIB_DLL_EXPORTS \
    -DGMM_UNIFIED_LIB \
    -DGMM_UNIFY_DAF_API \
    -DISTDLIB_UMD \
    -DSMALL_POOL_ALLOC \
    -DUNUSED_ISTDLIB_MT \
    -D_ATL_NO_WIN_SUPPORT \
    -D_X64 \
    -D__GFX_MACRO_C__ \
    -D__GMM \
    -D__STDC_CONSTANT_MACROS \
    -D__STDC_LIMIT_MACROS \
    -D__UMD \
    -Wno-error \
    -Digfx_gmmumd_dll_EXPORTS

LOCAL_CFLAGS += \
    -msse4 \
    -msse4.1 \
    -msse4.2

LOCAL_CPPFLAGS = \
    -fexceptions \
    -std=c++11

LOCAL_C_INCLUDES = \
    $(LOCAL_PATH)/Source/GmmLib \
    $(LOCAL_PATH)/Source/OpenGL/source/os/linux/oskl \
    $(LOCAL_PATH)/Source/GmmLib/Utility/GmmLog \
    $(LOCAL_PATH)/Source/GmmLib/inc \
    $(LOCAL_PATH)/Source/GmmLib/Utility \
    $(LOCAL_PATH)/Source/GmmLib/GlobalInfo \
    $(LOCAL_PATH)/Source/GmmLib/Texture \
    $(LOCAL_PATH)/Source/GmmLib/TranslationTable \
    $(LOCAL_PATH)/Source/GmmLib/Resource \
    $(LOCAL_PATH)/Source/GmmLib/Platform \
    $(LOCAL_PATH)/Source/util \
    $(LOCAL_PATH)/Source/inc \
    $(LOCAL_PATH)/Source/inc/common \
    $(LOCAL_PATH)/Source/inc/umKmInc \
    $(LOCAL_PATH)/Source/install

LOCAL_EXPORT_C_INCLUDE_DIRS = \
    $(LOCAL_PATH)/Source/GmmLib/inc \
    $(LOCAL_PATH)/Source/inc \
    $(LOCAL_PATH)/Source/inc/common

include $(BUILD_SHARED_LIBRARY)
