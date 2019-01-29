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
/// @file GmmGen11TextureCalc.h
/// @brief This file contains the functions and members definations for texture alloc-
///        ation on all Gen11 platforms.
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members for Gen11 platforms.
    /// This class is derived from the base GmmTextureCalc class  so clients
    /// shouldn't have to ever interact  with this class directly.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmGen11TextureCalc :
                                public GmmGen10TextureCalc
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

            virtual GMM_STATUS  GMM_STDCALL FillTexPlanar(
                                    GMM_TEXTURE_INFO    *pTexInfo,
                                    __GMM_BUFFER_TYPE   *pRestrictions);

            GMM_STATUS   FillLinearCCS(
                                    GMM_TEXTURE_INFO    *pTexInfo,
                                    __GMM_BUFFER_TYPE   *pRestrictions);

            virtual void FillPlanarOffsetAddress(GMM_TEXTURE_INFO *pTexInfo);

        public:
            /* Constructors */

            GmmGen11TextureCalc()
            {

            }

            ~GmmGen11TextureCalc()
            {

            }

            /* Virtual Functions */
            /////////////////////////////////////////////////////////////////////////
            // Returns unaligned/unpadded width of a given LOD
            //  Default         : WL = ((width >> L) > 0 ? width >> L : 1)
            //  CornerTexelMode : WL = MAX(1,(width-1) >> L) + 1 or WL = MAX(1,(WLminus1-1) >> 1) + 1
            /////////////////////////////////////////////////////////////////////////
            virtual GMM_GFX_SIZE_T  GmmTexGetMipWidth(GMM_TEXTURE_INFO *pTexInfo, uint32_t MipLevel)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);

                if (!pTexInfo->Flags.Info.CornerTexelMode)
                {
                    return(GFX_MAX(1, pTexInfo->BaseWidth >> MipLevel));
                }
                else
                {
                    if (pTexInfo->BaseWidth == 1)
                    {
                        return pTexInfo->BaseWidth;
                    }
                    else
                    {
                        return(GFX_MAX(1, (pTexInfo->BaseWidth - 1) >> MipLevel) + 1);
                    }
                }
            }

            /////////////////////////////////////////////////////////////////////////
            // Returns unaligned/unpadded height of a given LOD
            //  Default         : HL = ((height >> L) > 0 ? height >> L : 1)
            //  CornerTexelMode : HL = MAX(1,(height-1) >> L) + 1 or HL = MAX(1,(HLminus1-1) >> 1) + 1
            /////////////////////////////////////////////////////////////////////////
            virtual uint32_t  GmmTexGetMipHeight(GMM_TEXTURE_INFO *pTexInfo, uint32_t MipLevel)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);

                if (!pTexInfo->Flags.Info.CornerTexelMode)
                {
                    return(GFX_MAX(1, pTexInfo->BaseHeight >> MipLevel));
                }
                else
                {
                    if (pTexInfo->BaseHeight == 1)
                    {
                        return pTexInfo->BaseHeight;
                    }
                    else
                    {
                        return(GFX_MAX(1, (pTexInfo->BaseHeight - 1) >> MipLevel) + 1);
                    }
                }
            }

            /////////////////////////////////////////////////////////////////////////
            // Returns unaligned/unpadded depth of a given LOD
            //  Default         : DL = ((depth >> L) > 0 ? depth >> L : 1)
            //  CornerTexelMode : DL = MAX(1,(depth-1) >> L) + 1 or DL = MAX(1,(DLminus1-1) >> 1) + 1
            /////////////////////////////////////////////////////////////////////////
            virtual uint32_t  GmmTexGetMipDepth(GMM_TEXTURE_INFO *pTexInfo, uint32_t MipLevel)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);

                if (!pTexInfo->Flags.Info.CornerTexelMode)
                {
                    return(GFX_MAX(1, pTexInfo->Depth >> MipLevel));
                }
                else
                {
                    if (pTexInfo->Depth == 1)
                    {
                        return pTexInfo->Depth;
                    }
                    else
                    {
                        return(GFX_MAX(1, (pTexInfo->Depth - 1) >> MipLevel) + 1);
                    }
                }
            }

            /* Function prototypes */

            /* inline functions */
    };
}
#endif // #ifdef __cplusplus