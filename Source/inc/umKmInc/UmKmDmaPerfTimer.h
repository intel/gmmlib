/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2010-2017).
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
** File Name:    UmKmDmaPerfTimer.h
**
** Description:
**      Contains perftag class and subtype codes used by km dma timer.
**
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#pragma once

#if defined (LHDM) || defined(KM_PERF_CONTROLLER_BUILD) || defined(_PERF_REPORT)
    #include "UmKmEnum.h"
#endif

// Set packing alignment
#pragma pack(push, 1)

#ifdef __cplusplus
extern "C" {
#endif


//===========================================================================
// typedef:
//      PERF_DATA
//
// Description:
//
//---------------------------------------------------------------------------
typedef struct _PERF_DATA
{
    union
    {
        struct 
        {
            DWORD dmaBufID   : 16;
            DWORD frameID    :  8;
            DWORD bufferID   :  4;
            DWORD batchBufID :  4;
        };

        DWORD PerfTag;
    };
} PERF_DATA;

//===========================================================================
// enum:
//      PERFTAG_CLASS_ENUM
//
// Description:
//      PerfTag class cmd buffer classification.
//
//---------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// IMPORTANT NOTE: Please DONOT change the existing perftag values in below enum.
// If adding a new perf tag, assign a new value
//--------------------------------------------------------------------------------
typedef enum PERFTAG_CLASS_ENUM
{
    PERFTAG_ALL                = 0x0000,    // Used for capturing all (perfcontroller)
    PERFTAG_KMD                = 0x1000,
    PERFTAG_3D                 = 0x2000,
    PERFTAG_DECODE             = 0x3000,    // Media Decoding
    PERFTAG_DXVA2              = 0x4000,
    PERFTAG_LIBVA              = 0x5000,
    PERFTAG_ENCODE             = 0x6000,    // Media Encoding
    PERFTAG_DXVAHD             = 0x7000,
    PERFTAG_DXVA1              = 0x8000,
    PERFTAG_VPREP              = 0x9000,
    PERFTAG_CM                 = 0xA000,    // C for media
    PERFTAG_WIDI               = 0xB000,
    PERFTAG_OCL                = 0xC000,    // OpenCL
    PERFTAG_RESERVED           = 0xD000,    
    PERFTAG_DXVA11             = 0xE000,    // DX11 Video
    PERFTAG_FRAME_CAPTURE      = 0xF000,    // Gfx Frame Capture
    PERFTAG_FRAME_CAPTURE_NV12 = 0xF100,    // Gfx Frame Capture with NV12 output
    PERFTAG_FRAME_CAPTURE_RGB8 = 0xF200,    // Gfx Frame Capture with RGB8 output
    PERFTAG_UNKNOWN            = 0x0FFF
}PERFTAG_CLASS;

#define PERFTAG_SUBTYPE(PerfTag)            ( PerfTag &  (ULONG)0x00000FFF )    // Bits[0,11]  Usage component specific
#define GET_PERFTAG_CLASS(PerfTag)          ( PerfTag &  (ULONG)0x0000F000 )    // Bits[12,15]
#define PERFTAG_CLASS_AND_SUBTYPE(PerfTag)  ( PerfTag &  (ULONG)0x0000FFFF )    // Bits[0,15]
#define PERFTAG_UNKNOWN_BITS(PerfTag)       ( PerfTag &  (ULONG)0xFFFF0000 )    // Bits[16,31] Usage component specific
#define PERFTAG_FRAMEID(PerfTag)            ( PerfTag &  (ULONG)0x00FF0000 )    // Bits[16,23] Media Specific - frame id
#define PERFTAG_BUFFERID(PerfTag)           ( PerfTag &  (ULONG)0x0F000000 )    // Bits[24,27] Media Specific - buffer id
#define PERFTAG_FRAMEID_SHIFT                 16
#define PERFTAG_BUFFERID_SHIFT                24
#define PERFTAG_BATCHBUFFERID_SHIFT           28

//===========================================================================
// enum:
//      VPHAL_PERFTAG
//
// Description:
//      Video Postprocessing perftag sub type.
// NOTE: 
//      Please follow instructions in below enum when adding new perf tag values.
//      Sub Tags (bits 11:0).
//      When adding new perf tag values, please update 
//      Source\miniport\LHDM\KmPerfTools\PerfController\PerfReportGenerator.h
//      with the string.
//---------------------------------------------------------------------------
typedef enum _VPHAL_PERFTAG
{
    // No pri video
    VPHAL_NONE    = 0x0,
    VPHAL_1LAYER ,
    VPHAL_2LAYERS,
    VPHAL_3LAYERS,
    VPHAL_4LAYERS,
    VPHAL_5LAYERS,
    VPHAL_6LAYERS,
    VPHAL_7LAYERS,
    VPHAL_8LAYERS,
    // ADD NEW TAGS FOR NO PRI-VIDEO CASE HERE

    // pri video present
    VPHAL_PRI     = 0x10,
    VPHAL_PRI_1LAYER,
    VPHAL_PRI_2LAYERS,
    VPHAL_PRI_3LAYERS,
    VPHAL_PRI_4LAYERS,
    VPHAL_PRI_5LAYERS,
    VPHAL_PRI_6LAYERS,
    VPHAL_PRI_7LAYERS,
    VPHAL_PRI_8LAYERS,
    // ADD NEW TAGS FOR PRI-VIDEO CASE HERE

    // video rotation present
    VPHAL_ROT   = 0x20,
    VPHAL_ROT_1LAYER,
    VPHAL_ROT_2LAYERS,
    VPHAL_ROT_3LAYERS,
    VPHAL_ROT_4LAYERS,
    VPHAL_ROT_5LAYERS,
    VPHAL_ROT_6LAYERS,
    VPHAL_ROT_7LAYERS,
    VPHAL_ROT_8LAYERS,

    // PERFTAGs for AdvProc using VEBOX
    VPHAL_PA_DI_PA = 0x100,
    VPHAL_PA_DN_PA,
    VPHAL_PA_DNUV_PA,
    VPHAL_PA_DNDI_PA,
    VPHAL_PA_DI_422CP,
    VPHAL_PA_DN_422CP,
    VPHAL_PA_DNDI_422CP,
    VPHAL_PA_422CP,
    VPHAL_PA_DN_420CP,
    VPHAL_PA_420CP,
    VPHAL_PA_DN_RGB32CP,
    VPHAL_PA_RGB32CP,

    VPHAL_PL_DI_PA,
    VPHAL_PL_DI_422CP,

    VPHAL_NV12_DN_NV12,
    VPHAL_NV12_DNUV_NV12,
    VPHAL_NV12_DNDI_PA,
    VPHAL_NV12_DN_420CP,
    VPHAL_NV12_DN_422CP,
    VPHAL_NV12_DNDI_422CP,
    VPHAL_NV12_420CP,
    VPHAL_NV12_422CP,
    VPHAL_NV12_DN_RGB32CP,
    VPHAL_NV12_RGB32CP,

    VPHAL_PL3_DN_PL3,
    VPHAL_PL3_DNUV_PL3,
    VPHAL_PL3_DNDI_PA,
    VPHAL_PL3_DN_422CP,
    VPHAL_PL3_DNDI_422CP,
    VPHAL_PL3_422CP,

    VPHAL_VEBOX_CRYPTO_BLOCK_COPY,
    VPHAL_VEBOX_UPDATE_DN_STATE,
    VPHAL_VEBOX_ACE,
    VPHAL_VEBOX_FMD_VAR,

    VPHAL_VEBOX_P010,
    VPHAL_VEBOX_P016,
    VPHAL_VEBOX_P210,
    VPHAL_VEBOX_P216,
    VPHAL_VEBOX_Y210,
    VPHAL_VEBOX_Y216,
    VPHAL_VEBOX_Y410,
    VPHAL_VEBOX_Y416,

    // PERFTAGs for AdvProc using Render
    VPHAL_ISTAB_PH1_PLY_PLY = 0x200,
    VPHAL_ISTAB_PH1_PA_PLY,
    VPHAL_ISTAB_PH2_ME_BS_ATOMIC,
    VPHAL_ISTAB_PH3_GMC_BS_ATOMIC,
    VPHAL_ISTAB_PH4_NV12_NV12_BS,
    VPHAL_ISTAB_PH4_PA_PA_BS,

    VPHAL_FRC_COPY,
    VPHAL_FRC_WIDE_SCREEN_DETECTION,
    VPHAL_FRC_PYRAMIDAL_SCALING,
    VPHAL_FRC_MOTION_ESTIMATION_L3,
    VPHAL_FRC_MV_VOTING_L3,
    VPHAL_FRC_MOTION_ESTIMATION_L2,
    VPHAL_FRC_MV_VOTING_L2,
    VPHAL_FRC_MOTION_ESTIMATION_L1,
    VPHAL_FRC_MV_VOTING_L1,
    VPHAL_FRC_GMV,
    VPHAL_FRC_MV_SANITY_CHECK,
    VPHAL_FRC_GRADIENT_Y,
    VPHAL_FRC_GRADIENT_UV,
    VPHAL_FRC_TEMPORAL_DIFF,
    VPHAL_FRC_SPD_MAP,
    VPHAL_FRC_CLEAN_MAP,
    VPHAL_FRC_MOTION_COMP,

    VPHAL_DRDB_NV12_DERING_NV12,
    VPHAL_DRDB_NV12_HDEBLOCK_NV12,
    VPHAL_DRDB_NV12_VDEBLOCK_NV12,

    VPHAL_3P,

    VPHAL_DPROTATION_NV12_NV12,
    VPHAL_DPROTATION_NV12_AVG,
    VPHAL_DPROTATION_NV12_REP,

    VPHAL_LACE_HIST_SUM,
    VPHAL_LACE_STD,
    VPHAL_LACE_PWLF,
    VPHAL_LACE_LUT,

    VPHAL_EU3DLUT,

    // Capture pipe present
    VPHAL_CP = 0x300,
    VPHAL_CP_BAYER8,
    VPHAL_CP_BAYER16,
    VPHAL_CP_LGCA_PH1_CALCPARAMS,
    VPHAL_CP_LGCA_PH2_GEOCORRECTION,

    // Hdr
    VPHAL_HDR_GENERIC = 0x400,
    VPHAL_HDR_1LAYER,
    VPHAL_HDR_2LAYERS,
    VPHAL_HDR_3LAYERS,
    VPHAL_HDR_4LAYERS,
    VPHAL_HDR_5LAYERS,
    VPHAL_HDR_6LAYERS,
    VPHAL_HDR_7LAYERS,
    VPHAL_HDR_8LAYERS,
    VPHAL_HDR_AUTO_PER_FRAME_STATE,

    // Fdfb - FD
    VPHAL_FDFB_FD_DOWNSCALE = 0x500,
    VPHAL_FDFB_FD_MLBP,
    VPHAL_FDFB_FD_CASCADE,
    VPHAL_FDFB_FD_CASCADE1,
    VPHAL_FDFB_FD_DOWNSCALE_NOT_FULLY_ENQUEUE,
    VPHAL_FDFB_FD_MLBP_NOT_FULLY_ENQUEUE,
    VPHAL_FDFB_FD_CASCADE_NOT_FULLY_ENQUEUE,
    VPHAL_FDFB_FD_CASCADE1_NOT_FULLY_ENQUEUE,
    VPHAL_FDFB_FD_MERGE,
    VPHAL_FDFB_FD_MERGE1,

    // Fdfb - FLD
    VPHAL_FDFB_FLD_CAL_GAUSSIAN_WEIGHT = 0x510,
    VPHAL_FDFB_FLD_PREPROCESSING,
    VPHAL_FDFB_FLD_RESIZE,
    VPHAL_FDFB_FLD_EXTRACT_FEATURE_GET_LANDMARK,
    VPHAL_FDFB_FLD_POST_PROCESSING,
    VPHAL_FDFB_FLD_VALIDATOR,
    VPHAL_FDFB_ELD_PREPROCESSING,
    VPHAL_FDFB_ELD_GET_LANDMARK,
    VPHAL_FDFB_ELD_POST_PROCESSING,
    VPHAL_FDFB_ELD_FILTERING,

    // Fdfb - FB
    VPHAL_FDFB_FB_VEBOX_SKIN_MAP = 0x520,
    VPHAL_FDFB_FB_CMK_AVERAGE_FILTER,
    VPHAL_FDFB_FB_CMK_PROCESSING,
    VPHAL_FDFB_FB_SMOOTHER,
    VPHAL_FDFB_FB_SKIN_BLENDER,
    VPHAL_FDFB_FB_CMK_FOUNDATION,
    VPHAL_FDFB_FB_CAL_REGIONS,
    VPHAL_FDFB_FB_CMK_BLUSH_MAP,
    VPHAL_FDFB_FB_CMK_EYE_CIRCLES,
    VPHAL_FDFB_FB_SMOOTHER_SKIN_ROI_BLENDER_0,
    VPHAL_FDFB_FB_SMOOTHER_SKIN_ROI_BLENDER_1,
    VPHAL_FDFB_FB_RED_LIP_CURVE_1,
    VPHAL_FDFB_FB_RED_LIP_CURVE_2,
    VPHAL_FDFB_FB_RED_LIP_CURVE_3,
    VPHAL_FDFB_FB_RED_LIP_CURVE_4,
    VPHAL_FDFB_FB_REFINE_LIP_MASK_1,
    VPHAL_FDFB_FB_REFINE_LIP_MASK_2,
    VPHAL_FDFB_FB_CMK_COLOR_LIP,
    VPHAL_FDFB_FB_CMK_BWD_WARP_SCALING,
    VPHAL_FDFB_FB_CMK_DATA_MOVE,
    VPHAL_FDFB_FB_CMK_BWD_WARP,
    VPHAL_FDFB_FB_MDF_SURFACE_COPY,
    VPHAL_FDFB_FB_RED_LIP,
    VPHAL_FDFB_FB_STD_GEN,
    VPHAL_FDFB_FB_CAL_EYECURVE,
    VPHAL_FDFB_FB_EYE_BRIGHT_PARAM_GEN,
    VPHAL_FDFB_FB_EYE_BRIGHT,
    VPHAL_FDFB_FB_EYE_LASH,
    VPHAL_FDFB_FB_EYE_LINE,
    VPHAL_FDFB_FB_TEETH_WHITEN,
    VPHAL_FDFB_FB_EYE_SAHDOW_MASK,
    VPHAL_FDFB_FB_EYE_SAHDOW,
    VPHAL_FDFB_FB_EYE_COLOR,

    // ADD TAGS FOR NEW ADVPROC KRNS HERE

    VPHAL_PERFTAG_MAX
} VPHAL_PERFTAG, *PVPHAL_PERFTAG;


#ifdef __cplusplus
}
#endif

// Reset packing alignment to project default
#pragma pack(pop)
