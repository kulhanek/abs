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

#include <GlobalConfig.hpp>
#include <XMLParser.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <string.h>
#include <vector>
#include <FileSystem.hpp>
#include <Site.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//------------------------------------------------------------------------------

CGlobalConfig GlobalConfig;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGlobalConfig::CGlobalConfig(void)
{
    // must be here
    AMSRoot         = CShell::GetSystemVariable("AMS_ROOT");
    HostName        = CShell::GetSystemVariable("HOSTNAME");
    ActiveSiteID    = CShell::GetSystemVariable("AMS_SITE");
    ActiveModules   = CShell::GetSystemVariable("AMS_ACTIVE_MODULES");
    ExportedModules = CShell::GetSystemVariable("AMS_EXPORTED_MODULES");

    ActiveSiteName  = CUtils::GetSiteName(ActiveSiteID);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CGlobalConfig::GetActiveSiteID(void)
{
    return(ActiveSiteID);
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetActiveSiteName(void)
{
    return(ActiveSiteName);
}

//------------------------------------------------------------------------------

const CSmallString& CGlobalConfig::GetHostName(void)
{
    return(HostName);
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetABSModuleVersion(void)
{
    return( GetActiveModuleVersion("abs") );
}

//------------------------------------------------------------------------------

bool CGlobalConfig::IsModuleActive(const CSmallString& name)
{
    string         sact_modules(ActiveModules);
    vector<string> act_modules;
    split(act_modules,sact_modules,is_any_of("|"));

    for(unsigned int i=0; i < act_modules.size(); i++){
        CSmallString module = act_modules[i];
        CSmallString lname;
        if( CUtils::ParseModuleName(module,lname) ){
            if( lname == name ){
                return(true);
            }
        }
    }

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetActiveModuleVersion(const CSmallString& name)
{
    string         sact_modules(ActiveModules);
    vector<string> act_modules;
    split(act_modules,sact_modules,is_any_of("|"));

    for(unsigned int i=0; i < act_modules.size(); i++){
        CSmallString module = act_modules[i];
        CSmallString lname,version;
        if( CUtils::ParseModuleName(module,lname,version) ){
            if( lname == name ){
                return(version);
            }
        }
    }

    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CGlobalConfig::GetAMSRootDir(void)
{
    return(AMSRoot);
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetABSRootDir(void)
{
    return( CShell::GetSystemVariable("ABS_ROOT") );
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetSystemSiteConfigDir(void)
{
    CFileName      config_name;
    CSmallString   ams_site;

    ams_site = GlobalConfig.GetActiveSiteID();
    config_name = GetABSRootDir() / "etc" / "sites" / ams_site / "aliases.xml";

    return(config_name);
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetPluginsConfigDir(void)
{
    CFileName dir;
    dir = GetABSRootDir() / "etc" / "plugins";
    return(dir);
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetPluginsDir(void)
{
    CFileName dir;
    dir = GetABSRootDir() / "lib";
    return(dir);
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetUserSiteConfigDir(void)
{
    return( GetUserConfigDir(GlobalConfig.GetActiveSiteID()) );
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetUserGlobalConfigDir(void)
{
    return( GetUserConfigDir("") );
}

//------------------------------------------------------------------------------

const CFileName CGlobalConfig::GetUserConfigDir(const CFileName& sub_dir)
{
    CFileName user_config_dir;
    user_config_dir = CShell::GetSystemVariable("ABS_USER_CONFIG_DIR");

    if( user_config_dir == NULL ) {
        user_config_dir = CShell::GetSystemVariable("HOME");
        user_config_dir = user_config_dir / ".abs" ;
    }

    if( sub_dir != NULL ) user_config_dir = user_config_dir / sub_dir;

    // is file?
    if( CFileSystem::IsFile(user_config_dir) == true ) {
        CSmallString error;
        error << "user config dir '" << user_config_dir << "' is a file";
        ES_ERROR(error);
        return("");
    }

    if( CFileSystem::IsDirectory(user_config_dir) == false ) {
        // create directory
        if( CFileSystem::CreateDir(user_config_dir) == false ) {
            CSmallString error;
            error << "unable to create user config dir '" << user_config_dir << "'";
            ES_ERROR(error);
            return("");
        }
    }

    return(user_config_dir);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString& CGlobalConfig::GetActiveModules(void)
{
    return(ActiveModules);
}

//------------------------------------------------------------------------------

const CSmallString& CGlobalConfig::GetExportedModules(void)
{
    return(ExportedModules);
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetActiveModuleSpecification(
    const CSmallString& name)
{
    if( ActiveModules == NULL ) return("");

    char* p_lvar;

    CSmallString tmp(ActiveModules);

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        if( CUtils::GetModuleName(p_lvar) == name ) {
            return(p_lvar);
        }
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    return("");
}

//------------------------------------------------------------------------------

const CSmallString CGlobalConfig::GetExportedModuleSpecification(
    const CSmallString& name)
{
    if( ExportedModules == NULL ) return("");

    char* p_lvar;

    CSmallString tmp(ExportedModules);

    char* p_saveptr = NULL;
    p_lvar = strtok_r(tmp.GetBuffer(),"|",&p_saveptr);
    while( p_lvar != NULL ) {
        if( CUtils::GetModuleName(p_lvar) == name ) {
            return(p_lvar);
        }
        p_lvar = strtok_r(NULL,"|",&p_saveptr);
    }

    return("");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

