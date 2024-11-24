/*==============================================================================
Copyright(c) 2022 Intel Corporation

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
#include "GmmGen12TextureCalc.h"
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
    class NON_PAGED_SECTION GmmXe_LPGTextureCalc :
                                public GmmGen12TextureCalc
    {
        public:
            /* Constructors */

            GmmXe_LPGTextureCalc(Context *pGmmLibContext)
                : GmmGen12TextureCalc(pGmmLibContext)
            {

            }

            ~GmmXe_LPGTextureCalc()
            {

            }

            /* Function prototypes */

            virtual GMM_STATUS  GMM_STDCALL FillTexPlanar(GMM_TEXTURE_INFO    *pTexInfo,
                                                          __GMM_BUFFER_TYPE   *pRestrictions);
            virtual void                    FillPlanarOffsetAddress(GMM_TEXTURE_INFO *pTexInfo);
            virtual GMM_STATUS              GetTexLockOffset(GMM_TEXTURE_INFO    *pTexInfo,
                                                             GMM_REQ_OFFSET_INFO *pReqInfo);
            virtual GMM_GFX_SIZE_T          GetMipMapByteAddress(GMM_TEXTURE_INFO    *pTexInfo,
                                                                    GMM_REQ_OFFSET_INFO *pReqInfo);
            virtual uint32_t GMM_STDCALL       IsTileAlignedPlanes(GMM_TEXTURE_INFO *pTexInfo);
            virtual uint32_t GMM_STDCALL       GetNumberOfPlanes(GMM_TEXTURE_INFO *pTexInfo);
            virtual void GMM_STDCALL           SetPlanarOffsetInfo(GMM_TEXTURE_INFO            *pTexInfo,
                                                                GMM_RESCREATE_CUSTOM_PARAMS &CreateParams);
#ifndef __GMM_KMD__
            virtual void GMM_STDCALL SetPlanarOffsetInfo_2(GMM_TEXTURE_INFO              *pTexInfo,
                                                        GMM_RESCREATE_CUSTOM_PARAMS_2 &CreateParams);
#endif
            virtual void GMM_STDCALL SetPlaneUnAlignedTexOffsetInfo(GMM_TEXTURE_INFO *pTexInfo,
                                                                    uint32_t YHeight,
                                                                    uint32_t VHeight);
            virtual void GMM_STDCALL GetPlaneIdForCpuBlt(GMM_TEXTURE_INFO *pTexInfo,
                                                        GMM_RES_COPY_BLT *pBlt,
                                                        uint32_t *PlaneId);
            virtual void GMM_STDCALL GetBltInfoPerPlane(GMM_TEXTURE_INFO *pTexInfo,
                                                             GMM_RES_COPY_BLT *pBlt,
                                                             uint32_t PlaneId);

            /* inline functions */
    };
}
#endif // #ifdef __cplusplus
