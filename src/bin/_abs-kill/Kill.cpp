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

#include "Kill.hpp"
#include <ErrorSystem.hpp>
#include <SmallTimeAndDate.hpp>
#include <PluginDatabase.hpp>
#include <GlobalConfig.hpp>
#include <Torque.hpp>
#include <ABSConfig.hpp>
#include <ShellProcessor.hpp>

using namespace std;

//------------------------------------------------------------------------------

MAIN_ENTRY(CKill)

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

CKill::CKill(void)
{
}

//==============================================================================
//------------------------------------------------------------------------------
//==============================================================================

int CKill::Init(int argc,char* argv[])
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
    vout << "# pkill (ABS utility) started at " << dt.GetSDateAndTime() << endl;
    vout << "# ==============================================================================" << endl;

    return(SO_CONTINUE);
}

//------------------------------------------------------------------------------

bool CKill::Run(void)
{
    // init all subsystems
    if( ABSConfig.LoadSystemConfig() == false ){
        ES_ERROR("unable to load ABSConfig config");
        return(false);
    }

    PluginDatabase.SetPluginPath(GlobalConfig.GetPluginsDir());
    if( PluginDatabase.LoadPlugins(GlobalConfig.GetPluginsConfigDir()) == false ){
        ES_ERROR("unable to load plugins");
        return(false);
    }

    vout << low;

    // check if user has valid ticket
    if( ABSConfig.IsUserTicketValid(vout) == false ){
        ES_TRACE_ERROR("user does not have valid ticket");
        return(false);
    }

    if( Torque.Init() == false ){
        ES_ERROR("unable to init torque");
        return(false);
    }

    // get list of info files
    if( Options.GetNumberOfProgArgs() > 0 ){
        for( int i=0; i < Options.GetNumberOfProgArgs(); i++ ){
            if( Jobs.AddJob(Options.GetProgArg(i)) == false ){
                vout << endl;
                vout << "<b><red> ERROR: Unable to load job info file '" << Options.GetProgArg(i) << "'!</red></b>" << endl;
                vout << endl;
                return(false);
            }
        }

    } else {
        Jobs.InitByInfoFiles(".",Options.GetOptRecursive());
    }

    if( Jobs.GetNumberOfJobs() == 0 ){
        vout << endl;
        vout << "<b><red> ERROR: No job info files were found!</red></b>" << endl;
        vout << endl;
        return(false);
    }

    Jobs.UpdateJobStatuses();
    // for soft kill all jobs must be running
    Jobs.KeepOnlyLiveJobs(Options.GetOptSoft() == false);

    if( Jobs.GetNumberOfJobs() == 0 ){
        if( Options.GetOptSoft() == true ){
            vout << endl;
            vout << "<b><red> ERROR: No running jobs were found for soft job termination!</red></b>" << endl;
            vout << endl;
            return(false);
        } else {
            vout << endl;
            vout << "<b><red> ERROR: No submitted or running jobs were found!</red></b>" << endl;
            vout << endl;
            return(false);
        }
    }

    // full job info
    Jobs.PrintInfos(vout);
    vout << endl;

    vout << ">>> Satisfying job(s) for pkill action ..." << endl;

    Jobs.PrintInfosCompact(vout,true,true);
    vout << endl;

    if( Options.GetOptSoft() == true ){
        if( Jobs.GetNumberOfJobs() != 1 ){
            vout << endl;
            vout << "<b><red> ERROR: Only one job can be selected for soft job termination!</red></b>" << endl;
            vout << endl;
            return(false);
        }
    }

    if( ! Options.GetOptAssumeYes() ){
        // ask if jobs should be killed
        if( Options.GetOptSoft() == true ) {
            vout << "<b><red>Do you want to softly kill listed job (YES/NO)?</red></b>" << endl;
        } else {
            vout << "<b><red>Do you want to kill listed jobs (YES/NO)?</red></b>" << endl;
        }
        vout << "> ";

        string answer;
        cin >> answer;

        CSmallString sanswer(answer.c_str());
        sanswer.ToUpperCase();

        if( sanswer != "YES" ){
            vout << "No jobs were killed!" << endl;
            vout << endl;
            return(true);
        }
    }

    if( Options.GetOptSoft() == true ) {
        // kill softly the job
        Jobs.KillJobSoftly(vout);
    } else {
        // kill all jobs
        Jobs.KillAllJobs();
        vout << "<b><red>Listed jobs were killed!</red></b>" << endl;
        vout << endl;

        // save info files
        Jobs.SaveAllInfoFiles();
    }

    return(true);
}

//------------------------------------------------------------------------------

bool CKill::Finalize(void)
{
    // unload plugins
    PluginDatabase.UnloadPlugins();

    if( ! ErrorSystem.IsError() ){
        ShellProcessor.SetExitCode(0);
    } else {
        ShellProcessor.RollBack();
        ShellProcessor.SetExitCode(1);
    }

    CSmallTimeAndDate dt;
    dt.GetActualTimeAndDate();

    vout << high;
    vout << "# ==============================================================================" << endl;
    vout << "# pkill terminated at " << dt.GetSDateAndTime() << endl;
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

