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

// Helper Macros for CoherentPATIndex Value handling.
#define CONCATENATE_COHERENT_PATINDEX(a, b) (((a & __BIT(0)) << 5) | b) // 'a' ->higher order bits 6th bit [5] MSB, 'b' -> Lower order 5 bits [4:0]

#define GET_COHERENT_PATINDEX_VALUE(pGmmLibContext, usage) (CONCATENATE_COHERENT_PATINDEX(pGmmLibContext->GetCachePolicyElement(usage).CoherentPATIndexHigherBit, \
                                                                                          pGmmLibContext->GetCachePolicyElement(usage).CoherentPATIndex))

#define GET_COHERENT_PATINDEX_LOWER_BITS(value) (value & (~(~0 << 5)))

#define GET_COHERENT_PATINDEX_HIGHER_BIT(value) ((value >> 5) & __BIT(0))
