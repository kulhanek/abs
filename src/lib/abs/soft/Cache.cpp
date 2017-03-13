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

#include <Cache.hpp>
#include <FileName.hpp>
#include <string.h>
#include <dirent.h>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLElement.hpp>
#include <XMLComment.hpp>
#include <XMLIterator.hpp>
#include <ErrorSystem.hpp>
#include <GlobalConfig.hpp>
#include <DirectoryEnum.hpp>
#include <FileSystem.hpp>
#include <Shell.hpp>
#include <Utils.hpp>
#include <XMLText.hpp>
#include <fnmatch.h>
#include <iomanip>
#include <list>
#include <User.hpp>

//------------------------------------------------------------------------------

CCache Cache;

using namespace std;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CCache::CCache(void)
{

}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::LoadCache(void)
{
    Cache.RemoveAllChildNodes();

    if( GlobalConfig.GetActiveSiteID() == NULL ) {
        ES_ERROR("no site is active");
        return(false);
    }

    CFileName    config_path;

    config_path = GlobalConfig.GetAMSRootDir();
    config_path = config_path / "etc" / "sites" / GlobalConfig.GetActiveSiteID() / "cache.xml";

    if( CFileSystem::IsFile(config_path) ) {

        CXMLParser xml_parser;
        xml_parser.SetOutputXMLNode(&Cache);

        if( xml_parser.Parse(config_path) == false ) {
            CSmallString error;
            error << "unable to load site '" << GlobalConfig.GetActiveSiteID() << "' cache";
            ES_ERROR(error);
            return(false);
        }
    } else {
        Cache.CreateChildElement("cache");
        ES_WARNING("cache not found - it is created");
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CCache::GetModule(const CSmallString& name)
{
    CXMLElement* p_mele;

    CSmallString name_only = CUtils::GetModuleName(name);

    p_mele = Cache.GetFirstChildElement("cache");
    if( p_mele == NULL ) {
        return(NULL);
    }

    CXMLIterator    I(p_mele);
    CXMLElement*     p_ele;

    while( (p_ele = I.GetNextChildElement("module")) != NULL ) {
        CSmallString lname;
        p_ele->GetAttribute("name",lname);
        if( lname == name_only ) return(p_ele);
    }

    return(NULL);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleVersion(CXMLElement* p_module,const CSmallString& ver)
{
    if( p_module == NULL ) return(false);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        p_sele->GetAttribute("ver",lver);
        if( lver == ver ) return(true);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleArchitecture(CXMLElement* p_module,
        const CSmallString& ver,
        const CSmallString& arch)
{
    if( p_module == NULL ) return(false);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        CSmallString larch;
        p_sele->GetAttribute("ver",lver);
        p_sele->GetAttribute("arch",larch);
        if( (larch == arch) && (lver == ver) ) return(true);
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CCache::CheckModuleMode(CXMLElement* p_module,
        const CSmallString& ver,
        const CSmallString& arch,
        const CSmallString& mode)
{
    if( p_module == NULL ) return(false);

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*     p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString lver;
        CSmallString larch;
        CSmallString lmode;
        p_sele->GetAttribute("ver",lver);
        p_sele->GetAttribute("arch",larch);
        p_sele->GetAttribute("mode",lmode);
        if( (lmode == mode)&&(larch == arch)&&(lver == ver) ) return(true);
    }

    return(false);
}

//------------------------------------------------------------------------------

bool CCache::DoesItNeedGPU(const CSmallString& name)
{
    CXMLElement* p_module = GetModule(name);
    if( p_module == NULL ){
        CSmallString warning;
        warning << "module '" << name << "'' was not found in the cache";
        ES_WARNING(warning);
        return(false);
    }

    CXMLElement* p_list = p_module->GetFirstChildElement("builds");

    CXMLIterator    I(p_list);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("build")) != NULL ) {
        CSmallString larch;
        p_sele->GetAttribute("arch",larch);
        if( larch.FindSubString("gpu") >= 0 ){
            return(true);
        }
    }

    return(false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CXMLElement* CCache::GetRootElementOfCache(void)
{
    CXMLElement* p_mele;
    p_mele = Cache.GetFirstChildElement("cache");
    return(p_mele);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
