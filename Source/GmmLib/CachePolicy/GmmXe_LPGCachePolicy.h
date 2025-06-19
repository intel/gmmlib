/*==============================================================================
Copyright(c) 2022 Intel Corporation
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

#define _SN 0x1
#define _IA_GPU_SN 0x2
#define _WT 0x2
#define _L1_WB 0x2

// Cache Policy Definition
// L3_SCC       : L3 skip caching control (disabled if L3_SCC = 0)
// GO           : Global observable point for L3-uncached (0=Default is L3, 1= Memory)
// UcLookup     : Snoop L3 for uncached (0=Default is no-snoop, 1 =Snoop)
// L1CC         : L1 cache control (0: WBP write bypass mode, 1: 0 uncached, 2: WB Write back, 3:WT write-through, 4: WS Write-Streaming) : Valid only for DG2+
// L2CC         : 0 : 0, 1: WB
// L4CC         : 0 : UC, 1: wB, 2: WT
// Coherency    : 0= NoSnoop (non coherent) 1: 1 way coherent with IA Snoop (GPU snoop of CPU cache) 2: 2 way coherent IA GPU Snoop  [Snoop is always to LLC from CPU/GPU internal caches]

//******************************************************************************************************************************************************************/
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency)
/*******************************************************************************************************************************************************************/
// KMD Usages
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BATCH_BUFFER                                          , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COMP_FRAME_BUFFER                                     , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SWITCH_BUFFER                                 , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CURSOR                                                , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAY_STATIC_IMG_FOR_SMOOTH_ROTATION_BUFFER         , 1  ,  0   ,      0,   1,        1,      0    ,  _WT      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DUMMY_PAGE                                            , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GDI_SURFACE                                           , 1  ,  0   ,      0,   1,        1,      0    ,  1        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GENERIC_KMD_RESOURCE                                  , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
// GMM_RESOURCE_USAGE_GFX_RING is only used if WaEnableRingHostMapping is enabled.
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GFX_RING                                              , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GTT_TRANSFER_REGION                                   , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HW_CONTEXT                                            , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATE_MANAGER_KERNEL_STATE                            , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_STAGING_SURFACE                                   , 1  ,  0   ,      0,   1,        1,      0    ,  1        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MBM_BUFFER                                            , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_NNDI_BUFFER                                           , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OVERLAY_MBM                                           , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRIMARY_SURFACE                                       , 1  ,  0   ,      0,   1,        1,      0    ,  _WT      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SCREEN_PROTECTION_INTERMEDIATE_SURFACE                , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADOW_SURFACE                                        , 1  ,  0   ,      0,   1,        1,      0    ,  1        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SM_SCRATCH_STATE                                      , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATUS_PAGE                                           , 1  ,  0   ,      0,   1,        1,      0    ,  1        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TIMER_PERF_QUEUE                                      , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNKNOWN                                               , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNMAP_PAGING_RESERVED_GTT_DMA_BUFFER                  , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VSC_BATCH_BUFFER                                      , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WA_BATCH_BUFFER                                       , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_OCA_BUFFER                                        , 0  ,  0   ,      0,   1,        1,      0    ,  0        ,  0     );

//
// 3D Usages
//
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_DEPTH_BUFFER                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_HIZ                                             , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UMD_BATCH_BUFFER                                      , 0  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BINDING_TABLE_POOL                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CCS                                                   , 0  ,  0   ,      1,   0,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONSTANT_BUFFER_POOL                                  , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DEPTH_BUFFER                                          , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAYABLE_RENDER_TARGET                             , 1  ,  0   ,      0,   1,        1,      0    , _WT    ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GATHER_POOL                                           , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_SURFACE_STATE                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_DYNAMIC_STATE                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE_UC                                 , 0  ,  0   ,      0,   1,        1,      0    , 0      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_STATELESS_DATA_PORT                              , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_STATELESS_DATA_PORT_L1_CACHED                    , 1  ,  0   ,      0,   1,        _L1_WB, 0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INDIRECT_OBJECT                                  , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INSTRUCTION                                      , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HIZ                                                   , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER                                          , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER_L3_COHERENT_UC                           , 0  ,  0   ,      0,   1,        1,      0    , 0      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER_L3_CACHED                                , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MCS                                                   , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PUSH_CONSTANT_BUFFER                                  , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PULL_CONSTANT_BUFFER                                  , 1  ,  0   ,      0,   1,        0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_QUERY                                                 , 0  ,  0   ,      0,   1,        1,      0    , 1      , _SN    ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET                                         , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADER_RESOURCE                                       , 1  ,  0   ,      0,   1,        0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STAGING                                               , 0  ,  0   ,      1,   0,        1,      0    , 1      , _SN    );  
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STENCIL_BUFFER                                        , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STREAM_OUTPUT_BUFFER                                  , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILE_POOL                                             , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADER_RESOURCE_LLC_BYPASS                            , 1  ,  0   ,      0,   1,        0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MOCS_62                                               , 0  ,  0   ,      1,   0,        1,      0    , 0      ,  0     ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_L3_EVICTION                                           , 0  ,  0   ,      1,   0,        1,      0    , 0      ,  0     ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_L3_EVICTION_SPECIAL                                   , 0  ,  0   ,      1,   0,        1,      0    , 0      ,  0     );  
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PROCEDURAL_TEXTURE                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );

// Tiled Resource
//
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_DEPTH_BUFFER                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_HIZ                                             , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_MCS                                             , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_CCS                                             , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET                                   , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET_AND_SHADER_RESOURCE               , 1  ,  0   ,      0,   1,        0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_SHADER_RESOURCE                                 , 1  ,  0   ,      0,   1,        0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_UAV                                             , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UAV                                                   , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER                                         , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER_L3_COHERENT_UC                          , 0  ,  0   ,      0,   1,        1,      0    , 1      ,  0     ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER_L3_CACHED                               , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OGL_WSTN_VERTEX_BUFFER                                , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_POSH_VERTEX_BUFFER                                    , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET_AND_SHADER_RESOURCE                     , 1  ,  0   ,      0,   1,        0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WDDM_HISTORY_BUFFER                                   , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SAVE_RESTORE                                  , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PTBR_PAGE_POOL                                        , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PTBR_BATCH_BUFFER                                     , 1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );

//
// CM USAGES
//
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_SurfaceState,                                            1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_L1_Enabled_SurfaceState,                                 1  ,  0   ,      0,   1,        2,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_StateHeap,                                               1  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_L3_SurfaceState,                                      0  ,  0   ,      0,   1,        1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_CACHE_SurfaceState,                                   0  ,  0   ,      1,   0,        1,      0    , 0      ,  0     );

//
// MP USAGES
//
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_BEGIN,                                                   0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT,                                                 0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT_FF,                                              0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT_RCS,                                             0  ,  0   ,      0,   1,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState,                                            1  ,  0   ,      0,   1,        1,      0    ,  1       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState_FF,                                         0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState_RCS,                                        1  ,  0   ,      0,   1,        1,      0    ,  1       ,  0     );
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_END,                                                     0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     ); 

// MHW - SFC
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,                               0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,                               0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,                               0  ,  0   ,      1,   0,        1,      0    ,  0       ,  0     );



/**********************************************************************************/

//
// OCL Usages
//
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER                                            , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CONST                                      , 1 ,  0   ,       0,   1,         0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CSR_UC                                     , 0 ,  0   ,       1,   1,         1,      0    , 0      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CACHELINE_MISALIGNED                       , 0 ,  0   ,       1,   1,         1,      0    , 1      ,  0     ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_IMAGE                                             , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_INLINE_CONST                                      , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_INLINE_CONST_HDC                                  , 1 ,  0   ,       0,   1,         0,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SCRATCH                                           , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRIVATE_MEM                                       , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRINTF_BUFFER                                     , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_STATE_HEAP_BUFFER                                 , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER                              , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER_CACHELINE_MISALIGNED         , 0 ,  0   ,       1,   1,         1,      0    , 1      ,  0     ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_ISH_HEAP_BUFFER                                   , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TAG_MEMORY_BUFFER                                 , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TEXTURE_BUFFER                                    , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SELF_SNOOP_BUFFER                                 , 1 ,  0   ,       0,   1,         1,      0    , 1      ,  0     );
/**********************************************************************************/

// Cross Adapter
//                   USAGE TYPE                                                               , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE                             , 0  ,  0   ,      0,   1,         1,      0    ,  0       ,  0     );
/**********************************************************************************/

// BCS
//                   USAGE TYPE                                                                 , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_BLT_SOURCE                                             , 0  ,  0   ,     1,   0,          1,      0    ,  0       ,  0     );
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_BLT_DESTINATION                                        , 0  ,  0   ,     1,   0,          1,      0    ,  0       ,  0     );

/**********************************************************************************/
//
// MEDIA USAGES
//                   USAGE TYPE                                                         , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency )
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS                             , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
// DECODE
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM                          , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INPUT_REFERENCE                          , 0   , 0    ,     1,     0,      1,      1,      1,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ                            , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_WRITE                           , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE                , 0   , 0    ,     1,     0,      1,      0,      0,         0        ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE              , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE                           , 0   , 0    ,     0,     1,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE                  , 0   , 0    ,     1,     0,      1,      0,      0,        _SN       );  
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE             , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
// ENCODE
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INPUT_RAW                                , 0   , 0    ,     1,     0,      1,      0,      1,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INPUT_RECON                              , 0   , 0    ,     1,     0,      1,      1,      1,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ                            , 0   , 0    ,     1,     0,      1,      0,      1,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE                           , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE                , 0   , 0    ,     1,     0,      1,      0,      1,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE              , 0   , 0    ,     1,     0,      1,      0,      0,         0        );  
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_EXTERNAL_READ                            , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_PICTURE                           , 0   , 0    ,     1,     0,      1,      0,      1,         0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM                         , 0   , 0    ,     1,     0,      1,      0,      1,        _SN       );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_WRITE                  , 0   , 0    ,     1,     0,      1,      0,      0,        _SN       ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_READ_WRITE             , 0   , 0    ,     1,     0,      1,      0,      0,         0        );
// VP
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_FF                             , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_FF                           , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_FF                             , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF                            , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF                       , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF                            , 1   , 0    ,     0,     1,      1,      0,      _WT,     0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER                         , 1   , 0    ,     0,     1,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_RENDER                       , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER                         , 0   , 0    ,     1,     0,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_RENDER                        , 1   , 0    ,     0,     1,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER                   , 1   , 0    ,     0,     1,      1,      0,      1,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_RENDER                        , 1   , 0    ,     0,     1,      1,      0,      _WT,     0        ); 
// CP
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CP_EXTERNAL_READ                                , 0   , 0    ,     1,     0,      1,      0,      0,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CP_INTERNAL_WRITE                               , 0   , 0    ,     1,     0,      1,      0,      0,       0        );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GSC_KMD_RESOURCE                                , 0   , 0    ,     0,     1,      1,      0,      0,       0        );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_NULL_CONTEXT_BB                             , 0  ,  0   ,      0,     1,        1,      0    ,  0,       0        );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COMMAND_STREAMER                                , 0  ,  0   ,      1,     1,        1,      0    ,  0,       0        );

//                   USAGE TYPE                                                        , L3, L3_SCC,     GO, UcLookup,   L1CC,   L2CC,   L4CC,     Coherency)
// Uncacheable copies
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_SOURCE                                     , 0   , 0    ,   1  ,  0   ,       1 ,    0,     0,        0);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_DEST                                       , 0   , 0    ,   1  ,  0   ,       1 ,    0,     0,        0);


#include "GmmCachePolicyUndefineConditionals.h"
