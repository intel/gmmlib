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
#include "External/Common/CachePolicy/GmmCachePolicyGen11.h"

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
int32_t GmmLib::GmmGen11CachePolicy::IsSpecialMOCSUsage(GMM_RESOURCE_USAGE_TYPE Usage, bool &UpdateMOCS)
{
    int32_t MocsIdx = -1;
    UpdateMOCS      = true;

    switch(Usage)
    {
        case GMM_RESOURCE_USAGE_MOCS_62:
            __GMM_ASSERT(pCachePolicy[Usage].L3 == 0); //Architecturally, TR/Aux-TT node isn't L3-cacheable.
            pCachePolicy[Usage].L3 = 0;
            MocsIdx                = 62;
            break;
        case GMM_RESOURCE_USAGE_L3_EVICTION:
            pCachePolicy[Usage].L3 = 0;
            MocsIdx                = 63;
            break;
        default:
            UpdateMOCS = false;
            break;
    }

    return MocsIdx;
}

//=============================================================================
//
// Function: __GmmGen11InitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmGen11CachePolicy::InitCachePolicy()
{
    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, llc, ellc, l3, wt, age, aom, lecc_scc, l3_scc, scf, sso, cos) DEFINE_CP_ELEMENT(usage, llc, ellc, l3, wt, age, aom, lecc_scc, l3_scc, scf, sso, cos, 0, 0, 0, 0, 0)
#include "GmmGen11CachePolicy.h"

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
#define CLASS_SERVICE_ZERO (0x0)

    // Setup Static MOCS Table
    {
        this->SetUpMOCSTable();
    }

    {
        // Define index of cache element
        uint32_t Usage                  = 0;
        uint32_t CurrentMaxSpecialIndex = GMM_GEN9_MAX_NUMBER_MOCS_INDEXES - 1;

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
        OverrideCachePolicy();
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
            }

            if(pCachePolicy[Usage].SSO & ENABLE_SELF_SNOOP_OVERRIDE)
            {
                UsageEle.LeCC.SelfSnoop = pCachePolicy[Usage].SSO & ENABLE_SELF_SNOOP_ALWAYS;
            }
            if(pCachePolicy[Usage].CoS)
            {
                UsageEle.LeCC.CoS = pCachePolicy[Usage].CoS;
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

            if(CPTblIdx >= GMM_GEN9_MAX_NUMBER_MOCS_INDEXES)
            {
                GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[CPTblIdx];
                CurrentMaxSpecialIndex               = ((uint32_t)CPTblIdx > CurrentMaxSpecialIndex) ? (uint32_t)CPTblIdx : CurrentMaxSpecialIndex;

                if(SpecialMOCS && //Update if one of special MOCS enums
                   !(TblEle->LeCC.DwordValue == UsageEle.LeCC.DwordValue &&
                     TblEle->L3.UshortValue == UsageEle.L3.UshortValue))
                {
                    //Assert if being overwritten!
                    __GMM_ASSERT(TblEle->LeCC.DwordValue == 0 &&
                                 TblEle->L3.UshortValue == 0);
                }
            }
            else
            {
                for(j = 1; j <= CurrentMaxMocsIndex; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[j];
                    if(TblEle->LeCC.DwordValue == UsageEle.LeCC.DwordValue &&
                       TblEle->L3.UshortValue == UsageEle.L3.UshortValue)
                    {
                        CPTblIdx = j;
                        break;
                    }
                }
            }
            // Didn't find the caching settings in one of the already programmed Explicit Mocs lookup table entries
            // Need to add a new explicit mocs lookup table entry.
            if(CPTblIdx == -1)
            {

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
                // If the Cache Policy setting is overriden through regkey,
                // don't raise an assert/log error. Raising an assert for debug/perf testing isn't really helpful
                if(pCachePolicy[Usage].IsOverridenByRegkey)
                {
                    if(CurrentMaxMocsIndex < GMM_GEN9_MAX_NUMBER_MOCS_INDEXES - 1)
                    {
                        GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pGmmGlobalContext->GetCachePolicyTlbElement()[++CurrentMaxMocsIndex]);
                        CPTblIdx                             = CurrentMaxMocsIndex;

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
                        GMM_GEN9_MAX_NUMBER_MOCS_INDEXES - 1);
                        // Set cache policy index to uncached.
                        CPTblIdx = 0;
                    }
                }
                else
#endif
                {
                    GMM_ASSERTDPF(false, "CRITICAL ERROR: Cache Policy Usage value specified by Client in not defined in Fixed MOCS Table!");

// Log Error using regkey to indicate the above error
#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL) && __GMM_KMD__)
                    REGISTRY_OVERRIDE_WRITE(Usage, NewMOCSEntryLeCCValue, UsageEle.LeCC.DwordValue);
                    REGISTRY_OVERRIDE_WRITE(Usage, NewMOCSEntryL3Value, UsageEle.L3.UshortValue);
#endif

                    CachePolicyError = true;
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support GMM_GEN_MAX_NUMBER_MOCS_INDEXES = %d)",
                    CurrentMaxMocsIndex);

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

            pCachePolicy[Usage].MemoryObjectOverride.Gen11.Index = CPTblIdx;

            pCachePolicy[Usage].Override = ALWAYS_OVERRIDE;

            if(CachePolicyError)
            {
                GMM_ASSERTDPF("Cache Policy Init Error: Invalid Cache Programming - Element %d", Usage);
            }
        }

        CurrentMaxSpecialMocsIndex = CurrentMaxSpecialIndex;
    }

    return GMM_SUCCESS;
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
void GmmLib::GmmGen11CachePolicy::SetUpMOCSTable()
{
    GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = &(pGmmGlobalContext->GetCachePolicyTlbElement()[0]);

#define GMM_DEFINE_MOCS(Index, L3_ESC, L3_SCC, L3_CC, LeCC_CC, LeCC_TC, LeCC_LRUM, LeCC_AOM, LeCC_ESC, LeCC_SCC, LeCC_PFM, LeCC_SCF, LeCC_CoS, LeCC_SelfSnoop) \
    {                                                                                                                                                          \
        pCachePolicyTlbElement[Index].L3.ESC            = L3_ESC;                                                                                              \
        pCachePolicyTlbElement[Index].L3.SCC            = L3_SCC;                                                                                              \
        pCachePolicyTlbElement[Index].L3.Cacheability   = L3_CC;                                                                                               \
        pCachePolicyTlbElement[Index].LeCC.Cacheability = LeCC_CC;                                                                                             \
        pCachePolicyTlbElement[Index].LeCC.TargetCache  = LeCC_TC;                                                                                             \
        pCachePolicyTlbElement[Index].LeCC.LRUM         = LeCC_LRUM;                                                                                           \
        pCachePolicyTlbElement[Index].LeCC.AOM          = LeCC_AOM;                                                                                            \
        pCachePolicyTlbElement[Index].LeCC.ESC          = LeCC_ESC;                                                                                            \
        pCachePolicyTlbElement[Index].LeCC.SCC          = LeCC_SCC;                                                                                            \
        pCachePolicyTlbElement[Index].LeCC.PFM          = LeCC_PFM;                                                                                            \
        pCachePolicyTlbElement[Index].LeCC.SCF          = LeCC_SCF;                                                                                            \
        pCachePolicyTlbElement[Index].LeCC.CoS          = LeCC_CoS;                                                                                            \
        pCachePolicyTlbElement[Index].LeCC.SelfSnoop    = LeCC_SelfSnoop;                                                                                      \
    }

    // clang-format off

    //Default MOCS Table
    for(int index = 0; index < GMM_MAX_NUMBER_MOCS_INDEXES; index++)
    {  //              Index     ESC    SCC      L3CC    LeCC    TC      LRUM    DAoM    ERSC    SCC     PFM     SCF     CoS     SSE
        GMM_DEFINE_MOCS(index   , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    }
    // Explicit MOCS Table
    //              Index     ESC	  SCC	  L3CC    LeCC    TC      LRUM    DAoM	  ERSC	  SCC	  PFM	  SCF     CoS     SSE
    GMM_DEFINE_MOCS( 1      , 0     , 0     , 3     , 0     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 2      , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 3      , 0     , 0     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 4      , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 5      , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 6      , 0     , 0     , 1     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 7      , 0     , 0     , 3     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 8      , 0     , 0     , 1     , 3     , 1     , 2     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 9      , 0     , 0     , 3     , 3     , 1     , 2     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 10     , 0     , 0     , 1     , 3     , 1     , 3     , 1     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 11     , 0     , 0     , 3     , 3     , 1     , 3     , 1     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 12     , 0     , 0     , 1     , 3     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 13     , 0     , 0     , 3     , 3     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 14     , 0     , 0     , 1     , 3     , 1     , 2     , 1     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 15     , 0     , 0     , 3     , 3     , 1     , 2     , 1     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 16     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 17     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 18     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 3 )
    GMM_DEFINE_MOCS( 19     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 7     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 20     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 3     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 21     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 0     , 1     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 22     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 1     , 3     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 23     , 0     , 0     , 3     , 3     , 1     , 3     , 0     , 1     , 7     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 62     , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )
    GMM_DEFINE_MOCS( 63     , 0     , 0     , 1     , 3     , 1     , 3     , 0     , 0     , 0     , 0     , 0     , 0     , 0 )

    if (pGmmGlobalContext->GetSkuTable().FtrLLCBypass)
    {
        GMM_DEFINE_MOCS( 16     , 0     , 0     , 1     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 1     , 0     , 0 )
        GMM_DEFINE_MOCS( 17     , 0     , 0     , 3     , 1     , 1     , 0     , 0     , 0     , 0     , 0     , 1     , 0     , 0 )
    }

    CurrentMaxMocsIndex         = 23;

// clang-format on
#undef GMM_DEFINE_MOCS
}
