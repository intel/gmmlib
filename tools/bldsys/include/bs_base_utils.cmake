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


if(NOT DEFINED _bs_include_base_utils)
  set(_bs_include_base_utils TRUE)

  # bs_set_if_undefined - If not defined, assign value from env or arg
  macro(bs_set_if_undefined VAR_NAME VAR_VALUE)
    if(DEFINED ${VAR_NAME} AND NOT ${VAR_NAME} STREQUAL "")
      # Already defined
    elseif(DEFINED ENV{VAR_NAME} AND NOT "$ENV{VAR_NAME}" STREQUAL "")
      set(${VAR_NAME} "$ENV{VAR_NAME}")
    else()
      set(${VAR_NAME} "${VAR_VALUE}")
    endif()
  endmacro()

  set(_bs_check_build_type_done 0)

  # function bs_check_build_type
  # Deal with the ambiguity of the three different variables for BUILD_TYPE:
  # Give them priority in this order:
  #   UFO_BUILD_TYPE
  #   BUILD_TYPE
  # Note: Despite the similarity of name, CMAKE_BUILD_TYPE is not analogous
  # to UFO_BUILD_TYPE and BUILD_TYPE.
  function(bs_check_build_type)
    if(NOT ${_bs_check_build_type_done})
      set(_bs_check_build_type_done 1 PARENT_SCOPE)
      set(_bs_bt_var_names UFO_BUILD_TYPE BUILD_TYPE)
      foreach(_bs_bt_var_a ${_bs_bt_var_names})
        foreach(_bs_bt_var_b ${_bs_bt_var_names})
          if(NOT _bs_bt_var_a STREQUAL _bs_bt_var_b)
            if(DEFINED ${_bs_bt_var_a} AND DEFINED ${_bs_bt_var_b} AND NOT ${_bs_bt_var_a} STREQUAL ${_bs_bt_var_b})
              message(FATAL_ERROR "Conflict: ${_bs_bt_var_a}=${${_bs_bt_var_a}} vs ${_bs_bt_var_b=${${_bs_bt_var_b}}}")
            endif()
          endif()
        endforeach()
      endforeach()
      set(_bs_bt_value "")
      foreach(_bs_bt_var_a ${_bs_bt_var_names})
        if(DEFINED ${_bs_bt_var_a})
          set(_bs_bt_value ${${_bs_bt_var_a}})
          break()
        endif()
      endforeach()

      if(_bs_bt_value STREQUAL "")
        message("*BUILD_TYPE not defined, default to: release")
        set(_bs_bt_value "release")
      endif()

      foreach(_bs_bt_var_a ${_bs_bt_var_names})
        if(NOT DEFINED ${_bs_bt_var_a})
          set(${_bs_bt_var_a} "${_bs_bt_value}" PARENT_SCOPE)
        endif()
      endforeach()
    endif()
  endfunction(bs_check_build_type)


endif(NOT DEFINED _bs_include_base_utils)
