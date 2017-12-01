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

/* Usage: 
    On local builds, this header file sets the BUILD_TYPE and IS_DEVELOPER_BUILD
    will resolve to TRUE. 
    This allows compile-time and runtime detection of local builds desk builds vs.
    official non-desk builds builds with the intended use being to keep
    insecure/etc. code from production or continous integration binaries.

   Example:
    #include "BuildRequestType.h"
    ...
    #if(IS_DEVELOPER_BUILD)
        // Do something you do not want in a production or CI driver
    #else
        // The production worthy version
    #endif
*/

#define BUILD_TYPE_LOCAL 1              // Local desk build
#define BUILD_TYPE_NON_LOCAL 2          // Non-local build (production or CI)

#define BUILD_TYPE BUILD_TYPE_LOCAL     // The default value is 'local'. 

#define IS_DEVELOPER_BUILD ((BUILD_TYPE) == BUILD_TYPE_LOCAL)

#if(NOTE_BUILD_TYPE_IN_COMPILER_LOG) // A single .c/cpp file in the driver should do this. (No point in multiple files doing it.)
    #if   (BUILD_TYPE             == BUILD_TYPE_LOCAL)
        #pragma message("BUILD_TYPE: BUILD_TYPE_LOCAL")
    #elif (BUILD_TYPE             == BUILD_TYPE_NON_LOCAL)
        #pragma message("BUILD_TYPE: BUILD_TYPE_NON_LOCAL")
    #else
        #pragma message("BUILD_TYPE: Unknown")
    #endif

#endif // NOTE_BUILD_TYPE_IN_COMPILER_LOG
