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

#include "GmmGen12ResourceULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Resource fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
//  It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/// @see    CTestGen12Resource::SetUpTestCase()
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen12Resource::SetUpTestCase()
{
    printf("%s\n", __FUNCTION__);
    GfxPlatform.eProductFamily    = IGFX_TIGERLAKE_LP;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN12_CORE;

    pGfxAdapterInfo = (ADAPTER_INFO *)malloc(sizeof(ADAPTER_INFO));
    if(pGfxAdapterInfo)
    {
        memset(pGfxAdapterInfo, 0, sizeof(ADAPTER_INFO));

        pGfxAdapterInfo->SkuTable.FtrLinearCCS = 1; //legacy y =>0 - test both
        pGfxAdapterInfo->SkuTable.FtrTileY     = 1;
        pGfxAdapterInfo->SkuTable.FtrLLCBypass = 0;
        CommonULT::SetUpTestCase();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/// @see    CTestGen12Resource::TearDownTestCase()
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen12Resource::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

/// @brief ULT for 2D TileYs Compressed Resource
/// tests both Separate and Unified CCS allcoation
TEST_F(CTestGen12Resource, Test2DTileYsCompressedResource)
{
    const uint32_t HAlign[TEST_BPP_MAX] = {256, 256, 128, 128, 64}; //REM-comment: YS-shape in pixels per-bpp
    const uint32_t VAlign[TEST_BPP_MAX] = {256, 128, 128, 64, 64};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{256, 256}, //REM-comment: YS-shape in bytes per-bpp
                                                {512, 128},
                                                {512, 128},
                                                {1024, 64},
                                                {1024, 64}};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_2D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Info.TiledYs          = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;
    // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
    // to 4x1 tile (reqd by HW for perf reasons)
    // If unifiedAuxSurf reqd (mandatory for displayable or cross-adapter shared), turn on .CCS/.MMC and .UnifiedAuxSurface too

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = 0x1;
        gmmParams.BaseHeight   = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0]); // As wide as 1 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);       // 1 Tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64));   // 1 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);            // Not Tested

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 16KB aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        //EXPECT_EQ(0, ResourceInfo->GetRenderPitchInTiles() % GMM_KBYTE(16));   // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        else
        {
            //Test AuxSurf H/Valign, size etc (Not POR- can be removed)
            ALIGNMENT UnitAlign;
            pGmmULTClientContext->GetExtendedTextureAlign(CCS_MODE(TileMode), UnitAlign);

            EXPECT_EQ(UnitAlign.Width, ResourceInfo->GetAuxHAlign());
            EXPECT_EQ(UnitAlign.Height, ResourceInfo->GetAuxVAlign());
            EXPECT_EQ(0x80, ResourceInfo->GetUnifiedAuxPitch()); //TileY = 0x80 x 0x20
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeAllocation());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = 0x1;
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64) * 2);   // 2 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 16KB aligned

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        else
        { //Not POR
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_2D, bpp);

        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        //**Not compression specific -BEGIN
        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64) * 4);   // 4 tile big
        VerifyResourceQPitch<false>(ResourceInfo, 0);                // Not tested
        //**Not compression specific -END

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D TileYf Compressed Resource
TEST_F(CTestGen12Resource, Test2DTileYfCompressedResource)
{
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 64, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{64, 64},
                                                {128, 32},
                                                {128, 32},
                                                {256, 16},
                                                {256, 16}};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_2D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Info.TiledYf          = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;
    // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
    // to 4x1 tile (reqd by HW )
    // If unifiedAuxSurf reqd (mandatory for displayable or cross-adapter shared), turn on .CCS/.MMC and .UnifiedAuxSurface too

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = 0x1;
        gmmParams.BaseHeight   = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, 4 * TileSize[i][0]); // As wide as 4 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);           // 4 Tile wide
        VerifyResourceSize<true>(ResourceInfo, 4 * GMM_KBYTE(4));    // 4 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);                // Not Tested

        //test main surface base alignment is 64KB
        //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = 0x1;
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 4); // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);           // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4);    // 2 tile big, but 4-tile pitch alignment

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 4);  // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);            // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4 * 2); // 4 tile wide; and 2-tile high

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 64KB
                                                      //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);


        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D TileY Compressed Resource
TEST_F(CTestGen12Resource, Test2DTileYCompressedResource)
{
    const uint32_t HAlign = {16};
    const uint32_t VAlign = {4};

    const uint32_t TileSize[2] = {128, 32};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;

    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;
    // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf/Y its also used to pad surf
    // to 4x1 tile (reqd by HW for perf reasons)
    // If unifiedAuxSurf reqd (mandatory for displayable or cross-adapter shared), turn on .CCS/.MMC and .UnifiedAuxSurface too

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = 0x1;
        gmmParams.BaseHeight   = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, 4 * TileSize[0]); // As wide as 4 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 4 Tile wide
        VerifyResourceSize<true>(ResourceInfo, 4 * GMM_KBYTE(4)); // 4 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);             // Not Tested

        //test main surface base alignment is 64KB
        //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = 0x1;
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0] * 4); // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4); // 2 tile big, but 4-tile pitch alignment

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = TileSize[1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0] * 4);     // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);            // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4 * 2); // 4 tile wide; and 2-tile high

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 64KB
                                                      //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D TileY lossless Compressed lossy Resource
TEST_F(CTestGen12Resource, Test2DTileYLossyCompressedResource)
{
    const uint32_t HAlign = {4};
    const uint32_t VAlign = {4};

    const uint32_t TileSize[2] = {128, 32};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;

    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;
    // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
    // to 4x1 tile (reqd by HW for perf reasons)
    // If unifiedAuxSurf reqd (mandatory for displayable or cross-adapter shared), turn on .CCS/.MMC and .UnifiedAuxSurface too

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = GMM_FORMAT_ETC2_RGB8;
        gmmParams.BaseWidth64  = 0x80;
        gmmParams.BaseHeight   = 0x20;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, 4 * TileSize[0]); // As wide as 4 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 4 Tile wide
        VerifyResourceSize<true>(ResourceInfo, 4 * GMM_KBYTE(4)); // 4 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);             // Not Tested

        //test main surface base alignment is 64KB
        //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = GMM_FORMAT_ETC2_RGB8;
        gmmParams.BaseWidth64  = (TileSize[0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = 0x1;
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0] * 4); // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4); // 2 tile big, but 4-tile pitch alignment

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = GMM_FORMAT_ETC2_RGB8;
        gmmParams.BaseWidth64  = (TileSize[0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = TileSize[1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0] * 4); // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4); // 4 tile wide; max compressed height = 0x24/4 = 9, so fits in 1 tile-height

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 64KB
                                                      //For Yf/Y test main surface pitch is 4-tileYF/Y aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

//Y416(64bpp -how is 10/12/16bit depth given?)
//Y410(32bpp), Y216(64bpp), YCRCB_NORMAL(16bpp), YCRCB_SWAPUV(16bpp),
//YCRCB_SWAPUVY(16bpp), YCRCB_SWAPY(16bpp)

/// @brief ULT for Planar Compressed Resource
TEST_F(CTestGen12Resource, DISABLED_TestPlanarYfCompressedResource)
{
    const uint32_t TileSize[TEST_BPP_MAX][2] = {
    {64, 64}, {128, 32}, {128, 32}, {256, 16}, {256, 16}}; // TileYf

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    //gmmParams.Flags.Gpu.MMC = 1;
    gmmParams.Flags.Gpu.CCS              = 1;
    gmmParams.Flags.Info.MediaCompressed = 1;
    gmmParams.BaseWidth64                = 0x100;
    gmmParams.BaseHeight                 = 0x50;
    gmmParams.Depth                      = 0x1;
    SetTileFlag(gmmParams, TEST_TILEYF); // TileYF only

    //UV-Packed formats
    GMM_RESOURCE_FORMAT Format[4] = {GMM_FORMAT_NV12, GMM_FORMAT_NV21, GMM_FORMAT_P010, GMM_FORMAT_P016};
    for(auto fmt : Format)
    {
        gmmParams.Format = fmt; // 8bpp (NV12, NV21), 16bpp (P016,P010)

        TEST_BPP Ybpp, UVbpp;
        //Yf/Ys could be accessed on CPU/app where UV plane bpp is double
        switch(pGmmULTClientContext->GetPlatformInfo().FormatTable[fmt].Element.BitsPer)
        {
            case 8:
                Ybpp  = TEST_BPP_8;
                UVbpp = TEST_BPP_16;
                break;
            case 16:
                Ybpp  = TEST_BPP_16;
                UVbpp = TEST_BPP_32;
                break;
            default:
                return;
        }
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;

        /*Aux is TileY (later Linear), not redescribing, its bytes are allocated using one bpp*/
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, Ybpp);

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        //Redescribed Pitch isn't modified unless Y, UV pitch differ
        //But, original Pitch is padded to have even Tile, hence use Ybpp ExpectedPitch
        //to verify Pitch, but redescribed size
        uint32_t ExpectedPitch    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 * (int)pow(2.0, Ybpp), TileSize[Ybpp][0] * 4);
        uint32_t RedescribedPitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64 / 2 * (int)pow(2.0, UVbpp), TileSize[UVbpp][0] * 4);

        //ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, 2 * TileSize[Ybpp][0]);   //pad to even tile
        if(ExpectedPitch != RedescribedPitch)
        {
            ExpectedPitch = RedescribedPitch;
        }

        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[Ybpp][0]);

        int YSizeInTiles = (GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[Ybpp][1]) / TileSize[Ybpp][1]) *
                           RedescribedPitch / TileSize[Ybpp][0];
        int UVSizeInTiles = (GMM_ULT_ALIGN(gmmParams.BaseHeight / 2, TileSize[UVbpp][1]) / TileSize[UVbpp][1]) *
                            RedescribedPitch / TileSize[UVbpp][0];

        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * (YSizeInTiles + UVSizeInTiles));
        VerifyResourceHAlign<true>(ResourceInfo, TileSize[UVbpp][0] / pow(2.0, UVbpp)); // For Yf/Ys planar redescription causes UV width, Y height alignment
        VerifyResourceVAlign<true>(ResourceInfo, TileSize[Ybpp][1]);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-mipped surface

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4) * 2, ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS)); // Y and UV Aux are on separate tiles

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            EXPECT_EQ(GMM_KBYTE(4) * 2, AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Planar Y Compressed Resource
TEST_F(CTestGen12Resource, TestPlanarYCompressedResource)
{
    const uint32_t TileSize[2] = {128, 32};
    const uint32_t HAlign      = 16;
    const uint32_t VAlign      = 4;

    GMM_RESCREATE_PARAMS gmmParams         = {};
    gmmParams.Type                         = RESOURCE_2D;
    gmmParams.NoGfxMemory                  = 1;
    gmmParams.Flags.Gpu.Texture            = 1;
    gmmParams.Flags.Gpu.RenderTarget       = 1;
    gmmParams.Flags.Gpu.MMC                = 1;
    gmmParams.Flags.Gpu.CCS                = 1;
    gmmParams.Flags.Gpu.IndirectClearColor = 1;
    gmmParams.Flags.Info.MediaCompressed   = 1;
    gmmParams.Flags.Info.NotLockable       = 1;
    gmmParams.Flags.Info.Cacheable         = 1;
    gmmParams.BaseWidth64                  = 0xB2;
    gmmParams.BaseHeight                   = 0x92;
    gmmParams.Depth                        = 0x1;
    SetTileFlag(gmmParams, TEST_TILEY); // TileYF only

    GMM_RESOURCE_FORMAT Format[4] = {GMM_FORMAT_NV12, GMM_FORMAT_NV21, GMM_FORMAT_P010, GMM_FORMAT_P016};
    for(auto fmt : Format)
    {
        gmmParams.Format = fmt; // 8bpp (NV12, NV21), 16bpp (P016, P010)

        TEST_BPP Ybpp, UVbpp;
        //Yf/Ys could be accessed on CPU/app where UV plane bpp is double
        switch(pGmmULTClientContext->GetPlatformInfo().FormatTable[fmt].Element.BitsPer)
        {
            case 8:
                Ybpp  = TEST_BPP_8;
                UVbpp = TEST_BPP_16;
                break;
            case 16:
                Ybpp  = TEST_BPP_16;
                UVbpp = TEST_BPP_32;
                break;
            default:
                return;
        }
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;

        GMM_TILE_MODE      TileMode = LEGACY_TILE_Y;
        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        //Redescribed Pitch isn't modified unless Y, UV pitch differ
        //But, original Pitch is padded to have even Tile, hence use Ybpp ExpectedPitch
        //to verify Pitch, but redescribed size
        uint32_t ExpectedPitch    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 * (int)pow(2.0, Ybpp), TileSize[0] * 4);
        uint32_t RedescribedPitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64 / 2 * (int)pow(2.0, UVbpp), TileSize[0] * 4);

        //ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, 2 * TileSize[Ybpp][0]);   //pad to even tile
        if(ExpectedPitch != RedescribedPitch)
        {
            ExpectedPitch = RedescribedPitch;
        }

        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[0]);

        int YSizeInTiles = (GMM_ULT_ALIGN(gmmParams.BaseHeight, 4 * TileSize[1]) / TileSize[1]) * //Default 64K-alignment for Y/UV base (AuxT 64K)
                           RedescribedPitch / TileSize[0];
        int UVSizeInTiles = (GMM_ULT_ALIGN(gmmParams.BaseHeight / 2, 4 * TileSize[1]) / TileSize[1]) * //Default 64K-alignment for Y/UV base (AuxT 64K)
                            RedescribedPitch / TileSize[0];

        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * (YSizeInTiles + UVSizeInTiles)); //when main-surf planes are tile-aligned, make it verify-true
        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-mipped surface

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileY aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_UV_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4) * 2, ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS)); // Y and UV Aux are on separate tiles

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Planar Ys Compressed resource
TEST_F(CTestGen12Resource, TestPlanarYsCompressedResource)
{
    const TEST_TILE_TYPE TileTypeSupported = {TEST_TILEYS};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {
    {256, 256}, {512, 128}, {512, 128}, {1024, 64}, {1024, 64}}; // TileYS

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.Flags.Gpu.MMC        = 1;
    //gmmParams.Flags.Gpu.CCS = 1;
    gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
    gmmParams.Flags.Info.MediaCompressed  = 1;
    gmmParams.BaseWidth64                 = 0x100;
    gmmParams.BaseHeight                  = 0x50;
    gmmParams.Depth                       = 0x1;
    SetTileFlag(gmmParams, TEST_TILEYS); // TileYS only

    GMM_RESOURCE_FORMAT Format[4] = {GMM_FORMAT_NV12, GMM_FORMAT_NV21, GMM_FORMAT_P010, GMM_FORMAT_P016};
    for(auto fmt : Format)
    {
        gmmParams.Format = fmt; // 8bpp(NV12) , P016 (16bpp), P010 (16bpp), NV21(8bpp)

        TEST_BPP Ybpp, UVbpp;
        //Yf/Ys could be accessed on CPU/app where UV plane bpp is double
        switch(pGmmULTClientContext->GetPlatformInfo().FormatTable[gmmParams.Format].Element.BitsPer)
        {
            case 8:
                Ybpp  = TEST_BPP_8;
                UVbpp = TEST_BPP_16;
                break;
            case 16:
                Ybpp  = TEST_BPP_16;
                UVbpp = TEST_BPP_32;
                break;
            default:
                return;
        }

        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
        GMM_TILE_MODE TileMode                = DEFINE_TILE(YS_2D, Ybpp);

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        //Redescribed Pitch isn't modified unless Y, UV pitch differ
        //But, original Pitch is padded to have even Tile, hence use Ybpp ExpectedPitch
        //to verify Pitch, but redescribed pitch to verify size
        uint32_t ExpectedPitch    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 * (int)pow(2.0, Ybpp), TileSize[Ybpp][0]);
        uint32_t RedescribedPitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64 / 2 * (int)pow(2.0, UVbpp), TileSize[UVbpp][0]);

        if(ExpectedPitch != RedescribedPitch)
        {
            ExpectedPitch = RedescribedPitch;
        }
        else
        {
            //ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, 2 * TileSize[Ybpp][0]); //pad to even tile
            //ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, (2 * TileSize[UVbpp][0]/ (int)pow(2.0, UVbpp))); //pad to even tile
        }

        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[Ybpp][0]);

        int YSizeInTiles = (GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[Ybpp][1]) / TileSize[Ybpp][1]) *
                           RedescribedPitch / TileSize[Ybpp][0];
        int UVSizeInTiles = (GMM_ULT_ALIGN(gmmParams.BaseHeight / 2, TileSize[UVbpp][1]) / TileSize[UVbpp][1]) *
                            RedescribedPitch / TileSize[UVbpp][0];
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64) * (YSizeInTiles + UVSizeInTiles));
        VerifyResourceHAlign<true>(ResourceInfo, TileSize[UVbpp][0] / pow(2.0, UVbpp)); // For Yf/Ys planar redescription causes UV width, Y height alignment
        VerifyResourceVAlign<true>(ResourceInfo, TileSize[Ybpp][1]);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-mipped surface

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        //EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);   // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_UV_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }

        EXPECT_EQ(GMM_KBYTE(4) * 2, ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS)); // Y and UV Aux are on separate tiles

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D arrayed compressed Resource
TEST_F(CTestGen12Resource, TestArrayedCompressedResource)
{
    //printf("%s\n", __FUNCTION__);
    //Test for 3D array
}

/// @brief ULT for mip-mapped compressed Resource
TEST_F(CTestGen12Resource, TestMipMapCompressedResource)
{
    //printf("%s\n", __FUNCTION__);
}

/// @brief ULT for cube Compressed Resource
TEST_F(CTestGen12Resource, TestCubeCompressedResource)
{
    //Tests 2D array
    const uint32_t HAlign[5][TEST_BPP_MAX] = {{0}, {0}, {0}, {256, 256, 128, 128, 64}, {64, 64, 32, 32, 16}};
    const uint32_t VAlign[5][TEST_BPP_MAX] = {{0}, {0}, {0}, {256, 128, 128, 64, 64}, {64, 32, 32, 16, 16}};

    const TEST_TILE_TYPE TileTypeSupported[2] = {TEST_TILEYS, TEST_TILEYF};

    const uint32_t TileSize[5][TEST_BPP_MAX][2] = {
    {0},
    {0},
    {0},
    {{256, 256}, {512, 128}, {512, 128}, {1024, 64}, {1024, 64}}, // TileYS
    {{64, 64}, {128, 32}, {128, 32}, {256, 16}, {256, 16}}};      //TileYf

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_CUBE;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;

    // Allocate 1x1 surface so that it occupies 1 Tile in X dimension
    for(auto Tiling : TileTypeSupported)
    {
        gmmParams.Flags.Info.TiledY  = 1;
        gmmParams.Flags.Info.TiledYf = (Tiling == TEST_TILEYF);
        gmmParams.Flags.Info.TiledYs = (Tiling == TEST_TILEYS);
        for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
        {
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
            gmmParams.Flags.Gpu.CCS               = 1;

            TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
            GMM_TILE_MODE TileMode = (Tiling == TEST_TILEYF) ? DEFINE_TILE(YF_2D, bpp) : DEFINE_TILE(YS_2D, bpp);

            gmmParams.Format      = SetResourceFormat(bpp);
            gmmParams.BaseWidth64 = 0x1;
            gmmParams.BaseHeight  = 0x1;
            gmmParams.Depth       = 0x1;

            GMM_RESOURCE_INFO *ResourceInfo;
            ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            VerifyResourceHAlign<true>(ResourceInfo, HAlign[Tiling][i]);
            VerifyResourceVAlign<true>(ResourceInfo, VAlign[Tiling][i]);

            uint32_t ExpectedPitch = 4 * TileSize[TEST_TILEYF][i][0];
            VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);                          // As wide as 4 tile-YF
            VerifyResourcePitchInTiles<true>(ResourceInfo, (Tiling == TEST_TILEYF) ? 4 : 1); // 1 tile wide

            uint32_t ExpectedQPitch = VAlign[Tiling][i];
            VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile

            VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                     ExpectedPitch *
                                     __GMM_MAX_CUBE_FACE * ExpectedQPitch);

            //test main surface base alignment is 64KB
            //For Yf test main surface pitch is 4-tileYF aligned
            EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
            EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % ((Tiling == TEST_TILEYF) ? 4 : 1));

            if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
            {
                EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
                EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
            }

            EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

            { //separate Aux
                gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

                GMM_RESOURCE_INFO *AuxResourceInfo;
                AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
                EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
                EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
                EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
                EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
                pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
            }

            for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
            {
                GMM_REQ_OFFSET_INFO OffsetInfo = {};
                OffsetInfo.ReqRender           = 1;
                OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
                ResourceInfo->GetOffset(OffsetInfo);

                EXPECT_EQ((CubeFaceIndex * ExpectedQPitch) * ExpectedPitch,
                          OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
                EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0 as cube face starts on tile boundary
                EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Y Offset should be 0 as cube face starts on tile boundary
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
            }
            pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
        }

        // Allocate 2 tiles in X dimension.
        // Width and Height must be equal
        for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
        {
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
            gmmParams.Flags.Gpu.CCS               = 1;
            TEST_BPP      bpp                     = static_cast<TEST_BPP>(i);
            GMM_TILE_MODE TileMode                = (Tiling == TEST_TILEYF) ? DEFINE_TILE(YF_2D, bpp) : DEFINE_TILE(YS_2D, bpp);

            gmmParams.Format      = SetResourceFormat(bpp);
            gmmParams.BaseWidth64 = (TileSize[Tiling][i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
            gmmParams.BaseHeight  = gmmParams.BaseWidth64;                           // Heigth must be equal to width.
            gmmParams.Depth       = 0x1;

            GMM_RESOURCE_INFO *ResourceInfo;
            ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            VerifyResourceHAlign<true>(ResourceInfo, HAlign[Tiling][i]);
            VerifyResourceVAlign<true>(ResourceInfo, VAlign[Tiling][i]);

            uint32_t ExpectedPitch = TileSize[Tiling][i][0] * 2 * ((Tiling == TEST_TILEYF) ? 2 : 1); // As wide as 2 tile, padded to 4 tile-pitch
            VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
            VerifyResourcePitchInTiles<true>(ResourceInfo, 2 * ((Tiling == TEST_TILEYF) ? 2 : 1)); // 2 tile wide

            uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign[Tiling][i]);
            VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

            VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                     ExpectedPitch *
                                     __GMM_MAX_CUBE_FACE * ExpectedQPitch);

            //test main surface base alignment is 64KB
            //For Yf test main surface pitch is 4-tileYF aligned
            EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
            EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % ((Tiling == TEST_TILEYF) ? 4 : 1)); // Check on YF only

            if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
            {
                EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
                EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
            }

            { //separate Aux
                gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

                GMM_RESOURCE_INFO *AuxResourceInfo;
                AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
                EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
                EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
                EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
                EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
                pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
            }

            for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
            {
                GMM_REQ_OFFSET_INFO OffsetInfo = {};
                OffsetInfo.ReqRender           = 1;
                OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
                ResourceInfo->GetOffset(OffsetInfo);
                EXPECT_EQ((CubeFaceIndex * ExpectedQPitch) * ExpectedPitch,
                          OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
                EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0 as cube face starts on tile boundary
                EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Y Offset should be 0 as cube face starts on tile boundary
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
            }
            pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
        }
    }
}

/// @brief ULT for 3D TileYs Compressed Resource
TEST_F(CTestGen12Resource, Test3DTileYsCompressedResource)
{
    if(!const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
    {
        return;
    }
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 32, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {32, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][3] = {{64, 32, 32},
                                                {64, 32, 32},
                                                {128, 32, 16},
                                                {256, 16, 16},
                                                {256, 16, 16}};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_3D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Info.TiledYs          = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1; // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
                                               // to 4x1 tile (reqd by HW for perf reasons)

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_3D, bpp);

        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0]); // As wide as 1 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);       // 1 tile wide
        VerifyResourceSize<false>(ResourceInfo, GMM_KBYTE(64));  // 1 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_3D, bpp);

        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<false>(ResourceInfo, GMM_KBYTE(64) * 2);  // 2 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 64KB
                                                      //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_3D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<false>(ResourceInfo, GMM_KBYTE(64) * 4);  // 4 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YS_3D, bpp);

        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth       = TileSize[i][2] + 1;                      // 1 plane larger than 1 tile depth

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64) * 8);   // 8 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 64KB
                                                      //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileYf Compressed Resource
TEST_F(CTestGen12Resource, Test3DTileYfCompressedResource)
{
    if(!const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
    {
        return;
    }
    // Horizontal/Verticle pixel alignment
    const uint32_t HAlign[TEST_BPP_MAX]      = {16, 8, 8, 8, 4};
    const uint32_t VAlign[TEST_BPP_MAX]      = {16, 16, 16, 8, 8};
    const uint32_t TileSize[TEST_BPP_MAX][3] = {{16, 16, 16},
                                                {16, 16, 16},
                                                {32, 16, 8},
                                                {64, 8, 8},
                                                {64, 8, 8}};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_3D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Info.TiledYf          = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1; // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
                                               // to 4x1 tile (reqd by HW for perf reasons)

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation
                                                   //gmmParams.Flags.Gpu.MMC = 0;              //Turn on to check unifiedaux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_3D, bpp);

        gmmParams.Format = SetResourceFormat(bpp);
        //gmmParams.BaseWidth64 = 0x30;
        //gmmParams.BaseHeight = 0x30;
        //gmmParams.Depth = 0x20;
        gmmParams.BaseWidth64         = 1;
        gmmParams.BaseHeight          = 1;
        gmmParams.Depth               = 1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i][0] * 4));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i][0] * 4) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, (GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i][0] * 4) / TileSize[i][0]) *
                                               (GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[i][1]) / TileSize[i][1]) *
                                               (GMM_ULT_ALIGN(gmmParams.Depth, TileSize[i][2]) / TileSize[i][2]) * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, (GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[i][1])));

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_3D, bpp);

        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = 0x1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;


        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment) / TileSize[i][0] * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_3D, bpp);

        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = TileSize[i][1] + 1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment) / TileSize[i][0] * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1] * 2);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_3D, bpp);

        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = TileSize[i][1] + 1;
        gmmParams.Depth               = TileSize[i][2] + 1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 4, PitchAlignment) / TileSize[i][0] * 2 * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1] * 2);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
            EXPECT_GE(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), ResourceInfo->GetSizeMainSurface() >> 8);
        }
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileY Compressed Resource
TEST_F(CTestGen12Resource, Test3DTileYCompressedResource)
{
    if(!const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
    {
        return;
    }
    // Horizontal/Verticle pixel alignment
    const uint32_t HAlign      = {16};
    const uint32_t VAlign      = {4};
    const uint32_t TileSize[3] = {128, 32, 1};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_3D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1; // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
                                               // to 4x1 tile (reqd by HW for perf reasons)

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;

        gmmParams.Format = SetResourceFormat(bpp);
        //gmmParams.BaseWidth64 = 0x30;
        //gmmParams.BaseHeight = 0x30;
        //gmmParams.Depth = 0x20;
        gmmParams.BaseWidth64         = 1;
        gmmParams.BaseHeight          = 1;
        gmmParams.Depth               = 1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[0] * 4));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[0] * 4) / TileSize[0]);
        VerifyResourceSize<true>(ResourceInfo, (GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[0] * 4) / TileSize[0]) *
                                               (GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]) / TileSize[1]) *
                                               (GMM_ULT_ALIGN(gmmParams.Depth, TileSize[2]) / TileSize[2]) * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, (GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1])));

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp             = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode        = LEGACY_TILE_Y;
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = 0x1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;


        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment) / TileSize[0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment) / TileSize[0] * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[1]);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp             = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode        = LEGACY_TILE_Y;
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = TileSize[1] + 1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment) / TileSize[0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment) / TileSize[0] * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[1] * 2);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp             = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode        = LEGACY_TILE_Y;
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = TileSize[1] + 1;
        gmmParams.Depth               = TileSize[2] + 1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment) / TileSize[0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[0] * 4, PitchAlignment) / TileSize[0] * 2 * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[1] * 2);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D Yf mipped compressed resource
TEST_F(CTestGen12Resource, Test2DTileYfMippedCompressedResource)
{
    if(!const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
    {
        return;
    }
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 64, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{64, 64},
                                                {128, 32},
                                                {128, 32},
                                                {256, 16},
                                                {256, 16}};

    const uint32_t MtsWidth[TEST_BPP_MAX]  = {32, 32, 16, 16, 8};
    const uint32_t MtsHeight[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_2D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Info.TiledYf          = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1; // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
                                               // to 4x1 tile (reqd by HW for perf reasons)
    gmmParams.MaxLod    = 4;
    gmmParams.ArraySize = 4;

    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        uint32_t AlignedWidth    = 0;
        uint32_t AlignedHeight   = 0;
        uint32_t ExpectedPitch   = 0;
        uint32_t MipTailStartLod = 0;
        // Valigned Mip Heights
        uint32_t Mip0Height    = 0;
        uint32_t Mip1Height    = 0;
        uint32_t Mip2Height    = 0;
        uint32_t Mip3Height    = 0;
        uint32_t Mip4Height    = 0;
        uint32_t Mip5Height    = 0;
        uint32_t Mip2Higher    = 0; // Sum of aligned heights of Mip2 and above
        uint32_t MipTailHeight = 0;
        // Haligned Mip Widths
        uint32_t Mip0Width = 0;
        uint32_t Mip1Width = 0;
        uint32_t Mip2Width = 0;

        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x38;
        gmmParams.BaseHeight  = 0x38;

        // If unifiedAuxSurf reqd (mandatory for displayable or cross-device shared), turn on .CCS/.MMC and .UnifiedAuxSurface too
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);

        // find the miptail start level
        {
            uint32_t MipWidth  = gmmParams.BaseWidth64;
            uint32_t MipHeight = gmmParams.BaseHeight;
            while(!(MipWidth <= MtsWidth[i] && MipHeight <= MtsHeight[i]))
            {
                MipTailStartLod++;
                MipWidth  = (uint32_t)(GMM_ULT_MAX(1, gmmParams.BaseWidth64 >> MipTailStartLod));
                MipHeight = GMM_ULT_MAX(1, gmmParams.BaseHeight >> MipTailStartLod);
            }
        }

        // Mip resource Aligned Width calculation
        Mip0Width  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign[i]);
        Mip0Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign[i]);

        if(MipTailStartLod == 1)
        {
            EXPECT_EQ(1, ResourceInfo->GetPackedMipTailStartLod());
            // Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
            Mip1Height   = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 1, VAlign[i]);
            AlignedWidth = Mip0Width;
        }
        if(MipTailStartLod == 2)
        {
            EXPECT_EQ(2, ResourceInfo->GetPackedMipTailStartLod());
            // Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
            Mip1Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 1, VAlign[i]);
            Mip2Height = Mip2Higher = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 2, VAlign[i]);

            Mip1Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> 1, HAlign[i]);
            Mip2Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> 2, HAlign[i]);
            AlignedWidth = GMM_ULT_MAX(Mip0Width, Mip1Width + Mip2Width);
        }
        if(MipTailStartLod == 3)
        {
            EXPECT_EQ(3, ResourceInfo->GetPackedMipTailStartLod());
            // Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
            Mip1Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 1, VAlign[i]);
            Mip2Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 2, VAlign[i]);
            // Miptail started lod
            MipTailHeight = VAlign[i];
            Mip2Higher    = Mip2Height + Mip3Height + MipTailHeight;

            Mip1Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> 1, HAlign[i]);
            Mip2Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> 2, HAlign[i]);
            AlignedWidth = GMM_ULT_MAX(Mip0Width, Mip1Width + Mip2Width);
        }

        uint32_t MaxHeight = GMM_ULT_MAX(Mip1Height, Mip2Higher);
        AlignedHeight      = Mip0Height + MaxHeight;
        AlignedHeight      = GMM_ULT_ALIGN(AlignedHeight, VAlign[i]);

        ExpectedPitch = AlignedWidth * GetBppValue(bpp);
        ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, GMM_BYTES(32));
        ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, TileSize[i][0] * 4); //Only for displayables - 16K pitch align
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);

        VerifyResourcePitchInTiles<true>(ResourceInfo, static_cast<uint32_t>(ExpectedPitch / TileSize[i][0]));
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * AlignedHeight * gmmParams.ArraySize, PAGE_SIZE));
        VerifyResourceQPitch<false>(ResourceInfo, AlignedHeight);
        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        //Aux-size enough to cover all
        if(const_cast<SKU_FEATURE_TABLE &>(pGfxAdapterInfo->SkuTable).FtrLinearCCS)
        {
            EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        }

        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Linear Buffer Compressed Resource
TEST_F(CTestGen12Resource, TestLinearCompressedResource)
{
    // Horizontal pixel alignment
    const uint32_t MinPitch = 32;

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_BUFFER;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.Linear           = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;

    // Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP bpp                             = static_cast<TEST_BPP>(i);
        gmmParams.Format                         = SetResourceFormat(bpp);
        gmmParams.BaseWidth64                    = 0x1;
        gmmParams.BaseHeight                     = 1;
        gmmParams.Flags.Info.AllowVirtualPadding = (bpp != TEST_BPP_8); //OCL uses 8bpp buffers. doc doesn't comment if Linear buffer compr allowed or not on bpp!=8.

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, MinPitch);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<false>(ResourceInfo, 0);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for buffer
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for buffer
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        //test main surface base alignment is 64KB
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());

        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }


    // Allocate more than 1 page
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp                             = static_cast<TEST_BPP>(i);
        gmmParams.Format                         = SetResourceFormat(bpp);
        gmmParams.BaseWidth64                    = 0x1001;
        gmmParams.BaseHeight                     = 1;
        gmmParams.Flags.Info.AllowVirtualPadding = (bpp != TEST_BPP_8); //OCL uses 8bpp buffers. document doesn't comment if Linear buffer compr allowed or not on bpp!=8.
        gmmParams.Flags.Gpu.UnifiedAuxSurface    = 1;          //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS                  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, MinPitch);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<false>(ResourceInfo, MinPitch);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for buffer
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for buffer
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        //test main surface base alignment is 64KB
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());

        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        ASSERT_LE(GMM_ULT_ALIGN(ResourceInfo->GetSizeMainSurface() / (GMM_KBYTE(16) / GMM_BYTES(64)), GMM_KBYTE(4)),
                  ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}
///TODO Add MSAA/Depth Compressed Resource tests
TEST_F(CTestGen12Resource, TestLosslessMSAACompressedResource)
{
}

///TODO Add MSAA/Depth Compressed Resource tests
TEST_F(CTestGen12Resource, DISABLED_TestDepthCompressedResource)
{
    const uint32_t HAlign = 8; //HiZ alignment (16x4 ie general alignment), [Depth 16bit: 8x8; ow 8x4]
    uint32_t       VAlign = 4; // 8; Need to debug why driver uses VAlign/2

    //const uint32_t DepthTileSize[1][2] = { 64, 64 };  //Depth/Stencil buffer should be TileY/Ys/Yf only (16,24,32 bpp only) no 3D or MSAA
    const uint32_t AllocTileSize[1][2] = {128, 32}; //HiZ is TileY

    GMM_RESCREATE_PARAMS gmmParams         = {};
    gmmParams.NoGfxMemory                  = 1;
    gmmParams.Flags.Info.TiledY            = 1; //Not supported for Depth buffer, but HiZ output is TileY
    gmmParams.Flags.Gpu.Depth              = 1; //GPU Flags= Depth/SeparateStencil + HiZ
    gmmParams.Flags.Gpu.HiZ                = 1;
    gmmParams.Flags.Gpu.IndirectClearColor = 1;
    gmmParams.Flags.Gpu.CCS                = 1;
    gmmParams.Flags.Info.RenderCompressed  = 1;
    gmmParams.Flags.Gpu.UnifiedAuxSurface  = 1;

    // Allocate 1x1 surface so that it occupies 1 Tile in X dimension
    for(uint32_t j = TEST_BPP_8; j <= TEST_BPP_128; j++) //Depth bpp doesn't matter, Depth px dimensions decide HiZ size in HW
    {
        {
            VAlign = (j == TEST_BPP_16) ? 8 : 4;
        }
        gmmParams.Format = SetResourceFormat(static_cast<TEST_BPP>(j)); //Only 16,24,32 supported; But driver creates the resource even for other bpps without failing
        for(uint32_t i = RESOURCE_2D; i <= RESOURCE_CUBE; i++)          //3D doesn't support HiZ - test driver returns proper?
        {
            gmmParams.Type        = static_cast<GMM_RESOURCE_TYPE>(i);
            gmmParams.BaseWidth64 = 0x1;
            gmmParams.BaseHeight  = 0x1; //0x24; //not 1 tile
            //gmmParams.MaxLod      = 6; --add expectedheight calc- mip0+max{mip1, sum{mip2,...n}}
            gmmParams.Depth = 0x1;
            if(i == RESOURCE_1D || i == RESOURCE_3D)
            {
                gmmParams.Flags.Gpu.HiZ = 0;
            }

            GMM_RESOURCE_INFO *ResourceInfo = NULL;
            ResourceInfo                    = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            //EXPECT_NE(NULL, ResourceInfo);

            VerifyResourceHAlign<true>(ResourceInfo, HAlign);
            VerifyResourceVAlign<true>(ResourceInfo, VAlign);
            uint32_t ExpectedPitch = AllocTileSize[0][0] * 4;
            VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
            VerifyResourcePitchInTiles<true>(ResourceInfo, 4); // 1 tileY wide
            uint32_t ExpectedHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);

            if(gmmParams.ArraySize > 1 || gmmParams.Type == RESOURCE_CUBE)
            {
                uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign); //Apply formula as per specification
                ExpectedQPitch          = GMM_ULT_ALIGN(ExpectedQPitch / 2, VAlign);
                ExpectedHeight *= (gmmParams.Type == RESOURCE_CUBE) ? 6 : 1;

                VerifyResourceQPitch<false>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile, Turn on verification after clarity
            }

            VerifyResourceSize<true>(ResourceInfo,
                                     GFX_ALIGN(ExpectedPitch * ExpectedHeight, 4 * PAGE_SIZE)); //1 Tile should be enough
            pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
        }

        // Allocate 2 tiles in X dimension. (muti-tiles Tiles in Y dimension for cube/array)
        for(uint32_t i = RESOURCE_2D; i <= RESOURCE_CUBE; i++)
        {
            gmmParams.Type        = static_cast<GMM_RESOURCE_TYPE>(i);
            gmmParams.BaseWidth64 = AllocTileSize[0][0] + 0x1;
            gmmParams.BaseHeight  = (gmmParams.Type == RESOURCE_1D) ? 0x1 :
                                                                     (gmmParams.Type == RESOURCE_CUBE) ? gmmParams.BaseWidth64 :
                                                                                                         VAlign / 2;
            gmmParams.ArraySize = (gmmParams.Type != RESOURCE_3D) ? VAlign : 1; // Gen8 doesn't support 3D-arrays (so HiZ not supported) [test 3d arrays once -- HiZ would fail but ResCreate doesn't?]
            gmmParams.Depth     = 0x1;

            GMM_RESOURCE_INFO *ResourceInfo;
            ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            VerifyResourceHAlign<true>(ResourceInfo, HAlign);
            VerifyResourceVAlign<true>(ResourceInfo, VAlign);
            uint32_t ExpectedPitch = GFX_ALIGN(gmmParams.BaseWidth * (int)pow(2, j), AllocTileSize[0][0] * 4);
            VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
            VerifyResourcePitchInTiles<true>(ResourceInfo, 4); // 2 tileY wide

            uint32_t ExpectedQPitch = 0;
            if(gmmParams.ArraySize > 1 || gmmParams.Type == RESOURCE_CUBE)
            {
                ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
                //ExpectedQPitch = GMM_ULT_ALIGN(ExpectedQPitch / 2, VAlign);

                VerifyResourceQPitch<false>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile. Turn on verification after clarity
            }

            VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = (__GMM_MAX_CUBE_FACE x QPitch) /2 (Stencil height = halved due to interleaving), then aligned to tile boundary
                                     ((gmmParams.Type == RESOURCE_CUBE) ?
                                      ExpectedPitch * GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.ArraySize * __GMM_MAX_CUBE_FACE, AllocTileSize[0][1]) : //cube
                                      ((gmmParams.ArraySize > 1) ?
                                       ExpectedPitch * GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.ArraySize, AllocTileSize[0][1]) : //array
                                       4 * GMM_KBYTE(4))));

            pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
        }

        // Allocate 2 tiles in X/Y dimension (non-arrayed) Multi-tiles for 3D
        for(uint32_t i = RESOURCE_2D; i <= RESOURCE_3D; i++)
        {
            gmmParams.Type        = static_cast<GMM_RESOURCE_TYPE>(i);
            gmmParams.BaseWidth64 = AllocTileSize[0][0] + 0x1;
            gmmParams.BaseHeight  = 2 * AllocTileSize[0][1] + 0x1; //Half-Depth Height or QPitch (lod!=0), aligned to 8 required by HW
            gmmParams.Depth       = (gmmParams.Type == RESOURCE_2D) ? 0x1 :
                                                                VAlign + 1;
            gmmParams.ArraySize = 1;

            GMM_RESOURCE_INFO *ResourceInfo;
            ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            VerifyResourceHAlign<true>(ResourceInfo, HAlign);
            VerifyResourceVAlign<true>(ResourceInfo, VAlign);
            uint32_t ExpectedPitch = AllocTileSize[0][0] * 4;
            VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
            VerifyResourcePitchInTiles<true>(ResourceInfo, 4); // 2 tile wide

            uint32_t TwoDQPitch, ExpectedQPitch = 0;
            if(gmmParams.Type == RESOURCE_3D)
            {
                TwoDQPitch     = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
                ExpectedQPitch = gmmParams.Depth * GMM_ULT_ALIGN(TwoDQPitch / 2, VAlign); //Depth slices arranged as 2D-arrayed slices.
            }
            else
            {
                //HiZ for 3D not supported. Driver still allocates like IVB/HSW. (should Qpitch or only overall buffer height be Valigned ?)
                VerifyResourceSize<true>(ResourceInfo,
                                         ((gmmParams.Type == RESOURCE_3D) ?
                                          ExpectedPitch * GMM_ULT_ALIGN(ExpectedQPitch, AllocTileSize[0][1]) :
                                          2 * 2 * GMM_KBYTE(4)));
            }
            pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
        }
    }
}

TEST_F(CTestGen12Resource, TestStencilCompressedResource)
{
    const uint32_t HAlign = {16};
    const uint32_t VAlign = {8};

    const uint32_t TileSize[2] = {128, 32};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    //gmmParams.ArraySize = 4;

    gmmParams.Flags.Gpu.SeparateStencil   = 1;
    gmmParams.Flags.Info.RenderCompressed = 1;
    // Turn on .MC or .RC flag - mandatory to tell compression-type, for Yf its also used to pad surf
    // to 4x1 tile (reqd by HW for perf reasons)
    // If unifiedAuxSurf reqd (mandatory for displayable or cross-adapter shared), turn on .CCS/.MMC and .UnifiedAuxSurface too

    //Allocate 1x1 surface
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface  = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS                = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.IndirectClearColor = 1;

        TEST_BPP      bpp      = static_cast<TEST_BPP>(TEST_BPP_8);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = 0x1;
        gmmParams.BaseHeight   = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, 4 * TileSize[0]); // As wide as 4 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 4 Tile wide
        VerifyResourceSize<true>(ResourceInfo, 4 * GMM_KBYTE(4)); // 4 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);             // Not Tested

        //test main surface base alignment is 64KB
        //For Yf test main surface pitch is 4-tileYF aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4); // Check on YF only

        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(TEST_BPP_8);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = 0x1;
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0] * 4); // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);        // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4); // 2 tile big, but 4-tile pitch alignment

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 64KB
        //For Y test main surface pitch is 4-tileY aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);
        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X/Y dimension
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface  = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS                = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.IndirectClearColor = 1;

        TEST_BPP      bpp      = static_cast<TEST_BPP>(TEST_BPP_8);
        GMM_TILE_MODE TileMode = LEGACY_TILE_Y;
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = TileSize[1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0] * 4);     // As wide as 2 tile, but 4-tile pitch alignment
        VerifyResourcePitchInTiles<true>(ResourceInfo, 4);            // 2 tile wide, but 4-tile pitch alignment
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4 * 2); // 4 tile wide; and 2-tile high

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 64KB
                                                      //For Y test main surface pitch is 4-tileY aligned
        EXPECT_EQ(GMM_KBYTE(64), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetRenderPitchTiles() % 4);
        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}


/// @brief ULT for 2D TileYf Compressed Resource
TEST_F(CTestGen12Resource, Test2DTileYfAMFSResource)
{
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 64, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{64, 64},
                                                {128, 32},
                                                {128, 32},
                                                {256, 16},
                                                {256, 16}};

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_2D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.Flags.Info.TiledYf          = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Gpu.ProceduralTexture = 1;
    // If unifiedAuxSurf reqd (mandatory for displayable or cross-adapter shared), turn on .CCS/.MMC and .UnifiedAuxSurface too

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation
                                                   //gmmParams.Flags.Gpu.MMC = 0;              //Turn on to check unifiedaux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = 0x1;
        gmmParams.BaseHeight   = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0]); // As wide as 1 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);       // 1 Tile wide
        //VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4));    // 1 Tile Big,  old behaviour consuming bytes for main-surface (paired Texture dimensions) only used to obtain GPUVA for indirect (Auxtable mapped) CCS access by sampler.
	VerifyResourceSize<true>(ResourceInfo, 0);    // New behaviour, optimized SFT size, sampler doesn't access CCS via main.. kernels refer the CCS-via its GPUVA (w/o main).
        VerifyResourceQPitch<false>(ResourceInfo, 0);            // Not Tested

        //test main surface base alignment is 4KB, since AMFS PT isn't compressed
        //but uses same linear CCS as compression
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));


        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            EXPECT_EQ(0, AuxResourceInfo->GmmGetTileMode());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, bpp);

        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        //VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 2);    // 2 tile big, old behaviour consuming bytes for main-surface (paired Texture dimensions) only used to obtain GPUVA for indirect (Auxtable mapped) CCS access by sampler.
        VerifyResourceSize<true>(ResourceInfo, 0); // New behaviour, optimized SFT size, sampler doesn't access CCS via main.. kernels refer the CCS-via its GPUVA (w/o main).

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        //test main surface base alignment is 4KB
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            EXPECT_EQ(0, AuxResourceInfo->GmmGetTileMode());

            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        gmmParams.Flags.Gpu.UnifiedAuxSurface = 1; //Turn off for separate aux creation
        gmmParams.Flags.Gpu.CCS               = 1; //Turn off for separate aux creation

        TEST_BPP      bpp      = static_cast<TEST_BPP>(i);
        GMM_TILE_MODE TileMode = DEFINE_TILE(YF_2D, bpp);
        gmmParams.Format       = SetResourceFormat(bpp);
        gmmParams.BaseWidth64  = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight   = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth        = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2);  // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);            // 2 tile wide
        //VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 2 * 2); // 2 tile wide; and 2-tile high, old behaviour consuming bytes for main-surface (paired Texture dimensions) only used to obtain GPUVA for indirect (Auxtable mapped) CCS access by sampler.
        VerifyResourceSize<true>(ResourceInfo, 0); // New behaviour, optimized SFT size, sampler doesn't access CCS via main.. kernels refer the CCS-via its GPUVA (w/o main).

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested
                                                      //test main surface base alignment is 4KB
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetBaseAlignment());
        EXPECT_EQ(0, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS) % PAGE_SIZE);
        EXPECT_EQ(GMM_KBYTE(4), ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

        { //separate Aux
            gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;

            GMM_RESOURCE_INFO *AuxResourceInfo;
            AuxResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);
            EXPECT_EQ(ResourceInfo->GetAuxHAlign(), AuxResourceInfo->GetHAlign());
            EXPECT_EQ(ResourceInfo->GetAuxVAlign(), AuxResourceInfo->GetVAlign());
            EXPECT_EQ(ResourceInfo->GetUnifiedAuxPitch(), AuxResourceInfo->GetRenderPitch());
            EXPECT_EQ(ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS), AuxResourceInfo->GetSizeSurface());
            EXPECT_EQ(0, AuxResourceInfo->GmmGetTileMode());
            pGmmULTClientContext->DestroyResInfoObject(AuxResourceInfo);
        }
        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for MSAA Resource - TODO adddepth MSAA, MCS surf param verificaton, compression case
TEST_F(CTestGen12Resource, TestColorMSAA)
{
    //Tile dimensions in Bytes
    const uint32_t MCSTileSize[1][2] = {128, 32}; //MCS is TileY

    const uint32_t TestDimensions[4][2] = {
    //Input dimensions in #Tiles
    {15, 20}, //16 Tiles x 20 <Max Width: Depth MSS crosses Pitch limit beyond this>
    {0, 0},   //1x1x1
    {1, 0},   //2 Tilesx1
    {1, 1},   //2 Tiles x 2
    };

    uint32_t TestArraySize[2] = {1, 5};
    uint32_t MinPitch         = 32;

    uint32_t HAlign, VAlign, TileDimX, TileDimY, MCSHAlign, MCSVAlign, TileSize;
    uint32_t ExpectedMCSBpp;
    std::vector<tuple<int, int, int, bool, int, int>> List; //TEST_TILE_TYPE, TEST_BPP, TEST_RESOURCE_TYPE, Depth or RT, TestDimension index, ArraySize
    auto Size = BuildInputIterator(List, 4, 2);             // Size of arrays TestDimensions, TestArraySize

    for(auto element : List)
    {
        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Flags.Info           = {0};

        TEST_TILE_TYPE     Tiling     = (TEST_TILE_TYPE)std::get<0>(element);
        TEST_BPP           Bpp        = (TEST_BPP)std::get<1>(element);
        TEST_RESOURCE_TYPE ResType    = (TEST_RESOURCE_TYPE)std::get<2>(element);
        bool               IsRT       = std::get<3>(element); // True for RT, False for Depth
        int                TestDimIdx = std::get<4>(element); //index into TestDimensions array
        int                ArrayIdx   = std::get<5>(element); //index into TestArraySize
        TileSize                      = (Tiling == TEST_TILEYS) ? GMM_KBYTE(64) : GMM_KBYTE(4);

        //Discard un-supported Tiling/Res_type/bpp for this test
        if(ResType != TEST_RESOURCE_2D ||                            //No 1D/3D/Cube. Supported 2D mip-maps/array
           (!IsRT && (Tiling == TEST_TILEX ||                        // doesn't support TileX for Depth
                      !(Bpp == TEST_BPP_16 || Bpp == TEST_BPP_32)))) //depth supported on 16bit, 32bit formats only
            continue;

        if(!IsRT)
            continue; //comment depth msaa for now (requires change in h/v align)

        SetTileFlag(gmmParams, Tiling);
        SetResType(gmmParams, ResType);
        SetResGpuFlags(gmmParams, IsRT);
        SetResArraySize(gmmParams, TestArraySize[ArrayIdx]);

        gmmParams.NoGfxMemory = 1;
        gmmParams.Format      = SetResourceFormat(Bpp);
        for(uint32_t k = MSAA_2x; k <= MSAA_16x; k++)
        {
            GetAlignmentAndTileDimensionsForMSAA(Bpp, IsRT, Tiling, (TEST_MSAA)k,
                                                 TileDimX, TileDimY, HAlign, VAlign,
                                                 ExpectedMCSBpp, MCSHAlign, MCSVAlign);

            //gmmParams.BaseWidth64 = TestDimensions[TestDimIdx][0] * TileDimX + 0x1;
            //gmmParams.BaseHeight = TestDimensions[TestDimIdx][1] * TileDimY + 0x1;
            gmmParams.BaseWidth64     = 4;
            gmmParams.BaseHeight      = 4;
            gmmParams.Depth           = 0x1;
            gmmParams.MSAA.NumSamples = static_cast<uint32_t>(pow((double)2, k));
            gmmParams.Flags.Gpu.MCS   = 0;

            //MSS surface
            GMM_RESOURCE_INFO *MSSResourceInfo;
            MSSResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            if(MSSResourceInfo)
            {
                VerifyResourceHAlign<true>(MSSResourceInfo, HAlign);
                VerifyResourceVAlign<true>(MSSResourceInfo, VAlign);
                if(IsRT) //Arrayed MSS
                {
                    uint32_t ExpectedPitch = 0, ExpectedQPitch = 0;
                    ExpectedPitch = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign) * (int)pow(2.0, Bpp), TileDimX); // Aligned width * bpp, aligned to TileWidth
                    ExpectedPitch = GFX_MAX(ExpectedPitch, MinPitch);
                    VerifyResourcePitch<true>(MSSResourceInfo, ExpectedPitch);
                    if(Tiling != TEST_LINEAR)
                        VerifyResourcePitchInTiles<true>(MSSResourceInfo, ExpectedPitch / TileDimX);

                    ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
                    if(gmmParams.ArraySize > 1) //Gen9: Qpitch is distance between array slices (not sample slices)
                    {
                        VerifyResourceQPitch<true>(MSSResourceInfo, ExpectedQPitch);
                    }

                    uint32_t ExpectedHeight = GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.MSAA.NumSamples * gmmParams.ArraySize, TileDimY); //Align Height =ExpectedPitch * NumSamples * ExpectedQPitch, to Tile-Height
                    VerifyResourceSize<true>(MSSResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * ExpectedHeight, TileSize));
                }
            }

            pGmmULTClientContext->DestroyResInfoObject(MSSResourceInfo);
        } //NumSamples = k
    }     //Iterate through all Input types

    //Mip-mapped, MSAA case:
}
