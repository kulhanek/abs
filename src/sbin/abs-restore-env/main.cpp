// =============================================================================
// Infinity - Uniform Job Management for Scientific Calculations
// -----------------------------------------------------------------------------
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

#include <stdio.h>
#include "EnvRestore.h"
#include <ShellProcessor.h>

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int main(int argc, char* argv[])
{
 CEnvRestore env_restore;

 int result = env_restore.Init(argc,argv);

 switch(result){
    case SO_EXIT:
        ShellProcessor.SetExitCode(0);
        break;
    case SO_CONTINUE:
        if( env_restore.Run() == false ){
            ShellProcessor.RollBack();
            ShellProcessor.SetExitCode(1);
            }
        env_restore.Finalize();
        break;
    case SO_USER_ERROR:
        env_restore.Finalize();
        ShellProcessor.SetExitCode(2);
        break;
    case SO_OPTS_ERROR:
    default:
        ShellProcessor.SetExitCode(3);
        break;
    }

 ShellProcessor.BuildEnvironment();

 return(-1);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================
