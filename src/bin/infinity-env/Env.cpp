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

#include "Env.hpp"
#include <ErrorSystem.hpp>
#include <Shell.hpp>
#include <unistd.h>

//------------------------------------------------------------------------------

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CEnv)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CEnv::CEnv(void)
{
    tout.Attach(stderr);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CEnv::Init(int argc,char* argv[])
{
    // encode program options
    int result = Options.ParseCmdLine(argc,argv);
    return(result);
}

//------------------------------------------------------------------------------

bool CEnv::Run(void)
{
    // is it Infinity job?
    if( CShell::GetSystemVariable("_INFINITY_JOB_") != "_INFINITY_JOB_" ){

        tout << endl;
        tout << " <b><red><b>ERROR: This script can be run as an Infinity job only!</b></red></b>" << endl;
        tout << endl;
        tout << "        The job script is protected by the <b>infinity-env</b> command," << endl;
        tout << "        which allows execution of the script as an Infinity batch job only." << endl;
        tout << "        To submit your job into the batch system, use the <b>psubmit</b> command." << endl;

        return(false);
    }

    // prepare arguments
    int    nargs = Options.GetNumberOfProgArgs() + 2;
    char** args;

    args = (char**)malloc(sizeof(char*)*nargs);
    if( args == NULL ){
        ES_ERROR("unable to allocate args");
        return(false);
    }

    args[0] = strdup("bash");
    for(int i=0; i < Options.GetNumberOfProgArgs(); i++){
        args[i+1] = strdup(Options.GetProgArg(i));
    }
    args[nargs-1] = NULL;

    if( Options.GetOptVerbose() ){
        printf("nargs %d\n",Options.GetNumberOfProgArgs());
        for(int i=0; i < nargs; i++){
            printf("arg   %d : %s\n",i,args[i]);
            }
    }

    int ret = execvp(args[0],args);

    for(int i=0; i < nargs; i++){
        if( args[i] != NULL ) free(args[i]);
    }
    free(args);

    return(ret == 0);
}

//------------------------------------------------------------------------------

void CEnv::Finalize(void)
{
    if( Options.GetOptVerbose() || ErrorSystem.IsError() ) {
        ErrorSystem.PrintErrors(stderr);
    }

    tout << endl;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

