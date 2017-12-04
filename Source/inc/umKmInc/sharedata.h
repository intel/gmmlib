/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
**
** Copyright (c) Intel Corporation (2011-2013).
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
** File Name:    sharedata.h
**
** Description:  These are data structures that are shared between the KMD
**               and the UMD
**++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _SHAREDATA_H_
#define _SHAREDATA_H_

#ifdef _WIN32
#include "../BuildRequestType.h"
#endif
#include "../common/gtsysinfo.h"
#include "../common/sku_wa.h"
#include "../../install/intcver.h"
#include "../common/igfxfmid.h"
#include "UmKmEnum.h"
#include "UmKmDmaPerfTimer.h"
#include "../common/kmDafEnabler.h"
#define ADAPTER_STRING_SIZE 250
#ifdef __cplusplus
extern "C" {
#endif

// Command Buffer Rev ID (i.e. Command buffer doc Rev (0.60)]
//     Value is used to detect Command Buffer mis Match between UMD and KMD
//     (Note we only have 16 bits for Rev ID) so value of rev ID is kept below 64K
#define   UMD_KMD_COMMAND_BUFFER_REV_ID         (0x0060)
#define   UMD_KMD_MAX_REGISTRY_PATH_LENGTH         (512)

//===========================================================================
// enum:
//      KM_GRAPHICS_PREEMPTION_GRANULARITY_ENUM
//
// Description:
//      Graphics preemption caps
//
//---------------------------------------------------------------------------
typedef enum KM_GRAPHICS_PREEMPTION_GRANULARITY_ENUM
{
    KM_GRAPHICS_PREEMPTION_NONE                = 0,
    KM_GRAPHICS_PREEMPTION_DMA_BUFFER_BOUNDARY = 1,
    KM_GRAPHICS_PREEMPTION_PRIMITIVE_BOUNDARY  = 2,
    KM_GRAPHICS_PREEMPTION_TRIANGLE_BOUNDARY   = 3,
    KM_GRAPHICS_PREEMPTION_PIXEL_BOUNDARY      = 4,
    KM_GRAPHICS_PREEMPTION_SHADER_BOUNDARY     = 5,
} KM_GRAPHICS_PREEMPTION_GRANULARITY;

//===========================================================================
// enum:
//      KM_COMPUTE_PREEMPTION_GRANULARITY_ENUM
//
// Description:
//      Compute preemption caps
//
//---------------------------------------------------------------------------
typedef enum KM_COMPUTE_PREEMPTION_GRANULARITY_ENUM
{
    KM_COMPUTE_PREEMPTION_NONE                     = 0,
    KM_COMPUTE_PREEMPTION_DMA_BUFFER_BOUNDARY      = 1,
    KM_COMPUTE_PREEMPTION_DISPATCH_BOUNDARY        = 2,
    KM_COMPUTE_PREEMPTION_THREAD_GROUP_BOUNDARY    = 3,
    KM_COMPUTE_PREEMPTION_THREAD_BOUNDARY          = 4,
    KM_COMPUTE_PREEMPTION_SHADER_BOUNDARY          = 5,
} KM_COMPUTE_PREEMPTION_GRANULARITY;

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
    UINT            OverrideOverlayCaps         : 1;    // Override request
    UINT            RGBOverlay                  : 1;    // RGB overlay override
    UINT            YUY2Overlay                 : 1;    // YUY2 overlay override
    UINT            Reserved                    :29;
} KMD_OVERLAY_OVERRIDE;

// Overlay caps info needed by WDDM 1.1 Changes
typedef struct __KMD_OVERLAY_CAPS_INFO
{
    union
    {
        struct
        {
            UINT            FullRangeRGB        : 1;  // 0x00000001
            UINT            LimitedRangeRGB     : 1;  // 0x00000002
            UINT            YCbCr_BT601         : 1;  // 0x00000004
            UINT            YCbCr_BT709         : 1;  // 0x00000008
            UINT            YCbCr_BT601_xvYCC   : 1;  // 0x00000010
            UINT            YCbCr_BT709_xvYCC   : 1;  // 0x00000020
            UINT            StretchX            : 1;  // 0x00000040
            UINT            StretchY            : 1;  // 0x00000080
            UINT            Reserved            :24;  // 0xFFFFFF00
        } Caps;
        UINT CapsValue;
    };

    KMD_OVERLAY_OVERRIDE    OVOverride;

    UINT                    MaxOverlayDisplayWidth;
    UINT                    MaxOverlayDisplayHeight;
    BOOLEAN                 HWScalerExists;
    ULONG                   MaxHWScalerStride;
} KMD_OVERLAY_CAPS_INFO;

// Frame Rate
typedef struct
{
    UINT uiNumerator;
    UINT uiDenominator;
} FRAME_RATE;

typedef struct __KM_SYSTEM_INFO
{

    GT_SYSTEM_INFO      SystemInfo;
    // ShadowReg 119 caluclated value required for WA <WaProgramMgsrForCorrectSliceSpecificMmioReads>
    ULONG               ShadowRegValue;
    ULONG               ShadowRegValueforL3SpecificRegReads; // Shadow reg value for L3 bank specific MMIO reads.

    ULONG               GfxDevId;         // DeviceID
}KM_SYSTEM_INFO;



typedef struct _KM_DEFERRED_WAIT_INFO
{
    ULONG                                   FeatureSupported;
    ULONG                                   ActiveDisplay;
} KM_DEFERRED_WAIT_INFO;


#if(LHDM)
    // WDDM2+ Gfx Space Partitioning...
    typedef struct __GMM_GFX_PARTITIONING
    {
        struct 
        {
            UINT64      Base, Limit; // In raw form--not canonical.
        }           Standard,     // Standard Gfx Space (N/A for FtrWddm2Svm--Just call pfnLock2Cb.)
                    Standard64KB, // Standard Gfx Space for 64KB pages (N/A for FtrWddm2Svm--Just call pfnLock2Cb.) 
                    Kmd,          // TODO(KMD_ALLOCS_IN_GPU_VA): Remove when possible.
                    InternalGpuVA,// For system-level resources allocation by OS eg: monitored fence
                    SVM,          // Address-Equivalent SVM Space (N/A for FtrWddm2Svm--Just call pfnLock2Cb.)
                    TR,           // "Sparse/Tiled Resource"/TR/TR-TT Space
                    Heap32[4];    // For 32-bit-offset-limited GeneralState/DynamicState/Instruction/etc allocs.
                                  // UMD owns array size.
                                  // UMD will override for Heap32 partition locations for FtrWddm2Svm.
    } GMM_GFX_PARTITIONING;
#endif


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
    UINT                        KmdVersionInfo;         // Version ID
    DRIVER_VERSION_INFO         DriverVersionInfo;      //
    PLATFORM                    GfxPlatform;            // Chipset Gfx family, product, render core, display core, etc
    SKU_FEATURE_TABLE           SkuTable;               // SKU feature table
    WA_TABLE                    WaTable;                // WA table
    UINT                        GfxTimeStampFreq;       // In Hz (No. of clock ticks per second). So timestamp base = 1 / GfxTimeStampFreq
    UINT                        GfxCoreFrequency;       // In MHz.
    UINT                        FSBFrequency;           // In MHz.
    UINT                        MinRenderFreq;          // In MHz.
    UINT                        MaxRenderFreq;          // In MHz.
    ULONG                       PackageTdp;             // TDP Power for the platform (In Watt)
    UINT                        MaxFillRate;            // Fillrate with Alphablend (In Pix/Clk)
    UINT                        NumberOfEUs;            // Number of EUs in GT

    // NOTE: Name is kept same so that we don't have to change PC_TARGET macro and usage
    //       of that.
    UINT                        dwReleaseTarget;        // PC Release Target Information supplied by INF

    // Following member can be use by UMD for optimal use of DMA and
    // command buffer
    UINT                        SizeOfDmaBuffer;        // Size of DMA buffer set (In Bytes)
    UINT                        PatchLocationListSize;  // Size of Patch Location List (In number of entries)
    UINT                        AllocationListSize;     // Size of Patch Location List (In number of entries)
    UINT                        SmallPatchLocationListSize;  // Size of Patch Location List for UMD context that needs a small Patch Location List,
                                                             // currently only used by media context (In number of entries)
    UINT                        DefaultCmdBufferSize;   // Size of Cmd buffer default location

    // Following Members can be use for any UMD optimization (like Size of vertex buffer to allocate)
    // any cacheline related read/write, etc
    INT64                       GfxMemorySize;          // Total GFX memory (In MBytes)
    UINT                        SystemMemorySize;       // Total System Memory (In MBytes)
    UINT                        CacheLineSize;          // Processor CacheLine size
    PROCESSOR_FAMILY            ProcessorFamily;        // Processor Family
    BOOLEAN                     IsHTSupported;          // Is Hyper Threaded CPU
    BOOLEAN                     IsMutiCoreCpu;          // Is Multi Core CPU
    BOOLEAN                     IsVTDSupported;         // Is Chipset VT is supported
    char                        DeviceRegistryPath[UMD_KMD_MAX_REGISTRY_PATH_LENGTH];    // Array that contains the device registry path
    UINT                        RegistryPathLength;     // Actual chars (not including any trailing NULL) in the array set by the KMD
    INT64                       DedicatedVideoMemory;   // Dedicated Video Memory
    INT64                       SystemSharedMemory;     // System Shared Memory
    INT64                       SystemVideoMemory;      // SystemVideoMemory
    FRAME_RATE                  OutputFrameRate;        // Output Frame Rate
    FRAME_RATE                  InputFrameRate;         // Input Frame Rate
#ifdef _WIN32
    wchar_t                     wcAdapterString[ADAPTER_STRING_SIZE];   //Adapter String
#endif
    KMD_CAPS_INFO               Caps;                   // List of capabilities supported by the KMD
    KMD_OVERLAY_CAPS_INFO       OverlayCaps;            // List of overlay capabilities supported
    GT_SYSTEM_INFO              SystemInfo;             // List of system details
    KM_DEFERRED_WAIT_INFO       DeferredWaitInfo;       // Indicates if DeferredWait feature is enabled and value of active display

#if(LHDM)
    union {
        HANDLE                  hRTAdapter;             // D3D10DDIARG_OPENADAPTER.hRTAdapter.handle (Needed to call EscapeCb.)
        char                    __Padding[8];

    };

    GMM_GFX_PARTITIONING        GfxPartition;           // WDDM2+ Gfx Space Partitioning
#endif

} ADAPTER_INFO, *PADAPTER_INFO;
#pragma pack (pop)

#define MAX_ENGINE_INSTANCE_PER_CLASS 4

typedef struct _CREATECONTEXT_PVTDATA
{
    union
    {
        DWORD*      pHwContextId;
        UINT64      Reserved;           //64bit padding for 32bit apps running on 64bit OS
    };
    DWORD           NumberOfHwContextIds;   // Available memory for HW Context Ids.

    DWORD           ProcessID;
    BOOLEAN         IsProtectedProcess;
    BOOLEAN         IsDwm;
    BOOLEAN         IsForcedGdiContext; // This flag is set for GDI testing in Aero Off mode.
    BOOLEAN         IsMediaUsage;       // Indicate the context is for media usage
    BOOLEAN         EnableInstPerCtxtBb;    // This flag indicates if the instrumentation needs per context batch buffer to be enabled.
                                            // This per context BB needs to be run everytime this context gets loaded to the HW.
    DWORD          IsPoshSupported;    // Individual bit for every H/W context (max: NumberOfHwContextIds) to indicate if UMD requires POSH support for that context. Should be set only for 3D contexts.
#ifdef DXGKDDI_INTERFACE_VERSION_WDDM2_0
    BOOLEAN         GpuVAContext;  // 1 if CreateContextVirtualCb is used to create the context.
    BOOLEAN         RTManagesResidency; // 1 if UMD doesn't manage residency
    DWORD           NumberOfGpuContexts; //UMD Specifies how many GPU contexts need to be created for this device handle.
    BOOLEAN         IsRealTimePriority; // 1 if context is created from real time priority D3D12 command queue (run GT at higher frequency)
#endif
} CREATECONTEXT_PVTDATA;

// Bit-Struct for Driver's Use of D3DDDI_PATCHLOCATIONLIST.DriverId
typedef union __D3DDDI_PATCHLOCATIONLIST_DRIVERID
{
    struct
    {
        UINT UseGlobalGtt               : 1; // Indicates patch is to use global GTT space address (instead of PPGTT space).
        UINT HasDecryptBits             : 1; 
        UINT RenderTargetEnable         : 1; // Indicates this is an output surface that may need to be encrypted
        INT  DecryptBitDwordOffset      : 8; // signed offset (in # of DWORDS) from the patch location to where the cmd's decrypt bit is
        UINT DecryptBitNumber           : 5; // which bit to set in the dword specified by DecryptBitDwordOffset (0 - 31)
        UINT GpGpuCsrBaseAddress        : 1; // this patch location is for the GPGPU Preempt buffer
        UINT SurfaceStateBaseAddress    : 1; // Indicates this is patch for SBA.SurfaceStateBaseAddress
        UINT PreemptPatchType           : 3; // Contains list of addresses that may need patching due to Preemption/Resubmit.
        UINT PatchLowDword              : 1; // 32-bit patch despite 64-bit platform--Low DWORD.
        UINT PatchHighDword             : 1; // 32-bit patch despite 64-bit platform--High DWORD.
        UINT StateSip                   : 1; // STATE_SIP address that needs to be patched in the context image (for thread-level preemption)
        UINT GpGpuWSDIBegin             : 1; // The start of the SDI command seq before GPGPU_WALKER (for restarting thread-level workload)
        UINT GpGpuWSDIEnd               : 1; // The end of the SDI command seq before GPGPU_WALKER (for restarting thread-level workload)
        UINT NullPatch                  : 1; // Don't perform KMD patching (used for SVM, Tiled/Sparse Resources and ExistingSysMem Virtual Padding).
        UINT UpperBoundsPatch           : 1; // Indicates the patch is for an UpperBounds/"out-of-bounds" address
        UINT BindingTablePoolPatch      : 1; // Indicates the patch is for Binding Table Pool. Needed for bindless head testing. Remove later.
    };
    UINT Value;
} D3DDDI_PATCHLOCATIONLIST_DRIVERID;

//===========================================================================
// enum:
//      PREEMPT_PATCH_TYPE
//
// Description:
//      Address types for Batch Buffer Preemption/Restoration
//
//---------------------------------------------------------------------------
typedef enum _PREEMPT_PATCH_TYPE
{
    PREEMPT_PATCH_TYPE_NONE				= 0,  //This patch is NOT for a SBA cmd field
    PREEMPT_PATCH_TYPE_GSB				= 1,  //General State Base
    PREEMPT_PATCH_TYPE_SSB				= 2,  //Surface State Base
	PREEMPT_PATCH_TYPE_DSB				= 3,  //Dynamic State Base
	PREEMPT_PATCH_TYPE_DSUB				= 4,  //Dynamic State Upper Bound
    PREEMPT_PATCH_TYPE_ISB				= 5,  //Instruction State Base
    PREEMPT_PATCH_TYPE_MAX
} PREEMPT_PATCH_TYPE;

// Ensure that the PREEMPT_PATCH_TYPE_MAX value does not exceed 7 since this value is stored
// in PreemptDwordOffset of size 3 bits.
#ifdef _WIN32
    C_ASSERT( PREEMPT_PATCH_TYPE_MAX <= 7 );
#endif

#define GPGPUW_SDI_BEGIN_NUM_NOOPS       (5) //Number of noops that must precede GpGpuKernelddressPatchingSDIs (Must be equal to DW in MI_STORE_DATA_IMM)
#define GPGPUW_SDI_END_NUM_NOOPS         (1) //Number of noops that must follow GpGpuKernelddressPatchingSDIs (Must be equal to DW in BATCH_BUFFER_END)


#define NUM_SURFACESTATE_BASEADDRESS_PATCHLOCATIONS (8)

#ifdef _WIN32
    #ifdef __MINIPORT // Only check in miniport since other proj's have inclusion issues with it...
        C_ASSERT(sizeof(D3DDDI_PATCHLOCATIONLIST_DRIVERID) == sizeof(((D3DDDI_PATCHLOCATIONLIST *) 0)->DriverId));
    #endif
#endif

//*****************************************************************************
//  PRIVATE_DRIVER_DATA
//*****************************************************************************
typedef struct PRESENT_PRIVATE_DRIVER_DATA_REC
{
    DWORD   RotationAngle;
}PRESENT_PRIVATE_DRIVER_DATA;

#pragma pack (push,1)

typedef struct RESERVED_WORKLOAD_REQUIREMENT_REC
{
   DWORD  Reserved[11];

} RESERVED_WORKLOAD_REQUIREMENT, *PRESERVED_WORKLOAD_REQUIREMENT;

#ifdef _WIN32
    C_ASSERT( sizeof( MEDIA_WORKLOAD_REQUIREMENT ) == 11 * sizeof( DWORD ) );
#endif
//===========================================================================
// typedef:
//        COMMAND_BUFFER_HEADER
//
// Description:
//     This structure is private data structure that get passed by UMD to
//     KMD during RenderCb
//
// Note: Structure will be filled by UMD.
//---------------------------------------------------------------------------
typedef struct COMMAND_BUFFER_HEADER_REC
{
    // DW 0
    // Common
    DWORD   DriverBuildNumber                   : 16;
    DWORD   UmdContextType                      :  4;   // Identifies current work load type.
    DWORD   UmdPatchList                        :  1;   // UMD generates patch list
    DWORD   DeferWaitForFlip                    :  1;
    DWORD   MSAAReallocateWayMasks              :  1;   // application is rendering to MSAA surface. This bit of information is used to enable
                                                        // an optimization for parts with eDRAM.  When running fullscreeen MSAA workloads,
                                                        // eDRAM way masks will be re-allocated to only be used by GT.  KMPresent will execute
                                                        // this behavior based off of the flip type.
    DWORD   PlatformOverride                    :  1;   // Indicates that Cmd Buffer is sent for a different platform than the actual H/W
    DWORD   KmDafCaptureStart                   :  1;   // i.e. Begin capture with this submission.
    DWORD   KmDafCaptureStop                    :  1;   // i.e. End capture after this submission.
    DWORD                                       :  6;

    // DW 1
    DWORD   IndirectHeapSize                    : 10;   // in KB
    DWORD   IndirectHeapSizeUsed                : 10;   // in KB

    // NOTE: the number of bits for this field should allow for
    // the max number of slices as defined in KM_SLICE_STATE_ENUM
    DWORD   UmdRequestedSliceState              :  3;   // UMD requesting specific slice state (number of slices - 0 means use default)
    DWORD   UmdRequestForPowerGatingValid       :  1;	// CHV, GEN9+
    DWORD   UmdRequestedSubsliceCount           :  3;   // CHV, GEN9+
    DWORD   UmdRequestedEUCount                 :  5;   // CHV, GEN9+

    // DW 2
    // 3D\GPGPU
    DWORD   IndirectCommandBuffer               :  1;   // Copy command buffer and patch list from UMD pointers
    DWORD   UsesResourceStreamer                :  1;   // Batch Buffer uses Resource Streamer (HSW+)
    DWORD   NeedsMidBatchPreEmptionSupport      :  1;   // UMD wants preemption support on this batch buffer (i.e. the BB has embedded MI_ARB_CHECK cmds)
    DWORD   Uses3DPipeline                      :  1;   // Batch buffer uses 3d pipeline
    DWORD   UsesMediaPipeline                   :  1;   // Batch buffer uses Media pipeline
    DWORD   UsesGPGPUPipeline                   :  1;   // Batch buffer uses GPGPU pipeline
    DWORD   RequiresCoherency                   :  1;
	DWORD   IsMDFLoad							:  1;	// Identifies whether request comes from MDF or other Media Components
    DWORD	KeepIndirectStatePointers           :  1;   // Indicates whether IndirectStatePointersDisable should be set (SKL+ 3D UMD request based on PushConstantCulling enabling status)
    DWORD   PoshEnable                          :  1;   // Indicates if 'POSH Enable' needs to be set in BB START for this command buffer.
    DWORD   PoshStart                           :  1;   // Indicates if 'POSH Start' needs to be set in BB START for this command buffer.
    DWORD                                       :  2;

    // Media\DXVA
    DWORD   HDDVDTurboMode                      :  1;   // 1 - turbo mode, 0 - normal mode.
    DWORD   ExecuteClearBuffer                  :  1;   // 1 - execute CB2, 0 - do not execute CB2
    DWORD   SendMediaReset                      :  1;   // 1 - send render/media reset, 0 - don't send it
    DWORD   UseHybridMfxOnHAS                   :  1;   // Identifies whether to use hybrid MFX parser
    DWORD   ProtectedMemoryEnable               :  1;

    // Overlay
    DWORD   FlipOverlay                         :  1;
    DWORD   EnableOverlay                       :  1;
    DWORD   UpdateOverlay                       :  1;

    // Debug controls to stall scheduler before/after submission
    DWORD   BreakBeforeSubmit                   :   1;
    DWORD   BreakAfterSubmit                    :   1;
    DWORD                                       :   9;

    struct
    {   // DW 3
        DWORD Enable;
        struct
        {   //DW 4, DW 5
            union
            {
                struct
                {
                    DWORD Index;
                    DWORD Offset;
                }Allocation;
                struct
                {
                    DWORD LowPart;
                    DWORD HighPart;
                }GfxAddress;
            }Address;
            //DW 6
            DWORD Value;
        }Tag;
    }MediaFrameTracking;

    // DW 7
    DWORD   PerfTag;

    // Remove the below two DWORDS that has been temporarily added to test the bindless heap.
    // DW 8
    DWORD   BindlessHeapSize;       

    //DW 9
    ULONG       NumUmdCpuCmdBufPtrs; // number of CPU cmd buffer ptrs in the UmdCpuCmdBufHeader array

    // DW 10-11
    UINT64        UmdCpuCmdBufHeaderPtrs; // Array of COMMAND_LIST_HEADER_ELEMENT's, NumUmdCpuCmdBufPtrs in array

    // DW 12-13
    UINT64   MonitorFenceVA;                     // WDDM 2.0, GpuVa for monitor fence, Use pipe control to write Monitor fence after BB
    // DW 14-15
    UINT64   MonitorFenceValue;     // WDDM 2.0, Value of monitor fence tag, Use pipe control to write Monitor fence after BB
    // DW 16
    DWORD    HwContextIDToUse;      //UMD specifies which HW context to use.

    // DW 17
    DWORD    DisplayID;

    // DW 18-28,
    RESERVED_WORKLOAD_REQUIREMENT   ReservedWorkloadRequirement;

    // DW 29
    DWORD   QWAligned_Reserved; // force QWORD alignment

#ifdef KM_DEBUG_ASSISTANCE_FUNCTIONALITY
    // DW 30-31
    UINT64  KMDAFDXAllocationList;

    // DW 32
    ULONG   KMDAFDXAllocationListSize;

    // DW 33
    DWORD   QWAligned_Reserved1; // force QWORD alignment
#endif

} COMMAND_BUFFER_HEADER, *PCOMMAND_BUFFER_HEADER;
#pragma pack (pop)

#ifdef KM_DEBUG_ASSISTANCE_FUNCTIONALITY
    #define COMMAND_BUFFER_HEADER_MORESIZE_1 4 * sizeof(DWORD)
#else
    #define COMMAND_BUFFER_HEADER_MORESIZE_1 0 * sizeof(DWORD)
#endif

#ifdef _WIN32
    C_ASSERT( sizeof( COMMAND_BUFFER_HEADER ) == 30 * sizeof( DWORD ) + COMMAND_BUFFER_HEADER_MORESIZE_1 );
#endif
//===========================================================================
// MACRO:
//        COMMAND_BUFFER_INITIALIZE
//
// Description:
//     This MACRO allows umd components to initialize their COMMAND_BUFFER_HEADER
//     without worrying about changes to the structure.
//
// PARAMS: VARNAME     = Variable Name to be used
//         CONTEXTTYPE = UmdContextType         UMD_DMA_TYPE
//         PATCHLIST   = UmdPatchList           BOOL
//         STREAMER    = UseResourceStreamer    BOOL
//         PERFTAG     = PerfTag                PERFTAG_CLASS
//---------------------------------------------------------------------------
#define DECLARE_COMMAND_BUFFER(VARNAME, CONTEXTTYPE, PATCHLIST, STREAMER, PERFTAG)  \
            static const COMMAND_BUFFER_HEADER VARNAME = {                  \
            BUILD_NUMBER,        /* DriverBuildNumber                */     \
            CONTEXTTYPE,         /* UmdContextType                   */     \
            PATCHLIST,           /* UmdPatchList                     */     \
            FALSE,               /* Deferred Wait                    */     \
            FALSE,               /* MSAAReallocateWayMasks           */     \
            FALSE,               /* PlatformOverride                 */     \
            FALSE,               /* KmDafCaptureStart                */     \
            FALSE,               /* KmDafCaptureStop                 */     \
            0,                   /* IndirectHeapSize                 */     \
            0,                   /* IndirectHeapSizeUsed             */     \
            0,                   /* UmdRequestedSliceState           */     \
            0,                   /* UmdRequestForPowerGatingValid    */     \
            0,                   /* UmdRequestedSubsliceCount        */     \
            0,                   /* UmdRequestedEUCount              */     \
            FALSE,               /* IndirectCommandBuffer            */     \
            STREAMER,            /* UsesResourceStreamer             */     \
            FALSE,               /* NeedsMidBatchPreEmptionSupport   */     \
            FALSE,               /* Uses3DPipeline                   */     \
            FALSE,               /* UsesMediaPipeline                */     \
            FALSE,               /* UsesGPGPUPipeline                */     \
            FALSE,               /* RequiresCoherency                */     \
            FALSE,               /* IsMDFLoad                        */     \
            FALSE,               /* KeepIndirectStatePointers        */     \
            FALSE,               /* PoshEnable                       */     \
            FALSE,               /* PoshStart                        */     \
            0,                   /* HDDVDTurboMode                   */     \
            FALSE,               /* ExecuteClearBuffer               */     \
            FALSE,               /* SendMediaReset                   */     \
            FALSE,               /* UseHybridMfxOnHAS                */     \
            FALSE,               /* ProtectedMemoryEnable            */     \
            FALSE,               /* FlipOverlay                      */     \
            FALSE,               /* EnableOverlay                    */     \
            FALSE,               /* UpdateOverlay                    */     \
            FALSE,               /* BreakBeforeSubmi                 */     \
            FALSE,               /* BreakAfterSubmit                 */     \
            FALSE,               /* MediaFrameTracking -Enable       */     \
            0,                   /* MediaFrameTrackingTagAddressLow  */     \
            0,                   /* MediaFrameTrackingTagAddressHigh */     \
            0,                   /* MediaFrameTag                    */     \
            PERFTAG,             /* PerfTag                          */     \
            0,                   /* BindlessHeapSize                 */     \
            0,                   /* NumUmdCpuCmdBufPtrs              */     \
            0,                   /* UmdCpuCmdBufHeaderPtrs           */     \
            0,                   /* MonitorFenceVA                   */     \
            0,                   /* MonitorFenceValue                */     \
            0,                   /* HwContextIDToUse                 */     \
            0,                   /* DisplayID                        */     \
            0                    /* ReservedWorkloadRequirement      */     \
    };

#define INDIRECT_HEAP_SIZE_UNITS    (1024)

typedef struct COMMAND_BUFFER_INDIRECT_HEADER_REC
{
    UINT64  UmdCommandBuffer;
    UINT64  UmdPatchLocationList;
} COMMAND_BUFFER_INDIRECT_HEADER;

typedef struct COMMAND_BUFFER_BLOCK_TOKEN_REC
{
    DWORD   Header;
    DWORD   NextOffset;
} COMMAND_BUFFER_BLOCK_TOKEN, *PCOMMAND_BUFFER_BLOCK_TOKEN;

#define COMMAND_BUFFER_BLOCK_COUNT_MAX          (4096)
#define COMMAND_BUFFER_BLOCK_HEADER_UNSECURE    (0xffff0000)
#define COMMAND_BUFFER_BLOCK_HEADER_SECURE      (0xffff0001)
#define COMMAND_BUFFER_POINTERS_COUNT_MAX       (0x0fffffff)

// Element used in header array passed as part of private data to KMD when executing a command list
typedef struct COMMAND_LIST_HEADER_ELEMENT_REC
{
    UINT64     pCmdBuffCpuAddress;
    ULONG      cmdBuffSize;
    ULONG      UmdUnused;  // currently here to force QWORD alignment
} COMMAND_LIST_HEADER_ELEMENT, *PCOMMAND_LIST_HEADER_ELEMENT;

#ifdef __cplusplus
}
#endif
#endif // _SHAREDATA_H_
