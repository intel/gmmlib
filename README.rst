
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
   
   git clone https://github.com/intel/gmmlib.git

2) Change it to root directory

   ``$ cd gmmlib``

3) Make a build directory
   
   ``$ mkdir build && cd build``

4) Run the cmake command to prepare build files

|        ``$ cmake [-DCMAKE_BUILD_TYPE=Release | Debug | ReleaseInternal] ..``  
|        where,
|        -DCMAKE_BUILD_TYPE can be set to one build type flag at a time.
|        Example:
|        ``$ cmake -DCMAKE_BUILD_TYPE=Release ..``, For Release build

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

Xe_LPG (MTL: Meteor Lake, ARL: Arrow Lake)

Xe2_HPG (BMG: Battlemage, LNL: Lunar Lake)

Xe3_LPG (Panther Lake)

No code changes may be introduced knowingly, that would regress for any currently supported hardware.
All contributions must ensure continued compatibility and functionality across all supported hardware platforms.
Failure to maintain hardware compatibility may result in the rejection or reversion of the contribution.

Any deliberate modifications or removal of hardware support will be transparently communicated.

API options are solely considered as a stable interface.
Any debug parameters, environmental variables and internal data structures are not considered as an interface and may be changed or removed at any time.

To contribute, Create a pull request on https://github.com/intel/gmmlib with your changes. Ensure that your modifications build without errors.
A maintainer will get in touch with you if there are any inquiries or concerns.

If you have any feedback or questions, please open an issue through our repository interface: https://github.com/intel/gmmlib/issues

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

