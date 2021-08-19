/*==============================================================================
Copyright(c) 2019 Intel Corporation

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
#include "GmmGen11TextureCalc.h"
#include "../Platform/GmmGen12Platform.h"

/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmGen12TextureCalc.h
/// @brief This file contains the functions and members definations for texture alloc-
///        ation on all Gen12 platforms.
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members for Gen12 platforms.
    /// This class is derived from the base GmmTextureCalc class  so clients
    /// shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmGen12TextureCalc :
                                public GmmGen11TextureCalc
    {
        private:

        protected:
            virtual uint32_t GetMipTailByteOffset(
            GMM_TEXTURE_INFO *pTexInfo,
            uint32_t          MipLevel);

            virtual void GetMipTailGeometryOffset(
            GMM_TEXTURE_INFO *pTexInfo,
            uint32_t          MipLevel,
            uint32_t *        OffsetX,
            uint32_t *        OffsetY,
            uint32_t *        OffsetZ);

            virtual uint32_t           Get2DMipMapHeight(
                                        GMM_TEXTURE_INFO   *pTexInfo);
            virtual GMM_STATUS      FillTexCCS(
                                        GMM_TEXTURE_INFO  *pSurf,
                                        GMM_TEXTURE_INFO  *pAuxTexInfo);
        public:
            /* Constructors */
            GmmGen12TextureCalc(Context *pGmmLibContext)
                : GmmGen11TextureCalc(pGmmLibContext)
            {

            }

            ~GmmGen12TextureCalc()
            {

            }

            /* Function prototypes */

            virtual GMM_STATUS GMM_STDCALL  FillTex2D(GMM_TEXTURE_INFO   *pTexInfo,
                __GMM_BUFFER_TYPE  *pRestrictions);


            virtual GMM_STATUS  GMM_STDCALL FillTexPlanar(GMM_TEXTURE_INFO    *pTexInfo,
                                __GMM_BUFFER_TYPE   *pRestrictions);

            virtual GMM_STATUS GMM_STDCALL GetCCSScaleFactor(GMM_TEXTURE_INFO * pTexInfo,
                          CCS_UNIT& ScaleFactor);

            GMM_STATUS GMM_STDCALL GetCCSExMode(GMM_TEXTURE_INFO * AuxSurf);

            virtual uint32_t  GMM_STDCALL ScaleTextureHeight(GMM_TEXTURE_INFO * pTexInfo,
                                                          uint32_t Height);

            virtual uint32_t GMM_STDCALL ScaleTextureWidth (GMM_TEXTURE_INFO* pTexInfo,
                                                         uint32_t Width);

            virtual uint32_t  GMM_STDCALL ScaleFCRectHeight(GMM_TEXTURE_INFO * pTexInfo,
                                                          uint32_t Height);

            virtual uint64_t GMM_STDCALL ScaleFCRectWidth(GMM_TEXTURE_INFO* pTexInfo,
                                                         uint64_t Width);
            virtual GMM_STATUS GMM_STDCALL MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo);
            virtual void GMM_STDCALL AllocateOneTileThanRequied(GMM_TEXTURE_INFO *pTexInfo,
                                                                GMM_GFX_SIZE_T &WidthBytesRender,
                                                                GMM_GFX_SIZE_T &WidthBytesPhysical,
                                                                GMM_GFX_SIZE_T &WidthBytesLock)
            {
                GMM_UNREFERENCED_PARAMETER(pTexInfo);
                GMM_UNREFERENCED_PARAMETER(WidthBytesRender);
                GMM_UNREFERENCED_PARAMETER(WidthBytesPhysical);
                GMM_UNREFERENCED_PARAMETER(WidthBytesLock);
            }

            /* inline functions */
    };
}
#endif // #ifdef __cplusplus
