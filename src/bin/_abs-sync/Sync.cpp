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

#include "Sync.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <PluginDatabase.hpp>
#include <ABSConfig.hpp>
#include <ShellProcessor.hpp>
#include <FileSystem.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CSync)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSync::CSync(void)
{
    ExitCode = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSync::Init(int argc,char* argv[])
{
// encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    if(result != SO_CONTINUE) return(result);

// attach verbose stream to terminal stream and set desired verbosity level
    // all output to stderr
    Console.Attach(stderr);

    vout.Attach(Console);
    if( Options.GetOptVerbose() ) {
        vout.Verbosity(CVerboseStr::high);
    } else {
        vout.Verbosity(CVerboseStr::low);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# psync (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CSync::Run(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;
    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        vout << endl;        
        ES_TRACE_ERROR("user does not have valid ticket");
        ExitCode = 1;
        return(false);
    }

    bool result = true;

    // no arguments - always by info file
    result = SyncByInfoFiles();
    if( result == false ){
        ES_ERROR("psync cannot be used");
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CSync::SyncByInfoFiles(void)
{
    Jobs.InitByInfoFiles(".",false);

    if( Jobs.GetNumberOfJobs() == 0 ){
        vout << endl;
        vout << "<b><red> ERROR: No job info files were found!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    // update status of live jobs
    Jobs.UpdateJobStatuses();

    // print final information
    vout << endl;

    // analyze jobs for psync action
    if( Jobs.IsSyncActionPossible(vout) == false ){
        ES_ERROR("psync action is not possible");
        return(false);
    }

    // satisfying job
    vout << ">>> Satisfying job for the psync action ..." << endl;
    Jobs.PrintInfosCompact(vout,false,true);
    vout << endl;

    // get last job and prepare environment for psync command
    Jobs.PrepareSyncWorkingDirEnv();

    return(true);
}

//------------------------------------------------------------------------------

bool CSync::Finalize(void)
{
    if( Options.GetOptSyncAll() ){
        ShellProcessor.SetVariable("INF_SYNC_ALL","YES");
    }

    if( ErrorSystem.IsError() || (ExitCode > 0) ){
        ShellProcessor.RollBack();
        ShellProcessor.SetExitCode(1);        
    } else {
        ShellProcessor.SetExitCode(0);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# psync terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( Options.GetOptVerbose() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;

    ShellProcessor.BuildEnvironment();

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

