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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "GmmCachePolicyExt.h"

#define ALWAYS_OVERRIDE 0xffffffff
#define NEVER_OVERRIDE  0x0

// IA PAT MSR Register
#define GMM_IA32e_PAT_MSR 0x277

#define GMM_PAT_ERROR ((uint32_t)-1)

typedef struct GMM_CACHE_POLICY_ELEMENT_REC
{
    uint32_t                       IDCode;
    union {
       struct{
            uint64_t                  LLC         : 1;
            uint64_t                    ELLC        : 1;
            uint64_t                   L3          : 1;
            uint64_t                   WT          : 1;
            uint64_t                   AGE         : 2;
            uint64_t                   AOM         : 1;
            uint64_t                   LeCC_SCC    : 3;
            uint64_t                   L3_SCC      : 3;
            uint64_t                   SCF         : 1; // Snoop Control Field for BXT
            uint64_t                   CoS         : 2; // Class of Service, driver default to Class 0
            uint64_t                   SSO         : 2; // Self Snoop Override  control and value
            uint64_t                   HDCL1       : 1; // HDC L1 caching enable/disable
            uint64_t                   L3Eviction  : 2; // Specify L3-eviction type (NA, ReadOnly, Standard, Special)
            uint64_t                   SegOv       : 3; // Override seg-pref (none, local-only, sys-only, etc)
            uint64_t                   GlbGo       : 1; // Global GO point - L3 or Memory
            uint64_t                   UcLookup    : 1; // Snoop L3 for uncached 
            uint64_t                   L1CC        : 3; // L1 Cache Control
	    uint64_t                   Initialized : 1;
            uint64_t                   L2CC        : 2; // media internal cache 0:UC, 1:WB
            uint64_t                   L4CC        : 2; // ADM memory cache 0: UC, 1:WB, 2: WT
            uint64_t                   Coherency   : 2; // 0 non-coh, 1: 1 way coh IA snoop 2: 2 way coh IA GPU snopp
	    uint64_t                   CoherentPATIndex : 5;
	    uint64_t                   Reserved    : 23;

	};
        uint64_t Value;    
    };

    MEMORY_OBJECT_CONTROL_STATE               MemoryObjectOverride;
  
    union
    {
        MEMORY_OBJECT_CONTROL_STATE           MemoryObjectNoOverride;
        uint32_t                              PATIndex;
    };
    
    GMM_PTE_CACHE_CONTROL_BITS                PTE; 
    uint32_t                                  Override;
    uint32_t                                  IsOverridenByRegkey; // Flag to indicate If usage settings are overridden by regkey
}GMM_CACHE_POLICY_ELEMENT;

// One entry in the SKL/CNL cache lookup table
typedef struct GMM_CACHE_POLICY_TBL_ELEMENT_REC {
    union {
        struct
        {
            uint32_t Cacheability        : 2; // LLC/eDRAM Cacheability Control (LeCC).
            uint32_t TargetCache         : 2; // Target Cache (TC).
            uint32_t LRUM                : 2; // LRU (Cache Replacement) Management (LRUM).
            uint32_t AOM                 : 1; // Do Not Allocate on Miss (AOM) - Not used
            uint32_t ESC                 : 1; // Enable Skip Caching (ESC) on LLC
            uint32_t SCC                 : 3; // Skip Caching Control (SCC).
            uint32_t PFM                 : 3; // Page Fault Mode (PFM)
            uint32_t SCF                 : 1; // Snoop Control Field (SCF)
            uint32_t CoS                 : 2; // Class of Service (COS)- CNL+
            uint32_t SelfSnoop           : 2; // Self Snoop override or not MIDI settings - CNL+
            uint32_t Reserved            : 13;
        } ;
	
        uint32_t DwordValue;

	union {
            struct
            {
                uint32_t Reserved0       : 2;
                uint32_t L4CC            : 2;
                uint32_t Reserved1       : 2;
                uint32_t Reserved2       : 2;
                uint32_t igPAT           : 1; // selection between MOCS and PAT
                uint32_t Reserved3       : 23;
            };
	    
	    uint32_t DwordValue;

        } Xe_LPG;

    } LeCC;

    union {
        struct
        {
            uint16_t ESC                : 1; // Enable Skip Caching (ESC) for L3.
            uint16_t SCC                : 3; // Skip Caching Control (SCC) for L3.
            uint16_t Cacheability       : 2; // L3 Cacheability Control (L3CC).
            uint16_t GlobalGo           : 1; // Global Go (GLBGO).
            uint16_t UCLookup           : 1; // UC L3 Lookup (UcL3Lookup).
            uint16_t Reserved           : 8;
        } ;
        uint16_t UshortValue;
    } L3;

    uint8_t    HDCL1;

} GMM_CACHE_POLICY_TBL_ELEMENT;

typedef enum GMM_IA32e_MEMORY_TYPE_REC
{
    GMM_IA32e_UC        = 0x0,
    GMM_IA32e_WC        = 0x1,
    GMM_IA32e_WT        = 0x4,
    GMM_IA32e_WP        = 0x5,
    GMM_IA32e_WB        = 0x6,
    GMM_IA32e_UCMINUS   = 0x7
} GMM_IA32e_MEMORY_TYPE;

typedef enum GMM_GFX_MEMORY_TYPE_REC
{
    GMM_GFX_UC_WITH_FENCE   = 0x0,
    GMM_GFX_WC              = 0x1,
    GMM_GFX_WT              = 0x2,
    GMM_GFX_WB              = 0x3
} GMM_GFX_MEMORY_TYPE;

typedef enum GMM_L4_CACHING_POLICY_REC
{
    GMM_CP_COHERENT_WB                    = 0x0,
    GMM_CP_NON_COHERENT_WB                = 0x0,
    GMM_CP_NON_COHERENT_WT                = 0x1,
    GMM_CP_NON_COHERENT_UC                = 0x3,
} GMM_L4_CACHING_POLICY;

typedef enum GMM_GFX_COHERENCY_TYPE_REC
{
    GMM_GFX_NON_COHERENT_NO_SNOOP           = 0x0,
    GMM_GFX_NON_COHERENT                    = 0x1,
    GMM_GFX_COHERENT_ONE_WAY_IA_SNOOP       = 0x2,
    GMM_GFX_COHERENT_TWO_WAY_IA_GPU_SNOOP   = 0x3
} GMM_GFX_COHERENCY_TYPE;

typedef enum GMM_GFX_PAT_TYPE_REC
{
    GMM_GFX_PAT_WB_COHERENT     = 0x0, // WB + Snoop : Atom
    GMM_GFX_PAT_WC              = 0x1,
    GMM_GFX_PAT_WB_MOCSLESS     = 0x2, // WB + eLLC/LLC : MOCSLess Resource (PageTables)
    GMM_GFX_PAT_WB              = 0x3,
    GMM_GFX_PAT_WT              = 0x4,
    GMM_GFX_PAT_UC              = 0x5
} GMM_GFX_PAT_TYPE;

typedef enum GMM_GFX_PAT_IDX_REC
{
    PAT0 = 0x0,     // Will be tied to GMM_GFX_PAT_WB_COHERENT or GMM_GFX_PAT_UC depending on WaGttPat0WB
    PAT1,           // Will be tied to GMM_GFX_PAT_UC or GMM_GFX_PAT_WB_COHERENT depending on WaGttPat0WB
    PAT2,           // Will be tied to GMM_GFX_PAT_WB_MOCSLESS
    PAT3,           // Will be tied to GMM_GFX_PAT_WB
    PAT4,           // Will be tied to GMM_GFX_PAT_WT
    PAT5,           // Will be tied to GMM_GFX_PAT_WC
    PAT6,           // Will be tied to GMM_GFX_PAT_WC
    PAT7,           // Will be tied to GMM_GFX_PAT_WC
    // Additional registers
    PAT8,
    PAT9,
    PAT10,
    PAT11,
    PAT12,
    PAT13,
    PAT14,
    PAT15	    
}GMM_GFX_PAT_IDX;

#define GFX_IS_ATOM_PLATFORM(pGmmLibContext) (GmmGetSkuTable(pGmmLibContext)->FtrLCIA)

typedef enum GMM_GFX_TARGET_CACHE_REC
{
    GMM_GFX_TC_ELLC_ONLY = 0x0,
    GMM_GFX_TC_LLC_ONLY  = 0x1,
    GMM_GFX_TC_ELLC_LLC  = 0x3
} GMM_GFX_TARGET_CACHE;

typedef union GMM_PRIVATE_PAT_REC {

    struct
    {
        uint32_t MemoryType : 2;
        uint32_t TargetCache : 2;
        uint32_t Age : 2;
        uint32_t Snoop : 1;
        uint32_t Reserved : 1;
    }PreGen10;
#if (IGFX_GEN >= IGFX_GEN10)
    struct
    {
        uint32_t MemoryType : 2;
        uint32_t TargetCache : 2;
        uint32_t Age : 2;
        uint32_t Reserved1 : 2;
        uint32_t CoS : 2;
        uint32_t Reserved2 : 22;
    }Gen10;
#endif
    struct
    {
        uint32_t MemoryType : 2;
        uint32_t Reserved   : 30;
    }Gen12;
    
     struct
    {
        uint32_t MemoryType : 2;  //L3
        uint32_t L3CLOS : 2;
        uint32_t Reserved : 28;
    } Xe_HPC;

    struct
    {
        uint32_t Coherency          : 2;     
        uint32_t L4CC               : 2;    
        uint32_t Reserved           : 28;
    }Xe_LPG;

    uint32_t   Value;

} GMM_PRIVATE_PAT;

#ifdef _WIN32

#define GMM_CACHE_POLICY_OVERRIDE_REGISTY_PATH_REGISTRY_UMD "SOFTWARE\\Intel\\IGFX\\CachePolicy\\"
#define GMM_CACHE_POLICY_OVERRIDE_REGISTY_PATH_REGISTRY_KMD "\\REGISTRY\\MACHINE\\" GMM_CACHE_POLICY_OVERRIDE_REGISTY_PATH_REGISTRY_UMD

// No local variables or ternary (?:) operators should be defined in the
// REG_OVERRIDE macro block. This block will generate hundreds of instances
// of itself in the OverrideCachePolicy, each of which will geneate a
// copy of these local variables on the stack. The ternary operator will
// also generate local temparoary stack variables, and all these add up and
// can cause a stack overflow.

#define READOVERRIDES(Usage)                                                        \
        if (!REGISTRY_OVERRIDE_READ(Usage,LLC))        LLC        = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,ELLC))       ELLC       = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,L3))         L3         = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,Age))        Age        = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,WT))         WT         = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,AOM))        AOM        = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,LeCC_SCC))   LeCC_SCC   = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,L3_SCC))     L3_SCC     = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,SCF))        SCF        = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,SSO))        SSO        = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,CoS))        CoS        = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,HDCL1))      HDCL1      = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,L3Eviction)) L3Eviction = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,GlbGo))      GlbGo      = -1;             \
        if (!REGISTRY_OVERRIDE_READ(Usage,UcLookup))   UcLookup   = -1;             \



#define SETOVERRIDES(Usage)                                                     \
        if (LLC != -1)                                                          \
        {                                                                       \
            pCachePolicy[Usage].LLC = LLC;                                      \
        }                                                                       \
        if (ELLC != -1)                                                         \
        {                                                                       \
            pCachePolicy[Usage].ELLC = ELLC;                                    \
        }                                                                       \
        if (L3 != -1)                                                           \
        {                                                                       \
            pCachePolicy[Usage].L3 = L3;                                        \
        }                                                                       \
        if (WT != -1)                                                           \
        {                                                                       \
            pCachePolicy[Usage].WT = WT;                                        \
        }                                                                       \
        if (Age != -1)                                                          \
        {                                                                       \
            pCachePolicy[Usage].AGE = Age;                                      \
        }                                                                       \
        if (AOM != -1)                                                          \
        {                                                                       \
            pCachePolicy[Usage].AOM = AOM;                                      \
        }                                                                       \
        if (LeCC_SCC != -1)                                                     \
        {                                                                       \
            pCachePolicy[Usage].LeCC_SCC = LeCC_SCC;                            \
        }                                                                       \
        if (L3_SCC != -1)                                                       \
        {                                                                       \
            pCachePolicy[Usage].L3_SCC = L3_SCC;                                \
        }                                                                       \
        if (SCF != -1)                                                          \
        {                                                                       \
            pCachePolicy[Usage].SCF = SCF;                                      \
        }                                                                       \
        if (SSO != -1)                                                          \
        {                                                                       \
            pCachePolicy[Usage].SSO = SSO;                                      \
        }                                                                       \
        if (CoS != -1)                                                          \
        {                                                                       \
            pCachePolicy[Usage].CoS = CoS;                                      \
        }                                                                       \
        if (HDCL1 != -1)                                                        \
        {                                                                       \
            pCachePolicy[Usage].HDCL1 = HDCL1;                                  \
        }                                                                       \
        if (L3Eviction != -1)                                                   \
        {                                                                       \
            pCachePolicy[Usage].L3Eviction = L3Eviction;                        \
        }                                                                       \
        if (GlbGo != -1)                                                        \
        {                                                                       \
            pCachePolicy[Usage].GlbGo = GlbGo;                                  \
        }                                                                       \
        if (UcLookup != -1)                                                     \
        {                                                                       \
            pCachePolicy[Usage].UcLookup = UcLookup;                            \
        }                                                                       \
        {                                                                       \
            pCachePolicy[Usage].IsOverridenByRegkey = 1;                        \
        }

#ifdef __GMM_KMD__
#define REG_OVERRIDE(Usage)                                                        \
{                                                                                  \
    enum {IGNORED = 0, CURRENT, DEFAULT, UNCACHED};                                \
                                                                                   \
    REGISTRY_OVERRIDE_READ(Usage,Enable);                                          \
                                                                                   \
    if (Enable && (GenerateKeys != UNCACHED))                                      \
    {                                                                              \
        READOVERRIDES(Usage)                                                       \
    }                                                                              \
                                                                                   \
    if (GenerateKeys == DEFAULT || (GenerateKeys == CURRENT && !Enable))           \
    {                                                                              \
        REGISTRY_OVERRIDE_WRITE(Usage,LLC, pCachePolicy[Usage].LLC);               \
        REGISTRY_OVERRIDE_WRITE(Usage,ELLC,pCachePolicy[Usage].ELLC);              \
        REGISTRY_OVERRIDE_WRITE(Usage,L3,  pCachePolicy[Usage].L3);                \
        REGISTRY_OVERRIDE_WRITE(Usage,WT,  pCachePolicy[Usage].WT);                \
        REGISTRY_OVERRIDE_WRITE(Usage,Age, pCachePolicy[Usage].AGE);               \
        REGISTRY_OVERRIDE_WRITE(Usage,AOM, pCachePolicy[Usage].AOM);               \
        REGISTRY_OVERRIDE_WRITE(Usage,LeCC_SCC, pCachePolicy[Usage].LeCC_SCC);     \
        REGISTRY_OVERRIDE_WRITE(Usage,L3_SCC, pCachePolicy[Usage].L3_SCC);         \
        REGISTRY_OVERRIDE_WRITE(Usage,SCF, pCachePolicy[Usage].SCF);               \
        REGISTRY_OVERRIDE_WRITE(Usage,SSO, pCachePolicy[Usage].SSO);               \
        REGISTRY_OVERRIDE_WRITE(Usage,CoS, pCachePolicy[Usage].CoS);               \
        REGISTRY_OVERRIDE_WRITE(Usage,HDCL1, pCachePolicy[Usage].HDCL1);           \
        REGISTRY_OVERRIDE_WRITE(Usage,L3Eviction, pCachePolicy[Usage].L3Eviction); \
        REGISTRY_OVERRIDE_WRITE(Usage,Enable,0);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,GlbGo, pCachePolicy[Usage].GlbGo);           \
        REGISTRY_OVERRIDE_WRITE(Usage,UcLookup, pCachePolicy[Usage].UcLookup);     \

    }                                                                              \
    else if (GenerateKeys == UNCACHED || GenerateKeys == CURRENT)                  \
    {                                                                              \
        if (GenerateKeys == UNCACHED)                                              \
        {                                                                          \
            Enable = 1;                                                            \
        }                                                                          \
        REGISTRY_OVERRIDE_WRITE(Usage,LLC, LLC);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,ELLC,ELLC);                                  \
        REGISTRY_OVERRIDE_WRITE(Usage,L3,  L3);                                    \
        REGISTRY_OVERRIDE_WRITE(Usage,WT,  WT);                                    \
        REGISTRY_OVERRIDE_WRITE(Usage,Age, Age);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,AOM, AOM);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,LeCC_SCC, LeCC_SCC);                         \
        REGISTRY_OVERRIDE_WRITE(Usage,L3_SCC, L3_SCC);                             \
        REGISTRY_OVERRIDE_WRITE(Usage,SCF, SCF);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,SSO, SSO);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,CoS, CoS);                                   \
        REGISTRY_OVERRIDE_WRITE(Usage,HDCL1, HDCL1);                               \
        REGISTRY_OVERRIDE_WRITE(Usage,L3Eviction, L3Eviction);                     \
        REGISTRY_OVERRIDE_WRITE(Usage,Enable,Enable);                              \
        REGISTRY_OVERRIDE_WRITE(Usage,GlbGo, GlbGo);                               \
        REGISTRY_OVERRIDE_WRITE(Usage,UcLookup, UcLookup);                         \
    }                                                                              \
                                                                                   \
    if (Enable)                                                                    \
    {                                                                              \
        SETOVERRIDES(Usage)                                                        \
    }                                                                              \
                                                                                   \
    UsageCount++;                                                                  \
}
#else
#define REG_OVERRIDE(Usage)                                                     \
{                                                                               \
    REGISTRY_OVERRIDE_READ(Usage,Enable);                                       \
    if (Enable)                                                                 \
    {                                                                           \
        READOVERRIDES(Usage)                                                    \
        SETOVERRIDES(Usage)                                                     \
    }                                                                           \
    UsageCount++;                                                               \
}
#endif

#endif // #if _WIN32

#if __cplusplus
#include "GmmCachePolicyCommon.h"
#include "CachePolicy/GmmCachePolicyGen8.h"
#include "CachePolicy/GmmCachePolicyGen9.h"
#endif

// Function Prototypes
GMM_STATUS  GmmInitializeCachePolicy();
GMM_GFX_MEMORY_TYPE GmmGetWantedMemoryType(void *pLibContext, GMM_CACHE_POLICY_ELEMENT CachePolicy);

// Used for GMM ULT testing.
#ifdef __GMM_ULT
extern void __GmmULTCachePolicy(GMM_CACHE_POLICY_ELEMENT *pCachePolicy);
#endif
#ifdef __cplusplus
}
#endif
