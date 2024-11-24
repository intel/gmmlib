/*==============================================================================
Copyright(c) 2019 Intel Corporation

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
#include "Internal/Common/Platform/GmmGen12Platform.h"

/************************  RT->CCS Sizing definitions  ************************
    H/V/D Align and Downscale factor to obtain CCS from given RT dimensions
    Restrictions:
       CCS's RT (2D/3D YF) alignment to 4x1   (2D/3D YF) pages sharing 1x1 Aux$line
                (2D    YS)              2x2   ( 2D   YF) pages     "
                (3D    YS)              2x1x2 ( 3D   YF) pages     "   ie Slices share Aux$line
         (Color MSAA'd YF)              4x1   (MSAA'dYF) pages     "   ie all samples share Aux$line (Samples are array'd ie YF 4KB = YF-MSAA x MSAA-Samples)
         (Color MSAA 2x/4x YS)          2x2x1 ( 2D   YF) pages     "   ie Single sample per Aux$line
         (Color MSAA 8x YS)             1x2x2 ( 2D   YF) pages     "   ie 2 samples share Aux$line
         (Color MSAA 16x YS)            1x1x4 ( 2D   YF) pages     "   ie 4 samples share Aux$line
           (Depth MSAA YF)              4x1   ( 2D   YF) pages     "   ie all samples share Aux$line
         (Depth MSAA 2x/4x YS)          2x2x1 ( 2D   YF) pages     "   ie Single sample per Aux$line
         (Depth MSAA 8x YS)             1x2x2 ( 2D   YF) pages     "   ie 2 samples share Aux$line
         (Depth MSAA 16x YS)            1x1x4 ( 2D   YF) pages     "   ie 4 samples share Aux$line
         ie Depth/Color MSAA have common alignment, but due to different pixel packing (Depth MSS is interleaved, Color MSS is arrayed)
         SamplePerAux$line samples are X-major (for Depth), while Y-major (for Color) packed ie For Depth MSAA, Hdownscale *=SamplePerAux$line;
         for color MSAA, Vdownscale = Vdownscale; for both, MSAA-samples/SamplePerAux$line times sample shared CCS-size

         HAlign: Horizontal Align in pixels
         VAlign: Vertical Align in pixels
         DAlign: Depth Align in pixels
         HAlignxVAlignxDAlign [RT size] occupies one Aux$line
         SamplesPerAux$line: Samples sharing CCS; NSamples divisor on MSAA-samples giving multiple (on shared CCS) to cover all samples
         HDownscale: width divisor on CCSRTAlign`d width
         VDownscale: height divisor on CCSRTAlign`d height
     Convention:
         (+ve) HDownscale/VDownscale are downscale factors, and used as divisors
         (-ve) HDownscale/VDownscale are upscale factors, their absolute value used as multipliers
         ie if HDownscale etc is smaller than 1, its reciprocal is stored with -ve sign
                            <----  CCSRTALIGN  -----> <-- RT->CCS downscale-->
            (   TileMode,     HAlign   , VAlign, DAlign,   HDownscale, VDownscale)
                                                   or
                                      SamplesPerAux$line,
eg:
  CCSRTALIGN(TILE_YF_2D_8bpe,       256,     64,      1,         16,        16 )
**********************************************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
/// Allocates This function will initialize the necessary info based on platform.
///              - Buffer type restrictions (Eg: Z, Color, Display)
///              - X/Y tile dimensions
///
/// @param[in]  Platform: Contains information about platform to initialize an object
/////////////////////////////////////////////////////////////////////////////////////
GmmLib::PlatformInfoGen12::PlatformInfoGen12(PLATFORM &Platform, Context *pGmmLibContext)
    : PlatformInfoGen11(Platform, pGmmLibContext)
{
    __GMM_ASSERTPTR(pGmmLibContext, VOIDRETURN);

    //Compression format update
    GMM_RESOURCE_FORMAT GmmFormat;
#define GMM_FORMAT_SKU(FtrXxx) (pGmmLibContext->GetSkuTable().FtrXxx != 0)
#define GMM_FORMAT(Name, bpe, _Width, _Height, _Depth, IsRT, IsASTC, RcsSurfaceFormat, SSCompressionFmt, Availability) \
                                                                                                                       \
    {                                                                                                                  \
        GmmFormat                                                       = GMM_FORMAT_##Name;                           \
        Data.FormatTable[GmmFormat].CompressionFormat.CompressionFormat = static_cast<uint8_t>(SSCompressionFmt);      \
    }

#include "External/Common/GmmFormatTable.h"


    // --------------------------
    // Surface Alignment Units
    // --------------------------

    // 3DSTATE_DEPTH_BUFFER
    //======================================================================
    //   Surf Format       |     MSAA         |     HAlign    |   VAlign   |
    //======================================================================
    //     D16_UNORM       |    1x, 4x, 16x   |       8       |      8     |
    //     D16_UNORM       |      2x, 8x      |      16       |      4     |
    // Not D16_UNORM       |  1x,2x,4x,8x,16x |       8       |      4     |
    //======================================================================


    // 3DSTATE_STENCIL_BUFFER
    //======================================================================
    //   Surf Format       |     MSAA         |     HAlign    |   VAlign   |
    //======================================================================
    //       N/A           |       N/A        |      16       |      8     |
    //======================================================================

    Data.SurfaceMaxSize                      = GMM_GBYTE(16384);
    Data.MaxGpuVirtualAddressBitsPerResource = 44;

    //Override the Height VP9 VdEnc requirement for Gen12 16k resolution.
    Data.ReconMaxHeight = GMM_KBYTE(48);
    Data.ReconMaxWidth  = GMM_KBYTE(32);

    if((GFX_GET_CURRENT_PRODUCT(Data.Platform) >= IGFX_DG1))
    {
        Data.HiZPixelsPerByte = 4;
    }

    Data.TexAlign.Depth.Width                      = 8; // Not D16_UNORM
    Data.TexAlign.Depth.Height                     = 4;
    Data.TexAlign.Depth_D16_UNORM_1x_4x_16x.Width  = 8;
    Data.TexAlign.Depth_D16_UNORM_1x_4x_16x.Height = 8;
    Data.TexAlign.Depth_D16_UNORM_2x_8x.Width      = 16;
    Data.TexAlign.Depth_D16_UNORM_2x_8x.Height     = 4;
    Data.TexAlign.SeparateStencil.Width            = 16;
    Data.TexAlign.SeparateStencil.Height           = 8;

    //CCS unit size ie cacheline
    Data.TexAlign.CCS.Align.Width     = 16;
    Data.TexAlign.CCS.Align.Height    = 4;
    Data.TexAlign.CCS.Align.Depth     = 1;
    Data.TexAlign.CCS.MaxPitchinTiles = 1024;

    // clang-format off
    SET_TILE_MODE_INFO(TILE4,                        128,       32,        1,         0,         0,         0)

    // TILE__64 1D
    SET_TILE_MODE_INFO(TILE__64_1D_128bpe,       4096,        1,        1,      1024,         1,         1)
    SET_TILE_MODE_INFO(TILE__64_1D_64bpe,        8192,        1,        1,      2048,         1,         1)
    SET_TILE_MODE_INFO(TILE__64_1D_32bpe,       16384,        1,        1,      4096,         1,         1)
    SET_TILE_MODE_INFO(TILE__64_1D_16bpe,       32768,        1,        1,      8192,         1,         1)
    SET_TILE_MODE_INFO(TILE__64_1D_8bpe,        65536,        1,        1,     16384,         1,         1)

    // TILE__64 2D
    SET_TILE_MODE_INFO(TILE__64_2D_128bpe,       1024,       64,        1,        32,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_64bpe,        1024,       64,        1,        64,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_32bpe,         512,      128,        1,        64,       128,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_16bpe,         512,      128,        1,       128,       128,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_8bpe,          256,      256,        1,       128,       256,         1)

    // TILE__64 2D 2X
    if(pGmmLibContext->GetSkuTable().FtrXe2PlusTiling)
    {
        SET_TILE_MODE_INFO(TILE__64_2D_2X_128bpe,     1024,      32,        1,        32,        32,         1) 
    }
    else
    {
        SET_TILE_MODE_INFO(TILE__64_2D_2X_128bpe,     512,       64,        1,        32,        32,         1) 
    }

    SET_TILE_MODE_INFO(TILE__64_2D_2X_64bpe,      512,       64,        1,        64,        32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_2X_32bpe,      256,      128,        1,        64,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_2X_16bpe,      256,      128,        1,       128,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_2X_8bpe,       128,      256,        1,       128,       128,         1)

    // TILE__64 2D 4X
    SET_TILE_MODE_INFO(TILE__64_2D_4X_128bpe,     512,       32,        1,        16,        32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_4X_64bpe,      512,       32,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_4X_32bpe,      256,       64,        1,        32,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_4X_16bpe,      256,       64,        1,        64,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_4X_8bpe,       128,      128,        1,        64,       128,         1)

    // TILE__64 2D 8X
    SET_TILE_MODE_INFO(TILE__64_2D_8X_128bpe,     256,       32,        1,        8,         32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_8X_64bpe,      256,       32,        1,        16,        32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_8X_32bpe,      256,       32,        1,        16,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_8X_16bpe,      128,       64,        1,        32,        64,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_8X_8bpe,       128,       64,        1,        32,       128,         1)

    // TILE__64 2D 16X
    SET_TILE_MODE_INFO(TILE__64_2D_16X_128bpe,     256,       16,        1,        8,         16,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_16X_64bpe,      128,       32,        1,        16,        16,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_16X_32bpe,      128,       32,        1,        16,        32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_16X_16bpe,      128,       32,        1,        32,        32,         1)
    SET_TILE_MODE_INFO(TILE__64_2D_16X_8bpe,        64,       64,        1,        32,        64,         1)

    // TILE__64 3D
    SET_TILE_MODE_INFO(TILE__64_3D_128bpe,        256,       16,       16,         8,        16,         16)
    SET_TILE_MODE_INFO(TILE__64_3D_64bpe,         256,       16,       16,        16,        16,         16)
    SET_TILE_MODE_INFO(TILE__64_3D_32bpe,         128,       32,       16,        16,        32,         16)
    SET_TILE_MODE_INFO(TILE__64_3D_16bpe,          64,       32,       32,        16,        32,         32)
    SET_TILE_MODE_INFO(TILE__64_3D_8bpe,           64,       32,       32,        32,        32,         32)
    // clang-format off

//Extended CCS alignment for per bpp/Tiling CCS alignment
#define CCSRTALIGN(TileMode, HAlign, VAlign, DAlign, HDownscale, VDownscale) \
    {                                                                        \
        TexAlignEx.CCSEx[CCS_MODE(TileMode)].Align.Width      = HAlign;      \
        TexAlignEx.CCSEx[CCS_MODE(TileMode)].Align.Height     = VAlign;      \
        TexAlignEx.CCSEx[CCS_MODE(TileMode)].Align.Depth      = DAlign;      \
        TexAlignEx.CCSEx[CCS_MODE(TileMode)].Downscale.Width  = HDownscale;  \
        TexAlignEx.CCSEx[CCS_MODE(TileMode)].Downscale.Height = VDownscale;  \
        TexAlignEx.CCSEx[CCS_MODE(TileMode)].Downscale.Depth  = DAlign;      \
    }

    // clang-format off
//See "RT->CCS Sizing definitions" comments above for explanation on fields
/********* TileMode           HAlign,  VAlign,  DAlign,   HDownscale,  VDownscale ***/
CCSRTALIGN(TILE_YF_2D_8bpe,       256,     64,      1,         16,        16 );
CCSRTALIGN(TILE_YF_2D_16bpe,      256,     32,      1,         16,         8 );
CCSRTALIGN(TILE_YF_2D_32bpe,      128,     32,      1,          8,         8 );
CCSRTALIGN(TILE_YF_2D_64bpe,      128,     16,      1,          8,         4 );
CCSRTALIGN(TILE_YF_2D_128bpe,      64,     16,      1,          4,         4 );

CCSRTALIGN(TILE_YF_3D_8bpe,        64,     16,     16,          4,         4 );
CCSRTALIGN(TILE_YF_3D_16bpe,       32,     16,     16,          2,         4 );
CCSRTALIGN(TILE_YF_3D_32bpe,       32,     16,      8,          2,         4 );
CCSRTALIGN(TILE_YF_3D_64bpe,       32,      8,      8,          2,         2 );
CCSRTALIGN(TILE_YF_3D_128bpe,      16,      8,      8,          1,         2 );

CCSRTALIGN(TILE_YF_2D_2X_8bpe,    128,     64,      2,          8,        16 );
CCSRTALIGN(TILE_YF_2D_2X_16bpe,   128,     32,      2,          8,         8 );
CCSRTALIGN(TILE_YF_2D_2X_32bpe,    64,     32,      2,          4,         8 );
CCSRTALIGN(TILE_YF_2D_2X_64bpe,    64,     16,      2,          4,         4 );
CCSRTALIGN(TILE_YF_2D_2X_128bpe,   32,     16,      2,          2,         4 );

CCSRTALIGN(TILE_YF_2D_4X_8bpe,    128,     32,      4,          8,         8 );
CCSRTALIGN(TILE_YF_2D_4X_16bpe,   128,     16,      4,          8,         4 );
CCSRTALIGN(TILE_YF_2D_4X_32bpe,    64,     16,      4,          4,         4 );
CCSRTALIGN(TILE_YF_2D_4X_64bpe,    64,      8,      4,          4,         2 );
CCSRTALIGN(TILE_YF_2D_4X_128bpe,   32,      8,      4,          2,         2 );

CCSRTALIGN(TILE_YF_2D_8X_8bpe,     64,     32,      8,          4,         8 );
CCSRTALIGN(TILE_YF_2D_8X_16bpe,    64,     16,      8,          4,         4 );
CCSRTALIGN(TILE_YF_2D_8X_32bpe,    32,     16,      8,          2,         4 );
CCSRTALIGN(TILE_YF_2D_8X_64bpe,    32,      8,      8,          2,         2 );
CCSRTALIGN(TILE_YF_2D_8X_128bpe,   16,      8,      8,          1,         2 );

CCSRTALIGN(TILE_YF_2D_16X_8bpe,    64,     16,     16,          4,         4 );
CCSRTALIGN(TILE_YF_2D_16X_16bpe,   64,      8,     16,          4,         2 );
CCSRTALIGN(TILE_YF_2D_16X_32bpe,   32,      8,     16,          2,         2 );
CCSRTALIGN(TILE_YF_2D_16X_64bpe,   32,      4,     16,          2,         1 );
CCSRTALIGN(TILE_YF_2D_16X_128bpe,  16,      4,     16,          1,         1 );

CCSRTALIGN(TILE_YS_2D_8bpe,       128,    128,      1,          8,        32 );
CCSRTALIGN(TILE_YS_2D_16bpe,      128,     64,      1,          8,        16 );
CCSRTALIGN(TILE_YS_2D_32bpe,       64,     64,      1,          4,        16 );
CCSRTALIGN(TILE_YS_2D_64bpe,       64,     32,      1,          4,         8 );
CCSRTALIGN(TILE_YS_2D_128bpe,      32,     32,      1,          2,         8 );

CCSRTALIGN(TILE_YS_3D_8bpe,        32,     16,     32,          2,         4 );
CCSRTALIGN(TILE_YS_3D_16bpe,       16,     16,     32,          1,         4 );
CCSRTALIGN(TILE_YS_3D_32bpe,       16,     16,     16,          1,         4 );
CCSRTALIGN(TILE_YS_3D_64bpe,       16,      8,     16,          1,         2 );
CCSRTALIGN(TILE_YS_3D_128bpe,       8,      8,     16,         -2,         2 );

CCSRTALIGN(TILE_YS_2D_2X_8bpe,    128,    128,      1,          8,        32 );
CCSRTALIGN(TILE_YS_2D_2X_16bpe,   128,     64,      1,          8,        16 );
CCSRTALIGN(TILE_YS_2D_2X_32bpe,    64,     64,      1,          4,        16 );
CCSRTALIGN(TILE_YS_2D_2X_64bpe,    64,     32,      1,          4,         8 );
CCSRTALIGN(TILE_YS_2D_2X_128bpe,   32,     32,      1,          2,         8 );

CCSRTALIGN(TILE_YS_2D_4X_8bpe,    128,    128,      1,          8,        32 );
CCSRTALIGN(TILE_YS_2D_4X_16bpe,   128,     64,      1,          8,        16 );
CCSRTALIGN(TILE_YS_2D_4X_32bpe,    64,     64,      1,          4,        16 );
CCSRTALIGN(TILE_YS_2D_4X_64bpe,    64,     32,      1,          4,         8 );
CCSRTALIGN(TILE_YS_2D_4X_128bpe,   32,     32,      1,          2,         8 );

CCSRTALIGN(TILE_YS_2D_8X_8bpe,     64,    128,      2,          4,        32 );
CCSRTALIGN(TILE_YS_2D_8X_16bpe,    64,     64,      2,          4,        16 );
CCSRTALIGN(TILE_YS_2D_8X_32bpe,    32,     64,      2,          2,        16 );
CCSRTALIGN(TILE_YS_2D_8X_64bpe,    32,     32,      2,          2,         8 );
CCSRTALIGN(TILE_YS_2D_8X_128bpe,   16,     32,      2,          1,         8 );

CCSRTALIGN(TILE_YS_2D_16X_8bpe,    64,     64,      4,          4,        16 );
CCSRTALIGN(TILE_YS_2D_16X_16bpe,   64,     32,      4,          4,         8 );
CCSRTALIGN(TILE_YS_2D_16X_32bpe,   32,     32,      4,          2,         8 );
CCSRTALIGN(TILE_YS_2D_16X_64bpe,   32,     16,      4,          2,         4 );
CCSRTALIGN(TILE_YS_2D_16X_128bpe,  16,     16,      4,          1,         4 );
#undef CCSRTALIGN
// clang-format on

#define FCRECTALIGN(TileMode, bpp, HAlign, VAlign, DAlign, HDownscale, VDownscale) \
    {                                                                              \
        FCTileMode[FCMode(TileMode, bpp)].Align.Width      = HAlign;               \
        FCTileMode[FCMode(TileMode, bpp)].Align.Height     = VAlign;               \
        FCTileMode[FCMode(TileMode, bpp)].Align.Depth      = DAlign;               \
        FCTileMode[FCMode(TileMode, bpp)].Downscale.Width  = HDownscale;           \
        FCTileMode[FCMode(TileMode, bpp)].Downscale.Height = VDownscale;           \
        FCTileMode[FCMode(TileMode, bpp)].Downscale.Depth  = 1;                    \
    }

    // clang-format off
FCRECTALIGN(LEGACY_TILE_Y    ,   8, 512,  32, 1,  256, 16);
FCRECTALIGN(LEGACY_TILE_Y    ,  16, 256,  32, 1, 128, 16);
FCRECTALIGN(LEGACY_TILE_Y    ,  32, 128,  32, 1,  64, 16);
FCRECTALIGN(LEGACY_TILE_Y    ,  64,  64,  32, 1, 32, 16);
FCRECTALIGN(LEGACY_TILE_Y    , 128,  32,  32, 1, 16, 16);

FCRECTALIGN(TILE_YF_2D_8bpe  ,   8, 256,  64, 1, 128, 32);
FCRECTALIGN(TILE_YF_2D_16bpe ,  16, 256,  32, 1, 128, 16);
FCRECTALIGN(TILE_YF_2D_32bpe ,  32, 128,  32, 1,  64, 16);
FCRECTALIGN(TILE_YF_2D_64bpe ,  64, 128,  16, 1,  64,  8);
FCRECTALIGN(TILE_YF_2D_128bpe, 128,  64,  16, 1,  32,  8);

FCRECTALIGN(TILE_YS_2D_8bpe  ,   8, 128, 128,  1, 64, 64);
FCRECTALIGN(TILE_YS_2D_16bpe ,  16, 128,  64,  1, 64, 32);
FCRECTALIGN(TILE_YS_2D_32bpe ,  32,  64,  64,  1, 32, 32);
FCRECTALIGN(TILE_YS_2D_64bpe ,  64,  64,  32,  1, 32, 16);
FCRECTALIGN(TILE_YS_2D_128bpe, 128,  32,  32,  1, 16, 16);

if(pGmmLibContext->GetSkuTable().FtrXe2Compression)
{   

    FCRECTALIGN(TILE4           ,   8, 64, 4, 1, 64, 4);
    FCRECTALIGN(TILE4           ,  16, 32, 4, 1, 32, 4);
    FCRECTALIGN(TILE4           ,  32, 16, 4, 1, 16, 4);
    FCRECTALIGN(TILE4           ,  64, 8,  4, 1, 8, 4);
    FCRECTALIGN(TILE4           , 128, 4,  4, 1, 4, 4);

    FCRECTALIGN(TILE__64_2D_8bpe  ,   8, 64, 4, 1, 64, 4);
    FCRECTALIGN(TILE__64_2D_16bpe ,  16, 32, 4, 1, 32, 4);
    FCRECTALIGN(TILE__64_2D_32bpe ,  32, 16, 4, 1, 16, 4);
    FCRECTALIGN(TILE__64_2D_64bpe ,  64,  8, 4, 1, 8,  4);
    FCRECTALIGN(TILE__64_2D_128bpe, 128,  4, 4, 1, 4,  4);

    FCRECTALIGN(TILE__64_3D_8bpe  ,   8,  64,  32, 32,  4, 8);
    FCRECTALIGN(TILE__64_3D_16bpe ,  16,  32,  32, 32,  8, 4);
    FCRECTALIGN(TILE__64_3D_32bpe ,  32,  32,  32, 16,  4, 4);
    FCRECTALIGN(TILE__64_3D_64bpe ,  64,  32,  16, 16,  4, 4);
    FCRECTALIGN(TILE__64_3D_128bpe, 128,  16,  16, 16,  4, 4);
}
else
{
    FCRECTALIGN(TILE4           ,   8, 1024, 16,  1, 1024, 16);
    FCRECTALIGN(TILE4           ,  16,  512, 16,  1, 512, 16);
    FCRECTALIGN(TILE4           ,  32,  256,  16, 1,  256, 16);
    FCRECTALIGN(TILE4           ,  64,  128,  16, 1, 128, 16);
    FCRECTALIGN(TILE4           , 128,   64,  16, 1,  64, 16);

    FCRECTALIGN(TILE__64_2D_8bpe  ,   8, 128, 128, 1,  128, 128);
    FCRECTALIGN(TILE__64_2D_16bpe ,  16, 128,  64, 1,  128, 64);
    FCRECTALIGN(TILE__64_2D_32bpe ,  32,  64,  64, 1,   64, 64);
    FCRECTALIGN(TILE__64_2D_64bpe ,  64,  64,  32, 1,  64, 32);
    FCRECTALIGN(TILE__64_2D_128bpe, 128,  32,  32, 1,  32, 32);

    FCRECTALIGN(TILE__64_3D_8bpe  ,   8,  1,  1, 1,  1, 1);
    FCRECTALIGN(TILE__64_3D_16bpe ,  16,  1,  1, 1,  1, 1);
    FCRECTALIGN(TILE__64_3D_32bpe ,  32,  1,  1, 1,  1, 1);
    FCRECTALIGN(TILE__64_3D_64bpe ,  64,  1,  1, 1,  1, 1);
    FCRECTALIGN(TILE__64_3D_128bpe, 128,  1,  1, 1,  1, 1);

    
}
#undef FCRECTALIGN

    // clang-format on
    Data.NoOfBitsSupported                = 39;
    Data.HighestAcceptablePhysicalAddress = GFX_MASK_LARGE(0, 38);
	
    if (GFX_GET_CURRENT_PRODUCT(Data.Platform) >= IGFX_BMG)
    {
        Data.NoOfBitsSupported                = 52;
        Data.HighestAcceptablePhysicalAddress = GFX_MASK_LARGE(0, 51);
    }
    if(GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_PVC)
    {
        Data.NoOfBitsSupported                = 52;
        Data.HighestAcceptablePhysicalAddress = GFX_MASK_LARGE(0, 51);
    }
    else if(GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_ALDERLAKE_S ||
       (GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_ALDERLAKE_P) || 
       (GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_ALDERLAKE_N) ||
       (GFX_GET_CURRENT_PRODUCT(Data.Platform) >= IGFX_XE_HP_SDV))
    {
        Data.NoOfBitsSupported                = 46;
        Data.HighestAcceptablePhysicalAddress = GFX_MASK_LARGE(0, 45);
    }
}

void GmmLib::PlatformInfoGen12::ApplyExtendedTexAlign(uint32_t CCSMode, ALIGNMENT &UnitAlign)
{
    if(CCSMode < CCS_MODES)
    {
        UnitAlign.Width  = TexAlignEx.CCSEx[CCSMode].Align.Width;
        UnitAlign.Height = TexAlignEx.CCSEx[CCSMode].Align.Height;
        UnitAlign.Depth  = TexAlignEx.CCSEx[CCSMode].Align.Depth;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Copies parameters or sets flags based on info sent by the client.
///
/// @param[in]  CreateParams: Flags which specify what sort of resource to create
/////////////////////////////////////////////////////////////////////////////////////
void GmmLib::PlatformInfoGen12::SetCCSFlag(GMM_RESOURCE_FLAG &Flags)
{
    if(Flags.Gpu.MMC)
    {
        Flags.Gpu.CCS = Flags.Gpu.MMC;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the MMC parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfoGen12::ValidateMMC(GMM_TEXTURE_INFO &Surf)
{
    if(Surf.Flags.Gpu.MMC && //For Media Memory Compression --
       (!(GMM_IS_4KB_TILE(Surf.Flags) || GMM_IS_64KB_TILE(Surf.Flags)) &&
        (!Surf.Flags.Gpu.__NonMsaaLinearCCS)))
    {
        return 0;
    }
    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfoGen12::ValidateCCS(GMM_TEXTURE_INFO &Surf)
{

    if (!(                                    //--- Legitimate CCS Case ----------------------------------------
        ((Surf.Flags.Gpu.ProceduralTexture || //procedural texture, or compressed surface (no more separate Aux-CCS)
          Surf.Flags.Info.RenderCompressed || Surf.Flags.Info.MediaCompressed) ||
         pGmmLibContext->GetSkuTable().FtrXe2Compression && !Surf.Flags.Info.NotCompressed) &&
        (((Surf.Type >= RESOURCE_2D && Surf.Type <= RESOURCE_CUBE) &&       //Not supported: 1D (until Flat CCS); Others Supported: Buffer, 2D, 3D, cube, Arrays, mip-maps, MSAA, Depth/Stencil
          (GMM_IS_4KB_TILE(Surf.Flags) || GMM_IS_64KB_TILE(Surf.Flags))) || //Only on 2D + Y/Ys or Lienar buffer (until Flat CCS)
         (Surf.Flags.Info.Linear && Surf.Type == RESOURCE_BUFFER) ||
         ((pGmmLibContext->GetSkuTable().FtrFlatPhysCCS) && !Surf.Flags.Info.TiledX))))
    {
        GMM_ASSERTDPF(0, "Invalid CCS usage!");
        return 0;
    }

    if (!pGmmLibContext->GetSkuTable().FtrFlatPhysCCS &&
        Surf.Flags.Info.Linear && Surf.Type == RESOURCE_BUFFER && !Surf.Flags.Info.RenderCompressed)
    {
        GMM_ASSERTDPF(0, "Invalid CCS usage - MLC only supported as RC!");
        return 0;
    }

    //Compressed resource (main surf) must pre-define MC/RC type
    if(!(Surf.Flags.Gpu.__NonMsaaTileYCcs || Surf.Flags.Gpu.__NonMsaaLinearCCS) &&
       !Surf.Flags.Gpu.ProceduralTexture &&
        !(Surf.Flags.Info.RenderCompressed || Surf.Flags.Info.MediaCompressed || !Surf.Flags.Info.NotCompressed))
    {
        GMM_ASSERTDPF(0, "Invalid CCS usage - RC/MC type unspecified!");
        return 0;
    }

    if(Surf.Flags.Info.RenderCompressed && Surf.Flags.Info.MediaCompressed)
    {
        GMM_ASSERTDPF(0, "Invalid CCS usage - can't be both RC and MC!");
        return 0;
    }

    if(!pGmmLibContext->GetSkuTable().FtrLinearCCS &&
       (Surf.Type == RESOURCE_3D || Surf.MaxLod > 0 || Surf.MSAA.NumSamples > 1 ||
        !(Surf.Flags.Info.TiledYf || GMM_IS_64KB_TILE(Surf.Flags))))
    {
        GMM_ASSERTDPF(0, "CCS support for (volumetric, mip'd, MSAA'd, TileY) resources only enabled with Linear CCS!");
        return 0;
    }

    GMM_ASSERTDPF((Surf.Flags.Wa.PreGen12FastClearOnly == 0), "FastClear Only unsupported on Gen12+!");
    Surf.Flags.Wa.PreGen12FastClearOnly = 0;

    return 1;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Validates the UnifiedAuxSurface parameters passed in by clients to make sure they do not
/// conflict or ask for unsupporting combinations/features.
///
/// @param[in]  GMM_TEXTURE_INFO which specify what sort of resource to create
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::PlatformInfoGen12::ValidateUnifiedAuxSurface(GMM_TEXTURE_INFO &Surf)
{

    if ((Surf.Flags.Gpu.UnifiedAuxSurface) &&
        !( //--- Legitimate UnifiedAuxSurface Case ------------------------------------------
        (Surf.Flags.Gpu.CCS &&
         (Surf.MSAA.NumSamples <= 1 && (Surf.Flags.Gpu.RenderTarget || Surf.Flags.Gpu.Texture))) ||
        (Surf.Flags.Gpu.Depth || Surf.Flags.Gpu.SeparateStencil || Surf.MSAA.NumSamples > 1)))
    {
        GMM_ASSERTDPF(0, "Invalid UnifiedAuxSurface usage!");
        return 0;
    }

    return 1;
}
//=============================================================================
//
// Function: CheckFmtDisplayDecompressible
//
// Desc: Returns true if display hw supports lossless render/media decompression
//       else returns false. Restrictions are from 
//       Umds can call it to decide if full resolve is required
//
// Parameters:
//      See function arguments.
//
// Returns:
//      uint8_t
//-----------------------------------------------------------------------------
uint8_t GmmLib::PlatformInfoGen12::CheckFmtDisplayDecompressible(GMM_TEXTURE_INFO &Surf,
                                                                 bool              IsSupportedRGB64_16_16_16_16,
                                                                 bool              IsSupportedRGB32_8_8_8_8,
                                                                 bool              IsSupportedRGB32_2_10_10_10,
                                                                 bool              IsSupportedMediaFormats)
{

    //Check fmt is display decompressible
    if (((Surf.Flags.Info.RenderCompressed || Surf.Flags.Info.MediaCompressed || !Surf.Flags.Info.NotCompressed) &&
        (IsSupportedRGB64_16_16_16_16 ||                             //RGB64 16:16 : 16 : 16 FP16
         IsSupportedRGB32_8_8_8_8 ||                                 //RGB32 8 : 8 : 8 : 8
         IsSupportedRGB32_2_10_10_10)) ||                            //RGB32 2 : 10 : 10 : 10) ||
       (Surf.Flags.Info.MediaCompressed && IsSupportedMediaFormats)) //YUV444 - Y412, Y416
    {
        //Display supports compression on TileY, but not Yf/Ys (deprecated for display support)
        if(GMM_IS_4KB_TILE(Surf.Flags) &&
           !(Surf.Flags.Info.TiledYf || GMM_IS_64KB_TILE(Surf.Flags)))
        {
            return true;
        }
    }
    return false;
}

//=============================================================================
//
// Function: OverrideCompressionFormat
//
// Desc: SurfaceState compression format encoding differ for MC vs RC on few formats. This function
//       overrides default RC encoding for MC requests
//
// Parameters:
//      See function arguments.
//
// Returns:
//      uint8_t
//-----------------------------------------------------------------------------
uint8_t GmmLib::PlatformInfoGen12::OverrideCompressionFormat(GMM_RESOURCE_FORMAT Format, uint8_t IsMC)
{

    uint8_t CompressionFormat = Data.FormatTable[Format].CompressionFormat.CompressionFormat;
    if (pGmmLibContext->GetSkuTable().FtrXe2Compression)
    {
        if ((CompressionFormat < GMM_XE2_UNIFIED_COMP_MIN_FORMAT) ||
            (CompressionFormat > GMM_XE2_UNIFIED_COMP_MAX_FORMAT))
        {
            CompressionFormat = GMM_XE2_UNIFIED_COMP_FORMAT_INVALID;
        }
        __GMM_ASSERT(CompressionFormat != GMM_XE2_UNIFIED_COMP_FORMAT_INVALID);
    }
    else if (pGmmLibContext->GetSkuTable().FtrFlatPhysCCS || pGmmLibContext->GetSkuTable().FtrUnified3DMediaCompressionFormats)
    {
        if(!IsMC &&
           !pGmmLibContext->GetSkuTable().FtrUnified3DMediaCompressionFormats &&
           (CompressionFormat < GMM_FLATCCS_MIN_RC_FORMAT ||
            CompressionFormat > GMM_FLATCCS_MAX_RC_FORMAT))
        {
            CompressionFormat = GMM_FLATCCS_FORMAT_INVALID;
        }

	if(!IsMC &&
           pGmmLibContext->GetSkuTable().FtrUnified3DMediaCompressionFormats &&
           (CompressionFormat < GMM_UNIFIED_COMP_MIN_RC_FORMAT ||
            CompressionFormat > GMM_UNIFIED_COMP_MAX_RC_FORMAT))
        {
            CompressionFormat = GMM_UNIFIED_COMP_FORMAT_INVALID;
        }
	else if(IsMC)
        {
            if(!pGmmLibContext->GetSkuTable().FtrUnified3DMediaCompressionFormats)
            {
		if(CompressionFormat >= GMM_FLATCCS_MIN_MC_FORMAT && CompressionFormat <= GMM_FLATCCS_MAX_MC_FORMAT)
                {
                    //True MC format encodings, drop MC-identify bit (ie bit5)
                    CompressionFormat -= (GMM_FLATCCS_MIN_MC_FORMAT - 1);
                }
                else
                {
                    // RC format encoding, needs MC format encoding for MC usage
                    switch(CompressionFormat)
                    {
                        case GMM_FLATCCS_FORMAT_RGB10A2:
                            CompressionFormat = GMM_FLATCCS_FORMAT_RGB10b;
                            break;
                        case GMM_FLATCCS_FORMAT_RGBA16U:
                            CompressionFormat = GMM_FLATCCS_FORMAT_RGBA16_MEDIA;
                            break;
                        case GMM_FLATCCS_FORMAT_RGBA8U:
                            CompressionFormat = GMM_FLATCCS_FORMAT_ARGB8b;
                            break;
                        default:
                            if(CompressionFormat < GMM_FLATCCS_MIN_MC_FORMAT || CompressionFormat > GMM_FLATCCS_MAX_MC_FORMAT)
                            {
                                CompressionFormat = GMM_FLATCCS_FORMAT_INVALID;
                            }
                            break;
                    }

                    if(CompressionFormat != GMM_FLATCCS_FORMAT_INVALID)
                    {
                        //drop MC-identify bit (ie bit 5)
                        CompressionFormat -= (GMM_FLATCCS_MIN_MC_FORMAT - 1);
                    }
                }
	    }
            else
            {
                if(CompressionFormat >= GMM_UNIFIED_COMP_MIN_MC_FORMAT && CompressionFormat <= GMM_UNIFIED_COMP_MAX_MC_FORMAT)
                {
                    //True MC format encodings, drop MC-identify bit (ie bit5)
                    CompressionFormat -= (GMM_UNIFIED_COMP_MIN_MC_FORMAT - 1);
                }
                else
                {
                    // RC format encoding, needs MC format encoding for MC usage
                    switch(CompressionFormat)
                    {
                        case GMM_UNIFIED_COMP_FORMAT_RGB10A2:
                            CompressionFormat = GMM_UNIFIED_COMP_FORMAT_RGB10b;
                            break;
                        case GMM_UNIFIED_COMP_FORMAT_RGBA16U:
                        case GMM_UNIFIED_COMP_FORMAT_RGBA16F:
                            CompressionFormat = GMM_UNIFIED_COMP_FORMAT_RGBA16_MEDIA;
                            break;
                        case GMM_UNIFIED_COMP_FORMAT_RGBA8U:
                        case GMM_UNIFIED_COMP_FORMAT_RGBA8S:
                            CompressionFormat = GMM_UNIFIED_COMP_FORMAT_ARGB8b;
                            break;
                        default:
                            if(CompressionFormat < GMM_UNIFIED_COMP_MIN_MC_FORMAT || CompressionFormat > GMM_UNIFIED_COMP_MAX_MC_FORMAT)
                            {
                                CompressionFormat = GMM_UNIFIED_COMP_FORMAT_INVALID;
                            }
                            break;
                    }

                    if(CompressionFormat != GMM_UNIFIED_COMP_FORMAT_INVALID)
                    {
                        //drop MC-identify bit (ie bit 5)
                        CompressionFormat -= (GMM_UNIFIED_COMP_MIN_MC_FORMAT - 1);
                    }
		}
	    }
        }

	//Assert if out of MC/RC encoding range -ie format-table must be corrected for Compression code
        __GMM_ASSERT(!pGmmLibContext->GetSkuTable().FtrUnified3DMediaCompressionFormats && CompressionFormat != GMM_FLATCCS_FORMAT_INVALID ||
                     pGmmLibContext->GetSkuTable().FtrUnified3DMediaCompressionFormats && CompressionFormat != GMM_UNIFIED_COMP_FORMAT_INVALID);
    }

    return CompressionFormat;
}
