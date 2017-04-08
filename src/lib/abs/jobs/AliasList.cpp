// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
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

#include <AliasList.hpp>
#include <SimpleIterator.hpp>
#include <XMLDocument.hpp>
#include <XMLParser.hpp>
#include <XMLPrinter.hpp>
#include <XMLIterator.hpp>
#include <Shell.hpp>
#include <FileSystem.hpp>
#include <ErrorSystem.hpp>
#include <ABSConfig.hpp>

using namespace std;

//------------------------------------------------------------------------------

CAliasList AliasList;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CAliasList::CAliasList(void)
{

}

//------------------------------------------------------------------------------

CAliasList::~CAliasList(void)
{
    ClearAllAliases();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAliasList::LoadConfig(void)
{
    bool result = true;
    // first load user aliases
    result &= LoadUserConfig();

    // then load system aliases
    result &= LoadSystemConfig();

    return(result);
}

//------------------------------------------------------------------------------

bool CAliasList::LoadSystemConfig(void)
{
    CFileName      config_name;

    config_name = ABSConfig.GetSystemSiteConfigDir();

    if( CFileSystem::IsFile(config_name) == false ){
        CSmallString warning;
        warning << "system aliases '" << config_name << "' does not exist";
        ES_WARNING(warning);
        return(true);
    }

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&xml_doc);

    // load system setup ---------------------------
    if( xml_parser.Parse(config_name) == false ){
        ES_ERROR("unable to load system aliases");
        return(false);
    }

    // load aliases
    CXMLElement* p_ele = xml_doc.GetFirstChildElement("aliases");
    return( LoadAliases(p_ele,true) );

    return(true);
}

//------------------------------------------------------------------------------

bool CAliasList::LoadUserConfig(void)
{
    CFileName      config_name;
    config_name = ABSConfig.GetUserSiteConfigDir() / "aliases.xml";

    // is file?
    if( CFileSystem::IsFile(config_name) == false ){
        CSmallString warning;
        warning << "user aliases file '" << config_name << "' does not exist";
        ES_WARNING(warning);
        return(true);
    }

    CXMLDocument    xml_doc;
    CXMLParser      xml_parser;
    xml_parser.SetOutputXMLNode(&xml_doc);

    // load system setup ---------------------------
    if( xml_parser.Parse(config_name) == false ){
        ES_ERROR("unable to load user aliases");
        return(false);
    }

    // load aliases
    CXMLElement* p_ele = xml_doc.GetFirstChildElement("aliases");
    return( LoadAliases(p_ele,false) );
}

//------------------------------------------------------------------------------

bool CAliasList::SaveUserConfig(void)
{
    CFileName      config_name;
    config_name = ABSConfig.GetUserSiteConfigDir() / "aliases.xml";

    CXMLDocument    xml_doc;

    // save aliases
    CXMLElement* p_ele = xml_doc.CreateChildElement("aliases");
    if( p_ele == NULL ){
        ES_ERROR("unable to create aliases element");
        return(false);
    }

    if( SaveUserAliases(p_ele) == false ){
        ES_ERROR("unable to save aliases");
        return(false);
    }

    CXMLPrinter xml_printer;
    xml_printer.SetPrintedXMLNode(&xml_doc);

    // load system setup ---------------------------
    if( xml_printer.Print(config_name) == false ){
        ES_ERROR("unable to save user aliases");
        return(false);
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAliasList::LoadAliases(CXMLElement* p_ele,bool system)
{
    if( p_ele == NULL ){
        ES_ERROR("p_ele is NULL");
        return(false);
    }
    if( p_ele->GetName() != "aliases" ){
        ES_ERROR("p_ele is not <aliases>");
        return(false);
    }

    CXMLIterator    I(p_ele);
    CXMLElement*    p_sele;

    while( (p_sele = I.GetNextChildElement("alias")) != NULL ){
        CAliasPtr alias(new CAlias);
        if( alias->Load(p_sele,system) == false ){
            ES_ERROR("unable to load the alias");
            return(false);
        }
        if( system == true ){
            if( FindAlias(alias->GetName()) == NULL ){
                // insert to the list only alias that does not collide with user one
                push_back(alias);
            }
        } else {
            // user alias is always add to the list
            push_back(alias);
        }
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CAliasList::SaveUserAliases(CXMLElement* p_ele)
{
    if( p_ele == NULL ){
        ES_ERROR("p_ele is NULL");
        return(false);
    }
    if( p_ele->GetName() != "aliases" ){
        ES_ERROR("p_ele is not <aliases>");
        return(false);
    }

    std::list<CAliasPtr>::iterator it = begin();
    std::list<CAliasPtr>::iterator et = end();

    while( it != et ){
        if( ! (*it)->IsSystemAlias() ){
            CXMLElement* p_sele = p_ele->CreateChildElement("alias");
            if( p_sele == NULL ){
                ES_ERROR("unable to create alias element");
                return(false);
            }
            (*it)->Save(p_sele);
        }
        it++;
    }

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CAliasList::AddAlias(std::ostream& sout,bool expertmode,const CSmallString& name,const CSmallString& queue,
                          const CSmallString& resources)
{
    // remove previous occurences
    RemoveAlias(name);

    // add new alias
    CAliasPtr alias(new CAlias(name,queue,resources));
    if( alias->TestAlias(sout,expertmode) == false ) return(false);
    push_back(alias);
    return(true);
}

//------------------------------------------------------------------------------

class has_same_name{
public:
    has_same_name(const CSmallString& name)
        : Name(name){};

    bool operator()(const CAliasPtr& alias)
    { return( alias->GetName() == Name ); };

    CSmallString Name;
};

//------------------------------------------------------------------------------

void CAliasList::RemoveAlias(const CSmallString& name)
{
    remove_if(has_same_name(name));
}

//------------------------------------------------------------------------------

void CAliasList::ClearAllAliases(void)
{
    clear();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CAliasList::PrintInfos(std::ostream& sout)
{
    sout << "#      Name        Destination         Resources        " << endl;
    sout << "# -------------- ---------------- --------------------- " << endl;

    std::list<CAliasPtr>::iterator it = begin();
    std::list<CAliasPtr>::iterator et = end();

    while( it != et ){
        (*it)->PrintLineInfo(sout);
        it++;
    }
}

//------------------------------------------------------------------------------

void CAliasList::GetAliasesSuggestions(std::list<CSmallString>& suggestions)
{
    std::list<CAliasPtr>::iterator it = begin();
    std::list<CAliasPtr>::iterator ie = end();

    while( it != ie ){
        suggestions.push_back((*it)->GetName());
        it++;
    }
}

//------------------------------------------------------------------------------

CAliasPtr CAliasList::FindAlias(const CSmallString& name)
{
    std::list<CAliasPtr>::iterator it = begin();
    std::list<CAliasPtr>::iterator et = end();

    while( it != et ){
        if( (*it)->GetName() == name ) return(*it);
        it++;
    }

    return(CAliasPtr());
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

