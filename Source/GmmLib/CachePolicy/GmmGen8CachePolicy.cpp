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
// Function: __GmmGen8InitCachePolicy
//
// Desc: This function initializes the cache policy
//
// Parameters: pCachePolicy  -> Ptr to array to be populated with the
//             mapping of usages -> cache settings.
//
// Return: GMM_STATUS
//-----------------------------------------------------------------------------
GMM_STATUS GmmLib::GmmGen8CachePolicy::InitCachePolicy()
{

    __GMM_ASSERTPTR(pCachePolicy, GMM_ERROR);

#define DEFINE_CACHE_ELEMENT(usage, llc, ellc, l3, wt, age) DEFINE_CP_ELEMENT(usage, llc, ellc, l3, wt, age, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#include "GmmGen8CachePolicy.h"

    {
// Gen8 Memory Object Definitions
#define MO_ELLC 0x0
#define MO_LLC 0x1
#define MO_LLC_ELLC 0x2
#define MO_L3_LLC_ELLC 0x3

#define MO_USE_PTE 0x0
#define MO_UC 0x1
#define MO_WT 0x2
#define MO_WB 0x3

        // Define index of cache element
        uint32_t Usage = 0;

        // Process Cache Policy and fill in look up table
        for(; Usage < GMM_RESOURCE_USAGE_MAX; Usage++)
        {
            bool     CachePolicyError = false;
            uint64_t PTEValue         = 0;

            if(pCachePolicy[Usage].LLC && pCachePolicy[Usage].ELLC && pCachePolicy[Usage].L3)
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.TargetCache = MO_L3_LLC_ELLC;
            else if(pCachePolicy[Usage].LLC && pCachePolicy[Usage].ELLC)
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.TargetCache = MO_LLC_ELLC;
            else if(pCachePolicy[Usage].ELLC)
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.TargetCache = MO_ELLC;
            else if(pCachePolicy[Usage].LLC)
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.TargetCache = MO_LLC;

            pCachePolicy[Usage].MemoryObjectOverride.Gen8.Age = pCachePolicy[Usage].AGE;

            if(pCachePolicy[Usage].WT)
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.CacheControl = MO_WT;
            // L3 is not included because WT vs UC vs WB only effects uncore
            else if(!(pCachePolicy[Usage].LLC || pCachePolicy[Usage].ELLC))
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.CacheControl = MO_UC;

            else
                pCachePolicy[Usage].MemoryObjectOverride.Gen8.CacheControl = MO_WB;


            if(!GetUsagePTEValue(pCachePolicy[Usage], Usage, &PTEValue))
            {
                CachePolicyError = true;
            }
            // On error, the PTE value is set to a UC PAT entry
            pCachePolicy[Usage].PTE.DwordValue     = PTEValue & 0xFFFFFFFF;
            pCachePolicy[Usage].PTE.HighDwordValue = 0;
	    pCachePolicy[Usage].Override       = ALWAYS_OVERRIDE;

            if(CachePolicyError)
            {
                GMM_ASSERTDPF("Cache Policy Init Error: Invalid Cache Programming - Element %d", Usage);
            }
        }
    }

    return GMM_SUCCESS;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns true if usage PTE entries are set for caching, false otherwise.
///
/// @param[in]      Usage: type of usage
///
/// @return         true if the usage PTE entry is set for cached, false otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmLib::GmmGen8CachePolicy::CachePolicyIsUsagePTECached(GMM_RESOURCE_USAGE_TYPE Usage)
{
    GMM_UNREFERENCED_PARAMETER(Usage);
    return 0;
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
GMM_STATUS GmmLib::GmmGen8CachePolicy::SetupPAT()
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

        if(pGmmLibContext->GetWaTable().WaNoMocsEllcOnly)	
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
                GfxMemType = GMM_GFX_WB;
		
		if(pGmmLibContext->GetWaTable().WaNoMocsEllcOnly)
                {
                    GfxTargetCache = GMM_GFX_TC_ELLC_ONLY;
                }
                else
                {
                    GfxTargetCache = GMM_GFX_TC_ELLC_LLC;
                }

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
    Status                                          = GMM_ERROR;
#endif
    return Status;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Initializes WA's needed for setting up the Private PATs
/// WaNoMocsEllcOnly, WaGttPat0, WaGttPat0GttWbOverOsIommuEllcOnly, WaGttPat0WB
///
/// @return        GMM_STATUS
///
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GmmLib::GmmGen8CachePolicy::SetPATInitWA()
{
    GMM_STATUS Status   = GMM_SUCCESS;
    WA_TABLE * pWaTable = &const_cast<WA_TABLE &>(pGmmLibContext->GetWaTable());

#if(defined(__GMM_KMD__))

    pWaTable->WaGttPat0                         = 1;
    pWaTable->WaGttPat0WB                       = 1;
    pWaTable->WaGttPat0GttWbOverOsIommuEllcOnly = 1;

    // Platforms which support OS-IOMMU.
    if(pGmmLibContext->GetSkuTable().FtrWddm2Svm)    
    {
        pWaTable->WaGttPat0GttWbOverOsIommuEllcOnly = 0;
        pWaTable->WaGttPat0WB                       = 0;
    }

#else
    Status                                          = GMM_ERROR;
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
uint32_t GmmLib::GmmGen8CachePolicy::BestMatchingPATIdx(GMM_CACHE_POLICY_ELEMENT CachePolicy)
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

    for(i = 1; i < NumPATRegisters; i++)
    {
        GMM_PRIVATE_PAT PAT1 = GetPrivatePATEntry(PATIdx);
        GMM_PRIVATE_PAT PAT2 = GetPrivatePATEntry(i);

        if(SelectNewPATIdx(WantedMemoryType, WantedTC,
                           (GMM_GFX_MEMORY_TYPE)PAT1.PreGen10.MemoryType, (GMM_GFX_TARGET_CACHE)PAT1.PreGen10.TargetCache,
                           (GMM_GFX_MEMORY_TYPE)PAT2.PreGen10.MemoryType, (GMM_GFX_TARGET_CACHE)PAT2.PreGen10.TargetCache))
        {
            PATIdx = i;
        }
    }

    MemoryType = (GMM_GFX_MEMORY_TYPE)GetPrivatePATEntry(PATIdx).PreGen10.MemoryType;

    if(MemoryType != WantedMemoryType)
    {
        // Failed to find a matching PAT entry
        return GMM_PAT_ERROR;
    }
    return PATIdx;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Sets the GMM Private PAT in the PrivatePATTable for the PATIdx, GMM_PRIVATE_PAT
/// Entry passed
///
/// @param[in]      PATIdx
/// @param[in]      GMM_PRIVATE_PAT: PAT Entry
///
/// @return         Pass/ fail
/////////////////////////////////////////////////////////////////////////////////////
bool GmmLib::GmmGen8CachePolicy::SetPrivatePATEntry(uint32_t PATIdx, GMM_PRIVATE_PAT Entry)
{
    if(PATIdx >= NumPATRegisters)
    {
        GMM_ASSERTDPF(false, "CRITICAL ERROR: INVALID PAT IDX");
        return false;
    }
    
    pGmmLibContext->GetPrivatePATTable()[PATIdx] = Entry;    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Gets the GMM Private PAT from the PrivatePATTable for the PATIdx passed
///
/// @param[in]     PATIdx
///
/// @return        GMM_PRIVATE_PAT: Entry
/////////////////////////////////////////////////////////////////////////////////////
GMM_PRIVATE_PAT GmmLib::GmmGen8CachePolicy::GetPrivatePATEntry(uint32_t PATIdx)
{
    GMM_PRIVATE_PAT NullPAT = {0};

    if(PATIdx >= NumPATRegisters)
    {
        GMM_ASSERTDPF(false, "CRITICAL ERROR: INVALID PAT IDX");
        return NullPAT;
    }
    return pGmmLibContext->GetPrivatePATTable()[PATIdx];    
}

/////////////////////////////////////////////////////////////////////////////////////
/// Return true if (MT2, TC2) is a better match for (WantedMT, WantedTC)
///       than (MT1, TC1)
///
/// @param[in]         WantedMT: Wanted Memory Type
/// @param[in]         WantedTC: Wanted Target Cache
/// @param[in]         MT1: Memory Type for PATIdx1
/// @param[in]         TC1: Target Cache for PATIdx1
/// @param[in]         MT2: Memory Type for PATIdx2
/// @param[in]         TC2: Target Cache for PATIdx2
///
/// @return            Select the new PAT Index True/False
/////////////////////////////////////////////////////////////////////////////////////
bool GmmLib::GmmGen8CachePolicy::SelectNewPATIdx(GMM_GFX_MEMORY_TYPE WantedMT, GMM_GFX_TARGET_CACHE WantedTC,
                                                 GMM_GFX_MEMORY_TYPE MT1, GMM_GFX_TARGET_CACHE TC1,
                                                 GMM_GFX_MEMORY_TYPE MT2, GMM_GFX_TARGET_CACHE TC2)
{
    bool SelectPAT2 = false;

    // select on Memory Type
    if(MT1 != WantedMT)
    {
        if(MT2 == WantedMT || MT2 == GMM_GFX_UC_WITH_FENCE)
        {
            SelectPAT2 = true;
        }
        goto EXIT;
    }

    // select on Target Cache
    if(WantedTC != TC1)
    {
        if(WantedMT == MT2 && WantedTC == TC2)
        {
            SelectPAT2 = true;
        }
        goto EXIT;
    }

EXIT:
    return SelectPAT2;
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns PTE value
///
/// @param[in]       CachePolicyUsage: Cache Policy for Usage
///
/// @return          true: success, false: failure
/////////////////////////////////////////////////////////////////////////////////////
bool GmmLib::GmmGen8CachePolicy::GetUsagePTEValue(GMM_CACHE_POLICY_ELEMENT CachePolicyUsage,
                                                  uint32_t                 Usage,
                                                  uint64_t *               pPTEDwordValue)
{
    GMM_PTE_CACHE_CONTROL_BITS PTE     = {0};
    bool                       Success = true;
    uint32_t                   PATIdx  = 0;

// Don't setup PTE values in UMD
#if __GMM_KMD__
    if((PATIdx = BestMatchingPATIdx(CachePolicyUsage)) == GMM_PAT_ERROR)
    {
        // IAe32 PAT table does not necessarily have an entry for WT memory type
        // => not a cache policy init error if WT is unavailable.
        Success = CachePolicyUsage.WT ? true : false;

        // degrade to UC
        {
            GMM_CACHE_POLICY_ELEMENT CachePolicyElement = {0};

            const char *MemTypes[4] = {"UC", "WC", "WT", "WB"}; // matches GMM_GFX_MEMORY_TYPE enum values

            CachePolicyElement.Initialized = 1;

            GMM_DPF(GFXDBG_NORMAL,
                    "Cache Policy Init: Degrading PAT settings to UC (uncached) from %s for Element %d\n",
                    MemTypes[GetWantedMemoryType(CachePolicyUsage)], Usage);

            PATIdx = BestMatchingPATIdx(CachePolicyElement);
            if(PATIdx == GMM_PAT_ERROR)
            {
                Success = false;
            }
        }
    }
    if(PATIdx != GMM_PAT_ERROR)
    {
        PTE.Gen8.PAT = (PATIdx & __BIT(2)) ? 1 : 0;
        PTE.Gen8.PCD = (PATIdx & __BIT(1)) ? 1 : 0;
        PTE.Gen8.PWT = (PATIdx & __BIT(0)) ? 1 : 0;
    }
    else
    {
        PTE.DwordValue = 0x0;
    }
#else
    GMM_UNREFERENCED_PARAMETER(CachePolicyUsage);
    GMM_UNREFERENCED_PARAMETER(Usage);
#endif
    *pPTEDwordValue = PTE.DwordValue;
    return Success;
}
