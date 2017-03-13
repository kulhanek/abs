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

#include <Site.hpp>
#include <FileName.hpp>
#include <XMLParser.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <GlobalConfig.hpp>
#include <XMLIterator.hpp>
#include <fnmatch.h>
#include <XMLText.hpp>
#include <ShellProcessor.hpp>
#include <string.h>
#include <Utils.hpp>
#include <User.hpp>
#include <iomanip>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

CSite    Site;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSite::CSite(void)
{

}

//------------------------------------------------------------------------------

CSite::~CSite(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CSite::LoadConfig(void)
{
    if( GlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    return(LoadConfig(GlobalConfig.GetActiveSiteID()));
}

//------------------------------------------------------------------------------

bool CSite::LoadConfig(const CSmallString& site_id)
{
    if( site_id == NULL ){
        ES_WARNING("site_id is NULL");
        return(false);
    }

    CFileName    config_path;
    SiteConfig.RemoveAllChildNodes();

    config_path = GlobalConfig.GetAMSRootDir();
    config_path = config_path / "etc" / "sites" / site_id / "site.xml";

    CXMLParser xml_parser;
    xml_parser.SetOutputXMLNode(&SiteConfig);

    if( xml_parser.Parse(config_path) == false ) {
        CSmallString error;
        error << "unable to load site '" << site_id << "' config";
        ES_ERROR(error);
        return(false);
    }

    SiteID = site_id;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

const CSmallString CSite::GetName(void) const
{
    CSmallString name;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site path");
        return(name);
    }
    if( p_ele->GetAttribute("name",name) == false ) {
        ES_ERROR("unable to get setup item");
        return(name);
    }
    return(name);
}

//------------------------------------------------------------------------------

const CSmallString CSite::GetID(void)
{
    CSmallString id;
    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site path");
        return(id);
    }
    if( p_ele->GetAttribute("id",id) == false ) {
        ES_ERROR("unable to get setup item");
        return(id);
    }
    return(id);
}

//------------------------------------------------------------------------------

bool CSite::IsActive(void)
{
    // get active site id
    CSmallString active_site = GlobalConfig.GetActiveSiteID();

    // compare with site id
    return((active_site == GetID()) && (active_site.GetLength() > 0));
}

//------------------------------------------------------------------------------

bool CSite::CanBeActivated(bool allow_localhost)
{
    int priority;
    return(CanBeActivated(priority,allow_localhost));
}

//------------------------------------------------------------------------------

bool CSite::CanBeActivated(int& priority,bool allow_localhost)
{
    priority = 0;

    CXMLElement* p_ele = SiteConfig.GetChildElementByPath("site/computers");
    if( p_ele == NULL ) {
        ES_ERROR("unable to open site/computers path");
        return(false);
    }

    // handle personal site
    if( GetName() == "personal" ){
        if( CShell::GetSystemVariable("AMS_PERSONAL") == "ON" ){
            priority = 1000;
            return(true);
        }
        ES_ERROR("site is 'personal' but AMS_PERSONAL variable is not set to 'ON'");
        return(false);
    }

    // go through the list computers
    CXMLIterator I(p_ele);
    CXMLElement* p_comp;
    CSmallString host_name = GlobalConfig.GetHostName();

    while( (p_comp = I.GetNextChildElement("computer")) != NULL ) {
        CSmallString comp_name;
        if( p_comp->GetAttribute("name",comp_name) == false ) {
            ES_ERROR("unable to get computer name");
            return(false);
        }
        if( (comp_name == "localhost") && allow_localhost ){
            p_comp->GetAttribute("priority",priority);
            return(true);
        }
        if( fnmatch(comp_name,host_name,0) == 0 ) {
            p_comp->GetAttribute("priority",priority);
            return(true);
        }
    }
    CSmallString error;
    error << "site '" << GetName() << "' is not allowed on host '" << host_name <<"'";
    ES_ERROR(error);
    return(false);
}

//------------------------------------------------------------------------------

CXMLElement* CSite::GetAutoloadedModules(void)
{
    return(SiteConfig.GetChildElementByPath("site/autoload"));
}

//------------------------------------------------------------------------------

void CSite::PrintShortSiteInfo(ostream& vout)
{
    CSmallString name = GetName();
    int  n1 = (80 - 26 - name.GetLength())/2;

    vout << endl;
    for(int i=0; i < n1; i++) vout << " ";
    vout << " *** Welcome to <b>" << name << "</b> site *** ";
    vout << endl;

    vout << "# ==============================================================================" << endl;
    if( IsActive() ) {
        vout << "# Site name : " << name << " (-active-)" << endl;
        vout << "# Site ID   : " << SiteID <<  endl;
    } else {
        vout << "# Site name : " << name << " (-not active-)" << endl;
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================



