
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
1) Get gmmlib repository

2) Change it to root directory

   ``$ cd gmmlib``

3) Make a build directory
   
   ``$ mkdir build && cd build``

4) Run the cmake command to prepare build files

   ``$ cmake [-DCMAKE_BUILD_TYPE= Release | Debug | ReleaseInternal] ..``

5) Build the project

   ``$ make -j"$(nproc)" (Also performs compile time ULT)``
 
Install
=======
``$ sudo make install``

This will install the following files (e.g. on Ubuntu):

| -- Install configuration: "Release"
| -- Installing: /usr/local/lib/libigdgmm.so.12.1.0
| -- Installing: /usr/local/lib/libigdgmm.so.12
| 


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

Release Tags
============

Gmmlib Release tag versioning schema follows:

| Tag ``intel-gmmlib-<x>.<y>.<z>`` will be stable release series with the same API and ABI version with only bug fixes where,
| x = GMMLIB_API_MAJOR_VERSION + 10,
| y = GMMLIB_API_MINOR_VERSION,
| z = RELEASE NUMBER which is incremented as 0,1,2,...n for changes including new flag, bug fixes, etc.
| 
| Example:
|   For GMM library ``libigdgmm.so.12.0.0``,
|   Tag = ``intel-gmmlib-22.0.0`` where,
|        22 = GMMLIB_API_MAJOR_VERSION + 10 = 12 + 10
|        0 = GMMLIB_API_MINOR_VERSION
|        0 = RELEASE NUMBER
|
On potential ABI break changes,

| Tag ``intel-gmmlib-<x>.<y>.<z>`` becomes ``intel-gmmlib-<x + 1>.0.0``
| i.e ``intel-gmmlib-22.5.3`` becomes ``intel-gmmlib-23.0.0``

Known Issues and Limitations
============================
Current Gmmlib support only limited to Linux

(*) Other names and brands may be claimed as property of others.
---------------------------------------------------------------

