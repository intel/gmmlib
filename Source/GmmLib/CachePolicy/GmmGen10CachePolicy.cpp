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
#include "External/Common/CachePolicy/GmmCachePolicyGen10.h"
//=============================================================================
//
// Function: __GmmGen10InitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmGen10CachePolicy::InitCachePolicy()
{

    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, llc, ellc, l3, wt, age, lecc_scc, l3_scc, sso, cos, hdcl1) DEFINE_CP_ELEMENT(usage, llc, ellc, l3, wt, age, 0, lecc_scc, l3_scc, 0, sso, cos, hdcl1, 0, 0, 0, 0)
#include "GmmGen10CachePolicy.h"

#define TC_LLC (1)
#define TC_ELLC (0) //Is this supported anymore in TargetCache?
#define TC_LLC_ELLC (2)

#define LeCC_UNCACHEABLE (0x1)
#define LeCC_WT_CACHEABLE (0x2) //Only used as MemPushWRite disqualifier if set along with eLLC-only
#define LeCC_WB_CACHEABLE (0x3)

#define L3_UNCACHEABLE (0x1)
#define L3_WB_CACHEABLE (0x3)

#define DISABLE_SKIP_CACHING_CONTROL (0x0)
#define ENABLE_SKIP_CACHING_CONTROL (0x1)

#define DISABLE_SELF_SNOOP_OVERRIDE (0x0)
#define ENABLE_SELF_SNOOP_OVERRIDE (0x1)
#define ENABLE_SELF_SNOOP_ALWAYS (0x3)
#define CLASS_SERVICE_ZERO 0
    {
        // Define index of cache element
        uint32_t Usage = 0;

        uint32_t                      CurrentMaxIndex        = 0;
        uint32_t                      CurrentMaxHDCL1Index   = GMM_GEN10_HDCL1_MOCS_INDEX_START - 1; // define constant
        GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = pGmmGlobalContext->GetCachePolicyTlbElement();

        // index 0 is uncached.
        {
            GMM_CACHE_POLICY_TBL_ELEMENT *Entry0 = &(pCachePolicyTlbElement[0]);
            Entry0->LeCC.Cacheability            = LeCC_UNCACHEABLE;
            Entry0->LeCC.TargetCache             = TC_LLC_ELLC;
            Entry0->LeCC.LRUM                    = 0;
            Entry0->LeCC.ESC                     = DISABLE_SKIP_CACHING_CONTROL;
            Entry0->LeCC.SCC                     = 0;
            Entry0->LeCC.CoS                     = CLASS_SERVICE_ZERO;
            Entry0->LeCC.SelfSnoop               = DISABLE_SELF_SNOOP_OVERRIDE;
            Entry0->L3.Cacheability              = L3_UNCACHEABLE;
            Entry0->L3.ESC                       = DISABLE_SKIP_CACHING_CONTROL;
            Entry0->L3.SCC                       = 0;
            Entry0->HDCL1                        = 0;
        }

        // Process the cache policy and fill in the look up table
        for(; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
        {
            bool                         CachePolicyError = false;
            int32_t                      CPTblIdx         = -1;
            uint32_t                     j                = 0;
            uint32_t                     PTEValue         = 0;
            GMM_CACHE_POLICY_TBL_ELEMENT UsageEle         = {0};
            UsageEle.LeCC.Reserved                        = 0; // Reserved bits zeroe'd, this is so we
                                                               // we can compare the unioned LeCC.DwordValue.
            UsageEle.LeCC.SelfSnoop = DISABLE_SELF_SNOOP_OVERRIDE;
            UsageEle.LeCC.CoS       = CLASS_SERVICE_ZERO;
            UsageEle.LeCC.SCC       = 0;
            UsageEle.LeCC.ESC       = 0;
            if(pCachePolicy[Usage].SSO & ENABLE_SELF_SNOOP_OVERRIDE)
            {
                UsageEle.LeCC.SelfSnoop = pCachePolicy[Usage].SSO & ENABLE_SELF_SNOOP_ALWAYS;
            }
            if(pCachePolicy[Usage].CoS)
            {
                UsageEle.LeCC.CoS = pCachePolicy[Usage].CoS;
            }
            if(pCachePolicy[Usage].HDCL1)
            {
                UsageEle.HDCL1 = 1;
            }
            if(pCachePolicy[Usage].LeCC_SCC)
            {
                UsageEle.LeCC.SCC = pCachePolicy[Usage].LeCC_SCC;
                UsageEle.LeCC.ESC = ENABLE_SKIP_CACHING_CONTROL;
            }
            UsageEle.LeCC.LRUM = pCachePolicy[Usage].AGE;

            // default to LLC/ELLC target cache.
            UsageEle.LeCC.TargetCache  = TC_LLC_ELLC;
            UsageEle.LeCC.Cacheability = LeCC_WB_CACHEABLE;
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
                if(pCachePolicy[Usage].WT)
                {
                    UsageEle.LeCC.Cacheability = LeCC_WT_CACHEABLE;
                }
            }
            else
            {
                UsageEle.LeCC.Cacheability = LeCC_UNCACHEABLE;
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
            //For HDC L1 caching, MOCS Table index 48-61 should be used
            if(UsageEle.HDCL1)
            {
                for(j = GMM_GEN10_HDCL1_MOCS_INDEX_START; j <= CurrentMaxHDCL1Index; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pCachePolicyTlbElement[j];
                    if(TblEle->LeCC.DwordValue == UsageEle.LeCC.DwordValue &&
                       TblEle->L3.UshortValue == UsageEle.L3.UshortValue &&
                       TblEle->HDCL1 == UsageEle.HDCL1)
                    {
                        CPTblIdx = j;
                        break;
                    }
                }
            }
            else
            {
                for(j = 0; j <= CurrentMaxIndex; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pCachePolicyTlbElement[j];
                    if(TblEle->LeCC.DwordValue == UsageEle.LeCC.DwordValue &&
                       TblEle->L3.UshortValue == UsageEle.L3.UshortValue &&
                       TblEle->HDCL1 == UsageEle.HDCL1)
                    {
                        CPTblIdx = j;
                        break;
                    }
                }
            }

            // Didn't find the caching settings in one of the already programmed lookup table entries.
            // Need to add a new lookup table entry.
            if(CPTblIdx == -1)
            {
                if(UsageEle.HDCL1 && CurrentMaxHDCL1Index < GMM_GEN9_MAX_NUMBER_MOCS_INDEXES - 1)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pCachePolicyTlbElement[++CurrentMaxHDCL1Index]);
                    CPTblIdx                             = CurrentMaxHDCL1Index;

                    TblEle->LeCC.DwordValue = UsageEle.LeCC.DwordValue;
                    TblEle->L3.UshortValue  = UsageEle.L3.UshortValue;
                    TblEle->HDCL1           = UsageEle.HDCL1;
                }
                else if(CurrentMaxIndex < GMM_GEN10_HDCL1_MOCS_INDEX_START)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pCachePolicyTlbElement[++CurrentMaxIndex]);
                    CPTblIdx                             = CurrentMaxIndex;

                    TblEle->LeCC.DwordValue = UsageEle.LeCC.DwordValue;
                    TblEle->L3.UshortValue  = UsageEle.L3.UshortValue;
                    TblEle->HDCL1           = UsageEle.HDCL1;
                }
                else
                {
                    // Too many unique caching combinations to program the
                    // MOCS lookup table.
                    CachePolicyError = true;
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support GMM_GEN_MAX_NUMBER_MOCS_INDEXES = %d)",
                    GMM_GEN9_MAX_NUMBER_MOCS_INDEXES - 1);
                    // Set cache policy index to uncached.
                    CPTblIdx = 0;
                }
            }

            // PTE entries do not control caching on SKL+ (for legacy context)
            if(!GetUsagePTEValue(pCachePolicy[Usage], Usage, &PTEValue))
            {
                CachePolicyError = true;
            }

            pCachePolicy[Usage].PTE.DwordValue = PTEValue;

            pCachePolicy[Usage].MemoryObjectOverride.Gen10.Index = CPTblIdx;

            pCachePolicy[Usage].Override = ALWAYS_OVERRIDE;

            if(CachePolicyError)
            {
                GMM_ASSERTDPF("Cache Policy Init Error: Invalid Cache Programming - Element %d", Usage);
            }
        }
        CurrentMaxMocsIndex      = CurrentMaxIndex;
        CurrentMaxL1HdcMocsIndex = CurrentMaxHDCL1Index;
    }

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Initializes WA's needed for setting up the Private PATs
/// WaNoMocsEllcOnly, WaGttPat0, WaGttPat0GttWbOverOsIommuEllcOnly, WaGttPat0WB
///
/// @return        GMM_STATUS
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmGen10CachePolicy::SetPATInitWA()
{
    GMM_STATUS Status = GMM_SUCCESS;

#if(defined(__GMM_KMD__))
    
#else
    Status = GMM_ERROR;
#endif

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the PAT idx that best matches the cache policy for this usage.
///
/// @param: CachePolicy: cache policy for a usage
///
/// @return        PAT Idx to use in the PTE
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen10CachePolicy::BestMatchingPATIdx(GMM_CACHE_POLICY_ELEMENT CachePolicy)
{
    uint32_t             i;
    uint32_t             PATIdx           = 0;
    GMM_GFX_MEMORY_TYPE  WantedMemoryType = GMM_GFX_UC_WITH_FENCE, MemoryType;
    GMM_GFX_TARGET_CACHE WantedTC         = GMM_GFX_TC_ELLC_LLC;

    WantedMemoryType = GetWantedMemoryType(CachePolicy);

    if(CachePolicy.LLC && CachePolicy.ELLC)
    {
        WantedTC = GMM_GFX_TC_ELLC_LLC;
    }
    else if(CachePolicy.LLC)
    {
        WantedTC = GMM_GFX_TC_LLC_ONLY;
    }
    else if(CachePolicy.ELLC)
    {
        WantedTC = GMM_GFX_TC_ELLC_ONLY; // Note: this overrides the MOCS target cache selection.
    }

    for(i = 1; i < GMM_NUM_PAT_ENTRIES; i++)
    {
        GMM_PRIVATE_PAT PAT1 = GetPrivatePATEntry(PATIdx);
        GMM_PRIVATE_PAT PAT2 = GetPrivatePATEntry(i);

        if(SelectNewPATIdx(WantedMemoryType, WantedTC,
                           (GMM_GFX_MEMORY_TYPE)PAT1.Gen10.MemoryType, (GMM_GFX_TARGET_CACHE)PAT1.Gen10.TargetCache,
                           (GMM_GFX_MEMORY_TYPE)PAT2.Gen10.MemoryType, (GMM_GFX_TARGET_CACHE)PAT2.Gen10.TargetCache))
        {
            PATIdx = i;
        }
    }

    MemoryType = (GMM_GFX_MEMORY_TYPE)GetPrivatePATEntry(PATIdx).Gen10.MemoryType;

    if(MemoryType != WantedMemoryType)
    {
        // Failed to find a matching PAT entry
        return GMM_PAT_ERROR;
    }
    return PATIdx;
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
GMM_STATUS GmmLib::GmmGen10CachePolicy::SetupPAT()
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
    pPrivatePATTableMemoryType                      = pGmmGlobalContext->GetPrivatePATTableMemoryType();

    __GMM_ASSERT(pGmmGlobalContext->GetSkuTable().FtrIA32eGfxPTEs);

    for(i = 0; i < GMM_NUM_GFX_PAT_TYPES; i++)
    {
        pPrivatePATTableMemoryType[i] = -1;
    }

    // Set values for GmmGlobalInfo PrivatePATTable
    for(i = 0; i < GMM_NUM_PAT_ENTRIES; i++)
    {
        GMM_PRIVATE_PAT PAT = {0};

        if(pGmmGlobalContext->GetWaTable().FtrMemTypeMocsDeferPAT)
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
                if(pGmmGlobalContext->GetWaTable().WaGttPat0)
                {
                    if(pGmmGlobalContext->GetWaTable().WaGttPat0WB)
                    {
                        GfxMemType = GMM_GFX_WB;
                        if(GFX_IS_ATOM_PLATFORM)
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
                    if(GFX_IS_ATOM_PLATFORM)
                    {
                        PAT.PreGen10.Snoop = 1;
                    }
                    pPrivatePATTableMemoryType[GMM_GFX_PAT_WB_COHERENT] = PAT0;
                }
                break;

            case PAT1:
                if(pGmmGlobalContext->GetWaTable().WaGttPat0 && !pGmmGlobalContext->GetWaTable().WaGttPat0WB)
                {
                    GfxMemType = GMM_GFX_WB;
                    if(GFX_IS_ATOM_PLATFORM)
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

        PAT.Gen10.MemoryType  = GfxMemType;
        PAT.Gen10.TargetCache = GfxTargetCache;
        PAT.Gen10.Age         = Age;
        PAT.Gen10.CoS         = ServiceClass;

        SetPrivatePATEntry(i, PAT);
    }

#else
    Status = GMM_ERROR;
#endif
    return Status;
}