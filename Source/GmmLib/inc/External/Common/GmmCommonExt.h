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

#define INCLUDE_CpuSwizzleBlt_c_AS_HEADER
#include "../../../Utility/CpuSwizzleBlt/CpuSwizzleBlt.c"
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_8bpp;
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_16_32bpp;
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_64_128bpp;

// Set packing alignment
#pragma pack(push, 8)

#define GMM_STDCALL             __stdcall   // GMM function calling convention
#define GMM_NO_FENCE_REG        0xDEADBEEF
#define GMM_MAX_DISPLAYS        3

//===========================================================================
// typedef:
//      GMM_GFX_ADDRESS/etc.
//
// Description:
//     The GMM GfxAddr/Size and related types and casting functions.
//----------------------------------------------------------------------------
// Always use x64 (D3D11.2 Tiled Resources needs >32 bit gfx address space)
typedef UINT64  GMM_GFX_ADDRESS, GMM_GFX_SIZE_T, GMM_VOIDPTR64;
#define GMM_GFX_ADDRESS_MAX ((GMM_GFX_ADDRESS) 0xffffffffffffffff)

typedef UINT32 GMM_GLOBAL_GFX_ADDRESS, GMM_GLOBAL_GFX_SIZE_T;
#define GMM_GLOBAL_GFX_ADDRESS_MAX ((GMM_GLOBAL_GFX_ADDRESS) 0xffffffff)

#ifdef _DEBUG
    #define GMM_GFX_ADDRESS_CAST(x)     \
    (                                   \
        ((x) <= GMM_GFX_ADDRESS_MAX) ?  \
            1 : __debugbreak(),         \
        (GMM_GFX_ADDRESS)(x)            \
    )
    #define GMM_GFX_SIZE_T_CAST GMM_GFX_ADDRESS_CAST

    #define GMM_GLOBAL_GFX_ADDRESS_CAST(x)      \
    (                                           \
        ((x) <= GMM_GLOBAL_GFX_ADDRESS_MAX) ?   \
            1 : __debugbreak(),                 \
        (GMM_GLOBAL_GFX_ADDRESS)(x)             \
    )
    #define GMM_GLOBAL_GFX_SIZE_T_CAST GMM_GLOBAL_GFX_ADDRESS_CAST
#else
    #define GMM_GFX_ADDRESS_CAST(x)         ((GMM_GFX_ADDRESS)(x))
    #define GMM_GFX_SIZE_T_CAST(x)          ((GMM_GFX_SIZE_T)(x))
    #define GMM_GLOBAL_GFX_ADDRESS_CAST(x)  ((GMM_GLOBAL_GFX_ADDRESS)(x))
    #define GMM_GLOBAL_GFX_SIZE_T_CAST(x)   ((GMM_GLOBAL_GFX_SIZE_T)(x))
#endif

#define GMM_GMADR_OFFSET_T          GMM_GLOBAL_GFX_ADDRESS
#define GMM_GMADR_SIZE_T            GMM_GLOBAL_GFX_SIZE_T
#define GMM_GMADR_OFFSET_T_CAST     GMM_GLOBAL_GFX_ADDRESS_CAST
#define GMM_GMADR_SIZE_T_CAST       GMM_GLOBAL_GFX_SIZE_T_CAST

typedef UINT32  TODO_GFX32; // TODO(Medium): Fix and remove!!

#define GMM_GFX_ADDRESS_CANONIZE(a)     (((INT64)(a) << (64 - 48)) >> (64 - 48)) // TODO(Minor): When GMM adds platform-dependent VA size caps, change from 48.
#define GMM_GFX_ADDRESS_DECANONIZE(a)   ((UINT64)(a) & (((UINT64) 1 << 48) - 1)) // "

#define GMM_BIT_RANGE(endbit, startbit)     ((endbit)-(startbit)+1)
#define GMM_BIT(bit)                        (1)

// TODO(Benign): Where should this live?
#define GMM_IA32e_BITS_PER_TABLE_INDEX  9
#define GMM_IA32e_ENTRIES_PER_TABLE     (1 << GMM_IA32e_BITS_PER_TABLE_INDEX)
#define GMM_IA32e_PTE_SIZE              8
#define GMM_IA32e_TABLE_SIZE            4096
#define GMM_IA32e_PTE_PRESENT           __BIT(0)
#define GMM_IA32e_PTE_WRITABLE          __BIT(1)
#define GMM_IA32e_PTE_USER_ACCESSIBLE   __BIT(2)
#define GMM_IA32e_PTE_PWT               __BIT(3)
#define GMM_IA32e_PTE_PCD               __BIT(4)
#define GMM_IA32e_PTE_ACCESSED          __BIT(5)
#define GMM_IA32e_PTE_PAT               __BIT(7)
#define GMM_IA32e_PTE_U_W_P             (GMM_IA32e_PTE_USER_ACCESSIBLE | GMM_IA32e_PTE_WRITABLE | GMM_IA32e_PTE_PRESENT)
#define GMM_IA32e_PTE_U_R_P             (GMM_IA32e_PTE_USER_ACCESSIBLE | GMM_IA32e_PTE_PRESENT)
#define GMM_PAT_IDX_PTE_BITS(idx)       ((((idx) & __BIT(2)) ? GMM_IA32e_PTE_PAT : 0) |\
                                         (((idx) & __BIT(1)) ? GMM_IA32e_PTE_PCD : 0) |\
                                         (((idx) & __BIT(0)) ? GMM_IA32e_PTE_PWT : 0) )
#define GMM_SKL_PTE_NULL_PAGE           __BIT(9)
#define GMM_IA32e_PDE_IPS               __BIT(11)

#define GMM_GFX_PHYS_ADDRESS_MAX        (0xffffffffffffffff)
#define GMM_WDDM_MAX_ALLOCATION_SIZE    (0xffffffff) // WDDM allocation sizes are limited by the MDL byte count, which is a uint32_t
//===========================================================================
// typedef:
//      GMM_STATUS_ENUM
//
// Description:
//     Decribes GMM lib function return values
//----------------------------------------------------------------------------
typedef enum GMM_STATUS_ENUM
{
    GMM_SUCCESS,
    GMM_ERROR,
    GMM_INVALIDPARAM,
    GMM_OUT_OF_MEMORY,
} GMM_STATUS;

//===========================================================================
// typedef:
//      GMM_CLIENT
//
// Description:
//     Decribes who is linking GMM lib. Allow Kernel mode driver to detect
//     origin of resource allocations.
//----------------------------------------------------------------------------
typedef enum GMM_CLIENT_ENUM
{
    GMM_KMD_VISTA = 0,
    GMM_OGL_VISTA = 1,
    GMM_D3D9_VISTA = 2,
    GMM_D3D10_VISTA = 3,
    GMM_XP = 4
}GMM_CLIENT;

//===========================================================================
// typedef:
//     GMM_TEXTURE_LAYOUT
//
// Description:
//     This enum details HW tile walk (i.e. X or Y walk)
//---------------------------------------------------------------------------
typedef enum GMM_TEXTURE_LAYOUT_REC
{
    GMM_2D_LAYOUT_BELOW = 0, // <- This should stay zero to be the default for all surfaces (including 3D, etc), since texture layout can be queried for 3D/etc surfaces. TODO: Is that querying proper behavior?!
    GMM_2D_LAYOUT_RIGHT = 1,
}GMM_TEXTURE_LAYOUT;

//===========================================================================
// typedef:
//     GMM_TILE_TYPE_ENUM
//
// Description:
//     This enum details tile walk (i.e. X or Y walk, or not tiled)
//---------------------------------------------------------------------------
typedef enum GMM_TILE_TYPE_ENUM
{
    GMM_TILED_X,
    GMM_TILED_Y,
    GMM_TILED_W,
    GMM_NOT_TILED
}GMM_TILE_TYPE;

//===========================================================================
// typedef:
//     GMM_CPU_CACHE_TYPE_ENUM
//
// Description:
//     This enum details the type of cacheable attributes for the memory
//---------------------------------------------------------------------------
typedef enum GMM_CPU_CACHE_TYPE_ENUM
{
    GMM_NOTCACHEABLE,
    GMM_CACHEABLE
}GMM_CPU_CACHE_TYPE;

//===========================================================================
// typedef:
//     GMM_GPU_CACHE_SETTINGS
//
// Description:
//     This struct details the type of cacheable attributes for the memory
//---------------------------------------------------------------------------
typedef struct GMM_GPU_CACHE_SETTINGS_REC
{
    uint32_t ELLC           : 1; // Gen7.5+
    uint32_t Age            : 2; // Gen7.5+
    uint32_t WriteThrough   : 1; // Gen7.5+
    uint32_t GFDT           : 1; // Gen6,7 (Graphics Flush Data Type)
    uint32_t L3             : 1; // Gen7+
    uint32_t LLC            : 1; // Gen6+
    uint32_t UsePteSettings : 1; // Gen6+
}GMM_GPU_CACHE_SETTINGS;

//===========================================================================
// typedef:
//     GMM_CACHE_SIZE
//
// Description:
//     This struct details the size of the L3 and EDram caches
//---------------------------------------------------------------------------
typedef struct GMM_CACHE_SIZES_REC
{
    GMM_GFX_SIZE_T TotalL3Cache;  // L3 Cache Size in Bytes
    GMM_GFX_SIZE_T TotalLLCCache; // LLC Cache Size in Bytes
    GMM_GFX_SIZE_T TotalEDRAM;    // eDRAM Size in Bytes
} GMM_CACHE_SIZES;

//------------------------------------------------------------------------
// GMM Legacy Flags
//------------------------------------------------------------------------

// GMM_TILE_W & GMM_TILE_X & GMM_TILE_Y are for internal use. Clients should use GMM_TILE_TYPE_ENUM
#define GMM_TILE_W               (__BIT(1))     // Tile W type
#define GMM_TILE_X               (__BIT(2))     // Tile X type
#define GMM_TILE_Y               (__BIT(3))     // Tile Y type

#if !defined(GHAL3D_ULT)
#define GMM_LINEAR               (__BIT(4))     // Linear type
#define GMM_TILED                (GMM_TILE_W | GMM_TILE_X | GMM_TILE_Y) // Tiled bit
#endif
#define GMM_HEAP_EXTERNAL_SYNC   (__BIT(7))     // Indicates GMM_HEAP should not use its internal sync protection.
#define GMM_UTILIZEFENCE         (__BIT(8))
#define GMM_NOT_OS_ADDR          0xA1B3C5D7

//===========================================================================
// typedef:
//     GMM_TILE_INFO
//
// Description:
//     This structure represents a tile type
//--------------------------------------------------------------------------
typedef struct GMM_TILE_INFO_REC
{
    uint32_t   LogicalTileWidth;       // width of tile in bytes
    uint32_t   LogicalTileHeight;      // Height of tile in rows [texels]
    uint32_t   LogicalTileDepth;       // Depth of tile
    uint32_t   LogicalSize;            // Size of the tile in bytes
    uint32_t   MaxPitch;               // Maximum pitch in bytes
    uint32_t   MaxMipTailStartWidth;   // Max mip tail start width in pixels
    uint32_t   MaxMipTailStartHeight;  // Max mip tail start height in rows
    uint32_t   MaxMipTailStartDepth;   // Max mip tail start depth in slices
} GMM_TILE_INFO;

//===========================================================================
// typedef:
//        GMM_CUBE_FACE_REC
//
// Description:
//     ENUM detailing cube map faces
//---------------------------------------------------------------------------
typedef enum GMM_CUBE_FACE_ENUM
{
   __GMM_CUBE_FACE_POS_X,
   __GMM_CUBE_FACE_NEG_X,
   __GMM_CUBE_FACE_POS_Y,
   __GMM_CUBE_FACE_NEG_Y,
   __GMM_CUBE_FACE_POS_Z,
   __GMM_CUBE_FACE_NEG_Z,
   __GMM_MAX_CUBE_FACE,
   __GMM_NO_CUBE_MAP,
} GMM_CUBE_FACE_ENUM;

//===========================================================================
// typedef:
//        GMM_YUV_PLANE_REC
//
// Description:
//
//--------------------------------------------------------------------------
typedef enum GMM_YUV_PLANE_ENUM
{
    GMM_NO_PLANE     = 0,
    GMM_PLANE_Y      = 1,
    GMM_PLANE_U      = 2,
    GMM_PLANE_V      = 3,
    GMM_MAX_PLANE    = 4,
    // GmmLib internal use only
    GMM_PLANE_3D_Y0  = 0,
    GMM_PLANE_3D_Y1  = 1,
    GMM_PLANE_3D_UV0 = 2,
    GMM_PLANE_3D_UV1 = 3
    // No internal use >= GMM_MAX_PLANE!
}GMM_YUV_PLANE;

//===========================================================================
// typedef:
//        GMM_RESOURCE_TYPE
//
// Description:
//     This enum describe type of resource requested
//---------------------------------------------------------------------------
typedef enum GMM_RESOURCE_TYPE_ENUM
{
    RESOURCE_INVALID,

    // User-mode use
    RESOURCE_1D,
    RESOURCE_2D,
    RESOURCE_3D,
    RESOURCE_CUBE,
    RESOURCE_SCRATCH,
    RESOURCE_BUFFER,

    //Kernel-mode use only
    RESOURCE_KMD_CHECK_START,
    RESOURCE_PRIMARY,
    RESOURCE_SHADOW,
    RESOURCE_STAGING,
    RESOURCE_CURSOR,
    RESOURCE_FBC,
    RESOURCE_PWR_CONTEXT,
    RESOURCE_PERF_DATA_QUEUE,
    RESOURCE_KMD_BUFFER,
    RESOURCE_HW_CONTEXT,
    RESOURCE_TAG_PAGE,
    RESOURCE_PRIVATE_SUBMIT_BUFFER,
    RESOURCE_NNDI,
    RESOURCE_HARDWARE_MBM,
    RESOURCE_OVERLAY_DMA,
    RESOURCE_OVERLAY_INTERMEDIATE_SURFACE,
    RESOURCE_GTT_TRANSFER_REGION,
    RESOURCE_GLOBAL_BUFFER,
    RESOURCE_GDI,
    RESOURCE_NULL_CONTEXT_INDIRECT_STATE,
    RESOURCE_GFX_CLIENT_BUFFER,
    RESOURCE_KMD_CHECK_END,
    RESOURCE_SEGMENT,
    RESOURCE_IFFS_MAPTOGTT,
#if _WIN32
    RESOURCE_WGBOX_ENCODE_STATE,
    RESOURCE_WGBOX_ENCODE_DISPLAY,
    RESOURCE_WGBOX_ENCODE_REFERENCE,
    RESOURCE_WGBOX_ENCODE_TFD,
#endif
    // only used for allocation of context specific SVM Present kernels buffer
    RESOURCE_SVM_KERNELS_BUFFER,

    GMM_MAX_HW_RESOURCE_TYPE
} GMM_RESOURCE_TYPE;

//===========================================================================
// typedef:
//      GMM_RESOURCE_FORMAT
//
// Description:
//      Enumeration of GMM supported resource formats.
//---------------------------------------------------------------------------
typedef enum GMM_RESOURCE_FORMAT_ENUM
{
    GMM_FORMAT_INVALID = 0, // <-- This stays zero! (For boolean and valid range checks.)

    #define GMM_FORMAT(Name, bpe, Width, Height, Depth, IsRT, IsASTC, RcsSurfaceFormat, AuxL1eFormat, Availability) \
        GMM_FORMAT_##Name, \
        GMM_FORMAT_##Name##_TYPE = GMM_FORMAT_##Name, // TODO(Minor): Remove _TYPE suffix from every GMM_FORMAT_ reference throughout driver and delete this aliasing. (And remove the \ from the line above.)
    #include "GmmFormatTable.h"

    GMM_RESOURCE_FORMATS
} GMM_RESOURCE_FORMAT;
C_ASSERT(GMM_FORMAT_INVALID == 0); // GMM_FORMAT_INVALID needs to stay zero--How did it end-up not...?
// Legacy GMM Aliases...
#define GMM_FORMAT_UYVY     GMM_FORMAT_YCRCB_SWAPY
#define GMM_FORMAT_VYUY     GMM_FORMAT_YCRCB_SWAPUVY
#define GMM_FORMAT_YUY2     GMM_FORMAT_YCRCB_NORMAL
#define GMM_FORMAT_YVYU     GMM_FORMAT_YCRCB_SWAPUV


//===========================================================================
// typedef:
//      GMM_SURFACESTATE_FORMAT
//
// Description:
//      Enumeration of RCS SURFACE_STATE.Formats.
//---------------------------------------------------------------------------
typedef enum GMM_SURFACESTATE_FORMAT_ENUM
{
    GMM_SURFACESTATE_FORMAT_INVALID = -1, // Can't use zero since that's an actual enum value.

    #define GMM_FORMAT_INCLUDE_SURFACESTATE_FORMATS_ONLY
    #define GMM_FORMAT(Name, bpe, Width, Height, Depth, IsRT, IsASTC, RcsSurfaceFormat, AuxL1eFormat, Availability) \
        GMM_SURFACESTATE_FORMAT_##Name = RcsSurfaceFormat,
    #include "GmmFormatTable.h"
} GMM_SURFACESTATE_FORMAT;

//===========================================================================
// typedef:
//     GMM_TILE_WALK
//
// Description:
//     This enum details HW tile walk (i.e. X or Y walk), used to program the
//     hardware.
//---------------------------------------------------------------------------
typedef enum GMM_TILE_WALK_REC
{
    GMM_HW_TILED_X_WALK     = 0,
    GMM_HW_TILED_Y_WALK     = 1,
    GMM_HW_TILED_W_WALK     = 2,
    GMM_HW_TILED_YF_WALK    = 3,
    GMM_HW_TILED_YS_WALK    = 4,
    GMM_HW_NOT_TILED        = 5
} GMM_TILE_WALK;

//===========================================================================
// typedef:
//      GMM_DISPLAY_FRAME_ENUM
//
// Description:
//      This enum details the type of display frame (for S3D resources).
//---------------------------------------------------------------------------
typedef enum GMM_DISPLAY_FRAME_ENUM
{
    GMM_DISPLAY_BASE        = 0,   // Legacy non-S3D display resource
    GMM_DISPLAY_L           = GMM_DISPLAY_BASE,
    GMM_DISPLAY_R           = 1,
    GMM_DISPLAY_BLANK_AREA  = 2,
    GMM_DISPLAY_FRAME_MAX
} GMM_DISPLAY_FRAME;

//===========================================================================
// typedef:
//        GMM_REQ_OFFSET_INFO
//
// Description:
//     This structure is used to request offset information to a surface mip
//     level or video plane.
//---------------------------------------------------------------------------
typedef struct GMM_REQ_OFFSET_INFO_REC
{
    uint32_t               ReqRender       :1;
    uint32_t               ReqLock         :1;
    uint32_t               ReqStdLayout    :1;
    uint32_t               Reserved        :29;

    uint32_t               ArrayIndex;
    uint32_t               MipLevel;
    uint32_t               Slice;
    GMM_CUBE_FACE_ENUM  CubeFace;
    GMM_YUV_PLANE       Plane;

    GMM_DISPLAY_FRAME   Frame;

    struct
    {
        union
        {
            uint32_t               Offset;
            GMM_GFX_SIZE_T      Offset64;
        };
        uint32_t               Pitch;
        union
        {
            uint32_t               Mip0SlicePitch;
            uint32_t               Gen9PlusSlicePitch;
        };
    }                   Lock;

    struct
    {
        union
        {
            uint32_t               Offset;
            GMM_GFX_SIZE_T      Offset64;
        };
        uint32_t               XOffset;
        uint32_t               YOffset;
        uint32_t               ZOffset;
    }                   Render;

    struct 
    {
        GMM_GFX_SIZE_T      Offset;
        GMM_GFX_SIZE_T      TileRowPitch;
        GMM_GFX_SIZE_T      TileDepthPitch;
    }                   StdLayout;

}GMM_REQ_OFFSET_INFO;

//===========================================================================
// typedef:
//     GMM_HW_COMMAND_STREAMER
//
// Description: Enumeration to allow callers to specify a command streamer.
//
//---------------------------------------------------------------------------
typedef enum GMM_HW_COMMAND_STREAMER_ENUM
{
    GMM_HW_COMMAND_STREAMER_NULL = 0, // <-- This stays zero.
    GMM_CS,      // Render Command Streamer
    GMM_BCS,     // Blitter (BLT) Command Streamer
    GMM_VCS,     // Video Codec (MFX) Command Streamer
    GMM_VECS,    // Video Enhancement (VEBOX) Command Streamer
    GMM_VCS2,    // Video Codec (MFX) Command Streamer 2
    GMM_COMPUTE, // Compute Only Command Streamer
    GMM_PICS,    // Pinning Command Streamer 
    GMM_CAPTURE, // Capture Command Streamer
    GMM_HW_COMMAND_STREAMERS // <-- This stays last.
} GMM_HW_COMMAND_STREAMER;

//===========================================================================
// typedef:
//     GMM_HW_COMMAND
//
// Description: Enumeration to allow callers to specify a HW command.
//              (Currently only used with GmmGetUseGlobalGtt--with the
//              commands that have a PPGTT/GTT space select field.)
//
//---------------------------------------------------------------------------

typedef enum GMM_HW_COMMAND_ENUM
{
    GMM_HW_COMMAND_NULL = 0, // <-- This stays zero.
    GMM_MI_ATOMIC,
    GMM_MI_BATCH_BUFFER_START,
    GMM_MI_CLFLUSH,
    GMM_MI_CONDITIONAL_BATCH_BUFFER_END,
    GMM_MI_COPY_MEM_MEM,
    GMM_MI_FLUSH_DW,
    GMM_MI_LOAD_REGISTER_MEM,
    GMM_MI_REPORT_PERF_COUNT,
    GMM_MI_SEMAPHORE_MBOX,
    GMM_MI_SEMAPHORE_SIGNAL,
    GMM_MI_SEMAPHORE_WAIT,
    GMM_MI_STORE_DATA_IMM,
    GMM_MI_STORE_DATA_INDEX,
    GMM_MI_STORE_REGISTER_MEM,
    GMM_MI_UPDATE_GTT,
    GMM_PIPE_CONTROL,
    GMM_HW_COMMANDS // <-- This stays last.
} GMM_HW_COMMAND;


// Reset packing alignment to project default
#pragma pack(pop)

