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

#include "Internal/Common/GmmLibInc.h"

/////////////////////////////////////////////////////////////////////////////////////
/// Allocates This function will initialize the necessary info based on platform.
///              - Buffer type restrictions (Eg: Z, Color, Display)
///              - X/Y tile dimensions
///
/// @param[in]  Platform: Contains information about platform to initialize an object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::PlatformInfoGen9::PlatformInfoGen9(PLATFORM &Platform)
    : PlatformInfo(Platform)
{
    __GMM_ASSERTPTR(pGmmGlobalContext, VOIDRETURN);

    // --------------------------
    // Non Native Dispay Interface buffer restriction. Register Ref: DSPACNTR, DSPASTRIDE, DSPASURF
    // Clamping res based on 2 Nndi buffers and GMM_NNDI_SEGMENT_SIZE reserved gfx memory
    // --------------------------
    Data.Nndi.Alignment            = PAGE_SIZE;
    Data.Nndi.PitchAlignment       = GMM_BYTES(1);
    Data.Nndi.RenderPitchAlignment = GMM_BYTES(1);
    Data.Nndi.LockPitchAlignment   = GMM_BYTES(1);
    Data.Nndi.MinPitch             = GMM_BYTES(640);
    Data.Nndi.MaxPitch             = GMM_BYTES(8192);
    Data.Nndi.MinAllocationSize    = PAGE_SIZE;
    Data.Nndi.MinHeight            = GMM_SCANLINES(200);
    Data.Nndi.MinWidth             = GMM_PIXELS(320);
    Data.Nndi.MinDepth             = 0;
    Data.Nndi.MaxHeight            = GMM_BYTES(1536);
    Data.Nndi.MaxWidth             = GMM_BYTES(2048);
    Data.Nndi.MaxDepth             = 1;
    Data.Nndi.MaxArraySize         = 1;

    // --------------------------
    // Depth Buffer Restriction. Inst Ref: 3DSTATE_DEPTH_BUFFER
    // --------------------------
    Data.Depth.Alignment            = PAGE_SIZE;
    Data.Depth.PitchAlignment       = GMM_BYTES(64);
    Data.Depth.RenderPitchAlignment = GMM_BYTES(64);
    Data.Depth.LockPitchAlignment   = GMM_BYTES(64);
    Data.Depth.MinPitch             = GMM_BYTES(64);
    Data.Depth.MaxPitch             = GMM_KBYTE(128); // 3DSTATE_DEPTH_BUFFER has conflicting info--but 128KB should be fine.
    Data.Depth.MinAllocationSize    = PAGE_SIZE;
    Data.Depth.MinHeight            = GMM_SCANLINES(1);
    Data.Depth.MinWidth             = GMM_PIXELS(1);
    Data.Depth.MinDepth             = 0;
    Data.Depth.MaxHeight            = GMM_KBYTE(16);
    Data.Depth.MaxWidth             = GMM_KBYTE(16);
    Data.Depth.MaxDepth             = GMM_KBYTE(2);
    Data.Depth.MaxArraySize         = GMM_KBYTE(2);

    // --------------------------
    // Stencil Buffer Restriction. Inst Ref: 3DSTATE_STENCIL_BUFFER
    // --------------------------
    Data.Stencil.Alignment            = PAGE_SIZE;
    Data.Stencil.PitchAlignment       = GMM_BYTES(128);
    Data.Stencil.RenderPitchAlignment = GMM_BYTES(128);
    Data.Stencil.LockPitchAlignment   = GMM_BYTES(128);
    Data.Stencil.MinPitch             = GMM_BYTES(128);
    Data.Stencil.MaxPitch             = GMM_KBYTE(128); // 3DSTATE_STENCIL_BUFFER: 2*Pitch <= 128KB (GMM client allocs 2x-width, so GMM limits to that.)
    Data.Stencil.MinAllocationSize    = PAGE_SIZE;
    Data.Stencil.MinHeight            = GMM_SCANLINES(1);
    Data.Stencil.MinWidth             = GMM_PIXELS(1);
    Data.Stencil.MinDepth             = 0;
    Data.Stencil.MaxHeight            = GMM_KBYTE(16);
    Data.Stencil.MaxWidth             = GMM_KBYTE(16);
    Data.Stencil.MaxDepth             = GMM_KBYTE(2);
    Data.Stencil.MaxArraySize         = GMM_KBYTE(2);

    // --------------------------
    // Hierarchical Depth Buffer Restriction. Inst Ref: 3DSTATE_HIER_DEPTH_BUFFER
    // --------------------------
    Data.HiZ.Alignment            = PAGE_SIZE;
    Data.HiZ.PitchAlignment       = GMM_BYTES(128);
    Data.HiZ.RenderPitchAlignment = GMM_BYTES(128);
    Data.HiZ.LockPitchAlignment   = GMM_BYTES(128);
    Data.HiZ.MinPitch             = GMM_BYTES(128);
    Data.HiZ.MaxPitch             = GMM_KBYTE(128);
    Data.HiZ.MinAllocationSize    = PAGE_SIZE;
    Data.HiZ.MinHeight            = GMM_SCANLINES(1);
    Data.HiZ.MinWidth             = GMM_PIXELS(1);
    Data.HiZ.MinDepth             = 0;
    Data.HiZ.MaxHeight            = GMM_KBYTE(16);
    Data.HiZ.MaxWidth             = GMM_KBYTE(16);
    Data.HiZ.MaxDepth             = GMM_KBYTE(2);
    Data.HiZ.MaxArraySize         = GMM_KBYTE(2);

    // --------------------------
    // Vertex Restriction. Inst Ref: 3DSTATE_VERTEX_BUFFER, 3DSTATE_INSTANCE_STEP_RATE
    // Note: restrictions are expanded here for UMD flexibility.
    // --------------------------
    Data.Vertex.Alignment            = PAGE_SIZE;
    Data.Vertex.PitchAlignment       = GMM_BYTES(1);
    Data.Vertex.LockPitchAlignment   = GMM_BYTES(1);
    Data.Vertex.RenderPitchAlignment = GMM_BYTES(1);
    Data.Vertex.MinPitch             = GMM_BYTES(1);
    Data.Vertex.MaxPitch             = GMM_GBYTE(2);
    Data.Vertex.MinAllocationSize    = PAGE_SIZE;
    Data.Vertex.MinHeight            = GMM_SCANLINES(1);
    Data.Vertex.MinWidth             = GMM_PIXELS(1);
    Data.Vertex.MinDepth             = 0;
    Data.Vertex.MaxHeight            = GMM_MBYTE(128); //TODO(Minor): How does Media fail when we change this to 1?!
    Data.Vertex.MaxWidth             = GMM_GBYTE(2);
    Data.Vertex.MaxDepth             = GMM_KBYTE(2);
    Data.Vertex.MaxArraySize         = GMM_KBYTE(2);

    // --------------------------
    // Index Buffer Restriction. Inst Ref: 3DSTATE_INDEX_BUFFER
    // --------------------------
    Data.Index = Data.Vertex;

    // --------------------------
    // Linear Buffer Restriction. General purpose. Flexible.
    // --------------------------
    Data.Linear.Alignment            = PAGE_SIZE;
    Data.Linear.PitchAlignment       = GMM_BYTES(1);
    Data.Linear.LockPitchAlignment   = GMM_BYTES(1);
    Data.Linear.RenderPitchAlignment = GMM_BYTES(1);
    Data.Linear.MinPitch             = GMM_BYTES(1);
    Data.Linear.MaxPitch             = GMM_GBYTE(256);
    Data.Linear.MinAllocationSize    = PAGE_SIZE;
    Data.Linear.MinHeight            = GMM_SCANLINES(1);
    Data.Linear.MinWidth             = GMM_PIXELS(1);
    Data.Linear.MinDepth             = 0;
    Data.Linear.MaxHeight            = 1;
    Data.Linear.MaxWidth             = GMM_GBYTE(256);
    Data.Linear.MaxDepth             = 1;
    Data.Linear.MaxArraySize         = 1;

    // --------------------------
    // No Surface Restriction. General purpose. Flexible.
    // --------------------------
    Data.NoRestriction.Alignment            = PAGE_SIZE;
    Data.NoRestriction.PitchAlignment       = GMM_BYTES(1);
    Data.NoRestriction.LockPitchAlignment   = GMM_BYTES(1);
    Data.NoRestriction.RenderPitchAlignment = GMM_BYTES(1);
    Data.NoRestriction.MinPitch             = GMM_BYTES(1);
    Data.NoRestriction.MaxPitch             = GMM_TBYTE(128);
    Data.NoRestriction.MinAllocationSize    = PAGE_SIZE;
    Data.NoRestriction.MinHeight            = GMM_SCANLINES(1);
    Data.NoRestriction.MinWidth             = GMM_PIXELS(1);
    Data.NoRestriction.MinDepth             = 0;
    Data.NoRestriction.MaxHeight            = GMM_GBYTE(256);
    Data.NoRestriction.MaxWidth             = GMM_TBYTE(128);
    Data.NoRestriction.MaxDepth             = GMM_KBYTE(2);
    Data.NoRestriction.MaxArraySize         = GMM_KBYTE(2);

    // --------------------------
    // Constant Buffer Restriction.
    // --------------------------
    Data.Constant = Data.NoRestriction;

    // --------------------------
    // Dx9 Constant Buffer pool Restriction. Inst Ref: 3DSTATE_DX9_CONSTANT_BUFFER_POOL_ALLOC
    // --------------------------
    Data.StateDx9ConstantBuffer           = Data.NoRestriction;
    Data.StateDx9ConstantBuffer.Alignment = GMM_KBYTE(8);

    // --------------------------
    // MC Buffer Restriction
    // --------------------------
    Data.MotionComp                      = Data.NoRestriction;
    Data.MotionComp.Alignment            = PAGE_SIZE;
    Data.MotionComp.PitchAlignment       = GMM_BYTES(32);
    Data.MotionComp.LockPitchAlignment   = GMM_BYTES(32);
    Data.MotionComp.RenderPitchAlignment = GMM_BYTES(32);
    Data.MotionComp.MinPitch             = GMM_BYTES(32);

    // --------------------------
    // Stream Buffer Restriction
    // --------------------------
    Data.Stream = Data.NoRestriction;

    // --------------------------
    // Interlace Scan Buffer Restriction
    // --------------------------
    Data.InterlacedScan = Data.NoRestriction;

    // --------------------------
    // Text API Buffer Restriction
    // --------------------------
    Data.TextApi = Data.NoRestriction;

    // --------------------------
    // RT & Texture2DSurface restrictions. Inst Ref: SURFACE_STATE
    // Greatest common restriction source comes from 8bpp RT
    // --------------------------
    Data.Texture2DSurface.Alignment            = PAGE_SIZE;
    Data.Texture2DSurface.PitchAlignment       = GMM_BYTES(32);
    Data.Texture2DSurface.LockPitchAlignment   = GMM_BYTES(32);
    Data.Texture2DSurface.RenderPitchAlignment = GMM_BYTES(32);
    Data.Texture2DSurface.MinPitch             = GMM_BYTES(32);
    Data.Texture2DSurface.MaxPitch             = (pGmmGlobalContext->GetWaTable().WaRestrictPitch128KB) ? GMM_KBYTE(128) : GMM_KBYTE(256);
    Data.Texture2DSurface.MinAllocationSize    = PAGE_SIZE;
    Data.Texture2DSurface.MinHeight            = GMM_SCANLINES(1);
    Data.Texture2DSurface.MinWidth             = GMM_PIXELS(1);
    Data.Texture2DSurface.MinDepth             = 0;
    Data.Texture2DSurface.MaxHeight            = GMM_KBYTE(16);
    Data.Texture2DSurface.MaxWidth             = GMM_KBYTE(16);
    Data.Texture2DSurface.MaxDepth             = GMM_FIELD_NA;
    Data.Texture2DSurface.MaxArraySize         = GMM_KBYTE(2);

    {
        // Linear surfaces accessed with Media Block Read/Write commands
        // require 64-byte-aligned pitch. Such commands only operate on 2D
        // resources, so we'll handle the requirement here. Though requirement
        // applies to linear surfaces only, our up'ing the pitch alignment to
        // 64 bytes here won't affect tiled surfaces, since their pitch
        // alignment is never smaller than that.
        Data.Texture2DLinearSurface                      = Data.Texture2DSurface;
        Data.Texture2DLinearSurface.PitchAlignment       = GFX_MAX(GMM_BYTES(64), Data.Texture2DSurface.PitchAlignment);
        Data.Texture2DLinearSurface.LockPitchAlignment   = GFX_MAX(GMM_BYTES(64), Data.Texture2DSurface.LockPitchAlignment);
        Data.Texture2DLinearSurface.RenderPitchAlignment = GFX_MAX(GMM_BYTES(64), Data.Texture2DSurface.RenderPitchAlignment);
    }

    // --------------------------
    // AsyncFlip Restriction. Register Ref: PRI_STRIDE, PRI_SURF, SRCSZ <-- TODO(Minor): SRCSZ correct reg for W/H req's?
    // --------------------------
    Data.ASyncFlipSurface.Alignment            = GMM_KBYTE(256);
    Data.ASyncFlipSurface.PitchAlignment       = GMM_BYTES(64);
    Data.ASyncFlipSurface.RenderPitchAlignment = GMM_BYTES(64);
    Data.ASyncFlipSurface.LockPitchAlignment   = GMM_BYTES(64);
    Data.ASyncFlipSurface.MinPitch             = GMM_BYTES(64);
    Data.ASyncFlipSurface.MaxPitch             = Data.Texture2DSurface.MaxPitch;
    Data.ASyncFlipSurface.MinAllocationSize    = PAGE_SIZE;
    Data.ASyncFlipSurface.MinHeight            = GMM_SCANLINES(1);
    Data.ASyncFlipSurface.MinWidth             = GMM_PIXELS(1);
    Data.ASyncFlipSurface.MinDepth             = 0;
    Data.ASyncFlipSurface.MaxHeight            = Data.Texture2DSurface.MaxHeight; // Beyond DE requirements-Necessary for mosaic framebuffers
    Data.ASyncFlipSurface.MaxWidth             = Data.Texture2DSurface.MaxWidth;  // Okay since GMM isn't actual display requirement gatekeeper.
    Data.ASyncFlipSurface.MaxDepth             = 1;
    Data.ASyncFlipSurface.MaxArraySize         = GMM_KBYTE(2);

    // --------------------------
    // Hardware MBM Restriction.
    // --------------------------
    Data.HardwareMBM = Data.ASyncFlipSurface;

    // --------------------------
    // Video Buffer Restriction
    // --------------------------
    Data.Video = Data.Texture2DLinearSurface;

    // --------------------------
    // Overlay Buffer Restriction. Overlay buffer restriction will be same as Async flip surface since SKL has universal planes.
    // --------------------------
    Data.Overlay = Data.ASyncFlipSurface;

    // --------------------------
    // RT & CubeSurface restrictions. Inst Ref: SURFACE_STATE
    // Greatest common restriction source comes from 8bpp RT
    // --------------------------
    Data.CubeSurface.Alignment            = PAGE_SIZE;
    Data.CubeSurface.PitchAlignment       = GMM_BYTES(32);
    Data.CubeSurface.LockPitchAlignment   = GMM_BYTES(32);
    Data.CubeSurface.RenderPitchAlignment = GMM_BYTES(32);
    Data.CubeSurface.MinPitch             = GMM_BYTES(32);
    Data.CubeSurface.MaxPitch             = (pGmmGlobalContext->GetWaTable().WaRestrictPitch128KB) ? GMM_KBYTE(128) : GMM_KBYTE(256);
    Data.CubeSurface.MinAllocationSize    = PAGE_SIZE;
    Data.CubeSurface.MinHeight            = GMM_SCANLINES(1);
    Data.CubeSurface.MinWidth             = GMM_PIXELS(1);
    Data.CubeSurface.MinDepth             = 0;
    Data.CubeSurface.MaxHeight            = GMM_KBYTE(16);
    Data.CubeSurface.MaxWidth             = GMM_KBYTE(16);
    Data.CubeSurface.MaxDepth             = 1;
    Data.CubeSurface.MaxArraySize         = GMM_KBYTE(2) / 6; // MaxElements / Cubefaces

    // --------------------------
    // RT & 3D Surface restrictions. Inst Ref: SURFACE_STATE
    // Greatest common restriction source comes from 8bpp RT
    // --------------------------
    Data.Texture3DSurface.Alignment            = PAGE_SIZE;
    Data.Texture3DSurface.PitchAlignment       = GMM_BYTES(32);
    Data.Texture3DSurface.LockPitchAlignment   = GMM_BYTES(32);
    Data.Texture3DSurface.RenderPitchAlignment = GMM_BYTES(32);
    Data.Texture3DSurface.MinPitch             = GMM_BYTES(32);
    Data.Texture3DSurface.MaxPitch             = (pGmmGlobalContext->GetWaTable().WaRestrictPitch128KB) ? GMM_KBYTE(128) : GMM_KBYTE(256);
    Data.Texture3DSurface.MinAllocationSize    = PAGE_SIZE;
    Data.Texture3DSurface.MinHeight            = GMM_SCANLINES(1);
    Data.Texture3DSurface.MinWidth             = GMM_PIXELS(1);
    Data.Texture3DSurface.MinDepth             = 0;
    Data.Texture3DSurface.MaxHeight            = GMM_KBYTE(16);
    Data.Texture3DSurface.MaxWidth             = GMM_KBYTE(16);
    Data.Texture3DSurface.MaxDepth             = GMM_KBYTE(2);
    Data.Texture3DSurface.MaxArraySize         = GMM_FIELD_NA;

    // --------------------------
    // RT & Buffer Type restrictions. Inst Ref: SURFACE_STATE
    // Greatest common restriction source comes from 8bpp RT
    // --------------------------
    Data.BufferType.Alignment            = PAGE_SIZE;
    Data.BufferType.PitchAlignment       = GMM_BYTES(32);
    Data.BufferType.LockPitchAlignment   = GMM_BYTES(32);
    Data.BufferType.RenderPitchAlignment = GMM_BYTES(32);
    Data.BufferType.MinPitch             = GMM_BYTES(32);
    Data.BufferType.MaxPitch             = GMM_GBYTE(256);
    Data.BufferType.MinAllocationSize    = PAGE_SIZE;
    Data.BufferType.MinHeight            = GMM_SCANLINES(0);
    Data.BufferType.MinWidth             = GMM_PIXELS(1);
    Data.BufferType.MinDepth             = 0;
    Data.BufferType.MaxHeight            = GMM_SCANLINES(1);
    Data.BufferType.MaxWidth             = GMM_GBYTE(256);
    Data.BufferType.MaxDepth             = GMM_FIELD_NA;
    Data.BufferType.MaxArraySize         = GMM_GBYTE(2);

    // --------------------------
    // Cursor surface restricion. Register Ref: CURACNTR, CURABASE
    // --------------------------
    Data.Cursor.Alignment            = pGmmGlobalContext->GetWaTable().WaCursor16K ? GMM_KBYTE(16) : PAGE_SIZE;
    Data.Cursor.PitchAlignment       = 1;
    Data.Cursor.LockPitchAlignment   = 1;
    Data.Cursor.RenderPitchAlignment = 1;
    Data.Cursor.MinPitch             = 1;
    Data.Cursor.MaxPitch             = 0xffffffff;
    Data.Cursor.MinAllocationSize    = 1;
    Data.Cursor.MinHeight            = GMM_SCANLINES(1);
    Data.Cursor.MinWidth             = 1;
    Data.Cursor.MinDepth             = 0;
    Data.Cursor.MaxHeight            = 0xffffffff;
    Data.Cursor.MaxWidth             = 0xffffffff;
    Data.Cursor.MaxDepth             = 0xffffffff;
    Data.Cursor.MaxArraySize         = 1;

    // clang-format off
    /******************************************************************************************************/
    /***************************************   Width,   Height,    Depth,  MtsWidth,  MtsHeight, MtsDepth */
    /******************************************************************************************************/
    // Legacy TILE_X/Y
    SET_TILE_MODE_INFO(LEGACY_TILE_X,            512,        8,        1,         0,         0,         0)
    SET_TILE_MODE_INFO(LEGACY_TILE_Y,            128,       32,        1,         0,         0,         0)
    // YS 1D
    SET_TILE_MODE_INFO(TILE_YS_1D_128bpe,       4096,        1,        1,      2048,         1,         1)
    SET_TILE_MODE_INFO(TILE_YS_1D_64bpe,        8192,        1,        1,      4096,         1,         1)
    SET_TILE_MODE_INFO(TILE_YS_1D_32bpe,       16384,        1,        1,      8192,         1,         1)
    SET_TILE_MODE_INFO(TILE_YS_1D_16bpe,       32768,        1,        1,     16384,         1,         1)
    SET_TILE_MODE_INFO(TILE_YS_1D_8bpe,        65536,        1,        1,     32768,         1,         1)
    // YS 2D
    SET_TILE_MODE_INFO(TILE_YS_2D_128bpe,       1024,       64,        1,        32,        64,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_64bpe,        1024,       64,        1,        64,        64,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_32bpe,         512,      128,        1,        64,       128,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_16bpe,         512,      128,        1,       128,       128,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_8bpe,          256,      256,        1,       128,       256,         1)
    // YS 2D 2X
    SET_TILE_MODE_INFO(TILE_YS_2D_2X_128bpe,     512,       64,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_2X_64bpe,      512,       64,        1,        64,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_2X_32bpe,      256,      128,        1,        64,        64,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_2X_16bpe,      256,      128,        1,       128,        64,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_2X_8bpe,       128,      256,        1,       128,       128,         1)
    // YS 2D 4X
    SET_TILE_MODE_INFO(TILE_YS_2D_4X_128bpe,     512,       32,        1,        16,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_4X_64bpe,      512,       32,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_4X_32bpe,      256,       64,        1,        32,        64,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_4X_16bpe,      256,       64,        1,        64,        64,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_4X_8bpe,       128,      128,        1,        64,       128,         1)
    // YS 2D 8X
    SET_TILE_MODE_INFO(TILE_YS_2D_8X_128bpe,     256,       32,        1,        16,        16,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_8X_64bpe,      256,       32,        1,        32,        16,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_8X_32bpe,      128,       64,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_8X_16bpe,      128,       64,        1,        64,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_8X_8bpe,        64,      128,        1,        64,        64,         1)
    // YS 2D 16X
    SET_TILE_MODE_INFO(TILE_YS_2D_16X_128bpe,    256,       16,        1,         8,        16,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_16X_64bpe,     256,       16,        1,        16,        16,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_16X_32bpe,     128,       32,        1,        16,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_16X_16bpe,     128,       32,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE_YS_2D_16X_8bpe,       64,       64,        1,        32,        64,         1)
    // YS 3D
    SET_TILE_MODE_INFO(TILE_YS_3D_128bpe,        256,       16,       16,         8,        16,        16)
    SET_TILE_MODE_INFO(TILE_YS_3D_64bpe,         256,       16,       16,        16,        16,        16)
    SET_TILE_MODE_INFO(TILE_YS_3D_32bpe,         128,       32,       16,        16,        32,        16)
    SET_TILE_MODE_INFO(TILE_YS_3D_16bpe,          64,       32,       32,        16,        32,        32)
    SET_TILE_MODE_INFO(TILE_YS_3D_8bpe,           64,       32,       32,        32,        32,        32)
    // YF 1D
    SET_TILE_MODE_INFO(TILE_YF_1D_128bpe,        256,        1,        1,       128,         1,         1)
    SET_TILE_MODE_INFO(TILE_YF_1D_64bpe,         512,        1,        1,       256,         1,         1)
    SET_TILE_MODE_INFO(TILE_YF_1D_32bpe,        1024,        1,        1,       512,         1,         1)
    SET_TILE_MODE_INFO(TILE_YF_1D_16bpe,        2048,        1,        1,      1024,         1,         1)
    SET_TILE_MODE_INFO(TILE_YF_1D_8bpe,         4096,        1,        1,      2048,         1,         1)
    // YF 2D
    SET_TILE_MODE_INFO(TILE_YF_2D_128bpe,        256,       16,        1,         8,        16,         1)
    SET_TILE_MODE_INFO(TILE_YF_2D_64bpe,         256,       16,        1,        16,        16,         1)
    SET_TILE_MODE_INFO(TILE_YF_2D_32bpe,         128,       32,        1,        16,        32,         1)
    SET_TILE_MODE_INFO(TILE_YF_2D_16bpe,         128,       32,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE_YF_2D_8bpe,           64,       64,        1,        32,        64,         1)
    // YF 3D
    SET_TILE_MODE_INFO(TILE_YF_3D_128bpe,         64,        8,        8,         4,         4,         8)
    SET_TILE_MODE_INFO(TILE_YF_3D_64bpe,          64,        8,        8,         8,         4,         8)
    SET_TILE_MODE_INFO(TILE_YF_3D_32bpe,          32,       16,        8,         8,         8,         8)
    SET_TILE_MODE_INFO(TILE_YF_3D_16bpe,          16,       16,       16,         8,         8,        16)
    SET_TILE_MODE_INFO(TILE_YF_3D_8bpe,           16,       16,       16,        16,         8,        16)
    // clang-format on

    //--------------------------
    // Fence paramaters. Register Ref: FENCE
    //--------------------------
    Data.NumberFenceRegisters = pGmmGlobalContext->GetWaTable().Wa16TileFencesOnly ? 16 : 32;
    Data.FenceLowBoundShift   = 12;
    Data.FenceLowBoundMask    = GFX_MASK(12, 31);
    Data.MinFenceSize         = GMM_MBYTE(1);

    Data.PagingBufferPrivateDataSize = GMM_KBYTE(4);
    Data.MaxLod                      = 14; // [0,14] --> 15 Total

    Data.FBCRequiredStolenMemorySize = GMM_MBYTE(8);

    // --------------------------
    // Surface Alignment Units
    // --------------------------
    Data.TexAlign.CCS.Align.Width                  = 128;
    Data.TexAlign.CCS.Align.Height                 = 64;
    Data.TexAlign.CCS.MaxPitchinTiles              = 512;
    Data.TexAlign.Compressed.Width                 = 4; // No reason for > HALIGN_4.
    Data.TexAlign.Compressed.Height                = 4; // No reason for > VALIGN_4.
    Data.TexAlign.Compressed.Depth                 = 1;
    Data.TexAlign.Depth.Width                      = 4; // See usage for 16bpp HALIGN_8 special-casing.
    Data.TexAlign.Depth.Height                     = 4;
    Data.TexAlign.Depth_D16_UNORM_1x_4x_16x.Width  = 8;
    Data.TexAlign.Depth_D16_UNORM_1x_4x_16x.Height = Data.TexAlign.Depth.Height;
    Data.TexAlign.Depth_D16_UNORM_2x_8x.Width      = 8;
    Data.TexAlign.Depth_D16_UNORM_2x_8x.Height     = Data.TexAlign.Depth.Height;
    Data.TexAlign.SeparateStencil.Width            = 8;
    Data.TexAlign.SeparateStencil.Height           = 8;
    Data.TexAlign.YUV422.Width                     = 4;
    Data.TexAlign.YUV422.Height                    = 4;
    Data.TexAlign.AllOther.Width                   = 16; // HALIGN_16 required for non-MSAA RT's for CCS Fast-Clear and...TBA
    Data.TexAlign.AllOther.Height                  = 4;  // VALIGN_4 should be sufficent.
    Data.TexAlign.XAdapter.Height                  = D3DKMT_CROSS_ADAPTER_RESOURCE_HEIGHT_ALIGNMENT;
    Data.TexAlign.XAdapter.Width                   = 1; //minimum should be one.

    // ----------------------------------
    // SURFACE_STATE YOffset Granularity
    // ----------------------------------
    Data.SurfaceStateYOffsetGranularity = 4;
    Data.SamplerFetchGranularityHeight  = 4;
    Data.SamplerFetchGranularityWidth   = 4;

    // ----------------------------------
    // Restrictions for Cross adapter resource
    // ----------------------------------
    Data.XAdapter.Alignment            = GMM_KBYTE(64); //64KB for DX12/StdSwizzle--Not worth special-casing.
    Data.XAdapter.PitchAlignment       = GMM_BYTES(D3DKMT_CROSS_ADAPTER_RESOURCE_PITCH_ALIGNMENT);
    Data.XAdapter.RenderPitchAlignment = GMM_BYTES(D3DKMT_CROSS_ADAPTER_RESOURCE_PITCH_ALIGNMENT);
    Data.XAdapter.LockPitchAlignment   = GMM_BYTES(D3DKMT_CROSS_ADAPTER_RESOURCE_PITCH_ALIGNMENT);
    Data.XAdapter.MinPitch             = GMM_BYTES(32);
    Data.XAdapter.MaxPitch             = (pGmmGlobalContext->GetWaTable().WaRestrictPitch128KB) ? GMM_KBYTE(128) : GMM_KBYTE(256);
    Data.XAdapter.MinAllocationSize    = PAGE_SIZE;
    Data.XAdapter.MinHeight            = GMM_SCANLINES(1);
    Data.XAdapter.MinWidth             = GMM_PIXELS(1);
    Data.XAdapter.MinDepth             = 0;
    Data.XAdapter.MaxHeight            = GMM_KBYTE(16);
    Data.XAdapter.MaxWidth             = GMM_KBYTE(16);
    Data.XAdapter.MaxDepth             = GMM_FIELD_NA;
    Data.XAdapter.MaxArraySize         = GMM_KBYTE(2);

    //---------------------------------------------
    //MaxSize for any surface type
    //---------------------------------------------
    Data.SurfaceMaxSize                      = GMM_GBYTE(256);
    Data.MaxGpuVirtualAddressBitsPerResource = 38;

    if(GFX_IS_PRODUCT(Data.Platform, IGFX_KABYLAKE) ||
       GFX_IS_PRODUCT(Data.Platform, IGFX_COFFEELAKE))
    {
        Data.MaxSLMSize = GMM_KBYTE(960);
    }
    else
    {
        Data.MaxSLMSize = GMM_KBYTE(576);
    }

    Data.HiZPixelsPerByte = 2;

    Data.ReconMaxHeight = Data.Texture2DSurface.MaxHeight; // Reconstructed surfaces require more height and width for higher resolutions.
    Data.ReconMaxWidth  = Data.Texture2DSurface.MaxWidth;

    Data.NoOfBitsSupported                = 39;
    Data.HighestAcceptablePhysicalAddress = GFX_MASK_LARGE(0, 38);
}
