// =============================================================================
//  ABS - Advanced Batch System
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

#include <XMLElement.hpp>
#include <XMLIterator.hpp>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ShellProcessor.hpp>
#include <ErrorSystem.hpp>

//------------------------------------------------------------------------------

CShellProcessor ShellProcessor;

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CShellProcessor::CShellProcessor(void)
{
    ExitCode = 0;
    ShellActions.CreateChildElement("actions");
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetVariable(const CSmallString& name,
                                    const CSmallString& value)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    // and create new variable
    CXMLElement* p_sele = p_ele->CreateChildElement("variable");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("remove",false);
    p_sele->SetAttribute("value",value);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::UnsetVariable(const CSmallString& name)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("variable");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("remove",true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetAlias(const CSmallString& name,const CSmallString& value)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("alias");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("value",value);
    p_sele->SetAttribute("remove",false);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::UnsetAlias(const CSmallString& name)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("alias");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("remove",true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::RegisterScript(const CSmallString& name,
        const CSmallString& args,
        EScriptType type)
{
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement* p_sele = p_ele->CreateChildElement("script");

    p_sele->SetAttribute("name",name);
    p_sele->SetAttribute("type",(int)type);
    p_sele->SetAttribute("args",args);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

void CShellProcessor::SetExitCode(int exitcode)
{
    ExitCode = exitcode;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

bool CShellProcessor::RollBack(void)
{
    ExitCode = 0;
    ShellActions.RemoveAllChildNodes();
    ShellActions.CreateChildElement("actions");
    return(true);
}

//------------------------------------------------------------------------------

void CShellProcessor::BuildEnvironment(void)
{
    CSmallString exit_code;

    exit_code.IntToStr(ExitCode);

    // set exit code variable
    SetVariable("ABS_EXIT_CODE",exit_code);

    // build environment
    CXMLElement* p_ele = ShellActions.GetFirstChildElement("actions");
    if( p_ele == NULL ){
        LOGIC_ERROR("p_ele is NULL");
    }

    CXMLElement*     p_sele;
    CXMLIterator    I(p_ele);

    while( (p_sele = I.GetNextChildElement()) != NULL ) {

        if( p_sele->GetName() == "variable" ) {
            CSmallString     name;
            CSmallString     value;
            bool            remove = false;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("remove",remove);
            p_sele->GetAttribute("value",value);
            if( remove == false ) {
                if( name != NULL ) {
                    if( value != NULL ) {
                        printf("export %s=\"%s\";\n",(const char*)name,(const char*)value);
                    } else {
                        printf("export %s=\"\";\n",(const char*)name);
                    }
                }
            } else {
                if( name != NULL ) {
                    printf("unset %s;\n",(const char*)name);
                }
            }
        }

        if( p_sele->GetName() == "script" ) {
            CSmallString     name;
            CSmallString    args;
            int             type;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("type",type);
            p_sele->GetAttribute("args",args);
            if( name != NULL ) {
                switch((EScriptType)type) {
                case EST_INLINE:
                    if( args != NULL ) {
                        printf("source \"%s\" %s;\n",(const char*)name,(const char*)args);
                    } else {
                        printf("source \"%s\";\n",(const char*)name);
                    }
                    break;
                case EST_CHILD:
                default:
                    if( args != NULL ) {
                        printf("\"%s\" %s;\n",(const char*)name,(const char*)args);
                    } else {
                        printf("\"%s\";\n",(const char*)name);
                    }
                    break;
                }
            }
        }

        if( p_sele->GetName() == "alias" ) {
            CSmallString     name;
            CSmallString     value;
            bool            remove = false;
            p_sele->GetAttribute("name",name);
            p_sele->GetAttribute("remove",remove);
            p_sele->GetAttribute("value",value);
            if( remove == false ) {
                if( (name != NULL) && (value != NULL) ) {
                    if( value != NULL ) {
                        printf("alias %s=\"%s\";\n",(const char*)name,(const char*)value);
                    }
                }
            } else {
                if( name != NULL ) {
                    printf("unalias %s &> /dev/null;\n",(const char*)name);
                }
            }
        }

    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================






