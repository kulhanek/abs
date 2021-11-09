// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
//    Copyright (C) 2021      Petr Kulhanek, kulhanek@chemi.muni.cz
//    Copyright (C) 2012      Petr Kulhanek, kulhanek@chemi.muni.cz
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

#include "GetWorkDir.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>
#include <ABSConfig.hpp>
#include <XMLElement.hpp>
#include <ErrorSystem.hpp>

//------------------------------------------------------------------------------

MAIN_ENTRY(CGetWorkDir)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGetWorkDir::CGetWorkDir(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CGetWorkDir::Init(int argc,char* argv[])
{
// encode program options
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    return(result);
}

//------------------------------------------------------------------------------

bool CGetWorkDir::Run(void)
{
    // load system and optionaly user configuration
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load system config");
        return(false);
    }

    CXMLElement* p_sele = ABSConfig.GetWorkDirConfig();

    CXMLElement* p_wele = p_sele->GetFirstChildElement("workdir");
    while( p_wele != NULL ){
        CSmallString name;
        p_wele->GetAttribute("name",name);
        if( name == Options.GetArgWorkDir() ) break;
        p_wele = p_wele->GetNextSiblingElement("workdir");
    }

    CXMLElement* p_mele = NULL;

    if( p_wele ){
        p_mele = p_wele->GetFirstChildElement(Options.GetOptMode());
    }

    CSmallString value;

    if( p_mele != NULL ){
        if( Options.GetOptMode() == "main" ){
            p_mele->GetAttribute("path",value);
        } else if ( Options.GetOptMode() == "tmp" ){
            p_mele->GetAttribute("path",value);
        } else if ( Options.GetOptMode() == "ijob" ){
            p_mele->GetAttribute("path",value);
        } else if ( Options.GetOptMode() == "clean" ){
            p_mele->GetAttribute("path",value);
        } else if ( Options.GetOptMode() == "preinit" ){
            p_mele->GetAttribute("script",value);
        } else {
            RUNTIME_ERROR("unsupported scratch mode " + Options.GetOptMode());
        }
    } else {
        // fallback defaults
        if( Options.GetOptMode() == "main" ){
            value = "/scratch/$USER/$INF_JOB_ID/main";
        } else if ( Options.GetOptMode() == "tmp" ){
            value = "/scratch/$USER/$INF_JOB_ID/tmp";
        } else if ( Options.GetOptMode() == "ijob" ){
            value = "/scratch/$USER/$INF_JOB_ID/$INF_IJOB_ID";
        } else if ( Options.GetOptMode() == "clean" ){
            value = "/scratch/$USER/$INF_JOB_ID";
        } else if ( Options.GetOptMode() == "preinit" ){
            value = "";
        } else {
            RUNTIME_ERROR("unsupported scratch mode " + Options.GetOptMode());
        }
    }

    std::cout << value;

    return(true);
}

//------------------------------------------------------------------------------

void CGetWorkDir::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

