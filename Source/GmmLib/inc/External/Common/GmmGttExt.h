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

#ifdef _WIN32

// Set packing alignment
#pragma pack(push, 8)


//===========================================================================
// typedef:
//        GMM_GTT_CACHETYPE
//
// Description:
//        GTT page table entry caching type.
//--------------------------------------------------------------------------
typedef enum GMM_GTT_CACHETYPE_ENUM
{
    GMM_GTT_CACHETYPE_INVALID           = 0,
    GMM_GTT_CACHETYPE_UNCACHED          = 1,    // Gen3.5+
    GMM_GTT_CACHETYPE_CACHED            = 2,    // Gen3.5+
    GMM_GTT_CACHETYPE_LLC               = 3,    // Gen6+
    GMM_GTT_CACHETYPE_VLV_SNOOPED       = 4,    //Gen 7, VLV Snooped setting.
    GMM_GTT_CACHETYPE_GEN7_5_BASE       = 4,    // Gen7.5+ - INTERNAL USE ONLY
    GMM_GTT_CACHETYPE_GEN7_5_INVALID    = 5,    // Gen7.5+ - PLACEHOLDER FOR INVALID BITS
    GMM_GTT_CACHETYPE_WB_LLC_AGE_3      = 6,    // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_AGE_0      = 7,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_ELLC_AGE_0     = 8,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_ELLC_AGE_3     = 9,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_LLC_ELLC_AGE_0 = 10,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_LLC_ELLC_AGE_3 = 11,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_3 = 12,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_2 = 13,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_1 = 14,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_0 = 15,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_3     = 16,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_2     = 17,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_1     = 18,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_0     = 19,   // Gen7.5+
    GMM_GTT_CACHETYPE_MAX
}GMM_GTT_CACHETYPE;

#endif