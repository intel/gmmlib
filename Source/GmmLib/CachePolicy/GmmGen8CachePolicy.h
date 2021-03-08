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


#include "GmmCachePolicyConditionals.h"
#define ULT (SKU(FtrULT))
#define EDRAM (_ELLC > MB(0))

// [!] On Gen8, according to bpsec TC = 11b means L3, LLC and eLLC cachable.
//     In order for resources to be places in L3, LLC/ELLC/L3 should be set to 1.
//     ELLC = EDRAM indicate resource needed in EDRAM but ELLC = 1 indicate resource needed in L3

// Cache Policy Definition
//********************************************************************************************************************/
//                   USAGE TYPE                                                         , LLC    , ELLC , L3 , WT , AGE )
/*********************************************************************************************************************/

// KMD Usages
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BATCH_BUFFER                                    , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COMP_FRAME_BUFFER                               , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SWITCH_BUFFER                           , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CURSOR                                          , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAY_STATIC_IMG_FOR_SMOOTH_ROTATION_BUFFER   , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DUMMY_PAGE                                      , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GDI_SURFACE                                     , 1      , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GENERIC_KMD_RESOURCE                            , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GFX_RING                                        , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GTT_TRANSFER_REGION                             , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HW_CONTEXT                                      , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATE_MANAGER_KERNEL_STATE                      , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_STAGING_SURFACE                             , 1      , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MBM_BUFFER                                      , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_NNDI_BUFFER                                     , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OVERLAY_MBM                                     , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRIMARY_SURFACE                                 , 1      , 1    , 1  , 1  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SCREEN_PROTECTION_INTERMEDIATE_SURFACE          , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADOW_SURFACE                                  , !ULT   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SM_SCRATCH_STATE                                , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATUS_PAGE                                     , 1      , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TIMER_PERF_QUEUE                                , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNKNOWN                                         , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNMAP_PAGING_RESERVED_GTT_DMA_BUFFER            , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VSC_BATCH_BUFFER                                , 0      , 0    , 0  , 0  , 0 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WA_BATCH_BUFFER                                 , 0      , 0    , 0  , 0  , 0 );

//
// 3D Usages
//
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BINDING_TABLE_POOL                              , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONSTANT_BUFFER_POOL                            , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DEPTH_BUFFER                                    , !EDRAM, EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAYABLE_RENDER_TARGET                       , 0     , EDRAM , 0  , EDRAM  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GATHER_POOL                                     , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_SURFACE_STATE                              , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_DYNAMIC_STATE                              , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE                              , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_STATELESS_DATA_PORT                        , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INDIRECT_OBJECT                            , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INSTRUCTION                                , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HIZ                                             , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER                                    , !EDRAM, EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MCS                                             , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PUSH_CONSTANT_BUFFER                            , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PULL_CONSTANT_BUFFER                            , 1     , 1     , 1  , 0  , 3 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_QUERY                                           , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET                                   , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADER_RESOURCE                                 , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STAGING                                         , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STENCIL_BUFFER                                  , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STREAM_OUTPUT_BUFFER                            , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILE_POOL                                       , 0     , EDRAM , 0  , 0  , 1 );
// Tiled Resource
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_DEPTH_BUFFER                              , !EDRAM, EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_HIZ                                       , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_MCS                                       , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET                             , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET_AND_SHADER_RESOURCE         , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_SHADER_RESOURCE                           , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_UAV                                       , 1     , 1     , 1  , 0  , 1 );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER                                   , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OGL_WSTN_VERTEX_BUFFER                          , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UAV                                             , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET_AND_SHADER_RESOURCE               , 1     , 1     , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WDDM_HISTORY_BUFFER                             , 0     , EDRAM , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SAVE_RESTORE                            , 0     , EDRAM , 0  , 0  , 1 );

//
// CM USAGES
//
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_SurfaceState,                                            1   , 1    , 1  , 0  , 1 );

//
// MP USAGES
//
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_BEGIN,                                                   0   , 0    , 0  , 0  , 0);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT,                                                 0   , 0    , 0  , 0  , 0);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState,                                            1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_No_L3_SurfaceState,                                      1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState,                                  0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_END,                                                     0   , 0    , 0  , 0  , 0);

//Media GMM Resource USAGES
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC                              , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_POST_DEBLOCKING_CODEC                             , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE              , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE              , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STREAMOUT_DATA_CODEC                              , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC               , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC    , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC                           , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MACROBLOCK_STATUS_BUFFER_CODEC                    , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE              , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC                      , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE                , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC                 , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC              , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC                 , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BITPLANE_READ_CODEC                               , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_AACSBIT_VECTOR_CODEC                              , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC                             , 0   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_CURR_ENCODE                               , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_REF_ENCODE                                , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE                    , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST                , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE                      , 1   , 1    , 1  , 0  , 1);
#if defined(__linux__) && !defined(ANDROID)
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE                            , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE                  , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PAK_OBJECT_ENCODE                                 , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE                     , 1   , 1    , 0  , 0  , 1);
#else
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE                            , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE                  , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PAK_OBJECT_ENCODE                                 , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE                     , 1   , 1    , 1  , 0  , 1);
#endif
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE                        , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP8_BLOCK_MODE_COST_ENCODE                        , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP8_MB_MODE_COST_ENCODE                           , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP8_MBENC_OUTPUT_ENCODE                           , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP8_HISTOGRAM_ENCODE                              , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP8_L3_LLC_ENCODE                                 , 1   , 1    , 1  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_MD_CODEC                                      , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_SAO_CODEC                                     , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_MV_CODEC                                      , 1   , 1    , 0  , 0  , 1);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MB_QP_CODEC                               , 1   , 1    , 1  , 0  , 1);
/**********************************************************************************/

//OCL Usages
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER                                    , 1   , 1    , 1  , 0  , 1 );
// This case is used for cases where we have kernels compiled for BTI 253 (non-cohrent) and they are cacheline mis-aligned
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CACHELINE_MISALIGNED               , 1   , 1    , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_IMAGE                                     , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_INLINE_CONST                              , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SCRATCH                                   , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRIVATE_MEM                               , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRINTF_BUFFER                             , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_STATE_HEAP_BUFFER                         , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER                      , 1   , 1    , 1  , 0  , 1 );
// This case is used for cases where we have kernels compiled for BTI 253 (non-cohrent) and they are cacheline mis-aligned
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER_CACHELINE_MISALIGNED , 1   , 1    , 0  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_ISH_HEAP_BUFFER                           , 1   , 1    , 1  , 0  , 1 );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TAG_MEMORY_BUFFER                         , 1   , 1    , 1  , 0  , 1 );
// Sampler overfetch issue is fixed on BDW
/*Project: BDW:B0+
For SURFTYPE_BUFFER, SURFTYPE_1D, and SURFTYPE_2D non-array, non-MSAA, non-mip-mapped surfaces in linear memory,
the only padding requirement is to the next aligned 64-byte boundary beyond the end of the surface.
The rest of the padding requirements documented above do not apply to these surfaces.*/
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TEXTURE_BUFFER                            , 1   , 1    , 1  , 0  , 1 );
// Image from buffer when the image and buffer are on the kernel arguments list
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_IMAGE_FROM_BUFFER                         , 1   , 1    , 0  , 0  , 1 );
/**********************************************************************************/

// Cross Adapter
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE                     , 0   , 0    , 0  , 0  , 0);
/**********************************************************************************/

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CAMERA_CAPTURE                                , CAM$ , 0    , 0  , 0  , CAM$ );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_SOURCE                                   , 0   , 0    , 0  , 0  , 0);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_DEST                                     , 0   , 0    , 0  , 0  , 0);

#include "GmmCachePolicyUndefineConditionals.h"
