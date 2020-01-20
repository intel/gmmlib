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


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#if _WIN32
#ifndef __GMM_KMD__
    typedef LONG NTSTATUS;
    #include <windows.h>
    #include <d3d9types.h>
    #include <d3dkmthk.h>
#endif
#endif

// Set packing alignment
#pragma pack(push, 8)

// Rotation bitmap fields
#define GMM_KMD_ROTATE              0x00000001  // this is to inform KMD to enable
                                                // rotation in Full screen case

// Color separation textures width division factors and array size
#define GMM_COLOR_SEPARATION_WIDTH_DIVISION       4
#define GMM_COLOR_SEPARATION_RGBX_WIDTH_DIVISION  3
#define GMM_COLOR_SEPARATION_ARRAY_SIZE           4   // XXX: Change that to 3 once 2nd green scanout using sprites is validated

//===========================================================================
// typedef:
//      GMM_RESOURCE_MMC_INFO
//
// Description:
//      This struct is used to describe Media Memory Compression information.
//---------------------------------------------------------------------------
typedef enum GMM_RESOURCE_MMC_INFO_REC
{
    GMM_MMC_DISABLED = 0,
    GMM_MMC_HORIZONTAL,
    GMM_MMC_VERTICAL,
}GMM_RESOURCE_MMC_INFO;

//===========================================================================
// typedef:
//      GMM_RESOURCE_MMC_HINT
//
// Description:
//      This struct is used to indicate if Media Memory Compression is needed.
//---------------------------------------------------------------------------
typedef enum GMM_RESOURCE_MMC_HINT_REC
{
    GMM_MMC_HINT_ON = 0,
    GMM_MMC_HINT_OFF,
}GMM_RESOURCE_MMC_HINT;

//===========================================================================
// typedef:
//     GMM_MSAA_SAMPLE_PATTERN
//
// Description:
//     This enum details the sample pattern of a MSAA texture
//---------------------------------------------------------------------------
typedef enum GMM_MSAA_SAMPLE_PATTERN_REC
{
    GMM_MSAA_DISABLED = 0,
    GMM_MSAA_STANDARD,
    GMM_MSAA_CENTEROID,
    GMM_MSAA_REGULAR

}GMM_MSAA_SAMPLE_PATTERN;

typedef enum GMM_TILE_RANGE_FLAG_ENUM
{
    GMM_TILE_RANGE_NULL = 0x00000001,
    GMM_TILE_RANGE_SKIP = 0x00000002,
    GMM_TILE_RANGE_REUSE_SINGLE_TILE = 0x00000004,
} GMM_TILE_RANGE_FLAG;

//===========================================================================
// typedef:
//        GMM_RESOURCE_MSAA_INFO
//
// Description:
//     This struct is used to describe MSAA information.
//---------------------------------------------------------------------------
typedef struct GMM_RESOURCE_MSAA_INFO_REC
{
    GMM_MSAA_SAMPLE_PATTERN     SamplePattern;
    uint32_t                    NumSamples;
}GMM_RESOURCE_MSAA_INFO;

//===========================================================================
// typedef:
//        GMM_RESOURCE_ALIGNMENT_UNITS
//
// Description:
//     Various alignment units of an allocation.
//     NOTE: H/VAlign and QPitch stored *UNCOMPRESSED* here, despite Gen9+
//           SURFACE_STATE using compressed!!!
//---------------------------------------------------------------------------
typedef struct GMM_RESOURCE_ALIGNMENT_REC
{
    uint8_t    ArraySpacingSingleLod;  // Single-LOD/Full Array Spacing
    uint32_t   BaseAlignment;          // Base Alignment
    uint32_t   HAlign, VAlign, DAlign; // Alignment Unit Width/Height
    uint32_t   MipTailStartLod;        // Mip Tail (SKL+)
    uint32_t   PackedMipStartLod;      // Packed Mip (Pre-Gen9 / Undefined64KBSwizzle)
    uint32_t   PackedMipWidth;         // Packed Mip Width in # of 64KB Tiles (Pre-Gen9 / Undefined64KBSwizzle)
    uint32_t   PackedMipHeight;        // Packed Mip Height in # of 64KB Tiles (Pre-Gen9 / Undefined64KBSwizzle)
    uint32_t   QPitch;                 // Programmable QPitch (BDW+)
}GMM_RESOURCE_ALIGNMENT;

//===========================================================================
// typedef:
//        GMM_PAGETABLE_MGR
//
// Description:
//     This struct is used to describe page table manager.
//     Forward Declaration: Defined in GmmPageTableMgr.h
//---------------------------------------------------------------------------
#ifdef __cplusplus
    namespace GmmLib
    {
        class GmmPageTableMgr;
    }
    typedef GmmLib::GmmPageTableMgr GMM_PAGETABLE_MGR;
#else
    typedef struct GmmPageTableMgr GMM_PAGETABLE_MGR;
#endif

//===========================================================================
// typedef:
//        GMM_RESOURCE_INFO
//
// Description:
//     This struct is used to describe resource allocations.
//     Forward Declaration: Defined in GmmResourceInfo*.h
//---------------------------------------------------------------------------
#ifdef __cplusplus
    namespace GmmLib
    {
        #ifdef _WIN32
            class GmmResourceInfoWin;
            typedef GmmResourceInfoWin GmmResourceInfo;
        #else
            class GmmResourceInfoLin;
            typedef GmmResourceInfoLin GmmResourceInfo;
        #endif
    }
    typedef GmmLib::GmmResourceInfo GMM_RESOURCE_INFO;
    typedef GmmLib::GmmResourceInfo GMM_RESOURCE_INFO_REC;
    typedef GmmLib::GmmResourceInfo* PGMM_RESOURCE_INFO;

#else
    typedef struct GmmResourceInfo GMM_RESOURCE_INFO;
    typedef struct GmmResourceInfo* PGMM_RESOURCE_INFO;
#endif

//===========================================================================
// TBD - Place holder for GMM_RESOURCE_FLAG definition.
//---------------------------------------------------------------------------
#include "GmmResourceFlags.h"
#if defined __linux__
    #include "External/Linux/GmmResourceInfoLinExt.h"
#endif

//==========================================================================
// typedef:
//     GMM_S3D_OFFSET_INFO
//
// Description:
//     Contains offset info for S3D resources.
//
//--------------------------------------------------------------------------
typedef struct GMM_S3D_OFFSET_INFO_REC
{
    uint32_t           AddressOffset;
    uint32_t           OffsetX;
    uint32_t           OffsetY;
    uint32_t           LineOffsetY;
} GMM_S3D_OFFSET_INFO;

//==========================================================================
// typedef:
//     GMM_S3D_INFO
//
// Description:
//     Describes properties of S3D feature.
//
//--------------------------------------------------------------------------
typedef struct GMM_S3D_INFO_REC
{
    uint32_t           DisplayModeHeight;    // Current display mode resolution
    uint32_t           NumBlankActiveLines;  // Number of blank lines
    uint32_t           RFrameOffset;         // R frame offset
    uint32_t           BlankAreaOffset;      // Blank area offset
    uint32_t           TallBufferHeight;     // Tall buffer height
    uint32_t           TallBufferSize;       // Tall buffer size
    uint8_t            IsRFrame;             // Flag indicating this is the R frame
} GMM_S3D_INFO;

//===========================================================================
// typedef:
//        GMM_RESCREATE_PARAMS
//
// Description:
//     This structure is used to describe an resource allocation request.
//     Please note that AllocationReuse makes use of member order.
//     more info: oskl.h: AllocationReusePredicate and Functor
//---------------------------------------------------------------------------
typedef struct GMM_RESCREATE_PARAMS_REC
{
    GMM_RESOURCE_TYPE                   Type;    // 1D/2D/.../SCRATCH/...
    GMM_RESOURCE_FORMAT                 Format;  // Pixel format e.g. NV12, GENERIC_8BIT
    GMM_RESOURCE_FLAG                   Flags;   // See substructure type.
    GMM_RESOURCE_MSAA_INFO              MSAA;    // How to sample this resource for anti-alisaing.

    GMM_RESOURCE_USAGE_TYPE             Usage;   // Intended use for this resource. See enumerated type.
    uint32_t                            CpTag;

    union
    {
        uint32_t              BaseWidth;         // Aligned width of buffer (aligned according to .Format)
        GMM_GFX_SIZE_T        BaseWidth64;

        // The HW buffer types, BUFFER and STRBUF, are arrays of user-defined
        // struct's. Their natural sizing parameters are (1) the struct size
        // and (2) the numer of array elements (i.e. the number of structs).
        // However, the GMM allows these to be allocated in either of two ways:
        // (1) Client computes total allocation size and passes via BaseWidth
        // (leaving ArraySize 0), or (2) Client passes natural sizing parameters
        // via BaseWidth and ArraySize. To make the latter more readable,
        // clients can use the BaseWidth union alias, BufferStructSize (as well
        // as the GmmResGetBaseWidth alias, GmmResGetBufferStructSize).
        uint32_t                           BufferStructSize; // Note: If ever de-union'ing, mind the GmmResGetBufferStructSize #define.
    };

    uint32_t                               BaseHeight;     // Aligned height of buffer (aligned according to .Format)
    uint32_t                               Depth;          // For 3D resources Depth>1, for 1D/2D, a default of 0 is uplifted to 1.

    uint32_t                               MaxLod;
    uint32_t                               ArraySize;      // A number of n-dimensional buffers can be allocated together.

    uint32_t                               BaseAlignment;
    uint32_t                               OverridePitch;
    #if(LHDM)
        D3DDDI_RATIONAL                 DdiRefreshRate;
        D3DDDI_RESOURCEFLAGS            DdiD3d9Flags;
        D3DDDIFORMAT                    DdiD3d9Format;
        D3DDDI_VIDEO_PRESENT_SOURCE_ID  DdiVidPnSrcId;
    #endif
    uint32_t                            RotateInfo;
    uint64_t                            pExistingSysMem;
    GMM_GFX_SIZE_T                      ExistingSysMemSize;
#ifdef _WIN32
    D3DKMT_HANDLE                       hParentAllocation; //For ExistingSysMem Virtual Padding
#endif

    #if __GMM_KMD__
        uint32_t                           CpuAccessible; // Kernel mode clients set if CPU pointer to resource is required.
        GMM_S3D_INFO                    S3d;
        void                           *pDeviceContext;
    #endif
    uint32_t                               MaximumRenamingListLength;
    uint8_t                             NoGfxMemory;
    GMM_RESOURCE_INFO                   *pPreallocatedResInfo;

} GMM_RESCREATE_PARAMS;

typedef struct GMM_RESCREATE_CUSTOM_PARAMS__REC
{
    GMM_RESOURCE_TYPE              Type;    // 1D/2D/.../SCRATCH/...
    GMM_RESOURCE_FORMAT            Format;         // Pixel format e.g. NV12, GENERIC_8BIT
    GMM_RESOURCE_FLAG              Flags;          // See substructure type.
    GMM_RESOURCE_USAGE_TYPE        Usage;   // Intended use for this resource. See enumerated type.
    GMM_GFX_SIZE_T                 BaseWidth64;
    uint32_t                       BaseHeight;     // Aligned height of buffer (aligned according to .Format)

    uint32_t                       Pitch;
    GMM_GFX_SIZE_T                 Size;
    uint32_t                       BaseAlignment;

    struct
    {
        uint32_t X[GMM_MAX_PLANE];
        uint32_t Y[GMM_MAX_PLANE];
    }PlaneOffset;

    uint32_t NoOfPlanes;
}GMM_RESCREATE_CUSTOM_PARAMS;

//===========================================================================
// enum :
//        GMM_UNIFIED_AUX_TYPE
//
// Description:
//     This enumarates various aux surface types in a unified
//     auxiliary surface
//---------------------------------------------------------------------------
// Usage :
//     UMD client use this enum to request the aux offset and size.
//     using via GmmResGetAuxSurfaceOffset, GmmResGetSizeAuxSurface
//
//              RT buffer with Clear Color
//      ________________________________________________
//     |                                                |
//     |                                                |
//     |                                                |
//     |             Pixel Data                         |
//     |                                                |
//     |                                                |
//     |                                                |
//     |                                                |
//     |                                                |
//     |                                                |
//   A |                                                |
//  -->|________________________________________________|
//     |                                  |             |
//     |                                  |             |
//     |             Tag plane            |             |
//     |       (a.k.a mcs, aux-plane,ccs) | ------------|->GmmResGetSizeAuxSurface(pRes, GMM_AUX_CCS)
//     |                                  |             |
//  -->|__________________________________|             |
//   B | CC FV | CC NV  | ------------------------------|--->GmmResGetSizeAuxSurface(pRes, GMM_AUX_CC)
//     |_______|________|                               |
//     |                                                |
//     |                                                |
//     |   Padded to Main Surf's(TileHeight & SurfPitch)|   --> Padding needed for Global GTT aliasing
//  -->|________________________________________________|
//   C |                                                |
//     |             Tag plane                          |
//     |          for MSAA/Depth compr                  |
//     |       (a.k.a ccs, zcs)             ------------|->GmmResGetSizeAuxSurface(pRes, GMM_AUX_CCS or GMM_AUX_ZCS)
//     |________________________________________________|
//
//     Where
//     FV. Clear color Float Value
//     NV. Clear color Native Value
//     A. GmmResGetAuxSurfaceOffset(pRes, GMM_AUX_CCS)
//     B. GmmResGetAuxSurfaceOffset(pRes, GMM_AUX_CC)
//     C. GmmResGetAuxSurfaceOffset(pRes, GMM_AUX_CCS or GMM_AUX_ZCS)
typedef enum
{
    GMM_AUX_CCS,    // RT buffer's color control surface (Unpadded)
    GMM_AUX_Y_CCS,  // color control surface for Y-plane
    GMM_AUX_UV_CCS, // color control surface for UV-plane
    GMM_AUX_CC,     // clear color value (4kb granularity)
    GMM_AUX_COMP_STATE, // Media compression state (cacheline aligned 64B)
    GMM_AUX_HIZ,    // HiZ surface for unified Depth buffer
    GMM_AUX_MCS,    // multi-sample control surface for unified MSAA
    GMM_AUX_ZCS,    // CCS for Depth Z compression
    GMM_AUX_SURF    // Total Aux Surface (CCS + CC + Padding)
} GMM_UNIFIED_AUX_TYPE;

//===========================================================================
// enum :
//        GMM_SIZE_PARAM
//
// Description:
//     This enumarates various surface size parameters available for GetSize query.
//     Refer GmmResourceInfoCommon.h  GetSize() api
//
//     Note:
//     Below legacy API to query surface size are deprecated and will be removed in
//     later gmm releases.
//     - GmmResGetSizeSurface()/ pResInfo->GetSizeSurface()
//     - GmmResGetSizeMainSurface()/  pResInfo->GetSizeAllocation()
//     - GmmResGetSizeAllocation()/ pResInfo->GetSizeMainSurface()
//---------------------------------------------------------------------------
// Usage :
//     UMD client use this enum to request the surface size.
//===========================================================================
typedef enum
{
    GMM_INVALID_PARAM,       // Leave 0 as invalid to force client to explictly set
    GMM_MAIN_SURF,           // Main surface size(w/o aux data)
    GMM_MAIN_PLUS_AUX_SURF,  // Main surface plus auxilary data, includes ccs, cc, zcs, mcs metadata. Renderable portion of the surface.
    GMM_TOTAL_SURF,          // Main+Aux with additional padding based on hardware PageSize.
    GMM_MAPGPUVA_SIZE = GMM_TOTAL_SURF,// To be used for mapping gpu virtual address space.
} GMM_SIZE_PARAM;


//===========================================================================
// typedef:
//        GMM_RES_COPY_BLT
//
// Description:
//     Describes a GmmResCpuBlt operation.
//---------------------------------------------------------------------------
typedef struct GMM_RES_COPY_BLT_REC
{
    struct // GPU Surface Description...
    {
        void            *pData;         // Pointer to base of the mapped resource data (e.g. D3DDDICB_LOCK.pData).
        uint32_t           Slice;          // Array/Volume Slice or Cube Face; zero if N/A.
        uint32_t           MipLevel;       // Index of applicable MIP, or zero if N/A.
        //uint32_t         MsaaSample;     // Index of applicable MSAA sample, or zero if N/A.
        uint32_t           OffsetX;        // Pixel offset from left-edge of specified (Slice/MipLevel) subresource.
        uint32_t           OffsetY;        // Pixel row offset from top of specified subresource.
        uint32_t           OffsetSubpixel; // Byte offset into the surface pixel of the applicable subpixel.
    }               Gpu;                // Surface description of GPU resource involved in BLT.

    struct // System Surface Description...
    {
        void            *pData;         // Pointer to system memory surface.
        uint32_t           RowPitch;       // Row pitch in bytes of pData surface.
        uint32_t           SlicePitch;     // Slice pitch in bytes of pData surface; ignored if Blt.Slices <= 1.
        uint32_t           PixelPitch;     // Number of bytes from one pData pixel to its horizontal neighbor; 0 = "Same as GPU Resource".
        //uint32_t         MsaaSamplePitch;// Number of bytes from one pData MSAA sample to the next; ignored if Blt.MsaaSamples <= 1.
        uint32_t           BufferSize;     // Number of bytes at pData. (Value used only in asserts to catch overuns.)
    }               Sys;                // Description of system memory surface being BLT'ed to/from the GPU surface.

    struct // BLT Description...
    {
        uint32_t           Width;          // Copy width in pixels; 0 = "Full Width" of specified subresource.
        uint32_t           Height;         // Copy height in pixel rows; 0 = "Full Height" of specified subresource.
        uint32_t           Slices;         // Number of slices being copied; 0 = 1 = "N/A or single slice".
        uint32_t           BytesPerPixel;  // Number of bytes to copy, per pixel; 0 = "Same as Sys.PixelPitch".
        //uint32_t         MsaaSamples;    // Number of samples to copy per pixel; 0 = 1 = "N/A or single sample".
        uint8_t            Upload;         // true = Sys-->Gpu; false = Gpu-->Sys.
    }               Blt;                // Description of the BLT being performed.
} GMM_RES_COPY_BLT;

//===========================================================================
// typedef:
//        GMM_GET_MAPPING
//
// Description:
//     GmmResGetMappingSpanDesc interface and inter-call state.
//---------------------------------------------------------------------------
typedef enum
{
    GMM_MAPPING_NULL = 0,
    GMM_MAPPING_LEGACY_Y_TO_STDSWIZZLE_SHAPE,
    GMM_MAPPING_GEN9_YS_TO_STDSWIZZLE,
} GMM_GET_MAPPING_TYPE;

typedef struct GMM_GET_MAPPING_REC
{
    GMM_GET_MAPPING_TYPE    Type;

    struct
    {
        GMM_GFX_SIZE_T      VirtualOffset;
        GMM_GFX_SIZE_T      PhysicalOffset;
        GMM_GFX_SIZE_T      Size;
    }                   Span, __NextSpan;

    struct
    {
        struct
        {
            uint32_t               Width, Height, Depth; // in Uncompressed Pixels
        }                   Element, Tile;
        struct
        {
            GMM_GFX_SIZE_T      Physical, Virtual;
        }                   Slice0MipOffset, SlicePitch;
        uint32_t               EffectiveLodMax, Lod, Row, RowPitchVirtual, Rows, Slice, Slices;
        GMM_YUV_PLANE       Plane, LastPlane;
    }                   Scratch; // Zero on initial call to GmmResGetMappingSpanDesc and then let persist.
} GMM_GET_MAPPING;


//***************************************************************************
//
//                      GMM_RESOURCE_INFO API
//
//***************************************************************************
uint8_t             GMM_STDCALL GmmIsPlanar(GMM_RESOURCE_FORMAT Format);
uint8_t             GMM_STDCALL GmmIsP0xx(GMM_RESOURCE_FORMAT Format);
uint8_t             GMM_STDCALL GmmIsUVPacked(GMM_RESOURCE_FORMAT Format);
bool                GMM_STDCALL GmmIsYUVFormatLCUAligned(GMM_RESOURCE_FORMAT Format);
#define                         GmmIsYUVPlanar GmmIsPlanar // TODO(Benign): Support old name until we have a chance to correct in UMD(s) using this. No longer YUV since there are now RGB planar formats.
uint8_t             GMM_STDCALL GmmIsReconstructableSurface(GMM_RESOURCE_FORMAT Format);
uint8_t             GMM_STDCALL GmmIsCompressed(GMM_RESOURCE_FORMAT Format);
uint8_t             GMM_STDCALL GmmIsYUVPacked(GMM_RESOURCE_FORMAT Format);
uint8_t             GMM_STDCALL GmmIsRedecribedPlanes(GMM_RESOURCE_INFO *pGmmResource);
uint8_t             GMM_STDCALL GmmResApplyExistingSysMem(GMM_RESOURCE_INFO *pGmmResource, void *pExistingSysMem, GMM_GFX_SIZE_T ExistingSysMemSize);
uint8_t             GMM_STDCALL GmmIsStdTilingSupported(GMM_RESCREATE_PARAMS *pCreateParams);
GMM_RESOURCE_INFO*  GMM_STDCALL GmmResCopy(GMM_RESOURCE_INFO *pGmmResource);
void                GMM_STDCALL GmmResMemcpy(void *pDst, void *pSrc);
uint8_t             GMM_STDCALL GmmResCpuBlt(GMM_RESOURCE_INFO *pGmmResource, GMM_RES_COPY_BLT *pBlt);
GMM_RESOURCE_INFO*  GMM_STDCALL GmmResCreate(GMM_RESCREATE_PARAMS *pCreateParams);
void                GMM_STDCALL GmmResFree(GMM_RESOURCE_INFO *pGmmResource);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetSizeMainSurface(const GMM_RESOURCE_INFO *pResourceInfo);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetSizeSurface(GMM_RESOURCE_INFO *pResourceInfo);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetSizeAllocation(GMM_RESOURCE_INFO *pResourceInfo);

uint32_t               GMM_STDCALL GmmResGetArraySize(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetAuxBitsPerPixel(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetAuxPitch(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetAuxQPitch(GMM_RESOURCE_INFO *pGmmResource);
uint8_t                GMM_STDCALL GmmResIs64KBPageSuitable(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetAuxSurfaceOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType);
GMM_GFX_SIZE_T         GMM_STDCALL GmmResGetAuxSurfaceOffset64(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType);
GMM_GFX_SIZE_T         GMM_STDCALL GmmResGetSizeAuxSurface(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType);
uint32_t               GMM_STDCALL GmmResGetAuxHAlign(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetAuxVAlign(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetBaseAlignment(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetBaseHeight(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetBaseWidth(GMM_RESOURCE_INFO *pGmmResource);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetBaseWidth64(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetBitsPerPixel(GMM_RESOURCE_INFO *pGmmResource);
#define                         GmmResGetBufferStructSize GmmResGetBaseWidth // See GMM_RESCREATE_PARAMS.BufferStructSize comment.
uint32_t               GMM_STDCALL GmmResGetCompressionBlockDepth(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetCompressionBlockHeight(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetCompressionBlockWidth(GMM_RESOURCE_INFO *pGmmResource);
GMM_CPU_CACHE_TYPE  GMM_STDCALL GmmResGetCpuCacheType(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetDepth(GMM_RESOURCE_INFO *pGmmResource);
void                GMM_STDCALL GmmResGetFlags(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_FLAG *pFlags /*output*/); //TODO: Remove after changing all UMDs
GMM_RESOURCE_FLAG   GMM_STDCALL GmmResGetResourceFlags(const GMM_RESOURCE_INFO *pGmmResource);
GMM_GFX_ADDRESS     GMM_STDCALL GmmResGetGfxAddress(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetHAlign(GMM_RESOURCE_INFO *pGmmResource);
#define                         GmmResGetLockPitch GmmResGetRenderPitch // Support old name until UMDs drop use.
uint8_t                GMM_STDCALL GmmResGetMappingSpanDesc(GMM_RESOURCE_INFO *pGmmResource, GMM_GET_MAPPING *pMapping);
uint32_t            GMM_STDCALL GmmResGetMaxLod(GMM_RESOURCE_INFO *pGmmResource);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetStdLayoutSize(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetSurfaceStateMipTailStartLod(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetSurfaceStateTileAddressMappingMode(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetSurfaceStateStdTilingModeExt(GMM_RESOURCE_INFO *pGmmResource);
GMM_RESOURCE_MMC_INFO GMM_STDCALL GmmResGetMmcMode(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex);
uint32_t               GMM_STDCALL GmmResGetNumSamples(GMM_RESOURCE_INFO *pGmmResource);
GMM_STATUS          GMM_STDCALL GmmResGetOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_REQ_OFFSET_INFO *pReqInfo);
GMM_STATUS          GMM_STDCALL GmmResGetOffsetFor64KBTiles(GMM_RESOURCE_INFO *pGmmResource, GMM_REQ_OFFSET_INFO *pReqInfo);
uint32_t               GMM_STDCALL GmmResGetPaddedHeight(GMM_RESOURCE_INFO *pGmmResource, uint32_t MipLevel);
uint32_t               GMM_STDCALL GmmResGetPaddedWidth(GMM_RESOURCE_INFO *pGmmResource, uint32_t MipLevel);
uint32_t               GMM_STDCALL GmmResGetPaddedPitch(GMM_RESOURCE_INFO *pGmmResource, uint32_t MipLevel);
void*               GMM_STDCALL GmmResGetPrivateData(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetQPitch(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetQPitchPlanar(GMM_RESOURCE_INFO *pGmmResource, GMM_YUV_PLANE Plane);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetQPitchInBytes(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetRenderAuxPitchTiles(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetRenderPitch(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetRenderPitchIn64KBTiles(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetRenderPitchTiles(GMM_RESOURCE_INFO *pGmmResource);
GMM_RESOURCE_FORMAT GMM_STDCALL GmmResGetResourceFormat(GMM_RESOURCE_INFO *pGmmResource);
GMM_RESOURCE_TYPE   GMM_STDCALL GmmResGetResourceType(GMM_RESOURCE_INFO *pGmmResource);
uint32_t                GMM_STDCALL GmmResGetRotateInfo(GMM_RESOURCE_INFO *pGmmResource);
GMM_MSAA_SAMPLE_PATTERN GMM_STDCALL GmmResGetSamplePattern(GMM_RESOURCE_INFO *pGmmResource);
uint32_t                GMM_STDCALL GmmResGetSizeOfStruct(void);
GMM_SURFACESTATE_FORMAT GMM_STDCALL GmmResGetSurfaceStateFormat(GMM_RESOURCE_INFO *pGmmResource);
GMM_SURFACESTATE_FORMAT GMM_STDCALL GmmGetSurfaceStateFormat(GMM_RESOURCE_FORMAT Format);
uint32_t               GMM_STDCALL GmmResGetSurfaceStateHAlign(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetSurfaceStateVAlign(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetSurfaceStateTiledResourceMode(GMM_RESOURCE_INFO *pGmmResource);
void*               GMM_STDCALL GmmResGetSystemMemPointer(GMM_RESOURCE_INFO *pGmmResource, uint8_t IsD3DDdiAllocation);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetSystemMemSize( GMM_RESOURCE_INFO*  pRes );
uint32_t            GMM_STDCALL GmmResGetTallBufferHeight(GMM_RESOURCE_INFO *pResourceInfo);
uint32_t            GMM_STDCALL GmmResGetMipHeight(GMM_RESOURCE_INFO *pResourceInfo, uint32_t MipLevel);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetMipWidth(GMM_RESOURCE_INFO *pResourceInfo, uint32_t MipLevel);
uint32_t            GMM_STDCALL GmmResGetMipDepth(GMM_RESOURCE_INFO *pResourceInfo, uint32_t MipLevel);
uint8_t                GMM_STDCALL GmmResGetCornerTexelMode(GMM_RESOURCE_INFO *pGmmResource);
GMM_TEXTURE_LAYOUT  GMM_STDCALL GmmResGetTextureLayout(GMM_RESOURCE_INFO *pGmmResource);
GMM_TILE_TYPE       GMM_STDCALL GmmResGetTileType(GMM_RESOURCE_INFO *pGmmResource);
uint32_t            GMM_STDCALL GmmResGetVAlign(GMM_RESOURCE_INFO *pGmmResource);
uint8_t             GMM_STDCALL GmmResIsArraySpacingSingleLod(GMM_RESOURCE_INFO *pGmmResource);
uint8_t             GMM_STDCALL GmmResIsASTC(GMM_RESOURCE_INFO *pGmmResource);
uint8_t             GMM_STDCALL GmmResIsLockDiscardCompatible(GMM_RESOURCE_INFO *pGmmResource);
uint8_t             GMM_STDCALL GmmResIsMediaMemoryCompressed(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex);
uint8_t             GMM_STDCALL GmmResIsMsaaFormatDepthStencil(GMM_RESOURCE_INFO *pGmmResource);
uint8_t             GMM_STDCALL GmmResIsSvm(GMM_RESOURCE_INFO *pGmmResource);
void                GMM_STDCALL GmmResSetMmcMode(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_MMC_INFO Mode, uint32_t ArrayIndex);
void                GMM_STDCALL GmmResSetMmcHint(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_MMC_HINT Hint, uint32_t ArrayIndex);
GMM_RESOURCE_MMC_HINT  GMM_STDCALL GmmResGetMmcHint(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex);
uint8_t                GMM_STDCALL GmmResIsColorSeparation(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResTranslateColorSeparationX(GMM_RESOURCE_INFO *pGmmResource, uint32_t x);
uint32_t               GMM_STDCALL GmmResGetColorSeparationArraySize(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetColorSeparationPhysicalWidth(GMM_RESOURCE_INFO *pGmmResource);
uint8_t                GMM_STDCALL GmmResGetSetHardwareProtection(GMM_RESOURCE_INFO *pGmmResource, uint8_t GetIsEncrypted, uint8_t SetIsEncrypted);
uint32_t               GMM_STDCALL GmmResGetMaxGpuVirtualAddressBits(GMM_RESOURCE_INFO *pGmmResource);
uint8_t                GMM_STDCALL GmmIsSurfaceFaultable(GMM_RESOURCE_INFO *pGmmResource);
uint32_t               GMM_STDCALL GmmResGetMaximumRenamingListLength(GMM_RESOURCE_INFO* pGmmResource);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetPlanarGetXOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_YUV_PLANE Plane);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetPlanarGetYOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_YUV_PLANE Plane);
GMM_GFX_SIZE_T      GMM_STDCALL GmmResGetPlanarAuxOffset(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex, GMM_UNIFIED_AUX_TYPE Plane);

// Remove when client moves to new interface
uint32_t            GMM_STDCALL GmmResGetRenderSize(GMM_RESOURCE_INFO *pResourceInfo);

//=====================================================================================================
//forward declarations
struct GMM_TEXTURE_INFO_REC;

// Hack to define and undefine typedef name to avoid redefinition of the
// typedef.  Part 1.

// typedef struct GMM_TEXTURE_INFO_REC GMM_TEXTURE_INFO;
#define GMM_TEXTURE_INFO struct GMM_TEXTURE_INFO_REC


// TODO: Allows UMD to override some parameters. Remove these functions once GMM comprehends UMD usage model and
// can support them properly.
void                GMM_STDCALL GmmResOverrideAllocationSize(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T Size);
void                GMM_STDCALL GmmResOverrideAllocationPitch(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T Pitch);
void                GMM_STDCALL GmmResOverrideAuxAllocationPitch(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T Pitch);
void                GMM_STDCALL GmmResOverrideAllocationFlags(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_FLAG *pFlags);
void                GMM_STDCALL GmmResOverrideAllocationHAlign(GMM_RESOURCE_INFO *pGmmResource, uint32_t HAlign);
void                GMM_STDCALL GmmResOverrideAllocationBaseAlignment(GMM_RESOURCE_INFO *pGmmResource, uint32_t Alignment);
void                GMM_STDCALL GmmResOverrideAllocationBaseWidth(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T BaseWidth);
void                GMM_STDCALL GmmResOverrideAllocationBaseHeight(GMM_RESOURCE_INFO *pGmmResource, uint32_t BaseHeight);
void                GMM_STDCALL GmmResOverrideAllocationDepth(GMM_RESOURCE_INFO *pGmmResource, uint32_t Depth);
void                GMM_STDCALL GmmResOverrideResourceTiling(GMM_RESOURCE_INFO *pGmmResource, uint32_t TileMode);
void                GMM_STDCALL GmmResOverrideAuxResourceTiling(GMM_RESOURCE_INFO *pGmmResource, uint32_t TileMode);
void                GMM_STDCALL GmmResOverrideAllocationTextureInfo(GMM_RESOURCE_INFO *pGmmResource, GMM_CLIENT Client, const GMM_TEXTURE_INFO *pTexInfo);
void                GMM_STDCALL GmmResOverrideAllocationFormat(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_FORMAT Format);
void                GMM_STDCALL GmmResOverrideSurfaceType(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_TYPE ResourceType);
void                GMM_STDCALL GmmResOverrideSvmGfxAddress(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_ADDRESS SvmGfxAddress);
void                GMM_STDCALL GmmResOverrideAllocationArraySize(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArraySize);
void                GMM_STDCALL GmmResOverrideAllocationMaxLod(GMM_RESOURCE_INFO *pGmmResource, uint32_t MaxLod);

//////////////////////////////////////////////////////////////////////////////////////
// GmmCachePolicy.c
//////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmCachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo ,
                                                                    GMM_RESOURCE_USAGE_TYPE Usage);
GMM_PTE_CACHE_CONTROL_BITS  GMM_STDCALL GmmCachePolicyGetPteType(GMM_RESOURCE_USAGE_TYPE Usage);
GMM_RESOURCE_USAGE_TYPE     GMM_STDCALL GmmCachePolicyGetResourceUsage(GMM_RESOURCE_INFO *pResInfo );
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmCachePolicyGetOriginalMemoryObject(GMM_RESOURCE_INFO *pResInfo);
uint8_t                     GMM_STDCALL GmmCachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage);
void                        GMM_STDCALL GmmCachePolicyOverrideResourceUsage(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage);
uint32_t                    GMM_STDCALL GmmCachePolicyGetMaxMocsIndex();
uint32_t                    GMM_STDCALL GmmCachePolicyGetMaxL1HdcMocsIndex();
uint32_t                    GMM_STDCALL GmmCachePolicyGetMaxSpecialMocsIndex();


void                        GMM_STDCALL GmmResSetPrivateData(GMM_RESOURCE_INFO *pGmmResource, void *pPrivateData);

#if (!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper functions for UMD clients Translation layer from OLD GMM APIs to New
/// unified GMM Lib APIs
///////////////////////////////////////////////////////////////////////////////////// 
GMM_STATUS              GmmCreateGlobalClientContext(GMM_CLIENT  ClientType);
void                    GmmDestroyGlobalClientContext();
GMM_RESOURCE_INFO*      GmmResCreateThroughClientCtxt(GMM_RESCREATE_PARAMS *pCreateParams);
void                    GmmResFreeThroughClientCtxt(GMM_RESOURCE_INFO *pRes);
GMM_RESOURCE_INFO*      GmmResCopyThroughClientCtxt(GMM_RESOURCE_INFO*  pSrcRes);
void                    GmmResMemcpyThroughClientCtxt(void *pDst, void *pSrc);
#endif

// Hack to define and undefine typedef name to avoid redefinition of the
// typedef.  Part 2.
#undef GMM_TEXTURE_INFO

// Reset packing alignment to project default
#pragma pack(pop)

#ifdef __cplusplus
}
#endif /*__cplusplus*/
