// =============================================================================
// NEMESIS - Molecular Modelling Package
// -----------------------------------------------------------------------------
//    Copyright (C) 2010      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2008-2009 Petr Kulhanek, kulhanek@enzim.hu,
//                            Jakub Stepan, xstepan3@chemi.muni.cz
//    Copyright (C) 1998-2004 Petr Kulhanek, kulhanek@chemi.muni.cz
//
//     This program is free software; you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation; either version 2 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License along
//     with this program; if not, write to the Free Software Foundation, Inc.,
//     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// =============================================================================

#include <PluSuLaMainHeader.hpp>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const char* LibBuildVersion_PluSuLa = PLUSULA_VERSION;

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
extern "C" void __scimafic_main(void) __attribute__ ((noreturn));

// define entry point
extern "C" void __scimafic_main(void)
{
    printf("\n");
    printf("===========================================================\n");
    printf(" ************ PluSuLa - Plugin Support Library *********** \n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf(" (c) 1998-2009                                             \n");
    printf("     Library maintainer: Petr Kulhanek                     \n");
    printf("                         kulhanek@chemi.muni.cz            \n");
    printf("     PluSuLa is licenced under GNU GPL.                    \n");
    printf("===========================================================\n");
    printf(" Build: %s\n",LibBuildVersion_PluSuLa);
    printf("===========================================================\n");
    printf("\n");
    _exit(0);
}

#endif

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

