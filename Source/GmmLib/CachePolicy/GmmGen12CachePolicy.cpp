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


#include "Internal/Common/GmmLibInc.h"
#include "External/Common/GmmCachePolicy.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen10.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen11.h"
#include "External/Common/CachePolicy/GmmCachePolicyGen12.h"

#if __GMM_KMD__
extern "C" NTSTATUS __GmmReadDwordKeyValue(void *pKmdGmmContext, char *pPath, WCHAR *pValueName, ULONG *pValueData);
extern "C" NTSTATUS __GmmWriteDwordKeyValue(void *pKmdGmmContext, char *pCStringPath, WCHAR *pValueName, ULONG DWord);
#endif

//=============================================================================
//
// Function: IsSpecialMOCSUsage
//
// Desc: This function returns special(hw-reserved) MocsIdx based on usage
//
// Parameters: usage  -> Resource usage type
//             UpdateMOCS  -> True if MOCS Table must be updated, ow false
//
// Return: int32_t
//
//-----------------------------------------------------------------------------
int32_t GmmLib::GmmGen12CachePolicy::IsSpecialMOCSUsage(GMM_RESOURCE_USAGE_TYPE Usage, bool &UpdateMOCS)
{
    int32_t MocsIdx = -1;
    UpdateMOCS      = true;
//Macros for L3-Eviction Type
#define NA 0x0
#define RO 0x1
#define RW 0x2
#define SP 0x3

    switch(Usage)
    {
        case GMM_RESOURCE_USAGE_CCS:
            __GMM_ASSERT(pCachePolicy[Usage].L3 == 0); //Architecturally, CCS isn't L3-cacheable.

            pCachePolicy[Usage].L3 = 0;
            MocsIdx                = 60;
            break;
        case GMM_RESOURCE_USAGE_MOCS_62:
            __GMM_ASSERT(pCachePolicy[Usage].L3 == 0); //Architecturally, TR/Aux-TT node isn't L3-cacheable.

            pCachePolicy[Usage].L3 = 0;
            MocsIdx                = 62;
            break;
        case GMM_RESOURCE_USAGE_L3_EVICTION:
            __GMM_ASSERT(pCachePolicy[Usage].L3 == 0 &&
                         pCachePolicy[Usage].L3Eviction == RW); //Reserved MOCS for L3-evictions

            pCachePolicy[Usage].L3         = 0;
            pCachePolicy[Usage].L3Eviction = RW;
            MocsIdx                        = 63;
            break;
        case GMM_RESOURCE_USAGE_L3_EVICTION_SPECIAL:
        case GMM_RESOURCE_USAGE_CCS_MEDIA_WRITABLE:
            __GMM_ASSERT(pCachePolicy[Usage].L3 &&
                         pCachePolicy[Usage].L3Eviction == SP); //Reserved MOCS for L3-evictions
                                                                //Special-case for Displayable, and similar non-LLC accesses
            GMM_ASSERTDPF(pCachePolicy[Usage].LLC == 0, "MOCS#61's Special Eviction isn't for LLC caching");

            pCachePolicy[Usage].L3         = 1;
            pCachePolicy[Usage].L3Eviction = SP;
            MocsIdx                        = 61;
            break;
        default:
            UpdateMOCS = false;
            break;
    }

    if(pCachePolicy[Usage].L3Eviction == RW)
    {
        GMM_CACHE_POLICY_ELEMENT L3Eviction;
        L3Eviction.Value = pCachePolicy[GMM_RESOURCE_USAGE_L3_EVICTION].Value;

        //For internal purpose, hw overrides MOCS#63 as L3-uncacheable, still using it for L3-evictions
        if(Usage != GMM_RESOURCE_USAGE_L3_EVICTION)
        {
            L3Eviction.L3 = 1; //Override L3, to verify MOCS#63 applicable or not
        }

        __GMM_ASSERT(pCachePolicy[Usage].Value == L3Eviction.Value); //Allow mis-match due to override registries
        //MocsIdx = 63;    //Use non-#63 MOCS, #63 itself is L3-uncached
    }
    else if(pCachePolicy[Usage].L3Eviction == SP)
    {
        __GMM_ASSERT(pCachePolicy[Usage].Value == pCachePolicy[GMM_RESOURCE_USAGE_L3_EVICTION_SPECIAL].Value); //Allow mis-match due to override registries
        MocsIdx = 61;
    }

    return MocsIdx;
}
//=============================================================================
//
// Function: __GmmGen12InitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmGen12CachePolicy::InitCachePolicy()
{

    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, llc, ellc, l3, wt, age, aom, lecc_scc, l3_scc, scf, sso, cos, hdcl1, l3evict) DEFINE_CP_ELEMENT(usage, llc, ellc, l3, wt, age, aom, lecc_scc, l3_scc, scf, sso, cos, hdcl1, l3evict, 0, 0, 0)

#include "GmmGen12CachePolicy.h"

#define TC_LLC (1)
#define TC_ELLC (0)
#define TC_LLC_ELLC (2)

#define LeCC_UNCACHEABLE (0x0)
#define LeCC_WC_UNCACHEABLE (0x1)
#define LeCC_WT_CACHEABLE (0x2) //Only used as MemPushWRite disqualifier if set along with eLLC-only -still holds on gen12+?
#define LeCC_WB_CACHEABLE (0x3)

#define L3_UNCACHEABLE (0x1)
#define L3_WB_CACHEABLE (0x3)

#define DISABLE_SKIP_CACHING_CONTROL (0x0)
#define ENABLE_SKIP_CACHING_CONTROL (0x1)

#define DISABLE_SELF_SNOOP_OVERRIDE (0x0)
#define ENABLE_SELF_SNOOP_OVERRIDE (0x1)
#define ENABLE_SELF_SNOOP_ALWAYS (0x3)
#define CLASS_SERVICE_ZERO (0x0)

    {
        SetUpMOCSTable();
    }

    {
        // Define index of cache element
        uint32_t Usage = 0;

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
        void *pKmdGmmContext = NULL;
#if(defined(__GMM_KMD__))
        pKmdGmmContext = pGmmGlobalContext->GetGmmKmdContext();
#endif

        OverrideCachePolicy(pKmdGmmContext);
#endif
        // Process the cache policy and fill in the look up table
        for(; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
        {
            bool                         CachePolicyError = false;
            bool                         SpecialMOCS      = false;
            int32_t                      CPTblIdx         = -1;
            uint32_t                     j                = 0;
            uint32_t                     PTEValue         = 0;
            GMM_CACHE_POLICY_TBL_ELEMENT UsageEle         = {0};

            CPTblIdx = IsSpecialMOCSUsage((GMM_RESOURCE_USAGE_TYPE)Usage, SpecialMOCS);

            UsageEle.LeCC.Reserved = 0; // Reserved bits zeroe'd, this is so we
                                        // we can compare the unioned LeCC.DwordValue.
            UsageEle.LeCC.SelfSnoop = DISABLE_SELF_SNOOP_OVERRIDE;
            UsageEle.LeCC.CoS       = CLASS_SERVICE_ZERO;
            UsageEle.LeCC.SCC       = 0;
            UsageEle.LeCC.ESC       = 0;

            if(pCachePolicy[Usage].SCF && pGmmGlobalContext->GetSkuTable().FtrLLCBypass)
            {
                UsageEle.LeCC.SCF = pCachePolicy[Usage].SCF;
                __GMM_ASSERT(pCachePolicy[Usage].LLC == 0); //LLC and ByPassLLC are mutually-exclusive
            }

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

            // default to LLC target cache.
            UsageEle.LeCC.TargetCache  = TC_LLC;
            UsageEle.LeCC.Cacheability = LeCC_WB_CACHEABLE;
            if(pCachePolicy[Usage].LLC)
            {
                UsageEle.LeCC.TargetCache = TC_LLC;
                __GMM_ASSERT(pCachePolicy[Usage].SCF == 0); //LLC and ByPassLLC are mutually-exclusive
            }
            else
            {
                UsageEle.LeCC.Cacheability = LeCC_WC_UNCACHEABLE;
            }
            UsageEle.L3.Reserved = 0; // Reserved bits zeroe'd, this is so we
                                      // we can compare the unioned L3.UshortValue.
            UsageEle.L3.ESC          = DISABLE_SKIP_CACHING_CONTROL;
            UsageEle.L3.SCC          = 0;
            UsageEle.L3.Cacheability = pCachePolicy[Usage].L3 ? L3_WB_CACHEABLE : L3_UNCACHEABLE;

            __GMM_ASSERT((pCachePolicy[Usage].L3 && pCachePolicy[Usage].L3Eviction != 0) ||
                         (pCachePolicy[Usage].L3 == 0 && (pCachePolicy[Usage].L3Eviction == 0 || Usage == GMM_RESOURCE_USAGE_L3_EVICTION)));

            if(pCachePolicy[Usage].L3_SCC)
            {
                UsageEle.L3.ESC = ENABLE_SKIP_CACHING_CONTROL;
                UsageEle.L3.SCC = (uint16_t)pCachePolicy[Usage].L3_SCC;
            }

            //Special-case MOCS handling for MOCS Table Index 60-63
            if(CPTblIdx >= GMM_GEN12_MAX_NUMBER_MOCS_INDEXES)
            {
                GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[CPTblIdx];

                if(SpecialMOCS &&
                   !(TblEle->LeCC.DwordValue == UsageEle.LeCC.DwordValue &&
                     TblEle->L3.UshortValue == UsageEle.L3.UshortValue &&
                     TblEle->HDCL1 == UsageEle.HDCL1))
                {
                    //Assert if being overwritten!
                    __GMM_ASSERT(TblEle->LeCC.DwordValue == 0 &&
                                 TblEle->L3.UshortValue == 0 &&
                                 TblEle->HDCL1 == 0);

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
                    if(pCachePolicy[Usage].IsOverridenByRegkey)
                    {
                        TblEle->LeCC.DwordValue = UsageEle.LeCC.DwordValue;
                        TblEle->L3.UshortValue  = UsageEle.L3.UshortValue;
                        TblEle->HDCL1           = UsageEle.HDCL1;
                    }
#endif
                }
            }
            //For HDC L1 caching, MOCS Table index 48-59 should be used
            else if(UsageEle.HDCL1)
            {
                for(j = GMM_GEN10_HDCL1_MOCS_INDEX_START; j <= CurrentMaxL1HdcMocsIndex; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[j];
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
                // Due to unstable system behavior on TGLLP, MOCS #0 index had to be programmed as UC in MOCS lookup table - pCachePolicyTlbElement
                // But still Index 0 is Reserved for Error by HW and should not be used.
                // Hence Gmmlib will opt out from the MOCS#0 usage and Lookup into MOCS table and MOCS index assigment must start from Index 1.
                for(j = 1; j <= CurrentMaxMocsIndex; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[j];
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

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
                // If the Cache Policy setting is overriden through regkey,
                // don't raise an assert/log error. Raising an assert for debug/perf testing isn't really helpful
                if(pCachePolicy[Usage].IsOverridenByRegkey)
                {
                    if(UsageEle.HDCL1 && CurrentMaxL1HdcMocsIndex < GMM_GEN12_MAX_NUMBER_MOCS_INDEXES - 1)
                    {
                        GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pGmmGlobalContext->GetCachePolicyTlbElement()[++CurrentMaxL1HdcMocsIndex]);
                        CPTblIdx                             = CurrentMaxL1HdcMocsIndex;

                        TblEle->LeCC.DwordValue = UsageEle.LeCC.DwordValue;
                        TblEle->L3.UshortValue  = UsageEle.L3.UshortValue;
                        TblEle->HDCL1           = UsageEle.HDCL1;
                    }
                    else if(CurrentMaxMocsIndex < GMM_GEN10_HDCL1_MOCS_INDEX_START)
                    {
                        GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pGmmGlobalContext->GetCachePolicyTlbElement()[++CurrentMaxMocsIndex]);
                        CPTblIdx                             = CurrentMaxMocsIndex;

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
                        GMM_MAX_NUMBER_MOCS_INDEXES - 1);
                        // Set cache policy index to uncached.
                        CPTblIdx = 3;
                    }
                }
                else
#endif
                {
                    GMM_ASSERTDPF(false, "CRITICAL ERROR: Cache Policy Usage value specified by Client is not defined in Fixed MOCS Table!");

// Log Error using regkey to indicate the above error
#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL) && __GMM_KMD__)
                    REGISTRY_OVERRIDE_WRITE(pKmdGmmContext, Usage, NewMOCSEntryLeCCValue, UsageEle.LeCC.DwordValue);
                    REGISTRY_OVERRIDE_WRITE(pKmdGmmContext, Usage, NewMOCSEntryL3Value, UsageEle.L3.UshortValue);
                    REGISTRY_OVERRIDE_WRITE(pKmdGmmContext, Usage, NewMOCSEntryHDCL1, UsageEle.HDCL1);
#endif

                    CachePolicyError = true;
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support GMM_GEN_MAX_NUMBER_MOCS_INDEXES = %d)",
                    CurrentMaxMocsIndex);

                    // Set cache policy index to uncached.
                    CPTblIdx = 3;
                }
            }

            // PTE entries do not control caching on SKL+ (for legacy context)
            if(!GetUsagePTEValue(pCachePolicy[Usage], Usage, &PTEValue))
            {
                CachePolicyError = true;
            }

            pCachePolicy[Usage].PTE.DwordValue = PTEValue;

            pCachePolicy[Usage].MemoryObjectOverride.Gen12.Index = CPTblIdx;

            pCachePolicy[Usage].Override = ALWAYS_OVERRIDE;

            if(CachePolicyError)
            {
                GMM_ASSERTDPF("Cache Policy Init Error: Invalid Cache Programming - Element %d", Usage);
            }
        }
    }

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Return true if (MT2) is a better match for (WantedMT)
///       than (MT1)
///
/// @param[in]         WantedMT: Wanted Memory Type
/// @param[in]         MT1: Memory Type for PATIdx1
/// @param[in]         MT2: Memory Type for PATIdx2
///
/// @return            Select the new PAT Index True/False
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GmmLib::GmmGen12CachePolicy::SelectNewPATIdx(GMM_GFX_MEMORY_TYPE WantedMT,
                                                     GMM_GFX_MEMORY_TYPE MT1, GMM_GFX_MEMORY_TYPE MT2)
{
    uint8_t SelectPAT2 = 0;

    // select on Memory Type
    if(MT1 != WantedMT)
    {
        if(MT2 == WantedMT || MT2 == GMM_GFX_UC_WITH_FENCE)
        {
            SelectPAT2 = 1;
        }
        goto EXIT;
    }

EXIT:
    return SelectPAT2;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the PAT idx that best matches the cache policy for this usage.
///
/// @param: CachePolicy: cache policy for a usage
///
/// @return        PAT Idx to use in the PTE
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GmmLib::GmmGen12CachePolicy::BestMatchingPATIdx(GMM_CACHE_POLICY_ELEMENT CachePolicy)
{
    uint32_t            i;
    uint32_t            PATIdx           = 0;
    GMM_GFX_MEMORY_TYPE WantedMemoryType = GMM_GFX_UC_WITH_FENCE, MemoryType;
    WA_TABLE *          pWaTable         = &const_cast<WA_TABLE &>(pGmmGlobalContext->GetWaTable());

    WantedMemoryType = GetWantedMemoryType(CachePolicy);

    // Override wantedMemoryType so that PAT.MT is UC
    // Gen12 uses max function to resolve PAT-vs-MOCS MemType, So unless PTE.PAT says UC, MOCS won't be able to set UC!
    if(pWaTable->WaMemTypeIsMaxOfPatAndMocs)
    {
        WantedMemoryType = GMM_GFX_UC_WITH_FENCE;
    }

    for(i = 1; i < GMM_NUM_PAT_ENTRIES; i++)
    {
        GMM_PRIVATE_PAT PAT1 = GetPrivatePATEntry(PATIdx);
        GMM_PRIVATE_PAT PAT2 = GetPrivatePATEntry(i);

        if(SelectNewPATIdx(WantedMemoryType,
                           (GMM_GFX_MEMORY_TYPE)PAT1.Gen12.MemoryType,
                           (GMM_GFX_MEMORY_TYPE)PAT2.Gen12.MemoryType))
        {
            PATIdx = i;
        }
    }

    MemoryType = (GMM_GFX_MEMORY_TYPE)GetPrivatePATEntry(PATIdx).Gen12.MemoryType;

    if(MemoryType != WantedMemoryType)
    {
        // Failed to find a matching PAT entry
        return GMM_PAT_ERROR;
    }
    return PATIdx;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Initializes WA's needed for setting up the Private PATs
/// WaNoMocsEllcOnly (reset)
/// WaGttPat0, WaGttPat0GttWbOverOsIommuEllcOnly, WaGttPat0WB (use from base class)
///
/// @return        GMM_STATUS
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmGen12CachePolicy::SetPATInitWA()
{
    GMM_STATUS Status   = GMM_SUCCESS;
    WA_TABLE * pWaTable = &const_cast<WA_TABLE &>(pGmmGlobalContext->GetWaTable());

#if(defined(__GMM_KMD__))
    __GMM_ASSERT(pGmmGlobalContext->GetSkuTable().FtrMemTypeMocsDeferPAT == 0x0); //MOCS.TargetCache supports eLLC only, PAT.TC -> reserved bits.
    pWaTable->WaGttPat0WB = 0;                                                 //Override PAT #0
#else
    Status = GMM_ERROR;
#endif

    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
///  Initializes the Gfx PAT tables for AdvCtx and Gfx MMIO/Private PAT
///    PAT0 = WB_COHERENT or UC depending on WaGttPat0WB
///    PAT1 = UC or WB_COHERENT depending on WaGttPat0WB
///    PAT2 = WB_MOCSLESS
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
GMM_STATUS GmmLib::GmmGen12CachePolicy::SetupPAT()
{
    GMM_STATUS Status = GMM_SUCCESS;
#if(defined(__GMM_KMD__))
    uint32_t i = 0;

    GMM_GFX_MEMORY_TYPE GfxMemType                 = GMM_GFX_UC_WITH_FENCE;
    int32_t *           pPrivatePATTableMemoryType = NULL;
    pPrivatePATTableMemoryType                     = pGmmGlobalContext->GetPrivatePATTableMemoryType();

    __GMM_ASSERT(pGmmGlobalContext->GetSkuTable().FtrIA32eGfxPTEs);

    for(i = 0; i < GMM_NUM_GFX_PAT_TYPES; i++)
    {
        pPrivatePATTableMemoryType[i] = -1;
    }

    // Set values for GmmGlobalInfo PrivatePATTable
    for(i = 0; i < GMM_NUM_PAT_ENTRIES; i++)
    {
        GMM_PRIVATE_PAT PAT = {0};

        switch(i)
        {
            case PAT0:
                if(pGmmGlobalContext->GetWaTable().WaGttPat0)
                {
                    if(pGmmGlobalContext->GetWaTable().WaGttPat0WB)
                    {
                        GfxMemType                                          = GMM_GFX_WB;
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
                    GfxMemType                                          = GMM_GFX_WB;
                    pPrivatePATTableMemoryType[GMM_GFX_PAT_WB_COHERENT] = PAT0;
                }
                break;

            case PAT1:
                if(pGmmGlobalContext->GetWaTable().WaGttPat0 && !pGmmGlobalContext->GetWaTable().WaGttPat0WB)
                {
                    GfxMemType                                          = GMM_GFX_WB;
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

        PAT.Gen12.MemoryType = GfxMemType;

        SetPrivatePATEntry(i, PAT);
    }

#else
    Status = GMM_ERROR;
#endif
    return Status;
}

//=============================================================================
//
// Function: SetUpMOCSTable
//
// Desc:
//
// Parameters:
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
void GmmLib::GmmGen12CachePolicy::SetUpMOCSTable()
{
    GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = &(pGmmGlobalContext->GetCachePolicyTlbElement()[0]);

#define GMM_DEFINE_MOCS(Index, L3_ESC, L3_SCC, L3_CC, LeCC_CC, LeCC_TC, LeCC_LRUM, LeCC_AOM, LeCC_ESC, LeCC_SCC, LeCC_PFM, LeCC_SCF, LeCC_CoS, LeCC_SelfSnoop, _HDCL1) \
    {                                                                                                                                                                  \
        pCachePolicyTlbElement[Index].L3.ESC            = L3_ESC;                                                                                                      \
        pCachePolicyTlbElement[Index].L3.SCC            = L3_SCC;                                                                                                      \
        pCachePolicyTlbElement[Index].L3.Cacheability   = L3_CC;                                                                                                       \
        pCachePolicyTlbElement[Index].LeCC.Cacheability = LeCC_CC;                                                                                                     \
        pCachePolicyTlbElement[Index].LeCC.TargetCache  = LeCC_TC;                                                                                                     \
        pCachePolicyTlbElement[Index].LeCC.LRUM         = LeCC_LRUM;                                                                                                   \
        pCachePolicyTlbElement[Index].LeCC.AOM          = LeCC_AOM;                                                                                                    \
        pCachePolicyTlbElement[Index].LeCC.ESC          = LeCC_ESC;                                                                                                    \
        pCachePolicyTlbElement[Index].LeCC.SCC          = LeCC_SCC;                                                                                                    \
        pCachePolicyTlbElement[Index].LeCC.PFM          = LeCC_PFM;                                                                                                    \
        pCachePolicyTlbElement[Index].LeCC.SCF          = LeCC_SCF;                                                                                                    \
        pCachePolicyTlbElement[Index].LeCC.CoS          = LeCC_CoS;                                                                                                    \
        pCachePolicyTlbElement[Index].LeCC.SelfSnoop    = LeCC_SelfSnoop;                                                                                              \
        pCachePolicyTlbElement[Index].HDCL1             = _HDCL1;                                                                                                      \
    }

    // clang-format off

   //Default MOCS Table
    for(int index = 0; index < GMM_MAX_NUMBER_MOCS_INDEXES; index++)
    {      //           Index     ES     SCC     L3CC    LeCC    TC      LRUM    DAoM    ERSC    SCC     PFM    SCF     CoS     SSE   HDCL1
        GMM_DEFINE_MOCS( index  , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    }

    // Fixed MOCS Table
    //              Index     ESC	  SCC	  L3CC    LeCC    TC      LRUM    DAoM	  ERSC	  SCC	  PFM	  SCF     CoS     SSE   HDCL1
    GMM_DEFINE_MOCS( 1      , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 2      , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 3      , 0     , 0     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 4      , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 5      , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 6      , 0     , 0     , 1     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 7      , 0     , 0     , 3     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 8      , 0     , 0     , 1     , 3     , 1     , 2     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 9      , 0     , 0     , 3     , 3     , 1     , 2     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 10     , 0     , 0     , 1     , 3     , 1     , 3     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 11     , 0     , 0     , 3     , 3     , 1     , 3     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 12     , 0     , 0     , 1     , 3     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 13     , 0     , 0     , 3     , 3     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 14     , 0     , 0     , 1     , 3     , 1     , 2     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 15     , 0     , 0     , 3     , 3     , 1     , 2     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 16     , 0     , 0     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 1     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 17     , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 1     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 18     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 3     , 0 )
    GMM_DEFINE_MOCS( 19     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 7     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 20     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 3     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 21     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 1     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 22     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 1     , 3     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 23     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 1     , 7     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 48     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 1 )
    GMM_DEFINE_MOCS( 49     , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 1 )
    GMM_DEFINE_MOCS( 50     , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 1 )
    GMM_DEFINE_MOCS( 51     , 0     , 0     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 1 )
    GMM_DEFINE_MOCS( 60     , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 61     , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 1     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 62     , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 63     , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )


    if(!pGmmGlobalContext->GetSkuTable().FtrLLCBypass ||
        GFX_GET_CURRENT_PRODUCT(pGmmGlobalContext->GetPlatformInfo().Platform) == IGFX_ROCKETLAKE)
    {
        GMM_DEFINE_MOCS( 16     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
        GMM_DEFINE_MOCS( 17     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
        GMM_DEFINE_MOCS( 61     , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    }
    // clang-format on

    CurrentMaxMocsIndex        = 23;
    CurrentMaxL1HdcMocsIndex   = 51;
    CurrentMaxSpecialMocsIndex = 63;

#undef GMM_DEFINE_MOCS
}
