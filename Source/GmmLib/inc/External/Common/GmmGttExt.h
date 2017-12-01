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

// Set packing alignment
#pragma pack(push, 8)


//===========================================================================
// typedef:
//        GMM_GTT_CACHETYPE
//
// Description:
//        GTT page table entry caching type.
//--------------------------------------------------------------------------
typedef enum GMM_GTT_CACHETYPE_ENUM
{
    GMM_GTT_CACHETYPE_INVALID           = 0, 
    GMM_GTT_CACHETYPE_UNCACHED          = 1,    // Gen3.5+
    GMM_GTT_CACHETYPE_CACHED            = 2,    // Gen3.5+
    GMM_GTT_CACHETYPE_LLC               = 3,    // Gen6+
    GMM_GTT_CACHETYPE_VLV_SNOOPED       = 4,    //Gen 7, VLV Snooped setting.
    GMM_GTT_CACHETYPE_GEN7_5_BASE       = 4,    // Gen7.5+ - INTERNAL USE ONLY
    GMM_GTT_CACHETYPE_GEN7_5_INVALID    = 5,    // Gen7.5+ - PLACEHOLDER FOR INVALID BITS
    GMM_GTT_CACHETYPE_WB_LLC_AGE_3      = 6,    // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_AGE_0      = 7,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_ELLC_AGE_0     = 8,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_ELLC_AGE_3     = 9,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_LLC_ELLC_AGE_0 = 10,    // Gen7.5+
    GMM_GTT_CACHETYPE_WT_LLC_ELLC_AGE_3 = 11,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_3 = 12,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_2 = 13,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_1 = 14,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_LLC_ELLC_AGE_0 = 15,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_3     = 16,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_2     = 17,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_1     = 18,   // Gen7.5+
    GMM_GTT_CACHETYPE_WB_ELLC_AGE_0     = 19,   // Gen7.5+
    GMM_GTT_CACHETYPE_MAX
}GMM_GTT_CACHETYPE;

//===========================================================================
// typedef:
//        GMM_GTT_ENTRY
//
// Description:
//        GMM can decodes fields. Clients can call export functions to query 
//        entry attributes. EG: cache type
//--------------------------------------------------------------------------
typedef UINT32    GMM_GTT_ENTRY32;
typedef UINT64    GMM_GTT_ENTRY64; //UNIT64 define to be compatible with Linux
typedef UINT64    GMM_GTT_ENTRY;

#define __GMM_PAGE_MARKED_READONLY 0
#define __GMM_PAGE_MARKED_WRITABLE 1

//===========================================================================
// typedef:
//        GMM_MAKE_PTE
//
// Description:
//        Clients use this struct to specify how to assemble a page table entry.
//--------------------------------------------------------------------------
typedef struct GMM_MAKE_PTE_REC
{
    BOOLEAN                 MDL;        // If TRUE, phys address is MDL page number
                                        // and needs to be translated to real addr.
                                        // If FALSE, phys addr is LARGE_INTEGER
    GMM_GTT_CACHETYPE       CacheType;  // PTE caching type
    GMM_RESOURCE_USAGE_TYPE Usage;      // Resource Usage

    BOOLEAN                 GFDT;       // Gen6+ gfx flush data type
    BOOLEAN                 Valid;      // Valid PTE
    BOOLEAN                 ReadOnly;   //From VLV2,B0 onwards indicate whether a page is readonly/writable by GT HW.
    BOOLEAN                 Snoopable;  // BXT: Set for resources which are WB cacheable
    
    BOOLEAN                 GGTT;        // IN : for GGTT PTEs

}GMM_MAKE_PTE;


//===========================================================================
// typedef:
//        GMM_PTE_INFO
//
// Description:
//        struct contains page table entry attributes such as cache type. Also
//        contains backing store address.
//--------------------------------------------------------------------------
typedef struct GMM_PTE_INFO_REC
{
    GMM_GTT_CACHETYPE       CacheType;  // PTE caching type
    BOOLEAN                 GFDT;       // Gen6-7 gfx flush data type
    BOOLEAN                 Valid;      // Valid PTE
    BOOLEAN                 ReadOnly;   // Indicated whether the page is Readonly or writeable , starting from VLV B0
    BOOLEAN                 Snoopable;  // BXT: Set for resources which are WB cacheable

    PHYSICAL_ADDRESS        PhysAddr;   // Physical address of backing store

    GMM_GTT_ENTRY           GTTEntry;   // Raw entry value

}GMM_PTE_INFO;


//===========================================================================
// typedef:
//        GMM_GTT_CACHETYPE
//
// Description:
//        GTT page table entry caching type.
//--------------------------------------------------------------------------
typedef enum GMM_PD_PAGESIZE_ENUM
{
    GMM_PD_PAGESIZE_4K, 
    GMM_PD_PAGESIZE_32K
}GMM_PD_PAGESIZE;

//===========================================================================
// typedef:
//        GMM_MAKE_PDE
//
// Description:
//        Clients use this struct to specify how to assemble a page directory entry.
//--------------------------------------------------------------------------
typedef struct GMM_MAKE_PDE_REC
{
    BOOLEAN             MDL;        // If TRUE, phys address is MDL page number
                                    // and needs to be translated to real addr.
                                    // If FALSE, phys addr is LARGE_INTEGER
    GMM_PD_PAGESIZE     Size;       // Size of backing store page
    BOOLEAN             Valid;      // Valid PTE

}GMM_MAKE_PDE;


//===========================================================================
// typedef:
//        GMM_PDE_INFO
//
// Description:
//        struct contains page directory entry attributes such as cache type. Also
//        contains backing store address.
//--------------------------------------------------------------------------
typedef struct GMM_PDE_INFO_REC
{
    GMM_PD_PAGESIZE     Size;       // Size of backing store page
    BOOLEAN             Valid;      // Valid PDE
    PHYSICAL_ADDRESS    PhysAddr;   // Physical address of backing store           
    
    uint32_t               PDByteOffset;

    GMM_PTE_INFO        PPGttInfo;
    void                *pPPGttEntryLinAddr;
}GMM_PDE_INFO;


//===========================================================================
// typedef:
//        GMM_PD_ENTRY32
//
// Description:
//        Page directory entry is 32bits wide. GMM can decodes fields. Clients can
//        call export functions to query entry attributes. EG: cache type
//--------------------------------------------------------------------------
typedef uint32_t GMM_PD_ENTRY32;



//===========================================================================
// typedef:
//        GMM_GTT_CONTEXT
//
// Description:
//        Context contains information regarding global GTT, PPGTT, Page directory.
//
//--------------------------------------------------------------------------
typedef struct GMM_GTT_CONTEXT_REC
{
    void*           pGlobalGttBaseLinAddr;  // Global GTT Info
    uint32_t           GlobalGTTSize;

    void*           pGfxApertBaseLinAddr;   // Global Aperture Info
    GMM_GFX_SIZE_T  GlobalGfxApertureSize;

    void            *pPageDirectoryLinAddr;
    uint32_t           PageDirectorySize;

    void            *pPPGttBaseLinAddr;     // Pointer to array of contiguous PPGTT's.
    GMM_GFX_SIZE_T  PPGttSize;              // Total size of PPGTT array (e.g. 2MB)
    GMM_GFX_SIZE_T  PPGttVASpace;           // Total PPGTT Virtual address space size based on PPGttSize
 
    struct GMM_GFX_ADDRESS_RANGE_REC 
    {
        struct 
        {
            GMM_GFX_ADDRESS Base;
            GMM_GFX_SIZE_T  Size;
        } Global, PP;                   // GTT and PPGTT location in unified gfx address space.
    } GfxAddrRange;

    GMM_STATUS (GMM_STDCALL *__pfnGmmGttGetEntryInfo)(
                        GMM_GFX_ADDRESS GfxAddress, 
                        GMM_PTE_INFO *pPteInfo);

    GMM_GTT_ENTRY (GMM_STDCALL *__pfnGmmGttMakeGTTEntry)(
                        void         *pAddress,
                        GMM_MAKE_PTE *pPteType);

}GMM_GTT_CONTEXT;

// Reset packing alignment to project default
#pragma pack(pop)

//***************************************************************************
//
//                      GTT API
//
//***************************************************************************
GMM_STATUS GMM_STDCALL GmmGttInitGttContext(GMM_GTT_CONTEXT *pInputContext);
GMM_PD_ENTRY32 GMM_STDCALL GmmGttMakePDEntry(void *pAddress, GMM_MAKE_PDE *pPdeType);
GMM_GTT_ENTRY GMM_STDCALL GmmGttMakeEntry(void *pAddress, GMM_MAKE_PTE *pPteType);
GMM_STATUS GMM_STDCALL GmmGttGetEntryInfo(GMM_GFX_ADDRESS GfxAddress, GMM_PTE_INFO *pPteInfo);
GMM_STATUS GMM_STDCALL GmmGttCopyToGfxAddress(GMM_GFX_ADDRESS DstGfxAddress, GMM_GFX_ADDRESS SrcGfxAddress, GMM_GFX_SIZE_T SizeBytes);
GMM_GTT_ENTRY GMM_STDCALL GmmGttGetEntry(GMM_GFX_ADDRESS GfxAddress);
GMM_PD_ENTRY32 GMM_STDCALL GmmGttGetPDEntry(GMM_GFX_ADDRESS GfxAddress);
void GMM_STDCALL GmmGttPutEntry(GMM_GTT_ENTRY GttEntry, GMM_GFX_ADDRESS GfxAddress);
void GMM_STDCALL GmmGttMapPageToRange(GMM_GFX_ADDRESS GfxAddress, GMM_GFX_SIZE_T Size, PHYSICAL_ADDRESS Page);
