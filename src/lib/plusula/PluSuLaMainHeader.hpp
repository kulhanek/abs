#ifndef PluSuLaMainHeaderH
#define PluSuLaMainHeaderH
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

#include <HiPoLyMainHeader.hpp>

//------------------------------------------------------------------------------

#define PLUSULA_VERSION "PluSuLa 1.0.SVNVERSION (DATE)"

extern const char* LibBuildVersion_PluSuLa;

//------------------------------------------------------------------------------

#if defined _WIN32 || defined __CYGWIN__
#ifdef PLUSULA_BUILDING_DLL
#ifdef __GNUC__
#define PLUSULA_DLL_PUBLIC __attribute__((dllexport))
#else
#define PLUSULA_DLL_PUBLIC __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define PLUSULA_DLL_PUBLIC __attribute__((dllimport))
#else
#define PLUSULA_DLL_PUBLIC __declspec(dllimport)
#endif
#define PLUSULA_DLL_LOCAL
#endif
#else
#if __GNUC__ >= 4
#define PLUSULA_DLL_PUBLIC __attribute__ ((visibility("default")))
#define PLUSULA_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define PLUSULA_DLL_PUBLIC
#define PLUSULA_DLL_LOCAL
#endif
#endif

#define PLUSULA_PACKAGE PLUSULA_DLL_PUBLIC

//--------------------------------------------------------------------------------

#endif
