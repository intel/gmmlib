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

//===========================================================================
// typedef:
//      GMM_RESOURCE_USAGE
//
// Description:
//     Defines Usage types for different Resources
//----------------------------------------------------------------------------
typedef enum GMM_RESOURCE_USAGE_TYPE_ENUM
{

    //Generic Usage
    GMM_RESOURCE_USAGE_UNKNOWN = 0, // <== MUST EQUAL 0

    #define DEFINE_RESOURCE_USAGE(Usage) Usage,
    #include "../../../CachePolicy/GmmCachePolicyResourceUsageDefinitions.h"
    #undef DEFINE_RESOURCE_USAGE

    GMM_RESOURCE_USAGE_MAX          // <== MUST BE LAST ELEMENT

}GMM_RESOURCE_USAGE_TYPE;

typedef union GMM_PTE_CACHE_CONTROL_BITS_REC
{
    struct
    {
        uint32_t Valid         : 1 ;
        uint32_t CacheControl  : 2 ;
        uint32_t GFDT          : 1 ;
        uint32_t               : 28;
    }Gen6;
    struct
    {
        uint32_t Valid          : 1 ;
        uint32_t CacheControlLo : 3 ;
        uint32_t                : 7 ;
        uint32_t CacheControlHi : 1 ;
        uint32_t                : 20;
    }Gen7_5;
    struct
    {
        uint32_t Valid          : 1 ;
        uint32_t                : 2 ;
        uint32_t PWT            : 1 ;
        uint32_t PCD            : 1 ;
        uint32_t                : 2 ;
        uint32_t PAT            : 1 ;
        uint32_t                : 24;
    }Gen8;
    uint32_t                       DwordValue;
}GMM_PTE_CACHE_CONTROL_BITS;

typedef union MEMORY_OBJECT_CONTROL_STATE_REC
{
    struct
    {
        uint32_t CacheControl : 2;
        uint32_t GFDT : 1;
    uint32_t: 1;
    uint32_t: 28;
    }Gen6;
    struct
    {
        uint32_t L3 : 1;
        uint32_t CacheControl : 1;
        uint32_t GFDT : 1;
        uint32_t EncryptedData : 1;
    uint32_t: 28;
    }Gen7;
    struct
    {
        uint32_t L3 : 1;
        uint32_t CacheControl : 2;
        uint32_t EncryptedData : 1;
    uint32_t: 28;
    }Gen7_5;
    struct
    {
        uint32_t Age : 2;
        uint32_t EncryptedData : 1;
        uint32_t TargetCache : 2;
        uint32_t CacheControl : 2;
    uint32_t: 25;
    }Gen8;
    struct
    {
        uint32_t EncryptedData : 1;
        uint32_t Index         : 6 ;
        uint32_t               : 25;
    }Gen9, Gen10, Gen11,Gen12, XE_HP;

    uint32_t                       DwordValue;
}MEMORY_OBJECT_CONTROL_STATE;

//===========================================================================
// typedef:
//        GMM_CACHE_POLICY
//
// Description:
//     This struct is used for accessing Cache Policy functions.
//     Forward Declaration: Defined in GmmCachePolicy.h
//---------------------------------------------------------------------------
#ifdef __cplusplus
namespace GmmLib
{
    class GmmCachePolicyCommon;
}

typedef GmmLib::GmmCachePolicyCommon GMM_CACHE_POLICY;
#else
struct GmmCachePolicyCommon;
typedef struct GmmCachePolicyCommon GMM_CACHE_POLICY;
#endif
