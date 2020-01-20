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
#include "External/Common/GmmMemAllocator.hpp"
#include "Internal/Common/GmmLibInc.h"
/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmTextureCalc.h
/// @brief This file contains the common functions and members for texture calculations
///        on all GENs/Platforms
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{

    /////////////////////////////////////////////////////////////////////////
    /// Contains texture calc functions and members that are common across all
    /// platform implementation.  This is an abstract class and provides a
    /// uniform interface to all the texture clients and provides gen specific
    /// texture allocation through derived concrete GmmGenXTextureCalc class.
    /////////////////////////////////////////////////////////////////////////
    class NON_PAGED_SECTION GmmTextureCalc :
                                public GmmMemAllocator
    {
        private:

            static int32_t RefCount;
            static int32_t OverrideRefCount;

            GMM_STATUS      FillTexBlockMem(
                                GMM_TEXTURE_INFO    *pTexInfo,
                                __GMM_BUFFER_TYPE   *pRestrictions);

            void            GetTexRestrictions(
                                GMM_TEXTURE_INFO* pTexInfo,
                                __GMM_BUFFER_TYPE* pBuff);

            bool         ValidateTexInfo(
                                GMM_TEXTURE_INFO  *pTexInfo,
                                __GMM_BUFFER_TYPE *pRestrictions);

            uint32_t           GetDisplayFrameOffset(
                                GMM_TEXTURE_INFO*    pTexInfo,
                                GMM_REQ_OFFSET_INFO* pReqInfo);

        protected:
            /* Function prototypes */


            virtual GMM_STATUS  GMM_STDCALL FillTexPlanar(
                                GMM_TEXTURE_INFO    *pTexInfo,
                                __GMM_BUFFER_TYPE   *pRestrictions);

            GMM_STATUS      FillTexPitchAndSize(GMM_TEXTURE_INFO  *pTexInfo,
                                GMM_GFX_SIZE_T    WidthBytesPhysical,
                                uint32_t             Height,
                                __GMM_BUFFER_TYPE *pBufferType);

            void            FillTexPlanar_SetTilingBasedOnRequiredAlignment(
                                GMM_TEXTURE_INFO    *pTexInfo,
                                uint32_t               YHeight, bool YHeightAlignmentNeeded,
                                uint32_t               VHeight, bool VHeightAlignmentNeeded);

            void            FillPlanarOffsetAddress(
                                GMM_TEXTURE_INFO   *pTexInfo);

            void            FindMipTailStartLod(GMM_TEXTURE_INFO *pTexInfo);
            GMM_VIRTUAL void                GetGenericRestrictions(GMM_TEXTURE_INFO* pTexInfo,
                                                                   __GMM_BUFFER_TYPE *pBuff);
            GMM_VIRTUAL __GMM_BUFFER_TYPE*  GetBestRestrictions(__GMM_BUFFER_TYPE *pFirstBuffer,
                                                                const __GMM_BUFFER_TYPE *pSecondBuffer);
            /* Inline functions */
            /////////////////////////////////////////////////////////////////////////////////////
            /// Checks where the restrictions are invalid or not
            /// @param[in]  pRestriction Restrictions to check
            /// @return     true if restriction is invalid. false otherwise.
            /////////////////////////////////////////////////////////////////////////////////////
            GMM_INLINE_VIRTUAL GMM_INLINE bool IsRestrictionInvalid(__GMM_BUFFER_TYPE *pRestriction)
            {
                return ((pRestriction->MinDepth == 0xffffffff) ? true : false);
            }
            /////////////////////////////////////////////////////////////////////////////////////
            /// Returns restrictions for a linear buffer.
            /// @param[out]     pBuff Restrictions are returned in this buffer
            /////////////////////////////////////////////////////////////////////////////////////
            GMM_INLINE_VIRTUAL GMM_INLINE void GetLinearRestrictions(GMM_TEXTURE_INFO* pTexInfo, __GMM_BUFFER_TYPE* pBuff)
            {
                *pBuff = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo)->Linear;
            }
            /////////////////////////////////////////////////////////////////////////////////////
            /// Returns restrictions for the primary buffer.
            /// @param[out]     pBuff Restrictions are returned in this buffer
            /////////////////////////////////////////////////////////////////////////////////////
            GMM_INLINE_VIRTUAL GMM_INLINE void GetPrimaryRestrictions(GMM_TEXTURE_INFO* pTexInfo, __GMM_BUFFER_TYPE* pBuff)
            {
                *pBuff = GMM_OVERRIDE_PLATFORM_INFO(pTexInfo)->ASyncFlipSurface;
            }

            virtual uint32_t   Get2DMipMapHeight(GMM_TEXTURE_INFO   *pTexInfo) = 0;

            virtual void    Fill2DTexOffsetAddress(GMM_TEXTURE_INFO *pTexInfo) = 0;

            virtual GMM_GFX_SIZE_T  Get2DTexOffsetAddressPerMip(
                                GMM_TEXTURE_INFO *pTexInfo,
                                uint32_t            MipLevel) = 0;

            virtual uint32_t   GetMipTailByteOffset(
                                GMM_TEXTURE_INFO *pTexInfo,
                                uint32_t            MipLevel)
            {
                GMM_UNREFERENCED_PARAMETER(pTexInfo);
                GMM_UNREFERENCED_PARAMETER(MipLevel);
                // Left empty
                return 0;
            }

            virtual void    GetMipTailGeometryOffset(
                                GMM_TEXTURE_INFO *pTexInfo,
                                uint32_t            MipLevel,
                                uint32_t*           OffsetX,
                                uint32_t*           OffsetY,
                                uint32_t*           OffsetZ)
            {
                GMM_UNREFERENCED_PARAMETER(pTexInfo);
                GMM_UNREFERENCED_PARAMETER(MipLevel);
                GMM_UNREFERENCED_PARAMETER(OffsetX);
                GMM_UNREFERENCED_PARAMETER(OffsetY);
                GMM_UNREFERENCED_PARAMETER(OffsetZ);

                // Left empty
            }

            GMM_GFX_SIZE_T  Get3DMipByteAddress(
                                GMM_TEXTURE_INFO*    pTexInfo,
                                GMM_REQ_OFFSET_INFO *pReqInfo);

            virtual uint32_t   GetAligned3DBlockHeight(
                                GMM_TEXTURE_INFO*    pTexInfo,
                                uint32_t BlockHeight,
                                uint32_t ExpandedArraySize)
            {
                GMM_UNREFERENCED_PARAMETER(pTexInfo);
                GMM_UNREFERENCED_PARAMETER(BlockHeight);
                GMM_UNREFERENCED_PARAMETER(ExpandedArraySize);

                // Left empty
                return 0;
            }

        public:
            /* Constructors */
            // "Creates GmmTextureCalc object based on platform ID"
            void            SetTileMode(GMM_TEXTURE_INFO* pTexInfo);
            static GmmTextureCalc* Create(PLATFORM Platform, uint8_t Override);

            static void IncrementRefCount()
            {
                #if defined(__GMM_KMD__) || _WIN32
                    InterlockedIncrement((LONG *)&RefCount);
                #elif defined(__linux__)
                    __sync_fetch_and_add(&RefCount, 1);
                #endif
                    //TODO[Android]
            }

            static int32_t DecrementRefCount()
            {
                #if defined(__GMM_KMD__) || _WIN32
                    return(InterlockedDecrement((LONG *)&RefCount));
                #elif defined(__linux__)
                    return(__sync_sub_and_fetch(&RefCount, 1));
                #endif
                    //TODO[Android]
            }

            GmmTextureCalc()
            {

            }

            virtual ~GmmTextureCalc()
            {

            }

            /* Function prototypes */
            GMM_STATUS      AllocateTexture(GMM_TEXTURE_INFO *pTexInfo);
            virtual GMM_STATUS      FillTexCCS(GMM_TEXTURE_INFO *pBaseSurf, GMM_TEXTURE_INFO *pTexInfo);

            GMM_STATUS      PreProcessTexSpecialCases(
                                GMM_TEXTURE_INFO* pTexInfo);

            uint32_t           ExpandHeight(
                                uint32_t Height,
                                uint32_t UnitAlignment,
                                uint32_t NumSamples);

            uint32_t           ExpandWidth(
                                uint32_t Width,
                                uint32_t UnitAlignment,
                                uint32_t NumSamples);

            void            GetCompressionBlockDimensions(
                                GMM_RESOURCE_FORMAT Format,
                                uint32_t *pWidth,
                                uint32_t *pHeight,
                                uint32_t *pDepth);

            GMM_STATUS      GetTexRenderOffset(
                                GMM_TEXTURE_INFO*    pTexInfo,
                                GMM_REQ_OFFSET_INFO* pReqInfo);

            GMM_STATUS      GetTexLockOffset(
                                GMM_TEXTURE_INFO* pTexInfo,
                                GMM_REQ_OFFSET_INFO *pReqInfo);

            GMM_STATUS      GetTexStdLayoutOffset(
                                GMM_TEXTURE_INFO* pTexInfo,
                                GMM_REQ_OFFSET_INFO *pReqInfo);

            GMM_GFX_SIZE_T  GetMipMapByteAddress(
                                GMM_TEXTURE_INFO*    pTexInfo,
                                GMM_REQ_OFFSET_INFO *pReqInfo);

            void            AlignTexHeightWidth(
                                GMM_TEXTURE_INFO*   pTexInfo,
                                uint32_t*              pHeight,
                                uint32_t*     pWidth);

            bool     GmmGetD3DToHwTileConversion(
                                GMM_TEXTURE_INFO *pTexInfo,
                                uint32_t             *pColFactor,
                                uint32_t             *pRowFactor);
            void    GetResRestrictions(
                                  GMM_TEXTURE_INFO* pTexInfo,
                                  __GMM_BUFFER_TYPE& pBuff);

            // Virtual functions
            virtual GMM_STATUS GMM_STDCALL  FillTex1D(
                                                GMM_TEXTURE_INFO  *pTexInfo,
                                                __GMM_BUFFER_TYPE *pRestrictions) = 0;
            virtual GMM_STATUS GMM_STDCALL  FillTex2D(
                                                GMM_TEXTURE_INFO  *pTexInfo,
                                                __GMM_BUFFER_TYPE *pRestrictions) = 0;
            virtual GMM_STATUS GMM_STDCALL  FillTex3D(
                                                GMM_TEXTURE_INFO  *pTexInfo,
                                                __GMM_BUFFER_TYPE *pRestrictions) = 0;
            virtual GMM_STATUS GMM_STDCALL  FillTexCube(
                                                GMM_TEXTURE_INFO    *pTexInfo,
                                                __GMM_BUFFER_TYPE   *pRestrictions) = 0;

            virtual GMM_STATUS GMM_STDCALL GetCCSExMode(GMM_TEXTURE_INFO * AuxSurf)
            {
                GMM_UNREFERENCED_PARAMETER(AuxSurf);

                return GMM_SUCCESS;
            }

            virtual uint32_t  GMM_STDCALL ScaleTextureWidth(GMM_TEXTURE_INFO* pTexInfo,
                                                         uint32_t Width)
            {
                __GMM_ASSERT(pTexInfo != NULL);

                switch (pTexInfo->BitsPerPixel)
                {
                    case 32:  Width /= 8; break;
                    case 64:  Width /= 4; break;
                    case 128: Width /= 2; break;
                    default: __GMM_ASSERT(0);
                }

                return Width;
            }

            virtual uint32_t  GMM_STDCALL ScaleTextureHeight(GMM_TEXTURE_INFO* pTexInfo,
                                                        uint32_t Height)
            {
                GMM_UNREFERENCED_PARAMETER(pTexInfo);
                return Height /= 16;
            }

            virtual GMM_GFX_SIZE_T  GmmTexGetMipWidth(GMM_TEXTURE_INFO *pTexInfo, uint32_t MipLevel)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);
                return(GFX_MAX(1, pTexInfo->BaseWidth >> MipLevel));
            }
            virtual uint32_t GmmTexGetMipHeight(GMM_TEXTURE_INFO *pTexInfo, uint32_t MipLevel)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);
                return(GFX_MAX(1, pTexInfo->BaseHeight >> MipLevel));
            }

            virtual uint32_t GmmTexGetMipDepth(GMM_TEXTURE_INFO *pTexInfo, uint32_t MipLevel)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);
                return(GFX_MAX(1, pTexInfo->Depth >> MipLevel));
            }

            virtual uint32_t GMM_STDCALL ScaleFCRectHeight(GMM_TEXTURE_INFO * pTexInfo, uint32_t Height)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);
                uint32_t ScaledHeight = Height;

                if (pTexInfo->TileMode == LEGACY_TILE_X)
                {
                    const uint16_t FastClearRccTileXAlignHeight = 64; // lines - RCC ( Render Color Cache ) Alignment Sizes
                    const uint16_t TileXClearHeightScale = 32;         // lines - Clear & Resolve Rect Scaling Sizes

                    ScaledHeight = GFX_ALIGN(ScaledHeight, FastClearRccTileXAlignHeight);
                    ScaledHeight /= TileXClearHeightScale;
                }
                else if (pTexInfo->TileMode == LEGACY_TILE_Y)
                {
                    const uint16_t FastClearRccTileYAlignHeight = 128; // bits
                    const uint16_t TileYClearHeightScale = 64;        // bits

                    ScaledHeight = GFX_ALIGN(ScaledHeight, FastClearRccTileYAlignHeight);
                    ScaledHeight /= TileYClearHeightScale;
                }
                return ScaledHeight;

            }

            virtual uint64_t GMM_STDCALL ScaleFCRectWidth(GMM_TEXTURE_INFO * pTexInfo, uint64_t Width)
            {
                __GMM_ASSERTPTR(pTexInfo, 0);
                uint64_t ScaledWidth = Width;

                if (pTexInfo->TileMode == LEGACY_TILE_X)
                {
                    const uint32_t FastClearRccTileXAlignWidth = 8192; // bits - RCC ( Render Color Cache ) Alignment Sizes
                    const uint32_t TileXClearWidthScale = 4096;        // bits - Clear & Resolve Rect Scaling Sizes

                    ScaledWidth = GFX_ALIGN(ScaledWidth, FastClearRccTileXAlignWidth / pTexInfo->BitsPerPixel);
                    ScaledWidth /= TileXClearWidthScale;
                }
                else if (pTexInfo->TileMode == LEGACY_TILE_Y)
                {
                    const uint32_t FastClearRccTileYAlignWidth = 4096; // bits
                    const uint32_t TileYClearWidthScale = 2048;        // bits

                    ScaledWidth = GFX_ALIGN(ScaledWidth, FastClearRccTileYAlignWidth / pTexInfo->BitsPerPixel);
                    ScaledWidth /= TileYClearWidthScale;
                }
                return ScaledWidth;
            }

            virtual GMM_STATUS GMM_STDCALL MSAACCSUsage(GMM_TEXTURE_INFO *pTexInfo);
            virtual void GMM_STDCALL AllocateOneTileThanRequied(GMM_TEXTURE_INFO *pTexInfo,
                                                                GMM_GFX_SIZE_T &WidthBytesRender,
                                                                GMM_GFX_SIZE_T &WidthBytesPhysical,
                                                                GMM_GFX_SIZE_T &WidthBytesLock);
            GMM_STATUS MSAACompression(GMM_TEXTURE_INFO *pTexInfo);

            /* inline functions */
    };

}
#endif // #ifdef __cplusplus
