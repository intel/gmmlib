/******************************************************************************************
** Copyright (c) Intel Corporation (2016).                                  
**                                                                          
** INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS    
** LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,     
** ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT    
** PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY     
** DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR  
** ANY PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all      
** liability, including liability for infringement of any proprietary       
** rights, relating to use of the code. No license, express or implied, by  
** estoppel or otherwise, to any intellectual property rights is            
** granted herein.                                                          
*******************************************************************************************
** 
** File Name     : version.h
** 
** Abastract     : Common version information.  "verinfo.ver" must be 
**                        included after this in the resource file.
** 
**                                                                                                  
** Environment   : Windows 8/8.1/10+   
** 
** Items in File :
**               : This file is only modified by the official builder 
**               : to update the BUILD_NUMBER, & VERSIONSTR values.
**               : Note NT and Vista have different values for the above.
**      
**  --------------------------------------------------------------------------------------*/
#ifdef _WIN32
#ifndef WINNT
#ifndef VER_H
#include "winver.h"             // winver.h defines VS_xxx constants 
#endif //!VER_H
#endif //WINNT
#endif //WIN32

#define OFFICIAL   1
#define FINAL      1

//-------------------------------------------------------------------
// These values should remain constant for all builds. 
//-------------------------------------------------------------------

/* AUTOBLD - Vista MANREV - DO NOT REMOVE */
#define MANVERSION           23
#define MANREVISION          20
#define SUBREVISION            16
//-----------------------------------------------------------------------------------
// The following values should be modified by the official      
// builder for each build.                                      
//   BUILD_NUMBER = current official build number of drivers    
//   VERSIONSTR = string version of concatenation of:           
//       MANVERSION, MANREVISION, SUBREVISION, BUILD_NUMBER-    
//       substring version info                                 
//-----------------------------------------------------------------------------------

#define INTERFACE_REV   0x00010000 // Major upper 16 bits, Minor in lower 16

#define DAY           01
#define MONTH      12
#define YEAR          2016

#define MANUFACTURER    "Intel Corporation" //Driver MFG.
#define DRIVER_VER      "Engineering Sample 0000"               //Driver Release Version.

/* AUTOBLD - Vista LABEL - DO NOT REMOVE */
#define BUILD_NUMBER       9999
#define VERSIONSTR         "23.20.16.9999\0"
#define CUIVERSIONSTR       23,20,16,9999

//---------------------------------------------------------------------------
// The following section defines values used in the version 
// data structure for all files, and which do not change
//---------------------------------------------------------------------------
#define VER_COMPANYNAME         "Intel Corporation\0"
#define VER_LEGALTRADEMARKS    "Windows(TM) is a trademark of Microsoft Corporation.\0"
#define VER_LEGALCOPYRIGHT    "Copyright (c) 1998-2016 Intel Corporation."
#define VER_LEGALCOPYRIGHT_YEARS  "1998-2016"
#define VER_PRODUCTNAME           "Intel HD Graphics Drivers for Windows(R)\0"
#ifdef RC_INVOKED
//-----------------------------------------------------------------------------
// The following section defines values dependant upon build    
// type.                                                        
//-----------------------------------------------------------------------------
// default is nodebug 
#ifndef DEBUG
#define VER_DEBUG                 0
#else
#define VER_DEBUG                 VS_FF_DEBUG
#endif

// default is privatebuild 
#ifndef OFFICIAL
#define VER_PRIVATEBUILD          VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD          0
#endif

// default is prerelease 
#ifndef FINAL
#define VER_PRERELEASE            VS_FF_PRERELEASE
#else
#define VER_PRERELEASE            0
#endif


//-----------------------------------------------------------------------------
// These values must be defined by each individual driver in    
// it's resource file:                                          
//                                                              
//   VER_FILEDESCRIPTION -  String that describes file purpose. 
//   VER_FILETYPE     - Driver type (see ver.h).                
//   VER_FILESUBTYPE  - Driver subtype (see ver.h).             
//   VER_FILEOS       - Target OS of driver (see ver.h).        
//   VER_INTERNALNAME - Internal name of driver.                
//   VER_ORIGINALNAME - Original filename of driver.            
//----------------------------------------------------------------------------
#define VER_FILEFLAGSMASK         VS_FFI_FILEFLAGSMASK
#define VER_FILEFLAGS             (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)


//----------------------------------------------------------------------------
// DO NOT DEFINE THESE UNLESS ABSOLUTELY NECESSARY!!!           
//                                                              
// The file version information defaults to the product version 
// information.  If an individual component is required to have 
// a different builddate than the rest of the components, these 
// values must be defined in that drivers resource file or      
// "verfile.h".                                                 
//                                                              
//   VER_FILEVERSIONSTR - Corresponds to VERSIONSTR.            
//   VER_FILEVERSION    - Corresponds to full version info.     
//    (i.e. MANVERSION, MANREVISION, SUBREVISION, BUILD_NUMBER) 
//                                                              
// See "verinfo.ver".  DO NOT modify "verinfo.ver".             
//---------------------------------------------------------------------------

#endif //RC_INVOKED

