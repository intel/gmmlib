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


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

// Set packing alignment
#pragma pack(push, 8)

extern const uint32_t __GmmMSAAConversion[5][2];
extern const uint32_t __GmmTileYConversionTable[5][2];

//
// Normally, GMM_RESOURCE_INFO should not contain any user mode pointers because
// if the resource is shared, then the pointer will be invalid in the other process.
// However, the pointers below are OK because pMappedTiles and pTiledPoolArray are
// associated with a TR surface, which is not shareable. AuxInfo.pTilePoolInfo is associated
// with tile pools; Tile Pools are shareable, but Aux tile pool aren't.
//
typedef struct TILE_POOL_INFO_REC
{
    uint64_t                          UsedTiles[4];       // Tiles being used in 1 tile pool
    GMM_GFX_ADDRESS                   StartingGfxAddress; // GfxAddress associated with the TilePool
} TILE_POOL_INFO;

typedef struct GMM_TILED_RESOURCE_INFO_REC
{
    GMM_GFX_ADDRESS                     TiledResourceGfxAddress; // used for tiled resources
    uint64_t                            pMappedTiles;            // tracks which tiles have been mapped
    uint64_t                            pTilePoolArray;          // list of tile pool allocation
                                                                 // handles (D3DKMT_HANDLE),
                                                                 // only used for tiled resources
    uint64_t                            pAuxTilePoolResArray;    // list of aux tile pool allocation resource, and handles

    uint32_t                            TilePoolArraySize;
    uint32_t                            AuxTilePoolArraySize;

    struct
    {
        uint64_t                        pTilePoolInfo;
        uint64_t                        PagingFenceValue;
        uint32_t                        TilePoolInfoTotalNumElements; // Number of elements in pTilePoolInfo array
        uint32_t                        TilePoolInfoFreeNumElements;  // Number of free tile pools
    } AuxInfo;
} GMM_TILED_RESOURCE_INFO;

typedef struct GMM_EXISTING_SYS_MEM_REC
{
    // 64bit kernel mode drivers must validate sizeof structs passed from
    // 32bit & 64bit user mode drivers. Store as 64bit to keep uniform size.
    uint64_t                        pExistingSysMem; //Original buffer address.
    uint64_t                        pVirtAddress;
    uint64_t                        pGfxAlignedVirtAddress;
#if(LHDM)
    D3DKMT_HANDLE                   hParentAllocation;
#endif
    GMM_GFX_SIZE_T                  Size;
    uint8_t                         IsGmmAllocated;
} GMM_EXISTING_SYS_MEM;
//===========================================================================
// typedef:
//      GMM_RESOURCE_INFO
//
// Description:
//     Struct describing the attributes and properties of a user mode resource
//
//----------------------------------------------------------------------------
#ifndef _WIN32
    #include "../Linux/GmmResourceInfoLin.h"
#endif

// Reset packing alignment to project default
#pragma pack(pop)

uint8_t     GMM_STDCALL GmmResValidateParams(GMM_RESOURCE_INFO *pResourceInfo);
void        GMM_STDCALL GmmResGetRestrictions(GMM_RESOURCE_INFO* pResourceInfo, __GMM_BUFFER_TYPE* pRestrictions);
uint8_t     __CanSupportStdTiling(GMM_TEXTURE_INFO Surface);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

