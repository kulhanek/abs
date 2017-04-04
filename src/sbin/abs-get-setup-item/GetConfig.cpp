// =============================================================================
// ABS - Advanced Batch System
// -----------------------------------------------------------------------------
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

#include "GetConfig.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>
#include <ABSConfig.hpp>

//------------------------------------------------------------------------------

MAIN_ENTRY(CGetConfig)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGetConfig::CGetConfig(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CGetConfig::Init(int argc,char* argv[])
{
// encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    return(result);
}

//------------------------------------------------------------------------------

bool CGetConfig::Run(void)
{
    // load system and optionaly user configuration
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load system config");
        return(false);
    }
    if( Options.GetOptUser() == true ){
        if( ABSConfig.LoadUserConfig() == false ){
            ES_WARNING("unable to load user config");
            }
    }

    CSmallString value;

    bool found = false;
    // print configuration item
    if( Options.GetOptUser() == true ){
        // first try user config item
        if( ABSConfig.GetUserConfigItem(Options.GetArgConfigItem(),value) == true ){
            found = true;
        } else {
            ES_WARNING("unable to get user config");
        }
    }
    if( found == false ){
        if( ABSConfig.GetSystemConfigItem(Options.GetArgConfigItem(),value) == false ){
            ES_ERROR("unable to get system config");
            return(false);
            }
    }

    // print item
    std::cout << value;

    return(true);
}

//------------------------------------------------------------------------------

void CGetConfig::Finalize(void)
{
    if( Options.GetOptVerbose() ) {
        ErrorSystem.PrintErrors(stderr);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

