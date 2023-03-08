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

set (EXE_NAME GMMULT)

set(GMMULT_HEADERS
     GmmCachePolicyULT.h
     GmmCommonULT.h
     GmmMultiAdapterULT.h
     GmmGen10CachePolicyULT.h
     GmmGen10ResourceULT.h
     GmmGen11CachePolicyULT.h
     GmmGen11ResourceULT.h
     GmmGen12ResourceULT.h
     GmmGen12dGPUResourceULT.h
     GmmGen12CachePolicyULT.h
     GmmGen12dGPUCachePolicyULT.h
     GmmXe_LPGCachePolicyULT.h
     GmmGen9CachePolicyULT.h
     GmmGen9ResourceULT.h
     GmmResourceULT.h
     GmmAuxTableULT.h
     stdafx.h
     targetver.h
     )

set(GMMULT_SOURCES
    GmmCachePolicyULT.cpp
    GmmCommonULT.cpp
    GmmMultiAdapterULT.cpp
    GmmGen10CachePolicyULT.cpp
    GmmGen10ResourceULT.cpp
    GmmGen11CachePolicyULT.cpp
    GmmGen12CachePolicyULT.cpp
    GmmGen12dGPUCachePolicyULT.cpp
    GmmXe_LPGCachePolicyULT.cpp
    GmmGen11ResourceULT.cpp
    GmmGen12ResourceULT.cpp
    GmmGen12dGPUResourceULT.cpp
    GmmGen9CachePolicyULT.cpp
    GmmGen9ResourceULT.cpp
    GmmResourceCpuBltULT.cpp
    GmmResourceULT.cpp
    GmmAuxTableULT.cpp
    googletest/src/gtest-all.cc
    GmmULT.cpp
)

source_group("Source Files\\Cache Policy" FILES
            GmmCachePolicyULT.cpp
            GmmGen9CachePolicyULT.cpp
            GmmGen10CachePolicyULT.cpp
            GmmGen11CachePolicyULT.cpp
            GmmGen12CachePolicyULT.cpp
            GmmGen12dGPUCachePolicyULT.cpp
            GmmXe_LPGCachePolicyULT.cpp
	    )

source_group("Source Files\\Resource" FILES
            GmmGen10ResourceULT.cpp
            GmmGen11ResourceULT.cpp
            GmmGen12ResourceULT.cpp
            GmmGen12dGPUResourceULT.cpp
            GmmGen9ResourceULT.cpp
            GmmResourceCpuBltULT.cpp
            GmmResourceULT.cpp
            )

source_group("Source Files\\TranslationTable" FILES
            GmmAuxTableULT.cpp
            )

source_group("Source Files\\MultiAdapter" FILES
            GmmMultiAdapterULT.cpp
            )

source_group("Header Files\\TranslationTable" FILES
            GmmAuxTableULT.h
            )

source_group("Header Files\\Cache Policy" FILES
             GmmCachePolicyULT.h
             GmmGen10CachePolicyULT.h
             GmmGen11CachePolicyULT.h
             GmmGen12CachePolicyULT.h
             GmmXe_LPGCachePolicyULT.h
	     GmmGen12dGPUCachePolicyULT.h
             GmmGen9CachePolicyULT.h
            )

source_group("Header Files\\Resource" FILES
             GmmGen10ResourceULT.h
             GmmGen11ResourceULT.h
             GmmGen12ResourceULT.h
             GmmGen12dGPUResourceULT.h
             GmmGen9ResourceULT.h
             GmmResourceULT.h
            )

source_group("Header Files\\MultiAdapter" FILES
            GmmMultiAdapterULT.h
            )

source_group("gtest" FILES
            googletest/gtest/gtest.h
            googletest/src/gtest-all.cc
            )

include_directories(BEFORE ./)

include_directories(BEFORE ${PROJECT_SOURCE_DIR})

include_directories(
    googletest
    googletest/gtest
    ${BS_DIR_INC}/umKmInc
    ${BS_DIR_INC}
    ${BS_DIR_GMMLIB}/inc
    ${BS_DIR_INC}/common
    )

macro(GmmLibULTSetTargetConfig ultTarget)
	if (TARGET ${ultTarget})
			set_property(TARGET ${ultTarget} APPEND PROPERTY COMPILE_DEFINITIONS
				$<$<CONFIG:Release>: _RELEASE>
				$<$<CONFIG:ReleaseInternal>: _RELEASE_INTERNAL>
				$<$<CONFIG:Debug>: _DEBUG>
			)
		endif()

endmacro()

add_executable(${EXE_NAME} ${GMMULT_HEADERS} ${GMMULT_SOURCES})

GmmLibULTSetTargetConfig(${EXE_NAME})

set_property(TARGET ${EXE_NAME} APPEND PROPERTY COMPILE_DEFINITIONS __GMM GMM_LIB_DLL __UMD)

if(NOT TARGET igfx_gmmumd_dll)
    add_subdirectory("${BS_DIR_GMMLIB}" "${CMAKE_BINARY_DIR}/gmmlib/ult")
endif()
target_link_libraries(${EXE_NAME} igfx_gmmumd_dll)

target_link_libraries(${EXE_NAME}
    pthread
    dl
)

add_custom_target(Run_ULT ALL DEPENDS GMMULT)

add_custom_command(
    TARGET Run_ULT
    POST_BUILD
    COMMAND echo running ULTs
    COMMAND "${CMAKE_COMMAND}" -E env "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:igfx_gmmumd_dll>" ${CMAKE_CFG_INTDIR}/${EXE_NAME} --gtest_filter=CTest*
)

add_test(
    NAME ULT
    COMMAND env "LD_LIBRARY_PATH=$<TARGET_FILE_DIR:igfx_gmmumd_dll>" $<TARGET_FILE:${EXE_NAME}> --gtest_filter=CTest*
)
