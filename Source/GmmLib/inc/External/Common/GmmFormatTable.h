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

// #pragma once <-- Don't use with this file! (Multi-inclusions, differnt uses.)

// Format Group Selection...
#if(defined(GMM_FORMAT_INCLUDE_ASTC_FORMATS_ONLY))
    #define INCLUDE_ASTC_FORMATS
#elif(defined(GMM_FORMAT_INCLUDE_SURFACESTATE_FORMATS_ONLY))
    #define INCLUDE_SURFACESTATE_FORMATS
#else
    #define INCLUDE_ASTC_FORMATS
    #define INCLUDE_MISC_FORMATS
    #define INCLUDE_SURFACESTATE_FORMATS
#endif

// Table Macros (for Readability)
#define A               1
#define ALWAYS          1
#define ASTC_3D         SKU(FtrAstc3D)
#define ASTC_HDR_2D     SKU(FtrAstcHdr2D)
#define ASTC_LDR_2D     SKU(FtrAstcLdr2D)
#define GEN             GMM_FORMAT_GEN
#define NA              GMM_SURFACESTATE_FORMAT_INVALID
#define R               1
#define SKU             GMM_FORMAT_SKU
#define VLV2            GFX_IS_PRODUCT(Data.Platform,IGFX_VALLEYVIEW)
#define WA              GMM_FORMAT_WA
#define x               0
#if(!defined(__GMM_KMD__))
#define NC GMM_COMPR_FORMAT_INVALID(pGmmLibContext)
#else
#define NC GMM_COMPR_FORMAT_INVALID
#endif
#define MC(n)           n | (0x1 << 5) //GMM_FLATCCS_MIN_MC_FORMAT - 1
#define FC(ver, bpc, fmtstr, bpcstr, typestr)                               \
    (ver == 1 || (SKU(FtrE2ECompression) && !(SKU(FtrFlatPhysCCS) || SKU(FtrUnified3DMediaCompressionFormats)))) ?\
        ((bpc == 16) ? GMM_E2ECOMP_FORMAT_RGBAFLOAT16 :                     \
         (bpc == 32) ? GMM_E2ECOMP_FORMAT_R32G32B32A32_FLOAT :              \
         (bpc == 8) ? GMM_E2ECOMP_FORMAT_ARGB8b :                           \
         (bpc == x) ? GMM_E2ECOMP_FORMAT_##fmtstr : NC) :                   \
    (ver == 2 || (SKU(FtrFlatPhysCCS) && !(SKU(FtrUnified3DMediaCompressionFormats)))) ?                 \
        (GMM_FLATCCS_FORMAT_##fmtstr##bpcstr##typestr) :                               \
    (ver == 3 || (SKU(FtrUnified3DMediaCompressionFormats))) ?                         \
        (GMM_UNIFIED_COMP_FORMAT_##fmtstr##bpcstr##typestr) :                          \
         NC

/****************************************************************************\
  GMM FORMAT TABLE
  (See bottom of file for more info.)

  Supported (ALWAYS / *) -----------------------------------------------------------------o
  SURFACE_STATE.CompressionFormat (or NC) --------------------------------------o         |
  RCS SURFACE_STATE.Format (or NA) --------------------------------o            |         |
  ASTC Format (A / x) ----------------------------------------o    |            |         |
  Render Target Eligibility (R / x / *) -------------------o  |    |            |         |
  Element Depth (Pixels) -------------------------------o  |  |    |            |         |
  Element Height (Pixels) ---------------------------o  |  |  |    |            |         |
  Element Width (Pixels) ------------------------o   |  |  |  |    |            |         |
  Bits-per-Element -------------------------o    |   |  |  |  |    |            |         |
                                            |    |   |  |  |  |    |            |         |
            Name                           bpe   w   h  d  R  A  RCS.SS  CompressFormat   Available
------------------------------------------------------------------------------------------*/
#ifdef INCLUDE_SURFACESTATE_FORMATS
GMM_FORMAT( A1B5G5R5_UNORM               ,  16,  1,  1, 1, R, x, 0x124, FC(3,  x,  RGB5A1,   ,   ), GEN(8) || VLV2  )
GMM_FORMAT( A4B4G4R4_UNORM               ,  16,  1,  1, 1, R, x, 0x125, FC(3,  x,  RGB5A1,   ,   ),     GEN(8)      )
GMM_FORMAT( A4P4_UNORM_PALETTE0          ,   8,  1,  1, 1, R, x, 0x148, NC                        ,     ALWAYS      )
GMM_FORMAT( A4P4_UNORM_PALETTE1          ,   8,  1,  1, 1, R, x, 0x14F, NC                        ,     ALWAYS      )
GMM_FORMAT( A8_UNORM                     ,   8,  1,  1, 1, R, x, 0x144, FC(3,  8,       R,  8,  U),     GEN(7)      )
GMM_FORMAT( A8P8_UNORM_PALETTE0          ,  16,  1,  1, 1, R, x, 0x10F, NC                        ,     ALWAYS      )
GMM_FORMAT( A8P8_UNORM_PALETTE1          ,  16,  1,  1, 1, R, x, 0x110, NC                        ,     ALWAYS      )
GMM_FORMAT( A8X8_UNORM_G8R8_SNORM        ,  32,  1,  1, 1, R, x, 0x0E7, NC                        ,     ALWAYS      )
GMM_FORMAT( A16_FLOAT                    ,  16,  1,  1, 1, R, x, 0x117, NC                        ,     GEN(7)      )
GMM_FORMAT( A16_UNORM                    ,  16,  1,  1, 1, R, x, 0x113, NC                        ,     GEN(7)      )
GMM_FORMAT( A24X8_UNORM                  ,  32,  1,  1, 1, R, x, 0x0E2, NC                        ,     GEN(7)      )
GMM_FORMAT( A32_FLOAT                    ,  32,  1,  1, 1, R, x, 0x0E5, NC                        ,     GEN(7)      )
GMM_FORMAT( A32_UNORM                    ,  32,  1,  1, 1, R, x, 0x0DE, NC                        ,     GEN(7)      )
GMM_FORMAT( A32X32_FLOAT                 ,  64,  1,  1, 1, R, x, 0x090, NC                        ,     ALWAYS      )
GMM_FORMAT( B4G4R4A4_UNORM               ,  16,  1,  1, 1, R, x, 0x104, FC(3,  x,   RGBA4,   ,   ),     ALWAYS      )
GMM_FORMAT( B4G4R4A4_UNORM_SRGB          ,  16,  1,  1, 1, R, x, 0x105, FC(3,  x,   RGBA4,   ,   ),     ALWAYS      )
GMM_FORMAT( B5G5R5A1_UNORM               ,  16,  1,  1, 1, R, x, 0x102, FC(3,  x,  RGB5A1,   ,   ),     ALWAYS      )
GMM_FORMAT( B5G5R5A1_UNORM_SRGB          ,  16,  1,  1, 1, R, x, 0x103, FC(3,  x,  RGB5A1,   ,   ),     ALWAYS      )
GMM_FORMAT( B5G5R5X1_UNORM               ,  16,  1,  1, 1, R, x, 0x11A, FC(3,  x,  RGB5A1,   ,   ),     ALWAYS      )
GMM_FORMAT( B5G5R5X1_UNORM_SRGB          ,  16,  1,  1, 1, R, x, 0x11B, FC(3,  x,  RGB5A1,   ,   ),     ALWAYS      )
GMM_FORMAT( B5G6R5_UNORM                 ,  16,  1,  1, 1, R, x, 0x100, FC(3,  x,  B5G6R5,   ,   ),     ALWAYS      )
GMM_FORMAT( B5G6R5_UNORM_SRGB            ,  16,  1,  1, 1, R, x, 0x101, FC(3,  x,  B5G6R5,   ,   ),     ALWAYS      )
GMM_FORMAT( B8G8R8A8_UNORM               ,  32,  1,  1, 1, R, x, 0x0C0, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( B8G8R8A8_UNORM_SRGB          ,  32,  1,  1, 1, R, x, 0x0C1, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( B8G8R8X8_UNORM               ,  32,  1,  1, 1, R, x, 0x0E9, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( B8G8R8X8_UNORM_SRGB          ,  32,  1,  1, 1, R, x, 0x0EA, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( B8X8_UNORM_G8R8_SNORM        ,  32,  1,  1, 1, R, x, 0x0E8, NC                        ,     ALWAYS      )
GMM_FORMAT( B10G10R10A2_SINT             ,  32,  1,  1, 1, R, x, 0x1BB, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( B10G10R10A2_SNORM            ,  32,  1,  1, 1, R, x, 0x1B7, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( B10G10R10A2_SSCALED          ,  32,  1,  1, 1, R, x, 0x1B9, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( B10G10R10A2_UINT             ,  32,  1,  1, 1, R, x, 0x1BA, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( B10G10R10A2_UNORM            ,  32,  1,  1, 1, R, x, 0x0D1, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( B10G10R10A2_UNORM_SRGB       ,  32,  1,  1, 1, R, x, 0x0D2, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( B10G10R10A2_USCALED          ,  32,  1,  1, 1, R, x, 0x1B8, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( B10G10R10X2_UNORM            ,  32,  1,  1, 1, R, x, 0x0EE, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( BC1_UNORM                    ,  64,  4,  4, 1, x, x, 0x186, NC                        ,     ALWAYS      )
GMM_FORMAT( BC1_UNORM_SRGB               ,  64,  4,  4, 1, x, x, 0x18B, NC                        ,     ALWAYS      )
GMM_FORMAT( BC2_UNORM                    , 128,  4,  4, 1, x, x, 0x187, NC                        ,     ALWAYS      )
GMM_FORMAT( BC2_UNORM_SRGB               , 128,  4,  4, 1, x, x, 0x18C, NC                        ,     ALWAYS      )
GMM_FORMAT( BC3_UNORM                    , 128,  4,  4, 1, x, x, 0x188, NC                        ,     ALWAYS      )
GMM_FORMAT( BC3_UNORM_SRGB               , 128,  4,  4, 1, x, x, 0x18D, NC                        ,     ALWAYS      )
GMM_FORMAT( BC4_SNORM                    ,  64,  4,  4, 1, x, x, 0x199, NC                        ,     ALWAYS      )
GMM_FORMAT( BC4_UNORM                    ,  64,  4,  4, 1, x, x, 0x189, NC                        ,     ALWAYS      )
GMM_FORMAT( BC5_SNORM                    , 128,  4,  4, 1, x, x, 0x19A, NC                        ,     ALWAYS      )
GMM_FORMAT( BC5_UNORM                    , 128,  4,  4, 1, x, x, 0x18A, NC                        ,     ALWAYS      )
GMM_FORMAT( BC6H_SF16                    , 128,  4,  4, 1, x, x, 0x1A1, NC                        ,     GEN(7)      )
GMM_FORMAT( BC6H_UF16                    , 128,  4,  4, 1, x, x, 0x1A4, NC                        ,     GEN(7)      )
GMM_FORMAT( BC7_UNORM                    , 128,  4,  4, 1, x, x, 0x1A2, NC                        ,     GEN(7)      )
GMM_FORMAT( BC7_UNORM_SRGB               , 128,  4,  4, 1, x, x, 0x1A3, NC                        ,     GEN(7)      )
GMM_FORMAT( DXT1_RGB                     ,  64,  4,  4, 1, x, x, 0x191, NC                        ,     ALWAYS      )
GMM_FORMAT( DXT1_RGB_SRGB                ,  64,  4,  4, 1, x, x, 0x180, NC                        ,     ALWAYS      )
GMM_FORMAT( EAC_R11                      ,  64,  4,  4, 1, x, x, 0x1AB, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( EAC_RG11                     , 128,  4,  4, 1, x, x, 0x1AC, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( EAC_SIGNED_R11               ,  64,  4,  4, 1, x, x, 0x1AD, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( EAC_SIGNED_RG11              , 128,  4,  4, 1, x, x, 0x1AE, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC1_RGB8                    ,  64,  4,  4, 1, x, x, 0x1A9, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC2_EAC_RGBA8               , 128,  4,  4, 1, x, x, 0x1C2, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC2_EAC_SRGB8_A8            , 128,  4,  4, 1, x, x, 0x1C3, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC2_RGB8                    ,  64,  4,  4, 1, x, x, 0x1AA, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC2_RGB8_PTA                ,  64,  4,  4, 1, x, x, 0x1C0, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC2_SRGB8                   ,  64,  4,  4, 1, x, x, 0x1AF, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( ETC2_SRGB8_PTA               ,  64,  4,  4, 1, x, x, 0x1C1, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( FXT1                         , 128,  8,  4, 1, x, x, 0x192, NC                        ,     ALWAYS      )
GMM_FORMAT( I8_SINT                      ,   8,  1,  1, 1, R, x, 0x155, NC                        ,     GEN(9)      )
GMM_FORMAT( I8_UINT                      ,   8,  1,  1, 1, R, x, 0x154, NC                        ,     GEN(9)      )
GMM_FORMAT( I8_UNORM                     ,   8,  1,  1, 1, R, x, 0x145, NC                        ,     ALWAYS      )
GMM_FORMAT( I16_FLOAT                    ,  16,  1,  1, 1, R, x, 0x115, NC                        ,     ALWAYS      )
GMM_FORMAT( I16_UNORM                    ,  16,  1,  1, 1, R, x, 0x111, NC                        ,     ALWAYS      )
GMM_FORMAT( I24X8_UNORM                  ,  32,  1,  1, 1, R, x, 0x0E0, NC                        ,     ALWAYS      )
GMM_FORMAT( I32_FLOAT                    ,  32,  1,  1, 1, R, x, 0x0E3, NC                        ,     ALWAYS      )
GMM_FORMAT( I32X32_FLOAT                 ,  64,  1,  1, 1, R, x, 0x092, NC                        ,     ALWAYS      )
GMM_FORMAT( L8_SINT                      ,   8,  1,  1, 1, R, x, 0x153, NC                        ,     GEN(9)      )
GMM_FORMAT( L8_UINT                      ,   8,  1,  1, 1, R, x, 0x152, NC                        ,     GEN(9)      )
GMM_FORMAT( L8_UNORM                     ,   8,  1,  1, 1, R, x, 0x146, NC                        ,     ALWAYS      )
GMM_FORMAT( L8_UNORM_SRGB                ,   8,  1,  1, 1, R, x, 0x14C, NC                        ,     ALWAYS      )
GMM_FORMAT( L8A8_SINT                    ,  16,  1,  1, 1, R, x, 0x127, NC                        ,     GEN(9)      )
GMM_FORMAT( L8A8_UINT                    ,  16,  1,  1, 1, R, x, 0x126, NC                        ,     GEN(9)      )
GMM_FORMAT( L8A8_UNORM                   ,  16,  1,  1, 1, R, x, 0x114, NC                        ,     ALWAYS      )
GMM_FORMAT( L8A8_UNORM_SRGB              ,  16,  1,  1, 1, R, x, 0x118, NC                        ,     ALWAYS      )
GMM_FORMAT( L16_FLOAT                    ,  16,  1,  1, 1, R, x, 0x116, NC                        ,     ALWAYS      )
GMM_FORMAT( L16_UNORM                    ,  16,  1,  1, 1, R, x, 0x112, NC                        ,     ALWAYS      )
GMM_FORMAT( L16A16_FLOAT                 ,  32,  1,  1, 1, R, x, 0x0F0, NC                        ,     ALWAYS      )
GMM_FORMAT( L16A16_UNORM                 ,  32,  1,  1, 1, R, x, 0x0DF, NC                        ,     ALWAYS      )
GMM_FORMAT( L24X8_UNORM                  ,  32,  1,  1, 1, R, x, 0x0E1, NC                        ,     ALWAYS      )
GMM_FORMAT( L32_FLOAT                    ,  32,  1,  1, 1, R, x, 0x0E4, NC                        ,     ALWAYS      )
GMM_FORMAT( L32_UNORM                    ,  32,  1,  1, 1, R, x, 0x0DD, NC                        ,     ALWAYS      )
GMM_FORMAT( L32A32_FLOAT                 ,  64,  1,  1, 1, R, x, 0x08A, NC                        ,     ALWAYS      )
GMM_FORMAT( L32X32_FLOAT                 ,  64,  1,  1, 1, R, x, 0x091, NC                        ,     ALWAYS      )
GMM_FORMAT( MONO8                        ,   1,  1,  1, 1, R, x, 0x18E, NC                        ,       x         ) // No current GMM support by this name.
GMM_FORMAT( P2_UNORM_PALETTE0            ,   2,  1,  1, 1, R, x, 0x184, NC                        ,       x         ) // No current GMM support by this name.
GMM_FORMAT( P2_UNORM_PALETTE1            ,   2,  1,  1, 1, R, x, 0x185, NC                        ,       x         ) // "
GMM_FORMAT( P4A4_UNORM_PALETTE0          ,   8,  1,  1, 1, R, x, 0x147, NC                        ,     ALWAYS      )
GMM_FORMAT( P4A4_UNORM_PALETTE1          ,   8,  1,  1, 1, R, x, 0x14E, NC                        ,     ALWAYS      )
GMM_FORMAT( P8_UNORM_PALETTE0            ,   8,  1,  1, 1, R, x, 0x14B, NC                        ,     ALWAYS      )
GMM_FORMAT( P8_UNORM_PALETTE1            ,   8,  1,  1, 1, R, x, 0x14D, NC                        ,     ALWAYS      )
GMM_FORMAT( P8A8_UNORM_PALETTE0          ,  16,  1,  1, 1, R, x, 0x122, NC                        ,     ALWAYS      )
GMM_FORMAT( P8A8_UNORM_PALETTE1          ,  16,  1,  1, 1, R, x, 0x123, NC                        ,     ALWAYS      )
GMM_FORMAT( PACKED_422_16                ,  64,  2,  1, 1, R, x, 0x1A7, NC                        ,     GEN(12)     )
GMM_FORMAT( PLANAR_420_8                 ,   8,  1,  1, 1, R, x, 0x1A5, NC                        ,       x         ) // No current GMM support by this name.
GMM_FORMAT( PLANAR_420_16                ,  16,  1,  1, 1, R, x, 0x1A6, NC                        ,       x         ) // "
GMM_FORMAT( PLANAR_422_8                 ,   8,  1,  1, 1, R, x, 0x00F, NC                        ,       x         )           // <-- TODO(Minor): Remove this HW-internal format.
GMM_FORMAT( R1_UNORM                     ,   1,  1,  1, 1, R, x, 0x181, NC                        ,       x         ) // "
GMM_FORMAT( R8_SINT                      ,   8,  1,  1, 1, R, x, 0x142, FC(3,  8,       R,  8, S1),     ALWAYS      )
GMM_FORMAT( R8_SNORM                     ,   8,  1,  1, 1, R, x, 0x141, FC(3,  8,       R,  8,  S),     ALWAYS      )
GMM_FORMAT( R8_SSCALED                   ,   8,  1,  1, 1, R, x, 0x149, FC(3,  8,       R,  8,  S),     ALWAYS      )
GMM_FORMAT( R8_UINT                      ,   8,  1,  1, 1, R, x, 0x143, FC(3,  8,       R,  8, U1),     ALWAYS      )
GMM_FORMAT( R8_UNORM                     ,   8,  1,  1, 1, R, x, 0x140, FC(3,  8,       R,  8,  U),     ALWAYS      )
GMM_FORMAT( R8_USCALED                   ,   8,  1,  1, 1, R, x, 0x14A, FC(3,  8,       R,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8_SINT                    ,  16,  1,  1, 1, R, x, 0x108, FC(3,  8,      RG,  8,  S),     ALWAYS      )
GMM_FORMAT( R8G8_SNORM                   ,  16,  1,  1, 1, R, x, 0x107, FC(3,  8,      RG,  8,  S),     ALWAYS      )
GMM_FORMAT( R8G8_SSCALED                 ,  16,  1,  1, 1, R, x, 0x11C, FC(3,  8,      RG,  8,  S),     ALWAYS      )
GMM_FORMAT( R8G8_UINT                    ,  16,  1,  1, 1, R, x, 0x109, FC(3,  8,      RG,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8_UNORM                   ,  16,  1,  1, 1, R, x, 0x106, FC(3,  8,      RG,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8_USCALED                 ,  16,  1,  1, 1, R, x, 0x11D, FC(3,  8,      RG,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8B8_SINT                  ,  24,  1,  1, 1, R, x, 0x1C9, NC                        ,     GEN(8)      )
GMM_FORMAT( R8G8B8_SNORM                 ,  24,  1,  1, 1, R, x, 0x194, NC                        ,     ALWAYS      )
GMM_FORMAT( R8G8B8_SSCALED               ,  24,  1,  1, 1, R, x, 0x195, NC                        ,     ALWAYS      )
GMM_FORMAT( R8G8B8_UINT                  ,  24,  1,  1, 1, R, x, 0x1C8, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( R8G8B8_UNORM                 ,  24,  1,  1, 1, R, x, 0x193, NC                        ,     ALWAYS      )
GMM_FORMAT( R8G8B8_UNORM_SRGB            ,  24,  1,  1, 1, R, x, 0x1A8, NC                        ,     GEN(7_5)    )
GMM_FORMAT( R8G8B8_USCALED               ,  24,  1,  1, 1, R, x, 0x196, NC                        ,     ALWAYS      )
GMM_FORMAT( R8G8B8A8_SINT                ,  32,  1,  1, 1, R, x, 0x0CA, FC(3,  8,    RGBA,  8,  S),     ALWAYS      )
GMM_FORMAT( R8G8B8A8_SNORM               ,  32,  1,  1, 1, R, x, 0x0C9, FC(3,  8,    RGBA,  8,  S),     ALWAYS      )
GMM_FORMAT( R8G8B8A8_SSCALED             ,  32,  1,  1, 1, R, x, 0x0F4, FC(3,  8,    RGBA,  8,  S),     ALWAYS      )
GMM_FORMAT( R8G8B8A8_UINT                ,  32,  1,  1, 1, R, x, 0x0CB, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8B8A8_UNORM               ,  32,  1,  1, 1, R, x, 0x0C7, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8B8A8_UNORM_SRGB          ,  32,  1,  1, 1, R, x, 0x0C8, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8B8A8_USCALED             ,  32,  1,  1, 1, R, x, 0x0F5, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8B8X8_UNORM               ,  32,  1,  1, 1, R, x, 0x0EB, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( R8G8B8X8_UNORM_SRGB          ,  32,  1,  1, 1, R, x, 0x0EC, FC(3,  8,    RGBA,  8,  U),     ALWAYS      )
GMM_FORMAT( R9G9B9E5_SHAREDEXP           ,  32,  1,  1, 1, R, x, 0x0ED, NC                        ,     ALWAYS      )
GMM_FORMAT( R10G10B10_FLOAT_A2_UNORM     ,  32,  1,  1, 1, R, x, 0x0D5, FC(3,  x, RGB10A2,   ,   ),     GEN(12)     )
GMM_FORMAT( R10G10B10_SNORM_A2_UNORM     ,  32,  1,  1, 1, R, x, 0x0C5, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( R10G10B10A2_SINT             ,  32,  1,  1, 1, R, x, 0x1B6, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( R10G10B10A2_SNORM            ,  32,  1,  1, 1, R, x, 0x1B3, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( R10G10B10A2_SSCALED          ,  32,  1,  1, 1, R, x, 0x1B5, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( R10G10B10A2_UINT             ,  32,  1,  1, 1, R, x, 0x0C4, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( R10G10B10A2_UNORM            ,  32,  1,  1, 1, R, x, 0x0C2, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( R10G10B10A2_UNORM_SRGB       ,  32,  1,  1, 1, R, x, 0x0C3, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( R10G10B10A2_USCALED          ,  32,  1,  1, 1, R, x, 0x1B4, FC(3,  x, RGB10A2,   ,   ),     GEN(8)      )
GMM_FORMAT( R10G10B10X2_USCALED          ,  32,  1,  1, 1, R, x, 0x0F3, FC(3,  x, RGB10A2,   ,   ),     ALWAYS      )
GMM_FORMAT( R11G11B10_FLOAT              ,  32,  1,  1, 1, R, x, 0x0D3, FC(3,  x, RG11B10,   ,   ),     ALWAYS      )
GMM_FORMAT( R16_FLOAT                    ,  16,  1,  1, 1, R, x, 0x10E, FC(3, 16,       R, 16, F1),     ALWAYS      )
GMM_FORMAT( R16_SINT                     ,  16,  1,  1, 1, R, x, 0x10C, FC(3, 16,       R, 16, S1),     ALWAYS      )
GMM_FORMAT( R16_SNORM                    ,  16,  1,  1, 1, R, x, 0x10B, FC(3, 16,       R, 16,  S),     ALWAYS      )
GMM_FORMAT( R16_SSCALED                  ,  16,  1,  1, 1, R, x, 0x11E, FC(3, 16,       R, 16,  S),     ALWAYS      )
GMM_FORMAT( R16_UINT                     ,  16,  1,  1, 1, R, x, 0x10D, FC(3, 16,       R, 16, U1),     ALWAYS      )
GMM_FORMAT( R16_UNORM                    ,  16,  1,  1, 1, R, x, 0x10A, FC(3, 16,       R, 16,  U),     ALWAYS      )
GMM_FORMAT( R16_USCALED                  ,  16,  1,  1, 1, R, x, 0x11F, FC(3, 16,        R, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16_FLOAT                 ,  32,  1,  1, 1, R, x, 0x0D0, FC(3, 16,      RG, 16,  F),     ALWAYS      )
GMM_FORMAT( R16G16_SINT                  ,  32,  1,  1, 1, R, x, 0x0CE, FC(3, 16,      RG, 16,  S),     ALWAYS      )
GMM_FORMAT( R16G16_SNORM                 ,  32,  1,  1, 1, R, x, 0x0CD, FC(3, 16,      RG, 16,  S),     ALWAYS      )
GMM_FORMAT( R16G16_SSCALED               ,  32,  1,  1, 1, R, x, 0x0F6, FC(3, 16,      RG, 16,  S),     ALWAYS      )
GMM_FORMAT( R16G16_UINT                  ,  32,  1,  1, 1, R, x, 0x0CF, FC(3, 16,      RG, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16_UNORM                 ,  32,  1,  1, 1, R, x, 0x0CC, FC(3, 16,      RG, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16_USCALED               ,  32,  1,  1, 1, R, x, 0x0F7, FC(3, 16,      RG, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16B16_FLOAT              ,  48,  1,  1, 1, R, x, 0x19B, NC                        ,     ALWAYS      )
GMM_FORMAT( R16G16B16_SINT               ,  48,  1,  1, 1, R, x, 0x1B1, NC                        ,     GEN(8)      )
GMM_FORMAT( R16G16B16_SNORM              ,  48,  1,  1, 1, R, x, 0x19D, NC                        ,     ALWAYS      )
GMM_FORMAT( R16G16B16_SSCALED            ,  48,  1,  1, 1, R, x, 0x19E, NC                        ,     ALWAYS      )
GMM_FORMAT( R16G16B16_UINT               ,  48,  1,  1, 1, R, x, 0x1B0, NC                        , GEN(8) || VLV2  )
GMM_FORMAT( R16G16B16_UNORM              ,  48,  1,  1, 1, R, x, 0x19C, NC                        ,     ALWAYS      )
GMM_FORMAT( R16G16B16_USCALED            ,  48,  1,  1, 1, R, x, 0x19F, NC                        ,     ALWAYS      )
GMM_FORMAT( R16G16B16A16_FLOAT           ,  64,  1,  1, 1, R, x, 0x084, FC(3, 16,    RGBA, 16,  F),     ALWAYS      )
GMM_FORMAT( R16G16B16A16_SINT            ,  64,  1,  1, 1, R, x, 0x082, FC(3, 16,    RGBA, 16,  S),     ALWAYS      )
GMM_FORMAT( R16G16B16A16_SNORM           ,  64,  1,  1, 1, R, x, 0x081, FC(3, 16,    RGBA, 16,  S),     ALWAYS      )
GMM_FORMAT( R16G16B16A16_SSCALED         ,  64,  1,  1, 1, R, x, 0x093, FC(3, 16,    RGBA, 16,  S),     ALWAYS      )
GMM_FORMAT( R16G16B16A16_UINT            ,  64,  1,  1, 1, R, x, 0x083, FC(3, 16,    RGBA, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16B16A16_UNORM           ,  64,  1,  1, 1, R, x, 0x080, FC(3, 16,    RGBA, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16B16A16_USCALED         ,  64,  1,  1, 1, R, x, 0x094, FC(3, 16,    RGBA, 16,  U),     ALWAYS      )
GMM_FORMAT( R16G16B16X16_FLOAT           ,  64,  1,  1, 1, R, x, 0x08F, FC(3, 16,    RGBA, 16,  F),     ALWAYS      )
GMM_FORMAT( R16G16B16X16_UNORM           ,  64,  1,  1, 1, R, x, 0x08E, FC(3, 16,    RGBA, 16,  U),     ALWAYS      )
GMM_FORMAT( R24_UNORM_X8_TYPELESS        ,  32,  1,  1, 1, R, x, 0x0D9, FC(3, 32,       R, 32, U1),     ALWAYS      )
GMM_FORMAT( R32_FLOAT                    ,  32,  1,  1, 1, R, x, 0x0D8, FC(3, 32,       R, 32, F1),     ALWAYS      )
GMM_FORMAT( R32_FLOAT_X8X24_TYPELESS     ,  64,  1,  1, 1, R, x, 0x088, FC(3, 32,       R, 32,  F),     ALWAYS      )
GMM_FORMAT( R32_SFIXED                   ,  32,  1,  1, 1, R, x, 0x1B2, FC(3, 32,       R, 32,  S),     GEN(8)      )
GMM_FORMAT( R32_SINT                     ,  32,  1,  1, 1, R, x, 0x0D6, FC(3, 32,       R, 32, S1),     ALWAYS      )
GMM_FORMAT( R32_SNORM                    ,  32,  1,  1, 1, R, x, 0x0F2, FC(3, 32,       R, 32,  S),     ALWAYS      )
GMM_FORMAT( R32_SSCALED                  ,  32,  1,  1, 1, R, x, 0x0F8, FC(3, 32,       R, 32,  S),     ALWAYS      )
GMM_FORMAT( R32_UINT                     ,  32,  1,  1, 1, R, x, 0x0D7, FC(3, 32,       R, 32, U1),     ALWAYS      )
GMM_FORMAT( R32_UNORM                    ,  32,  1,  1, 1, R, x, 0x0F1, FC(3, 32,       R, 32,  U),     ALWAYS      )
GMM_FORMAT( R32_USCALED                  ,  32,  1,  1, 1, R, x, 0x0F9, FC(3, 32,       R, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32_FLOAT                 ,  64,  1,  1, 1, R, x, 0x085, FC(3, 32,      RG, 32,  F),     ALWAYS      )
GMM_FORMAT( R32G32_SFIXED                ,  64,  1,  1, 1, R, x, 0x0A0, FC(3, 32,      RG, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32_SINT                  ,  64,  1,  1, 1, R, x, 0x086, FC(3, 32,      RG, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32_SNORM                 ,  64,  1,  1, 1, R, x, 0x08C, FC(3, 32,      RG, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32_SSCALED               ,  64,  1,  1, 1, R, x, 0x095, FC(3, 32,      RG, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32_UINT                  ,  64,  1,  1, 1, R, x, 0x087, FC(3, 32,      RG, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32_UNORM                 ,  64,  1,  1, 1, R, x, 0x08B, FC(3, 32,      RG, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32_USCALED               ,  64,  1,  1, 1, R, x, 0x096, FC(3, 32,      RG, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32B32_FLOAT              ,  96,  1,  1, 1, R, x, 0x040, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_SFIXED             ,  96,  1,  1, 1, R, x, 0x050, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_SINT               ,  96,  1,  1, 1, R, x, 0x041, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_SNORM              ,  96,  1,  1, 1, R, x, 0x044, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_SSCALED            ,  96,  1,  1, 1, R, x, 0x045, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_UINT               ,  96,  1,  1, 1, R, x, 0x042, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_UNORM              ,  96,  1,  1, 1, R, x, 0x043, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32_USCALED            ,  96,  1,  1, 1, R, x, 0x046, NC                        ,     ALWAYS      )
GMM_FORMAT( R32G32B32A32_FLOAT           , 128,  1,  1, 1, R, x, 0x000, FC(3, 32,    RGBA, 32,  F),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_SFIXED          , 128,  1,  1, 1, R, x, 0x020, FC(3, 32,    RGBA, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_SINT            , 128,  1,  1, 1, R, x, 0x001, FC(3, 32,    RGBA, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_SNORM           , 128,  1,  1, 1, R, x, 0x004, FC(3, 32,    RGBA, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_SSCALED         , 128,  1,  1, 1, R, x, 0x007, FC(3, 32,    RGBA, 32,  S),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_UINT            , 128,  1,  1, 1, R, x, 0x002, FC(3, 32,    RGBA, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_UNORM           , 128,  1,  1, 1, R, x, 0x003, FC(3, 32,    RGBA, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32B32A32_USCALED         , 128,  1,  1, 1, R, x, 0x008, FC(3, 32,    RGBA, 32,  U),     ALWAYS      )
GMM_FORMAT( R32G32B32X32_FLOAT           , 128,  1,  1, 1, R, x, 0x006, FC(3, 32,    RGBA, 32,  F),     ALWAYS      )
GMM_FORMAT( R5G5_SNORM_B6_UNORM          ,  16,  1,  1, 1, R, x, 0x119, NC                        ,     ALWAYS      )
GMM_FORMAT( R64_FLOAT                    ,  64,  1,  1, 1, R, x, 0x08D, NC                        ,     ALWAYS      )
GMM_FORMAT( R64_PASSTHRU                 ,  64,  1,  1, 1, R, x, 0x0A1, NC                        ,     ALWAYS      )
GMM_FORMAT( R64G64_FLOAT                 , 128,  1,  1, 1, R, x, 0x005, NC                        ,     ALWAYS      )
GMM_FORMAT( R64G64_PASSTHRU              , 128,  1,  1, 1, R, x, 0x021, NC                        ,     ALWAYS      )
GMM_FORMAT( R64G64B64_FLOAT              , 192,  1,  1, 1, R, x, 0x198, NC                        ,     ALWAYS      )
GMM_FORMAT( R64G64B64_PASSTHRU           , 192,  1,  1, 1, R, x, 0x1BD, NC                        ,     GEN(8)      )
GMM_FORMAT( R64G64B64A64_FLOAT           , 256,  1,  1, 1, R, x, 0x197, NC                        ,     ALWAYS      )
GMM_FORMAT( R64G64B64A64_PASSTHRU        , 256,  1,  1, 1, R, x, 0x1BC, NC                        ,     GEN(8)      )
GMM_FORMAT( RAW                          ,   8,  1,  1, 1, R, x, 0x1FF, NC                        ,     GEN(7)      ) // "8bpp" for current GMM implementation.
GMM_FORMAT( X24_TYPELESS_G8_UINT         ,  32,  1,  1, 1, R, x, 0x0DA, FC(3, 32,       R, 32, U1),     ALWAYS      )
GMM_FORMAT( X32_TYPELESS_G8X24_UINT      ,  64,  1,  1, 1, R, x, 0x089, FC(3, 32,      RG, 32,  U),     ALWAYS      )
GMM_FORMAT( X8B8_UNORM_G8R8_SNORM        ,  32,  1,  1, 1, R, x, 0x0E6, NC                        ,     ALWAYS      )
GMM_FORMAT( Y8_UNORM                     ,   8,  1,  1, 1, R, x, 0x150, FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( YCRCB_NORMAL                 ,  16,  1,  1, 1, R, x, 0x182, FC(2,  x,    YUY2,   ,   ),     ALWAYS      )
GMM_FORMAT( YCRCB_SWAPUV                 ,  16,  1,  1, 1, R, x, 0x18F, FC(2,  x, YCRCB_SWAPUV, ,),     ALWAYS      )
GMM_FORMAT( YCRCB_SWAPUVY                ,  16,  1,  1, 1, R, x, 0x183, FC(2,  x, YCRCB_SWAPUVY,,),     ALWAYS      )
GMM_FORMAT( YCRCB_SWAPY                  ,  16,  1,  1, 1, R, x, 0x190, FC(2,  x, YCRCB_SWAPY, , ),     ALWAYS      )
#endif // INCLUDE_SURFACESTATE_FORMATS
#ifdef INCLUDE_ASTC_FORMATS
GMM_FORMAT( ASTC_FULL_2D_4x4_FLT16       , 128,  4,  4, 1, x, A, 0x140, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_5x4_FLT16       , 128,  5,  4, 1, x, A, 0x148, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_5x5_FLT16       , 128,  5,  5, 1, x, A, 0x149, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_6x5_FLT16       , 128,  6,  5, 1, x, A, 0x151, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_6x6_FLT16       , 128,  6,  6, 1, x, A, 0x152, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_8x5_FLT16       , 128,  8,  5, 1, x, A, 0x161, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_8x6_FLT16       , 128,  8,  6, 1, x, A, 0x162, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_8x8_FLT16       , 128,  8,  8, 1, x, A, 0x164, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_10x5_FLT16      , 128, 10,  5, 1, x, A, 0x171, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_10x6_FLT16      , 128, 10,  6, 1, x, A, 0x172, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_10x8_FLT16      , 128, 10,  8, 1, x, A, 0x174, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_10x10_FLT16     , 128, 10, 10, 1, x, A, 0x176, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_12x10_FLT16     , 128, 12, 10, 1, x, A, 0x17e, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_2D_12x12_FLT16     , 128, 12, 12, 1, x, A, 0x17f, NC                        ,   ASTC_HDR_2D   )
GMM_FORMAT( ASTC_FULL_3D_3x3x3_FLT16     , 128,  3,  3, 3, x, A, 0x1c0, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_4x3x3_FLT16     , 128,  4,  3, 3, x, A, 0x1d0, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_4x4x3_FLT16     , 128,  4,  4, 3, x, A, 0x1d4, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_4x4x4_FLT16     , 128,  4,  4, 4, x, A, 0x1d5, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_5x4x4_FLT16     , 128,  5,  4, 4, x, A, 0x1e5, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_5x5x4_FLT16     , 128,  5,  5, 4, x, A, 0x1e9, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_5x5x5_FLT16     , 128,  5,  5, 5, x, A, 0x1ea, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_6x5x5_FLT16     , 128,  6,  5, 5, x, A, 0x1fa, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_6x6x5_FLT16     , 128,  6,  6, 5, x, A, 0x1fe, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_FULL_3D_6x6x6_FLT16     , 128,  6,  6, 6, x, A, 0x1ff, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_2D_4x4_FLT16        , 128,  4,  4, 1, x, A, 0x040, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_4x4_U8sRGB       , 128,  4,  4, 1, x, A, 0x000, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_5x4_FLT16        , 128,  5,  4, 1, x, A, 0x048, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_5x4_U8sRGB       , 128,  5,  4, 1, x, A, 0x008, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_5x5_FLT16        , 128,  5,  5, 1, x, A, 0x049, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_5x5_U8sRGB       , 128,  5,  5, 1, x, A, 0x009, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_6x5_FLT16        , 128,  6,  5, 1, x, A, 0x051, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_6x5_U8sRGB       , 128,  6,  5, 1, x, A, 0x011, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_6x6_FLT16        , 128,  6,  6, 1, x, A, 0x052, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_6x6_U8sRGB       , 128,  6,  6, 1, x, A, 0x012, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_8x5_FLT16        , 128,  8,  5, 1, x, A, 0x061, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_8x5_U8sRGB       , 128,  8,  5, 1, x, A, 0x021, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_8x6_FLT16        , 128,  8,  6, 1, x, A, 0x062, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_8x6_U8sRGB       , 128,  8,  6, 1, x, A, 0x022, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_8x8_FLT16        , 128,  8,  8, 1, x, A, 0x064, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_8x8_U8sRGB       , 128,  8,  8, 1, x, A, 0x024, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x5_FLT16       , 128, 10,  5, 1, x, A, 0x071, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x5_U8sRGB      , 128, 10,  5, 1, x, A, 0x031, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x6_FLT16       , 128, 10,  6, 1, x, A, 0x072, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x6_U8sRGB      , 128, 10,  6, 1, x, A, 0x032, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x8_FLT16       , 128, 10,  8, 1, x, A, 0x074, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x8_U8sRGB      , 128, 10,  8, 1, x, A, 0x034, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x10_FLT16      , 128, 10, 10, 1, x, A, 0x076, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_10x10_U8sRGB     , 128, 10, 10, 1, x, A, 0x036, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_12x10_FLT16      , 128, 12, 10, 1, x, A, 0x07e, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_12x10_U8sRGB     , 128, 12, 10, 1, x, A, 0x03e, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_12x12_FLT16      , 128, 12, 12, 1, x, A, 0x07f, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_2D_12x12_U8sRGB     , 128, 12, 12, 1, x, A, 0x03f, NC                        ,   ASTC_LDR_2D   )
GMM_FORMAT( ASTC_LDR_3D_3x3x3_U8sRGB     , 128,  3,  3, 3, x, A, 0x080, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_3x3x3_FLT16      , 128,  3,  3, 3, x, A, 0x0c0, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_4x3x3_U8sRGB     , 128,  4,  3, 3, x, A, 0x090, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_4x3x3_FLT16      , 128,  4,  3, 3, x, A, 0x0d0, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_4x4x3_U8sRGB     , 128,  4,  4, 3, x, A, 0x094, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_4x4x3_FLT16      , 128,  4,  4, 3, x, A, 0x0d4, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_4x4x4_U8sRGB     , 128,  4,  4, 4, x, A, 0x095, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_4x4x4_FLT16      , 128,  4,  4, 4, x, A, 0x0d5, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_5x4x4_U8sRGB     , 128,  5,  4, 4, x, A, 0x0a5, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_5x4x4_FLT16      , 128,  5,  4, 4, x, A, 0x0e5, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_5x5x4_U8sRGB     , 128,  5,  5, 4, x, A, 0x0a9, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_5x5x4_FLT16      , 128,  5,  5, 4, x, A, 0x0e9, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_5x5x5_U8sRGB     , 128,  5,  5, 5, x, A, 0x0aa, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_5x5x5_FLT16      , 128,  5,  5, 5, x, A, 0x0ea, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_6x5x5_U8sRGB     , 128,  6,  5, 5, x, A, 0x0ba, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_6x5x5_FLT16      , 128,  6,  5, 5, x, A, 0x0fa, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_6x6x5_U8sRGB     , 128,  6,  6, 5, x, A, 0x0be, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_6x6x5_FLT16      , 128,  6,  6, 5, x, A, 0x0fe, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_6x6x6_U8sRGB     , 128,  6,  6, 6, x, A, 0x0bf, NC                        ,     ASTC_3D     )
GMM_FORMAT( ASTC_LDR_3D_6x6x6_FLT16      , 128,  6,  6, 6, x, A, 0x0ff, NC                        ,     ASTC_3D     )
#endif // INCLUDE_ASTC_FORMATS
#ifdef INCLUDE_MISC_FORMATS
GMM_FORMAT( AUYV                         ,  32,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( AYUV                         ,  32,  1,  1, 1, R, x,   NA , FC(2,  x,    AYUV,   ,   ),     ALWAYS      )
GMM_FORMAT( BAYER_BGGR8                  ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = B
GMM_FORMAT( BAYER_BGGR16                 ,  16,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = B
GMM_FORMAT( BAYER_GBRG8                  ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = G, (1, 0) = B
GMM_FORMAT( BAYER_GBRG16                 ,  16,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = G, (1, 0) = B
GMM_FORMAT( BAYER_GRBG8                  ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = G, (1, 0) = R
GMM_FORMAT( BAYER_GRBG16                 ,  16,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = G, (1, 0) = R
GMM_FORMAT( BAYER_RGGB8                  ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = R
GMM_FORMAT( BAYER_RGGB16                 ,  16,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // (0, 0) = R
GMM_FORMAT( BC1                          ,  64,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // Legacy GMM name for related HW format.
GMM_FORMAT( BC2                          , 128,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( BC3                          , 128,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( BC4                          ,  64,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( BC5                          , 128,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( BC6                          , 128,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( BC6H                         , 128,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( BC7                          , 128,  4,  4, 1, x, x,   NA , NC                        ,     GEN(7)      ) // "
GMM_FORMAT( BGRP                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // FOURCC:BGRP
GMM_FORMAT( D16_UNORM                    ,  16,  1,  1, 1, x, x,   NA , FC(3, 16,       R, 16,  U),     ALWAYS      ) //Depth uses color format L1e.En
GMM_FORMAT( D24_UNORM_X8_UINT            ,  32,  1,  1, 1, x, x,   NA , FC(3, 32,       R, 32, U1),     ALWAYS      )
GMM_FORMAT( D32_FLOAT                    ,  32,  1,  1, 1, x, x,   NA , FC(3, 32,       R, 32, F1),     ALWAYS      )
GMM_FORMAT( DXT1                         ,  64,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // Legacy GMM name for related HW format.
GMM_FORMAT( DXT2_5                       , 128,  4,  4, 1, x, x,   NA , NC                        ,     ALWAYS      ) // "
GMM_FORMAT( ETC1                         ,  64,  4,  4, 1, x, x,   NA , NC                        , GEN(8) || VLV2  ) // "
GMM_FORMAT( ETC2                         ,  64,  4,  4, 1, x, x,   NA , NC                        , GEN(8) || VLV2  ) // "
GMM_FORMAT( ETC2_EAC                     , 128,  4,  4, 1, x, x,   NA , NC                        , GEN(8) || VLV2  ) // "
GMM_FORMAT( GENERIC_8BIT                 ,   8,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_16BIT                ,  16,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_24BIT                ,  24,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_32BIT                ,  32,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_48BIT                ,  48,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_64BIT                ,  64,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_96BIT                ,  96,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_128BIT               , 128,  1,  1, 1, x, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( GENERIC_192BIT               , 192,  1,  1, 1, x, x,   NA , NC                        ,     GEN(8)      )
GMM_FORMAT( GENERIC_256BIT               , 256,  1,  1, 1, x, x,   NA , NC                        ,     GEN(8)      )
GMM_FORMAT( I420                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      ) // Same as IYUV.
GMM_FORMAT( IYUV                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( IMC1                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( IMC2                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( IMC3                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( IMC4                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( L4A4                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      ) // A4L4. No HW support.
GMM_FORMAT( MFX_JPEG_YUV411              ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(7)      )
GMM_FORMAT( MFX_JPEG_YUV411R             ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(7)      )
GMM_FORMAT( MFX_JPEG_YUV420              ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(7)      ) // Same as IMC3.
GMM_FORMAT( MFX_JPEG_YUV422H             ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(7)      )
GMM_FORMAT( MFX_JPEG_YUV422V             ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(7)      )
GMM_FORMAT( MFX_JPEG_YUV444              ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(7)      )
GMM_FORMAT( NV11                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( NV12                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( NV21                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      )
GMM_FORMAT( P8                           ,   8,  1,  1, 1, R, x,   NA,  NC                        ,     ALWAYS      )
GMM_FORMAT( P010                         ,  16,  1,  1, 1, R, x,   NA , FC(2,  x,    P010,   ,   ),     ALWAYS      )
GMM_FORMAT( P012                         ,  16,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( P016                         ,  16,  1,  1, 1, R, x,   NA , FC(2,  x,    P016,   ,   ),     ALWAYS      )
GMM_FORMAT( P208                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( R10G10B10_XR_BIAS_A2_UNORM   ,  32,  1,  1, 1, x, x,   NA , FC(2,  x, RGB10A2,   ,   ),     ALWAYS      ) // DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM
GMM_FORMAT( R24G8_TYPELESS               ,  32,  1,  1, 1, x, x,   NA , FC(2, 32,       R, 32,  U),     ALWAYS      ) // DXGI_FORMAT_R24G8_TYPELESS (To differentiate between GENERIC_32BIT.)
GMM_FORMAT( R32G8X24_TYPELESS            ,  64,  1,  1, 1, x, x,   NA , FC(2, 32,       R, 32,  U),     ALWAYS      ) // DXGI_FORMAT_R32G8X24_TYPELESS (To differentiate between GENERIC_64BIT.)
GMM_FORMAT( RENDER_8BIT                  ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( RGBP                         ,   8,  1,  1, 1, R, x,   NA , FC(2,  x,    NV12,   ,   ),     ALWAYS      ) // FOURCC:RGBP
GMM_FORMAT( Y1_UNORM                     ,   1,  1,  1, 1, x, x,   NA , NC                        ,     GEN(8)      )
GMM_FORMAT( Y8_UNORM_VA                  ,   8,  1,  1, 1, x, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(8)      )
GMM_FORMAT( Y16_SNORM                    ,  16,  1,  1, 1, x, x,   NA , FC(2,  x,    P010,   ,   ),     GEN(8)      )
GMM_FORMAT( Y16_UNORM                    ,  16,  1,  1, 1, x, x,   NA , FC(2,  x,    P010,   ,   ),     GEN(8)      )
#if (IGFX_GEN >= IGFX_GEN10)
GMM_FORMAT( Y32_UNORM                    ,  32,  1,  1, 1, x, x,   NA , NC                        ,     GEN(10)     ) // Y32 removed from Gen9 but still referenced, only available Gen10+
#endif
GMM_FORMAT( Y210                         ,  64,  2,  1, 1, R, x,   NA , FC(2,  x,    Y210,   ,   ),     GEN(11)     ) // Packed 422 10/12/16 bit
GMM_FORMAT( Y212                         ,  64,  2,  1, 1, R, x,   NA , FC(2,  x,    Y216,   ,   ),     GEN(11)     )
GMM_FORMAT( Y410                         ,  32,  1,  1, 1, R, x,   NA , FC(2,  x,    Y410,   ,   ),     GEN(11)     )
GMM_FORMAT( Y412                         ,  64,  1,  1, 1, R, x,   NA , FC(2,  x,    Y416,   ,   ),     GEN(11)     )
GMM_FORMAT( Y216                         ,  64,  2,  1, 1, R, x,   NA,  FC(2,  x,    Y216,   ,   ),     ALWAYS      )
GMM_FORMAT( Y416                         ,  64,  1,  1, 1, R, x,   NA , FC(2,  x,    Y416,   ,   ),     ALWAYS      ) // Packed 444 10/12/16 bit,
GMM_FORMAT( YV12                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
GMM_FORMAT( YVU9                         ,   8,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
// Implement packed 4:2:2 YUV format (UYVY, VYUY, YUY2, YVYU) as compressed block format by suffixing _2x1.(i.e. 32bpe 2x1 pixel blocks instead of 16bpp 1x1 block)
// All OS components(UMDs/KMD) can switch to *_2x1 style independent of legacy implementation.
// Refer GmmCommonExt.h for legacy implemenation of UYVY, VYUY, YUY2, YVYU)
// TODO : Unify them when all OS-components switch to compressed block format
GMM_FORMAT( UYVY_2x1                     ,  32,  2,  1, 1, R, x,   NA , FC(2,  x,   SWAPY,   ,   ),     ALWAYS      )
GMM_FORMAT( VYUY_2x1                     ,  32,  2,  1, 1, R, x,   NA , FC(2,  x, SWAPUVY,   ,   ),     ALWAYS      )
GMM_FORMAT( YUY2_2x1                     ,  32,  2,  1, 1, R, x,   NA , FC(2,  x,    YUY2,   ,   ),     ALWAYS      )
GMM_FORMAT( YVYU_2x1                     ,  32,  2,  1, 1, R, x,   NA , FC(2,  x,  SWAPUV,   ,   ),     ALWAYS      )
GMM_FORMAT( MEDIA_Y1_UNORM               ,   1,  1,  1, 1, x, x,   NA , NC                        ,     GEN(8)      )
GMM_FORMAT( MEDIA_Y8_UNORM               ,   8,  1,  1, 1, x, x,   NA , FC(2,  x,    NV12,   ,   ),     GEN(8)      )
GMM_FORMAT( MEDIA_Y16_SNORM              ,  16,  1,  1, 1, x, x,   NA , FC(2,  x,    P010,   ,   ),     GEN(8)      )
GMM_FORMAT( MEDIA_Y16_UNORM              ,  16,  1,  1, 1, x, x,   NA , FC(2,  x,    P010,   ,   ),     GEN(8)      )
GMM_FORMAT( MEDIA_Y32_UNORM              ,   1,  1,  1, 1, x, x,   NA , NC                        ,     GEN(8)      ) // Y32 is BDW name for SKL Y1, and is 1bpp with 32b granularity
GMM_FORMAT( B16G16R16A16_UNORM           ,  64,  1,  1, 1, R, x,   NA , FC(3, 16,    RGBA, 16,  U),     ALWAYS      ) // Swapped ARGB16 for media-SFC output
GMM_FORMAT( P216                         ,  16,  1,  1, 1, R, x,   NA , NC                        ,     ALWAYS      )
#if _WIN32
GMM_FORMAT( WGBOX_YUV444                 ,  32,  1,  1, 1, x, x,   NA , NC                        ,     GEN(9)      ) // For testing purposes only.
GMM_FORMAT( WGBOX_PLANAR_YUV444          ,  32,  1,  1, 1, x, x,   NA , NC                        ,     GEN(9)      ) // For testing purposes only.
#endif
#endif // INCLUDE_MISC_FORMATS

/*****************************************************************************\
  Usage:
  File #include'ed into various areas of source code, to produce different
  things--various enums, struct/array-initializing code, etc.

  Format Names:
  The GMM_RESOURCE_FORMAT and GMM_SURFACESTATE_FORMAT enums are generated from
  this table.

  Supported Conditionals(*) and Meaning:
  GEN(X)........"Gen X or later" where X is text for IGFX_GEN[X]_CORE.
  SKU(FtrXxx)..."SKU FtrXxx is set".
  WA(WaXxx)....."WA WaXxx is set". Usually used with !/NOT prefix in table.

  Conditionals:
  Inclusions making use of columns supporting conditionals (e.g. "RT",
  "Available") must wrap the inclusion with macro definitions to service the
  supported conditionals in the local source--e.g....
    #define GMM_FORMAT_GEN(X)      (GFX_GET_CURRENT_RENDERCORE(pHwDevExt->platform) >= IGFX_GEN##X##_CORE)
    #define GMM_FORMAT_SKU(FtrXxx) (GFX_IS_SKU(pHwDevExt, FtrXxx))
    #define GMM_FORMAT_WA(WaXxx)   (GFX_IS_WA(pHwDevExt, WaXxx))
    #define GMM_FORMAT(Name, bpe, Width, Height, Depth, IsRT, IsASTC, RcsSurfaceFormat, AuxL1eFormat, Availability) ...
    #include "GmmFormatTable.h"

\*****************************************************************************/

#undef A
#undef ALWAYS
#undef ASTC_3D
#undef ASTC_HDR_2D
#undef ASTC_LDR_2D
#undef GEN
#undef INCLUDE_ASTC_FORMATS
#undef INCLUDE_MISC_FORMATS
#undef INCLUDE_SURFACESTATE_FORMATS
#undef NA
#undef NC
#undef R
#undef SKU
#undef VLV2
#undef WA
#undef x

// So include-side code doesn't have to do this...
#undef GMM_FORMAT
#undef GMM_FORMAT_INCLUDE_ASTC_FORMATS_ONLY
#undef GMM_FORMAT_INCLUDE_SURFACESTATE_FORMATS_ONLY
#undef GMM_FORMAT_GEN
#undef GMM_FORMAT_SKU
#undef GMM_FORMAT_WA
