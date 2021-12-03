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
============================================================================
**
** File Name:    sharedata.h
**
** Description:  These are data structures that are shared between the KMD
**               and the UMD
**
==============================================================================*/

#ifndef _SHAREDATA_H_
#define _SHAREDATA_H_

#include "../common/gtsysinfo.h"
#include "../common/sku_wa.h"
#include "../common/igfxfmid.h"
#include "UmKmEnum.h"
#include "UmKmDmaPerfTimer.h"
#define ADAPTER_STRING_SIZE 250
#ifdef __cplusplus
extern "C" {
#endif

#define   UMD_KMD_MAX_REGISTRY_PATH_LENGTH         (512)

//===========================================================================
// typedef:
//        DRIVER_VERION_INFO
//
// Description:
//     This structure is used to communicate Driver Build version between
//     KMD and UMD
//
//---------------------------------------------------------------------------
typedef struct __DRIVER_VERSION_INFO
{
    unsigned                        :16;
    unsigned    DriverBuildNumber   :16;
}DRIVER_VERSION_INFO;


// BIT field, '1' (i.e. Set) means cap is supported by KMD
typedef struct __KMD_CAPS_INFO
{
    unsigned    Gamma_Rgb256x3x16             :1;
    unsigned    GDIAcceleration               :1;
    unsigned    OsManagedHwContext            :1;   // wddm1.2+
    unsigned    GraphicsPreemptionGranularity :3;   // wddm1.2+
    unsigned    ComputePreemptionGranularity  :3;   // wddm1.2+
    unsigned    InstrumentationIsEnabled      :1;   // KMD instrumentation state
    unsigned    DriverStoreEnabled            :1;   // wddm2.1+
    unsigned                                  :21;
} KMD_CAPS_INFO;

// Programmatically override overlay caps for testing
typedef struct __KMD_OVERLAY_OVERRIDE
{
    uint32_t            OverrideOverlayCaps         : 1;    // Override request
    uint32_t            RGBOverlay                  : 1;    // RGB overlay override
    uint32_t            YUY2Overlay                 : 1;    // YUY2 overlay override
    uint32_t            Reserved                    :29;
} KMD_OVERLAY_OVERRIDE;

// Overlay caps info needed by WDDM 1.1 Changes
typedef struct __KMD_OVERLAY_CAPS_INFO
{
    union
    {
        struct
        {
            uint32_t            FullRangeRGB        : 1;  // 0x00000001
            uint32_t            LimitedRangeRGB     : 1;  // 0x00000002
            uint32_t            YCbCr_BT601         : 1;  // 0x00000004
            uint32_t            YCbCr_BT709         : 1;  // 0x00000008
            uint32_t            YCbCr_BT601_xvYCC   : 1;  // 0x00000010
            uint32_t            YCbCr_BT709_xvYCC   : 1;  // 0x00000020
            uint32_t            StretchX            : 1;  // 0x00000040
            uint32_t            StretchY            : 1;  // 0x00000080
            uint32_t            Reserved            :24;  // 0xFFFFFF00
        } Caps;
        uint32_t CapsValue;
    };

    KMD_OVERLAY_OVERRIDE    OVOverride;

    uint32_t                MaxOverlayDisplayWidth;
    uint32_t                MaxOverlayDisplayHeight;
    uint8_t                 HWScalerExists;
    uint32_t                MaxHWScalerStride;
} KMD_OVERLAY_CAPS_INFO;

// Frame Rate
typedef struct
{
    uint32_t uiNumerator;
    uint32_t uiDenominator;
} FRAME_RATE;

typedef struct __KM_SYSTEM_INFO
{

    GT_SYSTEM_INFO      SystemInfo;
    // ShadowReg 119 caluclated value required for WA <WaProgramMgsrForCorrectSliceSpecificMmioReads>
    uint32_t            ShadowRegValue;
    uint32_t            ShadowRegValueforL3SpecificRegReads; // Shadow reg value for L3 bank specific MMIO reads.

    uint32_t            GfxDevId;         // DeviceID
}KM_SYSTEM_INFO;



typedef struct _KM_DEFERRED_WAIT_INFO
{
    uint32_t                                   FeatureSupported;
    uint32_t                                   ActiveDisplay;
} KM_DEFERRED_WAIT_INFO;

// struct to hold Adapter's BDF
typedef struct _ADAPTER_BDF_
{
    union
    {
        struct
        {
            uint32_t    Bus         : 8;
            uint32_t    Device      : 8;
            uint32_t    Function    : 8;
            uint32_t    Reserved    : 8;
        };
        uint32_t    Data;
    };
}ADAPTER_BDF;

// Private data structure for D3D callback QueryAdapterInfoCB

//===========================================================================
// typedef:
//        _ADAPTER_INFO
//
// Description:
//     This structure is private data structure that get passed by UMD to
//     KMD during QueryAdapterInfoCB
//
// Note: Structure will be filled by KMD.
//---------------------------------------------------------------------------
#pragma pack (push,1)
typedef struct _ADAPTER_INFO
{
    uint32_t                    KmdVersionInfo;         // Version ID
    DRIVER_VERSION_INFO         DriverVersionInfo;      //
    PLATFORM                    GfxPlatform;            // Chipset Gfx family, product, render core, display core, etc
    SKU_FEATURE_TABLE           SkuTable;               // SKU feature table
    WA_TABLE                    WaTable;                // WA table
    uint32_t                    GfxTimeStampFreq;       // In Hz (No. of clock ticks per second). So timestamp base = 1 / GfxTimeStampFreq
    uint32_t                    GfxCoreFrequency;       // In MHz.
    uint32_t                    FSBFrequency;           // In MHz.
    uint32_t                    MinRenderFreq;          // In MHz.
    uint32_t                    MaxRenderFreq;          // In MHz.
    uint32_t                    PackageTdp;             // TDP Power for the platform (In Watt)
    uint32_t                    MaxFillRate;            // Fillrate with Alphablend (In Pix/Clk)
    uint32_t                    NumberOfEUs;            // Number of EUs in GT

    // NOTE: Name is kept same so that we don't have to change PC_TARGET macro and usage
    //       of that.
    uint32_t                    dwReleaseTarget;        // PC Release Target Information supplied by INF

    // Following member can be use by UMD for optimal use of DMA and
    // command buffer
    uint32_t                    SizeOfDmaBuffer;        // Size of DMA buffer set (In Bytes)
    uint32_t                    PatchLocationListSize;  // Size of Patch Location List (In number of entries)
    uint32_t                    AllocationListSize;     // Size of Patch Location List (In number of entries)
    uint32_t                    SmallPatchLocationListSize;  // Size of Patch Location List for UMD context that needs a small Patch Location List,
                                                             // currently only used by media context (In number of entries)
    uint32_t                    DefaultCmdBufferSize;   // Size of Cmd buffer default location

    // Following Members can be use for any UMD optimization (like Size of vertex buffer to allocate)
    // any cacheline related read/write, etc
    int64_t                     GfxMemorySize;          // Total GFX memory (In MBytes)
    uint32_t                    SystemMemorySize;       // Total System Memory (In MBytes)
    uint32_t                    CacheLineSize;          // Processor CacheLine size
    PROCESSOR_FAMILY            ProcessorFamily;        // Processor Family
    uint8_t                     IsHTSupported;          // Is Hyper Threaded CPU
    uint8_t                     IsMutiCoreCpu;          // Is Multi Core CPU
    uint8_t                     IsVTDSupported;         // Is Chipset VT is supported
    char                        DeviceRegistryPath[UMD_KMD_MAX_REGISTRY_PATH_LENGTH];    // Array that contains the device registry path
    uint32_t                    RegistryPathLength;     // Actual chars (not including any trailing NULL) in the array set by the KMD
    int64_t                     DedicatedVideoMemory;   // Dedicated Video Memory
    int64_t                     SystemSharedMemory;     // System Shared Memory
    int64_t                     SystemVideoMemory;      // SystemVideoMemory
    FRAME_RATE                  OutputFrameRate;        // Output Frame Rate
    FRAME_RATE                  InputFrameRate;         // Input Frame Rate
    KMD_CAPS_INFO               Caps;                   // List of capabilities supported by the KMD
    KMD_OVERLAY_CAPS_INFO       OverlayCaps;            // List of overlay capabilities supported
    GT_SYSTEM_INFO              SystemInfo;             // List of system details
    KM_DEFERRED_WAIT_INFO       DeferredWaitInfo;       // Indicates if DeferredWait feature is enabled and value of active display

#ifdef _WIN32
    ADAPTER_BDF                 stAdapterBDF;           // Adapter BDF
#endif

} ADAPTER_INFO, *PADAPTER_INFO;
#pragma pack (pop)

#define MAX_ENGINE_INSTANCE_PER_CLASS 4

// GEN11 Media Scalability 2.0: context based scheduling
typedef struct MEDIA_CONTEXT_REQUIREMENT_REC
{
    union
    {
        struct
        {
            uint32_t    UsingSFC                   :  1; // Use SFC or not
            uint32_t    HWRestrictedEngine         :  1;
#if (_DEBUG || _RELEASE_INTERNAL  || __KMDULT)
            uint32_t    Reserved                   : 29;
            uint32_t    DebugOverride              :  1; // Debug & validation usage
#else
            uint32_t    Reserved                   : 30;
#endif
        };

        uint32_t    Flags;
    };

    uint32_t      LRCACount;

    // Logical engine instances used by this context; valid only if flag DebugOverride is set.
    uint8_t    EngineInstance[MAX_ENGINE_INSTANCE_PER_CLASS];

} MEDIA_CONTEXT_REQUIREMENT, *PMEDIA_CONTEXT_REQUIREMENT;

// Bit-Struct for Driver's Use of D3DDDI_PATCHLOCATIONLIST.DriverId
typedef union __D3DDDI_PATCHLOCATIONLIST_DRIVERID
{
    struct
    {
        uint32_t UseGlobalGtt               : 1; // Indicates patch is to use global GTT space address (instead of PPGTT space).
        uint32_t HasDecryptBits             : 1;
        uint32_t RenderTargetEnable         : 1; // Indicates this is an output surface that may need to be encrypted
        int32_t  DecryptBitDwordOffset      : 8; // signed offset (in # of DWORDS) from the patch location to where the cmd's decrypt bit is
        uint32_t DecryptBitNumber           : 5; // which bit to set in the dword specified by DecryptBitDwordOffset (0 - 31)
        uint32_t GpGpuCsrBaseAddress        : 1; // this patch location is for the GPGPU Preempt buffer
        uint32_t SurfaceStateBaseAddress    : 1; // Indicates this is patch for SBA.SurfaceStateBaseAddress
        uint32_t PreemptPatchType           : 3; // Contains list of addresses that may need patching due to Preemption/Resubmit.
        uint32_t PatchLowDword              : 1; // 32-bit patch despite 64-bit platform--Low DWORD.
        uint32_t PatchHighDword             : 1; // 32-bit patch despite 64-bit platform--High DWORD.
        uint32_t StateSip                   : 1; // STATE_SIP address that needs to be patched in the context image (for thread-level preemption)
        uint32_t GpGpuWSDIBegin             : 1; // The start of the SDI command seq before GPGPU_WALKER (for restarting thread-level workload)
        uint32_t GpGpuWSDIEnd               : 1; // The end of the SDI command seq before GPGPU_WALKER (for restarting thread-level workload)
        uint32_t NullPatch                  : 1; // Don't perform KMD patching (used for SVM, Tiled/Sparse Resources and ExistingSysMem Virtual Padding).
        uint32_t UpperBoundsPatch           : 1; // Indicates the patch is for an UpperBounds/"out-of-bounds" address
        uint32_t BindingTablePoolPatch      : 1; // Indicates the patch is for Binding Table Pool. Needed for bindless head testing. Remove later.
    };
    uint32_t Value;
} D3DDDI_PATCHLOCATIONLIST_DRIVERID;

#ifdef __cplusplus
}
#endif
#endif // _SHAREDATA_H_
