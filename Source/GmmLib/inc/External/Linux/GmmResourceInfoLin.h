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
#include "../Common/GmmResourceInfoCommon.h"

/////////////////////////////////////////////////////////////////////////////////////
/// @file GmmResourceInfoLin.h
/// @brief This file contains the functions and members of GmmResourceInfo that is
///        Linux specific.
///
/////////////////////////////////////////////////////////////////////////////////////
namespace GmmLib
{
    class GMM_LIB_API NON_PAGED_SECTION GmmResourceInfoLin:
                                public GmmResourceInfoCommon
    {
        public:
            /* Constructors */
            GmmResourceInfoLin()
            {
            }

#ifndef __GMM_KMD__
        GmmResourceInfoLin(GmmClientContext  *pClientContextIn) : GmmResourceInfoCommon(pClientContextIn)
        {
        }
#endif

            virtual ~GmmResourceInfoLin()
            {
            }

    };

    typedef GmmResourceInfoLin GmmResourceInfo;

} // namespace GmmLib
#else
    typedef struct GmmResourceInfo GmmResourceInfo;
#endif // #ifdef __cplusplus
