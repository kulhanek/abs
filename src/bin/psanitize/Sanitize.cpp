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

#include "Sanitize.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <PluginDatabase.hpp>
#include <GlobalConfig.hpp>
#include <Torque.hpp>
#include <TorqueConfig.hpp>
#include <Shell.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CSanitize)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CSanitize::CSanitize(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CSanitize::Init(int argc,char* argv[])
{
// encode program options, all check procedures are done inside of CABFIntOpts
    int result = Options.ParseCmdLine(argc,argv);

// should we exit or was it error?
    if(result != SO_CONTINUE) return(result);

// attach verbose stream to terminal stream and set desired verbosity level
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
    vout << "# psanitize (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CSanitize::Run(void)
{    
    // init all subsystems
    if( TorqueConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load torque config");
        return(false);
    }

    PluginDatabase.SetPluginPath(GlobalConfig.GetPluginsDir());
    if( PluginDatabase.LoadPlugins(GlobalConfig.GetPluginsConfigDir()) == false ){
        ES_ERROR("unable to load plugins");
        return(false);
    }

    vout << low;

    // check if user has valid ticket
    if( TorqueConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    if( Torque.Init() == false ){
        ES_ERROR("unable to init torque");
        return(false);
    }

    // is it Infinity job?
    if( CShell::GetSystemVariable("_INFINITY_JOB_") == "_INFINITY_JOB_" ){
        Jobs.InitFromRunningJob();
    } else {
        // get list of info files
        Jobs.InitByInfoFiles(".",false);
    }

    // sort them
    Jobs.SortByPrepareDateAndTime();

    // update status of live jobs
    Jobs.UpdateJobStatuses();

    // keep running or submitted jobs
    Jobs.KeepOnlyLiveJobs();

    if( Jobs.GetNumberOfJobs() == 0 ){
        vout << endl;
        vout << "<b><blue> WARNING: No suitable runtime information are available, skipping ...</blue></b>" << endl;
        vout << endl;
        return(true);
    }

    // do we have one running job?
    if( Jobs.GetNumberOfJobs() != 1 ){
        vout << endl;
        vout << "<b><red> ERROR: Exactly one job is expected in running or submitted state!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    // print running job info
    Jobs.PrintInfos(vout);


    CJobPtr p_job = Jobs.front();

    // overwrite job input data
    p_job->SetItem("basic/arguments","INF_ARG_JOB",Options.GetArgJobInputName());

    // detect job input and potentially update its status
    vout << endl;
    vout << "# ------------------------------------------------------------------------------" << endl;
    vout << "# Job input file:  " << Options.GetArgJobInputName() << endl;
    if( p_job->DetectJobType(vout,Options.GetOptJobType()) != ERS_OK ){
        return(false);
    }
    vout << "# Autodetected as: " << p_job->GetItem("basic/jobinput","INF_JOB_TYPE") <<  endl;
    vout << "# ------------------------------------------------------------------------------" << endl;
    p_job->LastJobCheck(vout);
    vout << endl;

    return(true);
}

//------------------------------------------------------------------------------

bool CSanitize::Finalize(void)
{
    // unload plugins
    PluginDatabase.UnloadPlugins();

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << endl;
    vout << "# ==============================================================================" << endl;
    vout << "# psanitize terminated at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    if( ErrorSystem.IsError() || Options.GetOptVerbose() ){
        vout << low;
        ErrorSystem.PrintErrors(vout);
    }

    vout << endl;

    return(true);
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

