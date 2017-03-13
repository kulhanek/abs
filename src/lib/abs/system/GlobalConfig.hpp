#ifndef GlobalConfigH
#define GlobalConfigH
// =============================================================================
//  AMS - Advanced Module System
// -----------------------------------------------------------------------------
//     Copyright (C) 2012 Petr Kulhanek (kulhanek@chemi.muni.cz)
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <ABSMainHeader.hpp>
#include <XMLDocument.hpp>
#include <FileName.hpp>

//------------------------------------------------------------------------------

class ABS_PACKAGE CGlobalConfig {
public:
    // constructor and destructor -------------------------------------------------
    CGlobalConfig(void);

// information about active site ----------------------------------------------
    /// get ID of active site
    const CSmallString& GetActiveSiteID(void);

    /// get name of active site
    const CSmallString GetActiveSiteName(void);

    /// get hostname
    const CSmallString& GetHostName(void);

    /// get version of abs module
    const CSmallString GetABSModuleVersion(void);

    /// is module active
    bool IsModuleActive(const CSmallString& name);

    /// get version of active module
    const CSmallString GetActiveModuleVersion(const CSmallString& name);

// user config -----------------------------------------------------------------
    /// return AMS root directory
    const CFileName GetAMSRootDir(void);

    /// return ABS root directory
    const CFileName GetABSRootDir(void);

    /// return system site config
    const CFileName GetSystemSiteConfigDir(void);

    /// return path to plugins definition files
    const CFileName GetPluginsConfigDir(void);

    /// return path to plugins
    const CFileName GetPluginsDir(void);

// user config -----------------------------------------------------------------
    /// return the name of user config directory for active site
    const CFileName GetUserSiteConfigDir(void);

    /// return the name of user global config directory
    const CFileName GetUserGlobalConfigDir(void);

    /// return the name of any user config directory
    const CFileName GetUserConfigDir(const CFileName& sub_dir);

// information about modules ---------------------------------------------------

    /// return complete specification of active modules
    const CSmallString& GetActiveModules(void);

    /// return export specification of exported modules
    const CSmallString& GetExportedModules(void);

    /// return complete specification of active module
    const CSmallString GetActiveModuleSpecification(const CSmallString& name);

    /// return export specification of active module
    const CSmallString GetExportedModuleSpecification(const CSmallString& name);

// section of private data ----------------------------------------------------
private:
    // this values are initialized in constructor and can be updated anytime
    // by CSite during site activation/deactivation
    CSmallString    ActiveSiteID;
    CSmallString    ActiveSiteName;
    CSmallString    ActiveModules;      // list of active modules
    CSmallString    ExportedModules;    // list of exported modules
    CSmallString    HostName;
    CSmallString    AMSRoot;
};

//------------------------------------------------------------------------------

extern CGlobalConfig GlobalConfig;

//------------------------------------------------------------------------------

#endif
