
Intel(R) Graphics Memory Management Library
*******************************************

Introduction
=============

The Intel(R) Graphics Memory Management Library provides device specific and buffer
management for the Intel(R) Graphics Compute Runtime for OpenCL(TM) and the
Intel(R) Media Driver for VAAPI.

License
========

The Intel(R) Graphics Memory Management Library is distributed under the MIT
Open Source license.

You may obtain a copy of the License at:

https://opensource.org/licenses/MIT

Building
========

1) Get gmmlib repo

       |- gmmlib

2) Change it to root directory

   $ cd gmmlib

3) $ mkdir build && cd build

4) cmake [-DCMAKE_BUILD_TYPE= Release | Debug | ReleaseInternal] [-DARCH= 64 | 32]  ..

5) $ make -j8 ( Also performs compile time ULT)

 
Install
^^^^^^^
Not a stand alone software component.
GmmLib is built as dynamic library for Intel media driver and Compute runtime for OpenCL

Supported Platforms
-------------------
Intel Atom and Core Processors supporting Gen8/Gen9/Gen10 graphics devices

BDW (Broadwell)

SKL (Skylake, Kaby Lake, Coffee Lake)

BXTx (BXT: Broxton, APL: Apollo Lake, GLK: Gemini Lake)

KBLx (KBL: Kaby Lake, CFL: Coffe Lake, WHL: Whiskey Lake, CML: Comet Lake, AML: Amber Lake)

CNL (Cannonlake)

ICL (Icelake)

TGLx (TGL: Tiger Lake, RKL: Rocket Lake)

ADLx (ADL-S: Alder Lake S, ADL-P: Alder Lake P, ADL-N: Alder Lake N)

XE_LP (DG1)

XE_HP (XE_HP_SDV)

XE_HPC (PVC: Ponte Vecchio)

XE_HPG (DG2, ACM: Alchemist)

Known Issues and Limitations
----------------------------
- Current Gmmlib support only limited to Linux

(*) Other names and brands may be claimed as property of others.
---------------------------------------------------------------
