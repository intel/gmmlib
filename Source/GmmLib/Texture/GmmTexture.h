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

#include "External/Common/GmmInternal.h"             // UMD or KMD Windows header
#include "External/Common/GmmTextureExt.h"
#include "External/Common/GmmUtil.h"
#include "External/Common/GmmInfoExt.h"
#include "External/Common/GmmInfo.h"
#include "External/Common/GmmProto.h"
#ifdef __cplusplus
#include "Internal/Common/Texture/GmmTextureCalc.h"
//---------------------------------------------------------------------------
// ExpandWidth/Height Wrappers
//
// Gen7+ MSAA (non-Depth/Stencil) render targets use array expansion instead of
// Width/Height expansion--So they pass NumSamples=1 to __GmmExpandXxx functions.
//
//---------------------------------------------------------------------------
#define __GMM_EXPAND_Xxx(ptr, Xxx, Dimension, UnitAlignment, pTexInfo)                \
     (ptr)->Expand##Xxx(                                                        \
        (Dimension), (UnitAlignment),                                                 \
        ((pTexInfo)->Flags.Gpu.Depth || (pTexInfo)->Flags.Gpu.SeparateStencil) ?      \
            (pTexInfo)->MSAA.NumSamples : 1)

#define __GMM_EXPAND_WIDTH(ptr, __Width, UnitAlignment, pTexInfo) \
    __GMM_EXPAND_Xxx(ptr, Width, __Width, UnitAlignment, pTexInfo)

#define __GMM_EXPAND_HEIGHT(ptr, __Height, UnitAlignment, pTexInfo) \
    __GMM_EXPAND_Xxx(ptr, Height, __Height, UnitAlignment, pTexInfo)

//=============================================================================
//Function:
//    __GmmTexFillHAlignVAlign
//
//Description:
//     Stores in pTexInfo the appropriate unit aligment sizes.
//
//-----------------------------------------------------------------------------
// Gmmlib 2.0 TODO[Low] Move to Class and Inline function handling.
GMM_INLINE GMM_STATUS __GmmTexFillHAlignVAlign(GMM_TEXTURE_INFO *pTexInfo)
{
    uint32_t                   UnitAlignWidth = 0;
    uint32_t                   UnitAlignHeight = 0;
    uint32_t                   UnitAlignDepth = 0;
    const GMM_PLATFORM_INFO *pPlatform;
    GMM_TEXTURE_CALC        *pTextureCalc;

    GMM_DPF_ENTER;

    __GMM_ASSERTPTR(pTexInfo, GMM_ERROR);
    __GMM_ASSERTPTR(pGmmGlobalContext, GMM_ERROR);

#define SET_ALIGN_FACTOR(Xxx, Bytes)                            \
    if(!pGmmGlobalContext->GetSkuTable().FtrTileY)              \
    {                                                           \
        UnitAlign##Xxx =                                        \
            (pTexInfo->BitsPerPixel == 128) ? Bytes/16 :        \
            (pTexInfo->BitsPerPixel ==  64) ? Bytes/8 :         \
            (pTexInfo->BitsPerPixel ==  32) ? Bytes/4 :         \
            (pTexInfo->BitsPerPixel ==  16) ? Bytes/2 : Bytes ; \
                                                                \
        if(!pTexInfo->Flags.Info.Linear &&  \
           (pTexInfo->BitsPerPixel == 24 || pTexInfo->BitsPerPixel == 48 || pTexInfo->BitsPerPixel == 96)) \
        {                                                       \
            UnitAlign##Xxx = 16;                                \
        }                                                       \
        else if (pTexInfo->Flags.Info.Linear && \
            (pTexInfo->BitsPerPixel == 24 || pTexInfo->BitsPerPixel == 48 || pTexInfo->BitsPerPixel == 96))\
        {                                                       \
            UnitAlign##Xxx = 128;                               \
        }                                                       \
    }

    if (!((pTexInfo->Format > GMM_FORMAT_INVALID) &&
        (pTexInfo->Format < GMM_RESOURCE_FORMATS)))
    {
        GMM_DPF_CRITICAL("Invalid Resource Format");
        return GMM_ERROR;
    }

    if( !pTexInfo->Alignment.HAlign &&
        !pTexInfo->Alignment.VAlign)
    {
        pPlatform = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo);
        pTextureCalc = GMM_OVERRIDE_TEXTURE_CALC(pTexInfo);

        /// SKL TiledYf/Ys Surfaces //////////////////////////////////////////
        if( ((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE) &&
             (pTexInfo->Flags.Info.TiledYf || GMM_IS_64KB_TILE(pTexInfo->Flags))))
        {
            #define SET_ALIGN_INFO(Xxx, A1, A2, A3, A4, A5)     \
               UnitAlign##Xxx =                                 \
                    (pTexInfo->BitsPerPixel == 128) ? A1 :      \
                    (pTexInfo->BitsPerPixel ==  64) ? A2 :      \
                    (pTexInfo->BitsPerPixel ==  32) ? A3 :      \
                    (pTexInfo->BitsPerPixel ==  16) ? A4 : A5;  \

            if(pTexInfo->Type == RESOURCE_1D)
            {
                if(pTexInfo->Flags.Info.TiledYf)
                {
                    SET_ALIGN_INFO(Width, 256, 512, 1024, 2048, 4096);
                }
                else // if(pTexInfo->Flags.Info.TiledYs)
                {
                    SET_ALIGN_INFO(Width, 4096, 8192, 16384, 32768, 65536);
                }
            }
            else if((pTexInfo->Type == RESOURCE_2D) || (pTexInfo->Type == RESOURCE_CUBE) ||
                    (pTexInfo->Type == RESOURCE_PRIMARY))
            {
                if(pTexInfo->Flags.Info.TiledYf)
                {
                    SET_ALIGN_INFO(Width,  16, 32, 32, 64, 64);
                    SET_ALIGN_INFO(Height, 16, 16, 32, 32, 64);
                }
                else // if(pTexInfo->Flags.Info.TiledYs)
                {
                    SET_ALIGN_INFO(Width, 64, 128, 128, 256, 256);
                    SET_ALIGN_INFO(Height, 64, 64, 128, 128, 256);
                }

                // Only color buffer MSAA
                if(pTexInfo->MSAA.NumSamples > 1 &&
                   !pTexInfo->Flags.Gpu.Depth &&
                   !pTexInfo->Flags.Gpu.SeparateStencil)
                {
                    if(pGmmGlobalContext->GetSkuTable().FtrTileY)
                    {
                        switch(pTexInfo->MSAA.NumSamples)
                        {
                            case 16: UnitAlignWidth /= 4; UnitAlignHeight /= 4; break;
                            case 8:  UnitAlignWidth /= 4; UnitAlignHeight /= 2; break;
                            case 4:  UnitAlignWidth /= 2; UnitAlignHeight /= 2; break;
                            case 2:  UnitAlignWidth /= 2; break;
                            default: __GMM_ASSERT(0);
                        }
                    }
                    else
                    {
                        switch (pTexInfo->MSAA.NumSamples)
                        {
                            case 4:
                            case 8:
                            case 16: UnitAlignWidth /= 2; UnitAlignHeight /= 2; break;
                            case 2:  UnitAlignWidth /= 2; break;
                            default: __GMM_ASSERT(0);
                        }
                    }
                }
            }
            else if(pTexInfo->Type == RESOURCE_3D)
            {
                if(pTexInfo->Flags.Info.TiledYf)
                {
                    SET_ALIGN_INFO(Width,  4, 8,  8,  8, 16);
                    SET_ALIGN_INFO(Height, 8, 8, 16, 16, 16);
                    SET_ALIGN_INFO(Depth,  8, 8,  8, 16, 16);
                }
                else // if(pTexInfo->Flags.Info.TiledYs)
                {
                    SET_ALIGN_INFO(Width,  16, 32, 32, 32, 64);
                    SET_ALIGN_INFO(Height, 16, 16, 32, 32, 32);
                    SET_ALIGN_INFO(Depth,  16, 16, 16, 32, 32);
                }
            }

            #undef SET_ALIGN_INFO
            if (GmmIsCompressed(pTexInfo->Format))
            {
                uint32_t   ElementWidth, ElementHeight, ElementDepth;
                pTextureCalc->GetCompressionBlockDimensions(pTexInfo->Format, &ElementWidth, &ElementHeight, &ElementDepth);

                UnitAlignWidth *= ElementWidth;
                UnitAlignHeight *= ElementHeight;
                UnitAlignDepth *= ElementDepth;
            }
        }
        /// SKL 1D Surfaces ///////////////////////////////////////////////
        else if((GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE) &&
                (pTexInfo->Type == RESOURCE_1D))
        {
            UnitAlignWidth = 64;

            // Tile4/64
            SET_ALIGN_FACTOR(Width, 128);
        }
        /// CCS ///////////////////////////////////////////////////////////
        else if (pTexInfo->Flags.Gpu.CCS &&
            (pTexInfo->Flags.Gpu.__NonMsaaTileYCcs || pTexInfo->Flags.Gpu.__NonMsaaTileXCcs))
        {
            UnitAlignWidth = pPlatform->TexAlign.CCS.Align.Width;
            UnitAlignHeight = pPlatform->TexAlign.CCS.Align.Height;

            ALIGNMENT UnitAlign = { UnitAlignWidth , UnitAlignHeight, UnitAlignDepth };
            pGmmGlobalContext->GetPlatformInfoObj()->ApplyExtendedTexAlign(pTexInfo->CCSModeAlign, UnitAlign);

            if (UnitAlign.Width != UnitAlignWidth ||
                UnitAlign.Height != UnitAlignHeight ||
                UnitAlign.Depth != UnitAlignDepth)
            {
                UnitAlignWidth = UnitAlign.Width;
                UnitAlignHeight = UnitAlign.Height;
                UnitAlignDepth = UnitAlign.Depth;
            }
        }
        else if (GmmIsYUVPacked(pTexInfo->Format)) /////////////////////////
        {
            UnitAlignWidth = pPlatform->TexAlign.YUV422.Width;
            UnitAlignHeight = pPlatform->TexAlign.YUV422.Height;

            // For packed 8/16-bit formats alignment factor of 4 will give us < 16B so expand to 32B
            SET_ALIGN_FACTOR(Width, 32);
        }
        else if(GmmIsCompressed(pTexInfo->Format)) /////////////////////////////
        {
            uint32_t   ElementWidth, ElementHeight, ElementDepth;

            pTextureCalc->GetCompressionBlockDimensions(pTexInfo->Format, &ElementWidth, &ElementHeight, &ElementDepth);

            UnitAlignWidth  = ElementWidth * pPlatform->TexAlign.Compressed.Width;

            UnitAlignHeight = ElementHeight * pPlatform->TexAlign.Compressed.Height;

            UnitAlignDepth =  (pTexInfo->Type == RESOURCE_3D) ? ElementDepth * pPlatform->TexAlign.Compressed.Depth : pPlatform->TexAlign.Compressed.Depth;

        }
        /// Depth Buffer //////////////////////////////////////////////////
        else if(pTexInfo->Flags.Gpu.HiZ)
        {
            if( (GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN7_CORE) &&
                (pTexInfo->BitsPerPixel == 16))
            {
                UnitAlignWidth = 8; // Gen7 Special Case: HALIGN_8 for 16bpp Depth Buffers
            }
            else
            {
                UnitAlignWidth = pPlatform->TexAlign.Depth.Width;
            }

            UnitAlignHeight = pPlatform->TexAlign.Depth.Height;
        }
        else if (pTexInfo->Flags.Gpu.Depth)
        {
            if (pTexInfo->BitsPerPixel == 16)
            {
                if (pTexInfo->MSAA.NumSamples == 0x2 || pTexInfo->MSAA.NumSamples == 0x8)
                {
                    UnitAlignWidth = pPlatform->TexAlign.Depth_D16_UNORM_2x_8x.Width;
                    UnitAlignHeight = pPlatform->TexAlign.Depth_D16_UNORM_2x_8x.Height;
                }
                else
                {
                    UnitAlignWidth = pPlatform->TexAlign.Depth_D16_UNORM_1x_4x_16x.Width;
                    UnitAlignHeight = pPlatform->TexAlign.Depth_D16_UNORM_1x_4x_16x.Height;
                }

                SET_ALIGN_FACTOR(Width, 16);

            }
            else
            {
                UnitAlignWidth = pPlatform->TexAlign.Depth.Width;
                UnitAlignHeight = pPlatform->TexAlign.Depth.Height;

                SET_ALIGN_FACTOR(Width, 32);
            }
        }
        /// Separate Stencil //////////////////////////////////////////////
        else if(pTexInfo->Flags.Gpu.SeparateStencil)
        {
            UnitAlignWidth  = pPlatform->TexAlign.SeparateStencil.Width;
            UnitAlignHeight = pPlatform->TexAlign.SeparateStencil.Height;

            SET_ALIGN_FACTOR(Width, 16);

        }
        /// Cross Adapter //////////////////////////////////////////////
        else if(pTexInfo->Flags.Info.XAdapter)
        {
            //Add cross adapter height restriction.
            UnitAlignHeight = pPlatform->TexAlign.XAdapter.Height;
            UnitAlignWidth = pPlatform->TexAlign.XAdapter.Width;

            SET_ALIGN_FACTOR(Width, 128);

            __GMM_ASSERT(pTexInfo->MaxLod == 0);
        }
        else if(((pTexInfo->Flags.Gpu.MCS &&
                  GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN12_CORE) ||
                 (pTexInfo->Flags.Gpu.CCS && GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN9_CORE)) &&
                (pTexInfo->MSAA.NumSamples > 1))
        {
            UnitAlignWidth  = 16;
            UnitAlignHeight = 4;

            SET_ALIGN_FACTOR(Width, 128);
        }
        else if(pTexInfo->Flags.Wa.__ForceOtherHVALIGN4)
        {
            UnitAlignWidth  = 4;
            UnitAlignHeight = 4;
        }
        #ifndef _WIN32
        else if(pTexInfo->Flags.Gpu.NoRestriction)
        {
            UnitAlignWidth  = 1;
            UnitAlignHeight = 1;
        }
        #endif
        else /// All Other ////////////////////////////////////////////////
        {
            UnitAlignWidth  = pPlatform->TexAlign.AllOther.Width;

            if(GFX_GET_CURRENT_RENDERCORE(pPlatform->Platform) >= IGFX_GEN8_CORE)
            {
                UnitAlignHeight = pPlatform->TexAlign.AllOther.Height;

                // Let VAlign = 16, when bpp == 8 or 16 for both TileX and TileY on BDW
                if ((GmmGetWaTable(pGmmGlobalContext)->WaUseVAlign16OnTileXYBpp816) &&
                    (pTexInfo->BitsPerPixel == 8 || pTexInfo->BitsPerPixel == 16) &&
                    (pTexInfo->Flags.Info.TiledX || pTexInfo->Flags.Info.TiledY))
                {
                    UnitAlignHeight = 16;
                }

                if((GmmGetWaTable(pGmmGlobalContext)->Wa32bppTileY2DColorNoHAlign4) &&
                   (pTexInfo->BitsPerPixel == 32 && pTexInfo->Flags.Info.TiledY &&
                       pTexInfo->MSAA.NumSamples == 1 && pTexInfo->MaxLod > 1) &&
                   UnitAlignWidth <= 4)
                {
                    UnitAlignWidth = 8;
                }

                SET_ALIGN_FACTOR(Width, 128);
            }
            else if(pTexInfo->MSAA.NumSamples <= 1)
            {
                if ((GmmGetWaTable(pGmmGlobalContext)->WaValign2For96bppFormats) &&
                            ( pTexInfo->BitsPerPixel == 96 ) )
                {
                    UnitAlignHeight = 2;
                }
                else if ((GmmGetWaTable(pGmmGlobalContext)->WaValign2ForR8G8B8UINTFormat) &&
                            ( pTexInfo->Format == GMM_FORMAT_R8G8B8_UINT ) )
                {
                    UnitAlignHeight = 2;
                }
                else
                {
                    UnitAlignHeight = pPlatform->TexAlign.AllOther.Height;
                }
            }
            else
            {
                UnitAlignHeight = 4; // Gen6+ Special Case: VALIGN_4 for >= MSAA_4X Render Targets
            }
        }

        //ExistingSysMem override
        if(pTexInfo->Flags.Info.ExistingSysMem &&
           !pTexInfo->ExistingSysMem.IsGmmAllocated &&
           !pTexInfo->ExistingSysMem.IsPageAligned)
        {
            if(pTexInfo->Flags.Gpu.Texture)
            {
                UnitAlignWidth  = pPlatform->SamplerFetchGranularityWidth;
                UnitAlignHeight = pPlatform->SamplerFetchGranularityHeight;
            }
            else if(pTexInfo->Flags.Gpu.RenderTarget)
            {
                UnitAlignWidth = (GmmIsYUVPlanar(pTexInfo->Format)) ? 2 : 1;
                UnitAlignHeight = 1;
            }
        }

        pTexInfo->Alignment.HAlign = UnitAlignWidth;
        pTexInfo->Alignment.VAlign = UnitAlignHeight;
        pTexInfo->Alignment.DAlign = UnitAlignDepth;
    }
    else
    {
        // Don't reinitialize b/c special-case ResCreates (e.g. MCS) need the
        // values from their first pass through here to stick (but they'll come
        // through here more than once, with different parameters).
    }

    GMM_DPF_EXIT;
    return GMM_SUCCESS;
} // __GmmTexFillHAlignVAlign
#endif //__cpluscplus
//===========================================================================
// typedef:
//      GMM_MIPTAIL_SLOT_OFFSET_REC
//
// Description:
//      This structure used to describe the offset between miptail slot and
//      miptail starting address
//---------------------------------------------------------------------------
typedef struct GMM_MIPTAIL_SLOT_OFFSET_REC
{
    uint32_t X;
    uint32_t Y;
    uint32_t Z;
}GMM_MIPTAIL_SLOT_OFFSET;

#define GEN9_MIPTAIL_SLOT_OFFSET_1D_SURFACE {                                               \
/*  |     128 bpe    |     64 bpe    |     32 bpe    |      16 bpe    |      8 bpe     | */ \
    { { 2048, 0, 0 }, { 4096, 0, 0 }, { 8192, 0, 0 }, { 16384, 0, 0 }, { 32768, 0, 0 } },   \
    { { 1024, 0, 0 }, { 2048, 0, 0 }, { 4096, 0, 0 }, {  8192, 0, 0 }, { 16384, 0, 0 } },   \
    { {  512, 0, 0 }, { 1024, 0, 0 }, { 2048, 0, 0 }, {  4096, 0, 0 }, {  8192, 0, 0 } },   \
    { {  256, 0, 0 }, {  512, 0, 0 }, { 1024, 0, 0 }, {  2048, 0, 0 }, {  4096, 0, 0 } },   \
    { {  128, 0, 0 }, {  256, 0, 0 }, {  512, 0, 0 }, {  1024, 0, 0 }, {  2048, 0, 0 } },   \
    { {   64, 0, 0 }, {  128, 0, 0 }, {  256, 0, 0 }, {   512, 0, 0 }, {  1024, 0, 0 } },   \
    { {   48, 0, 0 }, {   96, 0, 0 }, {  192, 0, 0 }, {   384, 0, 0 }, {   768, 0, 0 } },   \
    { {   32, 0, 0 }, {   64, 0, 0 }, {  128, 0, 0 }, {   256, 0, 0 }, {   512, 0, 0 } },   \
    { {   28, 0, 0 }, {   56, 0, 0 }, {  112, 0, 0 }, {   224, 0, 0 }, {   448, 0, 0 } },   \
    { {   24, 0, 0 }, {   48, 0, 0 }, {   96, 0, 0 }, {   192, 0, 0 }, {   384, 0, 0 } },   \
    { {   20, 0, 0 }, {   40, 0, 0 }, {   80, 0, 0 }, {   160, 0, 0 }, {   320, 0, 0 } },   \
    { {   16, 0, 0 }, {   32, 0, 0 }, {   64, 0, 0 }, {   128, 0, 0 }, {   256, 0, 0 } },   \
    { {   12, 0, 0 }, {   24, 0, 0 }, {   48, 0, 0 }, {    96, 0, 0 }, {   192, 0, 0 } },   \
    { {    8, 0, 0 }, {   16, 0, 0 }, {   32, 0, 0 }, {    64, 0, 0 }, {   128, 0, 0 } },   \
    { {    4, 0, 0 }, {    8, 0, 0 }, {   16, 0, 0 }, {    32, 0, 0 }, {    64, 0, 0 } },   \
    { {    0, 0, 0 }, {    0, 0, 0 }, {    0, 0, 0 }, {     0, 0, 0 }, {     0, 0, 0 } },   \
}

#define GEN9_MIPTAIL_SLOT_OFFSET_2D_SURFACE {                                           \
/*  |    128 bpe    |    64 bpe   |     32 bpe    |     16 bpe    |      8 bpe      | */\
    { { 32,  0, 0 }, { 64,  0, 0 }, { 64,  0, 0 }, { 128,  0, 0 }, { 128,   0, 0 } },   \
    { {  0, 32, 0 }, {  0, 32, 0 }, {  0, 64, 0 }, {   0, 64, 0 }, {   0, 128, 0 } },   \
    { { 16,  0, 0 }, { 32,  0, 0 }, { 32,  0, 0 }, {  64,  0, 0 }, {  64,   0, 0 } },   \
    { {  0, 16, 0 }, {  0, 16, 0 }, {  0, 32, 0 }, {   0, 32, 0 }, {   0,  64, 0 } },   \
    { {  8,  0, 0 }, { 16,  0, 0 }, { 16,  0, 0 }, {  32,  0, 0 }, {  32,   0, 0 } },   \
    { {  4,  8, 0 }, {  8,  8, 0 }, {  8, 16, 0 }, {  16, 16, 0 }, {  16,  32, 0 } },   \
    { {  0, 12, 0 }, {  0, 12, 0 }, {  0, 24, 0 }, {   0, 24, 0 }, {   0,  48, 0 } },   \
    { {  0,  8, 0 }, {  0,  8, 0 }, {  0, 16, 0 }, {   0, 16, 0 }, {   0,  32, 0 } },   \
    { {  4,  4, 0 }, {  8,  4, 0 }, {  8,  8, 0 }, {  16,  8, 0 }, {  16,  16, 0 } },   \
    { {  4,  0, 0 }, {  8,  0, 0 }, {  8,  0, 0 }, {  16,  0, 0 }, {  16,   0, 0 } },   \
    { {  0,  4, 0 }, {  0,  4, 0 }, {  0,  8, 0 }, {   0,  8, 0 }, {   0,  16, 0 } },   \
    { {  3,  0, 0 }, {  6,  0, 0 }, {  4,  4, 0 }, {   8,  4, 0 }, {   0,  12, 0 } },   \
    { {  2,  0, 0 }, {  4,  0, 0 }, {  4,  0, 0 }, {   8,  0, 0 }, {   0,   8, 0 } },   \
    { {  1,  0, 0 }, {  2,  0, 0 }, {  0,  4, 0 }, {   0,  4, 0 }, {   0,   4, 0 } },   \
    { {  0,  0, 0 }, {  0,  0, 0 }, {  0,  0, 0 }, {   0,  0, 0 }, {   0,   0, 0 } },   \
}

#define GEN9_MIPTAIL_SLOT_OFFSET_3D_SURFACE {                                       \
/*  |   128 bpe   |    64 bpe   |    32 bpe   |     16 bpe   |      8 bpe      | */ \
    { { 8, 0, 0 }, { 16, 0, 0 }, { 16,  0, 0 }, { 16,  0,  0 }, { 32,  0,  0 } },   \
    { { 0, 8, 0 }, {  0, 8, 0 }, {  0, 16, 0 }, {  0, 16,  0 }, {  0, 16,  0 } },   \
    { { 0, 0, 8 }, {  0, 0, 8 }, {  0,  0, 8 }, {  0,  0, 16 }, {  0,  0, 16 } },   \
    { { 4, 0, 0 }, {  8, 0, 0 }, {  8,  0, 0 }, {  8,  0,  0 }, { 16,  0,  0 } },   \
    { { 0, 4, 0 }, {  0, 4, 0 }, {  0,  8, 0 }, {  0,  8,  0 }, {  0,  8,  0 } },   \
    { { 0, 0, 4 }, {  0, 0, 4 }, {  0,  0, 4 }, {  0,  0,  8 }, {  0,  0,  8 } },   \
    { { 3, 0, 0 }, {  6, 0, 0 }, {  4,  4, 0 }, {  0,  4,  4 }, {  0,  4,  4 } },   \
    { { 2, 0, 0 }, {  4, 0, 0 }, {  0,  4, 0 }, {  0,  4,  0 }, {  0,  4,  0 } },   \
    { { 1, 0, 3 }, {  2, 0, 3 }, {  4,  0, 3 }, {  0,  0,  7 }, {  0,  0,  7 } },   \
    { { 1, 0, 2 }, {  2, 0, 2 }, {  4,  0, 2 }, {  0,  0,  6 }, {  0,  0,  6 } },   \
    { { 1, 0, 1 }, {  2, 0, 1 }, {  4,  0, 1 }, {  0,  0,  5 }, {  0,  0,  5 } },   \
    { { 1, 0, 0 }, {  2, 0, 0 }, {  4,  0, 0 }, {  0,  0,  4 }, {  0,  0,  4 } },   \
    { { 0, 0, 3 }, {  0, 0, 3 }, {  0,  0, 3 }, {  0,  0,  3 }, {  0,  0,  3 } },   \
    { { 0, 0, 2 }, {  0, 0, 2 }, {  0,  0, 2 }, {  0,  0,  2 }, {  0,  0,  2 } },   \
    { { 0, 0, 1 }, {  0, 0, 1 }, {  0,  0, 1 }, {  0,  0,  1 }, {  0,  0,  1 } },   \
    { { 0, 0, 0 }, {  0, 0, 0 }, {  0,  0, 0 }, {  0,  0,  0 }, {  0,  0,  0 } },   \
}

#define GEN10_MIPTAIL_SLOT_OFFSET_1D_SURFACE {                                              \
/*  |     128 bpe    |     64 bpe    |     32 bpe    |      16 bpe    |      8 bpe     | */ \
    { { 2048, 0, 0 }, { 4096, 0, 0 }, { 8192, 0, 0 }, { 16384, 0, 0 }, { 32768, 0, 0 } },   \
    { { 1024, 0, 0 }, { 2048, 0, 0 }, { 4096, 0, 0 }, {  8192, 0, 0 }, { 16384, 0, 0 } },   \
    { {  512, 0, 0 }, { 1024, 0, 0 }, { 2048, 0, 0 }, {  4096, 0, 0 }, {  8192, 0, 0 } },   \
    { {  256, 0, 0 }, {  512, 0, 0 }, { 1024, 0, 0 }, {  2048, 0, 0 }, {  4096, 0, 0 } },   \
    { {  128, 0, 0 }, {  256, 0, 0 }, {  512, 0, 0 }, {  1024, 0, 0 }, {  2048, 0, 0 } },   \
    { {   96, 0, 0 }, {  192, 0, 0 }, {  384, 0, 0 }, {   768, 0, 0 }, {  1536, 0, 0 } },   \
    { {   80, 0, 0 }, {  160, 0, 0 }, {  320, 0, 0 }, {   640, 0, 0 }, {  1280, 0, 0 } },   \
    { {   64, 0, 0 }, {  128, 0, 0 }, {  256, 0, 0 }, {   512, 0, 0 }, {  1024, 0, 0 } },   \
    { {   48, 0, 0 }, {   96, 0, 0 }, {  192, 0, 0 }, {   384, 0, 0 }, {   768, 0, 0 } },   \
    { {   32, 0, 0 }, {   64, 0, 0 }, {  128, 0, 0 }, {   256, 0, 0 }, {   512, 0, 0 } },   \
    { {   16, 0, 0 }, {   32, 0, 0 }, {   64, 0, 0 }, {   128, 0, 0 }, {   256, 0, 0 } },   \
    { {   12, 0, 0 }, {   24, 0, 0 }, {   48, 0, 0 }, {    96, 0, 0 }, {   192, 0, 0 } },   \
    { {    8, 0, 0 }, {   16, 0, 0 }, {   32, 0, 0 }, {    64, 0, 0 }, {   128, 0, 0 } },   \
    { {    4, 0, 0 }, {    8, 0, 0 }, {   16, 0, 0 }, {    32, 0, 0 }, {    64, 0, 0 } },   \
    { {    0, 0, 0 }, {    0, 0, 0 }, {    0, 0, 0 }, {     0, 0, 0 }, {     0, 0, 0 } },   \
}

#define GEN10_MIPTAIL_SLOT_OFFSET_2D_SURFACE {                                          \
/*  |    128 bpe    |    64 bpe   |     32 bpe    |     16 bpe    |      8 bpe     | */ \
    { { 32,  0, 0 }, { 64,  0, 0 }, { 64,  0, 0 }, { 128,  0, 0 }, { 128,   0, 0 } },   \
    { {  0, 32, 0 }, {  0, 32, 0 }, {  0, 64, 0 }, {   0, 64, 0 }, {   0, 128, 0 } },   \
    { { 16,  0, 0 }, { 32,  0, 0 }, { 32,  0, 0 }, {  64,  0, 0 }, {  64,   0, 0 } },   \
    { {  0, 16, 0 }, {  0, 16, 0 }, {  0, 32, 0 }, {   0, 32, 0 }, {   0,  64, 0 } },   \
    { {  8,  0, 0 }, { 16,  0, 0 }, { 16,  0, 0 }, {  32,  0, 0 }, {  32,   0, 0 } },   \
    { {  4,  8, 0 }, {  8,  8, 0 }, {  8, 16, 0 }, {  16, 16, 0 }, {  16,  32, 0 } },   \
    { {  0, 12, 0 }, {  0, 12, 0 }, {  0, 24, 0 }, {   0, 24, 0 }, {   0,  48, 0 } },   \
    { {  0,  8, 0 }, {  0,  8, 0 }, {  0, 16, 0 }, {   0, 16, 0 }, {   0,  32, 0 } },   \
    { {  4,  4, 0 }, {  8,  4, 0 }, {  8,  8, 0 }, {  16,  8, 0 }, {  16,  16, 0 } },   \
    { {  4,  0, 0 }, {  8,  0, 0 }, {  8,  0, 0 }, {  16,  0, 0 }, {  16,   0, 0 } },   \
    { {  0,  4, 0 }, {  0,  4, 0 }, {  0,  8, 0 }, {   0,  8, 0 }, {   0,  16, 0 } },   \
    { {  0,  0, 0 }, {  0,  0, 0 }, {  0,  0, 0 }, {   0,  0, 0 }, {   0,   0, 0 } },   \
    { {  1,  0, 0 }, {  2,  0, 0 }, {  0,  4, 0 }, {   0,  4, 0 }, {   0,   4, 0 } },   \
    { {  2,  0, 0 }, {  4,  0, 0 }, {  4,  0, 0 }, {   8,  0, 0 }, {   0,   8, 0 } },   \
    { {  3,  0, 0 }, {  6,  0, 0 }, {  4,  4, 0 }, {   8,  4, 0 }, {   0,  12, 0 } },   \
}

#define GEN10_MIPTAIL_SLOT_OFFSET_3D_SURFACE {                                      \
/*  |   128 bpe   |    64 bpe   |    32 bpe   |     16 bpe   |      8 bpe      | */ \
    { { 8, 0, 0 }, { 16, 0, 0 }, { 16,  0, 0 }, { 16,  0,  0 }, { 32,  0,  0 } },   \
    { { 0, 8, 0 }, {  0, 8, 0 }, {  0, 16, 0 }, {  0, 16,  0 }, {  0, 16,  0 } },   \
    { { 0, 0, 8 }, {  0, 0, 8 }, {  0,  0, 8 }, {  0,  0, 16 }, {  0,  0, 16 } },   \
    { { 4, 0, 0 }, {  8, 0, 0 }, {  8,  0, 0 }, {  8,  0,  0 }, { 16,  0,  0 } },   \
    { { 0, 4, 0 }, {  0, 4, 0 }, {  0,  8, 0 }, {  0,  8,  0 }, {  0,  8,  0 } },   \
    { { 2, 0, 4 }, {  4, 0, 4 }, {  4,  0, 4 }, {  4,  0,  8 }, {  8,  0,  8 } },   \
    { { 0, 2, 4 }, {  0, 2, 4 }, {  0,  4, 4 }, {  0,  4,  8 }, {  0,  4,  8 } },   \
    { { 0, 0, 4 }, {  0, 0, 4 }, {  0,  0, 4 }, {  0,  0,  8 }, {  0,  0,  8 } },   \
    { { 2, 2, 0 }, {  4, 2, 0 }, {  4,  4, 0 }, {  4,  4,  0 }, {  8,  4,  0 } },   \
    { { 2, 0, 0 }, {  4, 0, 0 }, {  4,  0, 0 }, {  4,  0,  0 }, {  8,  0,  0 } },   \
    { { 0, 2, 0 }, {  0, 2, 0 }, {  0,  4, 0 }, {  0,  4,  0 }, {  0,  4,  0 } },   \
    { { 1, 0, 2 }, {  2, 0, 2 }, {  2,  0, 2 }, {  2,  0,  4 }, {  4,  0,  4 } },   \
    { { 0, 0, 2 }, {  0, 0, 2 }, {  0,  0, 2 }, {  0,  0,  4 }, {  0,  0,  4 } },   \
    { { 1, 0, 0 }, {  2, 0, 0 }, {  2,  0, 0 }, {  2,  0,  0 }, {  4,  0,  0 } },   \
    { { 0, 0, 0 }, {  0, 0, 0 }, {  0,  0, 0 }, {  0,  0,  0 }, {  0,  0,  0 } },   \
}

#define GEN11_MIPTAIL_SLOT_OFFSET_1D_SURFACE {                                              \
/*  |     128 bpe    |     64 bpe    |     32 bpe    |      16 bpe    |      8 bpe     | */ \
    { { 2048, 0, 0 }, { 4096, 0, 0 }, { 8192, 0, 0 }, { 16384, 0, 0 }, { 32768, 0, 0 } },   \
    { { 1024, 0, 0 }, { 2048, 0, 0 }, { 4096, 0, 0 }, {  8192, 0, 0 }, { 16384, 0, 0 } },   \
    { {  512, 0, 0 }, { 1024, 0, 0 }, { 2048, 0, 0 }, {  4096, 0, 0 }, {  8192, 0, 0 } },   \
    { {  256, 0, 0 }, {  512, 0, 0 }, { 1024, 0, 0 }, {  2048, 0, 0 }, {  4096, 0, 0 } },   \
    { {  128, 0, 0 }, {  256, 0, 0 }, {  512, 0, 0 }, {  1024, 0, 0 }, {  2048, 0, 0 } },   \
    { {   96, 0, 0 }, {  192, 0, 0 }, {  384, 0, 0 }, {   768, 0, 0 }, {  1536, 0, 0 } },   \
    { {   80, 0, 0 }, {  160, 0, 0 }, {  320, 0, 0 }, {   640, 0, 0 }, {  1280, 0, 0 } },   \
    { {   64, 0, 0 }, {  128, 0, 0 }, {  256, 0, 0 }, {   512, 0, 0 }, {  1024, 0, 0 } },   \
    { {   48, 0, 0 }, {   96, 0, 0 }, {  192, 0, 0 }, {   384, 0, 0 }, {   768, 0, 0 } },   \
    { {   32, 0, 0 }, {   64, 0, 0 }, {  128, 0, 0 }, {   256, 0, 0 }, {   512, 0, 0 } },   \
    { {   16, 0, 0 }, {   32, 0, 0 }, {   64, 0, 0 }, {   128, 0, 0 }, {   256, 0, 0 } },   \
    { {    0, 0, 0 }, {    0, 0, 0 }, {    0, 0, 0 }, {     0, 0, 0 }, {     0, 0, 0 } },   \
    { {    4, 0, 0 }, {    8, 0, 0 }, {   16, 0, 0 }, {    32, 0, 0 }, {    64, 0, 0 } },   \
    { {    8, 0, 0 }, {   16, 0, 0 }, {   32, 0, 0 }, {    64, 0, 0 }, {   128, 0, 0 } },   \
    { {   12, 0, 0 }, {   24, 0, 0 }, {   48, 0, 0 }, {    96, 0, 0 }, {   192, 0, 0 } },   \
}

#define GEN11_MIPTAIL_SLOT_OFFSET_2D_SURFACE GEN10_MIPTAIL_SLOT_OFFSET_2D_SURFACE

#define GEN11_MIPTAIL_SLOT_OFFSET_3D_SURFACE {                                      \
/*  |   128 bpe   |    64 bpe   |    32 bpe   |     16 bpe   |      8 bpe      | */ \
    { { 8, 0, 0 }, { 16, 0, 0 }, { 16,  0, 0 }, { 16,  0,  0 }, { 32,  0,  0 } },   \
    { { 0, 8, 0 }, {  0, 8, 0 }, {  0, 16, 0 }, {  0, 16,  0 }, {  0, 16,  0 } },   \
    { { 0, 0, 8 }, {  0, 0, 8 }, {  0,  0, 8 }, {  0,  0, 16 }, {  0,  0, 16 } },   \
    { { 4, 0, 0 }, {  8, 0, 0 }, {  8,  0, 0 }, {  8,  0,  0 }, { 16,  0,  0 } },   \
    { { 0, 4, 0 }, {  0, 4, 0 }, {  0,  8, 0 }, {  0,  8,  0 }, {  0,  8,  0 } },   \
    { { 2, 0, 4 }, {  4, 0, 4 }, {  4,  0, 4 }, {  0,  4,  8 }, {  0,  4,  8 } },   \
    { { 1, 0, 4 }, {  2, 0, 4 }, {  0,  4, 4 }, {  0,  0, 12 }, {  0,  0, 12 } },   \
    { { 0, 0, 4 }, {  0, 0, 4 }, {  0,  0, 4 }, {  0,  0,  8 }, {  0,  0,  8 } },   \
    { { 3, 0, 0 }, {  6, 0, 0 }, {  4,  4, 0 }, {  0,  4,  4 }, {  0,  4,  4 } },   \
    { { 2, 0, 0 }, {  4, 0, 0 }, {  4,  0, 0 }, {  0,  4,  0 }, {  0,  4,  0 } },   \
    { { 1, 0, 0 }, {  2, 0, 0 }, {  0,  4, 0 }, {  0,  0,  4 }, {  0,  0,  4 } },   \
    { { 0, 0, 0 }, {  0, 0, 0 }, {  0,  0, 0 }, {  0,  0,  0 }, {  0,  0,  0 } },   \
    { { 0, 0, 1 }, {  0, 0, 1 }, {  0,  0, 1 }, {  0,  0,  1 }, {  0,  0,  1 } },   \
    { { 0, 0, 2 }, {  0, 0, 2 }, {  0,  0, 2 }, {  0,  0,  2 }, {  0,  0,  2 } },   \
    { { 0, 0, 3 }, {  0, 0, 3 }, {  0,  0, 3 }, {  0,  0,  3 }, {  0,  0,  3 } },   \
}
