#ifndef INFMainHeaderH
#define INFMainHeaderH
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

#include <HiPoLyMainHeader.hpp>

//------------------------------------------------------------------------------

extern const char* LibConfigVersion_ABS;
extern const char* LibBuildVersion_ABS;
extern const char* LibBuildVersion_ABS_Web;

//------------------------------------------------------------------------------

#if defined _WIN32 || defined __CYGWIN__
#ifdef ABS_BUILDING_DLL
#ifdef __GNUC__
#define ABS_DLL_PUBLIC __attribute__((dllexport))
#else
#define ABS_DLL_PUBLIC __declspec(dllexport)
#endif
#else
#ifdef __GNUC__
#define ABS_DLL_PUBLIC __attribute__((dllimport))
#else
#define ABS_DLL_PUBLIC __declspec(dllimport)
#endif
#define INF_DLL_LOCAL
#endif
#else
#if __GNUC__ >= 4
#define ABS_DLL_PUBLIC __attribute__ ((visibility("default")))
#define INF_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define ABS_DLL_PUBLIC
#define INF_DLL_LOCAL
#endif
#endif

#define ABS_PACKAGE ABS_DLL_PUBLIC

//------------------------------------------------------------------------------

#endif
