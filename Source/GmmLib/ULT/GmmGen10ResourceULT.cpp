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

#include "GmmGen10ResourceULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Resource fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
//  It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/// @see    CTestGen9Resource::SetUpTestCase()
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen10Resource::SetUpTestCase()
{
    printf("%s\n", __FUNCTION__);
    GfxPlatform.eProductFamily    = IGFX_CANNONLAKE;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN10_CORE;

    CommonULT::SetUpTestCase();
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/// @see    CTestGen10Resource::TearDownTestCase()
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen10Resource::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

/// @brief ULT for 2D TileYs Resource
TEST_F(CTestGen10Resource, Test2DTileYsResource)
{
    printf("%s\n", __FUNCTION__);
}

/// @brief ULT for 2D TileYf Resource
TEST_F(CTestGen10Resource, Test2DTileYfResource)
{
    printf("%s\n", __FUNCTION__);
}

TEST_F(CTestGen10Resource, TestMSAA)
{
    //Tile dimensions in Bytes
    const uint32_t MCSTileSize[1][2] = {128, 32}; //MCS is TileY

    //Gen9: MSAA 16x no MCS for width > 8K
    //No MSAA for YUV/compressed formats
    //Interleaved MSS (IMS) for Depth/Stencil. Arrayed MSS (CMS) for Color RT
    //MSS (Arrayed): px_wL, px_hL = pixel width/height of single sample at Lod L
    //  MSS width = px_wL, MSS height = NumSamples*px_hL
    //MSS (Interleaved): px_wL, px_hL = pixel width/height of single sample at Lod L
    // Samples         MSS width                MSS Height
    //   2x            4*ceil(px_wL/2)             px_hL
    //   4x            4*ceil(px_wL/2)           4*ceil(px_hL/2)
    //   8x            8*ceil(px_wL/2)           4*ceil(px_hL/2)
    //  16x            8*ceil(px_wL/2)           8*ceil(px_hL/2)
    //MCS (bpp): 2x/4x - bpp_8, 8x - bpp_32, 16x - bpp_64

    const uint32_t TestDimensions[4][2] = {
    //Input dimensions in #Tiles
    {15, 20}, //16 Tiles x 20 <Max Width: Depth MSS crosses Pitch limit beyond this>
    {0, 0},   //1x1x1
    {1, 0},   //2 Tilesx1
    {1, 1},   //2 Tiles x 2
    };

    uint32_t TestArraySize[2] = {1, 5};
    uint32_t MinPitch         = 32;

    uint32_t HAlign = 0, VAlign = 0;
    uint32_t TileDimX = 0, TileDimY = 0;
    uint32_t MCSHAlign = 0, MCSVAlign = 0, TileSize = 0;
    uint32_t ExpectedMCSBpp = 0;
    std::vector<tuple<int, int, int, bool, int, int>> List; //TEST_TILE_TYPE, TEST_BPP, TEST_RESOURCE_TYPE, Depth or RT, TestDimension index, ArraySize
    auto Size = BuildInputIterator(List, 4, 2, false);             // Size of arrays TestDimensions, TestArraySize

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
           (!IsRT && (Tiling == TEST_TILEX ||
                      !(Bpp == TEST_BPP_16 || Bpp == TEST_BPP_32)))) //depth supported on 16bit, 32bit formats only
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

            gmmParams.BaseWidth64     = TestDimensions[TestDimIdx][0] * TileDimX + 0x1;
            gmmParams.BaseHeight      = TestDimensions[TestDimIdx][1] * TileDimY + 0x1;
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
                    ExpectedPitch = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign) * (uint32_t)pow(2.0, Bpp), TileDimX); // Aligned width * bpp, aligned to TileWidth
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
                else // Interleaved MSS
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

                    uint64_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight * HeightMultiplier, VAlign);
                    if(gmmParams.ArraySize > 1)
                    {
                        VerifyResourceQPitch<true>(MSSResourceInfo, ExpectedQPitch);
                    }
                    uint64_t ExpectedHeight = GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.ArraySize, TileDimY);            //Align Height = ExpectedQPitch*ArraySize, to Tile-Height
                    VerifyResourceSize<true>(MSSResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * ExpectedHeight, TileSize)); //ExpectedPitch *ExpectedHeight
                }
            }

            //No MCS surface if MSS creation failed
            if(MSSResourceInfo)
            {
                gmmParams.Flags.Gpu.MCS = 1;
                GMM_RESOURCE_INFO *MCSResourceInfo;
                MCSResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

                VerifyResourceHAlign<true>(MCSResourceInfo, MCSHAlign);
                VerifyResourceVAlign<true>(MCSResourceInfo, MCSVAlign);

                uint32_t ExpectedPitch = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseWidth64, MCSHAlign) * ExpectedMCSBpp, MCSTileSize[0][0]); // Align in texels, tehn multiply w/ Bpt
                VerifyResourcePitch<true>(MCSResourceInfo, ExpectedPitch);
                VerifyResourcePitchInTiles<true>(MCSResourceInfo, ExpectedPitch / MCSTileSize[0][0]);

                uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, MCSVAlign);
                if(gmmParams.ArraySize > 1)
                {
                    ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, MCSVAlign); //QPitch only for array
                    VerifyResourceQPitch<true>(MCSResourceInfo, ExpectedQPitch);
                }

                uint32_t ExpectedHeight = GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.ArraySize, MCSTileSize[0][1]);
                VerifyResourceSize<true>(MCSResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * ExpectedHeight, GMM_KBYTE(4))); //MCS Tile is TileY

                pGmmULTClientContext->DestroyResInfoObject(MCSResourceInfo);
            } //MCS

            pGmmULTClientContext->DestroyResInfoObject(MSSResourceInfo);
        } //NumSamples = k
    }     //Iterate through all Input types

    //Mip-mapped, MSAA case:
}
