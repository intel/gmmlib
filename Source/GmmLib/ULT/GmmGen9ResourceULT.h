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

#include "GmmResourceULT.h"

class CTestGen9Resource : public CTestResource
{
protected:
    void                FillExpectedPitch();
    void                FillExpectedPitchInTiles();
    void                FillExpectedHAlign();
    void                FillExpectedVAlign();
    void                FillExpectedDAlign();
    void                FillExpectedSize();
    void                FillExpectedQPitch();

    void                FillExpectedMipOffsets();

public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    /////////////////////////////////////////////////////////////////////////////////////
    /// Get the Tile dimension and RT->CCS downscale factor
    ///
    /// @param[in]  Bpp: bits per pixel
    /// @param[in]  Tiling: Tile Type
    /// @param[in]  ResType: Resource Type
    /// @param[out] TileDimX: Tile Width for given Tile, Resource, bpp
    /// @param[out] TileDimY: Tile Height for given Tile, Resource, bpp
    /// @param[out] TileDimZ: Tile Depth for given Tile, Resource, bpp
    /// @param[out] WidthDivisor: RT->CCS width downscale factor
    /// @param[out] HeightDivisor: RT->CCS height downscale factor
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void GetAlignmentAndTileDimensionsForCCS(TEST_BPP Bpp, TEST_TILE_TYPE Tiling, TEST_RESOURCE_TYPE ResType,
                                        uint32_t &TileDimX, uint32_t &TileDimY, uint32_t & TileDimZ,
                                        uint32_t &WidthDivisor, uint32_t &HeightDivisor)
    {
        const uint32_t RT2DTileSize[TEST_TILE_MAX][TEST_BPP_MAX][3] = {
            { { 64, 1, 1 },{ 64, 1, 1 },{ 64, 1, 1 },{ 64, 1, 1 },{ 64, 1, 1 } },                  //Linear - no Tile Size, but min PitchAlign = 64 (cacheLine size)
            { { 512, 8, 1 },{ 512, 8,1 },{ 512, 8,1 },{ 512, 8, 1 },{ 512, 8, 1 } },                //TileX
            { { 128, 32, 1 },{ 128, 32, 1 },{ 128, 32, 1 },{ 128, 32, 1 },{ 128, 32, 1 } },        //TileY
            { { 256, 256, 1 },{ 512, 128, 1 },{ 512, 128, 1 },{ 1024, 64, 1 },{ 1024, 64, 1 } },   //TileYs
            { { 64, 64, 1 },{ 128, 32, 1 },{ 128, 32,1 },{ 256, 16, 1 },{ 256, 16, 1 } }           //TileYf
        };

        const uint32_t RT3DTileSize[TEST_TILE_MAX][TEST_BPP_MAX][3] = {
            { { 64, 1, 1 },{ 64, 1, 1 },{ 64, 1, 1 },{ 64, 1, 1 },{ 64, 1, 1 } },                 //Linear - no Tile Size, but min PitchAlign = 64 (cacheLine size)
            { { 512, 8, 1 },{ 512, 8, 1 },{ 512, 8, 1 },{ 512, 8, 1 },{ 512, 8, 1 } },            //TileX
            { { 128, 32, 1 },{ 128, 32, 1 },{ 128, 32, 1 },{ 128, 32, 1 },{ 128, 32, 1 } },       //TileY
            { { 64, 32, 32 },{ 64, 32, 32 },{ 128, 32, 16 },{ 256, 16, 16 },{ 512, 16, 16 } },    //TileYs
            { { 16, 16, 16 },{ 16, 16, 16 },{ 32, 16, 8 },{ 64, 8, 8 },{ 64, 8, 8 } }             //TileYf
        };
        uint32_t TileDim[3] = { RT2DTileSize[Tiling][Bpp][0], RT2DTileSize[Tiling][Bpp][1], RT2DTileSize[Tiling][Bpp][2] };
        if (ResType == TEST_RESOURCE_3D)
        {
            TileDim[0] = RT3DTileSize[Tiling][Bpp][0];
            TileDim[1] = RT3DTileSize[Tiling][Bpp][1];
            TileDim[2] = RT3DTileSize[Tiling][Bpp][2];
        }
        TileDimX = TileDim[0];
        TileDimY = TileDim[1];
        TileDimZ = TileDim[2];

        uint32_t ExpectedCCSBpp = 1;              //1 byte per pixel (contains 4 2b-CCS,
                                               //Each 2b covers 2CLs = 128B RT)
        /***
        2b-CCS  per 2CLs of RT (2:1 compression ie 2CL->1CL)
        1B-CCS covers 4x2 RT CLs (as square as possible in px)

        CL Byte size 16 x 4
        8CLs could cover {(16x32), (128x4), (64x8), (32x16)} Byte-blocks, of which last one is most-square hence should be used
        ie RT coverage by 1B CCS is 32x16, taking RT-bpp into account:

        bpp         RT-coverage in pixel by 1B CCS
        32         32/4  x 16 = 8x16
        64         32/8  x 16 = 4x16
        128        32/16 x 16 = 2x16

        Finally CCS needs to be Tile-aligned (TileY)
        ***/
        const uint32_t RTWidthDivisor[2][TEST_BPP_MAX] = { { 1, 1, 16, 8, 4 },{ 1, 1, 8, 4, 2 } }; //Divisor for TileX, TileY
        const uint32_t RTHeightDivisor[2] = { 8, 16 };
        uint32_t Idx = (Tiling == TEST_TILEX) ? 0 : 1;

        WidthDivisor = RTWidthDivisor[Idx][Bpp];
        HeightDivisor = RTHeightDivisor[Idx];
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if Mip tail start Lod matches the expected value. Fails test if value
    /// doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourceMipTailStartLod(GMM_RESOURCE_INFO *ResourceInfo, uint32_t ExpectedValue)
    {
        if (Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetMipTailStartLodSurfaceState());
        }
    }

    CTestGen9Resource();
    ~CTestGen9Resource();
};

