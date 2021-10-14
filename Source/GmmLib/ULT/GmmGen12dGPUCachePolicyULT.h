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

#include "GmmCachePolicyULT.h"

class CTestGen12dGPUCachePolicy : public CTestCachePolicy
{
protected:
    virtual void SetUpGen12dGPUVariant(PRODUCT_FAMILY);
    virtual void TearDownGen12dGPUVariant();
    virtual void CheckL3Gen12dGPUCachePolicy();
    virtual void CheckSpecialMocs(uint32_t                    Usage,
                                  uint32_t                    AssignedMocsIdx,
                                  GMM_CACHE_POLICY_ELEMENT ClientRequest);

    virtual void CheckMocsIdxHDCL1(uint32_t                    Usage,
                                   uint32_t                    AssignedMocsIdx,
                                   GMM_CACHE_POLICY_ELEMENT ClientRequest);

public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};

class CTestXe_HP_CachePolicy : public CTestGen12dGPUCachePolicy
{
protected:
    virtual void SetUpPlatformVariant(PRODUCT_FAMILY);
    virtual void TearDownPlatformVariant();
    virtual void CheckL3CachePolicy();

public:
    static void SetUpTestCase();
    static void TearDownTestCase();
};
#pragma once
