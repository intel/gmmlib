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

#include "Internal/Common/GmmLibInc.h"
#include "External/Common/GmmCachePolicy.h"

//=============================================================================
//
// Function: __GmmGen9InitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmGen9CachePolicy::InitCachePolicy()
{
    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#if defined(GMM_DYNAMIC_MOCS_TABLE)
#define DEFINE_CACHE_ELEMENT(usage, llc, ellc, l3, age, i915) DEFINE_CP_ELEMENT(usage, llc, ellc, l3, 0, age, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#else
// i915 only supports three GEN9 MOCS entires:
//     MOCS[0]...LLC=0, ELLC=0, L3=0, AGE=0
//     MOCS[1]...<N/A for GmmLib Purposes>
//     MOCS[2]...LLC=1, ELLC=1, L3=1, AGE=3
#define DEFINE_CACHE_ELEMENT(usage, llc, ellc, l3, age, i915)                           \
    do                                                                                  \
    {                                                                                   \
        if((i915) == 0)                                                                 \
        {                                                                               \
            DEFINE_CP_ELEMENT(usage, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);\
        }                                                                               \
        else if((i915) == 2)                                                            \
        {                                                                               \
            DEFINE_CP_ELEMENT(usage, 1, 1, 1, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);\
        }                                                                               \
        else                                                                            \
        {                                                                               \
            GMM_ASSERTDPF(0, "Invalid i915 MOCS specified");                            \
        }                                                                               \
    } while(0) ////////////////////////////////////////////////////////////////
#endif
#include "GmmGen9CachePolicy.h"

#define TC_LLC (1)
#define TC_ELLC (0)
#define TC_LLC_ELLC (2)

#define LeCC_UNCACHEABLE (0x1)
#define LeCC_WB_CACHEABLE (0x3)

#define L3_UNCACHEABLE (0x1)
#define L3_WB_CACHEABLE (0x3)

#define DISABLE_SKIP_CACHING_CONTROL (0x0)
#define ENABLE_SKIP_CACHING_CONTROL (0x1)

    {
        uint32_t                      CurrentMaxIndex        = 0;
        GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTblElement = pGmmLibContext->GetCachePolicyTlbElement();

        bool LLC = (pGmmLibContext->GetGtSysInfo()->LLCCacheSizeInKb > 0); // aka "Core -vs- Atom".

#if defined(_WIN32)
        {
            pCachePolicyTblElement[0].L3.Cacheability   = L3_UNCACHEABLE;
            pCachePolicyTblElement[0].LeCC.Cacheability = LeCC_UNCACHEABLE;
            pCachePolicyTblElement[0].LeCC.TargetCache  = LLC ? TC_LLC_ELLC : TC_ELLC; // No LLC for Broxton, GLK - keep clear configuration for LLC
        }
#else
        {
#define I915_GEN9_MOCS_ENTRIES 3
            GMM_CACHE_POLICY_TBL_ELEMENT *pEntry = pCachePolicyTblElement;
            C_ASSERT(I915_GEN9_MOCS_ENTRIES <= GMM_GEN9_MAX_NUMBER_MOCS_INDEXES);

            // I915_MOCS_UNCACHED(0)...
            pEntry[0].L3.Cacheability   = L3_UNCACHEABLE;
            pEntry[0].LeCC.Cacheability = LeCC_UNCACHEABLE;
            pEntry[0].LeCC.TargetCache  = TC_LLC_ELLC;

            // I915_MOCS_PTE(1)...
            pEntry[1] = pEntry[0]; // Unused by GmmLib clients, so set to UC.
            CurrentMaxIndex++;

            // I915_MOCS_CACHED(2)...
            pEntry[2].L3.Cacheability   = L3_WB_CACHEABLE;
            pEntry[2].LeCC.Cacheability = LLC ? LeCC_WB_CACHEABLE : LeCC_UNCACHEABLE;
            pEntry[2].LeCC.TargetCache  = TC_LLC_ELLC;
            pEntry[2].LeCC.LRUM         = 3;
            CurrentMaxIndex++;
        }
#endif

        // Process the cache policy and fill in the look up table
        for(uint32_t Usage = 0; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
        {
            bool                         CachePolicyError = false;
            uint64_t                     PTEValue         = 0;
            int32_t                      CPTblIdx         = -1;
            uint32_t                     j                = 0;
            GMM_CACHE_POLICY_TBL_ELEMENT UsageEle         = {0};
            UsageEle.LeCC.Reserved                        = 0; // Reserved bits zeroe'd, this is so we
                                                               // we can compare the unioned LeCC.DwordValue.
            UsageEle.LeCC.SCF = pCachePolicy[Usage].SCF;
            UsageEle.LeCC.PFM = 0; // TODO: decide what the page faulting mode should be
            UsageEle.LeCC.SCC = 0;
            UsageEle.LeCC.ESC = 0;
            if(pCachePolicy[Usage].LeCC_SCC)
            {
                UsageEle.LeCC.SCC = pCachePolicy[Usage].LeCC_SCC;
                UsageEle.LeCC.ESC = ENABLE_SKIP_CACHING_CONTROL;
            }
            UsageEle.LeCC.AOM  = pCachePolicy[Usage].AOM;
            UsageEle.LeCC.LRUM = pCachePolicy[Usage].AGE;

            // default to LLC/ELLC target cache.
            UsageEle.LeCC.TargetCache  = TC_LLC_ELLC;
            UsageEle.LeCC.Cacheability = LeCC_WB_CACHEABLE;

            if(pGmmLibContext->GetPlatformInfo().Platform.eProductFamily == IGFX_BROXTON ||
               pGmmLibContext->GetPlatformInfo().Platform.eProductFamily == IGFX_GEMINILAKE)
            {
                UsageEle.LeCC.AOM          = 0;
                UsageEle.LeCC.Cacheability = LeCC_UNCACHEABLE; // To avoid side effects use 01b even though 01b(UC) 11b(WB) are equivalent option

#if defined(GMM_DYNAMIC_MOCS_TABLE)
                UsageEle.LeCC.TargetCache = TC_LLC; // No LLC for Broxton, but we still set it to LLC since it is needed for IA coherency cases
                UsageEle.LeCC.LRUM         = 0;
#else
                UsageEle.LeCC.TargetCache = TC_LLC_ELLC; // To match I915_GEN9_MOCS[0]
#endif
            }
            else
            {
                if(pCachePolicy[Usage].LLC && pCachePolicy[Usage].ELLC)
                {
                    UsageEle.LeCC.TargetCache = TC_LLC_ELLC;
                }
                else if(pCachePolicy[Usage].LLC)
                {
                    UsageEle.LeCC.TargetCache = TC_LLC;
                }
                else if(pCachePolicy[Usage].ELLC)
                {
                    UsageEle.LeCC.TargetCache = TC_ELLC;
                }
                else
                {
                    UsageEle.LeCC.Cacheability = LeCC_UNCACHEABLE;
                }
            }

            UsageEle.L3.Reserved = 0; // Reserved bits zeroe'd, this is so we
                                      // we can compare the unioned L3.UshortValue.
            UsageEle.L3.ESC          = DISABLE_SKIP_CACHING_CONTROL;
            UsageEle.L3.SCC          = 0;
            UsageEle.L3.Cacheability = pCachePolicy[Usage].L3 ? L3_WB_CACHEABLE : L3_UNCACHEABLE;
            if(pCachePolicy[Usage].L3_SCC)
            {
                UsageEle.L3.ESC = ENABLE_SKIP_CACHING_CONTROL;
                UsageEle.L3.SCC = (uint16_t)pCachePolicy[Usage].L3_SCC;
            }
            for(j = 0; j <= CurrentMaxIndex; j++)
            {
                GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pCachePolicyTblElement[j];
                if(TblEle->LeCC.DwordValue == UsageEle.LeCC.DwordValue &&
                   TblEle->L3.UshortValue == UsageEle.L3.UshortValue)
                {
                    CPTblIdx = j;
                    break;
                }
            }

            // Didn't find the caching settings in one of the already programmed lookup table entries.
            // Need to add a new lookup table entry.
            if(CPTblIdx == -1)
            {
                if(CurrentMaxIndex < GMM_GEN9_MAX_NUMBER_MOCS_INDEXES - 1)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pCachePolicyTblElement[++CurrentMaxIndex]);
                    CPTblIdx                             = CurrentMaxIndex;

                    TblEle->LeCC.DwordValue = UsageEle.LeCC.DwordValue;
                    TblEle->L3.UshortValue  = UsageEle.L3.UshortValue;
                }
                else
                {
                    // Too many unique caching combinations to program the
                    // MOCS lookup table.
                    CachePolicyError = true;
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support GMM_GEN_MAX_NUMBER_MOCS_INDEXES = %d)",
                    GMM_GEN9_MAX_NUMBER_MOCS_INDEXES);
                    // Set cache policy index to uncached.
                    CPTblIdx = 0;
                }
            }

            if(!GetUsagePTEValue(pCachePolicy[Usage], Usage, &PTEValue))
            {
                CachePolicyError = true;
            }

            pCachePolicy[Usage].PTE.DwordValue     = PTEValue & 0xFFFFFFFF;
            pCachePolicy[Usage].PTE.HighDwordValue = 0;
            pCachePolicy[Usage].MemoryObjectOverride.Gen9.Index = CPTblIdx;

            pCachePolicy[Usage].Override = ALWAYS_OVERRIDE;

            if(CachePolicyError)
            {
                GMM_ASSERTDPF("Cache Policy Init Error: Invalid Cache Programming - Element %d", Usage);
            }
        }
        CurrentMaxMocsIndex      = CurrentMaxIndex;
        CurrentMaxL1HdcMocsIndex = 0;
    }

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Initializes the Gfx PAT tables for AdvCtx and Gfx MMIO/Private PAT
///    PAT0 = WB_COHERENT or UC depending on WaGttPat0WB
///    PAT1 = UC or WB_COHERENT depending on WaGttPat0WB
///    PAT2 = WB_MOCSLESS, with TC = eLLC+LLC
///    PAT3 = WB
///    PAT4 = WT
///    PAT5 = WC
///    PAT6 = WC
///    PAT7 = WC
///  HLD says to set to PAT0/1 to WC, but since we don't have a WC in GPU,
///  WC option is same as UC. Hence setting PAT0 or PAT1 to UC.
///  Unused PAT's (5,6,7) are set to WC.
///
/// @return        GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmGen9CachePolicy::SetupPAT()
{
    GMM_STATUS Status = GMM_SUCCESS;
#if(defined(__GMM_KMD__))
    uint32_t i = 0;

    GMM_GFX_MEMORY_TYPE GfxMemType = GMM_GFX_UC_WITH_FENCE;
    // No optional selection on Age or Target Cache because for an SVM-OS Age and
    // Target Cache would not work [for an SVM-OS the Page Table is shared with IA
    // and we don't have control of the PAT Idx]. If there is a strong ask from D3D
    // or the performance analysis team, Age could be added.
    // Add Class of Service when required.
    GMM_GFX_TARGET_CACHE GfxTargetCache             = GMM_GFX_TC_ELLC_LLC;
    uint8_t              Age                        = 1;
    uint8_t              ServiceClass               = 0;
    int32_t *            pPrivatePATTableMemoryType = NULL;

    pPrivatePATTableMemoryType = pGmmLibContext->GetPrivatePATTableMemoryType();

    __GMM_ASSERT(pGmmLibContext->GetSkuTable().FtrIA32eGfxPTEs);

    for(i = 0; i < GMM_NUM_GFX_PAT_TYPES; i++)
    {
        pPrivatePATTableMemoryType[i] = -1;
    }

    // Set values for GmmGlobalInfo PrivatePATTable
    for(i = 0; i < NumPATRegisters; i++)
    {
        GMM_PRIVATE_PAT PAT = {0};

        if(pGmmLibContext->GetSkuTable().FtrMemTypeMocsDeferPAT)
        {
            GfxTargetCache = GMM_GFX_TC_ELLC_ONLY;
        }
        else
        {
            GfxTargetCache = GMM_GFX_TC_ELLC_LLC;
        }

        switch(i)
        {
            case PAT0:
                if(pGmmLibContext->GetWaTable().WaGttPat0)
                {
                    if(pGmmLibContext->GetWaTable().WaGttPat0WB)
                    {
                        GfxMemType = GMM_GFX_WB;
                        if(GFX_IS_ATOM_PLATFORM(pGmmLibContext))
                        {
                            PAT.PreGen10.Snoop = 1;
                        }
                        pPrivatePATTableMemoryType[GMM_GFX_PAT_WB_COHERENT] = PAT0;
                    }
                    else
                    {
                        GfxMemType                                 = GMM_GFX_UC_WITH_FENCE;
                        pPrivatePATTableMemoryType[GMM_GFX_PAT_UC] = PAT0;
                    }
                }
                else // if GTT is not tied to PAT0 then WaGttPat0WB is NA
                {
                    GfxMemType = GMM_GFX_WB;
                    if(GFX_IS_ATOM_PLATFORM(pGmmLibContext))
                    {
                        PAT.PreGen10.Snoop = 1;
                    }
                    pPrivatePATTableMemoryType[GMM_GFX_PAT_WB_COHERENT] = PAT0;
                }
                break;

            case PAT1:
                if(pGmmLibContext->GetWaTable().WaGttPat0 && !pGmmLibContext->GetWaTable().WaGttPat0WB)
                {
                    GfxMemType = GMM_GFX_WB;
                    if(GFX_IS_ATOM_PLATFORM(pGmmLibContext))
                    {
                        PAT.PreGen10.Snoop = 1;
                    }
                    pPrivatePATTableMemoryType[GMM_GFX_PAT_WB_COHERENT] = PAT1;
                }
                else
                {
                    GfxMemType                                 = GMM_GFX_UC_WITH_FENCE;
                    pPrivatePATTableMemoryType[GMM_GFX_PAT_UC] = PAT1;
                }
                break;

            case PAT2:
                // This PAT idx shall be used for MOCS'Less resources like Page Tables
                // Page Tables have TC hardcoded to eLLC+LLC in Adv Ctxt. Hence making this to have same in Leg Ctxt.
                // For BDW-H, due to Perf issue, TC has to be eLLC only for Page Tables when eDRAM is present.
                GfxMemType                                          = GMM_GFX_WB;
                GfxTargetCache                                      = GMM_GFX_TC_ELLC_LLC;
                pPrivatePATTableMemoryType[GMM_GFX_PAT_WB_MOCSLESS] = PAT2;
                break;

            case PAT3:
                GfxMemType                                 = GMM_GFX_WB;
                pPrivatePATTableMemoryType[GMM_GFX_PAT_WB] = PAT3;
                break;

            case PAT4:
                GfxMemType                                 = GMM_GFX_WT;
                pPrivatePATTableMemoryType[GMM_GFX_PAT_WT] = PAT4;
                break;

            case PAT5:
            case PAT6:
            case PAT7:
                GfxMemType                                 = GMM_GFX_WC;
                pPrivatePATTableMemoryType[GMM_GFX_PAT_WC] = PAT5;
                break;

            default:
                __GMM_ASSERT(0);
                Status = GMM_ERROR;
        }

        PAT.PreGen10.MemoryType  = GfxMemType;
        PAT.PreGen10.TargetCache = GfxTargetCache;
        PAT.PreGen10.Age         = Age;

        SetPrivatePATEntry(i, PAT);
    }

#else
    Status                                = GMM_ERROR;
#endif
    return Status;
}
