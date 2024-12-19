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
============================================================================
**
** File Name     : igfxfmid.h
**
** Abastract     : Family IDs for device abstraction
**
** Authors       : Ken Stevens
**
** Environment   : Win9x and WinNT builds.
**
** Items in File : Family IDs for Intel Graphics Controllers
**
**  ---------------------------------------------------------------------- */
#ifndef __IGFXFMID_H__
#define __IGFXFMID_H__

typedef enum {
    IGFX_UNKNOWN        = 0,
    IGFX_GRANTSDALE_G,
    IGFX_ALVISO_G,
    IGFX_LAKEPORT_G,
    IGFX_CALISTOGA_G,
    IGFX_BROADWATER_G,
    IGFX_CRESTLINE_G,
    IGFX_BEARLAKE_G,
    IGFX_CANTIGA_G,
    IGFX_CEDARVIEW_G,
    IGFX_EAGLELAKE_G,
    IGFX_IRONLAKE_G,
    IGFX_GT,
    IGFX_IVYBRIDGE,
    IGFX_HASWELL,
    IGFX_VALLEYVIEW,
    IGFX_BROADWELL,
    IGFX_CHERRYVIEW,
    IGFX_SKYLAKE,
    IGFX_KABYLAKE,
    IGFX_COFFEELAKE,
    IGFX_WILLOWVIEW,
    IGFX_BROXTON,
    IGFX_GEMINILAKE,
    IGFX_CANNONLAKE,
    IGFX_ICELAKE,
    IGFX_ICELAKE_LP,
    IGFX_LAKEFIELD,
    IGFX_JASPERLAKE,
    IGFX_ELKHARTLAKE     =IGFX_JASPERLAKE,

    IGFX_TIGERLAKE_LP,
    IGFX_ROCKETLAKE,
    IGFX_ALDERLAKE_S,
    IGFX_ALDERLAKE_P,
    IGFX_ALDERLAKE_N,
    IGFX_DG1  = 1210,
    IGFX_XE_HP_SDV = 1250,
    IGFX_DG2 = 1270,
    IGFX_PVC = 1271,
    IGFX_METEORLAKE = 1272,
    IGFX_ARROWLAKE = 1273,
    IGFX_BMG = 1274,
    IGFX_LUNARLAKE = 1275,
    IGFX_PTL = 1300,
    
    IGFX_MAX_PRODUCT,
    IGFX_GENNEXT               = 0x7ffffffe,
    PRODUCT_FAMILY_FORCE_ULONG = 0x7fffffff
} PRODUCT_FAMILY;

typedef enum {
    PCH_UNKNOWN    = 0,
    PCH_IBX,            // Ibexpeak
    PCH_CPT,            // Cougarpoint,
    PCH_CPTR,           // Cougarpoint Refresh,
    PCH_PPT,            // Panther Point
    PCH_LPT,            // Lynx Point
    PCH_LPTR,           // Lynx Point Refresh
    PCH_WPT,            // Wildcat point
    PCH_SPT,            //
    PCH_KBP,            // Kabylake PCH
    PCH_CNP_LP,         // Cannonlake LP PCH
    PCH_CNP_H,          // Cannonlake Halo PCH
    PCH_ICP_LP,         // ICL LP PCH
    PCH_ICP_N,          // ICL N PCH
    PCH_LKF,            // LKF PCH
    PCH_TGL_LP,         // TGL LP PCH
    PCH_CMP_LP,         // CML LP PCH
    PCH_CMP_H,          // CML Halo PCH
    PCH_CMP_V,          // CML V PCH
    PCH_JSP_N,          // JSL N PCH Device IDs for JSL+ Rev02
    PCH_ADL_S,          // ADL_S PCH
    PCH_ADL_P,          // ADL_P PCH
    PCH_TGL_H,          // TGL H PCH
    PCH_ADL_N,          // ADL_N PCHDL
    PCH_MTL,            // MTL PCH
    PCH_ARL,            // ARL PCH
    
    PCH_PRODUCT_FAMILY_FORCE_ULONG = 0x7fffffff
} PCH_PRODUCT_FAMILY;

typedef enum {
    IGFX_UNKNOWN_CORE    = 0,
    IGFX_GEN3_CORE       = 1,   //Gen3 Family
    IGFX_GEN3_5_CORE     = 2,   //Gen3.5 Family
    IGFX_GEN4_CORE       = 3,   //Gen4 Family
    IGFX_GEN4_5_CORE     = 4,   //Gen4.5 Family
    IGFX_GEN5_CORE       = 5,   //Gen5 Family
    IGFX_GEN5_5_CORE     = 6,   //Gen5.5 Family
    IGFX_GEN5_75_CORE    = 7,   //Gen5.75 Family
    IGFX_GEN6_CORE       = 8,   //Gen6 Family
    IGFX_GEN7_CORE       = 9,   //Gen7 Family
    IGFX_GEN7_5_CORE     = 10,  //Gen7.5 Family
    IGFX_GEN8_CORE       = 11,  //Gen8 Family
    IGFX_GEN9_CORE       = 12,  //Gen9 Family
    IGFX_GEN10_CORE      = 13,  //Gen10 Family
    IGFX_GEN10LP_CORE    = 14,  //Gen10 LP Family
    IGFX_GEN11_CORE      = 15,  //Gen11 Family
    IGFX_GEN11LP_CORE    = 16,  //Gen11 LP Family
    IGFX_GEN12_CORE      = 17,  //Gen12 Family
    IGFX_GEN12LP_CORE    = 18,  //Gen12 LP Family
    IGFX_XE_HP_CORE      = 0x0c05,  //XE_HP family
    IGFX_XE_HPG_CORE     = 0x0c07,  // XE_HPG Family
    IGFX_XE_HPC_CORE     = 0x0c08,  // XE_HPC Family
    IGFX_XE2_LPG_CORE    = 0x0c09,  // XE2_LPG Family
    IGFX_XE2_HPG_CORE    = IGFX_XE2_LPG_CORE,  //XE2_HPG Family
    IGFX_XE3_CORE        = 0x1e00,  // XE3 Family

    //Please add new GENs BEFORE THIS !
    IGFX_MAX_CORE,

    IGFX_GENNEXT_CORE          = 0x7ffffffe,  //GenNext
    GFXCORE_FAMILY_FORCE_ULONG = 0x7fffffff
} GFXCORE_FAMILY;

typedef enum {
    IGFX_SKU_NONE		= 0,	// IGFX_SKU_UNKNOWN defined in \opengl\source\desktop\ail\ailgl_profiles.h
    IGFX_SKU_ULX        = 1,
    IGFX_SKU_ULT        = 2,
    IGFX_SKU_T          = 3,
    IGFX_SKU_ALL        = 0xff
} PLATFORM_SKU;

typedef enum __GTTYPE
{
    GTTYPE_GT1 = 0x0,
    GTTYPE_GT2,
    GTTYPE_GT2_FUSED_TO_GT1,
    GTTYPE_GT2_FUSED_TO_GT1_6, //IVB
    GTTYPE_GTL, // HSW
    GTTYPE_GTM, // HSW
    GTTYPE_GTH, // HSW
    GTTYPE_GT1_5,//HSW
    GTTYPE_GT1_75,//HSW
    GTTYPE_GT3,//BDW
    GTTYPE_GT4,//BDW
    GTTYPE_GT0,//BDW
    GTTYPE_GTA,// BXT
    GTTYPE_GTC,// BXT
    GTTYPE_GTX, // BXT
    GTTYPE_GT2_5,//CNL
    GTTYPE_GT3_5,//SKL
    GTTYPE_GT0_5,//CNL
    GTTYPE_UNDEFINED,//Always at the end.
}GTTYPE, *PGTTYPE;

/////////////////////////////////////////////////////////////////
//
//    Platform types which are used during Sku/Wa initialization.
//
#ifndef _COMMON_PPA
    typedef enum {
        PLATFORM_NONE       = 0x00,
        PLATFORM_DESKTOP    = 0x01,
        PLATFORM_MOBILE     = 0x02,
        PLATFORM_TABLET     = 0X03,
        PLATFORM_ALL        = 0xff, // flag used for applying any feature/WA for All platform types
    } PLATFORM_TYPE;
#endif
typedef struct PLATFORM_STR {
    PRODUCT_FAMILY      eProductFamily;
    PCH_PRODUCT_FAMILY  ePCHProductFamily;
    GFXCORE_FAMILY      eDisplayCoreFamily;
    GFXCORE_FAMILY      eRenderCoreFamily;
    #ifndef _COMMON_PPA
    PLATFORM_TYPE       ePlatformType;
    #endif

    unsigned short      usDeviceID;
    unsigned short      usRevId;
    unsigned short      usDeviceID_PCH;
    unsigned short      usRevId_PCH;
    // GT Type
    // Note: Is valid only till Gen9. From Gen10 SKUs are not identified by any GT flags. 'GT_SYSTEM_INFO' should be used instead.
    GTTYPE              eGTType;

} PLATFORM;

// add enums at the end
typedef enum __SKUIDTYPE
{
    SKU_FULL_TYPE = 0x0,
    SKU_VALUE_TYPE,
    SKU_PLUS_FULL_TYPE,
    SKU_PLUS_VALUE_TYPE,
    SKU_T_TYPE,
    SKU_PLUS_T_TYPE,
    SKU_P_TYPE,
    SKU_PLUS_P_TYPE,
    SKU_SMALL_TYPE,
    SKU_LIGHT_TYPE,
    SKU_N_TYPE
}SKUIDTYPE, *PSKUIDTYPE;

typedef enum __CPUTYPE
{
    CPU_UNDEFINED = 0x0,
    CPU_CORE_I3,
    CPU_CORE_I5,
    CPU_CORE_I7,
    CPU_PENTIUM,
    CPU_CELERON,
    CPU_CORE,
    CPU_VPRO,
    CPU_SUPER_SKU,
    CPU_ATOM,
    CPU_CORE1,
    CPU_CORE2,
    CPU_WS,
    CPU_SERVER,
    CPU_CORE_I5_I7,
    CPU_COREX1_4,
    CPU_ULX_PENTIUM,
    CPU_MB_WORKSTATION,
    CPU_DT_WORKSTATION,
    CPU_M3,
    CPU_M5,
    CPU_M7,
	CPU_MEDIA_SERVER //Added for KBL
}CPUTYPE, *PCPUTYPE;

// the code below convert platform real revision number to pre-defined revision number, the revision will be set as follow
// REVISION_A0 - this will include all incarnations for A stepping in all packages types A = {A0}
// REVISION_A1 - this will include all incarnations for A stepping in all packages types A = {A1}
// REVISION_A3 - this will include all incarnations for A stepping in all packages types A = {A3,...,A7}
// REVISION_B - this will include all incarnations for B stepping in all packages types B = {B0,B1,..,B7}
// REVISION_C - this will include all incarnations for C stepping in all packages types C = {C0,C1,..,C7}
// REVISION_D - this will include all incarnations for C stepping in all packages types C = {D0,D1}
// REVISION_K - this will include all incarnations for K stepping in all packages types K = {K0,K1,..,K7}
typedef enum __REVID
{
    REVISION_A0 = 0,
    REVISION_A1, //1
    REVISION_A3,//2
    REVISION_B,//3
    REVISION_C,//4
    REVISION_D,//5
    REVISION_K//6
}REVID, *PREVID;

typedef enum __NATIVEGTTYPE
{
    NATIVEGTTYPE_HSW_UNDEFINED  = 0x00,
    NATIVEGTTYPE_HSW_GT1        = 0x01,
    NATIVEGTTYPE_HSW_GT2        = 0x02,
    NATIVEGTTYPE_HSW_GT3        = 0x03,
}NATIVEGTTYPE;

// Following macros return true/false depending on the current PCH family
#define PCH_IS_PRODUCT(p, r)            ( (p).ePCHProductFamily == r )
#define PCH_GET_CURRENT_PRODUCT(p)      ( (p).ePCHProductFamily )

// These macros return true/false depending on current product/core family.
#define GFX_IS_PRODUCT(p, r)           ( (p).eProductFamily == r )
#define GFX_IS_DISPLAYCORE(p, d)       ( (p).eDisplayCoreFamily == d )
#define GFX_IS_RENDERCORE(p, r)        ( (p).eRenderCoreFamily == r )
// These macros return the current product/core family enum.
// Relational compares (</>) should not be done when using GFX_GET_CURRENT_PRODUCT
// macro.  There are no relationships between the PRODUCT_FAMILY enum values.
#define GFX_GET_CURRENT_PRODUCT(p)     ( (p).eProductFamily )
#define GFX_GET_CURRENT_DISPLAYCORE(p) ( (p).eDisplayCoreFamily )
#define GFX_GET_CURRENT_RENDERCORE(p)  ( (p).eRenderCoreFamily )

// This macro returns true if the product family is discrete
#define GFX_IS_DISCRETE_PRODUCT(pf)    ( ( pf == IGFX_DG1 )             ||   \
                                         ( pf == IGFX_DG2 )             ||   \
                                         ( pf == IGFX_XE_HP_SDV )       ||   \
                                         ( pf == IGFX_BMG ) )

#define GFX_IS_DISCRETE_FAMILY(p)      GFX_IS_DISCRETE_PRODUCT(GFX_GET_CURRENT_PRODUCT(p))

#define GFX_IS_INTEGRATED_PRODUCT(pf)  (!GFX_IS_DISCRETE_PRODUCT(pf))

#define GFX_IS_INTEGRATED_FAMILY(p)    (!GFX_IS_DISCRETE_FAMILY(p))


// These macros return true/false depending on the current render family.
#define GFX_IS_NAPA_RENDER_FAMILY(p)   ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN3_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN3_5_CORE ) )

#define GFX_IS_GEN_RENDER_FAMILY(p)    ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN4_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN4_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_75_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN6_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPC_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE2_HPG_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_5_OR_LATER(p)       ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_75_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN6_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPC_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE2_HPG_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_5_75_OR_LATER(p)    ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN5_75_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN6_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPC_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE2_HPG_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_6_OR_LATER(p)       ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN6_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPC_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE2_HPG_CORE ) ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_7_OR_LATER(p)       ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_5_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||   \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPC_CORE )  ||   \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE2_HPG_CORE ) ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_7_5_OR_LATER(p)     ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN7_5_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_XE_HPC_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_8_OR_LATER(p)       ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN8_CORE )    ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_XE_HPC_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_8_CHV_OR_LATER(p)   ( ( GFX_GET_CURRENT_PRODUCT(p) == IGFX_CHERRYVIEW )      ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_XE_HPC_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_9_OR_LATER(p)       ( ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN9_CORE )    ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_XE_HPC_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_10_OR_LATER(p)       (( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN10_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_XE_HPC_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_11_OR_LATER(p)       (( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN11_CORE )   ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GEN12_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HP_CORE )   ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_XE_HPG_CORE )  ||  \
					 ( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_XE_HPC_CORE )  ||  \
                                         ( GFX_GET_CURRENT_RENDERCORE(p) == IGFX_GENNEXT_CORE ) )

#define GFX_IS_GEN_12_OR_LATER(p)       (( GFX_GET_CURRENT_RENDERCORE(p) >= IGFX_GEN12_CORE ))
#define GFX_IS_ATOM_PRODUCT_FAMILY(p)  ( GFX_IS_PRODUCT(p, IGFX_VALLEYVIEW)   ||  \
                                         GFX_IS_PRODUCT(p, IGFX_CHERRYVIEW)   ||  \
                                         GFX_IS_PRODUCT(p, IGFX_BROXTON) )

///////////////////////////////////////////////////////////////////
//
// macros for comparing Graphics family and products
//
///////////////////////////////////////////////////////////////////
#define GFX_IS_FAMILY_EQUAL_OR_ABOVE(family1, family2) ((family1)>=(family2) ? true : false)
#define GFX_IS_FAMILY_EQUAL_OR_BELOW(family1, family2) ((family1)<=(family2) ? true : false)
#define GFX_IS_FAMILY_BELOW(family1, family2) ((family1)<(family2) ? true : false)
#define GFX_IS_PRODUCT_EQUAL_OR_ABOVE(product1, product2) ((product1)>=(product2) ? true : false)
#define GFX_IS_PRODUCT_EQUAL_OR_BELOW(product1, product2) ((product1)<=(product2) ? true : false)
#define GFX_IS_PRODUCT_BELOW(product1, product2)  ((product1) <(product2) ? true : false)

//Feature ID: Graphics PRD PC11.0 - Brookdale-G Support
//Description: Move device and vendor ID's to igfxfmid.h.
//  Add #include "igfxfmid.h".
//Other Files Modified: dispconf.c, kcconfig.c, kchmisc.c, kchsys.c,
//  driver.h, igfxfmid.h, imdefs.h, kchialm.h, kchname.h, softbios.h,
//  swbios.h, vddcomm.h, vidmini.h

#define INTEL_VENDOR_ID              0x8086   // Intel Corporation

//Device IDs
#define UNKNOWN_DEVICE_ID            0xFFFF   // Unknown device
#define IALM_DEVICE0_ID_M            0x3575
#define IALM_DEVICE0_ID_MG           0x3578
#define IALM_DEVICE1_ID              0x3576
#define IALM_DEVICE_ID               0x3577   // Almador Base
#define BROOKDALE_G_DEVICE_ID        0x2562   // Brookdale_G
#define IMGM_DEVICE_ID               0x3582   // MontaraGM Base
#define IMGM_DEVICE0_ID              0x3580
#define ISDG_DEVICE0_ID              0x2570   // Springdale-G Device 0 ID
#define ISDG_DEVICE1_ID              0x2571   // Springdale-G Device 1 ID
#define ISDG_DEVICE2_ID              0x2572   // Springdale-G Device 2 ID
#define ISDG_DEVICE_ID               ISDG_DEVICE2_ID // Springdale-G Graphics Controller Device ID
#define IGDG_DEVICE_F0_ID            0x2582   // Grantsdale-G graphics function 0 ID
#define IGDG_DEVICE_F1_ID            0x2782   // Grantsdale-G graphics function 1 ID
#define IALG_DEVICE_F0_ID            0x2592   // Alviso-G , function 0
#define IALG_DEVICE_F1_ID            0x2792   // Alviso-G , function 1

#define ILPG_DEVICE_F0_ID            0x2772   // Lakeport-G graphics function 0 ID
#define ILPG_DEVICE_F1_ID            0x2776   // Lakeport-G graphics function 1 ID

#define ICLG_DEVICE_F0_ID            0x27A2   // Calistoga-G graphics function 0 ID
#define ICLG_DEVICE_F1_ID            0x27A6   // Calistoga-G graphics function 1 ID
#define ICLG_GME_DEVICE_F0_ID        0x27AE   // Calistoga-G graphics function 0 ID for GME SKU

#define IBWG_DEVICE_F0_ID            0x2982   // Broadwater-G graphics function 0 ID
#define IBWG_DEVICE_F1_ID            0x2983   // Broadwater-G graphics function 1 ID

#define IBWG_GQ_DEVICE_F0_ID         0x2992   // Broadwater-GQ graphics function 0 ID
#define IBWG_GQ_DEVICE_F1_ID         0x2993   // Broadwater-GQ graphics function 1 ID

#define IBWG_GC_DEVICE_F0_ID         0x29A2   // Broadwater-GC graphics function 0 ID
#define IBWG_GC_DEVICE_F1_ID         0x29A3   // Broadwater-GC graphics function 1 ID

#define IBWG_GV_DEVICE_F0_ID         0x2972   // Broadwater-GL/GZ graphics function 0 ID
#define IBWG_GV_DEVICE_F1_ID         0x2973   // Broadwater-GL/GZ graphics function 1 ID

#define ICRL_DEVICE_F0_ID            0x2A02   // Crestline-G graphics function 0 ID
#define ICRL_DEVICE_F1_ID            0x2A03   // Crestline-G graphics function 1 ID
#define ICRL_GME_DEVICE_F0_ID        0x2A12   // Crestline-G graphics function 0 ID for GME SKU
#define ICRL_GME_DEVICE_F1_ID        0x2A13   // Crestline-G graphics function 1 ID for GME SKU

#define IBLK_GBA_DEVICE_F0_ID         0x29C2   // BearLake-GBA graphics function 0 ID
#define IBLK_GBA_DEVICE_F1_ID         0x29C3   // BearLake-GBA graphics function 1 ID

#define IBLK_GBB_DEVICE_F0_ID        0x29B2   // BearLake-GBB Corporate skew graphics function 0 ID
#define IBLK_GBB_DEVICE_F1_ID        0x29B3   // BearLake-GBB Corporate skew graphics function 1 ID

#define IBLK_GBC_DEVICE_F0_ID         0x29D2   // BearLake-GBC graphics function 0 ID
#define IBLK_GBC_DEVICE_F1_ID         0x29D3   // BearLake-GBC graphics function 1 ID

#define ICTG_DEVICE_F0_ID            0x2A42   // Cantiga-G graphics function 0 ID
#define ICTG_DEVICE_F1_ID            0x2A43   // Cantiga-G graphics function 1 ID

#define ICDV_DEVICE_F0_ID            0x0BE0   // CDV graphics function 0 ID
#define ICDV_DEVICE_F0_ID_MASK       0xFFF0   // Mask of Penwell_D graphics function 0 ID. Bits [3:0]
                                              // are used to identify SKU from B1 QS

#define IELK_DEVICE_SUPER_SKU_F0_ID     0x2E02   // EagleLake-G graphics function 0 ID for Super SKU
#define IELK_DEVICE_SUPER_SKU_F1_ID     0x2E03   // EagleLake-G graphics function 1 ID for Super SKU

#define IELK_DEVICE_Q45_Q43_SKU_F0_ID   0x2E12   // EagleLake-G graphics function 0 ID for Q45 and Q43 SKUs
#define IELK_DEVICE_Q45_Q43_SKU_F1_ID   0x2E13   // EagleLake-G graphics function 1 ID for Q45 and Q43 SKUs

#define IELK_DEVICE_G45_G43_P45_SKU_F0_ID  0x2E22 // EagleLake-G graphics function 0 ID for G45, G43 and P45 SKUs
#define IELK_DEVICE_G45_G43_P45_SKU_F1_ID  0x2E23 // EagleLake-G graphics function 1 ID for G45, G43 and P45 SKUs

#define IELK_DEVICE_G41_SKU_F0_ID      0x2E32  // EagleLake-G graphics function 0 ID for G41 SKU
#define IELK_DEVICE_G41_SKU_F1_ID      0x2E33  // EagleLake-G graphics function 1 ID for G41 SKU

#define IELK_DEVICE_F0_ID            0x2E02   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_F1_ID            0x2E03   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_Q45_F0_ID        0x2E12   // EagleLake-G graphics Corporate function 0 ID
#define IELK_DEVICE_Q45_F1_ID        0x2E13   // EagleLake-G graphics Corporate function 1 ID

#define IELK_DEVICE_Q43_F0_ID        0x2E22   // EagleLake-G graphics Corporate function 0 ID
#define IELK_DEVICE_Q43_F1_ID        0x2E23   // EagleLake-G graphics Corporate function 1 ID

#define IELK_DEVICE_G45_F0_ID        0x2E32   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_G45_F1_ID        0x2E33   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_G43_F0_ID        0x2E42   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_G43_F1_ID        0x2E43   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_B43_SKU_F0_ID    0x2E42   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_B43_SKU_F1_ID    0x2E43   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_G41_F0_ID        0x2E52   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_G41_F1_ID        0x2E53   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_P45_F0_ID        0x2E62   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_P45_F1_ID        0x2E63   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_P43_F0_ID        0x2E72   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_P43_F1_ID        0x2E73   // EagleLake-G graphics function 1 ID

#define IELK_DEVICE_B43_UPGRD_F0_ID  0x2E92   // EagleLake-G graphics function 0 ID
#define IELK_DEVICE_B43_UPGRD_F1_ID  0x2E93   // EagleLake-G graphics function 0 ID

#define IILK_DESK_DEVICE_F0_ID       0x0042   // IronLake-G Desktop graphics function 0 ID
#define IILK_MOBL_DEVICE_F0_ID       0x0046   // IronLake-G Mobile graphics function 0 ID

#define IGT_DESK_DEVICE_GT1_ID      0x0102   // GT / SandyBridge Desktop GT1 ID
#define IGT_MOBL_DEVICE_GT1_ID      0x0106   // GT / SandyBridge Mobile GT1 ID
#define IGT_DESK_SERVER_DEVICE_ID   0x010A   // GT / SandyBridge Server ID
#define IGT_DESK_DEVICE_GT2_ID      0x0112   // GT / SandyBridge Desktop GT2 ID
#define IGT_MOBL_DEVICE_GT2_ID      0x0116   // GT / SandyBridge Mobile GT2 ID
#define IGT_DESK_DEVICE_GT_PLUS_ID  0x0122   // GT / SandyBridge Desktop GT2+ ID
#define IGT_MOBL_DEVICE_GT_PLUS_ID  0x0126   // GT / SandyBridge Mobile GT2+ ID
#define IGT_DESK_VERSATILE_ACCELERATION_MODE_ID  0x010B //GT / SandyBridge Versatile Acceleration Mode. Desk/Mobl??

//ivb issue366162
#define IIVB_DESK_DEVICE_F0_ID       0x0162   // IVB Desktop graphics function 0 ID
#define IIVB_DESK_SERVER_DEVICE_ID   0x016A   // IVB Server graphics function 0 ID
#define IIVB_MOBL_DEVICE_F0_ID       0x0166   // IVB Mobile graphics function 0 ID

#define IIVB_GT1_DESK_DEVICE_F0_ID       0x0152   // IVB Desktop graphics function 0 ID
#define IIVB_GT1_DESK_SERVER_DEVICE_ID   0x015A   // IVB Server graphics function 0 ID
#define IIVB_GT1_MOBL_DEVICE_F0_ID       0x0156   // IVB Mobile graphics function 0 ID

//ivb placeholder
#define IIVB_GT2p_DESK_DEVICE_F0_ID    0x0172
#define IIVB_GT2p_MOBL_DEVICE_F0_ID    0x0176

#define IIVB_GT16_DESK_DEVICE_F0_ID    0x0182
#define IIVB_GT16_MOBL_DEVICE_F0_ID    0x0186

#define IIVB_GT2_XEON_DEVICE_F0_ID     0x016A

//HSW TEMP
#define IHSW_GTH_DESK_DEVICE_F0_ID       0x0090   // HSW Desktop graphics function 0 ID
#define IHSW_GTM_DESK_DEVICE_F0_ID       0x0091
#define IHSW_GTL_DESK_DEVICE_F0_ID       0x0092
#define IHSW_DESK_DEV_F0_ID              0x0C02  // HSW Desktop Device ID
#define IHSW_MOBL_DEV_F0_ID              0x0C06  // HSW Mobile Device ID
#define IHSW_DESK_DEV_F0_M_ID            0x0C12  // HSW Desktop Device ID GT - M SKU.
#define IHSW_MOBL_DEV_F0_M_ID            0x0C16  // HSW Mobile Device ID GT - M SKU.
#define IHSW_DESK_DEV_F0_H_ID            0x0C22  // HSW Desktop Device ID GT - H SKU.
#define IHSW_MOBL_DEV_F0_H_ID            0x0C26  // HSW Mobile Device ID GT - H SKU.
#define IHSW_VA_DEV_F0_ID                0x0C0B  // HSW Mobile Device ID GT - H SKU.


#define IHSW_MOBL_DEVICE_F0_ID          0x0094 // Not used currently

//HSW - client
#define IHSW_CL_DESK_GT1_DEV_ID              0x402 //CLIENT Desktop - GT1
#define IHSW_CL_MOBL_GT1_DEV_ID              0x406 //CLIENT Mobile - GT1
#define IHSW_CL_SERV_GT1_DEV_ID              0x40A //CLIENT Server - GT1
#define IHSW_CL_DESK_GT2_DEV_ID              0x412
#define IHSW_CL_MOBL_GT2_DEV_ID              0x416
#define IHSW_CL_WS_GT2_DEV_ID                0x41B // Use for WS GT2 SKU, not confirmed
#define IHSW_CL_SERV_GT2_DEV_ID              0x41A
#define IHSW_CL_MRKT_GT1_5_DEV_ID            0x41E // Use for GT1.5w/12EUs AKA GT2.12

//HSW - ULT

#define IHSW_ULT_MOBL_GT1_DEV_ID              0xA06
#define IHSW_ULT_MOBL_GT2_DEV_ID              0xA16 //Mob ULT GT1.5 and GT2
#define IHSW_ULT_MOBL_GT3_DEV_ID              0xA26
#define IHSW_ULT_MRKT_GT3_DEV_ID              0xA2E

// HSW - ULX

#define IHSW_ULX_MOBL_GT1_DEV_ID              0xA0E
#define IHSW_ULX_MOBL_GT2_DEV_ID              0xA1E //Mob ULX GT1.5 and GT2

//HSW - CRW

#define IHSW_CRW_DESK_GT2_DEV_ID              0xD12
#define IHSW_CRW_MOBL_GT2_DEV_ID              0xD16
#define IHSW_CRW_DESK_GT3_DEV_ID              0xD22
#define IHSW_CRW_MOBL_GT3_DEV_ID              0xD26
#define IHSW_CRW_SERV_GT3_DEV_ID              0xD2A

//VLV device ids
#define IVLV_DESK_DEVICE_F0_ID           0x0F32
#define IVLV_MOBL_DEVICE_F0_ID           0x0F30   // VLV Mobile graphics function 0 ID
//VLV Plus device ids
#define IVLV_PLUS_DESK_DEVICE_F0_ID      0x0F33
#define IVLV_PLUS_MOBL_DEVICE_F0_ID      0x0F31   // VLV Plus Mobile graphics function 0 ID

//CHV device ids
#define ICHV_MOBL_DEVICE_F0_ID           0x22B0   // CHV TABLET i.e CHT
#define ICHV_PLUS_MOBL_DEVICE_F0_ID      0x22B1   // Essential i.e Braswell
#define ICHV_DESK_DEVICE_F0_ID           0x22B2   // Reserved
#define ICHV_PLUS_DESK_DEVICE_F0_ID      0x22B3   // Reserved

//BDW device ids
#define IBDW_GT1_HALO_MOBL_DEVICE_F0_ID         0x1602
#define IBDW_GT1_ULT_MOBL_DEVICE_F0_ID          0x1606
#define IBDW_GT1_RSVD_DEVICE_F0_ID              0x160B
#define IBDW_GT1_SERV_DEVICE_F0_ID              0x160A
#define IBDW_GT1_WRK_DEVICE_F0_ID               0x160D
#define IBDW_GT1_ULX_DEVICE_F0_ID               0x160E
#define IBDW_GT2_HALO_MOBL_DEVICE_F0_ID         0x1612
#define IBDW_GT2_ULT_MOBL_DEVICE_F0_ID          0x1616
#define IBDW_GT2_RSVD_DEVICE_F0_ID              0x161B
#define IBDW_GT2_SERV_DEVICE_F0_ID              0x161A
#define IBDW_GT2_WRK_DEVICE_F0_ID               0x161D
#define IBDW_GT2_ULX_DEVICE_F0_ID               0x161E
#define IBDW_GT3_HALO_MOBL_DEVICE_F0_ID         0x1622
#define IBDW_GT3_ULT_MOBL_DEVICE_F0_ID          0x1626
#define IBDW_GT3_ULT25W_MOBL_DEVICE_F0_ID       0x162B //This is actually 28w
#define IBDW_GT3_SERV_DEVICE_F0_ID              0x162A
#define IBDW_GT3_WRK_DEVICE_F0_ID               0x162D
#define IBDW_GT3_ULX_DEVICE_F0_ID               0x162E
#define IBDW_RSVD_MRKT_DEVICE_F0_ID             0x1632
#define IBDW_RSVD_ULT_MOBL_DEVICE_F0_ID         0x1636
#define IBDW_RSVD_HALO_MOBL_DEVICE_F0_ID        0x163B
#define IBDW_RSVD_SERV_DEVICE_F0_ID             0x163A
#define IBDW_RSVD_WRK_DEVICE_F0_ID              0x163D
#define IBDW_RSVD_ULX_DEVICE_F0_ID              0x163E

//skl placeholder

#define ISKL_GT4_DT_DEVICE_F0_ID                0x1932
#define ISKL_GT2_DT_DEVICE_F0_ID                0x1912 // Used on actual Silicon

#define ISKL_GT1_DT_DEVICE_F0_ID                0x1902


#define ISKL_GT2_ULT_DEVICE_F0_ID               0x1916
#define ISKL_GT2F_ULT_DEVICE_F0_ID              0x1921
#define ISKL_GT3e_ULT_DEVICE_F0_ID_540          0x1926
#define ISKL_GT3e_ULT_DEVICE_F0_ID_550          0x1927

#define ISKL_GT2_ULX_DEVICE_F0_ID               0x191E
#define ISKL_GT1_ULT_DEVICE_F0_ID               0x1906
#define ISKL_GT3_MEDIA_SERV_DEVICE_F0_ID        0x192D
#define ISKL_GT1_5_ULT_DEVICE_F0_ID             0x1913

#define ISKL_GT3_ULT_DEVICE_F0_ID               0x1923

#define ISKL_GT2_HALO_MOBL_DEVICE_F0_ID         0x191B

#define ISKL_GT4_HALO_MOBL_DEVICE_F0_ID         0x193B
#define ISKL_GT4_SERV_DEVICE_F0_ID				0x193A
#define ISKL_GT2_WRK_DEVICE_F0_ID				0x191D
#define ISKL_GT4_WRK_DEVICE_F0_ID				0x193D


#define ISKL_GT0_DESK_DEVICE_F0_ID              0x0900
#define ISKL_GT1_DESK_DEVICE_F0_ID              0x0901
#define ISKL_GT2_DESK_DEVICE_F0_ID              0x0902
#define ISKL_GT3_DESK_DEVICE_F0_ID              0x0903
#define ISKL_GT4_DESK_DEVICE_F0_ID              0x0904
#define ISKL_GT1_ULX_DEVICE_F0_ID               0x190E
//SKL strings to be be deleted in future

#define ISKL_GT1_HALO_MOBL_DEVICE_F0_ID         0x190B
#define ISKL_GT1_SERV_DEVICE_F0_ID				0x190A
#define ISKL_GT1_5_ULX_DEVICE_F0_ID             0x1915
#define ISKL_GT1_5_DT_DEVICE_F0_ID              0x1917
#define ISKL_GT2_SERV_DEVICE_F0_ID				0x191A
#define ISKL_LP_DEVICE_F0_ID                    0x9905
#define ISKL_GT3_HALO_MOBL_DEVICE_F0_ID         0x192B
#define ISKL_GT3_SERV_DEVICE_F0_ID				0x192A
#define ISKL_GT0_MOBL_DEVICE_F0_ID              0xFFFF





// KabyLake Device ids
#define IKBL_GT1_ULT_DEVICE_F0_ID               0x5906
#define IKBL_GT1_5_ULT_DEVICE_F0_ID             0x5913
#define IKBL_GT2_ULT_DEVICE_F0_ID               0x5916
#define IKBL_GT2F_ULT_DEVICE_F0_ID              0x5921
#define IKBL_GT2_R_ULX_DEVICE_F0_ID             0x591C
#define IKBL_GT3_15W_ULT_DEVICE_F0_ID           0x5926
//#define IKBL_GT3E_ULT_DEVICE_F0_ID              0x5926
#define IKBL_GT1_ULX_DEVICE_F0_ID               0x590E
#define IKBL_GT1_5_ULX_DEVICE_F0_ID             0x5915
#define IKBL_GT2_ULX_DEVICE_F0_ID               0x591E
#define IKBL_GT1_DT_DEVICE_F0_ID                0x5902
#define IKBL_GT2_R_ULT_DEVICE_F0_ID             0x5917
#define IKBL_GT2_DT_DEVICE_F0_ID                0x5912
#define IKBL_GT1_HALO_DEVICE_F0_ID              0x590B
#define IKBL_GT1F_HALO_DEVICE_F0_ID             0x5908
#define IKBL_GT2_HALO_DEVICE_F0_ID              0x591B
#define IKBL_GT4_HALO_DEVICE_F0_ID              0x593B
#define IKBL_GT1_SERV_DEVICE_F0_ID              0x590A
#define IKBL_GT2_SERV_DEVICE_F0_ID              0x591A
#define IKBL_GT2_WRK_DEVICE_F0_ID               0x591D
#define IKBL_GT3_ULT_DEVICE_F0_ID               0x5923
#define IKBL_GT3_28W_ULT_DEVICE_F0_ID           0x5927
//keeping the below ids as its been used in linux . need to be removed once removed from linux files.
#define IKBL_GT4_DT_DEVICE_F0_ID                0x5932
#define IKBL_GT3_HALO_DEVICE_F0_ID              0x592B
#define IKBL_GT3_SERV_DEVICE_F0_ID              0x592A
#define IKBL_GT4_SERV_DEVICE_F0_ID              0x593A
#define IKBL_GT4_WRK_DEVICE_F0_ID               0x593D

//GLK Device ids
#define IGLK_GT2_ULT_18EU_DEVICE_F0_ID          0x3184
#define IGLK_GT2_ULT_12EU_DEVICE_F0_ID          0x3185

//GWL
#define IGWL_GT1_MOB_DEVICE_F0_ID               0xFF0F      //For Pre-Si, temp

#define IBXT_A_DEVICE_F0_ID                 0x9906
#define IBXT_C_DEVICE_F0_ID                 0x9907
#define IBXT_X_DEVICE_F0_ID                 0x9908

//BXT BIOS programmed Silicon ids
#define IBXT_GT_3x6_DEVICE_ID                0x0A84
#define IBXT_PRO_3x6_DEVICE_ID               0x1A84 //18EU
#define IBXT_PRO_12EU_3x6_DEVICE_ID          0x1A85 //12 EU
#define IBXT_P_3x6_DEVICE_ID                 0x5A84 //18EU APL
#define IBXT_P_12EU_3x6_DEVICE_ID            0x5A85 //12EU APL

#define ICNL_3x8_DESK_DEVICE_F0_ID              0x0A01
#define ICNL_5x8_DESK_DEVICE_F0_ID              0x0A02
#define ICNL_9x8_DESK_DEVICE_F0_ID              0x0A05
#define ICNL_13x8_DESK_DEVICE_F0_ID             0x0A07

// CNL Si device ids
#define ICNL_5x8_ULX_DEVICE_F0_ID               0x5A51      //GT2
#define ICNL_5x8_ULT_DEVICE_F0_ID               0x5A52      //GT2
#define ICNL_4x8_ULT_DEVICE_F0_ID               0x5A5A      //GT1.5
#define ICNL_3x8_ULT_DEVICE_F0_ID               0x5A42      //GT1
#define ICNL_2x8_ULT_DEVICE_F0_ID               0x5A4A      //GT0.5
#define ICNL_9x8_ULT_DEVICE_F0_ID               0x5A62
#define ICNL_9x8_SUPERSKU_DEVICE_F0_ID          0x5A60
#define ICNL_5x8_SUPERSKU_DEVICE_F0_ID          0x5A50      //GT2
#define ICNL_1x6_5x8_SUPERSKU_DEVICE_F0_ID      0x5A40      //GTx
#define ICNL_5x8_HALO_DEVICE_F0_ID              0x5A54      //GT2
#define ICNL_3x8_HALO_DEVICE_F0_ID              0x5A44      //GT1
#define ICNL_5x8_DESKTOP_DEVICE_F0_ID           0x5A55
#define ICNL_3x8_DESKTOP_DEVICE_F0_ID           0x5A45
#define ICNL_4x8_ULX_DEVICE_F0_ID               0x5A59      //GT1.5
#define ICNL_3x8_ULX_DEVICE_F0_ID               0x5A41      //GT1
#define ICNL_2x8_ULX_DEVICE_F0_ID               0x5A49      //GT0.5
#define ICNL_4x8_HALO_DEVICE_F0_ID              0x5A5C      //GT1.5
#define ICNL_2x8_HALO_DEVICE_F0_ID              0x5A4C      //GT0.5

//CFL
#define ICFL_GT1_DT_DEVICE_F0_ID                0x3E90
#define ICFL_GT2_DT_DEVICE_F0_ID                0x3E92

#define ICFL_GT1_S61_DT_DEVICE_F0_ID            0x3E90
#define ICFL_GT1_S41_DT_DEVICE_F0_ID            0x3E93
#define ICFL_GT2_S62_DT_DEVICE_F0_ID            0x3E92
#define ICFL_GT2_HALO_DEVICE_F0_ID              0x3E9B
#define ICFL_GT2_SERV_DEVICE_F0_ID              0x3E96
#define ICFL_GT2_HALO_WS_DEVICE_F0_ID           0x3E94
#define ICFL_GT2_S42_DT_DEVICE_F0_ID            0x3E91
#define ICFL_GT3_ULT_15W_DEVICE_F0_ID           0x3EA6
#define ICFL_GT3_ULT_15W_42EU_DEVICE_F0_ID      0x3EA7
#define ICFL_GT3_ULT_28W_DEVICE_F0_ID           0x3EA8
#define ICFL_GT3_ULT_DEVICE_F0_ID               0x3EA5
#define ICFL_HALO_DEVICE_F0_ID                  0x3E95
#define ICFL_GT2_S8_S2_DT_DEVICE_F0_ID          0x3E98
#define ICFL_GT1_S6_S4_S2_F1F_DT_DEVICE_F0_ID   0x3E99
#define ICFL_GT2_S82_S6F2_DT_DEVICE_F0_ID       0x3E9A
#define ICFL_GT2_U42F_U2F2_ULT_DEVICE_F0_ID     0x3EA0
#define ICFL_GT1_U41F_U2F1F_ULT_DEVICE_F0_ID    0x3EA1
#define ICFL_GT3_U43_ULT_DEVICE_F0_ID           0x3EA2
#define ICFL_GT2_U42F_U2F1F_ULT_DEVICE_F0_ID    0x3EA3
#define ICFL_GT1_41F_2F1F_ULT_DEVICE_F0_ID      0x3EA4
#define ICFL_GT2_U42F_U2F2F_ULT_DEVICE_F0_ID    0x3EA9

//CML- continue to follow CFL Macro
#define ICFL_GT2_ULT_DEVICE_V0_ID               0x9B41
#define ICFL_GT1_ULT_DEVICE_V0_ID               0x9B21
#define ICFL_GT2_ULT_DEVICE_A0_ID               0x9BCA
#define ICFL_GT1_ULT_DEVICE_A0_ID               0x9BAA
#define ICFL_GT2_ULT_DEVICE_S0_ID               0x9BCB
#define ICFL_GT1_ULT_DEVICE_S0_ID               0x9BAB
#define ICFL_GT2_ULT_DEVICE_K0_ID               0x9BCC
#define ICFL_GT1_ULT_DEVICE_K0_ID               0x9BAC
#define ICFL_GT2_ULX_DEVICE_S0_ID               0x9BC0
#define ICFL_GT1_ULX_DEVICE_S0_ID               0x9BA0
#define ICFL_GT2_DT_DEVICE_P0_ID                0x9BC5
#define ICFL_GT1_DT_DEVICE_P0_ID                0x9BA5
#define ICFL_GT2_DT_DEVICE_G0_ID                0x9BC8
#define ICFL_GT1_DT_DEVICE_G0_ID                0x9BA8
#define ICFL_GT2_WKS_DEVICE_P0_ID               0x9BC6
#define ICFL_GT2_WKS_DEVICE_G0_ID               0x9BE6
#define ICFL_GT2_HALO_DEVICE_15_ID              0x9BC4
#define ICFL_GT1_HALO_DEVICE_16_ID              0x9BA4
#define ICFL_GT2_HALO_DEVICE_17_ID              0x9BC2
#define ICFL_GT1_HALO_DEVICE_18_ID              0x9BA2




// PCH related definitions
#define PCH_IBX_DESK_DEVICE_FULL_ID             0x3B00
#define PCH_IBX_DESK_DEVICE_P55_ID              0x3B02
#define PCH_IBX_DESK_DEVICE_H55_ID              0x3B06
#define PCH_IBX_DESK_DEVICE_H57_ID              0x3B08
#define PCH_IBX_DESK_DEVICE_Q57_ID              0x3B0A

#define PCH_IBX_MOBL_DEVICE_FULL_ID             0x3B01
#define PCH_IBX_MOBL_DEVICE_PM55_ID             0x3B03
#define PCH_IBX_MOBL_DEVICE_QM57_ID             0x3B07
#define PCH_IBX_MOBL_DEVICE_HM55_ID             0x3B09
#define PCH_IBX_MOBL_DEVICE_HM57_ID             0x3B0B
#define PCH_IBX_MOBL_DEVICE_SFF_FULL_ID         0x3B0D

#define PCH_IBX_DEVICE_QS57_ID                  0x3B0F
#define PCH_IBX_DEVICE_3400_ID                  0x3B12
#define PCH_IBX_DEVICE_3420_ID                  0x3B14
#define PCH_IBX_DEVICE_3450_ID                  0x3B16

#define PCH_CPT_UNFUSED_PART_DEV_ID             0x1C40
#define PCH_CPT_DESKTOP_SUPER_SKU_DEV_ID        0x1C42
#define PCH_CPT_MOBL_SUPER_SKU_DEV_ID           0x1C43
#define PCH_CPT_DESKTOP_DH_SKU_DEV_ID           0x1C44
#define PCH_CPT_DESKTOP_DO_SKU_DEV_ID           0x1C46
#define PCH_CPT_MOBL_DO_SKU_DEV_ID              0x1C47
#define PCH_CPT_DESKTOP_R_SKU_DEV_ID            0x1C48
#define PCH_CPT_MOBL_ENHANCED_SKU_DEV_ID        0x1C49
#define PCH_CPT_DESKTOP_BASE_SKU_DEV_ID         0x1C4A
#define PCH_CPT_MOBL_BASE_SKU_DEV_ID            0x1C4B
#define PCH_CPT_Q65_SKU_DEV_ID                  0x1C4C
#define PCH_CPT_QS67_SKU_DEV_ID                 0x1C4D
#define PCH_CPT_Q67_SKU_DEV_ID                  0x1C4E
#define PCH_CPT_QM67_SKU_DEV_ID                 0x1C4F
#define PCH_CPT_B65_SKU_DEV_ID                  0x1C50
#define PCH_CPT_ESSENTIAL_SKU_DEV_ID            0x1C52
#define PCH_CPT_STANDARD_SKU_DEV_ID             0x1C54
#define PCH_CPT_ADVANCED_SKU_DEV_ID             0x1C56
#define PCH_CPT_B65_SKU_LEVEL_III_DEV_ID        0x1C58
#define PCH_CPT_HM67_SKU_LEVEL_III_DEV_ID       0x1C59
#define PCH_CPT_Q67_SKU_LEVEL_I_DEV_ID          0x1C5A
#define PCH_CPT_H61_SKU_LEVEL_I_DEV_ID          0x1C5C

// CPT Refresh Device IDs..
#define PCH_CPT_REF_RSVD1_DEV_ID                      0x1CC0 //Reserved for future use
#define PCH_CPT_REF_MOB_SFF_SUPER_DEV_ID        0x1CC1
#define PCH_CPT_REF_MOB_SUPER_DEV_ID            0x1CC2
#define PCH_CPT_REF_RSVD2_DEV_ID                    0x1CC3 //Reserved for future use
#define PCH_CPT_REF_QM77_DEV_ID                 0x1CC4
#define PCH_CPT_REF_QS77_DEV_ID                 0x1CC5
#define PCH_CPT_REF_HM77_DEV_ID                 0x1CC6
#define PCH_CPT_REF_UM77_DEV_ID                 0x1CC7
#define PCH_CPT_REF_HM75_DEV_ID                 0x1CC8
#define PCH_CPT_REF_HM75_RAID_RST_DEV_ID        0x1CC9
#define PCH_CPT_REF_UM77_RAID_RST_DEV_ID        0x1CCA
#define PCH_CPT_REF_HM77_MNG_DEV_ID             0x1CCB
#define PCH_CPT_REF_RSVD3_DEV_ID            0x1CCC
#define PCH_CPT_REF_RSVD4_DEV_ID            0x1CCD
#define PCH_CPT_REF_RSVD5_DEV_ID            0x1CCE
#define PCH_CPT_REF_RSVD6_DEV_ID            0x1CCF
#define PCH_CPT_REF_RSVD7_DEV_ID            0x1CD0
#define PCH_CPT_REF_RSVD8_DEV_ID            0x1CD1
#define PCH_CPT_REF_RSVD9_DEV_ID            0x1CD2
#define PCH_CPT_REF_RSVD10_DEV_ID            0x1CD3
#define PCH_CPT_REF_RSVD11_DEV_ID            0x1CD4
#define PCH_CPT_REF_RSVD12_DEV_ID            0x1CD5
#define PCH_CPT_REF_RSVD13_DEV_ID            0x1CD6
#define PCH_CPT_REF_RSVD14_DEV_ID            0x1CD7
#define PCH_CPT_REF_RSVD15_DEV_ID            0x1CD8
#define PCH_CPT_REF_RSVD16_DEV_ID            0x1CD9
#define PCH_CPT_REF_RSVD17_DEV_ID            0x1CDA
#define PCH_CPT_REF_RSVD18_DEV_ID            0x1CDB
#define PCH_CPT_REF_RSVD19_DEV_ID            0x1CDC
#define PCH_CPT_REF_RSVD20_DEV_ID            0x1CDD
#define PCH_CPT_REF_RSVD21_DEV_ID            0x1CDE
#define PCH_CPT_REF_RSVD22_DEV_ID            0x1CDF



//PPT Device IDs..
#define PCH_PPT_DEV_ID                                      0x1E40
#define PCH_PPT_DESK_SUPER_DEV_ID                           0x1E41
#define PCH_PPT_MOB_SUPER_DEV_ID                            0x1E42
#define PCH_PPT_MOB_SFF_SUPER_DEV_ID                        0x1E43
#define PCH_PPT_Z7x_DEV_ID                                  0x1E44
#define PCH_PPT_H71_DEV_ID                                  0x1E45
#define PCH_PPT_P77_DEV_ID                                  0x1E46
#define PCH_PPT_Q77_DEV_ID                                  0x1E47
#define PCH_PPT_Q75_DEV_ID                                  0x1E48
#define PCH_PPT_B75_DEV_ID                                  0x1E49
#define PCH_PPT_H77_DEV_ID                                  0x1E4A
#define PCH_PPT_B75_MNG_DEV_ID                              0x1E4B
#define PCH_PPT_B75_RAID_RST_DEV_ID                         0x1E4C
#define PCH_PPT_B75_RAID_RST_MNG_DEV_ID                     0x1E4D
#define PCH_PPT_H77_RST_DEV_ID                              0x1E4E
#define PCH_PPT_P77_RST_DEV_ID                              0x1E4F
#define PCH_PPT_Q77_RST_DEV_ID                              0x1E50
#define PCH_PPT_H71_RAID_RST_DEV_ID                         0x1E51
#define PCH_PPT_Q75_RAID_RST_DEV_ID                         0x1E52
#define PCH_PPT_SERVER_DEV_ID                               0x1E53
#define PCH_PPT_RSVD1_DEV_ID                                0x1E54
#define PCH_PPT_QM78_DEV_ID                                 0x1E55
#define PCH_PPT_QS78_DEV_ID                                 0x1E56
#define PCH_PPT_HM78_DEV_ID                                 0x1E57
#define PCH_PPT_UM78_DEV_ID                                 0x1E58
#define PCH_PPT_HM76_DEV_ID                                 0x1E59
#define PCH_PPT_HM76_RAID_RST_DEV_ID                        0x1E5A
#define PCH_PPT_UM78_RAID_RST_DEV_ID                        0x1E5B
#define PCH_PPT_HM78_MNG_DEV_ID                             0x1E5C
#define PCH_PPT_DEV_RSVD2_USB1_ID                           0x1E5D
#define PCH_PPT_DEV_RSVD3_USB2_ID                           0x1E5E
#define PCH_PPT_DEV_RSVD4_ID                                0x1E5F

//define LPT device ids
#define PCH_LPT_DEV_ID                                      0x8C40
#define PCH_LPT_DESK_SUPER_DEV_ID                           0x8C41
#define PCH_LPT_MOB_SUPER_DEV_ID                            0x8C42
#define PCH_LPT_DEV_RSVD1_ID                                0x8C43
#define PCH_LPT_Z87_DEV_ID                                  0x8C44
#define PCH_LPT_DEV_RSVD2_ID                                0x8C45
#define PCH_LPT_Z85_DEV_ID                                  0x8C46
#define PCH_LPT_DEV_RSVD3_ID                                0x8C47
#define PCH_LPT_DEV_RSVD4_ID                                0x8C48
#define PCH_LPT_HM86_DEV_ID                                 0x8C49
#define PCH_LPT_H87DEV_ID                                   0x8C4A
#define PCH_LPT_HM87_DEV_ID                                 0x8C4B
#define PCH_LPT_Q85_DEV_ID                                  0x8C4C
#define PCH_LPT_DEV_RSVD5_ID                                0x8C4D
#define PCH_LPT_Q87_DEV_ID                                  0x8C4E
#define PCH_LPT_QM87_DEV_ID                                 0x8C4F
#define PCH_LPT_B85_DEV_ID                                  0x8C50
#define PCH_LPT_DEV_RSVD6_ID                                0x8C51
#define PCH_LPT_SERVER_ESS_DEV_ID                           0x8C52
#define PCH_LPT_DEV_RSVD7_ID                                0x8C53
#define PCH_LPT_SERVER_STD_DEV_ID                           0x8C54
#define PCH_LPT_DEV_RSVD8_ID                                0x8C55
#define PCH_LPT_SERVER_ADV_DEV_ID                           0x8C56
#define PCH_LPT_DEV_RSVD9_ID                                0x8C57
#define PCH_LPT_WS_DEV_ID                                   0x8C58
#define PCH_LPT_DEV_RSVD10_ID                               0x8C59
#define PCH_LPT_DEV_RSVD11_ID                               0x8C5A
#define PCH_LPT_DEV_RSVD12_ID                               0x8C5B
#define PCH_LPT_H81_DEV_ID                                  0x8C5C
#define PCH_LPT_DEV_RSVD13_ID                               0x8C5D
#define PCH_LPT_DEV_RSVD14_ID                               0x8C5E
#define PCH_LPT_DEV_RSVD15_ID                               0x8C5F
#define PCH_LPT_MOB_FFE_DEV_ID								0x8CC1 // D31:F0 - LPC Controller (Mobile Full Featured Engineering Sample)
#define PCH_LPT_DESK_FFE_DEV_ID								0X8CC2 // D31:F0 - LPC Controller (Desktop Full Featured Engineering Sample)
#define PCH_LPT_HM97_DEV_ID									0X8CC3 // D31:F0 - LPC Controller (HM97 SKU)
#define PCH_LPT_Z97_DEV_ID									0X8CC4 // D31:F0 - LPC Controller (Z97 SKU)
#define PCH_LPT_QM97_DEV_ID									0X8CC5 // D31:F0 - LPC Controller (QM97 SKU)
#define PCH_LPT_H97_DEV_ID									0X8CC6 // D31:F0 - LPC Controller (H97 SKU)

#define PCH_LPT_LP_DEV_UNFUSED_ID                           0x9C40
#define PCH_LPT_LP_DEV_SUPER_ID                             0x9C41
#define PCH_LPT_LP_DEV_PREMIUM_ID                           0x9C43
#define PCH_LPT_LP_DEV_MAINSTREAM_ID                        0x9C45
#define PCH_LPT_LP_DEV_VALUE_ID                             0x9C47

//define WPT device ids
#define PCH_WPT_DEV_SIM_ID                                  0x99FF
#define PCH_WPT_LPC_DEV_UNFUSED_ID                          0x9CC0 // for BDW P0, D31:F0 - LPC Controller (Unfused part)
#define PCH_WPT_LPC_DEV_SUPER_HSW_ID                        0x9CC1 // for BDW P0, D31:F0 - LPC Controller (Super SKU) w/ HSW
#define PCH_WPT_LPC_DEV_SUPER_BDW_U_CPU_ID                  0x9CC2 // for BDW P0, D31:F0 - LPC Controller (Super SKU) w/ BDW U CPU
#define PCH_WPT_LPC_DEV_PREMIUM_BDW_U_CPU_ID                0x9CC3 // for BDW P0, D31:F0 - LPC Controller (Premimum SKU) w/ BDW U CPU
#define PCH_WPT_LPC_DEV_BASE_BDW_U_CPU_ID                   0x9CC5 // for BDW P0, D31:F0 - LPC Controller (Base SKU) w/ BDW U CPU
#define PCH_WPT_LPC_DEV_SUPER_BDW_Y_CPU_ID                  0x9CC6 // for BDW P0, D31:F0 - LPC Controller (Super SKU) w/ BDW Y CPU
#define PCH_WPT_LPC_DEV_PREMIUM_BDW_Y_CPU_ID                0x9CC7 // for BDW P0, D31:F0 - LPC Controller (Premimum SKU) w/ BDW Y CPU
#define PCH_WPT_LPC_DEV_BASE_BDW_Y_CPU_ID                   0x9CC9 // for BDW P0, D31:F0 - LPC Controller (Base SKU) w/ BDW Y CPU
#define PCH_WPT_LPC_DEV_PERFORMANCE_ID                      0x9CCB // for BDW P0, D31:F0 - LPC Controller (Performance SKU)

//define SPT LP device ids
#define PCH_SPT_DEV_SIM_ID                                  0x99EF
#define PCH_SPT_LPC_DEV_UNFUSED_ID                          0x9D40 // (SPT-LP)  Unfused
#define PCH_SPT_LPC_DEV_SUPER_U_ID                          0x9D41 // (SPT-LP)  Super SKU(Unlocked)
#define PCH_SPT_LPC_DEV_SUPER_L_ID                          0x9D42 // (SPT-LP)  Super SKU(locked)
#define PCH_SPT_LPC_DEV_BASE_SKL_U_ID                       0x9D43 // (SPT-LP)  Base Consumer/Corp SKL-U
#define PCH_SPT_LPC_DEV_P1_ID                               0x9D44 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_P2_ID                               0x9D45 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PREMIUM_SKL_Y_ID                    0x9D46 // (SPT-LP)  Premium Consumer/Corp SKL-Y
#define PCH_SPT_LPC_DEV_P3_ID                               0x9D47 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PREMIUM_SKL_U_ID                    0x9D48 // (SPT-LP)  Premium Consumer/Corp SKL-U, yet to be finalized
#define PCH_SPT_DEV_PREMIUM_KBL_Y_ID                        0x9D4B // (PCH-SPT)  Premium Consumer/Corp KBL-Y
#define PCH_SPT_DEV_PREMIUM_KBL_U_ID                        0x9D4E // (PCH-SPT)  Premium Consumer/Corp KBL-U

#define PCH_SPT_LPC_DEV_KBL_SUPERSKU_UNLOCKED_ID            0x9D51 // (SPT-LP)  Super SKU Unlocked
#define PCH_SPT_LPC_DEV_KBL_SUPERSKU_LOCKED_ID              0x9D52 // (SPT-LP)  Super SKU locked
#define PCH_SPT_LPC_DEV_PREMIUM_KBL_Y_ID                    0x9D56 // (SPT-LP)  Premium Consumer/Corp KBL-Y
#define PCH_SPT_LPC_DEV_PREMIUM_KBL_U_ID                    0x9D58 // (SPT-LP)  (SPT-LP)  Premium Consumer/Corp KBL-Y
#define PCH_SPT_LPC_DEV_KBL_BASE_ID                         0x9D53 // (SPT-LP)  Base Consumer/Corp KBL-U

#define PCH_SPT_LPC_DEV_P4_ID                               0x9D49 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_P5_ID                               0x9D50 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_P6_ID                               0x9D54 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_P7_ID                               0x9D55 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_P8_ID                               0x9D57 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_P9_ID                               0x9D59 // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PA_ID                               0x9D5A // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PB_ID                               0x9D5B // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PC_ID                               0x9D5C // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PD_ID                               0x9D5D // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PE_ID                               0x9D5E // (SPT-LP)  Placeholder, yet to be finalized
#define PCH_SPT_LPC_DEV_PF_ID                               0x9D5F // (SPT-LP)  Placeholder, yet to be finalized


//define SPT Halo Device ids
#define PCH_SPT_HALO_DEV_UNFUSED_ID                         0xA140 // (SPT-H)  Unfused
#define PCH_SPT_CLIENT_UNLOCKED_ID                          0xA141 // (SPT-H)  SuperSKU Client UnLocked
#define PCH_SPT_CLIENT_LOCKED_ID                            0xA142 // (SPT-H)  SuperSKU Client Locked
#define PCH_SPT_HALO_DEV_H110_ID                            0xA143 // (SPT-H)  H110 SKU
#define PCH_SPT_HALO_DEV_H170_ID                            0xA144 // (SPT-H)  H170 SKU
#define PCH_SPT_HALO_DEV_Z170_ID                            0xA145 // (SPT-H)  Z170 SKU
#define PCH_SPT_HALO_DEV_Q170_ID                            0xA146 // (SPT-H)  Q170 SKU
#define PCH_SPT_HALO_DEV_Q150_ID                            0xA147 // (SPT-H)  Q150 SKU
#define PCH_SPT_HALO_DEV_B150_ID                            0xA148 // (SPT-H)  B150 SKU
#define PCH_SPT_HALO_DEV_C236_ID                            0xA149 // (SPT-H)  C236 SKU
#define PCH_SPT_HALO_DEV_C232_ID                            0xA14A // (SPT-H)  C232 SKU
#define PCH_SPT_SERVER_UNLOCKED_ID                          0xA14B // (SPT-H)  Super SKU Server Unlocked
#define PCH_SPT_SERVER_LOCKED_ID                            0xA14C // (SPT-H)  Super SKU Server locked
#define PCH_SPT_HALO_DEV_QM170_ID                           0xA14D // (SPT-H)  QM170 SKU
#define PCH_SPT_HALO_DEV_HM170_ID                           0xA14E // (SPT-H)  HM170 SKU
#define PCH_SPT_HALO_DEV_QMS170_ID                          0xA14F // (SPT-H)  QMS170 SKU
#define PCH_SPT_HALO_DEV_CM236_ID                           0xA150 // (SPT-H)  CM236 SKU
#define PCH_SPT_HALO_DEV_QMS180_ID                          0xA151 // SPT-H QMS180 SKU
#define PCH_SPT_HALO_DEV_HM172_ID                           0xA152 // SPT-H HM172 SKU
#define PCH_SPT_HALO_DEV_QM172_ID                           0xA153 // SPT-H QM172 SKU
#define PCH_SPT_HALO_DEV_CM238_ID                           0xA154 // SPT-H CM238 SKU
#define PCH_SPT_HALO_DEV_P1_ID                              0xA155 // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P2_ID                              0xA156 // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P3_ID                              0xA157 // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P4_ID                              0xA158 // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P5_ID                              0xA159 // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P6_ID                              0xA15A // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P7_ID                              0xA15B // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P8_ID                              0xA15C // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_P9_ID                              0xA15D // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_PA_ID                              0xA15E // Placeholder, yet to be finalized
#define PCH_SPT_HALO_DEV_PB_ID                              0xA15F // Placeholder, yet to be finalized




//define KBP Halo Device Ids
#define PCH_KBP_HALO_DEV_P1_ID                             0xA280
#define PCH_KBP_HALO_DEV_P2_ID                             0xA281
#define PCH_KBP_HALO_DEV_P3_ID                             0xA282
#define PCH_KBP_HALO_DEV_P4_ID                             0xA283
#define PCH_KBP_HALO_DEV_P5_ID                             0xA284
#define PCH_KBP_HALO_DEV_P6_ID                             0xA285
#define PCH_KBP_HALO_DEV_P7_ID                             0xA286
#define PCH_KBP_HALO_DEV_P8_ID                             0xA287
#define PCH_KBP_HALO_DEV_P9_ID                             0xA288
#define PCH_KBP_HALO_DEV_PA_ID                             0xA289
#define PCH_KBP_HALO_DEV_PB_ID                             0xA28A
#define PCH_KBP_HALO_DEV_PC_ID                             0xA28B
#define PCH_KBP_HALO_DEV_PD_ID                             0xA28C
#define PCH_KBP_HALO_DEV_PE_ID                             0xA28D
#define PCH_KBP_HALO_DEV_PF_ID                             0xA28E
#define PCH_KBP_HALO_DEV_P10_ID                            0xA28F
#define PCH_KBP_HALO_DEV_P11_ID                            0xA290
#define PCH_KBP_HALO_DEV_P12_ID                            0xA291
#define PCH_KBP_HALO_DEV_P13_ID                            0xA292
#define PCH_KBP_HALO_DEV_P14_ID                            0xA293
#define PCH_KBP_HALO_DEV_P15_ID                            0xA294
#define PCH_KBP_HALO_DEV_P16_ID                            0xA295
#define PCH_KBP_HALO_DEV_P17_ID                            0xA296
#define PCH_KBP_HALO_DEV_P18_ID                            0xA297
#define PCH_KBP_HALO_DEV_P19_ID                            0xA298
#define PCH_KBP_HALO_DEV_P1A_ID                            0xA299
#define PCH_KBP_HALO_DEV_P1B_ID                            0xA29A
#define PCH_KBP_HALO_DEV_P1C_ID                            0xA29B
#define PCH_KBP_HALO_DEV_P1D_ID                            0xA29C
#define PCH_KBP_HALO_DEV_P1E_ID                            0xA29D
#define PCH_KBP_HALO_DEV_P1F_ID                            0xA29E
#define PCH_KBP_HALO_DEV_P20_ID                            0xA29F

#define PCH_KBP_HALO_ES_SUPER_SKU_CLIENT                   0xA2C0
#define PCH_KBP_HALO_ES_SUPER_SKU_SERVER                   0xA2CE
#define PCH_KBP_SUPERSKU_CLIENT_ID                         0xA2C1
#define PCH_KBP_S_DEV_P0_ID                                0xA2C2
#define PCH_KBP_S_DEV_P1_ID                                0xA2C3
#define PCH_KBP_S_H270_ID                                  0xA2C4
#define PCH_KBP_S_Z270_ID                                  0xA2C5
#define PCH_KBP_S_Q270_ID                                  0xA2C6
#define PCH_KBP_S_Q250_ID                                  0xA2C7
#define PCH_KBP_S_B250_ID                                  0xA2C8
#define PCH_KBP_S_DEV_P2_ID                                0xA2C9
#define PCH_KBP_S_DEV_P3_ID                                0xA2CA
#define PCH_KBP_S_DEV_P4_ID                                0xA2CB
#define PCH_KBP_S_DEV_P5_ID                                0xA2CC
#define PCH_KBP_S_DEV_P6_ID                                0xA2CD
#define PCH_KBP_SUPERSKU_SERVER_ID                         0xA2CF



//define CNL LP PCH Device Ids
#define PCH_CNP_LP_DEV_P1_ID                                0x9D80
#define PCH_CNP_LP_DEV_P2_ID                                0x9D81
#define PCH_CNP_LP_DEV_P3_ID                                0x9D82
#define PCH_CNP_LP_DEV_P4_ID                                0x9D83
#define PCH_CNP_LP_DEV_P5_ID                                0x9D84
#define PCH_CNP_LP_DEV_P6_ID                                0x9D85
#define PCH_CNP_LP_DEV_P7_ID                                0x9D86
#define PCH_CNP_LP_DEV_P8_ID                                0x9D87
#define PCH_CNP_LP_DEV_P9_ID                                0x9D88
#define PCH_CNP_LP_DEV_PA_ID                                0x9D89
#define PCH_CNP_LP_DEV_PB_ID                                0x9D8A
#define PCH_CNP_LP_DEV_PC_ID                                0x9D8B
#define PCH_CNP_LP_DEV_PD_ID                                0x9D8C
#define PCH_CNP_LP_DEV_PE_ID                                0x9D8D
#define PCH_CNP_LP_DEV_PF_ID                                0x9D8E
#define PCH_CNP_LP_DEV_P10_ID                               0x9D8F
#define PCH_CNP_LP_DEV_P11_ID                               0x9D90
#define PCH_CNP_LP_DEV_P12_ID                               0x9D91
#define PCH_CNP_LP_DEV_P13_ID                               0x9D92
#define PCH_CNP_LP_DEV_P14_ID                               0x9D93
#define PCH_CNP_LP_DEV_P15_ID                               0x9D94
#define PCH_CNP_LP_DEV_P16_ID                               0x9D95
#define PCH_CNP_LP_DEV_P17_ID                               0x9D96
#define PCH_CNP_LP_DEV_P18_ID                               0x9D97
#define PCH_CNP_LP_DEV_P19_ID                               0x9D98
#define PCH_CNP_LP_DEV_P1A_ID                               0x9D99
#define PCH_CNP_LP_DEV_P1B_ID                               0x9D9A
#define PCH_CNP_LP_DEV_P1C_ID                               0x9D9B
#define PCH_CNP_LP_DEV_P1D_ID                               0x9D9C
#define PCH_CNP_LP_DEV_P1E_ID                               0x9D9D
#define PCH_CNP_LP_DEV_P1F_ID                               0x9D9E
#define PCH_CNP_LP_DEV_P20_ID                               0x9D9F

//define CNL Halo PCH Device Ids
#define PCH_CNP_HALO_DEV_P1_ID                              0xA300
#define PCH_CNP_HALO_DEV_P2_ID                              0xA301
#define PCH_CNP_HALO_DEV_P3_ID                              0xA302
#define PCH_CNP_HALO_DEV_P4_ID                              0xA303
#define PCH_CNP_HALO_DEV_P5_ID                              0xA304
#define PCH_CNP_HALO_DEV_P6_ID                              0xA305
#define PCH_CNP_HALO_DEV_P7_ID                              0xA306
#define PCH_CNP_HALO_DEV_P8_ID                              0xA307
#define PCH_CNP_HALO_DEV_P9_ID                              0xA308
#define PCH_CNP_HALO_DEV_PA_ID                              0xA309
#define PCH_CNP_HALO_DEV_PB_ID                              0xA30A
#define PCH_CNP_HALO_DEV_PC_ID                              0xA30B
#define PCH_CNP_HALO_DEV_PD_ID                              0xA30C
#define PCH_CNP_HALO_DEV_PE_ID                              0xA30D
#define PCH_CNP_HALO_DEV_PF_ID                              0xA30E
#define PCH_CNP_HALO_DEV_P10_ID                             0xA30F
#define PCH_CNP_HALO_DEV_P11_ID                             0xA310
#define PCH_CNP_HALO_DEV_P12_ID                             0xA311
#define PCH_CNP_HALO_DEV_P13_ID                             0xA312
#define PCH_CNP_HALO_DEV_P14_ID                             0xA313
#define PCH_CNP_HALO_DEV_P15_ID                             0xA314
#define PCH_CNP_HALO_DEV_P16_ID                             0xA315
#define PCH_CNP_HALO_DEV_P17_ID                             0xA316
#define PCH_CNP_HALO_DEV_P18_ID                             0xA317
#define PCH_CNP_HALO_DEV_P19_ID                             0xA318
#define PCH_CNP_HALO_DEV_P1A_ID                             0xA319
#define PCH_CNP_HALO_DEV_P1B_ID                             0xA31A
#define PCH_CNP_HALO_DEV_P1C_ID                             0xA31B
#define PCH_CNP_HALO_DEV_P1D_ID                             0xA31C
#define PCH_CNP_HALO_DEV_P1E_ID                             0xA31D
#define PCH_CNP_HALO_DEV_P1F_ID                             0xA31E
#define PCH_CNP_HALO_DEV_P20_ID                             0xA31F

//GEN11LP
#define IICL_LP_GT1_MOB_DEVICE_F0_ID            0xFF05
#define IICL_LP_1x8x8_SUPERSKU_DEVICE_F0_ID     0x8A50
#define IICL_LP_1x8x8_ULX_DEVICE_F0_ID          0x8A51
#define IICL_LP_1x6x8_ULX_DEVICE_F0_ID          0x8A5C
#define IICL_LP_1x4x8_ULX_DEVICE_F0_ID          0x8A5D
#define IICL_LP_1x8x8_ULT_DEVICE_F0_ID          0x8A52
#define IICL_LP_1x6x8_ULT_DEVICE_F0_ID          0x8A5A
#define IICL_LP_1x4x8_ULT_DEVICE_F0_ID          0x8A5B
#define IICL_LP_0x0x0_ULT_DEVICE_A0_ID          0x8A70
#define IICL_LP_1x1x8_ULT_DEVICE_A0_ID          0x8A71
#define IICL_LP_1x4x8_LOW_MEDIA_ULT_DEVICE_F0_ID 0x8A56
#define IICL_LP_1x4x8_LOW_MEDIA_ULX_DEVICE_F0_ID 0x8A58

//TGL LP
#define IGEN12LP_GT1_MOB_DEVICE_F0_ID           0xFF20
#define ITGL_LP_1x6x16_UNKNOWN_SKU_F0_ID_5      0x9A49      // Remove this once newer enums are merged in OpenCL. Added this to avoid build failure with Linux/OpenCL.
#define ITGL_LP_1x6x16_ULT_15W_DEVICE_F0_ID     0x9A49      // Mobile    - U42 - 15W
#define ITGL_LP_1x6x16_ULX_5_2W_DEVICE_F0_ID    0x9A40      // Mobile    - Y42 - 5.2W
#define ITGL_LP_1x6x16_ULT_12W_DEVICE_F0_ID     0x9A59      // Mobile    - U42 - 12W
#define ITGL_LP_1x2x16_HALO_45W_DEVICE_F0_ID    0x9A60      // Halo      - H81 - 45W
#define ITGL_LP_1x2x16_DESK_65W_DEVICE_F0_ID    0x9A68      // Desktop   - S81 - 35W/65W/95W
#define ITGL_LP_1x2x16_HALO_WS_45W_DEVICE_F0_ID 0x9A70      // Mobile WS - H81 - 45W
#define ITGL_LP_1x2x16_DESK_WS_65W_DEVICE_F0_ID 0x9A78      // Desktop WS- S81 - 35W/65W/95W
#define ITGL_LP_GT0_ULT_DEVICE_F0_ID            0x9A7F      // GT0 - No GFX, Display Only

#define DEV_ID_0205                                0x0205
#define DEV_ID_020A                                0x020A
//Internal Validation Sku's Only
#define DEV_ID_0201                                0x0201
#define DEV_ID_0202                                0x0202
#define DEV_ID_0203                                0x0203
#define DEV_ID_0204                                0x0204
#define DEV_ID_0206                                0x0206
#define DEV_ID_0207                                0x0207
#define DEV_ID_0208                                0x0208
#define DEV_ID_0209                                0x0209
#define DEV_ID_020B                                0x020B
#define DEV_ID_020C                                0x020C
#define DEV_ID_020D                                0x020D
#define DEV_ID_020E                                0x020E
#define DEV_ID_020F                                0x020F
#define DEV_ID_0210                                0x0210

#define DEV_ID_FF20                             0xFF20
#define DEV_ID_9A49                             0x9A49
#define DEV_ID_9A40                             0x9A40
#define DEV_ID_9A59                             0x9A59
#define DEV_ID_9A60                             0x9A60
#define DEV_ID_9A68                             0x9A68
#define DEV_ID_9A70                             0x9A70
#define DEV_ID_9A78                             0x9A78
#define DEV_ID_9A7F                             0x9A7F

#define DEV_ID_4905                             0x4905
#define DEV_ID_4906                             0x4906
#define DEV_ID_4907                             0x4907

// Rocketlake
#define DEV_ID_4C80                             0x4C80
#define DEV_ID_4C8A                             0x4C8A
#define DEV_ID_4C8B                             0x4C8B
#define DEV_ID_4C8C                             0x4C8C
#define DEV_ID_4C90                             0x4C90
#define DEV_ID_4C9A                             0x4C9A

//LKF
#define ILKF_1x8x8_DESK_DEVICE_F0_ID            0x9840
#define ILKF_GT0_DESK_DEVICE_A0_ID              0x9850
#define ILKF_1x6x8_DESK_DEVICE_F0_ID            0x9841
#define ILKF_1x4x8_DESK_DEVICE_F0_ID            0x9842

//JSL
#define IJSL_1x4x8_DEVICE_A0_ID                 0x4500

//EHL
#define IEHL_1x4x8_SUPERSKU_DEVICE_A0_ID        0x4500
#define IEHL_1x2x4_DEVICE_A0_ID                 0x4541
#define IEHL_1x4x4_DEVICE_A0_ID                 0x4551
#define IEHL_1x4x8_DEVICE_A0_ID                 0x4571

#define DEV_ID_4500                             0x4500
#define DEV_ID_4541                             0x4541
#define DEV_ID_4551                             0x4551
#define DEV_ID_4571                             0x4571
#define DEV_ID_4555                             0x4555

//JSL+ Rev02
#define IJSL_1x4x4_DEVICE_B0_ID                 0x4E51
#define IJSL_1x4x6_DEVICE_B0_ID                 0x4E61
#define IJSL_1x4x8_DEVICE_B0_ID                 0x4E71

#define DEV_ID_4E51                             0x4E51
#define DEV_ID_4E61                             0x4E61
#define DEV_ID_4E71                             0x4E71
#define DEV_ID_4E55                             0x4E55

//ADL-S PCH Device IDs
#define DEV_ID_4680                             0x4680
#define DEV_ID_4681                             0x4681
#define DEV_ID_4682                             0x4682
#define DEV_ID_4683                             0x4683
#define DEV_ID_4690                             0x4690
#define DEV_ID_4691                             0x4691
#define DEV_ID_4692                             0x4692
#define DEV_ID_4693                             0x4693
#define DEV_ID_4698                             0x4698
#define DEV_ID_4699                             0x4699

// ADL-P
#define DEV_ID_46A0                             0x46A0
#define DEV_ID_46A1                             0x46A1
#define DEV_ID_46A2                             0x46A2
#define DEV_ID_46A3                             0x46A3
#define DEV_ID_46A6                             0x46A6
#define DEV_ID_46A8                             0x46A8
#define DEV_ID_46AA                             0x46AA
#define DEV_ID_4626                             0x4626
#define DEV_ID_4628                             0x4628
#define DEV_ID_462A                             0x462A
#define DEV_ID_46B0                             0x46B0
#define DEV_ID_46B1                             0x46B1
#define DEV_ID_46B2                             0x46B2
#define DEV_ID_46B3                             0x46B3
#define DEV_ID_46C0                             0x46C0
#define DEV_ID_46C1                             0x46C1
#define DEV_ID_46C2                             0x46C2
#define DEV_ID_46C3                             0x46C3

//ICL PCH LP Device IDs
#define ICP_LP_RESERVED_FUSE_ID                 0x3480
#define ICP_LP_U_SUPER_SKU_ID                   0x3481
#define ICP_LP_U_PREMIUM_ID                     0x3482
#define ICP_LP_U_MAINSTREAM_ID                  0x3483
#define ICL_LP_UNKNOWN_SKU_ID_1                 0x3484
#define ICL_LP_UNKNOWN_SKU_ID_2                 0x3485
#define ICP_LP_Y_SUPER_SKU_ID                   0x3486
#define ICP_LP_Y_PREMIUM_ID                     0x3487
#define ICL_LP_UNKNOWN_SKU_ID_3                 0x3488
#define ICL_LP_UNKNOWN_SKU_ID_4                 0x3489
#define ICL_LP_UNKNOWN_SKU_ID_5                 0x348A
#define ICL_LP_UNKNOWN_SKU_ID_6                 0x348B
#define ICL_LP_UNKNOWN_SKU_ID_7                 0x348C
#define ICL_LP_UNKNOWN_SKU_ID_8                 0x348D
#define ICL_LP_UNKNOWN_SKU_ID_9                 0x348E
#define ICL_LP_UNKNOWN_SKU_ID_10                0x348F
#define ICL_LP_UNKNOWN_SKU_ID_11                0x3490
#define ICL_LP_UNKNOWN_SKU_ID_12                0x3491
#define ICL_LP_UNKNOWN_SKU_ID_13                0x3492
#define ICL_LP_UNKNOWN_SKU_ID_14                0x3493
#define ICL_LP_UNKNOWN_SKU_ID_15                0x3494
#define ICL_LP_UNKNOWN_SKU_ID_16                0x3495
#define ICL_LP_UNKNOWN_SKU_ID_17                0x3496
#define ICL_LP_UNKNOWN_SKU_ID_18                0x3497
#define ICL_LP_UNKNOWN_SKU_ID_19                0x3498
#define ICL_LP_UNKNOWN_SKU_ID_20                0x3499
#define ICL_LP_UNKNOWN_SKU_ID_21                0x349A
#define ICL_LP_UNKNOWN_SKU_ID_22                0x349B
#define ICL_LP_UNKNOWN_SKU_ID_23                0x349C
#define ICL_LP_UNKNOWN_SKU_ID_24                0x349D
#define ICL_LP_UNKNOWN_SKU_ID_25                0x349E
#define ICL_LP_UNKNOWN_SKU_ID_26                0x349F

// JSL N PCH Device IDs for JSL+ Rev02
#define PCH_JSP_N_UNKNOWN_SKU_ID_1              0x4D80
#define PCH_JSP_N_UNKNOWN_SKU_ID_2              0x4D81
#define PCH_JSP_N_UNKNOWN_SKU_ID_3              0x4D82
#define PCH_JSP_N_UNKNOWN_SKU_ID_4              0x4D83
#define PCH_JSP_N_UNKNOWN_SKU_ID_5              0x4D84
#define PCH_JSP_N_UNKNOWN_SKU_ID_6              0x4D85
#define PCH_JSP_N_UNKNOWN_SKU_ID_7              0x4D86
#define PCH_JSP_N_UNKNOWN_SKU_ID_8              0x4D87
#define PCH_JSP_N_UNKNOWN_SKU_ID_9              0x4D88
#define PCH_JSP_N_UNKNOWN_SKU_ID_10             0x4D89
#define PCH_JSP_N_UNKNOWN_SKU_ID_11             0x4D8A
#define PCH_JSP_N_UNKNOWN_SKU_ID_12             0x4D8B
#define PCH_JSP_N_UNKNOWN_SKU_ID_13             0x4D8C
#define PCH_JSP_N_UNKNOWN_SKU_ID_14             0x4D8D
#define PCH_JSP_N_UNKNOWN_SKU_ID_15             0x4D8E
#define PCH_JSP_N_UNKNOWN_SKU_ID_16             0x4D8F
#define PCH_JSP_N_UNKNOWN_SKU_ID_17             0x4D90
#define PCH_JSP_N_UNKNOWN_SKU_ID_18             0x4D91
#define PCH_JSP_N_UNKNOWN_SKU_ID_19             0x4D92
#define PCH_JSP_N_UNKNOWN_SKU_ID_20             0x4D93
#define PCH_JSP_N_UNKNOWN_SKU_ID_21             0x4D94
#define PCH_JSP_N_UNKNOWN_SKU_ID_22             0x4D95
#define PCH_JSP_N_UNKNOWN_SKU_ID_23             0x4D96
#define PCH_JSP_N_UNKNOWN_SKU_ID_24             0x4D97
#define PCH_JSP_N_UNKNOWN_SKU_ID_25             0x4D98
#define PCH_JSP_N_UNKNOWN_SKU_ID_26             0x4D99
#define PCH_JSP_N_UNKNOWN_SKU_ID_27             0x4D9A
#define PCH_JSP_N_UNKNOWN_SKU_ID_28             0x4D9B
#define PCH_JSP_N_UNKNOWN_SKU_ID_29             0x4D9C
#define PCH_JSP_N_UNKNOWN_SKU_ID_30             0x4D9D
#define PCH_JSP_N_UNKNOWN_SKU_ID_31             0x4D9E
#define PCH_JSP_N_UNKNOWN_SKU_ID_32             0x4D9F

// LKF-PCH Device IDs
#define PCH_LKF_UNFUSED_SKU_ID                  0x9880
#define PCH_LKF_SUPER_SKU_ID                    0x9881
// TGL_LP PCH Device ID range 0xA080-0xA09F
#define PCH_TGL_LP_UNKNOWN_SKU_ID_1             0xA080
#define PCH_TGL_LP_UNKNOWN_SKU_ID_2             0xA081
#define PCH_TGL_LP_UNKNOWN_SKU_ID_3             0xA082
#define PCH_TGL_LP_UNKNOWN_SKU_ID_4             0xA083
#define PCH_TGL_LP_UNKNOWN_SKU_ID_5             0xA084
#define PCH_TGL_LP_UNKNOWN_SKU_ID_6             0xA085
#define PCH_TGL_LP_UNKNOWN_SKU_ID_7             0xA086
#define PCH_TGL_LP_UNKNOWN_SKU_ID_8             0xA087
#define PCH_TGL_LP_UNKNOWN_SKU_ID_9             0xA088
#define PCH_TGL_LP_UNKNOWN_SKU_ID_10            0xA089
#define PCH_TGL_LP_UNKNOWN_SKU_ID_11            0xA08A
#define PCH_TGL_LP_UNKNOWN_SKU_ID_12            0xA08B
#define PCH_TGL_LP_UNKNOWN_SKU_ID_13            0xA08C
#define PCH_TGL_LP_UNKNOWN_SKU_ID_14            0xA08D
#define PCH_TGL_LP_UNKNOWN_SKU_ID_15            0xA08E
#define PCH_TGL_LP_UNKNOWN_SKU_ID_16            0xA08F
#define PCH_TGL_LP_UNKNOWN_SKU_ID_17            0xA090
#define PCH_TGL_LP_UNKNOWN_SKU_ID_18            0xA091
#define PCH_TGL_LP_UNKNOWN_SKU_ID_19            0xA092
#define PCH_TGL_LP_UNKNOWN_SKU_ID_20            0xA093
#define PCH_TGL_LP_UNKNOWN_SKU_ID_21            0xA094
#define PCH_TGL_LP_UNKNOWN_SKU_ID_22            0xA095
#define PCH_TGL_LP_UNKNOWN_SKU_ID_23            0xA096
#define PCH_TGL_LP_UNKNOWN_SKU_ID_24            0xA097
#define PCH_TGL_LP_UNKNOWN_SKU_ID_25            0xA098
#define PCH_TGL_LP_UNKNOWN_SKU_ID_26            0xA099
#define PCH_TGL_LP_UNKNOWN_SKU_ID_27            0xA09A
#define PCH_TGL_LP_UNKNOWN_SKU_ID_28            0xA09B
#define PCH_TGL_LP_UNKNOWN_SKU_ID_29            0xA09C
#define PCH_TGL_LP_UNKNOWN_SKU_ID_30            0xA09D
#define PCH_TGL_LP_UNKNOWN_SKU_ID_31            0xA09E
#define PCH_TGL_LP_UNKNOWN_SKU_ID_32            0xA09F

//define CML LP PCH Device Ids
#define PCH_CMP_LP_DEV_P1_ID                    0x0280
#define PCH_CMP_LP_DEV_P2_ID                    0x0281
#define PCH_CMP_LP_DEV_P3_ID                    0x0282
#define PCH_CMP_LP_DEV_P4_ID                    0x0283
#define PCH_CMP_LP_DEV_P5_ID                    0x0284
#define PCH_CMP_LP_DEV_P6_ID                    0x0285
#define PCH_CMP_LP_DEV_P7_ID                    0x0286
#define PCH_CMP_LP_DEV_P8_ID                    0x0287
#define PCH_CMP_LP_DEV_P9_ID                    0x0288
#define PCH_CMP_LP_DEV_P10_ID                   0x0289
#define PCH_CMP_LP_DEV_P11_ID                   0x028A
#define PCH_CMP_LP_DEV_P12_ID                   0x028B
#define PCH_CMP_LP_DEV_P13_ID                   0x028C
#define PCH_CMP_LP_DEV_P14_ID                   0x028D
#define PCH_CMP_LP_DEV_P15_ID                   0x028E
#define PCH_CMP_LP_DEV_P16_ID                   0x028F
#define PCH_CMP_LP_DEV_P17_ID                   0x0290
#define PCH_CMP_LP_DEV_P18_ID                   0x0291
#define PCH_CMP_LP_DEV_P19_ID                   0x0292
#define PCH_CMP_LP_DEV_P20_ID                   0x0293
#define PCH_CMP_LP_DEV_P21_ID                   0x0294
#define PCH_CMP_LP_DEV_P22_ID                   0x0295
#define PCH_CMP_LP_DEV_P23_ID                   0x0296
#define PCH_CMP_LP_DEV_P24_ID                   0x0297
#define PCH_CMP_LP_DEV_P25_ID                   0x0298
#define PCH_CMP_LP_DEV_P26_ID                   0x0299
#define PCH_CMP_LP_DEV_P27_ID                   0x029A
#define PCH_CMP_LP_DEV_P28_ID                   0x029B
#define PCH_CMP_LP_DEV_P29_ID                   0x029C
#define PCH_CMP_LP_DEV_P30_ID                   0x029D
#define PCH_CMP_LP_DEV_P31_ID                   0x029E
#define PCH_CMP_LP_DEV_P32_ID                   0x029F

// TGL_H PCH Device ID range 0x4380-0x439F
#define PCH_TGL_H_UNKNOWN_SKU_ID_1              0x4380
#define PCH_TGL_H_UNKNOWN_SKU_ID_2              0x4381
#define PCH_TGL_H_UNKNOWN_SKU_ID_3              0x4382
#define PCH_TGL_H_UNKNOWN_SKU_ID_4              0x4383
#define PCH_TGL_H_UNKNOWN_SKU_ID_5              0x4384
#define PCH_TGL_H_UNKNOWN_SKU_ID_6              0x4385
#define PCH_TGL_H_UNKNOWN_SKU_ID_7              0x4386
#define PCH_TGL_H_UNKNOWN_SKU_ID_8              0x4387
#define PCH_TGL_H_UNKNOWN_SKU_ID_9              0x4388
#define PCH_TGL_H_UNKNOWN_SKU_ID_10             0x4389
#define PCH_TGL_H_UNKNOWN_SKU_ID_11             0x438A
#define PCH_TGL_H_UNKNOWN_SKU_ID_12             0x438B
#define PCH_TGL_H_UNKNOWN_SKU_ID_13             0x438C
#define PCH_TGL_H_UNKNOWN_SKU_ID_14             0x438D
#define PCH_TGL_H_UNKNOWN_SKU_ID_15             0x438E
#define PCH_TGL_H_UNKNOWN_SKU_ID_16             0x438F
#define PCH_TGL_H_UNKNOWN_SKU_ID_17             0x4390
#define PCH_TGL_H_UNKNOWN_SKU_ID_18             0x4391
#define PCH_TGL_H_UNKNOWN_SKU_ID_19             0x4392
#define PCH_TGL_H_UNKNOWN_SKU_ID_20             0x4393
#define PCH_TGL_H_UNKNOWN_SKU_ID_21             0x4394
#define PCH_TGL_H_UNKNOWN_SKU_ID_22             0x4395
#define PCH_TGL_H_UNKNOWN_SKU_ID_23             0x4396
#define PCH_TGL_H_UNKNOWN_SKU_ID_24             0x4397
#define PCH_TGL_H_UNKNOWN_SKU_ID_25             0x4398
#define PCH_TGL_H_UNKNOWN_SKU_ID_26             0x4399
#define PCH_TGL_H_UNKNOWN_SKU_ID_27             0x439A
#define PCH_TGL_H_UNKNOWN_SKU_ID_28             0x439B
#define PCH_TGL_H_UNKNOWN_SKU_ID_29             0x439C
#define PCH_TGL_H_UNKNOWN_SKU_ID_30             0x439D
#define PCH_TGL_H_UNKNOWN_SKU_ID_31             0x439E
#define PCH_TGL_H_UNKNOWN_SKU_ID_32             0x439F

//define CML H PCH Device Ids
#define PCH_CMP_H_DEV_P1_ID                     0x0680
#define PCH_CMP_H_DEV_P2_ID                     0x0681
#define PCH_CMP_H_DEV_P3_ID                     0x0682
#define PCH_CMP_H_DEV_P4_ID                     0x0683
#define PCH_CMP_H_DEV_P5_ID                     0x0684
#define PCH_CMP_H_DEV_P6_ID                     0x0685
#define PCH_CMP_H_DEV_P7_ID                     0x0686
#define PCH_CMP_H_DEV_P8_ID                     0x0687
#define PCH_CMP_H_DEV_P9_ID                     0x0688
#define PCH_CMP_H_DEV_P10_ID                    0x0689
#define PCH_CMP_H_DEV_P11_ID                    0x068A
#define PCH_CMP_H_DEV_P12_ID                    0x068B
#define PCH_CMP_H_DEV_P13_ID                    0x068C
#define PCH_CMP_H_DEV_P14_ID                    0x068D
#define PCH_CMP_H_DEV_P15_ID                    0x068E
#define PCH_CMP_H_DEV_P16_ID                    0x068F
#define PCH_CMP_H_DEV_P17_ID                    0x0690
#define PCH_CMP_H_DEV_P18_ID                    0x0691
#define PCH_CMP_H_DEV_P19_ID                    0x0692
#define PCH_CMP_H_DEV_P20_ID                    0x0693
#define PCH_CMP_H_DEV_P21_ID                    0x0694
#define PCH_CMP_H_DEV_P22_ID                    0x0695
#define PCH_CMP_H_DEV_P23_ID                    0x0696
#define PCH_CMP_H_DEV_P24_ID                    0x0697
#define PCH_CMP_H_DEV_P25_ID                    0x0698
#define PCH_CMP_H_DEV_P26_ID                    0x0699
#define PCH_CMP_H_DEV_P27_ID                    0x069A
#define PCH_CMP_H_DEV_P28_ID                    0x069B
#define PCH_CMP_H_DEV_P29_ID                    0x069C
#define PCH_CMP_H_DEV_P30_ID                    0x069D
#define PCH_CMP_H_DEV_P31_ID                    0x069E
#define PCH_CMP_H_DEV_P32_ID                    0x069F

//define CML V PCH Device Ids
#define PCH_CMP_V_DEV_P65_ID                   0xA3C0
#define PCH_CMP_V_DEV_P66_ID                   0xA3C1
#define PCH_CMP_V_DEV_P67_ID                   0xA3C2
#define PCH_CMP_V_DEV_P68_ID                   0xA3C3
#define PCH_CMP_V_DEV_P69_ID                   0xA3C4
#define PCH_CMP_V_DEV_P70_ID                   0xA3C5
#define PCH_CMP_V_DEV_P71_ID                   0xA3C6
#define PCH_CMP_V_DEV_P72_ID                   0xA3C7
#define PCH_CMP_V_DEV_P73_ID                   0xA3C8
#define PCH_CMP_V_DEV_P74_ID                   0xA3C9
#define PCH_CMP_V_DEV_P75_ID                   0xA3CA
#define PCH_CMP_V_DEV_P76_ID                   0xA3CB
#define PCH_CMP_V_DEV_P77_ID                   0xA3CC
#define PCH_CMP_V_DEV_P78_ID                   0xA3CD
#define PCH_CMP_V_DEV_P79_ID                   0xA3CE
#define PCH_CMP_V_DEV_P80_ID                   0xA3CF
#define PCH_CMP_V_DEV_P81_ID                   0xA3D0
#define PCH_CMP_V_DEV_P82_ID                   0xA3D1
#define PCH_CMP_V_DEV_P83_ID                   0xA3D2
#define PCH_CMP_V_DEV_P84_ID                   0xA3D3
#define PCH_CMP_V_DEV_P85_ID                   0xA3D4
#define PCH_CMP_V_DEV_P86_ID                   0xA3D5
#define PCH_CMP_V_DEV_P87_ID                   0xA3D6
#define PCH_CMP_V_DEV_P88_ID                   0xA3D7
#define PCH_CMP_V_DEV_P89_ID                   0xA3D8
#define PCH_CMP_V_DEV_P90_ID                   0xA3D9
#define PCH_CMP_V_DEV_P91_ID                   0xA3DA
#define PCH_CMP_V_DEV_P92_ID                   0xA3DB
#define PCH_CMP_V_DEV_P93_ID                   0xA3DC
#define PCH_CMP_V_DEV_P94_ID                   0xA3DD
#define PCH_CMP_V_DEV_P95_ID                   0xA3DE
#define PCH_CMP_V_DEV_P96_ID                   0xA3DF

// ADL_S PCH Device ID range
#define DEV_ID_7A80                            0x7A80
#define DEV_ID_7A81                            0x7A81
#define DEV_ID_7A82                            0x7A82
#define DEV_ID_7A83                            0x7A83
#define DEV_ID_7A84                            0x7A84
#define DEV_ID_7A85                            0x7A85
#define DEV_ID_7A86                            0x7A86
#define DEV_ID_7A87                            0x7A87
#define DEV_ID_7A88                            0x7A88
#define DEV_ID_7A89                            0x7A89
#define DEV_ID_7A8A                            0x7A8A
#define DEV_ID_7A8B                            0x7A8B
#define DEV_ID_7A8C                            0x7A8C
#define DEV_ID_7A8D                            0x7A8D
#define DEV_ID_7A8E                            0x7A8E
#define DEV_ID_7A8F                            0x7A8F
#define DEV_ID_7A90                            0x7A90
#define DEV_ID_7A91                            0x7A91
#define DEV_ID_7A92                            0x7A92
#define DEV_ID_7A93                            0x7A93
#define DEV_ID_7A94                            0x7A94
#define DEV_ID_7A95                            0x7A95
#define DEV_ID_7A96                            0x7A96
#define DEV_ID_7A97                            0x7A97
#define DEV_ID_7A98                            0x7A98
#define DEV_ID_7A99                            0x7A99
#define DEV_ID_7A9A                            0x7A9A
#define DEV_ID_7A9B                            0x7A9B
#define DEV_ID_7A9C                            0x7A9C
#define DEV_ID_7A9D                            0x7A9D
#define DEV_ID_7A9E                            0x7A9E
#define DEV_ID_7A9F                            0x7A9F

// ADL_P PCH Device ID range
#define PCH_DEV_ID_5180         0x5180
#define PCH_DEV_ID_5181         0x5181
#define PCH_DEV_ID_5182         0x5182
#define PCH_DEV_ID_5183         0x5183
#define PCH_DEV_ID_5184         0x5184
#define PCH_DEV_ID_5185         0x5185
#define PCH_DEV_ID_5186         0x5186
#define PCH_DEV_ID_5187         0x5187
#define PCH_DEV_ID_5188         0x5188
#define PCH_DEV_ID_5189         0x5189
#define PCH_DEV_ID_518A         0x518A
#define PCH_DEV_ID_518B         0x518B
#define PCH_DEV_ID_518C         0x518C
#define PCH_DEV_ID_518D         0x518D
#define PCH_DEV_ID_518E         0x518E
#define PCH_DEV_ID_518F         0x518F
#define PCH_DEV_ID_5190         0x5190
#define PCH_DEV_ID_5191         0x5191
#define PCH_DEV_ID_5192         0x5192
#define PCH_DEV_ID_5193         0x5193
#define PCH_DEV_ID_5194         0x5194
#define PCH_DEV_ID_5195         0x5195
#define PCH_DEV_ID_5196         0x5196
#define PCH_DEV_ID_5197         0x5197
#define PCH_DEV_ID_5198         0x5198
#define PCH_DEV_ID_5199         0x5199
#define PCH_DEV_ID_519A         0x519A
#define PCH_DEV_ID_519B         0x519B
#define PCH_DEV_ID_519C         0x519C
#define PCH_DEV_ID_519D         0x519D
#define PCH_DEV_ID_519E         0x519E
#define PCH_DEV_ID_519F         0x519F

// ADL_N PCH Device ID range
#define PCH_DEV_ID_5480         0x5480
#define PCH_DEV_ID_5481         0x5481
#define PCH_DEV_ID_5482         0x5482
#define PCH_DEV_ID_5483         0x5483
#define PCH_DEV_ID_5484         0x5484
#define PCH_DEV_ID_5485         0x5485
#define PCH_DEV_ID_5486         0x5486
#define PCH_DEV_ID_5487         0x5487
#define PCH_DEV_ID_5488         0x5488
#define PCH_DEV_ID_5489         0x5489
#define PCH_DEV_ID_548A         0x548A
#define PCH_DEV_ID_548B         0x548B
#define PCH_DEV_ID_548C         0x548C
#define PCH_DEV_ID_548D         0x548D
#define PCH_DEV_ID_548E         0x548E
#define PCH_DEV_ID_548F         0x548F
#define PCH_DEV_ID_5490         0x5490
#define PCH_DEV_ID_5491         0x5491
#define PCH_DEV_ID_5492         0x5492
#define PCH_DEV_ID_5493         0x5493
#define PCH_DEV_ID_5494         0x5494
#define PCH_DEV_ID_5495         0x5495
#define PCH_DEV_ID_5496         0x5496
#define PCH_DEV_ID_5497         0x5497
#define PCH_DEV_ID_5498         0x5498
#define PCH_DEV_ID_5499         0x5499
#define PCH_DEV_ID_549A         0x549A
#define PCH_DEV_ID_549B         0x549B
#define PCH_DEV_ID_549C         0x549C
#define PCH_DEV_ID_549D         0x549D
#define PCH_DEV_ID_549E         0x549E
#define PCH_DEV_ID_549F         0x549F
#define PCH_DEV_ID_54A0         0x54A0
#define PCH_DEV_ID_54A1         0x54A1
#define PCH_DEV_ID_54A2         0x54A2
#define PCH_DEV_ID_54A3         0x54A3
#define PCH_DEV_ID_54A4         0x54A4
#define PCH_DEV_ID_15FB         0x15FB
#define PCH_DEV_ID_15FC         0x15FC
#define PCH_DEV_ID_54A6         0x54A6
#define PCH_DEV_ID_54A7         0x54A7
#define PCH_DEV_ID_54A8         0x54A8
#define PCH_DEV_ID_54A9         0x54A9
#define PCH_DEV_ID_54AA         0x54AA
#define PCH_DEV_ID_54AB         0x54AB
#define PCH_DEV_ID_54AC         0x54AC
#define PCH_DEV_ID_54AD         0x54AD
#define PCH_DEV_ID_54AE         0x54AE
#define PCH_DEV_ID_54AF         0x54AF
#define PCH_DEV_ID_54B0         0x54B0
#define PCH_DEV_ID_54B1         0x54B1
#define PCH_DEV_ID_54B2         0x54B2
#define PCH_DEV_ID_54B3         0x54B3
#define PCH_DEV_ID_54B4         0x54B4
#define PCH_DEV_ID_54B5         0x54B5
#define PCH_DEV_ID_54B6         0x54B6
#define PCH_DEV_ID_54B7         0x54B7
#define PCH_DEV_ID_54B8         0x54B8
#define PCH_DEV_ID_54B9         0x54B9
#define PCH_DEV_ID_54BA         0x54BA
#define PCH_DEV_ID_54BB         0x54BB
#define PCH_DEV_ID_54BC         0x54BC
#define PCH_DEV_ID_54BD         0x54BD
#define PCH_DEV_ID_54BE         0x54BE
#define PCH_DEV_ID_54BF         0x54BF
#define PCH_DEV_ID_54C4         0x54C4
#define PCH_DEV_ID_54C5         0x54C5
#define PCH_DEV_ID_54C6         0x54C6
#define PCH_DEV_ID_54C7         0x54C7
#define PCH_DEV_ID_54C8         0x54C8
#define PCH_DEV_ID_54C9         0x54C9
#define PCH_DEV_ID_54CA         0x54CA
#define PCH_DEV_ID_54CB         0x54CB
#define PCH_DEV_ID_54CC         0x54CC
#define PCH_DEV_ID_54CD         0x54CD
#define PCH_DEV_ID_54CE         0x54CE
#define PCH_DEV_ID_54CF         0x54CF
#define PCH_DEV_ID_54D0         0x54D0
#define PCH_DEV_ID_54D1         0x54D1
#define PCH_DEV_ID_54D2         0x54D2
#define PCH_DEV_ID_54D3         0x54D3
#define PCH_DEV_ID_54D4         0x54D4
#define PCH_DEV_ID_54D6         0x54D6
#define PCH_DEV_ID_54D7         0x54D7
#define PCH_DEV_ID_282A         0x282A
#define PCH_DEV_ID_54D8         0x54D8
#define PCH_DEV_ID_54D9         0x54D9
#define PCH_DEV_ID_54DA         0x54DA
#define PCH_DEV_ID_54DB         0x54DB
#define PCH_DEV_ID_54DC         0x54DC
#define PCH_DEV_ID_54DD         0x54DD
#define PCH_DEV_ID_54DE         0x54DE
#define PCH_DEV_ID_54DF         0x54DF
#define PCH_DEV_ID_54E0         0x54E0
#define PCH_DEV_ID_54E1         0x54E1
#define PCH_DEV_ID_54E2         0x54E2
#define PCH_DEV_ID_54E3         0x54E3
#define PCH_DEV_ID_54E4         0x54E4
#define PCH_DEV_ID_54E5         0x54E5
#define PCH_DEV_ID_54E6         0x54E6
#define PCH_DEV_ID_54E7         0x54E7
#define PCH_DEV_ID_54E8         0x54E8
#define PCH_DEV_ID_54E9         0x54E9
#define PCH_DEV_ID_54EA         0x54EA
#define PCH_DEV_ID_54EB         0x54EB
#define PCH_DEV_ID_54ED         0x54ED
#define PCH_DEV_ID_54EE         0x54EE
#define PCH_DEV_ID_54EF         0x54EF
#define PCH_DEV_ID_54F0         0x54F0
#define PCH_DEV_ID_54F1         0x54F1
#define PCH_DEV_ID_54F2         0x54F2
#define PCH_DEV_ID_54F3         0x54F3
#define PCH_DEV_ID_54F4         0x54F4
#define PCH_DEV_ID_54F5         0x54F5
#define PCH_DEV_ID_54F6         0x54F6
#define PCH_DEV_ID_54F7         0x54F7
#define PCH_DEV_ID_54F9         0x54F9
#define PCH_DEV_ID_54FA         0x54FA
#define PCH_DEV_ID_54FB         0x54FB
#define PCH_DEV_ID_54FC         0x54FC
#define PCH_DEV_ID_54FD         0x54FD
#define PCH_DEV_ID_54FE         0x54FE
#define PCH_DEV_ID_54FF         0x54FF

// MTL PCH Dev IDs
#define PCH_DEV_ID_7E00         0x7E00
#define PCH_DEV_ID_7E01         0x7E01
#define PCH_DEV_ID_7E02         0x7E02
#define PCH_DEV_ID_7E03         0x7E03
#define PCH_DEV_ID_7E04         0x7E04
#define PCH_DEV_ID_7E05         0x7E05
#define PCH_DEV_ID_7E06         0x7E06
#define PCH_DEV_ID_7E07         0x7E07
#define PCH_DEV_ID_7E08         0x7E08
#define PCH_DEV_ID_7E09         0x7E09
#define PCH_DEV_ID_7E0A         0x7E0A
#define PCH_DEV_ID_7E0B         0x7E0B
#define PCH_DEV_ID_7E0C         0x7E0C
#define PCH_DEV_ID_7E0D         0x7E0D
#define PCH_DEV_ID_7E0E         0x7E0E
#define PCH_DEV_ID_7E0F         0x7E0F
#define PCH_DEV_ID_7E10         0x7E10
#define PCH_DEV_ID_7E11         0x7E11
#define PCH_DEV_ID_7E12         0x7E12
#define PCH_DEV_ID_7E13         0x7E13
#define PCH_DEV_ID_7E14         0x7E14
#define PCH_DEV_ID_7E15         0x7E15
#define PCH_DEV_ID_7E16         0x7E16
#define PCH_DEV_ID_7E17         0x7E17
#define PCH_DEV_ID_7E18         0x7E18
#define PCH_DEV_ID_7E19         0x7E19
#define PCH_DEV_ID_7E1A         0x7E1A
#define PCH_DEV_ID_7E1B         0x7E1B
#define PCH_DEV_ID_7E1C         0x7E1C
#define PCH_DEV_ID_7E1D         0x7E1D
#define PCH_DEV_ID_7E1E         0x7E1E
#define PCH_DEV_ID_7E1F         0x7E1F

#define PCH_DEV_ID_AE00         0xAE00
#define PCH_DEV_ID_AE01         0xAE01
#define PCH_DEV_ID_AE02         0xAE02
#define PCH_DEV_ID_AE03         0xAE03
#define PCH_DEV_ID_AE04         0xAE04
#define PCH_DEV_ID_AE05         0xAE05
#define PCH_DEV_ID_AE06         0xAE06
#define PCH_DEV_ID_AE07         0xAE07
#define PCH_DEV_ID_AE08         0xAE08
#define PCH_DEV_ID_AE09         0xAE09
#define PCH_DEV_ID_AE0A         0xAE0A
#define PCH_DEV_ID_AE0B         0xAE0B
#define PCH_DEV_ID_AE0C         0xAE0C
#define PCH_DEV_ID_AE0D         0xAE0D
#define PCH_DEV_ID_AE0E         0xAE0E
#define PCH_DEV_ID_AE0F         0xAE0F
#define PCH_DEV_ID_AE10         0xAE10
#define PCH_DEV_ID_AE11         0xAE11
#define PCH_DEV_ID_AE12         0xAE12
#define PCH_DEV_ID_AE13         0xAE13
#define PCH_DEV_ID_AE14         0xAE14
#define PCH_DEV_ID_AE15         0xAE15
#define PCH_DEV_ID_AE16         0xAE16
#define PCH_DEV_ID_AE17         0xAE17
#define PCH_DEV_ID_AE18         0xAE18
#define PCH_DEV_ID_AE19         0xAE19
#define PCH_DEV_ID_AE1A         0xAE1A
#define PCH_DEV_ID_AE1B         0xAE1B
#define PCH_DEV_ID_AE1C         0xAE1C
#define PCH_DEV_ID_AE1D         0xAE1D
#define PCH_DEV_ID_AE1E         0xAE1E
#define PCH_DEV_ID_AE1F         0xAE1F

// ARL PCH Dev IDs
#define PCH_DEV_ID_7700         0x7700
#define PCH_DEV_ID_7701         0x7701
#define PCH_DEV_ID_7702         0x7702
#define PCH_DEV_ID_7703         0x7703
#define PCH_DEV_ID_7704         0x7704
#define PCH_DEV_ID_7705         0x7705
#define PCH_DEV_ID_7706         0x7706
#define PCH_DEV_ID_7707         0x7707
#define PCH_DEV_ID_7708         0x7708
#define PCH_DEV_ID_7709         0x7709
#define PCH_DEV_ID_770A         0x770A
#define PCH_DEV_ID_770B         0x770B
#define PCH_DEV_ID_770C         0x770C
#define PCH_DEV_ID_770D         0x770D
#define PCH_DEV_ID_770E         0x770E
#define PCH_DEV_ID_770F         0x770F
#define PCH_DEV_ID_7710         0x7710
#define PCH_DEV_ID_7711         0x7711
#define PCH_DEV_ID_7712         0x7712
#define PCH_DEV_ID_7713         0x7713
#define PCH_DEV_ID_7714         0x7714
#define PCH_DEV_ID_7715         0x7715
#define PCH_DEV_ID_7716         0x7716
#define PCH_DEV_ID_7717         0x7717
#define PCH_DEV_ID_7718         0x7718
#define PCH_DEV_ID_7719         0x7719
#define PCH_DEV_ID_771A         0x771A
#define PCH_DEV_ID_771B         0x771B
#define PCH_DEV_ID_771C         0x771C
#define PCH_DEV_ID_771D         0x771D
#define PCH_DEV_ID_771E         0x771E
#define PCH_DEV_ID_771F         0x771F

//PVC Device ID
#define DEV_ID_0BD0                            0x0BD0
#define DEV_ID_0BD5                            0x0BD5
#define DEV_ID_0BD6                            0x0BD6
#define DEV_ID_0BD7                            0x0BD7
#define DEV_ID_0BD8                            0x0BD8
#define DEV_ID_0BD9                            0x0BD9
#define DEV_ID_0BDA                            0x0BDA
#define DEV_ID_0BDB                            0x0BDB
#define DEV_ID_0B69                            0x0B69
#define DEV_ID_0B6E                            0x0B6E
#define DEV_ID_0BD4                            0x0BD4

// Macro to identify PVC device ID
#define GFX_IS_XT_CONFIG(d) ((d == DEV_ID_0BD5)             ||  \
                             (d == DEV_ID_0BD6)             ||  \
                             (d == DEV_ID_0BD7)             ||  \
                             (d == DEV_ID_0BD8)             ||  \
                             (d == DEV_ID_0BD9)             ||  \
                             (d == DEV_ID_0BDA)             ||  \
                             (d == DEV_ID_0BDB)		    ||  \
                             (d == DEV_ID_0B69)             ||  \
			     (d == DEV_ID_0B6E)             ||  \
			     (d == DEV_ID_0BD4))

//DG2 Device IDs
#define DEV_ID_4F80                             0x4F80
#define DEV_ID_4F81                             0x4F81
#define DEV_ID_4F82                             0x4F82
#define DEV_ID_4F83                             0x4F83
#define DEV_ID_4F84                             0x4F84
#define DEV_ID_4F85                             0x4F85
#define DEV_ID_4F86                             0x4F86
#define DEV_ID_4F87                             0x4F87
#define DEV_ID_4F88                             0x4F88
#define DEV_ID_5690                             0x5690
#define DEV_ID_5691                             0x5691
#define DEV_ID_5692                             0x5692
#define DEV_ID_5693                             0x5693
#define DEV_ID_5694                             0x5694
#define DEV_ID_5695                             0x5695
#define DEV_ID_5696                             0x5696
#define DEV_ID_5697                             0x5697
#define DEV_ID_5698                             0x5698
#define DEV_ID_56A0                             0x56A0
#define DEV_ID_56A1                             0x56A1
#define DEV_ID_56A2                             0x56A2
#define DEV_ID_56A3                             0x56A3
#define DEV_ID_56A4                             0x56A4
#define DEV_ID_56A5                             0x56A5
#define DEV_ID_56A6                             0x56A6
#define DEV_ID_56AF                             0x56AF
#define DEV_ID_56B0                             0x56B0
#define DEV_ID_56B1                             0x56B1
#define DEV_ID_56B2                             0x56B2
#define DEV_ID_56B3                             0x56B3
#define DEV_ID_56BA                             0x56BA
#define DEV_ID_56BB                             0x56BB
#define DEV_ID_56BC                             0x56BC
#define DEV_ID_56BD                             0x56BD
#define DEV_ID_56BE                             0x56BE
#define DEV_ID_56BF                             0x56BF
#define DEV_ID_56C0                             0x56C0
#define DEV_ID_56C1                             0x56C1
#define DEV_ID_56C2                             0x56C2

// RPL-P/U
#define DEV_ID_A7A0                             0xA7A0
#define DEV_ID_A7A1                             0xA7A1
#define DEV_ID_A7A8                             0xA7A8
#define DEV_ID_A7A9                             0xA7A9
#define DEV_ID_A720                             0xA720
#define DEV_ID_A721                             0xA721
#define DEV_ID_A7AA                             0xA7AA
#define DEV_ID_A7AB                             0xA7AB
#define DEV_ID_A7AC                             0xA7AC
#define DEV_ID_A7AD                             0xA7AD

// ADL-N
#define DEV_ID_46D0                             0x46D0
#define DEV_ID_46D1                             0x46D1
#define DEV_ID_46D2                             0x46D2
#define DEV_ID_46D3                             0x46D3
#define DEV_ID_46D4                             0x46D4

// MTL
#define DEV_ID_7D40                             0x7D40
#define DEV_ID_7D45                             0x7D45
#define DEV_ID_7D55                             0x7D55
#define DEV_ID_7D57                             0x7D57
#define DEV_ID_7D60                             0x7D60
#define DEV_ID_7DD5                             0x7DD5
#define DEV_ID_7DD7                             0x7DD7

// ARL-S
#define DEV_ID_7D67                             0x7D67

// ARL-H
#define DEV_ID_7D41                             0x7D41
#define DEV_ID_7D51                             0x7D51
#define DEV_ID_7DD1                             0x7DD1

// LNL
#define DEV_ID_64A0                             0x64A0
#define DEV_ID_6420                             0x6420
#define DEV_ID_64B0                             0x64B0

//BMG
#define DEV_ID_E202                             0xE202
#define DEV_ID_E20B                             0xE20B
#define DEV_ID_E20C                             0xE20C
#define DEV_ID_E20D                             0xE20D
#define DEV_ID_E212                             0xE212

// PTL
#define DEV_ID_B080                             0xB080
#define DEV_ID_B081                             0xB081
#define DEV_ID_B082                             0xB082
#define DEV_ID_B083                             0xB083
#define DEV_ID_B08F                             0xB08F
#define DEV_ID_B090                             0xB090
#define DEV_ID_B0A0                             0xB0A0
#define DEV_ID_B0B0                             0xB0B0

#define MGM_HAS     0

//#define SDG_HAS      1              //Reserve place for Springdale-G HAS
//#define SDG_SUPPORT    1              //Springdale G build switch

// Macro to identify DG2 device IDs
#define GFX_IS_DG2_G11_CONFIG(d) ( ( d == DEV_ID_56A5 )               ||   \
                                 ( d == DEV_ID_56A6 )             ||   \
                                 ( d == DEV_ID_5693 )             ||   \
                                 ( d == DEV_ID_5694 )             ||   \
                                 ( d == DEV_ID_5695 )             ||   \
                                 ( d == DEV_ID_56B0 )             ||   \
                                 ( d == DEV_ID_56B1 )             ||   \
                                 ( d == DEV_ID_56BA )             ||   \
                                 ( d == DEV_ID_56BB )             ||   \
                                 ( d == DEV_ID_56BC )             ||   \
                                 ( d == DEV_ID_56BD )             ||   \
				 ( d == DEV_ID_56C1 )             ||   \
                                 ( d == DEV_ID_4F87 )             ||   \
                                 ( d == DEV_ID_4F88 ))

#define GFX_IS_DG2_G10_CONFIG(d) ( ( d == DEV_ID_56A0 )                              ||   \
                                      ( d == DEV_ID_56A1 )                              ||   \
                                      ( d == DEV_ID_56A2 )                              ||   \
                                      ( d == DEV_ID_5690 )                              ||   \
                                      ( d == DEV_ID_5691 )                              ||   \
                                      ( d == DEV_ID_5692 )                              ||   \
                                      ( d == DEV_ID_56BE )                              ||   \
                                      ( d == DEV_ID_56BF )                              ||   \
				      ( d == DEV_ID_56C0 )                              ||   \
	                              ( d == DEV_ID_56C2 )                              ||   \
                                      ( d == DEV_ID_4F80 )                              ||   \
                                      ( d == DEV_ID_4F81 )                              ||   \
                                      ( d == DEV_ID_4F82 )                              ||   \
                                      ( d == DEV_ID_4F83 )                              ||   \
                                      ( d == DEV_ID_4F84 )                              ||   \
                                      ( d == DEV_ID_56AF ))

#define GFX_IS_DG2_G12_CONFIG(d)   ( ( d == DEV_ID_4F85 )                              ||   \
                                      ( d == DEV_ID_4F86 )                              ||   \
                                      ( d == DEV_ID_56A3 )                              ||   \
                                      ( d == DEV_ID_56A4 )                              ||   \
                                      ( d == DEV_ID_5696 )                              ||   \
                                      ( d == DEV_ID_5697 )                              ||   \
                                      ( d == DEV_ID_56B2 )                              ||   \
                                      ( d == DEV_ID_56B3 ))

// Macro to identify ARL-S Device ID
#define GFX_IS_ARL_S(d)  ( ( d == DEV_ID_7D67 ) )

// Macro to identify ARL-H Device ID
#define GFX_IS_ARL_H(d)  ( ( d == DEV_ID_7D41 )  ||   \
                         ( d == DEV_ID_7D51 )    ||   \
                         ( d == DEV_ID_7DD1 ))

//we define the highest cap and lower cap of stepping IDs
#define SI_REV_ID(lo,hi) (lo | hi<<16)

#define SI_REV_LO(SteppingID) (SteppingID & 0xFFFF)

#define SI_WA_FROM(ulRevID, STEPPING) (ulRevID >= (int)SI_REV_LO(STEPPING))

//define DG2 Media Rev ID
#ifdef DG2_MEDIA_REV_ID_B0
#undef DG2_MEDIA_REV_ID_B0
#endif
#define DG2_MEDIA_REV_ID_B0   SI_REV_ID(4,4)

#ifdef ACM_G10_MEDIA_REV_ID_B0
#undef ACM_G10_MEDIA_REV_ID_B0
#endif
#define ACM_G10_MEDIA_REV_ID_B0   SI_REV_ID(4,4)

#endif
