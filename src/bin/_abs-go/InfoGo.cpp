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

#include "InfoGo.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <ABSConfig.hpp>
#include <ShellProcessor.hpp>
#include <FileSystem.hpp>
#include <BatchServers.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CInfoGo)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CInfoGo::CInfoGo(void)
{
    ExitCode = 0;
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CInfoGo::Init(int argc,char* argv[])
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
    vout << "# pgo (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CInfoGo::Run(void)
{
    if( ABSConfig.IsABSAvailable(vout) == false ){
        ES_TRACE_ERROR("abs not configured or available");
        return(false);
    }

    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    vout << low;

    // user must be initializaed before ABSConfig.IsUserTicketValid()
    User.InitUser();

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        vout << endl;        
        ES_TRACE_ERROR("user does not have valid ticket");
        ExitCode = 1;
        return(false);
    }

    bool result = true;

    // determine if it is job ID or job info file
    if( Options.GetNumberOfProgArgs() > 0 ){
        // is it file?
        CFileName info_file;
        info_file = Options.GetProgArg(0);
        if( info_file.GetFileNameExt() != ".info" ){
            info_file = info_file + ".info";
        }

        if( CFileSystem::IsFile(info_file) == true ){
            result = GoByInfoFiles();
        } else {
            result = GoByJobID();
        }

    } else {
        // no arguments - always by info file
        result = GoByInfoFiles();
    }

    return(result);
}

//------------------------------------------------------------------------------

bool CInfoGo::GoByInfoFiles(void)
{
    // get list of info files
    if( Options.GetNumberOfProgArgs() > 0 ){
        // actually only one can be loaded - see int CInfoGoOptions::CheckArguments(void)
        if( JobList.AddJob(Options.GetProgArg(0)) == false ){
            vout << endl;
            vout << "<b><red> ERROR: Unable to load job info file '" << Options.GetProgArg(0) << "'!</red></b>" << endl;
            vout << endl;
            return(false);
        }
    } else {
        JobList.InitByInfoFiles(".",false);
    }

    if( JobList.GetNumberOfJobs() == 0 ){
        vout << endl;
        vout << "<b><red> ERROR: No job info files were found!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    // update status of live jobs
    JobList.UpdateJobStatuses();
    JobList.SortByPrepareDateAndTime();

    // print final information
    vout << endl;

    if( Options.GetOptForce() == false ) {
        // analyze jobs for pgo action
        if( JobList.IsGoActionPossible(vout,false,Options.GetOptProxy()) == false ){

            if( JobList.GetNumberOfJobs() != 1 ) return(false); // no suitable job at all

            // we have one job
            if( Options.GetOptNoWait() == true ) return(false); // we cannot wait

            // we must wait until is in good conditions
            if( JobList.WaitForRunningJob(vout) == false ){
                ES_TRACE_ERROR("pgo action is not possible");
                return(false);
            }
        }
        // we have one suitable job
    } else {
        // preprocess jobs but ignore final result - only exit if more than one job remains
        JobList.IsGoActionPossible(vout,true,false);

        if( JobList.GetNumberOfJobs() != 1 ) return(false); // no suitable job at all

        vout << "<b><blue> WARNING: The pgo command is forced (--force)!</blue></b>" << endl;
        vout << "<b><blue>          The success is not guaranteed!</blue></b>" << endl;
        vout << endl;
    }

    // satisfying job
    vout << ">>> The job suitable for pgo ..." << endl;
    JobList.PrintInfosCompact(vout,false,true);
    vout << endl;

    // prepare environment for pgo command
    return(JobList.PrepareGoWorkingDirEnv(vout,Options.GetOptNoTerminal()));
}

//------------------------------------------------------------------------------

bool CInfoGo::GoByJobID(void)
{
    BatchServers.GetJob(JobList,Options.GetProgArg(0));

    if( JobList.GetNumberOfJobs() == 0 ){
        vout << endl;
        vout << "<b><red> ERROR: No job with the specified job ID was found!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    // print info about jobs
    JobList.PrintBatchInfo(vout,true,true);

    // prepare environment for pgo command
    return(JobList.PrepareGoInputDirEnv(vout));
}

//------------------------------------------------------------------------------

bool CInfoGo::Finalize(void)
{
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
    vout << "# pgo terminated at " << dt.GetSDateAndTime() << endl;
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

