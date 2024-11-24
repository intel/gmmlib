/*==============================================================================
Copyright(c) 2024 Intel Corporation

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

#include "GmmXe2_LPGResourceULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Resource fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
//  It also calls SetupTestCase from CommonULT to initialize global context and others.
///
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestXe2_LPGResource::SetUpTestCase()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestXe2_LPGResource::TearDownTestCase()
{
}

void CTestXe2_LPGResource::SetUp_Xe2Variant(PRODUCT_FAMILY platform)
{
    printf("%s\n", __FUNCTION__);

    if (platform == IGFX_BMG)
    {
        GfxPlatform.eProductFamily    = IGFX_BMG;
        GfxPlatform.eRenderCoreFamily = IGFX_XE2_HPG_CORE;
    }
    else if (platform == IGFX_LUNARLAKE)
    {
        GfxPlatform.eProductFamily = IGFX_LUNARLAKE;
        GfxPlatform.eRenderCoreFamily = IGFX_XE2_LPG_CORE;
    }

    pGfxAdapterInfo = (ADAPTER_INFO*)malloc(sizeof(ADAPTER_INFO));
    if (pGfxAdapterInfo)
    {
        memset(pGfxAdapterInfo, 0, sizeof(ADAPTER_INFO));

        pGfxAdapterInfo->SkuTable.FtrLinearCCS = 1; //legacy y =>0 
        pGfxAdapterInfo->SkuTable.FtrStandardMipTailFormat = 1;
        pGfxAdapterInfo->SkuTable.FtrTileY = 0;
        pGfxAdapterInfo->SkuTable.FtrXe2PlusTiling = 1;
        pGfxAdapterInfo->SkuTable.FtrXe2Compression = 1;
        pGfxAdapterInfo->SkuTable.FtrFlatPhysCCS = 1;
        pGfxAdapterInfo->SkuTable.FtrLocalMemory = 0;
        pGfxAdapterInfo->SkuTable.FtrDiscrete = 0;
        pGfxAdapterInfo->SkuTable.FtrE2ECompression = 1;

        if (platform == IGFX_BMG)
        {
            pGfxAdapterInfo->SkuTable.FtrLocalMemory = 1;
            pGfxAdapterInfo->SkuTable.FtrDiscrete    = 1;
        }

        if (platform == IGFX_LUNARLAKE || platform == IGFX_BMG)
        {
            pGfxAdapterInfo->WaTable.Wa_14018976079           = 1;
            pGfxAdapterInfo->WaTable.Wa_14018984349           = 1;
            pGfxAdapterInfo->SkuTable.FtrL3TransientDataFlush = 1;
        }

        CommonULT::SetUpTestCase();
    }
}

void CTestXe2_LPGResource::TearDown_Xe2Variant()
{
    printf("%s\n", __FUNCTION__);
    CommonULT::TearDownTestCase();
}

TEST_F(CTestXe2_LPGResource, TestMSAA_BMG)
{
    SetUp_Xe2Variant(IGFX_BMG);
    TestMSAA();
    TearDown_Xe2Variant();
}

TEST_F(CTestXe2_LPGResource, TestMSAA_LNL)
{
    SetUp_Xe2Variant(IGFX_LUNARLAKE);
    TestMSAA();
    TearDown_Xe2Variant();
}

/// @brief ULT for MSAA Resource - Depth and Colour MSAA verification for Tile64 resources
//  Note: Verify with and without FtrXe2PlusTiling in Setup, Default: FtrXe2PlusTiling
void CTestXe2_LPGResource::TestMSAA()
{	
    GMM_GFX_SIZE_T AuxCC, AuxCCS, AuxMCS;
    const uint32_t TestDimensions[4][2] = {
    //Input dimensions in #Tiles
    {16, 4},    //occupies single tile for Depth for all MSAAs and BPPs, multiple tiles for colours
    {128, 128}, // crosses a tile for > 4X MSAA for depth
    {128, 257}, // Crosses a tile in Y direction and for >4X MSAA, crosses in X direction too for depth
    {1, 1},
    };

    uint32_t TestArraySize[2] = {1, 5};
    uint32_t MinPitch         = 32;

    uint32_t HAlign, VAlign, TileDimX, TileDimY, MCSHAlign, MCSVAlign, TileSize;
    uint32_t ExpectedMCSBpp;
    std::vector<tuple<int, int, int, bool, int, int>> List; //TEST_TILE_TYPE, TEST_BPP, TEST_RESOURCE_TYPE, Depth or RT, TestDimension index, ArraySize
    auto Size = BuildInputIterator(List, 4, 2, true);       // Size of arrays TestDimensions, TestArraySize

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
        TileSize                      = (Tiling == TEST_TILE64) ? GMM_KBYTE(64) : GMM_KBYTE(4);

        //Discard un-supported Tiling/Res_type/bpp for this test
        if((ResType != TEST_RESOURCE_2D) || //No 1D/3D/Cube. Supported 2D mip-maps/array
                                            // depth tested outside this function due to diff in halign/valign
           (Tiling != TEST_TILE64))         // MSAA not supported on Tile4
            continue;

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
            gmmParams.BaseWidth64     = TestDimensions[TestDimIdx][0] * (unsigned int)pow(2.0, Bpp);
            gmmParams.BaseHeight                   = TestDimensions[TestDimIdx][1];
            gmmParams.Depth                        = 0x1;
            gmmParams.MSAA.NumSamples              = static_cast<uint32_t>(pow((double)2, k));
            gmmParams.Flags.Gpu.MCS                = 1;
            gmmParams.Flags.Gpu.CCS                = 1;
            gmmParams.Flags.Gpu.UnifiedAuxSurface  = 1;
            gmmParams.Flags.Gpu.IndirectClearColor = 1;
            //MSS surface
            GMM_RESOURCE_INFO *MSSResourceInfo;
            MSSResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            if(MSSResourceInfo)
            {
                VerifyResourceHAlign<true>(MSSResourceInfo, HAlign);
                VerifyResourceVAlign<true>(MSSResourceInfo, VAlign);

                if (gmmParams.Flags.Gpu.IndirectClearColor && pGfxAdapterInfo->SkuTable.FtrXe2Compression && (gmmParams.MSAA.NumSamples > 1))
                {
                    AuxCC  = MSSResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC);
                    AuxMCS = MSSResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_MCS);
                    AuxCCS = MSSResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);

                    EXPECT_EQ(AuxCC, AuxMCS);
                    EXPECT_EQ(AuxCCS, 0);

                    AuxCC  = MSSResourceInfo->GetSizeAuxSurface(GMM_AUX_CC);
                    AuxMCS = MSSResourceInfo->GetSizeAuxSurface(GMM_AUX_MCS);
                    AuxCCS = MSSResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS);

                    EXPECT_EQ(AuxCC, AuxMCS);
                    EXPECT_EQ(AuxCCS, 0);
                }
                if(IsRT) //Arrayed MSS
                {
                    uint32_t ExpectedPitch = 0, ExpectedQPitch = 0, ExpectedHeight = 0;
                    ExpectedPitch = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign) * (unsigned int)pow(2.0, Bpp), TileDimX); // Aligned width * bpp, aligned to TileWidth
                    ExpectedPitch = GFX_MAX(ExpectedPitch, MinPitch);
                    VerifyResourcePitch<true>(MSSResourceInfo, ExpectedPitch);
                    if(Tiling != TEST_LINEAR)
                        VerifyResourcePitchInTiles<true>(MSSResourceInfo, ExpectedPitch / TileDimX);

                    ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
                    if(gmmParams.ArraySize > 1) //Gen9: Qpitch is distance between array slices (not sample slices)
                    {
                        VerifyResourceQPitch<true>(MSSResourceInfo, ExpectedQPitch);
                    }

                    ExpectedHeight = GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.MSAA.NumSamples * gmmParams.ArraySize, TileDimY); // For Tile64 layout prior to Xe2Tiling, MSAA8x and 16x follows MSAA4x. MSAA4x*2 for MSAA8x and MSAA4x*4 for MSAA16x.
                                                                                                                                // Height getting multiplied by numsamples here is good enough for these special layouts too
                    VerifyResourceSize<true>(MSSResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * ExpectedHeight, TileSize));
                }
                else
                {
                    uint32_t WidthMultiplier, HeightMultiplier;
                    GetInterleaveMSSPattern((TEST_MSAA)k, WidthMultiplier, HeightMultiplier, IsRT, Bpp);
                    gmmParams.BaseWidth64 = WidthMultiplier > 1 ? GMM_ULT_ALIGN(gmmParams.BaseWidth64, 2) : gmmParams.BaseWidth64;
                    gmmParams.BaseHeight  = HeightMultiplier > 1 ? GMM_ULT_ALIGN(gmmParams.BaseHeight, 2) : gmmParams.BaseHeight;

                    uint32_t ExpectedPitch = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseWidth64 * WidthMultiplier, HAlign) * (uint32_t)pow(2.0, Bpp), TileDimX);
                    VerifyResourcePitch<true>(MSSResourceInfo, ExpectedPitch);
                    if(Tiling != TEST_LINEAR)
                    {
                        VerifyResourcePitchInTiles<true>(MSSResourceInfo, ExpectedPitch / TileDimX);
                    }

                    uint64_t ExpectedQPitch    = GMM_ULT_ALIGN(gmmParams.BaseHeight * HeightMultiplier, VAlign);
                    uint32_t ExpandedArraySize = gmmParams.ArraySize * (((MSSResourceInfo->GetTileType() == GMM_TILED_64) && !pGfxAdapterInfo->SkuTable.FtrTileY && !pGfxAdapterInfo->SkuTable.FtrXe2PlusTiling && (gmmParams.MSAA.NumSamples == 16)) ? 4 : // MSAA x8/x16 stored as pseudo array planes each with 4x samples
                                                                        ((MSSResourceInfo->GetTileType() == GMM_TILED_64) && !pGfxAdapterInfo->SkuTable.FtrTileY && !pGfxAdapterInfo->SkuTable.FtrXe2PlusTiling && (gmmParams.MSAA.NumSamples == 8)) ? 2 : 1);
                    if(ExpandedArraySize > 1)
                    {
                        VerifyResourceQPitch<true>(MSSResourceInfo, ExpectedQPitch);
                    }
                    uint64_t ExpectedHeight = GMM_ULT_ALIGN(ExpectedQPitch * ExpandedArraySize, TileDimY);              //Align Height = ExpectedQPitch*ArraySize, to Tile-Height
                    VerifyResourceSize<true>(MSSResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * ExpectedHeight, TileSize)); //ExpectedPitch *ExpectedHeight
                }
            }

            pGmmULTClientContext->DestroyResInfoObject(MSSResourceInfo);
        } //NumSamples = k
    }     //Iterate through all Input types{

}
