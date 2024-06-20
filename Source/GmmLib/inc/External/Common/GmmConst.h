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


//------------------------------------------------------------------------
// Texture Values
//------------------------------------------------------------------------
#define __GMM_MIN_TEXTURE_WIDTH                         0x1
#define __GMM_EVEN_ROW                                  0x2
#define __GMM_LINEAR_RENDER_ALIGNMENT                   0x4     // need DWORD aligned
#define __GMM_8BPP_LINEAR_RENDER_ALIGNMENT              0x8     // need QWORD aligned (HW workaround)
#define GMM_IMCx_PLANE_ROW_ALIGNMENT                    16

//------------------------------------------------------------------------
// Misc
//------------------------------------------------------------------------
#define GMM_MDL_PAGE_SHIFT                              12      // Size of bit-shift to convert btween MDL page NUMBER and page ADDRESS.
#define GMM_FIELD_NA                                   0xFFFFFFFF
#define GMM_INTERNAL_RESOURCE                           0       // Used for Alloc Tag Mapping
#define GMM_MAX_NUMBER_MOCS_INDEXES                    (64)
#define GMM_XE_NUM_MOCS_ENTRIES                        (16)
#define GMM_GEN9_MAX_NUMBER_MOCS_INDEXES               (62)     // On SKL there are 64 MOCS indexes, but the last two are reserved by h/w.
#define GMM_XE2_NUM_MOCS_ENTRIES                       (16)
#define GMM_NUM_PAT_ENTRIES_LEGACY                     (8)
#define GMM_NUM_PAT_ENTRIES                            (32)
#define GMM_NUM_MEMORY_TYPES                            4
#define GMM_NUM_GFX_PAT_TYPES                           6
#define GMM_TILED_RESOURCE_NO_MIP_TAIL                 0xF
#define GMM_TILED_RESOURCE_NO_PACKED_MIPS              0xF
#define GMM_GEN10_HDCL1_MOCS_INDEX_START               (48)     // CNL+ MOCS index 48-61 allows HDC L1 caching, last 2 are reserved by h/w.
#define GMM_MSAA_SAMPLES_MIN                            1       //Define min and max MSAA samples
#define GMM_MSAA_SAMPLES_MAX                            16
#define GMM_HIZ_CLEAR_COLOR_SIZE                       (8)
#define GMM_MEDIA_COMPRESSION_STATE_SIZE               (64)
#define GMM_CLEAR_COLOR_FLOAT_SIZE                     (16)
#define GMM_MAX_LCU_SIZE                                64  // Media Largest coding Unit
