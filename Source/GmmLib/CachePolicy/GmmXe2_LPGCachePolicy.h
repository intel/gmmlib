/*==============================================================================
Copyright(c) 2024 Intel Corporation
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

#define _SN        0x1
#define _IA_GPU_SN 0x2
#define _WT        0x2
#define _L1_WB     0x2
#define dGPU       SKU(FtrDiscrete)

#if (_DEBUG || _RELEASE_INTERNAL)
#define _WA_WB_Emu (WA(Wa_EmuMufasaSupportOnBmg))
#else
#define _WA_WB_Emu 0
#endif

// GmmLib can apply 2Way WA to GMM_RESOURCE_USAGE_HW_CONTEXT.
#define _WA_2W (WA(Wa_14018976079) || WA(Wa_14018984349)) ? 2 : 0
#define _L3_P  ((_WA_2W == 2) ? 1 : 0) // L3 Promotion to WB if 2Way Coh WA is set

// clang-format off
//typedef enum GMM_CACHING_POLICY_REC
//{
//    GMM_UC   = 0x0, //uncached
//    GMM_WB   = 0x1, // Write back
//    GMM_WT   = 0x2, // write-through
//    GMM_WBTD = 0x3, // WB_T_Display
//    GMM_WBTA = 0x4, // WB_T_App
//    GMM_WBP  = 0x5, // write bypass mode
//    GMM_WS   = 0x6, // Write-Streaming
//} GMM_CACHING_POLICY;
//
// typedef enum GMM_COHERENCY_TYPE_REC
//{
//GMM_NON_COHERENT_NO_SNOOP         = 0x0,
//GMM_COHERENT_ONE_WAY_IA_SNOOP     = 0x1,
//GMM_COHERENT_TWO_WAY_IA_GPU_SNOOP = 0x2
//} GMM_COHERENCY_TYPE;
// Cache Policy Definition
// L3_CLOS      : L3 class of service (0,1,2,3)
// IgPAT        : Ignore PAT 1 = Override by MOCS, 0 = Defer to PAT
//Macros for segment-preference
#define NoP                          0x0
//Wa_14018443005
#define COMPRESSED_PAT_WITH_L4WB_L3UC_0 PAT10
#define COMPRESSED_PAT_WITH_L4WB_L3WB_0 PAT14
#define COMPRESSED_PAT_WITH_L4UC_L3UC_0 PAT12
#define COMPRESSED_PAT_WITH_L4UC_L3WB_0 PAT9

#define ISWA_1401844305USAGE(usage)       ((Usage == GMM_RESOURCE_USAGE_BLT_SOURCE) ||      \
                                           (Usage == GMM_RESOURCE_USAGE_BLT_DESTINATION) || \
                                           (Usage == GMM_RESOURCE_USAGE_COPY_SOURCE) ||     \
                                           (Usage == GMM_RESOURCE_USAGE_COPY_DEST))
//******************************************************************************************************************************************************************/
//                   USAGE TYPE                                                               L3_CC, L3_CLOS, L1CC,   L2CC,   L4CC,     Coherency,   IgPAT,  SegOv)
/*******************************************************************************************************************************************************************/
// KMD Usages
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BATCH_BUFFER                                          ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COMP_FRAME_BUFFER                                     ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SWITCH_BUFFER                                 ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CURSOR                                                ,  3,     0,     0,      0    ,  0			,  0     , 0,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAY_STATIC_IMG_FOR_SMOOTH_ROTATION_BUFFER         ,  3,     0,     0,      0    ,  0			,  0     , 0,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DUMMY_PAGE                                            ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GDI_SURFACE                                           ,  1,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GENERIC_KMD_RESOURCE                                  ,  _L3_P,	0,     0,      0    ,  0    		        ,  _WA_2W, 1,    NoP);
// GMM_RESOURCE_USAGE_GFX_RING is only used if WaEnableRingHostMapping is enabled .
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GFX_RING                                              ,  0,     0,     0,      0    ,  0			,  0     , 1,	  NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GTT_TRANSFER_REGION                                   ,  0,     0,     0,      0    ,  0			,  0     , 1,	  NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HW_CONTEXT                                            ,  _L3_P,	0,     0,      0    ,  0			,  _WA_2W, 1,     NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATE_MANAGER_KERNEL_STATE                            ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_STAGING_SURFACE                                   ,  1,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MBM_BUFFER                                            ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_NNDI_BUFFER                                           ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OVERLAY_MBM                                           ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PRIMARY_SURFACE                                       ,  3,     0,     0,      0    ,  0			,  0     , 0,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SCREEN_PROTECTION_INTERMEDIATE_SURFACE                ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADOW_SURFACE                                        ,  1,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SM_SCRATCH_STATE                                      ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STATUS_PAGE                                           ,  1,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TIMER_PERF_QUEUE                                      ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNKNOWN                                               ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UNMAP_PAGING_RESERVED_GTT_DMA_BUFFER                  ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VSC_BATCH_BUFFER                                      ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WA_BATCH_BUFFER                                       ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_OCA_BUFFER                                        ,  0,     0,     0,      0    ,  0			,  0     , 1,    NoP);

//
// 3D Usages
//
//                   USAGE TYPE                                                               L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency , IgPAT)
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_DEPTH_BUFFER                                    ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_HIZ                                             ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UMD_BATCH_BUFFER                                      ,  0,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_BINDING_TABLE_POOL                                    ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CCS                                                   ,  0,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONSTANT_BUFFER_POOL                                  ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DEPTH_BUFFER                                          ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DISPLAYABLE_RENDER_TARGET                             ,  3,     0,     0,      0    , 0		,  0     ,   0,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GATHER_POOL                                           ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_SURFACE_STATE                                    ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_DYNAMIC_STATE                                    ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE                                    ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_GENERAL_STATE_UC                                 ,  0,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_STATELESS_DATA_PORT                              ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_STATELESS_DATA_PORT_L1_CACHED                    ,  1,     0,     1,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INDIRECT_OBJECT                                  ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HEAP_INSTRUCTION                                      ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_HIZ                                                   ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER                                          ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER_L3_COHERENT_UC                           ,  0,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_INDEX_BUFFER_L3_CACHED                                ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MCS                                                   ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PUSH_CONSTANT_BUFFER                                  ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PULL_CONSTANT_BUFFER                                  ,  1,     0,     5,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_QUERY                                                 ,  _WA_WB_Emu,     0,     0,      0    , 0		,  1     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET                                         ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADER_RESOURCE                                       ,  1,     0,     5,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STAGING                                               ,  _WA_WB_Emu,     0,     0,      0    , 0		,  1     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STENCIL_BUFFER                                        ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_STREAM_OUTPUT_BUFFER                                  ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILE_POOL                                             ,  1,     0,     0,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_SHADER_RESOURCE_LLC_BYPASS                            ,  1,     0,     5,      0    , 0		,  0     ,   1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MOCS_62                                               ,  0,     0,     0,      0    , 0		,  0     ,   1,	   NoP); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_L3_EVICTION                                           ,  0,     0,     0,      0    , 0		,  0     ,   1,	   NoP); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_L3_EVICTION_SPECIAL                                   ,  0,     0,     0,      0    , 0		,  0     ,   1,	   NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UMD_OCA_BUFFER                                        ,  0,     0,     0,      0    , 0		,  0     ,   1,	   NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PROCEDURAL_TEXTURE                                    ,  1,     0,     0,      0    , 0		,  0     ,   1,	   NoP);

// Tiled Resource
//
//                   USAGE TYPE                                                               L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency,  IgPAT)
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_DEPTH_BUFFER                                    ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_HIZ                                             ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_MCS                                             ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_CCS                                             ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET                                   ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_RENDER_TARGET_AND_SHADER_RESOURCE               ,  1,     0,     5,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_SHADER_RESOURCE                                 ,  1,     0,     5,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_TILED_UAV                                             ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_UAV                                                   ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER                                         ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER_L3_COHERENT_UC                          ,  0,     0,     0,      0    , 0      ,  0     ,    1,    NoP); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VERTEX_BUFFER_L3_CACHED                               ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OGL_WSTN_VERTEX_BUFFER                                ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_POSH_VERTEX_BUFFER                                    ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_RENDER_TARGET_AND_SHADER_RESOURCE                     ,  1,     0,     5,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_WDDM_HISTORY_BUFFER                                   ,  1,     0,     0,      0    , 0      ,  0     ,    1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CONTEXT_SAVE_RESTORE                                  ,  1,     0,     0,      0    , 0      ,  0     ,    1,	 NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PTBR_PAGE_POOL                                        ,  1,     0,     0,      0    , 0      ,  0     ,    1,	 NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_PTBR_BATCH_BUFFER                                     ,  1,     0,     0,      0    , 0      ,  0     ,    1,	 NoP);

//
// CM USAGES
//
//                   USAGE TYPE                                                                L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency, IgPAT)
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_SurfaceState,                                            1,     0,     0,      0    , 1      ,  0     ,     1,	NoP);
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_L1_Enabled_SurfaceState,                                 1,     0,     1,      0    , 1      ,  0     ,     1,	NoP);
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_StateHeap,                                               1,     0,     0,      0    , 1      ,  0     ,     1,	NoP);
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_L3_SurfaceState,                                      0,     0,     0,      0    , 1      ,  0     ,     1,	NoP);
DEFINE_CACHE_ELEMENT(CM_RESOURCE_USAGE_NO_CACHE_SurfaceState,                                   0,     0,     0,      0    , 0      ,  0     ,     1,	NoP);

//
// MP USAGES
//
//                   USAGE TYPE                                                               L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency,  IgPAT )
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_BEGIN,                                                   0,     0,     0,      0    ,  0       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT,                                                 0,     0,     0,      0    ,  0       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT_FF,                                              0,     0,     0,      0    ,  0       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_DEFAULT_RCS,                                             0,     0,     0,      0    ,  0       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState,                                            1,     0,     0,      0    ,  1       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState_FF,                                         0,     0,     0,      0    ,  0       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_SurfaceState_RCS,                                        1,     0,     0,      0    ,  1       ,  0     ,    1,	NoP);
DEFINE_CACHE_ELEMENT(MP_RESOURCE_USAGE_END,                                                     0,     0,     0,      0    ,  0       ,  0     ,    1,	NoP); 

// MHW - SFC
//                   USAGE TYPE                                                               , L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency, IgPAT)
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,                               0,     0,     0,      0    ,  0       ,  0     ,   1,	  NoP);
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,                               0,     0,     0,      0    ,  0       ,  0     ,   1,	  NoP);
DEFINE_CACHE_ELEMENT(MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,                               0,     0,     0,      0    ,  0       ,  0     ,   1,	  NoP);



/**********************************************************************************/

//
// OCL Usages
//
//                   USAGE TYPE                                                               L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency , IgPAT)
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER                                            ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CONST                                      ,  1,     0,      5,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CSR_UC                                     ,  0,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_BUFFER_CACHELINE_MISALIGNED                       , _L3_P,     0,      0,      0    , 0		,  _WA_2W      ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_IMAGE                                             ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_INLINE_CONST                                      ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_INLINE_CONST_HDC                                  ,  1,     0,      5,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SCRATCH                                           ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRIVATE_MEM                                       ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_PRINTF_BUFFER                                     ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_STATE_HEAP_BUFFER                                 ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER                              ,  1,     0,      0,      0    , 0		,  1       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SYSTEM_MEMORY_BUFFER_CACHELINE_MISALIGNED         , _L3_P,     0,      0,      0    , 0		,  _WA_2W       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_ISH_HEAP_BUFFER                                   ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TAG_MEMORY_BUFFER                                 ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_TEXTURE_BUFFER                                    ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_OCL_SELF_SNOOP_BUFFER                                 ,  1,     0,      0,      0    , 0		,  0       ,  1,    NoP);
/**********************************************************************************/

// Cross Adapter
//                   USAGE TYPE                                                               ,L3_CC, L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency , IgPAT)
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE                             ,  1,     0,     1,      0    ,  0		,  0       , 1, NoP);
/**********************************************************************************/

// BCS
//                   USAGE TYPE                                                                L3_CC,  L3_CLOS, L1CC,   L2CC,   L4CC,     Coherency, IgPAT)
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_BLT_SOURCE                                           ,  0,      0,      0,      0,      0,           0,       1, NoP);
DEFINE_CACHE_ELEMENT( GMM_RESOURCE_USAGE_BLT_DESTINATION                                      ,  0,      0,      0,      0,      0,           0,       1, NoP);

/**********************************************************************************/
//
// MEDIA USAGES
//                   USAGE TYPE                                                         L3_CC,   L3_CLOS,L1CC,   L2CC,   L4CC,     Coherency,	IgPAT )
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS                             ,  0,     0,      0,      0,		0,         0 ,        1,	  NoP	 );
// DECODE
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM                          ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INPUT_REFERENCE                          ,  dGPU,  0,     0,      1,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ                            ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_WRITE                           ,  0,     0,     0,      0,			0,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE                ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE              ,  0,     0,     0,      0,			0,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE                           ,  3,     0,     0,      0,			2,         0  ,        0,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE                  ,  0,     0,     0,      0,			0,         1  ,        1,    NoP    );  
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE             ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP	 );
// ENCODE
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INPUT_RAW                                ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INPUT_RECON                              ,  dGPU,  0,     0,      1,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ                            ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE                           ,  0,     0,     0,      0,			0,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE                ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE              ,  0,     0,     0,      0,			0,         0  ,        1,    NoP    );  
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_EXTERNAL_READ                            ,  0,     0,     0,      0,			0,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_PICTURE                           ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM                         ,  0,     0,     0,      0,			0,         1  ,        1,	  NoP    ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_WRITE                  ,  0,     0,     0,      0,			0,         1  ,        1,	  NoP    ); 
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_READ_WRITE             ,  dGPU,  0,     0,      0,			1,         0  ,        1,	  NoP    );
// VP
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_FF                             ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_FF                           ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_FF                             ,  0,     0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF                            ,  0,     0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF                       ,  dGPU,  0,     0,      0,			1,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF                            ,  3,     0,     0,      0,			2,         0  ,        0,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER                         ,  1,     0,     0,      0,			0,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_RENDER                       ,  1,     0,     0,      0,			0,         0  ,        1,    NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER                         ,  0,     0,     0,      0,			0,         0  ,        1,	  NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_RENDER                        ,  0,     0,     0,      0,			0,         0  ,        1,	  NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER                   ,  1,     0,     0,      0,			0,         0  ,        1,	  NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_RENDER                        ,  3,     0,     0,      0,			0,         0  ,        0,	  NoP    ); 
// CP
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CP_EXTERNAL_READ                                ,  0,     0,     0,      0,			0,         0  ,        1,	  NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_CP_INTERNAL_WRITE                               ,  0,     0,     0,      0,			0,         0  ,        1,	  NoP    );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_GSC_KMD_RESOURCE                                ,  0,     0,     0,      0,			0,         0  ,        1,	  NoP    );

DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_KMD_NULL_CONTEXT_BB                             ,  0,     0,     0,      0    ,		0,         0  ,        1,	  NoP    );
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COMMAND_STREAMER                                ,  0,     0,     0,      0    ,		0,         0  ,        1,	  NoP    );

//                   USAGE TYPE                                                        , L3_CC,   L3_CLOS, L1CC,   L2CC,   L4CC,   Coherency, IgPAT)
// Uncacheable copies
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_SOURCE                                     , 0,         0,     0 ,      0,	     0,       0,        1,	  NoP);
DEFINE_CACHE_ELEMENT(GMM_RESOURCE_USAGE_COPY_DEST                                       , 0,         0,     0 ,      0,      0,       0,        1,	  NoP);

// clang-format on

#undef _WT
#include "GmmCachePolicyUndefineConditionals.h"

