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
#include "GmmTextureCalc.h"
/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmGen9TextureCalc.h
/// @brief This file contains the functions and members definations for texture alloc-
///        ation on all Gen9 platforms.
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members for Gen9 platforms.
    /// This class is derived from the base GmmTextureCalc class  so clients
    /// shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmGen9TextureCalc :
                                public GmmTextureCalc
    {
        private:
            void                    Fill1DTexOffsetAddress(
                                        GMM_TEXTURE_INFO *pTexInfo);

            GMM_GFX_SIZE_T          Get1DTexOffsetAddressPerMip(
                                        GMM_TEXTURE_INFO *pTexInfo,
                                        uint32_t            MipLevel);
        protected:
            /* Function prototypes */

            virtual void            Fill2DTexOffsetAddress(
                                        GMM_TEXTURE_INFO *pTexInfo);

            virtual GMM_GFX_SIZE_T  Get2DTexOffsetAddressPerMip(
                                        GMM_TEXTURE_INFO   *pTexInfo,
                                        uint32_t               MipLevel);

            virtual uint32_t           Get2DMipMapHeight(
                                        GMM_TEXTURE_INFO   *pTexInfo);

            uint32_t                   Get2DMipMapTotalHeight(
                                        GMM_TEXTURE_INFO *pTexInfo);

            virtual uint32_t           GetMipTailByteOffset(
                                        GMM_TEXTURE_INFO *pTexInfo,
                                        uint32_t            MipLevel);

            virtual void            GetMipTailGeometryOffset(
                                        GMM_TEXTURE_INFO *pTexInfo,
                                        uint32_t            MipLevel,
                                        uint32_t*           OffsetX,
                                        uint32_t*           OffsetY,
                                        uint32_t*           OffsetZ);

            virtual uint32_t           GetAligned3DBlockHeight(
                                        GMM_TEXTURE_INFO*    pTexInfo,
                                        uint32_t BlockHeight,
                                        uint32_t ExpandedArraySize);

        public:
            /* Constructors */

            GmmGen9TextureCalc()
            {

            }

            ~GmmGen9TextureCalc()
            {

            }

            /* Function prototypes */

            virtual GMM_STATUS GMM_STDCALL  FillTex1D(GMM_TEXTURE_INFO   *pTexInfo,
                                                      __GMM_BUFFER_TYPE  *pRestrictions);

            virtual GMM_STATUS GMM_STDCALL  FillTex2D(GMM_TEXTURE_INFO   *pTexInfo,
                                                      __GMM_BUFFER_TYPE  *pRestrictions);

            virtual GMM_STATUS GMM_STDCALL  FillTex3D(GMM_TEXTURE_INFO   *pTexInfo,
                                                      __GMM_BUFFER_TYPE  *pRestrictions);

            virtual GMM_STATUS GMM_STDCALL  FillTexCube(GMM_TEXTURE_INFO   *pTexInfo,
                                                        __GMM_BUFFER_TYPE  *pRestrictions);
            virtual GMM_STATUS GMM_STDCALL MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo);

            virtual uint32_t GMM_STDCALL ScaleFCRectHeight(GMM_TEXTURE_INFO * pTexInfo, uint32_t Height)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);
                uint32_t ScaledHeight = Height;

                if (pTexInfo->TileMode == LEGACY_TILE_Y)
                {
                    const uint16_t FastClearRccTileYAlignHeight = 64; // lines - RCC ( Render Color Cache ) Alignment Sizes
                    const uint16_t TileYClearHeightScale = 32;        // lines - Clear & Resolve Rect Scaling Sizes

                    ScaledHeight = GFX_ALIGN(ScaledHeight, FastClearRccTileYAlignHeight);
                    ScaledHeight /= TileYClearHeightScale;
                }
                return ScaledHeight;
            }

            /* inline functions */
    };
}
#endif // #ifdef __cplusplus