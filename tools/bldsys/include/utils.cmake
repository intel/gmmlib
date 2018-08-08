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

# utility functions for cmake

if(NOT DEFINED _bs_include_utils)
set(_bs_include_utils TRUE)

include(${BUILD_SYS_INC}/bs_base_utils.cmake)


# macro bs_assignments_from_file - Execute a file of assignments.
# This must be a macro and not a function so as to not provide an
# additional level of symbol scope.
#
# For an example, see file include/bs_enable.mk .
#
# Example file content:
#     # A comment looks like this.  Assignments follow:
#     BS_ENABLE_gmmlib          = 0
#     BS_ENABLE_igc             = 0
macro(bs_assignments_from_file file_path)
    file(STRINGS "${file_path}" bsa_list)
    bs_assignments_from_string("${bsa_list}")
endmacro(bs_assignments_from_file)


# Function to capitalize SourceString and return
# the result on ResultString
function(_bs_capitalize SourceString ResultString)
    string(SUBSTRING ${SourceString} 0 1 FIRST_LETTER)
    string(TOUPPER ${FIRST_LETTER} FIRST_LETTER)
    string(REGEX REPLACE "^.(.*)" "${FIRST_LETTER}\\1" CapString "${SourceString}")
    set(${ResultString} ${CapString} PARENT_SCOPE)
endfunction(_bs_capitalize)

# Function to return the canonical string for gen
# Ex: gen7.5 becomes gen75
function(bs_canonicalize_string_gen SourceGen AllGens)
    if ( ${SourceGen} MATCHES "[0-9].[0-9]")
        STRING(REPLACE "." "" tempstr ${SourceGen})
    else()
        set(tempstr "${SourceGen}0")
    endif()
    set(${AllGens} ${tempstr} PARENT_SCOPE)
endfunction(bs_canonicalize_string_gen)

# bs_list_to_string - Convert a cmake list to a string.
macro(bs_list_to_string varnam)
    string(REPLACE ";" " " ${varnam} "${${varnam}}")
endmacro(bs_list_to_string)

# bs_string_to_list - Convert a string to a cmake list.
macro(bs_string_to_list varnam)
    string(REPLACE " " ";" ${varnam} "${${varnam}}")
endmacro(bs_string_to_list)

# Macro to find the xsltproc installed on
# the system.
macro(bs_find_xsltproc)
    if (NOT ${PLATFORM} STREQUAL Windows)
        find_program(XSLTPROC xsltproc)
    else()
        find_program(XSLTPROC NAMES xslt msxsl msxsl.exe PATHS "${BS_DIR_INSTRUMENTATION}/tools")
    endif()
endmacro(bs_find_xsltproc)

# Macro to find the python installed on
# the system.
macro(bs_find_python)
    if (NOT ${PLATFORM} STREQUAL Windows)
        find_program(PYTHON python)
    else()
        find_program(PYTHON NAMES python.exe PATHS "${GFX_DEVELOPMENT_DIR}/Tools/Build/scripts/deps/python27" NO_DEFAULT_PATH)
    endif()
    message("python is ${PYTHON}")
endmacro(bs_find_python)

# Macro to find the flex installed on
# the system.
macro(bs_find_flex)
    if (NOT ${PLATFORM} STREQUAL Windows)
        find_program(FLEX flex)
    else()
        find_program(FLEX NAMES flex flex.exe PATHS "${GFX_DEVELOPMENT_DIR}/Tools//OpenGL/BisonFlex/win")
    endif()
endmacro(bs_find_flex)

# Macro to find the bison installed on
# the system.
macro(bs_find_bison)
    if (NOT ${PLATFORM} STREQUAL Windows)
        find_program(BISON bison)
    else()
        find_program(BISON NAMES bison bison.exe PATHS "${GFX_DEVELOPMENT_DIR}/Tools//OpenGL/BisonFlex/win")
    endif()
endmacro(bs_find_bison)

# Macro to find the patch installed on
# the system.
macro(bs_find_patch)
    find_program(PATCH NAMES patch patch.exe)
endmacro()

macro(bs_find_7z)
    if (NOT ${PLATFORM} STREQUAL Windows)
        find_program(7Z NAMES 7za 7z)
    else()
        find_program(7Z NAMES 7z.exe PATHS "${GFX_DEVELOPMENT_DIR}/Tools/Build/scripts/deps/7zip")
    endif()
endmacro()


function(bs_check_component_enable component_name enable_flag_name)
    set(_component_enable "${BS_ENABLE_${component_name}}")
    if("${_component_enable}" STREQUAL "1")
        set(_enable_flag 1)
    elseif("${_component_enable}" STREQUAL "W")
        if("${PLATFORM}" STREQUAL "Windows")
            set(_enable_flag 1)
        else()
            set(_enable_flag 0)
        endif()
    elseif("${_component_enable}" STREQUAL "NW")
        if(NOT "${PLATFORM}" STREQUAL "Windows")
            set(_enable_flag 1)
        else()
            set(_enable_flag 0)
        endif()
    elseif("${_component_enable}" STREQUAL "0")
        set(_enable_flag 0)
    elseif("${_component_enable}" STREQUAL "t")
        set(_enable_flag 0)
        message(WARNING "${CMAKE_CURRENT_LIST_FILE}: warning: Obsolete component enable flag, now same as 0: ${component_name}: \"${_component_enable}\"")
    else()
        set(_enable_flag 0)
        message(SEND_ERROR "${CMAKE_CURRENT_LIST_FILE}: error: Invalid component enable flag: ${component_name}: \"${_component_enable}\"")
    endif()
    if("${_enable_flag}")
        message("${CMAKE_CURRENT_LIST_FILE}: component enabled (${_component_enable}): ${component_name}")
    else()
        message("${CMAKE_CURRENT_LIST_FILE}: component disabled: ${component_name}")
    endif()
    set(${enable_flag_name} ${_enable_flag} PARENT_SCOPE)
endfunction(bs_check_component_enable)


# macro to setup output name to OUTPUT_NAME
# and turn on position independent code. This
# was pulled in to cover the separate v2c hooks
# that each component was using to a standard macro
# for Linux builds only.  Can be extended to
# cover other OS targets without the need to update
# individual component lists.
macro(bs_set_post_target)
    if (${PLATFORM} STREQUAL linux)
        set_property(TARGET ${LIB_NAME} PROPERTY OUTPUT_NAME ${OUTPUT_NAME})
        set_property(TARGET ${LIB_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif(${PLATFORM} STREQUAL linux)
endmacro()

# macro to setup standard defines This
# was pulled in to cover the separate v2c hooks
# that each component was using to a standard macro
# Can be extended to cover more OS targets without the need to update
# individual component lists.
macro(bs_set_defines)
    if (${PLATFORM} STREQUAL linux)
        add_definitions(-D__STDC_LIMIT_MACROS)
        add_definitions(-D__STDC_CONSTANT_MACROS)
    endif (${PLATFORM} STREQUAL linux)
endmacro()

# macro to setup forced exceptions for Linux
# builds only.  Should be extended for other
# targets that require forced exceptions.
macro(bs_set_force_exceptions)
    if (${PLATFORM} STREQUAL "linux")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_C_FLAGS_RELEASEINTERNAL "${CMAKE_C_FLAGS_RELEASEINTERNAL}")

        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "no-exceptions" "exceptions" CMAKE_CXX_FLAGS_RELEASEINTERNAL "${CMAKE_CXX_FLAGS_RELEASEINTERNAL}")

        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_C_FLAGS_RELEASEINERNAL "${CMAKE_C_FLAGS_RELEASEINERNAL}")

        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        string(REPLACE "-DNO_EXCEPTION_HANDLING" "" CMAKE_CXX_FLAGS_RELEASEINTERNAL "${CMAKE_CXX_FLAGS_RELEASEINTERNAL}")
    endif()
endmacro()

# function to force an error if a variable if it is not
# defined
function(bs_fatal_if_undefined)
    string(REPLACE " " ";" _var_names "${ARGV}")
    set(_undef_seen FALSE)
    foreach(_varnam ${_var_names})
        if(NOT DEFINED ${_varnam})
            message(SEND_ERROR "Required variable not defined: ${_varnam}")
            set(_undef_seen TRUE)
        endif()
    endforeach()
    if(_undef_seen)
        message(FATAL_ERROR "Stopping due to undefined variables")
    endif(_undef_seen)
endfunction(bs_fatal_if_undefined)

# macro to return the llvm directory path
# for host and target builds.
macro(bs_get_llvm_dir)
    set(LLVM_DIR ${DUMP_DIR}/igc/llvm/${LLVM_INT_DIR})
    set(CODEGEN_DIR ${DUMP_DIR}/codegen)
endmacro()

# macro to add common includes used by multiple components.
macro(bs_add_some_common_includes)
    bs_fatal_if_undefined(PLATFORM)
    if (${PLATFORM} STREQUAL linux)
        bs_get_llvm_dir()
        bs_fatal_if_undefined(CODEGEN_DIR LLVM_DIR GFX_DEVELOPMENT_DIR)
        include_directories(${CODEGEN_DIR})
        include_directories(${LLVM_DIR}/include)
        include_directories(${GFX_DEVELOPMENT_DIR}/Source/OpenGL/source/os/linux/oskl)

        if(NOT "${LIBDRM_SRC}" STREQUAL "")
            message("using LIBDRM_SRC=${LIBDRM_SRC}")
            include_directories(${LIBDRM_SRC})
            include_directories(${LIBDRM_SRC}/include/drm)
            include_directories(${LIBDRM_SRC}/intel)
        else()
            include_directories(${BS_DIR_OPENGL}/source/os/linux/oskl/drm_intel)
            include_directories(${BS_DIR_INSTRUMENTATION}/driver/linux/drm_intel)
        endif()
        set(DRM_LIB_PATH drm)
        set(DRM_INTEL_LIB_PATH drm_intel)
    endif()
endmacro()

# macro to allow setting a list of extra compile
# definitions.
macro(bs_set_extra_target_properties targ propValues)
    string(REPLACE " " ";" PROP_VALUES "${ARGV}")
    if(TARGET "${targ}")
        foreach(prop ${PROP_VALUES})
            if (${prop} STREQUAL ${targ})
                continue()
            endif()
            set_property(TARGET "${targ}" APPEND PROPERTY COMPILE_DEFINITIONS
                ${prop}
            )
        endforeach()
    endif(TARGET "${targ}")
endmacro()


#
# Macro to help find libraries when they are build as external projects.
#
macro(bs_external_add_library names paths target_name shared)
    find_library(_LIB NAMES ${names} PATHS ${paths})

    if(_LIB AND NOT TARGET ${target_name})
        message("importing external library ${target_name}")
        if(${shared} MATCHES y)
            add_library(${target_name} SHARED IMPORTED GLOBAL)
        else()
            add_library(${target_name} STATIC IMPORTED GLOBAL)
        endif()
        set_target_properties(${target_name} PROPERTIES IMPORTED_LOCATION "${_LIB}")
    endif()
endmacro()






endif(NOT DEFINED _bs_include_utils)
