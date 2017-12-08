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
#include "GmmMemAllocator.hpp"
#include "GmmLibInc.h"
/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmTextureCalc.h
/// @brief This file contains the common functions and members for texture calculations
///        on all GENs/Platforms
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members that are common across all platform
    /// implementation.  This class is inherited by gen specific class so
    /// so clients shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmTextureCalc :
                public GmmMemAllocator
    {
        // GMMLIB2_TODO:
        // 1) Change it to protected when all functions are part of the class
        // 2) Mode Gen specific implementation in derived class later.
        private:

        protected:
            /* Function prototypes */

        public:
            /* Constructors */

            // "Creates GmmTextureCalc object based on platform ID"
            static GmmTextureCalc* Create(PLATFORM Platform)
            {
                return new GmmTextureCalc();
            }

            GmmTextureCalc()
            {

            }

            ~GmmTextureCalc()
            {

            }

            /* Function prototypes */

            //GmmTextureAlloc.cpp
            GMM_STATUS AllocateTexture(GMM_TEXTURE_INFO *pTexInfo);

            GMM_STATUS __GmmTexFillBlockMem(
                            GMM_TEXTURE_INFO    *pTexInfo,
                            __GMM_BUFFER_TYPE   *pRestrictions);

            GMM_STATUS  GmmTexFillPitchAndSize(GMM_TEXTURE_INFO  *pTexInfo,
                            GMM_GFX_SIZE_T    WidthBytesPhysical,
                            uint32_t             Height,
                            __GMM_BUFFER_TYPE *pBufferType);

            void        GmmTexGetRestrictions(
                            GMM_TEXTURE_INFO* pTexInfo,
                            __GMM_BUFFER_TYPE* pBuff);

            GMM_STATUS  GMM_STDCALL __GmmTexFillPlanar(
                            GMM_TEXTURE_INFO    *pTexInfo,
                            __GMM_BUFFER_TYPE   *pRestrictions);

            void        __GmmTexFillPlanar_SetTilingBasedOnRequiredAlignment(
                            GMM_TEXTURE_INFO    *pTexInfo,
                            uint32_t               YHeight, bool YHeightAlignmentNeeded,
                            uint32_t               VHeight, bool VHeightAlignmentNeeded);

            bool     __GmmTexValidateTexInfo(
                            GMM_TEXTURE_INFO  *pTexInfo,
                            __GMM_BUFFER_TYPE *pRestrictions);

            void        __GmmSetTileMode(
                            GMM_TEXTURE_INFO* pTexInfo);

            GMM_STATUS  __GmmTexSpecialCasePreProcessing(
                            GMM_TEXTURE_INFO* pTexInfo);

            //GmmTexture.cpp

            uint32_t       __GmmExpandHeight(
                            uint32_t Height,
                            uint32_t UnitAlignment,
                            uint32_t NumSamples);

            uint32_t       __GmmExpandWidth(
                            uint32_t Width,
                            uint32_t UnitAlignment,
                            uint32_t NumSamples);

            void        __GmmGetElementDimensions(
                            GMM_RESOURCE_FORMAT Format,
                            uint32_t *pWidth,
                            uint32_t *pHeight,
                            uint32_t *pDepth);

            void        __GmmFillPlanarOffsetAddress(
                            GMM_TEXTURE_INFO   *pTexInfo);

            void        __GmmFindMipTailStartLod(
                            GMM_TEXTURE_INFO *pTexInfo);


            // Virtual functions
            // TODO GmmLib2.0 Create derived class of TextureCalc and add virtual functions.
            /*virtual GMM_STATUS GMM_STDCALL __GmmTexFill1D(
                                                GMM_TEXTURE_INFO  *pTexInfo,
                                                __GMM_BUFFER_TYPE *pRestrictions) = 0;
            virtual GMM_STATUS GMM_STDCALL __GmmTexFill2D(
                                                GMM_TEXTURE_INFO  *pTexInfo,
                                                __GMM_BUFFER_TYPE *pRestrictions) = 0;
            virtual GMM_STATUS GMM_STDCALL __GmmTexFill3D(
                                                GMM_TEXTURE_INFO  *pTexInfo,
                                                __GMM_BUFFER_TYPE *pRestrictions) = 0;
            virtual GMM_STATUS GMM_STDCALL __GmmTexFillCube(
                                                GMM_TEXTURE_INFO    *pTexInfo,
                                                __GMM_BUFFER_TYPE   *pRestrictions) = 0;*/

            /* inline functions */
    };

}
#endif // #ifdef __cplusplus