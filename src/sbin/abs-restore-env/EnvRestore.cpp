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

#include "EnvRestore.hpp"
#include <ErrorSystem.hpp>
#include <ShellProcessor.hpp>
#include <TerminalStr.hpp>

//------------------------------------------------------------------------------

MAIN_ENTRY(CEnvRestore)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CEnvRestore::CEnvRestore(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CEnvRestore::Init(int argc,char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);
    return(result);
}

//------------------------------------------------------------------------------

bool CEnvRestore::Run(void)
{
    if( Job.LoadInfoFile(Options.GetArgInfoFile()) == false ){
        ES_ERROR("unable to load info file");
        return(false);
    }

    Job.RestoreEnv();

    return(true);
}

//------------------------------------------------------------------------------

void CEnvRestore::Finalize(void)
{
    if( ! ErrorSystem.IsError() ){
        ShellProcessor.SetExitCode(0);
    } else {
        ShellProcessor.RollBack();
        ShellProcessor.SetExitCode(1);
    }

    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }

    ShellProcessor.BuildEnvironment();
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================


