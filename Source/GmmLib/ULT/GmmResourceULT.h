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

#include "GmmCommonULT.h"

//===========================================================================
// typedef:
//      TEST_RESOURCE_TYPE_ENUM
//
// Description:
//     Decribes Resource types to test
//     when editing/adding existing/new enum, make sure to update corresponding
//     string array CTestResource::TestResourceTypeStr[].
//----------------------------------------------------------------------------
typedef enum TEST_RESOURCE_TYPE_ENUM
{
    TEST_RESOURCE_1D,
    TEST_RESOURCE_2D,
    TEST_RESOURCE_3D,
    TEST_RESOURCE_CUBE,
    TEST_RESOURCE_BUFFER,
    TEST_RESOURCE_MAX
}TEST_RESOURCE_TYPE;

//===========================================================================
// typedef:
//      TEST_RESOURCE_TYPE_ENUM
//
// Description:
//     Decribes Resource Tile Types to test .
//     when editing/adding existing/new enum, make sure to update corresponding
//     string array CTestResource::TestTileTypeStr[].
//----------------------------------------------------------------------------
typedef enum TEST_TILE_TYPE_ENUM
{
    TEST_LINEAR,
    TEST_TILEX,
    TEST_TILEY,
    TEST_TILEYS,
    TEST_TILEYF,
    TEST_TILE_MAX
}TEST_TILE_TYPE;


//===========================================================================
// typedef:
//      TEST_BPP_ENUM
//
// Description:
//     Decribes BPP to test - 8, 16, 32, 64, 128 (Ignore 24/96 until need arises)
//     when editing/adding existing/new enum, make sure to update corresponding
//     string array CTestResource::TestBppStr[].
//----------------------------------------------------------------------------
typedef enum TEST_BPP_ENUM
{
    TEST_BPP_8,
    TEST_BPP_16,
    TEST_BPP_32,
    TEST_BPP_64,
    TEST_BPP_128,
    TEST_BPP_MAX
}TEST_BPP;

typedef enum TEST_MSAA_Samples
{
    MSAA_None,
    MSAA_2x,
    MSAA_4x,
    MSAA_8x,
    MSAA_16x,
    TEST_MSAA_MAX = MSAA_16x        //Should be equal to last MSAA type
} TEST_MSAA;

//===========================================================================
// GmmLib ULT macros for size alignment. Compitable with 64-bit numbers.
//---------------------------------------------------------------------------
#define GMM_ULT_ALIGN(x, a)          (((x) + ((a) - 1)) - (((x) + ((a) - 1)) & ((a) - 1))) // Alt implementation with bitwise not (~) has issue with uint32 align used with 64-bit value, since ~'ed value will remain 32-bit.
#define GMM_ULT_ALIGN_FLOOR(x, a)    ((x) - ((x) & ((a) - 1)))
#define GMM_ULT_MAX(a, b)            ((a) > (b)) ? (a) : (b)
#define GMM_ULT_MIN(a,b)             (((a) < (b)) ? (a) : (b))
#define GMM_ULT_ALIGN_NP2(x, a)      (((a) > 0) ? ((x) + (((a) - 1) - (((x) + ((a) - 1)) % (a)))) : (x))  //Next power of 2
#define GMM_ULT_MAX_MIPMAP           15
#define GMM_ULT_MAX_MIPTAIL_SLOTS    15
//////////////////////////////////////////////////////////////////////////
// typdef:
//     TEST_MIPTAIL_SLOT_OFFSET_REC
//
// Description:
//     Structure to hold the offset of the mip resources for unit level testing
//////////////////////////////////////////////////////////////////////////
typedef struct TEST_MIPTAIL_SLOT_OFFSET_REC
{
    uint32_t X;
    uint32_t Y;
    uint32_t Z;
}TEST_MIPTAIL_SLOT_OFFSET;


/////////////////////////////////////////////////////////////////////////
/// Fixture class for Resource. - This is Resource Test Case to test
/// all generic resource types, tile types, bpp and special allocations.
/// Contains Base implementation and inherits CommonULT class
/////////////////////////////////////////////////////////////////////////
class CTestResource : public CommonULT
{
protected:

    /////////////////////////////////////////////////////////////////////////////////////
    /// Returns GMM format for given test BPP input.
    ///
    /// @param[in]  Bpp: test Bpp value
    ///
    /// @return     ::GMM_RESOURCE_FORMAT
    /////////////////////////////////////////////////////////////////////////////////////
    GMM_RESOURCE_FORMAT SetResourceFormat(TEST_BPP Bpp)
    {
        switch (Bpp)
        {
            case TEST_BPP_8:    return GMM_FORMAT_GENERIC_8BIT;
            case TEST_BPP_16:   return GMM_FORMAT_GENERIC_16BIT;
            case TEST_BPP_32:   return GMM_FORMAT_GENERIC_32BIT;
            case TEST_BPP_64:   return GMM_FORMAT_GENERIC_64BIT;
            case TEST_BPP_128:  return GMM_FORMAT_GENERIC_128BIT;
            default: break;
        }

        return GMM_FORMAT_INVALID;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Returns Bpp (bytes per pixel) for a given bpp (bits per pixel) enum.
    ///
    /// @param[in]  bpp: test bpp value
    ///
    /// @return     Bytes per pixel
    /////////////////////////////////////////////////////////////////////////////////////
    uint32_t GetBppValue(TEST_BPP bpp)
    {
        uint32_t Bpp = 0;
        switch (bpp)
        {
            case TEST_BPP_8:    Bpp = 8;    break;
            case TEST_BPP_16:   Bpp = 16;   break;
            case TEST_BPP_32:   Bpp = 32;   break;
            case TEST_BPP_64:   Bpp = 64;   break;
            case TEST_BPP_128:  Bpp = 128;  break;
            default: break;
        }

        return Bpp >> 3;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Set the tile flag in Gmm ResCreate Params
    ///
    /// @param[in]  Parms: Gmm Rescreate params
    /// @param[in]  Tile: Tile Type
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void SetTileFlag(GMM_RESCREATE_PARAMS &Params, TEST_TILE_TYPE Tile)
    {
        switch (Tile)
        {
            case TEST_LINEAR:
                Params.Flags.Info.Linear = 1;
                break;
            case TEST_TILEX:
                Params.Flags.Info.TiledX = 1;
                break;
            case TEST_TILEY:
                Params.Flags.Info.TiledY = 1;
                break;
            case TEST_TILEYF:
                Params.Flags.Info.TiledY  = 1;
                Params.Flags.Info.TiledYf = 1;
                break;
            case TEST_TILEYS:
                Params.Flags.Info.TiledY  = 1;
                Params.Flags.Info.TiledYs = 1;
                break;
            default: break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Set the tile flag in Gmm Custom ResCreate Params
    ///
    /// @param[in]  Parms: Gmm Rescreate params
    /// @param[in]  Tile: Tile Type
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void SetTileFlag_Custom(GMM_RESCREATE_CUSTOM_PARAMS& Params, TEST_TILE_TYPE Tile)
    {
        switch (Tile)
        {
        case TEST_LINEAR:
            Params.Flags.Info.Linear = 1;
            break;
        case TEST_TILEX:
            Params.Flags.Info.TiledX = 1;
            break;
        case TEST_TILEY:
            Params.Flags.Info.TiledY = 1;
            break;
        case TEST_TILEYF:
            Params.Flags.Info.TiledY = 1;
            Params.Flags.Info.TiledYf = 1;
            break;
        case TEST_TILEYS:
            Params.Flags.Info.TiledY = 1;
            Params.Flags.Info.TiledYs = 1;
            break;
        default: break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Sets Resource Type in GmmParams
    ///
    /// @param[in]  Params: GmmParams
    /// @param[in]  ResType: Resource type
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void SetResType(GMM_RESCREATE_PARAMS& Params, TEST_RESOURCE_TYPE ResType)
    {
        Params.Type = static_cast<GMM_RESOURCE_TYPE>((ResType == TEST_RESOURCE_BUFFER) ? ResType + 2 : ResType + 1);
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Sets RenderTarget or Depth Gpu Flags in GmmParams
    ///
    /// @param[in]  Params: GmmParams
    /// @param[in]  IsRT:  true for RT, false for Depth
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void SetResGpuFlags(GMM_RESCREATE_PARAMS& Params, bool IsRT)
    {
        if (IsRT)
        {
            Params.Flags.Gpu.Depth = 0;
            Params.Flags.Gpu.RenderTarget = 1;
        }
        else
        {
            Params.Flags.Gpu.Depth = 1;
            Params.Flags.Gpu.RenderTarget = 0;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Sets ArraySize in GmmParams
    ///
    /// @param[in]  Params: GmmParams
    /// @param[in]  Size: Array Size
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void SetResArraySize(GMM_RESCREATE_PARAMS& Params, int Size)
    {
        Params.ArraySize = Size;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Get the Tile dimension and H/V Align for MSS and MCS surfaces
    ///
    /// @param[in]  Bpp: bits per pixel
    /// @param[in]  isRT: Request is for RT or Depth MSS
    /// @param[in]  Tiling: Tile Type
    /// @param[in]  MSAA: Num of Samples
    /// @param[out] HAlign: H Align for MSS
    /// @param[out] VAlign: V Align for MSS
    /// @param[out] TileDimX: Tile Width for given Tile, Resource, bpp
    /// @param[out] TileDimY: Tile Height for given Tile, Resource, bpp
    /// @param[out] MCSHAlign: H Align for MCS
    /// @param[out] MCSVAlign: V Align for MCS
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void GetAlignmentAndTileDimensionsForMSAA(TEST_BPP Bpp, bool isRT, TEST_TILE_TYPE Tiling, TEST_MSAA MSAA,
        uint32_t& TileDimX, uint32_t& TileDimY, uint32_t& HAlign, uint32_t& VAlign,
        uint32_t& ExpectedMCSBpp, uint32_t &MCSHAlign, uint32_t &MCSVAlign)
    {
        const uint32_t MSSTileSize[TEST_TILE_MAX][TEST_BPP_MAX][2] = {
            { { 64, 1 },{ 64, 1 },{ 64, 1 },{ 64, 1 },{ 64, 1 } },             //Linear - no Tile Size, but min PitchAlign = 64
            { { 512, 8 },{ 512, 8 },{ 512, 8 },{ 512, 8 },{ 512, 8 } },        //TileX
            { { 128, 32 },{ 128, 32 },{ 128, 32 },{ 128, 32 },{ 128, 32 } },   //TileY
            { { 256, 256 },{ 512, 128 },{ 512, 128 },{ 1024, 64 },{ 1024, 64 } },   //TileYs
            { { 64, 64 },{ 128, 32 },{ 128, 32 },{ 256, 16 },{ 256, 16 } }     //TileYf
        };
        uint32_t WMul, HMul;

        HAlign = 16;                              // RT H/VAlign
        VAlign = 4;

        if (!isRT)
        {
            HAlign = (Bpp == TEST_BPP_16) ? 8 : 4;     //Depth 16bit = 8x4, ow 4x4
            VAlign = 4;
            MCSHAlign = 4;                            //MCS uses base H/VAlign for 8bpp
        }

        uint32_t Tile[2] = { MSSTileSize[Tiling][Bpp][0], MSSTileSize[Tiling][Bpp][1] };
        if (Tiling == TEST_TILEYS || Tiling == TEST_TILEYF)
        {
            GetInterleaveMSSPattern(MSAA, WMul, HMul);

            //Std Tiling interleaves MSAA into 1x, decreasing std Tile size for MSAA'd sample
            //Std Tiling types should have std size alignment always
            Tile[0] = HAlign = (isRT) ? (MSSTileSize[Tiling][Bpp][0] / WMul) : MSSTileSize[Tiling][Bpp][0];
            Tile[1] = VAlign = (isRT) ? (MSSTileSize[Tiling][Bpp][1] / HMul) : MSSTileSize[Tiling][Bpp][1];

            HAlign /= pow(2.0, Bpp);      //Unit alignment in pixels
        }
        TileDimX = Tile[0];
        TileDimY = Tile[1];

        ExpectedMCSBpp = (MSAA == MSAA_2x || MSAA == MSAA_4x) ? 1 :
            (MSAA == MSAA_8x) ? 4 : 8;

        uint32_t ExpectedMcsBppIdx = log2(ExpectedMCSBpp);

        MCSHAlign = isRT ? HAlign : MCSHAlign;              //MCS uses base H/V ALign for 8bpp
        MCSVAlign = VAlign;

        MCSHAlign = (isRT && (Tiling == TEST_TILEYS || Tiling == TEST_TILEYF)) ?
                             MSSTileSize[Tiling][ExpectedMcsBppIdx][0] / (WMul * ExpectedMCSBpp) :         //Std Tile dim in pixels
                            (Tiling == TEST_TILEYS || Tiling == TEST_TILEYF) ?
                             MSSTileSize[Tiling][ExpectedMcsBppIdx][0] / ExpectedMCSBpp : MCSHAlign;       //For legacy tile, MCS alignment is base (RT or Depth) alignment
        MCSVAlign = (isRT && (Tiling == TEST_TILEYS || Tiling == TEST_TILEYF)) ? MSSTileSize[Tiling][ExpectedMcsBppIdx][1] / HMul :
                            (Tiling == TEST_TILEYS || Tiling == TEST_TILEYF) ? MSSTileSize[Tiling][ExpectedMcsBppIdx][1] : MCSVAlign;
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Get the interleave pattern for given Num Samples
    ///
    /// @param[in]  MSAA: Num of Samples
    /// @param[out] WidthMultiplier: Number of samples arranged side-by-side
    /// @param[out] HeightMultiplier: Number of samples arranged top-bottom
    ///
    /////////////////////////////////////////////////////////////////////////////////////
    void GetInterleaveMSSPattern(TEST_MSAA MSAA, uint32_t& WidthMultiplier, uint32_t& HeightMultiplier)
    {
        WidthMultiplier = 1; HeightMultiplier = 1;

        switch (MSAA)
        {
        case MSAA_2x: WidthMultiplier = 2; break;
        case MSAA_4x: WidthMultiplier = 2; HeightMultiplier = 2; break;
        case MSAA_8x: WidthMultiplier = 4; HeightMultiplier = 2; break;
        case MSAA_16x:WidthMultiplier = 4; HeightMultiplier = 4; break;
        default: break;
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if HAlign matches the expected value.  Fails test if value doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourceHAlign(GMM_RESOURCE_INFO *ResourceInfo, uint32_t ExpectedValue)
    {
        if(Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetHAlign());
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if HAlign matches the expected value.  Fails test if value doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourceVAlign(GMM_RESOURCE_INFO *ResourceInfo, uint32_t ExpectedValue)
    {
        if(Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetVAlign());
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if Pitch (in bytes) matches the expected value.  Fails test if value
    /// doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourcePitch(GMM_RESOURCE_INFO *ResourceInfo, uint32_t ExpectedValue)
    {
        if(Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetRenderPitch());
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if Pitch in tiles matches the expected value.  Fails test if value
    /// doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourcePitchInTiles(GMM_RESOURCE_INFO *ResourceInfo, uint32_t ExpectedValue)
    {
        if(Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetRenderPitchTiles());
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if Size matches the expected value.  Fails test if value doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourceSize(GMM_RESOURCE_INFO *ResourceInfo, uint64_t ExpectedValue)
    {
        if(Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetSizeMainSurface());
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    /// Verifies if QPitch matches the expected value.  Fails test if value doesn't match
    ///
    /// @param[in]  ResourceInfo: ResourceInfo returned by GmmLib
    /// @param[in]  ExpectedValue: expected value to check against
    /////////////////////////////////////////////////////////////////////////////////////
    template <bool Verify>
    void VerifyResourceQPitch(GMM_RESOURCE_INFO *ResourceInfo, uint64_t ExpectedValue)
    {
        if(Verify)
        {
            EXPECT_EQ(ExpectedValue, ResourceInfo->GetQPitch());
        }
    }

public:
    CTestResource();
    ~CTestResource();

    static void SetUpTestCase();
    static void TearDownTestCase();

};


/////////////////////////////////////////////////////////////////////////
/// Fixture class for Resources targeted for CpuBlt. This is CpuBlt resource
/// test case. Inherits CTestResource class.
/// @see      CTestResource class
/////////////////////////////////////////////////////////////////////////
class CTestCpuBltResource : public CTestResource
{
public:
    CTestCpuBltResource();
    ~CTestCpuBltResource();

    static void SetUpTestCase();
    static void TearDownTestCase();

};

/////////////////////////////////////////////////////////////////////////
/// Helper function - builds list of input tuples
///
/// @param[in/out]  List: vector of tuple<int,int,int,bool,int,int>
/// @param[in]  maxTestDimension: Number of elements in TestDimensions[]
/// @param[in]  TestArray: Number fo elements in TestArraySize
///
/// @return   Number of tuples in the list
/// @see      GmmGen9ResourceULT.cpp
/////////////////////////////////////////////////////////////////////////
int BuildInputIterator(std::vector<std::tuple<int, int, int, bool, int, int>> &List, int maxTestDimension, int TestArray);
