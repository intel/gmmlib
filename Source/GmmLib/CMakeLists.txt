
# Copyright(c) 2017 Intel Corporation

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


cmake_minimum_required(VERSION 3.5)
project(igfx_gmmumd)

# GmmLib Api Version used for so naming
set(GMMLIB_API_MAJOR_VERSION 11)
set(GMMLIB_API_MINOR_VERSION 2)

if(NOT DEFINED MAJOR_VERSION)
	set(MAJOR_VERSION 11)
endif()

if(NOT DEFINED MINOR_VERSION)
	set(MINOR_VERSION 2)
endif()

if(NOT DEFINED PATCH_VERSION)
	set(PATCH_VERSION 0)
endif()

if(NOT DEFINED GMMLIB_API_PATCH_VERSION)
    set(GMMLIB_API_PATCH_VERSION ${PATCH_VERSION})
endif()

message(STATUS "API version: ${GMMLIB_API_MAJOR_VERSION}.${GMMLIB_API_MINOR_VERSION}.${GMMLIB_API_PATCH_VERSION}")
message(STATUS "Package version: ${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

if(NOT DEFINED BS_USE_OSDM_BUILD_SYSTEM)
    if(DEFINED ENV{BS_USE_OSDM_BUILD_SYSTEM})
        set(BS_USE_OSDM_BUILD_SYSTEM "$ENV{BS_USE_OSDM_BUILD_SYSTEM}")
    else()
        set(BS_USE_OSDM_BUILD_SYSTEM FALSE)
    endif()
endif()

# begin -- label bldsys file prologue
# WARNING: The "project" statement triggers reading of CMAKE_TOOLCHAIN_FILE
#   and so must precede the inclusion below of bs_init.cmake .
function(bs_find_build_system gfx_dev_dir build_sys_dir build_sys_inc)
    # If we are not building as a standalone project
    if(DEFINED GFX_DEVELOPMENT_DIR)
        set(_bs_gfx_development_dir "${GFX_DEVELOPMENT_DIR}")
    elseif(DEFINED ENV{GFX_DEVELOPMENT_DIR})
        set(_bs_gfx_development_dir "$ENV{GFX_DEVELOPMENT_DIR}")
    else()
        get_filename_component(_bs_cur_cmake_dir "${CMAKE_CURRENT_LIST_FILE}" PATH)
        get_filename_component(_bs_parent_dir "${_bs_cur_cmake_dir}" DIRECTORY)
        set(_bs_gfx_dir_found false)
        while(NOT _bs_gfx_dir_found)
            set(_bs_bldsys_dir "${_bs_parent_dir}/Tools/bldsys")
            if(EXISTS ${_bs_bldsys_dir})
                set(_bs_gfx_development_dir ${_bs_parent_dir})
                set(_bs_gfx_dir_found true)
                break()
            endif()
            get_filename_component(_bs_parent_dir "${_bs_parent_dir}" DIRECTORY)
            if(${_bs_parent_dir} STREQUAL "/")
                break()
            endif()
        endwhile(NOT _bs_gfx_dir_found)
        if (NOT _bs_gfx_development_dir)
            message(FATAL_ERROR "GFX_DEVELOPMENT_DIR not found (${_bs_gfx_development_dir}) - exiting!")
            exit(1)
        endif()
    endif()
    set(${gfx_dev_dir} "${_bs_gfx_development_dir}" PARENT_SCOPE)
    set(${build_sys_dir} "${_bs_gfx_development_dir}/Tools/bldsys" PARENT_SCOPE)
    set(${build_sys_inc} "${_bs_gfx_development_dir}/Tools/bldsys/include" PARENT_SCOPE)
endfunction(bs_find_build_system)

bs_find_build_system(GFX_DEVELOPMENT_DIR BUILD_SYS_DIR BUILD_SYS_INC)

include(${BUILD_SYS_DIR}/bs_init.cmake)
include(${BUILD_SYS_INC}/bs_dir_names.cmake)
# file prologue done


set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

################################################################################
# Define GMM_DLL Target
################################################################################
set (GMM_LIB_DLL_NAME igfx_gmmumd_dll)

macro(GmmLibSetTargetConfig libTarget)
	if (TARGET ${libTarget})
			set_property(TARGET ${libTarget} APPEND PROPERTY COMPILE_DEFINITIONS
				$<$<CONFIG:Release>: _RELEASE>
				$<$<CONFIG:ReleaseInternal>: _RELEASE_INTERNAL>
				$<$<CONFIG:Debug>: _DEBUG>
			)
		endif()

endmacro()

if(CMAKE_CONFIGURATION_TYPES)
  set( CMAKE_CONFIGURATION_TYPES
      "Debug"
      "Release"
      "ReleaseInternal")

  set( CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
       "Reset the configurations to what we need"
       FORCE)
endif()

set(GMMLIB_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (GMMLIB_ARCH "64")
else()
    set (GMMLIB_ARCH "32")
endif()

if ("${BUILD_TYPE}" STREQUAL "release")
    set(CMAKE_BUILD_TYPE "Release")
elseif ("${BUILD_TYPE}" STREQUAL "release-internal")
    set(CMAKE_BUILD_TYPE "ReleaseInternal")
elseif ("${BUILD_TYPE}" STREQUAL "debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if("${GMMLIB_MARCH}" STREQUAL "")
    set(GMMLIB_MARCH "corei7")
endif()

MESSAGE("platform: ${CMAKE_HOST_SYSTEM_NAME}")
MESSAGE("source_dir: ${BS_DIR_GMMLIB}")
MESSAGE("arch: ${GMMLIB_ARCH}")
MESSAGE("build type: ${CMAKE_BUILD_TYPE}")
MESSAGE("SourceConfiguration:")
MESSAGE("CommonDir: ${BS_DIR_COMMON}")
MESSAGE("IncDir: ${BS_DIR_INC}")

# If '-DGMM_DYNAMIC_MOCS_TABLE=TRUE' (default is FALSE) passed to cmake
# configure command gmmlib will generate MOCS table dynamically depending on
# usage requests (Gen9). In this case on Linux user responsibility is to
# make sure that generated MOCS table is programmed on KMD level.
if (GMM_DYNAMIC_MOCS_TABLE)
    MESSAGE("MOCS table: Dynamic")
    add_definitions(-DGMM_DYNAMIC_MOCS_TABLE)
else()
    MESSAGE("MOCS table: Static")
endif()

if(DEFINED UFO_DRIVER_OPTIMIZATION_LEVEL)
    if(${UFO_DRIVER_OPTIMIZATION_LEVEL} GREATER 0)
        add_definitions(-DGMM_GFX_GEN=${GFXGEN})
    endif()
endif()

set(HEADERS_
	${BS_DIR_GMMLIB}/CachePolicy/GmmCachePolicyConditionals.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmCachePolicyResourceUsageDefinitions.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmCachePolicyUndefineConditionals.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmGen10CachePolicy.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmGen11CachePolicy.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmGen12CachePolicy.h
        ${BS_DIR_GMMLIB}/CachePolicy/GmmGen12dGPUCachePolicy.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmGen8CachePolicy.h
	${BS_DIR_GMMLIB}/CachePolicy/GmmGen9CachePolicy.h
	${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen10.h
	${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen11.h
	${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen12.h
        ${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen12dGPU.h
	${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen8.h
	${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen9.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmCachePolicy.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmCachePolicyCommon.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmCachePolicyExt.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmCommonExt.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmConst.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmDebug.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmFormatTable.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmHw.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmInfo.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmInfoExt.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmInternal.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmMemAllocator.hpp

	${BS_DIR_GMMLIB}/inc/External/Common/GmmPlatformExt.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmProto.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceFlags.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceInfo.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceInfoCommon.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceInfoExt.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmTextureExt.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmUtil.h
	${BS_DIR_GMMLIB}/inc/External/Linux/GmmResourceInfoLin.h
        ${BS_DIR_GMMLIB}/inc/External/Linux/GmmResourceInfoLinExt.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen10Platform.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen11Platform.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen12Platform.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen8Platform.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen9Platform.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen10TextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen11TextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen12TextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen7TextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen8TextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen9TextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmTextureCalc.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/GmmCommonInt.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/GmmLibInc.h
	${BS_DIR_GMMLIB}/inc/GmmLib.h
	${BS_DIR_GMMLIB}/inc/Internal/Common/GmmLogger.h
)

set(UMD_HEADERS
	${HEADERS_}
	${BS_DIR_GMMLIB}/inc/External/Common/GmmPageTableMgr.h
	${BS_DIR_GMMLIB}/inc/External/Common/GmmClientContext.h
        ${BS_DIR_GMMLIB}/inc/External/Common/GmmLibDll.h
        ${BS_DIR_GMMLIB}/inc/External/Common/GmmLibDllName.h
	)


set(SOURCES_
  ${BS_DIR_COMMON}/AssertTracer/AssertTracer.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmCachePolicy.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmCachePolicyCommon.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmGen8CachePolicy.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmGen9CachePolicy.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmGen10CachePolicy.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmGen11CachePolicy.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmGen12CachePolicy.cpp
  ${BS_DIR_GMMLIB}/CachePolicy/GmmGen12dGPUCachePolicy.cpp
  ${BS_DIR_GMMLIB}/Platform/GmmGen11Platform.cpp
  ${BS_DIR_GMMLIB}/Platform/GmmGen12Platform.cpp
  ${BS_DIR_GMMLIB}/Platform/GmmGen8Platform.cpp
  ${BS_DIR_GMMLIB}/Platform/GmmGen9Platform.cpp
  ${BS_DIR_GMMLIB}/Platform/GmmGen10Platform.cpp
  ${BS_DIR_GMMLIB}/Platform/GmmPlatform.cpp
  ${BS_DIR_GMMLIB}/Resource/GmmResourceInfo.cpp
  ${BS_DIR_GMMLIB}/Resource/GmmResourceInfoCommon.cpp
  ${BS_DIR_GMMLIB}/Resource/GmmResourceInfoCommonEx.cpp
  ${BS_DIR_GMMLIB}/Resource/GmmRestrictions.cpp
  ${BS_DIR_GMMLIB}/Resource/Linux/GmmResourceInfoLinCWrapper.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmGen7Texture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmGen8Texture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmGen9Texture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmGen10Texture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmGen11Texture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmGen12Texture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmTexture.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmTextureAlloc.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmTextureSpecialCases.cpp
  ${BS_DIR_GMMLIB}/Texture/GmmTextureOffset.cpp
  ${BS_DIR_GMMLIB}/GlobalInfo/GmmInfo.cpp
  ${BS_DIR_GMMLIB}/Utility/CpuSwizzleBlt/CpuSwizzleBlt.c
  ${BS_DIR_GMMLIB}/Utility/GmmLibObject.cpp
  ${BS_DIR_GMMLIB}/Utility/GmmLog/GmmLog.cpp
  ${BS_DIR_GMMLIB}/Utility/GmmUtility.cpp
)

set(UMD_SOURCES
  ${SOURCES_}
  ${BS_DIR_GMMLIB}/TranslationTable/GmmAuxTable.cpp
  ${BS_DIR_GMMLIB}/TranslationTable/GmmPageTableMgr.cpp
  ${BS_DIR_GMMLIB}/TranslationTable/GmmUmdTranslationTable.cpp
  ${BS_DIR_GMMLIB}/GlobalInfo/GmmClientContext.cpp
  ${BS_DIR_GMMLIB}/GlobalInfo/GmmOldApi.cpp
   ${BS_DIR_GMMLIB}/GlobalInfo/GmmLibDllMain.cpp
  )

source_group("Source Files\\Cache Policy\\Client Files" FILES
			${BS_DIR_GMMLIB}/CachePolicy/GmmCachePolicyResourceUsageDefinitions.h
			${BS_DIR_GMMLIB}/CachePolicy/GmmGen10CachePolicy.h
			${BS_DIR_GMMLIB}/CachePolicy/GmmGen11CachePolicy.h
			${BS_DIR_GMMLIB}/CachePolicy/GmmGen12CachePolicy.h
                        ${BS_DIR_GMMLIB}/CachePolicy/GmmGen12dGPUCachePolicy.h
			${BS_DIR_GMMLIB}/CachePolicy/GmmGen8CachePolicy.h
			${BS_DIR_GMMLIB}/CachePolicy/GmmGen9CachePolicy.h
			)

source_group("Source Files\\Cache Policy" ${BS_DIR_GMMLIB}/CachePolicy/*.cpp)
source_group("Source Files\\Global" ${BS_DIR_GMMLIB}/GlobalInfo/.*)
source_group("Source Files\\Platform" ${BS_DIR_GMMLIB}/Platform/.*)
source_group("Source Files\\Texture" ${BS_DIR_GMMLIB}/Texture/.*)
source_group("Source Files\\Translation Table" ${BS_DIR_GMMLIB}/TranslationTable/.*)
source_group("Source Files\\Utility" ${BS_DIR_GMMLIB}/Utility/.*)

source_group("Source Files\\Resource" FILES
			${BS_DIR_GMMLIB}/Resource/GmmResourceInfo.cpp
			${BS_DIR_GMMLIB}/Resource/GmmResourceInfoCommon.cpp
			${BS_DIR_GMMLIB}/Resource/GmmResourceInfoCommonEx.cpp
			${BS_DIR_GMMLIB}/Resource/GmmRestrictions.cpp)

source_group("Source Files\\Resource\\Linux" FILES
			${BS_DIR_GMMLIB}/Resource/Linux/GmmResourceInfoLinCWrapper.cpp
			)

source_group("Source Files\\TranslationTable\\Windows" FILES
			 ${BS_DIR_GMMLIB}/TranslationTable/GmmAuxTable.cpp
			 ${BS_DIR_GMMLIB}/TranslationTable/GmmPageTableMgr.cpp
			 ${BS_DIR_GMMLIB}/TranslationTable/GmmUmdTranslationTable.cpp)

source_group("Source Files\\TranslationTable" FILES
			${BS_DIR_GMMLIB}/TranslationTable/GmmUmdTranslationTable.h)

source_group("Header Files\\External\\Common" FILES
			${BS_DIR_GMMLIB}/inc/External/Common/GmmCachePolicy.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmCachePolicyCommon.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmCachePolicyExt.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmCommonExt.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmConst.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmDebug.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmFormatTable.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmHw.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmInfo.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmInfoExt.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmInternal.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmMemAllocator.hpp
			${BS_DIR_GMMLIB}/inc/External/Common/GmmPageTableMgr.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmPlatformExt.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceFlags.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceInfo.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceInfoCommon.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmResourceInfoExt.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmTextureExt.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmUtil.h
			${BS_DIR_GMMLIB}/inc/External/Common/GmmClientContext.h
			)

source_group("Header Files" FILES
			${BS_DIR_GMMLIB}/inc/GmmLib.h
			)

source_group("Header Files\\External\\Common\\Cache Policy" FILES
			${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen10.h
			${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen11.h
			${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen12.h
                        ${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen12dGPU.h
			${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen8.h
			${BS_DIR_GMMLIB}/inc/External/Common/CachePolicy/GmmCachePolicyGen9.h
			)

source_group("Header Files\\External\\Linux" FILES
			 ${BS_DIR_GMMLIB}/inc/External/Linux/GmmResourceInfoLin.h
			 ${BS_DIR_GMMLIB}/inc/External/Linux/GmmResourceInfoLinExt.h
			)

source_group("Header Files\\Internal\\Common" FILES
			${BS_DIR_GMMLIB}/inc/Internal/Common/GmmLibInc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/GmmProto.h
			)

source_group("Header Files\\Internal\\Common\\Platform" FILES
			${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen10Platform.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen11Platform.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen12Platform.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen8Platform.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Platform/GmmGen9Platform.h
			)

source_group("Header Files\\Internal\\Common\\Texture" FILES
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen10TextureCalc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen11TextureCalc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen12TextureCalc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen7TextureCalc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen8TextureCalc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmGen9TextureCalc.h
			${BS_DIR_GMMLIB}/inc/Internal/Common/Texture/GmmTextureCalc.h
			)

include_directories(BEFORE ${BS_DIR_GMMLIB}/)

include_directories(BEFORE ${PROJECT_SOURCE_DIR})

bs_add_some_common_includes()

  include_directories(
  ${BS_DIR_GMMLIB}
  ${BS_DIR_GMMLIB}/Utility/GmmLog
  ${BS_DIR_GMMLIB}/inc
  ${BS_DIR_GMMLIB}/Utility
  ${BS_DIR_GMMLIB}/GlobalInfo
  ${BS_DIR_GMMLIB}/Texture
  ${BS_DIR_GMMLIB}/Resource
  ${BS_DIR_GMMLIB}/Platform
  ${BS_DIR_UTIL}
  ${BS_DIR_INC}
  ${BS_DIR_INC}/common
  ${BS_DIR_INC}/umKmInc
  ${BS_DIR_INSTALL}
  #${BS_DIR_ANDROID}/include
  )

set(headers
	${HEADERS_}
)

  set(SOURCES
    ${SOURCES_}
  )

# set compiler options
include(Linux.cmake)

###################################################################################
# create dll library
###################################################################################
add_library( ${GMM_LIB_DLL_NAME} SHARED igdgmm.rc ${UMD_SOURCES} ${UMD_HEADERS})

if(MSVC)

set_target_properties(${GMM_LIB_DLL_NAME} PROPERTIES OUTPUT_NAME "igdgmm${GMMLIB_ARCH}")

bs_set_wdk(${GMM_LIB_DLL_NAME})
GmmLibSetTargetConfig( ${GMM_LIB_DLL_NAME} )

set_target_properties(${GMM_LIB_DLL_NAME} PROPERTIES VS_GLOBAL_DriverTargetPlatform Universal)
set_target_properties(${GMM_LIB_DLL_NAME} PROPERTIES VS_PLATFORM_TOOLSET WindowsApplicationForDrivers10.0)
windows_umd_props_universal(${GMM_LIB_DLL_NAME})

target_link_libraries( ${GMM_LIB_DLL_NAME}
            onecoreuap.lib
        )
else()
	set_target_properties(${GMM_LIB_DLL_NAME} PROPERTIES OUTPUT_NAME "igdgmm")
	set_target_properties(${GMM_LIB_DLL_NAME} PROPERTIES VERSION ${GMMLIB_API_MAJOR_VERSION}.${GMMLIB_API_MINOR_VERSION}.${GMMLIB_API_PATCH_VERSION})
	set_target_properties(${GMM_LIB_DLL_NAME} PROPERTIES SOVERSION ${GMMLIB_API_MAJOR_VERSION})
        set(THREADS_PREFER_PTHREAD_FLAG ON)
        find_package(Threads REQUIRED)
        target_link_libraries(${GMM_LIB_DLL_NAME} Threads::Threads)

endif()


###################################################################################
# End of DLL create library
###################################################################################

bs_set_defines()
bs_set_force_exceptions()
bs_set_post_target()

###################################################################################
# Set common macros for DLL
###################################################################################
bs_set_extra_target_properties(${GMM_LIB_DLL_NAME}
      _ATL_NO_WIN_SUPPORT
      SMALL_POOL_ALLOC
      __GMM
      __GFX_MACRO_C__
      UNUSED_ISTDLIB_MT
	  __UMD
	  GMM_UNIFY_DAF_API
	  )

if(CMAKE_BUILD_TYPE STREQUAL "ReleaseInternal")
	bs_set_extra_target_properties(${GMM_LIB_DLL_NAME} _RELEASE_INTERNAL)
endif()

target_include_directories(${GMM_LIB_DLL_NAME} INTERFACE
        ${BS_DIR_GMMLIB}/inc
        ${BS_DIR_INC}
        ${BS_DIR_INC}/common)

###################################################################################
# End of Set macros DLL
###################################################################################

bs_set_extra_target_properties(${GMM_LIB_DLL_NAME}
	ISTDLIB_UMD
	UNUSED_ISTDLIB_MT
	GMM_UNIFIED_LIB
	GMM_LIB_DLL
        GMM_LIB_DLL_EXPORTS
)

if("${GMMLIB_ARCH}" MATCHES "64")
	bs_set_extra_target_properties(${GMM_LIB_DLL_NAME}
		_X64)
endif()

if(NOT DEFINED RUN_TEST_SUITE OR RUN_TEST_SUITE)
    add_subdirectory(ULT)
endif()

set (GMM_UMD_DLL "igdgmm")

include(os_release_info.cmake)

get_os_release_info(os_name os_version)

if("${os_name}" STREQUAL "clear-linux-os")
    # clear-linux-os distribution avoids /etc for distribution defaults.
    # Set this variable explicitly before including GNUInstallDirs.
    set(CMAKE_INSTALL_SYSCONFDIR "usr/share/defaults/etc")
endif()

if(UNIX)
    include(GNUInstallDirs)

    configure_file(${BS_DIR_GMMLIB}/igdgmm.h.in ${CMAKE_BINARY_DIR}/igdgmm.h)
    configure_file(${BS_DIR_GMMLIB}/igdgmm.pc.in ${CMAKE_BINARY_DIR}/igdgmm.pc @ONLY)

    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tmp/postinst "/sbin/ldconfig\n")
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tmp/postrm "/sbin/ldconfig\n")
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/tmp/postinst DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    file(COPY ${CMAKE_CURRENT_BINARY_DIR}/tmp/postrm DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

    install(TARGETS ${GMM_LIB_DLL_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT gmmlib NAMELINK_SKIP)
    install(TARGETS ${GMM_LIB_DLL_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT gmmlib-devel NAMELINK_ONLY)

    install(DIRECTORY ${BS_DIR_GMMLIB} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igdgmm COMPONENT gmmlib-devel
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "Internal" EXCLUDE
        PATTERN "ULT" EXCLUDE
	PATTERN "spdlog" EXCLUDE)

    install (DIRECTORY ${BS_DIR_INC} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igdgmm COMPONENT gmmlib-devel
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp")

    install (DIRECTORY ${BS_DIR_UTIL} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igdgmm COMPONENT gmmlib-devel
        FILES_MATCHING PATTERN "*.h"
        PATTERN "*.hpp")

    install (FILES ${BS_DIR_GMMLIB}/Utility/CpuSwizzleBlt/CpuSwizzleBlt.c
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igdgmm/GmmLib/Utility/CpuSwizzleBlt/ COMPONENT gmmlib-devel)

    install(FILES ${CMAKE_BINARY_DIR}/igdgmm.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig COMPONENT gmmlib-devel)
    install(FILES ${CMAKE_BINARY_DIR}/igdgmm.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/igdgmm COMPONENT gmmlib-devel)

    if(GMMLIB_CPACK_GENERATOR)
        set(CPACK_GENERATOR "${GMMLIB_CPACK_GENERATOR}")
    else()
        # If generators list was not define build native package for current distro
        if(EXISTS "/etc/debian_version")
        set(CPACK_GENERATOR "DEB")
        elseif(EXISTS "/etc/redhat-release")
        set(CPACK_GENERATOR "RPM")
        else()
        set(CPACK_GENERATOR "TXZ")
        endif()
    endif()

    set(CPACK_PACKAGE_NAME "intel")
    set(CPACK_PACKAGE_VENDOR "Intel Corporation")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Intel(R) Graphics Memory Management Library Package")
    set(CPACK_PACKAGE_ARCHITECTURE "x86_64")

    set(CPACK_PACKAGE_VERSION_MAJOR ${MAJOR_VERSION})
    set(CPACK_PACKAGE_VERSION_MINOR ${MINOR_VERSION})
    set(CPACK_PACKAGE_VERSION_PATCH ${PATCH_VERSION})
    set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

    set(CPACK_PACKAGE_INSTALL_DIRECTORY ${GMMLIB_INSTALL_TIME_ROOT_DIR})
    set(CPACK_SET_DESTDIR TRUE)
    set(CPACK_PACKAGE_RELOCATABLE FALSE)

    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Intel")
    set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_BINARY_DIR}/postinst;${CMAKE_CURRENT_BINARY_DIR}/postrm")

    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    set(CPACK_RPM_PACKAGE_RELEASE 1)
    set(CPACK_RPM_COMPRESSION_TYPE "xz")
    set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/postinst")
    set(CPACK_RPM_POST_UNINSTALL_SCRIPT_FILE "${CMAKE_CURRENT_BINARY_DIR}/postrm")

    if(CMAKE_VERSION VERSION_GREATER 3.6 OR CMAKE_VERSION VERSION_EQUAL 3.6)
      set(CPACK_DEBIAN_GMMLIB_FILE_NAME "intel-gmmlib_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")
      set(CPACK_DEBIAN_GMMLIB-DEVEL_FILE_NAME "intel-gmmlib-devel_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")

      set(CPACK_RPM_GMMLIB_FILE_NAME "intel-gmmlib-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}%{?dist}.${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")
      set(CPACK_RPM_GMMLIB-DEVEL_FILE_NAME "intel-gmmlib-devel-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}%{?dist}.${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")

      set(CPACK_ARCHIVE_GMMLIB_FILE_NAME "intel-gmmlib-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCHITECTURE}")
      set(CPACK_ARCHIVE_GMMLIB-DEVEL_FILE_NAME "intel-gmmlib-devel-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCHITECTURE}")
    else()
      if(CPACK_GENERATOR STREQUAL "DEB")
        set(CPACK_PACKAGE_FILE_NAME "intel-gmmlib_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
      elseif(CPACK_GENERATOR STREQUAL "RPM")
        set(CPACK_PACKAGE_FILE_NAME "intel-gmmlib-${CPACK_PACKAGE_VERSION}-${CPACK_RPM_PACKAGE_RELEASE}%{?dist}.${CPACK_RPM_PACKAGE_ARCHITECTURE}.rpm")
      else()
        set(CPACK_PACKAGE_FILE_NAME "intel-gmmlib-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCHITECTURE}")
      endif()
    endif()

    set(CPACK_DEBIAN_GMMLIB-DEVEL_PACKAGE_DEPENDS "intel-gmmlib(=${CPACK_PACKAGE_VERSION})")
    set(CPACK_RPM_GMMLIB-DEVEL_PACKAGE_REQUIRES "intel-gmmlib = ${CPACK_PACKAGE_VERSION}")

    set(CPACK_COMPONENT_INSTALL ON)
    set(CPACK_DEB_COMPONENT_INSTALL ON)
    set(CPACK_RPM_COMPONENT_INSTALL ON)
    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    set(CPACK_COMPONENTS_ALL gmmlib gmmlib-devel)

    include (CPack)

endif()
