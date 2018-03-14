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

#ifndef __GFXMACRO_H__
#define __GFXMACRO_H__

#include <limits.h>
#include <string.h> // for memcpy
#include <stdint.h>

// ### __GFXMACRO_ASSERT ######################################################
// Since an "always-callable" GFX_ASSERT/etc. is no longer really in-place,
// this file will define its own assert statement...
#if DBG
    #define __GFXMACRO_ASSERT(Expression)   \
    {                                       \
        if(!(Expression))                   \
        {                                   \
            __debugbreak();                 \
        }                                   \
    } // __GFXMACRO_ASSERT ##################
#else // Release Build
    #define __GFXMACRO_ASSERT(Expression)
#endif


//------------------------------------------------------------------------
// Useful constants.

#define GFX_ZERO                (0.0f)
#define GFX_HALF                (0.5f)
#define GFX_PI                  ((float) 3.14159265358979323846)
#define GFX_SQR_ROOT_OF_TWO     ((float) 1.41421356237309504880)
#define GFX_E                   ((float) 2.7182818284590452354)

//------------------------------------------------------------------------

// !!! Many of these are macros (instead of functions) so 64-bit friendly (even on 32-bit builds)!!!

#define GFX_IS_ALIGNED(A, B)       (((B) > 0) && (((A) % (B)) == 0))
#define GFX_ALIGN(x, a)          (((x) + ((a) - 1)) - (((x) + ((a) - 1)) & ((a) - 1))) // Alt implementation with bitwise not (~) has issue with uint32 align used with 64-bit value, since ~'ed value will remain 32-bit.
#define GFX_ALIGN_FLOOR(x, a)    ((x) - ((x) & ((a) - 1)))

#define GFX_ALIGN_NP2(x, a)      (((a) > 0) ? ((x) + (((a) - 1) - (((x) + ((a) - 1)) % (a)))) : (x))
#define GFX_ALIGN_FLOOR_NP2(x, a) (((a) > 0) ? ((x) - ((x) % (a))) : (x))

#define GFX_MASK(lo,hi)          ((1UL << (hi)) |    \
                                 ((1UL << (hi)) -    \
                                  (1UL << (lo))))

#define GFX_MASK_LARGE(lo,hi)    (((uint64_t)1 << (hi)) |  \
                                 (((uint64_t)1 << (hi)) -  \
                                  ((uint64_t)1 << (lo))))

#define GFX_IS_POWER_OF_2(a)     (((a) > 0) && !((a) & ((a) - 1)))

#define GFX_SWAP_VAR(a,b,t)      (t=a, a=b, b=t)
#define GFX_SWAP_VAR3(a,b,c,t)   (t=a, a=b, b=c, c=t)

#define GFX_UF_ROUND(a)          ((uint32_t) ((a) + 0.5F))
#define GFX_F_ROUND(a)           ((int32_t) ((a) + ((a) < 0 ? -0.5F : 0.5F)))
#define GFX_ABS(a)               (((a) < 0) ? -(a) : (a))

#define GFX_MIN(a,b)             (((a) < (b)) ? (a) : (b))
#define GFX_MAX(a,b)             (((a) > (b)) ? (a) : (b))

#define GFX_MIN3(a,b,c)          (((a) < (b)) ?  GFX_MIN((a), (c)) : GFX_MIN((b), (c)))
#define GFX_MAX3(a,b,c)          (((a) > (b)) ?  GFX_MAX((a), (c)) : GFX_MAX((b), (c)))

#define GFX_CEIL_DIV(a,b)        (((b) > 0) ? (((a) + ((b) - 1)) / (b)) : (a))

#define GFX_SQ(a)                ((a) * (a))

#define GFX_CLAMP_MIN_MAX(a,min,max)   ((a) < (min) ? (min) : GFX_MIN ((a), (max)))

#define GFX_KB(k)                ((k) * 1024)
#define GFX_MB(m)                ((m) * 1024 * 1024)

//------------ Macros for setting and removing bits.

#define GFX_BIT(n)               (1UL << (n))
#define GFX_BIT_ON(a,bit)        ((a) |= (bit))
#define GFX_BIT_OFF(a,bit)       ((a) &= ~(bit))
#define GFX_IS_BIT_SET(a,bit)    ((a) & (bit))

//*****************************************************************************
// MACRO: GFX_BIT_RANGE
// PURPOSE: Calculates the number of bits between the startbit and the endbit
// and count is inclusive of both bits. The bits are 0 based.
//*****************************************************************************
#define GFX_BIT_RANGE(endbit, startbit)     ((endbit)-(startbit)+1)

//------------ Macros for dealing with void pointers

#define GFX_VOID_PTR_INC(p,n)   ((void *) ((char *)(p) + (n)))
#define GFX_VOID_PTR_DEC(p,n)   ((void *) ((char *)(p) - (n)))

// While the difference of two pointers on a 64-bit machine can exceed
// 32-bits, it is mostly limited to 32-bits for the graphics driver. In
// order to avoid compilation warnings arising from assigning a 64-bit
// quantity to a 32-bit lvalue, we have two separate macros for obtaining
// pointer difference of two pointers.

#define GFX_VOID_PTR_DIFF(a,b)       (int32_t) ((char *) (a) - (char *) (b))
#define GFX_VOID_PTR_DIFF_LARGE(a,b)        ((char *) (a) - (char *) (b))

//------------ Bytes to page conversion

#define GFX_BYTES_TO_PAGES(b)   (((b) + PAGE_SIZE - 1) / PAGE_SIZE)
#define GFX_PAGES_TO_BYTES(p)   ((p) * PAGE_SIZE)

#define GFX_MEMSET(p,d,s)       (memset(p, d, s))

#if DBG
    #define GFX_ULONG_CAST(x)   \
    (                           \
        ((x) <= 0xffffffff) ?   \
            1 : __debugbreak(), \
        (uint32_t)(x)              \
    )
#else // Release Build
    #define GFX_ULONG_CAST(x) ((uint32_t)(x))
#endif

// Since hardware addresses are still 32 bits, we need a safe way
// to convert 64 bit pointers into 32 bit hardware addresses.
// ASSERT that the upper 32 bits are 0 before truncating address.
#define GFX_VOID_PTR_TO_ULONG(ptr)  ((uint32_t)(ptr))


//------------------------------------------------------------------------
// FAST FLOAT-TO_LONG CONVERSION
//
// We can convert floats to integers quickly by adding a floating-point
// constant to an IEEE float so that the integer value of the IEEE float is
// found in the least significant bits of the sum. We add 2^23 + 2^22, and
// thus the number is represented in IEEE single-precision as
// 2^23*(1.1xxxxxxx), where the first 1 is the implied one, the second is 1 .
//
// This technique has several limitations:
//   1. It only works on values in the range [0,2^22-1].
//   2. It is subject to the processor rounding mode, which we assume to be
//      "round to nearest (even)".  In this rounding mode, the conversion
//      yields round(x), not floor(x) as called for in C.
//
// It can be made to work on negative values with a little fixed point
// trickery by:
//      result = ((LONG) (result ^ 0x00800000) << 8) >> 8,
//
// However, when the result is masked to a byte or a short and directly
// assigned to the right type, no sign extension is required.
//
// The macros for -ve numbers since we use a constant of "1100" in binary
// representation. The msb 1 is force it to be the implicit bit. The next
// 1 is used for -ve numbers which will force other bits to be FFF..
//
// which flips and then smears the sign bit into the rest of the number.  The
// intermediate values must be signed, so we do an arithmetic (fill with sign
// bit) instead of logical shift.
//
// We approximate the floor operation by subtracting off 0.5 before doing the
// round.  This would work perfectly except that the processor breaks ties in
// rounding be selecting the even value.  Thus, we compute floor(1.0) as
// round(0.5) = 0!  Not good -- the caller must be careful to use
// __GL_FLOAT_TO_LONG_TRUNC only in cases where he can live with this
// behavior.

#define GFX_FLOAT_TO_LONG_ROUND(ux,x,mask)      \
{                                               \
    float fx;                                   \
                                                \
    fx = x;                                     \
    fx += 0x00C00000;                           \
    ux = mask & *(uint32_t *) &fx;					\
}

#define GFX_FLOAT_TO_LONG_TRUNC(ux,x,mask)      \
{                                               \
    float fx;                                   \
                                                \
    fx = x - GFX_HALF;                          \
    fx += 0x00C00000;                           \
    ux = mask & *(uint32_t *) &fx;					\
}

// Note: Double has 1 sign bit, 11 exponent and 52 mantissa bits.
// We need to add the following constant for fast conversion
//
//  fx += (__LONG64) 0x18000000000000;
//
// This is done in a portable/decipherable manner through
// multiplications which are collapsed by the compiler at compile time.

#define GFX_DOUBLE_TO_LONG_ROUND(ux,x,mask)     		  \
{                                               		  \
    double fx;                                		      \
                                                		  \
    fx = x;                                     		  \
	fx += 24.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 256; \
    ux = mask & *(uint32_t *) &fx;                		      \
}

#define GFX_DOUBLE_TO_LONG_TRUNC(ux,x,mask)     		  \
{                                               		  \
    double fx;                                   		  \
                                                		  \
    fx = x - GFX_HALF;                             		  \
	fx += 24.0 * 1024.0 * 1024.0 * 1024.0 * 1024.0 * 256; \
    ux = mask & *(uint32_t *) &fx;							  \
}

#if __DEFINE_PROTO || defined __GFX_MACRO_C__
    extern uint32_t GFX_LOG2 (uint32_t a);
    extern uint32_t GFX_REAL_TO_UBYTE (float a);
    extern int64_t GFX_POW2_SIZE (int64_t x);
    extern uint32_t GFX_2_TO_POWER_OF (uint32_t w);
    extern uint32_t GFX_MEMCPY_S(void *d, int dn, const void *s, int n);
    extern void* GFX_MEMCPY_S_INC(void *d, int dn, const void *s, int n);
#endif // __DEFINE_PROTO

#ifndef __S_INLINE
    #define __S_INLINE __inline
#endif

#if __DEFINE_MACRO || defined __GFX_MACRO_C__
//------------------------------------------------------------------------

__S_INLINE uint32_t GFX_LOG2 (uint32_t a)
{
	long	i = 1;

	while ((a >> i) > 0) {
		i++;
	}

	return (i - 1);
}

// Round-up to next power-of-2
__S_INLINE int64_t GFX_POW2_SIZE (int64_t x)
{	                            //-- Signed instead of unsigned since 64-bit is large enough that we don't need unsigned's range extension.
    int64_t Pow2Size;

    /* Algorithm:
        If there are no bits lit beneath the highest bit lit (HBL), the value
        is already a power-of-2 and needs no further rounding but if there are
        additional bits lit beneath the highest, the next higher power-of-2 is
        (1 << (HBL + 1)). To determine if there are bits lit beneath the HBL,
        we will subtract 1 from the given value before scanning for the HBL,
        and only if there are no lower bits lit will the subtraction reduce the
        HBL but in both cases, (1 << (HBL + 1)) will then produce the
        appropriately rounded-up (or not) power-of-2.
    */

    if(x > 1) // <-- Since we bit-scan for (x - 1) and can't bit-scan zero.
    {
        #define MSB (sizeof(x) * CHAR_BIT - 1)

        uint32_t HighBit;

        { // HighBit = HighestBitLit(x - 1)...
            #if defined(__GNUC__) || defined(__clang__)
            {
                HighBit = MSB - __builtin_clzll(x - 1);
            }
            #else // Windows
            {
                #ifdef __CT__
                {
                    _BitScanReverse64((DWORD *)&HighBit, x - 1);
                }
                #else // Break into separate Upper/Lower scans...
                {
                    #define UDW_1 ((int64_t) _UI32_MAX + 1) // <-- UpperDW Value of 1 (i.e. 0x00000001`00000000).

                    if(x < UDW_1)
                    {
                        _BitScanReverse((DWORD *)&HighBit, GFX_ULONG_CAST(x - 1));
                    }
                    else if(x > UDW_1)
                    {
                        _BitScanReverse((DWORD *)&HighBit, GFX_ULONG_CAST((x - 1) >> 32));
                        HighBit += 32;
                    }
                    else
                    {
                        HighBit = 31;
                    }

                    #undef UDW_1
                }
                #endif
            }
            #endif
        }

        if(HighBit < (MSB - 1)) // <-- -1 since operating on signed type.
        {
            Pow2Size = (int64_t) 1 << (HighBit + 1);
        }
        else
        {
            __GFXMACRO_ASSERT(0); // Overflow!
            Pow2Size = 0;
        }

        #undef MSB
    }
    else
    {
        Pow2Size = 1;
    }

    return(Pow2Size);
} // GFX_POW2_SIZE

// Find 2 to the power of w
__S_INLINE uint32_t GFX_2_TO_POWER_OF (uint32_t w)
{
    __GFXMACRO_ASSERT(w < (sizeof(w) * CHAR_BIT)); // Assert no overflow.

    return(1UL << w);
}

__S_INLINE uint32_t GFX_REAL_TO_UBYTE (float a)
{
    uint32_t x;
    GFX_FLOAT_TO_LONG_ROUND(x, a, 0xFF);
    return x;
}

__S_INLINE uint32_t GFX_MEMCPY_S(void *d, int dn, const void *s, int n)
{
    uint32_t Error;

    // Check for the size, overlapping, etc.
    // Calling code responsibility to avoid overlap regions
    __GFXMACRO_ASSERT(n >= 0);
    __GFXMACRO_ASSERT(
        (((char*) d >= (char*) s) && ((ULONG_PTR)((char*) d - (char*) s) >= (ULONG_PTR) n) ) ||
        (((char*) s >= (char*) d) && ((ULONG_PTR)((char*) s - (char*) d) >= (ULONG_PTR) n) ));

    #ifndef _WIN32
        Error = 0;
        if(n <= dn)
        {
            memcpy(d, s, n);
        }
        else
        {
            Error = !Error;
        }
    #else
        Error = (uint32_t) memcpy_s(d, dn, s, n);
    #endif

    __GFXMACRO_ASSERT(!Error);
    return(Error);
}

__S_INLINE void* GFX_MEMCPY_S_INC(void *d, int dn, const void *s, int n)
{
    GFX_MEMCPY_S(d, dn, s, n);
    return GFX_VOID_PTR_INC(d, n);
}

__S_INLINE uint32_t GFX_GET_NONZERO_BIT_COUNT(uint32_t bitMask)
{
    uint32_t bitCount = 0;
    while(bitMask)
    {
        bitCount += bitMask & 0x1;
        bitMask >>= 1;
    }

    return bitCount;
}

#endif

//#include <intrin.h>

// __readmsr should not be called for vGT cases, use __try __except to handle the
// exception and assign it with hard coded values
// isVGT flag will be set if it's being called from vGT.


#define GFX_READ_MSR(pHwDevExt, value, reg, retParam)        \
    if(!gfx_read_msr((pHwDevExt), &(value), (reg))) return retParam;
#define GFX_WRITE_MSR(pHwDevExt, reg, value, retParam)\
    if(!gfx_write_msr((pHwDevExt), (value), (reg))) return retParam;

#define GFX_READ_MSR_GOTO_LABEL_ON_ERROR(pHwDevExt, value, reg, label)        \
    if(!gfx_read_msr((pHwDevExt), &(value), (reg))) goto label;

#define GFX_WRITE_MSR_GOTO_LABEL_ON_ERROR(pHwDevExt, reg, value,  label)\
    if(!gfx_write_msr((pHwDevExt), (value), (reg))) goto label;
#endif // __GFXMACRO_H__
