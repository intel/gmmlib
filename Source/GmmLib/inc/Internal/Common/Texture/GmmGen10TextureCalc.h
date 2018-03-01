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
#include "GmmGen9TextureCalc.h"
/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmGen10TextureCalc.h
/// @brief This file contains the functions and members definations for texture alloc-
///        ation on all Gen10 platforms.
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members for Gen10 platforms.
    /// This class is derived from the base GmmTextureCalc class  so clients
    /// shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmGen10TextureCalc :
                                public GmmGen9TextureCalc
    {
        private:

        protected:
            /* Function prototypes */

            virtual uint32_t   GetMipTailByteOffset(
                                GMM_TEXTURE_INFO *pTexInfo,
                                uint32_t            MipLevel);

            virtual void    GetMipTailGeometryOffset(
                                GMM_TEXTURE_INFO *pTexInfo,
                                uint32_t            MipLevel,
                                uint32_t*           OffsetX,
                                uint32_t*           OffsetY,
                                uint32_t*           OffsetZ);

            virtual uint32_t   GetAligned3DBlockHeight(
                                GMM_TEXTURE_INFO*    pTexInfo,
                                uint32_t BlockHeight,
                                uint32_t ExpandedArraySize);

        public:
            /* Constructors */

            GmmGen10TextureCalc()
            {

            }

            ~GmmGen10TextureCalc()
            {

            }

            /* Function prototypes */

            virtual GMM_STATUS GMM_STDCALL  FillTex2D(
                GMM_TEXTURE_INFO   *pTexInfo,
                __GMM_BUFFER_TYPE  *pRestrictions);

            virtual GMM_STATUS  GMM_STDCALL FillTexPlanar(
                GMM_TEXTURE_INFO    *pTexInfo,
                __GMM_BUFFER_TYPE   *pRestrictions);

            /* inline functions */
    };
}
#endif // #ifdef __cplusplus