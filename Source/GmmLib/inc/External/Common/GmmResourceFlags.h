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


// Set packing alignment
#pragma pack(push, 8)


//===========================================================================
// typedef:
//        GMM_RESOURCE_FLAG
//
// Description:
//     This structure describe flags that are used for each allocation
//---------------------------------------------------------------------------
typedef struct GMM_RESOURCE_FLAG_REC
{
    // Gpu: Used to describe how the surface will be used by the GPU
    struct
    {
        uint32_t CameraCapture             : 1; // Camera Capture Buffer flag to be registered with DXGK for Camera Child VRAM buffer sharing
        uint32_t CCS                       : 1; // Color Control Surface (Gen9+ for MSAA Compression or Non-MSAA Fast Color Clear) + Apply CCS restrictions when padding/aligning this resource (see GmmRestrictions.c)
        uint32_t ColorDiscard              : 1; // Tile-pass color discard
        uint32_t ColorSeparation           : 1; // Color Separated surface (Sakura display), surface scanned out as 16bpp
        uint32_t ColorSeparationRGBX       : 1; // Color Separated surface (Sakura display), surface scanned out as 24bpp (stored as 32bpp)
        uint32_t Constant                  : 1;
        uint32_t Depth                     : 1;
        uint32_t FlipChain                 : 1; // Indicating that this buffer is to be (potentially) scanned out by the display engine.
        uint32_t FlipChainPreferred        : 1; // .FlipChain & .Overlay are softer, in that they will be cleared in the result if other parameters cause conflict -- e.g. Info.TiledY.
        uint32_t HistoryBuffer             : 1; // History buffer for performace timestamp feature
        uint32_t HiZ                       : 1;
        uint32_t Index                     : 1;
        uint32_t IndirectClearColor        : 1; // Hardware managed clear color
        uint32_t InstructionFlat           : 1; // (Not for heaps!) EU Instruction Resource (addressed by *flat* STATE_BASE_ADDRESS:InstructionState--Do NOT use for STATE_BASE_ADDRESS heap resources!!)
        uint32_t InterlacedScan            : 1;
        uint32_t MCS                       : 1; // Multisample Control Surface (Pre-Gen9 for MSAA Compression or Non-MSAA Fast Color Clear)
        uint32_t MMC                       : 1; // Media Memory Compression (Gen9+): (right-side) extension to the pixel buffer for media oriented clear-compression data for texture sampling.
        uint32_t MotionComp                : 1;
        uint32_t NoRestriction             : 1; // Apply no (more correctly, minimal) restrictions when padding/aligning this resource (see GmmRestrictions.c) e.g. BLOBS.
        uint32_t Overlay                   : 1; // Equivalent to FlipChain, mutually exclusive to it. Legacy (pre gen9) name for sprite plane scane-out buffer..
        uint32_t Presentable               : 1; // The pixel format is to be validated against those supported by the render core (GPU).
        uint32_t ProceduralTexture         : 1;
        uint32_t Query                     : 1;
        uint32_t RenderTarget              : 1; // Apply RenderTarget restrictions when padding/aligning this resource (see GmmRestrictions.c)
        uint32_t S3d                       : 1;
        uint32_t S3dDx                     : 1;
        uint32_t __S3dNonPacked            : 1; // For OS unaware S3D only
        uint32_t __S3dWidi                 : 1; // For OS unaware S3D only
        uint32_t ScratchFlat               : 1; // (Not for heaps!) General State Resource (addressed by *flat* STATE_BASE_ADDRESS:GeneralState--Do NOT use for STATE_BASE_ADDRESS heap resources!!)
        uint32_t SeparateStencil           : 1;
        uint32_t State                     : 1;
        uint32_t StateDx9ConstantBuffer    : 1; // Use w/ HSW+ cmd 3DSTATE_DX9_CONSTANT_BUFFER_POOL_ALLOC
        uint32_t Stream                    : 1; // Stream-Output Resource (e.g. the target of DX10's Stream-Output Stage)
        uint32_t TextApi                   : 1;
        uint32_t Texture                   : 1; // Apply Texture restrictions when padding/aligning this resource (see GmmRestrictions.c)
        uint32_t TiledResource             : 1; // Used for DirectX11.2 tiled resources aka sparse resources
        uint32_t TilePool                  : 1; // Used for DirectX11.2 tiled resources
        uint32_t UnifiedAuxSurface         : 1; // Flag to indicate unified aux surfaces where a number of buffers are allocated as one resource creation call (e.g. for Android/gralloc when a combined CCS with its partner MSAA). A conveneince over making two GmmResCreate calls. Gen9+.
        uint32_t Vertex                    : 1;
        uint32_t Video                     : 1; // Apply Video restrictions when padding/aligning this resource (see GmmRestrictions.c)
        uint32_t __NonMsaaTileXCcs         : 1; // Internal GMM flag--Clients don't set.
        uint32_t __NonMsaaTileYCcs         : 1; // Internal GMM flag--Clients don't set.
        uint32_t __MsaaTileMcs             : 1; // Internal GMM flag--Clients don't set.
        uint32_t __NonMsaaLinearCCS        : 1; // Internal GMM flag--Clients don't set.

        uint32_t __Remaining               : 20;// Defining rather than letting float for the two zero-and-memcmp we do with the .Gpu struct (in case ={0} doesn't zero unnamed fields).
    } Gpu;

    // Info: Used to specify preferences at surface creation time
    struct
    {
        uint32_t AllowVirtualPadding       : 1;
        uint32_t BigPage                   : 1;
        uint32_t Cacheable                 : 1;
        uint32_t ContigPhysMemoryForiDART  : 1; // iDART clients only; resource allocation must be physically contiguous.
        uint32_t CornerTexelMode           : 1; // Corner Texel Mode
        uint32_t ExistingSysMem            : 1;
        uint32_t ForceResidency            : 1; // (SVM Only) Forces CPU/GPU residency of the allocation's backing pages at creation.
        uint32_t Gfdt                      : 1;
        uint32_t GttMapType                : 5;
        uint32_t HardwareProtected         : 1;
        uint32_t KernelModeMapped          : 1; // Sets up pGmmBlock->pKernelModeMapping to allow kernel-mode mapping of the backing memory
        uint32_t LayoutBelow               : 1; // Indicates the orientation of MIP data in the buffer. This is the surviving option and may be inferred as the default.
        uint32_t LayoutMono                : 1; // Legacy, deprecated MIP layout. Used for internal debugging.
        uint32_t LayoutRight               : 1; // Legacy, deprecated MIP layout.
        uint32_t LocalOnly                 : 1;
        uint32_t Linear                    : 1; // (non-)tiling preference for the allocation. (lowest priority) Y>X>W>L. See GmmLib::GmmTextureCalc::SetTileMode()
        uint32_t MediaCompressed           : 1;
        uint32_t NoOptimizationPadding     : 1; // don't swell size for sake of 64KB pages - FtrWddm2_1_64kbPages
        uint32_t NoPhysMemory              : 1; // KMD Gfx Client Submission. Client miniport drivers may want to map their physical pages to Gfx memory space instead of allocating Gfx physical memory.
        uint32_t NotLockable               : 1; // Resource is GPU-exclusive and shall not be reference by the CPU. Relevant to memory allocation components as an optimisation opportunity for mapping buffers in CPU-side.
        uint32_t NonLocalOnly              : 1;
        uint32_t StdSwizzle                : 1; // Standard Swizzle (YS) support on SKL+
        uint32_t PseudoStdSwizzle          : 1; // Only applicable to D3D12+ UMD's, for special-case of limited Standard Swizzle (YS) support on HSW/BDW/CHV.
        uint32_t Undefined64KBSwizzle      : 1; // Only applicable if system doesn't support StdSwizzle (i.e. Pre-Gen9). If set, surface is using one of the INTEL_64KB_UNDEFINED_* swizzles.
        uint32_t RedecribedPlanes          : 1; // The resource has redescribed planar surfaces
        uint32_t RenderCompressed          : 1;
        uint32_t Rotated                   : 1;
        uint32_t Shared                    : 1;
        uint32_t SoftwareProtected         : 1; // Resource is driver protected against CPU R/W access
        uint32_t SVM                       : 1; // Shared Virtual Memory (i.e. GfxAddr = CpuAddr) Can only be set for ExistingSysMem allocations.
	uint32_t TiledW                    : 1; // Tiling preference for the allocation. (second lowest priority) Y>X>W>L. Special use case.
        uint32_t TiledX                    : 1; // Tiling preference for the allocation. (second highest priority) Y>X>W>L. Common use case.
        uint32_t TiledY                    : 1; // Tiling preference for the allocation. (highest priority) Y>X>W>L. Common use case. Displayable GEn9+
        uint32_t TiledYf                   : 1; // Tiling modifier for the allocation. Affects Linear and Y preferences. Gen9+
        uint32_t TiledYs                   : 1; // Tiling modifier for the allocation. Affects Linear and Y preferences. Gen9+
        uint32_t WddmProtected             : 1; // Sets the DXGK_ALLOCATIONINFOFLAGS.Protected flag
        uint32_t XAdapter                  : 1; // For WinBlue: to support Hybrid graphics
        uint32_t __PreallocatedResInfo     : 1; // Internal GMM flag--Clients don't set.
        uint32_t __PreWddm2SVM             : 1; // Internal GMM flag--Clients don't set.
        uint32_t Tile4                     : 1; // 4KB tile
        uint32_t Tile64                    : 1; // 64KB tile
        uint32_t NotCompressed           : 1; // UMD to set this for a resource, this will be a request to GMM which need not be honoured always
        uint32_t __MapCompressible       : 1; // Internal GMM flag which marks a resource as compressed during map, used for tracking
        uint32_t __MapUnCompressible     : 1; // Internal GMM flag which marks a resource as not compressed during map, used for tracking
		
    } Info;

    // Wa: Any Surface specific Work Around will go in here
    struct
    {
        uint32_t GTMfx2ndLevelBatchRingSizeAlign   : 1; // GT steppings prior to C0 require MFX 2nd level bb's to have certain alignment.
        uint32_t ILKNeedAvcMprRowStore32KAlign     : 1; // ILK A0 requires 32K align WA for AVC MPR RowStore
        uint32_t ILKNeedAvcDmvBuffer32KAlign       : 1; // ILK A0 requires 32K align WA for AVC DMV Buffers
        uint32_t NoBufferSamplerPadding            : 1; // GMM not to add pre-BDW buffer sampler padding--UMD's take responsibility to flush L3 after sampling, etc.
        uint32_t NoLegacyPlanarLinearVideoRestrictions : 1; // GMM not to apply Legacy Planar "Linear Video" Restrictions (64B pitch alignment)
        uint32_t CHVAstcSkipVirtualMips            : 1; // GMM adjusts mip-map allocation to skip mip0 size
        uint32_t DisablePackedMipTail              : 1; // Disables Yf/Ys MIP Tail packing--forcing each MIP to its own tile boundary.
        uint32_t __ForceOtherHVALIGN4              : 1;
        uint32_t DisableDisplayCcsClearColor       : 1; // Disables display clear color
        uint32_t DisableDisplayCcsCompression      : 1; // Disables display decompression on the surface (it disables display awareness of both fast clear/render compression)
        uint32_t PreGen12FastClearOnly             : 1; // i.e. AUX_CCS_D (instead of AUX_CCS_E). Flag carried by GMM between UMDs to support shared resources.
	uint32_t MediaPipeUsage                    : 1; // TileHeight Aligned ArrayQPitch on Tile4/TileY
        uint32_t ForceStdAllocAlign                : 1; // Align standard allocation to 2MB, allowing 64K-PageTable. Set by KMD, not be used by UMDs
        uint32_t DeniableLocalOnlyForCompression   : 1; // Align standard allocation to 2MB, allowing 64K-PageTable. Set by KMD, not be used by UMDs
        uint32_t SlicePitchPadding64KB                : 1; // WA for MultiEngine compression, to enable slice pitch padding
        uint32_t SizePadding64KB                      : 1; // WA for MultiEngine compression, to enable size padding to 64KB
        uint32_t IgnoreMultiEngineCompression64KBWA   : 1; // ignore multi engine compression 64KB WA
    } Wa;

} GMM_RESOURCE_FLAG;

// Reset packing alignment to project default
#pragma pack(pop)

#ifdef __cplusplus
}
#endif /*__cplusplus*/
