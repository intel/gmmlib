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
#include "Internal/Common/Platform/GmmGen10Platform.h"
#include "Internal/Common/Platform/GmmGen11Platform.h"

GmmLib::PlatformInfoGen11::PlatformInfoGen11(PLATFORM &Platform)
    : PlatformInfoGen10(Platform)
{

    Data.SurfaceMaxSize                      = GMM_GBYTE(16384);
    Data.MaxGpuVirtualAddressBitsPerResource = 44;

    //Override CCS MaxPitch requirement
    if(GFX_GET_CURRENT_PRODUCT(Data.Platform) == IGFX_ICELAKE)
    {
        Data.TexAlign.CCS.MaxPitchinTiles = 1024;
    }

}