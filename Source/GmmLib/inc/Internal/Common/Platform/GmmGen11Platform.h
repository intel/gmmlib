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
#include "GmmGen10Platform.h"

#ifdef __cplusplus
namespace GmmLib
{
    class NON_PAGED_SECTION PlatformInfoGen11 : public PlatformInfoGen10
    {
    public:
        PlatformInfoGen11(PLATFORM &Platform, Context *pGmmLibContext);
	~PlatformInfoGen11() {};
        virtual uint8_t ValidateMMC(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t ValidateUnifiedAuxSurface(GMM_TEXTURE_INFO &Surf);
        virtual uint8_t CheckFmtDisplayDecompressible(GMM_TEXTURE_INFO &Surf,
                                                      bool IsSupportedRGB64_16_16_16_16,
                                                      bool IsSupportedRGB32_8_8_8_8,
                                                      bool IsSupportedRGB32_2_10_10_10,
                                                      bool IsSupportedMediaFormats);
    };
}

#endif
