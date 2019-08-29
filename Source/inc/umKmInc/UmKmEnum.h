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

#ifndef _UMKMENUM_H_
#define _UMKMENUM_H_

#ifdef __cplusplus
extern "C" {
#endif

// PROCESSOR_FAMILY
typedef enum PROCESSOR_FAMILY_REC
{
    UNKNOWN_PROCESSOR,
    P3_PROCESSOR,
    P4_PROCESSOR,
    MAX_NUM_PROCESSOR
} PROCESSOR_FAMILY;

// GPU Engine
typedef enum GPUENGINE_REC
{
    GPUENGINE_1 = 0,
    GPUENGINE_2 = 1,        // Dual GT Case
    GPUENGINE_MAX
} GPUENGINE_ORDINAL;

// GPUNODE
// NOTE: the GPUNODE_*** enums that represent real nodes and have rings must
// stay in sync with the corresponding ones defined in IGFX_GEN6_RING_TYPE
typedef enum GPUNODE_REC
{
    GPUNODE_3D = 0,         // available by default on all platform
    GPUNODE_VIDEO = 1,      // available on CTG+, Virtual node
    GPUNODE_BLT = 2,        // available on GT
    GPUNODE_VE = 3,         // available on HSW+ (VideoEnhancement), virtual node
    GPUNODE_VCS2 = 4,       // available on BDW/SKL/KBL GT3+ and CNL,
    GPUNODE_CCS0 = 5,   //
    GPUNODE_REAL_MAX,       // all nodes beyond this are virtual nodes - they don't have an actual GPU engine
    GPUNODE_PICS = 6,       // available on CNL+. Real node but only for KMD internal use. Hence kept after GPUNODE_REAL_MAX (Note: We need to keep it before overlay node)
    GPUNODE_OVERLAY = 7,
    GPUNODE_GDI2D = 8,      // GT virtual node for GDI/Present 2D blt/colorfill
    GPUNODE_VXD   = 9,      // available on BYT
    GPUNODE_MAX
} GPUNODE_ORDINAL;

// UMD_DMA_TYPE
typedef enum UMD_DMA_TYPE_REC
{
    UMD_UNKNOWN    = 0,
    UMD_MEDIA      = 1,
    UMD_DX9        = 2,
    UMD_DX10       = 3,
    UMD_OGL        = 4,
    UMD_OCL        = 5,
    UMD_DX_COMPUTE = 6,
    UMD_DX12       = 7,

    UMD_TYPE_MAX    // this should always be the last one
} UMD_DMA_TYPE;


typedef enum DXVA_OPERATION_ENUM
{
    DXVA_OPERATION_NONE  = 0,
    DXVA_OPERATION_DECODE,       // Decode
    DXVA_OPERATION_ENCODE,       // Encode
} DXVA_OPERATION;

extern char *UmdTypeStr[];

#ifdef __cplusplus
}
#endif

#endif // _UMKMENUM_H_
