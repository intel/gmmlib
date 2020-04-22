/*==============================================================================
Copyright(c) 2020 Intel Corporation

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

#include "GmmGen10ResourceULT.h"
#include "../GmmLib/inc/Internal/Common/Platform/GmmGen12Platform.h"

class CTestGen12dGPUResource : public CTestGen10Resource
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

#define DEFINE_TILE(xxx, bpp)                                                                                       \
    (bpp == TEST_BPP_8) ? TILE_##xxx##_8bpe :                                                                       \
                          (bpp == TEST_BPP_16) ? TILE_##xxx##_16bpe :                                               \
                                                 (bpp == TEST_BPP_32) ? TILE_##xxx##_32bpe :                        \
                                                                        (bpp == TEST_BPP_64) ? TILE_##xxx##_64bpe : \
                                                                                               TILE_##xxx##_128bpe
