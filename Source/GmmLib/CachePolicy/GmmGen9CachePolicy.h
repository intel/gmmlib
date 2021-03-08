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

// true if edram is present
#define EDRAM (_ELLC > MB(0))

// true if edram is present but not 128 MB (GT4e)
#define GT3e (_ELLC > MB(0) && _ELLC < MB(128))

// true if edram is 128 MB
#define GT4e (_ELLC == MB(128))

#define KBL_GT3e  (PRODUCT(KABYLAKE) && GT3e)

//eDRAM caching of displayables not supported on certain SKL CPUs
#define DISP_IN_EDRAM    (EDRAM && !pGmmGlobalContext->GetWaTable().WaDisableEdramForDisplayRT)

//eDRAM-Only caching, for a usage that might be encrypted, must use ENCRYPTED_PARTIALS_EDRAM
#define ENCRYPTED_PARTIALS_EDRAM (DISP_IN_EDRAM && !pGmmGlobalContext->GetWaTable().WaEncryptedEdramOnlyPartials)

//eDRAM-only caching of unencrypted flip chains on SKL GT4
#define UNENCRYPTED_RT_EDRAM (DISP_IN_EDRAM && (!pGmmGlobalContext->GetWaTable().WaEncryptedEdramOnlyPartials || !GT3e))

// for SKL 3e we generally want EDRAM_ONLY mode (LLC=0,ELLC=1) = (!GT3e, EDRAM)
// for SKL 4e we generally want "both" mode (LLC=1,ELLC=1) = (!GT3e, EDRAM)

// i915 only supports three GEN9 MOCS entires:
//     MOCS[0]...LLC=0, ELLC=0, L3=0, AGE=0
//     MOCS[1]...<N/A for GmmLib Purposes>
//     MOCS[2]...LLC=1, ELLC=1, L3=1, AGE=3
#define UC 0
#define WB 2

//***************************************************************************************************************/
//                   USAGE TYPE                                                               , LLC , ELLC , L3 , AGE , i915)
/****************************************************************************************************************/

// KMD Usages
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BATCH_BUFFER                                          , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COMP_FRAME_BUFFER                                     , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SWITCH_BUFFER                                 , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CURSOR                                                , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAY_STATIC_IMG_FOR_SMOOTH_ROTATION_BUFFER         , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DUMMY_PAGE                                            , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GDI_SURFACE                                           , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GENERIC_KMD_RESOURCE                                  , 0   , 0    , 0  , 0 , UC );
// GMM_RESOURCE_USAGE_GFX_RING is only used if WaEnableRingHostMapping is enabled.
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GFX_RING                                              , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GTT_TRANSFER_REGION                                   , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HW_CONTEXT                                            , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATE_MANAGER_KERNEL_STATE                            , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_STAGING_SURFACE                                   , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MBM_BUFFER                                            , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_NNDI_BUFFER                                           , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OVERLAY_MBM                                           , 0   , DISP_IN_EDRAM, 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRIMARY_SURFACE                                       , 0   , DISP_IN_EDRAM, 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SCREEN_PROTECTION_INTERMEDIATE_SURFACE                , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADOW_SURFACE                                        , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SM_SCRATCH_STATE                                      , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATUS_PAGE                                           , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TIMER_PERF_QUEUE                                      , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNKNOWN                                               , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNMAP_PAGING_RESERVED_GTT_DMA_BUFFER                  , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VSC_BATCH_BUFFER                                      , 0   , 0    , 0  , 0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WA_BATCH_BUFFER                                       , 0   , 0    , 0  , 0 , UC );

//
// 3D Usages
//
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UMD_BATCH_BUFFER                                      , 0      , 0     , 0  ,0 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BINDING_TABLE_POOL                                    , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CCS                                                   , 0      , EDRAM , 1  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONSTANT_BUFFER_POOL                                  , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DEPTH_BUFFER                                          , !GT3e  , EDRAM , 0  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAYABLE_RENDER_TARGET                             , 0      , ENCRYPTED_PARTIALS_EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GEN9_UNENCRYPTED_DISPLAYABLE                          , 0      , UNENCRYPTED_RT_EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GATHER_POOL                                           , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_SURFACE_STATE                                    , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_DYNAMIC_STATE                                    , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE                                    , !GT3e  , EDRAM , 1  ,3 , WB );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE_UC                                 , 0      , 0     , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_STATELESS_DATA_PORT                              , 1      , 1     , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INDIRECT_OBJECT                                  , 1      , 1     , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INSTRUCTION                                      , 1      , 1     , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HIZ                                                   , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER                                          , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MCS                                                   , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PUSH_CONSTANT_BUFFER                                  , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PULL_CONSTANT_BUFFER                                  , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_QUERY                                                 , !GT3e  , EDRAM , 0  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET                                         , !GT3e  , EDRAM , 0  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADER_RESOURCE                                       , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STAGING                                               , !GT3e  , EDRAM , 0  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STENCIL_BUFFER                                        , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STREAM_OUTPUT_BUFFER                                  , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILE_POOL                                             , !GT3e  , EDRAM , 1  ,3 , WB );

// Tiled Resource
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_DEPTH_BUFFER                                    , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_HIZ                                             , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_MCS                                             , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_CCS                                             , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET                                   , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET_AND_SHADER_RESOURCE               , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_SHADER_RESOURCE                                 , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_UAV                                             , !GT3e  , EDRAM , 1  ,3 , WB );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UAV                                                   , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER                                         , !GT3e  , EDRAM , 0  ,1 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OGL_WSTN_VERTEX_BUFFER                                , !GT3e,   EDRAM , 0  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET_AND_SHADER_RESOURCE                     , !GT3e  , EDRAM , 1  ,3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET_AND_SHADER_RESOURCE_PARTIALENCSURFACES  , !GT3e  , ENCRYPTED_PARTIALS_EDRAM, 1, 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WDDM_HISTORY_BUFFER                                   , 0      , EDRAM , 0  ,3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SAVE_RESTORE                                  , !GT3e  , EDRAM , 1  ,3 , WB );

//
// CM USAGES
//
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_SurfaceState                                          , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_L3_SurfaceState                                    , 1   , 1    , 0  , 3 , WB );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_LLC_ELLC_SurfaceState                              , 0   , 0    , 1  , 3 , UC );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_LLC_SurfaceState                                   , 0   , 1    , 1  , 3 , UC );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_ELLC_SurfaceState                                  , 1   , 0    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_LLC_L3_SurfaceState                                , 0   , 1    , 0  , 3 , UC );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_ELLC_L3_SurfaceState                               , 1   , 0    , 0  , 3 , WB );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_CACHE_SurfaceState                                 , 0   , 0    , 0  , 3 , UC );

//
// MP USAGES
//
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_BEGIN,                                                   0   , 0    , 0  , 0, UC );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT,                                                 0   , 0    , 0  , 0, UC );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState,                                            1   , EDRAM    , 1  , 1, WB);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_AGE3_SurfaceState,                                       1   , EDRAM    , 1  , 3, WB);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_No_L3_SurfaceState,                                      1   , EDRAM    , 0  , 1, WB);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState,                                  0   , EDRAM    , 0  , 1, UC);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_No_LLC_L3_AGE_SurfaceState,                              0   , EDRAM    , 0  , 0, UC);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_No_LLC_eLLC_L3_AGE_SurfaceState,                         0   ,     0    , 0  , 0, UC);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_PartialEnc_No_LLC_L3_AGE_SurfaceState,                   0   , ENCRYPTED_PARTIALS_EDRAM, 0  , 0, UC);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_END,                                                     0   , EDRAM    , 0  , 0, UC );

// MHW - SFC
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,                               0   , EDRAM    , 0  , 0, UC );
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface_PartialEncSurface,             0   , ENCRYPTED_PARTIALS_EDRAM, 0, 0, UC );
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,                               1   , EDRAM    , 1  , 1, WB );
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,                               1   , EDRAM    , 1  , 1, WB );

//Media GMM Resource USAGES
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC                                  , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC_PARTIALENCSURFACE                , 0   , ENCRYPTED_PARTIALS_EDRAM, 0, 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_POST_DEBLOCKING_CODEC                                 , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE                  , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE                  , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STREAMOUT_DATA_CODEC                                  , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC                   , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC        , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC                               , 1   , EDRAM    , 0  , 1, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MACROBLOCK_STATUS_BUFFER_CODEC                        , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE                  , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC                          , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE                    , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC                     , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC                  , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC                     , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BITPLANE_READ_CODEC                                   , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_AACSBIT_VECTOR_CODEC                                  , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC                                 , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_CURR_ENCODE                                   , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_REF_ENCODE                                    , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE                                , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE                        , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST                    , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE                          , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE                      , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PAK_OBJECT_ENCODE                                     , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE                         , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE                            , 1   , EDRAM    , 1  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC                          , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VDENC_STREAMIN_CODEC                                  , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SURFACE_MB_QP_CODEC                                   , 1   , EDRAM    , 1  , 3, WB );


DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_MD_CODEC                                          , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_SAO_CODEC                                         , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_MV_CODEC                                          , 1   , EDRAM    , 0  , 3, WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC                                , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC                          , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC                          , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC                           , 0   , EDRAM    , 0  , 3, UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC                         , 0   , EDRAM    , 0  , 3, UC );


/**********************************************************************************/

//
// OCL Usages
//
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER                                            , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CSR_UC                                     , 0   , 0    , 0  , 3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CACHELINE_MISALIGNED                       , 1   , 1    , 0  , 3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_IMAGE                                             , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_INLINE_CONST                                      , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SCRATCH                                           , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRIVATE_MEM                                       , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRINTF_BUFFER                                     , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_STATE_HEAP_BUFFER                                 , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER                              , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER_CACHELINE_MISALIGNED         , 1   , 1    , 0  , 3 , UC );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_ISH_HEAP_BUFFER                                   , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TAG_MEMORY_BUFFER                                 , 1   , 1    , 1  , 3 , WB );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TEXTURE_BUFFER                                    , 1   , 1    , 1  , 3 , WB );
// Image from buffer when the image and buffer are on the kernel arguments list
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_OCL_IMAGE_FROM_BUFFER                                , 1   , 1    , 0  , 3 , WB );
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_OCL_BUFFER_NO_LLC_CACHING                            , 0   , 1    , 1  , 3 , UC );
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_OCL_IMAGE_NO_LLC_CACHING                             , 0   , 1    , 1  , 3 , UC );
/**********************************************************************************/

// Cross Adapter
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE                             , 0   , 0    , 0  , 0, UC );
/**********************************************************************************/

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CAMERA_CAPTURE                                        , CAM$, 0    , 0  , CAM$ , WB );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_SOURCE                                           , 0   , 0    , 0  , 0  , UC);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_DEST                                             , 0   , 0    , 0  , 0  , UC);

#undef UC
#undef WB
#include "GmmCachePolicyUndefineConditionals.h"
