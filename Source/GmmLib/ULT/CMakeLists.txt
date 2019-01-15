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
     GmmGen10CachePolicyULT.h
     GmmGen10ResourceULT.h
     GmmGen11CachePolicyULT.h
     GmmGen9CachePolicyULT.h
     GmmGen9ResourceULT.h
     GmmResourceULT.h
     stdafx.h
     targetver.h
     )

set(GMMULT_SOURCES
    GmmCachePolicyULT.cpp
    GmmCommonULT.cpp
    GmmGen10CachePolicyULT.cpp
    GmmGen10ResourceULT.cpp
    GmmGen11CachePolicyULT.cpp
    GmmGen9CachePolicyULT.cpp
    GmmGen9ResourceULT.cpp
    GmmResourceCpuBltULT.cpp
    GmmResourceULT.cpp
    googletest/src/gtest-all.cc
    GmmULT.cpp
)

source_group("Source Files\\Cache Policy" FILES
            GmmCachePolicyULT.cpp
            GmmGen9CachePolicyULT.cpp
            GmmGen10CachePolicyULT.cpp
            GmmGen11CachePolicyULT.cpp
            )

source_group("Source Files\\Resource" FILES
            GmmGen10ResourceULT.cpp
            GmmGen9ResourceULT.cpp
            GmmResourceCpuBltULT.cpp
            GmmResourceULT.cpp
            )

source_group("Header Files\\Cache Policy" FILES
             GmmCachePolicyULT.h
             GmmGen10CachePolicyULT.h
             GmmGen11CachePolicyULT.h
             GmmGen9CachePolicyULT.h
            )

source_group("Header Files\\Resource" FILES
             GmmGen10ResourceULT.h
             GmmGen9ResourceULT.h
             GmmResourceULT.h
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
    COMMAND ${EXE_NAME}  --gtest_filter=CTest*
)