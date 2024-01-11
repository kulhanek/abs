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

#include <ABSConfig.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <ErrorSystem.hpp>
#include <XMLElement.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <string.h>
#include <vector>
#include <FileSystem.hpp>
#include <Site.hpp>
#include <TicketChecker.hpp>
#include <PluginDatabase.hpp>
#include <XMLIterator.hpp>
#include <fnmatch.h>
#include <SiteController.hpp>
#include <ModuleController.hpp>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;
using namespace boost::algorithm;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CABSConfig::CABSConfig(void)
{
    // must be here
    ABSRoot         = CShell::GetSystemVariable("ABS_ROOT");
    HostName        = CShell::GetSystemVariable("HOSTNAME");

    TicketChecked = false;
    ValidTicket = false;
}

//------------------------------------------------------------------------------

CABSConfig::~CABSConfig(void)
{
    // unload plugins
    PluginDatabase.UnloadPlugins();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CABSConfig::LoadConfig(void)
{
    bool result;
    result  = LoadSystemConfig();
    result &= LoadUserConfig();
    return(result);
}

//------------------------------------------------------------------------------

bool CABSConfig::LoadSystemConfig(void)
{
    CFileName      config_name;

    // load infinity config
    config_name = GetABSRootDir() / "etc" / "sites" / SiteController.GetActiveSite() / "abs.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&SystemConfig);

    // load system setup ---------------------------
    if( xml_parser.Parse(config_name) == false ){
        ES_ERROR("unable to load Infinity system config");
        return(false);
    }

    // init plugin subsystem
    PluginDatabase.SetPluginPath(GetPluginsDir());
    if( PluginDatabase.LoadPlugins(GetPluginsConfigDir()) == false ){
        ES_ERROR("unable to load plugins");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CABSConfig::LoadUserConfig(void)
{
    CFileName      config_name;
    config_name = GetUserSiteConfigDir() / "abs.xml";

    // is file?
    if( CFileSystem::IsFile(config_name) == false ){
        CSmallString warning;
        warning << "user config dir '" << config_name << "' does not exist";
        ES_WARNING(warning);

        UserConfig.GetChildElementByPath("infinity",true);
        return(true);
    }

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&UserConfig);

    // load system setup ---------------------------
    if( xml_parser.Parse(config_name) == false ){
        ES_ERROR("unable to load Infinity user config");
        return(false);
    }

    UserConfig.GetChildElementByPath("torque",true);

    return(true);
}

//------------------------------------------------------------------------------

bool CABSConfig::SaveUserConfig(void)
{
    CFileName      config_name;
    config_name =  GetUserSiteConfigDir() / "abs.xml";

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&UserConfig);

    // load system setup ---------------------------
    if( xml_printer.Print(config_name) == false ){
        ES_ERROR("unable to save Infinity user config");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CABSConfig::RemoveUserConfig(void)
{
    UserConfig.RemoveAllChildNodes();
    UserConfig.GetChildElementByPath("abs/config",true);
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CABSConfig::GetABSModuleVersion(void)
{
    CSmallString version;
    ModuleController.GetActiveModuleVersion("abs",version);
    return( version );
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CABSConfig::GetABSRootDir(void)
{
    return( CShell::GetSystemVariable("ABS_ROOT") );
}

//------------------------------------------------------------------------------

const CSmallString& CABSConfig::GetHostName(void)
{
    return(HostName);
}

//------------------------------------------------------------------------------

const CSmallString CABSConfig::GetDocURL(const CSmallString& key)
{
    return("FIXME");
}

//------------------------------------------------------------------------------

const CFileName CABSConfig::GetSystemSiteConfigDir(void)
{
    CFileName      config_name;
    CSmallString   ams_site;

    ams_site = SiteController.GetActiveSite();
    config_name = GetABSRootDir() / "etc" / "sites" / ams_site / "aliases.xml";

    return(config_name);
}

//------------------------------------------------------------------------------

const CFileName CABSConfig::GetPluginsConfigDir(void)
{
    CFileName dir;
    dir = GetABSRootDir() / "etc" / "plugins";
    return(dir);
}

//------------------------------------------------------------------------------

const CFileName CABSConfig::GetPluginsDir(void)
{
    CFileName dir;
    dir = GetABSRootDir() / "lib";
    return(dir);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CFileName CABSConfig::GetUserSiteConfigDir(void)
{
    return( GetUserConfigDir(SiteController.GetActiveSite()) );
}

//------------------------------------------------------------------------------

const CFileName CABSConfig::GetUserGlobalConfigDir(void)
{
    return( GetUserConfigDir("") );
}

//------------------------------------------------------------------------------

const CFileName CABSConfig::GetUserConfigDir(const CFileName& sub_dir)
{
    CFileName user_config_dir;
    user_config_dir = CShell::GetSystemVariable("ABS_USER_CONFIG_DIR");

    if( user_config_dir == NULL ) {
        user_config_dir = CShell::GetSystemVariable("HOME");
        user_config_dir = user_config_dir / ".abs" / LibConfigVersion_ABS ;
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

bool CABSConfig::GetSystemConfigItem(const CSmallString& item_name,
                                        CSmallString& item_value)
{
    CXMLElement*    p_ele = SystemConfig.GetChildElementByPath("abs/config");
    return( GetSystemConfigItem(p_ele,item_name,item_value) );
}

//------------------------------------------------------------------------------

const CSmallString CABSConfig::GetSystemConfigItem(const CSmallString& item_name)
{
    CSmallString out;
    GetSystemConfigItem(item_name,out);
    return(out);
}

//------------------------------------------------------------------------------

bool CABSConfig::GetUserConfigItem(const CSmallString& item_name,
                                          CSmallString& item_value)
{
    bool system;
    return( GetUserConfigItem(item_name,item_value,system) );
}

//------------------------------------------------------------------------------

bool CABSConfig::GetUserConfigItem(const CSmallString& item_name,
                                      CSmallString& item_value,bool& system)
{
    CXMLElement*    p_ele = UserConfig.GetChildElementByPath("abs/config");
    CXMLIterator    I(p_ele);
    CXMLElement*    p_sele;

    system = false;
    item_value = NULL;

    while( (p_sele = I.GetNextChildElement("item")) != NULL ){
        CSmallString    name;
        p_sele->GetAttribute("name",name);
        if( name == item_name ){
            p_sele->GetAttribute("value",item_value);
            system = false;
            return(true);
            }
    }

    if( GetSystemConfigItem(item_name,item_value) == true ){
        system = true;
        return(true);
    }

    CSmallString error;
    error << "unable to find user config item '" << item_name << "'";
    ES_ERROR(error);

    return(false);
}

//------------------------------------------------------------------------------

void CABSConfig::SetUserConfigItem(const CSmallString& item_name,
                                      const CSmallString& item_value)
{
    CXMLElement*    p_ele = UserConfig.GetChildElementByPath("abs/config",true);
    CXMLIterator    I(p_ele);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("item")) != NULL ){
        CSmallString    name;
        p_sele->GetAttribute("name",name);
        if( name == item_name ) break;
    }

    if( p_sele == NULL ){
        p_sele = p_ele->CreateChildElement("item");
    }

    p_sele->SetAttribute("name",item_name);
    p_sele->SetAttribute("value",item_value);
}

//------------------------------------------------------------------------------

bool CABSConfig::GetSystemConfigItem(CXMLElement* p_root, const CSmallString& item_name,
                                        CSmallString& item_value)
{
    CXMLIterator    I(p_root);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("item")) != NULL ){
        CSmallString    name;
        p_sele->GetAttribute("name",name);
        if( name == item_name ){
            p_sele->GetAttribute("value",item_value);
            return(true);
            }
    }

    CSmallString error;
    error << "unable to find system config item '" << item_name << "'";
    ES_TRACE_ERROR(error);
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CABSConfig::GetServerGroupConfig(void)
{
    return( SystemConfig.GetChildElementByPath("abs/servers") );
}

//------------------------------------------------------------------------------

CXMLElement* CABSConfig::GetWorkDirConfig(void)
{
    return( SystemConfig.GetChildElementByPath("abs/workdirs") );
}

//------------------------------------------------------------------------------

CXMLElement* CABSConfig::GetNodeGroupConfig(void)
{
    return( SystemConfig.GetChildElementByPath("abs/nodes") );
}

//------------------------------------------------------------------------------

bool CABSConfig::IsABSAvailable(std::ostream& sout)
{
    CFileName      config_name;

    // global abs config
    config_name = GetABSRootDir() / "etc" / "sites" / SiteController.GetActiveSite() / "abs.xml";

    bool enabled = true;

    CXMLParser      xml_parser;
    CXMLDocument    xml_config;
    xml_parser.SetOutputXMLNode(&xml_config);

    // load system setup ---------------------------

    if( CFileSystem::IsFile(config_name) == false ){
        CSmallString error;
        error << "no config file '" << config_name << "'";
        ES_TRACE_ERROR(error);
        enabled = false;
    }

    if( enabled == true ){
        if( xml_parser.Parse(config_name) == false ){
            CSmallString error;
            error << "unable to parse system config '" << config_name << "'";
            ES_TRACE_ERROR(error);
            enabled = false;
        }
    }

    if( enabled == true ){
        CXMLElement* p_ele = xml_config.GetChildElementByPath("abs");
        if( p_ele != NULL ){
            p_ele->GetAttribute("enabled",enabled);
            if( enabled == false ) ES_TRACE_ERROR("disabled by enabled=false in abs");
        } else {
            enabled = false;
            ES_TRACE_ERROR("no abs element");
        }
    }

    if( ! enabled ){
        sout << endl;
        sout << "<b><blue> >>> INFO: The ABS (Advanced Batch System) is not configured for this site!</blue></b>" << endl;
        sout << endl;
    }

    return(enabled);
}

//------------------------------------------------------------------------------

bool CABSConfig::IsUserTicketValid(std::ostream& sout)
{
    // is result chached?
    if( TicketChecked ){
        return(ValidTicket);
    }

    CSmallString ticket_validator;

    if( GetSystemConfigItem("INF_CHECK_TICKET",ticket_validator) == false ){
        // no ticket validator is requested
        TicketChecked = true;
        ValidTicket = true;
        return(true);
    }

    // create validator
    CComObject* p_obj = PluginDatabase.CreateObject(CExtUUID(ticket_validator));
    if( p_obj == NULL ){
        ES_ERROR("unable to create checker object");
        TicketChecked = true;
        ValidTicket = false;
        return(false);
    }

    CTicketChecker* p_chk = dynamic_cast<CTicketChecker*>(p_obj);
    if( p_chk == NULL ){
        delete p_obj;
        ES_ERROR("object is not of correct type");
        TicketChecked = true;
        ValidTicket = false;
        return(false);
    }

    // validate ticket
    bool result = p_chk->IsTicketValid(sout);

    // destroy object
    delete p_chk;

    TicketChecked = true;
    ValidTicket = result;

    return(result);
}

//------------------------------------------------------------------------------

bool CABSConfig::IsInputJobPathAllowed(const CFileName& path)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("abs/inpdirs/filter");
    if( p_ele == NULL ) return(true);

    while( p_ele != NULL ){
        CSmallString pattern;
        if( p_ele->GetAttribute("path",pattern) == true ){
            if( fnmatch(pattern,path,0) == 0 ) return(true);
        }
        p_ele = p_ele->GetNextSiblingElement("filter");
    }

    return(false);
}

//------------------------------------------------------------------------------

void CABSConfig::PrintAllowedJobInputPaths(std::ostream& sout)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("abs/inpdirs/filter");
    if( p_ele == NULL ) return;

    //sout << "<b><red> ERROR: The job input directory '<u>" << outpath << "</u>' is not allowed!</red></b>";

      sout <<         "        Allowed paths:" << endl;

    while( p_ele != NULL ){
        CSmallString pattern;
        if( p_ele->GetAttribute("path",pattern) == true ){
            sout <<         "        " << pattern << endl;
        }

        p_ele = p_ele->GetNextSiblingElement("filter");
    }
}

//------------------------------------------------------------------------------

bool CABSConfig::IsServerConfigured(const CSmallString& name)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("abs/servers/server");
    if( p_ele == NULL ) return(false);

    while( p_ele != NULL ){
        CSmallString lname;
        if( p_ele->GetAttribute("name",lname) == true ){
            if( name == lname ) return(true);
        }
        p_ele = p_ele->GetNextSiblingElement("server");
    }
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

