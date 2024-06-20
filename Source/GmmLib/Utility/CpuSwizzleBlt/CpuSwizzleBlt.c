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
// clang-format off
// CpuSwizzleBlt.c - Surface swizzling definitions and BLT functionality.

// [!] File serves as its own header:
//      #define INCLUDE_CpuSwizzleBlt_c_AS_HEADER
//      #include "CpuSwizzleBlt.c"

#define SUB_ELEMENT_SUPPORT         // Support for Partial Element Transfer (e.g. separating/merging depth-stencil).
#define INTEL_TILE_W_SUPPORT        // Stencil Only;

#ifndef CpuSwizzleBlt_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// Background ##################################################################

/* Pixel-based surfaces commonly stored in memory row-by-row. This convention
has simple "y * Pitch + x" addressing but has spatial locality only in
horizontal direction--i.e. horizontal pixel neighbors stored next to each other
but vertical neighbors stored entire pitch away.

Since many graphics operations involve multi-dimensional data access, to
improve cache/memory access performance it is often more beneficial to use
alternative storage conventions which have multi-dimensional spatial locality--
i.e. where pixels tend to be stored near both their horizontal and vertical
neighbors.

"Tiling/Swizzling" is storage convention that increases multi-dimensional
spatial locality by treating surface as series of smaller regions/"tiles",
laid out in row-major order across surface, with entire content of each tile
stored contiguously. Data within each tile is stored in pattern that further
maximizes the locality. */


// Swizzle Descriptors #########################################################

/* Tile sizes always powers of 2 and chosen to be architecturally convenient--
e.g. 4KB to match physical page size. Tile dimensions also powers of 2, usually
chosen to produce square tiles for targeted pixel size--e.g. 4KB = 128 bytes x
32 rows = 32 x 32 pixels @ 4 bytes-per-pixel.

Since tile size and dimensions all powers of two, the spatial-to-linear mapping
required to store a tile can be trivial: spatial indexing bits can simply be
mapped to linear offset bits--e.g. for a 4KB, 128x32 tile...each byte within
tile can be referenced with a 7-bit X index and 5-bit Y index--and each of
those 12 index bits can be individually mapped to a bit in the 12-bit offset of
the tile's linear storage.

The order in which spatial index bits are mapped to linear offset bits
determines the spatial locality properties of the surface data. E.g. the
following mapping...

    Linear[11:0] = Y4 Y3 Y2 Y1 Y0 X6 X5 X4 X3 X2 X1 X0
                   \-- Y[4:0] --/ \----- X[6:0] -----/

...stores bytes of tile in row-major order, with horizontal neighbors stored
contiguously and vertical neighbors stored 128 bytes away. If instead, Y index
bits were mapped to the low-order...

    Linear[11:0] = X6 X5 X4 X3 X2 X1 X0 Y4 Y3 Y2 Y1 Y0
                   \----- X[6:0] -----/ \-- Y[4:0] --/

...bytes of tile would be stored in column-major order, with vertical neighbors
stored contiguously and horizontal neighbors stored 32 bytes away.

Individual X and Y bits can be separated and interspersed in mapping to
increase locality via sub-tiling--e.g...

    Linear[11:0] = Y4 Y3 Y2 X6 X5 X4 Y1 Y0 X3 X2 X1 X0
                                     \-- Sub-Tile ---/

...subdivies tile into 16x4 sub-tiles laid out in row-major order across tile,
with sub-tile content further stored in row-major order, with horizontal byte
neighbors within sub-tile stored contiguously and vertical neighbors only 16
bytes away. This means single 64-byte cache line contains 4x4 group of 32bpp
pixels--which is powerful spatial locality for graphics processing.

If mappings restricted to being "parallel" for index bits (i.e. bits of given
index can change position but not relative order during mapping), then bit
indexes need not be explicitly denoted--e.g. the previous sub-tiling mapping
can be represented as...

    Linear[11:0] = Y Y Y X X X Y Y X X X X

...where X and Y index bits are implied to be zero-based-counted in order they
are encountered.

In software, spatial-to-linear mapping conveniently described with bit mask for
each dimension, where a set bit indicates the next bit of that dimension's
index is mapped to that position in the linear offset--e.g....

    Linear[11:0] = Y Y Y X X X Y Y X X X X
    MaskX =        0 0 0 1 1 1 0 0 1 1 1 1
    MaskY =        1 1 1 0 0 0 1 1 0 0 0 0

Such dimensional masks all that's needed to describe given tiling/swizzling
convention, since tile size and dimensions can be derived from the masks:

    TileWidth =  2 ^ NumberOfSetBits(MaskX)
    TileHeight = 2 ^ NumberOfSetBits(MaskY)
    TileSize =   2 ^ NumberOfSetBits(MaskX OR MaskY)

Tiling/swizzling is not limited to 2D. With addition of another tile dimension,
spatial locality for 3D or MSAA sample neighbors can be controlled, also. */

    typedef struct  _SWIZZLE_DESCRIPTOR {
        struct          _SWIZZLE_DESCRIPTOR_MASKS {
            int             x, y, z;
        }               Mask;
    }               SWIZZLE_DESCRIPTOR;

    typedef enum _EXTERNAL_SWIZZLE_NAME
    {
        TILEX = 0,
        TILEY,
        TILEW,
        TILEYS,
        TILEYF
    }EXTERNAL_SWIZZLE_NAME;

    typedef enum  _EXTERNAL_RES_TYPE{
        Res_2D = 0,
        Res_3D = 1,
        MSAA_2X,
        MSAA_4X,
        MSAA_8X,
        MSAA_16X
    }EXTERNAL_RES_TYPE;

    // Definition Helper Macros...
    #define X ,'x'
    #define Y ,'y'
    #define Z ,'z'
    #define S ,'z' // S = MSAA Sample Index
    #define o ,0   // o = N/A Swizzle Bit
    #ifdef INCLUDE_CpuSwizzleBlt_c_AS_HEADER
        #define __SWIZZLE(Name, b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0) \
            extern const SWIZZLE_DESCRIPTOR Name;
    #else // C Compile...
        #define __SWIZZLE(Name, b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0) \
            const SWIZZLE_DESCRIPTOR Name = \
                { (b15 == 'x' ? 0x8000 : 0) + (b14 == 'x' ? 0x4000 : 0) + (b13 == 'x' ? 0x2000 : 0) + (b12 == 'x' ? 0x1000 : 0) + (b11 == 'x' ? 0x0800 : 0) + (b10 == 'x' ? 0x0400 : 0) + (b9 == 'x' ? 0x0200 : 0) + (b8 == 'x' ? 0x0100 : 0) + (b7 == 'x' ? 0x0080 : 0) + (b6 == 'x' ? 0x0040 : 0) + (b5 == 'x' ? 0x0020 : 0) + (b4 == 'x' ? 0x0010 : 0) + (b3 == 'x' ? 0x0008 : 0) + (b2 == 'x' ? 0x0004 : 0) + (b1 == 'x' ? 0x0002 : 0) + (b0 == 'x' ? 0x0001 : 0), \
                  (b15 == 'y' ? 0x8000 : 0) + (b14 == 'y' ? 0x4000 : 0) + (b13 == 'y' ? 0x2000 : 0) + (b12 == 'y' ? 0x1000 : 0) + (b11 == 'y' ? 0x0800 : 0) + (b10 == 'y' ? 0x0400 : 0) + (b9 == 'y' ? 0x0200 : 0) + (b8 == 'y' ? 0x0100 : 0) + (b7 == 'y' ? 0x0080 : 0) + (b6 == 'y' ? 0x0040 : 0) + (b5 == 'y' ? 0x0020 : 0) + (b4 == 'y' ? 0x0010 : 0) + (b3 == 'y' ? 0x0008 : 0) + (b2 == 'y' ? 0x0004 : 0) + (b1 == 'y' ? 0x0002 : 0) + (b0 == 'y' ? 0x0001 : 0), \
                  (b15 == 'z' ? 0x8000 : 0) + (b14 == 'z' ? 0x4000 : 0) + (b13 == 'z' ? 0x2000 : 0) + (b12 == 'z' ? 0x1000 : 0) + (b11 == 'z' ? 0x0800 : 0) + (b10 == 'z' ? 0x0400 : 0) + (b9 == 'z' ? 0x0200 : 0) + (b8 == 'z' ? 0x0100 : 0) + (b7 == 'z' ? 0x0080 : 0) + (b6 == 'z' ? 0x0040 : 0) + (b5 == 'z' ? 0x0020 : 0) + (b4 == 'z' ? 0x0010 : 0) + (b3 == 'z' ? 0x0008 : 0) + (b2 == 'z' ? 0x0004 : 0) + (b1 == 'z' ? 0x0002 : 0) + (b0 == 'z' ? 0x0001 : 0) }
#endif
    #define SWIZZLE(__SWIZZLE_Args) __SWIZZLE __SWIZZLE_Args

    // Legacy Intel Tiling Swizzles...
    SWIZZLE(( INTEL_TILE_X              o o o o Y Y Y X X X X X X X X X ));
    SWIZZLE(( INTEL_TILE_Y              o o o o X X X Y Y Y Y Y X X X X ));

    #ifdef INTEL_TILE_W_SUPPORT
        SWIZZLE(( INTEL_TILE_W          o o o o X X X Y Y Y Y X Y X Y X ));
    #endif
// Gen9 Swizzles...
    SWIZZLE(( INTEL_TILE_YF_128         o o o o X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_64          o o o o X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_32          o o o o X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_16          o o o o X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_8           o o o o X Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YS_128         X Y X Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_64          X Y X Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_32          X Y X Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_16          X Y X Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_8           X Y X Y X Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YF_MSAA2_128   o o o o S Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA2_64    o o o o S Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA2_32    o o o o S Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA2_16    o o o o S Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA2_8     o o o o S Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YS_MSAA2_128   S Y X Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA2_64    S Y X Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA2_32    S Y X Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA2_16    S Y X Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA2_8     S Y X Y X Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YF_MSAA4_128   o o o o S S X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA4_64    o o o o S S X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA4_32    o o o o S S X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA4_16    o o o o S S X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA4_8     o o o o S S X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YS_MSAA4_128   S S X Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA4_64    S S X Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA4_32    S S X Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA4_16    S S X Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA4_8     S S X Y X Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YF_MSAA8_128   o o o o S S S Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA8_64    o o o o S S S Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA8_32    o o o o S S S Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA8_16    o o o o S S S Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA8_8     o o o o S S S Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YS_MSAA8_128   S S S Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA8_64    S S S Y X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA8_32    S S S Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA8_16    S S S Y X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA8_8     S S S Y X Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YF_MSAA16_128  o o o o S S S S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA16_64   o o o o S S S S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA16_32   o o o o S S S S X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA16_16   o o o o S S S S X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_MSAA16_8    o o o o S S S S Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YS_MSAA16_128  S S S S X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA16_64   S S S S X Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA16_32   S S S S X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA16_16   S S S S X Y X Y X Y Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_MSAA16_8    S S S S X Y X Y Y Y Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YF_3D_128      o o o o Y Z X X Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_3D_64       o o o o Y Z X X Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_3D_32       o o o o Y Z X Y Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_3D_16       o o o o Y Z Y Z Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YF_3D_8        o o o o Y Z Y Z Z Z Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_YS_3D_128      X Y Z X Y Z X X Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_3D_64       X Y Z X Y Z X X Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_3D_32       X Y Z X Y Z X Y Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_3D_16       X Y Z X Y Z Y Z Z Z Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_YS_3D_8        X Y Z X Y Z Y Z Z Z Y Y X X X X ));

    // XE_HP_SDV Swizzles...
    SWIZZLE(( INTEL_TILE_4              o o o o Y Y X Y X X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_128         Y X X X Y Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_64          Y X X X Y Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_32          Y Y X X Y Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_16          Y Y X X Y Y X Y X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_8           Y Y Y X Y Y X Y X X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_MSAA2_128   Y X X X Y Y X Y S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA2_64    Y X X X Y Y X Y S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA2_32    Y Y X X Y Y X Y S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA2_16    Y Y X X Y Y X Y S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA2_8     Y Y Y X Y Y X Y S X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_MSAA_128    Y X X X Y Y X S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA_64     Y X X X Y Y X S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA_32     Y Y X X Y Y X S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA_16     Y Y X X Y Y X S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_MSAA_8      Y Y Y X Y Y X S S X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_3D_128      Z Z Y X X X Z Y Z X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_3D_64       Z Z Y X X X Z Y Z X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_3D_32       Z Z Y X Y X Z Y Z X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_3D_16       Z Z Z Y Y X Z Y Z X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_3D_8        Z Z Z X Y Y Z Y Z X Y Y X X X X ));

    //Tile64 updated layout for Render Compression 256B and Physical L3

    SWIZZLE(( INTEL_TILE_64_V2_MSAA2_128   Y X X X Y Y X S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA2_64    Y Y X X Y Y X S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA2_32    Y Y Y X Y Y X S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA2_16    Y Y Y X Y Y X S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA2_8     Y Y Y Y Y Y X S X X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_V2_MSAA4_128   Y X X X Y Y S S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA4_64    Y X X X Y Y S S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA4_32    Y Y X X Y Y S S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA4_16    Y Y X X Y Y S S X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA4_8     Y Y Y X Y Y S S X X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_V2_MSAA8_128   Y Y X X Y X S S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA8_64    Y Y X X Y X S S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA8_32    Y Y X X Y X S S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA8_16    Y Y Y X Y X S S S X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA8_8     Y Y Y X Y X S S S X Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_V2_MSAA16_128   Y X X X Y X S S S S Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA16_64    Y Y X X Y X S S S S Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA16_32    Y Y X X Y X S S S S Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA16_16    Y Y X X Y X S S S S Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_MSAA16_8     Y Y Y X Y X S S S S Y Y X X X X ));

    SWIZZLE(( INTEL_TILE_64_V2_3D_128      Z Z Y X X Y Z Z X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_3D_64       Z Z Y X X Y Z Z X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_3D_32       Z Z Y X Y Y Z Z X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_3D_16       Z Z Z Y Y Y Z Z X X Y Y X X X X ));
    SWIZZLE(( INTEL_TILE_64_V2_3D_8        Z Z Z Y Y Y Z Z X X Y Y X X X X ));


    #undef X
    #undef Y
    #undef Z
    #undef S
    #undef o
    #undef __SWIZZLE
    #undef SWIZZLE

// Accessing Swizzled Surface ##################################################

/* While graphics hardware prefers to access surfaces stored in tiled/swizzled
formats, logically accessing such surfaces with CPU-based software is non-
trivial when high throughput is goal.

This file implements (1) SwizzleOffset function to compute swizzled offset of
dimensionally-specified surface byte, and (2) CpuSwizzleBlt function to BLT
between linear ("y * pitch + x") and swizzled surfaces--with goal of providing
high-performance, swizzling BLT implementation to be used both in production
and as a guide for those seeking to understand swizzled access or implement
functionality beyond the simple BLT. */

// Surface Descriptor for CpuSwizzleBlt function...
typedef struct _CPU_SWIZZLE_BLT_SURFACE
{
    void                        *pBase;         // Pointer to surface base.
    int                         Pitch, Height;  // Row-pitch in bytes, and height, of surface.
    const SWIZZLE_DESCRIPTOR    *pSwizzle;      // Pointer to surface's swizzle descriptor, or NULL if unswizzled.
    int                         OffsetX;        // Horizontal offset into surface for BLT rectangle, in bytes.
    int                         OffsetY;        // Vertical offset into surface for BLT rectangle, in physical/pitch rows.
    int                         OffsetZ;        // Zero if N/A, or 3D offset into surface for BLT rectangle, in 3D slices or MSAA samples as appropriate.

    #ifdef SUB_ELEMENT_SUPPORT
        struct _CPU_SWIZZLE_BLT_SURFACE_ELEMENT
        {
            int                     Pitch, Size; // Zero if full-pixel BLT, or pitch and size, in bytes, of pixel element being BLT'ed.
        }                       Element;

        /* e.g. to BLT only stencil data from S8D24 surface to S8 surface...
            Dest.Element.Size = Src.Element.Size = sizeof(S8) = 1;
            Dest.Element.Pitch = sizeof(S8) = 1;
            Src.Element.Pitch = sizeof(S8D24) = 4;
            Src.OffsetX += BYTE_OFFSET_OF_S8_WITHIN_S8D24; */
    #endif
} CPU_SWIZZLE_BLT_SURFACE;

extern int SwizzleOffset(const SWIZZLE_DESCRIPTOR *pSwizzle, int Pitch, int OffsetX, int OffsetY, int OffsetZ);
extern void CpuSwizzleBlt(CPU_SWIZZLE_BLT_SURFACE *pDest, CPU_SWIZZLE_BLT_SURFACE *pSrc, int CopyWidthBytes, int CopyHeight);

#ifdef __cplusplus
}
#endif

#define CpuSwizzleBlt_INCLUDED

#endif


#ifndef INCLUDE_CpuSwizzleBlt_c_AS_HEADER

//#define MINIMALIST                // Use minimalist, unoptimized implementation.

#include "assert.h" // Quoted to allow local-directory override.

#if(_MSC_VER >= 1400)
    #include <intrin.h>
#elif defined(__ARM_ARCH)
    #include <sse2neon.h>
#elif((defined __clang__) ||(__GNUC__ > 4) || ((__GNUC__ == 4) && (__GNUC_MINOR__ >= 5)))
    #include <cpuid.h>
    #include <x86intrin.h>
#else
    #error "Unexpected compiler!"
#endif


// POPCNT: Count Lit Bits...                 0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
static unsigned char PopCnt4[16] =          {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
#define POPCNT4(x)  (PopCnt4[(x) & 0xf])
#define POPCNT16(x) (POPCNT4((x) >> 12) + POPCNT4((x) >> 8) + POPCNT4((x) >> 4) + POPCNT4(x))


int SwizzleOffset( // ##########################################################

    /* Return swizzled offset of dimensionally-specified surface byte. */

    const SWIZZLE_DESCRIPTOR    *pSwizzle,  // Pointer to applicable swizzle descriptor.
    int                         Pitch,      // Pointer to applicable surface row-pitch.
    int                         OffsetX,    // Horizontal offset into surface of the target byte, in bytes.
    int                         OffsetY,    // Vertical offset into surface of the target byte, in physical/pitch rows.
    int                         OffsetZ)    // Zero if N/A, or 3D offset into surface of the target byte, in 3D slices or MSAA samples as appropriate.

    /* Given logically-specified (x, y, z) byte within swizzled surface,
    function returns byte's linear/memory offset from surface's base--i.e. it
    performs the swizzled, spatial-to-linear mapping.

    Function makes no real effort to perform optimally, since should only used
    outside loops in CpuSwizzleBlt and similar functions. If any of this
    functionality was needed in performance path, a custom implementation
    should be used that limits itself to functionality specifically needed
    (probably single-dimension, intra-tile offsets) and uses a fast computation
    (e.g. LUT's, hard-codings, PDEP). */

{ // ###########################################################################

    char PDepSupported = -1; // AVX2/BMI2 PDEP (Parallel Deposit) Instruction

    int SwizzledOffset; // Return value being computed.

    int TileWidthBits =  POPCNT16(pSwizzle->Mask.x); // Log2(Tile Width in Bytes)
    int TileHeightBits = POPCNT16(pSwizzle->Mask.y); // Log2(Tile Height)
    int TileDepthBits =  POPCNT16(pSwizzle->Mask.z); // Log2(Tile Depth or MSAA Samples)
    int TileSizeBits =   TileWidthBits + TileHeightBits + TileDepthBits; // Log2(Tile Size in Bytes)
    int TilesPerRow =    Pitch >> TileWidthBits;     // Surface Width in Tiles

    int Row, Col;   // Tile grid position on surface, of tile containing specified byte.
    int x, y, z;    // Position of specified byte within tile that contains it.

    if(PDepSupported == -1)
    {
        #if(_MSC_VER >= 1700)
            #define PDEP(Src, Mask) _pdep_u32((Src), (Mask))
            int CpuInfo[4];
            __cpuidex(CpuInfo, 7, 0);
            PDepSupported = ((CpuInfo[1] & (1 << 8)) != 0); // EBX[8] = BMI2
        #elif ( defined (__BMI2__ ))
            #define PDEP(Src, Mask) _pdep_u32((Src), (Mask))
            unsigned int eax, ebx, ecx, edx;
            __cpuid_count(7, 0, eax, ebx, ecx, edx);
            PDepSupported = ((ebx & (1 << 8)) != 0); // EBX[8] = BMI2
        #else
            #define PDEP(Src, Mask) 0
            PDepSupported = 0;
        #endif
    }

    assert( // Mutually Exclusive Swizzle Positions...
        (pSwizzle->Mask.x | pSwizzle->Mask.y | pSwizzle->Mask.z) ==
        (pSwizzle->Mask.x + pSwizzle->Mask.y + pSwizzle->Mask.z));

    assert( // Swizzle Limited to 16-bit (else expand POPCNT'ing)...
        (pSwizzle->Mask.x | pSwizzle->Mask.y | pSwizzle->Mask.z) < (1 << 16));

    assert( // Pitch is Multiple of Tile Width...
        Pitch == ((Pitch >> TileWidthBits) << TileWidthBits));

    { // Break Positioning into Tile-Granular and Intra-Tile Components...
        assert((OffsetZ >>       TileDepthBits) == 0); // When dealing with 3D tiling, treat as separate single-tile-deep planes.
        z =     OffsetZ & ((1 << TileDepthBits) - 1);

        Row =   OffsetY >>       TileHeightBits;
        y =     OffsetY & ((1 << TileHeightBits) - 1);

        Col =   OffsetX >>       TileWidthBits;
        x =     OffsetX & ((1 << TileWidthBits) - 1);
    }

    SwizzledOffset = // Start with surface offset of given tile...
        (Row * TilesPerRow + Col) << TileSizeBits; // <-- Tiles laid across surface in row-major order.

    // ...then OR swizzled offset of byte within tile...
    if(PDepSupported)
    {
        SwizzledOffset +=
            PDEP(x, pSwizzle->Mask.x) +
            PDEP(y, pSwizzle->Mask.y) +
            PDEP(z, pSwizzle->Mask.z);
    }
    else // PDEP workalike...
    {
        int bitIndex = 0, bitMask = 1;
        int terminationMask = pSwizzle->Mask.x | pSwizzle->Mask.y | pSwizzle->Mask.z;
        while(bitMask < terminationMask)
        {
            int MaskQ;
            #define PROCESS(Q) {                    \
                MaskQ = bitMask & pSwizzle->Mask.Q; \
                SwizzledOffset += Q & MaskQ;        \
                Q <<= 1 ^ (MaskQ >> bitIndex);      \
            }
            PROCESS(x);
            PROCESS(y);
            PROCESS(z);

            bitIndex++;
            bitMask <<= 1;

            #undef PROCESS
        }
    }

    return(SwizzledOffset);
}


void CpuSwizzleBlt( // #########################################################

    /* Performs specified swizzling BLT between two given surfaces. */

    CPU_SWIZZLE_BLT_SURFACE *pDest,         // Pointer to destination surface descriptor.
    CPU_SWIZZLE_BLT_SURFACE *pSrc,          // Pointer to source surface descriptor.
    int                     CopyWidthBytes, // Width of BLT rectangle, in bytes.
    int                     CopyHeight)     // Height of BLT rectangle, in physical/pitch rows.

    #ifdef SUB_ELEMENT_SUPPORT

        /* When copying between surfaces with different pixel pitches, specify
        CopyWidthBytes in terms of unswizzled surface's element-pitches:

            CopyWidthBytes = CopyWidthPixels * pLinearSurface.Element.Pitch; */

    #endif

{ // ###########################################################################

    CPU_SWIZZLE_BLT_SURFACE *pLinearSurface, *pSwizzledSurface;
    int LinearToSwizzled;

    { // One surface swizzled, the other unswizzled (aka "linear")...
        assert((pDest->pSwizzle != NULL) ^ (pSrc->pSwizzle != NULL));

        LinearToSwizzled = !pSrc->pSwizzle;
        if(LinearToSwizzled)
        {
            pSwizzledSurface =  pDest;
            pLinearSurface =    pSrc;
        }
        else // Swizzled-to-Linear...
        {
            pSwizzledSurface =  pSrc;
            pLinearSurface =    pDest;
        }
    }

    #ifdef SUB_ELEMENT_SUPPORT
    {
        assert( // Either both or neither specified...
            (pDest->Element.Pitch != 0) == (pSrc->Element.Pitch != 0));

        assert( // Surfaces agree on transfer element size...
            pDest->Element.Size == pSrc->Element.Size);

        assert( // Element pitch not specified without element size...
            !(pDest->Element.Pitch && !pDest->Element.Size));

        assert( // Legit element sizes...
            (pDest->Element.Size <= pDest->Element.Pitch) &&
            (pSrc->Element.Size <= pSrc->Element.Pitch));

        assert( // Sub-element CopyWidthBytes in terms of LinearSurface pitch...
            (pLinearSurface->Element.Pitch == 0) ||
            ((CopyWidthBytes % pLinearSurface->Element.Pitch) == 0));
    }
    #endif

    { // No surface overrun...
        int NoOverrun =
            #ifdef SUB_ELEMENT_SUPPORT
            (
                // Sub-element transfer...
                ((pLinearSurface->Element.Size != pLinearSurface->Element.Pitch) ||
                    (pSwizzledSurface->Element.Size != pSwizzledSurface->Element.Pitch)) &&
                // No overrun...
                ((pLinearSurface->OffsetX + CopyWidthBytes) <=
                    (pLinearSurface->Pitch +
                     // CopyWidthBytes's inclusion of uncopied bytes...
                     (pLinearSurface->Element.Pitch - pLinearSurface->Element.Size))) &&
                ((pLinearSurface->OffsetY + CopyHeight) <= pLinearSurface->Height) &&
                ((pSwizzledSurface->OffsetX +
                    // Adjust CopyWidthBytes from being in terms of LinearSurface pitch...
                    (CopyWidthBytes / pLinearSurface->Element.Pitch * pSwizzledSurface->Element.Pitch)
                    ) <=
                    (pSwizzledSurface->Pitch +
                     // CopyWidthBytes's inclusion of uncopied bytes...
                     (pSwizzledSurface->Element.Pitch - pSwizzledSurface->Element.Size))) &&
                ((pSwizzledSurface->OffsetY + CopyHeight) <= pSwizzledSurface->Height)
            ) ||
            #endif

            ((pDest->OffsetX + CopyWidthBytes) <= pDest->Pitch) &&
            ((pDest->OffsetY + CopyHeight) <= pDest->Height) &&
            ((pSrc->OffsetX + CopyWidthBytes) <= pSrc->Pitch) &&
            ((pSrc->OffsetY + CopyHeight) <= pSrc->Height);

        assert(NoOverrun);
    }

    { // No surface overlap...
        char *pDest0 = (char *) pDest->pBase;
        char *pDest1 = (char *) pDest->pBase + pDest->Pitch * CopyHeight;
        char *pSrc0 =  (char *)  pSrc->pBase;
        char *pSrc1 =  (char *)  pSrc->pBase +  pSrc->Pitch * CopyHeight;

        assert(!(
            ((pDest0 >= pSrc0) && (pDest0 < pSrc1)) ||
            ((pSrc0 >= pDest0) && (pSrc0 < pDest1))));
    }

    {
        /* BLT will have pointer in each surface between which data will be
        copied from source to destination. Each pointer will be appropriately
        incremented/positioned through its surface, as BLT rectangle is
        traversed. */

        char *pLinearAddress, *pSwizzledAddress;

        // Convenient to track traversal in swizzled surface offsets...
        int x0 = pSwizzledSurface->OffsetX;
        int x1 = x0 + CopyWidthBytes;
        int y0 = pSwizzledSurface->OffsetY;
        int y1 = y0 + CopyHeight;
        int x, y;

        // Start linear pointer at specified base...
        pLinearAddress =
            (char *) pLinearSurface->pBase +
            pLinearSurface->OffsetY * pLinearSurface->Pitch +
            pLinearSurface->OffsetX;

        #ifdef MINIMALIST // Simple implementation for functional understanding/testing/etc.
        {
            #ifdef SUB_ELEMENT_SUPPORT
                assert( // No Sub-Element Transfer...
                    (pLinearSurface->Element.Size == pLinearSurface->Element.Pitch) &&
                    (pSwizzledSurface->Element.Size == pSwizzledSurface->Element.Pitch));
            #endif

            for(y = y0; y < y1; y++)
            {
                for(x = x0; x < x1; x++)
                {
                    pSwizzledAddress =
                        (char *) pSwizzledSurface->pBase +
                        SwizzleOffset(
                            pSwizzledSurface->pSwizzle,
                            pSwizzledSurface->Pitch,
                            x, y, pSwizzledSurface->OffsetZ);

                    if(LinearToSwizzled)
                    {
                        *pSwizzledAddress = *pLinearAddress;
                    }
                    else
                    {
                        *pLinearAddress = *pSwizzledAddress;
                    }

                    pLinearAddress++;
                }

                pLinearAddress += pLinearSurface->Pitch - CopyWidthBytes;
            }
        }
        #else // Production/Performance Implementation...
        {
            /* Key Performance Gains from...
                (1) Efficient Memory Transfers (Ordering + Instruction)
                (2) Minimizing Work in Inner Loops */

            #if(_MSC_VER >= 1600)
                #include <stdint.h>

                #pragma warning(push)
                #pragma warning(disable:4127) // Constant Conditional Expressions

                unsigned long LOW_BIT_Index;
                #define LOW_BIT(x)  (_BitScanForward(&LOW_BIT_Index, (x)), LOW_BIT_Index)

                unsigned long HIGH_BIT_Index;
                #define HIGH_BIT(x) (_BitScanReverse(&HIGH_BIT_Index, (x)), HIGH_BIT_Index)
            #elif(__GNUC__ >= 4)
                #include <stdint.h>

                #define LOW_BIT(x)  __builtin_ctz(x)
                #define HIGH_BIT(x) ((sizeof(x) * CHAR_BIT - 1) - __builtin_clz(x))
            #else
                #error "Unexpected compiler!"
            #endif

            typedef struct ___m24
            {
                uint8_t byte[3];
            } __m24; // 24-bit/3-byte memory element.

            // Macros intended to compile to various types of "load register from memory" instructions...
            #define MOVB_R(  Reg, Src) (*(uint8_t  *)&(Reg) = *(uint8_t  *)(Src))
            #define MOVW_R(  Reg, Src) (*(uint16_t *)&(Reg) = *(uint16_t *)(Src))
            #define MOV3_R(  Reg, Src) (*(__m24    *)&(Reg) = *(__m24 *)(Src))
            #define MOVD_R(  Reg, Src) (*(uint32_t *)&(Reg) = *(uint32_t *)(Src))

            #define MOVQ_R(  Reg, Src) ((Reg) = _mm_loadl_epi64((__m128i *)(Src)))
            #define MOVDQ_R( Reg, Src) ((Reg) = _mm_load_si128( (__m128i *)(Src)))
            #define MOVDQU_R(Reg, Src) ((Reg) = _mm_loadu_si128((__m128i *)(Src)))

            // As above, but the other half: "store to memory from register"...
            #define MOVB_M(    Dest, Reg)(*(uint8_t  *)(Dest) = *(uint8_t  *)&(Reg))
            #define MOVW_M(    Dest, Reg)(*(uint16_t *)(Dest) = *(uint16_t *)&(Reg))
            #define MOV3_M(    Dest, Reg)(*(__m24    *)(Dest) = *(__m24    *)&(Reg))
            #define MOVD_M(    Dest, Reg)(*(uint32_t *)(Dest) = *(uint32_t *)&(Reg))

            #define MOVQ_M(    Dest, Reg)(_mm_storel_epi64((__m128i *)(Dest), (Reg)))
            #define MOVDQ_M(   Dest, Reg)(_mm_store_si128( (__m128i *)(Dest), (Reg)))
            #define MOVDQU_M(  Dest, Reg)(_mm_storeu_si128((__m128i *)(Dest), (Reg)))
            #define MOVNTDQ_M( Dest, Reg)(_mm_stream_si128((__m128i *)(Dest), (Reg)))


            #define MIN_CONTAINED_POW2_BELOW_CAP(x, Cap) (1 << LOW_BIT((1 << LOW_BIT(x)) | (1 << HIGH_BIT(Cap))))

            #define SWIZZLE_OFFSET(OffsetX, OffsetY, OffsetZ) \
                SwizzleOffset(pSwizzledSurface->pSwizzle, pSwizzledSurface->Pitch, OffsetX, OffsetY, OffsetZ)

            #define MAX_XFER_WIDTH  16  // See "Compute Transfer Dimensions".
            #define MAX_XFER_HEIGHT 4   // "

            char StreamingLoadSupported = -1; // SSE4.1: MOVNTDQA

            int TileWidthBits = POPCNT16(pSwizzledSurface->pSwizzle->Mask.x);   // Log2(Tile Width in Bytes)
            int TileHeightBits = POPCNT16(pSwizzledSurface->pSwizzle->Mask.y);  // Log2(Tile Height)
            int TileDepthBits = POPCNT16(pSwizzledSurface->pSwizzle->Mask.z);   // Log2(Tile Depth or MSAA Samples)
            int BytesPerRowOfTiles = pSwizzledSurface->Pitch << (TileDepthBits + TileHeightBits);

            struct { int LeftCrust, MainRun, RightCrust; } CopyWidth;
            int MaskX[MAX_XFER_WIDTH + 1], MaskY[MAX_XFER_HEIGHT + 1];
            int SwizzledOffsetX0, SwizzledOffsetY;
            struct { int Width, Height; } SwizzleMaxXfer;

            char *pSwizzledAddressCopyBase =
                (char *) pSwizzledSurface->pBase +
                SWIZZLE_OFFSET(0, 0, pSwizzledSurface->OffsetZ);

            assert(sizeof(__m24) == 3);

            if(StreamingLoadSupported == -1)
            {
                #if(_MSC_VER >= 1500)
                    #define MOVNTDQA_R(Reg, Src) ((Reg) = _mm_stream_load_si128((__m128i *)(Src)))
                    int CpuInfo[4];
                    __cpuid(CpuInfo, 1);
                    StreamingLoadSupported = ((CpuInfo[2] & (1 << 19)) != 0); // ECX[19] = SSE4.1
                #elif(defined(__ARM_ARCH))
                    #define MOVNTDQA_R(Reg, Src) ((Reg) = (Reg))
                    StreamingLoadSupported = 0;
                #elif((defined __clang__) || (__GNUC__ > 4) || (__GNUC__ == 4) && (__GNUC_MINOR__ >= 5))
                    #define MOVNTDQA_R(Reg, Src) ((Reg) = _mm_stream_load_si128((__m128i *)(Src)))
                    unsigned int eax, ebx, ecx, edx;
                    __cpuid(1, eax, ebx, ecx, edx);
                    StreamingLoadSupported = ((ecx & (1 << 19)) != 0); // ECX[19] = SSE4.1
                #else
                    #define MOVNTDQA_R(Reg, Src) ((Reg) = (Reg))
                    StreamingLoadSupported = 0;
                #endif
            }

            { // Compute Transfer Dimensions...

                /* When transferring between linear and swizzled surfaces, we
                can't traverse linearly through memory of both since they have
                drastically different memory orderings--Moving linearly through
                one means bouncing around the other.

                Moving linearly through linear surface is more programmatically
                convenient--especially when BLT rectangles not constrained to
                tile boundaries. But moving linearly through swizzled surface
                memory is often more performance-friendly--especially when that
                memory is CPU-mapped as WC (Write Combining), which is often
                the case for graphics memory.

                Fortunately, we can avoid shortcomings of both extremes by
                using hybrid traversal: Traverse mostly linearly through linear
                surface, but have innermost loop transfer small 2D chunks sized
                to use critical runs of linearity in the swizzled memory.

                The "critical runs of linearity" that we want to hit in the
                sizzled memory are aligned, cache-line-sized memory chunks. If
                we bounce around with finer granularity we'll incur penalties
                of partial WC buffer use (whether from WC memory use or non-
                temporal stores).

                The size of 2D chunks with cache-line-sized linearity in
                swizzled memory is determined by swizzle mapping's low-order
                six bits (for 64-byte cache lines). Most swizzles use
                "Y Y X X X X" in their low-order bits, which means their cache
                lines store 16x4 chunks--So our implementation will use those
                dimensions as our target/maximum 2D transfer chunk. If we had
                any 8x8 (or taller) swizzles, we should add such support and
                increase our maximum chunk height. If we had any 32x2 swizzles,
                we should add such support and increase our maximum chunk width.

                Our implementation only bothers optimizing for 2D transfer
                chunks stored in row-major order--i.e. those whose swizzle
                mapping bits have a series of X's in the low-order, followed by
                Y's in the higher-order. Where a swizzle mapping inflection
                from Y back to X occurs, contiguous row-ordering is lost, and
                we would use that smaller, row-ordered chunk size. */

                int TargetMask;

                // Narrow optimized transfer Width by looking for inflection from X's...
                SwizzleMaxXfer.Width = MAX_XFER_WIDTH;
                while(  (TargetMask = SwizzleMaxXfer.Width - 1) &&
                        ((pSwizzledSurface->pSwizzle->Mask.x & TargetMask) != TargetMask))
                {
                    SwizzleMaxXfer.Width >>= 1;
                }

                // Narrow optimized transfer height by looking for inflection from Y's...
                SwizzleMaxXfer.Height = MAX_XFER_HEIGHT;

                while(  (TargetMask = (SwizzleMaxXfer.Height - 1) * SwizzleMaxXfer.Width) &&
                        ((pSwizzledSurface->pSwizzle->Mask.y & TargetMask) != TargetMask))
                {
                    SwizzleMaxXfer.Height >>= 1;
                }
            }

            { // Separate CopyWidthBytes into unaligned left/right "crust" and aligned "MainRun"...
                int MaxXferWidth = MIN_CONTAINED_POW2_BELOW_CAP(SwizzleMaxXfer.Width, CopyWidthBytes);

                CopyWidth.LeftCrust = // i.e. "bytes to xfer-aligned boundary"
                    (MaxXferWidth - x0) & (MaxXferWidth - 1); // Simplification of ((MaxXferWidth - (x0 % MaxXferWidth)) % MaxXferWidth)

                CopyWidth.MainRun =
                    (CopyWidthBytes - CopyWidth.LeftCrust) & ~(SwizzleMaxXfer.Width - 1); // MainRun is of SwizzleMaxXfer.Width's--not MaxXferWidth's.

                CopyWidth.RightCrust = CopyWidthBytes - (CopyWidth.LeftCrust + CopyWidth.MainRun);

                #ifdef SUB_ELEMENT_SUPPORT
                {
                    // For partial-pixel transfers, there is no crust and MainRun is done pixel-by-pixel...
                    if( (pLinearSurface->Element.Size != pLinearSurface->Element.Pitch) ||
                        (pSwizzledSurface->Element.Size != pSwizzledSurface->Element.Pitch))
                    {
                        CopyWidth.LeftCrust = CopyWidth.RightCrust = 0;
                        CopyWidth.MainRun = CopyWidthBytes;
                    }
                }
                #endif
            }


            /* Unlike in MINIMALIST implementation, which fully computes
            swizzled offset for each transfer element, we want to minimize work
            done in our inner loops.

            One way we'll reduce work is to separate pSwizzledAddress into
            dimensional components--e.g. so Y-swizzling doesn't have to be
            recomputed in X-loop.

            But a more powerful way we'll reduce work is...Instead of linearly
            incrementing spatial offsets and then converting to their swizzled
            counterparts, we'll compute swizzled bases outside the loops and
            keep them swizzled using swizzled incrementing inside the loops--
            since swizzled incrementing can be much cheaper than repeatedly
            swizzling spatial offsets.

            Intra-tile swizzled incrementing can be done by using the inverse
            of a spatial component's swizzle mask to ripple-carry a +1 to and
            across the bits of a currently swizzled value--e.g. with...

                SwizzledOffsetY:   Y X Y X Y Y X X X X
                         ~MaskY:   0 1 0 1 0 0 1 1 1 1
                                 +                   1
                                -----------------------

            ...set low-order ~MaskY bits will always ripple-carry the
            incrementing +1 to wherever Y0 happens to be, and wherever there is
            an arithmetic carry out of one Y position, set ~MaskY bits will
            carry it across any gaps to the next Y position.

            The above algorithm only works for adding one, but the mask used
            can be modified to deliver the +1 to any bit location, so any power
            of two increment can be achieved.

            After swizzled increment, residue from mask addition and undesired
            carries outside targeted fields must be removed using the natural
            mask--So the final intra-tile swizzled increment is...

                SwizzledOffsetQ = (SwizzledOffsetQ + ~MaskQ + 1) & MaskQ
                    ...where Q is the applicable X/Y/Z dimensional component.

                Or since in two's compliment, (~MaskQ + 1) = -MaskQ...

                SwizzledOffsetQ = (SwizzledOffsetQ - MaskQ) & MaskQ

            Since tile sizes are powers of two and tiles laid out in row-major
            order across surface, the above swizzled incrementing can
            additionally be used for inter-tile incrementing of X component by
            extending applicable mask to include offset bits beyond the tile--
            so arithmetic carries out of intra-tile X component will ripple to
            advance swizzled inter-tile X offset to next tile. Same is not true
            of inter-tile Y incrementing since surface pitches not restricted
            to powers of two. */

            { // Compute Mask[IncSize] for Needed Increment Values...
                int ExtendedMaskX = // Bits beyond the tile (so X incrementing can operate inter-tile)...
                    ~(pSwizzledSurface->pSwizzle->Mask.x |
                      pSwizzledSurface->pSwizzle->Mask.y |
                      pSwizzledSurface->pSwizzle->Mask.z);

                /* Subtraction below delivers natural mask for +1 increment,
                and appropriately altered mask to deliver +1 to higher bit
                positions for +2/4/8/etc. increments. */

                for(x = SwizzleMaxXfer.Width; x >= 1; x >>= 1)
                {
                    MaskX[x] = SWIZZLE_OFFSET((1 << TileWidthBits) - x, 0, 0) | ExtendedMaskX;
                }

                for(y = SwizzleMaxXfer.Height; y >= 1; y >>= 1)
                {
                    MaskY[y] = SWIZZLE_OFFSET(0, (1 << TileHeightBits) - y, 0);
                }
            }

            { // Base Dimensional Swizzled Offsets...
                int IntraTileY = y0 & ((1 << TileHeightBits) - 1);
                int TileAlignedY = y0 - IntraTileY;

                SwizzledOffsetY = SWIZZLE_OFFSET(0, IntraTileY, 0);

                SwizzledOffsetX0 =
                    SWIZZLE_OFFSET(
                        x0,
                        TileAlignedY, // <-- Since SwizzledOffsetX will include "bits beyond the tile".
                        0);
            }

            // BLT Loops ///////////////////////////////////////////////////////

            /* Traverse BLT rectangle, transferring small, optimally-aligned 2D
            chunks, as appropriate for given swizzle format. Use swizzled
            incrementing of dimensional swizzled components. */

            for(y = y0; y < y1; )
            {
                char *pSwizzledAddressLine = pSwizzledAddressCopyBase + SwizzledOffsetY;
                int xferHeight =
                    // Largest pow2 xfer height that alignment, MaxXfer, and lines left will permit...
                    MIN_CONTAINED_POW2_BELOW_CAP(y | SwizzleMaxXfer.Height, y1 - y);
                int SwizzledOffsetX = SwizzledOffsetX0;

                __m128i xmm[MAX_XFER_HEIGHT];
                char *pLinearAddressEnd;
                int _MaskX;

                // XFER Macros /////////////////////////////////////////////////

                /* We'll define "XFER" macro to contain BLT X-loop work.

                In simple implementation, XFER would be WHILE loop that does
                SSE transfer and performs pointer and swizzled offset
                incrementing.

                ...but we have multiple conditions to handle...
                  - Transfer Direction (Linear <--> Swizzled)
                  - Optimal 2D Transfer Chunk Size
                  - Available/Desired CPU Transfer Instructions
                  - Unaligned Crust

                Don't want X-loop to have conditional logic to handle
                variations since would retard performance--but neither do we
                want messy multitude of slightly different, copy-pasted code
                paths. So instead, XFER macro will provide common code template
                allowing instantiation of multiple X-loop variations--i.e. XFER
                calls from conditional Y-loop code will expand into separate,
                conditional-free, "lean and mean" X-loops.

                Some conditional logic remains in XFER chain--but only outside
                X-loop. The two IF statements that remain in X-loop (i.e. those
                in XFER_LOAD/STORE) expand to compile-time constant conditional
                expressions, so with optimizing compiler, no runtime-
                conditional code will be generated--i.e. constant conditionals
                will simply decide whether given instantiation has that code or
                not. */

                #define XFER(XFER_Store, XFER_Load, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch, XFER_Crust) \
                {                                                                                                   \
                         XFER_LINES(4, XFER_Store, XFER_Load, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch, XFER_Crust) \
                    else XFER_LINES(2, XFER_Store, XFER_Load, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch, XFER_Crust) \
                    else XFER_LINES(1, XFER_Store, XFER_Load, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch, XFER_Crust);\
                }

                #define XFER_LINES(XFER_LINES_Lines, XFER_Store, XFER_Load, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch, XFER_Crust) \
                    if(xferHeight == (XFER_LINES_Lines))    \
                    {                                       \
                        if(XFER_Crust)                      \
                        {                                   \
                            XFER_SPAN(MOVB_M, MOVB_R, CopyWidth.LeftCrust  & 1, 1, 1, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                            XFER_SPAN(MOVW_M, MOVW_R, CopyWidth.LeftCrust  & 2, 2, 2, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                            XFER_SPAN(MOVD_M, MOVD_R, CopyWidth.LeftCrust  & 4, 4, 4, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                            XFER_SPAN(MOVQ_M, MOVQ_R, CopyWidth.LeftCrust  & 8, 8, 8, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                        }                                   \
                                                            \
                        XFER_SPAN(XFER_Store, XFER_Load, CopyWidth.MainRun, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch);\
                                                            \
                        if(XFER_Crust)                      \
                        {                                   \
                            XFER_SPAN(MOVQ_M, MOVQ_R, CopyWidth.RightCrust & 8, 8, 8, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                            XFER_SPAN(MOVD_M, MOVD_R, CopyWidth.RightCrust & 4, 4, 4, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                            XFER_SPAN(MOVW_M, MOVW_R, CopyWidth.RightCrust & 2, 2, 2, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                            XFER_SPAN(MOVB_M, MOVB_R, CopyWidth.RightCrust & 1, 1, 1, XFER_LINES_Lines, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch); \
                        }                                   \
                    }

                #define XFER_SPAN(XFER_Store, XFER_Load, XFER_CopyWidthBytes, XFER_Pitch_Swizzled, XFER_Pitch_Linear, XFER_Height, XFER_pDest, XFER_DestPitch, XFER_pSrc, XFER_SrcPitch) \
                {                                                                           \
                    pLinearAddressEnd = pLinearAddress + (XFER_CopyWidthBytes);             \
                    _MaskX = MaskX[XFER_Pitch_Swizzled];                                    \
                    while(pLinearAddress < pLinearAddressEnd)                               \
                    {                                                                       \
                        pSwizzledAddress = pSwizzledAddressLine + SwizzledOffsetX;          \
                                                                                            \
                        XFER_LOAD(0, XFER_Load, XFER_pSrc, XFER_SrcPitch, XFER_Height);     \
                        XFER_LOAD(1, XFER_Load, XFER_pSrc, XFER_SrcPitch, XFER_Height);     \
                        XFER_LOAD(2, XFER_Load, XFER_pSrc, XFER_SrcPitch, XFER_Height);     \
                        XFER_LOAD(3, XFER_Load, XFER_pSrc, XFER_SrcPitch, XFER_Height);     \
                        XFER_STORE(0, XFER_Store, XFER_pDest, XFER_DestPitch, XFER_Height); \
                        XFER_STORE(1, XFER_Store, XFER_pDest, XFER_DestPitch, XFER_Height); \
                        XFER_STORE(2, XFER_Store, XFER_pDest, XFER_DestPitch, XFER_Height); \
                        XFER_STORE(3, XFER_Store, XFER_pDest, XFER_DestPitch, XFER_Height); \
                                                                                            \
                        SwizzledOffsetX = (SwizzledOffsetX - _MaskX) & _MaskX;              \
                        pLinearAddress += (XFER_Pitch_Linear);                              \
                    }                                                                       \
                }

                #define XFER_LOAD(XFER_Line, XFER_Load, XFER_pSrc, XFER_SrcPitch, XFER_Height) \
                {                                                           \
                    if((XFER_Line) < (XFER_Height))                         \
                    {                                                       \
                        XFER_Load(                                          \
                            xmm[XFER_Line],                                 \
                            (XFER_pSrc) + (XFER_Line) * (XFER_SrcPitch));   \
                    }                                                       \
                }

                #define XFER_STORE(XFER_Line, XFER_Store, XFER_pDest, XFER_DestPitch, XFER_Height) \
                {                                                           \
                    if((XFER_Line) < (XFER_Height))                         \
                    {                                                       \
                        XFER_Store(                                         \
                            (XFER_pDest) + (XFER_Line) * (XFER_DestPitch),  \
                            xmm[XFER_Line]);                                \
                    }                                                       \
                }

                // Perform Applicable Transfer /////////////////////////////////
                assert( // DQ Alignment...
                    ((intptr_t) pSwizzledSurface->pBase % 16 == 0) &&
                    (pSwizzledSurface->Pitch % 16 == 0));

                #ifdef SUB_ELEMENT_SUPPORT
                    if( (pLinearSurface->Element.Size != pLinearSurface->Element.Pitch) ||
                        (pSwizzledSurface->Element.Size != pSwizzledSurface->Element.Pitch))
                    {
                        if(LinearToSwizzled)
                        {
                            switch(pLinearSurface->Element.Size)
                            {
                                case 16: XFER(MOVNTDQ_M, MOVDQU_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, pLinearAddress, pLinearSurface->Pitch, 0); break;
                                case  8: XFER(   MOVQ_M,   MOVQ_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, pLinearAddress, pLinearSurface->Pitch, 0); break;
                                case  4: XFER(   MOVD_M,   MOVD_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, pLinearAddress, pLinearSurface->Pitch, 0); break;
                                case  3: XFER(   MOV3_M,   MOV3_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, pLinearAddress, pLinearSurface->Pitch, 0); break;
                                case  2: XFER(   MOVW_M,   MOVW_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, pLinearAddress, pLinearSurface->Pitch, 0); break;
                                case  1: XFER(   MOVB_M,   MOVB_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, pLinearAddress, pLinearSurface->Pitch, 0); break;
                                default: assert(0);
                            }
                        }
                        else
                        {
                            switch(pLinearSurface->Element.Size)
                            {
                                case 16:
                                {
                                    if(StreamingLoadSupported)
                                    {
                                        XFER(MOVDQU_M, MOVNTDQA_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0);
                                    }
                                    else
                                    {
                                        XFER(MOVDQU_M,    MOVDQ_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0);
                                    }
                                    break;
                                }
                                case  8: XFER(   MOVQ_M,   MOVQ_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0); break;
                                case  4: XFER(   MOVD_M,   MOVD_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0); break;
                                case  3: XFER(   MOV3_M,   MOV3_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0); break;
                                case  2: XFER(   MOVW_M,   MOVW_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0); break;
                                case  1: XFER(   MOVB_M,   MOVB_R, pSwizzledSurface->Element.Pitch, pLinearSurface->Element.Pitch, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, SwizzleMaxXfer.Width, 0); break;
                                default: assert(0);
                            }
                        }
                    } else
                #endif // SUB_ELEMENT_SUPPORT
                if(LinearToSwizzled)
                {
                    switch(SwizzleMaxXfer.Width)
                    {
                        case 16: XFER(MOVNTDQ_M, MOVDQU_R, 16, 16, pSwizzledAddress, 16, pLinearAddress, pLinearSurface->Pitch, 1); break;
                        #ifdef INTEL_TILE_W_SUPPORT
                            case  2: XFER(MOVW_M,  MOVW_R,  2,  2, pSwizzledAddress,  2, pLinearAddress, pLinearSurface->Pitch, 1); break;
                        #endif
                        default: assert(0); // Unexpected cases excluded to save compile time/size of multiplying instantiations.
                    }
                }
                else
                {
                    switch(SwizzleMaxXfer.Width)
                    {
                        case 16:
                        {
                            if(StreamingLoadSupported)
                            {
                                XFER(MOVDQU_M, MOVNTDQA_R, 16, 16, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, 16, 1);
                            }
                            else
                            {
                                XFER(MOVDQU_M,    MOVDQ_R, 16, 16, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress, 16, 1);
                            }
                            break;
                        }
                        #ifdef INTEL_TILE_W_SUPPORT
                            case 2: XFER(MOVW_M,   MOVW_R,  2,  2, pLinearAddress, pLinearSurface->Pitch, pSwizzledAddress,  2, 1); break;
                        #endif
                        default: assert(0);
                    }
                }


                // Swizzled inc of SwizzledOffsetY...
                SwizzledOffsetY = (SwizzledOffsetY - MaskY[xferHeight]) & MaskY[xferHeight];
                if(!SwizzledOffsetY) SwizzledOffsetX0 += BytesPerRowOfTiles; // Wraps advance SwizzledOffsetX0, since that includes "bits beyond the tile".

                y += xferHeight;

                /* X-loop only advanced pLinearAddress by CopyWidthBytes--even
                when transferred multiple lines. Advance rest of way: */
                pLinearAddress += xferHeight * pLinearSurface->Pitch - CopyWidthBytes;

            } // foreach(y)

            _mm_sfence(); // Flush Non-Temporal Writes

            #if(_MSC_VER)
                #pragma warning(pop)
            #endif
        }
        #endif
    }
} // CpuSwizzleBlt

#endif // #ifndef INCLUDE_CpuSwizzleBlt_c_AS_HEADER
// clang-format on
