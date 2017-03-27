// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//     Copyright (C) 2011 Petr Kulhanek (kulhanek@chemi.muni.cz)
//     Copyright (C) 2004,2005,2008,2010 Petr Kulhanek (kulhanek@chemi.muni.cz)
//
//     This library is free software; you can redistribute it and/or
//     modify it under the terms of the GNU Lesser General Public
//     License as published by the Free Software Foundation; either
//     version 2.1 of the License, or (at your option) any later version.
//
//     This library is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//     Lesser General Public License for more details.
//
//     You should have received a copy of the GNU Lesser General Public
//     License along with this library; if not, write to the Free Software
//     Foundation, Inc., 51 Franklin Street, Fifth Floor,
//     Boston, MA  02110-1301  USA
// =============================================================================

#include <ABSMainHeader.hpp>

//------------------------------------------------------------------------------

const char* LibConfigVersion_ABS    = "3.x";
const char* LibBuildVersion_ABS     = "3.x.SVNVERSION (DATE) [ABS]";
const char* LibBuildVersion_ABS_Web = "3.x.SVNVERSION (DATE)";

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

#if defined _WIN32 || defined __CYGWIN__

#include <windows.h>

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return(TRUE);
}

//==============================================================================
#else
//==============================================================================

#include <stdio.h>
#include <unistd.h>

// under UNIX we try to make library as executable object that will
// print copyright notice and build number

// you need to compile shared library with following options

// -e __hipoly_main                         <- define entry point
// --dynamic-loader /ld/ld-linux.so.2       <- redefine dynamic loader
// -pie                                     <- make position independent executable
// --export-dynamic                         <- export symbols

// declare entry point
extern "C" void __ams_main(void) __attribute__ ((noreturn));

// define entry point
extern "C" void __ams_main(void)
{
    printf("\n");
    printf("===========================================================\n");
    printf(" Build: %s\n",LibBuildVersion_ABS);
    printf("===========================================================\n");
    printf("\n");
    _exit(0);
}

#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


//--------------------------------------------------------------------------
