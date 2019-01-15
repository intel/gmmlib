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
#include "GmmGen7TextureCalc.h"
/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmGen8TextureCalc.h
/// @brief This file contains the functions and members definations for texture alloc-
///        ation on all Gen8 platforms.
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members for Gen8 platforms.
    /// This class is derived from the base GmmGen7TextureCalc class  so clients
    /// shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmGen8TextureCalc :
                                public GmmGen7TextureCalc
    {
        private:

        protected:
            /* Function prototypes */
            virtual void            Fill2DTexOffsetAddress(
                                        GMM_TEXTURE_INFO *pTexInfo);

        public:
            /* Constructors */

            GmmGen8TextureCalc()
            {

            }

            ~GmmGen8TextureCalc()
            {

            }

            /* Function prototypes */
            virtual GMM_STATUS GMM_STDCALL  FillTex1D(GMM_TEXTURE_INFO   *pTexInfo,
                                                      __GMM_BUFFER_TYPE  *pRestrictions);

            virtual GMM_STATUS GMM_STDCALL  FillTex2D(GMM_TEXTURE_INFO   *pTexInfo,
                                                      __GMM_BUFFER_TYPE  *pRestrictions);

            virtual GMM_STATUS GMM_STDCALL  FillTexCube(GMM_TEXTURE_INFO   *pTexInfo,
                                                        __GMM_BUFFER_TYPE  *pRestrictions);
            virtual GMM_STATUS GMM_STDCALL MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo);

            /* inline functions */
    };
}
#endif // #ifdef __cplusplus