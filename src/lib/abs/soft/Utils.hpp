#ifndef UtilsH
#define UtilsH
// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
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
#include <SmallString.hpp>

//------------------------------------------------------------------------------

class ABS_PACKAGE CUtils {
public:
    // miscelaneous ---------------------------------------------------------------

    // site methods ---------------------------------------------------------------
    /// lookup for site id from site name
    static const CSmallString GetSiteID(const CSmallString& site_name);

    /// lookup for site name from site id
    static const CSmallString GetSiteName(const CSmallString& site_id);

    /// is site id valid
    static bool IsSiteIDValid(const CSmallString& site_id);

    /// generate unique ID
    static CSmallString GenerateUUID(void);

    // module name operation ------------------------------------------------------
    /// divide module name to its parts (name,ver,arch,mode)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name,
            CSmallString& ver,
            CSmallString& arch,
            CSmallString& mode);

    /// divide module name to its parts (name,ver,arch)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name,
            CSmallString& ver,
            CSmallString& arch);

    /// divide module name to its parts (name,ver)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name,
            CSmallString& ver);

    /// divide module name to its parts (name)
    static bool ParseModuleName(const CSmallString& module,
            CSmallString& name);

// module name operation -------------------------------------------------------
    /// extract name part from full or incomplete module name
    static const CSmallString GetModuleName(const CSmallString& module);

    /// extract version part from full or incomplete module name
    static const CSmallString GetModuleVer(const CSmallString& module);

    /// extract architecture part from full or incomplete module name
    static const CSmallString GetModuleArch(const CSmallString& module);

    /// extract parascheme part from full or incomplete module name
    static const CSmallString GetModuleMode(const CSmallString& module);
};

//------------------------------------------------------------------------------

#endif
