//=================================================================================/
//Copyright (c) 2017 Intel Corporation                                             /
//                                                                                 /
//Permission is hereby granted, free of charge, to any person obtaining a copy     /
//of this software and associated documentation files (the "Software"), to deal    /
//in the Software without restriction, including without limitation the rights     / 
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell        /
//copies of the Software, and to permit persons to whom the Software is furnished  /
//to do so, subject to the following conditions:                                   /
//
//The above copyright notice and this permission notice shall be included in all   /
//copies or substantial portions of the Software.                                  /
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR       / 
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,         /
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE      /
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,/   
//WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN  /
//CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.       /
//---------------------------------------------------------------------------------

#ifndef _G_GFXDEBUG_H_
#define _G_GFXDEBUG_H_


//===================== Debug Message Levels========================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

#define GFXDBG_OFF                   (0x00000000)
#define GFXDBG_CRITICAL              (0x00000001)
#define GFXDBG_NORMAL                (0x00000002)
#define GFXDBG_VERBOSE               (0x00000004)
#define GFXDBG_FUNCTION              (0x80000000)
#define GFXDBG_NONCRITICAL           (0x00000010)
#define GFXDBG_CRITICAL_DEBUG        (0x00000020)
#define GFXDBG_VERBOSE_VERBOSITY     (0x00000040)
#define GFXDBG_PROTOCAL              (0x00000100)
#define GFXDBG_FUNCTION_ENTRY        (0x80000000)
#define GFXDBG_FUNCTION_EXIT         (0x80000000)
#define GFXDBG_FUNCTION_ENTRY_VERBOSE (0x20000000)
#define GFXDBG_FUNCTION_EXIT_VERBOSE (0x20000000)

//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
#define DBG_OFF                        GFXDBG_OFF
#define DBG_CRITICAL                   GFXDBG_CRITICAL
#define DBG_NORMAL                     GFXDBG_NORMAL
#define DBG_VERBOSE                    GFXDBG_VERBOSE
#define DBG_FUNCTION                   GFXDBG_FUNCTION
#define DBG_NONCRITICAL                GFXDBG_NONCRITICAL
#define DBG_CRITICAL_DEBUG             GFXDBG_CRITICAL_DEBUG
#define DBG_VERBOSE_VERBOSITY          GFXDBG_VERBOSE_VERBOSITY
#define DBG_PROTOCAL                   GFXDBG_PROTOCAL
#define DBG_FUNCTION_ENTRY             GFXDBG_FUNCTION_ENTRY
#define DBG_FUNCTION_EXIT              GFXDBG_FUNCTION_EXIT
#define DBG_FUNCTION_ENTRY_VERBOSE     GFXDBG_FUNCTION_ENTRY_VERBOSE
#define DBG_FUNCTION_EXIT_VERBOSE      GFXDBG_FUNCTION_EXIT_VERBOSE


//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
// This enum is used by DebugControlUI
// Use #define GFXDBG_<LEVEL> to specify debug level in driver code.

enum
{
    GFXDBG_ML_OFF                   =  0,
    GFXDBG_ML_CRITICAL              =  1,
    GFXDBG_ML_NORMAL                =  2,
    GFXDBG_ML_VERBOSE               =  3,
    GFXDBG_ML_FUNCTION              =  4,
    GFXDBG_ML_COUNT                 =  5,
};

//===================== Component ID's =============================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

enum
{
    GFX_COMPONENT_GMM               =  2,
	GFX_COMPONENT_COUNT             = 12,
};

//===================== Component Masks ============================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

enum
{
    GFX_GMM_MASK             = (1 << GFX_COMPONENT_GMM),
};

//================= Component Memory Allocation Tags ===============
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
//
// Tags are 4 byte ASCII character codes in the range 0-127 decimal.
// The leading @ character is to provide Intel unique tags.
// Passed in calls to ExAllocatePoolWithTag/ExFreePoolWithTag.
// Useful for pool tracking.
//

#define GFX_COMPONENT_GMM_TAG                   'MMG@'
#define GFX_DEFAULT_TAG                         'CTNI'

//======================= Component ALLOC_TAG ======================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

#ifdef __MINIPORT
#define REG_ASSERT_ENABLE_MASK                  L"z AssertEnableMask"
#define REG_DEBUG_ENABLE_MASK                   L"z DebugEnableMask"
#define REG_RINGBUF_DEBUG_MASK                  L"z RingBufDbgMask"
#define REG_REPORT_ASSERT_ENABLE                L"z ReportAssertEnable"
#define REG_ASSERT_BREAK_DISABLE                L"z AssertBreakDisable"
#define REG_GFX_COMPONENT_GMM                   L"z GMM_Debug_Lvl"

#endif

//======================= Component ALLOC_TAG ======================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

#if defined(__MINIPORT)
#define ALLOC_TAG               GFX_COMPONENT_MINIPORT_TAG
#elif defined(__SOFTBIOS)
#define ALLOC_TAG               GFX_COMPONENT_SOFTBIOS_TAG
#elif defined(__KCH)
#define ALLOC_TAG               GFX_COMPONENT_KCH_TAG
#elif defined(__AIM)
#define ALLOC_TAG               GFX_COMPONENT_AIM_TAG
#else
#define ALLOC_TAG               GFX_DEFAULT_TAG
#endif

//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
#ifdef __GFXDEBUG_C__    // Only to be defined by "gfxDebug.c" file.

    // Define the array of component ID strings. Note that the sequence
    // of these strings must match the component ID's listed above.

    char *ComponentIdStrings[] = {
        "INTC GMM: ",

    };

#endif /* __GFXDEBUG_C__ */

//================ Prototype for print routines ====================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

#ifdef __cplusplus
extern "C" {
#endif
void GMMPrintMessage(unsigned long DebugLevel, const char *DebugMessageFmt, ...);

#ifdef __cplusplus
}
#endif
//==================== Generate debug routines =====================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
//
// Debug routines either map into the base PrintMessage
// routine or they map into nothing. A code segment such as
// 'DebugPrint(args)' gets converted to '(args)' which
// is converted to nothing by the C compiler.

#if __DEBUG_MESSAGE
    #define GMMDebugMessage             GMMPrintMessage
	#define GMMDebugMessage(...)
#else
    #define GMMDebugMessage(...)
#endif /* __DEBUG_MESSSAGE */

//=================== Generate release routines ====================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
//
// Release routines either map into the base PrintMessage
// routine or they map into nothing. A code segment such as
// 'ReleasePrint(args)' gets converted to '(args)' which
// is converted to nothing by the C compiler.

#if __RELEASE_MESSAGE
    #define GMMReleaseMessage             GMMPrintMessage
#else
    #define GMMReleaseMessage(...)
#endif /* __RELEASE_MESSSAGE */

//=================== Generate ASSERT macros  ======================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

#define GMMASSERT(expr)      __ASSERT(pDebugControl, GFX_COMPONENT_GMM, GFX_GMM_MASK, expr)

//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
#define GMMASSERTPTR(expr, ret)      __ASSERTPTR(pDebugControl, GFX_COMPONENT_GMM, GFX_GMM_MASK, expr, ret)

//=========== void return value for ASSERPTR macros  ==========================
#define VOIDRETURN

//============= Generate Message printing routines =================
//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.

#ifdef __GFXDEBUG_C__    // Only to be defined by "gfxDebug.c" file.

  #ifdef __GMM
     MESSAGE_FUNCTION(GMMPrintMessage, GFX_COMPONENT_GMM);
  #endif /* __GMM */

//  WARNING!!! DO NOT MODIFY this file - see detail above copyright.
#endif /* __GFXDEBUG_C__  */

#endif //_G_GFXDEBUG_H_
