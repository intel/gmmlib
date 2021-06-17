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

typedef enum GMM_FLATCCS_FORMAT_ENUM
{
    GMM_FLATCCS_FORMAT_R16S = 0,
    GMM_FLATCCS_FORMAT_R16U = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RG16F = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RG16U = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RG16S = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RGBA16S = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RGBA16U = GMM_FLATCCS_FORMAT_R16S,
    GMM_FLATCCS_FORMAT_RGBA16F = GMM_FLATCCS_FORMAT_R16S,

    GMM_FLATCCS_MIN_RC_FORMAT = GMM_FLATCCS_FORMAT_R16S,

    GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_R32S = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_R32U = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RG32F = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RG32S = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RG32U = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RGBA32F = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RGBA32S = GMM_FLATCCS_FORMAT_R32F,
    GMM_FLATCCS_FORMAT_RGBA32U = GMM_FLATCCS_FORMAT_R32F,

    GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RGBA4 = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_B5G6R5 = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_R8S = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_R8U = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RG8S = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RG8U = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RGBA8S = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_RGBA8U = GMM_FLATCCS_FORMAT_RGB5A1,
    GMM_FLATCCS_FORMAT_ML8    = GMM_FLATCCS_FORMAT_RGB5A1,

    GMM_FLATCCS_FORMAT_RGB10A2,
    GMM_FLATCCS_FORMAT_RG11B10,

    GMM_FLATCCS_FORMAT_R32F1,
    GMM_FLATCCS_FORMAT_R32S1 = GMM_FLATCCS_FORMAT_R32F1,
    GMM_FLATCCS_FORMAT_R32U1 = GMM_FLATCCS_FORMAT_R32F1,

    GMM_FLATCCS_FORMAT_R16F1,
    GMM_FLATCCS_FORMAT_R16S1 = GMM_FLATCCS_FORMAT_R16F1,
    GMM_FLATCCS_FORMAT_R16U1 = GMM_FLATCCS_FORMAT_R16F1,

    GMM_FLATCCS_FORMAT_R8S1,
    GMM_FLATCCS_FORMAT_R8U1 = GMM_FLATCCS_FORMAT_R8S1,

    GMM_FLATCCS_MAX_RC_FORMAT = GMM_FLATCCS_FORMAT_R8U1,

    GMM_FLATCCS_MIN_MC_FORMAT = 0x21,               //(0x1 <<5) ie Msb-5th bit turned on to identify MC encoding, to drop before SurfaceState usage
    GMM_FLATCCS_FORMAT_RGBA16_MEDIA = GMM_FLATCCS_MIN_MC_FORMAT,
    GMM_FLATCCS_FORMAT_Y210,
    GMM_FLATCCS_FORMAT_YUY2,
    GMM_FLATCCS_FORMAT_Y410,
    GMM_FLATCCS_FORMAT_Y216,
    GMM_FLATCCS_FORMAT_Y416,
    GMM_FLATCCS_FORMAT_P010,
    GMM_FLATCCS_FORMAT_P016,
    GMM_FLATCCS_FORMAT_AYUV,
    GMM_FLATCCS_FORMAT_ARGB8b,
    GMM_FLATCCS_FORMAT_SWAPY,
    GMM_FLATCCS_FORMAT_SWAPUV,
    GMM_FLATCCS_FORMAT_SWAPUVY,
    GMM_FLATCCS_FORMAT_RGB10b,
    GMM_FLATCCS_FORMAT_NV12,

    GMM_FLATCCS_FORMAT_YCRCB_SWAPUV = GMM_FLATCCS_FORMAT_SWAPUV,
    GMM_FLATCCS_FORMAT_YCRCB_SWAPUVY = GMM_FLATCCS_FORMAT_SWAPUVY,
    GMM_FLATCCS_FORMAT_YCRCB_SWAPY = GMM_FLATCCS_FORMAT_SWAPY,

    GMM_FLATCCS_MAX_MC_FORMAT = GMM_FLATCCS_FORMAT_NV12,    //should always be equal to last format encoding

    GMM_FLATCCS_FORMAT_INVALID,                          //equal to last valid encoding plus one
} GMM_FLATCCS_FORMAT;

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

// Set packing alignment
#pragma pack(push, 8)

#ifndef __GMM_KMD__
#ifdef _WIN32
    #ifndef PHYSICAL_ADDRESS
    #define PHYSICAL_ADDRESS LARGE_INTEGER
    #endif
#endif
    #ifndef PAGE_SIZE
    #define PAGE_SIZE 4096
    #endif
#endif /*__GMM_KMD__*/

//===========================================================================
// typedef:
//        GMM_FORMAT_ENTRY
//
// Description:
//      This struct is used to describe each surface format in the
//      GMM_RESOURCE_FORMAT enum. Each surface format is desginated as a
//      supported format on the running platform, as well as if the format is
//      renderable.
//
//---------------------------------------------------------------------------
typedef struct GMM_FORMAT_ENTRY_REC
{
    struct
    {
        uint32_t               ASTC         : 1;
        uint32_t               Compressed   : 1;
        uint32_t               RenderTarget : 1;
        uint32_t               Supported    : 1;
    };
    struct
    {
        uint16_t                BitsPer;
        uint8_t                 Depth;
        uint8_t                 Height;
        uint8_t                 Width;
    }                       Element;
    GMM_SURFACESTATE_FORMAT SurfaceStateFormat;
    union {
        GMM_E2ECOMP_FORMAT      AuxL1eFormat;
        uint8_t                 CompressionFormat;
    } CompressionFormat;
}GMM_FORMAT_ENTRY;

//===========================================================================
// typedef:
//     GMM_TILE_MODE_ENUM
//
// Description:
//     Enumeration of supported tile modes.
//
//--------------------------------------------------------------------------
#define DEFINE_TILE_BPEs(TileName) \
    TILE_##TileName##_8bpe,        \
    TILE_##TileName##_16bpe,       \
    TILE_##TileName##_32bpe,       \
    TILE_##TileName##_64bpe,       \
    TILE_##TileName##_128bpe       \

typedef enum GMM_TILE_MODE_ENUM
{
    TILE_NONE,
    // Legacy Tile Modes
    LEGACY_TILE_X,
    LEGACY_TILE_Y,
    // Tile-W is a 64x64 tile swizzled
    // onto a 128x32 Tile-Y.
    // For allocation purposes Tile-W
    // can be treated like Tile-Y
    //     TILE_W

    // Tiled Resource Modes (SKL+)
    DEFINE_TILE_BPEs( YF_1D     ),
    DEFINE_TILE_BPEs( YS_1D     ),
    DEFINE_TILE_BPEs( YF_2D     ),
    DEFINE_TILE_BPEs( YF_2D_2X  ),
    DEFINE_TILE_BPEs( YF_2D_4X  ),
    DEFINE_TILE_BPEs( YF_2D_8X  ),
    DEFINE_TILE_BPEs( YF_2D_16X ),
    DEFINE_TILE_BPEs( YF_3D     ),
    DEFINE_TILE_BPEs( YS_2D     ),
    DEFINE_TILE_BPEs( YS_2D_2X  ),
    DEFINE_TILE_BPEs( YS_2D_4X  ),
    DEFINE_TILE_BPEs( YS_2D_8X  ),
    DEFINE_TILE_BPEs( YS_2D_16X ),
    DEFINE_TILE_BPEs( YS_3D     ),

    // XE-HP
    TILE4,
    DEFINE_TILE_BPEs( _64_1D ),
    DEFINE_TILE_BPEs( _64_2D ),
    DEFINE_TILE_BPEs( _64_2D_2X),
    DEFINE_TILE_BPEs( _64_2D_4X),
    DEFINE_TILE_BPEs( _64_3D),

    GMM_TILE_MODES
}GMM_TILE_MODE;

#undef DEFINE_TILE_BPEs

typedef struct __TEX_ALIGNMENT
{
    uint32_t Width;  // pixels
    uint32_t Height; // scanlines
    uint32_t Depth;  // pixels
} ALIGNMENT;

//===========================================================================
// typedef:
//        GMM_TEXTURE_ALIGN
//
// Description:
//      The following struct describes the texture mip map unit alignment
//      required for each map format. The alignment values are platform
//      dependent.
//
//---------------------------------------------------------------------------
typedef struct GMM_TEXTURE_ALIGN_REC
{
    ALIGNMENT Compressed, Depth, Depth_D16_UNORM_1x_4x_16x, Depth_D16_UNORM_2x_8x, SeparateStencil, YUV422, XAdapter, AllOther;

    struct
    {
        ALIGNMENT  Align;
        uint32_t      MaxPitchinTiles;
    } CCS;
}GMM_TEXTURE_ALIGN;

//===========================================================================
// typedef:
//        __GMM_BUFFER_TYPE_REC
//
// Description:
//     This structure represents a buffer type. Common buffer types are
//     Display buffers, Color buffers, Linear buffers and ring buffers.
//     Each buffer type has platform specific size, dimension  and alignment
//     restricions that are stored here.
//
//---------------------------------------------------------------------------
typedef struct __GMM_BUFFER_TYPE_REC
{
    uint32_t           Alignment;              // Base Address Alignment
    uint32_t           PitchAlignment;         // Pitch Alignment restriction.
    uint32_t           RenderPitchAlignment;   // Pitch Alignment for render surface
    uint32_t           LockPitchAlignment;     // Pitch Alignment for locked surface
    uint32_t           MinPitch;               // Minimum pitch
    GMM_GFX_SIZE_T     MaxPitch;               // Maximum pitch
    GMM_GFX_SIZE_T     MinAllocationSize;      // Minimum Allocation size requirement

    uint32_t           MinHeight;              // Mininum height in bytes
    GMM_GFX_SIZE_T     MinWidth;               // Minimum width in bytes
    uint32_t           MinDepth;               // Minimum depth  (only for volume)
    GMM_GFX_SIZE_T     MaxHeight;              // Maximum height in bytes
    GMM_GFX_SIZE_T     MaxWidth;               // Maximum Width in bytes
    uint32_t           MaxDepth;               // Maximum depth  (only for volume)
    uint32_t           MaxArraySize;
    uint8_t            NeedPow2LockAlignment;  // Locking surface need to be power of 2 aligned
} __GMM_BUFFER_TYPE;

//===========================================================================
// typedef:
//        __GMM_PLATFORM_RESOURCE
//
// Description:
//     This structure represents various platform specific restrictions for
//      - buffer types
//      - tile dimensions
//      - # of fences regisers platform supports
//      - # of addressable bits
//      - aperture size
//
//----------------------------------------------------------------------------
typedef struct __GMM_PLATFORM_RESOURCE_REC
{
    PLATFORM             Platform;
    //
    // Define memory type req., alignment, min allocation size;
    //
    __GMM_BUFFER_TYPE    Vertex;           // Vertex Buffer restrictions
    __GMM_BUFFER_TYPE    Index;            // Index Buffer restrictions
    __GMM_BUFFER_TYPE    Constant;         //
    __GMM_BUFFER_TYPE    StateDx9ConstantBuffer; // Dx9 Constant Buffer pool restrictions

    __GMM_BUFFER_TYPE    Texture2DSurface; // 2D texture surface
    __GMM_BUFFER_TYPE    Texture2DLinearSurface; // 2D Linear media surface
    __GMM_BUFFER_TYPE    Texture3DSurface; // 3D texture surface
    __GMM_BUFFER_TYPE    CubeSurface;      // cube texture surface
    __GMM_BUFFER_TYPE    BufferType;       // Buffer type surface

    __GMM_BUFFER_TYPE    Color;            // Color (Render Target) Buffer
    __GMM_BUFFER_TYPE    Depth;            // Depth Buffer Restriction
    __GMM_BUFFER_TYPE    Stencil;          // Stencil Buffer Restrictions
    __GMM_BUFFER_TYPE    HiZ;              // Hierarchical Depth Buffer Resrictions
    __GMM_BUFFER_TYPE    Stream;           //

    __GMM_BUFFER_TYPE    Video;            // Video Planar surface restrictions
    __GMM_BUFFER_TYPE    MotionComp;       // Motion Compensation buffer

    __GMM_BUFFER_TYPE    Overlay;          // Overlay Buffer
    __GMM_BUFFER_TYPE    Nndi;             // Non native display buffer restrictions
    __GMM_BUFFER_TYPE    ASyncFlipSurface; // ASync flip chain Buffers

    __GMM_BUFFER_TYPE    HardwareMBM;      // Buffer Restrictions

    __GMM_BUFFER_TYPE    InterlacedScan;   //
    __GMM_BUFFER_TYPE    TextApi;          //

    __GMM_BUFFER_TYPE    Linear;           // Linear(Generic) Buffer restrictions
    __GMM_BUFFER_TYPE    Cursor;           // Cursor surface restrictions
    __GMM_BUFFER_TYPE    NoRestriction;    // Motion Comp Buffer

    __GMM_BUFFER_TYPE    XAdapter;         // Cross adapter linear buffer restrictions

    GMM_TEXTURE_ALIGN    TexAlign;         // Alignment Units for Texture Maps

    //
    // various tile dimension based on platform
    //
    GMM_TILE_INFO        TileInfo[GMM_TILE_MODES];

    //
    // General platform Restriction
    //
    uint32_t                NumberFenceRegisters;
    uint32_t                MinFenceSize;               // 1 MB for Napa, 512 KB for Almador

    uint32_t                FenceLowBoundShift;
    uint32_t                FenceLowBoundMask;

    uint32_t                PageTableSteer;             // Default for page table steer register

    uint32_t                PagingBufferPrivateDataSize;
    uint32_t                MaxLod;
    uint32_t                FBCRequiredStolenMemorySize; // Stolen Memory size required for FBC

    GMM_FORMAT_ENTRY     FormatTable[GMM_RESOURCE_FORMATS];

    uint32_t                ResAllocTag[GMM_MAX_HW_RESOURCE_TYPE];  // uint32_t = 4 8-bit ASCII characters

    uint32_t                SurfaceStateYOffsetGranularity;
    uint32_t                SamplerFetchGranularityWidth;
    uint32_t                SamplerFetchGranularityHeight;

    int64_t                SurfaceMaxSize; // int64_t - Surface size is 64 bit for all configurations
    uint32_t                MaxGpuVirtualAddressBitsPerResource;
    uint32_t                MaxSLMSize;

    uint8_t                 HiZPixelsPerByte; //HiZ-Bpp is < 1, keep inverse
    uint64_t                ReconMaxHeight;
    uint64_t                ReconMaxWidth;
    uint8_t                 NoOfBitsSupported;                 // No of bits supported for System physcial address on GPU
    uint64_t                HighestAcceptablePhysicalAddress;  // Highest acceptable System physical Address
}__GMM_PLATFORM_RESOURCE, GMM_PLATFORM_INFO;

//***************************************************************************
//
//                      GMM_PLATFORM_INFO API
//
//***************************************************************************
uint32_t GMM_STDCALL GmmPlatformGetBppFromGmmResourceFormat(GMM_RESOURCE_FORMAT Format);

// Reset packing alignment to project default
#pragma pack(pop)

#ifdef __cplusplus
}
#endif /*__cplusplus*/
