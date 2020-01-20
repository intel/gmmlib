/*==============================================================================
Copyright(c) 2016 Intel Corporation

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

#include "GmmGen11ResourceULT.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Resource fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
//  It also calls SetupTestCase from CommonULT to initialize global context and others.
///
/// @see    CTestGen9Resource::SetUpTestCase()
///
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen11Resource::SetUpTestCase()
{
    printf("%s\n", __FUNCTION__);
    GfxPlatform.eProductFamily    = IGFX_LAKEFIELD;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN11_CORE;

    CommonULT::SetUpTestCase();
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
///
/// @see    CTestGen10Resource::TearDownTestCase()
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen11Resource::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

/// @brief ULT for 2D TileYs Resource
TEST_F(CTestGen11Resource, TestPlanar2D_NV12_MediaCompressed)
{
    /* Test planar surface with hybrid UV planes where UV plane is half the size
    of Y and U/V data is packed together */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // [UV-Packing]
    // YccsYccsYccs
    // YccsYccsYccs
    // UVccsUVccsUVccs
    // UVccsUVccsUVccs

    const uint32_t TileSize[2] = {128, 32}; // TileY

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_2D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Gpu.MMC               = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.BaseWidth64                 = 0x2048; //8264
    gmmParams.BaseHeight                  = 0x274;  // 628
    gmmParams.Depth                       = 0x1;
    SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(TEST_TILEY));
    gmmParams.Format = GMM_FORMAT_NV12;

    GMM_RESOURCE_INFO *ResourceInfo;
    //__debugbreak();
    ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

    uint32_t Pitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[0]);

    uint32_t Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]) +
                   GMM_ULT_ALIGN(gmmParams.BaseHeight / 2, TileSize[1]);
    uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4));

    VerifyResourcePitch<true>(ResourceInfo, Pitch);
    VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[0]);

    VerifyResourceSize<true>(ResourceInfo, Size);
    VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
    VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
    VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

    // Y plane should be at 0,0
    EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
    EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

    // U/V plane should be at end of Y plane
    EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
    EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
    EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));

    EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
    EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));

    // Y Plane CCS should be at the end of NV12 surface.
    EXPECT_EQ(Size, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS));
    EXPECT_EQ(Size, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_Y_CCS));

    // UV Plane CCS offset
    uint32_t YCcsSize = GMM_ULT_ALIGN(((GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]) * Pitch) / 1024), PAGE_SIZE);
    EXPECT_EQ(Size + YCcsSize, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_UV_CCS));

    uint32_t UVCcsSize        = GMM_ULT_ALIGN((Height - GFX_ALIGN(gmmParams.BaseHeight, TileSize[1])) * Pitch / 1024, PAGE_SIZE);
    uint32_t MediaStateOffset = Size + YCcsSize + UVCcsSize;
    EXPECT_EQ(MediaStateOffset, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_COMP_STATE));

    pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
}

TEST_F(CTestGen11Resource, TestPlanar2D_NV12_MediaCompressedArray)
{
    /* Test planar surface with hybrid UV planes where UV plane is half the size
    of Y and U/V data is packed together */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // [UV-Packing]
    // YccsYccsYccs
    // YccsYccsYccs
    // UVccsUVccsUVccs
    // UVccsUVccsUVccs
    // ...

    const uint32_t TileSize[2] = {128, 32}; // TileY

    GMM_RESCREATE_PARAMS gmmParams        = {};
    gmmParams.Type                        = RESOURCE_2D;
    gmmParams.NoGfxMemory                 = 1;
    gmmParams.Flags.Gpu.MMC               = 1;
    gmmParams.Flags.Gpu.Texture           = 1;
    gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
    gmmParams.Flags.Info.TiledY           = 1;
    gmmParams.BaseWidth64                 = 0x2048;
    gmmParams.BaseHeight                  = 0x274;
    gmmParams.Depth                       = 0x1;
    gmmParams.ArraySize                   = 20;
    SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(TEST_TILEY));
    gmmParams.Format = GMM_FORMAT_NV12;

    GMM_RESOURCE_INFO *ResourceInfo;
    //__debugbreak();
    ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

    uint32_t Pitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[0]);

    uint32_t Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]) +
                   GMM_ULT_ALIGN(gmmParams.BaseHeight / 2, TileSize[1]);
    uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4)) * gmmParams.ArraySize;

    VerifyResourcePitch<true>(ResourceInfo, Pitch);
    VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[0]);

    VerifyResourceSize<true>(ResourceInfo, Size);
    VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
    VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
    VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

    // Y plane should be at 0,0
    EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
    EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

    // U/V plane should be at end of Y plane
    EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
    EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
    EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));

    EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
    EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));

    // Y Plane CCS should be at the end of NV12 surface.
    EXPECT_EQ(Size, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS));
    EXPECT_EQ(Size, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_Y_CCS));

    // UV Plane CCS offset
    uint32_t YCcsSize = GMM_ULT_ALIGN(((GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[1]) * Pitch) / 1024), PAGE_SIZE);
    EXPECT_EQ(Size + YCcsSize, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_UV_CCS));

    uint32_t UVCcsSize        = GMM_ULT_ALIGN((Height - GFX_ALIGN(gmmParams.BaseHeight, TileSize[1])) * Pitch / 1024, PAGE_SIZE);
    uint32_t MediaStateOffset = Size + YCcsSize + UVCcsSize;
    EXPECT_EQ(MediaStateOffset, ResourceInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_COMP_STATE));

    uint32_t AuxSize = YCcsSize + UVCcsSize + PAGE_SIZE;
    EXPECT_EQ(AuxSize * gmmParams.ArraySize, ResourceInfo->GetSizeAuxSurface(GMM_AUX_CCS));

    for(int i = 0; i < gmmParams.ArraySize; i++)
    {
        EXPECT_EQ(Size + AuxSize * i, ResourceInfo->GetPlanarAuxOffset(i, GMM_AUX_Y_CCS));
        EXPECT_EQ(Size + (AuxSize * i) + YCcsSize, ResourceInfo->GetPlanarAuxOffset(i, GMM_AUX_UV_CCS));
        EXPECT_EQ(Size + (AuxSize * i) + (YCcsSize + UVCcsSize), ResourceInfo->GetPlanarAuxOffset(i, GMM_AUX_COMP_STATE));
    }

    pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
}

/// @brief ULT for 2D TileYf Resource
TEST_F(CTestGen11Resource, Test2DTileYfResource)
{
    printf("%s\n", __FUNCTION__);
}

/// @brief ULT for Plannar 2D Resource - RGBP
TEST_F(CTestGen11Resource, TestPlanar2D_RGBP)
{
    /* Test planar surfaces where all planes are full-sized */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // UUUUUUUU
    // UUUUUUUU
    // UUUUUUUU
    // UUUUUUUU
    // VVVVVVVV
    // VVVVVVVV
    // VVVVVVVV
    // VVVVVVVV
    const TEST_TILE_TYPE TileTypes[]       = {TEST_LINEAR, TEST_TILEX, TEST_TILEY};
    const uint32_t          PlaneRowAlignment = 16;

    const uint32_t TileSize[3][2] = {{1, 1},     //Linear
                                  {512, 8},   // TileX
                                  {128, 32}}; // TileY
    for(uint32_t TileIndex = 0; TileIndex < sizeof(TileTypes) / sizeof(TileTypes[0]); TileIndex++)
    {
        TEST_TILE_TYPE Tile = TileTypes[TileIndex];

        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.BaseWidth64          = 0x101;
        gmmParams.BaseHeight           = 0x101;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));
        gmmParams.Format = GMM_FORMAT_RGBP;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t Pitch, Height;
        if(Tile != TEST_LINEAR)
        {
            Pitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[TileIndex][0]);
            //Since Tile alignment factor is greater than GMM_IMCx_PLANE_ROW_ALIGNMENT=16
            Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            Height = GMM_ULT_ALIGN(Height, TileSize[TileIndex][1]) * 3 /*Y, U, V*/;
        }
        else
        {
            Pitch  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, GMM_BYTES(64));
            Height = gmmParams.BaseHeight * 3 /*Y, U, V*/;
        }

        uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4));

        VerifyResourcePitch<true>(ResourceInfo, Pitch);
        if(Tile != TEST_LINEAR)
        {
            VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[TileIndex][0]);
        }
        VerifyResourceSize<true>(ResourceInfo, Size);
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

        // Y plane should be at 0,0
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
        EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

        // U plane should be at end of Y plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
        EXPECT_EQ(Height / 3, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));

        // V plane should be at end of U plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));
        EXPECT_EQ(2 * (Height / 3), ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Plannar 2D Resource - RGBP
TEST_F(CTestGen11Resource, TestPlanar2DCustom_RGBP)
{
    /* Test planar surfaces where all planes are full-sized */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // UUUUUUUU
    // UUUUUUUU
    // UUUUUUUU
    // UUUUUUUU
    // VVVVVVVV
    // VVVVVVVV
    // VVVVVVVV
    // VVVVVVVV
    const TEST_TILE_TYPE TileTypes[]          = {TEST_LINEAR, TEST_TILEX, TEST_TILEY};
    const uint32_t          PlaneRowAlignment = 16;

    const uint32_t TileSize[3][2] = {{1, 1},     //Linear
                                  {512, 8},   // TileX
                                  {128, 32}}; // TileY
    for(uint32_t TileIndex = 0; TileIndex < sizeof(TileTypes) / sizeof(TileTypes[0]); TileIndex++)
    {
        TEST_TILE_TYPE Tile = TileTypes[TileIndex];

        GMM_RESCREATE_CUSTOM_PARAMS gmmParams = {};
        gmmParams.Type                        = RESOURCE_2D;
        gmmParams.Flags.Gpu.Texture           = 1;
        gmmParams.BaseWidth64                 = 0x101;
        gmmParams.BaseHeight                  = GMM_ULT_ALIGN(0x101, PlaneRowAlignment);
        SetTileFlag_Custom(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));
        gmmParams.Format = GMM_FORMAT_RGBP;

        uint32_t Pitch, Height;

        Pitch  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, GMM_BYTES(64));
        Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment /* min16 rows*/) * 3 /*Y, U, V*/;

        uint32_t Size                        = Pitch * Height;
        gmmParams.Pitch                      = Pitch;
        gmmParams.Size                       = Size;
        gmmParams.PlaneOffset.X[GMM_PLANE_Y] = 0;
        gmmParams.PlaneOffset.Y[GMM_PLANE_Y] = 0;
        gmmParams.PlaneOffset.X[GMM_PLANE_U] = 0;
        gmmParams.PlaneOffset.Y[GMM_PLANE_U] = Height / 3;
        gmmParams.PlaneOffset.X[GMM_PLANE_V] = 0;
        gmmParams.PlaneOffset.Y[GMM_PLANE_V] = 2 * (Height / 3);
        gmmParams.NoOfPlanes                 = 3;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateCustomResInfoObject(&gmmParams);

        VerifyResourcePitch<true>(ResourceInfo, Pitch);
        if(Tile != TEST_LINEAR)
        {
            VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[TileIndex][0]);
        }
        VerifyResourceSize<true>(ResourceInfo, Size);
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

        // Y plane should be at 0,0
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
        EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

        // U plane should be at end of Y plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
        EXPECT_EQ(Height / 3, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));

        // V plane should be at end of U plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));
        EXPECT_EQ(2 * (Height / 3), ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Plannar 2D Resource - MFX_JPEG_YUV422V , IMC1, IMC3
TEST_F(CTestGen11Resource, TestPlanar2D_MFX_JPEG_YUV422V)
{
    /* Test planar surfaces where both U and V are half the size of Y */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // UUUUUUUU
    // UUUUUUUU
    // VVVVVVVV
    // VVVVVVVV
    const TEST_TILE_TYPE TileTypes[]       = {TEST_LINEAR, TEST_TILEX, TEST_TILEY};
    const uint32_t          PlaneRowAlignment = 16;
    const uint32_t          TileSize[3][2]    = {{1, 1}, //Linear
                                  {512, 8},           // TileX
                                  {128, 32}};         // TileY
    for(uint32_t TileIndex = 0; TileIndex < sizeof(TileTypes) / sizeof(TileTypes[0]); TileIndex++)
    {
        TEST_TILE_TYPE Tile = TileTypes[TileIndex];

        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.BaseWidth64          = 0x101;
        gmmParams.BaseHeight           = 0x101;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));
        gmmParams.Format = GMM_FORMAT_MFX_JPEG_YUV422V;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t Pitch, Height;
        uint32_t YHeight, VHeight;
        if(Tile != TEST_LINEAR)
        {
            Pitch   = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[TileIndex][0]);
            YHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            YHeight = GMM_ULT_ALIGN(YHeight, TileSize[TileIndex][1]);

            VHeight = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseHeight, 2) / 2, PlaneRowAlignment);
            VHeight = GMM_ULT_ALIGN(VHeight, TileSize[TileIndex][1]);
        }
        else
        {
            Pitch   = GMM_ULT_ALIGN(gmmParams.BaseWidth64, GMM_BYTES(64));
            YHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            VHeight = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseHeight, 2) / 2, PlaneRowAlignment);
        }
        Height = YHeight + 2 * VHeight;

        uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4));

        VerifyResourcePitch<true>(ResourceInfo, Pitch);
        if(Tile != TEST_LINEAR)
        {
            VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[TileIndex][0]);
        }
        VerifyResourceSize<true>(ResourceInfo, Size);
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

        // Y plane should be at 0,0
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
        EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

        // U plane should be at end of Y plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
        EXPECT_EQ(YHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));

        // V plane should be at end of U plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));
        EXPECT_EQ(YHeight + VHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Plannar 2D Resource - MFX_JPEG_YUV411R
TEST_F(CTestGen11Resource, TestPlanar2D_MFX_JPEG_YUV411R)
{
    /* Test planar surfaces where both U and V are quarter the size of Y */
    //YYYYYYYY
    //YYYYYYYY
    //YYYYYYYY
    //YYYYYYYY
    //UUUUUUUU
    //VVVVVVVV
    const TEST_TILE_TYPE TileTypes[]       = {TEST_LINEAR, TEST_TILEX, TEST_TILEY};
    const uint32_t          PlaneRowAlignment = 16;
    const uint32_t          TileSize[3][2]    = {{1, 1}, //Linear
                                  {512, 8},           // TileX
                                  {128, 32}};         // TileY
    for(uint32_t TileIndex = 0; TileIndex < sizeof(TileTypes) / sizeof(TileTypes[0]); TileIndex++)
    {
        TEST_TILE_TYPE Tile = TileTypes[TileIndex];

        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.BaseWidth64          = 0x101;
        gmmParams.BaseHeight           = 0x101;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));

        gmmParams.Format = GMM_FORMAT_MFX_JPEG_YUV411R_TYPE;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t Pitch, Height;
        uint32_t YHeight, VHeight;
        if(Tile != TEST_LINEAR)
        {
            Pitch   = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[TileIndex][0]);
            YHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            YHeight = GMM_ULT_ALIGN(YHeight, TileSize[TileIndex][1]);

            VHeight = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseHeight, 4) / 4, PlaneRowAlignment);
            VHeight = GMM_ULT_ALIGN(VHeight, TileSize[TileIndex][1]);
        }
        else
        {
            Pitch   = GMM_ULT_ALIGN(gmmParams.BaseWidth64, GMM_BYTES(64));
            YHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            VHeight = GMM_ULT_ALIGN(GMM_ULT_ALIGN(gmmParams.BaseHeight, 4) / 4, PlaneRowAlignment);
        }

        Height     = YHeight + 2 * VHeight;
        uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4));

        VerifyResourcePitch<true>(ResourceInfo, Pitch);

        if(Tile != TEST_LINEAR)
        {
            VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[TileIndex][0]);
        }
        VerifyResourceSize<true>(ResourceInfo, Size);
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

        // Y plane should be at 0,0
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
        EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

        // U plane should be at end of Y plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
        EXPECT_EQ(YHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));

        // V plane should be at end of U plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));
        EXPECT_EQ(YHeight + VHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Plannar 2D Resource - NV12
TEST_F(CTestGen11Resource, TestPlanar2D_NV12)
{
    /* Test planar surface with hybrid UV planes where UV plane is half the size
    of Y and U/V data is packed together */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // [UV-Packing]
    const TEST_TILE_TYPE TileTypes[] = {TEST_LINEAR, TEST_TILEX, TEST_TILEY};

    const uint32_t TileSize[3][2] = {{1, 1},     //Linear
                                  {512, 8},   // TileX
                                  {128, 32}}; // TileY
    for(uint32_t TileIndex = 0; TileIndex < sizeof(TileTypes) / sizeof(TileTypes[0]); TileIndex++)
    {
        TEST_TILE_TYPE Tile = TileTypes[TileIndex];

        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.BaseWidth64          = 0x100;
        gmmParams.BaseHeight           = 0x100;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));
        gmmParams.Format = GMM_FORMAT_NV12;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t Pitch, Height;

        if(Tile != TEST_LINEAR)
        {
            Pitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[TileIndex][0]);

            Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[TileIndex][1]) +
                     GMM_ULT_ALIGN(gmmParams.BaseHeight / 2, TileSize[TileIndex][1]);
        }
        else
        {
            Pitch  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[TileIndex][0]);
            Height = GMM_ULT_ALIGN(gmmParams.BaseHeight /*Y*/ + gmmParams.BaseHeight / 2 /*UV*/, TileSize[TileIndex][1]);
        }

        uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4));

        VerifyResourcePitch<true>(ResourceInfo, Pitch);
        if(Tile != TEST_LINEAR)
        {
            VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[TileIndex][0]);
        }
        VerifyResourceSize<true>(ResourceInfo, Size);
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

        // Y plane should be at 0,0
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
        EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

        // U/V plane should be at end of Y plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));

        if(Tile != TEST_LINEAR)
        {
            EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[TileIndex][1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
            EXPECT_EQ(GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[TileIndex][1]), ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));
        }
        else
        {
            EXPECT_EQ(gmmParams.BaseHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
            EXPECT_EQ(gmmParams.BaseHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Planar 2D Resource - IMC4
TEST_F(CTestGen11Resource, TestPlanar2D_IMC4)
{
    /* Test planar surface V surface is on the right of U */
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // YYYYYYYY
    // UUUUVVVV
    // UUUUVVVV
    const TEST_TILE_TYPE TileTypes[]       = {TEST_LINEAR, TEST_TILEX, TEST_TILEY};
    const uint32_t          PlaneRowAlignment = 16;
    const uint32_t          TileSize[3][2]    = {{1, 1}, //Linear
                                  {512, 8},           // TileX
                                  {128, 32}};         // TileY
    for(uint32_t TileIndex = 0; TileIndex < sizeof(TileTypes) / sizeof(TileTypes[0]); TileIndex++)
    {
        TEST_TILE_TYPE Tile = TileTypes[TileIndex];

        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.BaseWidth64          = 0x101;
        gmmParams.BaseHeight           = 0x101;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));
        gmmParams.Format = GMM_FORMAT_IMC4;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t Pitch, Height;
        uint32_t YHeight, VHeight;
        if(Tile != TEST_LINEAR)
        {
            Pitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[TileIndex][0]);
            if(Pitch / TileSize[TileIndex][0] % 2)
            {
                Pitch += TileSize[TileIndex][0];
            }

            YHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            VHeight = YHeight / 2;

            YHeight = GMM_ULT_ALIGN(YHeight, TileSize[TileIndex][1]);
            VHeight = GMM_ULT_ALIGN(VHeight, TileSize[TileIndex][1]); // No need of PlaneRowAlignment since last plane
        }
        else
        {
            Pitch   = GMM_ULT_ALIGN(gmmParams.BaseWidth64, GMM_BYTES(64));
            YHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, PlaneRowAlignment);
            VHeight = YHeight / 2;
        }

        Height = YHeight + VHeight;

        uint32_t Size = GMM_ULT_ALIGN(Pitch * Height, GMM_KBYTE(4));

        VerifyResourcePitch<true>(ResourceInfo, Pitch);

        if(Tile != TEST_LINEAR)
        {
            VerifyResourcePitchInTiles<true>(ResourceInfo, Pitch / TileSize[TileIndex][0]);
        }
        VerifyResourceSize<true>(ResourceInfo, Size);
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Same as any other 2D surface -- tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for planar

        // Y plane should be at 0,0
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_Y));
        EXPECT_EQ(0, ResourceInfo->GetPlanarYOffset(GMM_PLANE_Y));

        // U plane should be at end of Y plane
        // V plane should be at end of U plane
        EXPECT_EQ(0, ResourceInfo->GetPlanarXOffset(GMM_PLANE_U));
        EXPECT_EQ(Pitch / 2, ResourceInfo->GetPlanarXOffset(GMM_PLANE_V));

        if(Tile != TEST_LINEAR)
        {
            EXPECT_EQ(YHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
            EXPECT_EQ(YHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));
        }
        else
        {
            EXPECT_EQ(YHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_U));
            EXPECT_EQ(YHeight, ResourceInfo->GetPlanarYOffset(GMM_PLANE_V));
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}
