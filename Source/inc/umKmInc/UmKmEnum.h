/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2010-2013).
**
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS 
** LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT, 
** ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT 
** PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY 
** DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
** PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability, 
** including liability for infringement of any proprietary rights, relating to
** use of the code. No license, express or implied, by estoppel or otherwise, 
** to any intellectual property rights is granted herein.
**
**
** File Name:    UmKmEnum.h
**
** Description:  These are enumerations that are shared between the KMD 
**               and the UMD
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

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
    GPUNODE_RESERVED = 5,   // 
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
