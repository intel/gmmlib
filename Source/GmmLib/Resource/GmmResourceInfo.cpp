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

#include "Internal/Common/GmmLibInc.h"

#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::Create.
/// @see        GmmLib::GmmResourceInfoCommon::Create()
///
/// @param[in] pCreateParams: Flags which specify what sort of resource to create
/// @return     Pointer to GmmResourceInfo class.
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmResCreate(GMM_RESCREATE_PARAMS *pCreateParams)
{
    GMM_RESOURCE_INFO *pRes = NULL;

#if(!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
    pRes = GmmResCreateThroughClientCtxt(pCreateParams);
    return pRes;
#else

    // GMM_RESOURCE_INFO...
    if(pCreateParams->pPreallocatedResInfo)
    {
        pRes = new(pCreateParams->pPreallocatedResInfo) GmmLib::GmmResourceInfo(); // Use preallocated memory as a class
        pCreateParams->Flags.Info.__PreallocatedResInfo =
        pRes->GetResFlags().Info.__PreallocatedResInfo = 1; // Set both in case we can die before copying over the flags.
    }
    else
    {
        if((pRes = new GMM_RESOURCE_INFO) == NULL)
        {
            GMM_ASSERTDPF(0, "Allocation failed!");
            goto ERROR_CASE;
        }
    }

    if(pRes->Create(*pGmmGlobalContext, *pCreateParams) != GMM_SUCCESS)
    {
        goto ERROR_CASE;
    }

    return (pRes);

ERROR_CASE:
    if(pRes)
    {
        GmmResFree(pRes);
    }

    GMM_DPF_EXIT;
    return (NULL);

#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::opeartor=. Allocates a new class and
/// returns a pointer to it. The new class must be free'd explicitly by the client.
///
/// @see GmmLib::GmmResourceInfoCommon::operator=()
///
/// @param[in] pRes: Pointer to the GmmResourceInfo class that needs to be copied
/// @return     Pointer to newly copied GmmResourceInfo class
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_INFO *GMM_STDCALL GmmResCopy(GMM_RESOURCE_INFO *pRes)
{
    GMM_RESOURCE_INFO *pResCopy = NULL;

    GMM_DPF_ENTER;
    __GMM_ASSERTPTR(pRes, NULL);

#if(!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
    pResCopy = GmmResCopyThroughClientCtxt(pRes);
    return pResCopy;
#else

    pResCopy = new GMM_RESOURCE_INFO;

    if(!pResCopy)
    {
        GMM_ASSERTDPF(0, "Allocation failed.");
        return NULL;
    }

    *pResCopy = *pRes;

    // We are allocating new class, flag must be false to avoid leak at DestroyResource
    pResCopy->GetResFlags().Info.__PreallocatedResInfo = 0;

    GMM_DPF_EXIT;
    return (pResCopy);

#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// Helps clients copy one GmmResourceInfo object to another
///
/// @param[in] pDst: Pointer to memory when pSrc will be copied.
/// @param[in] pSrc: Pointer to GmmResourceInfo class that needs to be copied
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResMemcpy(void *pDst, void *pSrc)
{
#if(!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
    GmmResMemcpyThroughClientCtxt(pDst, pSrc);
#else
    GMM_RESOURCE_INFO *pResSrc = reinterpret_cast<GMM_RESOURCE_INFO *>(pSrc);
    // Init memory correctly, in case the pointer is a raw memory pointer
    GMM_RESOURCE_INFO *pResDst = new(pDst) GMM_RESOURCE_INFO();

    *pResDst = *pResSrc;
#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for ~GmmResourceInfoCommon. Frees the resource if it wasn't part of
/// ::GMM_RESCREATE_PARAMS::pPreallocatedResInfo.
/// @see        GmmLib::GmmResourceInfoCommon::~GmmResourceInfoCommon()
///
/// @param[in]  pRes: Pointer to the GmmResourceInfo class that needs to be freed
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResFree(GMM_RESOURCE_INFO *pRes)
{
    GMM_DPF_ENTER;
    __GMM_ASSERTPTR(pRes, VOIDRETURN);

#if(!defined(__GMM_KMD__) && !defined(GMM_UNIFIED_LIB))
    GmmResFreeThroughClientCtxt(pRes);
#else

    if(pRes->GetResFlags().Info.__PreallocatedResInfo)
    {
        *pRes = GmmLib::GmmResourceInfo();
    }
    else
    {
        delete pRes;
        pRes = NULL;
    }

#endif
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetSystemMemPointer.
/// @see        GmmLib::GmmResourceInfoCommon::GetSystemMemPointer()
///
/// @param[in]  pRes: Pointer to the GmmResourceInfo class
/// @param[in]  IsD3DDdiAllocation: Specifies where allocation was made by a D3D client
/// @return     Pointer to system memory. NULL if not available.
/////////////////////////////////////////////////////////////////////////////////////
void *GMM_STDCALL GmmResGetSystemMemPointer(GMM_RESOURCE_INFO *pRes,
                                            uint8_t            IsD3DDdiAllocation)
{
    GMM_DPF_ENTER;
    __GMM_ASSERTPTR(pRes, NULL);

    return pRes->GetSystemMemPointer(IsD3DDdiAllocation);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetSystemMemSize.
/// @see        GmmLib::GmmResourceInfoCommon::GetSystemMemSize()
///
/// @param[in]  pRes: Pointer to the GmmResourceInfo class
/// @return     Size of memory.
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetSystemMemSize(GMM_RESOURCE_INFO *pRes)
{
    __GMM_ASSERTPTR(pRes, ((GMM_GFX_SIZE_T)0));
    return pRes->GetSystemMemSize();
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function returns size of GMM_RESOURCE_INFO
///
/// @return     size of class
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSizeOfStruct(void)
{
    return (sizeof(GMM_RESOURCE_INFO));
}

/////////////////////////////////////////////////////////////////////////////////////
/// This function returns resource flags
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[out] pFlags: Memory where resource flags will be copied
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResGetFlags(GMM_RESOURCE_INFO *pGmmResource,
                                GMM_RESOURCE_FLAG *pFlags /*output*/)
{
    GMM_DPF_ENTER;
    __GMM_ASSERTPTR(pGmmResource, VOIDRETURN);
    __GMM_ASSERTPTR(pFlags, VOIDRETURN);

    *pFlags = GmmResGetResourceFlags(pGmmResource);

    GMM_DPF_EXIT;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetResourceType.
/// @see        GmmLib::GmmResourceInfoCommon::GetResourceType()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Resource Type
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_TYPE GMM_STDCALL GmmResGetResourceType(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, RESOURCE_INVALID);
    return pGmmResource->GetResourceType();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetResourceFormat.
/// @see        GmmLib::GmmResourceInfoCommon::GetResourceFormat()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Resource Format
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_FORMAT GMM_STDCALL GmmResGetResourceFormat(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, GMM_FORMAT_INVALID);
    return pGmmResource->GetResourceFormat();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetPaddedWidth.
/// @see        GmmLib::GmmResourceInfoCommon::GetPaddedWidth()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]  MipLevel: Requested mip level
/// @return     Padded Width
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetPaddedWidth(GMM_RESOURCE_INFO *pGmmResource,
                                          uint32_t           MipLevel)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetPaddedWidth(MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetPaddedHeight.
/// @see        GmmLib::GmmResourceInfoCommon::GetPaddedHeight()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]  MipLevel: Requested mip level
/// @return     Padded Height
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetPaddedHeight(GMM_RESOURCE_INFO *pGmmResource,
                                           uint32_t           MipLevel)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetPaddedHeight(MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetPaddedPitch.
/// @see        GmmLib::GmmResourceInfoCommon::GetPaddedPitch()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]  MipLevel: Requested mip level
/// @return     Padded Pitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetPaddedPitch(GMM_RESOURCE_INFO *pGmmResource,
                                          uint32_t           MipLevel)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetPaddedPitch(MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetBaseWidth. Truncates width to
/// 32-bit.
/// @see        GmmLib::GmmResourceInfoCommon::GetBaseWidth()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Width
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetBaseWidth(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return GFX_ULONG_CAST(pGmmResource->GetBaseWidth());
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetBaseWidth.
/// @see        GmmLib::GmmResourceInfoCommon::GetBaseWidth()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Width
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetBaseWidth64(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetBaseWidth();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetBaseAlignment.
/// @see        GmmLib::GmmResourceInfoCommon::GetBaseAlignment()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Base Alignment
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetBaseAlignment(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetBaseAlignment();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetBaseHeight.
/// @see        GmmLib::GmmResourceInfoCommon::GetBaseHeight()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Height
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetBaseHeight(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetBaseHeight();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GmmResGetDepth.
/// @see        GmmLib::GmmResourceInfoCommon::GmmResGetDepth()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Depth
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetDepth(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetBaseDepth();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMaxLod.
/// @see        GmmLib::GmmResourceInfoCommon::GetMaxLod()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Max Lod
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetMaxLod(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetMaxLod();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMipTailStartLod.SurfaceState
/// @see        GmmLib::GmmResourceInfoCommon::GetMipTailStartLodSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Mip Tail Starts
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSurfaceStateMipTailStartLod(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetMipTailStartLodSurfaceState();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetTileAddressMappingMode.SurfaceState
/// @see        GmmLib::GmmResourceInfoCommon::GetTileAddressMappingModeSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Tile Address Mapping Mode
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSurfaceStateTileAddressMappingMode(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetTileAddressMappingModeSurfaceState();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetStdTilingModeExt.SurfaceState
/// @see        GmmLib::GmmResourceInfoCommon::GetStdTilingModeExtSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Std Tiling Mode Ext
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSurfaceStateStdTilingModeExt(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, GMM_INVALIDPARAM);
    return pGmmResource->GetStdTilingModeExtSurfaceState();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetArraySize.
/// @see        GmmLib::GmmResourceInfoCommon::GetArraySize()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Array Size
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetArraySize(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetArraySize();
}

#if(LHDM)
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetRefreshRate.
/// @see        GmmLib::GmmResourceInfoCommon::GetRefreshRate()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Refresh rate
/////////////////////////////////////////////////////////////////////////////////////
D3DDDI_RATIONAL GMM_STDCALL GmmResGetRefreshRate(GMM_RESOURCE_INFO *pGmmResource)
{
    D3DDDI_RATIONAL RetVal = {0};
    __GMM_ASSERTPTR(pGmmResource, RetVal);
    return pGmmResource->GetRefreshRate();
}
#endif // LHDM

#if(LHDM)
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoWin::GetD3d9Flags.
/// @see        GmmLib::GmmResourceInfoWin::GetD3d9Flags()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[out] pD3d9Flags: Mscaps data is copied to this param
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResGetD3d9Flags(GMM_RESOURCE_INFO *   pGmmResource,
                                    D3DDDI_RESOURCEFLAGS *pD3d9Flags)
{
    __GMM_ASSERTPTR(pGmmResource, VOIDRETURN);
    __GMM_ASSERTPTR(pD3d9Flags, VOIDRETURN);

    *pD3d9Flags = pGmmResource->GetD3d9Flags();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoWin::GetD3d9Format.
/// @see        GmmLib::GmmResourceInfoWin::GetD3d9Format()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     D3d9 format for the resource
/////////////////////////////////////////////////////////////////////////////////////
D3DDDIFORMAT GMM_STDCALL GmmResGetD3d9Format(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, (D3DDDIFORMAT)0);
    return pGmmResource->GetD3d9Format();
}


/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoWin::GetVidSourceId.
/// @see        GmmLib::GmmResourceInfoWin::GetVidSourceId()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Source Id
/////////////////////////////////////////////////////////////////////////////////////
D3DDDI_VIDEO_PRESENT_SOURCE_ID GMM_STDCALL GmmResGetVidSourceId(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetVidSourceId();
}

#endif // LHDM

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoWin::Is64KBPageSuitable.
/// @see        GmmLib::GmmResourceInfoWin::Is64KBPageSuitable()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     1/0
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIs64KBPageSuitable(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->Is64KBPageSuitable();
}



/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetRotateInfo.
/// @see        GmmLib::GmmResourceInfoCommon::GetRotateInfo()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     rotation info
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetRotateInfo(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetRotateInfo();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetAuxQPitch.
/// @see        GmmLib::GmmResourceInfoCommon::GetAuxQPitch()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Aux QPitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetAuxQPitch(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetAuxQPitch();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetQPitch.
/// @see        GmmLib::GmmResourceInfoCommon::GetQPitch()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     QPitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetQPitch(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetQPitch();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetQPitchPlanar.
/// @see        GmmLib::GmmResourceInfoCommon::GetQPitchPlanar()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Planar QPitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetQPitchPlanar(GMM_RESOURCE_INFO *pGmmResource, GMM_YUV_PLANE Plane)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetQPitchPlanar(Plane);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetQPitchInBytes.
/// @see        GmmLib::GmmResourceInfoCommon::GetQPitchInBytes()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     QPitch
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetQPitchInBytes(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetQPitchInBytes();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetRenderPitch.
/// @see        GmmLib::GmmResourceInfoCommon::GetRenderPitch()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Pitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetRenderPitch(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return GFX_ULONG_CAST(pGmmResource->GetRenderPitch());
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetRenderPitchTiles.
/// @see        GmmLib::GmmResourceInfoCommon::GetRenderPitchTiles()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Pitch in tiles
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetRenderPitchTiles(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetRenderPitchTiles();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetRenderAuxPitchTiles.
/// @see        GmmLib::GmmResourceInfoCommon::GetRenderAuxPitchTiles()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Aux Pitch in tiles
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetRenderAuxPitchTiles(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetRenderAuxPitchTiles();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetUnifiedAuxPitch.
/// @see        GmmLib::GmmResourceInfoCommon::GetUnifiedAuxPitch()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Aux Pitch
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetAuxPitch(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return GFX_ULONG_CAST(pGmmResource->GetUnifiedAuxPitch());
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetBitsPerPixel.
/// @see        GmmLib::GmmResourceInfoCommon::GetBitsPerPixel()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     bpp
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetBitsPerPixel(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetBitsPerPixel();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetUnifiedAuxBitsPerPixel.
/// @see        GmmLib::GmmResourceInfoCommon::GetUnifiedAuxBitsPerPixel()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     bpp
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetAuxBitsPerPixel(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetUnifiedAuxBitsPerPixel();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetCompressionBlockXxx.
/// @see        GmmLib::GmmResourceInfoCommon::GetCompressionBlockWidth()
/// @see        GmmLib::GmmResourceInfoCommon::GetCompressionBlockHeight()
/// @see        GmmLib::GmmResourceInfoCommon::GetCompressionBlockDepth()
///
/// @param[in]  pRes: Pointer to the GmmResourceInfo class
/// @return     Compression Block Width/Height/Depth
/////////////////////////////////////////////////////////////////////////////////////
#define GmmResGetCompressionBlockXxx(Xxx)                                                \
    uint32_t GMM_STDCALL GmmResGetCompressionBlock##Xxx(GMM_RESOURCE_INFO *pGmmResource) \
    {                                                                                    \
        __GMM_ASSERTPTR(pGmmResource, 1);                                                \
        return pGmmResource->GetCompressionBlock##Xxx();                                 \
    } ///////////////////////////////////////////////////////
GmmResGetCompressionBlockXxx(Width)
GmmResGetCompressionBlockXxx(Height)
GmmResGetCompressionBlockXxx(Depth)

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetUnifiedAuxBitsPerPixel.
/// @see        GmmLib::GmmResourceInfoCommon::GetUnifiedAuxBitsPerPixel()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in][out] pReqInfo: Has info about which offset client is requesting. Offset is also
///                 passed back to the client in this parameter.
/// @return         ::GMM_STATUS
/////////////////////////////////////////////////////////////////////////////////////
GMM_STATUS GMM_STDCALL GmmResGetOffset(GMM_RESOURCE_INFO *  pGmmResource,
                                       GMM_REQ_OFFSET_INFO *pReqInfo)
{
    __GMM_ASSERTPTR(pGmmResource, GMM_ERROR);
    __GMM_ASSERTPTR(pReqInfo, GMM_ERROR);

    return pGmmResource->GetOffset(*pReqInfo);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetTextureLayout.
/// @see        GmmLib::GmmResourceInfoCommon::GetTextureLayout()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @return         GMM_2D_LAYOUT_RIGHT or GMM_2D_LAYOUT_BELOW
/////////////////////////////////////////////////////////////////////////////////////
GMM_TEXTURE_LAYOUT GMM_STDCALL GmmResGetTextureLayout(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetTextureLayout();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetTileType.
/// @see        GmmLib::GmmResourceInfoCommon::GetTileType()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @return         ::GMM_TILE_TYPE
/////////////////////////////////////////////////////////////////////////////////////
GMM_TILE_TYPE GMM_STDCALL GmmResGetTileType(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetTileType();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMipHeight.
/// @see        GmmLib::GmmResourceInfoCommon::GetMipHeight()
///
/// @param[in]      pResourceInfo: Pointer to the GmmResourceInfo class
/// @param[in]      MipLevel: Mip level for which the info is needed
/// @return         Mip Height
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetMipHeight(GMM_RESOURCE_INFO *pResourceInfo, uint32_t MipLevel)
{
    return pResourceInfo->GetMipHeight(MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMipWidth.
/// @see        GmmLib::GmmResourceInfoCommon::GetMipWidth()
///
/// @param[in]      pResourceInfo: Pointer to the GmmResourceInfo class
/// @param[in]      MipLevel: Mip level for which the info is needed
/// @return         Mip Width
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetMipWidth(GMM_RESOURCE_INFO *pResourceInfo, uint32_t MipLevel)
{
    return pResourceInfo->GetMipWidth(MipLevel);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMipDepth.
/// @see        GmmLib::GmmResourceInfoCommon::GetMipDepth()
///
/// @param[in]      pResourceInfo: Pointer to the GmmResourceInfo class
/// @param[in]      MipLevel: Mip level for which the info is needed
/// @return         Mip Depth
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetMipDepth(GMM_RESOURCE_INFO *pResourceInfo, uint32_t MipLevel)
{
    return pResourceInfo->GetMipDepth(MipLevel);
}

//=============================================================================
//
// Function:GmmResGetCornerTexelMode
//
// Desc:
//      Simple getter function to return the Corner Texel Mode of a surface.
//
// Parameters:
//      pGmmResource: ==> A previously allocated resource.
//
// Returns:
//      CornerTexelMode flag ==> uint8_t
//-----------------------------------------------------------------------------
uint8_t GMM_STDCALL GmmResGetCornerTexelMode(GMM_RESOURCE_INFO *pGmmResource)
{
    GMM_DPF_ENTER;
    __GMM_ASSERT(pGmmResource);

    return ((pGmmResource->GetResFlags().Info.CornerTexelMode) ? 1 : 0);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetCpuCacheType.
/// @see        GmmLib::GmmResourceInfoCommon::GetCpuCacheType()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @return         ::GMM_CPU_CACHE_TYPE
/////////////////////////////////////////////////////////////////////////////////////
GMM_CPU_CACHE_TYPE GMM_STDCALL GmmResGetCpuCacheType(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetCpuCacheType();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMmcMode.
/// @see        GmmLib::GmmResourceInfoCommon::GetMmcMode()
///
/// @param[in]      pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]      ArrayIndex: ArrayIndex for which this info is needed
/// @return         Media Memory Compression Mode (Disabled, Horizontal, Vertical)
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_MMC_INFO GMM_STDCALL GmmResGetMmcMode(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetMmcMode(ArrayIndex);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::SetMmcMode.
/// @see        GmmLib::GmmResourceInfoCommon::SetMmcMode()
///
/// @param[in] pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in] Mode Media Memory Compression Mode (Disabled, Horizontal, Vertical)
/// @param[in] ArrayIndex ArrayIndex for which this info needs to be set
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResSetMmcMode(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_MMC_INFO Mode, uint32_t ArrayIndex)
{
    __GMM_ASSERTPTR(pGmmResource, VOIDRETURN);
    pGmmResource->SetMmcMode(Mode, ArrayIndex);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::IsMediaMemoryCompressed.
/// @see        GmmLib::GmmResourceInfoCommon::IsMediaMemoryCompressed()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]  ArrayIndex ArrayIndex for which this info is needed
/// @return     1 (enabled), 0 (disabled)
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIsMediaMemoryCompressed(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex)
{
    return pGmmResource->IsMediaMemoryCompressed(ArrayIndex);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetMmcHint.
/// @see        GmmLib::GmmResourceInfoCommon::GetMmcHint()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]  ArrayIndex ArrayIndex for which this info is needed
/// @return     true/false
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_MMC_HINT GMM_STDCALL GmmResGetMmcHint(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetMmcHint(ArrayIndex);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::SetMmcHint.
/// @see        GmmLib::GmmResourceInfoCommon::SetMmcHint()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @param[in]  Hint Mmc hint to store
/// @param[in]  ArrayIndex ArrayIndex for which this info is needed
/// @return     true/false
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResSetMmcHint(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_MMC_HINT Hint, uint32_t ArrayIndex)
{
    __GMM_ASSERTPTR(pGmmResource, VOIDRETURN);
    pGmmResource->SetMmcHint(Hint, ArrayIndex);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetNumSamples.
/// @see        GmmLib::GmmResourceInfoCommon::GetNumSamples()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Sample count
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetNumSamples(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetNumSamples();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetSamplePattern.
/// @see        GmmLib::GmmResourceInfoCommon::GetSamplePattern()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     Sample count
/////////////////////////////////////////////////////////////////////////////////////
GMM_MSAA_SAMPLE_PATTERN GMM_STDCALL GmmResGetSamplePattern(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetSamplePattern();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetHAlign.
/// @see        GmmLib::GmmResourceInfoCommon::GetHAlign()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     HAlign
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetHAlign(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetHAlign();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetVAlign.
/// @see        GmmLib::GmmResourceInfoCommon::GetVAlign()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     VAlign
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetVAlign(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetVAlign();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetAuxHAlign.
/// @see        GmmLib::GmmResourceInfoCommon::GetAuxHAlign()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     HAlign
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetAuxHAlign(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetAuxHAlign();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::GetAuxVAlign.
/// @see        GmmLib::GmmResourceInfoCommon::GetAuxVAlign()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     VAlign
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetAuxVAlign(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetAuxVAlign();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::IsArraySpacingSingleLod.
/// @see        GmmLib::GmmResourceInfoCommon::IsArraySpacingSingleLod()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     1/0
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIsArraySpacingSingleLod(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->IsArraySpacingSingleLod();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::IsASTC.
/// @see        GmmLib::GmmResourceInfoCommon::IsASTC()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     1/0
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIsASTC(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->IsASTC();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::IsMsaaFormatDepthStencil.
/// @see        GmmLib::GmmResourceInfoCommon::IsMsaaFormatDepthStencil()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     1/0
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIsMsaaFormatDepthStencil(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->IsMsaaFormatDepthStencil();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmLib::GmmResourceInfoCommon::IsSvm.
/// @see        GmmLib::GmmResourceInfoCommon::IsSvm()
///
/// @param[in]  pGmmResource: Pointer to the GmmResourceInfo class
/// @return     1/0
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIsSvm(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->IsSvm();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::ValidateParams
/// @see    GmmLib::GmmResourceInfoCommon::ValidateParams()
///
/// @param[in]  pResourceInfo: Pointer to GmmResourceInfo class
/// @return     1 is validation passed. 0 otherwise.
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResValidateParams(GMM_RESOURCE_INFO *pResourceInfo)
{
    return pResourceInfo->ValidateParams();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::SetPrivateData
/// @see    GmmLib::GmmResourceInfoCommon::SetPrivateData()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  pPrivateData: Pointer to opaque private data from clients
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResSetPrivateData(GMM_RESOURCE_INFO *pGmmResource, void *pPrivateData)
{
    __GMM_ASSERTPTR(pGmmResource, VOIDRETURN);
    pGmmResource->SetPrivateData(pPrivateData);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetPrivateData
/// @see    GmmLib::GmmResourceInfoCommon::GetPrivateData()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     pointer to opaque private data
/////////////////////////////////////////////////////////////////////////////////////
void *GMM_STDCALL GmmResGetPrivateData(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetPrivateData();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetGfxAddress
/// @see    GmmLib::GmmResourceInfoCommon::GetGfxAddress()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     Gfx address
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_ADDRESS GMM_STDCALL GmmResGetGfxAddress(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetGfxAddress();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetTallBufferHeight
/// @see    GmmLib::GmmResourceInfoCommon::GetTallBufferHeight()
///
/// @param[in]  pResourceInfo: Pointer to GmmResourceInfo class
/// @return     Surface height
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetTallBufferHeight(GMM_RESOURCE_INFO *pResourceInfo)
{
    __GMM_ASSERTPTR(pResourceInfo, 0);
    return pResourceInfo->GetTallBufferHeight();
};


/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetSizeMainSurface
/// @see    GmmLib::GmmResourceInfoCommon::GetSizeMainSurface()
///
/// @param[in]  pResourceInfo: Pointer to GmmResourceInfo class
/// @return     Size of main surface
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetSizeMainSurface(const GMM_RESOURCE_INFO *pResourceInfo)
{
    return pResourceInfo->GetSizeMainSurface();
}

//TODO(Low) : Remove when client moves to new interface
/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetSizeSurface
/// @see    GmmLib::GmmResourceInfoCommon::GetSizeSurface()
///
/// @param[in]  pResourceInfo: Pointer to GmmResourceInfo class
/// @return     Surface Size
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetRenderSize(GMM_RESOURCE_INFO *pResourceInfo)
{
    __GMM_ASSERTPTR(pResourceInfo, 0);
    return GFX_ULONG_CAST(pResourceInfo->GetSizeSurface());
}

uint32_t GMM_STDCALL GmmResGetAuxSurfaceSize(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType)
{
    return GFX_ULONG_CAST(GmmResGetSizeAuxSurface(pGmmResource, GmmAuxType));
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetSizeSurface
/// @see    GmmLib::GmmResourceInfoCommon::GetSizeSurface()
///
/// @param[in]  pResourceInfo: Pointer to GmmResourceInfo class
/// @return     Surface Size
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetSizeSurface(GMM_RESOURCE_INFO *pResourceInfo)
{
    __GMM_ASSERTPTR(pResourceInfo, 0);
    return pResourceInfo->GetSizeSurface();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetSizeAllocation
/// @see    GmmLib::GmmResourceInfoCommon::GetSizeAllocation()
///
/// @param[in]  pResourceInfo: Pointer to GmmResourceInfo class
/// @return     Allocation Size
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetSizeAllocation(GMM_RESOURCE_INFO *pResourceInfo)
{
    __GMM_ASSERTPTR(pResourceInfo, 0);
    return pResourceInfo->GetSizeAllocation();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetResourceFormatSurfaceState
/// @see    GmmLib::GmmResourceInfoCommon::GetResourceFormatSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     Resource format
/////////////////////////////////////////////////////////////////////////////////////
GMM_SURFACESTATE_FORMAT GMM_STDCALL GmmResGetSurfaceStateFormat(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, GMM_SURFACESTATE_FORMAT_INVALID);
    return pGmmResource->GetResourceFormatSurfaceState();
}

//=============================================================================
//
// Function: GmmGetSurfaceStateFormat
//
// Desc: See below.
//
// Returns:
//      SURFACE_STATE.Format for the given resource or
//      GMM_SURFACESTATE_FORMAT_INVALID if resource wasn't created with a
//      direct SURFACE_STATE.Format.
//
//-----------------------------------------------------------------------------
GMM_SURFACESTATE_FORMAT GMM_STDCALL GmmGetSurfaceStateFormat(GMM_RESOURCE_FORMAT Format)
{
    return ((Format > GMM_FORMAT_INVALID) &&
            (Format < GMM_RESOURCE_FORMATS)) ?
           pGmmGlobalContext->GetPlatformInfo().FormatTable[Format].SurfaceStateFormat :
           GMM_SURFACESTATE_FORMAT_INVALID;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetHAlignSurfaceState
/// @see    GmmLib::GmmResourceInfoCommon::GetHAlignSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     HAlign
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSurfaceStateHAlign(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetHAlignSurfaceState();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetVAlignSurfaceState
/// @see    GmmLib::GmmResourceInfoCommon::GetVAlignSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     VAlign
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSurfaceStateVAlign(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetVAlignSurfaceState();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetTiledResourceModeSurfaceState
/// @see    GmmLib::GmmResourceInfoCommon::GetTiledResourceModeSurfaceState()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     Tiled Resource Mode
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetSurfaceStateTiledResourceMode(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetTiledResourceModeSurfaceState();
}

/////////////////////////////////////////////////////////////////////////////////////
/// Returns the surface offset for unified allocations. Truncates the offset to size
/// of uint32_t.
/// @see    GmmResGetAuxSurfaceOffset64()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  GmmAuxType: the type of aux the offset is needed for
/// @return     Surface Offset in bytes
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetAuxSurfaceOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType)
{
    return GFX_ULONG_CAST(GmmResGetAuxSurfaceOffset64(pGmmResource, GmmAuxType));
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetUnifiedAuxSurfaceOffset
/// @see    GmmLib::GmmResourceInfoCommon::GetUnifiedAuxSurfaceOffset()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  GmmAuxType: the type of aux the offset is needed for
/// @return     Surface Offset in bytes
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetAuxSurfaceOffset64(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetUnifiedAuxSurfaceOffset(GmmAuxType);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetSizeAuxSurface
/// @see    GmmLib::GmmResourceInfoCommon::GetSizeAuxSurface()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  GmmAuxType: the type of aux the size is needed for
/// @return     Surface size in bytes
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetSizeAuxSurface(GMM_RESOURCE_INFO *pGmmResource, GMM_UNIFIED_AUX_TYPE GmmAuxType)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetSizeAuxSurface(GmmAuxType);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetSetHardwareProtection
/// @see    GmmLib::GmmResourceInfoCommon::GetSetHardwareProtection()
/// @see    GmmLib::GmmResourceInfoWin::GetSetHardwareProtection()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  GetIsEncrypted: Read encryption status
/// @param[in]  SetIsEncrypted: Write encryption status
/// @return     Whether surface is encrypted or not

/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResGetSetHardwareProtection(GMM_RESOURCE_INFO *pGmmResource, uint8_t GetIsEncrypted, uint8_t SetIsEncrypted)
{
    return pGmmResource ?
           pGmmResource->GetSetHardwareProtection(GetIsEncrypted, SetIsEncrypted) :
           0;
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::CpuBlt
/// @see    GmmLib::GmmResourceInfoCommon::CpuBlt()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  pBlt: Describes the blit operation. See ::GMM_RES_COPY_BLT for more info.
/// @return     1 if succeeded, 0 otherwise
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResCpuBlt(GMM_RESOURCE_INFO *pGmmResource, GMM_RES_COPY_BLT *pBlt)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->CpuBlt(pBlt);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetStdLayoutSize
/// @see    GmmLib::GmmResourceInfoCommon::GetStdLayoutSize()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     Size in bytes of Standard Layout version of surface.
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetStdLayoutSize(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetStdLayoutSize();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetMappingSpanDesc
/// @see    GmmLib::GmmResourceInfoCommon::GetMappingSpanDesc()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  pMapping: Clients call the function with initially zero'd out GMM_GET_MAPPING.
/// @return      1 if more span descriptors to report, 0 if all mapping is done
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResGetMappingSpanDesc(GMM_RESOURCE_INFO *pGmmResource, GMM_GET_MAPPING *pMapping)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetMappingSpanDesc(pMapping);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::IsColorSeparation
/// @see    GmmLib::GmmResourceInfoCommon::IsColorSeparation()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     1 if the resource is color separated target, 0 otherwise
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmResIsColorSeparation(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->IsColorSeparation();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::TranslateColorSeparationX
/// @see    GmmLib::GmmResourceInfoCommon::TranslateColorSeparationX()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  x: X coordinate
/// @return   Translated color separation target x coordinate
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResTranslateColorSeparationX(GMM_RESOURCE_INFO *pGmmResource, uint32_t x)
{
    __GMM_ASSERTPTR(pGmmResource, false);
    return pGmmResource->TranslateColorSeparationX(x);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetColorSeparationArraySize
/// @see    GmmLib::GmmResourceInfoCommon::GetColorSeparationArraySize()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return   Array size of a color separated target resource
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetColorSeparationArraySize(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetColorSeparationArraySize();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetColorSeparationPhysicalWidth
/// @see    GmmLib::GmmResourceInfoCommon::GetColorSeparationPhysicalWidth()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     Physical width of a color separated target resource
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetColorSeparationPhysicalWidth(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->GetColorSeparationPhysicalWidth();
}

//=============================================================================
//
// Function: GmmResGetMaxGpuVirtualAddressBits
//
// Desc: This function returns max no of GpuVA bits supported per surface on current platform
//
// Parameters:
//      GMM_RESOURCE_INFO *pGmmResource (Don't care)
//          Function will return the current Gen MaxGpuVirtualAddressBitsPerResource
//
// Returns:
//      uint32_t - Max no of GpuVA bits
//-----------------------------------------------------------------------------
uint32_t GMM_STDCALL GmmResGetMaxGpuVirtualAddressBits(GMM_RESOURCE_INFO *pGmmResource)
{
    if(pGmmResource == NULL)
    {
        __GMM_ASSERTPTR(pGmmGlobalContext, 0);
        const GMM_PLATFORM_INFO &PlatformInfo = pGmmGlobalContext->GetPlatformInfo();
        return PlatformInfo.MaxGpuVirtualAddressBitsPerResource;
    }

    return pGmmResource->GetMaxGpuVirtualAddressBits();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::IsSurfaceFaultable
/// @see    GmmLib::GmmResourceInfoCommon::IsSurfaceFaultable()
/// @see    GmmLib::GmmResourceInfoWin::IsSurfaceFaultable()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return   1 is surface can be faulted on
/////////////////////////////////////////////////////////////////////////////////////
uint8_t GMM_STDCALL GmmIsSurfaceFaultable(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERTPTR(pGmmResource, 0);
    return pGmmResource->IsSurfaceFaultable();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetResFlags
/// @see    GmmLib::GmmResourceInfoCommon::GetResFlags()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return     Copy of ::GMM_RESOURCE_FLAGS
/////////////////////////////////////////////////////////////////////////////////////
GMM_RESOURCE_FLAG GMM_STDCALL GmmResGetResourceFlags(const GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return const_cast<GMM_RESOURCE_INFO *>(pGmmResource)->GetResFlags();
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetMaximumRenamingListLength
/// @see    GmmLib::GmmResourceInfoCommon::GetMaximumRenamingListLength()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @return    maximum remaining list length
/////////////////////////////////////////////////////////////////////////////////////
uint32_t GMM_STDCALL GmmResGetMaximumRenamingListLength(GMM_RESOURCE_INFO *pGmmResource)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetMaximumRenamingListLength();
}

//=============================================================================
//
// Function: GmmGetLogicalTileShape
//
// Desc: This function returns the logical tile shape
//
// Parameters:
//      See Function arguments
//
// Returns:
//      GMM_STATUS
//-----------------------------------------------------------------------------
GMM_STATUS GMM_STDCALL GmmGetLogicalTileShape(uint32_t  TileMode,
                                              uint32_t *pWidthInBytes,
                                              uint32_t *pHeight,
                                              uint32_t *pDepth)
{
    __GMM_ASSERT(TileMode < GMM_TILE_MODES);

    if(pWidthInBytes)
    {
        *pWidthInBytes = pGmmGlobalContext->GetPlatformInfo().TileInfo[TileMode].LogicalTileWidth;
    }

    if(pHeight)
    {
        *pHeight = pGmmGlobalContext->GetPlatformInfo().TileInfo[TileMode].LogicalTileHeight;
    }

    if(pDepth)
    {
        *pDepth = pGmmGlobalContext->GetPlatformInfo().TileInfo[TileMode].LogicalTileDepth;
    }

    return GMM_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideSize
/// @see    GmmLib::GmmResourceInfoCommon::OverrideSize()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Size: new size of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationSize(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T Size)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideSize(Size);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverridePitch
/// @see    GmmLib::GmmResourceInfoCommon::OverridePitch()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Pitch: new pitch of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationPitch(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T Pitch)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverridePitch(Pitch);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideUnifiedAuxPitch
/// @see    GmmLib::GmmResourceInfoCommon::OverrideUnifiedAuxPitch()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Pitch: new pitch of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAuxAllocationPitch(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T Pitch)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideUnifiedAuxPitch(Pitch);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideUnifiedAuxPitch
/// @see    GmmLib::GmmResourceInfoCommon::OverrideUnifiedAuxPitch()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Pitch: new pitch of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationFlags(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_FLAG *pFlags)
{
    __GMM_ASSERT(pGmmResource);
    __GMM_ASSERT(pFlags);
    pGmmResource->OverrideAllocationFlags(*pFlags);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideHAlign
/// @see    GmmLib::GmmResourceInfoCommon::OverrideHAlign()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Pitch: new pitch of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationHAlign(GMM_RESOURCE_INFO *pGmmResource, uint32_t HAlign)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideHAlign(HAlign);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideBaseAlignment
/// @see    GmmLib::GmmResourceInfoCommon::OverrideBaseAlignment()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Alignment: new BaseAlignment of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationBaseAlignment(GMM_RESOURCE_INFO *pGmmResource, uint32_t Alignment)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideBaseAlignment(Alignment);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideBaseWidth
/// @see    GmmLib::GmmResourceInfoCommon::OverrideBaseWidth()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  BaseWidth: new BaseWidth of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationBaseWidth(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_SIZE_T BaseWidth)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideBaseWidth(BaseWidth);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideBaseHeight
/// @see    GmmLib::GmmResourceInfoCommon::OverrideBaseHeight()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  BaseHeight: new BaseWidth of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationBaseHeight(GMM_RESOURCE_INFO *pGmmResource, uint32_t BaseHeight)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideBaseHeight(BaseHeight);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideDepth
/// @see    GmmLib::GmmResourceInfoCommon::OverrideDepth()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Depth: new Depth of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationDepth(GMM_RESOURCE_INFO *pGmmResource, uint32_t Depth)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideDepth(Depth);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideTileMode
/// @see    GmmLib::GmmResourceInfoCommon::OverrideTileMode()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  TileMode: new tile mode of the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideResourceTiling(GMM_RESOURCE_INFO *pGmmResource, uint32_t TileMode)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideTileMode(static_cast<GMM_TILE_MODE>(TileMode));
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideUnifiedAuxTileMode
/// @see    GmmLib::GmmResourceInfoCommon::OverrideUnifiedAuxTileMode()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  TileMode: new tile mode of the unified auxresource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAuxResourceTiling(GMM_RESOURCE_INFO *pGmmResource, uint32_t TileMode)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideUnifiedAuxTileMode(static_cast<GMM_TILE_MODE>(TileMode));
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideSurfaceFormat
/// @see    GmmLib::GmmResourceInfoCommon::OverrideSurfaceFormat()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Format: new format for the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationFormat(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_FORMAT Format)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideSurfaceFormat(Format);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideSurfaceType
/// @see    GmmLib::GmmResourceInfoCommon::OverrideSurfaceType()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  ResourceType: new type for the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideSurfaceType(GMM_RESOURCE_INFO *pGmmResource, GMM_RESOURCE_TYPE ResourceType)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideSurfaceType(ResourceType);
}


/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GmmResOverrideSvmGfxAddress
/// @see    GmmLib::GmmResourceInfoCommon::GmmResOverrideSvmGfxAddress()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  SvmGfxAddress: new svm gfx address for the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideSvmGfxAddress(GMM_RESOURCE_INFO *pGmmResource, GMM_GFX_ADDRESS SvmGfxAddress)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideSvmGfxAddress(SvmGfxAddress);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideArraySize
/// @see    GmmLib::GmmResourceInfoCommon::OverrideArraySize()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  ArraySize: new array size for the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationArraySize(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArraySize)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideArraySize(ArraySize);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::OverrideArraySize
/// @see    GmmLib::GmmResourceInfoCommon::OverrideArraySize()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  MaxLod: new max LOD for the resource
/////////////////////////////////////////////////////////////////////////////////////
void GMM_STDCALL GmmResOverrideAllocationMaxLod(GMM_RESOURCE_INFO *pGmmResource, uint32_t MaxLod)
{
    __GMM_ASSERT(pGmmResource);
    pGmmResource->OverrideMaxLod(MaxLod);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetPlanarXOffset
/// @see    GmmLib::GmmResourceInfoCommon::GetPlanarXOffset()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Plane: Plane for which the offset is needed
/// @return     X offset
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetPlanarGetXOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_YUV_PLANE Plane)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetPlanarXOffset(Plane);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetPlanarYOffset
/// @see    GmmLib::GmmResourceInfoCommon::GetPlanarYOffset()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Plane: Plane for which the offset is needed
/// @return     Y offset
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetPlanarGetYOffset(GMM_RESOURCE_INFO *pGmmResource, GMM_YUV_PLANE Plane)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetPlanarYOffset(Plane);
}

/////////////////////////////////////////////////////////////////////////////////////
/// C wrapper for GmmResourceInfoCommon::GetPlanarAuxOffset
/// @see    GmmLib::GmmResourceInfoCommon::GetPlanarAuxOffset()
///
/// @param[in]  pGmmResource: Pointer to GmmResourceInfo class
/// @param[in]  Plane: Plane for which the offset is needed
/// @return     Y offset
/////////////////////////////////////////////////////////////////////////////////////
GMM_GFX_SIZE_T GMM_STDCALL GmmResGetPlanarAuxOffset(GMM_RESOURCE_INFO *pGmmResource, uint32_t ArrayIndex, GMM_UNIFIED_AUX_TYPE AuxType)
{
    __GMM_ASSERT(pGmmResource);
    return pGmmResource->GetPlanarAuxOffset(ArrayIndex, AuxType);
}

//=============================================================================
//
// Function: __CanSupportStdTiling
//
// Desc: Verifies texture parameters can support StdTiling
//
// Parameters:
//      GMM_TEXTURE_INFO&   Surf
//
// Returns:
//
//-----------------------------------------------------------------------------
uint8_t __CanSupportStdTiling(GMM_TEXTURE_INFO Surf)
{
    const __GMM_PLATFORM_RESOURCE *pPlatformResource = GMM_OVERRIDE_PLATFORM_INFO(&Surf);

    // SKL+ Tiled Resource Mode Restrictions
    if((Surf.Flags.Info.TiledYf || Surf.Flags.Info.TiledYs) &&
       !((GFX_GET_CURRENT_RENDERCORE(pPlatformResource->Platform) >= IGFX_GEN9_CORE) &&
         // TiledY must be set unless 1D surface.
         ((Surf.Flags.Info.TiledY && (Surf.Type != RESOURCE_1D)) ||
          (Surf.Flags.Info.Linear && (Surf.Type == RESOURCE_1D ||
                                      Surf.Type == RESOURCE_BUFFER))) &&
         // 8, 16, 32, 64, or 128 bpp
         ((!GmmIsCompressed(Surf.Format) &&
           ((Surf.BitsPerPixel == 8) ||
            (Surf.BitsPerPixel == 16) ||
            (Surf.BitsPerPixel == 32) ||
            (Surf.BitsPerPixel == 64) ||
            (Surf.BitsPerPixel == 128))) ||
          // Compressed Modes: BC*, ETC*, EAC*, ASTC*
          (GmmIsCompressed(Surf.Format) && (Surf.Format != GMM_FORMAT_FXT1)))))
    /* Not currently supported...
        // YCRCB* Formats
        GmmIsYUVPacked(Surf.Format) */
    {
        return 0;
    }

    return 1;
}
