// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2011      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2001-2008 Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include <stdlib.h>
#include <TorqueConfig.hpp>
#include <ErrorSystem.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <Shell.hpp>
#include <FileSystem.hpp>
#include <GlobalConfig.hpp>
#include <PluginDatabase.hpp>
#include <TicketChecker.hpp>
#include <fnmatch.h>
#include <vector>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

//------------------------------------------------------------------------------

CTorqueConfig TorqueConfig;

//------------------------------------------------------------------------------

using namespace std;
using namespace boost;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CTorqueConfig::CTorqueConfig(void)
{
    Mode = ETM_TORQUE;
}

//------------------------------------------------------------------------------

CTorqueConfig::~CTorqueConfig(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorqueConfig::LoadConfig(void)
{
    bool result;
    result  = LoadSystemConfig();
    result &= LoadUserConfig();
    return(result);
}

//------------------------------------------------------------------------------

bool CTorqueConfig::LoadSystemConfig(void)
{
    CFileName      config_name;

    // load infinity config
    config_name = GlobalConfig.GetABSRootDir() / "etc" / "sites" / GlobalConfig.GetActiveSiteID() / "torque.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&SystemConfig);

    // load system setup ---------------------------
    if( xml_parser.Parse(config_name) == false ){
        ES_ERROR("unable to load Infinity system config");
        return(false);
    }

    CSmallString smode;
    if( GetSystemConfigItem("INF_TORQUE_MODE",smode) == true ){
        // optional
        Mode = static_cast<ETorqueMode>(smode.ToInt());
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CTorqueConfig::LoadSystemConfig(const CSmallString& site_sid)
{
    CFileName      config_name;

    // load infinity config
    config_name = GlobalConfig.GetABSRootDir() / "etc" / "sites" / site_sid / "torque.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&SystemConfig);

    // load system setup ---------------------------
    if( xml_parser.Parse(config_name) == false ){
        ES_ERROR("unable to load Infinity system config");
        return(false);
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorqueConfig::LoadUserConfig(void)
{
    CFileName      config_name;
    config_name = GlobalConfig.GetUserSiteConfigDir() / "torque.xml";

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

bool CTorqueConfig::SaveUserConfig(void)
{
    CFileName      config_name;
    config_name = GlobalConfig.GetUserSiteConfigDir() / "torque.xml";

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

bool CTorqueConfig::RemoveUserConfig(void)
{
    UserConfig.RemoveAllChildNodes();
    UserConfig.GetChildElementByPath("torque/config",true);
    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorqueConfig::IsUserTicketValid(std::ostream& sout)
{
    CSmallString ticket_validator;

    if( GetSystemConfigItem("INF_CHECK_TICKET",ticket_validator) == false ){
        // no ticket validator is requested
        return(true);
    }

    // create validator
    CComObject* p_obj = PluginDatabase.CreateObject(CExtUUID(ticket_validator));
    if( p_obj == NULL ){
        ES_ERROR("unable to create checker object");
        return(false);
    }

    CTicketChecker* p_chk = dynamic_cast<CTicketChecker*>(p_obj);
    if( p_chk == NULL ){
        delete p_obj;
        ES_ERROR("object is not of correct type");
        return(false);
    }

    // validate ticket
    bool result = p_chk->IsTicketValid(sout);

    // destroy object
    delete p_chk;

    return(result);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CTorqueConfig::GetServerType(void)
{
    return("Torque/OpenPBS/PBSPro compatible (v2)");
}

//------------------------------------------------------------------------------

const CSmallString CTorqueConfig::GetServerName(void)
{
    CSmallString name;
    if( GetSystemConfigItem("INF_SERVER_NAME",name) == false){
        ES_ERROR("unable to get system config item INF_SERVER_NAME");
    }

    // substitute server name if necessary
    if( name == "@localhost" ){
        name = GlobalConfig.GetHostName();
    }

    return(name);
}

//------------------------------------------------------------------------------

CXMLElement* CTorqueConfig::GetNodeGroupConfig(void)
{
    return( SystemConfig.GetChildElementByPath("torque/nodes") );
}

//------------------------------------------------------------------------------

void CTorqueConfig::PrintAllowedSyncModes(std::ostream& sout)
{
    CSmallString allowed_sync_modes="sync";
    GetSystemConfigItem("INF_SUPPORTED_SYNCMODES",allowed_sync_modes);

    vector<string>  sync_modes;
    string sl = string(allowed_sync_modes);
    split(sync_modes,sl,is_any_of(":"));

    vector<string>::iterator    it = sync_modes.begin();
    vector<string>::iterator    ie = sync_modes.end();

    while( it != ie ){
        sout << *it;
        it++;
        if( it != ie ) sout << ",";
    }
    sout << endl;
}

//------------------------------------------------------------------------------

void CTorqueConfig::PrintAllowedResTokens(std::ostream& sout)
{
    CSmallString allowed_res_tokens="ncpus";
    GetSystemConfigItem("INF_SUPPORTED_RESOURCES",allowed_res_tokens);

    vector<string>  res_tokens;
    string sl = string(allowed_res_tokens);
    split(res_tokens,sl,is_any_of(":"));

    vector<string>::iterator    it = res_tokens.begin();
    vector<string>::iterator    ie = res_tokens.end();

    while( it != ie ){
        sout << *it;
        it++;
        if( it != ie ) sout << ", ";
    }
}

//------------------------------------------------------------------------------

bool CTorqueConfig::TestResourceTokens(const CSmallString& name)
{
    CSmallString allowed_res_tokens="ncpus";
    GetSystemConfigItem("INF_SUPPORTED_RESOURCES",allowed_res_tokens);

    vector<string>  res_tokens;
    string sl = string(allowed_res_tokens);
    split(res_tokens,sl,is_any_of(":"));

    vector<string>::iterator    it = res_tokens.begin();
    vector<string>::iterator    ie = res_tokens.end();

    while( it != ie ){
        if( CSmallString(*it) == name ){
            return(true);
        }
        it++;
    }
    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CTorqueConfig::GetSystemConfigItem(const CSmallString& item_name,
                                        CSmallString& item_value)
{
    CXMLElement*    p_ele = SystemConfig.GetChildElementByPath("abs/config");
    return( GetSystemConfigItem(p_ele,item_name,item_value) );
}

//------------------------------------------------------------------------------

const CSmallString CTorqueConfig::GetSystemConfigItem(const CSmallString& item_name)
{
    CSmallString out;
    GetSystemConfigItem(item_name,out);
    return(out);
}

//------------------------------------------------------------------------------

bool CTorqueConfig::GetUserConfigItem(const CSmallString& item_name,
                                          CSmallString& item_value)
{
    bool system;
    return( GetUserConfigItem(item_name,item_value,system) );
}

//------------------------------------------------------------------------------

bool CTorqueConfig::GetUserConfigItem(const CSmallString& item_name,
                                      CSmallString& item_value,bool& system)
{
    CXMLElement*    p_ele = UserConfig.GetChildElementByPath("torque");
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

void CTorqueConfig::SetUserConfigItem(const CSmallString& item_name,
                                      const CSmallString& item_value)
{
    CXMLElement*    p_ele = UserConfig.GetChildElementByPath("torque",true);
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

bool CTorqueConfig::GetSystemConfigItem(CXMLElement* p_root, const CSmallString& item_name,
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
    ES_ERROR(error);
    return(false);
}

//------------------------------------------------------------------------------

bool CTorqueConfig::GetSystemConfigItem(const CUUID& pobject, const CSmallString& item_name,
                                        CSmallString& item_value)
{
    CXMLElement*    p_ele = SystemConfig.GetChildElementByPath("torque/plugins");
    CXMLIterator    I(p_ele);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("plugin")) != NULL ){
        CSmallString    name;
        p_sele->GetAttribute("name",name);
        CExtUUID plugin_uuid(name);
        if( plugin_uuid == pobject ){
                return( GetSystemConfigItem(p_sele,item_name,item_value) );
        }
    }

    CSmallString error;
    error << "unable to find system config item '" << item_name << "' for plugin '" << pobject.GetStringForm() << "'";
    ES_ERROR(error);

    return(false);

}

//------------------------------------------------------------------------------

const CSmallString CTorqueConfig::GetSystemConfigItem(const CUUID& pobject, const CSmallString& item_name)
{
    CSmallString item_value;
    GetSystemConfigItem(pobject,item_name,item_value);
    return(item_value);
}

//------------------------------------------------------------------------------

ETorqueMode CTorqueConfig::GetTorqueMode(void) const
{
    return(Mode);
}

//------------------------------------------------------------------------------

const CSmallString CTorqueConfig::GetScratchDir(const CSmallString& type,const CSmallString& mode)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("torque/scratches/"+mode);

    while( p_ele != NULL ){
        CSmallString lname;
        p_ele->GetAttribute("name",lname);
        if( lname == type ){
            CSmallString lpath;
            p_ele->GetAttribute("path",lpath);
            return(lpath);
        }
        p_ele = p_ele->GetNextSiblingElement(mode);
    }

    if( mode == "main" ){
        return("/scratch/$USER/$INF_JOB_ID/main");
    } else if ( mode == "ijob" ){
        return("/scratch/$USER/$INF_JOB_ID/$INF_IJOB_ID");
    } else if ( mode == "clean" ){
        return("/scratch/$USER/$INF_JOB_ID");
    } else {
        RUNTIME_ERROR("unsupported scratch mode " + mode);
    }
}

//------------------------------------------------------------------------------

const CSmallString CTorqueConfig::GetSurrogateMachine(const CSmallString& machine,const CSmallString& jobdir)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("torque/surrogates/machine");

    while( p_ele != NULL ){
        CSmallString lhost;
        p_ele->GetAttribute("name",lhost);
        CSmallString info;
        info << "Surrogate: testing machine : " << lhost;
        ES_WARNING(info);
        if( fnmatch(lhost,machine,0) == 0 ){
            CSmallString info;
            info << " -- SUCCESS : " << machine;
            ES_WARNING(info);
            CXMLElement* p_sele = p_ele->GetFirstChildElement("surrogate");
            while( p_sele != NULL ){
                CSmallString lpath;
                p_sele->GetAttribute("dir",lpath);
                CSmallString info;
                info << " -- Testing dir : " << lpath;
                ES_WARNING(info);
                if( fnmatch(lpath,jobdir,0) == 0 ){
                    CSmallString lsurrogate;
                    p_sele->GetAttribute("name",lsurrogate);
                    CSmallString info;
                    info << "    -- SUCCESS : " << lsurrogate;
                    ES_WARNING(info);
                    if( lsurrogate == "JOBMACHINE" ){
                        return(machine);
                    } else {
                        return(lsurrogate);
                    }
                }
                p_sele = p_sele->GetNextSiblingElement("surrogate");
            }
            // not found
            return("");
        }
        p_ele = p_ele->GetNextSiblingElement("machine");
    }

    // not found
    return("");
}

//------------------------------------------------------------------------------

bool CTorqueConfig::GetSurrogateGroup(const CSmallString& machine,const CSmallString& jobdir,CSmallString& ugroup,CSmallString& ugroup_realm)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("torque/surrogates/machine");

    while( p_ele != NULL ){
        CSmallString lhost;
        p_ele->GetAttribute("name",lhost);
        if( fnmatch(lhost,machine,0) == 0 ){
            CXMLElement* p_sele = p_ele->GetFirstChildElement("surrogate");
            while( p_sele != NULL ){
                CSmallString lpath;
                p_sele->GetAttribute("dir",lpath);
                if( fnmatch(lpath,jobdir,0) == 0 ){
                    CSmallString lsurrogate;
                    p_sele->GetAttribute("name",lsurrogate);
                    // found
                    p_sele->GetAttribute("realm",ugroup_realm); // default realm
                    CXMLElement* p_mele = p_sele->GetFirstChildElement("map");
                    while( p_mele != NULL ){
                        CSmallString lgroup;
                        p_mele->GetAttribute("lgroup",lgroup);
                        if( lgroup == ugroup ){
                            p_mele->GetAttribute("sgroup",ugroup);
                            p_mele->GetAttribute("realm",ugroup_realm);
                            return(true);
                        }
                        p_mele = p_mele->GetNextSiblingElement("map");
                    }
                    return(false);
                }
                p_sele = p_sele->GetNextSiblingElement("surrogate");
            }
            // not found
            return(false);
        }
        p_ele = p_ele->GetNextSiblingElement("machine");
    }

    // not found
    return(false);
}

//------------------------------------------------------------------------------

bool CTorqueConfig::TestScratchType(const CSmallString& type,bool& pass)
{
    pass = false;
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("torque/scratches/main");

    while( p_ele != NULL ){
        CSmallString lname;
        p_ele->GetAttribute("name",lname);
        if( lname == type ){
            p_ele->GetAttribute("pass",pass);
            return(true);
        }
        p_ele = p_ele->GetNextSiblingElement("main");
    }

    return(false);
}

//------------------------------------------------------------------------------

const CSmallString CTorqueConfig::GetDefaultScratchType(void)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("torque/scratches/main");

    while( p_ele != NULL ){
        CSmallString lname;
        p_ele->GetAttribute("name",lname);
        bool def = false;
        p_ele->GetAttribute("default",def);
        if( def == true ) return(lname);
        p_ele = p_ele->GetNextSiblingElement("main");
    }

    return("scratch");
}

//------------------------------------------------------------------------------

void CTorqueConfig::PrintAllowedScratchTypes(std::ostream& sout)
{
    CXMLElement* p_ele = SystemConfig.GetChildElementByPath("torque/scratches/main");

    while( p_ele != NULL ){
        CSmallString lname;
        p_ele->GetAttribute("name",lname);
        sout << lname;
        p_ele = p_ele->GetNextSiblingElement("main");
        if( p_ele != NULL ){
            sout << ", ";
        }
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


