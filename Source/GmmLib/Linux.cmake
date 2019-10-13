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

#this file should contain only compiler and linker flags

SET (GMMLIB_COMPILER_FLAGS_COMMON
    #general warnings
    -Wall
    -Winit-self
    -Winvalid-pch
    -Wpointer-arith
    -Wno-unused
    -Wno-unknown-pragmas
    -Wno-comments
    -Wno-narrowing
    -Wno-overflow
    -Wno-parentheses
    -Wno-missing-braces
    -Wno-sign-compare
    -Werror=address
    -Werror=format-security
    -Werror=non-virtual-dtor
    -Werror=return-type

    # General optimization options
    -march=${GMMLIB_MARCH}
    -mpopcnt
    -msse
    -msse2
    -msse3
    -mssse3
    -msse4
    -msse4.1
    -msse4.2
    -mfpmath=sse
    -finline-functions
    -fno-short-enums
    -Wa,--noexecstack
    -fno-strict-aliasing
    # Common defines
    -DUSE_MMX
    -DUSE_SSE
    -DUSE_SSE2
    -DUSE_SSE3
    -DUSE_SSSE3
    # Other common flags
    -fstack-protector
    -fdata-sections
    -ffunction-sections
    -fmessage-length=0
    -fvisibility=hidden
    -fPIC
    -g
    # -m32 or -m64
    -m${GMMLIB_ARCH}
    )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#Gcc only flags
list (APPEND GMMLIB_COMPILER_FLAGS_COMMON
    -funswitch-loops
    -Wl,--no-undefined
    -Wl,--no-as-needed
    -Wl,--gc-sections
    )
endif()

SET (GMMLIB_COMPILER_CXX_FLAGS_COMMON
    #cpp
    -Wno-reorder
    -Wsign-promo
    -Wnon-virtual-dtor
    -Wno-invalid-offsetof
    -fvisibility-inlines-hidden
    -fno-use-cxa-atexit
    -fno-rtti
    -fexceptions
    -fcheck-new
    -std=c++11
    -pthread
    )

SET (GMMLIB_COMPILER_FLAGS_DEBUG
    -O0
    -DINSTR_GTUNE_EXT
    )

SET (GMMLIB_COMPILER_FLAGS_RELEASE
    -O2
    -fno-omit-frame-pointer
    #-flto
    #-Wl,-flto
    )

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")

list(APPEND GMMLIB_COMPILER_FLAGS_RELEASE
    -finline-limit=100
    )

endif()

#if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    # For LTO support, use special wrappers around ar and ranlib commands:
    # ... and if using "nm", use gcc-nm
 #   SET(CMAKE_AR "gcc-ar")
 #   SET(CMAKE_RANLIB "gcc-ranlib")
#endif()

SET( GMMLIB_COMPILER_FLAGS_RELEASEINTERNAL  ${GMMLIB_COMPILER_FLAGS_RELEASE})

#set predefined compiler flags set
add_compile_options("${GMMLIB_COMPILER_FLAGS_COMMON}")
add_compile_options("$<$<CONFIG:Debug>:${GMMLIB_COMPILER_FLAGS_DEBUG}>")
add_compile_options("$<$<CONFIG:Release>:${GMMLIB_COMPILER_FLAGS_RELEASE}>")
add_compile_options("$<$<CONFIG:ReleaseInternal>:${GMMLIB_COMPILER_FLAGS_RELEASEINTERNAL}>")
#cmake 3.3+, add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:${GMMLIB_COMPILER_CXX_FLAGS_COMMON}>")
foreach (flag ${GMMLIB_COMPILER_CXX_FLAGS_COMMON})
    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
endforeach()

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m${GMMLIB_ARCH}")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m${GMMLIB_ARCH}")
