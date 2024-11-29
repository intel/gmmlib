/*==============================================================================
Copyright(c) 2022 Intel Corporation

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
#include "External/Common/CachePolicy/GmmCachePolicyXe_LPG.h"
//=============================================================================
//
// Function: __:GmmXe_LPGInitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmXe_LPGCachePolicy::InitCachePolicy()
{
    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, l3, l3_scc, go, uclookup, l1cc, l2cc, l4cc, coherency) DEFINE_CP_ELEMENT(usage, 0, 0, l3, 0, 0, 0, 0, l3_scc, 0, 0, 0, 0, 0, 0, go, uclookup, l1cc, l2cc, l4cc, coherency, l3, 0, 0)

#include "GmmXe_LPGCachePolicy.h"

#define L3_UNCACHEABLE (0x1)
#define L3_WB_CACHEABLE (0x3)

#define DISABLE_SKIP_CACHING_CONTROL (0x0)
#define ENABLE_SKIP_CACHING_CONTROL (0x1)

#define ONE_WAY_COHERENT (0x2)


    SetUpMOCSTable();
    SetupPAT();

    // Define index of cache element
    uint32_t Usage           = 0;
    uint32_t ReservedMocsIdx = 10; /* Rsvd MOCS section 10-12 */

#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
    void *pKmdGmmContext = NULL;
#if(defined(__GMM_KMD__))
    pKmdGmmContext = pGmmLibContext->GetGmmKmdContext();
#endif
    OverrideCachePolicy(pKmdGmmContext);
#endif
    // Process the cache policy and fill in the look up table
    for(; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        bool                         CachePolicyError = false;
        int32_t                      CPTblIdx = -1, PATIdx = -1, CoherentPATIdx = -1;
        uint32_t                     i, j, k;
        GMM_CACHE_POLICY_TBL_ELEMENT UsageEle     = {0};
        uint32_t                     StartMocsIdx = 0;
        GMM_L4_CACHING_POLICY        PATCachePolicy;
        GMM_PRIVATE_PAT              UsagePATElement = {0};
        GMM_PTE_CACHE_CONTROL_BITS   PTE             = {0};

        // MOCS data
        {
            //L3
            UsageEle.L3.Reserved = 0; // Reserved bits zeroe'd, this is so we
                                      // we can compare the unioned L3.UshortValue.
            UsageEle.L3.ESC          = DISABLE_SKIP_CACHING_CONTROL;
            UsageEle.L3.SCC          = 0;
            UsageEle.L3.Cacheability = pCachePolicy[Usage].L3 ? L3_WB_CACHEABLE : L3_UNCACHEABLE;


            if(pCachePolicy[Usage].L3 == 0)
            {
                UsageEle.L3.GlobalGo = pCachePolicy[Usage].GlbGo;
            }
            UsageEle.L3.UCLookup = pCachePolicy[Usage].UcLookup;
            __GMM_ASSERT((pCachePolicy[Usage].UcLookup) || (pCachePolicy[Usage].L3 == 0 && pCachePolicy[Usage].UcLookup == 0));


            // L4 Data
            UsageEle.LeCC.Xe_LPG.L4CC = (pCachePolicy[Usage].L4CC - 1) % 4; // coverting indicator values to actual regiser values 0->3(GMM_CP_NON_COHERENT_UC), 2->1(GMM_CP_NON_COHERENT_WT) 1->0(GMM_CP_NON_COHERENT_WB)

            /* Valid MOCS Index starts from 1 */
            for(j = 1; j <= CurrentMaxMocsIndex; j++)
            {
                GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmLibContext->GetCachePolicyTlbElement()[j];
                if((TblEle->L3.UshortValue == UsageEle.L3.UshortValue) &&
                   (TblEle->LeCC.Xe_LPG.L4CC == UsageEle.LeCC.Xe_LPG.L4CC))
                {
                    CPTblIdx = j;
                    break;
                }
            }
            if(CPTblIdx == -1)
            {
#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
                // If the Cache Policy setting is overriden through regkey
                if((pCachePolicy[Usage].IsOverridenByRegkey) && (ReservedMocsIdx < 13)) /* Reserved MOCS 10-12 */
                {
                    /*Use the Reserved Section to add new MOCS settings,*/
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &(pGmmLibContext->GetCachePolicyTlbElement()[ReservedMocsIdx++]);
                    CPTblIdx                             = ReservedMocsIdx;
                    TblEle->L3.UshortValue               = UsageEle.L3.UshortValue;
                    TblEle->LeCC.Xe_LPG.DwordValue       = UsageEle.LeCC.Xe_LPG.DwordValue;
                    GMM_ASSERTDPF(false, "CRITICAL: Cache Policy Usage value for L3/L4 specified by Client is not defined in Fixed MOCS Table and added to reserved MOCS section !!");
                }
                else
#endif
                {
// Log Error using regkey to indicate the above error
#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL) && __GMM_KMD__)
                    REGISTRY_OVERRIDE_WRITE(pKmdGmmContext, Usage, UnSupportedMOCSEntryL3Value, UsageEle.L3.UshortValue);
#endif
                    CachePolicyError = true;
                    GMM_ASSERTDPF(false, "CRITICAL ERROR: Cache Policy Usage value for L3 specified by Client is not defined in Fixed MOCS Table!");
                    // Set cache policy index to default MOCS.
                    CPTblIdx = 0;
                }
            }
        }

        // PAT data
        {
            UsagePATElement.Xe_LPG.Reserved = 0;

            UsagePATElement.Xe_LPG.L4CC = (pCachePolicy[Usage].L4CC - 1) % 4; // coverting indicator values to actual regiser values 0->3(GMM_CP_NON_COHERENT_UC), 2->1(GMM_CP_NON_COHERENT_WT) 1->0(GMM_CP_NON_COHERENT_WB)


            UsagePATElement.Xe_LPG.Coherency = pCachePolicy[Usage].Coherency ? (pCachePolicy[Usage].Coherency + 1) : GMM_GFX_NON_COHERENT_NO_SNOOP; // pCachePolicy[Usage].Coherency -> UsagePATElement.Xe_LPG.Coherency : 0 -> GMM_GFX_NON_COHERENT_NO_SNOOP, 1 -> GMM_GFX_COHERENT_ONE_WAY_IA_SNOOP(2), 2 -> GMM_GFX_COHERENT_TWO_WAY_IA_GPU_SNOOP(3)

            if((UsagePATElement.Xe_LPG.L4CC >= GMM_CP_NON_COHERENT_WT) && (UsagePATElement.Xe_LPG.Coherency >= GMM_GFX_COHERENT_ONE_WAY_IA_SNOOP))
            {
                // restrictions
                // __GMM_ASSERT(FALSE);  // disable assert till there is more clarity on Snoop + UC combination support in PAT register

                // unsupported combination of coherency and L4cachepolicy
                // Promote caching to _WB and support snoop
                UsagePATElement.Xe_LPG.L4CC = GMM_CP_COHERENT_WB;
            }
            // try to find a match in static PAT table
            for(i = 0; i <= CurrentMaxPATIndex; i++)
            {
                GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(i);
                if(UsagePATElement.Xe_LPG.L4CC == PAT.Xe_LPG.L4CC &&
                   UsagePATElement.Xe_LPG.Coherency == PAT.Xe_LPG.Coherency)
                {
                    PATIdx = i;
                    break;
                }
            }

            if(PATIdx == -1)
            {
// Didn't find the caching settings in one of the already programmed PAT table entries.
// Need to add a new lookup table entry.
#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
                // If the Cache Policy setting is overriden through regkey,
                if(pCachePolicy[Usage].IsOverridenByRegkey)
                {
                    if(CurrentMaxPATIndex < NumPATRegisters)
                    {
                        SetPrivatePATEntry(++CurrentMaxPATIndex, UsagePATElement); // updates private PAT table
                        PATIdx = CurrentMaxPATIndex;
                    }
                    else
                    {
                        GMM_ASSERTDPF(
                        "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                        "(we only support NumPATRegisters = %d)",
                        NumPATRegisters - 1);
                        CachePolicyError = true;
                        // add rterror here <ToDo>
                        PATIdx = PAT2; //default to uncached PAT index 2: GMM_CP_NON_COHERENT_UC
                    }
                }
#else
                {
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support NumPATRegisters = %d)",
                    CurrentMaxPATIndex);
                    CachePolicyError = true;
                    // add rterror here <ToDo>
                    PATIdx = PAT2; // default to uncached PAT index 2: GMM_CP_NON_COHERENT_UC
                                   // Log Error using regkey to indicate the above error
#if(_WIN32 && (_DEBUG || _RELEASE_INTERNAL) && __GMM_KMD__)
                    REGISTRY_OVERRIDE_WRITE(pKmdGmmContext, Usage, NewPATCachingPolicy, UsagePATElement.Xe_LPG.L4CC);
                    REGISTRY_OVERRIDE_WRITE(pKmdGmmContext, Usage, NewPATCoherency, UsagePATElement.Xe_LPG.Coherency);
#endif
                }
#endif
            }


            // Find PATIndex matching coherency value of 2 in static PAT table (1 way coherent)
            for(k = 0; k <= CurrentMaxPATIndex; k++)
            {
                GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(k);
                if(UsagePATElement.Xe_LPG.L4CC == PAT.Xe_LPG.L4CC &&
                   (ONE_WAY_COHERENT == PAT.Xe_LPG.Coherency))
                {
                    CoherentPATIdx = k;
                    break;
                }
            }
            if(CoherentPATIdx == -1)
            {
                // redo PAT idnex mathcing with just coherency value,
                // ignore L4 cache setting since MTL is MOCS centric and only the coherency value comes from PAT anyways, caching policy is bound to come from MOCS on MTL
                for(k = 0; k <= CurrentMaxPATIndex; k++)
                {
                    GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(k);
                    if(ONE_WAY_COHERENT == PAT.Xe_LPG.Coherency)
                    {
                        CoherentPATIdx = k;
                        break;
                    }
                }
            }

            if(CoherentPATIdx == -1) // no match, switch to PATIndex
            {
                CachePolicyError = true;
                CoherentPATIdx   = PATIdx;
            }
        }

        pCachePolicy[Usage].PATIndex                          = PATIdx;
        pCachePolicy[Usage].CoherentPATIndex                  = GET_COHERENT_PATINDEX_LOWER_BITS(CoherentPATIdx);
        pCachePolicy[Usage].CoherentPATIndexHigherBit         = GET_COHERENT_PATINDEX_HIGHER_BIT(CoherentPATIdx);
        pCachePolicy[Usage].PTE.DwordValue                    = GMM_GET_PTE_BITS_FROM_PAT_IDX(PATIdx) & 0xFFFFFFFF;
        pCachePolicy[Usage].PTE.HighDwordValue                = GMM_GET_PTE_BITS_FROM_PAT_IDX(PATIdx) >> 32;
        pCachePolicy[Usage].MemoryObjectOverride.XE_LPG.Index = CPTblIdx;
        pCachePolicy[Usage].Override                          = ALWAYS_OVERRIDE;

        if(CachePolicyError)
        {
            GMM_ASSERTDPF(false, "Cache Policy Init Error: Invalid Cache Programming ");
            // add rterror here <ToDo>
        }
    }
    return GMM_SUCCESS;
}

//=============================================================================
//
// Function: __:SetL1CachePolicy
//
// Desc: This function converting indicator values to actual register values and store into pCachePolicy to return to UMD's.
// Gmm not using this values. UMD's queries for this values.
//
// Parameters: Usage
//
// Return: VOID
//
//-----------------------------------------------------------------------------
void GmmLib::GmmXe_LPGCachePolicy::SetL1CachePolicy(uint32_t Usage)
{

// As per HW, L1 cache control(L1CC) values  (0: WBP write bypass mode, 1: 0 uncached, 2: WB Write back, 3:WT write-through, 4: WS Write-Streaming).
#define L1_WBP_CACHEABLE (0x0)
#define L1_UNCACHEABLE   (0x1)
#define L1_WB_CACHEABLE  (0x2)
#define L1_WT_CACHEABLE  (0x3)
#define L1_WS_CACHEABLE  (0x4)

    switch (pCachePolicy[Usage].L1CC)
    {
    case GMM_UC:
        pCachePolicy[Usage].L1CC = L1_UNCACHEABLE;
        break;
    case GMM_WB:
        pCachePolicy[Usage].L1CC = L1_WB_CACHEABLE;
        break;
    case GMM_WT:
        pCachePolicy[Usage].L1CC = L1_WT_CACHEABLE;
        break;
    case GMM_WBP:
        pCachePolicy[Usage].L1CC = L1_WBP_CACHEABLE;
        break;
    case GMM_WS:
        pCachePolicy[Usage].L1CC = L1_WS_CACHEABLE;
        break;
    default:
        pCachePolicy[Usage].L1CC = L1_UNCACHEABLE;
    }

#undef L1_WBP_CACHEABLE
#undef L1_UNCACHEABLE
#undef L1_WB_CACHEABLE
#undef L1_WT_CACHEABLE
#undef L1_WS_CACHEABLE
}
/////////////////////////////////////////////////////////////////////////////////////
///      A simple getter function returning the PAT (cache policy) for a given
///      use Usage of the named resource pResInfo.
///      Typically used to populate PPGTT/GGTT.
///
/// @param[in]     pResInfo: Resource info for resource, can be NULL.
/// @param[in]     Usage: Current usage for resource.
/// @param[Optional] Not Applicable for MTL
/// @param[in]     Applicable for MTL
/// @return        PATIndex
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmXe_LPGCachePolicy::CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable)
{
    __GMM_ASSERT(pGmmLibContext->GetCachePolicyElement(Usage).Initialized);
    GMM_UNREFERENCED_PARAMETER(pCompressionEnable);

    // Prevent wrong Usage for XAdapter resources. UMD does not call GetMemoryObject on shader resources but,
    // when they add it someone could call it without knowing the restriction.
    if(pResInfo &&
       pResInfo->GetResFlags().Info.XAdapter &&
       Usage != GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE)
    {
        __GMM_ASSERT(false);
    }

    if(IsCpuCacheable)
    {
        return (uint32_t)(GET_COHERENT_PATINDEX_VALUE(pGmmLibContext, Usage));
    }
    else
    {
        return pGmmLibContext->GetCachePolicyElement(Usage).PATIndex;
    }
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
void GmmLib::GmmXe_LPGCachePolicy::SetUpMOCSTable()
{
    GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = &(pGmmLibContext->GetCachePolicyTlbElement()[0]);
    CurrentMaxL1HdcMocsIndex                             = 0;
    CurrentMaxSpecialMocsIndex                           = 0;

#define L3_UC (0x1)
#define L3_WB (0x3)

#define L4_WB (0x0)
#define L4_WT (0x1)
#define L4_UC (0x3)

#define GMM_DEFINE_MOCS(Index, L3_LookUp, L3_Go, L3_CC, L4Caching, ignorePAT)      \
    {                                                                              \
        pCachePolicyTlbElement[Index].L3.Cacheability      = L3_CC;                \
        pCachePolicyTlbElement[Index].L3.GlobalGo          = L3_Go;                \
        pCachePolicyTlbElement[Index].L3.UCLookup          = L3_LookUp;            \
        pCachePolicyTlbElement[Index].LeCC.Xe_LPG.L4CC     = L4Caching;            \
        pCachePolicyTlbElement[Index].LeCC.Xe_LPG.igPAT    = ignorePAT;            \
    }
    // clang-format off

    //Default MOCS Table
    for(int index = 0; index < GMM_MAX_NUMBER_MOCS_INDEXES; index++)
    {     //             Index    LookUp  Go      L3CC        L4CC   ignorePAT
        GMM_DEFINE_MOCS( index  , 1     , 0     , L3_UC     , L4_UC     , 0)
    }
    // Fixed MOCS Table
    //             Index      LookUp  Go      L3CC       L4CC   ignorePAT
    GMM_DEFINE_MOCS( 0      , 1     , 0     , L3_WB    , L4_WB , 0)
    GMM_DEFINE_MOCS( 1      , 1     , 0     , L3_WB    , L4_WB , 1)
    GMM_DEFINE_MOCS( 2      , 1     , 0     , L3_UC    , L4_WB , 1)
    GMM_DEFINE_MOCS( 3      , 1     , 0     , L3_UC    , L4_UC , 1)
    GMM_DEFINE_MOCS( 4      , 1     , 1     , L3_UC    , L4_WB , 1)
    GMM_DEFINE_MOCS( 5      , 1     , 1     , L3_UC    , L4_UC , 1)
    GMM_DEFINE_MOCS( 6      , 0     , 0     , L3_UC    , L4_WB , 1)
    GMM_DEFINE_MOCS( 7      , 0     , 0     , L3_UC    , L4_UC , 1)
    GMM_DEFINE_MOCS( 8      , 0     , 1     , L3_UC    , L4_WB , 1)
    GMM_DEFINE_MOCS( 9      , 0     , 1     , L3_UC    , L4_UC , 1)
    //Reserved 10-13
    GMM_DEFINE_MOCS( 14     , 1     , 0     , L3_WB    , L4_WT , 1) /* Note Update GMM_CC_DISP_MOCS_INDEX*/
    GMM_DEFINE_MOCS( 15     , 0     , 1     , L3_UC    , L4_WB , 1) /* Note Update GMM_NON_CC_DISP_MOCS_INDEX */

    CurrentMaxMocsIndex         = 15;
    CurrentMaxL1HdcMocsIndex    = 0;
    CurrentMaxSpecialMocsIndex  = 0;

// clang-format on
#undef GMM_DEFINE_MOCS
#undef L4_WB
#undef L4_WT
#undef L4_UC
#undef L3_UC
#undef L3_WB
}

//=============================================================================
//
// Function: SetupPAT
//
// Desc:
//
// Parameters:
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmXe_LPGCachePolicy::SetupPAT()
{
    GMM_PRIVATE_PAT *pPATTlbElement = &(pGmmLibContext->GetPrivatePATTable()[0]);

#define L4_WB (0x0)
#define L4_WT (0x1)
#define L4_UC (0x3)

#define GMM_DEFINE_PAT_ELEMENT(indx, L4Caching, Coh)       \
    {                                                      \
        pPATTlbElement[indx].Xe_LPG.Coherency = Coh;       \
        pPATTlbElement[indx].Xe_LPG.L4CC      = L4Caching; \
        pPATTlbElement[indx].Xe_LPG.Reserved  = 0;         \
    }

    // clang-format off

    // Default PAT Table
    for (uint32_t i = 0; i < NumPATRegisters; i++)
    {   //                      Index      CachingPolicy       Coherency
        GMM_DEFINE_PAT_ELEMENT( i,          L4_UC              , 0 );
    }

    // Fixed PAT Table
    //                      Index   CachingPolicy   Coherency
    GMM_DEFINE_PAT_ELEMENT( 0      , L4_WB              , 0)    // PATRegValue = 0x0
    GMM_DEFINE_PAT_ELEMENT( 1      , L4_WT              , 0)    // PATRegValue = 0x4
    GMM_DEFINE_PAT_ELEMENT( 2      , L4_UC              , 0)    // PATRegValue = 0xC
    GMM_DEFINE_PAT_ELEMENT( 3      , L4_WB              , 2)    // PATRegValue = 0x2
    GMM_DEFINE_PAT_ELEMENT( 4      , L4_WB              , 3)    // PATRegValue = 0x3

    CurrentMaxPATIndex = 4;

// clang-format on
#undef GMM_DEFINE_PAT
#undef L4_WB
#undef L4_WT
#undef L4_UC
    return GMM_SUCCESS;
}

uint32_t GMM_STDCALL GmmLib::GmmXe_LPGCachePolicy::GetSurfaceStateL1CachePolicy(GMM_RESOURCE_USAGE_TYPE Usage)
{
    __GMM_ASSERT(pCachePolicy[Usage].Initialized);

    return pCachePolicy[Usage].L1CC;
}

/////////////////////////////////////////////////////////////////////////////////////
///      A simple getter function returning the MOCS (cache policy) for a given
///      use Usage of the named resource pResInfo.
///      Typically used to populate a SURFACE_STATE for a GPU task.
///
/// @param[in]     pResInfo: Resource info for resource, can be NULL.
/// @param[in]     Usage: Current usage for resource.
///
/// @return        MEMORY_OBJECT_CONTROL_STATE: Gen adjusted MOCS structure (cache
///                                             policy) for the given buffer use.
/////////////////////////////////////////////////////////////////////////////////////
MEMORY_OBJECT_CONTROL_STATE GMM_STDCALL GmmLib::GmmXe_LPGCachePolicy::CachePolicyGetMemoryObject(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage)
{
    __GMM_ASSERT(pCachePolicy[Usage].Initialized);

    // Prevent wrong Usage for XAdapter resources. UMD does not call GetMemoryObject on shader resources but,
    // when they add it someone could call it without knowing the restriction.
    if (pResInfo &&
        pResInfo->GetResFlags().Info.XAdapter &&
         (Usage != GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE))
    {
        __GMM_ASSERT(false);
    }

    if (!pResInfo ||
        (pCachePolicy[Usage].Override & pCachePolicy[Usage].IDCode) ||
        (pCachePolicy[Usage].Override == ALWAYS_OVERRIDE))
    {
        return pCachePolicy[Usage].MemoryObjectOverride;
    }
    else
    {
        return pCachePolicy[Usage].MemoryObjectNoOverride;
    }
}

