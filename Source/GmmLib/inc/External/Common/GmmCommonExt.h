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

//===========================================================================
// Macros to be defined for GMM Lib DLL
//===========================================================================
#ifdef GMM_LIB_DLL                                      /* To be defined by Clients if GMMlib needs to be in DLL/so */

#define GMM_INLINE_VIRTUAL              virtual

#define GMM_VIRTUAL                     virtual

#define GMM_INLINE_EXPORTED                                            /* Macro To avoid inlining of exported member functions in Clients code in DLL mode*/

#define GMM_LIB_DLL_MA                  1               // Macro to indicate whether GMM Lib DLL is Multi-Adapter capable. Todo: Make this a build macro

#ifdef _WIN32

    #ifdef GMM_LIB_DLL_EXPORTS
        #define GMM_LIB_API                     __declspec(dllexport)   /* Macro to define GMM Lib DLL exports */
    #else
        #define GMM_LIB_API                     __declspec(dllimport)       /* Macro to define GMM Lib DLL imports */
    #endif  /* GMM_LIB_DLL_EXPORTS */

#define GMM_LIB_API_CONSTRUCTOR
#define GMM_LIB_API_DESTRUCTOR

#else // Linux
    #ifdef GMM_LIB_DLL_EXPORTS
        #define GMM_LIB_API                     __attribute__ ((visibility ("default")))
    #else
        #define GMM_LIB_API
    #endif

#define GMM_LIB_API_CONSTRUCTOR             __attribute__((constructor))
#define GMM_LIB_API_DESTRUCTOR              __attribute__((destructor))

#endif

#else // !GMM_LIB_DLL

#define GMM_INLINE_VIRTUAL
#define GMM_VIRTUAL
#define GMM_LIB_API
#define GMM_INLINE_EXPORTED __inline

#endif  /* GMM_LIB_DLL */


#define INCLUDE_CpuSwizzleBlt_c_AS_HEADER
#include "../../../Utility/CpuSwizzleBlt/CpuSwizzleBlt.c"
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_8bpp;
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_16_32bpp;
extern const SWIZZLE_DESCRIPTOR INTEL_64KB_UNDEFINED_64_128bpp;

// Set packing alignment
#pragma pack(push, 8)

#if defined(__ARM_ARCH)
#define GMM_STDCALL            // GMM function calling convention
#else
#define GMM_STDCALL             __stdcall   // GMM function calling convention
#endif
#define GMM_NO_FENCE_REG        0xDEADBEEF
#define GMM_MAX_DISPLAYS        3

#if defined __linux__
typedef void* HANDLE;
#endif

//===========================================================================
// typedef:
//      GMM_GFX_ADDRESS/etc.
//
// Description:
//     The GMM GfxAddr/Size and related types and casting functions.
//----------------------------------------------------------------------------
// Always use x64 (D3D11.2 Tiled Resources needs >32 bit gfx address space)
typedef uint64_t  GMM_GFX_ADDRESS, GMM_GFX_SIZE_T, GMM_VOIDPTR64;
#define GMM_GFX_ADDRESS_MAX ((GMM_GFX_ADDRESS) 0xffffffffffffffff)

typedef uint32_t GMM_GLOBAL_GFX_ADDRESS, GMM_GLOBAL_GFX_SIZE_T;
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

#define GMM_GFX_ADDRESS_CANONIZE(a)     (((int64_t)(a) << (64 - 48)) >> (64 - 48)) // TODO(Minor): When GMM adds platform-dependent VA size caps, change from 48.
#define GMM_GFX_ADDRESS_DECANONIZE(a)   ((uint64_t)(a) & (((uint64_t) 1 << 48) - 1)) // "

#define GMM_GFX_PLATFORM_VA_SIZE(pClientContext)        (((pClientContext)->GetLibContext()->GetSkuTable().Ftr57bGPUAddressing) ? 57 : 48)
#define VASize(pCC)                                     GMM_GFX_PLATFORM_VA_SIZE(pCC)

#define GMM_BIT_RANGE(endbit, startbit) ((endbit) - (startbit) + 1)
#define GMM_BIT(bit)                    (1)

#define GMM_GET_PTE_BITS_FROM_PAT_IDX(idx)         ((((idx)&__BIT(4))   ? __BIT64(61)   : 0) |  \
                                                    (((idx)&__BIT(3))   ? __BIT64(62)   : 0) |  \
                                                    (((idx)&__BIT(2))   ? __BIT64(7)    : 0) |  \
                                                    (((idx)&__BIT(1))   ? __BIT64(4)    : 0) |  \
                                                    (((idx)&__BIT(0))   ? __BIT64(3)    : 0) )

#define GMM_GET_PAT_IDX_FROM_PTE_BITS(Entry)       ((((Entry) & __BIT64(61))    ? __BIT(4) : 0) |  \
                                                    (((Entry) & __BIT64(62))    ? __BIT(3) : 0) |  \
                                                    (((Entry) & __BIT64(7))     ? __BIT(2) : 0) |  \
                                                    (((Entry) & __BIT64(4))     ? __BIT(1) : 0) |  \
                                                    (((Entry) & __BIT64(3))     ? __BIT(0) : 0) )

#define GMM_GET_PAT_IDX_FROM_PTE_BITS_GGTT(Entry)  ((((Entry) & __BIT64(53))    ? __BIT(1) : 0) | \
                                                    (((Entry) & __BIT64(52))    ? __BIT(0) : 0) )

#define GMM_GET_PTE_BITS_FROM_PAT_IDX(idx)         ((((idx)&__BIT(4))   ? __BIT64(61)   : 0) |  \
                                                    (((idx)&__BIT(3))   ? __BIT64(62)   : 0) |  \
                                                    (((idx)&__BIT(2))   ? __BIT64(7)    : 0) |  \
                                                    (((idx)&__BIT(1))   ? __BIT64(4)    : 0) |  \
                                                    (((idx)&__BIT(0))   ? __BIT64(3)    : 0) )

#define GMM_GET_PTE_BITS_FROM_PAT_IDX_LEAF_PD(idx) ((((idx)&__BIT(4))   ? __BIT64(61)   : 0) |  \
                                                    (((idx)&__BIT(3))   ? __BIT64(62)   : 0) |  \
                                                    (((idx)&__BIT(2))   ? __BIT64(12)   : 0) |  \
                                                    (((idx)&__BIT(1))   ? __BIT64(4)    : 0) |  \
                                                    (((idx)&__BIT(0))   ? __BIT64(3)    : 0) )

#define GMM_GET_PTE_BITS_FROM_PAT_IDX_GGTT(idx)    ((((idx)&__BIT(1))   ? __BIT64(53)   : 0) |  \
                                                    (((idx)&__BIT(0))   ? __BIT64(52)   : 0) )
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
    GMM_XP = 4,
    GMM_D3D12_VISTA = 5,
    GMM_OCL_VISTA = 6,
    GMM_VK_VISTA = 7,
    GMM_EXCITE_VISTA = 8,
    GMM_UNDEFINED_CLIENT = 9,
}GMM_CLIENT;

// Macros related to GMM_CLIENT Enum
#define USE_KMT_API(ClientType)         ((ClientType == GMM_OGL_VISTA) || (ClientType == GMM_OCL_VISTA) || (ClientType == GMM_VK_VISTA) || (ClientType == GMM_EXCITE_VISTA))
#define USE_DX12_API(ClientType)        (ClientType == GMM_D3D12_VISTA)
#define USE_DX10_API(ClientType)        (ClientType == GMM_D3D10_VISTA)
#define USE_DX_API(ClientType)          ((ClientType == GMM_D3D12_VISTA) || (ClientType == GMM_D3D10_VISTA) || (ClientType == GMM_D3D9_VISTA))

#define GET_GMM_CLIENT_TYPE(pContext, ClientType)   \
if(pContext)                                        \
{                                                   \
    ClientType = ((GmmClientContext*)pContext)->GetClientType();         \
}                                                   \
else                                                \
{                                                   \
    ClientType = GMM_UNDEFINED_CLIENT;              \
}                                                   \

#define GET_RES_CLIENT_TYPE(pResourceInfo, ClientType)  \
if(pResourceInfo)                                       \
{                                                       \
    ClientType = (pResourceInfo)->GetClientType();      \
}                                                       \
else                                                    \
{                                                       \
    ClientType = GMM_UNDEFINED_CLIENT;                  \
}                                                       \

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
    GMM_NOT_TILED,
    GMM_TILED_4,
    GMM_TILED_64
}GMM_TILE_TYPE;

//===========================================================================
// typedef:
//     GMM_E2E_COMPRESSION_TYPE_ENUM
//
// Description:
//     This enum details compression type (i.e. render or media compressed, or uncompressed )
//---------------------------------------------------------------------------
typedef enum GMM_E2E_COMPRESSION_TYPE_ENUM
{
    GMM_UNCOMPRESSED,
    GMM_RENDER_COMPRESSED,
    GMM_MEDIA_COMPRESSED
}GMM_E2E_COMPRESSION_TYPE;

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

//      GMM_RESOURCE_FORMAT
//
// Description:
//      Enumeration of GMM supported resource formats.
//---------------------------------------------------------------------------
typedef enum GMM_RESOURCE_FORMAT_ENUM
{
    GMM_FORMAT_INVALID = 0, // <-- This stays zero! (For boolean and valid range checks.)

    #define GMM_FORMAT(Name, bpe, Width, Height, Depth, IsRT, IsASTC, RcsSurfaceFormat, SSCompressionFmt, Availability) \
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
#define GMM_UNIFIED_CMF_INVALID 0xD

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
    #define GMM_FORMAT(Name, bpe, Width, Height, Depth, IsRT, IsASTC, RcsSurfaceFormat, SSCompressionFmt, Availability) \
        GMM_SURFACESTATE_FORMAT_##Name = RcsSurfaceFormat,
    #include "GmmFormatTable.h"
} GMM_SURFACESTATE_FORMAT;

typedef enum GMM_E2ECOMP_FORMAT_ENUM
{
    GMM_E2ECOMP_FORMAT_INVALID = 0,
    GMM_E2ECOMP_FORMAT_ML8     = GMM_E2ECOMP_FORMAT_INVALID,
    GMM_E2ECOMP_FORMAT_RGB64, //1h - Reserved
    GMM_E2ECOMP_FORMAT_RGB32, //2h - Reserved

    GMM_E2ECOMP_MIN_FORMAT = GMM_E2ECOMP_FORMAT_RGB32,

    GMM_E2ECOMP_FORMAT_YUY2,       //3h

    GMM_E2ECOMP_FORMAT_Y410,       //4h

    GMM_E2ECOMP_FORMAT_Y210,       //5h
    GMM_E2ECOMP_FORMAT_Y216 = GMM_E2ECOMP_FORMAT_Y210,

    GMM_E2ECOMP_FORMAT_Y416,       //6h
    GMM_E2ECOMP_FORMAT_P010,       //7h
    GMM_E2ECOMP_FORMAT_P010_L = GMM_E2ECOMP_FORMAT_P010,
    GMM_E2ECOMP_FORMAT_P010_C = GMM_E2ECOMP_FORMAT_P010,
    GMM_E2ECOMP_FORMAT_P016,       //8h
    GMM_E2ECOMP_FORMAT_P016_L = GMM_E2ECOMP_FORMAT_P016,
    GMM_E2ECOMP_FORMAT_P016_C = GMM_E2ECOMP_FORMAT_P016,
    GMM_E2ECOMP_FORMAT_AYUV,       //9h

    GMM_E2ECOMP_FORMAT_ARGB8b,     //Ah
    GMM_E2ECOMP_FORMAT_RGB5A1 = GMM_E2ECOMP_FORMAT_ARGB8b,
    GMM_E2ECOMP_FORMAT_RGBA4 = GMM_E2ECOMP_FORMAT_ARGB8b,
    GMM_E2ECOMP_FORMAT_B5G6R5 = GMM_E2ECOMP_FORMAT_ARGB8b,

    GMM_E2ECOMP_FORMAT_SWAPY,      //Bh
    GMM_E2ECOMP_FORMAT_SWAPUV,     //Ch
    GMM_E2ECOMP_FORMAT_SWAPUVY,    //Dh
    GMM_E2ECOMP_FORMAT_YCRCB_SWAPUV = GMM_E2ECOMP_FORMAT_SWAPY,
    GMM_E2ECOMP_FORMAT_YCRCB_SWAPUVY = GMM_E2ECOMP_FORMAT_SWAPUV,
    GMM_E2ECOMP_FORMAT_YCRCB_SWAPY = GMM_E2ECOMP_FORMAT_SWAPUVY,
    
    GMM_E2ECOMP_FORMAT_RGB10b,     //Eh  --Which media format is it?
    GMM_E2ECOMP_FORMAT_NV12,       //Fh
    GMM_E2ECOMP_FORMAT_NV12_L = GMM_E2ECOMP_FORMAT_NV12,
    GMM_E2ECOMP_FORMAT_NV12_C = GMM_E2ECOMP_FORMAT_NV12,

    GMM_E2ECOMP_FORMAT_RGBAFLOAT16,            //0x10h

    GMM_E2ECOMP_FORMAT_R32G32B32A32_FLOAT,     //0x11h
    GMM_E2ECOMP_FORMAT_R32G32B32A32_SINT,      //0x12h
    GMM_E2ECOMP_FORMAT_R32G32B32A32_UINT,      //0x13h
    GMM_E2ECOMP_FORMAT_R16G16B16A16_UNORM,     //0x14h
    GMM_E2ECOMP_FORMAT_R16G16B16A16_SNORM,     //0x15h
    GMM_E2ECOMP_FORMAT_R16G16B16A16_SINT,      //0x16h
    GMM_E2ECOMP_FORMAT_R16G16B16A16_UINT,      //0x17h

    GMM_E2ECOMP_FORMAT_R10G10B10A2_UNORM,      //0x18h
    GMM_E2ECOMP_FORMAT_RGB10A2 = GMM_E2ECOMP_FORMAT_R10G10B10A2_UNORM,

    GMM_E2ECOMP_FORMAT_R10G10B10FLOAT_A2_UNORM,    //0x19h
    GMM_E2ECOMP_FORMAT_R10G10B10A2_UINT,       //0x1Ah
    GMM_E2ECOMP_FORMAT_R8G8B8A8_SNORM,         //0x1Bh
    GMM_E2ECOMP_FORMAT_R8G8B8A8_SINT,          //0x1Ch
    GMM_E2ECOMP_FORMAT_R8G8B8A8_UINT,          //0x1Dh

    GMM_E2ECOMP_FORMAT_R11G11B10_FLOAT,        //0x1Eh
    GMM_E2ECOMP_FORMAT_RG11B10 = GMM_E2ECOMP_FORMAT_R11G11B10_FLOAT,

    GMM_E2ECOMP_MAX_FORMAT = GMM_E2ECOMP_FORMAT_R11G11B10_FLOAT,    //should always be equal to last format encoding

    GMM_E2ECOMP_FORMAT_RGBA = GMM_E2ECOMP_FORMAT_INVALID,
    GMM_E2ECOMP_FORMAT_R = GMM_E2ECOMP_FORMAT_INVALID,
    GMM_E2ECOMP_FORMAT_RG = GMM_E2ECOMP_FORMAT_INVALID,
    GMM_E2ECOMP_FORMAT_D    = GMM_E2ECOMP_FORMAT_INVALID,

} GMM_E2ECOMP_FORMAT;

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
    GMM_HW_TILED_4_WALK     = 5,
    GMM_HW_TILED_64_WALK    = 6,
    GMM_HW_NOT_TILED        = 7
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

// Reset packing alignment to project default
#pragma pack(pop)

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

typedef struct
{
    union
    {
        struct
        {
            uint64_t AilDisableXe2CompressionRequest: 1;
	    uint64_t reserved: 63;

        };

        uint64_t Value;
    };
} GMM_AIL_STRUCT;
