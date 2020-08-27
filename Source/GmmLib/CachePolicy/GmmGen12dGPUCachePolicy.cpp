/*==============================================================================
Copyright(c) 2020 Intel Corporation

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
#include "External/Common/CachePolicy/GmmCachePolicyGen12dGPU.h"

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
int32_t GmmLib::GmmGen12dGPUCachePolicy::IsSpecialMOCSUsage(GMM_RESOURCE_USAGE_TYPE Usage, bool &UpdateMOCS)
{
    int32_t MocsIdx = -1;
    UpdateMOCS      = true;

    switch(Usage)
    {
        case GMM_RESOURCE_USAGE_CCS:
            __GMM_ASSERT(pCachePolicy[Usage].L3 == 0)      //Architecturally, CCS isn't L3-cacheable.
            MocsIdx = 60;
            break;
        case GMM_RESOURCE_USAGE_MOCS_62:
            __GMM_ASSERT(pCachePolicy[Usage].L3 == 0); //Architecturally, TR/Aux-TT node isn't L3-cacheable.
            MocsIdx = 62;
            break;
        case GMM_RESOURCE_USAGE_L3_EVICTION:
            MocsIdx = 63;
            break;
        case GMM_RESOURCE_USAGE_L3_EVICTION_SPECIAL:
        case GMM_RESOURCE_USAGE_CCS_MEDIA_WRITABLE:
            MocsIdx = 61;
            break;
        default:
            UpdateMOCS = false;
            break;
    }

    return MocsIdx;
}

//=============================================================================
//
// Function: __GmmGen12dGPUInitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmGen12dGPUCachePolicy::InitCachePolicy()
{

    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, l3, l3_scc, hdcl1) DEFINE_CP_ELEMENT(usage, 0, 0, l3, 0, 0, 0, 0, l3_scc, 0, 0, 0, hdcl1, 0, 0, 0, 0)

#include "GmmGen12dGPUCachePolicy.h"

#define L3_UNCACHEABLE (0x1)
#define L3_WB_CACHEABLE (0x3)

#define DISABLE_SKIP_CACHING_CONTROL (0x0)
#define ENABLE_SKIP_CACHING_CONTROL (0x1)

#define DISABLE_SELF_SNOOP_OVERRIDE (0x0)
#define CLASS_SERVICE_ZERO (0x0)
#define GMM_GEN12_MAX_NUMBER_MOCS_INDEXES (60) // On TGL last four (#60-#63) are reserved by h/w, few? are sw configurable though (#60)
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
            uint32_t                     StartMocsIdx     = 0;

            switch(GFX_GET_CURRENT_PRODUCT(pGmmGlobalContext->GetPlatformInfo().Platform))
            {
                case IGFX_DG1:
                    StartMocsIdx = 1; // Index 0 is reserved for Error
                    break;
                default:
                    StartMocsIdx = 1;
                    break;
            }

            // No Special MOCS handling for next platform
            if(GFX_GET_CURRENT_PRODUCT(pGmmGlobalContext->GetPlatformInfo().Platform) <= IGFX_DG1)
            {
                CPTblIdx = IsSpecialMOCSUsage((GMM_RESOURCE_USAGE_TYPE)Usage, SpecialMOCS);
            }

            // Applicable upto only
            if(pCachePolicy[Usage].HDCL1 &&
               (GFX_GET_CURRENT_PRODUCT(pGmmGlobalContext->GetPlatformInfo().Platform) <= IGFX_DG1))
            {
                UsageEle.HDCL1 = 1;
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

            //Special-case MOCS handling for MOCS Table Index 60-63
            if(CPTblIdx >= GMM_GEN12_MAX_NUMBER_MOCS_INDEXES)
            {
                GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[CPTblIdx];

                if(SpecialMOCS &&
                   !(TblEle->L3.UshortValue == UsageEle.L3.UshortValue))
                {
                    //Assert if being overwritten!
                    __GMM_ASSERT(TblEle->L3.UshortValue == 0);

                }
            }
            //For HDC L1 caching, MOCS Table index 48-59 should be used
            else if(UsageEle.HDCL1)
            {
                for(j = GMM_GEN10_HDCL1_MOCS_INDEX_START; j <= CurrentMaxL1HdcMocsIndex; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[j];
                    if(TblEle->L3.UshortValue == UsageEle.L3.UshortValue &&
                       TblEle->HDCL1 == UsageEle.HDCL1)
                    {
                        CPTblIdx = j;
                        break;
                    }
                }
            }
            else
            {
                for(j = StartMocsIdx; j <= CurrentMaxMocsIndex; j++)
                {
                    if(pCachePolicy[Usage].L3 == 0)
                    {
                        CPTblIdx = StartMocsIdx;
                        break;
                    }
                    else
                    {
                        GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmGlobalContext->GetCachePolicyTlbElement()[j];
                        if(TblEle->L3.UshortValue == UsageEle.L3.UshortValue)
                        {
                            CPTblIdx = j;
                            break;
                        }
                    }
                }
            }

            // Didn't find the caching settings in one of the already programmed lookup table entries.
            // Need to add a new lookup table entry.
            if(CPTblIdx == -1)
            {

                {
                    GMM_ASSERTDPF(false, "CRITICAL ERROR: Cache Policy Usage value specified by Client is not defined in Fixed MOCS Table!");

                    CachePolicyError = true;
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support GMM_GEN_MAX_NUMBER_MOCS_INDEXES = %d)",
                    CurrentMaxMocsIndex);

                    // Set cache policy index to uncached.
                    CPTblIdx = StartMocsIdx;
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

#pragma optimize("", on)
//=============================================================================
//
// Function: SetUpMOCSTable
//
// Desc: For Gen12dGPU, Define new Fixed MOCS table
//
// Parameters:
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
void GmmLib::GmmGen12dGPUCachePolicy::SetUpMOCSTable()
{
    GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = &(pGmmGlobalContext->GetCachePolicyTlbElement()[0]);
    CurrentMaxL1HdcMocsIndex                             = 0;
    CurrentMaxSpecialMocsIndex                           = 0;

#define GMM_DEFINE_MOCS(Index, L3_ESC, L3_SCC, L3_CC, _HDCL1)        \
    {                                                                                  \
        pCachePolicyTlbElement[Index].L3.ESC            = L3_ESC;                      \
        pCachePolicyTlbElement[Index].L3.SCC            = L3_SCC;                      \
        pCachePolicyTlbElement[Index].L3.Cacheability   = L3_CC;                       \
        pCachePolicyTlbElement[Index].LeCC.Cacheability = 1;                           \
        pCachePolicyTlbElement[Index].LeCC.TargetCache  = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.LRUM         = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.AOM          = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.ESC          = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.SCC          = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.PFM          = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.SCF          = 0;                           \
        pCachePolicyTlbElement[Index].LeCC.CoS          = CLASS_SERVICE_ZERO;          \
        pCachePolicyTlbElement[Index].LeCC.SelfSnoop    = DISABLE_SELF_SNOOP_OVERRIDE; \
        pCachePolicyTlbElement[Index].HDCL1             = _HDCL1;                      \
    }

    // clang-format off

    if (GFX_GET_CURRENT_PRODUCT(pGmmGlobalContext->GetPlatformInfo().Platform) == IGFX_DG1)
    {
        //Default MOCS Table
        for(int index = 0; index < GMM_MAX_NUMBER_MOCS_INDEXES; index++)
        {     //             Index     ESC	  SCC	  L3CC       HDCL1
             GMM_DEFINE_MOCS( index  , 0     , 0     , 3     ,        0 )
        }
        // Fixed MOCS Table
        //              Index     ESC	  SCC	  L3CC    HDCL1
        GMM_DEFINE_MOCS( 1      , 0     , 0     , 1     , 0 )
        GMM_DEFINE_MOCS( 2      , 0     , 0     , 0     , 0 )
        GMM_DEFINE_MOCS( 3      , 0     , 0     , 0     , 0 )
        GMM_DEFINE_MOCS( 4      , 0     , 0     , 0     , 0 )
        GMM_DEFINE_MOCS( 5      , 0     , 0     , 3     , 0 )
        GMM_DEFINE_MOCS( 6      , 1     , 1     , 3     , 0 )
        GMM_DEFINE_MOCS( 7      , 1     , 3     , 3     , 0 )
        GMM_DEFINE_MOCS( 8      , 1     , 7     , 3     , 0 )
        GMM_DEFINE_MOCS( 48     , 0     , 0     , 3     , 1 )
        GMM_DEFINE_MOCS( 49     , 0     , 0     , 1     , 1 )
        GMM_DEFINE_MOCS( 60     , 0     , 0     , 1     , 0 )
        GMM_DEFINE_MOCS( 61     , 0     , 0     , 1     , 0 )
        GMM_DEFINE_MOCS( 62     , 0     , 0     , 1     , 0 )
        GMM_DEFINE_MOCS( 63     , 0     , 0     , 1     , 0 )

        CurrentMaxMocsIndex         = 8;
        CurrentMaxL1HdcMocsIndex    = 49;
        CurrentMaxSpecialMocsIndex  = 63;

    }

// clang-format on


#undef GMM_DEFINE_MOCS
}
