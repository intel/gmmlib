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


#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : Warning Msg: "
#define __LOC2__ __FILE__ "(" __STR1__(__LINE__) ") : "
//#pragma message(__LOC2__ "Header Message")

#ifndef _WIN32
#include <portable_compiler.h>
#endif

#if !defined(GMM_DIAG_APP_DIRECTIVE)

    #include "../../../../inc/umKmInc/sharedata.h"

    #include "../../../../inc/common/igfxfmid.h"       // Contains platform families
    #include "../../../../inc/common/gfxplatform.h"    // __GFX_MACRO_C__  must be defined before
    #include "../../../../inc/common/gfxmacro.h"       // including gfxplatform.h. See Preprocessor settings
                                                       // has MACRO ops like POW2, MASK, etc.
    #include "External/Common/GmmDebug.h"              // GMM definitions for assert and printf

#else

    #define DRIVER_VERSION_INFO uint32_t
#endif

//For compile time GFXGEN detection. GMM_GFX_GEN is optional cmd line definition
#if (!defined(GMM_GFX_GEN) || (GMM_GFX_GEN == 80))
    #define GMM_ENABLE_GEN8 1
#else
    #define GMM_ENABLE_GEN8 0
#endif

#if (!defined(GMM_GFX_GEN) || (GMM_GFX_GEN == 90))
    #define GMM_ENABLE_GEN9 1
#else
    #define GMM_ENABLE_GEN9 0
#endif

#if (!defined(GMM_GFX_GEN) || (GMM_GFX_GEN == 100))
    #define GMM_ENABLE_GEN10 1
#else
    #define GMM_ENABLE_GEN10 0
#endif

#if (!defined(GMM_GFX_GEN) || (GMM_GFX_GEN == 110))
    #define GMM_ENABLE_GEN11 1
#else
    #define GMM_ENABLE_GEN11 0
#endif

#if (!defined(GMM_GFX_GEN) || (GMM_GFX_GEN == 120))
#define GMM_ENABLE_GEN12 1
#else
#define GMM_ENABLE_GEN12 0
#endif


#if (IGFX_GEN >= IGFX_GEN11)
    #if !(GMM_ENABLE_GEN8 || GMM_ENABLE_GEN9 || GMM_ENABLE_GEN10 || \
        GMM_ENABLE_GEN11 || GMM_ENABLE_GEN12)
    #error "Unrecognized GMM_GFX_GEN !"
    #endif
#elif (IGFX_GEN >= IGFX_GEN10)
    #if !(GMM_ENABLE_GEN8 || GMM_ENABLE_GEN9 || GMM_ENABLE_GEN10)
    #error "Unrecognized GMM_GFX_GEN !"
    #endif
#else
    #if !(GMM_ENABLE_GEN8 || GMM_ENABLE_GEN9)
    #error "Unrecognized GMM_GFX_GEN !"
    #endif
#endif

