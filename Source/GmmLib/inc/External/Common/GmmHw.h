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


#pragma once
#include "gfxmacro.h"



////////////////////// Auxiliary Translation Table definitions//////////////////////////////////////////
//===========================================================================
// typedef:
//        GMM_AUXTTL3e
//
// Description:
//        Struct for Auxiliary Translation-Table L3 entry
//--------------------------------------------------------------------------
typedef union GMM_AUXTTL3e_REC
{
    struct {
        uint64_t Valid     :  1;
        uint64_t Reserved0 : 14;
        uint64_t L2GfxAddr : 33;
        uint64_t Reserved1 : 16;
    };
    uint64_t Value;
} GMM_AUXTTL3e;
C_ASSERT(sizeof(GMM_AUXTTL3e) == 8);

// Get the L2GfxAddr bit field as a full L2 graphics address
#define GMM_FULL_GFXADDR_FROM_AUX_L3e_L2GFXADDR(L2GfxAddr) ((L2GfxAddr) << 16)

// Set the L2GfxAddr bit field given a full L2 graphics address
#define GMM_TO_AUX_L3e_L2GFXADDR(L2GfxAddress) ((L2GfxAddress) >> 16)

//===========================================================================
// typedef:
//        GMM_AUXTTL2e
//
// Description:
//        Struct for Auxiliary Translation-Table L2 entry
//--------------------------------------------------------------------------
typedef union GMM_AUXTTL2e_REC
{
    struct
    {
        uint64_t Valid     :  1;
        uint64_t Reserved0 : 12;
        uint64_t L1GfxAddr : 35;
        uint64_t Reserved1 : 16;
    };
    uint64_t Value;
} GMM_AUXTTL2e;
C_ASSERT(sizeof(GMM_AUXTTL2e) == 8);

// Get the L1GfxAddr bit field as a full L1 graphics address
#define GMM_FULL_GFXADDR_FROM_AUX_L2e_L1GFXADDR(L1GfxAddr) ((L1GfxAddr) << 16)

// Set the L1GfxAddr bit field given a full L1 graphics address
#define GMM_TO_AUX_L2e_L1GFXADDR(L1GfxAddress) ((L1GfxAddress) >> 16)

typedef union GMM_AUXTTL1e_REC
{
    struct
    {
        uint64_t Valid      :  1;
        uint64_t Mode       :  2;      //Compression ratio (128B compr ie 2:1 for RC, 256B compr ie 4:n compr for MC)
        uint64_t Lossy      :  1;      //Lossy Compression
        uint64_t Reserved0  :  2;
        uint64_t Reserved2  :  2;      //LSbs of 64B-aligned CCS chunk/cacheline address
        uint64_t GfxAddress : 40;      //256B-aligned CCS chunk address
        uint64_t Reserved1  :  4;
        uint64_t TileMode   :  2;      //Ys = 0, Y=1,  Reserved=(2-3)
        uint64_t Depth      :  3;      //Packed/Planar bit-depth for MC; Bpp for RC
        uint64_t LumaChroma :  1;      //Planar Y=0 or Cr=1
        uint64_t Format     :  6;      //Format encoding shared with Vivante/Internal CC/DEC units to recognize surafce formats
    };
    uint64_t Value;
} GMM_AUXTTL1e;
C_ASSERT(sizeof(GMM_AUXTTL1e) == 8);

#define GMM_NO_TABLE            ((GMM_GFX_ADDRESS)(-1L))                 //common

#define GMM_INVALID_AUX_ENTRY      ~__BIT(0)

#define GMM_AUX_L1e_SIZE        (sizeof(GMM_AUXTTL1e))
#define GMM_AUX_L2e_SIZE        (sizeof(GMM_AUXTTL2e))
#define GMM_AUX_L3e_SIZE        (sizeof(GMM_AUXTTL3e))

#define GMM_AUX_L1_LOW_BIT      (14)
#define GMM_AUX_L1_HIGH_BIT     (23)
#define GMM_AUX_L2_LOW_BIT      (24)
#define GMM_AUX_L2_HIGH_BIT     (35)
#define GMM_AUX_L3_LOW_BIT      (36)
#define GMM_AUX_L3_HIGH_BIT     (47)

//For perf, AuxTable granularity changed to 64K
#define WA16K                   (pGmmGlobalContext->GetWaTable().WaAuxTable16KGranular)

// #L1 entries, i.e. 1024; 16K-granular ie 4 consequtive pages share Aux-cacheline;
// HW only tracks the distinct entries;
// Handle WA where HW chicken bit forces 64K-granularity
#define GMM_AUX_L1_SIZE(pGmmGlobalContext)         ((1 << (GMM_AUX_L1_HIGH_BIT - GMM_AUX_L1_LOW_BIT + 1)) / (!(WA16K) ? 4 : 1))
#define GMM_AUX_L1_SIZE_DWORD(pGmmGlobalContext)   (GFX_CEIL_DIV(GMM_AUX_L1_SIZE(pGmmGlobalContext), 32))

// #L2 entries, i.e. 4096
#define GMM_AUX_L2_SIZE         (1 << (GMM_AUX_L2_HIGH_BIT - GMM_AUX_L2_LOW_BIT + 1))
#define GMM_AUX_L2_SIZE_DWORD   (GFX_CEIL_DIV(GMM_AUX_L2_SIZE, 32))

// #L3 entries, i.e. 4096
#define GMM_AUX_L3_SIZE         (1 << (GMM_AUX_L3_HIGH_BIT - GMM_AUX_L3_LOW_BIT + 1))

#define GMM_AUX_L1_ENTRY_IDX(GfxAddress,pGmmGlobalContext)                                         \
    ((((GfxAddress) & GFX_MASK_LARGE(GMM_AUX_L1_LOW_BIT, GMM_AUX_L1_HIGH_BIT)) >> \
     (uint64_t)GMM_AUX_L1_LOW_BIT) / (!(WA16K) ? 4 : 1))


#define GMM_AUX_L1_ENTRY_IDX_EXPORTED(GfxAddress,WA64KEx)                         \
    ((((GfxAddress) & GFX_MASK_LARGE(GMM_AUX_L1_LOW_BIT, GMM_AUX_L1_HIGH_BIT)) >> \
     (uint64_t)GMM_AUX_L1_LOW_BIT) / ((WA64KEx) ? 4 : 1))


#define GMM_AUX_L2_ENTRY_IDX(GfxAddress)                                     \
    (((GfxAddress) & GFX_MASK_LARGE(GMM_AUX_L2_LOW_BIT, GMM_AUX_L2_HIGH_BIT)) >> \
    (uint64_t)GMM_AUX_L2_LOW_BIT)

#define GMM_AUX_L3_ENTRY_IDX(GfxAddress)                                     \
    (((GfxAddress) & GFX_MASK_LARGE(GMM_AUX_L3_LOW_BIT, GMM_AUX_L3_HIGH_BIT)) >> \
    (uint64_t)GMM_AUX_L3_LOW_BIT)

////////////////////// Auxiliary Translation Table definitions end//////////////////////////////////////////
