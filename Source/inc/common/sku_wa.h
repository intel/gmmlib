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
/*
File Name: sku_wa.h

Description:
    Common hardware sku and workaround information structures.

    This file is commented in a manner allowing automated parsing of WA information.
    Each entry inside WA table should include comments in form of:
        @WorkaroundName           <name-mandatory> //this field is mandatory
        @Description              <short description (can be multiline)
        @PerfImpact               <performance impact>
        @BugType                  <hang,crash etc.>
        @Component                <Gmm>

\*****************************************************************************/
#ifndef __SKU_WA_H__
#define __SKU_WA_H__

// Prevent the following...
// warning: ISO C++ prohibits anonymous structs [-pedantic]
// warning: ISO C90 doesn't support unnamed structs/unions [-pedantic]
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic" // clang only recognizes -Wpedantic
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#if __GNUC__ >= 6
#pragma GCC diagnostic ignored "-Wpedantic"
#else
#pragma GCC diagnostic ignored "-pedantic" // gcc <= 4.7.4 only recognizes -pedantic
#endif
#endif

//********************************** SKU ****************************************

// Sku Table structure to abstract sku based hw feature availability
// For any Sku based feature, add a field in this structure

typedef struct _SKU_FEATURE_TABLE
{
    // flags 1 = available, 0 = not available
    struct //_sku_Core
    {
        unsigned int   FtrULT       : 1;  // Indicates ULT SKU
        unsigned int   FtrVERing    : 1;  // Separate Ring for VideoEnhancement commands
        unsigned int   FtrVcs2      : 1;  // Second VCS engine supported on Gen8 to Gen10 (in some configurations);
        unsigned int   FtrLCIA      : 1;  // Indicates Atom (Low Cost Intel Architecture)
        unsigned int   FtrCCSRing : 1; // To indicate if CCS hardware ring support is present.
        unsigned int   FtrCCSNode : 1; // To indicate if CCS Node support is present.
        unsigned int   FtrTileY     : 1;  // Identifies Legacy tiles TileY/Yf/Ys on the platform
        unsigned int   FtrCCSMultiInstance : 1; // To indicate if driver supports MultiContext mode on RCS and more than 1 CCS.
    };


    struct //_sku_KMD_render
    {                                                               // MI commends are capable to set

        unsigned int   FtrPPGTT                         : 1;  // Per-Process GTT
        unsigned int   FtrIA32eGfxPTEs                  : 1;  // GTT/PPGTT's use 64-bit IA-32e PTE format.
        unsigned int   FtrMemTypeMocsDeferPAT           : 1;  // Pre-Gen12 MOCS can defers to PAT,  e.g. eLLC Target Cache for MOCS
        unsigned int   FtrPml4Support                   : 1;  // PML4-based gfx page tables are supported (in addition to PD-based tables).
        unsigned int   FtrSVM                           : 1;  // Shared Virtual Memory (i.e. support for SVM buffers which can be accessed by both the CPU and GPU at numerically equivalent addresses.)
        unsigned int   FtrTileMappedResource            : 1;  // Tiled Resource support aka Sparse Textures.
        unsigned int   FtrTranslationTable              : 1;  // Translation Table support for Tiled Resources.
        unsigned int   FtrUserModeTranslationTable      : 1;  // User mode managed Translation Table support for Tiled Resources.
        unsigned int   FtrNullPages                     : 1;  // Support for PTE-based Null pages for Sparse/Tiled Resources).
        unsigned int   FtrEDram                         : 1;  // embedded DRAM enable
	unsigned int   FtrLLCBypass                     : 1;  // Partial tunneling of UC memory traffic via CCF (LLC Bypass)
        unsigned int   FtrCrystalwell                   : 1;  // Crystalwell Sku
        unsigned int   FtrCentralCachePolicy            : 1;  // Centralized Cache Policy
        unsigned int   FtrWddm2GpuMmu                   : 1;  // WDDMv2 GpuMmu Model (Set in platform SKU files, but disabled by GMM as appropriate for given system.)
        unsigned int   FtrWddm2Svm                      : 1;  // WDDMv2 SVM Model (Set in platform SKU files, but disabled by GMM as appropriate for given system.)
        unsigned int   FtrStandardMipTailFormat         : 1;  // Dx Standard MipTail Format for TileYf/Ys
        unsigned int   FtrWddm2_1_64kbPages             : 1;  // WDDMv2.1 64KB page support
        unsigned int   FtrE2ECompression                : 1;  // E2E Compression ie Aux Table support
        unsigned int   FtrLinearCCS                     : 1;  // Linear Aux surface is supported
        unsigned int   FtrFrameBufferLLC                : 1;  // Displayable Frame buffers cached in LLC
        unsigned int   FtrDriverFLR                     : 1;  // Enable Function Level Reset (Gen11+)
        unsigned int   FtrLocalMemory                   : 1;
        unsigned int   FtrCameraCaptureCaching          : 1;
        unsigned int   FtrLocalMemoryAllows4KB          : 1;
        unsigned int   FtrPpgtt64KBWalkOptimization     : 1;  // XeHP 64KB Page table walk optimization on PPGTT.
        unsigned int   FtrFlatPhysCCS                   : 1;  // XeHP compression ie flat physical CCS
        unsigned int   FtrDisplayXTiling                : 1;  // Fallback to Legacy TileX Display, used for Pre-SI platforms.
        unsigned int   FtrMultiTileArch                 : 1;
	unsigned int   FtrDisplayPageTables             : 1;  // Display Page Tables: 2-Level Page walk for Displayable Frame buffers in GGTT.
        unsigned int   Ftr57bGPUAddressing              : 1;  // 57b GPUVA support eg: PVC
	unsigned int   FtrUnified3DMediaCompressionFormats : 1; // DG2 has unified Render/media compression(versus TGLLP/XeHP_SDV 's multiple instances) and requires changes to RC format h/w encodings.
        unsigned int   FtrForceTile4                    : 1;  // Flag to force Tile4 usage as default in Tile64 supported platforms.
        unsigned int   FtrTile64Optimization            : 1;
    };


    struct //_sku_3d
    {
        unsigned int   FtrAstcLdr2D : 1;  // ASTC 2D LDR Mode Support (mutually exclusive from other ASTC Ftr's)
        unsigned int   FtrAstcHdr2D : 1;  // ASTC 2D HDR Mode Support (mutually exclusive from other ASTC Ftr's)
        unsigned int   FtrAstc3D : 1;  // ASTC 3D LDR/HDR Mode Support (mutually exclusive from other ASTC Ftr's)
    };

    struct //_sku_PwrCons
    {
        //FBC
        unsigned int   FtrFbc : 1;  // Frame Buffer Compression

    };

    struct //_sku_Display
    {
        unsigned int   FtrRendComp : 1; // For Render Compression Feature on Gen9+
        unsigned int   FtrDisplayYTiling : 1; // For Y Tile Feature on Gen9+
        unsigned int   FtrDisplayDisabled : 1;  // Server skus with Display

	};

    struct
    {
        unsigned int   FtrS3D : 1;  // Stereoscopic 3D
        unsigned int   FtrDisplayEngineS3d : 1;  // Display Engine Stereoscopic 3D
    };

    struct // Virtualization features
    {
        unsigned int    FtrVgt : 1;
    };
    struct // For MultiTileArch, KMD reports default tile assignment to UMD-GmmLib - via __KmQueryDriverPrivateInfo
    {
        unsigned int FtrAssignedGpuTile : 3;  // Indicates Gpu Tile number assigned to a process for Naive apps.
    };

} SKU_FEATURE_TABLE, *PSKU_FEATURE_TABLE;

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

//********************************** WA ****************************************

#define WA_DECLARE( wa, wa_comment, wa_bugType, wa_impact, wa_component) unsigned int wa : 1;

enum WA_BUG_TYPE
{
    WA_BUG_TYPE_UNKNOWN = 0,
    WA_BUG_TYPE_CORRUPTION = 1,
    WA_BUG_TYPE_HANG = 2,
    WA_BUG_TYPE_PERF = 4,
    WA_BUG_TYPE_FUNCTIONAL = 8,
    WA_BUG_TYPE_SPEC = 16,
    WA_BUG_TYPE_FAIL = 32
};

#define WA_BUG_PERF_IMPACT(f) f
#define WA_BUG_PERF_IMPACT_UNKNOWN -1

enum WA_COMPONENT
{
    WA_COMPONENT_UNKNOWN = 0,
    WA_COMPONENT_GMM = 0x1,
    WA_COMPONENT_MEDIA = 0x2,
    WA_COMPONENT_OCL = 0x3,
};

// Workaround Table structure to abstract WA based on hw and rev id
typedef struct _WA_TABLE
{
        // struct wa_3d
        unsigned int : 0;

        WA_DECLARE(
        WaAlignIndexBuffer,
        "Force the end of the index buffer to be cacheline-aligned to work around a hardware bug that performs no bounds checking on accesses past the end of the index buffer when it only partially fills a cacheline.",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        // struct _wa_Gmm
        unsigned int : 0;

        WA_DECLARE(
        WaValign2ForR8G8B8UINTFormat,
        "sampler format decoding error in HW for this particular format double fetching is happening, WA is to use VALIGN_2 instead of VALIGN_4",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT(0), WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaValign2For96bppFormats,
        "VALIGN_2 only for 96bpp formats.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaCursor16K,
        "Cursor memory need to be mapped in GTT",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        Wa8kAlignforAsyncFlip,
        "Enable 8k pitch alignment for Asynchronous Flips in rotated mode. (!) Unconventional use! When used, set each XP mode-change (not in platform WA file)!",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        Wa29BitDisplayAddrLimit,
        "Sprite/Overlay/Display addresses limited to 29 bits (512MB)",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaAlignContextImage,
        "WA for context alignment",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaForceGlobalGTT,
        "WA for cmds requiring memory address to come from global GTT, not PPGTT.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaReportPerfCountForceGlobalGTT,
        "WA for MI_REPORT_PERF_COUNT cmd requiring memory address to come from global GTT, not PPGTT.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaOaAddressTranslation,
        "WA for STDW and PIPE_CONTROL cmd requiring memory address to come from global GTT, not PPGTT.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        Wa2RowVerticalAlignment,
        "WA to set VALIGN of sample and rt buffers.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaPpgttAliasGlobalGttSpace,
        "Disallow independent PPGTT space--i.e. the PPGTT must simply alias global GTT space. (N/A without FtrPageDirectory set.)",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaClearFenceRegistersAtDriverInit,
        "WA to clear all fence registers at driver init time.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaRestrictPitch128KB,
        "Restrict max surface pitch to 128KB.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaAvoidLLC,
        "Avoid LLC use. (Intended for debug purposes only.)",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaAvoidL3,
        "Avoid L3 use (but don't reconfigure; and naturally URB/etc. still need L3). (Intended for debug purposes only.)",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        Wa16TileFencesOnly,
        "Limit to 16 tiling fences --Set at run-time by GMM.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        Wa16MBOABufferAlignment,
        "WA align the base address of the OA buffer to 16mb",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaTranslationTableUnavailable,
        "WA for BXT and SKL skus without Tiled-Resource Translation-Table (TR-TT)",
        WA_BUG_TYPE_SPEC,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaNoMinimizedTrivialSurfacePadding,
        "(Not actual HW WA.) On BDW:B0+ trivial surfaces (single-LOD, non-arrayed, non-MSAA, 1D/2D/Buffers) are exempt from the samplers large padding requirements. This WA identifies platforms that dont yet support that.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaNoBufferSamplerPadding,
        "Client agreeing to take responsibility for flushing L3 after sampling/etc.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaSurfaceStatePlanarYOffsetAlignBy2,
        "WA to align SURFACE_STATE Y Offset for UV Plane by 2",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaGttCachingOffByDefault,
        "WA to enable the caching if off by defaultboth at driver init and Resume",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaTouchAllSvmMemory,
        "When in WDDM2 / SVM mode, all VA memory buffers/surfaces/etc need to be touched to ensure proper PTE mapping",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaIOBAddressMustBeValidInHwContext,
        "IndirectObjectBase address (of SBA cmd) in HW Context needs to be valid because it gets used every Context load",
        WA_BUG_TYPE_HANG,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaFlushTlbAfterCpuGgttWrites,
        "WA to flush TLB after CPU GTT writes because TLB entry invalidations on GTT writes use wrong address for look-up",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaMsaa8xTileYDepthPitchAlignment,
        "WA to use 256B pitch alignment for MSAA 8x + TileY depth surfaces.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaDisableNullPageAsDummy,
        "WA to disable use of NULL bit in dummy PTE",
        WA_BUG_TYPE_HANG | WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaUseVAlign16OnTileXYBpp816,
        "WA to make VAlign = 16, when bpp == 8 or 16 for both TileX and TileY on BDW",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaNoMocsEllcOnly,
        "WA to get eLLC Target Cache for MOCS surfaces, when MOCS defers to PAT",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaGttPat0,
        "GTT accesses hardwired to PAT0",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaGttPat0WB,
        "WA to set WB cache for GTT accessess on PAT0",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaMemTypeIsMaxOfPatAndMocs,
        "WA to set PAT.MT = UC. Since TGLLP uses MAX function to resolve PAT vs MOCS MemType So unless PTE.PAT says UC, MOCS won't be able to set UC!",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT, WA_COMPONENT_GMM)
		
        WA_DECLARE(
        WaGttPat0GttWbOverOsIommuEllcOnly,
        "WA to set PAT0 to full cacheable (LLC+eLLC) for GTT access over eLLC only usage for OS based SVM",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaAddDummyPageForDisplayPrefetch,
        "WA to add dummy page row after display surfaces to avoid issues with display pre-fetch",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaLLCCachingUnsupported,
        "There is no H/w support for LLC in VLV or VLV Plus",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaEncryptedEdramOnlyPartials,
        "Disable Edram only caching for encrypted usage",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaDisableEdramForDisplayRT,
        "WA to disable EDRAM cacheability of Displayable Render Targets on SKL Steppings until I0",
        WA_BUG_TYPE_PERF,
        WA_BUG_PERF_IMPACT, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaAstcCorruptionForOddCompressedBlockSizeX,
        "Enable CHV D0+ WA for ASTC HW bug: sampling from mip levels 2+ returns wrong texels. WA adds XOffset to mip2+, requires D0 HW ECO fix.",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaLosslessCompressionSurfaceStride,
        "WA to align surface stride for unified aux surfaces",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        Wa4kAlignUVOffsetNV12LinearSurface,
        "WA to align UV plane offset at 4k page for NV12 Linear FlipChain surfaces",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaFbcLinearSurfaceStride,
        "WA to align surface stride for linear primary surfaces",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaDoubleFastClearWidthAlignment,
        "For all HSW GT3 skus and for all HSW GT E0+ skus, must double the width alignment when performing fast clears.",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaCompressedResourceRequiresConstVA21,
        "3D and Media compressed resources should not have addresses that change within bit range [20:0]",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaDisregardPlatformChecks,
        "Disable plarform checks to surface allocation.",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaAlignYUVResourceToLCU,
        "source and recon surfaces need to be aligned to the LCU size",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        Wa32bppTileY2DColorNoHAlign4,
        "Wa to defeature HALIGN_4 for 2D 32bpp RT surfaces, due to bug introduced from daprsc changes to help RCPB generate correct offsets to deal with cam match",
        WA_BUG_TYPE_CORRUPTION,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaAuxTable16KGranular,
        "AuxTable map granularity changed to 16K ",
        WA_BUG_TYPE_PERF,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

	WA_DECLARE(
        WaAuxTable64KGranular,
        "AuxTable map granularity changed to 64K ..Remove once Neo switches reference to WaAuxTable16KGranular",
        WA_BUG_TYPE_PERF,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

        WA_DECLARE(
        WaLimit128BMediaCompr,
        "WA to limit media decompression on Render pipe to 128B (2CLs) 4:n.",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaUntypedBufferCompression,
        "WA to allow untyped raw buffer AuxTable mapping",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

	WA_DECLARE(
        Wa64kbMappingAt2mbGranularity,
        "WA to force 2MB alignment for 64KB-LMEM pages",
        WA_BUG_TYPE_FUNCTIONAL,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        WaDefaultTile4,
        "[XeHP] Keep Tile4 as default on XeHP till B stepping",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

	WA_DECLARE(
        Wa_1606955757,
        "[GPSSCLT] [XeHP] Multicontext (LB) : out-of-order write-read access to scratch space from hdctlbunit",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_OGL)

	WA_DECLARE(
        WaTile64Optimization,
        "Tile64 wastge a lot of memory so WA provides optimization to fall back to Tile4 when waste is relatively higher",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        Wa_15010089951,
        "[DG2][Silicon][Perf]DG2 VESFC performance when Compression feature is enabled.",
        WA_BUG_TYPE_PERF,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_GMM)

        WA_DECLARE(
        Wa_22016140776,
        "[PVC] operation unexpectedly results in NAN",
        WA_BUG_TYPE_UNKNOWN,
        WA_BUG_PERF_IMPACT_UNKNOWN, WA_COMPONENT_UNKNOWN)

} WA_TABLE, *PWA_TABLE;

//********************************** SKU/WA Macros *************************************

#if (defined(__MINIPORT) || defined(__KCH) || defined(__SOFTBIOS) || defined(__GRM) || defined(__PWRCONS))
#if LHDM || LINUX
#define GFX_IS_SKU(s, f) ((s)->SkuTable.f)
#define GFX_IS_WA(s, w)  ((s)->WaTable.w)
#define GFX_WRITE_WA(x, y, z) ((x)->WaTable.y = z)
//No checking is done in the GFX_WRITE_SKU macro that z actually fits into y.
//  It is up to the user to know the size of y and to pass in z accordingly.
#define GFX_WRITE_SKU(x, y, z) ((x)->SkuTable.y = z)
#else
#define GFX_IS_SKU(h, f) (((PHW_DEVICE_EXTENSION)(h))->pHWStatusPage->pSkuTable->f)
#define GFX_IS_WA(h, w) (((PHW_DEVICE_EXTENSION)(h))->pHWStatusPage->pWaTable->w)
#define GFX_WRITE_WA(x, y, z) (((HW_DEVICE_EXTENSION *)(x))->pHWStatusPage->pWaTable->y = z)
//No checking is done in the GFX_WRITE_SKU macro that z actually fits into y.
//  It is up to the user to know the size of y and to pass in z accordingly.
#define GFX_WRITE_SKU(x, y, z) (((HW_DEVICE_EXTENSION *)(x))->pHWStatusPage->pSkuTable->y = z)
#endif // end LHDM
#else
#define GFX_IS_SKU(s, f) ((s)->SkuTable.f)
#define GFX_IS_WA(s, w)  ((s)->WaTable.w)
#endif
#define GRAPHICS_IS_SKU(s, f) ((s)->f)
#define GRAPHICS_IS_WA(s, w)  ((s)->w)

#endif //__SKU_WA_H__
