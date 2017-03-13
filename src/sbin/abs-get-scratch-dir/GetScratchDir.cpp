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

#include "GetScratchDir.hpp"
#include <ErrorSystem.hpp>
#include <TerminalStr.hpp>
#include <TorqueConfig.hpp>

//------------------------------------------------------------------------------

MAIN_ENTRY(CGetScratchDir)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CGetScratchDir::CGetScratchDir(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CGetScratchDir::Init(int argc,char* argv[])
{
// encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    return(result);
}

//------------------------------------------------------------------------------

bool CGetScratchDir::Run(void)
{
    // load system and optionaly user configuration
    if( TorqueConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load system config");
        return(false);
    }

    CSmallString value = TorqueConfig.GetScratchDir(Options.GetArgScratchType(),Options.GetOptMode());
    std::cout << value;

    return(true);
}

//------------------------------------------------------------------------------

void CGetScratchDir::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

