/*==============================================================================
Copyright(c) 2024 Intel Corporation
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
#include "External/Common/CachePolicy/GmmCachePolicyXe2_LPG.h"
//=============================================================================
//
// Function: GmmXe2_LPGCachePolicy::InitCachePolicy()
//
// Desc: This function initializes the Xe2 cache policy
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmXe2_LPGCachePolicy::InitCachePolicy()
{
    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, l3_cc, l3_clos, l1cc, l2cc, l4cc, coherency, igPAT, segov) DEFINE_CP_ELEMENT(usage, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, segov, 0, 0, l1cc, l2cc, l4cc, coherency, l3_cc, l3_clos, igPAT)

#include "GmmXe2_LPGCachePolicy.h"

    SetUpMOCSTable();
    SetupPAT();

    // Define index of cache element
    uint32_t Usage          = 0;
    uint32_t ReservedPATIdx = 16; /* Rsvd PAT section 16-19 */
    uint32_t ReservedPATIdxEnd = 20;

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
        GMM_XE2_PRIVATE_PAT          UsagePATElement = {0};
        GMM_CACHE_POLICY_TBL_ELEMENT UsageEle        = {0};
        GMM_PTE_CACHE_CONTROL_BITS   PTE             = {0};

        // MOCS data
        {

            // Get L3 ,L4 and Convert  GMM indicative values to actual regiser values.
            GetL3L4(&UsageEle, &UsagePATElement, Usage);
            // Convert L1  GMM indicative values to actual regiser values and store into pCachePolicy to return to UMD's.
            SetL1CachePolicy(Usage);

            if ((!pGmmLibContext->GetSkuTable().FtrL3TransientDataFlush) && (UsageEle.L3.PhysicalL3.L3CC == GMM_GFX_PHY_L3_MT_WB_XD))
            {
                UsageEle.L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_WB; // No Transient Flush Support
            }

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

        /*
            Validate Caching restrictions as below
            1. MemoryType WB-XD must be used in Non-Coherent and allowed only for displayable surfaces
            2. Coherent mode(1-way/2-way) must be Memory Type WB
            3. No 2-way coherency on dGPU
            4. Memory Type WT is available only for L4 in Non Coherent Mode
            5. Memory Type UC must be used in Non-Coherent Mode
        */

        // PAT data
        {
            if (!pGmmLibContext->GetSkuTable().FtrL3TransientDataFlush && (UsagePATElement.Xe2.L3CC == GMM_GFX_PHY_L3_MT_WB_XD))
            {
                UsagePATElement.Xe2.L3CC = GMM_GFX_PHY_L3_MT_WB; // No Transient Flush Support
            }

            /* Find a PATIndex from the PAT table for uncompressed case*/
            if ((UsagePATElement.Xe2.L4CC == GMM_GFX_PHY_L4_MT_WT) && (UsagePATElement.Xe2.L3CC == GMM_GFX_PHY_L3_MT_WB_XD))
            {

                // With L3:XD, L4:WT, NC combination
                if (pGmmLibContext->GetSkuTable().FtrDiscrete)
                {
                    // On BMG, L4 is a pass through, demote L4 to UC, keep L3 at XD
                    PATIdx = PAT6;
                }
                else
                {
                    // On LNL, L3:XD is not needed
                    PATIdx = PAT13;
                }
            }
            else
            {
                for (i = 0; i <= CurrentMaxPATIndex; i++)
                {
                    GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(i);
                    if (UsagePATElement.Xe2.L4CC == PAT.Xe2.L4CC &&
                        UsagePATElement.Xe2.Coherency == PAT.Xe2.Coherency &&
                        UsagePATElement.Xe2.L3CC == PAT.Xe2.L3CC &&
                        UsagePATElement.Xe2.L3CLOS == PAT.Xe2.L3CLOS &&
                        false == PAT.Xe2.LosslessCompressionEn)
                    {
                        PATIdx = i;
                        break;
                    }
                }
            }

            /* Find a PATIndex from the PAT table for compressed case*/
            for (i = 0; i <= CurrentMaxPATIndex; i++)
            {
                GMM_PRIVATE_PAT PAT = GetPrivatePATEntry(i);
                if (UsagePATElement.Xe2.L4CC == PAT.Xe2.L4CC &&
                    UsagePATElement.Xe2.Coherency == PAT.Xe2.Coherency &&
                    UsagePATElement.Xe2.L3CC == PAT.Xe2.L3CC &&
                    UsagePATElement.Xe2.L3CLOS == PAT.Xe2.L3CLOS &&
                    true == PAT.Xe2.LosslessCompressionEn)
                {
                    PATIdxCompressed = i;
                    break;
                }
            }

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
            if ((UsagePATElement.Xe2.Coherency == GMM_GFX_PHY_COHERENT_ONE_WAY_IA_SNOOP) ||
                (UsagePATElement.Xe2.Coherency == GMM_GFX_PHY_COHERENT_TWO_WAY_IA_GPU_SNOOP))
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
                    if (UsagePATElement.Xe2.L4CC == PAT.Xe2.L4CC &&
                        UsagePATElement.Xe2.L3CC == PAT.Xe2.L3CC &&
                        UsagePATElement.Xe2.L3CLOS == PAT.Xe2.L3CLOS &&
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
                            UsagePATElement.Xe2.L3CLOS == PAT.Xe2.L3CLOS &&
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
// Desc: This function // converting  GMM indicative values to actual register values
//
// Parameters:
//
// Return: GMM_STATUS
//
//-----------------------------------------------------------------------------

void GmmLib::GmmXe2_LPGCachePolicy::GetL3L4(GMM_CACHE_POLICY_TBL_ELEMENT *pUsageEle, GMM_XE2_PRIVATE_PAT *pUsagePATElement, uint32_t Usage)
{

    //MOCS
    pUsageEle->L3.PhysicalL3.Reserved0 = pUsageEle->L3.PhysicalL3.Reserved = 0;
    //L3CLOS
    pUsageEle->L3.PhysicalL3.L3CLOS = 0; 
    //IgPAT
    pUsageEle->L3.PhysicalL3.igPAT = pCachePolicy[Usage].IgnorePAT;


    //PAT
    pUsagePATElement->Xe2.Reserved1 = 0;
    pUsagePATElement->Xe2.Reserved2 = 0;

    pUsagePATElement->Xe2.L3CLOS = 0; 
    switch (pCachePolicy[Usage].L3CC)
    {
    case GMM_UC:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_UC;
        pUsagePATElement->Xe2.L3CC    = GMM_GFX_PHY_L3_MT_UC;
        break;
    case GMM_WB:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_WB;
        pUsagePATElement->Xe2.L3CC    = GMM_GFX_PHY_L3_MT_WB;
        break;
    case GMM_WBTD:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_WB_XD; // Transient:Display on Xe2
        pUsagePATElement->Xe2.L3CC    = GMM_GFX_PHY_L3_MT_WB_XD;
        break;
    default:
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_UC;
        pUsagePATElement->Xe2.L3CC    = GMM_GFX_PHY_L3_MT_UC;
    }

    switch (pCachePolicy[Usage].L4CC)
    {
    case GMM_UC:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_UC;
        pUsagePATElement->Xe2.L4CC    = GMM_GFX_PHY_L4_MT_UC;
        break;
    case GMM_WB:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_WB;
        pUsagePATElement->Xe2.L4CC    = GMM_GFX_PHY_L4_MT_WB;
        break;
    case GMM_WT:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_WT;
        pUsagePATElement->Xe2.L4CC    = GMM_GFX_PHY_L4_MT_WT;
        break;
    default:
        pUsageEle->L3.PhysicalL3.L4CC = GMM_GFX_PHY_L4_MT_UC;
        pUsagePATElement->Xe2.L4CC    = GMM_GFX_PHY_L4_MT_UC;
    }

    switch (pCachePolicy[Usage].Coherency)
    {
    case GMM_NON_COHERENT_NO_SNOOP:
        pUsagePATElement->Xe2.Coherency = GMM_GFX_NON_COHERENT_NO_SNOOP;
        break;
    case GMM_COHERENT_ONE_WAY_IA_SNOOP:
        pUsagePATElement->Xe2.Coherency = GMM_GFX_COHERENT_ONE_WAY_IA_SNOOP;
        break;
    case GMM_COHERENT_TWO_WAY_IA_GPU_SNOOP:
        pUsagePATElement->Xe2.Coherency = GMM_GFX_COHERENT_TWO_WAY_IA_GPU_SNOOP;
        break;
    default:
        pUsagePATElement->Xe2.Coherency = GMM_GFX_NON_COHERENT_NO_SNOOP;
        break;
    }

    if (pGmmLibContext->GetWaTable().Wa_14018443005 &&
        (pCachePolicy[Usage].L3CC == GMM_UC) &&
        (ISWA_1401844305USAGE(Usage)) &&
        (pGmmLibContext->GetClientType() != GMM_KMD_VISTA) &&
        (pGmmLibContext->GetClientType() != GMM_OCL_VISTA))
    {
        pUsageEle->L3.PhysicalL3.L3CC = GMM_GFX_PHY_L3_MT_WB;
        pUsagePATElement->Xe2.L3CC    = GMM_GFX_PHY_L3_MT_WB;
        pCachePolicy[Usage].L3CC      = GMM_WB;
    }
}

/////////////////////////////////////////////////////////////////////////////////////
///      A simple getter function returning the PAT (cache policy) for a given
///      use Usage of the named resource pResInfo.
///      Typically used to populate PPGTT/GGTT.
///
/// @param[in]     pResInfo: Resource info for resource, can be NULL.
/// @param[in]     Usage: Current usage for resource.
/// @param[in]     pCompressionEnabl: for Xe2 compression parameter
/// @param[in]     IsCpuCacheable: Indicates Cacheability
/// @return        PATIndex
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmLib::GmmXe2_LPGCachePolicy::CachePolicyGetPATIndex(GMM_RESOURCE_INFO *pResInfo, GMM_RESOURCE_USAGE_TYPE Usage, bool *pCompressionEnable, bool IsCpuCacheable)
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
	GMM_ASSERTDPF(false, "Coherent Compressed is not supported on Xe2. However, respecting the coherency and returning CoherentPATIndex");
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
void GmmLib::GmmXe2_LPGCachePolicy::SetUpMOCSTable()
{
    GMM_CACHE_POLICY_TBL_ELEMENT *pCachePolicyTlbElement = &(pGmmLibContext->GetCachePolicyTlbElement()[0]);

#define L4_WB (0x0)
#define L4_WT (0x1)
#define L4_UC (0x3)

#define L3_WB (0x0)
#define L3_XD (pGmmLibContext->GetSkuTable().FtrL3TransientDataFlush ? 0x1 : 0x0)
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
    for(uint32_t j = 0; j < GMM_XE2_NUM_MOCS_ENTRIES; j++)
    {   //               Index            CachingPolicy   L3Caching      L3ClassOfService    ignorePAT
        GMM_DEFINE_MOCS( j,               L4_UC,          L3_UC,             0          ,     0  )
    }

    //             Index    L4 CachingPolicy   L3 CachingPolicy   L3 CLOS   ignorePAT
    GMM_DEFINE_MOCS( 0      , L4_UC              , L3_WB           , 0     , 0)   // Defer to PAT
    GMM_DEFINE_MOCS( 1      , L4_UC              , L3_WB           , 0     , 1)   // L3
    GMM_DEFINE_MOCS( 2      , L4_WB              , L3_UC           , 0     , 1)   // L4
    GMM_DEFINE_MOCS( 3      , L4_UC              , L3_UC           , 0     , 1)   // UC
    GMM_DEFINE_MOCS( 4      , L4_WB              , L3_WB           , 0     , 1)   // L3+L4

    CurrentMaxMocsIndex = 4;
    CurrentMaxL1HdcMocsIndex   = 0;
    CurrentMaxSpecialMocsIndex = 0;
    // clang-format on

#undef GMM_DEFINE_MOCS
#undef L4_WB
#undef L4_WT
#undef L4_UC

#undef L3_WB
#undef L3_XD
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
GMM_STATUS GmmLib::GmmXe2_LPGCachePolicy::SetupPAT()
{
    GMM_PRIVATE_PAT *pPATTlbElement = &(pGmmLibContext->GetPrivatePATTable()[0]);

#define L4_WB (0x0)
#define L4_WT (0x1)
#define L4_UC (0x3)

#define L3_WB (0x0)
#define L3_XD (pGmmLibContext->GetSkuTable().FtrL3TransientDataFlush ? 0x1 : 0x0)
#define L3_UC (0x3)
#define L3_XA (0x2) // WB Transient App

#define GMM_DEFINE_PAT_ELEMENT(indx, Coh, L4Caching, L3Caching, L3ClassOfService, CompressionEn, NoCachePromote) \
    {                                                                                                            \
        pPATTlbElement[indx].Xe2.Coherency             = Coh;                                                    \
        pPATTlbElement[indx].Xe2.L4CC                  = L4Caching;                                              \
        pPATTlbElement[indx].Xe2.Reserved1             = 0;                                                      \
        pPATTlbElement[indx].Xe2.Reserved2             = 0;                                                      \
        pPATTlbElement[indx].Xe2.L3CC                  = L3Caching;                                              \
        pPATTlbElement[indx].Xe2.L3CLOS                = L3ClassOfService;                                       \
        pPATTlbElement[indx].Xe2.LosslessCompressionEn = CompressionEn;                                          \
        pPATTlbElement[indx].Xe2.NoCachingPromote      = NoCachePromote;                                         \
    }

    // clang-format off

    // Default PAT Table
    // 32 nos
    for (uint32_t i = 0; i < (NumPATRegisters); i++)
    {   //                      Index  Coherency  CachingPolicy  L3Caching  L3ClassOfService  CompressionEn  NoCachingPromote
        GMM_DEFINE_PAT_ELEMENT( i,     3,         L4_UC,         L3_UC,     0,                0,             0);
    }

    // Fixed PAT Table
    //                      Index  Coherency  L4 CachingPolicy   L3 CachingPolicy   L3 CLOS      CompressionEn   NoCachingPromote
    //Group: GGT/PPGTT[4]
    GMM_DEFINE_PAT_ELEMENT( 0      , 0      , L4_UC              , L3_WB           , 0          , 0             , 0)    //          | L3_WB 
    GMM_DEFINE_PAT_ELEMENT( 1      , 2      , L4_UC              , L3_WB           , 0          , 0             , 0)    //          | L3_WB | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 2      , 3      , L4_UC              , L3_WB           , 0          , 0             , 0)    //          | L3_WB | 2 way coherent
    GMM_DEFINE_PAT_ELEMENT( 3      , 0      , L4_UC              , L3_UC           , 0          , 0             , 0)    // **UC   
    //Group: 1 way Coh
    GMM_DEFINE_PAT_ELEMENT( 4      , 2      , L4_WB              , L3_UC           , 0          , 0             , 0)    // L4_WB            | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 5      , 2      , L4_UC              , L3_UC           , 0          , 0             , 0)    // **UC             | 1 way coherent
    //Group: Compression Disabled
    GMM_DEFINE_PAT_ELEMENT( 6      , 0      , L4_UC              , L3_XD           , 0          , 0             , 1)    //          | L3_XD 
    GMM_DEFINE_PAT_ELEMENT( 7      , 3      , L4_WB              , L3_UC           , 0          , 0             , 0)    // L4_WB            | 2 way coherent
    GMM_DEFINE_PAT_ELEMENT( 8      , 0      , L4_WB              , L3_UC           , 0          , 0             , 0)    // L4_WB  
    //Group: Compression Enabled
    GMM_DEFINE_PAT_ELEMENT( 9      , 0      , L4_UC              , L3_WB           , 0          , 1             , 0)    //          | L3_WB | Comp 
    GMM_DEFINE_PAT_ELEMENT( 10     , 0      , L4_WB              , L3_UC           , 0          , 1             , 0)    // L4_WB            | Comp
    GMM_DEFINE_PAT_ELEMENT( 11     , 0      , L4_UC              , L3_XD           , 0          , 1             , 1)    //          | L3_XD | Comp 
    GMM_DEFINE_PAT_ELEMENT( 12     , 0      , L4_UC              , L3_UC           , 0          , 1             , 0)    // **UC             | Comp 

    GMM_DEFINE_PAT_ELEMENT( 13     , 0      , L4_WB              , L3_WB           , 0          , 0             , 0)     // L4_WB    | L3_WB 
    GMM_DEFINE_PAT_ELEMENT( 14     , 0      , L4_WB              , L3_WB           , 0          , 1             , 0)    // L4_WB    | L3_WB | Comp
    GMM_DEFINE_PAT_ELEMENT( 15     , 0      , L4_WT              , L3_XD           , 0          , 1             , 1)    // L4_WT    | L3_XD | Comp 
    
    //Reserved 16-19
    //Group: CLOS1
    GMM_DEFINE_PAT_ELEMENT( 20      , 0      , L4_UC             , L3_WB           , 1          , 0             , 0)    //          | L3_WB   
    GMM_DEFINE_PAT_ELEMENT( 21      , 0      , L4_UC             , L3_WB           , 1          , 1             , 0)    //          | L3_WB | Comp 
    GMM_DEFINE_PAT_ELEMENT( 22      , 2      , L4_UC             , L3_WB           , 1          , 0             , 0)    //          | L3_WB | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 23      , 3      , L4_UC             , L3_WB           , 1          , 0             , 0)    //          | L3_WB | 2 way coherent 
    //Group:CLOS2=>Clone of CLOS1
    GMM_DEFINE_PAT_ELEMENT( 24      , 0      , L4_UC             , L3_WB           , 2          , 0             , 0)    //          | L3_WB   
    GMM_DEFINE_PAT_ELEMENT( 25      , 0      , L4_UC             , L3_WB           , 2          , 1             , 0)    //          | L3_WB | Comp 
    GMM_DEFINE_PAT_ELEMENT( 26      , 2      , L4_UC             , L3_WB           , 2          , 0             , 0)    //          | L3_WB | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 27      , 3      , L4_UC             , L3_WB           , 2          , 0             , 0)    //          | L3_WB | 2 way coherent 
    //Group:CLOS3=>Clone of CLOS1
    GMM_DEFINE_PAT_ELEMENT( 28      , 0      , L4_UC             , L3_WB           , 3          , 0             , 0)    //          | L3_WB    
    GMM_DEFINE_PAT_ELEMENT( 29      , 0      , L4_UC             , L3_WB           , 3          , 1             , 0)    //          | L3_WB | Comp 
    GMM_DEFINE_PAT_ELEMENT( 30      , 2      , L4_UC             , L3_WB           , 3          , 0             , 0)    //          | L3_WB | 1 way coherent
    GMM_DEFINE_PAT_ELEMENT( 31      , 3      , L4_UC             , L3_WB           , 3          , 0             , 0)    //          | L3_WB | 2 way coherent 

    CurrentMaxPATIndex = 31;

// clang-format on
#undef GMM_DEFINE_PAT
#undef L4_WB
#undef L4_WT
#undef L4_UC

#undef L3_WB
#undef L3_XD
#undef L3_UC
    return GMM_SUCCESS;
}
