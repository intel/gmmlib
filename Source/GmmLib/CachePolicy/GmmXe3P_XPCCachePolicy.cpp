/*==============================================================================
Copyright(c) 2025 Intel Corporation
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
#include "External/Common/CachePolicy/GmmCachePolicyXe3P_XPC.h"

//=============================================================================
//
// Function: __:GmmXe3P_XPCInitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmXe3P_XPCCachePolicy::InitCachePolicy()
{
    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);
#define DEFINE_CACHE_ELEMENT(usage, l3_cc, l3_clos, l1cc, l2cc, l4cc, coherency, igPAT, segov) DEFINE_CP_ELEMENT(usage, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, segov, 0, 0, l1cc, l2cc, l4cc, coherency, l3_cc, l3_clos, igPAT)

#include "GmmXe3P_XPCCachePolicy.h"

    SetUpMOCSTable();
    SetupPAT();

    // Define index of cache element
    uint32_t Usage             = 0;
    uint32_t ReservedPATIdx    = 11; /* Rsvd PAT section 11-22 */
    uint32_t ReservedPATIdxEnd = 22;

#if (_WIN32 && (_DEBUG || _RELEASE_INTERNAL))
    void *pKmdGmmContext = NULL;
#if (defined(__GMM_KMD__))
    pKmdGmmContext = pGmmLibContext->GetGmmKmdContext();
#endif
    OverrideCachePolicy(pKmdGmmContext);
#endif
    // Process the cache policy and fill in the look up table
    for (; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
    {
        bool                         CachePolicyError = false;
        int32_t                      PATIdx = -1, CPTblIdx = -1, PATIdxCompressed = -1, CoherentPATIdx = -1;
        uint32_t                     i, j;
        GMM_XE3P_PRIVATE_PAT         UsagePATElement = {0};
        GMM_CACHE_POLICY_TBL_ELEMENT UsageEle        = {0};
        GMM_PTE_CACHE_CONTROL_BITS   PTE             = {0};

        // MOCS data
        {
            // Get L3 ,L4 and Convert  GMM indicative values to actual regiser values.
            GetL3L4(&UsageEle, &UsagePATElement, Usage);
            // Convert L1  GMM indicative values to actual regiser values and store into pCachePolicy to return to UMD's.
            SetL1CachePolicy(Usage);

            /* If MOCS is not needed fall back to Defer to PAT i.e MOCS#0 */
            if (false == UsageEle.L3.PhysicalL3.igPAT)
            {
                /* Set cache policy index to defered to PAT i.e. MOCS Index 0 */
                CPTblIdx = 0;
            }
            else
            {
                /* MOCS Index 1-3 are valid */
                for (j = 1; j <= CurrentMaxMocsIndex; j++)
                {
                    GMM_CACHE_POLICY_TBL_ELEMENT *TblEle = &pGmmLibContext->GetCachePolicyTlbElement()[j];
                    if (UsageEle.L3.PhysicalL3.L4CC == TblEle->L3.PhysicalL3.L4CC &&
                        UsageEle.L3.PhysicalL3.L3CC == TblEle->L3.PhysicalL3.L3CC &&
                        UsageEle.L3.PhysicalL3.L3CLOS == TblEle->L3.PhysicalL3.L3CLOS &&
                        UsageEle.L3.PhysicalL3.igPAT == true)
                    {
                        CPTblIdx = j;
                        break;
                    }
                }
            }

            if (CPTblIdx == -1)
            {
                {
                    /* Invalid MOCS setting Fail the GMM Initialzation */
                    GMM_ASSERTDPF(false, "CRITICAL: Cache Policy Usage value for L3/L4 specified by Client is not defined in Fixed MOCS Table");
                    CachePolicyError = true;
                }
            }
        }


        // PAT data
        {

            for (i = 0; i <= CurrentMaxPATIndex; i++)
            {
                GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(i);
                if (UsagePATElement.Xe3P.L4CC == PAT.Xe2.L4CC &&
                    UsagePATElement.Xe3P.Coherency == PAT.Xe2.Coherency &&
                    UsagePATElement.Xe3P.L3CC == PAT.Xe2.L3CC &&
                    UsagePATElement.Xe3P.L3CLOS == PAT.Xe2.L3CLOS &&
                    false == PAT.Xe2.LosslessCompressionEn)
                {
                    PATIdx = i;
                    break;
                }
            }

            // Compression is not supported in PAT table, compressed PAT to be same as uncompressed PAT
            PATIdxCompressed = PATIdx;

            if (PATIdx == -1)
            {
// Didn't find the caching settings in one of the already programmed PAT table entries.
// Need to add a new lookup table entry.
                    GMM_ASSERTDPF(
                    "Cache Policy Init Error: Invalid Cache Programming, too many unique caching combinations"
                    "(we only support NumPATRegisters = %d)",
                    CurrentMaxPATIndex);
                    CachePolicyError = true;

                    PATIdx = GMM_PAT_ERROR;
            }

            /* Find a PATIndex for a coherent uncompressed case, if usage is 2-way or 1-way already, take that, otherwise search for oneway*/
            if ((UsagePATElement.Xe3P.Coherency == GMM_GFX_PHY_COHERENT_ONE_WAY_IA_SNOOP) ||
                (UsagePATElement.Xe3P.Coherency == GMM_GFX_PHY_COHERENT_TWO_WAY_IA_GPU_SNOOP))
            {
                //Already coherent
                CoherentPATIdx = PATIdx;
            }
            else
            {
                // search for equivalent one way coherent index
                for (i = 0; i <= CurrentMaxPATIndex; i++)
                {
                    GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(i);
                    if (UsagePATElement.Xe3P.L4CC == PAT.Xe2.L4CC &&
                        UsagePATElement.Xe3P.L3CC == PAT.Xe2.L3CC &&
                        UsagePATElement.Xe3P.L3CLOS == PAT.Xe2.L3CLOS &&
                        GMM_GFX_PHY_COHERENT_ONE_WAY_IA_SNOOP == PAT.Xe2.Coherency)
                    {
                        if ((false == PAT.Xe2.LosslessCompressionEn) && (CoherentPATIdx == -1))
                        {
                            CoherentPATIdx = i;
                        }
                        if (CoherentPATIdx != -1)
                        {
                            break;
                        }
                    }
                }
                if (CoherentPATIdx == -1)
                {
                    //redo matching based on L3:UC, L4:UC, we should find one
                    for (i = 0; i <= CurrentMaxPATIndex; i++)
                    {
                        GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(i);
                        if (GMM_GFX_PHY_L4_MT_UC == PAT.Xe2.L4CC &&
                            GMM_GFX_PHY_L3_MT_UC == PAT.Xe2.L3CC &&
                            UsagePATElement.Xe3P.L3CLOS == PAT.Xe2.L3CLOS &&
                            GMM_GFX_PHY_COHERENT_ONE_WAY_IA_SNOOP == PAT.Xe2.Coherency)
                        {
                            if ((false == PAT.Xe2.LosslessCompressionEn) && (CoherentPATIdx == -1))
                            {
                                CoherentPATIdx = i;
                            }

                            if (CoherentPATIdx != -1)
                            {
                                break;
                            }
                        }
                    }
                }
            }
        }

        pCachePolicy[Usage].PATIndex                                 = PATIdx;
        pCachePolicy[Usage].CoherentPATIndex                         = GET_COHERENT_PATINDEX_LOWER_BITS(CoherentPATIdx); // Coherent uncompressed lower bits
        pCachePolicy[Usage].CoherentPATIndexHigherBit                = GET_COHERENT_PATINDEX_HIGHER_BIT(CoherentPATIdx); // Coherent uncompressed higher bits
        pCachePolicy[Usage].PATIndexCompressed                       = PATIdxCompressed;
        pCachePolicy[Usage].PTE.DwordValue                           = GMM_GET_PTE_BITS_FROM_PAT_IDX(PATIdx) & 0xFFFFFFFF;
        pCachePolicy[Usage].PTE.HighDwordValue                       = GMM_GET_PTE_BITS_FROM_PAT_IDX(PATIdx) >> 32;
        pCachePolicy[Usage].MemoryObjectOverride.XE_HP.Index         = CPTblIdx;
        pCachePolicy[Usage].MemoryObjectOverride.XE_HP.EncryptedData = 0;
        pCachePolicy[Usage].Override                                 = ALWAYS_OVERRIDE;

        //printf("Usage: [%d], PAT: [%d], Comp:[%d], MOCSIdx:[%d] \n", Usage, PATIdx, PATIdxCompressed, CPTblIdx);

        if (CachePolicyError)
        {
            GMM_ASSERTDPF(false, "Cache Policy Init Error: Invalid Cache Programming ");
            return GMM_INVALIDPARAM;
        }
    }
    return GMM_SUCCESS;
}

//=============================================================================
//
// Function: __:GetL3L4
//
// Desc: This function converts GMM indicative values to actual register values
//
// Parameters:
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
void GmmLib::GmmXe3P_XPCCachePolicy::GetL3L4(GMM_CACHE_POLICY_TBL_ELEMENT *pUsageEle, GMM_XE3P_PRIVATE_PAT *pUsagePATElement, uint32_t Usage)
{

    //MOCS
    pUsageEle->L3.PhysicalL3.Reserved0 = pUsageEle->L3.PhysicalL3.Reserved = 0;
    //L3CLOS
    pUsageEle->L3.PhysicalL3.L3CLOS = 0;
    //IgPAT
    pUsageEle->L3.PhysicalL3.igPAT = pCachePolicy[Usage].IgnorePAT;


    //PAT
    pUsagePATElement->Xe3P.Reserved1 = 0;
    pUsagePATElement->Xe3P.Reserved2 = 0;

    pUsagePATElement->Xe3P.L3CLOS = 0;
    
    switch (pCachePolicy[Usage].L3CC)
    {
    case GMM_UC:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_UC;
        pUsagePATElement->Xe3P.L3CC   = GMM_GFX_PHY_L3_MT_UC;
        break;
    case GMM_WB:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_WB;
        pUsagePATElement->Xe3P.L3CC   = GMM_GFX_PHY_L3_MT_WB;
        break;
    default:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_UC;
        pUsagePATElement->Xe3P.L3CC   = GMM_GFX_PHY_L3_MT_UC;
    }

    switch (pCachePolicy[Usage].L4CC)
    {
    case GMM_UC:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_UC;
        pUsagePATElement->Xe3P.L4CC   = GMM_GFX_PHY_L4_MT_UC;
        break;
    case GMM_WB:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_WB;
        pUsagePATElement->Xe3P.L4CC   = GMM_GFX_PHY_L4_MT_WB;
        break;
    default:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_UC;
        pUsagePATElement->Xe3P.L4CC   = GMM_GFX_PHY_L4_MT_UC;
    }

    switch (pCachePolicy[Usage].Coherency)
    {
    case GMM_NON_COHERENT_NO_SNOOP:
        pUsagePATElement->Xe3P.Coherency = GMM_GFX_NON_COHERENT_NO_SNOOP;
        break;
    case GMM_COHERENT_ONE_WAY_IA_SNOOP:
        pUsagePATElement->Xe3P.Coherency = GMM_GFX_COHERENT_ONE_WAY_IA_SNOOP;
        break;
    case GMM_COHERENT_TWO_WAY_IA_GPU_SNOOP:
        pUsagePATElement->Xe3P.Coherency = GMM_GFX_COHERENT_TWO_WAY_IA_GPU_SNOOP;
        break;
    default:
        pUsagePATElement->Xe3P.Coherency = GMM_GFX_NON_COHERENT_NO_SNOOP;
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///      A simple getter function returning the PAT (cache policy) for a given
///      use Usage of the named resource pResInfo.
///      Typically used to populate PPGTT/GGTT.
///
/// @param[in]     pResInfo: Resource info for resource, can be NULL.
/// @param[in]     Usage: Current usage for resource.
/// @param[Optional]    Usage: for Xe3P-XPC compression parameter
///
/// @return        PATIndex
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmXe3P_XPCCachePolicy::CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable)
{
    __GMM_ASSERT(pGmmLibContext->GetCachePolicyElement(Usage).Initialized);

    uint32_t                 PATIndex             = pGmmLibContext->GetCachePolicyElement(Usage).PATIndex;
    GMM_CACHE_POLICY_ELEMENT TempElement          = pGmmLibContext->GetCachePolicyElement(Usage);
    uint32_t                 TempCoherentPATIndex = 0;

    // This is to check if PATIndexCompressed, CoherentPATIndex are valid
    // Increment by 1 to have the rollover and value resets to 0 if the PAT in not valid.
    TempElement.PATIndexCompressed += 1;
    TempCoherentPATIndex = (uint32_t)GET_COHERENT_PATINDEX_VALUE(pGmmLibContext, Usage);

    // Higher bit of CoherentPATIndex would tell us if its a valid or not.0--> valid, 1-->invalid
    uint32_t CoherentPATIndex = (uint32_t)((GET_COHERENT_PATINDEX_HIGHER_BIT(TempCoherentPATIndex) == 1) ? GMM_PAT_ERROR : GET_COHERENT_PATINDEX_VALUE(pGmmLibContext, Usage));
    //For PATIndexCompressed, rollover value would be 0 if its invalid
    uint32_t PATIndexCompressed = (uint32_t)(TempElement.PATIndexCompressed == 0 ? GMM_PAT_ERROR : pGmmLibContext->GetCachePolicyElement(Usage).PATIndexCompressed);
    uint32_t ReturnPATIndex     = GMM_PAT_ERROR;
    bool     CompressionEnable  = (pCompressionEnable) ? *pCompressionEnable : false;

    // Prevent wrong Usage for XAdapter resources. UMD does not call GetMemoryObject on shader resources but,
    // when they add it someone could call it without knowing the restriction.
    if (pResInfo &&
        pResInfo->GetResFlags().Info.XAdapter &&
        (Usage != GMM_RESOURCE_USAGE_XADAPTER_SHARED_RESOURCE))
    {
        __GMM_ASSERT(false);
    }

    // requested compressed and coherent
    if (CompressionEnable && IsCpuCacheable)
    {
        // return coherent uncompressed
        ReturnPATIndex    = CoherentPATIndex;
        CompressionEnable = false;
        GMM_ASSERTDPF(false, "Coherent Compressed is not supported. However, respecting the coherency and returning CoherentPATIndex");
    }
    // requested compressed only
    else if (CompressionEnable)
    {

        if (GMM_PAT_ERROR != PATIndexCompressed)
        {
            // return compresed, may or may not coherent which depends on orinigal usage
            ReturnPATIndex    = PATIndexCompressed;
            CompressionEnable = true;
        }
        else
        {
            // return original index
            ReturnPATIndex    = PATIndex;
            CompressionEnable = false;
        }
    }
    // requested coherent only
    else if (IsCpuCacheable)
    {
        //return coherent uncompressed
        ReturnPATIndex    = CoherentPATIndex;
        CompressionEnable = false;
    }
    /* Requested UnCompressed PAT */
    else
    {
        if (GMM_PAT_ERROR != PATIndex)
        {
            ReturnPATIndex    = PATIndex;
            CompressionEnable = false;
        }
    }

    /* No valid PAT Index found */
    if (GMM_PAT_ERROR == ReturnPATIndex)
    {
        ReturnPATIndex    = GMM_XE2_DEFAULT_PAT_INDEX; //default to uncached PAT index 2: GMM_CP_NON_COHERENT_UC
        CompressionEnable = false;
        __GMM_ASSERT(false);
    }

    if (pCompressionEnable)
    {
        *pCompressionEnable = CompressionEnable;
    }

    return ReturnPATIndex;
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
void GmmLib::GmmXe3P_XPCCachePolicy::SetUpMOCSTable()
{
    GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = &(pGmmLibContext->GetCachePolicyTlbElement()[0]);

#define L4_WB (0x0)
#define L4_UC (0x3)

#define L3_WB (0x0)
#define L3_UC (0x3)

#define GMM_DEFINE_MOCS(indx, L4Caching, L3Caching, L3ClassOfService, ignorePAT) \
    {                                                                            \
        pCachePolicyTlbElement[indx].L3.PhysicalL3.L4CC      = L4Caching;        \
        pCachePolicyTlbElement[indx].L3.PhysicalL3.Reserved0 = 0;                \
        pCachePolicyTlbElement[indx].L3.PhysicalL3.L3CC      = L3Caching;        \
        pCachePolicyTlbElement[indx].L3.PhysicalL3.L3CLOS    = L3ClassOfService; \
        pCachePolicyTlbElement[indx].L3.PhysicalL3.igPAT     = ignorePAT;        \
    }

    // clang-format off
    // Default MOCS Table
    for(uint32_t j = 0; j < GMM_XE3P_NUM_MOCS_ENTRIES; j++)
    {   //               Index            CachingPolicy   L3Caching      L3ClassOfService    ignorePAT
        GMM_DEFINE_MOCS( j,               L4_UC,          L3_UC,             0          ,     0  )
    }

    //             Index    L4 CachingPolicy   L3 CachingPolicy   L3 CLOS   ignorePAT
    GMM_DEFINE_MOCS( 0      , L4_UC              , L3_WB           , 0     , 0)   // Defer to PAT
    GMM_DEFINE_MOCS( 1      , L4_UC              , L3_UC           , 0     , 1)   // UC
    GMM_DEFINE_MOCS( 2      , L4_UC              , L3_WB           , 0     , 1)   // L3
    GMM_DEFINE_MOCS( 3      , L4_WB              , L3_UC           , 0     , 1)   // L4
    GMM_DEFINE_MOCS( 4      , L4_WB              , L3_WB           , 0     , 1)   // L3+L4

    CurrentMaxMocsIndex = 4;
    CurrentMaxL1HdcMocsIndex   = 0;
    CurrentMaxSpecialMocsIndex = 0;
    // clang-format on

#undef GMM_DEFINE_MOCS
#undef L4_WB
#undef L4_UC

#undef L3_WB
#undef L3_UC
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
GMM_STATUS GmmLib::GmmXe3P_XPCCachePolicy::SetupPAT()
{
    GMM_PRIVATE_PAT *pPATTlbElement = &(pGmmLibContext->GetPrivatePATTable()[0]);

#define L4_WB (0x0)
#define L4_UC (0x3)

#define L3_WB (0x0)
#define L3_UC (0x3)

#define GMM_DEFINE_PAT_ELEMENT(indx, Coh, L4Caching, L3Caching, L3ClassOfService, NoCachePromote) \
    {                                                                                             \
        pPATTlbElement[indx].Xe2.Coherency             = Coh;                                     \
        pPATTlbElement[indx].Xe2.L4CC                  = L4Caching;                               \
        pPATTlbElement[indx].Xe2.Reserved1             = 0;                                       \
        pPATTlbElement[indx].Xe2.Reserved2             = 0;                                       \
        pPATTlbElement[indx].Xe2.L3CC                  = L3Caching;                               \
        pPATTlbElement[indx].Xe2.L3CLOS                = L3ClassOfService;                        \
        pPATTlbElement[indx].Xe2.LosslessCompressionEn = 0;                                       \
        pPATTlbElement[indx].Xe2.NoCachingPromote      = NoCachePromote;                          \
    }

    // clang-format off

    // Default PAT Table
    // 32 nos
    for (uint32_t i = 0; i < (NumPATRegisters); i++)
    {   //                      Index  Coherency  CachingPolicy  L3Caching  L3ClassOfService  NoCachingPromote
        GMM_DEFINE_PAT_ELEMENT( i,     3,         L4_UC,         L3_UC,     0,                0);
    }

    // Fixed PAT Table
    //                      Index  Coherency  L4 CachingPolicy   L3 CachingPolicy   L3 CLOS      NoCachingPromote
    //Group: GGT/PPGTT[4]
    GMM_DEFINE_PAT_ELEMENT( 0      , 0      , L4_WB              , L3_WB           , 0           , 0)    //          | L3_WB | L4_WB
    GMM_DEFINE_PAT_ELEMENT( 1      , 2      , L4_WB              , L3_WB           , 0           , 0)    //          | L3_WB | L4_WB | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 2      , 3      , L4_WB              , L3_WB           , 0           , 0)    //          | L3_WB | L4_WB | 2 way coherent
    GMM_DEFINE_PAT_ELEMENT( 3      , 0      , L4_UC              , L3_UC           , 0           , 0)    //          | UC   
    //Group: Other UC
    GMM_DEFINE_PAT_ELEMENT( 4      , 2      , L4_UC              , L3_UC           , 0           , 0)    //          | UC | 1 way coherent
    //Group L4 only
    GMM_DEFINE_PAT_ELEMENT( 5      , 0      , L4_WB              , L3_UC           , 0           , 0)    //          | L4_WB
    GMM_DEFINE_PAT_ELEMENT( 6      , 2      , L4_WB              , L3_UC           , 0           , 0)    //          | L4_WB | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 7      , 3      , L4_WB              , L3_UC           , 0           , 0)    //          | L4_WB | 2 way coherent
    //Group L3 only
    GMM_DEFINE_PAT_ELEMENT( 8      , 0      , L4_UC              , L3_WB           , 0           , 0)    //          | L3_WB  
    GMM_DEFINE_PAT_ELEMENT( 9      , 2      , L4_UC              , L3_WB           , 0           , 0)    //          | L3_WB | 1 way coherent 
    GMM_DEFINE_PAT_ELEMENT( 10     , 3      , L4_UC              , L3_WB           , 0           , 0)    //          | L3_WB | 2 way coherent
    // Reserved 11 - 22
    //Group: CLOS1
    GMM_DEFINE_PAT_ELEMENT( 23     , 0      , L4_WB              , L3_WB           , 1           , 0)    //          | L3_WB | L4_WB  
    GMM_DEFINE_PAT_ELEMENT( 24     , 2      , L4_WB              , L3_WB           , 1           , 0)    //          | L3_WB | L4_WB | 1 way coherent  
    GMM_DEFINE_PAT_ELEMENT( 25     , 3      , L4_WB              , L3_WB           , 1           , 0)    //          | L3_WB | L4_WB | 2 way coherent 
    //Group: CLOS2
    GMM_DEFINE_PAT_ELEMENT( 26     , 0      , L4_WB              , L3_WB           , 2           , 0)    //          | L3_WB | L4_WB  
    GMM_DEFINE_PAT_ELEMENT( 27     , 2      , L4_WB              , L3_WB           , 2           , 0)    //          | L3_WB | L4_WB | 1 way coherent  
    GMM_DEFINE_PAT_ELEMENT( 28     , 3      , L4_WB              , L3_WB           , 2           , 0)    //          | L3_WB | L4_WB | 2 way coherent
    //Group: CLOS3
    GMM_DEFINE_PAT_ELEMENT( 29     , 0      , L4_WB              , L3_WB           , 3           , 0)    //          | L3_WB | L4_WB  
    GMM_DEFINE_PAT_ELEMENT( 30     , 2      , L4_WB              , L3_WB           , 3           , 0)    //          | L3_WB | L4_WB | 1 way coherent  
    GMM_DEFINE_PAT_ELEMENT( 31     , 3      , L4_WB              , L3_WB           , 3           , 0)    //          | L3_WB | L4_WB | 2 way coherent
    
    CurrentMaxPATIndex = 31;

// clang-format on
#undef GMM_DEFINE_PAT
#undef L4_WB
#undef L4_UC

#undef L3_WB
#undef L3_UC
    return GMM_SUCCESS;
}
