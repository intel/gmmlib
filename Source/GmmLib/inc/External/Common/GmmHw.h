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
#include "gfxmacro.h"

// eDRAM MMIO Cap Structure for SKL and above.
#define EDRAM_CAP_REG   0x120010

typedef union EDRAM_CAP_REC
{
    struct
    {
        uint32_t   Enable : 1;             // Bit [    0]
        uint32_t   TagBanks : 4;           // Bit [ 4: 1]
        uint32_t   Ways : 3;               // Bit [ 7: 5]
        uint32_t   Sets : 2;               // Bit [ 9: 8]
        uint32_t   Reserved1 : 6;          // Bit [15:10]
        uint32_t   CurrentEtagWays : 6;    // Bit [21:16]
        uint32_t   Reserved2 : 10;         // Bit [31:22]
    };

    uint32_t   Value;

} EDRAM_CAP, *PEDRAM_CAP;

// TR stands for Tiled Resource

/* TR logic is disabled as by default. This register needs to 
   be enabled via s/w to get TR translation active.
   It should be done before there is any activity on GT. 
   Once enabled, should not be written again. It will be reset 
   only by BUS reset. When this bit is 0, the whole TR logic 
   is bypassed.
*/
#define GMM_GEN9_TR_CHICKEN_BIT_VECTOR                 0x04DFC

// TR registers in the H/W Context
#define GMM_GEN9_TRTT_L3_POINTER                       0x04DE0
#define GMM_GEN9_TRTT_L3_GFXADDR_MASK                  0x0000FFFFFFFFF0000
#define GMM_GEN9_TRTT_NULL_TILE_REG                    0x04DE8
#define GMM_GEN9_TRTT_INVD_TILE_REG                    0x04DEC
#define GMM_GEN9_TRTT_VA_MASKDATA_REG                  0x04DF0
#define GMM_GEN9_TRTT_TABLE_CONTROL                    0x04DF4

// Values for setting TR registers
#define GMM_GEN9_TRTT_IN_GFX_VA_SPACE               0x2
#define GMM_GEN9_TRTT_ENABLE                        0x1
#define GMM_GEN9_TRTT_BYPASS_DISABLE                0x1

////////////////////// Tiled-Resources Translation Table definitions//////////////////////////////////////////
//===========================================================================
// typedef:
//        GMM_TRTTL3e
//
// Description:
//        Struct for H/W Tiled-Resource Translation-Table L3 entry
//--------------------------------------------------------------------------
typedef struct GMM_TRTTL3e_REC
{
    UINT64 Invalid   :  1;
    UINT64 Null      :  1;
    UINT64 Reserved0 : 10;
    UINT64 L2GfxAddr : 36;
    UINT64 Reserved1 : 16;
} GMM_TRTTL3e;
C_ASSERT(sizeof(GMM_TRTTL3e) == 8);

// Get the L2GfxAddr bit field as a full L2 graphics address
#define GMM_FULL_GFXADDR_FROM_TRTT_L3e_L2GFXADDR(L2GfxAddr) ((L2GfxAddr) << 12) 

// Set the L2GfxAddr bit field given a full L2 graphics address
#define GMM_TO_TRTT_L3e_L2GFXADDR(L2GfxAddress) ((L2GfxAddress) >> 12)

//===========================================================================
// typedef:
//        GMM_TRTTL2e
//
// Description:
//        Struct for H/W Tiled-Resource Translation-Table L2 entry
//--------------------------------------------------------------------------
typedef union GMM_TRTTL2e_REC
{
    struct
    {
        UINT64 Invalid   :  1;
        UINT64 Null      :  1;
        UINT64 Reserved0 : 10;
        UINT64 L1GfxAddr : 36;
        UINT64 Reserved1 : 16;
    };
    UINT64 Value;
} GMM_TRTTL2e;
C_ASSERT(sizeof(GMM_TRTTL2e) == 8);

// Get the L1GfxAddr bit field as a full L1 graphics address
#define GMM_FULL_GFXADDR_FROM_TRTT_L2e_L1GFXADDR(L1GfxAddr) ((L1GfxAddr) << 12) 

// Set the L1GfxAddr bit field given a full L1 graphics address
#define GMM_TO_TRTT_L2e_L1GFXADDR(L1GfxAddress) ((L1GfxAddress) >> 12)

typedef struct GMM_TRTTL1e_REC
{
    UINT GfxAddr47_16    : 32; // bits 47:16 of gfx address
} GMM_TRTTL1e;
C_ASSERT(sizeof(GMM_TRTTL1e) == 4);

// if L1/L2GfxAddress == GMM_NO_TABLE then
// the L1/L2 table hasn't been created
#define GMM_NO_TABLE ((GMM_GFX_ADDRESS)(-1L))

#define GMM_NULL_TILE_VALUE         0xFFFFFFFF
// Set to ( NULL_TILE_VALUE - 1) all FFF's except last (i.e. we're not using invalid detection, for now)
#define GMM_INVALID_TILE_VALUE      (GMM_NULL_TILE_VALUE - 1)

#define GMM_TRTT_L1e_SIZE (sizeof(uint32_t))
#define GMM_TRTT_L2e_SIZE (sizeof(GMM_TRTTL2e))
#define GMM_TRTT_L3e_SIZE (sizeof(GMM_TRTTL3e))

#define GMM_TRTT_L1_LOW_BIT  (16)
#define GMM_TRTT_L1_HIGH_BIT (25)
#define GMM_TRTT_L2_LOW_BIT  (26)
#define GMM_TRTT_L2_HIGH_BIT (34)
#define GMM_TRTT_L3_LOW_BIT  (35)
#define GMM_TRTT_L3_HIGH_BIT (43)

// #L1 entries, i.e. 1024
#define GMM_TRTT_L1_SIZE         (1 << (GMM_TRTT_L1_HIGH_BIT - GMM_TRTT_L1_LOW_BIT + 1))
#define GMM_TRTT_L1_SIZE_DWORD   (GFX_CEIL_DIV( GMM_TRTT_L1_SIZE, 32))

// #L2 entries, i.e. 512
#define GMM_TRTT_L2_SIZE         (1 << (GMM_TRTT_L2_HIGH_BIT - GMM_TRTT_L2_LOW_BIT + 1))
#define GMM_TRTT_L2_SIZE_DWORD   (GFX_CEIL_DIV( GMM_TRTT_L2_SIZE, 32))

// #L3 entries, i.e. 512
#define GMM_TRTT_L3_SIZE         (1 << (GMM_TRTT_L3_HIGH_BIT - GMM_TRTT_L3_LOW_BIT + 1))

#define GMM_TRTT_L1_ENTRY_IDX(GfxAddress)                                     \
    (((GfxAddress) & GFX_MASK_LARGE(GMM_TRTT_L1_LOW_BIT, GMM_TRTT_L1_HIGH_BIT)) >> \
     (UINT64)GMM_TRTT_L1_LOW_BIT)

#define GMM_TRTT_L2_ENTRY_IDX(GfxAddress)                                     \
    (((GfxAddress) & GFX_MASK_LARGE(GMM_TRTT_L2_LOW_BIT, GMM_TRTT_L2_HIGH_BIT)) >> \
    (UINT64)GMM_TRTT_L2_LOW_BIT)

#define GMM_TRTT_L3_ENTRY_IDX(GfxAddress)                                     \
    (((GfxAddress) & GFX_MASK_LARGE(GMM_TRTT_L3_LOW_BIT, GMM_TRTT_L3_HIGH_BIT)) >> \
    (UINT64)GMM_TRTT_L3_LOW_BIT)

// Base gfx address of the TR-VA space is end of GFX memory ([47:44] = 1111)
#define GMM_TRVA_BASE_ADDRESS (15ULL << 44)

// The TRTT tables are located one PML4 entry above TRVA
#define GMM_TRTT_TABLES_BASE_ADDRESS (GMM_TRVA_BASE_ADDRESS - GMM_ADDRESS_SPACE_PER_PML4E)

// The base gfx address of the internal heap, for tile-pools and TRTT tables, 
// is the base gfx address of the TRTT tables
#define GMM_TR_INTERNAL_ALLOCS_BASE_ADDRESS GMM_TRTT_TABLES_BASE_ADDRESS

// Mask to get the uppers bits of the base address which aren't used for page walk
#define GMM_TRVA_UNUSED_ADDRESS_BITS (GMM_TRVA_BASE_ADDRESS & (15ULL << 44))

// BEGIN - =================================================================
// MACROS
//
// Description: Macros for going from L3/L2/L1 table indexes to TR-VA gfx
//              address that walks those L3/L2/L1 tables.
//--------------------------------------------------------------------------

// Returns a graphics address that will walk the TRTT with {L3 entry = L3eIdx, L2eIdx = 0, L1eIdx = 0} 
#define GMM_GFXADDR_FOR_TRTT_L3_WALK(L3eIdx) ((GMM_GFX_ADDRESS)(GMM_TRVA_BASE_ADDRESS + (((UINT64)L3eIdx) << GMM_TRTT_L3_LOW_BIT)))

// Returns a graphics address that will walk the TRTT with {L3 entry = L3eIdx, L2 entry = L2eIdx, L1eIdx = 0} 
#define GMM_GFXADDR_FOR_TRTT_L2_WALK(L3eIdx, L2eIdx) (GMM_GFXADDR_L3(L3eIdx) + (((UINT64)L2eIdx) << GMM_TRTT_L2_LOW_BIT))

// Returns a graphics address that will walk the TRTT with {L3 entry = L3eIdx, L2 entry = L2eIdx, L1 entry = L1eIdx} 
#define GMM_GFXADDR_FOR_TRTT_L1_WALK(L3eIdx, L2eIdx, L1eIdx) (GMM_GFXADDR_L3(L3eIdx) + GMM_GFXADDR_L2(L2eIdx) + (((UINT64)L1eIdx) << GMM_TRTT_L1_LOW_BIT))

//--------------------------------------------------------------------------
//END - ====================================================================
//MACROS

////////////////////// Tiled-Resources Translation Table definitions end//////////////////////////////////////////
