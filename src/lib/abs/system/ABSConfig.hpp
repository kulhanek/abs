#ifndef ABSConfigH
#define ABSConfigH
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
#include <XMLDocument.hpp>
#include <VerboseStr.hpp>

//------------------------------------------------------------------------------

class CUUID;

//------------------------------------------------------------------------------

class ABS_PACKAGE CABSConfig {
public:
    // constructor and destructor ----------------------------------------------
    CABSConfig(void);
    ~CABSConfig(void);

    /// LoadConfig
    bool LoadConfig(CVerboseStr& vout);

// system setup ----------------------------------------------------------------
    bool LoadSystemConfig(void);

// user setup ------------------------------------------------------------------
    /// load user config
    void LoadUserConfig(CVerboseStr& vout);

    /// remove user config
    bool RemoveUserConfig(void);

// information about active site -----------------------------------------------
    /// get version of abs module
    const CSmallString GetABSModuleVersion(void);

// global config ---------------------------------------------------------------
    /// return ABS root directory
    const CFileName GetABSRootDir(void);

    /// return system site config
    const CFileName GetSystemSiteConfigDir(void);

    /// return system site aliases config file
    const CFileName GetSystemSiteAliasesFile(void);

    /// return path to plugins definition files
    const CFileName GetPluginsConfigDir(void);

    /// return path to plugins
    const CFileName GetPluginsLibDir(void);

    /// get hostname
    const CSmallString& GetHostName(void);

    /// get documentation URL
    const CSmallString GetDocURL(const CSmallString& key);

// user config -----------------------------------------------------------------
    /// return the name of user config directory for active site
    const CFileName GetUserSiteConfigDir(void);

    /// return the name of user global config directory
    const CFileName GetUserGlobalConfigDir(void);

    /// return the name of any user config directory
    const CFileName GetUserConfigDir(const CFileName& sub_dir);

// jobs ------------------------------------------------------------------------
    /// get server group configuration
    CXMLElement* GetServerGroupConfig(void);

    /// get workdir configuration
    CXMLElement* GetWorkDirConfig(void);

    /// get node group configuration
    CXMLElement* GetNodeGroupConfig(void);

    /// have user valid ticket
    bool IsABSAvailable(std::ostream& sout);

    /// have user valid ticket
    bool IsUserTicketValid(std::ostream& sout);

    /// is input job path allowed?
    bool IsInputJobPathAllowed(const CFileName& path);

    /// print allowed job input paths
    void PrintAllowedJobInputPaths(std::ostream& sout);

    /// is server configured
    bool IsServerConfigured(const CSmallString& name);

// configuration methods -------------------------------------------------------
    /// return system configuration value
    bool GetSystemConfigItem(const CSmallString& item_name,
                             CSmallString& item_value);

    /// return system configuration value
    const CSmallString GetSystemConfigItem(const CSmallString& item_name);

    /// return user configuration value
    bool GetUserConfigItem(const CSmallString& item_name,
                           CSmallString& item_value);

    /// return user configuration value
    bool GetUserConfigItem(const CSmallString& item_name,
                           CSmallString& item_value,bool& system);

    /// set user configuration value
    void SetUserConfigItem(const CSmallString& item_name,
                           const CSmallString& item_value);

// section of private data -----------------------------------------------------
private:
    CSmallString    ABSRoot;
    CSmallString    HostName;

    CXMLDocument    SystemConfig;
    CXMLElement*    UserConfig;

    bool            TicketChecked;
    bool            ValidTicket;

    bool GetSystemConfigItem(CXMLElement* p_root, const CSmallString& item_name,
                             CSmallString& item_value);
};

//------------------------------------------------------------------------------

extern ABS_PACKAGE CABSConfig ABSConfig;

//------------------------------------------------------------------------------

#endif
