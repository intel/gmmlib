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

#include "GmmGen9ResourceULT.h"

using namespace std;


/////////////////////////////////////////////////////////////////////////////////////
/// CTestGen9Resource Constructor
///
/////////////////////////////////////////////////////////////////////////////////////
CTestGen9Resource::CTestGen9Resource()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// CTestGen9Resource Destructor
///
/////////////////////////////////////////////////////////////////////////////////////
CTestGen9Resource::~CTestGen9Resource()
{
}

/////////////////////////////////////////////////////////////////////////////////////
/// Sets up common environment for Resource fixture tests. this is called once per
/// test case before executing all tests under resource fixture test case.
///  It also calls SetupTestCase from CommonULT to initialize global context and others.
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen9Resource::SetUpTestCase()
{
    GfxPlatform.eProductFamily    = IGFX_SKYLAKE;
    GfxPlatform.eRenderCoreFamily = IGFX_GEN9_CORE;

    CommonULT::SetUpTestCase();

    printf("%s\n", __FUNCTION__);
}

/////////////////////////////////////////////////////////////////////////////////////
/// cleans up once all the tests finish execution.  It also calls TearDownTestCase
/// from CommonULT to destroy global context and others.
/////////////////////////////////////////////////////////////////////////////////////
void CTestGen9Resource::TearDownTestCase()
{
    printf("%s\n", __FUNCTION__);

    CommonULT::TearDownTestCase();
}

// ********************************************************************************//

/// @brief ULT for 1D Linear Resource
TEST_F(CTestGen9Resource, Test1DLinearResource)
{
    // Horizontal pixel alignment
    const uint32_t HAlign = 64;

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.Linear    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }


    // Allocate more than 1 page
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1001;
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D Linear Resource Arrays
TEST_F(CTestGen9Resource, Test1DLinearResourceArrays)
{
    // Horizontal pixel alignment
    const uint32_t HAlign = 64;

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.Linear    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.ArraySize            = 4;

    // Allocate more than 1 page
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1001;
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes * gmmParams.ArraySize, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<true>(ResourceInfo, AlignedWidth);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D Mipped Linear Resource
TEST_F(CTestGen9Resource, Test1DLinearResourceMips)
{
    // Horizontal pixel alignment
    const uint32_t HAlign = 64;

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.Linear    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.MaxLod               = 5;

    // Allocate 256x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x100;
        gmmParams.BaseHeight  = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        for(int mip = 1; mip <= gmmParams.MaxLod; mip++)
        {
            // Since 1D doesn't have a height, mips are just based on width
            AlignedWidth += GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> mip, HAlign);
        }
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        // Mip0 should be at offset 0. X/Y/Z Offset should be 0 for linear.
        GMM_REQ_OFFSET_INFO OffsetInfo = {};
        OffsetInfo.ReqRender           = 1;
        OffsetInfo.MipLevel            = 0; //Mip 0
        ResourceInfo->GetOffset(OffsetInfo);
        EXPECT_EQ(0, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        // All mips should be right after one another linearly
        uint32_t StartOfMip = 0;
        for(int mip = 1; mip <= gmmParams.MaxLod; mip++)
        {
            OffsetInfo           = {};
            OffsetInfo.ReqRender = 1;
            OffsetInfo.MipLevel  = mip;
            ResourceInfo->GetOffset(OffsetInfo);
            StartOfMip += GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> (mip - 1), HAlign) * GetBppValue(bpp);
            EXPECT_EQ(StartOfMip, OffsetInfo.Render.Offset64);
            EXPECT_EQ(0, OffsetInfo.Render.XOffset);
            EXPECT_EQ(0, OffsetInfo.Render.YOffset);
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D TileYs Resource  -TRMODE_64KB
TEST_F(CTestGen9Resource, Test1DTileYsResource)
{
    const uint32_t TileSize[TEST_BPP_MAX] = {65536, 32768, 16384, 8192, 4096};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i]);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate more than 1 "tile", where applicable. Max width of the surface can only be
    // 16K (see RENDER_SURFACE_STATE). Depending on the bpp, 16K width may not use more than
    // 1 "tile" (only 64/128 will use "multi-tile").
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 16 * 1024; // 16K is the max width you can specify
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i]);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D TileYS Resource Arrays
TEST_F(CTestGen9Resource, Test1DTileYsResourceArrays)
{
    const uint32_t TileSize[TEST_BPP_MAX] = {65536, 32768, 16384, 8192, 4096};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.ArraySize            = 4;

    // Allocate more than 1 page
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 16 * 1024; // 16K is the max width you can specify
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i]);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes * gmmParams.ArraySize, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<true>(ResourceInfo, AlignedWidth);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D Mipped TileYS Resource
TEST_F(CTestGen9Resource, Test1DTileYsResourceMips)
{
    const uint32_t TileSize[TEST_BPP_MAX] = {65536, 32768, 16384, 8192, 4096};
    const uint32_t Mts[TEST_BPP_MAX]      = {32768, 16384, 8192, 4096, 2048};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.MaxLod               = 5;

    // Allocate all mips in 1 tile or multiple tiles, depending on the bpp
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 16 * 1024; // 16K is the max width you can specify
        gmmParams.BaseHeight  = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t MaxMip;
        uint32_t MipTailStart = gmmParams.MaxLod;
        for(MaxMip = 0; MaxMip <= gmmParams.MaxLod; MaxMip++)
        {
            if((gmmParams.BaseWidth64 >> MaxMip) <= Mts[i])
            {
                MipTailStart = MaxMip;
                break;
            }
        }

        uint32_t AlignedWidth = 0;
        for(int mip = 0; mip <= MaxMip; mip++)
        {
            // Since 1D doesn't have a height, mips are just based on width
            AlignedWidth += GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> mip, TileSize[i]);
            ;
        }

        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        // All mips should be right after one another linearly, until the miptail
        uint32_t StartOfMip = 0;
        int      mip;
        for(mip = 0; mip < MaxMip; mip++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.MipLevel            = mip;
            ResourceInfo->GetOffset(OffsetInfo);
            StartOfMip += (mip == 0 ? 0 : GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> (mip - 1), TileSize[i]) * GetBppValue(bpp));
            EXPECT_EQ(StartOfMip, OffsetInfo.Render.Offset64);
            EXPECT_EQ(0, OffsetInfo.Render.XOffset);
            EXPECT_EQ(0, OffsetInfo.Render.YOffset);
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
        }

        uint32_t MipTailOffsets[GMM_ULT_MAX_MIPMAP] = {32768, 16384, 8192, 4096, 2048, 1024, 768, 512, 448, 384, 320, 256, 192, 128, 64};
        // Check for offset inside miptails.
        EXPECT_EQ(MipTailStart, ResourceInfo->GetPackedMipTailStartLod());
        StartOfMip += GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> (mip - 1), TileSize[i]) * GetBppValue(bpp); // Start of MipTail
        for(int slot = 0; mip <= gmmParams.MaxLod; mip++, slot++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.MipLevel            = mip;
            ResourceInfo->GetOffset(OffsetInfo);
            EXPECT_EQ(StartOfMip, OffsetInfo.Render.Offset64);          // Start of Miptail
            EXPECT_EQ(MipTailOffsets[slot], OffsetInfo.Render.XOffset); // Offset within miptail
            EXPECT_EQ(0, OffsetInfo.Render.YOffset);
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D TileYs Resource  -TRMODE_4KB
TEST_F(CTestGen9Resource, Test1DTileYfResource)
{
    const uint32_t TileSize[TEST_BPP_MAX] = {4096, 2048, 1024, 512, 256};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i]);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate more than 1 "tile"
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = TileSize[i] + 1;
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i]);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D TileYF Resource Arrays
TEST_F(CTestGen9Resource, Test1DTileYfResourceArrays)
{
    const uint32_t TileSize[TEST_BPP_MAX] = {4096, 2048, 1024, 512, 256};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.ArraySize            = 4;

    // Allocate more than 1 page
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 16 * 1024; // 16K is the max width you can specify
        gmmParams.BaseHeight  = 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[i]);
        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes * gmmParams.ArraySize, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<true>(ResourceInfo, AlignedWidth);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 1D Mipped TileYF Resource
TEST_F(CTestGen9Resource, Test1DTileYfResourceMips)
{
    const uint32_t TileSize[TEST_BPP_MAX] = {4096, 2048, 1024, 512, 256};
    const uint32_t Mts[TEST_BPP_MAX]      = {2048, 1024, 512, 256, 128};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_1D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.MaxLod               = 8;

    // Allocate all mips in 1 tile or multiple tiles, depending on the bpp
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 16 * 1024; // 16K is the max width you can specify
        gmmParams.BaseHeight  = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t MaxMip;
        uint32_t MipTailStart = gmmParams.MaxLod;
        for(MaxMip = 0; MaxMip <= gmmParams.MaxLod; MaxMip++)
        {
            if((gmmParams.BaseWidth64 >> MaxMip) <= Mts[i])
            {
                MipTailStart = MaxMip;
                break;
            }
        }

        uint32_t AlignedWidth = 0;
        for(int mip = 0; mip <= MaxMip; mip++)
        {
            // Since 1D doesn't have a height, mips are just based on width
            AlignedWidth += GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> mip, TileSize[i]);
        }

        uint32_t PitchInBytes = AlignedWidth * GetBppValue(bpp);
        uint32_t AlignedSize  = GMM_ULT_ALIGN(PitchInBytes, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, TileSize[i]);
        VerifyResourceVAlign<false>(ResourceInfo, 0);       // N/A for 1D
        VerifyResourcePitch<false>(ResourceInfo, 0);        // N/A for 1D
        VerifyResourcePitchInTiles<false>(ResourceInfo, 0); // N/A for linear
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-arrayed

        // All mips should be right after one another linearly, until the miptail
        uint32_t StartOfMip = 0;
        int      mip;
        for(mip = 0; mip < MaxMip; mip++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.MipLevel            = mip;
            ResourceInfo->GetOffset(OffsetInfo);
            StartOfMip += (mip == 0 ? 0 : GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> (mip - 1), TileSize[i]) * GetBppValue(bpp));
            EXPECT_EQ(StartOfMip, OffsetInfo.Render.Offset64);
            EXPECT_EQ(0, OffsetInfo.Render.XOffset);
            EXPECT_EQ(0, OffsetInfo.Render.YOffset);
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
        }

        uint32_t MipTailOffsets[12] = {2048, 1024, 768, 512, 448, 384, 320, 256, 192, 128, 64, 0};
        // Check for offset inside miptails.
        EXPECT_EQ(MipTailStart, ResourceInfo->GetPackedMipTailStartLod());
        StartOfMip += GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> (mip - 1), TileSize[i]) * GetBppValue(bpp); // Start of MipTail
        for(int slot = 0; mip <= gmmParams.MaxLod; mip++, slot++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.MipLevel            = mip;
            ResourceInfo->GetOffset(OffsetInfo);
            EXPECT_EQ(StartOfMip, OffsetInfo.Render.Offset64);          // Start of Miptail
            EXPECT_EQ(MipTailOffsets[slot], OffsetInfo.Render.XOffset); // Offset within miptail
            EXPECT_EQ(0, OffsetInfo.Render.YOffset);
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

// ********************************************************************************//

/// @brief ULT for 2D TileYs Resource
TEST_F(CTestGen9Resource, Test2DTileYsResource)
{

    const uint32_t HAlign[TEST_BPP_MAX] = {256, 256, 128, 128, 64};
    const uint32_t VAlign[TEST_BPP_MAX] = {256, 128, 128, 64, 64};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{256, 256},
                                                {512, 128},
                                                {512, 128},
                                                {1024, 64},
                                                {1024, 64}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0]); // As wide as 1 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);       // 1 Tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64));   // 1 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);            // Not Tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
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
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64) * 2);   // 2 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(64) * 4);   // 4 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D TileYs Resource with Mips
TEST_F(CTestGen9Resource, Test2DTileYsMippedResource)
{
    const uint32_t HAlign[TEST_BPP_MAX] = {256, 256, 128, 128, 64};
    const uint32_t VAlign[TEST_BPP_MAX] = {256, 128, 128, 64, 64};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{256, 256},
                                                {512, 128},
                                                {512, 128},
                                                {1024, 64},
                                                {1024, 64}};

    const uint32_t MtsWidth[TEST_BPP_MAX]  = {128, 128, 64, 64, 32};
    const uint32_t MtsHeight[TEST_BPP_MAX] = {256, 128, 128, 64, 64};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.MaxLod               = 5;
    gmmParams.ArraySize            = 4;

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
        uint32_t Mip2Higher    = 0; // Sum of aligned heights of Mip2 and above
        uint32_t MipTailHeight = 0;
        // Haligned Mip Widths
        uint32_t Mip0Width = 0;
        uint32_t Mip1Width = 0;
        uint32_t Mip2Width = 0;

        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x120;
        gmmParams.BaseHeight  = 0x120;

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
        Mip0Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign[i]);
        Mip1Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> 1, HAlign[i]);
        Mip2Width    = GMM_ULT_ALIGN(gmmParams.BaseWidth64 >> 2, HAlign[i]);
        AlignedWidth = GMM_ULT_MAX(Mip0Width, Mip1Width + Mip2Width);

        Mip0Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign[i]);
        if(MipTailStartLod == 2)
        {
            EXPECT_EQ(2, ResourceInfo->GetPackedMipTailStartLod());
            // Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
            Mip1Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 1, VAlign[i]);
            Mip2Height = Mip2Higher = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 2, VAlign[i]);
        }
        else if(MipTailStartLod == 3)
        {
            EXPECT_EQ(3, ResourceInfo->GetPackedMipTailStartLod());
            // Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
            Mip1Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 1, VAlign[i]);
            Mip2Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 2, VAlign[i]);
            // Miptail started lod
            MipTailHeight = VAlign[i];
            Mip2Higher    = Mip2Height + MipTailHeight;
        }
        else if(MipTailStartLod == 4)
        {
            EXPECT_EQ(4, ResourceInfo->GetPackedMipTailStartLod());
            // Block height...Mip0Height + Max(Mip1Height, Sum of Mip2Height..MipnHeight)
            Mip1Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 1, VAlign[i]);
            Mip2Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 2, VAlign[i]);
            Mip3Height = GMM_ULT_ALIGN(gmmParams.BaseHeight >> 3, VAlign[i]);
            // Miptail started lod
            MipTailHeight = VAlign[i];
            Mip2Higher    = Mip2Height + Mip3Height + MipTailHeight;
        }

        uint32_t MaxHeight = GMM_ULT_MAX(Mip1Height, Mip2Higher);
        AlignedHeight      = Mip0Height + MaxHeight;
        AlignedHeight      = GMM_ULT_ALIGN(AlignedHeight, VAlign[i]);

        ExpectedPitch = AlignedWidth * GetBppValue(bpp);
        ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, GMM_BYTES(32));
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);

        VerifyResourcePitchInTiles<true>(ResourceInfo, static_cast<uint32_t>(ExpectedPitch / TileSize[i][0]));
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * AlignedHeight * gmmParams.ArraySize, PAGE_SIZE));
        VerifyResourceQPitch<false>(ResourceInfo, AlignedHeight);

        // Mip 0 offsets, offset is 0,0
        GMM_REQ_OFFSET_INFO ReqInfo = {0};
        ReqInfo.MipLevel            = 0;
        ReqInfo.ReqRender           = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip0Size = ExpectedPitch * Mip0Height;
        EXPECT_EQ(0, ReqInfo.Render.Offset64);
        EXPECT_EQ(0, ReqInfo.Render.XOffset);
        EXPECT_EQ(0, ReqInfo.Render.YOffset);

        // Mip 1 offsets
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 1;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip1Offset = Mip0Size;
        EXPECT_EQ(Mip1Offset, ReqInfo.Render.Offset64);
        EXPECT_EQ(0, ReqInfo.Render.XOffset);
        EXPECT_EQ(0, ReqInfo.Render.YOffset);

        // Mip 2 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 2;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip2Offset              = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
        uint32_t Mip2X                   = GFX_ALIGN_FLOOR(uint32_t(Mip2Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip2Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip2Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip2RenderAlignedOffset = Mip2Y * ExpectedPitch + (Mip2X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip2RenderAlignedOffset, ReqInfo.Render.Offset64);
        switch(bpp)
        {
            case TEST_BPP_8:
                EXPECT_EQ(128, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                EXPECT_EQ(256, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }

        // Mip 3 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 3;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip3Offset = 0;
        switch(bpp)
        {
            case TEST_BPP_8:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(128, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(64, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(256, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(512, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        uint32_t Mip3X                   = GFX_ALIGN_FLOOR(uint32_t(Mip3Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip3Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip3Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip3RenderAlignedOffset = Mip3Y * ExpectedPitch + (Mip3X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip3RenderAlignedOffset, ReqInfo.Render.Offset64);

        // Mip 4 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 4;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip4Offset = 0;
        switch(bpp)
        {
            case TEST_BPP_8:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(64, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(128, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(64, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(32, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height + Mip3Height) * ExpectedPitch;
                EXPECT_EQ(512, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        uint32_t Mip4X                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip4Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip4RenderAlignedOffset = Mip4Y * ExpectedPitch + (Mip4X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip4RenderAlignedOffset, ReqInfo.Render.Offset64);

        // Mip 5 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 4;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip5Offset = 0;
        switch(bpp)
        {
            case TEST_BPP_8:
                Mip5Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(64, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                Mip5Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(128, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                Mip5Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(64, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                Mip5Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(32, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                Mip5Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height + Mip3Height) * ExpectedPitch;
                EXPECT_EQ(512, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        uint32_t Mip5X                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip5Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip5RenderAlignedOffset = Mip5Y * ExpectedPitch + (Mip5X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip5RenderAlignedOffset, ReqInfo.Render.Offset64);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D TileYf Resource
TEST_F(CTestGen9Resource, Test2DTileYfResource)
{
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 64, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{64, 64},
                                                {128, 32},
                                                {128, 32},
                                                {256, 16},
                                                {256, 16}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    //Allocate 1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0]); // As wide as 1 Tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);       // 1 Tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4));    // 1 Tile Big
        VerifyResourceQPitch<false>(ResourceInfo, 0);            // Not Tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate surface that requires multi tiles in two dimension
    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
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
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 2);    // 2 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4) * 4);    // 4 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D TileYf Mipped Resource
TEST_F(CTestGen9Resource, Test2DTileYfMippedResource)
{
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 64, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{64, 64},
                                                {128, 32},
                                                {128, 32},
                                                {256, 16},
                                                {256, 16}};

    const uint32_t MtsWidth[TEST_BPP_MAX]  = {32, 32, 16, 16, 8};
    const uint32_t MtsHeight[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_2D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.MaxLod               = 4;
    gmmParams.ArraySize            = 4;

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
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);

        VerifyResourcePitchInTiles<true>(ResourceInfo, static_cast<uint32_t>(ExpectedPitch / TileSize[i][0]));
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * AlignedHeight * gmmParams.ArraySize, PAGE_SIZE));
        VerifyResourceQPitch<false>(ResourceInfo, AlignedHeight);

        // Mip 0 offsets, offset is 0,0
        GMM_REQ_OFFSET_INFO ReqInfo = {0};
        ReqInfo.MipLevel            = 0;
        ReqInfo.ReqRender           = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip0Size = ExpectedPitch * Mip0Height;
        EXPECT_EQ(0, ReqInfo.Render.Offset64);
        EXPECT_EQ(0, ReqInfo.Render.XOffset);
        EXPECT_EQ(0, ReqInfo.Render.YOffset);

        // Mip 1 offsets
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 1;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip1Offset = Mip0Size;
        switch(bpp)
        {
            case TEST_BPP_8:
                EXPECT_EQ(32, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                EXPECT_EQ(64, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        EXPECT_EQ(Mip1Offset, ReqInfo.Render.Offset64);


        // Mip 2 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 2;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip2Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
        switch(bpp)
        {
            case TEST_BPP_8:
                EXPECT_EQ(16, ReqInfo.Render.XOffset);
                EXPECT_EQ(32, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                EXPECT_EQ(32, ReqInfo.Render.XOffset);
                EXPECT_EQ(16, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                EXPECT_EQ(64, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                EXPECT_EQ(128, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        uint32_t Mip2X                   = GFX_ALIGN_FLOOR(uint32_t(Mip2Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip2Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip2Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip2RenderAlignedOffset = Mip2Y * ExpectedPitch + (Mip2X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip2RenderAlignedOffset, ReqInfo.Render.Offset64);

        // Mip 3 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 3;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip3Offset = 0;
        switch(bpp)
        {
            case TEST_BPP_8:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(48, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(24, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(32, ReqInfo.Render.XOffset);
                EXPECT_EQ(16, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(64, ReqInfo.Render.XOffset);
                EXPECT_EQ(8, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                Mip3Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(128, ReqInfo.Render.XOffset);
                EXPECT_EQ(0, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        uint32_t Mip3X                   = GFX_ALIGN_FLOOR(uint32_t(Mip3Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip3Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip3Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip3RenderAlignedOffset = Mip3Y * ExpectedPitch + (Mip3X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip3RenderAlignedOffset, ReqInfo.Render.Offset64);

        // Mip 4 offset
        ReqInfo           = {0};
        ReqInfo.MipLevel  = 4;
        ReqInfo.ReqRender = 1;
        ResourceInfo->GetOffset(ReqInfo);
        uint32_t Mip4Offset = 0;
        switch(bpp)
        {
            case TEST_BPP_8:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(32, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_16:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(16, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_32:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(24, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_64:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
                EXPECT_EQ(0, ReqInfo.Render.XOffset);
                EXPECT_EQ(12, ReqInfo.Render.YOffset);
                break;
            case TEST_BPP_128:
                Mip4Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + Mip2Height) * ExpectedPitch;
                EXPECT_EQ(64, ReqInfo.Render.XOffset);
                EXPECT_EQ(8, ReqInfo.Render.YOffset);
                break;
            default:
                break;
        }
        uint32_t Mip4X                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset % ExpectedPitch), TileSize[i][0]);
        uint32_t Mip4Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset / ExpectedPitch), TileSize[i][1]);
        uint32_t Mip4RenderAlignedOffset = Mip4Y * ExpectedPitch + (Mip4X / TileSize[i][0]) * (TileSize[i][0] * TileSize[i][1]);
        EXPECT_EQ(Mip4RenderAlignedOffset, ReqInfo.Render.Offset64);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}
/// @brief ULT for 2D Stencil (TileW) Mipped-array Resource
TEST_F(CTestGen9Resource, Test2DStencilMippedArrayedResource)
{
    const uint32_t HAlign = {8};
    const uint32_t VAlign = {8};

    const uint32_t TileSize[2] = {64, 64}; //TileW

    GMM_RESCREATE_PARAMS gmmParams      = {};
    gmmParams.Type                      = RESOURCE_2D;
    gmmParams.NoGfxMemory               = 1;
    gmmParams.Flags.Info.TiledW         = 1;
    gmmParams.Flags.Gpu.SeparateStencil = 1;
    gmmParams.MaxLod                    = 4;
    gmmParams.ArraySize                 = 4;

    //for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        uint32_t AlignedWidth  = 0;
        uint32_t AlignedHeight = 0;
        uint32_t ExpectedPitch = 0;
        // Valigned Mip Heights
        uint32_t Mip0Height = 0;
        uint32_t Mip1Height = 0;
        uint32_t Mip2Height = 0;

        // Haligned Mip Widths
        uint32_t Mip0Width = 0;
        uint32_t Mip1Width = 0;
        uint32_t Mip2Width = 0;

        TEST_BPP bpp          = TEST_BPP_8;
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x10;
        gmmParams.BaseHeight  = 0x10;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        // Mip resource Aligned Width calculation
        Mip0Width  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        Mip0Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);

        for(uint32_t i = 1; i <= gmmParams.MaxLod; i++)
        {
            uint32_t MipWidth  = GMM_ULT_ALIGN(GMM_ULT_MAX(Mip0Width >> i, 1), HAlign);
            uint32_t MipHeight = GMM_ULT_ALIGN(GMM_ULT_MAX(Mip0Height >> i, 1), VAlign);
            if(i == 1)
            {
                Mip1Width = AlignedWidth = MipWidth;
                Mip1Height               = MipHeight;
            }
            else if(i == 2)
            {
                AlignedWidth += MipWidth;
                Mip2Height = MipHeight;
            }
            else
            {
                Mip2Height += MipHeight;
            }
        }

        uint32_t MaxHeight = GMM_ULT_MAX(Mip1Height, Mip2Height);
        AlignedHeight      = Mip0Height + MaxHeight;

        ExpectedPitch = GMM_ULT_MAX(AlignedWidth, Mip0Width) * GetBppValue(bpp);
        ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, TileSize[0]);
        //TileW is programmed as row-interleaved.. ie doubled pitch
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch * 2);

        VerifyResourcePitchInTiles<true>(ResourceInfo, static_cast<uint32_t>(ExpectedPitch / TileSize[0]));
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * AlignedHeight * gmmParams.ArraySize, PAGE_SIZE));
        VerifyResourceQPitch<true>(ResourceInfo, AlignedHeight);

        for(uint8_t i = 0; i < gmmParams.ArraySize && gmmParams.MaxLod >= 4; i++)
        {
            uint64_t ArrayOffset = AlignedHeight * ExpectedPitch * i;

            // Mip 0 offsets, offset is 0,0
            GMM_REQ_OFFSET_INFO ReqInfo = {0};
            ReqInfo.MipLevel            = 0;
            ReqInfo.ReqRender           = 1;
            ReqInfo.ArrayIndex          = i;
            ResourceInfo->GetOffset(ReqInfo);

            uint32_t Mip0Size        = ExpectedPitch * Mip0Height;
            uint64_t SliceTileOffset = GFX_ALIGN_FLOOR(AlignedHeight * i, TileSize[1]) * TileSize[0];
            uint32_t SliceY          = (AlignedHeight * i) % TileSize[1];
            EXPECT_EQ(SliceTileOffset, ReqInfo.Render.Offset64);
            EXPECT_EQ(0, ReqInfo.Render.XOffset);
            EXPECT_EQ(SliceY, ReqInfo.Render.YOffset);

            // Mip 1 offsets
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 1;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip1Offset = Mip0Size + ArrayOffset;
            uint32_t Mip1X      = uint32_t(Mip1Offset % ExpectedPitch);
            uint32_t Mip1Y      = uint32_t(Mip1Offset / ExpectedPitch);
            EXPECT_EQ(0, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip1Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip1X                            = GFX_ALIGN_FLOOR(Mip1X, TileSize[0]);
            Mip1Y                            = GFX_ALIGN_FLOOR(Mip1Y, TileSize[1]);
            uint32_t Mip1RenderAlignedOffset = Mip1Y * ExpectedPitch + (Mip1X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip1RenderAlignedOffset, ReqInfo.Render.Offset64);


            // Mip 2 offset
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 2;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip2Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch + ArrayOffset;
            uint32_t Mip2X      = uint32_t(Mip2Offset % ExpectedPitch);
            uint32_t Mip2Y      = uint32_t(Mip2Offset / ExpectedPitch);
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip2Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip2X                            = GFX_ALIGN_FLOOR(Mip2X, TileSize[0]);
            Mip2Y                            = GFX_ALIGN_FLOOR(Mip2Y, TileSize[1]);
            uint32_t Mip2RenderAlignedOffset = Mip2Y * ExpectedPitch + (Mip2X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip2RenderAlignedOffset, ReqInfo.Render.Offset64);

            // Mip 3 offset
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 3;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);

            uint32_t Mip3Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + GMM_ULT_ALIGN(Mip0Height >> 2, VAlign)) * ExpectedPitch + ArrayOffset;
            uint32_t Mip3X      = uint32_t(Mip3Offset % ExpectedPitch);
            uint32_t Mip3Y      = uint32_t(Mip3Offset / ExpectedPitch);
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip3Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip3X                            = GFX_ALIGN_FLOOR(Mip3X, TileSize[0]);
            Mip3Y                            = GFX_ALIGN_FLOOR(Mip3Y, TileSize[1]);
            uint32_t Mip3RenderAlignedOffset = Mip3Y * ExpectedPitch + (Mip3X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip3RenderAlignedOffset, ReqInfo.Render.Offset64);

            // Mip 4 offset
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 4;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip4Offset = 0;
            Mip4Offset          = Mip1Width * GetBppValue(bpp) + (Mip0Height + GMM_ULT_ALIGN(Mip0Height >> 2, VAlign) + GMM_ULT_ALIGN(Mip0Height >> 3, VAlign)) * ExpectedPitch + ArrayOffset;
            uint32_t Mip4X      = uint32_t(Mip4Offset % ExpectedPitch);
            uint32_t Mip4Y      = uint32_t(Mip4Offset / ExpectedPitch);
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip4Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip4X                            = GFX_ALIGN_FLOOR(Mip4X, TileSize[0]);
            Mip4Y                            = GFX_ALIGN_FLOOR(Mip4Y, TileSize[1]);
            uint32_t Mip4RenderAlignedOffset = Mip4Y * ExpectedPitch + (Mip4X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip4RenderAlignedOffset, ReqInfo.Render.Offset64);
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 2D Stencil (TileW) Mipped-array Resource, and CpuBlt
TEST_F(CTestGen9Resource, Test2DStencilArrayedCpuBltResource)
{
    const uint32_t HAlign = {8};
    const uint32_t VAlign = {8};

    const uint32_t TileSize[2] = {64, 64}; //TileW

    GMM_RESCREATE_PARAMS gmmParams      = {};
    gmmParams.Type                      = RESOURCE_2D;
    gmmParams.NoGfxMemory               = 1;
    gmmParams.Flags.Info.TiledW         = 1;
    gmmParams.Flags.Gpu.SeparateStencil = 1;
    gmmParams.MaxLod                    = 0;
    gmmParams.ArraySize                 = 6;

    {
        uint32_t AlignedWidth  = 0;
        uint32_t AlignedHeight = 0;
        uint32_t ExpectedPitch = 0;
        // Valigned Mip Heights
        uint32_t Mip0Height = 0;
        uint32_t Mip1Height = 0;
        uint32_t Mip2Height = 0;

        // Haligned Mip Widths
        uint32_t Mip0Width = 0;
        uint32_t Mip1Width = 0;
        uint32_t Mip2Width = 0;

        TEST_BPP bpp          = TEST_BPP_8;
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x4;
        gmmParams.BaseHeight  = 0x4;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        // Mip resource Aligned Width calculation
        Mip0Width  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        Mip0Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);

        for(uint32_t i = 1; i <= gmmParams.MaxLod; i++)
        {
            uint32_t MipWidth  = GMM_ULT_ALIGN(GMM_ULT_MAX(Mip0Width >> i, 1), HAlign);
            uint32_t MipHeight = GMM_ULT_ALIGN(GMM_ULT_MAX(Mip0Height >> i, 1), VAlign);
            if(i == 1)
            {
                Mip1Width = AlignedWidth = MipWidth;
                Mip1Height               = MipHeight;
            }
            else if(i == 2)
            {
                AlignedWidth += MipWidth;
                Mip2Height = MipHeight;
            }
            else
            {
                Mip2Height += MipHeight;
            }
        }

        uint32_t MaxHeight = GMM_ULT_MAX(Mip1Height, Mip2Height);
        AlignedHeight      = Mip0Height + MaxHeight;

        ExpectedPitch = GMM_ULT_MAX(AlignedWidth, Mip0Width) * GetBppValue(bpp);
        ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, TileSize[0]);
        //TileW is programmed as row-interleaved.. ie doubled pitch
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch * 2);

        VerifyResourcePitchInTiles<true>(ResourceInfo, static_cast<uint32_t>(ExpectedPitch / TileSize[0]));
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * AlignedHeight * gmmParams.ArraySize, PAGE_SIZE));
        VerifyResourceQPitch<true>(ResourceInfo, AlignedHeight);

        for(uint8_t i = 0; i < gmmParams.ArraySize && gmmParams.MaxLod >= 4; i++)
        {
            uint64_t ArrayOffset = AlignedHeight * ExpectedPitch * i;

            // Mip 0 offsets, offset is 0,0
            GMM_REQ_OFFSET_INFO ReqInfo = {0};
            ReqInfo.MipLevel            = 0;
            ReqInfo.ReqRender           = 1;
            ReqInfo.ArrayIndex          = i;
            ResourceInfo->GetOffset(ReqInfo);

            uint32_t Mip0Size        = ExpectedPitch * Mip0Height;
            uint64_t SliceTileOffset = GFX_ALIGN_FLOOR(AlignedHeight * i, TileSize[1]) * TileSize[0];
            uint32_t SliceY          = (AlignedHeight * i) % TileSize[1];
            EXPECT_EQ(SliceTileOffset, ReqInfo.Render.Offset64);
            EXPECT_EQ(0, ReqInfo.Render.XOffset);
            EXPECT_EQ(SliceY, ReqInfo.Render.YOffset);

            // Mip 1 offsets
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 1;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip1Offset = Mip0Size + ArrayOffset;
            uint32_t Mip1X      = uint32_t(Mip1Offset % ExpectedPitch);
            uint32_t Mip1Y      = uint32_t(Mip1Offset / ExpectedPitch);
            EXPECT_EQ(0, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip1Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip1X                            = GFX_ALIGN_FLOOR(Mip1X, TileSize[0]);
            Mip1Y                            = GFX_ALIGN_FLOOR(Mip1Y, TileSize[1]);
            uint32_t Mip1RenderAlignedOffset = Mip1Y * ExpectedPitch + (Mip1X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip1RenderAlignedOffset, ReqInfo.Render.Offset64);


            // Mip 2 offset
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 2;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip2Offset = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch + ArrayOffset;
            uint32_t Mip2X      = uint32_t(Mip2Offset % ExpectedPitch);
            uint32_t Mip2Y      = uint32_t(Mip2Offset / ExpectedPitch);
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip2Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip2X                            = GFX_ALIGN_FLOOR(Mip2X, TileSize[0]);
            Mip2Y                            = GFX_ALIGN_FLOOR(Mip2Y, TileSize[1]);
            uint32_t Mip2RenderAlignedOffset = Mip2Y * ExpectedPitch + (Mip2X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip2RenderAlignedOffset, ReqInfo.Render.Offset64);

            // Mip 3 offset
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 3;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);

            uint32_t Mip3Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height + GMM_ULT_ALIGN(Mip0Height >> 2, VAlign)) * ExpectedPitch + ArrayOffset;
            uint32_t Mip3X      = uint32_t(Mip3Offset % ExpectedPitch);
            uint32_t Mip3Y      = uint32_t(Mip3Offset / ExpectedPitch);
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip3Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip3X                            = GFX_ALIGN_FLOOR(Mip3X, TileSize[0]);
            Mip3Y                            = GFX_ALIGN_FLOOR(Mip3Y, TileSize[1]);
            uint32_t Mip3RenderAlignedOffset = Mip3Y * ExpectedPitch + (Mip3X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip3RenderAlignedOffset, ReqInfo.Render.Offset64);

            // Mip 4 offset
            ReqInfo            = {0};
            ReqInfo.MipLevel   = 4;
            ReqInfo.ReqRender  = 1;
            ReqInfo.ArrayIndex = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip4Offset = 0;
            Mip4Offset          = Mip1Width * GetBppValue(bpp) + (Mip0Height + GMM_ULT_ALIGN(Mip0Height >> 2, VAlign) + GMM_ULT_ALIGN(Mip0Height >> 3, VAlign)) * ExpectedPitch + ArrayOffset;
            uint32_t Mip4X      = uint32_t(Mip4Offset % ExpectedPitch);
            uint32_t Mip4Y      = uint32_t(Mip4Offset / ExpectedPitch);
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(Mip4Y % TileSize[1], ReqInfo.Render.YOffset);
            Mip4X                            = GFX_ALIGN_FLOOR(Mip4X, TileSize[0]);
            Mip4Y                            = GFX_ALIGN_FLOOR(Mip4Y, TileSize[1]);
            uint32_t Mip4RenderAlignedOffset = Mip4Y * ExpectedPitch + (Mip4X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip4RenderAlignedOffset, ReqInfo.Render.Offset64);
        }

        //Verify CpuBlt path (uses Render offset for upload)
        {
#ifdef _WIN32
#define ULT_ALIGNED_MALLOC(Size, alignBytes) _aligned_malloc(Size, alignBytes)
#define ULT_ALIGNED_FREE(ptr) _aligned_free(ptr)
#else
#define ULT_ALIGNED_MALLOC(Size, alignBytes) memalign(alignBytes, Size)
#define ULT_ALIGNED_FREE(ptr) free(ptr)
#endif

#ifdef _WIN32
            void *LockVA = ULT_ALIGNED_MALLOC(ResourceInfo->GetSizeSurface(), ResourceInfo->GetBaseAlignment());
            memset(LockVA, 0, ResourceInfo->GetSizeSurface());
            void *Sysmem = malloc(gmmParams.BaseWidth64 * gmmParams.BaseHeight);
            memset(Sysmem, 0xbb, gmmParams.BaseWidth64 * gmmParams.BaseHeight);
            //Test Upload
            GMM_RES_COPY_BLT Blt  = {0};
            Blt.Gpu.pData         = LockVA;
            Blt.Gpu.Slice         = 4;
            Blt.Gpu.MipLevel      = 0;
            Blt.Sys.BufferSize    = gmmParams.BaseWidth64 * gmmParams.BaseHeight;
            Blt.Sys.pData         = Sysmem;
            Blt.Sys.RowPitch      = gmmParams.BaseWidth64;
            Blt.Sys.PixelPitch    = 1;
            Blt.Sys.SlicePitch    = Blt.Sys.BufferSize;
            Blt.Blt.Upload        = 1;
            Blt.Blt.BytesPerPixel = 1;
            ResourceInfo->CpuBlt(&Blt);

            uint64_t Offset = 0x100; /*Blt.Gpu.Slice * ResourceInfo->GetQPitchInBytes();*/ // Need SwizzledOffset
            for(uint8_t byte = 0; byte < Blt.Sys.BufferSize; byte++)
            {
                uint8_t *Byte = ((uint8_t *)LockVA) + Offset + byte;
                EXPECT_EQ(Byte[0], 0xbb);
            }

            free(Sysmem);
            ULT_ALIGNED_FREE(LockVA);
#endif
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D Stencil (TileW) Mipped-array Resource
TEST_F(CTestGen9Resource, Test3DStencilMippedResource)
{
    const uint32_t HAlign = {8};
    const uint32_t VAlign = {8};

    const uint32_t TileSize[2] = {64, 64}; //TileW

    GMM_RESCREATE_PARAMS gmmParams      = {};
    gmmParams.Type                      = RESOURCE_3D;
    gmmParams.NoGfxMemory               = 1;
    gmmParams.Flags.Info.TiledW         = 1;
    gmmParams.Flags.Gpu.SeparateStencil = 1;
    gmmParams.MaxLod                    = 4;

    {
        uint32_t AlignedWidth  = 0;
        uint32_t AlignedHeight = 0;
        uint32_t ExpectedPitch = 0;
        // Valigned Mip Heights
        uint32_t Mip0Height = 0;
        uint32_t Mip1Height = 0;
        uint32_t Mip2Height = 0;

        // Haligned Mip Widths
        uint32_t Mip0Width = 0;
        uint32_t Mip1Width = 0;
        uint32_t Mip2Width = 0;

        TEST_BPP bpp          = TEST_BPP_8;
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x10;
        gmmParams.BaseHeight  = 0x10;
        gmmParams.Depth       = 0x10;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        // Mip resource Aligned Width calculation
        Mip0Width  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        Mip0Height = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);

        for(uint32_t i = 1; i <= gmmParams.MaxLod; i++)
        {
            uint32_t MipWidth  = GMM_ULT_ALIGN(Mip0Width >> i, HAlign);
            uint32_t MipHeight = GMM_ULT_ALIGN(Mip0Height >> i, VAlign);
            if(i == 1)
            {
                Mip1Width = AlignedWidth = MipWidth;
                Mip1Height               = MipHeight;
            }
            else if(i == 2)
            {
                AlignedWidth += MipWidth;
                Mip2Height = MipHeight;
            }
            else
            {
                Mip2Height += MipHeight;
            }
        }

        uint32_t MaxHeight = GMM_ULT_MAX(Mip1Height, Mip2Height);
        AlignedHeight      = Mip0Height + MaxHeight;

        ExpectedPitch = AlignedWidth * GetBppValue(bpp);
        ExpectedPitch = GMM_ULT_ALIGN(ExpectedPitch, TileSize[0]);
        //TileW is programmed as row-interleaved.. ie doubled pitch
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch * 2);

        VerifyResourcePitchInTiles<true>(ResourceInfo, static_cast<uint32_t>(ExpectedPitch / TileSize[0]));
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * AlignedHeight, PAGE_SIZE) * gmmParams.Depth);

        //3D QPitch must be aligned to Tile-height, div by 2 for Pitch is doubled
        // i.e. Slice-offset = Qpitch*Pitch is tile-aligned.
        AlignedHeight = GMM_ULT_ALIGN(AlignedHeight, TileSize[1]);
        VerifyResourceQPitch<true>(ResourceInfo, AlignedHeight / 2);

        for(uint8_t i = 0; i < gmmParams.Depth; i++)
        {
            uint64_t SliceOffset = AlignedHeight * ExpectedPitch;
            // Mip 0 offsets, offset is 0,0
            GMM_REQ_OFFSET_INFO ReqInfo = {0};
            ReqInfo.MipLevel            = 0;
            ReqInfo.ReqRender           = 1;
            ReqInfo.Slice               = i;
            ResourceInfo->GetOffset(ReqInfo);

            uint32_t Mip0Size = ExpectedPitch * Mip0Height;
            EXPECT_EQ((0 + SliceOffset * i), ReqInfo.Render.Offset64);
            EXPECT_EQ(0, ReqInfo.Render.XOffset);
            EXPECT_EQ(0, ReqInfo.Render.YOffset);

            // Mip 1 offsets -- uses 2d mip layout
            ReqInfo           = {0};
            ReqInfo.MipLevel  = 1;
            ReqInfo.ReqRender = 1;
            ReqInfo.Slice     = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip1Offset              = Mip0Size;
            uint32_t Mip1X                   = GFX_ALIGN_FLOOR(uint32_t(Mip1Offset % ExpectedPitch), TileSize[0]);
            uint32_t Mip1Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip1Offset / ExpectedPitch), TileSize[1]);
            uint32_t Mip1RenderAlignedOffset = Mip1Y * ExpectedPitch + (Mip1X / TileSize[0]) * (TileSize[0] * TileSize[1]) + SliceOffset * i;
            EXPECT_EQ(0, ReqInfo.Render.XOffset);
            EXPECT_EQ(16, ReqInfo.Render.YOffset);
            EXPECT_EQ(Mip1RenderAlignedOffset, ReqInfo.Render.Offset64);


            // Mip 2 offset
            ReqInfo           = {0};
            ReqInfo.MipLevel  = 2;
            ReqInfo.ReqRender = 1;
            ReqInfo.Slice     = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip2Offset = Mip1Width * GetBppValue(bpp) + (Mip0Height * ExpectedPitch) + SliceOffset * i;
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(16, ReqInfo.Render.YOffset);
            uint32_t Mip2X                   = GFX_ALIGN_FLOOR(uint32_t(Mip2Offset % ExpectedPitch), TileSize[0]);
            uint32_t Mip2Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip2Offset / ExpectedPitch), TileSize[1]);
            uint32_t Mip2RenderAlignedOffset = Mip2Y * ExpectedPitch + (Mip2X / TileSize[0]) * (TileSize[0] * TileSize[1]);
            EXPECT_EQ(Mip2RenderAlignedOffset, ReqInfo.Render.Offset64);

            // Mip 3 offset
            ReqInfo           = {0};
            ReqInfo.MipLevel  = 3;
            ReqInfo.ReqRender = 1;
            ReqInfo.Slice     = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip3Offset = 0;
            Mip3Offset          = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(24, ReqInfo.Render.YOffset);
            uint32_t Mip3X                   = GFX_ALIGN_FLOOR(uint32_t(Mip3Offset % ExpectedPitch), TileSize[0]);
            uint32_t Mip3Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip3Offset / ExpectedPitch), TileSize[1]);
            uint32_t Mip3RenderAlignedOffset = Mip3Y * ExpectedPitch + (Mip3X / TileSize[0]) * (TileSize[0] * TileSize[1]) + SliceOffset * i;
            EXPECT_EQ(Mip3RenderAlignedOffset, ReqInfo.Render.Offset64);

            // Mip 4 offset
            ReqInfo           = {0};
            ReqInfo.MipLevel  = 4;
            ReqInfo.ReqRender = 1;
            ReqInfo.Slice     = i;
            ResourceInfo->GetOffset(ReqInfo);
            uint32_t Mip4Offset = 0;
            Mip4Offset          = Mip1Width * GetBppValue(bpp) + Mip0Height * ExpectedPitch;
            EXPECT_EQ(8, ReqInfo.Render.XOffset);
            EXPECT_EQ(32, ReqInfo.Render.YOffset);
            uint32_t Mip4X                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset % ExpectedPitch), TileSize[0]);
            uint32_t Mip4Y                   = GFX_ALIGN_FLOOR(uint32_t(Mip4Offset / ExpectedPitch), TileSize[1]);
            uint32_t Mip4RenderAlignedOffset = Mip4Y * ExpectedPitch + (Mip4X / TileSize[0]) * (TileSize[0] * TileSize[1]) + SliceOffset * i;
            EXPECT_EQ(Mip4RenderAlignedOffset, ReqInfo.Render.Offset64);
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

// ********************************************************************************//
/// @brief ULT for 3D Linear Resource
TEST_F(CTestGen9Resource, Test3DLinearResource)
{
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign = 16;
    const uint32_t VAlign = 4;

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_3D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.Linear    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        const uint32_t MinPitch       = 32;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t AlignedWidth  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        const uint32_t AlignedHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
        uint32_t       PitchInBytes  = AlignedWidth * GetBppValue(bpp);
        PitchInBytes                 = GMM_ULT_MAX(PitchInBytes, MinPitch);
        PitchInBytes                 = GMM_ULT_ALIGN(PitchInBytes, PitchAlignment);
        const uint32_t AlignedSize   = GMM_ULT_ALIGN(PitchInBytes * AlignedHeight, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, PitchInBytes);
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<true>(ResourceInfo, AlignedHeight);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 256 x 256 x 256
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 256;
        gmmParams.BaseHeight  = 256;
        gmmParams.Depth       = 256;

        const uint32_t MinPitch       = 32;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t AlignedWidth  = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        const uint32_t AlignedHeight = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
        uint32_t       PitchInBytes  = AlignedWidth * GetBppValue(bpp);
        PitchInBytes                 = GFX_MAX(PitchInBytes, MinPitch);
        PitchInBytes                 = GMM_ULT_ALIGN(PitchInBytes, PitchAlignment);
        const uint32_t AlignedSize   = GMM_ULT_ALIGN(PitchInBytes * AlignedHeight * gmmParams.Depth, PAGE_SIZE);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, PitchInBytes);
        VerifyResourceSize<true>(ResourceInfo, AlignedSize);
        VerifyResourceQPitch<true>(ResourceInfo, AlignedHeight);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileX Resource
TEST_F(CTestGen9Resource, Test3DTileXResource)
{
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign                    = 16;
    const uint32_t VAlign                    = 4;
    const uint32_t TileSize[TEST_BPP_MAX][3] = {{512, 8, 1},
                                                {512, 8, 1},
                                                {512, 8, 1},
                                                {512, 8, 1},
                                                {512, 8, 1}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_3D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledX    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0]);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, 2 * TileSize[i][0]);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);
        VerifyResourceSize<true>(ResourceInfo, 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight  = TileSize[i][1] + 1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, 2 * TileSize[i][0]);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);
        VerifyResourceSize<true>(ResourceInfo, 2 * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, 2 * TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight  = TileSize[i][1] + 1;
        gmmParams.Depth       = TileSize[i][2] + 1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, 2 * TileSize[i][0]);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);
        VerifyResourceSize<true>(ResourceInfo, 2 * 2 * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, 2 * TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileY Resource
TEST_F(CTestGen9Resource, Test3DTileYResource)
{
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign                    = 16;
    const uint32_t VAlign                    = 4;
    const uint32_t TileSize[TEST_BPP_MAX][3] = {{128, 32, 1},
                                                {128, 32, 1},
                                                {128, 32, 1},
                                                {128, 32, 1},
                                                {128, 32, 1}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_3D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        const uint32_t MinPitch       = 32;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t       PitchInBytes = AlignedWidth * GetBppValue(bpp);
        PitchInBytes                = GFX_MAX(PitchInBytes, MinPitch);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, PitchAlignment);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, TileSize[i][0]);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, PitchInBytes);
        VerifyResourcePitchInTiles<true>(ResourceInfo, PitchInBytes / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, PitchInBytes / TileSize[i][0] * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        const uint32_t MinPitch       = 32;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t       PitchInBytes = AlignedWidth * GetBppValue(bpp);
        PitchInBytes                = GFX_MAX(PitchInBytes, MinPitch);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, PitchAlignment);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, TileSize[i][0]);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, PitchInBytes);
        VerifyResourcePitchInTiles<true>(ResourceInfo, PitchInBytes / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, PitchInBytes / TileSize[i][0] * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight  = TileSize[i][1] + 1;
        gmmParams.Depth       = 0x1;

        const uint32_t MinPitch       = 32;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t       PitchInBytes = AlignedWidth * GetBppValue(bpp);
        PitchInBytes                = GFX_MAX(PitchInBytes, MinPitch);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, PitchAlignment);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, TileSize[i][0]);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, PitchInBytes);
        VerifyResourcePitchInTiles<true>(ResourceInfo, PitchInBytes / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, PitchInBytes / TileSize[i][0] * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1] * 2);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight  = TileSize[i][1] + 1;
        gmmParams.Depth       = TileSize[i][2] + 1;

        const uint32_t MinPitch       = 32;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t AlignedWidth = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign);
        uint32_t       PitchInBytes = AlignedWidth * GetBppValue(bpp);
        PitchInBytes                = GFX_MAX(PitchInBytes, MinPitch);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, PitchAlignment);
        PitchInBytes                = GMM_ULT_ALIGN(PitchInBytes, TileSize[i][0]);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, PitchInBytes);
        VerifyResourcePitchInTiles<true>(ResourceInfo, PitchInBytes / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, PitchInBytes / TileSize[i][0] * 2 * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1] * 2);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileYs Resource
TEST_F(CTestGen9Resource, Test3DTileYsResource)
{
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 32, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {32, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][3] = {{64, 32, 32},
                                                {64, 32, 32},
                                                {128, 32, 16},
                                                {256, 16, 16},
                                                {256, 16, 16}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_3D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
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

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
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

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = TileSize[i][1] + 1;                      // 1 row larger than 1 tile height
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[i][0] * 2); // As wide as 2 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2);           // 2 tile wide
        VerifyResourceSize<false>(ResourceInfo, GMM_KBYTE(64) * 4);  // 4 tile big

        VerifyResourceQPitch<false>(ResourceInfo, 0); // Not tested

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
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

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileYs Mipped Resource
TEST_F(CTestGen9Resource, Test3DTileYsMippedResource)
{
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign[TEST_BPP_MAX] = {64, 32, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {32, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][3] = {{64, 32, 32},
                                                {64, 32, 32},
                                                {128, 32, 16},
                                                {256, 16, 16},
                                                {256, 16, 16}};
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        const uint32_t ResourceWidth  = 0x100;
        const uint32_t ResourceHeight = 0x100;
        const uint32_t ResourceDepth  = 0x100;
        const uint32_t MaxLod         = 0x5;

        TEST_BPP             bpp       = static_cast<TEST_BPP>(i);
        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_3D;
        gmmParams.Flags.Info.TiledY    = 1;
        gmmParams.Flags.Info.TiledYs   = 1;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.BaseWidth64          = ResourceWidth;
        gmmParams.BaseHeight           = ResourceHeight;
        gmmParams.Depth                = ResourceDepth;
        gmmParams.MaxLod               = MaxLod;
        gmmParams.Format               = SetResourceFormat(bpp);

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        const uint32_t Pitch      = ResourceWidth * GetBppValue(bpp);
        const uint32_t Mip0Height = ResourceHeight;

        // Mip0
        GMM_REQ_OFFSET_INFO OffsetInfo = {};
        OffsetInfo.ReqRender           = 1;
        OffsetInfo.MipLevel            = 0;
        ResourceInfo->GetOffset(OffsetInfo);

        EXPECT_EQ(0, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        // Mip1
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 1;
        ResourceInfo->GetOffset(OffsetInfo);

        const uint32_t SizeOfMip0 = Pitch * ResourceHeight * TileSize[i][2];
        const uint32_t Mip1Offset = SizeOfMip0;
        const uint32_t Mip1Width  = ResourceWidth >> 1;
        const uint32_t Mip1Height = ResourceHeight >> 1;

        EXPECT_EQ(Mip1Offset, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        // Mip2
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 2;
        ResourceInfo->GetOffset(OffsetInfo);

        const uint32_t Mip2Height = ResourceHeight >> 2;
        const uint32_t Mip2Offset = Mip1Offset + Mip1Width * GetBppValue(bpp) / TileSize[i][0] * GMM_KBYTE(64);

        EXPECT_EQ(Mip2Offset, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        // Mip3
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 3;
        ResourceInfo->GetOffset(OffsetInfo);

        const uint32_t Mip3Y      = (Mip0Height + Mip2Height) * TileSize[i][2];
        const uint32_t Mip3X      = Mip1Width;
        const uint32_t Mip3Offset = Mip3Y * Pitch + Mip3X * GetBppValue(bpp) / TileSize[i][0] * GMM_KBYTE(64);

        EXPECT_EQ(Mip3Offset, OffsetInfo.Render.Offset64);
        EXPECT_EQ(bpp == TEST_BPP_8 ? 32 : 0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        // Mip4
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 4;
        ResourceInfo->GetOffset(OffsetInfo);

        const uint32_t Mip3Height = ResourceHeight >> 3;
        const uint32_t Mip4Y      = (Mip0Height + Mip2Height + Mip3Height) * TileSize[i][2];
        const uint32_t Mip4X      = Mip1Width;
        const uint32_t Mip4Offset = Mip4Y * Pitch + Mip4X * GetBppValue(bpp) / TileSize[i][0] * GMM_KBYTE(64);

        switch(bpp)
        {
            case TEST_BPP_8:
                EXPECT_EQ(Mip3Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(0, OffsetInfo.Render.XOffset);
                EXPECT_EQ(16, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_16:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(32, OffsetInfo.Render.XOffset);
                EXPECT_EQ(0, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_32:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(64, OffsetInfo.Render.XOffset);
                EXPECT_EQ(0, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_64:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(128, OffsetInfo.Render.XOffset);
                EXPECT_EQ(0, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_128:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(0, OffsetInfo.Render.XOffset);
                EXPECT_EQ(0, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            default:
                break;
        }

        // Mip5
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 5;
        ResourceInfo->GetOffset(OffsetInfo);

        const uint32_t Mip4Height = ResourceHeight >> 4;
        const uint32_t Mip5Y      = (Mip0Height + Mip2Height + Mip3Height + Mip4Height) * TileSize[i][2];
        const uint32_t Mip5X      = Mip1Width;
        const uint32_t Mip5Offset = Mip5Y * Pitch + Mip4X * GetBppValue(bpp) / TileSize[i][0] * GMM_KBYTE(64);

        switch(bpp)
        {
            case TEST_BPP_8:
                EXPECT_EQ(Mip3Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(0, OffsetInfo.Render.XOffset);
                EXPECT_EQ(0, OffsetInfo.Render.YOffset);
                EXPECT_EQ(16, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_16:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(0, OffsetInfo.Render.XOffset);
                EXPECT_EQ(16, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_32:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(0, OffsetInfo.Render.XOffset);
                EXPECT_EQ(16, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_64:
                EXPECT_EQ(Mip4Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(0, OffsetInfo.Render.XOffset);
                EXPECT_EQ(8, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            case TEST_BPP_128:
                EXPECT_EQ(Mip5Offset, OffsetInfo.Render.Offset64);
                EXPECT_EQ(128, OffsetInfo.Render.XOffset);
                EXPECT_EQ(0, OffsetInfo.Render.YOffset);
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                break;
            default:
                break;
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for 3D TileYf Resource
TEST_F(CTestGen9Resource, Test3DTileYfResource)
{
    // Horizontal/Vertical pixel alignment
    const uint32_t HAlign[TEST_BPP_MAX]      = {16, 8, 8, 8, 4};
    const uint32_t VAlign[TEST_BPP_MAX]      = {16, 16, 16, 8, 8};
    const uint32_t TileSize[TEST_BPP_MAX][3] = {{16, 16, 16},
                                                {16, 16, 16},
                                                {32, 16, 8},
                                                {64, 8, 8},
                                                {64, 8, 8}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_3D;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1x1 surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp                  = static_cast<TEST_BPP>(i);
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = 0x1;
        gmmParams.BaseHeight          = 0x1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0], PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0], PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0], PitchAlignment) / TileSize[i][0] * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp                  = static_cast<TEST_BPP>(i);
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = 0x1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;


        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment) / TileSize[i][0] * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1]);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp                  = static_cast<TEST_BPP>(i);
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = TileSize[i][1] + 1;
        gmmParams.Depth               = 0x1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment) / TileSize[i][0] * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1] * 2);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X/Y/Z dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp                  = static_cast<TEST_BPP>(i);
        gmmParams.Format              = SetResourceFormat(bpp);
        gmmParams.BaseWidth64         = (TileSize[i][0] / GetBppValue(bpp)) + 1;
        gmmParams.BaseHeight          = TileSize[i][1] + 1;
        gmmParams.Depth               = TileSize[i][2] + 1;
        const uint32_t PitchAlignment = 32;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);
        VerifyResourcePitch<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment));
        VerifyResourcePitchInTiles<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment) / TileSize[i][0]);
        VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(TileSize[i][0] * 2, PitchAlignment) / TileSize[i][0] * 2 * 2 * GMM_KBYTE(4));
        VerifyResourceQPitch<true>(ResourceInfo, TileSize[i][1] * 2);

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}


/// @brief ULT for Cube Linear Resource
TEST_F(CTestGen9Resource, TestCubeLinearResource)
{
    const uint32_t HAlign = 16;
    const uint32_t VAlign = 4;

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.Linear    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        uint32_t ExpectedPitch = GMM_ULT_MAX(GMM_BYTES(32), HAlign * GetBppValue(bpp)); // Min Pitch = 32 bytes
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);                         // As wide as 1 tile
        VerifyResourcePitchInTiles<false>(ResourceInfo, 1);                             // not applicable

        uint32_t ExpectedQPitch = VAlign;
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned PAGE_SIZE
                                 GMM_ULT_ALIGN(ExpectedPitch *
                                               __GMM_MAX_CUBE_FACE * ExpectedQPitch,
                                               PAGE_SIZE));

        for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
            ResourceInfo->GetOffset(OffsetInfo);

            EXPECT_EQ(CubeFaceIndex * ExpectedQPitch * ExpectedPitch,
                      OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
            EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Y Offset should be 0
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate arbitrary size (X/Y dimension not applicable as linear surface)
    // Width and Height must be equal
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x201;                 // 512 + 1, help ult HAlign/VAlign/Pitch alignment logic as well.
        gmmParams.BaseHeight  = gmmParams.BaseWidth64; // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        uint32_t ExpectedPitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign) * GetBppValue(bpp); // HAligned-width in bytes.
        ExpectedPitch          = GMM_ULT_ALIGN(ExpectedPitch, GMM_BYTES(32));
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<false>(ResourceInfo, 2); // not applicable

        uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned PAGE_SIZE
                                 GMM_ULT_ALIGN(ExpectedPitch *
                                               __GMM_MAX_CUBE_FACE * ExpectedQPitch,
                                               PAGE_SIZE));

        for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
            ResourceInfo->GetOffset(OffsetInfo);

            EXPECT_EQ(CubeFaceIndex * ExpectedQPitch * ExpectedPitch,
                      OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
            EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Y Offset should be 0
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Cube Linear Mipped Resource Array
TEST_F(CTestGen9Resource, TestCubeLinearMippedResourceArray)
{
    const uint32_t HAlign       = 16;
    const uint32_t VAlign       = 4;
    const uint32_t MaxLod       = 9;
    const uint32_t ResWidth     = 0x401;
    const uint32_t MaxArraySize = 0x10;


    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.Linear    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;
    gmmParams.MaxLod               = MaxLod;
    gmmParams.ArraySize            = MaxArraySize;

    // Allocate arbitrary size (X/Y dimension not applicable as linear surface)
    // Width and Height must be equal
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        struct //Cache the value for verifying array elements/Cube face offset/Mip Offset
        {
            uint64_t Offset; // Note : absolute mip offset
        } RenderOffset[GMM_ULT_MAX_MIPMAP] = {};

        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = ResWidth;              // 1024 + 1, help ult HAlign/VAlign/Pitch alignment logic as well.
        gmmParams.BaseHeight  = gmmParams.BaseWidth64; // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        //------------------------------|
        //                              |
        //          LOD0                |
        //                              |
        //                              |
        //------------------------------|
        //    LOD1     |    LOD2  |
        //             |----------|
        //             | LOD3 |
        //-------------| LOD4 .. so on

        // Mip 0
        // Mip 0 decides the pitch of the entire surface
        const uint32_t AlignedWidthMip0  = GMM_ULT_ALIGN(ResWidth, HAlign); // HAlign width in pixel
        const uint32_t AlignedHeightMip0 = GMM_ULT_ALIGN(ResWidth, VAlign);
        uint32_t       ExpectedPitch     = GMM_ULT_ALIGN(gmmParams.BaseWidth64, HAlign) * GetBppValue(bpp); // HAligned-width in bytes.
        ExpectedPitch                    = GMM_ULT_ALIGN(ExpectedPitch, GMM_BYTES(32));
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<false>(ResourceInfo, 2); // Not applicable

        // Mip0 should be at offset 0 and tile aligned
        GMM_REQ_OFFSET_INFO OffsetInfo = {};
        OffsetInfo.ReqRender           = 1;
        OffsetInfo.MipLevel            = 0; //Mip 0
        ResourceInfo->GetOffset(OffsetInfo);
        EXPECT_EQ(0, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        //cache Mip 0 offset
        RenderOffset[0].Offset = 0;

        // Mip 1 should be under mip 0
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 1; //Mip 1
        ResourceInfo->GetOffset(OffsetInfo);

        EXPECT_EQ(AlignedHeightMip0 * ExpectedPitch, // Render offset is the absolute address at which the mip begins
                  OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset); // Not applicable for linear surface
        EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Not applicable for linear surface
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // n/a

        //cache Mip 1 offset
        RenderOffset[1].Offset = AlignedHeightMip0 * ExpectedPitch; //Absolute base

        const uint32_t AlignedWidthMip1  = GMM_ULT_ALIGN(ResWidth >> 1, HAlign); // Align width in pixel to HAlign
        const uint32_t AlignedHeightMip1 = GMM_ULT_ALIGN(ResWidth >> 1, VAlign);

        uint32_t HeightOfMip;
        uint32_t HeightLinesLevel2 = 0;

        // Mips 2-9 should be stacked on the right of Mip1 as shown in figure above.
        for(int i = 2; i <= MaxLod; i++)
        {
            OffsetInfo           = {};
            OffsetInfo.ReqRender = 1;
            OffsetInfo.MipLevel  = i;
            ResourceInfo->GetOffset(OffsetInfo);

            HeightOfMip = GMM_ULT_ALIGN(ResWidth >> i, VAlign);

            EXPECT_EQ((AlignedHeightMip0 + HeightLinesLevel2) * ExpectedPitch + // Render offset is tile's base address on which mip begins
                      (AlignedWidthMip1 * GetBppValue(bpp)),
                      OffsetInfo.Render.Offset64);

            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // Not applicable for linear surface
            EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Not applicable for linear surface
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

            //cache Mip i'th offset
            RenderOffset[i].Offset = (AlignedHeightMip0 + HeightLinesLevel2) * ExpectedPitch +
                                     (AlignedWidthMip1 * GetBppValue(bpp));

            HeightLinesLevel2 += HeightOfMip;
        }

        uint32_t Max2DHeight    = GMM_ULT_MAX(AlignedHeightMip1, HeightLinesLevel2);
        uint32_t ExpectedQPitch = GFX_ALIGN_NP2(AlignedHeightMip0 + Max2DHeight, VAlign);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 GMM_ULT_ALIGN(ExpectedPitch *
                                               MaxArraySize * __GMM_MAX_CUBE_FACE * ExpectedQPitch,
                                               PAGE_SIZE));

        // Verify each array element's  Mip offset, Cube face offset etc.
        for(uint32_t ArrayIndex = 0; ArrayIndex < __GMM_MAX_CUBE_FACE; ArrayIndex++)
        {
            for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
            {
                GMM_REQ_OFFSET_INFO OffsetInfo = {};
                OffsetInfo.ReqRender           = 1;
                OffsetInfo.ArrayIndex          = ArrayIndex;
                OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
                ResourceInfo->GetOffset(OffsetInfo);

                //Verify cube face offsets
                EXPECT_EQ(((6 * ArrayIndex) + CubeFaceIndex) * ExpectedQPitch * ExpectedPitch,
                          OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
                EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0 as linear surf
                EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Y Offset should be 0 as linear surf
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0

                uint32_t CubeFaceBaseOffset = ((6 * ArrayIndex) + CubeFaceIndex) * (ExpectedQPitch * ExpectedPitch);

                //Verify mip offsets in each cube face
                for(uint32_t Lod = 0; Lod <= MaxLod; Lod++)
                {
                    OffsetInfo.MipLevel = Lod;
                    ResourceInfo->GetOffset(OffsetInfo);

                    uint32_t MipOffset = CubeFaceBaseOffset + RenderOffset[Lod].Offset;

                    uint32_t OffsetX = MipOffset % ExpectedPitch;
                    uint32_t OffsetY = MipOffset / ExpectedPitch;

                    uint32_t RenderAlignOffset = OffsetY * ExpectedPitch + OffsetX;

                    EXPECT_EQ(RenderAlignOffset, OffsetInfo.Render.Offset64); // Render offset absolute address on which cube face begins.
                    EXPECT_EQ(0, OffsetInfo.Render.XOffset);                  // X Offset should be 0 as linear surf
                    EXPECT_EQ(0, OffsetInfo.Render.YOffset);                  // Y Offset should be 0 as linear surf
                    EXPECT_EQ(0, OffsetInfo.Render.ZOffset);                  // Z offset N/A should be 0
                }
            }
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Cube TileX Resource
TEST_F(CTestGen9Resource, TestCubeTileXResource)
{
    // Cube is allocated as an array of 6 2D surface representing each cube face below
    //===============================
    //  q  coordinate  |    face    |
    //      0          |    + x     |
    //      1          |    - x     |
    //      2          |    + y     |
    //      3          |    - y     |
    //      4          |    + z     |
    //      5          |    - z     |
    //===============================

    const uint32_t HAlign = 16;
    const uint32_t VAlign = 4;

    const uint32_t TileSize[1][2] = {512, 8};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledX    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface so that it occupies 1 Tile in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        uint32_t ExpectedPitch = TileSize[0][0];
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch); // As wide as 1 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);      // 1 tile wide

        uint32_t ExpectedQPitch = VAlign;
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 GMM_ULT_ALIGN(__GMM_MAX_CUBE_FACE * ExpectedQPitch,
                                               TileSize[0][1]));

        for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
            ResourceInfo->GetOffset(OffsetInfo);

            EXPECT_EQ(GMM_ULT_ALIGN_FLOOR(CubeFaceIndex * ExpectedQPitch, TileSize[0][1]) * ExpectedPitch,
                      OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
            EXPECT_EQ((CubeFaceIndex * ExpectedQPitch) % TileSize[0][1],
                      OffsetInfo.Render.YOffset);    // Y Offset should be (CubeFaceIndex * QPitch) % TileHeight
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension.
    // Width and Height must be equal
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[0][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = gmmParams.BaseWidth64;                   // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        uint32_t ExpectedPitch = TileSize[0][0] * 2; // As wide as 2 tile
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2); // 2 tile wide

        uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 GMM_ULT_ALIGN(__GMM_MAX_CUBE_FACE * ExpectedQPitch,
                                               TileSize[0][1]));

        for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
            ResourceInfo->GetOffset(OffsetInfo);
            EXPECT_EQ(GMM_ULT_ALIGN_FLOOR(CubeFaceIndex * ExpectedQPitch, TileSize[0][1]) * ExpectedPitch,
                      OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
            EXPECT_EQ((CubeFaceIndex * ExpectedQPitch) % TileSize[0][1],
                      OffsetInfo.Render.YOffset);    // Y Offset should be (CubeFaceIndex * QPitch) % TileHeight
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}


/// @brief ULT for Cube TileY Resource
TEST_F(CTestGen9Resource, TestCubeTileYResource)
{
    // Cube is allocated as an array of 6 2D surface representing each cube face below
    //===============================
    // �q� coordinate  |    face    |
    //      0          |    + x     |
    //      1          |    - x     |
    //      2          |    + y     |
    //      3          |    - y     |
    //      4          |    + z     |
    //      5          |    - z     |
    //===============================

    const uint32_t HAlign = 16;
    const uint32_t VAlign = 4;

    const uint32_t TileSize[1][2] = {128, 32};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface within a tile.
    for(uint32_t i = 0; i < TEST_BPP_128; i++) //TEST_BPP_128 cannot fit in a tile as HAlign = 16
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);
        VerifyResourcePitch<true>(ResourceInfo, TileSize[0][0]); // As wide as 1 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);       // 1 tile wide
        VerifyResourceSize<true>(ResourceInfo, GMM_KBYTE(4));    // All 6 faces should be accomated in a tile.
        VerifyResourceQPitch<true>(ResourceInfo, VAlign);        // Each face should be VAlign rows apart within a tile

        for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
            ResourceInfo->GetOffset(OffsetInfo);
            EXPECT_EQ(0, OffsetInfo.Render.Offset64);                     // Render offset should be 0 as its on single tile.
            EXPECT_EQ(0, OffsetInfo.Render.XOffset);                      // X Offset should be 0
            EXPECT_EQ(CubeFaceIndex * VAlign, OffsetInfo.Render.YOffset); // Y Offset should be VALIGN * CubeFace Index
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }

    // Allocate 2 tiles in X dimension.
    // Width and Height of Cube must be equal
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[0][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = gmmParams.BaseWidth64;                   // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        uint32_t ExpectedPitch = TileSize[0][0] * 2; // As wide as 2 tile
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2); // 2 tile wide

        uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 GMM_ULT_ALIGN(__GMM_MAX_CUBE_FACE * ExpectedQPitch,
                                               TileSize[0][1]));


        for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
        {
            GMM_REQ_OFFSET_INFO OffsetInfo = {};
            OffsetInfo.ReqRender           = 1;
            OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
            ResourceInfo->GetOffset(OffsetInfo);
            EXPECT_EQ(GMM_ULT_ALIGN_FLOOR(CubeFaceIndex * ExpectedQPitch, TileSize[0][1]) * ExpectedPitch,
                      OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
            EXPECT_EQ((CubeFaceIndex * ExpectedQPitch) % TileSize[0][1],
                      OffsetInfo.Render.YOffset);    // Y Offset should be (CubeFaceIndex * QPitch) % TileHeight
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Cube TileYs Resource
TEST_F(CTestGen9Resource, TestCubeTileYsResource)
{
    // Cube is allocated as an array of 6 2D surface representing each cube face below
    //===============================
    // �q� coordinate  |    face    |
    //      0          |    + x     |
    //      1          |    - x     |
    //      2          |    + y     |
    //      3          |    - y     |
    //      4          |    + z     |
    //      5          |    - z     |
    //===============================

    const uint32_t HAlign[TEST_BPP_MAX] = {256, 256, 128, 128, 64};
    const uint32_t VAlign[TEST_BPP_MAX] = {256, 128, 128, 64, 64};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{256, 256},
                                                {512, 128},
                                                {512, 128},
                                                {1024, 64},
                                                {1024, 64}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface so that it occupies 1 Tile in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);

        uint32_t ExpectedPitch = TileSize[i][0];
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch); // As wide as 1 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);      // 1 tile wide

        uint32_t ExpectedQPitch = VAlign[i];
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 __GMM_MAX_CUBE_FACE * ExpectedQPitch);

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
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = gmmParams.BaseWidth64;                   // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);

        uint32_t ExpectedPitch = TileSize[i][0] * 2; // As wide as 2 tile
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2); // 2 tile wide

        uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign[i]);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 __GMM_MAX_CUBE_FACE * ExpectedQPitch);

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

/// @brief ULT for Cube TileYf Resource
TEST_F(CTestGen9Resource, TestCubeTileYfResource)
{
    // Cube is allocated as an array of 6 2D surface representing each cube face below
    //===============================
    // �q� coordinate  |    face    |
    //      0          |    + x     |
    //      1          |    - x     |
    //      2          |    + y     |
    //      3          |    - y     |
    //      4          |    + z     |
    //      5          |    - z     |
    //===============================

    const uint32_t HAlign[TEST_BPP_MAX] = {64, 64, 32, 32, 16};
    const uint32_t VAlign[TEST_BPP_MAX] = {64, 32, 32, 16, 16};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{64, 64},
                                                {128, 32},
                                                {128, 32},
                                                {256, 16},
                                                {256, 16}};

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYf   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate 1x1 surface so that it occupies 1 Tile in X dimension
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = 0x1;
        gmmParams.BaseHeight  = 0x1;
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);

        uint32_t ExpectedPitch = TileSize[i][0];
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch); // As wide as 1 tile
        VerifyResourcePitchInTiles<true>(ResourceInfo, 1);      // 1 tile wide

        uint32_t ExpectedQPitch = VAlign[i];
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be VAlign rows apart within a tile

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 __GMM_MAX_CUBE_FACE * ExpectedQPitch);

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
        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = (TileSize[i][0] / GetBppValue(bpp)) + 1; // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = gmmParams.BaseWidth64;                   // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);

        uint32_t ExpectedPitch = TileSize[i][0] * 2; // As wide as 2 tile
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, 2); // 2 tile wide

        uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, VAlign[i]);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 __GMM_MAX_CUBE_FACE * ExpectedQPitch);

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


/// @brief ULT for Cube TileY Mipped Resource Array
TEST_F(CTestGen9Resource, TestCubeTileYMippedResourceArray)
{
    const uint32_t HAlign = 16;
    const uint32_t VAlign = 4;

    const uint32_t TileSize[2] = {128, 32};
    enum Coords
    {
        X = 0,
        Y = 1
    };

    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate CUBE surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        const uint32_t ResWidth     = 0x201;
        const uint32_t MaxLod       = 0x9;
        const uint32_t MaxArraySize = 0x10;

        struct //Cache the value for verifying array elements/Cube face offset/Mip Offset
        {
            uint64_t Offset; // Note : absolute mip offset
        } RenderOffset[GMM_ULT_MAX_MIPMAP];

        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = ResWidth;              // 1 pixel larger than 1 tile width
        gmmParams.BaseHeight  = gmmParams.BaseWidth64; // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;
        gmmParams.MaxLod      = MaxLod;
        gmmParams.ArraySize   = MaxArraySize;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign);

        //------------------------------|
        //                              |
        //          LOD0                |
        //                              |
        //                              |
        //------------------------------|
        //    LOD1     |    LOD2  |
        //             |----------|
        //             | LOD3 |
        //-------------| LOD4 .. so on

        //Mip 0
        //Mip 0 decides the pitch of the entire resource.
        const uint32_t AlignedWidthMip0  = GMM_ULT_ALIGN(ResWidth, HAlign); // HAlign width in pixel
        const uint32_t AlignedHeightMip0 = GMM_ULT_ALIGN(ResWidth, VAlign);
        uint32_t       ExpectedPitch     = GMM_ULT_ALIGN(AlignedWidthMip0 * GetBppValue(bpp), TileSize[X]); // Align AlignedWidthMip0 to 128 bytes
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[X]); // Pitch/TileY-Width

        // Mip0 should be at offset 0 and tile aligned
        GMM_REQ_OFFSET_INFO OffsetInfo = {};
        OffsetInfo.ReqRender           = 1;
        OffsetInfo.MipLevel            = 0; //Mip 0
        ResourceInfo->GetOffset(OffsetInfo);
        EXPECT_EQ(0, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        //cache Mip 0 offset
        RenderOffset[0].Offset = 0;

        // Mip 1 should be under mip 0
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 1; //Mip 1
        ResourceInfo->GetOffset(OffsetInfo);

        EXPECT_EQ(GMM_ULT_ALIGN_FLOOR(AlignedHeightMip0, TileSize[Y]) * ExpectedPitch, // Render offset is tile's base address on which mip begins
                  OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);                                 // Aligns with Mip0 at X = 0
        EXPECT_EQ((AlignedHeightMip0) % TileSize[Y], OffsetInfo.Render.YOffset); // AlignedHeightMip0 % TileY-Height
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        //cache Mip 1 offset
        RenderOffset[1].Offset = AlignedHeightMip0 * ExpectedPitch; //Absolute base

        const uint32_t AlignedWidthMip1  = GMM_ULT_ALIGN(ResWidth >> 1, HAlign); // Align width in pixel to HAlign
        const uint32_t AlignedHeightMip1 = GMM_ULT_ALIGN(ResWidth >> 1, VAlign);

        uint32_t HeightOfMip;
        uint32_t HeightLinesLevel2 = 0;

        // Mips 2-9 should be stacked on the right of Mip1 as shown in figure above.
        for(int i = 2; i <= MaxLod; i++)
        {
            OffsetInfo           = {};
            OffsetInfo.ReqRender = 1;
            OffsetInfo.MipLevel  = i;
            ResourceInfo->GetOffset(OffsetInfo);

            HeightOfMip = GMM_ULT_ALIGN(ResWidth >> i, VAlign);

            EXPECT_EQ(GMM_ULT_ALIGN_FLOOR(AlignedHeightMip0 + HeightLinesLevel2, TileSize[Y]) * ExpectedPitch + // Render offset is tile's base address on which mip begins
                      (AlignedWidthMip1 * GetBppValue(bpp) / TileSize[X]) * PAGE_SIZE,
                      OffsetInfo.Render.Offset64);

            EXPECT_EQ((AlignedWidthMip1 * GetBppValue(bpp)) % TileSize[X], OffsetInfo.Render.XOffset);   // Aligns with Mip0 at X = 0
            EXPECT_EQ((AlignedHeightMip0 + HeightLinesLevel2) % TileSize[Y], OffsetInfo.Render.YOffset); // AlignedHeightMip0 % TileY-Height
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

            //cache Mip i'th offset
            RenderOffset[i].Offset = (AlignedHeightMip0 + HeightLinesLevel2) * ExpectedPitch +
                                     (AlignedWidthMip1 * GetBppValue(bpp));

            HeightLinesLevel2 += HeightOfMip;
        }

        uint32_t Max2DHeight    = GMM_ULT_MAX(AlignedHeightMip1, HeightLinesLevel2);
        uint32_t ExpectedQPitch = GFX_ALIGN_NP2(AlignedHeightMip0 + Max2DHeight, VAlign);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 ExpectedPitch *
                                 GMM_ULT_ALIGN(MaxArraySize * __GMM_MAX_CUBE_FACE * ExpectedQPitch, TileSize[Y]));

        // Verify each array element's  Mip offset, Cube face offset etc.
        for(uint32_t ArrayIndex = 0; ArrayIndex < __GMM_MAX_CUBE_FACE; ArrayIndex++)
        {
            for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
            {
                GMM_REQ_OFFSET_INFO OffsetInfo = {};
                OffsetInfo.ReqRender           = 1;
                OffsetInfo.ArrayIndex          = ArrayIndex;
                OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
                ResourceInfo->GetOffset(OffsetInfo);

                //Verify cube face offsets
                EXPECT_EQ(GMM_ULT_ALIGN_FLOOR(((6 * ArrayIndex) + CubeFaceIndex) * ExpectedQPitch, TileSize[Y]) * ExpectedPitch,
                          OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
                EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
                EXPECT_EQ((((6 * ArrayIndex) + CubeFaceIndex) * ExpectedQPitch) % TileSize[Y],
                          OffsetInfo.Render.YOffset);    // Y Offset should be (CubeFaceIndex * QPitch) % TileHeight
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0

                uint32_t CubeFaceBaseOffset = ((6 * ArrayIndex) + CubeFaceIndex) * (ExpectedQPitch * ExpectedPitch);

                //Verify mip offsets in each cube face
                for(uint32_t Lod = 0; Lod <= MaxLod; Lod++)
                {
                    OffsetInfo.MipLevel = Lod;
                    ResourceInfo->GetOffset(OffsetInfo);

                    uint32_t MipOffset = CubeFaceBaseOffset + RenderOffset[Lod].Offset;

                    uint32_t OffsetX            = MipOffset % ExpectedPitch;
                    uint32_t TileAlignedOffsetX = GMM_ULT_ALIGN_FLOOR(OffsetX, TileSize[X]);
                    OffsetX -= TileAlignedOffsetX;

                    uint32_t OffsetY            = MipOffset / ExpectedPitch;
                    uint32_t TileAlignedOffsetY = GMM_ULT_ALIGN_FLOOR(OffsetY, TileSize[Y]);
                    OffsetY -= TileAlignedOffsetY;

                    uint32_t RenderAlignOffset =
                    TileAlignedOffsetY * ExpectedPitch +
                    (TileAlignedOffsetX / TileSize[X]) * PAGE_SIZE;


                    EXPECT_EQ(RenderAlignOffset, OffsetInfo.Render.Offset64); // Render offset is tile's base address on which cube face begins.
                    EXPECT_EQ(OffsetX, OffsetInfo.Render.XOffset);
                    EXPECT_EQ(OffsetY, OffsetInfo.Render.YOffset); // Y Offset should be (CubeFaceIndex * QPitch) % TileHeight
                    EXPECT_EQ(0, OffsetInfo.Render.ZOffset);       // Z offset N/A should be 0
                }
            }
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Cube TileYs Mipped Array Resource
TEST_F(CTestGen9Resource, TestCubeTileYsMippedResourceArray)
{
    enum Coords
    {
        X = 0,
        Y = 1
    };

    const uint32_t HAlign[TEST_BPP_MAX] = {256, 256, 128, 128, 64};
    const uint32_t VAlign[TEST_BPP_MAX] = {256, 128, 128, 64, 64};

    const uint32_t TileSize[TEST_BPP_MAX][2] = {{256, 256},
                                                {512, 128},
                                                {512, 128},
                                                {1024, 64},
                                                {1024, 64}};

    const uint32_t MaxMipTailSize[TEST_BPP_MAX][2] = {{128, 256},
                                                      {128, 128},
                                                      {64, 128},
                                                      {64, 64},
                                                      {32, 64}};

    const uint32_t MipTailSlotSize[GMM_ULT_MAX_MIPTAIL_SLOTS] = {
    GMM_KBYTE(32),
    GMM_KBYTE(16),
    GMM_KBYTE(8),
    GMM_KBYTE(4),
    GMM_KBYTE(2),
    GMM_KBYTE(1),
    GMM_BYTES(768),
    GMM_BYTES(512),
    GMM_BYTES(448),
    GMM_BYTES(384),
    GMM_BYTES(320),
    GMM_BYTES(256),
    GMM_BYTES(192),
    GMM_BYTES(128),
    GMM_BYTES(64)};

    const TEST_MIPTAIL_SLOT_OFFSET MipTailSlotOffsets[GMM_ULT_MAX_MIPTAIL_SLOTS][TEST_BPP_MAX] =
    {
    /*  |       8 bpe    |      16 bpe   |     32 bpe    |     64 bpe   |     128 bpe    | */
    {{128, 0, 0}, {128, 0, 0}, {64, 0, 0}, {64, 0, 0}, {32, 0, 0}},
    {{0, 128, 0}, {0, 64, 0}, {0, 64, 0}, {0, 32, 0}, {0, 32, 0}},
    {{64, 0, 0}, {64, 0, 0}, {32, 0, 0}, {32, 0, 0}, {16, 0, 0}},
    {{0, 64, 0}, {0, 32, 0}, {0, 32, 0}, {0, 16, 0}, {0, 16, 0}},
    {{32, 0, 0}, {32, 0, 0}, {16, 0, 0}, {16, 0, 0}, {8, 0, 0}},
    {{16, 32, 0}, {16, 16, 0}, {8, 16, 0}, {8, 8, 0}, {4, 8, 0}},
    {{0, 48, 0}, {0, 24, 0}, {0, 24, 0}, {0, 12, 0}, {0, 12, 0}},
    {{0, 32, 0}, {0, 16, 0}, {0, 16, 0}, {0, 8, 0}, {0, 8, 0}},
    {{16, 16, 0}, {16, 8, 0}, {8, 8, 0}, {8, 4, 0}, {4, 4, 0}},
    {{16, 0, 0}, {16, 0, 0}, {8, 0, 0}, {8, 0, 0}, {4, 0, 0}},
    {{0, 16, 0}, {0, 8, 0}, {0, 8, 0}, {0, 4, 0}, {0, 4, 0}},
    {{0, 12, 0}, {8, 4, 0}, {4, 4, 0}, {6, 0, 0}, {3, 0, 0}},
    {{0, 8, 0}, {8, 0, 0}, {4, 0, 0}, {4, 0, 0}, {2, 0, 0}},
    {{0, 4, 0}, {0, 4, 0}, {0, 4, 0}, {2, 0, 0}, {1, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};


    GMM_RESCREATE_PARAMS gmmParams = {};
    gmmParams.Type                 = RESOURCE_CUBE;
    gmmParams.NoGfxMemory          = 1;
    gmmParams.Flags.Info.TiledY    = 1;
    gmmParams.Flags.Info.TiledYs   = 1;
    gmmParams.Flags.Gpu.Texture    = 1;

    // Allocate CUBE surface
    for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
    {
        const uint32_t ResWidth     = 0x1001;
        const uint32_t MaxLod       = 0x9;
        const uint32_t MaxArraySize = 0x10;

        struct //Cache the value for verifying array elements/Cube face offset/Mip Offset
        {
            uint64_t Offset; // Note : absolute mip offset
        } RenderOffset[GMM_ULT_MAX_MIPMAP];

        TEST_BPP bpp          = static_cast<TEST_BPP>(i);
        gmmParams.Format      = SetResourceFormat(bpp);
        gmmParams.BaseWidth64 = ResWidth;
        gmmParams.BaseHeight  = gmmParams.BaseWidth64; // Heigth must be equal to width.
        gmmParams.Depth       = 0x1;
        gmmParams.MaxLod      = MaxLod;
        gmmParams.ArraySize   = MaxArraySize;

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        VerifyResourceHAlign<true>(ResourceInfo, HAlign[i]);
        VerifyResourceVAlign<true>(ResourceInfo, VAlign[i]);

        //------------------------------|
        //                              |
        //          LOD0                |
        //                              |
        //                              |
        //------------------------------|
        //    LOD1     |    LOD2  |
        //             |----------|
        //             | LOD3 |
        //-------------| LOD4 .. so on

        //Mip 0
        //Mip 0 decides the pitch of the entire resource.
        const uint32_t AlignedWidthMip0  = GMM_ULT_ALIGN(ResWidth, HAlign[i]); // HAlign width in pixel
        const uint32_t AlignedHeightMip0 = GMM_ULT_ALIGN(ResWidth, VAlign[i]);
        uint32_t       ExpectedPitch     = GMM_ULT_ALIGN(AlignedWidthMip0 * GetBppValue(bpp), TileSize[i][X]); // Align AlignedWidthMip0 to Tile width
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[i][X]); // Pitch/Tile-Width

        // Mip0 should be at offset 0 and tile aligned
        GMM_REQ_OFFSET_INFO OffsetInfo = {};
        OffsetInfo.ReqRender           = 1;
        OffsetInfo.MipLevel            = 0; //Mip 0
        ResourceInfo->GetOffset(OffsetInfo);
        EXPECT_EQ(0, OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset);
        EXPECT_EQ(0, OffsetInfo.Render.YOffset);
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        //cache Mip 0 offset
        RenderOffset[0].Offset = 0;

        // Mip 1 should be under mip 0
        OffsetInfo           = {};
        OffsetInfo.ReqRender = 1;
        OffsetInfo.MipLevel  = 1; //Mip 1
        ResourceInfo->GetOffset(OffsetInfo);

        EXPECT_EQ(AlignedHeightMip0 * ExpectedPitch, // Render offset is tile's base address on which mip begins
                  OffsetInfo.Render.Offset64);
        EXPECT_EQ(0, OffsetInfo.Render.XOffset); // Mips are always tile aligned
        EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Mips are always tile aligned
        EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

        //cache Mip 1 offset
        RenderOffset[1].Offset = AlignedHeightMip0 * ExpectedPitch; //Absolute base

        const uint32_t AlignedWidthMip1  = GMM_ULT_ALIGN(ResWidth >> 1, HAlign[i]); // Align width in pixel to HAlign
        const uint32_t AlignedHeightMip1 = GMM_ULT_ALIGN(ResWidth >> 1, VAlign[i]);

        // Determine MipTail start LOD
        uint32_t ExpectedMipTailStartLod = 0;
        {
            uint32_t MipWidth, MipHeight;
            MipWidth = MipHeight = ResWidth;
            while((ExpectedMipTailStartLod < MaxLod) &&
                  !((MipWidth <= MaxMipTailSize[i][X]) &&
                    (MipHeight <= MaxMipTailSize[i][Y])))
            {
                ExpectedMipTailStartLod++;

                MipWidth  = MipWidth >> 1;
                MipHeight = MipHeight >> 1;
            }
        }

        VerifyResourceMipTailStartLod<true>(ResourceInfo, ExpectedMipTailStartLod);

        uint32_t HeightOfMip;
        uint32_t HeightLinesLevel2 = 0;

        // Mips[2,MipTailStartLod - 1] should be stacked on the right of Mip1 as shown in figure above.
        for(int Lod = 2; Lod < ExpectedMipTailStartLod; Lod++)
        {
            OffsetInfo           = {};
            OffsetInfo.ReqRender = 1;
            OffsetInfo.MipLevel  = Lod;
            ResourceInfo->GetOffset(OffsetInfo);

            HeightOfMip = GMM_ULT_ALIGN(ResWidth >> Lod, VAlign[i]);

            EXPECT_EQ((AlignedHeightMip0 + HeightLinesLevel2) * ExpectedPitch + // Render offset is tile's base address on which mip begins
                      (AlignedWidthMip1 * GetBppValue(bpp) / TileSize[i][X]) * GMM_KBYTE(64),
                      OffsetInfo.Render.Offset64);

            EXPECT_EQ(0, OffsetInfo.Render.XOffset); // Mip lies on Tile boundary
            EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Mip lies on Tile boundary
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

            //cache Mip i'th offset
            RenderOffset[Lod].Offset = (AlignedHeightMip0 + HeightLinesLevel2) * ExpectedPitch +
                                       (AlignedWidthMip1 * GetBppValue(bpp));

            HeightLinesLevel2 += HeightOfMip;
        }

        // Mips[MipTailStartLod, MaxLod] will be on the mip tail.
        for(int Lod = ExpectedMipTailStartLod; Lod <= MaxLod; Lod++)
        {
            OffsetInfo           = {};
            OffsetInfo.ReqRender = 1;
            OffsetInfo.MipLevel  = Lod;
            ResourceInfo->GetOffset(OffsetInfo);

            uint64_t ExpectedRenderAlignOffset =
            (AlignedHeightMip0 + HeightLinesLevel2) * (uint64_t)ExpectedPitch + // Render offset is tile's base address on which mip begins
            (AlignedWidthMip1 * GetBppValue(bpp) / TileSize[i][X]) * GMM_KBYTE(64);

            EXPECT_EQ(ExpectedRenderAlignOffset, OffsetInfo.Render.Offset64);
            EXPECT_EQ(MipTailSlotOffsets[Lod - ExpectedMipTailStartLod][i].X * GetBppValue(bpp), OffsetInfo.Render.XOffset);
            EXPECT_EQ(MipTailSlotOffsets[Lod - ExpectedMipTailStartLod][i].Y, OffsetInfo.Render.YOffset);
            EXPECT_EQ(0, OffsetInfo.Render.ZOffset);

            RenderOffset[Lod].Offset = ExpectedRenderAlignOffset + MipTailSlotSize[Lod - ExpectedMipTailStartLod];
        }

        if(ExpectedMipTailStartLod) // Include mip tail height in the Level 2 height
        {
            HeightLinesLevel2 += TileSize[i][Y];
        }

        uint32_t Max2DHeight    = GMM_ULT_MAX(AlignedHeightMip1, HeightLinesLevel2);
        uint32_t ExpectedQPitch = GFX_ALIGN_NP2(AlignedHeightMip0 + Max2DHeight, VAlign[i]);
        VerifyResourceQPitch<true>(ResourceInfo, ExpectedQPitch); // Each face should be Valigned-BaseHeight rows apart

        VerifyResourceSize<true>(ResourceInfo, // PitchInBytes * Rows where Rows = __GMM_MAX_CUBE_FACE x QPitch, then aligned to tile boundary
                                 (uint64_t)ExpectedPitch * MaxArraySize * __GMM_MAX_CUBE_FACE * ExpectedQPitch);

        // Verify each array element's  Mip offset, Cube face offset etc.
        for(uint32_t ArrayIndex = 0; ArrayIndex < MaxArraySize; ArrayIndex++)
        {
            for(uint32_t CubeFaceIndex = 0; CubeFaceIndex < __GMM_MAX_CUBE_FACE; CubeFaceIndex++)
            {
                GMM_REQ_OFFSET_INFO OffsetInfo = {};
                OffsetInfo.ReqRender           = 1;
                OffsetInfo.ArrayIndex          = ArrayIndex;
                OffsetInfo.CubeFace            = static_cast<GMM_CUBE_FACE_ENUM>(CubeFaceIndex);
                ResourceInfo->GetOffset(OffsetInfo);

                //Verify cube face offsets
                EXPECT_EQ((((6 * ArrayIndex) + CubeFaceIndex) * ExpectedQPitch) * (uint64_t)ExpectedPitch,
                          OffsetInfo.Render.Offset64);   // Render offset is tile's base address on which cube face begins.
                EXPECT_EQ(0, OffsetInfo.Render.XOffset); // X Offset should be 0
                EXPECT_EQ(0, OffsetInfo.Render.YOffset); // Y Offset should be 0
                EXPECT_EQ(0, OffsetInfo.Render.ZOffset); // Z offset N/A should be 0

                uint64_t CubeFaceBaseOffset = ((6 * ArrayIndex) + CubeFaceIndex) * (ExpectedQPitch * (uint64_t)ExpectedPitch);

                //Verify mip offsets in each cube face
                // Mips[0,MipTailStartLod - 1]
                for(int Lod = 0; Lod < ExpectedMipTailStartLod; Lod++)
                {
                    OffsetInfo.MipLevel = Lod;
                    ResourceInfo->GetOffset(OffsetInfo);

                    uint64_t MipOffset = CubeFaceBaseOffset + RenderOffset[Lod].Offset;

                    uint32_t TileAlignedOffsetX = MipOffset % ExpectedPitch;
                    uint32_t TileAlignedOffsetY = MipOffset / ExpectedPitch;

                    uint64_t ExpectedRenderAlignOffset =
                    TileAlignedOffsetY * (uint64_t)ExpectedPitch +
                    (TileAlignedOffsetX / TileSize[i][X]) * GMM_KBYTE(64);

                    EXPECT_EQ(ExpectedRenderAlignOffset, OffsetInfo.Render.Offset64); // Render offset is tile's base address on which cube face begins.
                    EXPECT_EQ(0, OffsetInfo.Render.XOffset);                          // Mip lies on Tile boundary
                    EXPECT_EQ(0, OffsetInfo.Render.YOffset);                          // Mip lies on Tile boundary
                    EXPECT_EQ(0, OffsetInfo.Render.ZOffset);                          // Z offset N/A should be 0
                }

                // Mips[MipTailStartLod, MaxLod] will be on the mip tail.
                for(int Lod = ExpectedMipTailStartLod; Lod <= MaxLod; Lod++)
                {
                    OffsetInfo.MipLevel = Lod;
                    ResourceInfo->GetOffset(OffsetInfo);

                    uint64_t MipOffset = CubeFaceBaseOffset + RenderOffset[Lod].Offset;

                    uint64_t ExpectedRenderAlignOffset = GMM_ULT_ALIGN_FLOOR(MipOffset, GMM_KBYTE(64));

                    EXPECT_EQ(ExpectedRenderAlignOffset, OffsetInfo.Render.Offset64);
                    EXPECT_EQ(MipTailSlotOffsets[Lod - ExpectedMipTailStartLod][i].X * GetBppValue(bpp), OffsetInfo.Render.XOffset);
                    EXPECT_EQ(MipTailSlotOffsets[Lod - ExpectedMipTailStartLod][i].Y, OffsetInfo.Render.YOffset);
                    EXPECT_EQ(0, OffsetInfo.Render.ZOffset);
                }
            }
        }

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for Plannar 2D Resource - Tiling TileYs
TEST_F(CTestGen9Resource, TestPlanar2DTileYs)
{
}

/// @brief ULT for Plannar 2D Resource - Tiling TileYf
TEST_F(CTestGen9Resource, TestPlanar2DTileYf)
{
}

int BuildInputIterator(std::vector<std::tuple<int, int, int, bool, int, int>> &List, int maxTestDimension, int TestArray, bool XEHPPlus)
{
    for(uint32_t i = TEST_LINEAR; i < TEST_TILE_MAX; i++)
	{
        if(XEHPPlus)
        {
            if(i >= TEST_TILEX && i <= TEST_TILEY_MAX)
                continue;
        }
        else
        {
            if(i > TEST_TILEY_MAX)
                break;
        }
			
        for(uint32_t j = TEST_BPP_8; j < TEST_BPP_MAX; j++)
            for(uint32_t k = TEST_RESOURCE_1D; k < TEST_RESOURCE_MAX; k++)
                for(uint32_t l = 0; l < maxTestDimension; l++)
                    for(uint32_t m = 0; m < TestArray; m++)
                    {
                        List.emplace_back(std::make_tuple(i, j, k, true, l, m));
                        List.emplace_back(std::make_tuple(i, j, k, false, l, m));
                    }
	}

    return List.size();
}

/// @brief ULT for MSAA Resource
TEST_F(CTestGen9Resource, TestMSAA)
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

    uint32_t HAlign = 0, VAlign = 0, TileDimX = 0, TileDimY = 0;
    uint32_t MCSHAlign = 0, MCSVAlign = 0, TileSize = 0;
    uint32_t ExpectedMCSBpp;
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
        if(ResType != TEST_RESOURCE_2D || Tiling >= TEST_TILEYF         //No 1D/3D/Cube. Supported 2D mip-maps/array
           || (!IsRT && (Tiling == TEST_TILEX ||
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

int BuildInputIterator(std::vector<tuple<int, int, int, int, int>> &List, int maxTestDimension, int TestArraySize)
{
    for(uint32_t i = TEST_LINEAR; i < TEST_TILE_MAX; i++)
        for(uint32_t j = TEST_BPP_8; j < TEST_BPP_MAX; j++)
            for(uint32_t k = TEST_RESOURCE_1D; k < TEST_RESOURCE_MAX; k++)
                for(uint32_t l = 0; l < maxTestDimension; l++)
                    for(uint32_t m = 0; m < TestArraySize; m++)
                    {
                        List.emplace_back(make_tuple(i, j, k, l, m));
                    }

    return List.size();
}

/// @brief ULT for Color control Resource (non-MSAA compression)
TEST_F(CTestGen9Resource, TestCCS)
{
    const uint32_t CCSTileSize[1][2] = {128, 32}; //CCS is TileY

    const uint32_t TestDimensions[4][3] = {
    //Input dimensions in #Tiles
    {0, 0, 0}, //1x1x1
    {1, 0, 0}, //2 Tilesx1x1
    {1, 1, 0}, //2 Tilesx 2x1
    {1, 1, 1}, //2 Tilesx 2x2
    };
    uint32_t TestArraySize[2] = {1, 9};

    uint32_t RTHAlign = 128, RTVAlign = 64; //Gen9 CCS's RT should be aligned to 128x64

    uint32_t TileDimX, TileDimY, TileDimZ, WidthDivisor, HeightDivisor, TileSize;
    uint32_t ExpectedMCSBpp;
    std::vector<tuple<int, int, int, int, int>> List; //TEST_TILE_TYPE, TEST_BPP, TEST_RESOURCE_TYPE, TestDimension index, TestArraySize index
    auto Size = BuildInputIterator(List, 4, 2);       // Send size of TestDimensions, TestArraySize

    for(auto element : List)
    {
        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Flags.Info           = {0};

        TEST_TILE_TYPE     Tiling     = (TEST_TILE_TYPE)std::get<0>(element);
        TEST_BPP           Bpp        = (TEST_BPP)std::get<1>(element);
        TEST_RESOURCE_TYPE ResType    = (TEST_RESOURCE_TYPE)std::get<2>(element);
        int                TestDimIdx = std::get<3>(element); //index into TestDimensions array
        int                ArrayIdx   = std::get<4>(element); //index into TestArraySize
        TileSize                      = (Tiling == TEST_TILEYS) ? GMM_KBYTE(64) : GMM_KBYTE(4);

        //Discard if un-supported Tiling/Res_type/bpp for this test
        if(Tiling < TEST_TILEY || !(Bpp == TEST_BPP_32 ||                         //Gen8: NO TileYs/Yf, CCS not supported for !TileY/Yf/Ys/X - gen8 support tilex
                                    Bpp == TEST_BPP_64 || Bpp == TEST_BPP_128) || //LRTC not supported on <32bpp
           ResType == TEST_RESOURCE_1D ||
           ResType == TEST_RESOURCE_BUFFER) //non-MSAA CCS, 1D is showing issues CCS becoming linear (Buffer is linear)
            continue;

        SetTileFlag(gmmParams, Tiling);
        SetResType(gmmParams, ResType);
        SetResGpuFlags(gmmParams, true); //Depth not Supported w/ CCS. Depth only has HiZ, and IMS
        SetResArraySize(gmmParams, TestArraySize[ArrayIdx]);

        gmmParams.NoGfxMemory = 1;
        gmmParams.Format      = SetResourceFormat(Bpp);

        GetAlignmentAndTileDimensionsForCCS(Bpp, Tiling, ResType,
                                            TileDimX, TileDimY, TileDimZ, WidthDivisor, HeightDivisor);

        gmmParams.BaseWidth64 = TestDimensions[TestDimIdx][0] * TileDimX + 0x1;
        gmmParams.BaseHeight  = (gmmParams.Type == RESOURCE_CUBE) ? gmmParams.BaseWidth64 :
                                                                   TestDimensions[TestDimIdx][1] * TileDimY + 0x1;
        gmmParams.Depth           = (gmmParams.Type == RESOURCE_3D) ? TestDimensions[TestDimIdx][2] * TileDimZ + 0x1 : 0x1;
        gmmParams.MSAA.NumSamples = 1;
        gmmParams.Flags.Gpu.CCS   = 0;

        //RT surface
        GMM_RESOURCE_INFO *RTResourceInfo;
        RTResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        //No CCS surface if RT creation failed
        if(RTResourceInfo)
        {
            gmmParams.Flags.Gpu.CCS = 1;
            GMM_RESOURCE_INFO *CCSResourceInfo;
            CCSResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            VerifyResourceHAlign<true>(CCSResourceInfo, RTHAlign); //Check if RT for CCS creation was aligned to CCS's RT alignment
            VerifyResourceVAlign<true>(CCSResourceInfo, RTVAlign);
            uint32_t ExpectedPitch = GMM_ULT_ALIGN((uint32_t)(GMM_ULT_ALIGN(gmmParams.BaseWidth64, RTHAlign) / WidthDivisor), CCSTileSize[0][0]);
            VerifyResourcePitch<true>(CCSResourceInfo, ExpectedPitch);
            VerifyResourcePitchInTiles<true>(CCSResourceInfo, ExpectedPitch / CCSTileSize[0][0]); // 1 tileY wide

            uint32_t ExpectedQPitch = GMM_ULT_ALIGN(gmmParams.BaseHeight, RTVAlign); // / HeightDivisor;
            if(gmmParams.ArraySize > 1 || gmmParams.Type == RESOURCE_CUBE || gmmParams.Type == RESOURCE_3D)
            {
                uint32_t DepthSlice = gmmParams.Depth;
                //if (gmmParams.Type == RESOURCE_3D && DepthSlice > 1)
                //{
                //    ExpectedQPitch *= DepthSlice;
                //}
                //else { // Should 3D surface Aux QPitch be distance between R-slices or not
                // If it must be R-slice distance, compute 3D QPitch, remove depthslice from arraysize
                VerifyResourceQPitch<true>(CCSResourceInfo, ExpectedQPitch); //verify false, else QPitch given for RT-size not CCS
                //}
                gmmParams.ArraySize *= (gmmParams.Type == RESOURCE_3D) ? DepthSlice : //3D R-slices
                                       (gmmParams.Type == RESOURCE_CUBE) ? 6 : 1;     //cube faces treated as array slices
            }

            ExpectedQPitch /= HeightDivisor;
            uint32_t ExpectedHeight = GMM_ULT_ALIGN(ExpectedQPitch * gmmParams.ArraySize, CCSTileSize[0][1]);
            VerifyResourceSize<true>(CCSResourceInfo, GMM_ULT_ALIGN(ExpectedPitch * ExpectedHeight, GMM_KBYTE(4)));

            pGmmULTClientContext->DestroyResInfoObject(CCSResourceInfo);
        } //CCS

        pGmmULTClientContext->DestroyResInfoObject(RTResourceInfo);
    } //Iterate through all input tuples

    // Mip-mapped case
}

/// @brief ULT for MMC Resource
TEST_F(CTestGen9Resource, TestMMC)
{
    const TEST_TILE_TYPE TileTypeSupported[3] = {TEST_TILEY, TEST_TILEYS, TEST_TILEYF};

    const uint32_t TileSize[3][TEST_BPP_MAX][2] = {
    {{128, 32}, {128, 32}, {128, 32}, {128, 32}, {128, 32}},      // TileY
    {{256, 256}, {512, 128}, {512, 128}, {1024, 64}, {1024, 64}}, // TileYs
    {{64, 64}, {128, 32}, {128, 32}, {256, 16}, {256, 16}}};      // TileYf

    // Normal 2D surface
    for(uint32_t Tile : TileTypeSupported)
    {
        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.Flags.Gpu.MMC        = 1;
        gmmParams.BaseWidth64          = 0x100;
        gmmParams.BaseHeight           = 0x50;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, static_cast<TEST_TILE_TYPE>(Tile));

        for(uint32_t i = 0; i < TEST_BPP_MAX; i++)
        {
            TEST_BPP bpp     = static_cast<TEST_BPP>(i);
            gmmParams.Format = SetResourceFormat(bpp);

            GMM_RESOURCE_INFO *ResourceInfo;
            ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

            uint32_t ExpectedPitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[Tile - TEST_TILEY][i][0] / GetBppValue(bpp));
            ExpectedPitch *= GetBppValue(bpp);
            ExpectedPitch += TileSize[Tile - TEST_TILEY][i][0]; // MMC will have extra tile on the right
            VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
            VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[Tile - TEST_TILEY][i][0]);

            uint32_t Size = GMM_ULT_ALIGN(gmmParams.BaseHeight, TileSize[Tile - TEST_TILEY][i][1]) * ExpectedPitch;
            VerifyResourceSize<true>(ResourceInfo, GMM_ULT_ALIGN(Size, GMM_KBYTE(4)));

            VerifyResourceHAlign<false>(ResourceInfo, 0); // Tested elsewhere
            VerifyResourceVAlign<false>(ResourceInfo, 0); // Tested elsewhere
            VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-mipped surface

            pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
        }
    }

    // Planar 2D surface
    {
        GMM_RESCREATE_PARAMS gmmParams = {};
        gmmParams.Type                 = RESOURCE_2D;
        gmmParams.NoGfxMemory          = 1;
        gmmParams.Flags.Gpu.Texture    = 1;
        gmmParams.Flags.Gpu.MMC        = 1;
        gmmParams.BaseWidth64          = 0x100;
        gmmParams.BaseHeight           = 0x50;
        gmmParams.Depth                = 0x1;
        SetTileFlag(gmmParams, TEST_TILEY); // TileY only
        gmmParams.Format = GMM_FORMAT_NV12; // 8bpp

        GMM_RESOURCE_INFO *ResourceInfo;
        ResourceInfo = pGmmULTClientContext->CreateResInfoObject(&gmmParams);

        uint32_t ExpectedPitch = GMM_ULT_ALIGN(gmmParams.BaseWidth64, TileSize[0][0][0]);
        ExpectedPitch += TileSize[0][0][0]; // MMC will have extra tile on the right
        VerifyResourcePitch<true>(ResourceInfo, ExpectedPitch);
        VerifyResourcePitchInTiles<true>(ResourceInfo, ExpectedPitch / TileSize[0][0][0]);

        VerifyResourceSize<false>(ResourceInfo, 0);   // Tested elsewhere
        VerifyResourceHAlign<false>(ResourceInfo, 0); // Tested elsewhere
        VerifyResourceVAlign<false>(ResourceInfo, 0); // Tested elsewhere
        VerifyResourceQPitch<false>(ResourceInfo, 0); // N/A for non-mipped surface

        pGmmULTClientContext->DestroyResInfoObject(ResourceInfo);
    }
}

/// @brief ULT for StdSwizzle surfaces
TEST_F(CTestGen9Resource, TestStdSwizzle)
{
    // TODO: Test RedescribedPlanes, along with other StdSwizzle mappings
}
